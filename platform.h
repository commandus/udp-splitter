#ifndef PLATFORM_H
#define PLATFORM_H 1

#ifdef _MSC_VER
#define	SYSLOG(msg)
#define OPENSYSLOG()
#define CLOSESYSLOG()
#else
#include <syslog.h>
#include <sstream>
#define	SYSLOG(msg) { syslog (LOG_ALERT, "%s", msg); }
#define OPENSYSLOG() { setlogmask (LOG_UPTO(LOG_NOTICE)); openlog("humandetector", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1); }
#define CLOSESYSLOG() closelog();
#endif

#endif
