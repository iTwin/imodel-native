#pragma once

#include <pt/datatreeBranch.h>

namespace pt
{
namespace datatree
{
//
// utility functions
// print nodes, lists tree with printf
struct ListBranchVisitor
{
	int _indent;
	char id[NODE_ID_LENGTH+1];

	ListBranchVisitor()
	{
		_indent = 0;
	}
	static void _printindent(int indent)
	{
		for (int i=0; i<indent*3; i++) printf(" ");
	}
	void printindent()
	{
		_printindent(_indent);
	}
	static void _branchInfo(const Branch *br)
	{
		char id[64];
		br->id().get(id);

		int indent = br->level();
		printf("\n");
		_printindent(indent);
		printf("+ %s [%d %d %d]\n", 
			id, (int)br->flags(0), (int)br->flags(1), (int)br->flags(2));

		_printindent(indent); 

		if (br->numNodes())		printf("|\n");
		else					printf("|_");

		int num_blobs = (int)br->numBlobs();

		/* list blobs */ 
		if (num_blobs)
		{
			_printindent(indent);
			printf("* %d BLOBS\n", num_blobs);

			++indent;

			for (int b=0; b<num_blobs; b++)
			{
				NodeID id;
				const Blob *blob = br->getBlob(b, id);

				_printindent(indent);

				printf("|_ # %s : %d bytes\n", id.get(), blob->_size);

			};

			--indent;
		}
	}
	bool operator()(const Branch *br)
	{
		_indent = br->level();
		_branchInfo(br);
		return true;
	}
	static void _nodeInfo(const NodeID &nodeid, Node *n, Branch *owner)
	{
		char id[64];
		nodeid.get(id);
		_printindent(owner->level());
		printf("|_ %s\t| ", id);

		ListBranchVisitor V;
		n->visitType(V);
		printf("\n");			
	}
	void operator ()(const NodeID &nodeid, Node*n) 
	{
		nodeid.get(id);
		printindent();
		printf("|_ %s\t| ", id);

		ListBranchVisitor V;
		n->visitType(V);
		printf("\n");
	}
	void operator ()(const float &v)
	{
		printf("%f", v);
	}
	void operator ()(const double &v)
	{
		printf("%f", v);
	}
	void operator()(const int &v)
	{
		printf("%d", v);
	}
	void operator()(const bool &v)
	{
		printf((v ? "true" : "false"));
	}
	void operator()(const unsigned int &v)
	{
		printf("%d", v);
	}
	void operator()(const int64_t &v)
	{
		printf("%d", (int)v);
	}
	void operator()(const uint64_t &v)
	{
		printf("%d", (int)v);
	}
	void operator()(const vector3i &v)
	{
		printf("<%d, %d, %d>", v.x, v.y, v.z);
	}
	void operator()(const vector3 &v)
	{
		printf("<%ff, %ff, %ff>", v.x, v.y, v.z);
	}
	void operator()(const vector3d &v)
	{
		printf("<%f, %f, %f>", v.x, v.y, v.z);
	}
	void operator()(const pt::String &v)
	{
		if (v.length())
			printf( "%s", v.c_str() );
	}
};	
}
}