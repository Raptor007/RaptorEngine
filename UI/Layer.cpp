/*
 *  Layer.cpp
 */

#include "Layer.h"

#include "Math2D.h"
#include "RaptorGame.h"


Layer::Layer( SDL_Rect *rect )
{
	Container = NULL;
	
	Removed = false;
	Dirty = false;
	Enabled = true;
	Visible = true;
	Draggable = false;
	
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
	
	CalcRect.x = Rect.x;
	CalcRect.y = Rect.y;
	CalcRect.w = Rect.w;
	CalcRect.h = Rect.h;
	
	Red = 1.f;
	Green = 1.f;
	Blue = 1.f;
	Alpha = 1.f;
}


Layer::~Layer()
{
	Removed = true;
	Enabled = false;
	Visible = false;
	
	Container = NULL;
	Selected = NULL;
	
	for( std::list<Layer*>::iterator layer_iter = Elements.begin(); layer_iter != Elements.end(); )
	{
		std::list<Layer*>::iterator layer_iter_next = layer_iter;
		layer_iter_next ++;
		
		Layer *layer = *layer_iter;
		Elements.erase( layer_iter );
		delete layer;
		
		layer_iter = layer_iter_next;
	}
}


void Layer::Cleanup( void )
{
	if( Dirty )
	{
		Dirty = false;
		
		for( std::list<Layer*>::iterator layer_iter = Elements.begin(); layer_iter != Elements.end(); )
		{
			std::list<Layer*>::iterator layer_iter_next = layer_iter;
			layer_iter_next ++;
			
			Layer *layer = *layer_iter;
			if( layer->Removed )
			{
				Elements.erase( layer_iter );
				delete layer;
			}
			else if( layer->Dirty )
				layer->Cleanup();
			
			layer_iter = layer_iter_next;
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
	
	for( std::list<Layer*>::iterator layer_iter = Elements.begin(); layer_iter != Elements.end(); layer_iter ++ )
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
	for( std::list<Layer*>::iterator layer_iter = Elements.begin(); layer_iter != Elements.end(); layer_iter ++ )
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


void Layer::TrackEvent( SDL_Event *event )
{
	// If we are dragging this layer, we don't want its elements to track the mouse motion.
	if( Draggable && MouseIsWithin && MouseIsDown && (event->type == SDL_MOUSEMOTION) )
		return;
	
	// Pass the event down the stack so the elements can track it too.
	for( std::list<Layer*>::reverse_iterator layer_iter = Elements.rbegin(); layer_iter != Elements.rend(); layer_iter ++ )
		(*layer_iter)->TrackEvent( event );
	
	
	// Watch for MouseEnter/MouseLeave.
	
	bool mouse_is_within = MouseIsWithin;
	
	if( event->type == SDL_MOUSEMOTION )
	{
		mouse_is_within = WithinCalcRect( event->motion.x, event->motion.y );
	}
	else if( (event->type == SDL_MOUSEBUTTONDOWN) || (event->type == SDL_MOUSEBUTTONUP) )
	{
		mouse_is_within = WithinCalcRect( event->button.x, event->button.y );
	}
	
	if( mouse_is_within != MouseIsWithin )
	{
		MouseIsWithin = mouse_is_within;
		
		if( mouse_is_within )
			MouseEnter();
		else
			MouseLeave();
	}
	
	if( MouseIsDown && (event->type == SDL_MOUSEBUTTONUP) && ! mouse_is_within )
		MouseIsDown = false;
}


bool Layer::HandleEvent( SDL_Event *event )
{
	if( ! Enabled )
		return false;
	
	
	// Pass the event down the stack until one of the elements processes it.
	for( std::list<Layer*>::reverse_iterator layer_iter = Elements.rbegin(); layer_iter != Elements.rend(); layer_iter ++ )
	{
		if( (*layer_iter)->HandleEvent( event ) )
			return true;
	}
	
	
	// See what kind of event this is, and handle accordingly.
	
	bool handled = false;
	
	bool mouse_is_within = MouseIsWithin;
	bool mouse_is_down = MouseIsDown;
	Uint8 mouse_button = 0;
	
	if( event->type == SDL_MOUSEMOTION )
	{
		if( Draggable && MouseIsWithin && MouseIsDown )
		{
			Rect.x += event->motion.xrel;
			Rect.y += event->motion.yrel;
			return true;
		}
		
		mouse_is_within = WithinCalcRect( event->motion.x, event->motion.y );
	}
	else if( event->type == SDL_MOUSEBUTTONDOWN )
	{
		mouse_is_within = WithinCalcRect( event->button.x, event->button.y );
		mouse_is_down = true;
		mouse_button = event->button.button;
	}
	else if( event->type == SDL_MOUSEBUTTONUP )
	{
		mouse_is_within = WithinCalcRect( event->button.x, event->button.y );
		mouse_is_down = false;
		mouse_button = event->button.button;
	}
	else if( event->type == SDL_KEYDOWN )
	{
		handled = KeyDown( event->key.keysym.sym );
	}
	else if( event->type == SDL_KEYUP )
	{
		handled = KeyUp( event->key.keysym.sym );
	}
	
	if( mouse_is_within && (mouse_is_down != MouseIsDown) )
	{
		MouseIsDown = mouse_is_down;
		
		if( mouse_is_down )
			handled = MouseDown( mouse_button );
		else
			handled = MouseUp( mouse_button );
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
	return Draggable && (button != SDL_BUTTON_WHEELUP) && (button != SDL_BUTTON_WHEELDOWN);
}


bool Layer::MouseUp( Uint8 button )
{
	return Draggable && (button != SDL_BUTTON_WHEELUP) && (button != SDL_BUTTON_WHEELDOWN);
}


bool Layer::KeyDown( SDLKey key )
{
	return false;
}


bool Layer::KeyUp( SDLKey key )
{
	return false;
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
	element->Container = this;
	Elements.push_back( element );
}


void Layer::RemoveElement( Layer *element )
{
	element->Remove();
}


void Layer::RemoveAllElements( void )
{
	for( std::list<Layer*>::iterator layer_iter = Elements.begin(); layer_iter != Elements.end(); layer_iter ++ )
		(*layer_iter)->Remove();
}


bool Layer::IsTop( bool and_containers )
{
	if( Container )
	{
		if( and_containers )
			return (this == Container->TopElement()) && Container->IsTop();
		else
			return this == Container->TopElement();
	}
	else
		return this == Raptor::Game->Layers.TopLayer();
}


Layer *Layer::TopElement( void )
{
	if( Elements.size() )
		return Elements.back();
	
	return NULL;
}


void Layer::MoveElementToTop( Layer *element )
{
	if( element == TopElement() )
		return;
	
	for( std::list<Layer*>::iterator layer_iter = Elements.begin(); layer_iter != Elements.end(); layer_iter ++ )
	{
		if( *layer_iter == element )
		{
			Elements.erase( layer_iter );
			break;
		}
	}
	
	Elements.push_back( element );
}


void Layer::MoveToTop( bool and_containers )
{
	if( Container )
	{
		Container->MoveElementToTop( this );
		if( and_containers )
			Container->MoveToTop( true );
	}
	else
		Raptor::Game->Layers.MoveToTop( this );
}


size_t Layer::RemoveOthersAbove( bool and_containers )
{
	size_t removed = 0;
	if( Container )
	{
		while( ! IsTop(false) )
		{
			Container->RemoveElement( Container->TopElement() );
			removed ++;
		}
		if( and_containers )
			removed += Container->RemoveOthersAbove( true );
	}
	else while( ! IsTop(true) && Raptor::Game->Layers.RemoveTop() )
		removed ++;
	return removed;
}


Layer *Layer::FindElement( const std::string &name, bool recursive )
{
	for( std::list<Layer*>::reverse_iterator layer_iter = Elements.rbegin(); layer_iter != Elements.rend(); layer_iter ++ )
	{
		if( ! (*layer_iter)->Removed )
		{
			if( (*layer_iter)->Name == name )
				return *layer_iter;
			
			if( recursive )
			{
				Layer *found = (*layer_iter)->FindElement( name, true );
				if( found )
					return found;
			}
		}
	}
	
	return NULL;
}


void Layer::SizeToContainer( int buffer )
{
	Rect.x = buffer;
	Rect.y = buffer;
	
	if( Container )
	{
		Rect.w = Container->Rect.w - buffer * 2;
		Rect.h = Container->Rect.h - buffer * 2;
	}
	else
	{
		Rect.x = Raptor::Game->Gfx.W - buffer * 2;
		Rect.y = Raptor::Game->Gfx.H - buffer * 2;
	}
}


void Layer::SizeToElements( int max_buffer )
{
	int buffer = max_buffer, max_x = 0, max_y = 0;
	
	for( std::list<Layer*>::const_iterator layer_iter = Elements.begin(); layer_iter != Elements.end(); layer_iter ++ )
	{
		int x1 = std::max<int>( 0, (*layer_iter)->Rect.x );
		int x2 = std::max<int>( 0, (*layer_iter)->Rect.x + (*layer_iter)->Rect.w );
		int y2 = std::max<int>( 0, (*layer_iter)->Rect.y + (*layer_iter)->Rect.h );
		if( x1 < buffer )
			buffer = x1;
		if( x2 > max_x )
			max_x = x2;
		if( y2 > max_y )
			max_y = y2;
	}
	
	Rect.w = max_x + buffer;
	Rect.h = max_y + buffer;
}


void Layer::Center( void )
{
	if( Container )
	{
		Rect.x = Container->Rect.w/2 - Rect.w/2;
		Rect.y = Container->Rect.h/2 - Rect.h/2;
	}
	else
	{
		Rect.x = Raptor::Game->Gfx.W/2 - Rect.w/2;
		Rect.y = Raptor::Game->Gfx.H/2 - Rect.h/2;
	}
}
