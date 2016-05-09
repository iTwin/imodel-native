#include "PointoolsVortexAPIInternal.h"
#include <pt/project.h>
#include <pt/rect.h>
#include <ptengine/projAreaApproximation.h>

using namespace pt;
using namespace ProjArea;

extern	double g_unitScale;

//-----------------------------------------------------------------------------
template <class V>
inline static float quadArea2(const V &a, const V &b, const V &c, const V &d)
{
	/*two triangles - abc + acd*/ 
	double area = fabs(((b.x - a.x)*(c.y-a.y)) - ((c.x - a.x) * (b.y - a.y)));
	area += fabs(((c.x - a.x)*(d.y-a.y)) - ((d.x - a.x) * (c.y - a.y)));
	area *= 0.5;
	return static_cast<float>(area);
}
//-----------------------------------------------------------------------------
static void getVoxelBoundingBox( const pcloud::Node *v, pt::BoundingBoxD &bb )
{
	vector3d basePoint(Project3D::project().registration().matrix()(3,0), 
		Project3D::project().registration().matrix()(3,1), 
		Project3D::project().registration().matrix()(3,2));

	bb = v->extents();	

	bb.translateBy( -basePoint );

	bb.lx() *= g_unitScale;
	bb.ly() *= g_unitScale;
	bb.lz() *= g_unitScale;
	bb.ux() *= g_unitScale;
	bb.uy() *= g_unitScale;
	bb.uz() *= g_unitScale;
}
//-----------------------------------------------------------------------------
float AABB_Approximator::pixArea( const pcloud::Node *v, const pt::ViewParams &vs )
{
	BoundingBoxD bb;
	getVoxelBoundingBox( v, bb );
	
	vector4d lx_ly_lz(bb.lx(), bb.ly(), bb.lz(), 1.0);
	vector4d lx_uy_lz(bb.lx(), bb.uy(), bb.lz(), 1.0);
	vector4d lx_uy_uz(bb.lx(), bb.uy(), bb.uz(), 1.0);
	vector4d ux_uy_uz(bb.ux(), bb.uy(), bb.uz(), 1.0);

	vector4d ux_uy_lz(bb.ux(), bb.uy(), bb.lz(), 1.0);
	vector4d ux_ly_uz(bb.ux(), bb.ly(), bb.uz(), 1.0);
	vector4d ux_ly_lz(bb.ux(), bb.ly(), bb.lz(), 1.0);
	vector4d lx_ly_uz(bb.lx(), bb.ly(), bb.uz(), 1.0);

	vector3 d = bb.diagonal();

	float area_bias = 1.0f;

	/* thin box check and boost */ 
	for (int i=0; i<3; i++)
	{
		if (d[i] < 0.1 * d[(i+1)%3] && d[i] < 0.1 * d[(i+2)%3])
		{
			area_bias = 2.0f;
			break;
		}
	}

	vector4d _lx_ly_lz, _lx_uy_lz, _lx_uy_uz, _ux_uy_uz, _ux_uy_lz, _ux_ly_uz, _ux_ly_lz, _lx_ly_uz;

	vs.project4v(lx_ly_lz, _lx_ly_lz);
	vs.project4v(lx_uy_lz, _lx_uy_lz);
	vs.project4v(lx_uy_uz, _lx_uy_uz);
	vs.project4v(ux_uy_uz, _ux_uy_uz);
	vs.project4v(ux_uy_lz, _ux_uy_lz);
	vs.project4v(ux_ly_uz, _ux_ly_uz);
	vs.project4v(ux_ly_lz, _ux_ly_lz);
	vs.project4v(lx_ly_uz, _lx_ly_uz);

	float area =	quadArea2(_lx_ly_lz, _ux_ly_lz, _ux_uy_lz, _lx_uy_lz) +
					quadArea2(_lx_uy_lz, _ux_uy_lz, _ux_uy_uz, _lx_uy_uz) +
					quadArea2(_ux_ly_lz, _ux_ly_uz, _ux_uy_uz, _ux_uy_lz) +
					quadArea2(_ux_ly_uz, _lx_ly_uz, _lx_uy_uz, _ux_uy_uz) +
					quadArea2(_lx_ly_uz, _lx_ly_lz, _lx_uy_lz, _lx_uy_uz) +
					quadArea2(_lx_ly_lz, _lx_ly_uz, _ux_ly_uz, _ux_ly_lz);
	area *= 0.5f;

	return area * area_bias;
}
//-----------------------------------------------------------------------------
float AABB_Approximator::planeArea( const pcloud::Node *v, const pt::ViewParams &vs )
{
	return 0;
}
//-----------------------------------------------------------------------------
// approximate scan line area comp based on point samples
//-----------------------------------------------------------------------------
#define SL_PNT_SAMPLE_SIZE	500

struct ProjectPoints
{
	ProjectPoints( const pt::ViewParams &view ) : vs(view)
	{
	};

	void point(const pt::vector3d &v, int i, ubyte &f)
	{
		vs.project3v( v, projPnts[i] );

		if (!i)
		{
			projBox.set( projPnts[i].x, projPnts[i].y, projPnts[i].x, projPnts[i].y );
		}
		else
		{
			projBox += vec2<double>( projPnts[i].x, projPnts[i].y );
		}
	}
	vector3d		projPnts[ SL_PNT_SAMPLE_SIZE ];
	const			pt::ViewParams &vs;
	pt::Rectd		projBox;
};


struct ProjectPointsIntoGrid
{
	ProjectPointsIntoGrid( const pt::ViewParams &view ) : vs(view)
	{
	};

	void point(const pt::vector3d &v, int i, ubyte &f)
	{
		vs.project3v( v, projPnts[i] );

		int x = static_cast<int>(projPnts[i].x / 8);
		int y = static_cast<int>(projPnts[i].y / 8);

		grid.insert( x * 1000 + y );
	}
	int computeApproxArea(void)
	{
		return static_cast<int>(grid.size() * 64);
	} 
	vector3d		projPnts[ SL_PNT_SAMPLE_SIZE ];
	const			pt::ViewParams &vs;
	std::set<uint>  grid;
};

#define SCAN_ROW_HEIGHT 16

struct ScanPoints
{
	ScanPoints( const pt::ViewParams &view ) : vs(view), maxRows(0)
	{
		memset(validRow, 0, sizeof(validRow));
	};

	void point(const pt::vector3d &v, int index, ubyte &f)
	{
		double	pp_d[4];
		int		pp[2];
		vs.project3v( v, pp_d );
		pp[0] = (int)pp_d[0];
		pp[1] = (int)pp_d[1];

		int row = pp[1] / SCAN_ROW_HEIGHT + 100;
		if (row > 499) return;
		if (row < 0) return;

	//	if (pp[0] >= 0 && pp[0] <= vs.viewport[2] && pp[1] >=0 && pp[1] <= vs.viewport[3])
		{	
			if (row > maxRows) maxRows = row;

			Recti &sr = scanRows[row];

			if (!validRow[row])
			{
				validRow[row] = true;
				sr.set(pp[0], pp[1], pp[0], pp[1]);
			}
			else
			{
				sr.expand( vec2<int>(pp[0], pp[1]) );
			}
		}
	}
	float	computeApproxArea()
	{
		float area = 0;

		for (int i=0; i<maxRows; i++)
		{
			if (validRow[i])
			{
				float a = static_cast<float>(scanRows[i].dx() * SCAN_ROW_HEIGHT);
				area += a;// < 256 ? 256 : a;
			}
		}
		return area;
	}
	const			pt::ViewParams &vs;
	
	pt::Recti		scanRows[500];
	bool			validRow[500];
	int				maxRows;
	
};
//-----------------------------------------------------------------------------
float Scanline_Approximator::pixArea( const pcloud::Node *v, const pt::ViewParams &vs )
{
	if (!v->flag(pcloud::Visible)) return 0;

	//int numPntSample = v->lodPointCount() < SL_PNT_SAMPLE_SIZE ? v->lodPointCount() : SL_PNT_SAMPLE_SIZE;

	ScanPoints sp(vs);

	//const_cast<pcloud::Voxel*>(v)->
	//	iterateTransformedPointsRange(sp, pt::ProjectSpace, 0, 0, numPntSample );
	
	return sp.computeApproxArea();
}
//-----------------------------------------------------------------------------
float Scanline_Approximator::planeArea( const pcloud::Node *v, const pt::ViewParams &vs )
{
	return 0;
}
//-----------------------------------------------------------------------------
float ConvexHull_Approximator::pixArea( const pcloud::Node *v, const pt::ViewParams &vs )
{
	return 0;
}
//-----------------------------------------------------------------------------
float ConvexHull_Approximator::planeArea( const pcloud::Node *v, const pt::ViewParams &vs )
{
	return 0;
}
