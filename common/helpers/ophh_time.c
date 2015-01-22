#ifndef _WIN32

#define _DEFAULT_SOURCE
#define _GNU_SOURCE

#include <errno.h>
#include <time.h>

#include "ophh_time.h"

static struct timespec us_to_timespec(unsigned long long _us)
{
	struct timespec ret;
	time_t seconds = _us / 1000000ULL;
	_us -= seconds * 1000000ULL;
	ret.tv_sec = seconds;
	ret.tv_nsec = _us * 1000ULL;
	return ret;
}

void ophh_time_ussleep(unsigned long long _us)
{
	struct timespec requested_time = us_to_timespec(_us);
	while (nanosleep(&requested_time, &requested_time) == -1 && errno == EINTR)
		continue;
}

#endif /* _WIN32 */
