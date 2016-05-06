#pragma once
#include <ptcloud2/pointCloud.h>
#include <ptcloud2/scene.h>
#include <ptengine/pointsvisitor.h>
#include <ptengine/queryDensity.h>
#include <mutex>

namespace pointsengine
{
	class QueryTraversal
	{
	public:

		QueryTraversal(	pt::CoordinateSpace cs=pt::ProjectSpace ) 
			: m_cloudScope(0), m_sceneScope(0), m_cs(cs)
		{}

		QueryTraversal( pcloud::PointCloud *cloud, pt::CoordinateSpace cs=pt::ProjectSpace )
			: m_cloudScope(cloud), m_sceneScope(0), m_cs(cs)
		{}

		QueryTraversal( pcloud::Scene *scene, pt::CoordinateSpace cs=pt::ProjectSpace )
			: m_cloudScope(0), m_sceneScope(scene), m_cs(cs)
		{}

		virtual ~QueryTraversal()
		{}
 
		void setScope( pcloud::PointCloud *cloud )
		{
			m_cloudScope = cloud;
			m_sceneScope = 0;
		}
		void setScope( pcloud::Scene *scene )
		{
			m_cloudScope = 0;
			m_sceneScope = scene;
		}
		void resetScope( void )
		{
			m_cloudScope = 0;
			m_sceneScope = 0;
		}

		template <class Receiver, class DensityPolicy>
		void traverseVoxel( pcloud::Voxel *voxel, Receiver &R, DensityPolicy &D )
		{
            std::unique_lock<std::mutex> lock(voxel->mutex(), std::try_to_lock);
			if (!lock.owns_lock()) return; //pause engine to avoid this

			D.preQuery( voxel );
			float a = D.lodAmount(voxel);
			voxel->iterateTransformedPoints( R, m_cs, static_cast<ubyte>(a) );
			D.postQuery( voxel );
		}

	protected:
		pcloud::PointCloud	*m_cloudScope;
		pcloud::Scene		*m_sceneScope;
		pt::CoordinateSpace m_cs;

	};
	class ConstQueryTraversal
	{
	public:

		ConstQueryTraversal(	pt::CoordinateSpace cs=pt::ProjectSpace ) 
			: m_cloudScope(0), m_sceneScope(0), m_cs(cs)
		{}

		ConstQueryTraversal( const pcloud::PointCloud *cloud, pt::CoordinateSpace cs=pt::ProjectSpace )
			: m_cloudScope(cloud), m_sceneScope(0), m_cs(cs)
		{}

		ConstQueryTraversal( const pcloud::Scene *scene, pt::CoordinateSpace cs=pt::ProjectSpace )
			: m_cloudScope(0), m_sceneScope(scene), m_cs(cs)
		{}

		virtual ~ConstQueryTraversal()
		{}
 
		void setScope( const pcloud::PointCloud *cloud )
		{
			m_cloudScope = cloud;
			m_sceneScope = 0;
		}
		void setScope( const pcloud::Scene *scene )
		{
			m_cloudScope = 0;
			m_sceneScope = scene;
		}
		void resetScope( void )
		{
			m_cloudScope = 0;
			m_sceneScope = 0;
		}

		template <class Receiver, class DensityPolicy>
		void traverseVoxel( const pcloud::Voxel *voxel, Receiver &R, DensityPolicy &D )
		{
			pcloud::Voxel* non_const_voxel = const_cast<pcloud::Voxel*>(voxel);

            std::unique_lock<std::mutex> lock(non_const_voxel->mutex(), std::try_to_lock);
			if (!lock.owns_lock()) return; //pause engine to avoid this

			D.preQuery( non_const_voxel );
			non_const_voxel->iterateTransformedPoints( R, m_cs, D.lodAmount(non_const_voxel) );
			D.postQuery( non_const_voxel );
		}

	protected:
		const pcloud::PointCloud	*m_cloudScope;
		const pcloud::Scene			*m_sceneScope;
		pt::CoordinateSpace m_cs;

	};
}