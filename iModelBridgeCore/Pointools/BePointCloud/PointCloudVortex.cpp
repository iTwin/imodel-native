/*--------------------------------------------------------------------------------------+
|
|     $Source: PointCloudVortex.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <BePointCloudInternal.h>

USING_NAMESPACE_BENTLEY_BEPOINTCLOUD

#define BENTLEY_STATUS_FROM_PTRES(ptResult)  ((ptResult) == PTV_SUCCESS)

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
void PointCloudVortex::SetEnabledState (PtEnable option, int32_t value)
    {
    if (value)
        PointCloudVortex::Enable (option);
    else
        PointCloudVortex::Disable (option);
    }

void PointCloudVortex::SetViewport(int32_t index)
    {
    ptSetViewport(index);
    }

int32_t PointCloudVortex::AddViewport( int32_t index, WCharCP name)
    {
    return ptAddViewport (index,name,PT_SW_VIEWPORT); 
    }

void PointCloudVortex::RemoveViewport( int32_t index)
    {
    ptRemoveViewport (index); 
    }

PThandle PointCloudVortex::CreateSelPointsQuery()
    {
    return ptCreateSelPointsQuery();
    }
PThandle PointCloudVortex::CreateVisPointsQuery()
    {
    return ptCreateVisPointsQuery();
    }
PThandle PointCloudVortex::CreateBoundingBoxQuery(double minx, double miny, double minz, double maxx, double maxy, double maxz)
    {
    return ptCreateBoundingBoxQuery(minx, miny, minz, maxx, maxy, maxz);
    }
PThandle PointCloudVortex::CreateOrientedBoundingBoxQuery(double minx, double miny, double minz, double maxx, 
                                                         double maxy, double maxz, double posx, double posy, double posz, 
                                                         double ux, double uy, double uz, double vx, double vy, double vz)
    {
    return ptCreateOrientedBoundingBoxQuery (minx, miny, minz, maxx, maxy, maxz, posx, posy, posz, ux, uy, uz, vx, vy, vz);
    }

PThandle PointCloudVortex::CreateBoundingSphereQuery( double *cen, double radius )
    {
    return ptCreateBoundingSphereQuery(cen, radius);
    }
PThandle PointCloudVortex::CreateKrigSurfaceQuery (uint32_t numPoints, double *pnts )
    {
    return ptCreateKrigSurfaceQuery(numPoints, pnts);
    }

// No where to be found in pointools. How it worked before?
// PThandle PointCloudVortex::CreateFrustumPointsCache( uint32_t maxPoints )
//     {
//     return ptCreateFrustumPointsCache(maxPoints);
//     }

PThandle PointCloudVortex::CreateFrustumPointsQuery()
    {
    return ptCreateFrustumPointsQuery();
    }
bool PointCloudVortex::ResetQuery( PThandle query )
    {
    return ptResetQuery(query);
    }
void PointCloudVortex::SetQueryRGBMode( PThandle query, PtQueryRgbMode mode )
    {
    ptSetQueryRGBMode(query, mode);
    }
bool PointCloudVortex::DeleteQuery( PThandle query )
    {
    return ptDeleteQuery(query);
    }
void PointCloudVortex::SetQueryScope( PThandle query, PThandle sceneOrCloudHandle )
    {
    ptSetQueryScope(query, sceneOrCloudHandle);
    }
void PointCloudVortex::SetQueryDensity( PThandle query, PtQueryDensity densityType, float densityValue  )
    {
    ptSetQueryDensity(query, densityType, densityValue);
    }
uint32_t PointCloudVortex::GetQueryPointsd (PThandle query, uint32_t buffersize, double *geomBuffer, Byte *rgbBuffer, 
                                         int16_t *intensityBuffer, Byte *selectionBuffer, Byte *classificationBuffer)
    {
    return ptGetQueryPointsd(query, buffersize, geomBuffer, rgbBuffer, intensityBuffer, selectionBuffer, classificationBuffer);
    }  
uint32_t PointCloudVortex::GetQueryPointsf( PThandle query, uint32_t buffersize, float *geomBuffer, Byte *rgbBuffer,
                                        int16_t *intensityBuffer, Byte *selectionBuffer, Byte *classificationBuffer)
    {
    return ptGetQueryPointsf(query, buffersize, geomBuffer, rgbBuffer, intensityBuffer, selectionBuffer, classificationBuffer);
    }
uint32_t PointCloudVortex::GetDetailedQueryPointsf( PThandle query, uint32_t bufferSize, float *geomBuffer, Byte *rgbBuffer,
                                                int16_t *intensityBuffer, float *normalBuffer, Byte *filter, 
                                                Byte *classificationBuffer, uint32_t numPointChannels, 
                                                const PThandle *pointChannelsReq, void **pointChannels )
    {
    return ptGetDetailedQueryPointsf(query, bufferSize, geomBuffer, rgbBuffer, intensityBuffer, normalBuffer, filter,
                                        classificationBuffer, numPointChannels, pointChannelsReq, pointChannels);
    }
uint32_t PointCloudVortex::GetDetailedQueryPointsd (PThandle query, uint32_t bufferSize, double *geomBuffer, Byte *rgbBuffer,
                                                 int16_t *intensityBuffer, float *normalBuffer, Byte *filter,
                                                 Byte *classificationBuffer, uint32_t numPointChannels, 
                                                 const PThandle *pointChannelsReq, void **pointChannels )
    {
    return ptGetDetailedQueryPointsd(query, bufferSize, geomBuffer, rgbBuffer, intensityBuffer, normalBuffer, filter,
                                        classificationBuffer, numPointChannels, pointChannelsReq, pointChannels);
    }

/* shader options */ 
void PointCloudVortex::Enable(PtEnable option)
    {
    ptEnable(option);
    }
void PointCloudVortex::Disable(PtEnable option)
    {
    ptDisable(option);
    }
bool PointCloudVortex::IsEnabled(PtEnable option)
    {
    return ptIsEnabled(option);
    }
void PointCloudVortex::PointSize(float size)
    {
    ptPointSize(size);
    }
void PointCloudVortex::ShaderOptionf(PtShaderOptions shader_option, float value)
    {
    ptShaderOptionf(shader_option, value);
    }
void PointCloudVortex::ShaderOptionfv(PtShaderOptions shader_option, float *value)
    {
    ptShaderOptionfv(shader_option, value);
    }
void PointCloudVortex::ShaderOptioni(PtShaderOptions shader_option, int32_t value)
    {
    ptShaderOptioni(shader_option, value);
    }
StatusInt PointCloudVortex::GetShaderOptionf(PtShaderOptions shader_option, float *value)
    {
    PTres result = ptGetShaderOptionf(shader_option, value);
    return BENTLEY_STATUS_FROM_PTRES(result);
    }
StatusInt PointCloudVortex::GetShaderOptionfv(PtShaderOptions shader_option, float *values)
    {
    PTres result = ptGetShaderOptionfv(shader_option, values);
    return BENTLEY_STATUS_FROM_PTRES(result);
    }
StatusInt PointCloudVortex::GetShaderOptioni(PtShaderOptions shader_option, int32_t *value)
    {
    PTres result = ptGetShaderOptioni(shader_option, value);
    return BENTLEY_STATUS_FROM_PTRES(result);
    }
void PointCloudVortex::ResetShaderOptions()
    {
    ptResetShaderOptions();
    }
void PointCloudVortex::CopyShaderSettings(uint32_t dest_viewport)
    {
    ptCopyShaderSettings(dest_viewport);
    }
void PointCloudVortex::CopyShaderSettingsToAll()
    {
    ptCopyShaderSettingsToAll();
    }
int32_t PointCloudVortex::NumRamps()
    {
    return ptNumRamps();
    }
WCharCP PointCloudVortex::RampInfo( int32_t ramp, PtRampType *type )
    {
    return ptRampInfo(ramp, (PTenum*)type);
    }
StatusInt PointCloudVortex::AddCustomRamp(WCharCP name, int32_t numKeys, const float *positions, const Byte* colour3vals, bool interpolateInHSL)
    {
    PTres result = ptAddCustomRamp(name, numKeys, positions, colour3vals, interpolateInHSL);
    return BENTLEY_STATUS_FROM_PTRES(result);
    }

// options
void PointCloudVortex::DynamicFrameRate(float fps)
    {
    ptDynamicFrameRate(fps);
    }
float PointCloudVortex::GetDynamicFrameRate()
    {
    return ptGetDynamicFrameRate();
    }
void PointCloudVortex::StaticOptimizer(float opt)
    {
    ptStaticOptimizer(opt);
    }
//float PointCloudVortex::GetStaticOptimizer()
//    {
//    return ptGetStaticOptimizer();
//    }
void PointCloudVortex::GlobalDensity(float opt)
    {
    ptGlobalDensity(opt);
    }
float PointCloudVortex::GetGlobalDensity()
    {
    return ptGetGlobalDensity();
    }

int64_t PointCloudVortex::PtsLoadedInViewportSinceLastDraw(PThandle hScene)
    {
    return ptPtsLoadedInViewportSinceDraw(hScene);
    }

int64_t PointCloudVortex::PtsToLoadInViewport(PThandle hScene, bool recompute)
    {
    return ptPtsToLoadInViewport(hScene, recompute);
    }
void PointCloudVortex::StartDrawFrameMetrics()
    {
    ptStartDrawFrameMetrics();
    }
void PointCloudVortex::EndDrawFrameMetrics()
    {
    ptEndDrawFrameMetrics();
    }

// view
void PointCloudVortex::SetViewProjectionOrtho( double l, double r, double b, double t, double n, double f )
    {
    ptSetViewProjectionOrtho(l, r, b, t, n, f);
    }
void PointCloudVortex::SetViewProjectionFrustum( double l, double r, double b, double t, double n, double f )
    {
    ptSetViewProjectionFrustum(l, r, b, t, n, f);
    }
void PointCloudVortex::SetViewProjectionMatrix( const double *matrix, bool row_major )
    {
    ptSetViewProjectionMatrix(matrix, row_major);
    }
void PointCloudVortex::SetViewProjectionPerspective( PtPerspectiveType perspective_type, double fov_deg, double aspect, double n, double f)
    {
    ptSetViewProjectionPerspective(perspective_type, fov_deg, aspect, n, f);
    }
void PointCloudVortex::SetViewEyeLookAt( const double *eye, const double *target, const double *up )
    {
    ptSetViewEyeLookAt(eye, target, up);
    }
void PointCloudVortex::SetViewEyeMatrix( const double *matrix, bool row_major )
    {
    ptSetViewEyeMatrix(matrix, row_major);
    }
void PointCloudVortex::SetViewportSize( int32_t left, int32_t bottom, uint32_t width, uint32_t height )
    {
    ptSetViewportSize(left, bottom, width, height);
    }
void PointCloudVortex::GetViewEyeMatrix( double *matrix )
    {
    ptGetViewEyeMatrix(matrix);
    }
void PointCloudVortex::GetViewProjectionMatrix( double *matrix )
    {
    ptGetViewProjectionMatrix(matrix);
    }
