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
#include <stdbool.h>

#define 	_DEBUG
#define		INLINE		inline

typedef struct Packet_s
	{
	bool continuous;
	int command;
	int datalen;
	char data[256];
} Packet_t;

typedef struct Thread_s
{
	pthread_t pthread;
	bool inuse;
	time_t spawned;
} Thread_t;

typedef struct ThreadParam_s
{
	int *socketId;
	Thread_t *threads;
		
} ThreadParam_t;

typedef struct Client_s
{
	int socketDesc;
	bool inuse;
} Client_t;	

#endif
