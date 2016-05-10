
#include "PointoolsVortexAPIInternal.h"
#include <ptedit/cloudSelect.h>


using namespace pointsengine;

namespace ptedit
{
	struct SelCloud : public PointsVisitor
	{
		SelCloud( int64_t g ) : guid(g), res(false) {}


		bool cloud(pcloud::PointCloud *c)	
		{ 
			if (c->guid() == guid)
			{
				c->root()->flag( pcloud::WholeSelected, true, true );
				c->root()->flag( pcloud::PartSelected, false, true );
				res = true;
			}
			return true;
		}
		
		bool visitNode(const pcloud::Node *n)
		{
			if (n->isLeaf() && n->flag(pcloud::WholeSelected) && !n->flag(pcloud::PartHidden))
			{
				pcloud::Voxel *v = const_cast<pcloud::Voxel*>(static_cast<const pcloud::Voxel*>(n));
				v->destroyEditChannel();
			}
			return true;
		}
		bool res;
		int64_t guid;
	};

CloudSelect::CloudSelect() : EditNodeDef("CloudSelect") { cloudGuid = 0; };

bool CloudSelect::apply()
{
	if (!cloudGuid) return false;
	SelCloud sel( cloudGuid );
	thePointsScene().visitPointClouds( &sel );
	return sel.res;
}
bool CloudSelect::writeState( pt::datatree::Branch *b) const
{
	return b->addNode("cloud", cloudGuid);
}
bool CloudSelect::readState(const pt::datatree::Branch *b)
{
	return b->getNode("cloud", cloudGuid);
}
void CloudSelect::setCloud( const pcloud::PointCloud *pc ) { cloudGuid = pc ? pc->guid() : 0; }

}
// end namespace ptedit