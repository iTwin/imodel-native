//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicAPI/Vortex/VortexAPI.h $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

#include  <cstdint>
#include "VortexErrors.h"
#include "ExportMacros.h"

//NEEDS_WORK_VORTEX_DGNDB &&MM - split types and C export?
//     - Add a namespace
//     - Use __PUBLISH_SECTION_START__ and __PUBLISH_SECTION_END__ if relevant

typedef uint32_t        PTenum;
typedef bool            PTbool;
typedef int32_t         PTint;
typedef int32_t         PTres;
typedef uint32_t        PTuint;
typedef float           PTfloat;
typedef double          PTdouble;
typedef short           PTshort;
typedef unsigned short  PTushort;
typedef uint64_t        PTuint64;
typedef int64_t         PTint64;
typedef void            PTvoid;
typedef PTuint          PThandle;
typedef char            PTbyte;
typedef unsigned char   PTubyte;

/* All strings are WCHAR, ie 2bytes per char */
#define PTstr wchar_t*

/* Client Server */
#define PT_CLIENT_SERVER_CACHE_NAME_MODE_GUID            1
#define PT_CLIENT_SERVER_CACHE_NAME_MODE_SERVER_PATH    2

/* Shader Enables */
#define PT_RGB_SHADER                    0x01
#define PT_INTENSITY_SHADER                0x02
#define PT_BLENDING_SHADER                0x03
#define PT_PLANE_SHADER                    0x04
#define PT_LIGHTING                        0x05
#define PT_CLIPPING                        0x06
#define PT_CHANNEL_RENDER                0x07

/* Display Enables */
#define PT_ADAPTIVE_POINT_SIZE            0X100
#define PT_FRONT_BIAS                    0X101
#define PT_DELAYED_CHANNEL_LOAD            0X102

/* Shader Settings */
#define PT_PLANE_SHADER_DISTANCE        0x11    
#define PT_PLANE_SHADER_VECTOR            0x12
#define PT_PLANE_SHADER_OFFSET            0x13

#define PT_INTENSITY_SHADER_CONTRAST    0x14
#define PT_INTENSITY_SHADER_BRIGHTNESS    0x15

#define PT_RGB_SHADER_CONTRAST            0x16
#define PT_RGB_SHADER_BRIGHTNESS        0x17

#define PT_LIGHT_VECTOR                    0x18
#define PT_LIGHT_ANGLE                    0x19
#define PT_LIGHT_COLOUR                    0x1a
#define PT_LIGHT_AMBIENT_COLOUR            0x1b
#define PT_LIGHT_DIFFUSE_COLOUR            0x1c
#define PT_LIGHT_SPECULAR_COLOUR        0x1d
#define PT_LIGHT_STRENGTH                0x1f
#define PT_LIGHT_AMBIENT_STRENGTH        0x20
#define PT_LIGHT_DIFFUSE_STRENGTH        0x21
#define PT_LIGHT_SPECULAR_STRENGTH        0x22

#define PT_INTENSITY_SHADER_RAMP        0x23
#define PT_PLANE_SHADER_RAMP            0x24

#define PT_MATERIAL_AMBIENT                0X25
#define PT_MATERIAL_DIFFUSE                0x26
#define PT_MATERIAL_SPECULAR            0x27
#define PT_MATERIAL_GLOSSINESS            0x28

#define PT_PLANE_SHADER_EDGE            0x29

/* texture edge */
#define PT_EDGE_REPEAT                    0x00
#define PT_EDGE_CLAMP                    0x01
#define PT_EDGE_BLACK                    0x02
#define PT_EDGE_MIRROR                    0x03

/* units */
#define PT_METERS                        0x100
#define PT_DECIMETERS                    0x101
#define PT_CENTIMETERS                    0x102
#define PT_MILLIMETERS                    0x103
#define PT_FEET                            0x104
#define PT_FEET_US                        0x106
#define PT_INCHES                        0x105

/* draw modes */
#define PT_DRAW_MODE_STATIC                0x01
#define PT_DRAW_MODE_INTERACTIVE        0x02
#define PT_DRAW_MODE_DEFAULT            0x00
#define PT_DRAW_MODE_COMPATIBILITY        0x04

/* selection modes */
#define PT_SELECT                        0x01        
#define PT_DESELECT                        0x02
//#define PT_SELECT_TOGGLE                0x03

#define PT_MAX_VIEWPORTS        256

/* point attributes */
#define PT_HAS_INTENSITY        0x01
#define PT_HAS_RGB                0x02
#define PT_HAS_NORMAL            0x04
#define PT_HAS_FILTER            0x08
#define PT_HAS_CLASSIFICATION    0x10


/* ramps */
#define PT_INTENSITY_RAMP_TYPE    0x01
#define PT_PLANE_RAMP_TYPE        0x02

/* coordinate base */
#define PT_AUTO_BASE_DISABLED    0x0
#define PT_AUTO_BASE_CENTER        0x01
#define PT_AUTO_BASE_REDUCE        0x02
#define PT_AUTO_BASE_FIRST_ONLY 0x04

/* editing */
#define PT_EDIT_MODE_SELECT        0X01
#define PT_EDIT_MODE_UNSELECT    0X02
#define PT_EDIT_MODE_UNHIDE        0X03
#define PT_EDIT_MAX_LAYERS        6

/* editing mode */
#define PT_EDIT_WORK_ON_ALL            0x01
#define PT_EDIT_WORK_ON_VIEW        0x02
#define PT_EDIT_WORK_ON_PROPORTION    0x03

/* per point filter */
#define PT_EDIT_PNT_SELECTED        0x80
#define PT_EDIT_PNT_LYR1            0x01
#define PT_EDIT_PNT_LYR2            0x02
#define PT_EDIT_PNT_LYR3            0x03
#define PT_EDIT_PNT_LYR4            0x04
#define PT_EDIT_PNT_LYR5            0x05
#define PT_EDIT_PNT_LYR6            0x06
#define PT_EDIT_PNT_LYR7            0x07

/* query */
#define PT_QUERY_DENSITY_FULL            0x01
#define    PT_QUERY_DENSITY_VIEW            0X02
#define PT_QUERY_DENSITY_LIMIT            0X03
#define    PT_QUERY_DENSITY_VIEW_COMPLETE    0X04
#define PT_QUERY_DENSITY_SPATIAL        0x07

#define PT_QUERY_RGB_MODE_ACTUAL            0x04
#define PT_QUERY_RGB_MODE_SHADER            0x05
#define PT_QUERY_RGB_MODE_SHADER_NO_SELECT  0x06

/* tuning */
#define PT_LOADING_BIAS_SCREEN    0x01
#define PT_LOADING_BIAS_NEAR    0x02
#define PT_LOADING_BIAS_FAR        0x03
#define PT_LOADING_BIAS_POINT    0x04

/* eye perspective type */
#define PT_PROJ_PERSPECTIVE_GL        0x01
#define PT_PROJ_PERSPECTIVE_DX        0x02
#define PT_PROJ_PERSPECTIVE_BLINN    0x03

/* channel constants */
/* draw as */
#define PT_CHANNEL_AS_OFFSET    0x01
#define PT_CHANNEL_AS_RAMP        0x02
#define PT_CHANNEL_AS_ZSHIFT    0x03
#define PT_CHANNEL_AS_RGB        0x04

/* options */
#define PT_CHANNEL_OUT_OF_CORE    0X01

/* meta data */
#define PT_MAX_META_STR_LEN            1024

/* generic */
#define PT_TRUE    true
#define PT_FALSE false
#define PT_NULL 0

/* Viewport context types */
#define PT_GL_VIEWPORT                0X01
#define PT_DX_VIEWPORT                0X02
#define PT_SW_VIEWPORT                0X03

/* clipping options */
#define PT_CLIP_OUTSIDE                0x01
#define PT_CLIP_INSIDE                0x02

//NEEDS_WORK_VORTEX_DGNDB &&MM I think we should create another file for that ex: VortexCApi.h or something.
BEGIN_EXTERN_C

/* initialisation */
VORTEX_EXPORT PTbool      ptInitialize(const PTubyte *license);
VORTEX_EXPORT PTbool      ptIsInitialized();
VORTEX_EXPORT PTvoid      ptSetWorkingFolder(const PTstr folder);
VORTEX_EXPORT const PTstr ptGetWorkingFolder(void);
VORTEX_EXPORT const PTstr ptGetVersionString(void);
VORTEX_EXPORT PTvoid      ptGetVersionNum(PTubyte *version);
VORTEX_EXPORT PTvoid      ptRelease(void);

/* Client Server */
#if NEEDS_WORK_VORTEX_DGNDB
extern PTCREATEFAKEPOD                            ptCreateFakePOD;
extern PTSETSERVERCALLBACK                        ptSetServerCallBack;
extern PTSETRELEASECLIENTSERVERBUFFERCALLBACK    ptSetReleaseClientServerBufferCallBack;
extern PTPROCESSSERVERREQUEST                    ptProcessServerRequest;
extern PTPROCESSSERVERREQUESTCLIENTID            ptProcessServerRequestClientID;
extern PTPROCESSSERVERREQUESTCLIENTID2            ptProcessServerRequestClientID2;

extern PTSETCLIENTSERVERLOGFILE                    ptSetClientServerLogFile;
extern PTSETCLIENTSERVERSENDRETRIES                ptSetClientServerSendRetries;
extern PTGETCLIENTSERVERSENDRETRIES                ptGetClientServerSendRetries;
extern PTSETCLIENTSTREAMING                        ptSetClientStreaming;
extern PTSERVERCLIENTLOST                        ptServerClientLost;
extern PTGETSESSIONID                            ptGetSessionID;

/* Client Server Caching */
extern PTSETCLIENTCACHEFOLDER                    ptSetClientCacheFolder;
extern PTGETCLIENTCACHEFOLDER                    ptGetClientCacheFolder;
extern PTENABLECLIENTSERVERCACHING                ptEnableClientServerCaching;
extern PTGETCLIENTSERVERCACHINGENABLED            ptGetClientServerCachingEnabled;
extern PTSETCLIENTSERVERCACHEDATASIZE            ptSetClientServerCacheDataSize;
extern PTGETCLIENTSERVERCACHEDATASIZE            ptGetClientServerCacheDataSize;
extern PTSETCLIENTCACHECOMPLETIONTHRESHOLD        ptSetClientCacheCompletionThreshold;
extern PTGETCLIENTCACHECOMPLETIONTHRESHOLD        ptGetClientCacheCompletionThreshold;
extern PTSETCLIENTSERVERCACHENAMEMODE            ptSetClientServerCacheNameMode;
extern PTGETCLIENTSERVERCACHENAMEMODE            ptGetClientServerCacheNameMode;
#endif

/* file */
VORTEX_EXPORT PThandle     ptOpenPOD(const PTstr filepath);
VORTEX_EXPORT PThandle     ptOpenPODStructuredStorageStream(const PTstr filepath, PTvoid *stream);
VORTEX_EXPORT PThandle     ptIsOpen(const PTstr filepath);
VORTEX_EXPORT PThandle     ptBrowseAndOpenPOD(void);

VORTEX_EXPORT PTres     ptUnloadScene(PThandle scene);
VORTEX_EXPORT PTres     ptReloadScene(PThandle scene);
VORTEX_EXPORT PTres     ptRemoveScene(PThandle scene);
VORTEX_EXPORT PTvoid    ptRemoveAll();

/* scene info */
VORTEX_EXPORT PThandle     ptGetCloudHandleByIndex(PThandle scene, PTuint cloud_index);
VORTEX_EXPORT PTuint         ptGetNumCloudsInScene(PThandle scene);

VORTEX_EXPORT PTint     ptNumScenes(void);
VORTEX_EXPORT PTint     ptGetSceneHandles(PThandle *handles);
VORTEX_EXPORT PTbool     ptSceneInfo(PThandle scene, PTstr name, PTint &clouds, PTuint &num_points, PTuint &specification, PTbool &loaded, PTbool &visible);
VORTEX_EXPORT const PTstr     ptSceneFile(PThandle scene);
VORTEX_EXPORT PTres     ptCloudInfo(PThandle cloud, PTstr name, PTuint &num_points, PTuint &specification, PTbool &visible);

/* meta data */
VORTEX_EXPORT PThandle        ptReadPODMeta(const PTstr filepath);
VORTEX_EXPORT PThandle        ptGetMetaDataHandle(PThandle sceneHandle);
VORTEX_EXPORT PTres            ptGetMetaData(PThandle metadataHandle, PTstr name, PTint &num_clouds, PTuint64 &num_points, PTuint &scene_spec, PTdouble *lower3, PTdouble *upper3);
VORTEX_EXPORT PTres            ptGetMetaTag(PThandle metadataHandle, const PTstr tagName, PTstr value);
VORTEX_EXPORT PTres		       ptSetMetaTag(PThandle metadataHandle, const PTstr tagName, const PTstr value);
VORTEX_EXPORT PTres            ptWriteMetaTags(PThandle metadataHandle);
VORTEX_EXPORT PTvoid            ptFreeMetaData(PThandle metadataHandle);

/* user metatags */
VORTEX_EXPORT PTint            ptNumUserMetaSections(PThandle metadataHandle);
VORTEX_EXPORT const PTstr        ptUserMetaSectionName(PThandle metadataHandle, PTint section_index);

VORTEX_EXPORT PTint            ptNumUserMetaTagsInSection(PThandle metadataHandle, PTint section_index);
VORTEX_EXPORT PTres            ptGetUserMetaTagByIndex(PThandle metadataHandle, PTint section_index, PTint tag_index, PTstr name, PTstr value);
VORTEX_EXPORT PTres            ptGetUserMetaTagByName(PThandle metadataHandle, const PTstr sectionDotName, PTstr value);

/* scene instancing */
VORTEX_EXPORT PThandle        ptCreateSceneInstance(PThandle scene);

/* transformation */
VORTEX_EXPORT PTres            ptSetCloudTransform(PThandle cloud, const PTdouble *transform4x4, bool row_order);
VORTEX_EXPORT PTres            ptSetSceneTransform(PThandle scene, const PTdouble *transform4x4, bool row_order);
VORTEX_EXPORT PTres            ptGetCloudTransform(PThandle cloud, PTdouble *transform4x4, bool row_order);
VORTEX_EXPORT PTres            ptGetSceneTransform(PThandle scene, PTdouble *transform4x4, bool row_order);

/* bounds info */
VORTEX_EXPORT PTres     ptLayerBounds(PTuint layer, PTfloat *lower3, PTfloat *upper3, bool approx_fast);
VORTEX_EXPORT PTres     ptLayerBoundsd(PTuint layer, PTdouble *lower3, PTdouble *upper3, bool approx_fast);

VORTEX_EXPORT PTres     ptSceneBounds(PThandle scene, PTfloat *lower3, PTfloat *upper3);
VORTEX_EXPORT PTres     ptSceneBoundsd(PThandle scene, PTdouble *lower3, PTdouble *upper3);
VORTEX_EXPORT PTres     ptCloudBounds(PThandle cloud, PTfloat *lower3, PTfloat *upper3);
VORTEX_EXPORT PTres     ptCloudBoundsd(PThandle cloud, PTdouble *lower3, PTdouble *upper3);

VORTEX_EXPORT PTbool     ptGetLowerBound(PTdouble *lower);
VORTEX_EXPORT PTbool     ptGetUpperBound(PTdouble *upper);

/* coordinate system */
VORTEX_EXPORT PTvoid  ptNormaliseCoordinateSystem(void);
VORTEX_EXPORT PTvoid  ptProjectToWorldCoords(PTdouble *v);
VORTEX_EXPORT PTvoid  ptWorldToProjectCoords(PTdouble *v);
VORTEX_EXPORT PTvoid  ptSetAutoBaseMethod(PTenum type);
VORTEX_EXPORT PTenum  ptGetAutoBaseMethod(void);
VORTEX_EXPORT PTvoid  ptGetCoordinateBase(PTdouble *coordinateBase);
VORTEX_EXPORT PTvoid  ptSetCoordinateBase(PTdouble *coordinateBase);

/* error handling */
VORTEX_EXPORT PTstr     ptGetLastErrorString(void);
VORTEX_EXPORT PTres     ptGetLastErrorCode(void);

/* display */
VORTEX_EXPORT PTres     ptShowScene(PThandle scene, PTbool visible);
VORTEX_EXPORT PTres     ptShowCloud(PThandle cloud, PTbool visible);

VORTEX_EXPORT PTbool  ptIsSceneVisible(PThandle scene);
VORTEX_EXPORT PTbool  ptIsCloudVisible(PThandle cloud);

/* draw */
VORTEX_EXPORT PTvoid     ptOverrideDrawMode(PTenum mode);
VORTEX_EXPORT PTvoid     ptDrawGL(PTbool dynamic);
VORTEX_EXPORT PTvoid     ptDrawSceneGL(PThandle scene, PTbool dynamic);
VORTEX_EXPORT PTvoid     ptDrawInteractiveGL();

VORTEX_EXPORT PTuint     ptKbLoaded(PTbool reset);
VORTEX_EXPORT PTuint     ptWeightedPtsLoaded(PTbool reset);
VORTEX_EXPORT PTint64    ptPtsLoadedInViewportSinceDraw(PThandle forScene);
VORTEX_EXPORT PTint64    ptPtsToLoadInViewport(PThandle forScene, PTbool reCompute);
VORTEX_EXPORT PTvoid     ptEndDrawFrameMetrics();
VORTEX_EXPORT PTvoid     ptStartDrawFrameMetrics();

VORTEX_EXPORT PTres  ptSetQueryScope(PThandle query, PThandle sceneOrCloudHandle);
VORTEX_EXPORT PTres  ptSetQueryLayerMask(PThandle query, PTubyte layerMask);

VORTEX_EXPORT PTbool  ptResetQuery(PThandle query);

/* units */
VORTEX_EXPORT PTvoid     ptSetHostUnits(PTenum units);
VORTEX_EXPORT PTenum     ptGetHostUnits(void);

/* enable / disable */
VORTEX_EXPORT PTvoid     ptEnable(PTenum option);
VORTEX_EXPORT PTvoid     ptDisable(PTenum option);
VORTEX_EXPORT PTbool     ptIsEnabled(PTenum option);

/* lighting */
VORTEX_EXPORT PTres     ptLightOptionf(PTenum Light_option, PTfloat value);
VORTEX_EXPORT PTres     ptLightOptionfv(PTenum Light_option, PTfloat *value);
VORTEX_EXPORT PTres     ptLightOptioni(PTenum Light_option, PTint value);

VORTEX_EXPORT PTres     ptGetLightOptionf(PTenum Light_option, PTfloat *value);
VORTEX_EXPORT PTres     ptGetLightOptioni(PTenum Light_option, PTint *value);
VORTEX_EXPORT PTvoid    ptCopyLightSettings(PTuint dest_viewport);
VORTEX_EXPORT PTvoid    ptCopyLightSettingsToAll();
VORTEX_EXPORT PTvoid    ptResetLightOptions();

/* shader options */
VORTEX_EXPORT PTres     ptPointSize(PTfloat size);

VORTEX_EXPORT PTres     ptShaderOptionf(PTenum shader_option, PTfloat value);
VORTEX_EXPORT PTres     ptShaderOptionfv(PTenum shader_option, PTfloat *value);
VORTEX_EXPORT PTres     ptShaderOptioni(PTenum shader_option, PTint value);

VORTEX_EXPORT PTres     ptGetShaderOptionf(PTenum shader_option, PTfloat *value);
VORTEX_EXPORT PTres     ptGetShaderOptionfv(PTenum shader_option, PTfloat *values);
VORTEX_EXPORT PTres     ptGetShaderOptioni(PTenum shader_option, PTint *value);
VORTEX_EXPORT PTvoid    ptResetShaderOptions(void);

VORTEX_EXPORT PTvoid    ptCopyShaderSettings(PTuint dest_viewport);
VORTEX_EXPORT PTvoid    ptCopyShaderSettingsToAll(void);

VORTEX_EXPORT PTres     ptSetOverrideColor(PThandle cloud_or_scene, const PTfloat *rgb3);
VORTEX_EXPORT PTres     ptGetOverrideColor(PThandle cloud_or_scene, PTfloat *rgb3);
VORTEX_EXPORT PTres     ptRemoveOverrideColor(PThandle cloud_or_scene);

/* shading ramps */
VORTEX_EXPORT PTint     ptNumRamps(void);
VORTEX_EXPORT const PTstr  ptRampInfo(PTint ramp, PTenum *type);
VORTEX_EXPORT PTres     ptAddCustomRamp(const PTstr name, PTint numKeys, const PTfloat *positions,
                                      const PTubyte* colour3vals, PTbool interpolateInHSL);

/* viewports */
VORTEX_EXPORT PTint      ptAddViewport(PTint index, const PTstr name, PTenum contextType);
VORTEX_EXPORT PTvoid     ptRemoveViewport(PTint index);
VORTEX_EXPORT PTvoid     ptSetViewport(PTint index);
VORTEX_EXPORT PTint      ptSetViewportByName(const PTstr name);
VORTEX_EXPORT PTint      ptViewportIndexFromName(const PTstr name);

VORTEX_EXPORT PTvoid     ptCaptureViewportInfo(void);
VORTEX_EXPORT PTvoid     ptStoreView(void);
VORTEX_EXPORT PTint      ptCurrentViewport(void);

VORTEX_EXPORT PTvoid*    ptCreateBitmapViewport(int w, int h, const PTstr name);
VORTEX_EXPORT PTvoid     ptDestroyBitmapViewport(const PTstr name);

VORTEX_EXPORT PTvoid     ptEnableViewport(PTint index);
VORTEX_EXPORT PTvoid     ptDisableViewport(PTint index);
VORTEX_EXPORT PTbool     ptIsViewportEnabled(PTint index);
VORTEX_EXPORT PTbool     ptIsCurrentViewportEnabled(void);

/* persistence of viewport setup */
VORTEX_EXPORT PTuint     ptGetPerViewportDataSize();
VORTEX_EXPORT PTuint     ptGetPerViewportData(PTubyte *data);
VORTEX_EXPORT PTres      ptSetPerViewportData(const PTubyte *data);

VORTEX_EXPORT PTvoid     ptSetViewportPointsBudget(PTint budget);
VORTEX_EXPORT PTint      ptGetViewportPointsBudget(void);

/* view */
VORTEX_EXPORT PTbool     ptReadViewFromGL(void);

VORTEX_EXPORT PTvoid     ptSetViewProjectionOrtho(PTdouble l, PTdouble r, PTdouble b, PTdouble t, PTdouble n, PTdouble f);
VORTEX_EXPORT PTvoid     ptSetViewProjectionFrustum(PTdouble l, PTdouble r, PTdouble b, PTdouble t, PTdouble n, PTdouble f);
VORTEX_EXPORT PTvoid     ptSetViewProjectionMatrix(const PTdouble *matrix, bool row_major);
VORTEX_EXPORT PTvoid     ptSetViewProjectionPerspective(PTenum type, PTdouble fov, PTdouble aspect, PTdouble n, PTdouble f);

VORTEX_EXPORT PTvoid     ptSetViewEyeLookAt(const PTdouble *eye3, const PTdouble *target3, const PTdouble *up3);
VORTEX_EXPORT PTvoid	 ptSetViewEyeMatrix(PTdouble const *matrix, bool row_major);

VORTEX_EXPORT PTvoid     ptSetViewportSize(PTint left, PTint bottom, PTuint width, PTuint height);

VORTEX_EXPORT PTvoid  ptGetViewEyeMatrix(PTdouble *matrix);
VORTEX_EXPORT PTvoid  ptGetViewProjectionMatrix(PTdouble *matrix16);

/* editing */
VORTEX_EXPORT PTres     ptSetSelectPointsMode(PTenum select_mode);
VORTEX_EXPORT PTenum    ptGetSelectPointsMode(void);
VORTEX_EXPORT PTvoid    ptSelectPointsByRect(PTint x_edge, PTint y_edge, PTint width, PTint height);
VORTEX_EXPORT PTres     ptSelectPointsByFence(PTint num_vertices, const PTint *vertices);
VORTEX_EXPORT PTres     ptSelectPointsByBox(const PTdouble *lower, const PTdouble *upper);
VORTEX_EXPORT PTres     ptSelectPointsByOrientedBox(const PTdouble *lower, const PTdouble *upper, const PTdouble *pos, PTdouble *uAxis, PTdouble *vAxis);
VORTEX_EXPORT PTres     ptSelectPointsByCube(const PTdouble *centre, PTdouble radius);
VORTEX_EXPORT PTres     ptSelectPointsByPlane(const PTdouble *origin, const PTdouble *normal, PTdouble thickness);
VORTEX_EXPORT PTres     ptSelectPointsBySphere(const PTdouble *centre, PTdouble radius);

VORTEX_EXPORT PTvoid     ptHideSelected(void);
VORTEX_EXPORT PTvoid     ptIsolateSelected(void);
VORTEX_EXPORT PTvoid     ptUnhideAll(void);
VORTEX_EXPORT PTvoid     ptUnselectAll(void);
VORTEX_EXPORT PTvoid     ptResetSelection(void);
VORTEX_EXPORT PTvoid     ptSelectAll(void);

VORTEX_EXPORT PTuint64  _ptCountVisiblePoints(void);

VORTEX_EXPORT PTvoid     ptSetSelectionScope(PThandle sceneOrCloudHandle);
VORTEX_EXPORT PTvoid     ptSetSelectionDrawColor(const PTubyte *col3);
VORTEX_EXPORT PTvoid     ptGetSelectionDrawColor(PTubyte *col3);

VORTEX_EXPORT PTvoid   ptInvertSelection(void);
VORTEX_EXPORT PTvoid   ptInvertVisibility(void);

VORTEX_EXPORT PTres      ptResetSceneEditing(PThandle scene);
VORTEX_EXPORT PTvoid     ptRefreshEdit(void);
VORTEX_EXPORT PTvoid     ptClearEdit(void);

VORTEX_EXPORT PTvoid     ptStoreEdit(const PTstr name);
VORTEX_EXPORT PTbool     ptRestoreEdit(const PTstr name);
VORTEX_EXPORT PTbool     ptRestoreEditByIndex(PTint index);
VORTEX_EXPORT PTint      ptNumEdits(void);
VORTEX_EXPORT const PTstr ptEditName(PTint index);

VORTEX_EXPORT PTbool     ptDeleteEdit(const PTstr name);
VORTEX_EXPORT PTbool     ptDeleteEditByIndex(PTint index);
VORTEX_EXPORT PTvoid     ptDeleteAllEdits(void);

VORTEX_EXPORT PTint  ptGetEditData(PTint index, PTubyte *data);
VORTEX_EXPORT PTint  ptGetEditDataSize(PTint index);
VORTEX_EXPORT PTvoid ptCreateEditFromData(const PTubyte *data);

VORTEX_EXPORT PTvoid     ptSetEditWorkingMode(PTenum mode);
VORTEX_EXPORT PTenum     ptGetEditWorkingMode(void);

VORTEX_EXPORT PTvoid  ptSelectPointsInLayer(PTuint layer);
VORTEX_EXPORT PTvoid  ptDeselectPointsInLayer(PTuint layer);

/* editing options */
VORTEX_EXPORT PTres     ptSelectCloud(PThandle cloud);
VORTEX_EXPORT PTres     ptDeselectCloud(PThandle cloud);
VORTEX_EXPORT PTres     ptSelectScene(PThandle scene);
VORTEX_EXPORT PTres     ptDeselectScene(PThandle scene);


/* visualisation settings */
VORTEX_EXPORT PTvoid     ptDynamicFrameRate(PTfloat fps);
VORTEX_EXPORT PTfloat    ptGetDynamicFrameRate();

VORTEX_EXPORT PTvoid     ptStaticOptimizer(PTfloat opt);
VORTEX_EXPORT PTfloat    ptGetStaticOptimizer();

VORTEX_EXPORT PTvoid    ptGlobalDensity(PTfloat opt);
VORTEX_EXPORT PTfloat   ptGetGlobalDensity(void);

/* screen based point */
VORTEX_EXPORT PTint     ptFindNearestScreenPoint(PThandle scene, PTint screenx, PTint screeny, PTdouble *pnt);
VORTEX_EXPORT PTint     ptFindNearestScreenPointWDepth(PThandle scene, PTint screenx, PTint screeny, PTfloat *dpArray4x4, PTdouble *pnt);

VORTEX_EXPORT PTfloat   ptFindNearestPoint(PThandle scene, const PTdouble *pnt, PTdouble *nearest);

/* interaction */
VORTEX_EXPORT PTvoid     ptFlipMouseYCoords(void);
VORTEX_EXPORT PTvoid     ptDontFlipMouseYCoords(void);

/* ray intersection */
VORTEX_EXPORT PTres      ptSetIntersectionRadius(PTfloat radius);
VORTEX_EXPORT PTfloat    ptGetIntersectionRadius(void);
VORTEX_EXPORT PTbool     ptIntersectRay(PThandle scene, const PTdouble *origin, const PTdouble *direction, PTdouble *intersection, PTenum densityType, PTfloat densityValue);
VORTEX_EXPORT PTbool     ptIntersectRayPntIndex(PThandle scene, const PTdouble *origin, const PTdouble *direction, PThandle *cloud, PThandle *pntPartA, PThandle *pntPartB);
VORTEX_EXPORT PTbool     ptIntersectRayInterpolated(PThandle scene, const PTdouble *origin, const PTdouble *direction, PThandle *tmpPointHandle);

/* point data access */
VORTEX_EXPORT PTuint     ptGetCloudProxyPoints(PThandle scene, PTint num_points, PTfloat *pnts, PTubyte *col);
VORTEX_EXPORT PTuint     ptGetSceneProxyPoints(PThandle cloud, PTint num_points, PTfloat *pnts, PTubyte *col);

VORTEX_EXPORT PTbool    ptPointData(PThandle cloud, PThandle pntPartA, PThandle pntPartB, PTdouble *position, PTshort *intensity, PTubyte *rgb, PTfloat *normal);

VORTEX_EXPORT PTuint     ptPointAttributes(PThandle cloud, PThandle pntPartA, PThandle pntPartB);
VORTEX_EXPORT PTbool     ptGetPointAttribute(PThandle cloud, PThandle pntPartA, PThandle pntPartB, PTuint attribute, void* data);

VORTEX_EXPORT PThandle  ptCreateSelPointsQuery();
VORTEX_EXPORT PThandle  ptCreateVisPointsQuery();
VORTEX_EXPORT PThandle  ptCreateAllPointsQuery();
VORTEX_EXPORT PThandle  ptCreateBoundingBoxQuery(PTdouble minx, PTdouble miny, PTdouble minz, PTdouble maxx, PTdouble maxy, PTdouble maxz);
VORTEX_EXPORT PThandle  ptCreateOrientedBoundingBoxQuery(PTdouble minx, PTdouble miny, PTdouble minz, PTdouble maxx, PTdouble maxy, PTdouble maxz, PTdouble posx, PTdouble posy, PTdouble posz, PTdouble ux, PTdouble uy, PTdouble uz, PTdouble vx, PTdouble vy, PTdouble vz);
VORTEX_EXPORT PThandle  ptCreateBoundingSphereQuery(PTdouble *cen, PTdouble radius);
VORTEX_EXPORT PThandle  ptCreateKrigSurfaceQuery(PTuint numKrigPoints, PTdouble *krigPnts);

VORTEX_EXPORT PThandle  ptCreateFrustumPointsQuery();
VORTEX_EXPORT PThandle  ptCreateKNNQuery(PTfloat *vertices, PTint numQueryVertices, PTint k, PTfloat queryLOD);
VORTEX_EXPORT PTbool  ptDeleteQuery(PThandle query);

VORTEX_EXPORT PTres   ptSetQueryRGBMode(PThandle query, PTenum mode);
VORTEX_EXPORT PTres   ptSetQueryDensity(PThandle query, PTenum densityType, PTfloat densityValue);
VORTEX_EXPORT PTuint  ptGetQueryPointsd(PThandle query, PTuint bufferSize, PTdouble *geomBuffer, PTubyte *rgbBuffer, PTshort *intensityBuffer, PTubyte *selectionBuffer, PTubyte *classificationBuffer);
VORTEX_EXPORT PTuint  ptGetQueryPointsf(PThandle query, PTuint bufferSize, PTfloat *geomBuffer, PTubyte *rgbBuffer, PTshort *intensityBuffer, PTubyte *selectionBuffer, PTubyte *classificationBuffer);

VORTEX_EXPORT PTuint  ptGetDetailedQueryPointsf(PThandle query, PTuint bufferSize, PTfloat *geomBuffer, PTubyte *rgbBuffer,
                                                PTshort *intensityBuffer, PTfloat *normalBuffer, PTubyte *filter, PTubyte *classificationBuffer,
                                                PTuint numPointChannels, const PThandle *pointChannelsReq, PTvoid **pointChannels);

VORTEX_EXPORT PTuint  ptGetDetailedQueryPointsd(PThandle query, PTuint bufferSize, PTdouble *geomBuffer, PTubyte *rgbBuffer,
                                                PTshort *intensityBuffer, PTfloat *normalBuffer, PTubyte *selectionBuffer, PTubyte *classificationBuffer,
                                                PTuint numPointChannels, const PThandle *pointChannelsReq, PTvoid **pointChannels);

VORTEX_EXPORT PTuint  ptGetQueryPointsMultif(PThandle query, PTuint numResultSets, PTuint buffersize, PTuint *resultSetSize, PTfloat **geomBufferArray,
                                             PTubyte **rgbBufferArray, PTshort **intensityBufferArray, PTubyte **selectionBufferArray);

VORTEX_EXPORT PTuint  ptGetQueryPointsMultid(PThandle query, PTuint numResultSets, PTuint buffersize, PTuint *resultSetSize, PTdouble **geomBufferArray,
                                             PTubyte **rgbBufferArray, PTshort **intensityBufferArray, PTubyte **selectionBufferArray);
/* point layers */
VORTEX_EXPORT PTbool  ptSetCurrentLayer(PTuint layer);
VORTEX_EXPORT PTuint  ptGetCurrentLayer();
VORTEX_EXPORT PTbool  ptLockLayer(PTuint layer, PTbool lock);
VORTEX_EXPORT PTbool  ptIsLayerLocked(PTuint layer);
VORTEX_EXPORT PTbool  ptShowLayer(PTuint layer, PTbool show);
VORTEX_EXPORT PTbool  ptIsLayerShown(PTuint layer);
VORTEX_EXPORT PTbool  ptDoesLayerHavePoints(PTuint layer);
VORTEX_EXPORT PTvoid  ptClearPointsFromLayer(PTuint layer);
VORTEX_EXPORT PTvoid  ptResetLayers();

VORTEX_EXPORT PTbool    ptSetLayerColor(PTuint layer, PTfloat *rgb3, PTfloat blend);
VORTEX_EXPORT PTfloat * ptGetLayerColor(PTuint layer);
VORTEX_EXPORT PTfloat   ptGetLayerColorBlend(PTuint layer);
VORTEX_EXPORT PTvoid    ptResetLayerColors(void);

VORTEX_EXPORT PTbool  ptCopySelToCurrentLayer(PTbool deselect);
VORTEX_EXPORT PTbool  ptMoveSelToCurrentLayer(PTbool deselect);
VORTEX_EXPORT PTuint64  ptCountApproxPointsInLayer(PTuint layer);

/* tuning */
VORTEX_EXPORT PTvoid       ptSetCacheSizeMb(PTuint mb);
VORTEX_EXPORT PTuint       ptGetCacheSizeMb();
VORTEX_EXPORT PTvoid       ptAutoCacheSize();
VORTEX_EXPORT PTres        ptSetLoadingPriorityBias(PTenum bias);
VORTEX_EXPORT PTenum       ptGetLoadingPriorityBias();
VORTEX_EXPORT PTres        ptSetTuningParameterfv(PTenum param, const PTfloat *values);
VORTEX_EXPORT PTres        ptGetTuningParameterfv(PTenum param, PTfloat *values);

/* user point channel */
VORTEX_EXPORT PThandle     ptCreatePointChannel(PTstr name, PTenum typesize, PTuint multiple, void* default_value, PTuint flags);
VORTEX_EXPORT PThandle     ptCopyPointChannel(PThandle channel, PTstr destName, PTuint destFlags);
VORTEX_EXPORT PTres        ptDeletePointChannel(PThandle channel);
VORTEX_EXPORT PTres        ptSubmitPointChannelUpdate(PThandle query, PThandle channel);
VORTEX_EXPORT PTres        ptDrawPointChannelAs(PThandle channel, PTenum method, PTfloat param1, PTfloat param2);
VORTEX_EXPORT PTres        ptWriteChannelsFile(const PTstr filename, PTint numChannels, const PThandle *channels);

VORTEX_EXPORT PTuint64       ptWriteChannelsFileToBuffer(PTint numChannels, const PThandle *channels, PTubyte *&buffer, PTuint64 &bufferSize);
VORTEX_EXPORT PTvoid         ptReleaseChannelsFileBuffer(PTuint64 bufferHandle);
VORTEX_EXPORT PTres		     ptReadChannelsFile(const PTstr filename, PTint &numChannels, PThandle **channelHandles);
VORTEX_EXPORT PTres          ptReadChannelsFileFromBuffer(void *buffer, PTuint64 bufferSize, PTint &numChannels, PThandle **channelHandles);
VORTEX_EXPORT PTres          ptSetChannelOOCFolder(const PTstr foldername);
VORTEX_EXPORT PTvoid         ptDeleteAllChannels(void);
VORTEX_EXPORT PTres          ptGetChannelInfo(PThandle handle, PTstr name, PTenum& typesize, PTuint& multiple, void *defaultValue, PTuint& flags);
VORTEX_EXPORT PThandle       ptGetChannelByName(PTstr name);
VORTEX_EXPORT PThandle       ptCreatePointChannelFromLayers(PTstr name, PThandle sceneHandle);
VORTEX_EXPORT PTbool         ptLayersFromPointChannel(PThandle userChannel, PThandle sceneHandle);

/* Clipping planes */
VORTEX_EXPORT PTvoid        ptEnableClipping(void);
VORTEX_EXPORT PTvoid        ptDisableClipping(void);
VORTEX_EXPORT PTres         ptSetClipStyle(PTuint style);
VORTEX_EXPORT PTuint        ptGetNumClippingPlanes(void);
VORTEX_EXPORT PTbool        ptIsClippingPlaneEnabled(PTuint id);
VORTEX_EXPORT PTres         ptEnableClippingPlane(PTuint id);
VORTEX_EXPORT PTres         ptDisableClippingPlane(PTuint id);
VORTEX_EXPORT PTres         ptSetClippingPlaneParameters(PTuint id, PTdouble a, PTdouble b, PTdouble c, PTdouble d);

VORTEX_EXPORT PTres     _ptDiagnostic( PTvoid *data );

END_EXTERN_C













