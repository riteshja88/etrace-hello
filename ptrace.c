
/*-------------------------------------------------------------------------*/
/**
  @file		ptrace.c
  @author	N. Devillard, V. Chudnovsky
  @date		March 2004
  @version	$Revision: 1.1.1.1 $
  @brief	Add tracing capability to any program compiled with gcc.

  This module is only compiled when using gcc and tracing has been
  activated. It allows the compiled program to output messages whenever
  a function is entered or exited.

  To activate this feature, your version of gcc must support
  the -finstrument-functions flag.

  When using ptrace on a dynamic library, you must set the
  PTRACE_REFERENCE_FUNCTION macro to be the name of a function in the
  library. The address of this function when loaded will be the first
  line output to the trace file and will permit the translation of the
  other entry and exit pointers to their symbolic names. You may set
  the macro PTRACE_INCLUDE with any #include directives needed for
  that function to be accesible to this source file.

  The printed messages yield function addresses, not human-readable
  names. To link both, you need to get a list of symbols from the
  program. There are many (unportable) ways of doing that, see the
  'etrace' project on freshmeat for more information about how to dig
  the information.
*/
/*--------------------------------------------------------------------------*/

/*
	$Id: ptrace.c,v 1.1.1.1 2004-03-16 20:00:07 ndevilla Exp $
	$Author: ndevilla $
	$Date: 2004-03-16 20:00:07 $
	$Revision: 1.1.1.1 $
*/

#if (__GNUC__>2) || ((__GNUC__ == 2) && (__GNUC_MINOR__ > 95))

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <execinfo.h>
#include <pthread.h>

/*---------------------------------------------------------------------------
   							    User Macros
 ---------------------------------------------------------------------------*/
#define PTRACE_PIPENAME	 "TRACE"

/* When using ptrace on a dynamic library, the following must be defined:

#include "any files needed for PTRACE_REFERENCE_FUNCTION"
#define PTRACE_REFERENCE_FUNCTION functionName

`*/


/*---------------------------------------------------------------------------
   								Defines
 ---------------------------------------------------------------------------*/

#define REFERENCE_OFFSET "REFERENCE:"
#define FUNCTION_ENTRY   "enter"
#define FUNCTION_EXIT    "exit"
#define END_TRACE        "EXIT"
#define __NON_INSTRUMENT_FUNCTION__    __attribute__((__no_instrument_function__))
#define PTRACE_OFF        __NON_INSTRUMENT_FUNCTION__
#define STR(_x)          #_x
#define DEF(_x)          _x
#define GET(_x,_y)       _x(_y)
/*---------------------------------------------------------------------------
  							Function codes
 ---------------------------------------------------------------------------*/

#if 0

/** Final trace close */
static void
__NON_INSTRUMENT_FUNCTION__
gnu_ptrace_close(void)
{
	FILE   *trace ;
	if ((trace=fopen(PTRACE_PIPENAME, "a"))==NULL) return ;
	fprintf(trace, END_TRACE " %ld\n", (long)getpid());
	fclose(trace);
	return ;
}
#endif

pthread_mutex_t mutex;
/** Trace initialization */
static int
__NON_INSTRUMENT_FUNCTION__
gnu_ptrace_init(void)
{
	

	struct stat sta ;

	/* See if a trace file exists */
	if (stat(PTRACE_PIPENAME, &sta)!=0) {
		/* No trace file: do not trace at all */
		return 0 ;
	}
	/* Tracing requested: a trace file was found */
	//atexit(gnu_ptrace_close);
	return 1 ;
	pthread_mutex_init(&mutex, NULL);
}

/** Function called by every function event */
void
__NON_INSTRUMENT_FUNCTION__
gnu_ptrace(char * what, void * p)
{
	static int first=1 ;
	static int active=1 ;
	FILE * trace ;

	if (first) {
		active = gnu_ptrace_init();
	}
	if (active==0)
		return ;

	if ((trace=fopen(PTRACE_PIPENAME, "a"))==NULL) {
		return ;
	}
	if (first) {
#ifdef PTRACE_REFERENCE_FUNCTION
		backtrace_symbols_fd(&p, 1, fileno(trace));
#endif
	  first = 0;
	};
	pthread_mutex_lock(&mutex);
	fprintf(trace, "[ threadid:%08lx ]", pthread_self());
	backtrace_symbols_fd(&p, 1, fileno(trace));
	pthread_mutex_unlock(&mutex);
	fclose(trace);
	return ;
}

/** According to gcc documentation: called upon function entry */
void
__NON_INSTRUMENT_FUNCTION__
__cyg_profile_func_enter(void *this_fn, void *call_site)
{
	
	gnu_ptrace(FUNCTION_ENTRY, this_fn);
	(void)call_site;
}

/** According to gcc documentation: called upon function exit */
void
__NON_INSTRUMENT_FUNCTION__
__cyg_profile_func_exit(void *this_fn, void *call_site)
{
	//gnu_ptrace(FUNCTION_EXIT, this_fn);
	(void)call_site;
	(void)this_fn;
}

#endif
/* vim: set ts=4 et sw=4 tw=75 */
