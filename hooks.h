/*
	Created: 4/30/2014
	Author: Cyrus
	Purpose: Define the functions to hook
*/

#ifndef __HOOKS_H
#define __HOOKS_H

#include <WinSock2.h>
#include <Windows.h>

typedef int (WINAPI *_send) (SOCKET s, const char *buf, int len, int flags);
typedef int (WINAPI *_recv) (SOCKET s, char *buf, int len, int flags);

#endif