/*
 *  Shader.h
 */

#pragma once
class Shader;
class ShaderComponent;
class ShaderVar;
typedef unsigned int ShaderComponentType;

#include "PlatformSpecific.h"

#include <stdint.h>
#include <string>
#include <map>
#include <vector>
#include "RaptorGL.h"


class Shader
{
public:
	GLuint ProgramHandle;
	std::string Name;
	std::map<std::string,ShaderVar> Vars;
	
	Shader( std::string name, std::string filename, std::map<std::string,std::string> defs );
	~Shader();
	
	void Clear( void );
	void Load( std::string filename, std::map<std::string,std::string> defs );
	
	bool Ready( void );
	bool Active( void );
	
	bool Set1f( const char *name, double value );
	bool Set3f( const char *name, double x, double y, double z );
	bool Set4f( const char *name, double x, double y, double z, double w );
	bool Set1i( const char *name, int value );
	
	int CopyVarsFrom( const Shader *other );
	
private:
	std::vector<ShaderComponent*> Components;
};


class ShaderComponent
{
public:
	GLuint ShaderHandle;
	ShaderComponentType Type;
	std::string SourceCode, Log, FileName;
	
	ShaderComponent( ShaderComponentType type, std::string filename, std::map<std::string,std::string> defs );
	virtual ~ShaderComponent();
	
	bool Ready( void );
	
private:
	bool Compiled;
};


class ShaderVar
{
public:
	GLint Loc;
	uint8_t Type;
	double Float1, Float2, Float3, Float4;
	int Int1, Int2, Int3, Int4;
	
	ShaderVar( void );
	virtual ~ShaderVar();
	
	enum
	{
		TYPE_UNKNOWN = 0,
		TYPE_1F,
		TYPE_3F,
		TYPE_4F,
		TYPE_1I
	};
};
