/*
 *  LayerManager.cpp
 */

#include "LayerManager.h"

#include <cstddef>
#include <stdexcept>
#include "RaptorGame.h"


LayerManager::LayerManager( void )
{
	Dirty = false;
}


LayerManager::~LayerManager()
{
}


void LayerManager::Draw( void )
{
	Cleanup();
	
	for( std::list<Layer*>::iterator layer_iter = Layers.begin(); layer_iter != Layers.end(); layer_iter ++ )
	{
		// Draw all Layers, from bottom to top.
		(*layer_iter)->UpdateCalcRects();
		if( (*layer_iter)->Visible )
		{
			(*layer_iter)->DrawSetup();
			(*layer_iter)->Draw();
			(*layer_iter)->DrawElements();
		}
	}
}


void LayerManager::TrackEvent( SDL_Event *event )
{
	Cleanup();
	
	// Pass the event down the stack so all layers can see it.
	for( std::list<Layer*>::reverse_iterator layer_iter = Layers.rbegin(); layer_iter != Layers.rend(); layer_iter ++ )
		(*layer_iter)->TrackEvent( event );
}


bool LayerManager::HandleEvent( SDL_Event *event )
{
	Cleanup();
	
	// Pass the event down the stack until one of the layers processes it.
	for( std::list<Layer*>::reverse_iterator layer_iter = Layers.rbegin(); layer_iter != Layers.rend(); layer_iter ++ )
	{
		if( (*layer_iter)->HandleEvent( event ) )
			return true;
	}
	
	// Event was not handled by any Layer; check for special events.
	if( event->type == SDL_QUIT )
	{
		RemoveAll();
		return true;
	}
	
	// If unhandled, return false.
	return false;
}


void LayerManager::Cleanup( void )
{
	if( Dirty )
	{
		Dirty = false;
		
		for( std::list<Layer*>::iterator layer_iter = Layers.begin(); layer_iter != Layers.end(); )
		{
			std::list<Layer*>::iterator layer_iter_next = layer_iter;
			layer_iter_next ++;
			
			Layer *layer = *layer_iter;
			if( layer->Removed )
			{
				Layers.erase( layer_iter );
				delete layer;
			}
			else if( layer->Dirty )
				layer->Cleanup();
			
			layer_iter = layer_iter_next;
		}
	}
}


void LayerManager::Add( Layer *layer )
{
	Layers.push_back( layer );
}


void LayerManager::Remove( Layer *layer )
{
	layer->Remove();
}


bool LayerManager::RemoveTop( void )
{
	for( std::list<Layer*>::reverse_iterator layer_iter = Layers.rbegin(); layer_iter != Layers.rend(); layer_iter ++ )
	{
		if( ! (*layer_iter)->Removed )
		{
			(*layer_iter)->Remove();
			return true;
		}
	}
	
	return false;
}


void LayerManager::RemoveAll( void )
{
	for( std::list<Layer*>::iterator layer_iter = Layers.begin(); layer_iter != Layers.end(); layer_iter ++ )
		(*layer_iter)->Remove();
}


bool LayerManager::IsTop( Layer *layer )
{
	return layer == TopLayer();
}


Layer *LayerManager::TopLayer( void )
{
	for( std::list<Layer*>::reverse_iterator layer_iter = Layers.rbegin(); layer_iter != Layers.rend(); layer_iter ++ )
	{
		if( ! (*layer_iter)->Removed )
			return *layer_iter;
	}
	
	return NULL;
}


Layer *LayerManager::Find( const std::string &name )
{
	for( std::list<Layer*>::reverse_iterator layer_iter = Layers.rbegin(); layer_iter != Layers.rend(); layer_iter ++ )
	{
		if( (! (*layer_iter)->Removed) && ((*layer_iter)->Name == name) )
			return *layer_iter;
	}
	
	return NULL;
}
