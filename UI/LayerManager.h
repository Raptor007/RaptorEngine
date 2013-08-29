/*
 *  LayerManager.h
 */

#pragma once
class LayerManager;

#include "PlatformSpecific.h"

#include <list>
#include "Layer.h"


class LayerManager
{
public:
	std::list<Layer*> Layers;
	bool Dirty;
	
	LayerManager( void );
	virtual ~LayerManager();
	
	void Draw( void );
	void TrackEvent( SDL_Event *event );
	bool HandleEvent( SDL_Event *event );
	
	void Cleanup( void );
	
	void Add( Layer *layer );
	void Remove( Layer *layer );
	void RemoveAll( void );
	
	bool IsTop( Layer *layer );
	Layer *TopLayer( void );
};
