#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <unistd.h>
#include <err.h>

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
# include <stdnoreturn.h>
#else
# define noreturn __attribute__((noreturn))
#endif

#define MAX_JOBS 512

static noreturn void usage(void);
static int spawn_job(char **);
static void close_pipes(void);

static int pipes[MAX_JOBS];

int
main(int argc, char **argv)
{
	static char buf[4096];

	/* options */
	int num_jobs = -1;
	int batch_size = 1;
	char separator = '\n';
	
	int opt;
	int cur_job = 0;
	ssize_t num_read;
	size_t seps_left, len = 0, to_write;
	char *cursor, *sep_pos;
	int status, ret = 0;

#ifdef __OpenBSD__
	if (pledge("stdio proc exec", NULL) == -1)
		err(1, "pledge");
#endif

	while ((opt = getopt(argc, argv, "n:L:d:")) != -1) {
		switch (opt) {
		case 'n':
			num_jobs = (int)strtol(optarg, NULL, 10);
			if (num_jobs < 1 || num_jobs > MAX_JOBS)
				errx(1, "bad -n");
			break;
		case 'L':
			batch_size = (int)strtol(optarg, NULL, 10);
			if (batch_size < 1)
				errx(1, "bad -L");
			break;
		case 'd':
			if (strlen(optarg) != 1)
				errx(1, "bad -d");
			separator = optarg[0];
			break;
		default:
			usage();
		}
	}

	argc -= optind;
	argv += optind;

	if (num_jobs == -1 || !argc)
		usage();

	seps_left = batch_size;

	while (1) {
		/*
		 * Each iteration consits of one read (if needed) and
		 * one write to a job pipe.
		 */

		/* read data if we have none */
		if (!len) {
			num_read = read(0, buf, sizeof(buf));
			if (num_read == -1)
				err(1, "read");
			if (!num_read)
				break;
			
			cursor = buf;
			len = (size_t)num_read;
		}

		to_write = len;
		
		/*
		 * Count separators, subtracting from seps_left, the
		 * number of separators left in the current batch. When
		 * seps_left hits 0 we have a full batch, so cut off
		 * the write there and switch to the next job for the
		 * rest.
		 */
		sep_pos = cursor;
		while (1) {
			sep_pos = memchr(sep_pos, separator,
			    cursor + len - sep_pos);
			if (!sep_pos)
				break;
			sep_pos++;
			if (!(--seps_left)) {
				to_write = sep_pos - cursor;
				break;
			}
		}

		/* spawn jobs only as we need them */
		if (!pipes[cur_job])
			pipes[cur_job] = spawn_job(argv);
		
		if (write(pipes[cur_job], cursor, to_write) == -1)
			err(1, "write");

		/* end of batch so switch jobs */
		if (!seps_left) {
			if (++cur_job >= num_jobs)
				cur_job = 0;
			seps_left = batch_size;
		}

		len -= to_write;
		cursor += to_write;
	}

	/* if we don't the children may keep waiting for input */
	close_pipes();

	while (wait(&status) != -1)
		if (status)
			ret = 123;
	if (errno != ECHILD)
		err(1, "wait");

	return ret;
}

static noreturn void
usage(void)
{
	fprintf(stderr, "usage: divvy -n jobs [-L lines] "
	    "[-d delim] program ...\n");
	exit(1);
}

static int
spawn_job(char **argv)
{
	int fds[2];

	if (pipe(fds) == -1)
		err(1, "pipe");

	switch (fork()) {
	case -1:
		err(1, "fork");
	case 0:
		if (dup2(fds[0], 0) == -1)
			err(1, "dup2");
		
		/*
		 * Important to close unused file descriptors as to not
		 * leave the pipes open (and children waiting!).
		 */
		if (close(fds[0]) == -1)
			err(1, "close");
		if (close(fds[1]) == -1)
			err(1, "close");
		close_pipes();

		execvp(argv[0], argv);
		err(1, "%s", argv[0]);
	}

	if (close(fds[0]) == -1)
		err(1, "close");

	return fds[1];
}

static void
close_pipes(void)
{
	int i;

	for (i = 0; i < MAX_JOBS && pipes[i]; i++)
		if (close(pipes[i]) == -1)
			err(1, "close");
}
