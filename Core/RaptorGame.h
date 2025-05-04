/*
 *  RaptorGame.h
 */

#pragma once
class RaptorGame;

#include "PlatformSpecific.h"

#include <cstddef>
#include <time.h>
#include <signal.h>
#include <map>
#include <list>

#ifdef SDL2
	#include <SDL2/SDL.h>
#else
	#include <SDL/SDL.h>
#endif

#include "RaptorGL.h"
#include "RaptorDefs.h"
#include "Graphics.h"
#include "ShaderManager.h"
#include "Camera.h"
#include "SoundOut.h"
#include "Microphone.h"
#include "LayerManager.h"
#include "ClientConsole.h"
#include "ResourceManager.h"
#include "NetClient.h"
#include "ClientConfig.h"

#include "MouseState.h"
#include "KeyboardState.h"
#include "JoystickManager.h"
#include "InputManager.h"
#include "HeadState.h"

#ifdef WIN32
#include "SaitekManager.h"
#endif

#include "RaptorServer.h"


class RaptorGame
{
public:
	std::string Game;
	std::string Version;
	int DefaultPort;
	
	Graphics Gfx;
	ShaderManager ShaderMgr;
	Camera Cam;
	SoundOut Snd;
	Microphone Mic;
	LayerManager Layers;
	ClientConsole Console;
	TextConsole Msg;
	ResourceManager Res;
	NetClient Net;
	ClientConfig Cfg;
	double MaxFPS;
	
	double FrameTime;
	MouseState Mouse;
	KeyboardState Keys;
	JoystickManager Joy;
	InputManager Input;
	bool ReadKeyboard, ReadMouse;
	HeadState Head;
	
	#ifdef WIN32
	SaitekManager Saitek;
	#endif
	
	volatile int State;
	GameData Data;
	uint16_t PlayerID;
	
	std::string WaitText;
	Font *WaitFont;
	
	RaptorServer *Server;
	
	
	RaptorGame( std::string game, std::string version, RaptorServer *server = NULL );
	virtual ~RaptorGame();
	
	void SetServer( RaptorServer *server );
	
	virtual void SetDefaultJoyTypes( void );
	virtual void SetDefaultControls( void );
	virtual void SetDefaults( void );
	
	virtual void Initialize( int argc = 0, char **argv = NULL );
	virtual void Run( void );
	
	virtual void Setup( int argc, char **argv );
	virtual void Update( double dt );
	virtual void Draw( void );
	
	virtual bool HandleEvent( SDL_Event *event );
	virtual bool HandleCommand( std::string cmd, std::vector<std::string> *params = NULL );
	virtual void MessageReceived( std::string text, uint32_t type = TextConsole::MSG_NORMAL );
	virtual bool ProcessPacket( Packet *packet );
	virtual void SendUpdate( int8_t precision = 0 );
	virtual bool SetPlayerProperty( std::string name, std::string value, bool force = false );
	
	virtual void ChangeState( int state );
	virtual void Disconnected( void );
	virtual void Connecting( void );
	virtual void Connected( void );
	
	virtual GameObject *NewObject( uint32_t id, uint32_t type );
	virtual Player *NewPlayer( uint16_t id );
	
	virtual void Host( void );
	
	virtual void Quit( void );
};


namespace Raptor
{
#ifndef RAPTORGAME_CPP
	extern RaptorGame *Game;
#endif
	bool PreMain( int bits = sizeof(void*) * 8 );
}
