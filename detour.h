/*
	Author: Cyrus
	Purpose: Hook functions via detour, 5 byte patch
*/

#ifndef _DETOUR_H
#define _DETOUR_H

#include <Windows.h>

/*
	@param pFunc = pointer to the function to hook
	@param pHook = pointer to the hook, i.e., the detour
	@return = the original function with restored stolen code
*/
PBYTE DetourFunction( PBYTE pFunc, PBYTE pHook); 

#endif