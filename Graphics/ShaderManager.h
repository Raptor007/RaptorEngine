/*
 *  ShaderManager.h
 */

#pragma once
class ShaderManager;

#include "PlatformSpecific.h"

#include <list>
#include <map>
#include "Shader.h"
#include "Str.h"


class ShaderManager
{
public:
	bool Initialized;
	Shader *Selected;
	
	ShaderManager( void );
	virtual ~ShaderManager();
	
	bool Initialize( void );
	
	void Select( Shader *shader );
	void SelectAndCopyVars( Shader *shader );
	
	void StopShaders( void );
	void ResumeShaders( void );
	
	bool Active( void );
	bool Ready( void );
	
	bool Set1f( const char *name, double value );
	bool Set3f( const char *name, double x, double y, double z );
	bool Set4f( const char *name, double x, double y, double z, double w );
	bool Set1i( const char *name, int value );
};
