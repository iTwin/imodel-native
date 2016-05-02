#include "PointoolsVortexAPIInternal.h"

#define POINTOOLS_API_BUILD_DLL
#include <gl/glew.h>

#ifdef _DEBUG
#define FILE_TRACE	1
#endif
#include <pt/trace.h>

#include <ptapi/PointoolsVortexAPI.h>
#include <ptapi/PointoolsAPI_handle.h>

#include <ptengine/PointsScene.h>
#include <ptengine/PointsPager.h>
#include <ptengine/RenderEngine.h>
#include <ptengine/RenderSettings.h>
#include <ptengine/PointsExchanger.h>
#include <ptengine/VisibilityEngine.h>
#include <ptengine/PointLayers.h>
#include <ptengine/engine.h>

#include <ptcloud2/pod.h>
#include <pt/project.h>
#include <pt/datatreeIO.h>

#include <ptds/DataSourceMemBlock.h>

#include <ptgl/gltext.h>
#include <ptgl/Color.h>

#include <ptedit/EditManager.h>

#include <pt/memrw.h>
#include <list>

using namespace pt;
using namespace pcloud;
using namespace pointsengine;

extern pt::ViewParams *g_currentViewParams;

using namespace ptedit;
using namespace pcloud;
//	
extern PTvoid	_ptMakeVPContextCurrent();
extern PTvoid	_ptRestoreContext();
extern PTvoid	_ptAdjustMouseYVal( int &y );
extern int		setLastErrorCode( int );

extern Scene *		sceneFromHandle(PThandle handle);
extern PointCloud*	cloudFromHandle(PThandle cloud);

extern double g_unitScale;
//-----------------------------------------------------------------------------
PTres	PTAPI ptSetSelectPointsMode( PTenum select_mode )
{
	PTTRACE_FUNC_P1( select_mode )

	switch(select_mode)
	{
	case PT_EDIT_MODE_SELECT:
		PointEditManager::instance()->selectMode();
		break;

	case PT_EDIT_MODE_UNSELECT:
		PointEditManager::instance()->deselectMode();
		break;

	case PT_EDIT_MODE_UNHIDE:
		PointEditManager::instance()->unhideMode();
		break;

	default:
		return setLastErrorCode( PTV_INVALID_OPTION );
	}

	return setLastErrorCode( PTV_SUCCESS );
}
//-----------------------------------------------------------------------------
PTenum	PTAPI ptGetSelectPointsMode( void )
{
	PTTRACE_FUNC

	switch(PointEditManager::instance()->editMode())
	{
	case SelectPoint:	return PT_EDIT_MODE_SELECT;
	case DeselectPoint: return PT_EDIT_MODE_SELECT;
	case UnhidePoint:	return PT_EDIT_MODE_SELECT;
	}
	return 0;
}
//-----------------------------------------------------------------------------
PTvoid	PTAPI ptSetEditWorkingMode( PTenum mode )
{
	PTTRACE_FUNC_P1( mode )

	switch ( mode )
	{
	case PT_EDIT_WORK_ON_ALL:	
		PointEditManager::instance()->workingMode( EditWorkOnAll );
		PTTRACEOUT << "PT_EDIT_WORK_ON_ALL";
		break;

	case PT_EDIT_WORK_ON_VIEW:
		PointEditManager::instance()->workingMode( EditWorkOnView );
		PTTRACEOUT << "PT_EDIT_WORK_ON_VIEW";
		break;

	case PT_EDIT_WORK_ON_PROPORTION:
		PointEditManager::instance()->workingMode( EditWorkOnProportion );
		PTTRACEOUT << "PT_EDIT_WORK_ON_PROPORTION";
		break;
	}


	return;
}
//-----------------------------------------------------------------------------
PTenum	PTAPI ptGetEditWorkingMode( void )
{
	switch( PointEditManager::instance()->workingMode() )
	{
	case EditWorkOnAll: return PT_EDIT_WORK_ON_ALL;
	case EditWorkOnProportion: return PT_EDIT_WORK_ON_PROPORTION;
	case EditWorkOnView: return PT_EDIT_WORK_ON_VIEW;
	}
	return 0;
}
//-----------------------------------------------------------------------------
PTvoid PTAPI ptSetSelectionDrawColor( const PTubyte *col3 )
{
	PTTRACE_FUNC_P3( col3[0], col3[1], col3[2] )

	RenderSettings::selectionColour( col3 );
}
//-----------------------------------------------------------------------------
PTvoid	PTAPI ptGetSelectionDrawColor( PTubyte *col3 )
{
	const ubyte *col = RenderSettings::selectionColour();
	col3[0] = col[0];
	col3[1] = col[1];
	col3[2] = col[2];
}
//-----------------------------------------------------------------------------
PTvoid	PTAPI ptSelectPointsByRect( PTint x_edge, PTint y_edge, PTint width, PTint height )
{
	PTTRACE_FUNC_P4( x_edge, y_edge, width, height )
	
	_ptMakeVPContextCurrent();

	if (g_currentViewParams)
		PointEditManager::instance()->setViewParams(*g_currentViewParams);

	_ptAdjustMouseYVal(y_edge);
	
	PointEditManager::instance()->setUnits( g_unitScale );
	PointEditManager::instance()->rectangleSelect( x_edge, x_edge+width, y_edge, y_edge - height ); 
	_ptRestoreContext();

	//PointEditManager::instance()->regenOOCComplete();
}
//-----------------------------------------------------------------------------
PTres	PTAPI ptSelectPointsByFence( PTint num_vertices, const PTint *vertices )
{
	PTTRACE_FUNC_P2( num_vertices, vertices )

	PointEditManager::instance()->setUnits( g_unitScale );

	if (!vertices || num_vertices < 3)
	{
		return vertices ? setLastErrorCode( PTV_INVALID_PARAMETER )
			: setLastErrorCode( PTV_VOID_POINTER );
	}
	_ptMakeVPContextCurrent();

	if (g_currentViewParams)
		PointEditManager::instance()->setViewParams(*g_currentViewParams);

	pt::Fence<int> fence;
	for (int i=0; i<num_vertices; i++)
	{
		int y = vertices[i*2+1];
		_ptAdjustMouseYVal(y);

		fence.addPoint(pt::vector2i(vertices[i*2], y));
	}
	PointEditManager::instance()->fenceSelect(fence);
	_ptRestoreContext();

	return setLastErrorCode( PTV_SUCCESS );
}
//-----------------------------------------------------------------------------
PTres	PTAPI ptSelectPointsByBox( const PTdouble *lower, const PTdouble *upper )
{
	PTTRACE_FUNC_P6( lower[0], lower[1], lower[2], upper[0], upper[1], upper[2] )

	if (!lower || !upper)
		return setLastErrorCode( PTV_VOID_POINTER );

	if (g_currentViewParams)
		PointEditManager::instance()->setViewParams(*g_currentViewParams);

	PointEditManager::instance()->boxSelect( lower, upper );

	return setLastErrorCode( PTV_SUCCESS );
}
//-----------------------------------------------------------------------------
PTres	PTAPI ptSelectPointsByOrientedBox( const PTdouble *lower, const PTdouble *upper, const PTdouble *pos, PTdouble *uAxis, PTdouble *vAxis)
{
	PTTRACE_FUNC

	if (!lower || !upper || !pos || !uAxis || !vAxis)
		return setLastErrorCode( PTV_VOID_POINTER );

	if (g_currentViewParams)
		PointEditManager::instance()->setViewParams(*g_currentViewParams);

	PointEditManager::instance()->orientedBoxSelect(lower, upper, pos, uAxis, vAxis );

	return setLastErrorCode( PTV_SUCCESS );
}
//-----------------------------------------------------------------------------
PTres	PTAPI ptSelectPointsByCube( const PTdouble *centre, PTdouble radius )
{
	PTTRACE_FUNC

	if (!centre)
		return setLastErrorCode( PTV_VOID_POINTER );
	
	if (radius < 0) return setLastErrorCode( PTV_INVALID_PARAMETER );

	if (g_currentViewParams)
		PointEditManager::instance()->setViewParams(*g_currentViewParams);

	PointEditManager::instance()->paintSelCube();
	PointEditManager::instance()->paintRadius( radius );
	PointEditManager::instance()->paintSelectAtPoint( centre, true );

	return setLastErrorCode( PTV_SUCCESS );
}
//-----------------------------------------------------------------------------
PTres	PTAPI ptSelectPointsByPlane( const PTdouble *origin, const PTdouble *normal, PTdouble thickness )
{
	PTTRACE_FUNC

	if (!origin || !normal)
	{
		return setLastErrorCode( PTV_VOID_POINTER );
	}

	if (g_currentViewParams)
		PointEditManager::instance()->setViewParams(*g_currentViewParams);

	PointEditManager::instance()->planeSelect( origin, normal, thickness );

	return setLastErrorCode( PTV_SUCCESS );
}
//-----------------------------------------------------------------------------
PTres	PTAPI ptSelectPointsBySphere( const PTdouble *centre, PTdouble radius )
{
	PTTRACE_FUNC_P4( centre[0], centre[1], centre[2], radius )

	if (!centre)
		return setLastErrorCode( PTV_VOID_POINTER );
	
	if (radius < 0) return setLastErrorCode( PTV_INVALID_PARAMETER );


	if (g_currentViewParams)
		PointEditManager::instance()->setViewParams(*g_currentViewParams);

	PointEditManager::instance()->paintSelSphere();
	PointEditManager::instance()->paintRadius( radius );
	PointEditManager::instance()->paintSelectAtPoint( centre, false );

	return setLastErrorCode( PTV_SUCCESS );
}
//-----------------------------------------------------------------------------
PTvoid PTAPI ptSetSelectionScope( PThandle sceneOrCloudHandle )
{
	PTTRACE_FUNC_P1( sceneOrCloudHandle )

	if (!sceneOrCloudHandle)
	{
		PointEditManager::instance()->clearEditingScope();
		return;
	}
	const pcloud::Scene* scene = sceneFromHandle( sceneOrCloudHandle );
	const pcloud::PointCloud* cloud = cloudFromHandle( sceneOrCloudHandle );

	if (!cloud)
	{
		if (!scene || !scene->numObjects()) return;

		cloud = scene->cloud(0);
		PointEditManager::instance()->setEditingScope( cloud->guid(), true, scene->getInstanceIndex() );
	}
	else
	{
		PointEditManager::instance()->setEditingScope( cloud->guid(), false, scene->getInstanceIndex() );
	}
}
//-----------------------------------------------------------------------------
PTuint64 PTAPI _ptCountVisiblePoints( void )
{
	PTTRACE_FUNC

	PTuint64 count = PointEditManager::instance()->countVisiblePoints();
	return count;
}
//-----------------------------------------------------------------------------
PTvoid PTAPI ptInvertVisibility()
{
	PTTRACE_FUNC

	PointEditManager::instance()->invertVisibility();
}
//-----------------------------------------------------------------------------
PTvoid PTAPI ptIsolateSelected()
{
	PTTRACE_FUNC

	PointEditManager::instance()->isolateSelPoints();
}
//-----------------------------------------------------------------------------
PTvoid PTAPI ptInvertSelection()
{
	PTTRACE_FUNC

	PointEditManager::instance()->invertSelection();
}
//-----------------------------------------------------------------------------
PTvoid	PTAPI ptHideSelected()
{
	PTTRACE_FUNC

	PointEditManager::instance()->hideSelPoints();
}
//-----------------------------------------------------------------------------
PTvoid	PTAPI ptUnhideAll()
{
	PTTRACE_FUNC

	PointEditManager::instance()->clearAll();
}
//-----------------------------------------------------------------------------
PTvoid	PTAPI ptUnselectAll()
{
	PTTRACE_FUNC

	PointEditManager::instance()->deselectAll();
}
//-----------------------------------------------------------------------------
PTvoid	PTAPI ptResetSelection()
{
	PTTRACE_FUNC

	PointEditManager::instance()->resetSelection();
}
//-----------------------------------------------------------------------------
PTvoid	PTAPI ptSelectAll()
{
	PTTRACE_FUNC

	PointEditManager::instance()->selectAll();
}
//-----------------------------------------------------------------------------
PTvoid	PTAPI ptRefreshEdit()
{
	PTTRACE_FUNC

	if (PointEditManager::instance()->workingMode() == EditWorkOnAll)
	{
		PointEditManager::instance()->regenOOCComplete();	
	}
	else
	{
		PointEditManager::instance()->regenEditQuick();
	}
}
//-----------------------------------------------------------------------------
PTvoid	PTAPI ptClearEdit()
{
	PTTRACE_FUNC

	PointEditManager::instance()->clearEdit();
}
//-----------------------------------------------------------------------------
PTvoid	PTAPI ptStoreEdit( const PTstr name )
{
	PTTRACE_FUNC

	PointEditManager::instance()->storeEdit( pt::String(name) );
}
//-----------------------------------------------------------------------------
PTbool	PTAPI ptRestoreEdit( const PTstr name )
{
	PTTRACE_FUNC

	return PointEditManager::instance()->restoreEdit( pt::String(name) );
}
//-----------------------------------------------------------------------------
PTbool	PTAPI ptRestoreEditByIndex( PTint index )
{
	PTTRACE_FUNC

	return ptRestoreEdit(PointEditManager::instance()->editName( index ));
}
//-----------------------------------------------------------------------------
PTint	PTAPI ptNumEdits( void )
{
	return PointEditManager::instance()->numEdits();
}
//-----------------------------------------------------------------------------
const	PTstr PTAPI ptEditName( PTint index )
{
	return PointEditManager::instance()->editName( index ).c_wstr(); 
}
//-----------------------------------------------------------------------------
PTvoid PTAPI ptDeleteAllEdits()
{
	PTTRACE_FUNC

	PointEditManager::instance()->removeAllEdits(); 
}
//-----------------------------------------------------------------------------
PTbool PTAPI ptDeleteEdit( const PTstr name )
{
	PTTRACE_FUNC

	int num_edits = PointEditManager::instance()->numEdits();
	PointEditManager::instance()->removeEdit( pt::String(name) );
	return num_edits == PointEditManager::instance()->numEdits() ? false : true;
}
//-----------------------------------------------------------------------------
PTbool PTAPI ptDeleteEditByIndex( PTint index )
{
	PTTRACE_FUNC_P1( index )

	return ptDeleteEdit(PointEditManager::instance()->editName( index ));
}
//-----------------------------------------------------------------------------
using namespace datatree;
PTvoid *PTAPI _ptGetEditDatatree( PTint index )
{
	const Branch * dtree = PointEditManager::instance()->_getEditDatatree( index );
	return const_cast<Branch*>(dtree);
}
//-----------------------------------------------------------------------------
PTvoid PTAPI _ptCreateEditFromDatatree( PTvoid* dt )
{
	PTTRACE_FUNC

	Branch *dtree = (Branch*)dt;
	PointEditManager::instance()->_createEditFromDatatree( dtree );
}
//-----------------------------------------------------------------------------
PTres	PTAPI ptLayerBounds( PTuint layer, PTfloat *lower3, PTfloat *upper3, bool approx_fast )
{
	PTTRACE_FUNC

	pt::BoundingBoxD box;
	if (PointEditManager::instance()->getLayerBoundingBox( layer, box, approx_fast ))
	{
		lower3[0] = box.lx();
		lower3[1] = box.ly();
		lower3[2] = box.lz();

		upper3[0] = box.ux();
		upper3[1] = box.uy();
		upper3[2] = box.uz();

		PTTRACEOUT << " = " << lower3[0] << ", " << lower3[1] << ", " << lower3[2] << " to " << upper3[0] << ", " << upper3[1] << ", " << upper3[2] ;

		return 	setLastErrorCode( PTV_SUCCESS );
	}
	return 	setLastErrorCode( PTV_INVALID_HANDLE );
}
//-----------------------------------------------------------------------------
PTres	PTAPI ptLayerBoundsd( PTuint layer, PTdouble *lower3, PTdouble *upper3, bool approx_fast )
{
	pt::BoundingBoxD box;
	if (PointEditManager::instance()->getLayerBoundingBox( layer, box, approx_fast ))
	{
		//vector3d basepoint(Project3D::project().registration().matrix()(3,0), 
		//	Project3D::project().registration().matrix()(3,1), 
		//	Project3D::project().registration().matrix()(3,2));

		lower3[0] = box.lx();// - basepoint.x;
		lower3[1] = box.ly();// - basepoint.y;
		lower3[2] = box.lz();// - basepoint.z;

		upper3[0] = box.ux();// - basepoint.x;
		upper3[1] = box.uy();// - basepoint.y;
		upper3[2] = box.uz();// - basepoint.z;

		PTTRACEOUT << " = " << lower3[0] << ", " << lower3[1] << ", " << lower3[2] << " to " << upper3[0] << ", " << upper3[1] << ", " << upper3[2] ;

		return 	setLastErrorCode( PTV_SUCCESS );
	}
	return 	setLastErrorCode( PTV_INVALID_HANDLE );
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
PTint PTAPI ptGetEditData( PTint index, PTubyte *data )
{
	PTTRACE_FUNC

	Branch * dtree = PointEditManager::instance()->_getEditDatatree( index );
	
	if (dtree)
	{
		ptds::DataSourcePtr memBlock = ptds::DataSourceMemBlock::createNew();
		writeBinaryDatatree( dtree, memBlock );

		int fs = memBlock->getFileSize();
		void * buffer = static_cast<ptds::DataSourceMemBlock*>(memBlock)->getWriteBuffer();
		memcpy(data, buffer, fs);

		memBlock->closeAndDelete();

		return fs;
	}
	return 0;
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
PTint PTAPI ptGetEditDataSize( PTint index )
{
	PTTRACE_FUNC

	Branch * dtree = PointEditManager::instance()->_getEditDatatree( index );

	if (dtree)
	{
		ptds::DataSourcePtr memBlock = ptds::DataSourceMemBlock::createNew();

		writeBinaryDatatree( dtree, memBlock );

		int fs = memBlock->getFileSize();
		memBlock->closeAndDelete();
		return fs;
	}
	return 0;
}
//-----------------------------------------------------------------------------
PTvoid PTAPI ptCreateEditFromData( const PTubyte *data )
{
	PTTRACE_FUNC

	ptds::DataSourcePtr memBlock = ptds::DataSourceMemBlock::createNew(data);

	Branch *edit = new Branch( "root" );
	readBinaryDatatree( edit, memBlock );

	memBlock->close();

	PointEditManager::instance()->_createEditFromDatatree( edit );
}
//-----------------------------------------------------------------------------
// Layers - added in version 1.3
//-----------------------------------------------------------------------------
PTbool PTAPI ptSetCurrentLayer( PTuint layer )
{
	PTTRACE_FUNC_P1( layer )

	return PointEditManager::instance()->setCurrentLayer( layer );
}
//-----------------------------------------------------------------------------
PTuint PTAPI ptGetCurrentLayer()
{
	return PointEditManager::instance()->getCurrentLayer();
}
//-----------------------------------------------------------------------------
PTbool PTAPI ptLockLayer( PTuint layer, PTbool lock )
{
	PTTRACE_FUNC_P2( layer, (lock ? "true" : "false") )

	return PointEditManager::instance()->lockLayer( layer, lock );
}
//-----------------------------------------------------------------------------
PTbool PTAPI ptIsLayerLocked( PTuint layer )
{
	return PointEditManager::instance()->isLayerLocked( layer );
}
//-----------------------------------------------------------------------------
PTbool PTAPI ptShowLayer( PTuint layer, PTbool show )
{
	PTTRACE_FUNC_P2( layer, (show ? "true" : "false") )

	return PointEditManager::instance()->showLayer( layer, show );
}
//-----------------------------------------------------------------------------
PTbool PTAPI ptIsLayerShown( PTuint layer )
{
	return PointEditManager::instance()->isLayerVisible( layer );
}
//-----------------------------------------------------------------------------
PTbool PTAPI ptDoesLayerHavePoints( PTuint layer )
{
	return PointEditManager::instance()->doesLayerHavePoints( layer );
}
//-----------------------------------------------------------------------------
PTvoid PTAPI ptClearPointsFromLayer( PTuint layer )
{
	PTTRACE_FUNC_P1( layer )

	setLastErrorCode( PTV_NOT_IMPLEMENTED_IN_VERSION );
}
//-----------------------------------------------------------------------------
PTvoid PTAPI ptResetLayers()
{
	PTTRACE_FUNC

	setLastErrorCode( PTV_NOT_IMPLEMENTED_IN_VERSION );
}
//-----------------------------------------------------------------------------
PTres	PTAPI ptResetSceneEditing( PThandle scene )
{
	PTTRACE_FUNC_P1( scene )

	ptSetSelectionScope( scene );
	ptUnhideAll();
	ptSetSelectionScope( 0 );

	return PTV_SUCCESS;
}
//-----------------------------------------------------------------------------
PTbool PTAPI ptCopySelToCurrentLayer( PTbool deselect )
{
	PTTRACE_FUNC_P1( deselect )

	return PointEditManager::instance()->copySelToLayer( deselect );
}
//-----------------------------------------------------------------------------
PTbool PTAPI ptMoveSelToCurrentLayer( PTbool deselect )
{
	PTTRACE_FUNC_P1( deselect )

	return PointEditManager::instance()->moveSelToLayer( deselect );
}
//-----------------------------------------------------------------------------
PTvoid  PTAPI ptSelectPointsInLayer( PTuint layer )
{
	PTTRACE_FUNC_P1( layer )

	PointEditManager::instance()->selectPointsInLayer( layer );
}
//-----------------------------------------------------------------------------
PTvoid  PTAPI ptDeselectPointsInLayer( PTuint layer )
{ 
	PTTRACE_FUNC_P1( layer )

	PointEditManager::instance()->deselectPointsInLayer( layer );
}
//-----------------------------------------------------------------------------
PTres	PTAPI ptSelectCloud( PThandle cloud )
{
	PTTRACE_FUNC_P1( cloud )

	pcloud::PointCloud *c = cloudFromHandle( cloud );

	// this will not select on hidden layers
	if (c)
	{
		// for now we use scope
		ptSetSelectionScope( cloud );
		ptSelectAll();
		ptSetSelectionScope( 0 );

		return 	setLastErrorCode( PTV_SUCCESS );
	}
	return 	setLastErrorCode( PTV_INVALID_HANDLE );
}
//-----------------------------------------------------------------------------
PTres	PTAPI ptDeselectCloud( PThandle cloud )
{
	PTTRACE_FUNC_P1( cloud )

	pcloud::PointCloud *c = cloudFromHandle( cloud );

	if (c)
	{
		ptSetSelectionScope( cloud );
		ptUnselectAll();
		ptSetSelectionScope( 0 );

		return 	setLastErrorCode( PTV_SUCCESS );
	}
	return 	setLastErrorCode( PTV_INVALID_HANDLE );
}
//-----------------------------------------------------------------------------
PTres	PTAPI ptSelectScene( PThandle scene )
{
	PTTRACE_FUNC_P1( scene )

	pcloud::Scene *c = sceneFromHandle( scene );

	if (c)
	{
		ptSetSelectionScope( scene );
		ptSelectAll();
		ptSetSelectionScope( 0 );

		return 	setLastErrorCode( PTV_SUCCESS );
	}
	return 	setLastErrorCode( PTV_INVALID_HANDLE );
}
//-----------------------------------------------------------------------------
PTres	PTAPI ptDeselectScene( PThandle scene )
{
	PTTRACE_FUNC_P1( scene )

	pcloud::Scene *c = sceneFromHandle( scene );

	if (c)
	{
		ptSetSelectionScope( scene );
		ptUnselectAll();
		ptSetSelectionScope( 0 );

		return 	setLastErrorCode( PTV_SUCCESS );
	}
	return 	setLastErrorCode( PTV_INVALID_HANDLE );
}
//-----------------------------------------------------------------------------
PTbool   PTAPI ptSetLayerColor( PTuint layer, PTfloat *rgb3, PTfloat blend )
{
	if (!rgb3)
	{
		PTTRACE_FUNC

		setLastErrorCode(PTV_VOID_POINTER);
		return false;
	}

	PTTRACE_FUNC_P5( layer, rgb3[0], rgb3[1], rgb3[2], blend )

	ptgl::Color c(rgb3[0], rgb3[1], rgb3[2], blend);

	if ( layer < thePointLayersState().numLayers() )
	{
		thePointLayersState().setLayerColor( 1 << layer, c );
		thePointLayersState().setLayerColorAlpha( 1 << layer, blend );

		return true;
	}
	return false;
}
//-----------------------------------------------------------------------------
PTfloat *PTAPI ptGetLayerColor( PTuint layer )
{
	if ( layer < thePointLayersState().numLayers() )
	{
		static ptgl::Color c;
		c = thePointLayersState().getLayerColor( 1 << layer );
		return (PTfloat*)&c.r;
	}
	return 0;
}
//-----------------------------------------------------------------------------
PTfloat  PTAPI ptGetLayerColorBlend( PTuint layer )
{
	if ( layer < thePointLayersState().numLayers() )
	{
		static ptgl::Color c = thePointLayersState().getLayerColor( 1 << layer );
		return c.a;
	}
	return 0;
}
//-----------------------------------------------------------------------------
PTvoid   PTAPI ptResetLayerColors( void )
{
	PTTRACE_FUNC

	thePointLayersState().resetLayerColors();
}
//-----------------------------------------------------------------------------
// provides a count of the number of points represented, ie. ignores LOD and approximates the full count
PTuint64 PTAPI ptCountApproxPointsInLayer( PTuint layer )
{
	PTTRACE_FUNC_P1( layer )
	
	PTuint64 count = PointEditManager::instance()->countPointsInLayer( layer );
	
	PTTRACEOUT << "=" << count;

	return count;
}
