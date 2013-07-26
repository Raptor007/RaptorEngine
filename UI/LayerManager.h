/*
 *  LayerManager.h
 */

#pragma once
class LayerManager;

#include "PlatformSpecific.h"

#include <vector>
#include "Layer.h"


class LayerManager
{
public:
	std::vector<Layer *> Layers;
	bool Dirty;
	
	LayerManager( void );
	~LayerManager();
	
	void Draw( void );
	bool HandleEvent( SDL_Event *event );
	
	void Cleanup( void );
	
	void Add( Layer *layer );
	void Remove( Layer *layer );
	void Remove( int index );
	void RemoveAll( void );
	
	bool IsTop( Layer *layer );
	Layer *TopLayer( void );
};
