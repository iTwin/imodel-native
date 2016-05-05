#ifndef POINTOOLS_BRANCH_HANDLER_INC
#define POINTOOLS_BRANCH_HANDLER_INC

#include <pt/datatree.h>


#ifdef PTL_EXPORTS
#define PTL_API EXPORT_ATTRIBUTE
#else
	#ifdef POINTOOLS_API_INCLUDE
		#define PTL_API 
	#else
		#define PTL_API IMPORT_ATTRIBUTE
	#endif
#endif

namespace ptl
{
	class BranchHandler
	{
	public:
		typedef std::function<bool(pt::datatree::Branch*)>			write_cb;
		typedef std::function<bool(const pt::datatree::Branch*)>	read_cb;

		PTL_API BranchHandler(pt::datatree::NodeID id, read_cb rcb, write_cb wcb, bool configuration = false);
		PTL_API ~BranchHandler();

		pt::datatree::NodeID	identifier;

		write_cb	writebranch;
		read_cb		readbranch;
		bool		configuration;
		char		descriptor[32];
	};
};

#endif