/*
 *  PlatformSpecific.h
 */

#pragma once


#ifdef WIN32
#define _USE_MATH_DEFINES
#include <windows.h>
#define snprintf(...) (sprintf_s(__VA_ARGS__))
#endif

#if defined(__APPLE__) && ( defined(_ppc_) || defined(_ppc64_) || defined(__ppc__) || defined(__ppc64__) || defined(__POWERPC__) || defined(_M_PPC) )
#define APPLE_POWERPC
#endif

#if defined(APPLE_POWERPC) && ! defined(APPLE_POWERPC_GLEW)
#define NO_GLEW
#endif

#ifndef NULL
#define NULL 0x0
#endif
