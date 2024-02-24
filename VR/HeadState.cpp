/*
 *  HeadState.cpp
 */

#include "HeadState.h"

#include "RaptorGame.h"


HeadState::HeadState( void )
{
	Initialized = false;
	VR = false;
	EyeL = NULL;
	EyeR = NULL;
	
#ifndef NO_VR
	m_pHMD = NULL;
	m_pRenderModels = NULL;
#endif
	
	Recenter();
}

HeadState::~HeadState()
{
	Initialized = false;
	StopVR();
}


void HeadState::Initialize( void )
{
	StartVR();
	Recenter();
	Initialized = true;
}


void HeadState::StartVR( void )
{
	if( Raptor::Game->Gfx.VSync || ! Raptor::Game->Gfx.Framebuffers )
	{
		// Make sure we enable framebuffers, and reduce VR stutter by disabling vsync to main monitor.
		Raptor::Game->Gfx.Framebuffers = true;
		Raptor::Game->Gfx.VSync = false;
		Raptor::Game->Gfx.Restart();
	}
	
#ifndef NO_VR
	vr::EVRInitError eError = vr::VRInitError_None;
	m_pHMD = vr::VR_Init( &eError, vr::VRApplication_Scene );
	
	if( eError != vr::VRInitError_None )
	{
		m_pHMD = NULL;
		Raptor::Game->Console.Print( std::string("Unable to init VR runtime: ") + std::string(vr::VR_GetVRInitErrorAsEnglishDescription(eError)), TextConsole::MSG_ERROR );
		return;
	}
	
	m_pRenderModels = (vr::IVRRenderModels *) vr::VR_GetGenericInterface( vr::IVRRenderModels_Version, &eError );
	if( ! m_pRenderModels )
	{
		m_pHMD = NULL;
		vr::VR_Shutdown();
		Raptor::Game->Console.Print( std::string("Unable to get render model interface: ") + std::string(vr::VR_GetVRInitErrorAsEnglishDescription(eError)), TextConsole::MSG_ERROR );
		return;
	}
	
	uint32_t w = 1080, h = 1200;
	m_pHMD->GetRecommendedRenderTargetSize( &w, &h );
	
	EyeL = Raptor::Game->Res.GetFramebuffer( "vr_l", w, h );
	EyeR = Raptor::Game->Res.GetFramebuffer( "vr_r", w, h );
	if( !( EyeL && EyeR ) )
		return;
	
	vr::VRCompositor()->SetTrackingSpace( vr::TrackingUniverseSeated );
	
	// Make sure we actually have a VR device connected.
	vr::VRCompositor()->WaitGetPoses( m_rTrackedDevicePose, vr::k_unMaxTrackedDeviceCount, NULL, 0 );
	if( ! m_rTrackedDevicePose[ vr::k_unTrackedDeviceIndex_Hmd ].bDeviceIsConnected )
	{
		Raptor::Game->Console.Print( "No VR headset detected.", TextConsole::MSG_ERROR );
		StopVR();
		return;
	}
	
	VR = true;
#endif
}


void HeadState::StopVR( void )
{
#ifndef NO_VR
	if( m_pHMD )
	{
		vr::VR_Shutdown();
		m_pHMD = NULL;
		m_pRenderModels = NULL;
	}
#endif
	
	if( EyeL )
		delete EyeL;
	if( EyeR )
		delete EyeR;
	
	VR = false;
	EyeL = NULL;
	EyeR = NULL;
	
	if( Initialized )  // Make sure we don't use deleted Cam when quitting.
	{
		Raptor::Game->Cam.Offset.SetPos(0,0,0);
		Raptor::Game->Cam.Offset.SetFwdVec(1,0,0);
		Raptor::Game->Cam.Offset.SetUpVec(0,1,0);
	}
}


void HeadState::RestartVR( void )
{
	StopVR();
	StartVR();
}


void HeadState::Recenter( void )
{
#ifndef NO_VR
	if( VR )
		vr::VRSystem()->ResetSeatedZeroPose();
#endif
	
	Basis.SetPos(0,0,0);
	Basis.SetFwdVec(1,0,0);
	Basis.SetUpVec(0,1,0);
	Current.Copy( &Basis );
	
	if( Initialized )  // Make sure we don't use uninitialized Cam when starting.
		Raptor::Game->Cam.Offset.Copy( &Basis );
}


void HeadState::Draw( void )
{
	if( ! Initialized )
		Initialize();
	
#ifndef NO_VR
	if( ! VR )
		return;
	
	// Make sure the OpenGL context is still valid.
	if( EyeL->LoadedTime.ElapsedSeconds() > Raptor::Game->Res.ResetTime.ElapsedSeconds() )
		EyeL->Reload();
	if( EyeR->LoadedTime.ElapsedSeconds() > Raptor::Game->Res.ResetTime.ElapsedSeconds() )
		EyeR->Reload();
	
	// For now, just clear the VR event queue without acting on anything.
	vr::VREvent_t vr_event;
	while( m_pHMD->PollNextEvent( &vr_event, sizeof(vr_event) ) ) {}
	
	vr::VRCompositor()->WaitGetPoses( m_rTrackedDevicePose, vr::k_unMaxTrackedDeviceCount, NULL, 0 );
	
	if( ! m_rTrackedDevicePose[ vr::k_unTrackedDeviceIndex_Hmd ].bDeviceIsConnected )
	{
		StopVR();
		return;
	}
	
	if( m_rTrackedDevicePose[ vr::k_unTrackedDeviceIndex_Hmd ].bPoseIsValid )
	{
		const vr::HmdMatrix34_t *matrix = &(m_rTrackedDevicePose[ vr::k_unTrackedDeviceIndex_Hmd ].mDeviceToAbsoluteTracking);
		Current.SetPos(   -(matrix->m[2][3]),   matrix->m[1][3],    matrix->m[0][3]  );
		Current.SetUpVec( -(matrix->m[2][1]),   matrix->m[1][1],    matrix->m[0][1]  );
		Current.SetFwdVec(  matrix->m[2][2],  -(matrix->m[1][2]), -(matrix->m[0][2]) );
	}
	
	Raptor::Game->Cam.Offset.SetPos( Current.DistAlong( &(Basis.Fwd), &Basis ), Current.DistAlong( &(Basis.Up), &Basis ), Current.DistAlong( &(Basis.Right), &Basis ) );
	Raptor::Game->Cam.Offset.SetUpVec( Current.Up.X, Current.Up.Y, Current.Up.Z );
	Raptor::Game->Cam.Offset.SetFwdVec( Current.Fwd.X, Current.Fwd.Y, Current.Fwd.Z );
	
	// Left Eye
	double separation = Raptor::Game->Cfg.SettingAsDouble( "vr_separation", 0.0625 );
	EyeL->OffsetX = Raptor::Game->Cfg.SettingAsInt( "vr_offset", 87 );
	Raptor::Game->Cam.Offset.MoveAlong( &(Raptor::Game->Cam.Offset.Right), separation / -2. );
	Raptor::Game->Gfx.DrawTo = EyeL;
	Raptor::Game->Gfx.SelectDefaultFramebuffer();
	Raptor::Game->Draw();
	vr::Texture_t leftEyeTexture = { (void*)(uintptr_t)(EyeL->Texture), vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
	vr::VRCompositor()->Submit( vr::Eye_Left, &leftEyeTexture );
	
	double frame_time = Raptor::Game->FrameTime;
	Raptor::Game->FrameTime = 0.;  // Prevent multi-render side effects.
	
	// Right Eye
	EyeR->OffsetX = -(EyeL->OffsetX);
	Raptor::Game->Cam.Offset.MoveAlong( &(Raptor::Game->Cam.Offset.Right), separation );
	Raptor::Game->Gfx.DrawTo = EyeR;
	Raptor::Game->Gfx.SelectDefaultFramebuffer();
	Raptor::Game->Draw();
	vr::Texture_t rightEyeTexture = { (void*)(uintptr_t)(EyeR->Texture), vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
	vr::VRCompositor()->Submit( vr::Eye_Right, &rightEyeTexture );
	
	// Restore default draw conditions.
	Raptor::Game->FrameTime = frame_time;
	Raptor::Game->Gfx.DrawTo = NULL;
	Raptor::Game->Gfx.SelectDefaultFramebuffer();
#endif
}
