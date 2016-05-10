#include "PointoolsVortexAPIInternal.h"

#include <ptcloud2/Voxel.h>

#include <ptengine/VisibilityEngine.h>
#include <ptengine/pointsPager.h>
#include <ptengine/PointsScene.h>
#include <ptengine/PointsFilter.h>
#include <ptengine/engine.h>
#include <ptengine/clipManager.h>
#include <pt/timer.h>

#include <ptgl/glFrustum.h>
#include <pt/debugassert.h>

#include <pt/project.h>
#include <pt/trace.h>

#include <ptengine/projAreaApproximation.h>
#include <diagnostics/diagnostics.h>

using namespace pointsengine;
using namespace pt;

static vector3d basePoint;
static vector4d basePoint4;

//#define _VERBOSE				1
#define LOD_MIN_PTS				32
#define LOD_MIN					0.001
#define ALWAYS_FLUSH_LESS_THAN	32
#define LOD_MIN_VALUE			0.004

extern double g_unitScale;

inline static float quadArea2(const vector4d &a, const vector4d &b, const vector4d &c, const vector4d &d)
{
	/*two triangles - abc + acd*/ 
	double area = fabs(((b.x - a.x)*(c.y-a.y)) - ((c.x - a.x) * (b.y - a.y)));
	area += fabs(((c.x - a.x)*(d.y-a.y)) - ((d.x - a.x) * (c.y - a.y)));
	area *= 0.5;
	return static_cast<float>(area);
}
VisibilityEngine::VisibilityEngine()
{
	m_iteration = 0;
	m_thread = 0;
	m_run = 0;
	m_working = 0;
	m_waiting = 0;
	m_thread = 0;
	m_optimizerStrength = 0.5f;
	m_bias = BiasScreen;
	m_biasPnt.zero();
	m_pointsBudget = -1;
}
VisibilityEngine::~VisibilityEngine()
{
	VisibilityEngine::stop();

	//boost::thread *thread = reinterpret_cast<boost::thread*>(m_thread);
	//delete thread;
}
int VisibilityEngine::getIteration() const { return m_iteration; }
//--------------------------------------------------------------------------------
// Thread Control
//--------------------------------------------------------------------------------
void VisibilityEngine::stop()
{
	m_run = 0;
}
bool VisibilityEngine::pause()
{
	m_paused = 1;
	while (m_working);
	return true;
}
bool VisibilityEngine::unpause()
{
	m_paused = 0; 
	return true;
}
//--------------------------------------------------------------------------------
// Initialization and startup
//--------------------------------------------------------------------------------
bool VisibilityEngine::initialize()
{
	PTTRACE("VisibilityEngine::initialize"); 

	if (m_thread) return false;

	m_up = 0;
	m_iteration = 0;


	return true;
}

//---------------------------------------------------------
// VisibilityEngine | setViewParameters
//---------------------------------------------------------
void VisibilityEngine::setViewParameters( const pt::ViewParams &viewParams )
{
	m_view = viewParams;
	m_fr.buildFrustum( m_view.proj_matrix, m_view.eye_matrix );
	m_up = 1;
}
//---------------------------------------------------------
// VisibilityEngine | setFixedVisibility
//---------------------------------------------------------
void VisibilityEngine::setFixedVisibility(float vis)
{
	basePoint = vector3d(Project3D::project().registration().matrix()(3,0), 
		Project3D::project().registration().matrix()(3,1), 
		Project3D::project().registration().matrix()(3,2));

	pause();
	
    std::unique_lock<std::mutex> lock(m_mutex, std::try_to_lock);
	if (!lock.owns_lock()) return;

	/*do depth sort here in order to minimize cpu useage*/ 
	static int liststate = -1;
	PointsScene::UseDepthSortedVoxels voxelslock(m_voxlist, liststate);

	if (!m_voxlist.size()) return;

	PointsScene::VoxIterator i = m_voxlist.begin();
	PointsScene::VoxIterator e = m_voxlist.end();
	pcloud::Voxel *vox;
	
	bool filter = false;/*thePointsFilter().isActive();*/ 

	pt::Bounds<1, float> bounds;
	bounds.makeEmpty();

	int count = 0;
	while (i != e)
	{
		vox = (*i);
		++i;		
		{
			const pcloud::PointCloud* pc = vox->pointCloud();
			const pt::DisplayInfo &di = pc->displayInfo();

			BoundingBoxD bb = vox->extents();
			bb.translateBy(-basePoint);

			{
				vox->flag(pcloud::WholeClipped, false);

				if (!m_fr.inFrustum(&bb))
				{
					vox->flag(pcloud::Visible, false);
					vox->setRequestLOD(0);
				}
				else 
				{
					vox->flag(pcloud::Visible, true);
					vox->setRequestLOD(vis);
				}
			}
		}
	}
}

//---------------------------------------------------------
// FrustumCheck
//---------------------------------------------------------
struct FrustumCheck : public PointsVisitor
{
	FrustumCheck( ptgl::Frustum &fr ) : frustum(fr) {}

	bool scene(pcloud::Scene *sc)
	{
		bool vis = sc->displayInfo().visible();
		if (!vis || !sc->loaded())
		{
			for (int i=0; i<sc->numObjects(); i++)
				const_cast<pcloud::Node*>(sc->cloud(i)->root())->flag(pcloud::Visible, false, true);
			return false;
		}
		return true;
	}
	bool cloud(pcloud::PointCloud *cloud)
	{
		pcloud::Node * node = const_cast<pcloud::Node*>(cloud->root());
		bool vis = cloud->displayInfo().visible();
		if (!vis)
		{
			node->flag(pcloud::Visible, false, true);
			return false;
		}
		return true;
	}
	bool visitNode(const pcloud::Node *n)
	{
		pcloud::Node * node = const_cast<pcloud::Node*>(n);
		BoundingBoxD bb = node->extents();
		bb.translateBy(-basePoint);

		bb.lx() *= g_unitScale;
		bb.ly() *= g_unitScale;
		bb.lz() *= g_unitScale;
		bb.ux() *= g_unitScale;
		bb.uy() *= g_unitScale;
		bb.uz() *= g_unitScale;

		if (!frustum.inFrustum(&bb))
		{
			node->flag(pcloud::Visible, false, true);
			return false;
		}

		ptgl::Frustum::FrustumIntersect res = frustum.containedInFrustum(&bb);
		
		if (res == ptgl::Frustum::FRUSTUM_FULL) /*FRUSTUM_CULLED here is unreliable */ 
		{
			node->flag(pcloud::Visible, true, true);
			return false;
		}
		node->flag(pcloud::Visible, true, false);
		return true;
	}
	ptgl::Frustum	&frustum;
};
//---------------------------------------------------------
// ClipCheck
//---------------------------------------------------------
/*struct ClipBoxCheck : public PointsVisitor
{
	ClipBoxCheck( const mmatrix4d &box ) : boxMatrix(box), boxMatrixi(box)
	{
		//check units
		//boxMatrixi.invert();
	}

	// 1 = contained
	// 2= intersects
	// 3= outside

	// keep things simple, keep it all Axis Aligned
	int testBox(const pcloud::Node *n)
	{
		static BoundingBox box(1.0f, 0, 1.0f, 0, 1.0f, 0);
		BoundingBox clip;

		vector3d pnt;
		BoundingBoxD node = n->extents();
		node.translateBy(-basePoint);	

		for (unsigned i=0; i<8; i++)
		{
			node.getExtrema(i, pnt);
			boxMatrix.vec3_multiply_mat4(pnt);	
			
			clip.expandBy(pnt);
		}
		
		if (box.contains( &clip )) return 1; // inside
		if (box.intersects( &clip ) || clip.contains(&box)) return 2; // intersects
		return 0;
	}
	bool visitNode(const pcloud::Node* node)
	{
		pcloud::Node* n = const_cast<pcloud::Node*>(node);
		
		int res = testBox( n );
		if (res == 0)	// outside
		{
			n->flag(pcloud::PartClipped, false, false);
			n->flag(pcloud::WholeClipped, true, false);
			return true;
		}
		else if (res == 1) // inside
		{
			n->flag(pcloud::PartClipped, false, true);
			n->flag(pcloud::WholeClipped, false, true);
			return false;
		}
		else	//intersection
		{
			n->flag(pcloud::PartClipped, true, true);
			n->flag(pcloud::WholeClipped, false, true);
			return true;
		}
	}
	mmatrix4d		boxMatrix;
	mmatrix4d		boxMatrixi;
};*/
//---------------------------------------------------------
struct ClippingCheck : public PointsVisitor
{
	ClippingCheck()
	{		
	}

	bool visitNode(const pcloud::Node* node)
	{
		if (!currentCloud) return true;

		pcloud::Node* n = const_cast<pcloud::Node*>(node);

		ClipResult res = ClipManager::instance().clipNode( *currentCloud, n );
		if (res == OUTSIDE)
		{
			n->flag(pcloud::PartClipped, false, false);
			n->flag(pcloud::WholeClipped, true, false);
			return true;
		}
		else if (res == INSIDE) 
		{
			n->flag(pcloud::PartClipped, false, true);
			n->flag(pcloud::WholeClipped, false, true);
			return false;
		}
		else if (res == INTERSECTS)
		{
			n->flag(pcloud::PartClipped, true, true);
			n->flag(pcloud::WholeClipped, false, true);			
		}
		
		return true;
	}	
};
//---------------------------------------------------------
struct ClippingOff : public PointsVisitor
{
	bool visitNode(const pcloud::Node* n)
	{
		pcloud::Node * node = const_cast<pcloud::Node*>(n);

		node->flag(pcloud::WholeClipped, false, true);
		return false;
	}
};
/*
void		VisibilityEngine::setClipBox( const mmatrix4d &orientedBoxMat )
{		
	extern double g_unitScale;
	vector3d x,y,z,o,s;

	//extract clip planes from matrix and set up
	x.set( orientedBoxMat(0,0), orientedBoxMat(1,0), orientedBoxMat(2,0) );
	y.set( orientedBoxMat(0,1), orientedBoxMat(1,1), orientedBoxMat(2,1) );
	z.set( orientedBoxMat(0,2), orientedBoxMat(1,2), orientedBoxMat(2,2) );
	o.set( orientedBoxMat(3,0), orientedBoxMat(3,1), orientedBoxMat(3,2));  
		
	x /= g_unitScale;
	y /= g_unitScale;
	z /= g_unitScale;
	o /= g_unitScale;

	s.set( x.length(), y.length(), z.length() );

	x.normalize();
	y.normalize();
	z.normalize();

	// set matrix up correctly for shader and cliptest
	double s4[] = { s.x, s.y, s.z, 1};
	double t4[] = { o.x, o.y, o.z, 0};

	mmatrix4d mat = mmatrix4d::identity();
	
	mmatrix4d scale = mmatrix4d::scale(s4);
	mmatrix4d frame= mmatrix4d::identity();
	mmatrix4d translation = mmatrix4d::translation(t4);

	frame(0,0) = x.x;
	frame(1,0) = x.y;
	frame(2,0) = x.z;

	frame(0,1) = y.x;
	frame(1,1) = y.y;
	frame(2,1) = y.z;

	frame(0,2) = z.x;
	frame(1,2) = z.y;
	frame(2,2) = z.z;

	frame.invert();	//why?? don't know but it works

	mat >>= scale;
	mat >>= frame;
	mat >>= translation;
	
	// now this is a matrix that goes from project space to clip (0 to 1) space
	// everything outside the 0-1 interval should be clipped, easy to do in a shader
	mat.invert();

	m_clipBoxMatrix = mat;
}
const		mmatrix4d &VisibilityEngine::clipBox() const
{
	return m_clipBoxMatrix;
}
*/
//-----------------------------------------------------------------------------
// computeCurrentPntsNotLoaded
// faster than full compute because no set and not mutex lock
//-----------------------------------------------------------------------------
void	VisibilityEngine::computeCurrentPntsShortfall( const pcloud::Scene* inScene, LoadedShortfallMap &result  )
{
	if (!inScene) result.clear();
	else
	{
		LoadedShortfallMap::iterator f = result.find( inScene );
		if (f!= result.end()) f->second = 0;
	}

	/*do depth sort here in order to minimize cpu useage*/ 
	static int listState = -1;
	PointsScene::UseSceneVoxels voxelslock(m_voxlist, listState);

	if (!m_voxlist.size()) return;

	PointsScene::VoxIterator i = m_voxlist.begin();
	PointsScene::VoxIterator e = m_voxlist.end();
	pcloud::Voxel *vox;

	int64_t totalOutstandingRequests = 0;

	while (i != e)
	{
		float request = 0;
		vox = (*i);
		++i;

		if (	inScene && vox->pointCloud()->scene() != inScene 
			||	vox->flag( pcloud::OutOfCore )) 
		{
			continue;
		}
		else
		{
			pcloud::Voxel::LOD lod = vox->getCurrentLOD();
			pcloud::Voxel::LOD req = vox->getRequestLOD();

			int shortfall = 
                static_cast<int>(max(0, vox->getNumPointsAtLOD(req) - vox->getNumPointsAtLOD(lod)));
			
			if (shortfall>0)
			{
				const pcloud::Scene* scene = vox->pointCloud()->scene();

				LoadedShortfallMap::iterator sf = result.find( scene );
				if (sf == result.end())		result.insert( LoadedShortfallMap::value_type(scene, shortfall) );
				else						sf->second += shortfall;
			}
		}
	}
}

//-----------------------------------------------------------------------------
// VisibilityEngine | _computeVoxelPriority
//-----------------------------------------------------------------------------
float	VisibilityEngine::computeVoxelPriority( const pt::ViewParams &view, VisibilityBias bias, const vector3 &biasPt, 
											   pcloud::Node * n, const BoundingBoxD &extents )
{	
	vector4d mid(extents.center());
	vector4d midp;
	float p=0, p1=0;
	bool visible = n->flag( pcloud::Visible );
	
	if (!view.project4v(mid, midp))
		p = 0.001;
	else
	{
		switch (bias)
		{
		case BiasScreen:
			
			/* priority metric */ 
			if (view.viewport[2] && view.viewport[3])
			{
                p1 = static_cast<float>(midp.z);
				p1 *= p1 * p1;
				p1 = 1.0f - p1;
				p = p1 * 10.0f;

				midp.x /= view.viewport[2];
				midp.y /= view.viewport[3];

				/* check following line for correctness */ 
				p += static_cast<float>(0.2f * (2.0f - (2*fabs(midp.x - 0.5f) + 2*fabs(midp.y - 0.5f))));
				p /= 3.0f;

				//p -= vox->getCurrentLOD() * 0.1f;		
				if (!visible) p *= 0.2;	// out frustum, reduce lod
			}
			break;				
			
		case BiasNear:
			p = static_cast<float>(1 - midp.z);
			if (p< 0) p = 0;
			if (!visible) p *= 0.1;
			break;

		case BiasFar:
			p = static_cast<float>(midp.z);
			if (p<0) p = 0;
			if (!visible) p *= 0.1;
			break;

		case BiasPoint:
			{
			double d = biasPt.dist2( vector3(static_cast<float>(mid.x), static_cast<float>(mid.y), static_cast<float>(mid.z)) );
			if (d > 0) p = static_cast<float>(1.0 / d);
			if (!visible) p *= 0.75;
			}
			break;
		}
	}
	return p;
}
//---------------------------------------------------------
// FillOcclusionFrame
//---------------------------------------------------------
struct FillOcclusionFrame : public PointsVisitor
{
	FillOcclusionFrame( const pt::ViewParams &_vp, VisibilityEngine::OcclusionFrame &occ )
		: view(_vp), occFrame(occ)
	{
		occFrame.setSize( view.viewport[ViewParams::Width], view.viewport[ ViewParams::Height ] );
	}

	bool scene(pcloud::Scene *sc)
	{
		bool vis = sc->displayInfo().visible();
		if (!vis || !sc->loaded())
			return false;
		return true;
	}
	bool cloud(pcloud::PointCloud *cloud)
	{
		pcloud::Node * node = const_cast<pcloud::Node*>(cloud->root());
		bool vis = cloud->displayInfo().visible();

		if (!vis || !node->flag(pcloud::Visible)) 
			return false;

		return true;
	}

	bool visitNode(const pcloud::Node *n)
	{
		if (!n->flag(pcloud::Visible)
			|| n->flag(pcloud::WholeClipped)
			|| n->flag(pcloud::WholeHidden))
			return false;

		if (!n->isLeaf()) 
			return true;

		pcloud::Node * node = const_cast<pcloud::Node*>(n);
		pcloud::Voxel *voxel = static_cast<pcloud::Voxel*>(node);

        int numPts = static_cast<int>(voxel->getRequestLOD() * voxel->fullPointCount() * occFrame.factor() * 0.1);
		if (numPts < 50) numPts = 50;

		vid = voxel->indexInCloud();

		voxel->
			iterateTransformedPointsRange(*this, pt::ProjectSpace, 0, 0, numPts );

		return true;
	}
	
	void point(const pt::vector3d &v, int index, ubyte &f)
	{
		pt::vector4d v4(v.x,v.y,v.z,1.0);
		pt::vector4d w;
		if (view.project4v(v4, w))
		{
            occFrame.insert(static_cast<int>(w.x), static_cast<int>(w.y), static_cast<float>(w.z), vid);
		}
	}
	int									vid;
	VisibilityEngine::OcclusionFrame	&occFrame;
	const pt::ViewParams				&view;
};

//---------------------------------------------------------
// FillOcclusionFrame
//---------------------------------------------------------
struct CullOccluded : public PointsVisitor
{
	CullOccluded( const pt::ViewParams &_vp, VisibilityEngine::OcclusionFrame &occ )
		: view(_vp), occFrame(occ)
	{
	}

	bool scene(pcloud::Scene *sc)
	{
		bool vis = sc->displayInfo().visible();
		if (!vis || !sc->loaded())
			return false;
		return true;
	}
	bool cloud(pcloud::PointCloud *cloud)
	{
		pcloud::Node * node = const_cast<pcloud::Node*>(cloud->root());
		bool vis = cloud->displayInfo().visible();

		if (!vis || !node->flag(pcloud::Visible)) 
			return false;

		return true;
	}

	bool visitNode(const pcloud::Node *n)
	{
		if (!n->flag(pcloud::Visible)
			|| n->flag(pcloud::WholeClipped)
			|| n->flag(pcloud::WholeHidden))
			return false;

		if (!n->isLeaf()) 
			return true;

		pcloud::Node * node = const_cast<pcloud::Node*>(n);
		pcloud::Voxel *voxel = static_cast<pcloud::Voxel*>(node);

		float reqLodPts = voxel->getRequestLOD() * voxel->fullPointCount();

		if (!reqLodPts) return true;

		int numPts = static_cast<int>(reqLodPts * occFrame.factor() * occFrame.factor());
		if (numPts < 50) numPts = 50;
			
		vid = voxel->indexInCloud();
		pointsOccluded = 0;

		voxel->
			iterateTransformedPointsRange(*this, pt::ProjectSpace, 0, 0, numPts );

		if (pointsOccluded > 1)
		{
			float occlusionFactor = 1.0f - (float)pointsOccluded / numPts ;
		
			//if (occlusionFactor < 0.25f)
			{
                std::lock_guard<std::mutex> vlock( voxel->mutex() );
				float lod = voxel->getRequestLOD() * occlusionFactor;
				if (lod < 0) lod = 0;
				voxel->setRequestLOD( lod );
			}
		}
		return true;
	}
	
	void point(const pt::vector3d &v, int index, ubyte &f)
	{
		int						occ_vid;

		pt::vector4d v4(v.x,v.y,v.z,1.0);
		pt::vector4d w;
		if (view.project4v(v4, w))
		{
			if (occFrame.isOccluded(static_cast<int>(w.x), static_cast<int>(w.y), static_cast<float>(w.z), occ_vid))
			{
				if (occ_vid != vid)
					++pointsOccluded;
			}		
		}
	}
	int									pointsOccluded;
	int									vid;
	VisibilityEngine::OcclusionFrame	&occFrame;
	const pt::ViewParams				&view;
};
struct UpdateStats : public PointsVisitor
{
	UpdateStats(ptdg::VisibilityData1 &stats_) : stats(stats_) 
	{
		stats.m_averageLOD = 0;
		stats.m_numFullPtsInView = 0;
		stats.m_numLODPtsInView1 = 0;
		stats.m_maxLOD = 0;
		stats.m_minLOD = 2.0;
	}
	bool scene(pcloud::Scene *sc)
	{
		bool vis = sc->displayInfo().visible();
		if (!vis || !sc->loaded())
		{
			return false;
		}
		return true;
	}
	bool cloud(pcloud::PointCloud *cloud)
	{
		pcloud::Node * node = const_cast<pcloud::Node*>(cloud->root());
		bool vis = cloud->displayInfo().visible();
		if (!vis)
		{
			return false;
		}
		return true;
	}
	
	bool visitNode(const pcloud::Node *n)
	{
		if (!n->flag( pcloud::Visible ))
			return false;

		if (n->isLeaf())
		{
			const pcloud::Voxel *v = static_cast<const pcloud::Voxel *>(n);
			
			float rlod = v->getRequestLOD();

			stats.m_averageLOD += rlod;
			stats.m_numFullPtsInView += v->fullPointCount();
            stats.m_numLODPtsInView1 += static_cast<int64_t>(v->fullPointCount() * rlod);

			if (rlod > stats.m_maxLOD) stats.m_maxLOD = rlod;
			if (rlod < stats.m_minLOD) stats.m_minLOD = rlod;
		}
		return true;
	}				
	
	ptdg::VisibilityData1	&stats;
};

//---------------------------------------------------------
// Visibility Budget Adjust
//---------------------------------------------------------
struct BudgetAdjustment : public PointsVisitor
{
	BudgetAdjustment( float currentCount, float targetCount )
		: current( currentCount ), target( targetCount ) 
	{	
	}

	bool scene(pcloud::Scene *sc)
	{
		bool vis = sc->displayInfo().visible();
		if (!vis || !sc->loaded())
		{
			return false;
		}
		return true;
	}
	bool cloud(pcloud::PointCloud *cloud)
	{
		pcloud::Node * node = const_cast<pcloud::Node*>(cloud->root());
		bool vis = cloud->displayInfo().visible();
		if (!vis)
		{
			return false;
		}
		return true;
	}
	
	bool visitNode(const pcloud::Node *n)
	{
		if (!n->flag( pcloud::Visible ))
			return false;

		if (n->isLeaf())
		{
			pcloud::Voxel *v = static_cast<pcloud::Voxel*>(
				const_cast<pcloud::Node*>(n));
			
			float rlod = v->getRequestLOD();

			float maxFactor = target / current;
			if (maxFactor > 1.5f)
				maxFactor = 1.5f;

			rlod *= maxFactor;

            std::lock_guard<std::mutex> vlock( v->mutex() );

			v->setRequestLOD( rlod );
		}
		return true;
	}				
	
	float current;
	float target;
};

//---------------------------------------------------------
// Visibility Budget Adjust
//---------------------------------------------------------
struct ComputeShortfall : public PointsVisitor
{
	ComputeShortfall( VisibilityEngine::LoadedShortfallMap &shortfall )
		: shortfallMap( shortfall ) 
	{	
	}
	bool scene(pcloud::Scene *sc)
	{
		bool vis = sc->displayInfo().visible();
		if (!vis || !sc->loaded())
		{
			return false;
		}
		return true;
	}
	bool cloud(pcloud::PointCloud *cloud)
	{
		pcloud::Node * node = const_cast<pcloud::Node*>(cloud->root());
		bool vis = cloud->displayInfo().visible();
		if (!vis)
		{
			return false;
		}
		return true;
	}
	
	bool visitNode(const pcloud::Node *n)
	{
		if (!n->flag( pcloud::Visible ))
			return false;

		if (n->isLeaf())
		{
			const pcloud::Voxel *vox = static_cast<const pcloud::Voxel*>(n);
			
            int shortfall = static_cast<int>((vox->getRequestLOD() * vox->fullPointCount()) - vox->lodPointCount());
			
			if (shortfall > LOD_MIN_VALUE *  vox->fullPointCount() && !vox->flag( pcloud::OutOfCore ) )
			{
				const pcloud::Scene* scene = vox->pointCloud()->scene();

				VisibilityEngine::LoadedShortfallMap::iterator sf = shortfallMap.find( scene );
				if (sf == shortfallMap.end())
					shortfallMap.insert( VisibilityEngine::LoadedShortfallMap::value_type(scene, shortfall) );
				else
				{
					sf->second += shortfall;
				}
			}			
		}
		return true;					
	}
	VisibilityEngine::LoadedShortfallMap	&shortfallMap;
};
//---------------------------------------------------------
// VisibilityCompute
//---------------------------------------------------------
struct VisibilityCompute : public PointsVisitor
{
	VisibilityCompute( const pt::ViewParams &_vp, const VisibilityEngine::VisibilityBias &_bias,
		const pt::vector3 &_biasPnt)
		: vp(_vp), bias(_bias), biasPnt(_biasPnt)
	{
		basePoint = vector3d(Project3D::project().registration().matrix()(3,0), 
			Project3D::project().registration().matrix()(3,1), 
			Project3D::project().registration().matrix()(3,2));

		basePoint4 = vector4d(Project3D::project().registration().matrix()(3,0), 
			Project3D::project().registration().matrix()(3,1), 
			Project3D::project().registration().matrix()(3,2), 0);

		memset(&stats, 0, sizeof(stats));
		stats.m_minLOD = 1.0f;
		stats.m_numLODPtsInView1 =0;

		priorityRange.makeEmpty();
	}

	bool scene(pcloud::Scene *sc)
	{
		bool vis = sc->displayInfo().visible();
		if (!vis || !sc->loaded())
		{
			for (int i=0; i<sc->numObjects(); i++)
				const_cast<pcloud::Node*>(sc->cloud(i)->root())->flag(pcloud::Visible, false, true);
			return false;
		}
		return true;
	}
	bool cloud(pcloud::PointCloud *cloud)
	{
		pcloud::Node * node = const_cast<pcloud::Node*>(cloud->root());
		bool vis = cloud->displayInfo().visible();
		if (!vis)
		{
			node->flag(pcloud::Visible, false, true);
			return true;
		}
		return true;
	}
	int64_t getFullPointCount( const pcloud::Node *n, int64_t &count )
	{	
		if (n->isLeaf()) 
			count += static_cast<const pcloud::Voxel*>(n)->fullPointCount();
		else
		{
			for (int i=0; i<8; i++) 
				if (n->child(i)) 
					getFullPointCount(n->child(i), count);
		}
		return count;
	}
	bool visitNode(const pcloud::Node *n)
	{
		if (n->depth() < 3 && !n->isLeaf()) 
			return true;

		pcloud::Node * node = const_cast<pcloud::Node*>(n);
		BoundingBoxD bb = node->extents();
		bb.translateBy(-basePoint);

		bb.lx() *= g_unitScale;
		bb.ly() *= g_unitScale;
		bb.lz() *= g_unitScale;
		bb.ux() *= g_unitScale;
		bb.uy() *= g_unitScale;
		bb.uz() *= g_unitScale;

		int64_t fullPointCount = 0;

		getFullPointCount(n, fullPointCount);

		if ( n->flag(pcloud::Visible) &&
			!( n->flag(pcloud::WholeClipped) || n->flag(pcloud::WholeHidden) )) 
		{				
			++stats.m_numVoxelsInView;

			/* set full visibility to small number of points to ensure inclusion */ 
			//if (n->fullPointCount() < ALWAYS_FLUSH_LESS_THAN) vox->setRequestLOD(1.0f);
			//else
			{
				BoundingBoxD bb(n->extents());
				bb.translateBy(-basePoint);

				bool visible = n->flag( pcloud::Visible );
				
				vector4d mid(bb.center());
				vector4d midp;

				float p = VisibilityEngine::computeVoxelPriority( vp, bias, biasPnt, node, bb );					
				priorityRange.expand(&p);	// expand the range of priorities

				float projArea=0;
				float altLOD = 0;	// for diagnostics
				float bbLOD = 0;	// for diagnostics

				float computedLOD = VisibilityEngine::computeVoxelLOD( vp, node, basePoint, projArea, altLOD );
				bbLOD = computedLOD;

				if (!visible) 
				{
					if (computedLOD < 0.4f) computedLOD *= 0.01f;
					else computedLOD = 0.05f;
				}
				
				if (projArea < 1000 || n->isLeaf())
				{
					setLOD(node, computedLOD, p );
				}
				else
				{
					return true;	// continue depth iteration
				}
								
				if (visible) 
				{
                    stats.m_numLODPtsInView1 += static_cast<int64_t>(computedLOD * fullPointCount);
					stats.m_totalPxArea += projArea;
				}
			}
		}
		else
		{				
			// iterate through voxels and set zero priority
			setZeroLOD(node);
		}	
		return false;
	}
	void setZeroLOD( pcloud::Node *n )
	{
		setLOD(n,0,0);
	}
	void setLOD( pcloud::Node *n, float lod, float priority )
	{
		if (!n) return;

		if (n->isLeaf())
		{
			pcloud::Voxel *v = static_cast<pcloud::Voxel*>(n);
            std::lock_guard<std::mutex> vlock(v->mutex());
			v->priority(priority);
			v->setRequestLOD(lod);
		}
		else
		{
			for (int i=0; i<8; i++)
			{
				setLOD(n->child(i), lod, priority);
			}
		}		
	}
	ptdg::VisibilityData1	stats;

	vector3d				basePoint;
	vector4d				basePoint4; 
	vector3					biasPnt;

	const pt::ViewParams	&vp;
	const VisibilityEngine::VisibilityBias &bias;

	pt::Bounds<1, float>				priorityRange;
};
//-----------------------------------------------------------------------------
// VisibilityEngine | _computeVisibility
//-----------------------------------------------------------------------------
void VisibilityEngine::computeVisibility()
{
	PTTRACE("VisibilityEngine::_computeVisibility"); 

	ptdg::VisibilityData1 stats;
	memset(&stats, 0, sizeof(stats));
	stats.m_minLOD = 1.0f;

	m_loadShortfall.clear();
	m_iteration++;
	m_up = 0;

	basePoint = vector3d(Project3D::project().registration().matrix()(3,0), 
		Project3D::project().registration().matrix()(3,1), 
		Project3D::project().registration().matrix()(3,2));

	basePoint4 = vector4d(Project3D::project().registration().matrix()(3,0), 
		Project3D::project().registration().matrix()(3,1), 
		Project3D::project().registration().matrix()(3,2), 0);
	
    pt::Timer::TimeMs t0 = pt::Timer::tick();

	/* frustum visibility */ 
	FrustumCheck frustumcheck(m_fr);
	thePointsScene().visitPointClouds( &frustumcheck );

	/* clip box visibility */ 
	if (pointsengine::ClipManager::instance().clippingEnabled())
	{			
		ClippingCheck clipCheck;
		thePointsScene().visitPointClouds( &clipCheck );
	}
	else
	{
		ClippingOff	clipOff;
		thePointsScene().visitPointClouds( &clipOff );
	}
		
	// compute general visibilities
	VisibilityCompute visibility(m_view, m_bias, m_biasPnt);
	thePointsScene().visitPointClouds( &visibility );

	// adjust for budget cap
	if (m_pointsBudget != -1 && visibility.stats.m_numLODPtsInView1 > m_pointsBudget )
	{
        BudgetAdjustment budget(static_cast<float>(visibility.stats.m_numLODPtsInView1), static_cast<float>(m_pointsBudget));
		thePointsScene().visitPointClouds( &budget );
	}

	// compute the shortfall in LOD vs Requested LOD
	ComputeShortfall shortfall( m_loadShortfall );
	thePointsScene().visitPointClouds( &shortfall );

	//// fill the occlusion buffer 
	//FillOcclusionFrame		occFill( m_view, m_occFrame );
	//thePointsScene().visitPointClouds( &occFill );
	//	
	//// adjust for occlusion 
	//CullOccluded		occCull( m_view, m_occFrame );
	//thePointsScene().visitPointClouds( &occCull );
		
	UpdateStats			updateStats( visibility.stats );
	thePointsScene().visitPointClouds( &updateStats );
	
	if (visibility.stats.m_numVoxelsInView)
	{
		ptdg::Time::stamp( visibility.stats );
		visibility.stats.m_averageLOD /= visibility.stats.m_numVoxelsInView;
		visibility.stats.m_avPxArea /= visibility.stats.m_numVoxelsInView;
		ptdg::Diagnostics::instance()->addMetric( visibility.stats );
	}
 
	m_priorityBounds = visibility.priorityRange;

#if _VERBOSE

	//std::cout << "priority bounds = " << bounds.lower(0) << " to " << bounds.upper(0) << std::endl;

    pt::Timer::TimeMs t1 = pt::Timer::tick();
	time += pt::Timer::delta_m(t0,t1);
	if (m_iteration % 20 == 0)
	{
		time /= 20;
//		std::cout << "Visibility iteration time = " << time << "ms" << std::endl;
	}
#endif

}	

float thinness(const pt::BoundingBox &bb)
{
	pt::vector3 d = bb.diagonal();

	if (d.x < d.y && d.x < d.z)
	{
        return static_cast<float>(max(d.x / d.y, d.x / d.z));
	}
	else if (d.y < d.x && d.y < d.z)
	{
		return static_cast<float>(max( d.y / d.x, d.y / d.z ));
	}
	else if (d.z < d.x && d.z < d.y)
	{
		return static_cast<float>(max( d.z / d.y, d.z / d.x ));
	}
	return 1.0f;
}
/*****************************************************************************/
/**
* @brief
* @param vox
* @return float
*/
/*****************************************************************************/
int64_t getFullPointCount( pcloud::Node *n, int64_t &count )
{	
	if (n->isLeaf()) 
		count += static_cast<pcloud::Voxel*>(n)->fullPointCount();
	else
	{
		for (int i=0; i<8; i++) 
			if (n->child(i)) 
				getFullPointCount(n->child(i), count);
	}
	return count;
}
float VisibilityEngine::computeVoxelLOD( const pt::ViewParams &view, pcloud::Node* n, pt::vector3d basepoint, float &projArea, float &altLOD )
{
	float pixel_area[] = {
		ProjArea::AABB_Approximator::pixArea( n, view ),
		0,//ProjArea::Scanline_Approximator::pixArea( n, view ),
		0
	};
	
	if (pixel_area[1]) 
	{
		pixel_area[0] = pixel_area[1];
	}
	projArea = pixel_area[0];	// needed for diagnostics

	int64_t fullPointCount = 0;
	getFullPointCount(n, fullPointCount);

	if (pixel_area[0] > 0 && fullPointCount > 0)
	{
		BoundingBoxD bb( n->extents() );
		bb.translateBy(-basePoint);

		float required_density[2];
		float request[2];

		float factor = 3.0f;

		required_density[0] =  factor * pixel_area[0] / fullPointCount; 

		// clamp
		if (required_density[0] < LOD_MIN)
		{
			if (fullPointCount * LOD_MIN < LOD_MIN_PTS)	
				required_density[0] = 16.0f / fullPointCount;
			else
				required_density[0] = LOD_MIN;
		}
		request[0] = required_density[0] < 1.0f ? required_density[0] : 1.0f;
		
		altLOD = request[0];
 
		return request[0];
	}
	return LOD_MIN;
}

void	VisibilityEngine::OcclusionFrame::setSize(int viewport_x_res, int viewport_y_res, float factor)
{
	m_factor = factor;
	m_vp_x = viewport_x_res;
	m_vp_y = viewport_y_res;

    m_vp_x = static_cast<int>(viewport_x_res * factor + 1);
	m_vp_y = static_cast<int>(viewport_y_res * factor + 1);
	
	if (m_frame)
	{
		delete [] m_frame;
	}
	m_frame = new Pixel[m_vp_x * m_vp_y];

	clearFrame();
}
void	VisibilityEngine::OcclusionFrame::clearFrame()
{
	if (m_frame)
	{
		memset(m_frame, 0, sizeof(Pixel)* m_vp_x*m_vp_y);
	}
}

bool	VisibilityEngine::OcclusionFrame::isOccluded(int x, int y, float z, int &vid) const
{
	if (!m_frame) return false;

	int xi = static_cast<int>(x * m_factor);
	int yi = static_cast<int>(y * m_factor);

	if (xi < 0 || yi < 0 || xi >= m_vp_x || yi >= m_vp_y )
	{
		return false;	// out of bounds
	}

	// 
	const Pixel &p = m_frame[ xi + yi * m_vp_x ]; 
	vid = p.vid;
	return (p.z < z ) ? true : false;
}
bool	VisibilityEngine::OcclusionFrame::insert(int x, int y, float z, int vid)
{	
	int xi = static_cast<int>(x * m_factor);
	int yi = static_cast<int>(y * m_factor);

	if (xi < 0 || yi < 0 || xi >= m_vp_x || yi >= m_vp_y )
	{
		return false;	// out of bounds
	}
	Pixel &p = m_frame[ xi + yi * m_vp_x ]; 
	if (p.z < 0.000001 || p.z > z)
	{
		p.z = z;
		p.vid = vid;
		return true;
	}
	return false;
};