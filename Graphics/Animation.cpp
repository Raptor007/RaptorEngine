/*
 *  Animation.cpp
 */

#include "Animation.h"

#include <fstream>
#include <algorithm>
#include <math.h>
#include "Str.h"
#include "RaptorGame.h"


Animation::Animation( void )
{
	PlayCount = 0;
	Speed = 1.;
	MostRecentFrame = 0;
}


Animation::Animation( std::string name, std::string filename )
{
	PlayCount = 0;
	Speed = 1.;
	MostRecentFrame = 0;
	
	Name = name;
	Load( filename );
}


Animation::~Animation()
{
	Clear();
}


void Animation::Load( std::string filename )
{
	if( filename.compare( filename.size() - 4, 4, ".ani" ) == 0 )
	{
		std::ifstream input( filename.c_str() );
		if( input.is_open() )
		{
			Frames.clear();
			FrameTimes.clear();
			
			std::string subdir = "";
			if( strrchr( Name.c_str(), '/' ) )
			{
				std::list<std::string> path = Str::SplitToList( Name, "/" );
				path.pop_back();
				subdir = Str::Join( path, "/" ) + std::string("/");
			}
			
			char buffer[ 1024 ] = "";
			GLuint texture = 0;
			int count = 0;
			
			while( ! input.eof() )
			{
				buffer[ 0 ] = '\0';
				input.getline( buffer, 1024 );
				
				// Remove unnecessary characters from the buffer and skip empty lines.
				snprintf( buffer, 1024, "%s", Str::Join( CStr::SplitToList( buffer, "\r\n" ), "" ).c_str() );
				if( ! strlen(buffer) )
					continue;
				
				// The first line in the file is the play count.
				if( ! count )
					PlayCount = atoi(buffer);
				
				// We alternate between textures and times.
				else if( count % 2 )
				{
					if( buffer[ 0 ] != '*' )
						// Unless it's a framebuffer texture, look in same dir as ani file.
						texture = Raptor::Game->Res.GetTexture( subdir + std::string(buffer) );
					else
						texture = Raptor::Game->Res.GetTexture( std::string(buffer) );
				}
				else
					AddFrame( texture, atof(buffer) );
				
				count ++;
				buffer[ 0 ] = '\0';
			}
			
			input.close();
		}
	}
	else
	{
		Frames.clear();
		FrameTimes.clear();
		
		PlayCount = 0;
		Speed = 1.;
		MostRecentFrame = 0;
		
		AddFrame( Raptor::Game->Res.GetTexture(filename), 1. );
	}
	
	LoadedTime.Reset();
}


void Animation::Reload( void )
{
	Clock old_timer = Timer;
	int old_play_count = PlayCount;
	double old_speed = Speed;
	
	BecomeInstance( Raptor::Game->Res.GetAnimation(Name) );
	
	Timer = old_timer;
	PlayCount = old_play_count;
	Speed = old_speed;
}


void Animation::BecomeInstance( const Animation *a )
{
	Clear();
	
	if( ! a )
		return;
	
	Frames = a->Frames;
	FrameTimes = a->FrameTimes;
	Speed = a->Speed;
	PlayCount = a->PlayCount;
	Name = a->Name;
	LoadedTime = a->LoadedTime;
	
	Start();
}


void Animation::Clear( void )
{
	Name = "";
	
	Frames.clear();
	FrameTimes.clear();
	
	PlayCount = 0;
	Speed = 1.;
	MostRecentFrame = 0;
}


void Animation::AddFrame( GLuint texture, double sec )
{
	Frames.push_back( texture );
	
	if( sec < 0.0 )
		FrameTimes.push_back( 0.0 );
	else
		FrameTimes.push_back( sec );
}


void Animation::Start( void )
{
	MostRecentFrame = 0;
	Timer.Reset();
}


GLuint Animation::CurrentFrame( void )
{
	if( LoadedTime.ElapsedSeconds() > Raptor::Game->Res.ResetTime.ElapsedSeconds() )
		Reload();
	
	size_t count = FrameTimes.size();
	if( count && Frames.size() )
	{
		if( Finished() )
		{
			// If the animation is done looping, always return the last frame.
			MostRecentFrame = *(Frames.rbegin());
			return MostRecentFrame;
		}
		
		double loop_time = LoopTime();
		if( loop_time )
		{
			// Figure out where we are in the cycle and return that frame.
			double time_in_animation = fmod( Timer.ElapsedSeconds(), loop_time );
			for( size_t i = 0; i < count; i ++ )
			{
				time_in_animation -= FrameTimes.at( i ) / Speed;
				if( time_in_animation < 0.0 )
				{
					MostRecentFrame = (Frames.size() > i) ? Frames.at( i ) : *(Frames.rbegin());
					return MostRecentFrame;
				}
			}
		}
		else if( count > 1 )
		{
			// Multiple frames with zero time means play as fast as possible.
			std::vector<GLuint>::const_iterator frame_iter = std::find( Frames.begin(), Frames.end(), MostRecentFrame );
			if( frame_iter != Frames.end() )
			{
				if( (! Raptor::Game->Gfx.DrawTo) || (Raptor::Game->Gfx.DrawTo != Raptor::Game->Head.EyeR) ) // Make sure both eyes see the same frame in VR.
				{
					frame_iter ++;
					MostRecentFrame = (frame_iter != Frames.end()) ? *frame_iter : *(Frames.begin());
				}
				return MostRecentFrame;
			}
		}
		
		// If there was a problem determining the current frame, return the last frame.
		MostRecentFrame = *(Frames.rbegin());
		return MostRecentFrame;
	}
	
	// If there are no frames or something went wrong, return 0.
	MostRecentFrame = 0;
	return MostRecentFrame;
}


double Animation::StoredTime( void ) const
{
	double total = 0.0;
	
	int count = FrameTimes.size();
	for( int i = 0; i < count; i ++ )
		total += FrameTimes.at( i );
	
	return total;
}


double Animation::LoopTime( void ) const
{
	return StoredTime() / Speed;
}


bool Animation::Finished( void ) const
{
	// PlayCount 0 means loop forever, so it's never finished.
	if( PlayCount <= 0 )
		return false;
	
	if( Timer.ElapsedSeconds() >= (LoopTime() * (double) PlayCount) )
		return true;
	
	return false;
}
