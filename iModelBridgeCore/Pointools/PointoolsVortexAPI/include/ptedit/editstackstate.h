/*----------------------------------------------------------*/ 
/* EditStackState.h											*/ 
/* Edit Stack State Class Interface							*/ 
/*----------------------------------------------------------*/ 
/* (c) Copyright Pointools 2008-09							*/   
/*----------------------------------------------------------*/ 
/* Written by Faraz Ravi									*/ 
/*----------------------------------------------------------*/ 
#pragma once

#include <set>
#include <loki/AssocVector.h>

#include <ptfs/filepath.h>
#include <ptfs/iohelper.h>

#include <ptcloud2/voxel.h>
#include <ptcloud2/pointcloud.h>

#include <ptengine/ptengine_api.h>
#include <ptengine/module.h>

#include <ptedit/editstack.h>
#include <pt/datatree.h>

namespace ptedit
{
#define EditStateMapType Loki::AssocVector

struct LeafID
{
	pcloud::PointCloudGUID pc;
	unsigned int leafIndex;

	LeafID() {};
	LeafID( const pcloud::Voxel*v ) 
	{
		pc = v->pointCloud()->guid(); 
		leafIndex = v->indexInCloud(); 
	}
	static pcloud::Voxel * voxelFromID( LeafID lid );
};


//-----------------------------------------------------------------------------
struct LeafStateInfo
{
	ptds::DataPointer	fptr;

	uint				layersOccupied;
	uint				layersPartial;

	uint				flags;
};
//-----------------------------------------------------------------------------
//	EditStateStateHeader
//-----------------------------------------------------------------------------
struct EvaluatedOpStackHeader
{
	EvaluatedOpStackHeader( const pt::String &name, const OperationStack *editStack );
	EvaluatedOpStackHeader() : m_operations("operations") {};

	ubyte		m_version[4];

	pt::String	m_name;					// name of State
	int			m_numStackOps;			// num of stack operations
	int			m_numLeafStates;		// number of leaves
	int			m_numLeafData;			// number of leaves with data

	int			m_numLayerBytes;		// number bytes per point for layers
	bool		m_isExpandable;			// is state expandable to operations
	bool		m_hasRGB;				// has per point RGB values
	
	pt::datatree::Branch	m_operations;	// operations, used if is expandable

	typedef					std::map<LeafID, LeafStateInfo>		LeafNodeStates;
	LeafNodeStates			m_leafNodeStates;	// where the data is on the file

	bool		write( ptds::WriteBlock &wb );
	bool		read( ptds::ReadBlock &rb );
};

//-----------------------------------------------------------------------------
// Point State class : used for layer file saving / loading
//-----------------------------------------------------------------------------
class EvaluatedOpStack
{
public:

	static
	bool Apply( const ptds::FilePath &path ); // private, use create methods 
												
	static
	bool Write( const pt::String &name, const ptds::FilePath &path, OperationStack *editStack );

	static
	bool Expand( const ptds::FilePath &path, OperationStack *editStack );
};

} //namespace
