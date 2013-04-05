/*
 *  Shader.h
 */

#pragma once
class Shader;
typedef unsigned int ShaderType;

#include "platforms.h"

#include <string>
#include <GLEW/glew.h>
#include <OpenGL/gl.h>


class Shader
{
public:
	GLuint ShaderHandle;
	ShaderType Type;
	std::string SourceCode, Log, FileName;
	
	Shader( void );
	Shader( ShaderType type, const char *filename );
	~Shader();
	
	void Reload( void );
	
	bool Ready( void );
	
private:
	bool Compiled;
};
