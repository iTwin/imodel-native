#include <pt_edit/bitvoxelGrid.h>

	/* fill the voxel grid */ 
	struct VoxelGridFillVisitor : public pcloud::Node::Visitor
	{
		VoxelGridFillVisitor(pt::BitVoxelGrid &g) : grid(g)
		{
			g.getBoundingBox(box);
		}
		bool visitNode( const pcloud::Node *node )
		{
			if (node->flag(pcloud::WholeVisible) || node->flag(pcloud::PartVisible))
			{
				/* check for intersection */ 
				return true;
			}
			else return false;
		}
		inline void point(const pt::vector3d &pnt, uint index, ubyte &f) 
		{ 
			grid.set( pnt.x, pnt.y, pnt.z );
		}
		inline void mt_point(int t, const pt::vector3d &pnt, uint index, ubyte &f)
		{
			grid.set( pnt.x, pnt.y, pnt.z );
		}
		
		pcloud::Voxel *_currentVoxel;

		pt::BoundingBox box;
		pt::BitVoxelGrid &grid;
	};

	/* fill the voxel grid */ 
	struct VoxelGridFillANDVisitor : public pcloud::Node::Visitor
	{
		VoxelGridFillANDVisitor(pt::BitVoxelGrid &g) : grid(g)
		{
			g.getBoundingBox(box);
		}
		bool visitNode( const pcloud::Node *node )
		{
			if (node->flag(pcloud::WholeVisible) || node->flag(pcloud::PartVisible))
			{
				/* check for intersection */ 
				return true;
			}
			else return false;
		}
		inline void point(const pt::vector3d &pnt, uint index, ubyte &f) 
		{ 
			if (grid.get(pnt.x, pnt.y, pnt.z))
				grid.set( pnt.x, pnt.y, pnt.z,  );
		}
		inline void mt_point(int t, const pt::vector3d &pnt, uint index, ubyte &f)
		{
			grid.set( pnt.x, pnt.y, pnt.z );
		}
		
		pcloud::Voxel *_currentVoxel;

		pt::BoundingBox box;
		pt::BitVoxelGrid &grid;
	}
