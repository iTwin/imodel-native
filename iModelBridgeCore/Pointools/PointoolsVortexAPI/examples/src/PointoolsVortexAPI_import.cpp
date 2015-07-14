//----------------------------------------------------------------------------
//
// PointoolsVortexAPI_import.cpp
//
// Copyright (c) 2015 Bentley Systems, Incorporated. All rights reserved.
//
//----------------------------------------------------------------------------
#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN
#endif

#include <windows.h>
#include <stdio.h>
#include <assert.h>

#include "../include/PointoolsVortexAPI_import.h"

#ifndef PT_VORTEX_STATIC_LIB

PTINITIALIZE ptInitialize = 0; 
PTISINITIALIZED ptIsInitialized = 0;
PTSETWORKINGFOLDER ptSetWorkingFolder = 0;
PTGETWORKINGFOLDER ptGetWorkingFolder = 0;
PTGETVERSIONSTRING ptGetVersionString = 0;
PTGETVERSIONNUM ptGetVersionNum = 0;

PTCREATEFAKEPOD							ptCreateFakePOD	= 0;
PTSETRELEASECLIENTSERVERBUFFERCALLBACK	ptSetReleaseClientServerBufferCallBack = 0;
PTSETSERVERCALLBACK						ptSetServerCallBack = 0;
PTPROCESSSERVERREQUEST					ptProcessServerRequest = 0;
PTPROCESSSERVERREQUESTCLIENTID			ptProcessServerRequestClientID = 0;
PTPROCESSSERVERREQUESTCLIENTID2			ptProcessServerRequestClientID2 = 0;

PTSETCLIENTSERVERLOGFILE				ptSetClientServerLogFile = 0;	
PTSETCLIENTSERVERSENDRETRIES			ptSetClientServerSendRetries = 0;
PTGETCLIENTSERVERSENDRETRIES			ptGetClientServerSendRetries = 0;
PTSETCLIENTSTREAMING					ptSetClientStreaming = 0;
PTSERVERCLIENTLOST						ptServerClientLost = 0;
PTGETSESSIONID							ptGetSessionID = 0;

PTSETCLIENTCACHEFOLDER					ptSetClientCacheFolder = 0;	
PTGETCLIENTCACHEFOLDER					ptGetClientCacheFolder = 0;			
PTENABLECLIENTSERVERCACHING				ptEnableClientServerCaching = 0;		
PTGETCLIENTSERVERCACHINGENABLED			ptGetClientServerCachingEnabled = 0;	
PTSETCLIENTSERVERCACHEDATASIZE			ptSetClientServerCacheDataSize = 0;		
PTGETCLIENTSERVERCACHEDATASIZE			ptGetClientServerCacheDataSize = 0;		
PTSETCLIENTCACHECOMPLETIONTHRESHOLD		ptSetClientCacheCompletionThreshold = 0;
PTGETCLIENTCACHECOMPLETIONTHRESHOLD		ptGetClientCacheCompletionThreshold = 0;
PTSETCLIENTSERVERCACHENAMEMODE			ptSetClientServerCacheNameMode = 0;
PTGETCLIENTSERVERCACHENAMEMODE			ptGetClientServerCacheNameMode = 0;

PTOPENPOD ptOpenPOD = 0;
PTOPENPODSTRUCTUREDSTORAGESTREAM ptOpenPODStructuredStorageStream = 0;
PTISOPEN ptIsOpen = 0;
PTBROWSEANDOPENPOD ptBrowseAndOpenPOD = 0;

PTGETCLOUDHANDLEBYINDEX ptGetCloudHandleByIndex = 0;
PTGETNUMCLOUDSINSCENE ptGetNumCloudsInScene = 0;

PTNUMSCENES ptNumScenes = 0;
PTGETSCENEHANDLES ptGetSceneHandles = 0;
PTSCENEINFO ptSceneInfo = 0; 
PTSCENEFILE ptSceneFile = 0;
PTCLOUDINFO ptCloudInfo = 0;
PTSCENEBOUNDS ptSceneBounds = 0;
PTSCENEBOUNDSD ptSceneBoundsd = 0;
PTCLOUDBOUNDS ptCloudBounds = 0;
PTCLOUDBOUNDSD ptCloudBoundsd = 0;
PTLAYERBOUNDS ptLayerBounds = 0;
PTLAYERBOUNDSD ptLayerBoundsd = 0;

/* meta data */ 
PTREADPODMETA ptReadPODMeta=0;
PTGETMETADATAHANDLE ptGetMetaDataHandle=0;
PTGETMETADATA ptGetMetaData=0;

PTGETMETATAG ptGetMetaTag=0;
PTSETMETATAG ptSetMetaTag=0;
PTWRITEMETATAGS ptWriteMetaTags=0;
PTFREEMETADATA ptFreeMetaData=0;

PTNUMUSERMETASECTIONS ptNumUserMetaSections=0;
PTUSERMETASECTIONNAME ptUserMetaSectionName=0;

PTNUMUSERMETATAGSINSECTION ptNumUserMetaTagsInSection=0;
PTGETUSERMETATAGBYINDEX ptGetUserMetaTagByIndex=0;
PTGETUSERMETATAGBYNAME ptGetUserMetaTagByName=0;


PTSHOWSCENE ptShowScene = 0;
PTSHOWCLOUD ptShowCloud = 0;

PTISSCENEVISIBLE ptIsSceneVisible = 0;
PTISCLOUDVISIBLE ptIsCloudVisible = 0;

PTUNLOADSCENE ptUnloadScene = 0;
PTRELOADSCENE ptReloadScene = 0;

PTREMOVESCENE ptRemoveScene = 0;
PTREMOVEALL ptRemoveAll = 0;

PTOVERRIDEDRAWMODE ptOverrideDrawMode = 0;
PTDRAWGL ptDrawGL = 0;
PTDRAWSCENEGL ptDrawSceneGL = 0;
PTDRAWINTERACTIVEGL ptDrawInteractiveGL = 0;
PTKBLOADED ptKbLoaded = 0;
PTWEIGHTEDPTSLOADED ptWeightedPtsLoaded= 0;

PTPTSLOADEDINVIEWPORTSINCEDRAW ptPtsLoadedInViewportSinceDraw = 0;
PTPTSTOLOADINVIEWPORT ptPtsToLoadInViewport = 0;
PTENDDRAWFRAMEMETRICS ptEndDrawFrameMetrics = 0;
PTSTARTDRAWFRAMEMETRICS ptStartDrawFrameMetrics = 0;

PTSETHOSTUNITS ptSetHostUnits = 0;
PTGETHOSTUNITS ptGetHostUnits = 0;
PTGETCOORDINATEBASE ptGetCoordinateBase = 0;
PTSETCOORDINATEBASE ptSetCoordinateBase = 0;
PTSETAUTOBASEMETHOD ptSetAutoBaseMethod=0;
PTGETAUTOBASEMETHOD ptGetAutoBaseMethod=0;

PTGETCLOUDPROXYPOINTS ptGetCloudProxyPoints = 0;
PTGETSCENEPROXYPOINTS ptGetSceneProxyPoints = 0;

PTADDVIEWPORT ptAddViewport = 0;
PTREMOVEVIEWPORT ptRemoveViewport = 0;
PTSETVIEWPORT ptSetViewport = 0;
PTSETVIEWPORTBYNAME ptSetViewportByName = 0;
PTVIEWPORTINDEXFROMNAME ptViewportIndexFromName = 0;

PTCAPTUREVIEWPORTINFO ptCaptureViewportInfo = 0;
PTSTOREVIEW ptStoreView = 0;
PTCURRENTVIEWPORT ptCurrentViewport = 0;

PTENABLEVIEWPORT ptEnableViewport = 0;
PTDISABLEVIEWPORT ptDisableViewport = 0;
PTISVIEWPORTENABLED ptIsViewportEnabled = 0;
PTISCURRENTVIEWPORTENABLED ptIsCurrentViewportEnabled = 0;

PTGETLOWERBOUND ptGetLowerBound = 0;
PTGETUPPERBOUND ptGetUpperBound = 0;

PTENABLE ptEnable = 0;
PTDISABLE ptDisable = 0;
PTISENABLED ptIsEnabled = 0;

PTPOINTSIZE ptPointSize = 0;

PTSHADEROPTIONF ptShaderOptionf = 0;
PTSHADEROPTIONFV ptShaderOptionfv = 0;
PTSHADEROPTIONI ptShaderOptioni = 0;

PTGETSHADEROPTIONF ptGetShaderOptionf = 0;
PTGETSHADEROPTIONF ptGetShaderOptionfv = 0;
PTGETSHADEROPTIONI ptGetShaderOptioni = 0;
PTRESETSHADEROPTIONS ptResetShaderOptions = 0;

PTCOPYSHADERSETTINGS ptCopyShaderSettings = 0;
PTCOPYSHADERSETTINGSTOALL ptCopyShaderSettingsToAll = 0;

PTLIGHTOPTIONF ptLightOptionf = 0;
PTLIGHTOPTIONFV ptLightOptionfv = 0;
PTLIGHTOPTIONI ptLightOptioni = 0;

PTGETLIGHTOPTIONF ptGetLightOptionf = 0;
PTGETLIGHTOPTIONI ptGetLightOptioni = 0;
PTRESETLIGHTOPTIONS ptResetLightOptions = 0;

PTCOPYLIGHTSETTINGS ptCopyLightSettings = 0;
PTCOPYLIGHTSETTINGSTOALL ptCopyLightSettingsToAll = 0;

PTNUMRAMPS ptNumRamps = 0;
PTRAMPINFO ptRampInfo = 0;
PTADDCUSTOMRAMP ptAddCustomRamp = 0;

PTDYNAMICFRAMERATE ptDynamicFrameRate = 0;
PTGETDYNAMICFRAMERATE ptGetDynamicFrameRate = 0;

PTSTATICOPTIMIZER ptStaticOptimizer = 0;
PTGETSTATICOPTIMIZER ptGetStaticOptimizer = 0;

PTGLOBALDENSITY ptGlobalDensity = 0;
PTGETGLOBALDENSITY ptGetGlobalDensity = 0;

PTFINDNEARESTSCREENPOINT ptFindNearestScreenPoint = 0;
PTFINDNEARESTPOINT ptFindNearestPoint = 0;
PTFINDNEARESTSCREENPOINTWDEPTH  ptFindNearestScreenPointWDepth=0;
PTFLIPMOUSEYCOORDS ptFlipMouseYCoords = 0;
PTDONTFLIPMOUSEYCOORDS ptDontFlipMouseYCoords = 0;

PTGETPERVIEWPORTDATASIZE ptGetPerViewportDataSize = 0;
PTGETPERVIEWPORTDATA ptGetPerViewportData = 0;
PTSETPERVIEWPORTDATA ptSetPerViewportData = 0;

PTSETVIEWPORTPOINTSBUDGET ptSetViewportPointsBudget=0;
PTGETVIEWPORTPOINTSBUDGET ptGetViewportPointsBudget=0;

PTSETINTERSECTIONRADIUS  ptSetIntersectionRadius = 0;
PTGETINTERSECTIONRADIUS ptGetIntersectionRadius = 0;
PTINTERSECTRAY ptIntersectRay = 0;
PTINTERSECTRAYPNTINDEX ptIntersectRayPntIndex = 0;
PTINTERSECTRAYINTERPOLATED ptIntersectRayInterpolated = 0;

PTPOINTDATA ptPointData = 0;
PTPOINTATTRIBUTES ptPointAttributes = 0;
PTGETPOINTATTRIBUTE ptGetPointAttribute = 0; 

PTGETLASTERRORSTRING ptGetLastErrorString = 0;
PTGETLASTERRORCODE ptGetLastErrorCode = 0;

PTSETSELECTPOINTSMODE ptSetSelectPointsMode = 0;
PTGETSELECTPOINTSMODE ptGetSelectPointsMode = 0;
PTSELECTPOINTSBYRECT ptSelectPointsByRect = 0; 
PTSELECTPOINTSBYFENCE ptSelectPointsByFence = 0;
PTSELECTPOINTSBYBOX ptSelectPointsByBox = 0;
PTSELECTPOINTSBYORIENTEDBOX ptSelectPointsByOrientedBox = 0;
PTSELECTPOINTSBYCUBE ptSelectPointsByCube = 0;
PTSELECTPOINTSBYPLANE ptSelectPointsByPlane = 0;
PTSELECTPOINTSBYSPHERE ptSelectPointsBySphere = 0;
PTSETSELECTIONDRAWCOLOR ptSetSelectionDrawColor = 0;
PTGETSELECTIONDRAWCOLOR ptGetSelectionDrawColor = 0;

PTSELECTPOINTSINLAYER ptSelectPointsInLayer = 0;
PTDESELECTPOINTSINLAYER ptDeselectPointsInLayer = 0;
PTSELECTCLOUD ptSelectCloud = 0;
PTDESELECTCLOUD ptDeselectCloud = 0;
PTSELECTSCENE ptSelectScene = 0;
PTDESELECTSCENE ptDeselectScene = 0;

PTCOUNTAPPROXPOINTSINLAYER ptCountApproxPointsInLayer = 0;

_PTCOUNTVISIBLEPOINTS _ptCountVisiblePoints = 0;

PTHIDESELECTED		ptHideSelected = 0;
PTISOLATESELECTED	ptIsolateSelected = 0;
PTUNHIDEALL			ptUnhideAll = 0;
PTUNSELECTALL		ptUnselectAll = 0;
PTRESETSELECTION	ptResetSelection = 0;
PTSELECTALL			ptSelectAll = 0;

PTSETSELECTIONSCOPE ptSetSelectionScope = 0;

PTINVERTSELECTION ptInvertSelection=0;
PTINVERTVISIBILITY ptInvertVisibility=0;

PTREFRESHEDIT ptRefreshEdit = 0;
PTCLEAREDIT ptClearEdit = 0;
PTSTOREEDIT ptStoreEdit = 0;
PTRESTOREEDIT ptRestoreEdit = 0;
PTRESTOREEDITBYINDEX ptRestoreEditByIndex = 0;
PTNUMEDITS ptNumEdits = 0;
PTEDITNAME ptEditName = 0;

PTRESETSCENEEDITING ptResetSceneEditing = 0;

PTDELETEEDIT ptDeleteEdit = 0;
PTDELETEEDITBYINDEX ptDeleteEditByIndex = 0;
PTDELETEALLEDITS ptDeleteAllEdits = 0;

PTGETEDITDATA ptGetEditData = 0;
PTGETEDITDATASIZE ptGetEditDataSize = 0;
PTCREATEEDITFROMDATA ptCreateEditFromData = 0;

PTSETEDITWORKINGMODE ptSetEditWorkingMode = 0;
PTGETEDITWORKINGMODE ptGetEditWorkingMode = 0;

PTCREATESELPOINTSQUERY ptCreateSelPointsQuery = 0;
PTCREATEVISPOINTSQUERY ptCreateVisPointsQuery = 0;
PTCREATEBOUNDINGBOXQUERY ptCreateBoundingBoxQuery = 0;
PTCREATEORIENTEDBOUNDINGBOXQUERY ptCreateOrientedBoundingBoxQuery = 0;
PTCREATEBOUNDINGSPHEREQUERY ptCreateBoundingSphereQuery = 0;
PTCREATEKRIGSURFACEQUERY ptCreateKrigSurfaceQuery = 0;
PTCREATEFRUSTUMPOINTSCACHE ptCreateFrustumPointsCache = 0;
PTCREATEFRUSTUMPOINTSQUERY ptCreateFrustumPointsQuery = 0;
PTCREATEKNNQUERY ptCreateKNNQuery = 0;

PTSETQUERYSCOPE ptSetQueryScope = 0;
PTSETQUERYLAYERMASK ptSetQueryLayerMask = 0;

PTRESETQUERY ptResetQuery = 0;
PTDELETEQUERY ptDeleteQuery = 0;

PTSETQUERYRGBMODE ptSetQueryRGBMode = 0;

PTSETQUERYDENSITY ptSetQueryDensity = 0;
PTGETQUERYPOINTSD ptGetQueryPointsd = 0;
PTGETQUERYPOINTSF ptGetQueryPointsf = 0;
PTGETDETAILEDQUERYPOINTSF ptGetDetailedQueryPointsf = 0;
PTGETDETAILEDQUERYPOINTSD ptGetDetailedQueryPointsd = 0;

PTGETQUERYPOINTSMULTIF ptGetQueryPointsMultif = 0;
PTGETQUERYPOINTSMULTID ptGetQueryPointsMultid = 0;


PTSETCACHESIZEMB ptSetCacheSizeMb = 0;
PTGETCACHESIZEMB ptGetCacheSizeMb = 0;
PTAUTOCACHESIZE ptAutoCacheSize = 0;
PTSETLOADINGPRIORITYBIAS ptSetLoadingPriorityBias = 0;
PTGETLOADINGPRIORITYBIAS ptGetLoadingPriorityBias = 0;
PTSETTUNINGPARAMETERFV ptSetTuningParameterfv = 0;
PTGETTUNINGPARAMETERFV ptGetTuningParameterfv = 0;

PTCREATEBITMAPVIEWPORT ptCreateBitmapViewport = 0;
PTDESTROYBITMAPVIEWPORT ptDestroyBitmapViewport = 0;

PTREADVIEWFROMGL ptReadViewFromGL = 0;

PTSETVIEWPROJECTIONORTHO ptSetViewProjectionOrtho = 0;
PTSETVIEWPROJECTIONFRUSTUM ptSetViewProjectionFrustum = 0;
PTSETVIEWPROJECTIONMATRIX ptSetViewProjectionMatrix = 0;
PTSETVIEWPROJECTIONPERSPECTIVE ptSetViewProjectionPerspective = 0;

PTSETVIEWEYELOOKAT ptSetViewEyeLookAt = 0;
PTSETVIEWEYEMATRIX ptSetViewEyeMatrix = 0;

PTSETVIEWPORTSIZE ptSetViewportSize = 0;

PTGETVIEWEYEMATRIX ptGetViewEyeMatrix = 0;
PTGETVIEWPROJECTIONMATRIX ptGetViewProjectionMatrix = 0;

PTSETCURRENTLAYER ptSetCurrentLayer = 0;
PTGETCURRENTLAYER ptGetCurrentLayer = 0;
PTLOCKLAYER ptLockLayer = 0;
PTISLAYERLOCKED ptIsLayerLocked = 0;
PTSHOWLAYER ptShowLayer = 0;
PTISLAYERSHOWN ptIsLayerShown = 0;
PTDOESLAYERHAVEPOINTS ptDoesLayerHavePoints = 0;
PTCLEARPOINTSFROMLAYER ptClearPointsFromLayer = 0;
PTRESETLAYERS ptResetLayers = 0;

PTSETLAYERCOLOR ptSetLayerColor = 0;
PTGETLAYERCOLOR ptGetLayerColor = 0;
PTGETLAYERCOLORBLEND ptGetLayerColorBlend = 0;
PTRESETLAYERCOLORS ptResetLayerColors = 0;

PTCOPYSELTOCURRENTLAYER ptCopySelToCurrentLayer = 0;
PTMOVESELTOCURRENTLAYER ptMoveSelToCurrentLayer = 0;

PTRELEASE ptRelease = 0;

PTCREATEPOINTCHANNEL ptCreatePointChannel = 0;
PTCOPYPOINTCHANNEL ptCopyPointChannel = 0;
PTDELETEPOINTCHANNEL ptDeletePointChannel = 0;
PTDRAWPOINTCHANNELAS ptDrawPointChannelAs = 0;
PTSUBMITPOINTCHANNELUPDATE ptSubmitPointChannelUpdate = 0;
PTCREATESCENEINSTANCE ptCreateSceneInstance =0;
PTGETCHANNELINFO ptGetChannelInfo = 0;

PTWRITECHANNELSFILE	ptWriteChannelsFile = 0;
PTWRITECHANNELSFILETOBUFFER ptWriteChannelsFileToBuffer = 0;
PTRELEASECHANNELSFILEBUFFER ptReleaseChannelsFileBuffer = 0;
PTREADCHANNELSFILE	ptReadChannelsFile = 0;
PTREADCHANNELSFILEFROMBUFFER ptReadChannelsFileFromBuffer = 0;
PTSETCHANNELOOCFOLDER ptSetChannelOOCFolder = 0;
PTDELETEALLCHANNELS ptDeleteAllChannels = 0;
PTGETCHANNELBYNAME ptGetChannelByName = 0;
PTCREATEPOINTCHANNELFROMLAYERS ptCreatePointChannelFromLayers = 0;
PTLAYERSFROMPOINTCHANNEL ptLayersFromPointChannel = 0;

PTSETCLOUDTRANSFORM ptSetCloudTransform =0;
PTSETSCENETRANSFORM ptSetSceneTransform =0;
PTGETCLOUDTRANSFORM ptGetCloudTransform =0;
PTGETSCENETRANSFORM ptGetSceneTransform =0;

PTENABLECLIPPING ptEnableClipping = 0;
PTDISABLECLIPPING ptDisableClipping = 0;
PTSETCLIPSTYLE ptSetClipStyle = 0;
PTGETNUMCLIPPINGPLANES ptGetNumClippingPlanes = 0;
PTISCLIPPINGPLANEENABLED ptIsClippingPlaneEnabled = 0;
PTENABLECLIPPINGPLANE ptEnableClippingPlane = 0;
PTDISABLECLIPPINGPLANE ptDisableClippingPlane = 0;
PTSETCLIPPINGPLANEPARAMETERS ptSetClippingPlaneParameters = 0;

_PTDIAGNOSTIC _ptDiagnostic = 0;

#endif

HMODULE hPTAPI = 0;
static bool _failed = false;
static const char*_failedfunc = 0;

static char _errorMessage[256];

void* GetAPIFunc(const char*name)
{
	void *func = (void*)GetProcAddress(hPTAPI, name);
	assert(func);

	if (!func)
	{
		_failed = true;
		_failedfunc = name;
	}
	return func;
}
const char*DLLLoadErrorMessage() 
{ 
	if (!_failed) return 0;

	if (_failedfunc)
	{
		sprintf_s(_errorMessage, sizeof(_errorMessage), "Failed to find function %s", _failedfunc);
	}
	return _errorMessage; 
}
bool LoadPointoolsDLL(const TCHAR*filepath)
{

#ifndef PT_VORTEX_STATIC_LIB

	hPTAPI = LoadLibrary(filepath);
	if (hPTAPI)
	{ 
		ptInitialize = (PTINITIALIZE)GetAPIFunc("ptInitialize");
		ptIsInitialized = (PTISINITIALIZED)GetAPIFunc("ptIsInitialized");
		ptSetWorkingFolder = (PTSETWORKINGFOLDER)GetAPIFunc("ptSetWorkingFolder");
		ptGetWorkingFolder = (PTGETWORKINGFOLDER)GetAPIFunc("ptGetWorkingFolder");
		ptGetVersionString = (PTGETVERSIONSTRING)GetAPIFunc("ptGetVersionString");
		ptGetVersionNum = (PTGETVERSIONNUM)GetAPIFunc("ptGetVersionNum");

		ptOpenPOD = (PTOPENPOD)GetAPIFunc("ptOpenPOD");
		ptOpenPODStructuredStorageStream = (PTOPENPODSTRUCTUREDSTORAGESTREAM) GetAPIFunc("ptOpenPODStructuredStorageStream");

		ptIsOpen = (PTISOPEN)GetAPIFunc("ptIsOpen");
		ptBrowseAndOpenPOD = (PTBROWSEANDOPENPOD)GetAPIFunc("ptBrowseAndOpenPOD");

		ptCreateFakePOD	= (PTCREATEFAKEPOD)GetAPIFunc("ptCreateFakePOD");
		ptSetReleaseClientServerBufferCallBack = (PTSETRELEASECLIENTSERVERBUFFERCALLBACK)GetAPIFunc("ptSetReleaseClientServerBufferCallBack");
		ptSetServerCallBack = (PTSETSERVERCALLBACK)GetAPIFunc("ptSetServerCallBack");
		ptProcessServerRequest = (PTPROCESSSERVERREQUEST)GetAPIFunc("ptProcessServerRequest");
		ptProcessServerRequestClientID = (PTPROCESSSERVERREQUESTCLIENTID)GetAPIFunc("ptProcessServerRequestClientID");
		ptProcessServerRequestClientID2 = (PTPROCESSSERVERREQUESTCLIENTID2)GetAPIFunc("ptProcessServerRequestClientID2");
		ptSetClientServerLogFile = (PTSETCLIENTSERVERLOGFILE)GetAPIFunc("ptSetClientServerLogFile");	
		ptSetClientServerSendRetries = (PTSETCLIENTSERVERSENDRETRIES)GetAPIFunc("ptSetClientServerSendRetries");
		ptGetClientServerSendRetries = (PTGETCLIENTSERVERSENDRETRIES)GetAPIFunc("ptGetClientServerSendRetries");
		ptSetClientStreaming = (PTSETCLIENTSTREAMING)GetAPIFunc("ptSetClientStreaming");
		ptServerClientLost = (PTSERVERCLIENTLOST)GetAPIFunc("ptServerClientLost");
		ptGetSessionID = (PTGETSESSIONID)GetAPIFunc("ptGetSessionID");

		ptSetClientCacheFolder = (PTSETCLIENTCACHEFOLDER)GetAPIFunc("ptSetClientCacheFolder");	
		ptGetClientCacheFolder = (PTGETCLIENTCACHEFOLDER)GetAPIFunc("ptGetClientCacheFolder");			
		ptEnableClientServerCaching = (PTENABLECLIENTSERVERCACHING)GetAPIFunc("ptEnableClientServerCaching");		
		ptGetClientServerCachingEnabled = (PTGETCLIENTSERVERCACHINGENABLED)GetAPIFunc("ptGetClientServerCachingEnabled");
		ptSetClientServerCacheDataSize = (PTSETCLIENTSERVERCACHEDATASIZE)GetAPIFunc("ptSetClientServerCacheDataSize");		
		ptGetClientServerCacheDataSize = (PTGETCLIENTSERVERCACHEDATASIZE)GetAPIFunc("ptGetClientServerCacheDataSize");		
		ptSetClientCacheCompletionThreshold = (PTSETCLIENTCACHECOMPLETIONTHRESHOLD)GetAPIFunc("ptSetClientCacheCompletionThreshold");
		ptGetClientCacheCompletionThreshold = (PTGETCLIENTCACHECOMPLETIONTHRESHOLD)GetAPIFunc("ptGetClientCacheCompletionThreshold");
		ptSetClientServerCacheNameMode = (PTSETCLIENTSERVERCACHENAMEMODE)GetAPIFunc("ptSetClientServerCacheNameMode");
		ptGetClientServerCacheNameMode = (PTGETCLIENTSERVERCACHENAMEMODE)GetAPIFunc("ptGetClientServerCacheNameMode");

		ptGetCloudHandleByIndex		= (PTGETCLOUDHANDLEBYINDEX)GetAPIFunc("ptGetCloudHandleByIndex");
		ptGetNumCloudsInScene		= (PTGETNUMCLOUDSINSCENE)GetAPIFunc("ptGetNumCloudsInScene");
		ptGetCloudProxyPoints		= (PTGETCLOUDPROXYPOINTS)GetAPIFunc("ptGetCloudProxyPoints");
		ptGetSceneProxyPoints		= (PTGETSCENEPROXYPOINTS)GetAPIFunc("ptGetSceneProxyPoints");
		ptNumScenes					= (PTNUMSCENES)GetAPIFunc("ptNumScenes");
		ptGetSceneHandles			= (PTGETSCENEHANDLES)GetAPIFunc("ptGetSceneHandles");

		/*meta data */ 
		ptReadPODMeta = (PTREADPODMETA)GetAPIFunc("ptReadPODMeta");
		ptGetMetaDataHandle = (PTGETMETADATAHANDLE)GetAPIFunc("ptGetMetaDataHandle");
		ptGetMetaData = (PTGETMETADATA)GetAPIFunc("ptGetMetaData");
		ptSetMetaTag = (PTSETMETATAG)GetAPIFunc("ptSetMetaTag");
		ptWriteMetaTags = (PTWRITEMETATAGS)GetAPIFunc("ptWriteMetaTags");

		ptGetMetaTag = (PTGETMETATAG)GetAPIFunc("ptGetMetaTag");
		ptNumUserMetaSections = (PTNUMUSERMETASECTIONS) GetAPIFunc("ptNumUserMetaSections");
		ptUserMetaSectionName = (PTUSERMETASECTIONNAME)GetAPIFunc("ptUserMetaSectionName");

		ptNumUserMetaTagsInSection = (PTNUMUSERMETATAGSINSECTION) GetAPIFunc("ptNumUserMetaTagsInSection");
		ptGetUserMetaTagByIndex = (PTGETUSERMETATAGBYINDEX) GetAPIFunc("ptGetUserMetaTagByIndex");
		ptGetUserMetaTagByName = (PTGETUSERMETATAGBYNAME) GetAPIFunc("ptGetUserMetaTagByName");

		ptFreeMetaData = (PTFREEMETADATA) GetAPIFunc("ptFreeMetaData");

		ptSceneInfo = (PTSCENEINFO)GetAPIFunc("ptSceneInfo");
		ptSceneFile = (PTSCENEFILE)GetAPIFunc("ptSceneFile");
		ptCloudInfo = (PTCLOUDINFO)GetAPIFunc("ptCloudInfo");
		ptSceneBounds = (PTSCENEBOUNDS)GetAPIFunc("ptSceneBounds");
		ptSceneBoundsd = (PTSCENEBOUNDSD)GetAPIFunc("ptSceneBoundsd");
		ptCloudBounds = (PTCLOUDBOUNDS)GetAPIFunc("ptCloudBounds");
		ptCloudBoundsd = (PTCLOUDBOUNDSD)GetAPIFunc("ptCloudBoundsd");
		ptLayerBoundsd = (PTLAYERBOUNDSD)GetAPIFunc("ptLayerBoundsd");
		ptLayerBounds = (PTLAYERBOUNDS)GetAPIFunc("ptLayerBounds");

		ptShowScene = (PTSHOWSCENE)GetAPIFunc("ptShowScene");
		ptShowCloud = (PTSHOWCLOUD)GetAPIFunc("ptShowCloud");
		ptIsSceneVisible = (PTISSCENEVISIBLE) GetAPIFunc("ptIsSceneVisible");
		ptIsCloudVisible = (PTISCLOUDVISIBLE) GetAPIFunc("ptIsCloudVisible");
		ptUnloadScene = (PTUNLOADSCENE)GetAPIFunc("ptUnloadScene");
		ptReloadScene = (PTRELOADSCENE)GetAPIFunc("ptReloadScene");
		ptRemoveScene = (PTREMOVESCENE)GetAPIFunc("ptRemoveScene");
		ptRemoveAll = (PTREMOVEALL)GetAPIFunc("ptRemoveAll");
		ptOverrideDrawMode = (PTOVERRIDEDRAWMODE) GetAPIFunc("ptOverrideDrawMode"); 
		ptDrawGL = (PTDRAWGL)GetAPIFunc("ptDrawGL");
		ptDrawSceneGL = (PTDRAWSCENEGL)GetAPIFunc("ptDrawSceneGL");
		ptDrawInteractiveGL = (PTDRAWINTERACTIVEGL)GetAPIFunc("ptDrawInteractiveGL");
		ptKbLoaded= (PTKBLOADED)GetAPIFunc("ptKbLoaded");

		ptWeightedPtsLoaded = (PTWEIGHTEDPTSLOADED)GetAPIFunc("ptWeightedPtsLoaded");
		ptPtsLoadedInViewportSinceDraw = (PTPTSLOADEDINVIEWPORTSINCEDRAW) GetAPIFunc("ptPtsLoadedInViewportSinceDraw");
		ptPtsToLoadInViewport = (PTPTSTOLOADINVIEWPORT) GetAPIFunc("ptPtsToLoadInViewport");

		ptEndDrawFrameMetrics = (PTENDDRAWFRAMEMETRICS) GetAPIFunc("ptEndDrawFrameMetrics");
		ptStartDrawFrameMetrics = (PTSTARTDRAWFRAMEMETRICS) GetAPIFunc("ptStartDrawFrameMetrics");

		ptSetHostUnits = (PTSETHOSTUNITS)GetAPIFunc("ptSetHostUnits");
		ptGetHostUnits = (PTGETHOSTUNITS)GetAPIFunc("ptGetHostUnits");
		ptGetCoordinateBase = (PTGETCOORDINATEBASE)GetAPIFunc("ptGetCoordinateBase");
		ptSetCoordinateBase = (PTSETCOORDINATEBASE)GetAPIFunc("ptSetCoordinateBase");
		ptSetAutoBaseMethod = (PTSETAUTOBASEMETHOD)GetAPIFunc("ptSetAutoBaseMethod");
		ptGetAutoBaseMethod = (PTGETAUTOBASEMETHOD)GetAPIFunc("ptGetAutoBaseMethod");
		ptAddViewport = (PTADDVIEWPORT)GetAPIFunc("ptAddViewport");
		ptRemoveViewport = (PTREMOVEVIEWPORT)GetAPIFunc("ptRemoveViewport");
		ptSetViewport = (PTSETVIEWPORT)GetAPIFunc("ptSetViewport");
		ptSetViewportByName = (PTSETVIEWPORTBYNAME)GetAPIFunc("ptSetViewportByName");
		ptViewportIndexFromName = (PTVIEWPORTINDEXFROMNAME)GetAPIFunc("ptViewportIndexFromName");

		ptCaptureViewportInfo = (PTCAPTUREVIEWPORTINFO)GetAPIFunc("ptCaptureViewportInfo");
		ptStoreView = (PTSTOREVIEW)GetAPIFunc("ptStoreView");
		ptCurrentViewport = (PTCURRENTVIEWPORT)GetAPIFunc("ptCurrentViewport");
		ptEnableViewport = (PTENABLEVIEWPORT)GetAPIFunc("ptEnableViewport");
		ptDisableViewport = (PTDISABLEVIEWPORT)GetAPIFunc("ptDisableViewport");
		ptIsViewportEnabled = (PTISVIEWPORTENABLED)GetAPIFunc("ptIsViewportEnabled");
		ptIsCurrentViewportEnabled = (PTISCURRENTVIEWPORTENABLED)GetAPIFunc("ptIsCurrentViewportEnabled");
		ptGetLowerBound = (PTGETLOWERBOUND)GetAPIFunc("ptGetLowerBound");
		ptGetUpperBound = (PTGETUPPERBOUND)GetAPIFunc("ptGetUpperBound");
		ptEnable = (PTENABLE)GetAPIFunc("ptEnable");
		ptDisable = (PTDISABLE)GetAPIFunc("ptDisable");
		ptIsEnabled = (PTISENABLED)GetAPIFunc("ptIsEnabled");
		ptPointSize = (PTPOINTSIZE)GetAPIFunc("ptPointSize");
		ptShaderOptionf = (PTSHADEROPTIONF)GetAPIFunc("ptShaderOptionf");
		ptShaderOptionfv = (PTSHADEROPTIONFV)GetAPIFunc("ptShaderOptionfv");
		ptGetShaderOptionfv = (PTGETSHADEROPTIONFV)GetAPIFunc("ptGetShaderOptionfv");
		ptShaderOptioni = (PTSHADEROPTIONI)GetAPIFunc("ptShaderOptioni");
		ptGetShaderOptionf = (PTGETSHADEROPTIONF)GetAPIFunc("ptGetShaderOptionf");
		ptGetShaderOptioni = (PTGETSHADEROPTIONI)GetAPIFunc("ptGetShaderOptioni");
		ptResetShaderOptions = (PTRESETSHADEROPTIONS)GetAPIFunc("ptResetShaderOptions");
		ptCopyShaderSettings = (PTCOPYSHADERSETTINGS)GetAPIFunc("ptCopyShaderSettings");
		ptCopyShaderSettingsToAll = (PTCOPYSHADERSETTINGSTOALL)GetAPIFunc("ptCopyShaderSettingsToAll");
		ptLightOptionf = (PTLIGHTOPTIONF)GetAPIFunc("ptLightOptionf");
		ptLightOptionfv = (PTLIGHTOPTIONFV)GetAPIFunc("ptLightOptionfv");
		ptLightOptioni = (PTLIGHTOPTIONI)GetAPIFunc("ptLightOptioni");
		ptGetLightOptionf = (PTGETLIGHTOPTIONF)GetAPIFunc("ptGetLightOptionf");
		ptGetLightOptioni = (PTGETLIGHTOPTIONI)GetAPIFunc("ptGetLightOptioni");
		ptResetLightOptions = (PTRESETLIGHTOPTIONS)GetAPIFunc("ptResetLightOptions");
		ptCopyLightSettings = (PTCOPYLIGHTSETTINGS)GetAPIFunc("ptCopyLightSettings");
		ptCopyLightSettingsToAll = (PTCOPYLIGHTSETTINGSTOALL)GetAPIFunc("ptCopyLightSettingsToAll");
		ptNumRamps = (PTNUMRAMPS)GetAPIFunc("ptNumRamps");
		ptRampInfo = (PTRAMPINFO)GetAPIFunc("ptRampInfo");
		ptAddCustomRamp = (PTADDCUSTOMRAMP)GetAPIFunc("ptAddCustomRamp");
		ptDynamicFrameRate = (PTDYNAMICFRAMERATE)GetAPIFunc("ptDynamicFrameRate");
		ptGetDynamicFrameRate = (PTGETDYNAMICFRAMERATE)GetAPIFunc("ptGetDynamicFrameRate");
		ptStaticOptimizer = (PTSTATICOPTIMIZER)GetAPIFunc("ptStaticOptimizer");
		ptGetStaticOptimizer = (PTGETSTATICOPTIMIZER)GetAPIFunc("ptGetStaticOptimizer");
		ptGetGlobalDensity = (PTGETGLOBALDENSITY)GetAPIFunc("ptGetGlobalDensity");
		ptGlobalDensity = (PTGLOBALDENSITY)GetAPIFunc("ptGlobalDensity");
		ptFindNearestScreenPoint = (PTFINDNEARESTSCREENPOINT)GetAPIFunc("ptFindNearestScreenPoint");
		ptFindNearestPoint = (PTFINDNEARESTPOINT)GetAPIFunc("ptFindNearestPoint");
		ptFindNearestScreenPointWDepth = (PTFINDNEARESTSCREENPOINTWDEPTH)GetAPIFunc("ptFindNearestScreenPointWDepth");
		ptFlipMouseYCoords = (PTFLIPMOUSEYCOORDS)GetAPIFunc("ptFlipMouseYCoords");
		ptDontFlipMouseYCoords = (PTDONTFLIPMOUSEYCOORDS)GetAPIFunc("ptDontFlipMouseYCoords");
		ptGetPerViewportDataSize = (PTGETPERVIEWPORTDATASIZE) GetAPIFunc("ptGetPerViewportDataSize");
		ptGetPerViewportData = (PTGETPERVIEWPORTDATA) GetAPIFunc("ptGetPerViewportData");
		ptSetPerViewportData = (PTSETPERVIEWPORTDATA) GetAPIFunc("ptSetPerViewportData");
		ptSetViewportPointsBudget = (PTSETVIEWPORTPOINTSBUDGET) GetAPIFunc("ptSetViewportPointsBudget");
		ptGetViewportPointsBudget = (PTGETVIEWPORTPOINTSBUDGET)GetAPIFunc("ptGetViewportPointsBudget");

		/* snapping */ 
		ptGetIntersectionRadius = (PTGETINTERSECTIONRADIUS) GetAPIFunc("ptGetIntersectionRadius");
		ptSetIntersectionRadius = (PTSETINTERSECTIONRADIUS) GetAPIFunc("ptSetIntersectionRadius");

		ptIntersectRay = (PTINTERSECTRAY) GetAPIFunc("ptIntersectRay");
		ptIntersectRayPntIndex = (PTINTERSECTRAYPNTINDEX) GetAPIFunc("ptIntersectRayPntIndex");
		ptIntersectRayInterpolated = (PTINTERSECTRAYINTERPOLATED) GetAPIFunc("ptIntersectRayInterpolated");

		ptPointData = (PTPOINTDATA) GetAPIFunc("ptPointData");
		ptPointAttributes = (PTPOINTATTRIBUTES) GetAPIFunc("ptPointAttributes");
		ptGetPointAttribute = (PTGETPOINTATTRIBUTE) GetAPIFunc("ptGetPointAttribute");
		ptGetLastErrorString= (PTGETLASTERRORSTRING) GetAPIFunc("ptGetLastErrorString");
		ptGetLastErrorCode = (PTGETLASTERRORCODE) GetAPIFunc( "ptGetLastErrorCode" );

		/* editing */ 
		ptSetSelectPointsMode = (PTSETSELECTPOINTSMODE) GetAPIFunc("ptSetSelectPointsMode");
		ptGetSelectPointsMode = (PTGETSELECTPOINTSMODE) GetAPIFunc("ptGetSelectPointsMode");
		ptSelectPointsByRect = (PTSELECTPOINTSBYRECT) GetAPIFunc("ptSelectPointsByRect"); 
		ptSelectPointsByFence = (PTSELECTPOINTSBYFENCE) GetAPIFunc("ptSelectPointsByFence");
		ptSelectPointsByBox = (PTSELECTPOINTSBYBOX) GetAPIFunc("ptSelectPointsByBox");
		ptSelectPointsByOrientedBox = (PTSELECTPOINTSBYORIENTEDBOX) GetAPIFunc("ptSelectPointsByOrientedBox");
		ptSelectPointsByCube = (PTSELECTPOINTSBYCUBE) GetAPIFunc("ptSelectPointsByCube");
		ptSelectPointsByPlane = (PTSELECTPOINTSBYPLANE) GetAPIFunc("ptSelectPointsByPlane");
		ptSelectPointsBySphere = (PTSELECTPOINTSBYSPHERE) GetAPIFunc("ptSelectPointsBySphere");
		ptHideSelected = (PTHIDESELECTED) GetAPIFunc("ptHideSelected");
		ptIsolateSelected = (PTISOLATESELECTED) GetAPIFunc("ptIsolateSelected");
		ptUnhideAll = (PTUNHIDEALL) GetAPIFunc("ptUnhideAll");
		ptUnselectAll = (PTUNSELECTALL) GetAPIFunc("ptUnselectAll");
		ptResetSelection = (PTRESETSELECTION) GetAPIFunc("ptResetSelection");
		ptSelectAll = (PTSELECTALL) GetAPIFunc("ptSelectAll");
		ptSetSelectionScope = (PTSETSELECTIONSCOPE) GetAPIFunc("ptSetSelectionScope");
		ptRefreshEdit = (PTREFRESHEDIT) GetAPIFunc("ptRefreshEdit");
		ptClearEdit = (PTCLEAREDIT) GetAPIFunc("ptClearEdit");
		ptStoreEdit = (PTSTOREEDIT) GetAPIFunc("ptStoreEdit");
		ptRestoreEdit = (PTRESTOREEDIT) GetAPIFunc("ptRestoreEdit");
		ptRestoreEditByIndex = (PTRESTOREEDITBYINDEX) GetAPIFunc("ptRestoreEditByIndex");
		ptNumEdits = (PTNUMEDITS) GetAPIFunc("ptNumEdits");
		ptEditName = (PTEDITNAME) GetAPIFunc("ptEditName");
		ptDeleteEdit = (PTDELETEEDIT) GetAPIFunc("ptDeleteEdit");
		ptDeleteEditByIndex = (PTDELETEEDITBYINDEX) GetAPIFunc("ptDeleteEditByIndex");
		ptDeleteAllEdits = (PTDELETEALLEDITS) GetAPIFunc("ptDeleteAllEdits");
		
		_ptCountVisiblePoints = (_PTCOUNTVISIBLEPOINTS) GetAPIFunc("_ptCountVisiblePoints");
		ptCountApproxPointsInLayer = (PTCOUNTAPPROXPOINTSINLAYER) GetAPIFunc("ptCountApproxPointsInLayer");

		ptResetSceneEditing = (PTRESETSCENEEDITING) GetAPIFunc("ptResetSceneEditing");

		ptSetSelectionDrawColor = (PTSETSELECTIONDRAWCOLOR) GetAPIFunc("ptSetSelectionDrawColor");
		ptGetSelectionDrawColor = (PTGETSELECTIONDRAWCOLOR) GetAPIFunc("ptGetSelectionDrawColor");

		ptInvertVisibility = (PTINVERTVISIBILITY) GetAPIFunc("ptInvertVisibility");
		ptInvertSelection = (PTINVERTSELECTION) GetAPIFunc("ptInvertSelection");

		ptGetEditData = (PTGETEDITDATA) GetAPIFunc("ptGetEditData");
		ptGetEditDataSize = (PTGETEDITDATASIZE) GetAPIFunc("ptGetEditDataSize");
		ptCreateEditFromData = (PTCREATEEDITFROMDATA) GetAPIFunc("ptCreateEditFromData");

		ptSetEditWorkingMode = (PTSETEDITWORKINGMODE) GetAPIFunc("ptSetEditWorkingMode");
		ptGetEditWorkingMode = (PTGETEDITWORKINGMODE) GetAPIFunc("ptGetEditWorkingMode");
		
		ptSelectPointsInLayer = (PTSELECTPOINTSINLAYER) GetAPIFunc("ptSelectPointsInLayer");
		ptDeselectPointsInLayer = (PTDESELECTPOINTSINLAYER) GetAPIFunc("ptDeselectPointsInLayer");
		ptSelectCloud = (PTSELECTCLOUD) GetAPIFunc("ptSelectCloud");
		ptDeselectCloud = (PTDESELECTCLOUD) GetAPIFunc("ptDeselectCloud");
		ptSelectScene = (PTSELECTSCENE) GetAPIFunc("ptSelectScene");
		ptDeselectScene = (PTDESELECTSCENE) GetAPIFunc("ptDeselectScene");

		/* point layers */ 
		ptSetCurrentLayer  = (PTSETCURRENTLAYER) GetAPIFunc("ptSetCurrentLayer");
		ptGetCurrentLayer = (PTGETCURRENTLAYER) GetAPIFunc("ptGetCurrentLayer");
		ptLockLayer = (PTLOCKLAYER) GetAPIFunc("ptLockLayer");
		ptIsLayerLocked = (PTISLAYERLOCKED) GetAPIFunc("ptIsLayerLocked");
		ptShowLayer = (PTSHOWLAYER) GetAPIFunc("ptShowLayer");
		ptIsLayerShown = (PTISLAYERSHOWN) GetAPIFunc("ptIsLayerShown");
		ptDoesLayerHavePoints = (PTDOESLAYERHAVEPOINTS) GetAPIFunc("ptDoesLayerHavePoints");
		ptClearPointsFromLayer = (PTCLEARPOINTSFROMLAYER) GetAPIFunc("ptClearPointsFromLayer");
		ptResetLayers = (PTRESETLAYERS) GetAPIFunc("ptResetLayers");

		ptSetLayerColor = (PTSETLAYERCOLOR)GetAPIFunc("ptSetLayerColor");
		ptGetLayerColor = (PTGETLAYERCOLOR)GetAPIFunc("ptGetLayerColor");
		ptGetLayerColorBlend = (PTGETLAYERCOLORBLEND)GetAPIFunc("ptGetLayerColorBlend");
		ptResetLayerColors = (PTRESETLAYERCOLORS)GetAPIFunc("ptResetLayerColors");

		ptCopySelToCurrentLayer = (PTCOPYSELTOCURRENTLAYER) GetAPIFunc("ptCopySelToCurrentLayer");
		ptMoveSelToCurrentLayer = (PTMOVESELTOCURRENTLAYER) GetAPIFunc("ptMoveSelToCurrentLayer");

		/* Query */ 
		ptCreateSelPointsQuery = (PTCREATESELPOINTSQUERY) GetAPIFunc("ptCreateSelPointsQuery");
		ptCreateVisPointsQuery = (PTCREATEVISPOINTSQUERY) GetAPIFunc("ptCreateVisPointsQuery");
		ptCreateBoundingBoxQuery = (PTCREATEBOUNDINGBOXQUERY) GetAPIFunc("ptCreateBoundingBoxQuery");
		ptCreateOrientedBoundingBoxQuery = (PTCREATEORIENTEDBOUNDINGBOXQUERY) GetAPIFunc("ptCreateOrientedBoundingBoxQuery");
		ptCreateBoundingSphereQuery = (PTCREATEBOUNDINGSPHEREQUERY) GetAPIFunc("ptCreateBoundingSphereQuery");
		ptCreateKrigSurfaceQuery = (PTCREATEKRIGSURFACEQUERY) GetAPIFunc("ptCreateKrigSurfaceQuery");
		ptCreateFrustumPointsQuery =  (PTCREATEFRUSTUMPOINTSQUERY) GetAPIFunc("ptCreateFrustumPointsQuery");
		ptCreateKNNQuery = (PTCREATEKNNQUERY) GetAPIFunc("ptCreateKNNQuery");

		ptResetQuery = (PTRESETQUERY) GetAPIFunc("ptResetQuery");
		ptDeleteQuery = (PTDELETEQUERY) GetAPIFunc("ptDeleteQuery");

		ptSetQueryScope = (PTSETQUERYSCOPE) GetAPIFunc("ptSetQueryScope");
		ptSetQueryLayerMask = (PTSETQUERYLAYERMASK) GetAPIFunc("ptSetQueryLayerMask");

		ptSetQueryDensity = (PTSETQUERYDENSITY) GetAPIFunc("ptSetQueryDensity");
		ptGetQueryPointsd = (PTGETQUERYPOINTSD) GetAPIFunc("ptGetQueryPointsd");
		ptGetQueryPointsf = (PTGETQUERYPOINTSF) GetAPIFunc("ptGetQueryPointsf");
		ptSetQueryRGBMode = (PTSETQUERYRGBMODE) GetAPIFunc("ptSetQueryRGBMode");

		ptGetQueryPointsMultif = (PTGETQUERYPOINTSMULTIF) GetAPIFunc("ptGetQueryPointsMultif");
		ptGetQueryPointsMultid = (PTGETQUERYPOINTSMULTID) GetAPIFunc("ptGetQueryPointsMultid");

		/* Tuning */ 
		ptSetCacheSizeMb = (PTSETCACHESIZEMB) GetAPIFunc("ptSetCacheSizeMb"); 
		ptGetCacheSizeMb = (PTGETCACHESIZEMB) GetAPIFunc("ptGetCacheSizeMb");
		ptAutoCacheSize = (PTAUTOCACHESIZE) GetAPIFunc("ptAutoCacheSize");
		ptSetLoadingPriorityBias = (PTSETLOADINGPRIORITYBIAS) GetAPIFunc("ptSetLoadingPriorityBias");
		ptGetLoadingPriorityBias = (PTGETLOADINGPRIORITYBIAS) GetAPIFunc("ptGetLoadingPriorityBias");
		ptSetTuningParameterfv = (PTSETTUNINGPARAMETERFV) GetAPIFunc("ptSetTuningParameterfv");
		ptGetTuningParameterfv = (PTGETTUNINGPARAMETERFV) GetAPIFunc("ptGetTuningParameterfv");

		/* bitmap viewport */ 
		ptCreateBitmapViewport = (PTCREATEBITMAPVIEWPORT) GetAPIFunc("ptCreateBitmapViewport");
		ptDestroyBitmapViewport = (PTDESTROYBITMAPVIEWPORT) GetAPIFunc("ptDestroyBitmapViewport");

		/* view setup */ 
		ptReadViewFromGL = (PTREADVIEWFROMGL) GetAPIFunc("ptReadViewFromGL");

		ptSetViewProjectionOrtho = (PTSETVIEWPROJECTIONORTHO) GetAPIFunc("ptSetViewProjectionOrtho");
		ptSetViewProjectionFrustum = (PTSETVIEWPROJECTIONFRUSTUM) GetAPIFunc("ptSetViewProjectionFrustum");
		ptSetViewProjectionMatrix = (PTSETVIEWPROJECTIONMATRIX) GetAPIFunc("ptSetViewProjectionMatrix");
		ptSetViewProjectionPerspective = (PTSETVIEWPROJECTIONPERSPECTIVE) GetAPIFunc("ptSetViewProjectionPerspective");

		ptSetViewEyeLookAt = (PTSETVIEWEYELOOKAT) GetAPIFunc("ptSetViewEyeLookAt");
		ptSetViewEyeMatrix = (PTSETVIEWEYEMATRIX) GetAPIFunc("ptSetViewEyeMatrix");

		ptSetViewportSize = (PTSETVIEWPORTSIZE) GetAPIFunc("ptSetViewportSize");

		ptGetViewEyeMatrix = (PTGETVIEWEYEMATRIX) GetAPIFunc("ptGetViewEyeMatrix");
		ptGetViewProjectionMatrix = (PTGETVIEWPROJECTIONMATRIX) GetAPIFunc("ptGetViewProjectionMatrix");

		/* user channels */ 
		ptCreatePointChannel = (PTCREATEPOINTCHANNEL) GetAPIFunc("ptCreatePointChannel");
		ptCopyPointChannel = (PTCOPYPOINTCHANNEL) GetAPIFunc("ptCopyPointChannel");
		ptDeletePointChannel = (PTDELETEPOINTCHANNEL) GetAPIFunc("ptDeletePointChannel");
		ptSubmitPointChannelUpdate = (PTSUBMITPOINTCHANNELUPDATE) GetAPIFunc("ptSubmitPointChannelUpdate");
		ptGetDetailedQueryPointsf = (PTGETDETAILEDQUERYPOINTSF) GetAPIFunc("ptGetDetailedQueryPointsf");
		ptGetDetailedQueryPointsd = (PTGETDETAILEDQUERYPOINTSD) GetAPIFunc("ptGetDetailedQueryPointsd");
		ptDrawPointChannelAs = (PTDRAWPOINTCHANNELAS) GetAPIFunc("ptDrawPointChannelAs");
		ptWriteChannelsFile = (PTWRITECHANNELSFILE)	GetAPIFunc("ptWriteChannelsFile");
		ptWriteChannelsFileToBuffer = (PTWRITECHANNELSFILETOBUFFER) GetAPIFunc("ptWriteChannelsFileToBuffer");
		ptReleaseChannelsFileBuffer = (PTRELEASECHANNELSFILEBUFFER) GetAPIFunc("ptReleaseChannelsFileBuffer");
		ptReadChannelsFile = (PTREADCHANNELSFILE)	GetAPIFunc("ptReadChannelsFile");
		ptReadChannelsFileFromBuffer = (PTREADCHANNELSFILEFROMBUFFER) GetAPIFunc("ptReadChannelsFileFromBuffer");
		ptSetChannelOOCFolder = (PTSETCHANNELOOCFOLDER) GetAPIFunc("ptSetChannelOOCFolder");
		ptDeleteAllChannels = (PTDELETEALLCHANNELS) GetAPIFunc("ptDeleteAllChannels");
		ptGetChannelInfo = (PTGETCHANNELINFO) GetAPIFunc("ptGetChannelInfo");
		ptGetChannelByName = (PTGETCHANNELBYNAME) GetAPIFunc("ptGetChannelByName");

		ptCreatePointChannelFromLayers = (PTCREATEPOINTCHANNELFROMLAYERS) GetAPIFunc("ptCreatePointChannelFromLayers");
		ptLayersFromPointChannel = (PTLAYERSFROMPOINTCHANNEL) GetAPIFunc("ptLayersFromPointChannel");

		/* instance */ 
		ptCreateSceneInstance = (PTCREATESCENEINSTANCE) GetAPIFunc("ptCreateSceneInstance");

		/* transform */ 
		ptSetCloudTransform = (PTSETCLOUDTRANSFORM) GetAPIFunc( "ptSetCloudTransform" );
		ptSetSceneTransform = (PTSETSCENETRANSFORM) GetAPIFunc( "ptSetSceneTransform" );
		ptGetCloudTransform = (PTGETCLOUDTRANSFORM) GetAPIFunc( "ptGetCloudTransform" );
		ptGetSceneTransform = (PTGETSCENETRANSFORM) GetAPIFunc( "ptGetSceneTransform" );

		/* clipping */
		ptEnableClipping = (PTENABLECLIPPING) GetAPIFunc( "ptEnableClipping" );
		ptDisableClipping = (PTDISABLECLIPPING) GetAPIFunc( "ptDisableClipping" );
		ptSetClipStyle = (PTSETCLIPSTYLE) GetAPIFunc ( "ptSetClipStyle" );
		ptGetNumClippingPlanes = (PTGETNUMCLIPPINGPLANES) GetAPIFunc( "ptGetNumClippingPlanes" );		
		ptIsClippingPlaneEnabled = (PTISCLIPPINGPLANEENABLED) GetAPIFunc( "ptIsClippingPlaneEnabled" );
		ptEnableClippingPlane = (PTENABLECLIPPINGPLANE) GetAPIFunc( "ptEnableClippingPlane" );
		ptDisableClippingPlane = (PTDISABLECLIPPINGPLANE) GetAPIFunc( "ptDisableClippingPlane" );
		ptSetClippingPlaneParameters = (PTSETCLIPPINGPLANEPARAMETERS) GetAPIFunc( "ptSetClippingPlaneParameters" );

		ptRelease = (PTRELEASE) GetAPIFunc("ptRelease");

		_ptDiagnostic = (_PTDIAGNOSTIC) GetAPIFunc("_ptDiagnostic");
		return !_failed;
	}
	else
	{
		DWORD err = GetLastError();
		FormatMessageA( FORMAT_MESSAGE_FROM_SYSTEM, 0, err, LANG_SYSTEM_DEFAULT, _errorMessage, sizeof(_errorMessage),0); 
	}

#endif

	return false;
}