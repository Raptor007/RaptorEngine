/*
 *  TextFileViewer.h
 */

#pragma once
class TextFileViewer;
class TextFileViewerCloseButton;

#include "PlatformSpecific.h"
#include "Window.h"
#include <cstddef>

#ifdef SDL2
	#include <SDL2/SDL.h>
#else
	#include <SDL/SDL.h>
#endif

#include "Font.h"
#include "ListBox.h"
#include "LabelledButton.h"


class TextFileViewer : public Window
{
public:
	std::string Title;
	Font *TitleFont;
	ListBox *Contents;
	TextFileViewerCloseButton *CloseButton;
	bool AutoPosition;
	
	TextFileViewer( SDL_Rect *window_rect, const char *filename, Font *font = NULL, const char *title = NULL );
	virtual ~TextFileViewer();
	
	bool KeyUp( SDLKey key );
	
	void UpdateRects( void );
	void Draw( void );
};


class TextFileViewerCloseButton : public LabelledButton
{
public:
	TextFileViewerCloseButton( SDL_Rect *rect, Font *button_font );
	virtual ~TextFileViewerCloseButton();
	void Clicked( Uint8 button = SDL_BUTTON_LEFT );
};
