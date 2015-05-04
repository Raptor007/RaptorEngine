/*
 *  Animation.cpp
 */

#include "Animation.h"
#include <stdexcept>
#include <fstream>
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
	MostRecentFrame = 0;
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
	Timer.Reset();
}


GLuint Animation::CurrentFrame( void )
{
	if( LoadedTime.ElapsedSeconds() > Raptor::Game->Res.ResetTime.ElapsedSeconds() )
		Reload();
	
	int count = FrameTimes.size();
	if( count )
	{
		if( Finished() )
		{
			// If the animation is done looping, always return the last frame.
			
			try
			{
				MostRecentFrame = Frames.at( count - 1 );
				return MostRecentFrame;
			}
			catch( std::out_of_range &exception )
			{
				fprintf( stderr, "Animation::CurrentFrame: std::out_of_range\n" );
			}
			
			MostRecentFrame = 0;
			return MostRecentFrame;
		}
		
		
		// Figure out where we are in the cycle and return that frame.
		
		double time_in_animation = fmod( Timer.ElapsedSeconds(), LoopTime() );
		
		for( int i = 0; i < count; i ++ )
		{
			try
			{
				time_in_animation -= FrameTimes.at( i ) / Speed;
				if( time_in_animation < 0.0 )
				{
					MostRecentFrame = Frames.at( i );
					return MostRecentFrame;
				}
			}
			catch( std::out_of_range &exception )
			{
				fprintf( stderr, "Animation::CurrentFrame: std::out_of_range\n" );
			}
		}
		
		
		// If there was a problem determining the current frame, return the last frame.
		
		try
		{
			return Frames.at( count - 1 );
		}
		catch( std::out_of_range &exception )
		{
			fprintf( stderr, "Animation::CurrentFrame: std::out_of_range\n" );
		}
	}
	
	
	// If there are no frames or something went wrong, return 0.
	
	MostRecentFrame = 0;
	return MostRecentFrame;
}


double Animation::StoredTime( void )
{
	double total = 0.0;
	
	int count = FrameTimes.size();
	for( int i = 0; i < count; i ++ )
	{
		try
		{
			total += FrameTimes.at( i );
		}
		catch( std::out_of_range &exception )
		{
			fprintf( stderr, "Animation::StoredTime: std::out_of_range\n" );
		}
	}
	
	return total;
}


double Animation::LoopTime( void )
{
	return StoredTime() / Speed;
}


bool Animation::Finished( void )
{
	// PlayCount 0 means loop forever, so it's never finished.
	if( PlayCount <= 0 )
		return false;
	
	if( Timer.ElapsedSeconds() >= (LoopTime() * (double) PlayCount) )
		return true;
	
	return false;
}
