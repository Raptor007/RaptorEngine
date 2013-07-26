/*
 *  Shader.h
 */

#pragma once
class Shader;
typedef unsigned int ShaderType;

#include "PlatformSpecific.h"

#include <string>
#include <map>
#include "RaptorGL.h"


class Shader
{
public:
	GLuint ShaderHandle;
	ShaderType Type;
	std::string SourceCode, Log, FileName;
	
	Shader( void );
	Shader( ShaderType type, const char *filename, std::map<std::string,std::string> defs );
	~Shader();
	
	void Reload( std::map<std::string,std::string> defs );
	
	bool Ready( void );
	
private:
	bool Compiled;
};
