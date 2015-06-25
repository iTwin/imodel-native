#include <pt/os.h>
#define POINTOOLS_API_BUILD_DLL
#include <gl/glew.h>

#include <ptapi/PointoolsVortexAPI.h>
#include <ptapi/PointoolsVortexAPI_ResultCodes.h>
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
	switch ( mode )
	{
	case PT_EDIT_WORK_ON_ALL:	
		PointEditManager::instance()->workingMode( EditWorkOnAll );
		break;
	case PT_EDIT_WORK_ON_VIEW:
		PointEditManager::instance()->workingMode( EditWorkOnView );
		break;
	case PT_EDIT_WORK_ON_PROPORTION:
		PointEditManager::instance()->workingMode( EditWorkOnProportion );
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
	_ptMakeVPContextCurrent();

	if (g_currentViewParams)
		PointEditManager::instance()->setViewParams(*g_currentViewParams);

	_ptAdjustMouseYVal(y_edge);
	
	PointEditManager::instance()->setUnits( g_unitScale );
	PointEditManager::instance()->rectangleSelect( x_edge, x_edge+width, y_edge, y_edge - height ); 
	_ptRestoreContext();
}
//-----------------------------------------------------------------------------
PTres	PTAPI ptSelectPointsByFence( PTint num_vertices, const PTint *vertices )
{
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
	PTuint64 count = PointEditManager::instance()->countVisiblePoints();
	return count;
}
//-----------------------------------------------------------------------------
PTvoid PTAPI ptInvertVisibility()
{
	PointEditManager::instance()->invertVisibility();
}
//-----------------------------------------------------------------------------
PTvoid PTAPI ptIsolateSelected()
{
	PointEditManager::instance()->isolateSelPoints();
}
//-----------------------------------------------------------------------------
PTvoid PTAPI ptInvertSelection()
{
	PointEditManager::instance()->invertSelection();
}
//-----------------------------------------------------------------------------
PTvoid	PTAPI ptHideSelected()
{
	PointEditManager::instance()->hideSelPoints();
}
//-----------------------------------------------------------------------------
PTvoid	PTAPI ptUnhideAll()
{
	PointEditManager::instance()->clearAll();
}
//-----------------------------------------------------------------------------
PTvoid	PTAPI ptUnselectAll()
{
	PointEditManager::instance()->deselectAll();
}
//-----------------------------------------------------------------------------
PTvoid	PTAPI ptResetSelection()
{
	PointEditManager::instance()->resetSelection();
}
//-----------------------------------------------------------------------------
PTvoid	PTAPI ptSelectAll()
{
	PointEditManager::instance()->selectAll();
}
//-----------------------------------------------------------------------------
PTvoid	PTAPI ptRefreshEdit()
{
	PointEditManager::instance()->regenEditQuick();
}
//-----------------------------------------------------------------------------
PTvoid	PTAPI ptClearEdit()
{
	PointEditManager::instance()->clearEdit();
}
//-----------------------------------------------------------------------------
PTvoid	PTAPI ptStoreEdit( const PTstr name )
{
	PointEditManager::instance()->storeEdit( pt::String(name) );
}
//-----------------------------------------------------------------------------
PTbool	PTAPI ptRestoreEdit( const PTstr name )
{
	return PointEditManager::instance()->restoreEdit( pt::String(name) );
}
//-----------------------------------------------------------------------------
PTbool	PTAPI ptRestoreEditByIndex( PTint index )
{
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
	PointEditManager::instance()->removeAllEdits(); 
}
//-----------------------------------------------------------------------------
PTbool PTAPI ptDeleteEdit( const PTstr name )
{
	int num_edits = PointEditManager::instance()->numEdits();
	PointEditManager::instance()->removeEdit( pt::String(name) );
	return num_edits == PointEditManager::instance()->numEdits() ? false : true;
}
//-----------------------------------------------------------------------------
PTbool PTAPI ptDeleteEditByIndex( PTint index )
{
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
	Branch *dtree = (Branch*)dt;
	PointEditManager::instance()->_createEditFromDatatree( dtree );
}
//-----------------------------------------------------------------------------
PTres	PTAPI ptLayerBounds( PTuint layer, PTfloat *lower3, PTfloat *upper3, bool approx_fast )
{
	pt::BoundingBoxD box;
	if (PointEditManager::instance()->getLayerBoundingBox( layer, box, approx_fast ))
	{
		lower3[0] = box.lx();
		lower3[1] = box.ly();
		lower3[2] = box.lz();

		upper3[0] = box.ux();
		upper3[1] = box.uy();
		upper3[2] = box.uz();

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
		vector3d basepoint(Project3D::project().registration().matrix()(3,0), 
			Project3D::project().registration().matrix()(3,1), 
			Project3D::project().registration().matrix()(3,2));

		lower3[0] = box.lx() - basepoint.x;
		lower3[1] = box.ly() - basepoint.y;
		lower3[2] = box.lz() - basepoint.z;

		upper3[0] = box.ux() - basepoint.x;
		upper3[1] = box.uy() - basepoint.y;
		upper3[2] = box.uz() - basepoint.z;

		return 	setLastErrorCode( PTV_SUCCESS );
	}
	return 	setLastErrorCode( PTV_INVALID_HANDLE );
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
PTint PTAPI ptGetEditData( PTint index, PTubyte *data )
{
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
	setLastErrorCode( PTV_NOT_IMPLEMENTED_IN_VERSION );
}
//-----------------------------------------------------------------------------
PTvoid PTAPI ptResetLayers()
{
	setLastErrorCode( PTV_NOT_IMPLEMENTED_IN_VERSION );
}
//-----------------------------------------------------------------------------
PTres	PTAPI ptResetSceneEditing( PThandle scene )
{
	ptSetSelectionScope( scene );
	ptUnhideAll();
	ptSetSelectionScope( 0 );

	return PTV_SUCCESS;
}
//-----------------------------------------------------------------------------
PTbool PTAPI ptCopySelToCurrentLayer( PTbool deselect )
{
	return PointEditManager::instance()->copySelToLayer( deselect );
}
//-----------------------------------------------------------------------------
PTbool PTAPI ptMoveSelToCurrentLayer( PTbool deselect )
{
	return PointEditManager::instance()->moveSelToLayer( deselect );
}
//-----------------------------------------------------------------------------
PTvoid  PTAPI ptSelectPointsInLayer( PTuint layer )
{
	PointEditManager::instance()->selectPointsInLayer( layer );

	PointEditManager::instance()->regenEditQuick();
}
//-----------------------------------------------------------------------------
PTvoid  PTAPI ptDeselectPointsInLayer( PTuint layer )
{ 
	PointEditManager::instance()->deselectPointsInLayer( layer );
}
//-----------------------------------------------------------------------------
PTres	PTAPI ptSelectCloud( PThandle cloud )
{
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
		setLastErrorCode(PTV_VOID_POINTER);
		return false;
	}
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
	thePointLayersState().resetLayerColors();
}