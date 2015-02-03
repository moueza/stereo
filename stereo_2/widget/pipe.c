/* pipe.c - for opening a process as a pipe and reading both stderr and stdout together */
/*
   Copyright (C) 1997 Paul Sheer

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  
 */
#include <config.h>
#include "global.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#include <signal.h>
#ifdef HAVE_UNISTD_H
#   include <unistd.h>
#endif

#include "my_string.h"
#ifdef HAVE_SYS_SELECT_H
#  include <sys/select.h>
#endif
#include <errno.h>

#include "mad.h"

int data_read_ready (int f);
int data_read_wait (int f);
int data_write_ready (int f);
int data_write_wait (int f);
pid_t my_popen (int *in, int *out, int *err, int mix, const char *file, char *const argv[]);
char *read_pipe (int fd, int *len);

#undef min
#define min(x,y)     (((x) < (y)) ? (x) : (y))

/*
   This opens a process as a pipe. 'in', 'out' and 'err' are pointers to file handles
   which are filled in by popen. 'in' refers to stdin of the process, to
   which you can write. 'out' and 'err' refer to stdout and stderr of the process
   from which you can read. 'in', 'out' and 'err' can be passed
   as NULL if you want to ignore output or input of those pipes.
   If 'mix' is non-zero, then both stderr and stdout of the process
   can be read from 'out'. If mix is non-zero, then 'err' must be passed as NULL.
   Popen forks and then calls execvp (see execvp(3)) --- which must also take argv[0]
   and args must terminate with a NULL.
   Returns -1 if the fork failed, and -2 if pipe() failed.
   Otherwise returns the pid of the child.
 */
pid_t my_popen (int *in, int *out, int *err, int mix, const char *file, char *const argv[])
{
    pid_t p;
    int e;
    int f0[2], f1[2], f2[2];

    e = (pipe (f0) | pipe (f1) | pipe (f2));

    if (e) {
	close (f0[0]);
	close (f0[1]);
	close (f1[0]);
	close (f1[1]);
	close (f2[0]);
	close (f2[1]);
	return -2;
    }

    p = fork ();

    if (p == -1) {
	close (f0[0]);
	close (f0[1]);
	close (f1[0]);
	close (f1[1]);
	close (f2[0]);
	close (f2[1]);
	return -1;
    }
    if (p) {
	if (in) {
	    *in = f0[1];
	} else {
	    close (f0[1]);
	}
	if (out) {
	    *out = f1[0];
	} else {
	    close (f1[0]);
	}
	if (err) {
	    *err = f2[0];
	} else {
	    close (f2[0]);
	}
	close (f0[0]);
	close (f1[1]);
	close (f2[1]);
	return p;
    } else {
	int nulldevice;
	signal (SIGALRM, SIG_IGN);

	nulldevice = open ("/dev/null", O_WRONLY);

	close (0);
	dup (f0[0]);
	close (1);
	if (out)
	    dup (f1[1]);
	else
	    dup (nulldevice);
	close (2);
	if (err)
	    dup (f2[1]);
	else {
	    if (mix)
		dup (f1[1]);
	    else
		dup (nulldevice);
	}
	close (f0[0]);
	close (f0[1]);
	close (f1[0]);
	close (f1[1]);
	close (f2[0]);
	close (f2[1]);

	close (nulldevice);
	execvp (file, argv);
	exit (0);
    }
}


#define CHUNK 65536
#define MAX_BLOCKS 64

/*
   Reads all available data up to a maximum of 4MB and
   mallocs space for it plus one byte, and sets that byte
   to zero. If len is non-NULL len must point to the maximum
   number of bytes you would like to read, which, on return,
   will be replaced by the amount of bytes actually read.
   Returns NULL if memory not available and
   sets errno = ENOMEM. Returns NULL if error from read()
   call, in which case errno will be set as of read().
   Result must be free'd. This blocks waiting for data
   using data_read_wait() below, even if fd is normally
   non-blocking. Does not close() fd.
 */
char *read_pipe (int fd, int *len)
{
    char *blocks[MAX_BLOCKS], *t, *p;
    int i, j, count, l = 0, max_len;

    memset (blocks, 0, sizeof (char *) * MAX_BLOCKS);
    if (len)
	max_len = *len;
    else
	max_len = MAX_BLOCKS * CHUNK;

    for (i = 0; i < MAX_BLOCKS; i++) {
	blocks[i] = malloc (CHUNK);
	if (!blocks[i]) {
	    errno = ENOMEM;
	    goto freeall;
	}
	for (j = 0; j < CHUNK;) {
	    data_read_wait (fd);
	    count = read (fd, blocks[i] + j, min (CHUNK - j, max_len - l));
	    if (count < 1)
		break;
	    j += count;
	    if (l + j == max_len)
		break;
	}
	if (count == -1)
	    goto freeall;
	l += j;
	if (j < CHUNK)
	    break;
    }

    p = t = malloc (l + 1);
    if (!t) {
	errno = ENOMEM;
	goto freeall;
    }
    t[l] = 0;			/* can read data as a NULL-terminated string */

    if (len)
	*len = l;

    for (i = 0; i < MAX_BLOCKS && l > 0; i++, l -= CHUNK, p += CHUNK) {
	memcpy (p, blocks[i], min (l, CHUNK));
	free (blocks[i]);
    }
    if (blocks[i])
	free (blocks[i]);
    return t;

  freeall:
    for (i = 0; i < MAX_BLOCKS; i++)
	if (blocks[i])
	    free (blocks[i]);
    return 0;
}


static inline int data_select (int f, int waits, char read_write)
{
    fd_set fds;
    struct timeval tv;
    int result;

    if (waits) {
	if (read_write == 'r') {
	    for (;;) {
		FD_ZERO (&fds);
		FD_SET (f, &fds);
		result = select (f + 1, &fds, 0, 0, 0);
		if (result == -1 && errno == EINTR) {
		    tv.tv_sec = 0;
		    tv.tv_usec = 50;
		    select (0, 0, 0, 0, &tv);	/* non-blocking io, therefore wait 50 us before trying again */
		    continue;
		}
		break;
	    }
	    return result;
	} else {
	    for (;;) {
		FD_ZERO (&fds);
		FD_SET (f, &fds);
		result = select (f + 1, 0, &fds, 0, 0);
		if (result == -1 && errno == EINTR) {
		    tv.tv_sec = 0;
		    tv.tv_usec = 50;
		    select (0, 0, 0, 0, &tv);	/* non-blocking io, therefore wait 50 us before trying again */
		    continue;
		}
		break;
	    }
	    return result;
	}
    } else {
	if (read_write == 'r') {
	    FD_ZERO (&fds);
	    FD_SET (f, &fds);
	    tv.tv_sec = 0;
	    tv.tv_usec = 0;
	    return select (f + 1, &fds, 0, 0, &tv);
	}
    }
    FD_ZERO (&fds);
    FD_SET (f, &fds);
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    return select (f + 1, 0, &fds, 0, &tv);
}


/*
   The following procedures can be used as checks before
   read() and write() to prevent or force blocking.
 */

/* this will check if data is available from f and then return immediately */
int data_read_ready (int f)
{
    return data_select (f, 0, 'r');
}

/* blocks until data is ready to be read */
int data_read_wait (int f)
{
    return data_select (f, 1, 'r');
}

/* this will check if f is ready to receive data */
int data_write_ready (int f)
{
    return data_select (f, 0, 'w');
}

/* waits until data is ready to be written and return immediately */
int data_write_wait (int f)
{
    return data_select (f, 1, 'w');
}
