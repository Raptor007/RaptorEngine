/*
 *  ShaderManager.cpp
 */

#include "ShaderManager.h"

#include <cstddef>
#include "RaptorGame.h"


// http://people.freedesktop.org/~idr/OpenGL_tutorials/02-GLSL-hello-world.html


ShaderManager::ShaderManager( void )
{
	Initialized = false;
	Selected = NULL;
}


ShaderManager::~ShaderManager()
{
	if( ! Initialized )
		return;
	
	glUseProgram( 0 );
}


bool ShaderManager::Initialize( void )
{
	#ifndef NO_GLEW
	{
		GLenum result = glewInit();
		if( result == GLEW_OK )
			Initialized = true;
	}
	#else
		Initialized = true;
	#endif
	
	return Initialized;
}


void ShaderManager::Select( Shader *shader )
{
	bool active = Active();
	
	Selected = shader;
	
	if( active )
		glUseProgram( Selected->ProgramHandle );
}


void ShaderManager::SelectAndCopyVars( Shader *shader )
{
	bool active = Active();
	
	if( Ready() && shader && (shader != Selected) && shader->Ready() )
	{
		// OpenGL documentation says we should use the shader before setting its variables.
		glUseProgram( shader->ProgramHandle );
		
		shader->CopyVarsFrom( Selected );
		
		if( ! active )
			glUseProgram( 0 );
	}
	
	Selected = shader;
}


void ShaderManager::StopShaders( void )
{
	if( ! Initialized )
		return;
	
	glUseProgram( 0 );
}


void ShaderManager::ResumeShaders( void )
{
	if( ! Initialized )
		return;
	
	if( Selected )
		glUseProgram( Selected->ProgramHandle );
}


bool ShaderManager::Active( void )
{
	if( ! Initialized )
		return false;
	
	// It's active if any shader program is running.
	GLint current_program = 0;
	glGetIntegerv( GL_CURRENT_PROGRAM, &current_program );
	return current_program;
}


bool ShaderManager::Ready( void )
{
	return (Selected && Selected->ProgramHandle);
}


bool ShaderManager::Set1f( const char *name, double value )
{
	if( Selected )  // FIXME: Make sure shader is active?
		return Selected->Set1f( name, value );
	
	return false;
}


bool ShaderManager::Set3f( const char *name, double x, double y, double z )
{
	if( Selected )  // FIXME: Make sure shader is active?
		return Selected->Set3f( name, x, y, z );
	
	return false;
}


bool ShaderManager::Set4f( const char *name, double x, double y, double z, double w )
{
	if( Selected )  // FIXME: Make sure shader is active?
		return Selected->Set4f( name, x, y, z, w );
	
	return false;
}


bool ShaderManager::Set1i( const char *name, int value )
{
	if( Selected )  // FIXME: Make sure shader is active?
		return Selected->Set1i( name, value );
	
	return false;
}
