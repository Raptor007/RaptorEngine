/*
 *  TextFileViewer.h
 */

#pragma once
class TextFileViewer;
class TextFileViewerCloseButton;

#include "PlatformSpecific.h"
#include "Window.h"
#include <cstddef>
#include <SDL/SDL.h>
#include "Font.h"
#include "ListBox.h"
#include "LabelledButton.h"


class TextFileViewer : public Window
{
public:
	std::string Title;
	Font *TitleFont;
	ListBox *Contents;
	
	TextFileViewer( SDL_Rect *window_rect, const char *filename, Font *font = NULL, const char *title = NULL );
	virtual ~TextFileViewer();
	
	bool KeyUp( SDLKey key );
	
	void Draw( void );
};


class TextFileViewerCloseButton : public LabelledButton
{
public:
	TextFileViewerCloseButton( SDL_Rect *rect, Font *button_font );
	virtual ~TextFileViewerCloseButton();
	void Clicked( Uint8 button = SDL_BUTTON_LEFT );
};
