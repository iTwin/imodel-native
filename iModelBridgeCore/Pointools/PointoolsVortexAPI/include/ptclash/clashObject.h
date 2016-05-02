#pragma once
#include <ptclash/clashTree.h>
#include <pt/ObjectRef.h>
#include <vortexobjects/IClashObject.h>
#include <fastDelegate/fastDelegate.h>


namespace pt
{
enum ClashObjectHint
{
	ClashHintEnvironment,
	ClashHintObject,
	ClashHintDynamicObject
};

struct ClashObject : public vortex::IClashObject
{
	ClashObject( const pt::PersistentObjectRef &ref )
		: m_ref(ref), m_tree(0), m_refCount(0)
	{
		m_timeToPrepare = 0;
		m_lastTransform = mmatrix4d::identity();
		m_hasTransformed = true;

		m_iclashObjectCallback = NULL;
	}

	virtual ~ClashObject();

    typedef fastdelegate::FastDelegate1<float, bool> TreeFeedbackFunc;
    typedef fastdelegate::FastDelegate1<float, bool> CompareFeedbackFunc;

	// reference valid and tree generated
	bool							isPrepared() const;

	// generate the bounds tree
	bool							prepareForTest( TreeFeedbackFunc fb=0 );
	
	// object has moved since the last comparison
	bool							transformedSinceLastCompare() const		{ return m_hasTransformed; }

	// object 
	const pt::PersistentObjectRef	objRef() const		{ return m_ref; }
	ClashTree						*objTree()			{ return m_tree; }
	const ClashTree					*objTree()	const	{ return m_tree; }

	// compare ie clash detection
	ClashTree						*compareTrees( const ClashObject *b, bool difference=false, CompareFeedbackFunc func=0, bool preserveExistingResults = false );		// returns result tree

	bool							extendTree( const pt::OBBoxd &region );

#ifdef HAVE_OPENGL
    // draw the objects bounds tree
	int								drawObjBoundsTree(int depth=-1);
#endif

	int								msPreparationTime() const		{ return m_timeToPrepare; }
	int								maxTreeLeafDepth() const;
	int								minTreeLeafDepth() const;

	const mmatrix4d&				lastTransform() const	{ return m_lastTransform; }
	bool							updateTransform();

	// Interface from vortex::IClashObject, allows external client access to some ClashObject functionality	
	virtual PTres					_getClashTree(vortex::IClashTree*& clashTreeRet);		
	virtual PTres					_generateClashTree(vortex::IClashObjectCallback* callback);		
	virtual PTbool					_clashTreeFileExists(PTvoid);
	virtual const PTstr				_getClashTreeFilename(PTvoid);

	// support function to allow translation of internal TreeFeedbackFunc to external vortex::IClashObjectCallback
	bool							feedbackForIClashObject(float pcentComplete);

	// basic ref counting
	void							incRefCount();
	void							decRefCount();
	unsigned int					refCount();

private:
	// generate the bounds tree
	void							generateClashTree(int targetPerLeaf, float minDim, float maxDim );
	
	// check if object has been transformed since last comparision
	
	void							untransformTree();	// internal use. 
	void							retransformTree();	// internal use

	bool							preparePointCloud(TreeFeedbackFunc fb=0);
	
	ptds::FilePath					generateCacheFilename() const;	

	bool							m_hasTransformed;

	pt::PersistentObjectRef			m_ref;
	mmatrix4d						m_lastTransform;

	ClashObjectHint					m_hint;
	ClashTree						*m_tree;
	ClashTree						*getClashTree();

	double							m_timeToPrepare;

	vortex::IClashObjectCallback*	m_iclashObjectCallback;

	unsigned int					m_refCount;	
};
}