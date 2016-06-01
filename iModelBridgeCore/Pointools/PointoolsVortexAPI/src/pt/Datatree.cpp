#include "PointoolsVortexAPIInternal.h"
#include <pt/datatree.h>
#include <pt/datatreePrintf.h>

namespace pt
{
namespace datatree
{
//-----------------------------------------------------------------------------
void DataTree::root()
{
	while (_branchstack.size()) _branchstack.pop();
}
//-----------------------------------------------------------------------------
int DataTree::pushBranch( const NodeID &nid )
{
	Branch *parent = _branchstack.size() ? _branchstack.top() : this;
	_branchstack.push(parent->getBranch(nid, !_readonly));

	return (int)_branchstack.size();
}
//-----------------------------------------------------------------------------
int DataTree::popBranch()
{
	if (_branchstack.size()) _branchstack.pop();
	return (int)_branchstack.size();
}
//-----------------------------------------------------------------------------
Branch* DataTree::currentBranch()
{
	if (_branchstack.size())
		return _branchstack.top();
	else return this;
}
//-----------------------------------------------------------------------------
bool DataTree::addBranch( const Branch *br )
{
	if (_branchstack.size())
		return _branchstack.top()->addBranchCopy(br);
	else return Branch::addBranchCopy(br);
}
//-----------------------------------------------------------------------------
#ifndef DATATREE_NO_PARAMMAP
bool DataTree::addNodes( const ParameterMap &pm )
{
	if (_branchstack.size())
		return _branchstack.top()->addNodes(pm);
	else return Branch::addNodes(pm);	
	return false;
}
#endif
//-----------------------------------------------------------------------------
// write node
//-----------------------------------------------------------------------------
struct WriteValueVisitor
{
	void operator ()(const String &s)
	{
		uint32 size = s.length()+1;

		fwrite(&size, 4, 1, _file);

		if (size > 1)
		{
			fwrite(s.c_wstr(), size *  sizeof(wchar_t), 1, _file);
		}
	}
	template <class T> void operator ()(const T &t)
	{
		fwrite(&t, sizeof(T), 1, _file);
	}
	FILE *_file;
	Meta _meta;
};
//-----------------------------------------------------------------------------
struct NodeWriteVisitor
{
	void operator() (const NodeID &nid, const Node *node)
	{	
		uint8 type_index = (uint8)node->typeId();

		/*identifier	*/ 
		fwrite(&nid, NODE_ID_SIZE, 1, _file); 

		/* type index	*/ 
		fwrite(&type_index, 1, 1, _file);

		/* parameter	*/ 
		WriteValueVisitor wv;
		wv._file = _file;
		node->visitType(wv);
	};
	bool operator () (Branch *b)
	{
		/* type index	*/ 
		uint32 num_subbranches = (uint32)b->numBranches();
		uint32 num_nodes = (uint32)b->numNodes();
		uint32 num_blobs = (uint32)b->numBlobs();

		fwrite(&b->id(), NODE_ID_SIZE, 1, _file);
		fwrite(&num_subbranches, 4, 1, _file);
		fwrite(&num_nodes, 4, 1, _file);
		fwrite(&num_blobs, 4, 1, _file);

		uint8 v[] = { b->level(), b->flags(0),b->flags(1),b->flags(2) };
		fwrite(&v, 1, 4, _file);

		/*write blobs*/ 
		for (int i=0; i<b->numBlobs(); i++)
		{
			NodeID id;
			const Blob *blob = b->getBlob(i, id);
			if (blob)
			{
				fwrite(&id, NODE_ID_SIZE, 1, _file);
				fwrite(&blob->_size, 4, 1, _file);
				fwrite(blob->_data, blob->_size, 1, _file);
			}
		}
		return true;
	}
	FILE *_file;
};
//-----------------------------------------------------------------------------
bool DataTree::writeTree( const String &path )
{
	Meta m;
	Meta::makeMeta(m);

	NodeWriteVisitor wv;
    wv._file = fopen(path.c_str(), "wb");
	if (NULL == wv._file)
		return false;

	char id[16] = "dtree1.1";
	fwrite(id, 8, 1, wv._file);

	if (!wv._file) return false;

	/*write the identifier*/ 

	fwrite(&m, m.sizeof_meta, 1, wv._file);
	visitNodes(wv);

	fclose(wv._file);
	return true;
}
//-----------------------------------------------------------------------------
bool DataTree::isDTreeFile( const String &filepath )
{
	/*read tree from file*/ 
	FILE * file = fopen(filepath.c_str(), "rb");
	if(NULL == file)
        return false;

	/*is this a valid dtree file*/ 
	char id[9];
	fread(id, 8, 1, file);

	bool isdtree = memcmp(id, "dtree", 5) == 0;

	fclose(file);
	return isdtree;
}
//-----------------------------------------------------------------------------
// read from file
//-----------------------------------------------------------------------------
struct Reader
{
	Reader(const Meta &m) : meta(m)		{}

	void readTree(FILE *file, Branch *root)
	{
		readBranch(file, root);
	}

	/*read functions*/ 
	void	readBranch(FILE *file, Branch *root, Branch *parent=0);
	void	readNode(FILE *file, Branch *owner);

	const	Meta &meta;
};
//-----------------------------------------------------------------------------
bool DataTree::readTree( const String &filepath )
{
	/*read tree from file*/ 
    FILE * file = fopen(filepath.c_str(), "rb");
    if (NULL == file)
        return false;

	/*is this a valid dtree file*/ 
	char id[9];
	fread(id, 8, 1, file);
	if (memcmp(id, "dtree", 5) != 0)
	{
		fclose(file);
		return false;
	}
	/* note that this does not handle	*/ 
	/* differing size Meta				*/ 
	Meta m;
	fread(&m, sizeof(Meta), 1, file);

	Reader r(m);
	r.readBranch(file, this);

	/*read root*/ 
	fclose(file);

	return true;
}
//-----------------------------------------------------------------------------
DataTree::DataTree( const String &path ) : Branch("Root")
{
	readTree(path);
}


void Reader::readBranch( FILE *file, Branch *root, Branch *parent/*=0*/ )
{
	NodeID nodeid;
	uint32 num_subbranches;
	uint32 num_nodes;
	uint32 num_blobs;
	uint8 level;
	uint8 flags[4];

	// read the meta data
	fread(&nodeid, NODE_ID_SIZE, 1, file);
	fread(&num_subbranches, 4, 1, file);
	fread(&num_nodes, 4, 1, file);
	fread(&num_blobs, 4, 1, file);
	fread(&level, 1, 1, file);
	fread(&flags, 3, 1, file);

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
		branch = parent->getBranch(nodeid, true);
	}
	unsigned int i;

	// copy flags
	memcpy(branch->_flags, flags, 3);

	// read blobs 
	NodeID nid;
	uint32	blob_size;
	uint8*	blob_data;

	for (i=0; i<num_blobs; i++)
	{
		fread(&nid, NODE_ID_SIZE, 1, file);
		fread(&blob_size, 4, 1, file);

		blob_data = new uint8[blob_size];
		fread(blob_data, blob_size, 1, file);

		branch->addBlob(nid, blob_size, blob_data, false, true);
	}

#ifdef DATATREE_DEBUGGING
	char d_id[64];
	nodeid.get(d_id);
	for (int ind=0; ind < branch->_level; ind++) printf("    ");
	ListBranchVisitor::_branchInfo(branch);
	printf("\n");
#endif
	// read nodes
	for (i=0;i<num_nodes;i++)
	{
		readNode(file, branch);
	}

	// continue recursively with branches
	for (i=0; i<num_subbranches; i++)
		readBranch(file, root, branch);
}

void Reader::readNode( FILE *file, Branch *owner )
{
	NodeID nodeid;
	uint8 type_index;

	// meta
	fread(&nodeid, NODE_ID_SIZE, 1, file);
	fread(&type_index, 1, 1, file);

	// value
	switch(type_index)
	{
	case 0: { int v; fread(&v, sizeof(int), 1, file); owner->addNode(nodeid, v); } break;
	case 1: { bool v; fread(&v, sizeof(bool), 1, file); owner->addNode(nodeid, v); } break;
	case 2: { float v; fread(&v, sizeof(float), 1, file); owner->addNode(nodeid, v); } break;
	case 3: { double v; fread(&v, sizeof(double), 1, file); owner->addNode(nodeid, v); } break;
	case 4: { uint32 v; fread(&v, sizeof(uint32), 1, file); owner->addNode(nodeid, v); } break;
	case 5: { int64_t v; fread(&v, sizeof(int64_t), 1, file); owner->addNode(nodeid, v); } break;
	case 6: { vector3i v; fread(&v, sizeof(int)*3, 1, file); owner->addNode(nodeid, v); } break;
	case 7: { vector3 v; fread(&v, sizeof(float)*3, 1, file); owner->addNode(nodeid, v); } break; 
	case 8: { vector3d v; fread(&v, sizeof(double)*3, 1, file); owner->addNode(nodeid, v); } break;
	case 9: { 
		pt::String v;
		uint32 strlength;
		fread(&strlength, 4, 1, file); 

		if (strlength > 1  && strlength < 1024)
		{
			if (meta.version[2] >= 2) /* wchar */ 
			{
				wchar_t buff[1024];
				fread(buff, strlength*sizeof(wchar_t), 1, file); 
				String str(buff);
				owner->addNode(nodeid, str); 
			}
			else
			{
				char buff[1024];
				fread(buff, strlength, 1, file); 
				String str(buff);
				owner->addNode(nodeid, str); 
			}
		}
		else
		{
			owner->addNode(nodeid, pt::String("[empty]"));
		}
			} 
			break;
	} /* switch */ 
#ifdef DATATREE_DEBUGGING
	Node* nd = const_cast<Node*>(owner->getNode(nodeid));
	ListBranchVisitor::_nodeInfo(nodeid, nd, owner);
#endif
}

} // namespace datatree
} // namespace pt