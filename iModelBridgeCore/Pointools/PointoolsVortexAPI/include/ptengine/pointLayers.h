#pragma once

#include <ptengine/ptengine_api.h>
#include <ptedit/edit.h>
#include <ptcloud2/node.h>
#include <pt/typedefs.h>
#include <pt/ptstring.h>
#include <ptgl/color.h>

// Holds the current layer state - used only in Edit
// does not do anything other than hold the layer state and allow this to be shared
// is not concerned with layer locking
namespace pointsengine
{
enum InLayer
{
	FullExclusion = 0,
	PartialInclusion = 1,
	FullInclusion = 2
};
class PTENGINE_API PointLayersState
{
public:

	inline const uint &visibleBitMask() const { return _visibleLayersBitMask; }
	inline const uint &currentBitMask() const { return _currentLayerBitMask; }

	void setVisibleBitMask( const uint &mask ) { if (_visibleLayersBitMask != mask) { _visibleLayersBitMask = mask;} }
	void setCurrentBitMask( const uint &mask ) { if (_currentLayerBitMask != mask) { _currentLayerBitMask = mask; } }

	bool initialize();

	void setLayerName( int lyr, const pt::String &name );
	const pt::String &getLayerName( int lyr ) const;

	inline InLayer inVisibleLayer( const pcloud::Node *node ) const 
	{
		if (node->layers(1) & (ubyte)_visibleLayersBitMask) return PartialInclusion;
		if (node->layers(0) & (ubyte)_visibleLayersBitMask) return FullInclusion;
		return FullExclusion;
	}
	inline InLayer inLayer( int layerIndex, const pcloud::Node *node )
	{
		ubyte layerMask = 1 << layerIndex;
		if (node->layers(0) & layerMask) return FullInclusion;
		if (node->layers(1) & layerMask) return PartialInclusion;
		return FullExclusion;
	}
	inline ubyte pointLayerMask( uint layerMask )
	{
		return layerMask & ~SELECTED_PNT_BIT;
	}
	inline ubyte pointVisLayerMask()
	{
		return _visibleLayersBitMask & ~SELECTED_PNT_BIT;
	}
	int numLayers() const { return 7; }

	void	setLayerColorAlpha( uint layerMask, float a )
	{
		if (_colors[layerMask % 256].a != a)
		{
			_colors[layerMask % 256].a = a;
			_state++;
		}
	}
	void	setLayerColor( uint layerMask, ptgl::Color &col )
	{
		if ( _colors[layerMask % 256] != col )
		{
			_colors[layerMask % 256] = col;
			_state++;
		}
	}	
	const ptgl::Color &getLayerColor( uint layerMask ) const
	{
		return _colors[layerMask%256];
	}
	void	resetLayerColors( void );

	// flag used to issue shader texture up to date
	uint	stateID() const			
	{ 
		return _state; 
	}

	// return a hash that encapsulates the state
	uint	stateHash( void ) const
	{
		return (_state * 256 + _visibleLayersBitMask);
	}
private:
	uint		_visibleLayersBitMask;
	uint		_currentLayerBitMask;

	ptgl::Color	_colors[256];
	pt::String	_names[32];

	uint		_state;
};
}