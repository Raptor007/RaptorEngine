/*
 *  RaptorGL.h
 */

#pragma once

#include "PlatformSpecific.h"

#ifndef NO_GLEW
	#ifdef __APPLE__
		#include <GLEW/glew.h>
	#else
		#include <GL/glew.h>
	#endif
#endif

#ifdef __APPLE__
	#include <OpenGL/gl.h>
#else
	#include <GL/gl.h>
#endif

#ifdef NO_GLEW
	#ifdef __APPLE__
		#include <OpenGL/glu.h>
		#include <OpenGL/glext.h>
	#else
		#include <GL/glu.h>
		#include <GL/glext.h>
	#endif
	
	#define GL_FRAMEBUFFER GL_FRAMEBUFFER_EXT
	#define GL_RENDERBUFFER GL_RENDERBUFFER_EXT
	#define GL_COLOR_ATTACHMENT0 GL_COLOR_ATTACHMENT0_EXT
	#define GL_DEPTH_ATTACHMENT GL_DEPTH_ATTACHMENT_EXT
	#define GL_FRAMEBUFFER_COMPLETE GL_FRAMEBUFFER_COMPLETE_EXT
	#ifdef GL_PROGRAM_POINT_SIZE_EXT
		#define GL_PROGRAM_POINT_SIZE GL_PROGRAM_POINT_SIZE_EXT
	#endif
	#ifdef GL_FRAMEBUFFER_UNSUPPORTED_EXT
		#define GL_FRAMEBUFFER_UNSUPPORTED GL_FRAMEBUFFER_UNSUPPORTED_EXT
	#endif
	
	#define glGenerateMipmap glGenerateMipmapEXT
	#define glGenFramebuffers glGenFramebuffersEXT
	#define glBindFramebuffer glBindFramebufferEXT
	#define glGenRenderbuffers glGenRenderbuffersEXT
	#define glBindRenderbuffer glBindRenderbufferEXT
	#define glRenderbufferStorage glRenderbufferStorageEXT
	#define glFramebufferRenderbuffer glFramebufferRenderbufferEXT
	#define glFramebufferTexture2D glFramebufferTexture2DEXT
	#define glCheckFramebufferStatus glCheckFramebufferStatusEXT
	#define glDeleteFramebuffer glDeleteFramebufferEXT
	#define glDeleteFramebuffers glDeleteFramebuffersEXT
	#define glDeleteRenderbuffers glDeleteRenderbuffersEXT
#endif
