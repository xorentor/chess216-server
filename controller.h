#ifndef __CONTROLLER_H_
#define __CONTROLLER_H_

typedef void (*ParseAction)( ClientLocalData_t*, Player_t* );

void Controller( ClientLocalData_t* );
ParseAction ParseCmd( const char *cmd );

#endif
