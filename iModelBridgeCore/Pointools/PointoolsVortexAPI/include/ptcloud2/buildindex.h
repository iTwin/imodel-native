/*----------------------------------------------------------*/ 
/* BuildIndex.h												*/ 
/* Point Cloud Index builder Interface file					*/ 
/*----------------------------------------------------------*/ 
/* (c) Copyright Pointools 2004								*/   
/*----------------------------------------------------------*/ 
/* Written by Faraz Ravi									*/ 
/*----------------------------------------------------------*/ 
#ifndef POINTCLOUD_BUILDINDEX
#define POINTCLOUD_BUILDINDEX 1

#include <vector>

#include <ptcloud2/pcloud.h>
#include <ptcloud2/voxel.h>
#include <ptcloud2/datachannel.h>

namespace pcloud
{
	struct PCLOUD_API BuildIndex
	{
		class ChannelData
		{
		public:
			ChannelData(Channel ch, void*d, int n) : data(d), count(n), channel(ch) {}
			void * data;
			int count;
			Channel channel;
		};
		static int &maxPointsPerLeaf();
		static int &minDepth();
		static DataType &geomStoreType();

		static Node* buildUniformPointsTree(std::vector<ChannelData*> &cd, std::vector<Voxel*> &voxels);
		static void calcBoundingBox(pt::vector3 *points, int count, pt::BoundingBox &bb);

	private:
	};
}
#endif