#ifndef __LOG_H_
#define __LOG_H_

#define 	LOG_NOTICE	1
#define		LOG_WARNING	2
#define 	LOG_ERROR	4

void LogMessage( const int severity, const char *msg );

#endif
