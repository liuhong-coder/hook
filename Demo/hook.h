#pragma once
#define _WIN32_WINNT 0X0500
#include<Windows.h>

__declspec(dllexport) void setHook(int flag, WORD mykey);



__declspec(dllexport) void stopHook();