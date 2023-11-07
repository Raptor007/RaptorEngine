/*
 *  PlatformSpecific.h
 */

#pragma once


#ifdef WIN32
	
	#ifndef SDL2
		#define _USE_MATH_DEFINES 1
	#endif
	
	#define __STDC_WANT_LIB_EXT1__ 1
	#ifndef _MSC_VER
		#define UNICODE 1
		#define _UNICODE 1
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
	#endif
	
	#undef GetObject
	#undef DrawText
	#undef SearchPath
	
	#define GLEW_STATIC 1
	
#else  // Not WIN32
	
	#if defined(__APPLE__) && ( defined(_ppc_) || defined(_ppc64_) || defined(__ppc__) || defined(__ppc64__) || defined(__POWERPC__) || defined(_M_PPC) )
		#define APPLE_POWERPC 1
		#ifndef NO_VR
			#define NO_VR 1
		#endif
	#endif
	
	#if defined(APPLE_POWERPC) && ! defined(APPLE_POWERPC_GLEW)
		#define NO_GLEW 1
	#endif

#endif


#ifdef SDL2
	typedef int SDLKey;
	#define SDL_BUTTON_WHEELUP   254
	#define SDL_BUTTON_WHEELDOWN 255
	#define SDLK_KP0       SDLK_KP_0
	#define SDLK_KP1       SDLK_KP_1
	#define SDLK_KP2       SDLK_KP_2
	#define SDLK_KP3       SDLK_KP_3
	#define SDLK_KP4       SDLK_KP_4
	#define SDLK_KP5       SDLK_KP_5
	#define SDLK_KP6       SDLK_KP_6
	#define SDLK_KP7       SDLK_KP_7
	#define SDLK_KP8       SDLK_KP_8
	#define SDLK_KP9       SDLK_KP_9
	#define SDLK_PRINT     SDLK_PRINTSCREEN
	#define SDLK_NUMLOCK   SDLK_NUMLOCKCLEAR
	#define SDLK_SCROLLOCK SDLK_SCROLLLOCK
	#define SDLK_LMETA     SDLK_LGUI
	#define SDLK_RMETA     SDLK_RGUI
	#define SDLK_LSUPER    SDLK_LGUI
	#define SDLK_RSUPER    SDLK_RGUI
	#define SDLK_COMPOSE   SDLK_APPLICATION
	#define SDLK_BREAK     SDLK_CANCEL
	#define SDLK_EURO      SDLK_CURRENCYUNIT
#endif
