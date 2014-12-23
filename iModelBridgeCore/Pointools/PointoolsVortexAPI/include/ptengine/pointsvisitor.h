#ifndef POINTOOLS_POINTSENGINE_POINTSVISITOR_H
#define POINTOOLS_POINTSENGINE_POINTSVISITOR_H	1

namespace pointsengine
{
	class PointsVisitor : public pcloud::Node::Visitor
	{
	public:
		PointsVisitor(pt::CoordinateSpace cs = pt::ProjectSpace) 
			: currentVoxel(0), currentCloud(0), currentScene(0), cspace(cs) {}

		virtual ~PointsVisitor() {}
		virtual bool scene(pcloud::Scene *sc)			{ return true; }
		virtual bool cloud(pcloud::PointCloud *cloud)	{ return true; }
		virtual bool voxel(pcloud::Voxel *vox)			{ return true; }
		virtual void point(pt::vector3 &pnt)			{};

		/* used for visitNodes of PointsScene only */ 
		virtual bool visitNode( const pcloud::Node* node )	{ return false; }

		pcloud::Voxel		*currentVoxel;
		pcloud::PointCloud	*currentCloud;
		pcloud::Scene		*currentScene;
		pt::CoordinateSpace  cspace;

		pt::BoundingBoxD	objCsBounds;
	};
}

#endif