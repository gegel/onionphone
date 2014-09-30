/*
 * Copyright 1992 by Jutta Degener and Carsten Bormann, Technische
 * Universitaet Berlin.  See the accompanying file "COPYRIGHT" for
 * details.  THERE IS ABSOLUTELY NO WARRANTY FOR THIS SOFTWARE.
 */

/* $Header: /cvsroot/speak-freely-u/speak_freely/gsm/inc/toast.h,v 1.1.1.1 2002/11/09 12:41:01 johnwalker Exp $ */

#ifndef	TOAST_H
#define	TOAST_H				/* Guard against multiple includes */

#include "config.h"

#include <sys/types.h>
#include <sys/stat.h>

#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <unistd.h>

#include <errno.h>
#ifndef	HAS_ERRNO_DECL
	 extern int	errno;
#endif

#ifdef	HAS_LIMITS_H
#include <limits.h>
#endif

#ifdef	HAS_FCNTL_H
# include <fcntl.h>
#endif

#include <utime.h>

#include "gsm.h"

#ifndef	S_ISREG
#define	S_ISREG(x)	((x) & S_IFREG)
#endif	/* S_ISREG */


# define	READ	"r"
# define	WRITE	"w"
# define	O_WRITE_EXCL	O_WRONLY|O_CREAT|O_EXCL

#ifndef SIGHANDLER_T
#define SIGHANDLER_T	void	/* what does a signal handler return? */
#endif


#ifdef	HAS_STRING_H
#include	<string.h>
#else
#	ifdef HAS_STRINGS_H
#	include <strings.h>
#	else
#		include "proto.h"

		extern int	strlen	P((char *));
		extern char *	strcpy  P((char *, char *));
		extern char *	strcat  P((char *,  char *));
		extern char *	strrchr P((char *, int));

#		include "unproto.h"
#	endif
#endif


#ifdef	HAS_STDLIB_H
#include	<stdlib.h>
#else
#	include "proto.h"
#	ifdef	HAS_MALLOC_H
#	include <malloc.h>
#	else
		extern char	* malloc P((unsigned));
#	endif
	extern int	exit P((int));
#	include "unproto.h"
#endif

/*
 *	This suffix is tacked onto/removed from filenames
 *	similar to the way freeze and compress do it.
 */
#define	SUFFIX_TOASTED		".gsm"

#include	"proto.h"

#endif		/* TOAST_H */
