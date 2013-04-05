/*
 *  Animation.h
 */

#pragma once
class Animation;

#include "platforms.h"

#include <string>
#include <vector>
#include <GLEW/glew.h>
#include <OpenGL/gl.h>

#include "Clock.h"


class Animation
{
public:
	std::vector<GLuint> Frames;
	std::vector<double> FrameTimes;
	int PlayCount;
	double Speed;
	std::string Name;
	Clock Timer;
	GLuint MostRecentFrame;
	
	Animation( void );
	Animation( std::string name, std::string filename );
	virtual ~Animation();
	
	void Load( std::string filename );
	void Reload( void );
	void BecomeInstance( const Animation *a );
	void Clear( void );
	
	void AddFrame( GLuint texture, double sec );
	void Start( void );
	
	GLuint CurrentFrame( void );
	double StoredTime( void );
	double LoopTime( void );
	bool Finished( void );
	
private:
	Clock LoadedTime;
};
