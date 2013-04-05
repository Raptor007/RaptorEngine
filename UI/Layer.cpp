/*
 *  Layer.cpp
 */

#include "Layer.h"

#include "Math2D.h"
#include "RaptorGame.h"


Layer::Layer( Layer *container, SDL_Rect *rect )
{
	Container = container;
	
	Removed = false;
	Dirty = false;
	Enabled = true;
	Visible = true;
	
	MouseIsWithin = false;
	MouseIsDown = false;
	Selected = NULL;
	
	if( rect )
	{
		Rect.x = rect->x;
		Rect.y = rect->y;
		Rect.w = rect->w;
		Rect.h = rect->h;
	}
	else
	{
		Rect.x = 0;
		Rect.y = 0;
		Rect.w = Raptor::Game->Gfx.W;
		Rect.h = Raptor::Game->Gfx.H;
	}
	
	Red = 1.f;
	Green = 1.f;
	Blue = 1.f;
	Alpha = 1.f;
	
	UpdateCalcRects();
}


Layer::~Layer()
{
	Removed = true;
	Enabled = false;
	Visible = false;
	
	Container = NULL;
	Selected = NULL;
	
	for( int i = Elements.size() - 1; i >= 0; i -- )
	{
		try
		{
			Layer *element = Elements.at( i );
			delete element;
			Elements[ i ] = NULL;
		}
		catch( std::out_of_range &exception )
		{
			fprintf( stderr, "Layer::~Layer: std::out_of_range\n" );
		}
	}
	
	Elements.clear();
}


void Layer::Cleanup( void )
{
	if( Dirty )
	{
		Dirty = false;

		for( int i = Elements.size() - 1; i >= 0; i -- )
		{
			try
			{
				Layer *element = Elements.at( i );
				if( element->Removed )
				{
					// Create an iterator so the vector::erase method will work properly.
					std::vector<Layer *>::iterator iter = Elements.begin() + i;
					Elements.erase( iter );

					delete element;
				}
				else if( element->Dirty )
					element->Cleanup();
			}
			catch( std::out_of_range &exception )
			{
				fprintf( stderr, "Layer::Cleanup: std::out_of_range\n" );
				Dirty = true;
			}
		}
	}
}


void Layer::UpdateCalcRects( int offset_x, int offset_y )
{
	CalcRect.w = Rect.w;
	CalcRect.h = Rect.h;
	
	if( Container )
	{
		CalcRect.x = Rect.x + Container->CalcRect.x + offset_x;
		CalcRect.y = Rect.y + Container->CalcRect.y + offset_y;
	}
	else
	{
		CalcRect.x = Rect.x + offset_x;
		CalcRect.y = Rect.y + offset_y;
	}
	
	for( std::vector<Layer*>::iterator layer_iter = Elements.begin(); layer_iter != Elements.end(); layer_iter ++ )
	{
		(*layer_iter)->UpdateCalcRects();
	}
}


void Layer::DrawSetup( void )
{
	// This assumes 2D; override this method in a 3D layer.
	Raptor::Game->Gfx.Setup2D( 0 - CalcRect.x, 0 - CalcRect.y, Raptor::Game->Gfx.W - CalcRect.x, Raptor::Game->Gfx.H - CalcRect.y );
}


void Layer::Draw( void )
{
	// This method should be overridden.
}


void Layer::DrawElements( void )
{
	for( std::vector<Layer*>::iterator layer_iter = Elements.begin(); layer_iter != Elements.end(); layer_iter ++ )
	{
		// Draw all Layers, from bottom to top.
		if( (*layer_iter)->Visible )
		{
			(*layer_iter)->DrawSetup();
			(*layer_iter)->Draw();
			(*layer_iter)->DrawElements();
		}
	}
}


bool Layer::WithinCalcRect( int x, int y )
{
	return Math2D::WithinRect( x, y, CalcRect.x, CalcRect.y, CalcRect.x + CalcRect.w, CalcRect.y + CalcRect.h );
}



bool Layer::IsSelected( void )
{
	return Container && (Container->Selected == this);
}


bool Layer::HandleEvent( SDL_Event *event, bool already_handled )
{
	if( ! Enabled )
		already_handled = true;
	
	bool handled = false;
	
	
	// Pass the event down the stack to see if one of the elements processes it.
	
	for( std::vector<Layer*>::iterator layer_iter = Elements.begin(); layer_iter != Elements.end(); layer_iter ++ )
	{
		if( (*layer_iter)->HandleEvent( event, handled ) )
			handled = true;
	}
	
	
	// See what kind of event this is, and handle accordingly.
	
	bool mouse_is_within = MouseIsWithin;
	bool mouse_is_down = MouseIsDown;
	Uint8 button = 0;
	
	if( event->type == SDL_MOUSEMOTION )
		mouse_is_within = WithinCalcRect( event->motion.x, event->motion.y );
	else if( event->type == SDL_MOUSEBUTTONDOWN )
	{
		mouse_is_within = WithinCalcRect( event->button.x, event->button.y );
		mouse_is_down = true;
		button = event->button.button;
	}
	else if( event->type == SDL_MOUSEBUTTONUP )
	{
		mouse_is_within = WithinCalcRect( event->button.x, event->button.y );
		mouse_is_down = false;
		button = event->button.button;
	}
	else if( event->type == SDL_KEYDOWN )
	{
		if( !( handled || already_handled ) )
			handled = KeyDown( event->key.keysym.sym );
	}
	else if( event->type == SDL_KEYUP )
	{
		if( !( handled || already_handled ) )
			handled = KeyUp( event->key.keysym.sym );
	}
	
	if( mouse_is_within != MouseIsWithin )
	{
		MouseIsWithin = mouse_is_within;
		
		// Always do MouseEnter and MouseLeave, even if the event was handled.
		if( mouse_is_within )
			MouseEnter();
		else
			MouseLeave();
	}
	
	if( mouse_is_down != MouseIsDown )
	{
		// Only track MouseIsDown if it's within this element and wasn't already handled.
		if( (MouseIsWithin || ! mouse_is_down) && !( handled || already_handled ) )
			MouseIsDown = mouse_is_down;
		
		// If the mouse is within this element and the event hasn't been handled yet, do MouseDown or MouseUp.
		if( MouseIsWithin && !( handled || already_handled ) )
		{
			if( mouse_is_down )
				handled = MouseDown( button );
			else
				handled = MouseUp( button );
		}
	}
	
	
	return handled;
}


void Layer::MouseEnter( void )
{
}


void Layer::MouseLeave( void )
{
}


bool Layer::MouseDown( Uint8 button )
{
	return false;
}


bool Layer::MouseUp( Uint8 button )
{
	return false;
}


bool Layer::KeyDown( SDLKey key )
{
	return false;
}


bool Layer::KeyUp( SDLKey key )
{
	return false;
}


bool Layer::IsTop( void )
{
	if( Container )
		return (this == Container->TopElement()) && Container->IsTop();
	else
		return this == Raptor::Game->Layers.TopLayer();
}


Layer *Layer::TopElement( void )
{
	if( int size = Elements.size() )
	{
		try
		{
			return Elements.at( size - 1 );
		}
		catch( std::out_of_range exception )
		{
			fprintf( stderr, "Layer::TopElement: std::out_of_range\n" );
		}
	}
	
	return NULL;
}


void Layer::Remove( void )
{
	Removed = true;
	SetDirty();
}


void Layer::SetDirty( void )
{
	Dirty = true;

	if( Container )
		Container->SetDirty();
	else
		Raptor::Game->Layers.Dirty = true;
}


void Layer::AddElement( Layer *element )
{
	Elements.push_back( element );
}


void Layer::RemoveElement( Layer *element )
{
	element->Remove();
}


void Layer::RemoveElement( int index )
{
	try
	{
		Layer *element = Elements.at( index );
		element->Remove();
	}
	catch( std::out_of_range &exception )
	{
		fprintf( stderr, "State::RemoveElement: std::out_of_range\n" );
	}
}

