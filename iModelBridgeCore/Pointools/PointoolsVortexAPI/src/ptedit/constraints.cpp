#include "PointoolsVortexAPIInternal.h"

#ifdef NEEDS_WORK_VORTEX_DGNDB
#include <ptedit/constriants.h>

using namespace ptedit;

#ifndef max3
	#ifndef max
		#define max(a,b) (((a)>(b) ? (a) : (b)))
		#define min(a,b) (((a)<(b) ? (a) : (b)))
	#endif

	#define max3(a,b,c) (max((a), max((b),(c))))
	#define min3(a,b,c) (min((a), min((b),(c))))
#endif

#define CONS_COL_MATCH(CX) \
			(instance(0).matchCol[0] == GetRValue((CX)) && instance(0).matchCol[1] == GetGValue((CX)) && instance(0).matchCol[2] == GetBValue(CX) )

//
// Colour constraint implementation
//
ColConstraint::ColConstraint() : EditConstraint("ColConstraint"), tolerance(0)
{
	matchCol[0] = 0;
	matchCol[1] = 0;
	matchCol[2] = 0;
}
//
void ColConstraint::setCol(COLORREF col, int tol)
{
	if (tol < 1) tol = 1;
	if (tol > 255) tol = 255;

	if (instance(0).tolerance != tol
		|| !CONS_COL_MATCH(col))
	{
		for (int i=0;i<EDT_MAX_THREADS;i++)
		{
			instance(i).tolerance = tol;
			instance(i).matchCol[0] = GetRValue(col);
			instance(i).matchCol[1] = GetGValue(col);
			instance(i).matchCol[2] = GetBValue(col);
			instance(i).stateIndex++;
		}
	}
}
//
bool ColConstraint::match(int t, pcloud::Voxel *v, const pt::vector3d &p, uint i)
{
	pcloud::DataChannel *dc[EDT_MAX_THREADS];
	dc[t] = v->channel(pcloud::PCloud_RGB);
	if (!dc[t]) return true;

	int tol[EDT_MAX_THREADS];
	tol[t] = instance(t).tolerance;

	const ubyte *mcol[EDT_MAX_THREADS];
	mcol[t] = instance(t).matchCol;

	ubyte *src[EDT_MAX_THREADS];
	v->channel(pcloud::PCloud_RGB)->getptr(&src[t], i);
	
	return ( abs((int)src[t][0] - mcol[t][0]) < tol[t]
		&& abs((int)src[t][1] - mcol[t][1]) < tol[t]
		&& abs((int)src[t][2] - mcol[t][2]) < tol[t])
		? true : false;
}
//
bool ColConstraint::readState(const pt::datatree::Branch *b)
{
	uint col;
	int tol;
	if ( b->getNode( "matchCol", col ) 
		&& b->getNode( "tolerance", tol ))
	{
		setCol( col, tol );
		return true;
	}
	return false;
}
//
bool ColConstraint::writeState( pt::datatree::Branch *b) const
{
	if (writtenStateIndex != stateIndex)
	{
		uint col = RGB( matchCol[0], matchCol[1], matchCol[2]);
		b->addNode( "matchCol", col );
		b->addNode( "tolerance", tolerance );
		
		const_cast<ColConstraint*>(this)->writtenStateIndex = stateIndex;
		return true;
	}
	else return false;
}
//
// Greyscale constraint implementation
//
void GreyConstraint::setTol(int tol)
{
	if (tol < 1) tol = 1;
	if (tol > 255) tol = 255;

	if (tol != instance(0).tolerance)
	{
		for (int i=0;i<EDT_MAX_THREADS;i++)
		{
			instance(i).tolerance = tol;
			instance(i).stateIndex++;
		}
	}
}
//
// Grey Constraint
//
bool GreyConstraint::match( int t, pcloud::Voxel *v, const pt::vector3d &p, uint i )
{
	pcloud::DataChannel *dc[EDT_MAX_THREADS];
	dc[t] = v->channel(pcloud::PCloud_RGB);
	if (!dc[t]) return true;

	int tol[EDT_MAX_THREADS];
	tol[t] = instance(t).tolerance;

	ubyte *src[EDT_MAX_THREADS];
	v->channel(pcloud::PCloud_RGB)->getptr(&src[t], i);
	
	return ( abs((int)src[t][0] - src[t][1]) < tol[t] &&
		 abs((int)src[t][1] - src[t][2] < tol[t]) &&
		 abs((int)src[t][0] - src[t][2] < tol[t]))
		 ? true : false;
}
//
bool GreyConstraint::readState(const pt::datatree::Branch *b)
{
	 return b->getNode("tolerance", tolerance);
}
//
bool GreyConstraint::writeState( pt::datatree::Branch *b) const
{
	if (writtenStateIndex != stateIndex)
	{
		b->addNode("tolerance", tolerance);
		const_cast<GreyConstraint*>(this)->writtenStateIndex = stateIndex;
		return true;
	}
	return false;
}
//
// Hue Constraint
//
void HueConstraint::setHue(COLORREF huecol, int tol)
{
	if (tol < 1) tol = 1;
	if (tol > 255) tol = 255;

	if (tol != instance(0).tolerance)
	{		
		pt::Color c;
		c.rgb( GetRValue(huecol), GetGValue(huecol), GetBValue(huecol) );
	
		for (int i=0;i<EDT_MAX_THREADS;i++)
		{
			instance(i).tolerance = tol;
			instance(i).hue = c.hue();
			instance(i).stateIndex++;
		}
	}
}
//
bool HueConstraint::match(int t, pcloud::Voxel *v, const pt::vector3d &p, uint i)
{
	pcloud::DataChannel *dc[EDT_MAX_THREADS];
	dc[t] = v->channel(pcloud::PCloud_RGB);
	if (!dc[t]) return true;

	int tol[EDT_MAX_THREADS];
	tol[t] = instance(t).tolerance;

	int mhue[EDT_MAX_THREADS];
	mhue[t] = static_cast<int>(instance(t).hue);

	ubyte *src[EDT_MAX_THREADS];
	v->channel(pcloud::PCloud_RGB)->getptr(&src[t], i);

	pt::Color h[EDT_MAX_THREADS];
	h[t].rgb(src[t][0], src[t][1], src[t][2]);
	int ch[EDT_MAX_THREADS];
	ch[t] = static_cast<int>(h[t].hue());

	return ( abs(ch[t] - mhue[t]) < tol[t] ) ? true : false;
}
//
bool HueConstraint::readState(const pt::datatree::Branch *b)
{
	int tol;
	if ( b->getNode( "hue", hue ) 
		&& b->getNode( "tolerance", tol ))
	{
		return true;
	}
	return false;
}
//
bool HueConstraint::writeState( pt::datatree::Branch *b) const
{
	if (writtenStateIndex != stateIndex)
	{
		b->addNode( "hue", hue );
		b->addNode( "tolerance", tolerance );
		const_cast<HueConstraint*>(this)->writtenStateIndex = stateIndex;
		return true;
	}
	else return false;
}
//
// Lum Constraint
//
void LumConstraint::setLum( COLORREF lumcol, int tol )
{
	if (tol < 1) tol = 1;
	if (tol > 255) tol = 255;

	if (tol != instance(0).tolerance)
	{
		pt::Color c;
		c.rgb( GetRValue(lumcol), GetGValue(lumcol), GetBValue(lumcol) );

		for (int i=0;i<EDT_MAX_THREADS;i++)
		{
			instance(i).tolerance = tol;
			instance(i).lum = static_cast<int>(c.luminance() * 510);
			instance(i).stateIndex++;
		}
	}
}
//
bool LumConstraint::match(int t, pcloud::Voxel *v, const pt::vector3d &p, uint i)
{
	pcloud::DataChannel *dc[EDT_MAX_THREADS];
	dc[t] = v->channel(pcloud::PCloud_RGB);
	if (!dc[t]) return true;

	int tol[EDT_MAX_THREADS];
	tol[t] = instance(t).tolerance;

	int mlum[EDT_MAX_THREADS];
	mlum[t] = instance(t).lum;

	ubyte *src[EDT_MAX_THREADS];
	v->channel(pcloud::PCloud_RGB)->getptr(&src[t], i);

	int mn[EDT_MAX_THREADS]; 
	int mx[EDT_MAX_THREADS];
	mn[t] = min3(src[t][0], src[t][1], src[t][2]);
	mx[t] = max3(src[t][0], src[t][1], src[t][2]);
	int cl[EDT_MAX_THREADS];
	cl[t] = (mx[t] + mn[t]);

	return ( abs(cl[t] - mlum[t]) < tol[t] ) ? true : false;
}

//
bool LumConstraint::readState(const pt::datatree::Branch *b)
{
	int tol;
	if ( b->getNode( "lum", lum ) 
		&& b->getNode( "tolerance", tol ))
	{
		return true;
	}
	return false;
}
//
bool LumConstraint::writeState( pt::datatree::Branch *b) const
{
	if (writtenStateIndex != stateIndex)
	{
		b->addNode( "lum", lum );
		b->addNode( "tolerance", tolerance );
		const_cast<LumConstraint*>(this)->writtenStateIndex = stateIndex;
		return true;
	}
	else return false;
}
//
// Intensity Constraint
//
void IntensityConstraint::setIntensity( short intens, int tol )
{
	if (tol != instance(0).intensity != intens)
	{
		for (int i=0;i<EDT_MAX_THREADS;i++)
		{
			instance(i).intensity = intens;
			instance(i).tolerance = tol;
			instance(i).stateIndex++;
		}
	}
}
//
bool IntensityConstraint::match(int t, pcloud::Voxel *v, const pt::vector3d &p, uint i)
{
	pcloud::DataChannel *dc[EDT_MAX_THREADS];
	dc[t] = v->channel(pcloud::PCloud_Intensity);
	if (!dc[t]) return true;

	int tol[EDT_MAX_THREADS];
	tol[t] = instance(t).tolerance;

	short intens[EDT_MAX_THREADS];
	intens[t] = instance(t).intensity;

	short pint[EDT_MAX_THREADS];
	v->channel(pcloud::PCloud_Intensity)->getval(pint[t], i);

	return ( abs(pint[t] - intens[t]) < tol[t] ) ? true : false;
}

//
bool IntensityConstraint::readState(const pt::datatree::Branch *b)
{
	int tol;
	int intens;
	if ( b->getNode( "intensity", intens ) 
		&& b->getNode( "tolerance", tol ))
	{
		intensity = intens;
		return true;
	}
	return false;
}
//
bool IntensityConstraint::writeState( pt::datatree::Branch *b) const
{
	if (writtenStateIndex != stateIndex)
	{
		int intens = intensity;

		b->addNode( "intensity", intens );
		b->addNode( "tolerance", tolerance );
		const_cast<IntensityConstraint*>(this)->writtenStateIndex = stateIndex;
		return true;
	}
	else return false;
}
#endif