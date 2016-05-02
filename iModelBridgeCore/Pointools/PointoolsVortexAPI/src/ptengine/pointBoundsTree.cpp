#include "PointoolsVortexAPIInternal.h"
#include <ptapi/PointoolsVortexAPI_ResultCodes.h>
#include <ptapi/PointoolsVortexAPI.h>

#include <ptengine/pointBoundsTree.h>
#include <ptengine/engine.h>
#include <ptengine/queryScene.h>
#include <ptengine/pointsScene.h>
#include <ptengine/renderEngine.h>

#include <ptengine/queryEngine.h>
#include <ptengine/voxelLoader.h>
#include <ptengine/queryDensity.h>
#include <ptengine/queryFilterGrid.h>
#include <ptengine/queryFilterOBB.h>
#include <ptengine/queryPointsArray.h>
#include <ptengine/queryTraversalOctree.h>

#include <ptengine/StreamManager.h>

using namespace pt;
using namespace pointsengine;

#define MAX_CANDIDATE_PTS	5000000
#define MAX_LEAF_DEPTH		24
#define MIN_LEAF_DEPTH		12
#define PRELIMINARY_DEPTH	8

//#define  ENABLE_TRACING	1

#ifdef ENABLE_TRACING
void trace(const char* format, ...)
{
	static char buffer[1024];

	va_list argptr;
	va_start(argptr, format);
	wvsprintf(buffer, format, argptr);
	va_end(argptr);

	OutputDebugString(buffer);
}
#else
void trace(const char* format, ...)
{
}
#endif
//-----------------------------------------------------------------------------
struct AggregatePoints
{	
	template <class T>
	void point( const vec3<T> &pnt, int index, const ubyte &layers )
	{
		vec3<T> p(pnt.x,pnt.y,pnt.z);
		pts.push_back(p);
	} 
	IMPL_FILTER_IN

	std::vector<pt::vector3d> pts;
};
//-----------------------------------------------------------------------------
struct FixedNumPtsDensity	
{	
	FixedNumPtsDensity(int target_points )
		: targetPoints(target_points), loader(0) {}

	void	preQuery( pcloud::Voxel*v )
	{
		float lod = (float)targetPoints / v->fullPointCount();
		if (lod > 1.0f) lod = 1.0f;

		loader = new VoxelLoader(v,lod,false, false);
	}
	float	lodAmount( pcloud::Voxel*v )
	{
		return v->getCurrentLOD();
	}
	void	postQuery( pcloud::Voxel *v )
	{
		delete loader;
		loader = 0;
	}

	VoxelLoader *loader;
	int targetPoints;
};
//-----------------------------------------------------------------------------
// gather the corners of voxel extents
struct GatherCorners
{
	QueryPointsArrayd points;
	
	template <class T>
	void point( const vec3<T> &pnt, int index, const ubyte &layers ) {}

	inline FilterResult node( const pcloud::Node *n )			
	{	
		pt::BoundingBoxD b = n->extents();		
		
		if (n->isLeaf())
		{		
			pt::vector3d p;
			for (int i=0; i<8; i++)
			{
				b.getExtrema(i, p);

				points.addPoint(p);
			}
			return FilterOut;  // prevent per point, no need
		}
		return FilterIn;
	} 
	inline FilterResult cloud( const pcloud::PointCloud *pc )	
	{			
		return FilterIn;	
	} 
	inline FilterResult scene( const pcloud::Scene *sc )			
	{	
		return FilterIn;	
	} 
	
};
//-----------------------------------------------------------------------------
// count points in voxels that intersect the box
struct ApproxCountPointsInBox
{
	ApproxCountPointsInBox(const pt::OBBoxd &obb) 
		: box(obb), numVoxelsIntersecting(0), numVoxels(0), numPoints(0) {}

	pt::OBBoxd	box;
	int		numVoxelsIntersecting;
	int		numVoxels;
	__int64	numPoints;

	template <class T>
	void point( const vec3<T> &pnt, int index, const ubyte &layers ) {}

	inline FilterResult node( const pcloud::Node *n )			
	{	
		if (n->isLeaf())
		{
			pt::BoundingBoxD ext = n->extents();			

			const pcloud::Voxel * v= static_cast<const pcloud::Voxel*>(n);

			if (box.contains(ext))
			{
				++numVoxels;
				numPoints += v->fullPointCount();
			}
			else if (box.intersects(ext) || (box.containedBy(ext)))
			{
				++numVoxelsIntersecting;
				++numVoxels;
				double overlap = approxOverlap(ext, box);
				numPoints += static_cast<__int64>(overlap * static_cast<double>(v->fullPointCount()));
			}			
			else return FilterOut;
			
			return FilterOut;  // prevent per point, no need
		}
		return FilterIn;
	} 
	inline FilterResult cloud( const pcloud::PointCloud *pc )	
	{			
		return FilterIn;	
	} 
	inline FilterResult scene( const pcloud::Scene *sc )			
	{	
		return FilterIn;	
	} 

private:
	// Calculate the approximate percentage overlaps of node extents and a bouding box.
	// @return 0.0 to 1.0 overlapping
	double approxOverlap(const pt::BoundingBoxD& nodeExtents, const pt::OBBoxd& box)
	{
		// the node extents are axis aligned, the box may not be so create an axis 
		// aligned box to give a fast approximation		
		pt::BoundingBoxD aaBox;
		box.getAABB(aaBox);

		double overlapX = overlap(nodeExtents.lx(), nodeExtents.ux(), aaBox.lx(), aaBox.ux());
		double overlapY = overlap(nodeExtents.ly(), nodeExtents.uy(), aaBox.ly(), aaBox.uy());
		double overlapZ = overlap(nodeExtents.lz(), nodeExtents.ux(), aaBox.lz(), aaBox.uz());
		
		double volExt = nodeExtents.volume();
		double volOverlap = overlapX * overlapY * overlapZ;				

		// prevent divide by zero
		if (volExt < 0.00000000001)
			volExt = 0.00000000001;

		return (volOverlap < volExt) ? (volOverlap / volExt) : 1.0; // return the percentage overlap
	}
	
	// only to be used with known intersecting boxes
	double overlap(double alower, double aupper, double blower, double bupper)
	{
		if (alower > aupper) return 0.0; // invalid case
		if (blower > bupper) return 0.0; // invalid case
		if (aupper < blower) return 0.0; // no overlap
		if (bupper < alower) return 0.0; // no overlap

		double ret = 0.0;

		if (alower < blower)
		{
			if (aupper < bupper)
				ret = aupper - blower; 
			else
				ret = bupper - blower; // b is completely contained within a
		}
		else
		{
			if (bupper < aupper)
				ret = bupper - alower;
			else
				ret = aupper - alower; // a is completely contained within b

		}	

		// special case for this filter, if the lines are just touching at either end
		// return 0.001 times the length of the shortest line so that at least a small 
		// proportion of points are taken into account rather than none
		if (fabs(ret) < 0.00000000001)
		{
			if ((fabs(aupper - blower) < 0.00000000001)
				|| (fabs(bupper - alower) < 0.00000000001))
			{
				if ((aupper - alower) < (bupper - blower))
					ret = 0.001 * (aupper - alower);
				else
					ret = 0.001 * (bupper - blower);
			}			
		}

		return fabs(ret);

	}

};

// extend the tree for the area indicated by the bounding box
//-----------------------------------------------------------------------------
bool PointsBoundsTree::extendTree( 
			PointsBoundsTree *tree, 
			pcloud::PointCloud *cloud, const pt::OBBoxd &region,
			int target_pnts_per_box, 
			double min_dim, 
			double max_dim,
			ProgressFeedback feedback)
{
	// first extract voxels that overlap the bounding box
	// BB and point cloud must be untransformed
	// The tree must already be generated to voxel level - although its not checked for here
	
	tree->pointsProcessed = 0;

	float vol_region = region.extents().x * region.extents().y * region.extents().z;
	if (vol_region < 0.001) vol_region = 1.0;

	float vol_cloud = cloud->root()->extents().volume();

	tree->totalPoints = cloud->numPoints() * vol_cloud / vol_region;	// of course this is not exact

	// extract overlapping tree nodes
	std::set< const TreeNode* > leaves;
	std::set< const TreeNode* >::iterator leaves_it;
	tree->root()->collectIntersectingLeaves( leaves, region );
	

	int nodes_processed = 0;

	// build out incomplete leaves
	for (leaves_it = leaves.begin(); leaves_it != leaves.end(); ++leaves_it)
	{
		if (!((*leaves_it)->flags() & TreeNode::Complete))
		{
			TreeNode *node = const_cast<TreeNode*>(*leaves_it);			
			if (node->intersects(region))
			{
				tree->buildTree( node, false, target_pnts_per_box, min_dim, max_dim, cloud, 0, feedback );
				++nodes_processed;				
			}
		}		
	}
	return nodes_processed ? true : false;
};

//-----------------------------------------------------------------------------
PointsBoundsTree *PointsBoundsTree::createFromPointCloud( pcloud::PointCloud *cloud, int target_pnts_per_box, 
														 double min_dim, double max_dim, 
														 ProgressFeedback feedback )
{	
#define NEW_TREE 1
#ifdef NEW_TREE

	if(getStreamManager().begin() == false)
	{
		return NULL;
	}

	GatherCorners corners;
	OctreeTraversal tr(cloud, pt::ProjectSpace);
	FullDensity fd;
	QueryEngine::run( tr, corners, fd );

	if (!corners.points.empty())
	{
		PointsBoundsTree *tree= new PointsBoundsTree();

		// this is AA to minimise overlap for the first few levels
		OBBoxd box = pt::createFittingAABBd( corners.points.data(), corners.points.size() );
		tree->setRootBounds( box );
		tree->root()->setBounds( box );

		tree->buildTree(tree->root(), false, target_pnts_per_box, min_dim, max_dim, cloud, 0, feedback );

		//transform into local coordinates 
//		mmatrix4d objMat;
//		Transform::evaluateMatrixStack(objMat, cloud->transform()); 
//		objMat.invert();	// to make it project to local

		//tree->root()->transform(objMat);

		getStreamManager().end();

		return tree;
	}

	getStreamManager().end();

	return 0;
#else

	if(getStreamManager().lockStreamManager() == false)
	{
		return NULL;
	}

	AggregatePoints aggr;
	OctreeTraversal tr(cloud, pt::LocalSpace);
	QueryEngine::run(tr, aggr);

	PointsBoundsTree *tree= new PointsBoundsTree();

	OBBoxf box = pt::createFittingOBBf( aggr.pts );
	tree->setRootBounds( box );
	tree->root()->setBounds( box );

	tree->buildTree( tree->root(), aggr.pts, target_pnts_per_box, min_dim, max_dim, cloud, 0, feedback );

	getStreamManager().releaseStreamManager();

	return tree;
#endif

}
//-----------------------------------------------------------------------------
PointsBoundsTree *PointsBoundsTree::createFromScene( pcloud::Scene *scene, int target_pnts_per_box, 
													double min_dim, double max_dim, ProgressFeedback feedback )
{
	if (scene->size() == 1)
	{
		return createFromPointCloud( scene->cloud(0), target_pnts_per_box, min_dim, max_dim, feedback );
	}

	if(getStreamManager().begin() == false)
	{
		return NULL;
	}

	GatherCorners corners;
	OctreeTraversal tr(scene, pt::ProjectSpace);
	QueryEngine::run( tr, corners );

	if (!corners.points.empty())
	{
		PointsBoundsTree *tree= new PointsBoundsTree();

		// this is AA to minimise overlap for the first few levels
		OBBoxd box = pt::createFittingAABBd( corners.points.data(), corners.points.size() );
		tree->setRootBounds( box );
		tree->root()->setBounds( box );

		tree->buildTree(tree->root(), false, target_pnts_per_box, min_dim, max_dim, 0, scene, feedback );

		//transform into local coordinates 
//		mmatrix4d objMat;
//		Transform::evaluateMatrixStack(objMat, scene->transform()); 
//		objMat.invert();	// to make it project to local

		//tree->root()->transform(objMat);

		getStreamManager().end();

		return tree;
	}

	getStreamManager().end();

	return 0;
}
//-----------------------------------------------------------------------------
PointsBoundsTree *PointsBoundsTree::createFromProject(int target_pnts_per_box, double min_dim, double max_dim,
													  ProgressFeedback feedback )
{
	return 0;
}
//-----------------------------------------------------------------------------
static void filterIntoBox( const QueryPointsArrayd *pnts, const OBBoxd &leftBox, const OBBoxd &rightBox,
						  QueryPointsArrayd *left_pnts, QueryPointsArrayd *right_pnts )					  
{
	for (int i=0; i<pnts->size(); i++)
	{
			// must put into a box
		if (leftBox.center().dist2( pnts->at(i) ) <
			rightBox.center().dist2( pnts->at(i) ))
		{
			left_pnts->push_back( pnts->at(i) );
		}
		else right_pnts->push_back( pnts->at(i) );
	}
}
//-----------------------------------------------------------------------------
PointsBoundsTree::PointsBoundsTree()
{
	pointsProcessed=0;
	totalPoints=1;	// avoid dbz
}

//-----------------------------------------------------------------------------------------	
// compute bounds based only on voxel tight fit Bounding Boxes
// return false if needs points load
//-----------------------------------------------------------------------------------------	
static bool calculateFastBounds(	OBBoxd &left, /*initial left box*/ 
									OBBoxd &right, /*initial right box */
									pcloud::PointCloud *cloud,
									pcloud::Scene *scene,
									double filterSpacing)
{
	//get total point count in bounds, including intersecting voxels
	ApproxCountPointsInBox count_left(left);
	ApproxCountPointsInBox count_right(right);

	OctreeTraversal traverseC(cloud, pt::ProjectSpace);
	OctreeTraversal traverseS(cloud, pt::ProjectSpace);
	QueryEngine::run(cloud ? traverseC : traverseS, count_left);
	QueryEngine::run(cloud ? traverseC : traverseS, count_right);	

	if ( ((count_left.numPoints + count_right.numPoints) < 10e6)	// want to limit too many queries, better to take few big ones
		|| ((count_left.numVoxels + count_right.numVoxels) < 4) )
	{		
		return false; // ie better to load points and use points
	}
	else
	{
		QueryPointsArrayd left_pts;
		QueryPointsArrayd right_pts;

		left_pts.clear();
		right_pts.clear();

		OBBFilter2NodeExtents<double, QueryPointsArrayd> 
			filterExtents(left_pts, left, right_pts, right, filterSpacing);

		ProportionalDensity p_density(0);

		QueryEngine::run(cloud ? traverseC : traverseS, filterExtents, p_density);


		//left_pts.reserve(500 * count_left.numVoxelsIntersecting );
		//right_pts.reserve(500 * count_right.numVoxelsIntersecting );

		//OBBFilter2<float, QueryPointsArrayf > 
		//	leftRightFilter( left_pts, left, right_pts, right, true, true);


		// evaluate intersecting points only and put corners of extents in array
		//QueryEngine::run(traverse, leftRightFilter, p_density);

		// fit boxes
		if (left_pts.size()==0)
		{
			left.extents(vector3d(0,0,0));
		}
		else 
		{
			// fast bounds use oriented BB - because this will cause less overlap
			// in the lower structure
			left = createFittingAABBd( left_pts.data(), left_pts.size() );
		}

		if (right_pts.size()==0)
		{
			right.extents(vector3d(0,0,0));
		}
		else 
		{
			// fast bounds use oriented BB - because this will cause less overlap
			// in the lower structure

			right = createFittingAABBd( right_pts.data(), right_pts.size() );
		}


		return true;
	}
}
//-----------------------------------------------------------------------------------------	
static bool extractPoints( OBBoxd &left, /*initial left box*/ 
							OBBoxd &right, /*initial right box */
							pcloud::PointCloud *cloud, //cloud or
							pcloud::Scene *scene,      // scene
							 const pt::vector3d &spacing,
							 bool axisAligned,
							QueryPointsArrayd &left_pts, /* out left pts array, if small number of points */ 
							QueryPointsArrayd &right_pts, /* out left pts array, if small number of points */				
							float &proportionFiltered		// proportion of total points filtered into boxes based on grid filtering
							)
{	
	trace("Extracting Points at %imm spacing\n", (int)(spacing.x*1000));

	typedef OBBFilter2<double, QueryPointsArrayd > LeftRightFilter;

	// extract all points
	LeftRightFilter lrfilter( left_pts, left, right_pts, right );

	// filter points to grid to avoid higher density than required
	typedef GridFilter<LeftRightFilter, HashGrid> SubSampleFilter;
	SubSampleFilter ss(lrfilter, spacing*0.5f);

	OctreeTraversal traverseC(cloud, pt::ProjectSpace);
	OctreeTraversal traverseS(cloud, pt::ProjectSpace);

	FullDensity fd;

	QueryEngine::run(cloud ? traverseC : traverseS, lrfilter/*ss*/, fd );

	// calc proportion filtered
	proportionFiltered = (float)ss.filteredCount() / ss.insertCount();

	//trace("Left:%i Right:%i pts extracted, %i%%\n", 
	//	(int)left_pts.size(), (int)right_pts.size(), (int)100*proportionFiltered);

	// fit boxes
	if (left_pts.size()==0)
	{
		left.extents(vector3d(0,0,0));
	}
	else left = axisAligned ? createFittingAABBd( left_pts.data(), left_pts.size() )
		: createFittingOBBd( left_pts.data(), left_pts.size() );

	if (right_pts.size()==0)
	{
		right.extents(vector3d(0,0,0));
	}
	else right = axisAligned ? createFittingAABBd( right_pts.data(), right_pts.size() )
		: createFittingOBBd( right_pts.data(), right_pts.size() );

	return true;
}
//-----------------------------------------------------------------------------
static void splitBox( const OBBoxd &box, OBBoxd &left, OBBoxd &right )
{
	// split box on largest axis
	int a = box.extents().major_axis();

	// left/right prelim boxes for filtering
	left = box;
	right = box;

	double extent = box.extents()[a]*0.5;
	left.extent(a, extent );					// half extent
	right.extent(a, extent );	

	left.translate( box.axis(a) * -extent );	// shift left
	right.translate( box.axis(a) * extent );	// shift right
}
//-----------------------------------------------------------------------------
// build tree in detail using points
bool PointsBoundsTree::buildTreeDetail( TreeNode *_node, TreeNode *_parent,  
										QueryPointsArrayd &pts, float proportionFiltered, bool prelim,
										int target_pts, double min_dim, double max_dim,
										pcloud::PointCloud *cloud, pcloud::Scene *scene,										
										ProgressFeedback feedback )
{
	int iterations = 0;

	pt::vector3d spacing(min_dim, min_dim, min_dim);

	// inline stack based recursion is faster than program stack
	std::stack<SplitData*> splitStack;	
	SplitData *split = new SplitData( _node, _parent, &pts );
	splitStack.push( split );	// root split
	
	bool first = true;

	while (splitStack.size())
	{
	// get split on top
		SplitData *data = splitStack.top();
		splitStack.pop();

		__int64 debug_count = QueryPointsArrayd::totalPointsAllocatedCount();
		__int64 debug_mem = QueryPointsArrayd::totalPointsStoredCount();

		// start of algo
		OBBoxd left, right;
		OBBoxd box = data->node->bounds();
		
		// if its a prelim tree, quit if we have reached depth
		if (prelim && data->node->depth() >= PRELIMINARY_DEPTH)
		{
			if (!first) delete data->pnts;	// first is on stack not heap!
			delete data;
			continue;
		}

		// don't move this outside the loop - although at first it seems these allocations can be re-used, 
		// its not that simple
		QueryPointsArrayd *left_pnts = new QueryPointsArrayd((int)1e6,(int)5e5);
		QueryPointsArrayd *right_pnts = new QueryPointsArrayd((int)1e6,(int)5e5);

		// split box on largest axis
		data->node->split();
		splitBox(box, left, right);

		filterIntoBox( data->pnts, left, right, left_pnts, right_pnts);

		//left_pnts->resize( left_pnts->size() );
		//right_pnts->resize( right_pnts->size() );

		++iterations;

		data->pnts->clear();	// free up memory
		if (!first) delete data->pnts;	// first is on stack not heap!
		first = false;

		data->pnts = 0;

		// compute tight fitting obbs
		if (left_pnts->empty())
		{
			data->node->removeLeft();
			delete left_pnts;
			left_pnts = 0;
		}
		else
		{
			if (data->node->depth() < PRELIMINARY_DEPTH)
			{
				left = createFittingAABBd( left_pnts->data(), left_pnts->size() );
			}
			else
			{
				left = createFittingOBBd( left_pnts->data(), left_pnts->size() );
			}
			data->node->left()->setBounds(left);

			//recursive
			// if target not reached
			bool too_many_pnts = left_pnts->size() > target_pts*1.25 ? true : false;
			bool too_shallow = data->node->left()->depth() < MIN_LEAF_DEPTH ? true : false;
			bool too_deep = data->node->left()->depth() >= MAX_LEAF_DEPTH ? true : false;
			bool too_small = ( left.extents().x < min_dim ||	left.extents().y < min_dim || left.extents().z < min_dim ) ? true : false;
			bool too_big = ( left.extents().x > max_dim || left.extents().y > max_dim || left.extents().z > max_dim ) ? true : false;

			if ( !too_deep && ( too_shallow || (too_big || ( !too_small && too_many_pnts))))
			{ 
				splitStack.push( 
					new SplitData( data->node->left(), data->node, left_pnts )
					); 
				if (data->node->depth() % 4)
					trace("dep=%i\n", data->node->depth());
			}
			else	//hit a leaf
			{
				// add to progress on leaf creation			
				pointsProcessed += left_pnts->size();
				if (pointsProcessed-lastProgress > totalPoints/100 && feedback )
				{
					float feedback_perc = 100 * (float)pointsProcessed/totalPoints;
					if (feedback_perc>1.1f)	// otherwise < that this will hide bar
						feedback( feedback_perc );
					lastProgress = pointsProcessed;
				}
				data->node->left()->setFlag( TreeNode::Complete );

				// set the element count, scale by lod of points
				int numPoints =  (float)left_pnts->size() / ((proportionFiltered>0) ?  proportionFiltered : 1.0f);
				data->node->left()->setElementCount( numPoints );

				left_pnts->destroy();
				delete left_pnts;
				left_pnts = 0;
			}
		}
		// compute tight fitting obbs
		if (right_pnts->empty())
		{
			data->node->removeRight();
			delete right_pnts;
		}
		else
		{
			if (data->node->depth() < PRELIMINARY_DEPTH)
			{
				right = createFittingAABBd( right_pnts->data(), right_pnts->size() );
			}
			else
			{
				right = createFittingOBBd( right_pnts->data(), right_pnts->size() );
			}

			data->node->right()->setBounds(right);

			// if target not reached
			bool too_many_pnts = right_pnts->size() > target_pts*1.25 ? true : false;
			bool too_shallow = data->node->right()->depth() < MIN_LEAF_DEPTH ? true : false;
			bool too_deep = data->node->right()->depth() >= MAX_LEAF_DEPTH ? true : false;
			bool too_small = ( right.extents().x < min_dim ||	right.extents().y < min_dim || right.extents().z < min_dim ) ? true : false;
			bool too_big = ( right.extents().x > max_dim || right.extents().y > max_dim || right.extents().z > max_dim ) ? true : false;

			if ( !too_deep && ( too_shallow || (too_big || ( !too_small && too_many_pnts))))						
			{ 
				splitStack.push( 
					new SplitData( data->node->right(), data->node, right_pnts )
					); 
//				if (data->node->depth() % 4)
//					trace("dep=%i\n", data->node->depth());
			}
			else
			{
				// add to progress on leaf creation			
				pointsProcessed += right_pnts->size();
				if (pointsProcessed-lastProgress > totalPoints/100 && feedback)
				{
					float feedback_perc = 100 * (float)pointsProcessed/totalPoints;
					if (feedback_perc>1.1f)	// otherwise < that this will hide bar
						feedback( feedback_perc );
					lastProgress = pointsProcessed;
				}	
				data->node->right()->setFlag( TreeNode::Complete );
				
				// set the element count, scale by lod of points
				int numPoints =  (float)right_pnts->size() / ((proportionFiltered>0) ?  proportionFiltered : 1.0f);

				data->node->right()->setElementCount( numPoints );

				right_pnts->destroy();
				delete right_pnts;
				right_pnts = 0;
			}		
		} //if right empty
		delete data;
	}
	return true;
}
//-----------------------------------------------------------------------------
bool PointsBoundsTree::buildTree( TreeNode *_node, bool preliminary, int target_pts, double min_dim, double max_dim,
								 pcloud::PointCloud *cloud, pcloud::Scene *scene, 
								 ProgressFeedback feedback )
{
	// initialise root
	if (!_node->depth())
	{
		pointsProcessed=0;
		totalPoints = cloud ? cloud->numPoints() : scene->fullPointCount();
		lastProgress = 0;
	}
	
	m_minDim = min_dim;
	m_maxDim = max_dim;
	m_targetPntsPerBox = target_pts;

	// sanity check
	assert (min_dim < max_dim);

	int iterations = 0;

	pt::vector3d spacing(min_dim, min_dim, min_dim);

	// inline stack based recursion is faster than program stack
	std::stack<SplitData*> splitStack;	
	SplitData *split = new SplitData( _node, 0, 0 );
	splitStack.push( split );	// root split

	while (!splitStack.empty())
	{
		SplitData *data = splitStack.top();
		splitStack.pop();

		if (data->node->depth() > PRELIMINARY_DEPTH && preliminary)
			continue;

		// start of algo
		OBBoxd box = data->node->bounds();
		
		pt::vector3d ext = box.extents();
		
		// extend the extents by a small amount to avoid boundary conditions causing
		// boxes aligned to boundaries failing when they should be included
		// SA: this appears to be unnessecary after fixing other issues, but leaving the code here
		// commented out in case it is needed in the future
//		ext.x += 1e-12;
//		ext.y += 1e-12;
//		ext.z += 1e-12;
//		box.extents(ext);		
				
		//split left and right
		OBBoxd left, right;
		splitBox( box, left, right );

		data->node->split();

		bool fast_ok = calculateFastBounds( left, right, cloud, scene, min_dim );

		bool axis_aligned = data->node->depth() < PRELIMINARY_DEPTH ? true : false;

		if (fast_ok)
		{
			if ( left.extents().is_zero() == false )
			{
				data->node->left()->setBounds(left);

				if (!preliminary || data->node->depth() < PRELIMINARY_DEPTH )
				{
					splitStack.push( new SplitData(data->node->left(), data->node, 0 ) ); 
				}
			}
			else data->node->removeLeft();

			if ( right.extents().is_zero() == false )
			{			
				data->node->right()->setBounds(right);

				if (!preliminary || data->node->depth() < PRELIMINARY_DEPTH )
				{
					splitStack.push( new SplitData(data->node->right(), data->node, 0 ) ); 
				}
			}		
			else data->node->removeRight();
		}
		else if (!preliminary)
		{
			QueryPointsArrayd left_pnts;
			QueryPointsArrayd right_pnts;

			float proportionFiltered = 1.0f;

			extractPoints(left, right, cloud, scene, spacing, axis_aligned, left_pnts, right_pnts, proportionFiltered);			
			
			if (left_pnts.size() > 1e4)
				left_pnts.resizeToFit();

			if (right_pnts.size() > 1e4)
				right_pnts.resizeToFit();

			//left / right is mutated to be tight fit
			if ( left.extents().is_zero() == false && left_pnts.size())
			{
				data->node->left()->setBounds(left);
				
				// build tree down to detail
				buildTreeDetail( data->node->left(), data->node, left_pnts, 
							proportionFiltered, preliminary, 
							target_pts, min_dim, max_dim, 
							cloud, scene, feedback ); 
			}
			else data->node->removeLeft();

			if ( right.extents().is_zero() == false && right_pnts.size())
			{
				data->node->right()->setBounds(right);

				// build tree down to detail
				buildTreeDetail( data->node->right(), data->node, right_pnts, 
								proportionFiltered, preliminary, 
								target_pts, min_dim, max_dim,
								cloud, scene, feedback );
			}
			else data->node->removeRight();

			if (data->node->isLeaf())
				data->node->setFlag( TreeNode::Complete );

		} // end engine read		
	}
	return true;
}
#ifdef HAVE_OPENGL
//-----------------------------------------------------------------------------
void PointsBoundsTree::drawBox( const OBBoxd &box )
{
	vector3d vertices[8];
	box.computeVertices( vertices );

	// simple wire-frame box
	glBegin(GL_LINE_STRIP);
		glVertex3dv( vertices[0] );
		glVertex3dv( vertices[1] );
		glVertex3dv( vertices[3] );
		glVertex3dv( vertices[2] );
		glVertex3dv( vertices[0] );
	glEnd();

	glBegin(GL_LINE_STRIP);
		glVertex3dv( vertices[4] );
		glVertex3dv( vertices[6] );
		glVertex3dv( vertices[7] );
		glVertex3dv( vertices[5] );
		glVertex3dv( vertices[4] );
	glEnd();

	glBegin(GL_LINES);
		glVertex3dv( vertices[3] );
		glVertex3dv( vertices[7] );

		glVertex3dv( vertices[2] );
		glVertex3dv( vertices[6] );

		glVertex3dv( vertices[0] );
		glVertex3dv( vertices[4] );

		glVertex3dv( vertices[1] );
		glVertex3dv( vertices[5] );
	glEnd();
}
//-----------------------------------------------------------------------------
void PointsBoundsTree::drawLeaves()
{
	static int interation = 0;

	if (root())
	{
		std::vector< OBBoxd > boxes;
		root()->collectLeaves(boxes);

		for (int i=0; i<boxes.size(); i++)
		{
			drawBox( boxes[i] );
		}
	}
}
//-----------------------------------------------------------------------------
void PointsBoundsTree::drawTree()
{
	if (root())
	{
		std::vector< OBBoxd > boxes;
		root()->collectLeaves(boxes);

		for (int i=0; i<boxes.size(); i++)
		{	
			drawBox( boxes[i] );
		}
	}
}
//-----------------------------------------------------------------------------
void PointsBoundsTree::drawNodes( int level )
{
	if (root())
	{
		std::vector< OBBoxd > boxes;
		root()->collectNodes(boxes, level);

		for (int i=0; i<boxes.size(); i++)
		{	
			drawBox( boxes[i] );
		}
	}
}
#endif
//-----------------------------------------------------------------------------
void PointsBoundsTree::cullLargeLeaves( TreeNode *node, double max_dim )
{
	if (node->left())
	{
		if (node->left()->isLeaf())
		{
			const OBBoxd &box = node->left()->bounds();
			if (box.axis(0).length() > max_dim
				|| box.axis(1).length() > max_dim
				|| box.axis(2).length() > max_dim)
			{
				node->removeLeft();
			}
		}
		else cullLargeLeaves( node->left(), max_dim );
	}
	
	if (node->right())
	{
		if (node->right()->isLeaf())
		{
			const OBBoxd &box = node->right()->bounds();
			if (box.axis(0).length() > max_dim
				|| box.axis(1).length() > max_dim
				|| box.axis(2).length() > max_dim)
			{
				node->removeRight();
			}
		}
		else cullLargeLeaves( node->right(), max_dim );
	}
}
int		PointsBoundsTree::getBuildTargetPntsPerBox() const
{
	return m_targetPntsPerBox;
}	
int		PointsBoundsTree::getBuildMinBoxDim() const
{
	return m_minDim;
}
int		PointsBoundsTree::getBuildMaxBoxDim() const
{
	return m_maxDim;
}


