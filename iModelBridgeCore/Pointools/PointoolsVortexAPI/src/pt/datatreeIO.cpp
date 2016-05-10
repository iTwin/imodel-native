#include "PointoolsVortexAPIInternal.h"
#include <pt/datatreeIO.h>
#include <ptds/DataSourceBufferedWrite.h>
#include <ttl/meta/typelist.hpp>
#include <ptfs/iohelper.h>

namespace pt { namespace datatree {

// WARNING
// THIS IS NOT COMPATIBLE WITH Datatree::readTree/writeTree
//-----------------------------------------------------------------------------
struct GetSize
{
	GetSize() : size(0) {}

	template <class T>
	void operator()() { size = sizeof(T); }

	int size;
};
//-----------------------------------------------------------------------------
struct IOMeta
{
	IOMeta()
	{
		sizeof_meta = sizeof(IOMeta);
		littleEndian = 1;
		version[0] = 2;
		version[1] = 0;
		version[2] = 0;
		version[3] = 0;

		nodeIdSize = NODE_ID_SIZE;
		nodeIdIsWchar = 0;

		numTypes = Variant::list::length;

		// table of type sizes. Valid for all but String
		memset(typesizeTable, 0, sizeof(typesizeTable));
		ttl::meta::type_switch<Variant::list> ts;
		GetSize gs;

		for (int i=0; i<numTypes; i++)
		{
			ts(i, gs);
			typesizeTable[i] = gs.size;
		}
	}

	int idSize() const
	{
		return nodeIdSize * (nodeIdIsWchar ? sizeof(wchar_t) : 1);
	}
	int		sizeof_meta;
	uint8	version[4];
	uint8	littleEndian;
	uint8	nodeIdIsWchar;
	uint8	nodeIdSize;
	uint8	res;	// 4 byte alignment
	int		numTypes;
	int		typesizeTable[256];
};
//-----------------------------------------------------------------------------
// write node
//-----------------------------------------------------------------------------
struct WriteNodeValueVisitor
{
	WriteNodeValueVisitor( ptds::DataSourceBufferedWrite &buffer )
		: _buffer(buffer)				{};

	void operator ()(const String &s)	{	_buffer.writeString(s);	}
	
	template <class T> 
	void operator ()(const T &t)		{	_buffer.writeBytes(t);	}

private:
	ptds::DataSourceBufferedWrite	&_buffer;
};

//-----------------------------------------------------------------------------
struct WriteNodeVisitor
{
	WriteNodeVisitor( ptds::DataSourceBufferedWrite &buffer, IOMeta &meta )
		: _buffer(buffer), _meta(meta)	
	{}

	bool writeBranchMeta( const Branch *b )
	{
		uint32 num_subbranches = (uint32)b->numBranches();
		uint32 num_nodes = (uint32)b->numNodes();
		uint32 num_blocks = (uint32)b->numBlobs();	// blocks are only blobs for now, could be extended

		// write identifier	
		writeId( b->id() );

		// write counts
		_buffer.writeBytes( num_subbranches );
		_buffer.writeBytes( num_nodes );
		_buffer.writeBytes( num_blocks );

		// write flags
		uint8 levelAndFlags[] = { b->level(), b->flags(0), b->flags(1), b->flags(2) };
		_buffer.writeBytes( levelAndFlags );

		return true;
	};
	bool writeId( const NodeID &nodeId )
	{
		return (_buffer.writeBytes( &nodeId, _meta.idSize() ) == _meta.idSize() ? true : false );
	}
	// visitor functors
	void operator() (const NodeID &nid, const Node *node)
	{	
		uint8 type_index = node->typeId();

		// write identifier	
		writeId( nid );

		// type index	 
		_buffer.writeBytes( type_index );

		// write value
		WriteNodeValueVisitor wv(_buffer);
		node->visitType(wv);
	};
	bool operator () (const Branch *b)
	{
		writeBranchMeta( b );

		// write additional variable branch data blocks
		// each will be:
		// - type (8 chars)
		// - id
		// - size (bytes to skip/read from here)
		// - data
		
		// for now this is only blobs, 
		// but arrays could be added in the future
		const char *blobId="bin_blob";
		int num_blocks = b->numBlobs();

		for (int i=0; i<num_blocks; i++)
		{
			NodeID id;
			const Blob *blob = b->getBlob(i, id, false);
			if (blob)
			{
				_buffer.writeBytes( blobId, 8 );	// "BIN_BLOB"
				
				writeId( id );				

				_buffer.writeBytes( static_cast<unsigned int>(blob->_size + sizeof(int)));
				_buffer.writeBytes( blob->_compression );
				_buffer.writeBytes( blob->_data, blob->_size );
			}
		}
		return true;
	}
private:
	ptds::DataSourceBufferedWrite	&_buffer;
	IOMeta							&_meta;
};
//-----------------------------------------------------------------------------
bool writeBinaryDatatree( const Branch *dtree, ptds::DataSource * dataSrc)
{
	// Header
	const char *tag = "dtree2.0";

	ptds::DataSourceBufferedWrite buffer(dataSrc, 512*1024);

	// write Data tree tag
	buffer.writeBytes(tag, 8);

	// write Meta data
	IOMeta meta;
	buffer.writeBytes(meta.sizeof_meta);
	buffer.writeBytes(meta.littleEndian);
	buffer.writeBytes(meta.version);
	buffer.writeBytes(meta.nodeIdIsWchar);
	buffer.writeBytes(meta.nodeIdSize);
	buffer.writeBytes(meta.numTypes);
	buffer.writeBytes(meta.typesizeTable, sizeof(meta.typesizeTable));
	buffer.writeBytes(meta.res);

	WriteNodeVisitor wv(buffer, meta);

	// recursively write tree
	dtree->visitNodes(wv);

	return true;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
struct ReadValAndAddNode
{
	ReadValAndAddNode(ptds::DataSource * dataSrc, Branch *owner, const NodeID &id )
		: _dataSrc(dataSrc), _owner(owner), _nodeid(id)
	{}

	template <class T>
	void operator()()
	{		
        T myT;
        Variant v(myT);
		ttl::var::apply_visitor(*this, v);
	}
	
	template <class T>
	void operator()( const T &v )
	{
		T val;
		_dataSrc->readBytes( val ); 
		_owner->addNode(_nodeid, val); 
	}

	void operator()(const pt::String &v)
	{
		pt::String val;
		_dataSrc->readString( val );
		_owner->addNode(_nodeid, val); 
	}

private:
	NodeID _nodeid;
	ptds::DataSource * _dataSrc;
	Branch *_owner;
};
//-----------------------------------------------------------------------------
static bool readId( ptds::DataSource * dataSrc, NodeID &id,  IOMeta &meta )
{
	return (dataSrc->readBytes( &id, meta.idSize() ) == meta.idSize() ? true : false );
}
//-----------------------------------------------------------------------------
static void readNode( ptds::DataSource * dataSrc, Branch *owner, IOMeta &meta )
{
	NodeID nodeid;
	uint8 type_index;

	dataSrc->readBytes( &nodeid, meta.idSize() );
	dataSrc->readBytes( type_index );

	// if unknown Variant type
	if (type_index > Variant::list::length-1)
	{
		//skip this
		dataSrc->movePointerBy( meta.typesizeTable[type_index] );
	}
	else
	{
		ReadValAndAddNode r(dataSrc, owner, nodeid);

		ttl::meta::type_switch<Variant::list> ts;
		ts(type_index, r); 

	#ifdef DATATREE_DEBUGGING
	//	Node* nd = const_cast<Node*>(owner->getNode(nodeid));
	//	ListBranchVisitor::_nodeInfo(nodeid, nd, owner);
	#endif
	}
}
//-----------------------------------------------------------------------------
static bool readBranch( ptds::DataSource * dataSrc, IOMeta &meta, Branch *root, Branch *parent=0 )
{
	NodeID nodeid;
	uint32 num_subbranches;
	uint32 num_nodes;
	uint32 num_blocks;
	//uint8 level;
	//uint8 flags[4];

	// Branch meta
	// write identifier	
	readId( dataSrc, nodeid, meta );

	// read counts
	dataSrc->readBytes( num_subbranches );
	dataSrc->readBytes( num_nodes );
	dataSrc->readBytes( num_blocks );

	// read flags
	uint8 levelAndFlags[4];
	dataSrc->readBytes( levelAndFlags );

	Branch *branch = 0;

	/*root case*/ 
	if (!parent)
	{
		branch = root;
		branch->setID(nodeid);
	}
	else
	{
		/*other cases*/ 
		branch = parent->getBranch(nodeid, true);	// will create if not found
	}
	unsigned int i;

	// copy flags
	branch->setFlags(0, levelAndFlags[1]);
	branch->setFlags(1, levelAndFlags[2]);
	branch->setFlags(2, levelAndFlags[3]);

	// read blobs 
	NodeID	nid;
	uint32	block_size;
	uint8*	blob_data;

	char block_type[9];
	block_type[8] = 0;	// terminating /0

	// Blocks
	for (i=0; i<num_blocks; i++)
	{
		dataSrc->readBytes( block_type, 8 );	// variable block type
		
		readId( dataSrc, nid, meta );			// variable block id

		dataSrc->readBytes( block_size );		// variable block size

		if (strncmp("bin_blob", block_type, 8)==0)	// only blobs supported in this version
		{
			try
			{
				int compression = 0;
				dataSrc->readBytes( compression );

				block_size-=sizeof(int);

				blob_data = new uint8[block_size];
				if (block_size != dataSrc->readBytes( blob_data, block_size ))
					break;

				branch->addBlob(nid, block_size, blob_data, false, true/*del*/, false/*compress*/);
				Blob *b = branch->getBlob(nid,false);
				b->_compression = compression;
				
			}
			catch(std::bad_alloc)
			{
				continue;
			}
		}
		else 
		{
			dataSrc->movePointerBy( block_size );	// skip this block if not handled
		}
	}

	// Nodes
	for (i=0;i<num_nodes;i++)
	{
		readNode(dataSrc, branch, meta);
	}

	// Branches
	for (i=0; i<num_subbranches; i++)
		readBranch( dataSrc, meta, root, branch);

	return true;
}
//-----------------------------------------------------------------------------
bool readBinaryDatatree( Branch *root, ptds::DataSource * dataSrc )
{
	// Header
	char tag[8];

	// write Data tree tag
	dataSrc->readBytes(tag, 8);

	if (memcmp(tag, "dtree2.0", 8)==0)
	{
		// read Meta data
		IOMeta meta;
		dataSrc->readBytes(meta.sizeof_meta);
		dataSrc->readBytes(meta.littleEndian);
		dataSrc->readBytes(meta.version);
		dataSrc->readBytes(meta.nodeIdIsWchar);
		dataSrc->readBytes(meta.nodeIdSize);
		dataSrc->readBytes(meta.numTypes);
		dataSrc->readBytes(meta.typesizeTable, sizeof(meta.typesizeTable));
		dataSrc->readBytes(meta.res);
		
		readBranch( dataSrc, meta, root );

		return true;	
	}
	return false;
}
//-----------------------------------------------------------------------------


}} // namespace 