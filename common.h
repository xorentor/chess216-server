#ifndef __COMMON_H_
#define __COMMON_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <pthread.h>
#include <memory.h>

#define 	_DEBUG
#define		INLINE		inline

#define		F_QUIT		0x0001

typedef struct Thread_s
{
	pthread_t pthread;
	int inuse;
	time_t spawned;
} Thread_t;

typedef struct ThreadParam_s
{
	int *socketId;
	Thread_t *threads;
		
} ThreadParam_t;

typedef struct PacketData_s
{
	char command;
	char length;	
	char data[ 225 ];
} PacketData_t;

INLINE int       asm_strcmp( const char *s, const char *d, const int c )
{
        int r;
        __asm__
        (
                "testl  %%eax, %%eax;"
                "jz     .cmpf;"
                "testl  %%ebx, %%ebx;"
                "jz     .cmpf;"
                "mov    %%eax, %%esi;"
                "mov    %%ebx, %%edi;"
                "cld;"
                "repe   cmpsb;"
                "je     .cmpt;"
                ".cmpf:;"
                "movl   $1, %%eax;"
                "jmp    .cmpend;"
                ".cmpt:;"
                "xorl   %%eax, %%eax;"
                ".cmpend:;"
                : "=a"(r)
                : "a"(s), "b"(d), "c"(c)
        );
        return r;
}


#endif
