// ***************************************************************
//  Pointools Vortex API   version:  1.5.1.1   ·  date July 2010
//  -------------------------------------------------------------
//  Header file for Pointools Vortex API
//  For Build Use Only. Clients should use PointoolsVortexAPI_import.h
//  -------------------------------------------------------------
//  Copyright (C) Pointools Ltd 2007-09 - All Rights Reserved
// ***************************************************************
// 
// ***************************************************************
#ifndef POINTOOLS_VORTEX_API_HEADER
#define POINTOOLS_VORTEX_API_HEADER 1

/* typedefs */ 
typedef unsigned int		PTenum;
typedef bool				PTbool;
typedef int					PTint;
typedef int					PTres;
typedef unsigned int		PTuint;
typedef float				PTfloat;
typedef double				PTdouble;
typedef short				PTshort;
typedef unsigned short		PTushort;
typedef char				PTbyte;
typedef unsigned char		PTubyte;
typedef unsigned __int64	PTuint64;
typedef __int64				PTint64;
#define PTstr wchar_t*


typedef void				PTvoid;
typedef PTuint				PThandle;
typedef unsigned char		PTubyte;

/* Shader Enables */ 
#define PT_RGB_SHADER					0x01
#define PT_INTENSITY_SHADER				0x02
#define PT_BLENDING_SHADER				0x03
#define PT_PLANE_SHADER					0x04
#define PT_LIGHTING						0x05
#define PT_CLIPPING						0x06
#define PT_CHANNEL_RENDER				0x07

/* Display Enables */ 
#define PT_ADAPTIVE_POINT_SIZE			0X100
#define PT_FRONT_BIAS					0X101
#define PT_DELAYED_CHANNEL_LOAD			0X102

/* Shader Settings */ 
#define PT_PLANE_SHADER_DISTANCE		0x11	
#define PT_PLANE_SHADER_VECTOR			0x12
#define PT_PLANE_SHADER_OFFSET			0x13

#define PT_INTENSITY_SHADER_CONTRAST	0x14
#define PT_INTENSITY_SHADER_BRIGHTNESS	0x15

#define PT_RGB_SHADER_CONTRAST			0x16
#define PT_RGB_SHADER_BRIGHTNESS		0x17

#define PT_LIGHT_VECTOR					0x18
#define PT_LIGHT_ANGLE					0x19
#define PT_LIGHT_COLOUR					0x1a
#define PT_LIGHT_AMBIENT_COLOUR			0x1b
#define PT_LIGHT_DIFFUSE_COLOUR			0x1c
#define PT_LIGHT_SPECULAR_COLOUR		0x1d
#define PT_LIGHT_STRENGTH				0x1f
#define PT_LIGHT_AMBIENT_STRENGTH		0x20
#define PT_LIGHT_DIFFUSE_STRENGTH		0x21
#define PT_LIGHT_SPECULAR_STRENGTH		0x22

#define PT_INTENSITY_SHADER_RAMP		0x23
#define PT_PLANE_SHADER_RAMP			0x24

#define PT_MATERIAL_AMBIENT				0X25
#define PT_MATERIAL_DIFFUSE				0x26
#define PT_MATERIAL_SPECULAR			0x27
#define PT_MATERIAL_GLOSSINESS			0x28

#define PT_PLANE_SHADER_EDGE			0x29

#define PT_EDGE_REPEAT					0x00
#define PT_EDGE_CLAMP					0x01
#define PT_EDGE_BLACK					0x02
#define PT_EDGE_MIRROR					0x03

/* units */ 
#define PT_METERS						0x100
#define PT_DECIMETERS					0x101
#define PT_CENTIMETERS					0x102
#define PT_MILLIMETERS					0x103
#define PT_FEET							0x104
#define PT_FEET_US						0x106
#define PT_INCHES						0x105

/* draw modes */ 
#define PT_DRAW_MODE_STATIC				0x01
#define PT_DRAW_MODE_INTERACTIVE		0x02
#define PT_DRAW_MODE_DEFAULT			0x00
#define PT_DRAW_MODE_COMPATIBILITY		0x04

/* selection modes */ 
#define PT_SELECT						0x01		
#define PT_DESELECT						0x02
#define PT_SELECT_TOGGLE				0x03

/*context */ 
#define PT_GLOBAL_CONTEXT				0x01
#define PT_SCENE_CONTEXT				0x02
#define PT_CLOUD_CONTEXT				0x03
#define	PT_VIEWPORT_CONTEXT				0x04

/* constants */ 
#define PT_MAX_VIEWPORTS	256
#define PT_TRUE				true
#define PT_FALSE			false
#define PT_NULL				0
#define PT_ERROR			0

/* coordinate base */ 
#define PT_AUTO_BASE_DISABLED	0x0
#define PT_AUTO_BASE_CENTER		0x01
#define PT_AUTO_BASE_REDUCE		0x02
#define PT_AUTO_BASE_FIRST_ONLY 0x04

/* ramps */ 
#define PT_INTENSITY_RAMP_TYPE	0x01
#define PT_PLANE_RAMP_TYPE		0x02

/* point attributes */ 
#define PT_HAS_INTENSITY		0x01
#define PT_HAS_RGB				0x02
#define PT_HAS_NORMAL			0x04
#define PT_HAS_FILTER			0x08
#define PT_HAS_CLASSIFICATION	0x10

/* editing */ 
#define PT_EDIT_MODE_SELECT		0X01
#define PT_EDIT_MODE_UNSELECT	0X02
#define PT_EDIT_MODE_UNHIDE		0X03

#define PT_EDIT_WORK_ON_ALL			0x01
#define PT_EDIT_WORK_ON_VIEW		0x02
#define PT_EDIT_WORK_ON_PROPORTION	0x03

/* query */ 
#define PT_QUERY_DENSITY_FULL			0x01
#define	PT_QUERY_DENSITY_VIEW			0X02
#define PT_QUERY_DENSITY_LIMIT			0X03
#define	PT_QUERY_DENSITY_VIEW_COMPLETE	0X04
#define PT_QUERY_DENSITY_SPATIAL		0x07

#define PT_QUERY_RGB_MODE_ACTUAL 0x04
#define PT_QUERY_RGB_MODE_SHADER 0x05
#define PT_QUERY_RGB_MODE_SHADER_NO_SELECT 0x06

/* imaging */ 
#define PT_IMAGE_TYPE_COLOUR		0x01
#define PT_IMAGE_TYPE_NORMAL		0x02
#define PT_IMAGE_TYPE_DEPTH			0x03
#define PT_IMAGE_TYPE_BUMP			0X04

/* tuning */ 
#define PT_LOADING_BIAS_SCREEN		0x01
#define PT_LOADING_BIAS_NEAR		0x02
#define PT_LOADING_BIAS_FAR			0x03
#define PT_LOADING_BIAS_POINT		0x04

/* fitting */ 
#define PT_FIT_MODE_USE_SELECTED	0x01
#define PT_FIT_MODE_USE_INPUT		0x02

/* eye perspective type */ 
#define PT_PROJ_PERSPECTIVE_GL		0x01
#define PT_PROJ_PERSPECTIVE_DX		0x02
#define PT_PROJ_PERSPECTIVE_BLINN	0x03

/* channel constants */ 
/* draw as */ 
#define PT_CHANNEL_AS_OFFSET		0x01
#define PT_CHANNEL_AS_RAMP			0x02
#define PT_CHANNEL_AS_ZSHIFT		0x03
#define PT_CHANNEL_AS_RGB			0x04

/* options */ 
#define PT_CHANNEL_OUT_OF_CORE		0X01

/* meta data */ 
#define PT_MAX_META_STR_LEN			1024

/* Viewport context types */ 
#define PT_GL_VIEWPORT				0X01
#define PT_DX_VIEWPORT				0X02
#define PT_SW_VIEWPORT				0X03

/* clipping options */
#define PT_CLIP_OUTSIDE				0x01
#define PT_CLIP_INSIDE				0x02

#define PTAPI	__stdcall

/* Pointools Vortex API v1.2 */ 
/* intitialization */ 
PTbool	PTAPI ptInitialize( const PTubyte *license );
PTbool	PTAPI ptIsInitialized();
PTvoid	PTAPI ptSetWorkingFolder( const PTstr folder );
const	PTstr	PTAPI ptGetWorkingFolder( void );
const	PTstr	PTAPI ptGetVersionString( void );
PTvoid	PTAPI	ptGetVersionNum( PTubyte *version );
PTvoid PTAPI ptRelease( void );

/* handle management */ 
PThandle	PTAPI ptGetCloudHandleByIndex( PThandle scene, PTuint cloud_index );
PTuint		PTAPI ptGetNumCloudsInScene( PThandle scene );

/* importing scene data */ 
PThandle	PTAPI ptOpenPOD( const PTstr filepath );
PThandle	PTAPI ptOpenPODStructuredStorageStream(const PTstr filepath, PTvoid *stream);
PThandle	PTAPI ptIsOpen( const PTstr filepath );
PThandle	PTAPI ptBrowseAndOpenPOD( void );

/* management */ 
PTint	PTAPI ptNumScenes( void );
PTint	PTAPI ptGetSceneHandles( PThandle *handles );
PTbool	PTAPI ptSceneInfo( PThandle scene, PTstr name, PTint &clouds, PTuint &num_points, PTuint &specification, PTbool &loaded, PTbool &visible );
const PTstr	PTAPI ptSceneFile( PThandle scene );
PTres	PTAPI ptCloudInfo( PThandle cloud, PTstr name, PTuint &num_points, PTuint &specification, PTbool &visible );

PTres	PTAPI ptLayerBounds( PTuint layer, PTfloat *lower3, PTfloat *upper3, bool approx_fast );
PTres	PTAPI ptLayerBoundsd( PTuint layer, PTdouble *lower3, PTdouble *upper3, bool approx_fast );

PTres	PTAPI ptSceneBounds( PThandle scene, PTfloat *lower3, PTfloat *upper3 );
PTres	PTAPI ptSceneBoundsd( PThandle scene, PTdouble *lower3, PTdouble *upper3 );
PTres	PTAPI ptCloudBounds( PThandle cloud, PTfloat *lower3, PTfloat *upper3 );
PTres	PTAPI ptCloudBoundsd( PThandle cloud, PTdouble *lower3, PTdouble *upper3 );

PTres	PTAPI ptShowScene( PThandle scene, PTbool visible );
PTres	PTAPI ptShowCloud( PThandle cloud, PTbool visible );

PTbool PTAPI ptIsSceneVisible( PThandle scene );
PTbool PTAPI ptIsCloudVisible( PThandle cloud );

PTres	PTAPI ptUnloadScene( PThandle scene );
PTres	PTAPI ptReloadScene( PThandle scene );
PTres	PTAPI ptRemoveScene( PThandle scene );
PTvoid	PTAPI ptRemoveAll();

/* Meta data */ 
PThandle	PTAPI	ptReadPODMeta( const PTstr filepath );
PThandle	PTAPI	ptGetMetaDataHandle( PThandle sceneHandle );
PTres		PTAPI	ptGetMetaData( PThandle metadataHandle, PTstr name, PTint &num_clouds, 
						PTuint64 &num_points, PTuint &scene_spec, PTdouble *lower3, PTdouble *upper3 );
PTres		PTAPI	ptGetMetaTag( PThandle metadataHandle, const PTstr tagName, PTstr value );
PTres		PTAPI	ptSetMetaTag( PThandle metadataHandle, const PTstr tagName, PTstr value );
PTres		PTAPI	ptWriteMetaTags( PThandle metadataHandle );
PTvoid		PTAPI	ptFreeMetaData( PThandle metadataHandle );

/* user metatags */ 
PTint		PTAPI	ptNumUserMetaSections( PThandle metadataHandle );
const PTstr	PTAPI	ptUserMetaSectionName( PThandle metadataHandle, PTint section_index );

PTint		PTAPI	ptNumUserMetaTagsInSection( PThandle metadataHandle, PTint section_index );
PTres		PTAPI	ptGetUserMetaTagByIndex( PThandle metadataHandle, PTint section_index, PTint tag_index, PTstr name, PTstr value );
PTres		PTAPI	ptGetUserMetaTagByName( PThandle metadataHandle, const PTstr sectionDotName, PTstr value );

/* scene duplication */ 
PThandle	PTAPI	ptCreateSceneInstance( PThandle scene );

/* transformation */ 
PTres		PTAPI	ptSetCloudTransform( PThandle cloud, const PTdouble *transform4x4, bool row_order );
PTres		PTAPI	ptSetSceneTransform( PThandle scene, const PTdouble *transform4x4, bool row_order );
PTres		PTAPI	ptGetCloudTransform( PThandle cloud, PTdouble *transform4x4, bool row_order );
PTres		PTAPI	ptGetSceneTransform( PThandle scene, PTdouble *transform4x4, bool row_order );

/* persistence of viewport setup */ 
PTuint	PTAPI ptGetPerViewportDataSize();
PTuint	PTAPI ptGetPerViewportData( PTubyte *data );
PTres	PTAPI ptSetPerViewportData( const PTubyte *data );

PTvoid	PTAPI ptSetViewportPointsBudget( PTint budget );
PTint	PTAPI ptGetViewportPointsBudget( void );

/* points */ 
PTuint	PTAPI ptGetCloudProxyPoints( PThandle scene, PTint num_points, PTfloat *pnts, PTubyte *col );
PTuint	PTAPI ptGetSceneProxyPoints( PThandle cloud, PTint num_points, PTfloat *pnts, PTubyte *col );

/* error handling */ 
PTstr	PTAPI ptGetLastErrorString( void );
PTres	PTAPI ptGetLastErrorCode( void );

/* view parameters - these operate in current viewport */ 
PTbool	PTAPI ptReadViewFromGL( void );
PTbool	PTAPI ptReadViewFromDX( void );

PTvoid	PTAPI ptSetViewProjectionOrtho( PTdouble l, PTdouble r, PTdouble b, PTdouble t, PTdouble n, PTdouble f );
PTvoid	PTAPI ptSetViewProjectionFrustum( PTdouble l, PTdouble r, PTdouble b, PTdouble t, PTdouble n, PTdouble f );
PTvoid	PTAPI ptSetViewProjectionMatrix( const PTdouble *matrix, bool row_major );
PTvoid	PTAPI ptSetViewProjectionPerspective( PTenum type, PTdouble fov, PTdouble aspect, PTdouble n, PTdouble f);

PTvoid	PTAPI ptSetViewEyeLookAt( const PTdouble *eye3, const PTdouble *target3, const PTdouble *up3 );
PTvoid	PTAPI ptSetViewEyeMatrix( const PTdouble *matrix16, bool row_major );

PTvoid	PTAPI ptSetViewportSize( PTint left, PTint bottom, PTuint width, PTuint height ); 

PTvoid PTAPI ptGetViewEyeMatrix( PTdouble *matrix );
PTvoid PTAPI ptGetViewProjectionMatrix( PTdouble *matrix16 );

/* draw */ 
PTvoid	PTAPI ptOverrideDrawMode( PTenum mode );
PTvoid	PTAPI ptDrawGL(PTbool dynamic);
PTvoid	PTAPI ptDrawDX( void );
PTvoid	PTAPI ptDrawCB( void );
PTvoid	PTAPI ptDrawSceneGL( PThandle scene, PTbool dynamic );
PTvoid	PTAPI ptDrawSceneDX( PThandle scene, PTbool dynamic );
PTvoid	PTAPI ptDrawSceneCB( PThandle scene, PTbool dynamic );
PTuint	PTAPI ptKbLoaded( PTbool reset );
PTuint	PTAPI ptWeightedPtsLoaded( PTbool reset );

PTint64	PTAPI ptPtsLoadedInViewportSinceDraw( PThandle forScene );
PTint64	PTAPI ptPtsToLoadInViewport( PThandle forScene, PTbool reCompute );
PTvoid	PTAPI ptEndDrawFrameMetrics();
PTvoid	PTAPI ptStartDrawFrameMetrics();

/* units */ 
PTvoid	PTAPI ptSetHostUnits( PTenum units );
PTenum	PTAPI ptGetHostUnits( void );

/* Coordinate Truncation */ 
PTvoid PTAPI ptSetAutoBaseMethod( PTenum type );
PTenum PTAPI ptGetAutoBaseMethod( void );
PTvoid PTAPI ptGetCoordinateBase( PTdouble *coordinateBase );
PTvoid PTAPI ptSetCoordinateBase( PTdouble *coordinateBase );

/* viewports */ 
PTint	PTAPI ptAddViewport( PTint index, const PTstr name, PTenum contextType );
PTvoid	PTAPI ptRemoveViewport( PTint index );
PTvoid	PTAPI ptSetViewport( PTint index );
PTint	PTAPI ptSetViewportByName( const PTstr name );
PTint	PTAPI ptViewportIndexFromName( const PTstr name );
PTvoid	PTAPI ptCaptureViewportInfo( void );
PTvoid	PTAPI ptStoreView( void );
PTint	PTAPI ptCurrentViewport( void );

PTvoid	PTAPI ptEnableViewport( PTint index );
PTvoid	PTAPI ptDisableViewport( PTint index );
PTbool	PTAPI ptIsViewportEnabled( PTint index );
PTbool	PTAPI ptIsCurrentViewportEnabled( void );

/* offscreen viewport */ 
PTvoid* PTAPI ptCreateBitmapViewport(int w, int h, const PTstr name);
PTvoid	PTAPI ptDestroyBitmapViewport(const PTstr name);

/* bounds of data */ 
PTbool	PTAPI ptGetLowerBound( PTdouble *lower );
PTbool	PTAPI ptGetUpperBound( PTdouble *upper );

/* shader options */ 
PTvoid	PTAPI ptEnable( PTenum option );
PTvoid	PTAPI ptDisable( PTenum option );
PTbool	PTAPI ptIsEnabled( PTenum option );

PTres	PTAPI ptPointSize( PTfloat size );

PTres	PTAPI ptShaderOptionf( PTenum shader_option, PTfloat value );
PTres	PTAPI ptShaderOptionfv( PTenum shader_option, PTfloat *value );
PTres	PTAPI ptShaderOptioni( PTenum shader_option, PTint value );

PTres	PTAPI ptGetShaderOptionf( PTenum shader_option, PTfloat *value );
PTres	PTAPI ptGetShaderOptionfv( PTenum shader_option, PTfloat *values );
PTres	PTAPI ptGetShaderOptioni( PTenum shader_option, PTint *value );
PTvoid	PTAPI ptResetShaderOptions( void );

PTvoid	PTAPI ptCopyShaderSettings( PTuint dest_viewport );
PTvoid	PTAPI ptCopyShaderSettingsToAll( void );

PTint	PTAPI ptNumRamps( void );
const	PTstr PTAPI ptRampInfo( PTint ramp, PTenum *type );
PTres	PTAPI ptAddCustomRamp( const PTstr name, PTint numKeys, const PTfloat *positions, 
							  const PTubyte* colour3vals, PTbool interpolateInHSL );

/* lighting */ 
PTres	PTAPI ptLightOptionf( PTenum Light_option, PTfloat value );
PTres	PTAPI ptLightOptionfv( PTenum Light_option, PTfloat *value );
PTres	PTAPI ptLightOptioni( PTenum Light_option, PTint value );

PTres	PTAPI ptGetLightOptionf( PTenum Light_option, PTfloat *value );
PTres	PTAPI ptGetLightOptioni( PTenum Light_option, PTint *value );
PTvoid	PTAPI ptCopyLightSettings( PTuint dest_viewport );
PTvoid	PTAPI ptCopyLightSettingsToAll();
PTvoid	PTAPI ptResetLightOptions();

/* editing options */ 
PTres	PTAPI ptSetSelectPointsMode( PTenum select_mode );
PTenum	PTAPI ptGetSelectPointsMode( void );
PTvoid	PTAPI ptSelectPointsByRect( PTint x_edge, PTint y_edge, PTint width, PTint height ); 
PTres	PTAPI ptSelectPointsByFence( PTint num_vertices, const PTint *vertices );
PTres	PTAPI ptSelectPointsByCube( const PTdouble *centre, PTdouble radius );
PTres	PTAPI ptSelectPointsByPlane( const PTdouble *origin, const PTdouble *normal, PTdouble thickness );
PTres	PTAPI ptSelectPointsByBox( const PTdouble *lower, const PTdouble *upper );
PTres	PTAPI ptSelectPointsByOrientedBox( const PTdouble *lower, const PTdouble *upper, const PTdouble *pos, PTdouble *uAxis, PTdouble *vAxis);
PTres	PTAPI ptSelectPointsBySphere( const PTdouble *centre, PTdouble radius );
PTres	PTAPI ptSelectCloud( PThandle cloud );
PTres	PTAPI ptDeselectCloud( PThandle cloud );
PTres	PTAPI ptSelectScene( PThandle scene );
PTres	PTAPI ptDeselectScene( PThandle scene );
PTvoid  PTAPI ptInvertSelection( void );
PTvoid  PTAPI ptInvertVisibility( void );
PTvoid	PTAPI ptHideSelected( void );
PTvoid	PTAPI ptIsolateSelected( void );
PTvoid	PTAPI ptUnhideAll( void );
PTvoid	PTAPI ptUnselectAll( void );
PTvoid	PTAPI ptResetSelection( void );
PTvoid	PTAPI ptSelectAll( void );
PTvoid	PTAPI ptSetSelectionScope( PThandle sceneOrCloudHandle );
PTvoid	PTAPI ptSetSelectionDrawColor( const PTubyte *col3 );
PTvoid  PTAPI ptGetSelectionDrawColor( PTubyte *col3 );
PTres	PTAPI ptResetSceneEditing( PThandle scene );
PTvoid	PTAPI ptRefreshEdit( void );
PTvoid	PTAPI ptClearEdit( void );

PTvoid	PTAPI ptStoreEdit( const PTstr name );
PTbool	PTAPI ptRestoreEdit( const PTstr name );
PTbool	PTAPI ptRestoreEditByIndex( PTint index );
PTbool	PTAPI ptDeleteEdit( const PTstr name );
PTbool	PTAPI ptDeleteEditByIndex( PTint index );
PTvoid	PTAPI ptDeleteAllEdits( void );
PTint	PTAPI ptNumEdits( void );
const	PTstr PTAPI ptEditName( PTint index );

PTvoid	PTAPI ptSetEditWorkingMode( PTenum mode );
PTenum	PTAPI ptGetEditWorkingMode( void );

PTint PTAPI ptGetEditData( PTint index, PTubyte *data );
PTint PTAPI ptGetEditDataSize( PTint index );
PTvoid PTAPI ptCreateEditFromData( const PTubyte *data );

/* point layers */ 
PTbool PTAPI ptSetCurrentLayer( PTuint layer );
PTuint PTAPI ptGetCurrentLayer();
PTbool PTAPI ptLockLayer( PTuint layer, PTbool lock );
PTbool PTAPI ptIsLayerLocked( PTuint layer );
PTbool PTAPI ptShowLayer( PTuint layer, PTbool show );
PTbool PTAPI ptIsLayerShown( PTuint layer );
PTbool PTAPI ptDoesLayerHavePoints( PTuint layer );
PTvoid PTAPI ptClearPointsFromLayer( PTuint layer );
PTvoid PTAPI ptSelectPointsInLayer( PTuint layer );
PTvoid PTAPI ptDeselectPointsInLayer( PTuint layer );

PTuint64 PTAPI ptCountApproxPointsInLayer( PTuint layer );

PTbool   PTAPI ptSetLayerColor( PTuint layer, PTfloat *rgb3, PTfloat blend );
PTfloat *PTAPI ptGetLayerColor( PTuint layer );
PTfloat  PTAPI ptGetLayerColorBlend( PTuint layer );
PTvoid   PTAPI ptResetLayerColors( void );

PTvoid PTAPI ptResetLayers();

PTbool PTAPI ptCopySelToCurrentLayer( PTbool deselect );
PTbool PTAPI ptMoveSelToCurrentLayer( PTbool deselect );

/* optimisation and rendering options*/ 
PTvoid	PTAPI ptDynamicFrameRate( PTfloat fps );
PTfloat	PTAPI ptGetDynamicFrameRate();

PTvoid	PTAPI ptStaticOptimizer( PTfloat opt );
PTfloat	PTAPI ptGetStaticOptimizer();

PTvoid	PTAPI ptGlobalDensity( PTfloat opt );
PTfloat	PTAPI ptGetGlobalDensity( void );

/* Query */ 
PTres	PTAPI ptSetIntersectionRadius(PTfloat radius);
PTfloat	PTAPI ptGetIntersectionRadius( void );

PTint	PTAPI ptFindNearestScreenPoint( PThandle scene, PTint screenx, PTint screeny, PTdouble *pnt );
PTint	PTAPI ptFindNearestScreenPointWDepth( PThandle scene, PTint screenx, PTint screeny, 
											 PTfloat *dpArray4x4, PTdouble *pnt );

PTfloat PTAPI ptFindNearestPoint( PThandle scene, const PTdouble *pnt, PTdouble *nearest );

PTbool	PTAPI ptIntersectRay( PThandle scene, const PTdouble *origin, const PTdouble *direction,
							 PTdouble *intersection, PTenum densityType, PTfloat densityValue );

PTbool	PTAPI ptIntersectRayPntIndex( PThandle scene, const PTdouble *origin, const PTdouble *direction,
							 PThandle *cloud, PThandle *pntPartA, PThandle *pntPartB );

PTbool	PTAPI ptIntersectRayInterpolated( PThandle scene, const PTdouble *origin, 
										const PTdouble *direction, PThandle *tmpPointHandle ); /* Not Implemented */ 

PTbool	PTAPI ptPointData( PThandle cloud, PThandle pointIndex, 
							 PTdouble *position, PTshort *intensity, PTubyte *rgb, PTfloat *normal );

PTuint	PTAPI ptPointAttributes( PThandle cloud, PThandle pntPartA, PThandle pntPartB );

PTbool	PTAPI ptGetPointAttribute( PThandle cloud, PThandle pntPartA, PThandle pntPartB, 
								  PTuint attribute, void* data );

/* query : Warning Partial implementation only may assert */ 
PThandle PTAPI ptCreateSelPointsQuery();
PThandle PTAPI ptCreateVisPointsQuery();
PThandle PTAPI ptCreateFrustumPointsQuery();
PThandle PTAPI ptCreatePlaneQuery( PTdouble planeX, PTdouble planeY, PTdouble planeZ, PTdouble planeK, PTdouble thickness );
PThandle PTAPI ptCreatePolygonQuery( PTint numVertices, PTdouble *vertices, PTdouble thickness );
PThandle PTAPI ptCreateBoundingBoxQuery( PTdouble minx, PTdouble miny, PTdouble minz, PTdouble maxx, PTdouble maxy, PTdouble maxz );
PThandle PTAPI ptCreateOrientedBoundingBoxQuery( PTdouble minx, PTdouble miny, PTdouble minz, PTdouble maxx, PTdouble maxy, PTdouble maxz, PTdouble posx, PTdouble posy, PTdouble posz, PTdouble ux, PTdouble uy, PTdouble uz, PTdouble vx, PTdouble vy, PTdouble vz);
PThandle PTAPI ptCreateBoundingSphereQuery( PTdouble *cen, PTdouble radius );
PThandle PTAPI ptCreateKrigSurfaceQuery( PTuint numKrigPoints, PTdouble *krigPnts );
PThandle PTAPI ptCreateKNNQuery(PTfloat *vertices, PTint numQueryVertices, PTint k, PTfloat queryLOD);

PTbool PTAPI ptDeleteQuery( PThandle query );
PTbool PTAPI ptResetQuery( PThandle query );

PTres PTAPI ptSetQueryRGBMode( PThandle query, PTenum mode );
PTres PTAPI ptSetQueryDensity( PThandle query, PTenum densityType, PTfloat densityValue );
PTres PTAPI ptSetQueryScope( PThandle query, PThandle sceneOrCloudHandle );
PTres PTAPI ptSetQueryLayerMask( PThandle query, PTubyte layerMask );

PTuint PTAPI ptGetQueryPointsd( PThandle query, PTuint bufferSize, PTdouble *geomBuffer, PTubyte *rgbBuffer, 
							   PTshort *intensityBuffer, PTubyte *selectionBuffer, PTubyte *classificationBuffer );

PTuint PTAPI ptGetDetailedQueryPointsd( PThandle query, PTuint bufferSize, PTdouble *geomBuffer, PTubyte *rgbBuffer, 
							   PTshort *intensityBuffer, PTfloat *normalBuffer, PTubyte *selectionBuffer, PTubyte *classificationBuffer,
							   PTuint numPointChannels, const PThandle *pointChannelsReq, PTvoid **pointChannels );

PTuint PTAPI ptGetQueryPointsf( PThandle query, PTuint bufferSize, PTfloat *geomBuffer, PTubyte *rgbBuffer, 
							   PTshort *intensityBuffer, PTubyte *selectionBuffer, PTubyte *classificationBuffer);

PTuint PTAPI ptGetDetailedQueryPointsf( PThandle query, PTuint bufferSize, PTfloat *geomBuffer, PTubyte *rgbBuffer, 
							   PTshort *intensityBuffer, PTfloat *normalBuffer, PTubyte *filter, PTubyte *classificationBuffer,
							   PTuint numPointChannels, const PThandle *pointChannelsReq, PTvoid **pointChannels );

PTuint PTAPI ptGetQueryPointsMultif( PThandle query, PTuint numResultSets, PTuint buffersize, PTuint *resultSetSize, PTfloat **geomBufferArray, 
							   PTubyte **rgbBufferArray, PTshort **intensityBufferArray, PTubyte **selectionBufferArray );

PTuint PTAPI ptGetQueryPointsMultid( PThandle query, PTuint numResultSets, PTuint buffersize, PTuint *resultSetSize, PTdouble **geomBufferArray, 
							   PTubyte **rgbBufferArray, PTshort **intensityBufferArray, PTubyte **selectionBufferArray );

/* interaction */ 
PTvoid	PTAPI ptFlipMouseYCoords( void );
PTvoid	PTAPI ptDontFlipMouseYCoords( void );

/* tuning and memory management */ 
PTvoid	PTAPI ptSetCacheSizeMb( PTuint mb );
PTuint	PTAPI ptGetCacheSizeMb();
PTvoid	PTAPI ptAutoCacheSize();
PTres	PTAPI ptSetLoadingPriorityBias( PTenum bias );
PTenum	PTAPI ptGetLoadingPriorityBias();
PTres	PTAPI ptSetTuningParameterfv( PTenum param, const PTfloat *values );
PTres	PTAPI ptGetTuningParameterfv( PTenum param, PTfloat *values );

/* image generation : Warning not implemented, will assert */ 
PThandle	PTAPI ptCreateImageBuffer( PTint width, PTint height, PTubyte *buffer );
PTbool		PTAPI ptBindImageBuffer( PThandle );
PTbool		PTAPI ptGeneratePolygonImage( PTdouble *fencePoints, PTuint numPoints, PTdouble thickness );
PTbool		PTAPI ptGenerateViewportImage( void );
PTuint		PTAPI ptGetImageBufferHBITMAP( PThandle image );
PTvoid		PTAPI ptDeleteImage( PThandle image );

/* User data channel */ 
PThandle	PTAPI ptCreatePointChannel( PTstr name, PTenum typesize, PTuint multiple, void* default_value, PTuint flags );
PThandle	PTAPI ptCopyPointChannel(PThandle channel, PTstr destName, PTuint destFlags);
PTres		PTAPI ptGetChannelInfo(PThandle handle, PTstr name, PTenum& typesize, PTuint& multiple, void *defaultValue, PTuint& flags);
PTres		PTAPI ptDeletePointChannel( PThandle channel );
PTres		PTAPI ptSubmitPointChannelUpdate( PThandle query, PThandle channel );
PTres		PTAPI ptWriteChannelsFile( const PTstr filename, PTint numChannels, const PThandle *channels );
PTuint64	PTAPI ptWriteChannelsFileToBuffer(PTint numChannels, const PThandle *channels, PTubyte *&buffer, PTuint64 &bufferSize);
PTvoid		PTAPI ptReleaseChannelsFileBuffer(PTuint64 bufferHandle);
PTres		PTAPI ptReadChannelsFile( const PTstr filename, PTint &numChannels, const PThandle **channels );
PTres		PTAPI ptReadChannelsFileFromBuffer(void *buffer, PTuint64 bufferSize, PTint &numChannels, const PThandle **channels);
PTres		PTAPI ptDrawPointChannelAs( PThandle channel, PTenum method, PTfloat param1, PTfloat param2 );
PTres		PTAPI ptSetChannelOOCFolder( const PTstr foldername );
PThandle	PTAPI ptGetChannelByName( const PTstr channelname );
PTvoid		PTAPI ptDeleteAllChannels( void );

PThandle	PTAPI ptCreatePointChannelFromLayers( PTstr name, PThandle sceneHandle );
PTbool		PTAPI ptLayersFromPointChannel( PThandle userChannel, PThandle sceneHandle );

/* Clipping planes */
PTvoid		PTAPI ptEnableClipping( void );
PTvoid		PTAPI ptDisableClipping( void );
PTres		PTAPI ptSetClipStyle( PTuint style );
PTuint		PTAPI ptGetNumClippingPlanes( void );
PTbool		PTAPI ptIsClippingPlaneEnabled( PTuint id );
PTres		PTAPI ptEnableClippingPlane( PTuint id );
PTres		PTAPI ptDisableClippingPlane( PTuint id );
PTres		PTAPI ptSetClippingPlaneParameters( PTuint id, PTdouble a, PTdouble b, PTdouble c, PTdouble d );

/* unit tests */ 
PTbool	PTAPI _ptUnitTests( PTenum test );
PTres	PTAPI _ptDiagnostic( PTvoid *data );
#endif