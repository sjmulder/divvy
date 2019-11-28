divvy
=====

**divvy** **-n** *jobs* [**-L** lines] [**-d** *delim*] *program* ...

Divide input between multiple instances of a utility.

Description
-----------
Reads batches of *lines* (1 by default) lines or items delimited by
*delim* from standard input and sends them to up to *jobs* instances of
the given program in a round-robin fashion.

Exits 0 on success, 123 if a *program* exits with a nonzero status code,
or 1 if another error occurs.

Example
-------
Divide a SQL file between 4 Postgres clients, in batches of 50
statements:

    $ divvy -n4 -L50 -d\; psql <inserts.sql

The first client will receive the first 50 statements, the second the
next 50, and so on.

Running
-------
Should work with any Unix, including Linux and macOS. To compile,
install and uninstall from source:

    make
    make install   [DESTDIR=] [PREFIX=/usr/local] [MANPREFIX=PREFIX/man]
    make uninstall [DESTDIR=] [PREFIX=/usr/local] [MANPREFIX=PREFIX/man]

Author
------
Sijmen J. Mulder (<ik@sjmulder.nl>). See LICENSE.md.
