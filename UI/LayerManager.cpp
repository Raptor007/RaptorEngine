/*
 *  LayerManager.cpp
 */

#include "LayerManager.h"

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
	
	for( std::vector<Layer*>::iterator layer_iter = Layers.begin(); layer_iter != Layers.end(); layer_iter ++ )
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


bool LayerManager::HandleEvent( SDL_Event *event )
{
	Cleanup();

	if( Layers.size() )
	{
		for( std::vector<Layer*>::iterator layer_iter = Layers.end() - 1; true; layer_iter -- )
		{
			// Pass the event down the stack until one of the layers processes it.
			if( bool result = (*layer_iter)->HandleEvent( event ) )
				return result;
			else if( layer_iter == Layers.begin() )
				break;
		}
	}
	
	// Event was not handled by any Layer; check for special events.
	if( event->type == SDL_QUIT )
	{
		RemoveAll();
		return true;
	}
	
	// If unhandled, return 0.
	return false;
}


void LayerManager::Cleanup( void )
{
	if( Dirty )
	{
		Dirty = false;
		
		for( int i = Layers.size() - 1; i >= 0; i -- )
		{
			try
			{
				Layer *layer = Layers.at( i );
				if( layer->Removed )
				{
					// Create an iterator so the vector::erase method will work properly.
					std::vector<Layer *>::iterator iter = Layers.begin() + i;
					Layers.erase( iter );
					
					delete layer;
				}
				else if( layer->Dirty )
					layer->Cleanup();
			}
			catch( std::out_of_range &exception )
			{
				fprintf( stderr, "LayerManager::Cleanup: std::out_of_range\n" );
				Dirty = true;
			}
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


void LayerManager::Remove( int index )
{
	try
	{
		Layer *layer = Layers.at( index );
		layer->Remove();
	}
	catch( std::out_of_range &exception )
	{
		fprintf( stderr, "LayerManager::RemoveLayer: std::out_of_range\n" );
	}
}


void LayerManager::RemoveAll( void )
{
	Layers.clear();
}


bool LayerManager::IsTop( Layer *layer )
{
	return layer == TopLayer();
}


Layer *LayerManager::TopLayer( void )
{
	if( int size = Layers.size() )
	{
		try
		{
			return Layers.at( size - 1 );
		}
		catch( std::out_of_range &exception )
		{
			fprintf( stderr, "LayerManager::TopLayer: std::out_of_range\n" );
		}
	}

	return NULL;
}

