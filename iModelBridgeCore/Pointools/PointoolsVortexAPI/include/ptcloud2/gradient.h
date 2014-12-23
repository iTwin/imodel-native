#pragma once

#include <pt/color.h>
#include <pt/typedefs.h>

namespace pt
{
	enum GradInterp
	{
		GradInterpLinear,
		GradInterpSpline,	// not implemented
		GradInterpStep
	};
	enum GradColMode
	{
		GradColRGB,
		GradColHSL
	};
//-----------------------------------------------------------------------------
// Grad Key Class
//-----------------------------------------------------------------------------
class GradKey
{
public:

	GradKey() 
		: 
		mode( GradColRGB ), 
		interp( GradInterpLinear ), 
		transparency(0),
		pos(0),
		next(0),
		prev(0)
	{}
	
	float			position() const	{ return pos; }
	GradKey			*nextKey()			{ return next; }
	GradKey			*prevKey()			{ return prev; }

	GradColMode		mode;
	GradInterp		interp;
	pt::Color		colour;
	float			transparency;

	friend class	Gradient;

	GradKey			*first();
	GradKey			*last();

	bool			insert( GradKey *key );

private:
	void			insertBefore( GradKey *key );
	void			insertAfter( GradKey *key );

	GradKey			*next;
	GradKey			*prev;
	float			pos;

};
//-----------------------------------------------------------------------------
// Gradient Class
//-----------------------------------------------------------------------------
class Gradient
{
public:
	Gradient();

	~Gradient();

	void			addKey( float pos, const pt::Color &color, GradColMode mode, 
							GradInterp interp=GradInterpLinear, float transparency=0 );

	void			remKey( float pos );
	void			remKey( int index );

	void			normalise();


	const ubyte		*img();
	int				imgWidth() const;
	int				imgHeight() const;
	void			updateImg();
	
	void			reverse();

	void			setInterpolation( GradInterp interp );
	void			setSize( int size );

private:
	void			updateGradientImg();
	void			fillInterpolatedGradient( const GradKey *a, const GradKey *b );

	ubyte		   *m_gradImg;
	int				m_gradSize;

	GradKey		   *m_firstKey;
};
}