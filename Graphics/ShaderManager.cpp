/*
 *  ShaderManager.cpp
 */

#include "ShaderManager.h"

#include "RaptorGame.h"


// http://people.freedesktop.org/~idr/OpenGL_tutorials/02-GLSL-hello-world.html


ShaderManager::ShaderManager( void )
{
	Initialized = false;
	ProgramHandle = 0;
}


ShaderManager::~ShaderManager()
{
	if( ! Initialized )
		return;
	
	glUseProgram( 0 );
	
	if( ProgramHandle )
		glDeleteProgram( ProgramHandle );
	ProgramHandle = 0;
}


bool ShaderManager::Initialize( void )
{
	GLenum result = glewInit();
	if( result == GLEW_OK )
		Initialized = true;
	
	if( Initialized )
		ProgramHandle = glCreateProgram();
	
	return Initialized;
}


void ShaderManager::LoadShaders( std::string shader_name )
{
	if( ! Initialized )
		return;
	
	/*
	for( std::map<std::string,Shader*>::iterator shader_iter = LoadedShaders.begin(); shader_iter != LoadedShaders.end(); shader_iter ++ )
		shader_iter->second->Reload();
	*/
	
	// Load model shaders.
	// FIXME: Put this somewhere else?
	Shader *vertex_shader = LoadShader( GL_VERTEX_SHADER, (std::string("Shaders/") + shader_name + std::string(".vert")).c_str() );
	Shader *fragment_shader = LoadShader( GL_FRAGMENT_SHADER, (std::string("Shaders/") + shader_name + std::string(".frag")).c_str() );
	if( vertex_shader && !(vertex_shader->Ready()) )
		Raptor::Game->Console.Print( vertex_shader->FileName + std::string(":\n") + vertex_shader->Log, TextConsole::MSG_ERROR );
	if( fragment_shader && !(fragment_shader->Ready()) )
		Raptor::Game->Console.Print( fragment_shader->FileName + std::string(":\n") + fragment_shader->Log, TextConsole::MSG_ERROR );
	
	std::list<Shader*> shaders;
	if( vertex_shader && fragment_shader && vertex_shader->Ready() && fragment_shader->Ready() )
	{
		shaders.push_back( vertex_shader );
		shaders.push_back( fragment_shader );
	}
	UseShaders( shaders );
	StopShaders();
}


Shader *ShaderManager::LoadShader( ShaderType type, const char *filename )
{
	if( ! Initialized )
		return NULL;
	
	Shader *shader = new Shader( type, filename );
	if( shader )
	{
		std::map<std::string,Shader*>::iterator old_shader = LoadedShaders.find( shader->FileName );
		if( old_shader != LoadedShaders.end() )
		{
			delete old_shader->second;
			old_shader->second = NULL;
			LoadedShaders.erase( old_shader );
		}
		
		LoadedShaders[ shader->FileName ] = shader;
	}
	
	return shader;
}


bool ShaderManager::UseShaders( std::list<Shader*> shaders )
{
	if( ! Initialized )
		return false;
	
	bool is_program_active = false;
	
	if( ProgramHandle )
		glDeleteProgram( ProgramHandle );
	ProgramHandle = glCreateProgram();
	
	if( ProgramHandle )
	{
		for( std::list<Shader*>::iterator shader_iter = shaders.begin(); shader_iter != shaders.end(); shader_iter ++ )
		{
			if( *shader_iter && (*shader_iter)->Ready() )
				glAttachShader( ProgramHandle, (*shader_iter)->ShaderHandle );
		}
		
		glLinkProgram( ProgramHandle );
		GLint link_status = 0;
		glGetProgramiv( ProgramHandle, GL_LINK_STATUS, &link_status );
		
		if( link_status == GL_TRUE )
		{
			glUseProgram( ProgramHandle );
			
			GLint current_program = 0;
			glGetIntegerv( GL_CURRENT_PROGRAM, &current_program );
			is_program_active = ((GLuint) current_program == ProgramHandle);
		}
		else
			glUseProgram( 0 );
		
		if( ! is_program_active )
		{
			glDeleteProgram( ProgramHandle );
			ProgramHandle = 0;
		}
	}
	else
		glUseProgram( 0 );
	
	return is_program_active;
}


void ShaderManager::DeleteShaders( void )
{
	if( ! Initialized )
		return;
	
	StopShaders();
	
	glDeleteProgram( ProgramHandle );
	ProgramHandle = 0;
	
	for( std::map<std::string,Shader*>::iterator shader_iter = LoadedShaders.begin(); shader_iter != LoadedShaders.end(); shader_iter ++ )
		delete shader_iter->second;
	LoadedShaders.clear();
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
	
	glUseProgram( ProgramHandle );
}


bool ShaderManager::Active( void )
{
	if( ! Initialized )
		return false;
	
	if( ProgramHandle )
	{
		GLint current_program = 0;
		glGetIntegerv( GL_CURRENT_PROGRAM, &current_program );
		return ((GLuint) current_program == ProgramHandle);
	}
	
	return false;
}


bool ShaderManager::Ready( void )
{
	return ProgramHandle;
}


bool ShaderManager::Set3f( const char *name, double x, double y, double z )
{
	if( ProgramHandle )
	{
		GLuint loc = glGetUniformLocation( ProgramHandle, name );
		if( loc >= 0 )
		{
			glUniform3f( loc, x, y, z );
			return true;
		}
	}
	
	return false;
}


bool ShaderManager::Set1i( const char *name, int value )
{
	if( ProgramHandle )
	{
		GLuint loc = glGetUniformLocation( ProgramHandle, name );
		if( loc >= 0 )
		{
			glUniform1i( loc, value );
			return true;
		}
	}
	
	return false;
}
