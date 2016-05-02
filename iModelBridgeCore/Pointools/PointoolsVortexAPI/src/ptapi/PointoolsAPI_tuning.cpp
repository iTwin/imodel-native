#include "PointoolsVortexAPIInternal.h"
#include <pt/os.h>
#include <commdlg.h>
#define POINTOOLS_API_BUILD_DLL

#include <ptapi/PointoolsVortexAPI.h>

#include <diagnostics/diagnosticCmds.h>
#include <ptengine/PointsScene.h>
#include <ptengine/PointsPager.h>
#include <ptengine/RenderEngine.h>
#include <ptengine/engine.h>

#include <ptcloud2/pod.h>
#include <pt/project.h>

#include <ptl/branch.h>
#include <ptl/project.h>

extern int setLastErrorCode( int );
using namespace pt;
using namespace pointsengine;
//-----------------------------------------------------------------------------
PTvoid	PTAPI ptSetCacheSizeMb( PTuint mb )
{
	PointsPager::setCacheSizeMb( (int)mb );
}
//-----------------------------------------------------------------------------
PTuint	PTAPI ptGetCacheSizeMb()
{
	return (PTuint)PointsPager::getCacheSizeMb( );
}
//-----------------------------------------------------------------------------
PTvoid	PTAPI ptAutoCacheSize()
{
	PointsPager::useAutoCacheSize();
}
#ifdef HAVE_OPENGL
//-----------------------------------------------------------------------------
PTres	PTAPI ptSetLoadingPriorityBias( PTenum bias )
{
	switch (bias)
	{
		case PT_LOADING_BIAS_SCREEN:
			theVisibilityEngine().setBias( VisibilityEngine::BiasScreen ); break;

		case PT_LOADING_BIAS_NEAR:
			theVisibilityEngine().setBias( VisibilityEngine::BiasNear); break;

		case PT_LOADING_BIAS_FAR:
			theVisibilityEngine().setBias( VisibilityEngine::BiasFar ); break;

		case PT_LOADING_BIAS_POINT:
			theVisibilityEngine().setBias( VisibilityEngine::BiasPoint ); break;
		
		default:
			return setLastErrorCode( PTV_INVALID_OPTION );
	}
	return setLastErrorCode( PTV_SUCCESS );
}
//-----------------------------------------------------------------------------
PTenum	PTAPI ptGetLoadingPriorityBias()
{
	VisibilityEngine::VisibilityBias bias = theVisibilityEngine().getBias();

	switch (bias)
	{
		case VisibilityEngine::BiasScreen:
			return PT_LOADING_BIAS_SCREEN;

		case VisibilityEngine::BiasNear:
			return PT_LOADING_BIAS_NEAR; 

		case VisibilityEngine::BiasFar:
			return PT_LOADING_BIAS_FAR; 

		case VisibilityEngine::BiasPoint:
			return PT_LOADING_BIAS_POINT; 
	}
	return PT_LOADING_BIAS_SCREEN;
}
//-----------------------------------------------------------------------------
PTres	PTAPI ptSetTuningParameterfv( PTenum param, const PTfloat *values )
{
	if ( param != PT_LOADING_BIAS_POINT) 
	{
		return setLastErrorCode( PTV_INVALID_OPTION );
	}

	theVisibilityEngine().setBiasPoint( values );
	return setLastErrorCode( PTV_SUCCESS );
}
//-----------------------------------------------------------------------------
PTres	PTAPI ptGetTuningParameterfv( PTenum param, PTfloat *values )
{
	if ( param != PT_LOADING_BIAS_POINT) 
	{
		return setLastErrorCode( PTV_INVALID_OPTION );;
	}

	vector3 pnt ( theVisibilityEngine().getBiasPoint() );
	pnt.get(values);

	return setLastErrorCode( PTV_SUCCESS );;
}
#endif

extern pcloud::Scene *		sceneFromHandle(PThandle handle);
extern pcloud::PointCloud*	cloudFromHandle(PThandle cloud);
extern PTdouble					g_unitScale;

//-----------------------------------------------------------------------------
PTres	PTAPI _ptDiagnostic( PTvoid *data )
{
	DiagnosticData *d=reinterpret_cast<DiagnosticData*>(data);

	if (d)
	{
		assert( d->size == sizeof( DiagnosticData ) );

		switch(d->in_cmd)
		{
		case GetVoxelData:
			// expects a point cloud handle and point to specify voxel
			{
				if (d->in_handles[0])
				{
					// find pod
					pcloud::Scene *sc = sceneFromHandle( d->in_handles[0] );

					if (sc)
					{
						pcloud::PointCloud *cloud = cloudFromHandle( d->in_handles[1] );

						if (!cloud)
						{
							cloud = sc->cloud(0);
						}
						
						pt::vector3d basePoint = pt::vector3d(pt::Project3D::project().registration().matrix()(3,0), 
								pt::Project3D::project().registration().matrix()(3,1), 
								pt::Project3D::project().registration().matrix()(3,2));

						d->in_pnts[0] /= g_unitScale;
						//d->in_pnts[0] -= basePoint;

						pcloud::Voxel *voxel = const_cast<pcloud::Voxel*>(cloud->findContainingVoxel( d->in_pnts[0], pt::ProjectSpace ));

						memset( &d->voxel_data, 0, sizeof(VoxelData));

						if (voxel)
						{
							voxel->flag( pcloud::DebugShowPurple, true );
							d->voxel_data.id = voxel->indexInCloud();
							d->voxel_data.editChannel = voxel->channel( pcloud::PCloud_Filter ) ? true : false;
							d->voxel_data.editPoint = voxel->numPointsEdited();
							d->voxel_data.fullLayers = voxel->layers(0);
							d->voxel_data.partLayers = voxel->layers(1);
							d->voxel_data.wholeSelected = voxel->flag( pcloud::WholeSelected );
							d->voxel_data.partSelected = voxel->flag( pcloud::PartSelected );
							d->voxel_data.lod = voxel->getRequestLOD();
						}
					}
				}
			}
		}
		
		return PTV_SUCCESS;
	}
	else return PTV_INVALID_PARAMETER;
}