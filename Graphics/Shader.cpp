/*
 *  Shader.cpp
 */

#include "Shader.h"

#include <cstddef>
#include "File.h"
#include "RaptorGame.h"


// http://people.freedesktop.org/~idr/OpenGL_tutorials/02-GLSL-hello-world.html


Shader::Shader( std::string name, std::string filename, std::map<std::string,std::string> defs )
{
	ProgramHandle = 0;
	Name = name;
	
	Load( filename, defs );
}


Shader::~Shader()
{
	Clear();
}


void Shader::Clear( void )
{
	if( ProgramHandle )
		glDeleteProgram( ProgramHandle );
	ProgramHandle = 0;
	
	for( std::vector<ShaderComponent*>::iterator component_iter = Components.begin(); component_iter != Components.end(); component_iter ++ )
		delete *component_iter;
	Components.clear();
	
	Vars.clear();
}


void Shader::Load( std::string filename, std::map<std::string,std::string> defs )
{
	GLint previous_program = 0;
	glGetIntegerv( GL_CURRENT_PROGRAM, &previous_program );
	
	Clear();
	
	if( ! Raptor::Game->Cfg.SettingAsBool( "g_shader_enable", true ) )
		return;
	
	ProgramHandle = glCreateProgram();
	
	if( ProgramHandle )
	{
		Components.push_back( new ShaderComponent( GL_VERTEX_SHADER, filename + std::string(".vert"), defs ) );
		Components.push_back( new ShaderComponent( GL_FRAGMENT_SHADER, filename + std::string(".frag"), defs ) );
		
		for( std::vector<ShaderComponent*>::iterator component_iter = Components.begin(); component_iter != Components.end(); component_iter ++ )
		{
			if( *component_iter && (*component_iter)->Ready() )
				glAttachShader( ProgramHandle, (*component_iter)->ShaderHandle );
		}
		
		glLinkProgram( ProgramHandle );
		GLint link_status = 0;
		glGetProgramiv( ProgramHandle, GL_LINK_STATUS, &link_status );
		
		if( link_status == GL_TRUE )
		{
			glUseProgram( ProgramHandle );
			
			GLint current_program = 0;
			glGetIntegerv( GL_CURRENT_PROGRAM, &current_program );
			
			if( (GLuint) current_program != ProgramHandle )
			{
				glDeleteProgram( ProgramHandle );
				ProgramHandle = 0;
			}
		}
	}
	
	glUseProgram( previous_program );
}


bool Shader::Ready( void )
{
	return ProgramHandle;
}


bool Shader::Active( void )
{
	if( ProgramHandle )
	{
		GLint current_program = 0;
		glGetIntegerv( GL_CURRENT_PROGRAM, &current_program );
		return ((GLuint) current_program == ProgramHandle);
	}
	
	return false;
}


bool Shader::Set1f( const char *name, double value )
{
	bool valid = false;
	
	if( ProgramHandle )
	{
		// See if we already know the variable's loc.  If not, look it up in the shader.
		ShaderVar *var = &(Vars[ std::string(name) ]);
		if( var->Loc < 0 )
			var->Loc = glGetUniformLocation( ProgramHandle, name );
		
		// Only operate on a valid shader variable.
		if( var->Loc >= 0 )
		{
			// Only tell the shader to change its value if the number is different.
			if( value != var->Float1 )
			{
				glUniform1f( var->Loc, value );
				var->Float1 = value;
				var->Type = ShaderVar::TYPE_1F;
			}
			valid = true;
		}
	}
	
	return valid;
}


bool Shader::Set3f( const char *name, double x, double y, double z )
{
	bool valid = false;
	
	if( ProgramHandle )
	{
		// See if we already know the variable's loc.  If not, look it up in the shader.
		ShaderVar *var = &(Vars[ std::string(name) ]);
		if( var->Loc < 0 )
			var->Loc = glGetUniformLocation( ProgramHandle, name );
		
		// Only operate on a valid shader variable.
		if( var->Loc >= 0 )
		{
			// Only tell the shader to change its values if the numbers are different.
			if( (x != var->Float1) || (y != var->Float2) || (z != var->Float3) )
			{
				glUniform3f( var->Loc, x, y, z );
				var->Float1 = x;
				var->Float2 = y;
				var->Float3 = z;
				var->Type = ShaderVar::TYPE_3F;
			}
			valid = true;
		}
	}
	
	return valid;
}


bool Shader::Set4f( const char *name, double x, double y, double z, double w )
{
	bool valid = false;
	
	if( ProgramHandle )
	{
		// See if we already know the variable's loc.  If not, look it up in the shader.
		ShaderVar *var = &(Vars[ std::string(name) ]);
		if( var->Loc < 0 )
			var->Loc = glGetUniformLocation( ProgramHandle, name );
		
		// Only operate on a valid shader variable.
		if( var->Loc >= 0 )
		{
			// Only tell the shader to change its values if the numbers are different.
			if( (x != var->Float1) || (y != var->Float2) || (z != var->Float3) || (w != var->Float4) )
			{
				glUniform4f( var->Loc, x, y, z, w );
				var->Float1 = x;
				var->Float2 = y;
				var->Float3 = z;
				var->Float4 = w;
				var->Type = ShaderVar::TYPE_4F;
			}
			valid = true;
		}
	}
	
	return valid;
}


bool Shader::Set1i( const char *name, int value )
{
	bool valid = false;
	
	if( ProgramHandle )
	{
		// See if we already know the variable's loc.  If not, look it up in the shader.
		ShaderVar *var = &(Vars[ std::string(name) ]);
		if( var->Loc < 0 )
			var->Loc = glGetUniformLocation( ProgramHandle, name );
		
		// Only operate on a valid shader variable.
		if( var->Loc >= 0 )
		{
			// Only tell the shader to change its value if the number is different.
			if( value != var->Int1 )
			{
				glUniform1i( var->Loc, value );
				var->Int1 = value;
				var->Type = ShaderVar::TYPE_1I;
			}
			valid = true;
		}
	}
	
	return valid;
}


int Shader::CopyVarsFrom( const Shader *other )
{
	int count = 0;
	
	for( std::map<std::string,ShaderVar>::const_iterator var_iter = other->Vars.begin(); var_iter != other->Vars.end(); var_iter ++ )
	{
		bool copied = false;
		
		if( var_iter->second.Type == ShaderVar::TYPE_1F )
			copied = Set1f( var_iter->first.c_str(), var_iter->second.Float1 );
		else if( var_iter->second.Type == ShaderVar::TYPE_3F )
			copied = Set3f( var_iter->first.c_str(), var_iter->second.Float1, var_iter->second.Float2, var_iter->second.Float3 );
		else if( var_iter->second.Type == ShaderVar::TYPE_4F )
			copied = Set4f( var_iter->first.c_str(), var_iter->second.Float1, var_iter->second.Float2, var_iter->second.Float3, var_iter->second.Float4 );
		else if( var_iter->second.Type == ShaderVar::TYPE_1I )
			copied = Set1i( var_iter->first.c_str(), var_iter->second.Int1 );
		
		if( copied )
			count ++;
	}
	
	return count;
}


// -----------------------------------------------------------------------------


ShaderComponent::ShaderComponent( ShaderComponentType type, std::string filename, std::map<std::string,std::string> defs )
{
	Type = type;
	ShaderHandle = 0;
	Compiled = false;
	
	FileName = filename;
	if( ! FileName.empty() )
	{
		ShaderHandle = glCreateShader( Type );
		if( ShaderHandle )
		{
			// First add definitions.
			SourceCode = "#version 110\n";
			for( std::map<std::string,std::string>::iterator def_iter = defs.begin(); def_iter != defs.end(); def_iter ++ )
			{
				if( ! def_iter->second.empty() )
					SourceCode += std::string("#define ") + def_iter->first + std::string(" ") + def_iter->second + std::string("\n");
			}
			
			// Load the source code from a file.
			SourceCode += File::AsString( FileName.c_str() );
			const GLchar *source_code_cstr = SourceCode.c_str();
			glShaderSource( ShaderHandle, 1, (const GLchar **) &(source_code_cstr), NULL );
			
			// Compile the shader and see if it worked.
			glCompileShader( ShaderHandle );
			GLint compile_status = 0;
			glGetShaderiv( ShaderHandle, GL_COMPILE_STATUS, &compile_status );
			if( compile_status == GL_TRUE )
				Compiled = true;
			
			// Log compilation results.
			char log_buffer[ 1024*128 ];
			glGetShaderInfoLog( ShaderHandle, 1024*128, NULL, log_buffer );
			Log = std::string(log_buffer);
			
			// If there were errors, display the log in the console.
			if( ! Compiled )
				Raptor::Game->Console.Print( FileName + std::string(":\n") + Log, TextConsole::MSG_ERROR );
		}
	}
}


ShaderComponent::~ShaderComponent()
{
	if( ShaderHandle )
		glDeleteShader( ShaderHandle );
}


bool ShaderComponent::Ready( void )
{
	return Compiled;
}


// -----------------------------------------------------------------------------


ShaderVar::ShaderVar( void )
{
	Loc = -1;
	Type = TYPE_UNKNOWN;
	
	Float1 = 0.;
	Float2 = 0.;
	Float3 = 0.;
	Float4 = 0.;
	Int1 = 0;
	Int2 = 0;
	Int3 = 0;
	Int4 = 0;
}


ShaderVar::~ShaderVar()
{
}
