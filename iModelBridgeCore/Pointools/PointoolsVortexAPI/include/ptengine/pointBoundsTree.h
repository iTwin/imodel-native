#pragma once

#include <ptengine/ptengine_api.h>
#include <ptcloud2/scene.h>
#include <pt/boundsTree.h>
#include <fastdelegate/fastdelegate.h>
#include <ptengine/queryFilter.h>
#include <ptengine/queryPointsArray.h>

namespace pointsengine
{
	class PointsBoundsTree : public pt::OrientedBoxBoundsTreed
	{
	public:
        typedef fastdelegate::FastDelegate1<float, bool>	ProgressFeedback;
		typedef pt::BoundsTreeNode<pt::OBBoxd>	TreeNode;

		PTENGINE_API static PointsBoundsTree *createFromScene( 
			pcloud::Scene *scene, 
			int target_pnts_per_box, 
			double min_dim, 
			double max_dim, 
			ProgressFeedback feedback=0 );

		PTENGINE_API static PointsBoundsTree *createFromPointCloud( 
			pcloud::PointCloud *cloud, 
			int target_pnts_per_box, 
			double min_dim, 
			double max_dim, 
			ProgressFeedback feedback=0	);

		PTENGINE_API static PointsBoundsTree *createPreliminaryFromPointCloud( 
			pcloud::PointCloud *cloud, 
			ProgressFeedback feedback=0	);

		PTENGINE_API static bool extendTree( 
			PointsBoundsTree *tree, 
			pcloud::PointCloud *cloud, const pt::OBBoxd &region,
			int target_pnts_per_box, 
			double min_dim, 
			double max_dim,
			ProgressFeedback feedback = 0);

		PTENGINE_API static PointsBoundsTree *createFromSelected( 
			int target_pnts_per_box, 
			double min_dim, 
			double max_dim, 
			ProgressFeedback feedback=0 );

		PTENGINE_API static PointsBoundsTree *createFromProject( 
			int target_pnts_per_box, 
			double min_dim, 
			double max_dim, 
			ProgressFeedback feedback=0 );

		PTENGINE_API void drawLeaves();
		PTENGINE_API void drawNodes(int level);
		PTENGINE_API void drawTree();

	private:
		PointsBoundsTree();

		//bool	buildTree( TreeNode *node, 
		//			int target_pts, float min_dim, float max_dim,
		//			pcloud::PointCloud*cloud, pcloud::Scene *scene,
		//			ProgressFeedback feedback );

		//bool	buildTree( TreeNode *node, std::vector<pt::vector3> &pnts,
		//			int target_pts, float min_dim, float max_dim,
		//			pcloud::PointCloud*cloud, pcloud::Scene *scene,
		//			ProgressFeedback feedback );

		bool	buildTree( TreeNode *_node, bool preliminary, int target_pts, double min_dim, double max_dim,
							 pcloud::PointCloud *cloud, pcloud::Scene *scene, ProgressFeedback feedback );

		bool	buildTreeDetail( TreeNode *_node, TreeNode *_parent,  
									QueryPointsArrayd &pts, float proportionFiltered, 
									bool prelim, int target_pts, double min_dim, double max_dim,
									pcloud::PointCloud *cloud, pcloud::Scene *scene,
									ProgressFeedback feedback );

		void	cullLargeLeaves( TreeNode *node, double  max_dim );

		void	drawBox( const pt::OBBoxd &box );

		int		getBuildTargetPntsPerBox() const;
		int		getBuildMinBoxDim() const;
		int		getBuildMaxBoxDim() const;


		struct SplitData
		{
			SplitData(TreeNode	*_node, TreeNode *_parent, QueryPointsArrayd *_pnts )
				: node(_node), parent(_parent), pnts(_pnts)
			{}

			TreeNode *node;
			TreeNode *parent;
			QueryPointsArrayd *pnts;
		};

		int64_t	pointsProcessed;
		int64_t totalPoints;
		int64_t lastProgress;

		int		m_targetPntsPerBox;
		double 	m_minDim;
		double 	m_maxDim;
	};

	// use as a query filter to filter in point that are contained in leaves
	struct FilterInPolicy
	{
		template <class PointType>
		inline static bool point(const PointsBoundsTree &tree, const PointType &pt )
		{
			return (tree.root()->containedInLeaf(pt));
		}
		inline static FilterResult node(const PointsBoundsTree &tree,  const pcloud::Node *n )
		{
			pt::BoundingBoxD bb;
			n->getBoundsD(bb);
			return (tree.root()->intersectsLeaf( bb )) ? FilterPartial : FilterOut;
		}
	};

	struct FilterOutPolicy
	{
		template <class PointType>
		inline static bool point(const PointsBoundsTree &tree, const PointType &pt )
		{
			return !(tree.root()->containedInLeaf(pt));
		}
		inline static FilterResult node(const PointsBoundsTree &tree,  const pcloud::Node *n )
		{
			pt::BoundingBoxD bb;
			n->getBoundsD(bb);
			return (tree.root()->intersectsLeaf( bb )) ? FilterOut : FilterPartial;
		}
	};

	template<class Receiver, class FilterPolicy=FilterInPolicy, class TreeType=PointsBoundsTree>
	struct PointBoundsTreeFilter
	{
		PointBoundsTreeFilter(const TreeType &tree) 
			: m_tree(tree), m_rec(Receiver()) 
		{}

		PointBoundsTreeFilter(const TreeType &tree, Receiver &r)
			:m_tree(tree), m_rec(r)
		{}

		inline FilterResult node( const pcloud::Node *n )			
		{	
			return FilterPolicy::node(m_tree, n);
		} 
		inline FilterResult cloud( const pcloud::PointCloud *pc )	
		{	
			return FilterIn;	
		}
		inline FilterResult scene( const pcloud::Scene *sc )			
		{	
			return FilterIn;	
		} 
		template <class PointType>
		inline void point( const PointType &pt, int &index, ubyte &layers)
		{
			if (FilterPolicy::point(m_tree, pt))
				m_rec.point( pt, index, layers );
		}
		const TreeType			&m_tree;
		Receiver				&m_rec;
	};
}