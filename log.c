#include <stdio.h>
#include <time.h>

#include "common.h"
#include "log.h"

INLINE void LogMessage( const int severity, const char *msg )
{
	char buf[ 0x40 ];
	sprintf( buf, "%d: %s\n" , (int)time( NULL ), msg );
	
	switch( severity )
	{
		case LOG_NOTICE:
			printf( "Notice: %s", buf );	
			break;
		case LOG_WARNING:
			printf( "Warning: %s", buf );	
			break;
		case LOG_ERROR:
			printf( "Error: %s", buf );	
			break;
	}	
}
