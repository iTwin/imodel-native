#include "PointoolsVortexAPIInternal.h"
#include <ptcloud2/gradient.h>
#include <vector>
#include <assert.h>

using namespace pt;

#define GRADIENT_SIZE	256

//-----------------------------------------------------------------------------
bool GradKey::insert( GradKey *key )
{
	if ( key->pos == pos ) key->pos += (1.0 / GRADIENT_SIZE);

	if ( key->pos < pos )
	{
		if (!prev || prev->pos < key->pos)
		{
			insertBefore(key);
		}
		else if (prev) return prev->insert( key );
		return false;
	}
	else if ( key->pos > pos)
	{
		if (!next || next->pos > key->pos)
		{
			insertAfter(key);
		}
		else if (next) return next->insert( key );
		return false;	
	}
	return true;
}
//-----------------------------------------------------------------------------
GradKey *GradKey::first() 
{ 
	if (!prev) return this; 
	else return prev->first(); 
}
//-----------------------------------------------------------------------------
GradKey *GradKey::last() 
{ 
	if (!next) return this; 
	else return next->last(); 
}
//-----------------------------------------------------------------------------
void GradKey::insertBefore( GradKey *key )
{
	assert( pos > key->pos && (!prev || prev->pos < key->pos));

	GradKey *oldPrev = prev;
	
	prev = key;
	prev->next = this;
	prev->prev = oldPrev;
	
	if (oldPrev) oldPrev->next = key;
}
//-----------------------------------------------------------------------------
void GradKey::insertAfter( GradKey *key )
{
	assert( pos < key->pos && (!next || next->pos > key->pos));

	GradKey *oldNext = next;
	
	next = key;
	next->prev = this;
	next->next = oldNext;
	
	if (oldNext) oldNext->prev = key;
}
//-----------------------------------------------------------------------------
Gradient::Gradient()
{
	m_gradImg = 0;
	m_firstKey = 0;
	m_gradSize = GRADIENT_SIZE;
}
//-----------------------------------------------------------------------------
Gradient::~Gradient()
{
	if (m_gradImg)
		delete [] m_gradImg;
}
#ifndef lerp
#define lerp
#endif
//-----------------------------------------------------------------------------
void Gradient::fillInterpolatedGradient( const GradKey *a, const GradKey *b )
{
	assert( m_gradImg );
	if (!m_gradImg) return;

	/* handle end key case */ 
	GradKey end;
	memcpy( &end, a, sizeof(GradKey) );
	end.pos = 1.0f;

	if (a->pos > 0.9999f)	return;
	if (!b)	b = &end;		// if no end key, set end as copy of start at 1.0

    int xa = static_cast<int>(a->pos * m_gradSize);
	if (xa > m_gradSize) xa = m_gradSize;

	int xb = static_cast<int>(b->pos * m_gradSize);
	if (xb > m_gradSize) xb = m_gradSize;

	float ca1, ca2, ca3;
	float cb1, cb2, cb3;

	if (b->mode == GradColRGB)
	{
		ca1 = (float)a->colour.red();
		ca2 = (float)a->colour.green();
		ca3 = (float)a->colour.blue();

		cb1 = (float) b->colour.red();
		cb2 = (float) b->colour.green();
		cb3 = (float)b->colour.blue();
	}
	else
	{
		ca1 = a->colour.hue();
		ca2 = a->colour.saturation();		
		ca3 = a->colour.luminance();

		cb1 = b->colour.hue();
		cb2 = b->colour.saturation();
		cb3 = b->colour.luminance();
	}
	
	if (xa == 1) xa = 0;				// little hack to prevent black first pixel
	

	for (int i=xa; i<xb; i++)
	{
		float pos = ((float)(i-xa) / (xb-xa));
		float trans = a->transparency + pos * (b->transparency - b->transparency);

		int px = i * 4;
		pt::Color col;
		
		if (b->interp == GradInterpStep)
		{
			switch ( b->mode )
			{
			case GradColRGB:
				col.rgb( (int)ca1, (int)ca2, (int)ca3 );
				break;
					
			case GradColHSL:
				col.hsl( ca1, ca2, ca3 );
				break;
			}
		}
		else
		{
			// compute interpolated col
			switch ( b->mode )
			{
			case GradColRGB:
				col.rgb( static_cast<int>(ca1 + pos * (cb1 - ca1)),
						 static_cast<int>(ca2 + pos * (cb2 - ca2)),
						 static_cast<int>(ca3 + pos * (cb3 - ca3)));
				break;
					
			case GradColHSL:
				col.hsl( ca1 + pos * (cb1 - ca1),
						ca2 + pos * (cb2 - ca2),
						ca3 + pos * (cb3 - ca3));
			}
		}

		// set the colour in the img as BGR
		m_gradImg[px+2] = (ubyte)col.red();
		m_gradImg[px+1] = (ubyte)col.green();
		m_gradImg[px] = (ubyte)col.blue();

		m_gradImg[px+3] = (ubyte)(255 - (trans * 255));
	}
}
//-----------------------------------------------------------------------------
void Gradient::updateGradientImg()
{
	if (m_gradImg)
		delete [] m_gradImg;

	int size = m_gradSize * 4;
	size *= size;

	m_gradImg = new ubyte[ size ];

	memset( m_gradImg, 0, size );
	
	if (!m_firstKey) return;

	GradKey * key = m_firstKey;

	while (key)
	{
		fillInterpolatedGradient( key, key->next );
		key = key->next;
	}
	/* copy row into other rows */ 
	for (int i=0; i<m_gradSize; i++)
	{
		memcpy( &m_gradImg[ m_gradSize * 4 ], m_gradImg, m_gradSize * 4 ); 
	}
}
//-----------------------------------------------------------------------------
void Gradient::normalise()
{
	// get extents of positions
	if (!m_firstKey) return;
	
	GradKey *key = m_firstKey;
	float start = key->pos;
	float range = key->last()->pos - start;

	while (key)
	{
		key->pos -= start;
		key->pos /= range;
		key = key->next;
	}
}
//
void Gradient::setSize( int size )
{
	m_gradSize = size;
	if (m_gradImg) delete [] m_gradImg;		//invalidates images
	m_gradImg = 0;
}
//-----------------------------------------------------------------------------
void Gradient::setInterpolation( GradInterp interp )
{
	GradKey *key = m_firstKey;

	while (key)
	{
		key->interp = interp;
		key = key->next;
	}
}
//-----------------------------------------------------------------------------
void Gradient::reverse()
{	
	/* disconnect */ 
	GradKey *key = m_firstKey;
	GradKey *end = key->last();

	std::vector<GradKey*> keys;

	while (key)
	{
		key->pos = 1.0f - key->pos;
		keys.push_back( key );
		key = key->next;
	}
	
	m_firstKey = end;
	m_firstKey->prev = 0;

	for (int i= static_cast<int>(keys.size()-1); i>=0; i--)
	{
		m_firstKey->insert( keys[i] );
	}
}
//-----------------------------------------------------------------------------
void Gradient::updateImg()
{
	updateGradientImg();	
}
//-----------------------------------------------------------------------------
void Gradient::addKey( float pos, const pt::Color &color, GradColMode mode, GradInterp interp, float transparency )
{
	GradKey* key = new GradKey;
	key->colour = color;
	key->interp = interp;
	key->mode = mode;
	key->transparency = transparency;
	key->pos = pos;

	if ( !m_firstKey )
	{
		m_firstKey = key;
		m_firstKey->insert( key );
	}
	else
	{
		m_firstKey->insert( key );
		m_firstKey = m_firstKey->first();
	}
}
//-----------------------------------------------------------------------------
const ubyte		*Gradient::img()
{
	if (!m_gradImg) 
		updateGradientImg();

	return m_gradImg;
}
//-----------------------------------------------------------------------------
int	Gradient::imgWidth() const
{
	return m_gradSize;
}
//-----------------------------------------------------------------------------
int	Gradient::imgHeight() const
{
	return m_gradSize;
}
//-----------------------------------------------------------------------------