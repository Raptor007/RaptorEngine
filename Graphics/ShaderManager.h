/*
 *  ShaderManager.h
 */

#pragma once
class ShaderManager;

#include "platforms.h"

#include <list>
#include <map>
#include "Shader.h"


class ShaderManager
{
public:
	bool Initialized;
	GLuint ProgramHandle;
	
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
