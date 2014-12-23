#pragma once

#define DATATREE_VERSION0	1
#define DATATREE_VERSION1	1
#define DATATREE_VERSION2	2
#define DATATREE_VERSION3	0

#define DATATREE_USEWCHAR	0

// NEVER change the following constants
#if (DATATREE_USEWCHAR==1)
#define NODEID_CHARTYPE		wchar_t
#else
#define NODEID_CHARTYPE		char
#endif

#include <pt/datatreeNode.h>

namespace pt
{
namespace datatree
{
//! Meta
//! Header data
struct Meta
{
	uint8	version[4];

	uint32	sizeof_meta;

	uint8	sizeof_nodeid;

	uint8	iswchar;

	uint8	res[2];

	uint8	res_[256];

	static void makeMeta(Meta &meta)
	{
		meta.version[0] = DATATREE_VERSION0;
		meta.version[1] = DATATREE_VERSION1;
		meta.version[2] = DATATREE_VERSION2;
		meta.version[3] = DATATREE_VERSION3;
		meta.sizeof_meta = sizeof(Meta);

		meta.sizeof_nodeid = NODE_ID_SIZE;
		meta.iswchar = DATATREE_USEWCHAR;
		memset(meta.res,0,2);
		memset(meta.res,0,256);
	}
};
}	// namespace datatree
}	// namespace pt