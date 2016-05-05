/*
 *  strtok_r.h
 */

#pragma once

#ifdef WIN32
#ifndef strtok_r

char *strtok_r( char *s1, const char *s2, char **s3 );

#endif
#endif
