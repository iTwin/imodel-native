#pragma once
#include <ptcloud2/voxel.h>
#include <ptengine/ptengine_api.h>

namespace pointsengine
{
	class VoxelLoader;

	struct PTENGINE_API CurrentDensity	
	{	
		void	preQuery( pcloud::Voxel*v );
		float	lodAmount( pcloud::Voxel*v );
		void	postQuery( pcloud::Voxel *v );
	};

	struct PTENGINE_API FullDensity
	{
		FullDensity(bool geometry_only=false);
		~FullDensity();

		void	preQuery( pcloud::Voxel*v );
		float	lodAmount( pcloud::Voxel*v );
		void	postQuery( pcloud::Voxel *v );

		VoxelLoader *loader;
		bool		geom_only;
	};

	struct PTENGINE_API ProportionalDensity
	{
		ProportionalDensity(float densityCoeff=1.0f, bool geometry_only=false);
		~ProportionalDensity();

		void	preQuery( pcloud::Voxel*v );
		float	lodAmount( pcloud::Voxel*v );
		void	postQuery( pcloud::Voxel *v );

	private:
		VoxelLoader		*loader;
		float			density;
		bool			geom_only;

	};
}
	