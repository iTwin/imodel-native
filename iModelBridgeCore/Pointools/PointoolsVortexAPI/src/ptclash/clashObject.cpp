/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PointoolsVortexAPIInternal.h"
// clashObject.cpp

#include <ptapi/PointoolsVortexAPI.h>

#include <ptengine/pointBoundsTree.h>
//#include <ptmodel/ModelBoundsTree.h>
#include <ptclash/clashObjectManager.h>
#include <ptclash/clashObject.h>
#include <ptfs/filepath.h>

#include <pt/timer.h>

using namespace pt;
using namespace pointsengine;
using namespace vortex;

//-----------------------------------------------------------------------------
ClashObject::~ClashObject()
{
	delete m_tree;	
}
//-----------------------------------------------------------------------------
bool		ClashObject::isPrepared() const
{
	// check if tree is on disk
	if (!m_tree)
	{
		ptds::FilePath filepath( generateCacheFilename() );
		return filepath.checkExists();
	}
	else return true;
}
//-----------------------------------------------------------------------------
void		ClashObject::untransformTree()
{
	mmatrix4d xform = m_lastTransform;
	xform.invert();

	m_lastTransform = mmatrix4d::identity();
	m_tree->tree()->root()->transform( xform );
}
//-----------------------------------------------------------------------------
void		ClashObject::retransformTree()
{
	m_tree->tree()->root()->transform( m_lastTransform );
}
//-----------------------------------------------------------------------------
bool		ClashObject::updateTransform()
{
	if (!m_tree)
		return false;

	// generate the bounds tree
	const pt::Object3D *obj = ClashObjectManager::theClashObjectManager().resolveReference(m_ref);

	if (!obj)
	{
		delete m_tree;
		m_tree = 0;
		return false;
	}

	// compute diff between applied matrix and new one	
	mmatrix4d objMat;
	obj->registration().compileMatrix(objMat, ProjectSpace);			 

	if (obj->isGroup())
	{
		const Group3D *group = static_cast<const Group3D*>(obj);

		if (group->numObjects() == 1)
		{			
			group->object(0)->registration().compileMatrix(objMat, ProjectSpace);			 			
		}
	}

	// is there anything to update?
	if (m_lastTransform != objMat)
	{
		m_lastTransform.invert();
		mmatrix4d diffMat = objMat >> m_lastTransform;
        UNUSED_VARIABLE(diffMat);

		m_tree->tree()->root()->transform( m_lastTransform );	// back to identity
		m_tree->tree()->root()->transform( objMat );

		m_lastTransform = objMat;
		m_hasTransformed = true;
		return true;
	}
	else
	{
		m_hasTransformed = false;
	}
	return true;
}

extern pcloud::Scene* sceneFromHandle( PThandle );
extern pcloud::PointCloud*	cloudFromHandle(PThandle cloud);

//-----------------------------------------------------------------------------
bool		ClashObject::prepareForTest( TreeFeedbackFunc feedback )
{
	if (m_tree) 
	{
		return updateTransform();
	}

	// generate the bounds tree
	const pt::Object3D *obj = ClashObjectManager::theClashObjectManager().resolveReference(m_ref);

	//m_tree = obj->extractBoundsTree()  // yes but not now
	if (!obj) return false;

	if (wcsncmp(obj->typeDescriptor(), L"Point Cloud", 11)==0)
	{
		return preparePointCloud(feedback);
	}

	return false;
}
#ifdef HAVE_OPENGL
//-----------------------------------------------------------------------------
int	ClashObject::drawObjBoundsTree(int depth)
{
	int nodes = 0;

	if (m_tree)
	{
		if (depth>=0)
		{
			nodes = m_tree->drawNodes(depth);
		}
		else nodes = m_tree->drawTree();
	}
	return nodes;
}
#endif
namespace 
{
#define NUM_DIFFERENCING_THREADS	8

//-----------------------------------------------------------------------------
static void computeTreeGenerationParametersGeneric( const pt::Object3D * obj, int &target_pts, double &leaf_min_dim, double &leaf_max_dim )
//-----------------------------------------------------------------------------
{
	pt::BoundingBoxD bb = obj->projectBounds().bounds();

	double size = bb.diagonal().length();

// 	leaf_min_dim = size / 2500.0f;
// 	leaf_max_dim = size / 750.0f;


	if (size < 20.0f)
	{
		leaf_min_dim = 0.02f;
		leaf_max_dim = 0.1f;
	}
	else if (size < 50.0f)
	{
		leaf_min_dim = 0.025f;
		leaf_max_dim = 0.15f;
	}
	else if (size < 100.0f)
	{
		leaf_min_dim = 0.05f;
		leaf_max_dim = 0.25f;
	}
	else if (size < 200.0f)
	{
		leaf_min_dim = 0.075f;
		leaf_max_dim = 0.5f;
	}
	else if (size < 500.0f)
	{
		leaf_min_dim = 0.1f;
		leaf_max_dim = 1.0f;
	}
	else if (size < 1000.0f)
	{
		leaf_min_dim = 0.2f;
		leaf_max_dim = 3.0f;
	}
	else if (size < 2500.0f)
	{
		leaf_min_dim = 0.5f;
		leaf_max_dim = 10.0f;
	}
	else if (size < 5000.0f)
	{
		leaf_min_dim = 1.25f;
		leaf_max_dim = 30.0f;
	}	
	else
	{
		leaf_min_dim = 5.f;
		leaf_max_dim = 100.0f;
	}
}
//-----------------------------------------------------------------------------
static void computeTreeGenerationParameters( const pcloud::PointCloud * cloud, int &target_pts, double &leaf_min_dim, double &leaf_max_dim ) 
//-----------------------------------------------------------------------------
{
	computeTreeGenerationParametersGeneric(cloud, target_pts,leaf_min_dim, leaf_max_dim);
	
	target_pts = static_cast<int>(cloud->numPoints() / 50000);
	if (target_pts < 1000)
		target_pts = 1000;

}
//-----------------------------------------------------------------------------
static void computeTreeGenerationParameters( const pcloud::Scene * scene, int &target_pts, double &leaf_min_dim, double &leaf_max_dim )
//-----------------------------------------------------------------------------
{
	computeTreeGenerationParametersGeneric(scene,target_pts,leaf_min_dim, leaf_max_dim);

	target_pts = static_cast<int>(scene->fullPointCount() / 50000);

	if (target_pts < 1000)
		target_pts = 1000;
}
//-----------------------------------------------------------------------------

	int mt_progress_val[NUM_DIFFERENCING_THREADS];
	std::mutex mt_progress_mutex;
}
//-----------------------------------------------------------------------------
struct DifferenceComputer // ala Signore Babbage
{
	typedef std::set< const OrientedBoxBoundsTreed::Node*> LeavesSet;

	DifferenceComputer(
		int				thread_id,
		const LeavesSet &leaves, 
		const ClashObject *b,
		ClashObject::CompareFeedbackFunc func )
		: m_leaves(leaves), m_obj(b), m_func(func), m_threadid(thread_id)
	{
	}

	void operator ()()
	{
		int num_leaves = static_cast<int>(m_leaves.size());

		if (!num_leaves)
			return;

		int pos = 0;
		float prog = 0;

		for (LeavesSet::const_iterator i=m_leaves.begin(); i!=m_leaves.end(); i++)
		{
			OrientedBoxBoundsTreed::Node* node =
				const_cast<OrientedBoxBoundsTreed::Node*>(*i);						

			// progress feedback - note multiple thread handling
			prog = (float)pos / num_leaves;	// prevent thrashing
			
			if (pos % 1000==0)
			{
				std::lock_guard<std::mutex> lk(mt_progress_mutex);
				mt_progress_val[m_threadid] = pos;
			}
			++pos;

			if ( !m_obj->objTree()->tree()->root()->intersectsLeaf(node->bounds()) )
			{			
				node->setFlag( OrientedBoxBoundsTreed::Node::NodeInterferance );
			}
		}
		// final completion
        std::lock_guard<std::mutex> lk(mt_progress_mutex);
		mt_progress_val[m_threadid] = pos;
	}
	int									m_threadid;
	const ClashObject *					m_obj;
	ClashObject::CompareFeedbackFunc	m_func;
	const LeavesSet						&m_leaves;
};
//-----------------------------------------------------------------------------
ClashTree	*ClashObject::compareTrees( const ClashObject *b, bool difference, CompareFeedbackFunc func, bool preserveExistingResults )
{
	OrientedBoxBoundsTreed *tr = 0;
	
	if (difference)
	{
#ifdef _DEBUG
		// Preserve existing results only implemented for interference
		assert(preserveExistingResults != true);
#endif // _DEBUG

		OrientedBoxBoundsTreed* tree = const_cast<OrientedBoxBoundsTreed*>(objTree()->tree());
		tree->root()->clearFlags(true);
		
		typedef std::set< const OrientedBoxBoundsTreed::Node*> LeavesSet;

		LeavesSet leaves;
		tree->root()->collectLeaves( leaves );

		int num_leaves = static_cast<int>(leaves.size());

		// distribute these to 8 threads
		int leaves_per_thread = num_leaves / NUM_DIFFERENCING_THREADS;

		int count = 0;
		LeavesSet threadLeaves[NUM_DIFFERENCING_THREADS];
		DifferenceComputer *diffComps[NUM_DIFFERENCING_THREADS];
		std::thread *diffThreads[NUM_DIFFERENCING_THREADS];

		for (LeavesSet::const_iterator i=leaves.begin(); i!=leaves.end(); i++)
		{
			int th = count++ / leaves_per_thread;
			if (th >= NUM_DIFFERENCING_THREADS)
				th = NUM_DIFFERENCING_THREADS-1;

			threadLeaves[ th ].insert( *i );
		}
		// run these threads
		for (int i=0; i<NUM_DIFFERENCING_THREADS; i++)
		{
			mt_progress_val[i]=0;
			diffComps[i] = new DifferenceComputer(i,threadLeaves[i],b,func);
			diffThreads[i] = new std::thread(*diffComps[i]);
		}

		// update progress bar in main thread
		int total_processed = 0;
		int perc =0;

		while (total_processed < num_leaves)
		{
            BeThreadUtilities::BeSleep(100);
			{
                std::lock_guard<std::mutex> lk(mt_progress_mutex);

				total_processed = 0;

				for (int i=0; i<NUM_DIFFERENCING_THREADS; i++)
					total_processed += mt_progress_val[i];
			}
			int p = static_cast<int>((100.0f * total_processed) / num_leaves);

			// only update on percentage change
			if (p > perc)
			{
                func(static_cast<float>(p));
				perc = p;
			}
		};

		// wait for completion
		for (int i=0; i<NUM_DIFFERENCING_THREADS; i++)
		{
			diffThreads[i]->join();
		}

		// clean up
		for (int i=0; i<NUM_DIFFERENCING_THREADS; i++)
		{
			delete diffThreads[i];
			delete diffComps[i];
		}

		// propagate flags up the tree
		tree->root()->collateChildFlags( OrientedBoxBoundsTreed::Node::NodeInterferance );

		// extract the clash tree
		OrientedBoxBoundsTreed::Node * diff_root = tree->root()->extractTree( OrientedBoxBoundsTreed::Node::NodeInterferance );

		if (diff_root)
		{
			tr = new OrientedBoxBoundsTreed(diff_root);
		}
	}
	else
	{
		tr = b->objTree()->tree()->computeInterferance(
			const_cast<OrientedBoxBoundsTreed*>( objTree()->tree()), preserveExistingResults );
	}

	if (tr)
	{
		if (tr->root()->isLeaf())
		{
			// false positive
			delete tr;
			return 0;
		}
		
		double maxDim = max(b->objTree()->maxLeafDim(), objTree()->maxLeafDim() );	// also we can recompute from the resulting tree
		double minDim = max(b->objTree()->minLeafDim(), objTree()->minLeafDim() );   // but this is faster
		double avPts = (b->objTree()->targetPtsPerLeaf() + objTree()->targetPtsPerLeaf()) * 0.5;	// not accurate, but doesn't matter

		ClashTree *ct = new ClashTree( m_ref, tr, minDim, maxDim, static_cast<int>(avPts) );
		return ct;
	}
	return 0;
}
//-----------------------------------------------------------------------------
bool	ClashObject::extendTree( const pt::OBBoxd &region )
{
	if (!m_tree) 
		return false;
/*
	// extend the bounds tree
	pt::Object3D *obj = m_ref.resolveReference();

	if (!obj) return false;

	bool hasCache = false;

	if (wcsncmp(obj->typeDescriptor(), L"Point Cloud", 11)==0)
	{
		pcloud::PointCloud *cloud=0;
		pcloud::Scene *scene = 0;

		if (m_ref.resolveReference(scene))
		{
			cloud = scene->cloud(0);	// TODO: support full scenes
		}
		// try to get cloud object
		if (cloud || m_ref.resolveReference(cloud))
		{
			// extend the tree
			PointsBoundsTree *ptree = static_cast<PointsBoundsTree*>(m_tree->tree());
			// TODO: Handle min/max/targetpnts, these are not stored in clashdata file
			bool didextend = PointsBoundsTree::extendTree( ptree, cloud, region, TARGET_PNTS_PER_LEAF, LEAF_MIN_DIM, LEAF_MAX_DIM );			

			ptfs::FilePath filepath( generateCacheFilename() );

			if (didextend)
			{
				m_tree->saveToFile( filepath, m_lastTransform );
			}
		}
	}
	else if (wcsncmp(obj->typeDescriptor(), L"3D Model", 8)==0)
	{
		return false;	// not supported
	}*/
	return true;
}
//-----------------------------------------------------------------------------
bool ClashObject::preparePointCloud(TreeFeedbackFunc fb )
{
	ptds::FilePath filepath( generateCacheFilename() );

	if (m_tree) return true;
	else m_tree = new ClashTree(m_ref);	// to try to load

    pt::Timer::TimeMs t0=0, t1=0;

	int		target_pts;
	double	leaf_min_dim, leaf_max_dim;

	if (!m_tree->loadFromFile( filepath, m_lastTransform ))
	{
		delete m_tree;
		m_tree = 0;

		pcloud::PointCloud *cloud=cloudFromHandle(m_ref.key());
		if (cloud)
		{
			// is it a scene
			t0 = pt::Timer::tick();
			
			computeTreeGenerationParameters( cloud, target_pts, leaf_min_dim, leaf_max_dim );

			PointsBoundsTree *tree = PointsBoundsTree::createFromPointCloud( cloud, target_pts, leaf_min_dim, leaf_max_dim, fb );

			t1 = pt::Timer::tick();

			if (tree)
			{
				m_tree = new ClashTree( m_ref, tree, leaf_min_dim, leaf_max_dim, target_pts );							
				cloud->registration().compileMatrix(m_lastTransform, ProjectSpace);			
			}
		}
		else
		{
			pcloud::Scene *scene = sceneFromHandle(m_ref.key());
			if (scene)
			{
				t0 = pt::Timer::tick();

				computeTreeGenerationParameters( scene, target_pts, leaf_min_dim, leaf_max_dim );

				PointsBoundsTree *tree = PointsBoundsTree::createFromScene( scene, target_pts, leaf_min_dim, leaf_max_dim, fb );

				t1 = pt::Timer::tick();

				if (tree)
				{
					m_tree = new ClashTree( m_ref, tree, leaf_min_dim, leaf_max_dim, target_pts );									
					scene->registration().compileMatrix(m_lastTransform, ProjectSpace);				

					//special handling for scene with only one point cloud
					if (scene->size() == 1)
					{											
						scene->cloud(0)->registration().compileMatrix(m_lastTransform, ProjectSpace);						
					}
				}
			}
		}		
		m_tree->saveToFile( filepath, m_lastTransform );
	}
	else
	{
		if (m_tree) 
		{
			bool ret = updateTransform();
			fb(100.0f);
			return ret;
		}
	} 
	
	if (t0>0 && t1>0)
	{
		m_timeToPrepare = (double)pt::Timer::delta_m(t0,t1);
	}
	return m_tree ? true : false;
}
//-----------------------------------------------------------------------------
 ptds::FilePath ClashObject::generateCacheFilename() const
{
	// get the main file path
	ptds::FilePath scenePath;

	// clash data files load from the same folder as the pod file by default, see
	// if a different folder location has been set
	pt::String userClashDataFolder;
	ClashObjectManager::theClashObjectManager().getClashTreeFolder(userClashDataFolder);
	
	const Object3D *obj = ClashObjectManager::theClashObjectManager().resolveReference(m_ref);
	const Object3D *scene_obj = obj;

	while (scene_obj)
	{
		if (strcmp(scene_obj->objectClass(), "Scene3D")==0)
		{
			scenePath = static_cast<const Scene3D*>(scene_obj)->filepath();
			break;
		}
		scene_obj = scene_obj->parent();
	}
	
	if (!scene_obj) 
		return scenePath;	// reference failure

	// if a user folder has been set then use that instead of the local folder
	if (userClashDataFolder.length())
	{
		wchar_t tempPath[PT_MAXPATH] = {0};
        BeStringUtilities::Snwprintf(tempPath, PT_MAXPATH, L"%s\\%s", userClashDataFolder.c_wstr(), scenePath.filename());
		scenePath = tempPath;
	}

	pt::String cacheFilename;

	// Guid based filename
	const Guid &guid = m_ref.guid();

	if (guid) 
	{
		pt::String guidStr;
        guidStr.format(".%llx.%llx", guid.getPart1(), guid.getPart2());
		cacheFilename = scenePath.filename();
		cacheFilename += guidStr;
	}
	else
	{
		wchar_t dateStr[256];
		dateStr[0]=0;

        time_t lastWriteTime;
        if(BeFileNameStatus::Success == BeFileName::GetFileTime(nullptr, nullptr, &lastWriteTime, scenePath.filename()))
		    {
            DateTime dateTime;
            if(SUCCESS == DateTime::FromUnixMilliseconds(dateTime, (uint64_t) lastWriteTime * 1000))
                {
                
				// Build a string showing the date and time.
				swprintf(dateStr, 256, L"%02d%02d%02d%02d%02d", 
                         dateTime.GetMonth(), dateTime.GetDay (), dateTime.GetYear(), dateTime.GetHour (), dateTime.GetMinute ());
			}
		}
		cacheFilename = scenePath.filename();
		if (dateStr[0])
		{
			cacheFilename += pt::String(".");		
			cacheFilename += pt::String(dateStr);
		}
	}
	scenePath.setFilenameOnly( cacheFilename.c_wstr() );
	scenePath.setExtension(L"clashdata");

	return scenePath;
}
int ClashObject::maxTreeLeafDepth() const
{
	if (m_tree)
	{
		return m_tree->maxLeafDepth();
	}
	return 0;
}

int ClashObject::minTreeLeafDepth() const
 {
	if (m_tree)
	{
		return m_tree->minLeafDepth();
	}
	return 0;
 }

//----------------------------------------------------------------------------
// vortex::IClashObject implementation
//----------------------------------------------------------------------------
PTres ClashObject::_getClashTree(vortex::IClashTree*& clashTreeRet)
{
	if (m_tree)
	{
		clashTreeRet = m_tree;
		return PTV_SUCCESS;
	}

	return PTV_NOT_INITIALIZED;
}

bool ClashObject::feedbackForIClashObject(float pcentComplete)
{
	if (m_iclashObjectCallback)
		m_iclashObjectCallback->clashTreeGenerationProgress(static_cast<PTint>(pcentComplete));

	return true;
}

PTres ClashObject::_generateClashTree(vortex::IClashObjectCallback* callback)
{
	m_iclashObjectCallback = callback;

    ClashObject::TreeFeedbackFunc stub = fastdelegate::MakeDelegate(this, &ClashObject::feedbackForIClashObject);
	if (prepareForTest((m_iclashObjectCallback) ? stub : NULL))
		return PTV_SUCCESS;

	return PTV_FILE_FAILED_TO_CREATE;
}

PTbool ClashObject::_clashTreeFileExists()
{
	ptds::FilePath path = generateCacheFilename();
	return path.checkExists();
}

const PTstr ClashObject::_getClashTreeFilename()
{
	static ptds::FilePath currentFilePath = generateCacheFilename();	
	if (!currentFilePath.isEmpty())
	{
		return currentFilePath.path();
	}

	return NULL;
}

void ClashObject::incRefCount() 
{ 
	m_refCount++; 
}
	
void ClashObject::decRefCount() 
{ 
#ifdef _DEBUG
	assert(m_refCount > 0);
#endif // _DEBUG

	m_refCount--; 
}
	
unsigned int ClashObject::refCount() 
{ 
	return m_refCount; 
}
