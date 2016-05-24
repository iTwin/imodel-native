#pragma once

#include <ptedit/editSelect.h>
#include <ptedit/editState.h>
#include <ptedit/editNodeDef.h>
#include <ptedit/editFilters.h>
#include <ptedit/editApply.h>
#include <pt/plane.h>
#include <math.h>

#ifndef min
#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))
#endif

#define LUM(r,g,b) ((min(r,min(g,b)) + max(r,max(g,b))) * 0.5)

namespace ptedit
{
	enum BrushMode
	{
		ColourBrush = 0,
		EraseBrush = 1,
		BurnBrush = 2,
		DodgeBrush = 3		
	};

	enum BrushShape
	{
		BrushShapeBox = 1,
		BrushShapeBall = 2
	};

	struct PaintBrush
	{
		PaintBrush() : _mode(ColourBrush), _shape(BrushShapeBall), 
			_radius(0.5f), _radius2(0.25f) {  createBrush(); }

		inline const double &radius() const { return _radius; }
		inline const double &radius2() const { return _radius2; }

		void shape(BrushShape s)	{ _shape = s; }
		inline const BrushShape &shape() const { return _shape; }

		void radius(double r);

		inline const float &alpha(int p) const { return br[p]; }
		inline const float &ialpha(int p) const { return ibr[p]; }

		inline const int &mode() { return _mode; }
		void mode( int m ) { _mode = m; }
	private:
		double	_radius2;
		double	_radius;	
		BrushShape	_shape;
		int			_mode;

		float br[256];
		float ibr[256];

		void createBrush()
		{
			for (int i=0; i<256; i++)
			{
				ibr[i] = i / 256.0f;
				br[i] = 1.0f - ibr[i];
			}
		}
	};

	enum BlendFunc
	{
		PaintModeNormal		=0,
		PaintModeMultiply	=1,
		PaintModeOverlay	=2,
		PaintModeDarken		=3,
		PaintModeLighten	=4,
		PaintModeScreen		=5,
		PaintModeLinearDodge =6,
		PaintModeLinearBurn =7,
		PaintModeBlendIntensity = 8
	};

	struct PaintSettings
	{
		PaintSettings();

		float	alpha;
		ubyte	col[3];
		PaintBrush	brush;
		BlendFunc	blend;

		void readState( const pt::datatree::Branch* b );
		void writeState( pt::datatree::Branch* b );
	};

	struct SelBrushSettings
	{
		SelBrushSettings();

		PaintBrush	brush;
		void readState( const pt::datatree::Branch* b );
		void writeState( pt::datatree::Branch* b );
	};

	/* active paint and brush settings */ 
	extern PaintSettings		g_paint;
	extern SelBrushSettings		g_selBrush;
	//
	// paint select
	//
	struct PaintSelect : public EditNodeDef
	{
		PaintSelect();

		/* don't add EditNodePostConsolidateVis since groups always consolidate anyway */ 
		DECLARE_EDIT_NODE("PaintSelect", "Paint Selection", 2, 
			EditNodeMultithread )

		SELECTION_FILTER_DETAIL

		void setCenter(const pt::vector3 &cen);
		void setCenter(const pt::vector3d &cen);
		const pt::vector3d &getCenter() const { return center; }

		bool writeState(pt::datatree::Branch *b) const
		{
			b->addNode("cen", center);
			return true;
		}
		bool readState(const pt::datatree::Branch *b)
		{
			pt::vector3d c;
			b->getNode("cen", c);
			setCenter(c);
			return true;
		}
		bool apply()
		{
			_didSelect = 0;
			SelectionFilter<PaintSelect> s(*this);
			s.processFilter();
			_didSelect = s.didSelect();
			return _didSelect ? true : false;
		}
		int didSelect() const { return _didSelect; }
		
		static void drawWidget(const PaintSelect &) {}

		/* static intersect routines */ 
		static SelectionResult intersect(const PaintSelect &paint, const pt::BoundingBoxD &bb);

		inline static bool inside(int t, const PaintSelect &paint, const pt::vector3d &pnt)
		{
			if (g_selBrush.brush.shape() == BrushShapeBox )
			{
				int i[EDT_MAX_THREADS];
				for (i[t]=0; i[t]<3; i[t]++)
				{
					if ( pnt[i[t]] > paint.box.upper(i[t])) break;
					if ( pnt[i[t]] < paint.box.lower(i[t])) break;
				}
				return (i[t] == 3) ? true : false;
			}
			else
			{
				pt::vector3d d[EDT_MAX_THREADS];
				d[t] = paint.center - pnt;
				float dist[EDT_MAX_THREADS];
				dist[t] = static_cast<float>(d[t].x*d[t].x+d[t].y*d[t].y+d[t].z*d[t].z);

				return dist[t] < g_selBrush.brush.radius2() ? true : false;
			}
		}

		bool getSample( int x, int y, pt::vector3 pnt)
		{
			return false;
		}

	private:
		friend struct PaintIntersect;
		pt::vector3d	center;
		pt::BoundingBoxD box;
		int _didSelect;
	};
	//
	// Paint intersect routine for brush selection / painting
	//
	struct PaintIntersect
	{
		static bool pointInside(int t, const pt::vector3d &cen, const pt::vector3d &pnt)
		{
			pt::vector3d d[EDT_MAX_THREADS];
			double dist[EDT_MAX_THREADS];

			d[t] = cen - pnt;
			dist[t] = d[t].x*d[t].x+d[t].y*d[t].y+d[t].z*d[t].z;

			return dist[t] < g_paint.brush.radius2() ? true : false;
		}

		static SelectionResult intersect(const pt::BoundingBoxD &box, const pt::vector3d &cen, const pt::BoundingBoxD &bb)
		{
			SelectionResult res = FullyOutside;

			if ( g_paint.brush.shape() == BrushShapeBox )
			{
				if (box.contains(&bb)) res = FullyInside;
				else if (box.intersects(&bb)) res = PartiallyInside;
			}
			else
			{
				/* check for full inclusion */ 
				pt::vector3d corner;
				int i=0;
				for (; i<8; i++)
				{
					bb.getExtrema(i, corner);
					if (!pointInside(0, cen, corner)) break;
				}
				if (i == 8) res = FullyInside;
				else
				{
					float s, d = 0;

					for( int i=0 ; i<3 ; i++ ) 
					{
						if( cen[i] < bb.lower(i) )
						{
							s = static_cast<float>(cen[i] - bb.lower(i));
							d += s*s;
						}

						else if( cen[i] > bb.upper(i) )
						{
							s = static_cast<float>(cen[i] - bb.upper(i));
							d += s*s;
						}

					}
					res = (d <= g_paint.brush.radius2()) ? PartiallyInside : FullyOutside;
				}
			}
			return res;
		}
	};
	
	//
	// Blending
	//
	struct BlendMode
	{
		inline static void clamp(const int &v, ubyte &c)
		{
			if (v<0) c = 0;
			else if (v>255) c = 255;
			else c = (ubyte)v;
		}
		inline static void clamp(const int *v, ubyte *rgb )
		{
			clamp(v[0], rgb[0]);
			clamp(v[1], rgb[1]);
			clamp(v[2], rgb[2]);
		}
		static void blend( const ubyte *src, const ubyte *dst, float a, float ia, int *res )
		{
			if ( a > 0.9999f)
			{
				res[0] = src[0]; 
				res[1] = src[1]; 
				res[2] = src[2];
			}
			else
			{
				res[0] = static_cast<int>(dst[0] * ia);
				res[1] = static_cast<int>(dst[1] * ia);
				res[2] = static_cast<int>(dst[2] * ia);

				res[0] += static_cast<int>(a * src[0]);
				res[1] += static_cast<int>(a * src[1]);
				res[2] += static_cast<int>(a * src[2]);	
			}
		}
		static void mixBack( int *res, const ubyte *dst, float a, float ia)
		{
			res[0] = static_cast<int>((a * res[0]) + (ia * dst[0]));
			res[1] = static_cast<int>((a * res[1]) + (ia * dst[1]));
			res[2] = static_cast<int>((a * res[2]) + (ia * dst[2]));
		}
		static void multiply( const ubyte *src, const ubyte *dst, float a, float ia, int *res )
		{
			res[0] = static_cast<int>((float)dst[0] * (float)src[0] / 255);
			res[1] = static_cast<int>((float)dst[1] * (float)src[1] / 255);
			res[2] = static_cast<int>((float)dst[2] * (float)src[2] / 255);

			mixBack( res, dst, a, ia);
		}
		static void overlay( const ubyte *src, const ubyte *dst, float a, float ia, int *res )
		{
			if (dst[0] + dst[1] + dst[2] < 384)
			{
				res[0]= static_cast<int>((float)( 2 * dst[0] * src[0] ) / 255);
				res[1]= static_cast<int>((float)( 2 * dst[1] * src[1] ) / 255);
				res[2]= static_cast<int>((float)( 2 * dst[2] * src[2] ) / 255);
			}
			else
			{
				res[0]= static_cast<int>(255 - ((float)( 2* (255-dst[0])*(255-src[0]) ) / 255));
				res[1]= static_cast<int>(255 - ((float)( 2* (255-dst[1])*(255-src[1]) ) / 255));
				res[2]= static_cast<int>(255 - ((float)( 2* (255-dst[2])*(255-src[2]) ) / 255));
			}
			mixBack( res, dst, a, ia);
		}

		static void screen( const ubyte *src, const ubyte *dst, float a, float ia, int *res )
		{
			res[0] = 255 - (((255 - dst[0]) * (255 - src[0])) >> 8);
			res[1] = 255 - (((255 - dst[1]) * (255 - src[1])) >> 8);
			res[2] = 255 - (((255 - dst[2]) * (255 - src[2])) >> 8);
			mixBack( res, dst, a, ia);
		}

		static void lineardodge( const ubyte *src, const ubyte *dst, float a, float ia, int *res )
		{
			res[0] = (min(255, (src[0] + dst[0])));
			res[1] = (min(255, (src[1] + dst[1])));
			res[2] = (min(255, (src[2] + dst[2])));
			mixBack( res, dst, a, ia);
		}

		static void linearburn( const ubyte *src, const ubyte *dst, float a, float ia, int *res )
		{
			res[0] = (((src[0] + dst[0] < 255) ? 0:(src[0] + dst[0] - 255)));
			res[1] = (((src[1] + dst[1] < 255) ? 0:(src[1] + dst[1] - 255)));
			res[2] = (((src[2] + dst[2] < 255) ? 0:(src[2] + dst[2] - 255)));
			mixBack( res, dst, a, ia);
		}

		static void blend( const int &mode, int t, const ubyte* src, const ubyte *dst, float a, ubyte* res)
		{
			static int _tmp[EDT_MAX_THREADS*3];
			static float ia[EDT_MAX_THREADS];

			ia[t] = 1.0f - a;
			int *tmp = &_tmp[t*3];

			if (!res) res = const_cast<ubyte*>(dst);

			switch(mode)
			{
			case PaintModeNormal:
				blend(src, dst, a, ia[t], tmp); 
				clamp(tmp, res);
				break;

			case PaintModeMultiply:
				multiply(src, dst, a, ia[t], tmp);
				clamp(tmp, res);
				break;

			case PaintModeOverlay:
				overlay(src, dst, a, ia[t], tmp);
				clamp(tmp, res);
				break;

			case PaintModeDarken:
				blend(src, dst, a, ia[t], tmp);
				if ( LUM(tmp[0], tmp[1], tmp[2]) < LUM(dst[0], dst[1], dst[2]))
					clamp(tmp, res);
				break;

			case PaintModeLighten:
				blend(src, dst, a, ia[t], tmp);
				if ( LUM(tmp[0], tmp[1], tmp[2]) > LUM(dst[0], dst[1], dst[2]))
					clamp(tmp, res);
				break;

			case PaintModeScreen:
				screen(src, dst, a, ia[t], tmp);
				clamp(tmp, res);
				break;

			case PaintModeLinearDodge:
				lineardodge(src, dst, a, ia[t], tmp);
				clamp(tmp, res);
				break;

			case PaintModeLinearBurn:
				linearburn(src, dst, a, ia[t], tmp);
				clamp(tmp, res);
				break;

			default:
				break;
			}
		}

	};
	//
	// Fill Paint Filter
	//
	struct PaintFillFilterVisitor : public pcloud::Node::Visitor
	{
		bool visitNode( const pcloud::Node *n )
		{
			pcloud::Node *node = const_cast<pcloud::Node*>(n);
			
			if ( node->layers(1) & g_activeLayers
				|| node->layers(0) & g_activeLayers)
			{
				if (node->isLeaf())
				{
					pcloud::Voxel::LocalSpaceTransform lst;
					_currentVoxel = static_cast<pcloud::Voxel*>(node);
					
					rgb = _currentVoxel->channel(pcloud::PCloud_RGB);
					if (!rgb) return false;

					std::lock_guard<std::mutex> lock(_currentVoxel->mutex());
					VoxFiltering::iteratePoints( _currentVoxel, *this, lst );
					VoxFiltering::setPoint( _currentVoxel );
					_currentVoxel->flag(pcloud::Painted, true);
				}
				return true;
			}
			return false;
		}
		inline void mt_point(int t, const pt::vector3d &p, uint index, ubyte &f) 
		{ 
			if (f & g_activeLayers) 
			{ 
				BlendMode::blend( g_paint.blend, t, g_paint.col, 
					(ubyte*)rgb->element(index), g_paint.alpha, 0);
			}
		}
		inline void point(const pt::vector3d &p, uint index, ubyte &f) 
		{ 
			mt_point(0, p, index, f);
		}
		pcloud::DataChannel *rgb;
		pcloud::Voxel *_currentVoxel;
	};
	//
	class FilterOpPaintFill : public EditNodeDef
	{
	public:
		FilterOpPaintFill() : EditNodeDef("PaintFill") {};

		DECLARE_EDIT_NODE("PaintFill", "Paint Fill", 10, EditNodeMultithread | EditNodeDoesPaint)

		bool apply()
		{
			PaintFillFilterVisitor v;
			TraverseScene::withVisitor(&v);
			return true;
		}
	};
	//static FilterOpPaintFill s_opPaintFill;

	const ubyte *getOriginalRGB(const pcloud::Voxel*v, int p);

	//
	// Fill Erase Filter
	//
	struct EraseFilterVisitor : public pcloud::Node::Visitor
	{
		bool visitNode( const pcloud::Node *n )
		{
			if (NodeCheck::isExcluded(n)) return false;

			pcloud::Node *node = const_cast<pcloud::Node*>(n);
			
			if ( node->layers(1) & g_activeLayers
				|| node->layers(0) & g_activeLayers)
			{
				if (node->isLeaf() && node->flag(pcloud::Painted)) 
				{
					pcloud::Voxel::LocalSpaceTransform lst;
					_currentVoxel = static_cast<pcloud::Voxel*>(node);
					
					rgb = _currentVoxel->channel(pcloud::PCloud_RGB);
					if (!rgb) return false;

                    std::lock_guard<std::mutex> lock(_currentVoxel->mutex());
					VoxFiltering::iteratePoints( _currentVoxel, *this, lst );
					VoxFiltering::setPoint( _currentVoxel );
					
					if (!_currentVoxel->flag(pcloud::WholeHidden) 
						&& !_currentVoxel->flag(pcloud::PartHidden)
						&& g_paint.blend == PaintModeNormal
						&& g_paint.alpha > 0.999f)
					{
						_currentVoxel->flag(pcloud::Painted, false);
						clearRGBCache( _currentVoxel );
					}
				}
				return true;
			}
			return false;
		}
		inline void mt_point(int t, const pt::vector3d &p, uint index, ubyte &f) 
		{ 
			if (f & g_activeLayers) 
			{ 
				BlendMode::blend( g_paint.blend, t, getOriginalRGB(_currentVoxel, index), 
					(ubyte*)rgb->element(index), g_paint.alpha, 0);
			}
		}

		inline void point(const pt::vector3d &p, uint index, ubyte &f) 
		{ 
			mt_point(0, p, index, f);
		}
		void clearRGBCache( pcloud::Voxel *v );

		pcloud::DataChannel *rgb;
		pcloud::Voxel *_currentVoxel;
	};

	//
	class FilterOpPaintErase : public EditNodeDef
	{
	public:
		FilterOpPaintErase() : EditNodeDef("PaintErase") {};
		DECLARE_EDIT_NODE("PaintErase", "Paint Erase", 10, EditNodeDoesPaint )

		bool apply()
		{
			EraseFilterVisitor v;
			TraverseScene::withVisitor(&v);
			return true;
		}
	};
	//static FilterOpPaintErase s_opPaintErase;

#ifdef NEEDS_WORK_VORTEX_DGNDB
	//
	// Paint Filter - for painting not select
	//
	struct BrushPaintRGB : public EditNodeDef
	{
		/* don't add EditNodePostConsolidateVis since groups always consolidate anyway */ 

		BrushPaintRGB() : EditNodeDef("BrushPaintRGB") {}
		DECLARE_EDIT_NODE( "BrushPaintRGB", "Brush Paint RGB", 2, 
			EditNodeMultithread | EditNodeDoesPaint )

		PAINT_FILTER_DETAIL

		void setCenter(const pt::vector3 &cen);
		const pt::vector3d &getCenter() const { return center; }

		bool writeState(pt::datatree::Branch *b) const
		{
			b->addNode("cen", center);
			return true;
		}
		bool readState(const pt::datatree::Branch *b)
		{
			b->getNode("cen", center);
			box.centerAt(center);
			return true;
		}
		bool apply()
		{
			_didSelect = 0;
			SelectionFilter<BrushPaintRGB> s(*this);
			s.processFilter();
			_didSelect = s.didSelect();
			return _didSelect ? true : false;
		}
		int didSelect() const { return _didSelect; }

		/* save rgb channel for erase */ 
		static void brushPaint(int t, BrushPaintRGB &paint, const ubyte * col, ubyte *dst, float mult)
		{
			static int d[EDT_MAX_THREADS];
			static uint _tmp[EDT_MAX_THREADS*3];
			static float b[EDT_MAX_THREADS];

			uint *tmp = &_tmp[t*3];
			
			d[t] = static_cast<int>(255 * lastDist(t) / g_paint.brush.radius2());
			b[t] = g_paint.brush.alpha(d[t]) * g_paint.alpha;

			if (mult >= 0)
			{
				ubyte col_adj[EDT_MAX_THREADS*3];
				col_adj[t*3]   = static_cast<ubyte>((float)col[0] * mult);
				col_adj[t*3+1] = static_cast<ubyte>((float)col[1] * mult);
				col_adj[t*3+2] = static_cast<ubyte>((float)col[2] * mult);

				BlendMode::blend( PaintModeNormal, t, col_adj, dst, b[t], dst );
			}
			else
				BlendMode::blend( g_paint.blend, t, col, dst, b[t], dst );
		}

		inline static void paintPoint(int t, BrushPaintRGB *paint, pcloud::Voxel *v, const pt::vector3d &p, uint i)
		{
			if (!v->channel(pcloud::PCloud_RGB)) return;

			ubyte *src[EDT_MAX_THREADS];
			v->channel(pcloud::PCloud_RGB)->getptr(&src[t], i);

			v->flag(pcloud::Painted, true);

			if (g_paint.brush.mode() == EraseBrush)
				brushPaint(t, *paint, getOriginalRGB(v, i), src[t], -1.0f);
			else 
			{
				float m[EDT_MAX_THREADS];
				m[t] = -1;			
				if (g_paint.blend == PaintModeBlendIntensity)
				{
					if (v->channel(pcloud::PCloud_Intensity))
					{
						short it[EDT_MAX_THREADS];
						v->channel(pcloud::PCloud_Intensity)->getval(it[t], i);
						m[t] = (float)it[t] / 65535.0f + 0.5f;
						m[t] *= 0.5f;
						m[t] += 0.5f;
					}
				}
				brushPaint(t, *paint, g_paint.col, src[t], m[t]);
			}
		}
		inline static void paintPoint(int t, BrushPaintRGB *paint, pcloud::Voxel *v, const pt::vector3 &p, uint i)
		{
			paintPoint(t, paint, v, pt::vector3d(p), i); 
		}

		static SelectionResult intersect(const BrushPaintRGB &paint, const pt::BoundingBoxD &bb)
		{
			return PaintIntersect::intersect( paint.box, paint.center, bb ); 
		}
		inline static float &lastDist(int thread) { static float d[EDT_MAX_THREADS]; return d[thread]; }

		inline static bool inside(int t, const BrushPaintRGB &paint, const pt::vector3d &pnt)
		{
			if (g_paint.brush.shape() == BrushShapeBox)
			{
				int i[EDT_MAX_THREADS];
				for (i[t]=0; i[t]<3; i[t]++)
				{
					if ( pnt[i[t]] > paint.box.upper(i[t])) break;
					if ( pnt[i[t]] < paint.box.lower(i[t])) break;
				}
				lastDist(t) = 0;
				return (i[t] == 3) ? true : false;
			}
			else
			{
				pt::vector3d d[EDT_MAX_THREADS];
				d[t] = paint.center - pnt;

				float dist[EDT_MAX_THREADS];
				dist[t] = static_cast<float>(d[t].x*d[t].x+d[t].y*d[t].y+d[t].z*d[t].z);

				lastDist(t) = dist[t];
				return dist[t] < g_paint.brush.radius2() ? true : false;
			}
		}
		static void drawWidget(const BrushPaintRGB &data) {}
		
		bool getSample( int screen_x, int screen_y, pt::vector3 pnt)
		{
			/* get colour under cursor */ 
			HDC dc = GetWindowDC( 0 );
			POINT mpnt;
			mpnt.x = screen_x;
			mpnt.y = screen_y;
			int col = GetPixel( dc, mpnt.x, mpnt.y );

			g_paint.col[0] = GetRValue(col);
			g_paint.col[1] = GetGValue(col);
			g_paint.col[2] = GetBValue(col);
			return true;
		}
	private:
		pt::vector3d	center;
		pt::BoundingBoxD box;
		int _didSelect;
	};

#endif

} // end namespace ptedit