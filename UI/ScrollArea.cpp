/*
 *  ScrollArea.cpp
 */

#include "ScrollArea.h"

#include <cstddef>
#include "RaptorGame.h"


ScrollArea::ScrollArea( SDL_Rect *rect, int scroll_bar_size ) : Layer( rect )
{
	Red = 0.f;
	Green = 0.f;
	Blue = 0.f;
	Alpha = 0.f;
	
	ScrollBarSize = scroll_bar_size;
	ScrollBarRed = 1.f;
	ScrollBarGreen = 1.f;
	ScrollBarBlue = 1.f;
	ScrollBarAlpha = 1.f;
	ScrollBy = 50;
	
	ScrollX = 0;
	ScrollY = 0;
	ClickedScrollBar = false;
	
	AddElement( new ScrollAreaButton( ScrollArea::SCROLL_UP,   Raptor::Game->Res.GetAnimation("arrow_up.ani"),   Raptor::Game->Res.GetAnimation("arrow_up_mdown.ani")   ) );
	AddElement( new ScrollAreaButton( ScrollArea::SCROLL_DOWN, Raptor::Game->Res.GetAnimation("arrow_down.ani"), Raptor::Game->Res.GetAnimation("arrow_down_mdown.ani") ) );
	UpdateRects();
}


ScrollArea::~ScrollArea()
{
}


int ScrollArea::MaxY( void ) const
{
	int max_y = 0;
	
	for( std::list<Layer*>::const_iterator element_iter = Elements.begin(); element_iter != Elements.end(); element_iter ++ )
	{
		if( ((*element_iter)->Name != "ScrollAreaButton") )
		{
			int y = (*element_iter)->Rect.y + (*element_iter)->Rect.h;
			if( y > max_y )
				max_y = y;
		}
	}
	
	return max_y;
}


int ScrollArea::MaxScroll( void ) const
{
	int max_scroll = MaxY() - Rect.h;
	return (max_scroll > 0) ? max_scroll : 0;
}


void ScrollArea::ScrollUp( int dy )
{
	ScrollY -= dy ? dy : ScrollBy;
	
	if( ScrollY < 0 )
		ScrollY = 0;
}


void ScrollArea::ScrollDown( int dy )
{
	ScrollY += dy ? dy : ScrollBy;
	
	int max_scroll = MaxScroll();
	if( ScrollY > max_scroll )
		ScrollY = max_scroll;
}


void ScrollArea::ScrollTo( int y )
{
	ScrollY = y;
	
	if( ScrollY <= 0 )
		ScrollY = 0;
	else
	{
		int max_scroll = MaxScroll();
		if( ScrollY > max_scroll )
			ScrollY = max_scroll;
	}
}


void ScrollArea::UpdateRects( void )
{
	for( std::list<Layer*>::iterator element_iter = Elements.begin(); element_iter != Elements.end(); element_iter ++ )
	{
		if( (*element_iter)->Name == "ScrollAreaButton" )
		{
			ScrollAreaButton *button = (ScrollAreaButton*)(*element_iter);
			button->UpdateRect();
		}
	}
}


void ScrollArea::UpdateCalcRects( int offset_x, int offset_y )
{
	Layer::UpdateCalcRects( offset_x, offset_y );
	
	std::map<Layer*,SDL_Rect> button_rects;
	button_rects[ this ] = CalcRect;
	for( std::list<Layer*>::iterator element_iter = Elements.begin(); element_iter != Elements.end(); element_iter ++ )
	{
		if( (*element_iter)->Name == "ScrollAreaButton" )
			button_rects[ *element_iter ] = (*element_iter)->CalcRect;
	}
	
	float ui_scale = UIScaleMode ? Raptor::Game->UIScale : 1.f;
	Layer::UpdateCalcRects( offset_x - ScrollX * ui_scale, offset_y - ScrollY * ui_scale );
	
	for( std::map<Layer*,SDL_Rect>::iterator button_iter = button_rects.begin(); button_iter != button_rects.end(); button_iter ++ )
		button_iter->first->CalcRect = button_iter->second;
}


void ScrollArea::Draw( void )
{
	glColor4f( Red, Green, Blue, Alpha );
	glBegin( GL_QUADS );
		glVertex2i( 0, 0 );
		glVertex2i( Rect.w, 0 );
		glVertex2i( Rect.w, Rect.h );
		glVertex2i( 0, Rect.h );
	glEnd();
	glColor4f( 1.f, 1.f, 1.f, 1.f );
}


void ScrollArea::DrawElements( void )
{
	glPushAttrib( GL_VIEWPORT_BIT );
	Raptor::Game->Gfx.SetViewport( CalcRect.x, CalcRect.y, CalcRect.w, CalcRect.h );
	// FIXME: While this does work because only the main client thread should use Gfx.W/Gfx.H, there must be a cleaner way!
	int gfx_w = Raptor::Game->Gfx.W, gfx_h = Raptor::Game->Gfx.H;
	Raptor::Game->Gfx.W = CalcRect.w;
	Raptor::Game->Gfx.H = CalcRect.h;
	
	UpdateRects();
	CalcRect.x = 0;
	CalcRect.y = 0;
	float ui_scale = UIScaleMode ? Raptor::Game->UIScale : 1.f;
	for( std::list<Layer*>::iterator element_iter = Elements.begin(); element_iter != Elements.end(); element_iter ++ )
	{
		int y = ((*element_iter)->Name == "ScrollAreaButton") ? 0 : (int)( -ScrollY * ui_scale - 0.5f );
		(*element_iter)->UpdateCalcRects( 0, y );
	}
	
	Layer::DrawElements();
	
	Raptor::Game->Gfx.W = gfx_w;
	Raptor::Game->Gfx.H = gfx_h;
	UpdateCalcRects();
	glPopAttrib();
	
	// Draw the scrollbar.
	DrawSetup();
	int max_scroll = MaxScroll();
	if( max_scroll > 0 )
	{
		double percent_displayed = Rect.h / (double) MaxY();
		double scroll_start = (ScrollY / (double) max_scroll) * (1. - percent_displayed);
		int scaled_scroll_bar = ScrollBarSize * ui_scale + 0.5f;
		int h = CalcRect.h - scaled_scroll_bar * 2;
		Raptor::Game->Gfx.DrawRect2D( CalcRect.w - scaled_scroll_bar, (int)( scaled_scroll_bar + h * scroll_start + 0.5 ), CalcRect.w, (int)( scaled_scroll_bar + h * (scroll_start + percent_displayed) + 0.5 ), 0, ScrollBarRed, ScrollBarGreen, ScrollBarBlue, ScrollBarAlpha );
	}
}


void ScrollArea::TrackEvent( SDL_Event *event )
{
	Layer::TrackEvent( event );
	
	if( MouseIsDown && ClickedScrollBar && (event->type == SDL_MOUSEMOTION) )
	{
		// Update scroll state as we drag the scroll bar.
		
		float ui_scale = UIScaleMode ? Raptor::Game->UIScale : 1.f;
		int scaled_scroll_bar = ScrollBarSize * ui_scale + 0.5f;
		int y = Raptor::Game->Mouse.Y - CalcRect.y - scaled_scroll_bar;
		int h = CalcRect.h - scaled_scroll_bar * 2;
		
		int scroll = h ? (y / (float) h) * MaxY() * ui_scale - CalcRect.h * 0.5f : 0;
		if( scroll < 0 )
			scroll = 0;
		
		ScrollTo( scroll / ui_scale );
	}
}


bool ScrollArea::HandleEvent( SDL_Event *event )
{
	if( (event->type == SDL_MOUSEMOTION) && ! WithinCalcRect( event->motion.x, event->motion.y ) )
		return false;
	else if( ((event->type == SDL_MOUSEBUTTONDOWN) || (event->type == SDL_MOUSEBUTTONUP)) && ! WithinCalcRect( event->button.x, event->button.y ) )
		return false;
	
	bool handled = Layer::HandleEvent( event );
	
#if SDL_VERSION_ATLEAST(2,26,0)
	if( (event->type == SDL_MOUSEWHEEL) && event->wheel.y && WithinCalcRect( event->wheel.mouseX, event->wheel.mouseY ) )
		return true;
#elif SDL_VERSION_ATLEAST(2,0,0)
	if( (event->type == SDL_MOUSEWHEEL) && event->wheel.y && WithinCalcRect( Raptor::Game->Mouse.X - Raptor::Game->Mouse.OffsetX, Raptor::Game->Mouse.Y - Raptor::Game->Mouse.OffsetY ) ) // FIXME: Messy.
		return true;
#endif
	
	return handled;
}


void ScrollArea::MouseEnter( void )
{
}


void ScrollArea::MouseLeave( void )
{
}


bool ScrollArea::MouseDown( Uint8 button )
{
	if( ((button == SDL_BUTTON_WHEELDOWN) || (button == SDL_BUTTON_WHEELUP)) && ! Selected )
		return true;
	else if( button == SDL_BUTTON_LEFT )
	{
		float ui_scale = UIScaleMode ? Raptor::Game->UIScale : 1.f;
		int scaled_scroll_bar = ScrollBarSize * ui_scale + 0.5f;
		ClickedScrollBar = Raptor::Game->Mouse.X > CalcRect.x + CalcRect.w - scaled_scroll_bar;
		if( ClickedScrollBar )
		{
			int y = Raptor::Game->Mouse.Y - CalcRect.y - scaled_scroll_bar;
			int h = CalcRect.h - scaled_scroll_bar * 2;
			
			int scroll = h ? (y / (float) h) * MaxY() * ui_scale - CalcRect.h * 0.5f : 0;
			if( scroll < 0 )
				scroll = 0;
			
			ScrollTo( scroll / ui_scale );
			
			return true;
		}
	}
	
	return false;
}


bool ScrollArea::MouseUp( Uint8 button )
{
	if( (button == SDL_BUTTON_WHEELDOWN) && ! Selected )
		ScrollDown();
	else if( (button == SDL_BUTTON_WHEELUP) && ! Selected )
		ScrollUp();
	else if( (button == SDL_BUTTON_LEFT) && ClickedScrollBar )
	{
		ClickedScrollBar = false;
		
		float ui_scale = UIScaleMode ? Raptor::Game->UIScale : 1.f;
		int scaled_scroll_bar = ScrollBarSize * ui_scale + 0.5f;
		int y = Raptor::Game->Mouse.Y - CalcRect.y - scaled_scroll_bar;
		int h = CalcRect.h - scaled_scroll_bar * 2;
		
		int scroll = h ? (y / (float) h) * MaxY() * ui_scale - CalcRect.h * 0.5f : 0;
		if( scroll < 0 )
			scroll = 0;
		
		ScrollTo( scroll / ui_scale );
		
		Selected = NULL;
	}
	else
		return false;
	
	return true;
}


// ---------------------------------------------------------------------------


ScrollAreaButton::ScrollAreaButton( uint8_t action, Animation *normal, Animation *mouse_down ) : Button( NULL, normal, mouse_down )
{
	Name = "ScrollAreaButton";
	Action = action;
}


void ScrollAreaButton::UpdateRect( void )
{
	if( ! Container )
		return;
	
	ScrollArea *scroll_area = (ScrollArea*) Container;
	
	int scroll_button_h = scroll_area->ScrollBarSize;
	if( scroll_button_h * 2 > scroll_area->Rect.h )
		scroll_button_h = scroll_area->Rect.h / 2;
	
	if( Action == ScrollArea::SCROLL_UP )
	{
		Rect.x = scroll_area->Rect.w - scroll_area->ScrollBarSize;
		Rect.y = 0;
		Rect.w = scroll_area->ScrollBarSize;
		Rect.h = scroll_button_h;
	}
	else if( Action == ScrollArea::SCROLL_DOWN )
	{
		Rect.x = scroll_area->Rect.w - scroll_area->ScrollBarSize;
		Rect.y = scroll_area->Rect.h - scroll_area->ScrollBarSize;
		Rect.w = scroll_area->ScrollBarSize;
		Rect.h = scroll_button_h;
	}
}


void ScrollAreaButton::Clicked( Uint8 button )
{
	ScrollArea *scroll_area = (ScrollArea*) Container;
	if( (Action == ScrollArea::SCROLL_UP) && scroll_area )
		scroll_area->ScrollUp();
	else if( (Action == ScrollArea::SCROLL_DOWN) && scroll_area )
		scroll_area->ScrollDown();
}
