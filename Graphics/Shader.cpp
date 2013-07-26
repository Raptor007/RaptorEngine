/*
 *  Shader.cpp
 */

#include "Shader.h"

#include "File.h"


// http://people.freedesktop.org/~idr/OpenGL_tutorials/02-GLSL-hello-world.html


Shader::Shader( void )
{
	Type = 0;
	ShaderHandle = 0;
	Compiled = false;
}


Shader::Shader( ShaderType type, const char *filename, std::map<std::string,std::string> defs )
{
	Type = type;
	ShaderHandle = 0;
	Compiled = false;
	
	if( filename )
	{
		FileName = std::string(filename);
		Reload( defs );
	}
}


Shader::~Shader()
{
	if( ShaderHandle )
		glDeleteShader( ShaderHandle );
}


void Shader::Reload( std::map<std::string,std::string> defs )
{
	ShaderHandle = 0;
	Compiled = false;
	
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
		}
	}
}


bool Shader::Ready( void )
{
	return Compiled;
}
