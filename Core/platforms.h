/*
 *  platforms.h
 */

#pragma once


#ifdef WIN32
#define _USE_MATH_DEFINES
#include <windows.h>
#define snprintf(...) (sprintf_s(__VA_ARGS__))
#endif


#ifndef NULL
#define NULL 0x0
#endif

/*
#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif

#ifndef M_PI_2
#define M_PI_2 1.57079632679489661923132169163975144
#endif

#ifndef M_PI_4
#define M_PI_4 0.785398163397448309615660845819875721
#endif
*/
