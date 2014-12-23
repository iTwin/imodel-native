#include <ptengine/queryDensity.h>
#include <ptengine/voxelLoader.h>

using namespace pointsengine;
using namespace pcloud;
//-----------------------------------------------------------------------------
void	CurrentDensity::preQuery( pcloud::Voxel*v )
{
	//nothing to do
}
float	CurrentDensity::lodAmount( pcloud::Voxel*v )
{
	return v->getCurrentLOD();
}
void	CurrentDensity::postQuery( pcloud::Voxel *v )
{
	//nothing to do
}
//-----------------------------------------------------------------------------
FullDensity::FullDensity(bool geometry_only) : loader(0), geom_only(geometry_only) 
{}
FullDensity::~FullDensity()
{
	if (loader) delete loader;
}
void	FullDensity::preQuery( pcloud::Voxel*v )
{
	loader = new VoxelLoader(v, 1.0f, false, false, true, 1, geom_only);
}
float	FullDensity::lodAmount( pcloud::Voxel*v )
{
	return v->getCurrentLOD();
}
void	FullDensity::postQuery( pcloud::Voxel *v )
{
	delete loader;
	loader = 0;
}
//-----------------------------------------------------------------------------
ProportionalDensity::ProportionalDensity(float densityCoeff,bool geometry_only) 
: loader(0), density(densityCoeff), geom_only(geometry_only)
{}
ProportionalDensity::~ProportionalDensity()
{
	if (loader) delete loader;
}
void	ProportionalDensity::preQuery( pcloud::Voxel*v )
{
	loader = new VoxelLoader(v, density, false, false, true, 1, geom_only);
}
float	ProportionalDensity::lodAmount( pcloud::Voxel*v )
{
	return v->getCurrentLOD();
}
void	ProportionalDensity::postQuery( pcloud::Voxel *v )
{
	delete loader;
	loader = 0;
}
//-----------------------------------------------------------------------------
