#include <ptedit/editStackState.h>
#include <ptengine/pointsVisitor.h>
#include <ptengine/pointsScene.h>
#include <ptengine/engine.h>

#include <queue>

#ifndef PT_NULL_FILE_POS
#define PT_NULL_FILE_POS 0xFFFFFFFFFFFFFFFF
#endif

using namespace pointsengine;
using namespace ptedit;
using namespace pcloud;

//static
Voxel * LeafID::voxelFromID( LeafID lid )
{
	PointCloud *pcloud = thePointsScene().cloudByGUID( lid.pc );

	if (!pcloud) return 0;

	if (lid.leafIndex < pcloud->voxels().size())
		return pcloud->voxels()[ lid.leafIndex ];
	return 0;
}
//-----------------------------------------------------------------------------
// New structures to enable transforming clouds / changing visibility within the 
// editing stack. This is difficult without adding a lot of state data if 
// no order dependency is to be maintained - ie. for undo  redo etc 
// instead we add an index into the state information and save staus in a separate
// structure which must also be persisted
// Note positions are always stored in world space to avoid CS shift issues
//-----------------------------------------------------------------------------
/*
		const vector3d	&translation()	const				{ return m_translation; }
		const vector3d	&eulers()		const				{ return m_eulers; }
		const vector3d	&scale()		const				{ return m_scale; }
		const vector3d  &origin()		const				{ return m_origin; }
*/
struct SceneID
{
	static const char* gen_id( pcloud::Scene *sc)
	{
		static char buff[64];
		sprintf(buff, "sc%lld", (long long)sc->objectGuid().getPart1());
		return buff;
	}
};
struct TransformReader : public PointsVisitor
{
	TransformReader(pt::datatree::Branch *b)
	{
		write_branch = b;
		scene_count = 0;
		cloud_count = 0;
	}
	bool scene(pcloud::Scene *sc)			
	{ 
		const char* id = SceneID::gen_id( sc );

		if (!write_branch->getBranch(id))
		{
			scene_branch = write_branch->addBranch(id);
		
			scene_branch->addNode( "visible", sc->displayInfo().visible() );

			cloud_count = 0;
		}
		else scene_branch = 0;

		return true; 
	}
	bool cloud(pcloud::PointCloud *cloud)	
	{ 
		if (scene_branch)
		{
			char buff[64];
			sprintf(buff, "cloud_%i", cloud_count++);
			
			pt::datatree::Branch *cloud_branch = scene_branch->addBranch(buff);

			mmatrix4d m(cloud->transform().matrix());
			cloud_branch->addNode( "visible", cloud->displayInfo().visible() );
			cloud_branch->addBlob( "transform", sizeof(mmatrix4d), &m, true );
		}
		return false; 
	}

	int						scene_count;
	int						cloud_count;
	pt::datatree::Branch	*write_branch;
	pt::datatree::Branch	*scene_branch;
};
// write the transformations back to the scene / clouds
struct TransformWriter : public PointsVisitor
{
	TransformWriter(const pt::datatree::Branch *b)
	{
		read_branch = b;
		scene_count = 0;
		cloud_count = 0;
	}
	bool scene(pcloud::Scene *sc)			
	{ 
		const char* id = SceneID::gen_id( sc );

		scene_branch = read_branch->getBranch(id);

		if (scene_branch)
		{
			bool vis;
			
			if (scene_branch->getNode( "visible", vis ))	
			{
				if (vis)	sc->displayInfo().show();
				else		sc->displayInfo().hide();
			}
		}
		cloud_count = 0;
		return true; 
	}
	bool cloud(pcloud::PointCloud *cloud)	
	{ 
		if (scene_branch)
		{
			bool vis = true;
			bool changed = false;
			char buff[64];
			mmatrix4d m;

			sprintf(buff, "cloud_%i", cloud_count++);
			
			pt::datatree::Branch *cloud_branch = scene_branch->getBranch(buff);

			if (cloud_branch->getNode( "visible", vis ))
			{
				if (vis)	cloud->displayInfo().show();
				else		cloud->displayInfo().hide();
			}
			
			pt::datatree::Blob *b = cloud_branch->getBlob("transform");

			if (b && b->_size == sizeof(mmatrix4d))
			{
				memcpy(&m, b->_data, sizeof(m));

				if (cloud->transform().matrix() != m)
				{
					cloud->transform().useMatrix(m);
					changed = true;	
				}
			}
			if (changed)
			{
				// really want to avoid this
				// if there is not enough data in a voxel the bounds
				// will be taken from the node, this bad because the 
				// extents will be larger and so LOD effected - results
				// if bad performance
				cloud->projectBounds().dirtyBounds();
				cloud->computeBounds();

				const_cast<pcloud::Scene*>(cloud->scene())->computeBounds();
			}
		}
		return false; 
	}

	int						scene_count;
	int						cloud_count;
	const pt::datatree::Branch	*read_branch;
	const pt::datatree::Branch	*scene_branch;
};
struct PointCloudStateHandler 
{
	PointCloudStateHandler()
	{
		// register self
		static StateHandler s;	// just aggregates delegates..hmm might be issues in multi-thread stack use (not supported)

		s.read = pt::MakeDelegate( this, &PointCloudStateHandler::readState );
		s.write = pt::MakeDelegate( this, &PointCloudStateHandler::writeState );
		OperationStack::addStateHandler( &s );
	}
	void writeState(pt::datatree::Branch *b) const
	{
		// read the state of all point clouds and write to branch
		TransformReader tr(b);
		thePointsScene().visitPointClouds( &tr );
	}
	void readState(const pt::datatree::Branch *b)
	{
		// find the node
		TransformWriter tw(b);
		thePointsScene().visitPointClouds( &tw );
	}
};
static PointCloudStateHandler s_pointCloudStateHandler;
//-----------------------------------------------------------------------------
// Create Cloud State Visitor : creates the per cloud state structures
// THIS IS UNUSED AND ITS INTENT IS UNCLEAR
//-----------------------------------------------------------------------------
template <class Map>
struct CreateCloudStateVisitor : public PointsVisitor
{
	CreateCloudStateVisitor( Map &m, ubyte flags ) 
		:	m_data( m ), m_flags( flags )		
	{}

	bool cloud(pcloud::PointCloud *cloud)	
	{ 
		m_data.insert( 
			Map::value_type(
				cloud->objectGuid(), 
				new CloudEditState( cloud, (ubyte)m_flags ))		// insert the per cloud state object
				);
		return true;
	}

private:

	Map		&m_data;

	uint	m_byteSize;
	uint	m_flags;
};
//
// Visitor to gather leaves needing write
//
struct GatherForWriteVisitor : public PointsVisitor
{
	GatherForWriteVisitor( EvaluatedOpStackHeader::LeafNodeStates &nodeStates) 
		: hasPaint( false ), nodes( nodeStates ), numLayerBytes(0), 
		numLeafNodesWithData(0)
	{ }

	bool voxel(pcloud::Voxel *vox)			
	{ 
		LeafStateInfo leaf;
		leaf.fptr = 0;

		if (vox->channel( pcloud::PCloud_Filter )
			|| vox->flag(pcloud::Painted))
		{
			leaf.fptr = 1;
			if (vox->flag(pcloud::Painted)) hasPaint = true;
		}

		leaf.flags = 0;
		leaf.flags |= vox->flag( pcloud::WholeSelected ) ? 1 : 0;
		leaf.flags |= vox->flag( pcloud::WholeHidden ) ? 2 : 0;
		leaf.flags |= vox->flag( pcloud::PartSelected ) ? 4 : 0;
		leaf.flags |= vox->flag( pcloud::PartHidden ) ? 8 : 0;

		leaf.layersOccupied = vox->layers(0);
		leaf.layersPartial = vox->layers(1);
		
		if (leaf.flags || leaf.fptr || leaf.layersOccupied || leaf.layersPartial)
		{
			LeafID leafID( vox );
			nodes.insert( EvaluatedOpStackHeader::LeafNodeStates
				::value_type( leafID, leaf ));
		}
		if (vox->channel( pcloud::PCloud_Filter))
		{
			++numLeafNodesWithData;
			if (!numLayerBytes)
			{
				DataChannel *f = vox->channel( PCloud_Filter );
				numLayerBytes = f->typesize() * f->multiple();
			}
		}
		return false; // no point iteration
	}

	// members
	int										numLayerBytes;
	EvaluatedOpStackHeader::LeafNodeStates	&nodes;
	bool									hasPaint;
	int										numLeafNodesWithData;

};
//-----------------------------------------------------------------------------
// EditStackHeader - used for IO
//-----------------------------------------------------------------------------
EvaluatedOpStackHeader::EvaluatedOpStackHeader( const pt::String &name, const OperationStack *editStack )
: m_operations("operations")
{
	m_isExpandable = false;
	m_name = name;
	m_numStackOps = editStack->stack()->numBranches();
	m_isExpandable = false;
	m_version[0] = 1;
	m_version[1] = 0;
	m_version[2] = 0;
	m_version[3] = 0;

	GatherForWriteVisitor gather( m_leafNodeStates );	// gather voxels
	thePointsScene().visitVoxels( &gather, false ); 

	m_numLeafStates = m_leafNodeStates.size();	// num voxels to write
	m_hasRGB = gather.hasPaint;
	m_numLayerBytes = gather.numLayerBytes;
	m_numLeafData = gather.numLeafNodesWithData;
}
//-----------------------------------------------------------------------------
bool EvaluatedOpStackHeader::write( ptds::WriteBlock &wb )
{
	wb.write( m_version, 4 );

	wb.write( m_name );
	wb.write( m_numLayerBytes );
	wb.write( m_numStackOps );
	wb.write( m_isExpandable );
	wb.write( m_hasRGB );
	wb.write( m_numLeafStates );
	wb.write( m_numLeafData );
	
	LeafNodeStates::iterator i = m_leafNodeStates.begin();

	int filePtrIndex = 0;

	while (i != m_leafNodeStates.end())	// insert filepointers as place holders
	{						
		LeafStateInfo leaf( i->second );	
		LeafID id( i->first );

		wb.write( id.pc );						//write the full leaf id
		wb.write( id.leafIndex );

		if (leaf.fptr) 
		{
			wb.insertPlaceholder(filePtrIndex++);	//placeholder as this is not known until the data is written
		}
		else wb.write( leaf.fptr );				//null if no filter channel to write

		wb.write( leaf.layersOccupied );		
		wb.write( leaf.layersPartial );
		wb.write( leaf.flags );

		++i;
	}
	return true;
}
//-----------------------------------------------------------------------------
bool EvaluatedOpStackHeader::read( ptds::ReadBlock &rb )
{
	rb.read( m_version, 4 );

	rb.read( m_name );
	rb.read( m_numLayerBytes );
	rb.read( m_numStackOps );
	rb.read( m_isExpandable );
	rb.read( m_hasRGB );

	rb.read( m_numLeafStates );
	rb.read( m_numLeafData );
	
	std::set< pcloud::PointCloudGUID > pcs;
	std::set< pcloud::PointCloudGUID >::iterator pci;

	for (int i=0; i< m_numLeafStates; i++)
	{
		LeafID leafID;
		LeafStateInfo leaf;	

		rb.read( leafID.pc );
		rb.read( leafID.leafIndex );

		pcs.insert( leafID.pc );

		rb.read( leaf.fptr );

		rb.read( leaf.layersOccupied );
		rb.read( leaf.layersPartial );
		rb.read( leaf.flags );

		m_leafNodeStates.insert( LeafNodeStates::value_type( leafID, leaf ) );
	}
	/* update upper node states */ 
	pci = pcs.begin();

	while ( pci != pcs.end() )
	{
		PointCloud *pcloud = thePointsScene().cloudByGUID( (*pci) );

		if (pcloud)
		{
			pcloud->root()->recursiveLayerConsolidate();
			pcloud->root()->recursiveFlagsConsolidate();
		}
		++pci;
	}
	return true;
}
//-----------------------------------------------------------------------------
// Point State class
//-----------------------------------------------------------------------------
/* static */ 
bool EvaluatedOpStack::Write( const pt::String &name, const ptds::FilePath &path, OperationStack *editStack )
{
	/* start File */ 
	ptds::DataSourcePtr fhandle = ptds::dataSourceManager.openForWrite(&path);

	ptds::Tracker tracker( fhandle );
	
	if ( fhandle )
	{
		/* open file and write header */ 
		ptds::WriteBlock wblock( fhandle, 32655, &tracker );

		const char * identifier = "edit";
		wblock.write( identifier, 4 );
		
		EvaluatedOpStackHeader header( name, editStack );	// header object
		header.write( wblock );

		/* write the channel data */ 
		/* do this is managable chunks */ 
		/* gather voxels to write */ 
		EvaluatedOpStackHeader::LeafNodeStates::iterator i = 
			header.m_leafNodeStates.begin();

		int filePtrIndex = 0;

		while (i != header.m_leafNodeStates.end() )
		{
			LeafID lid = i->first;
			Voxel *v = LeafID::voxelFromID( lid );

			if (v)
			{
				DataChannel *filter = v->channel( PCloud_Filter );
				
				if (filter)
				{
					tracker.placeReference( filePtrIndex ); // filepointer to this data

					wblock.write( (int)PCloud_Filter );
					wblock.write( filter->bytesize() );
					wblock.write( filter->data(), filter->bytesize() );	// write to block				
					
					++filePtrIndex;
				}
				
			}
			++i;
		}
	}
	else return false;	

	tracker.writePlaceHolders();
	ptds::dataSourceManager.close(fhandle);

	return true;
}
//
//
//
namespace ptedit
{
bool EvaluatedOpStack::Apply( const ptds::FilePath &path )
{
	/* start File */ 
	ptds::DataSourcePtr fhandle = ptds::dataSourceManager.openForRead(&path);
	
	if ( fhandle )
	{
		/* open file and write header */ 
		ptds::Tracker tracker( fhandle );
		ptds::ReadBlock rblock( fhandle, &tracker );

		char id[4];
		rblock.read( id, 4 );
		if ( id[0] != 'e' || id[1] != 'd' || id[2] != 'i' || id[3] != 't' )
		{
			ptds::dataSourceManager.close(fhandle);

			return false; // not an edit file 
		}

		EvaluatedOpStackHeader header;	// header object
		header.read( rblock );

		/* write the channel data */ 
		/* do this is managable chunks */ 
		/* gather voxels to write */ 
		EvaluatedOpStackHeader::LeafNodeStates::iterator i = 
			header.m_leafNodeStates.begin();

		int filePtrIndex = 0;

		while (i != header.m_leafNodeStates.end() )
		{
			LeafID lid = i->first;
			LeafStateInfo leaf = i->second;

			Voxel *v = LeafID::voxelFromID( lid );

			if (v)	// could fail if the pointcloud is not loaded
			{
				/* basic state */ 
				v->layers(0) = (ubyte)leaf.layersOccupied;
				v->layers(1) = (ubyte)leaf.layersPartial;

				v->flag( pcloud::WholeSelected, leaf.flags & 1 ? true : false );
				v->flag( pcloud::WholeHidden, leaf.flags & 2 ? true : false );
				v->flag( pcloud::PartSelected, leaf.flags & 4 ? true : false );
				v->flag( pcloud::PartHidden, leaf.flags & 8 ? true : false );

				if (leaf.fptr && leaf.fptr > 16)	// channel data
				{
					DataChannel *filter = v->channel( PCloud_Filter );
					if (!filter)
					{
						v->buildEditChannel();
						filter = v->channel( PCloud_Filter );
						assert(filter);
						if (!filter) // out of mem
							continue;
					}
					
					if (tracker.moveTo( leaf.fptr ))	// invalidates readblock so use direct IO funcs
					{
						/* read data block */ 
						int channelType = 0;
						uint dataSize = 0;
						
						fhandle->readBytes(channelType);
						fhandle->readBytes(dataSize);
						
						// assert here means vox in mem does not match on file 
						assert( filter->bytesize() >= dataSize );
						fhandle->readBytes(filter->data(), dataSize < filter->bytesize() ? dataSize : filter->bytesize() );			
					}
				}
			}
			++i;
		}

		ptds::dataSourceManager.close(fhandle);

	}
	else return false;	

	return true;
}
}
//
//
//