/*
 *  PlatformSpecific.h
 */

#pragma once


#ifdef WIN32

#define _USE_MATH_DEFINES
#define __STDC_WANT_LIB_EXT1__ 1
#ifndef _MSC_VER
#define UNICODE
#define _UNICODE
#endif
#if ! _BIND_TO_CURRENT_VCLIBS_VERSION
#undef _BIND_TO_CURRENT_VCLIBS_VERSION
#define _BIND_TO_CURRENT_VCLIBS_VERSION 1
#endif
#if ! _BIND_TO_CURRENT_CRT_VERSION
#undef _BIND_TO_CURRENT_CRT_VERSION
#define _BIND_TO_CURRENT_CRT_VERSION 1
#endif
#if !defined(WINVER) || (WINVER < 0x502)
#undef WINVER
#define WINVER 0x502
#endif

#include <windows.h>

#ifdef _MSC_VER
#define snprintf(...) (sprintf_s(__VA_ARGS__))
#define strcasecmp(...) (_stricmp(__VA_ARGS__))
#define strncasecmp(...) (_strnicmp(__VA_ARGS__))
#define strtok_r(...) (strtok_s(__VA_ARGS__))
#if _MSC_VER < 1600
#define nullptr NULL
#endif
#endif

#undef GetObject
#undef DrawText
#undef SearchPath

#define Z_BITS (32)
#define GLEW_STATIC true

#else  // Not WIN32

#if defined(__APPLE__) && ( defined(_ppc_) || defined(_ppc64_) || defined(__ppc__) || defined(__ppc64__) || defined(__POWERPC__) || defined(_M_PPC) )
#define APPLE_POWERPC true
#define NO_VR true
#endif

#if defined(APPLE_POWERPC) && ! defined(APPLE_POWERPC_GLEW)
#define NO_GLEW true
#endif

#define Z_BITS (24)

#endif


#if ! defined(HAVE_NULLPTR) && ! defined(_MSC_VER)
#ifdef __GNUC__
#define nullptr __null
#else
#define nullptr NULL
#endif
#endif
