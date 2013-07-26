/*
 *  RaptorGL.h
 */

#pragma once

#include "PlatformSpecific.h"

#ifndef NO_GLEW
	#include <GLEW/glew.h>
#endif

#include <OpenGL/gl.h>

#ifdef NO_GLEW
	#include <OpenGL/glu.h>
	#include <OpenGL/glext.h>
	
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
