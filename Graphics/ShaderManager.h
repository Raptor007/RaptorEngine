/*
 *  ShaderManager.h
 */

#pragma once
class ShaderManager;
class ShaderVar;

#include "PlatformSpecific.h"

#include <list>
#include <map>
#include "Shader.h"
#include "Str.h"


class ShaderManager
{
public:
	bool Initialized;
	GLuint ProgramHandle;
	//std::map<const char *,ShaderVar,CStr::Less> Vars;
	std::map<std::string,ShaderVar> Vars;
	std::map<std::string,std::string> Defs;
	
	ShaderManager( void );
	~ShaderManager();
	
	bool Initialize( void );
	void LoadShaders( std::string shader_name = "model" );
	Shader *LoadShader( ShaderType type, const char *filename );
	bool UseShaders( std::list<Shader*> shaders );
	void DeleteShaders( void );
	
	void StopShaders( void );
	void ResumeShaders( void );
	
	bool Active( void );
	bool Ready( void );
	
	bool Set3f( const char *name, double x, double y, double z );
	bool Set1i( const char *name, int value );
	
private:
	std::map<std::string,Shader*> LoadedShaders;
};


class ShaderVar
{
public:
	GLint Loc;
	double Float1, Float2, Float3, Float4;
	int Int1, Int2, Int3, Int4;
	
	ShaderVar( void );
};
