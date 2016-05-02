/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/BePointCloud/PointCloudVortex.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__

#include <BePointCloud/BePointCloudCommon.h>
#include <BePointCloud/ExportMacros.h>

BEGIN_BENTLEY_BEPOINTCLOUD_NAMESPACE

enum PtEnable
    {
    RGB_SHADER              = 0x01,
    INTENSITY_SHADER        = 0x02,
    BLENDING_SHADER         = 0x03,
    PLANE_SHADER            = 0x04,
    LIGHTING                = 0x05,
    CLIPPING                = 0x06,
    ADAPTIVE_POINT_SIZE     = 0X100,
    FRONT_BIAS              = 0X101,
    DELAYED_CHANNEL_LOAD    = 0X102
    };

enum PtShaderOptions
    {
    PLANE_SHADER_DISTANCE       = 0x11,
    PLANE_SHADER_VECTOR         = 0x12,
    PLANE_SHADER_OFFSET         = 0x13,
    INTENSITY_SHADER_CONTRAST   = 0x14,
    INTENSITY_SHADER_BRIGHTNESS = 0x15,
    RGB_SHADER_CONTRAST         = 0x16,
    RGB_SHADER_BRIGHTNESS       = 0x17,
    INTENSITY_SHADER_RAMP       = 0x23,
    PLANE_SHADER_RAMP           = 0x24,
    PLANE_SHADER_EDGE           = 0x29,
    MATERIAL_AMBIENT            = 0X25,
    MATERIAL_DIFFUSE            = 0x26,
    MATERIAL_SPECULAR           = 0x27,
    MATERIAL_GLOSSINESS         = 0x28,
    };

enum PtRampType
    {
    INTENSITY_RAMP_TYPE = 0x01,
    PLANE_RAMP_TYPE     = 0x02
    };

enum PtQueryDensity
    {
    QUERY_DENSITY_FULL          = 0x01,
    QUERY_DENSITY_VIEW          = 0X02,
    QUERY_DENSITY_LIMIT         = 0X03,
    QUERY_DENSITY_VIEW_COMPLETE = 0X04,
    QUERY_DENSITY_SPATIAL       = 0x07,
    };

enum PtQueryRgbMode
    {
    QUERY_RGB_MODE_ACTUAL            = 0x04,
    QUERY_RGB_MODE_SHADER            = 0x05,
    QUERY_RGB_MODE_SHADER_NO_SELECT  = 0x06,
    };
   
enum PtPerspectiveType
    {
    PROJ_PERSPECTIVE_GL     = 0x01,
    PROJ_PERSPECTIVE_DX     = 0x02,
    PROJ_PERSPECTIVE_BLINN  = 0x03,
    };
    
//========================================================================================
// This class contains exports from PtVortex that are punctually needed from client dlls. 
// At some point, we could refactor these methods into a more OO Api.
// 
// @bsiclass                                                        Eric.Paquet     3/2015
//========================================================================================
struct PointCloudVortex
    {
    public:

        BEPOINTCLOUD_EXPORT static void        SetEnabledState (PtEnable option, int32_t value);

        // viewport management
        BEPOINTCLOUD_EXPORT static  void       SetViewport(int32_t index);
        BEPOINTCLOUD_EXPORT static int32_t     AddViewport( int32_t index, WCharCP name);
        BEPOINTCLOUD_EXPORT static void        RemoveViewport( int32_t index);

        // management
        BEPOINTCLOUD_EXPORT static PThandle    CreateSelPointsQuery();
        BEPOINTCLOUD_EXPORT static PThandle    CreateVisPointsQuery();
        BEPOINTCLOUD_EXPORT static PThandle    CreateBoundingBoxQuery(double minx, double miny, double minz, double maxx, double maxy, double maxz);
        BEPOINTCLOUD_EXPORT static PThandle    CreateOrientedBoundingBoxQuery(double minx, double miny, double minz, double maxx,double maxy, double maxz, double posx, double posy, double posz, double ux, double uy, double uz, double vx, double vy, double vz);
        BEPOINTCLOUD_EXPORT static PThandle    CreateBoundingSphereQuery( double *cen, double radius );
        BEPOINTCLOUD_EXPORT static PThandle    CreateKrigSurfaceQuery (uint32_t numPoints, double *pnts );
        //BEPOINTCLOUD_EXPORT static PThandle    CreateFrustumPointsCache( uint32_t maxPoints );
        BEPOINTCLOUD_EXPORT static PThandle    CreateFrustumPointsQuery();
        BEPOINTCLOUD_EXPORT static bool        ResetQuery( PThandle query );
        BEPOINTCLOUD_EXPORT static void        SetQueryRGBMode( PThandle query, PtQueryRgbMode mode );
        BEPOINTCLOUD_EXPORT static bool        DeleteQuery( PThandle query );
        BEPOINTCLOUD_EXPORT static void        SetQueryScope( PThandle query, PThandle sceneOrCloudHandle );
        BEPOINTCLOUD_EXPORT static void        SetQueryDensity( PThandle query, PtQueryDensity densityType, float densityValue );
        BEPOINTCLOUD_EXPORT static uint32_t    GetQueryPointsd (PThandle query, uint32_t buffersize, double *geomBuffer, Byte *rgbBuffer, int16_t *intensityBuffer, Byte *selectionBuffer, Byte *classificationBuffer);
        BEPOINTCLOUD_EXPORT static uint32_t    GetQueryPointsf( PThandle query, uint32_t buffersize, float *geomBuffer, Byte *rgbBuffer, int16_t *intensityBuffer, Byte *selectionBuffer, Byte *classificationBuffer);
        BEPOINTCLOUD_EXPORT static uint32_t    GetDetailedQueryPointsf( PThandle query, uint32_t bufferSize, float *geomBuffer, Byte *rgbBuffer, int16_t *intensityBuffer, float *normalBuffer, Byte *filter, Byte *classificationBuffer, uint32_t numPointChannels, const PThandle *pointChannelsReq, void **pointChannels );
        BEPOINTCLOUD_EXPORT static uint32_t    GetDetailedQueryPointsd (PThandle query, uint32_t bufferSize, double *geomBuffer, Byte *rgbBuffer, int16_t *intensityBuffer, float *normalBuffer, Byte *filter, Byte *classificationBuffer, uint32_t numPointChannels, const PThandle *pointChannelsReq, void **pointChannels );

        // options
        BEPOINTCLOUD_EXPORT static void        DynamicFrameRate(float fps);
        BEPOINTCLOUD_EXPORT static float       GetDynamicFrameRate();
        BEPOINTCLOUD_EXPORT static void        StaticOptimizer(float opt);
        BEPOINTCLOUD_EXPORT static float       GetStaticOptimizer();
        BEPOINTCLOUD_EXPORT static void        GlobalDensity(float opt);
        BEPOINTCLOUD_EXPORT static float       GetGlobalDensity();

        // draw
        BEPOINTCLOUD_EXPORT static int64_t     PtsLoadedInViewportSinceLastDraw(PThandle hScene);
        BEPOINTCLOUD_EXPORT static int64_t     PtsToLoadInViewport(PThandle hScene, bool recompute);
        BEPOINTCLOUD_EXPORT static void        StartDrawFrameMetrics();
        BEPOINTCLOUD_EXPORT static void        EndDrawFrameMetrics();

        // shader options
        BEPOINTCLOUD_EXPORT static void        Enable(PtEnable option);
        BEPOINTCLOUD_EXPORT static void        Disable(PtEnable option);
        BEPOINTCLOUD_EXPORT static bool        IsEnabled(PtEnable option);
        BEPOINTCLOUD_EXPORT static void        PointSize(float size);
        BEPOINTCLOUD_EXPORT static void        ShaderOptionf(PtShaderOptions shader_option, float value);
        BEPOINTCLOUD_EXPORT static void        ShaderOptionfv(PtShaderOptions shader_option, float *value);
        BEPOINTCLOUD_EXPORT static void        ShaderOptioni(PtShaderOptions shader_option, int32_t value);
        BEPOINTCLOUD_EXPORT static StatusInt   GetShaderOptionf(PtShaderOptions shader_option, float *value);
        BEPOINTCLOUD_EXPORT static StatusInt   GetShaderOptionfv(PtShaderOptions shader_option, float *values);
        BEPOINTCLOUD_EXPORT static StatusInt   GetShaderOptioni(PtShaderOptions shader_option, int32_t *value);
        BEPOINTCLOUD_EXPORT static void        ResetShaderOptions();
        BEPOINTCLOUD_EXPORT static void        CopyShaderSettings(uint32_t dest_viewport);
        BEPOINTCLOUD_EXPORT static void        CopyShaderSettingsToAll();
        BEPOINTCLOUD_EXPORT static int32_t     NumRamps();
        BEPOINTCLOUD_EXPORT static WCharCP     RampInfo( int32_t ramp, PtRampType *type );
        BEPOINTCLOUD_EXPORT static StatusInt   AddCustomRamp(WCharCP name, int32_t numKeys, const float *positions, const Byte* colour3vals, bool interpolateInHSL);

        // view
        BEPOINTCLOUD_EXPORT static bool        ReadViewFromGL();
        BEPOINTCLOUD_EXPORT static void        SetViewProjectionOrtho( double l, double r, double b, double t, double n, double f );
        BEPOINTCLOUD_EXPORT static void        SetViewProjectionFrustum( double l, double r, double b, double t, double n, double f );
        BEPOINTCLOUD_EXPORT static void        SetViewProjectionMatrix( const double *matrix, bool row_major );
        BEPOINTCLOUD_EXPORT static void        SetViewProjectionPerspective( PtPerspectiveType perspective_type, double fov_deg, double aspect, double n, double f);
        BEPOINTCLOUD_EXPORT static void        SetViewEyeLookAt( const double *eye, const double *target, const double *up );
        BEPOINTCLOUD_EXPORT static void        SetViewEyeMatrix( const double *matrix, bool row_major );
        BEPOINTCLOUD_EXPORT static void        SetViewportSize( int32_t left, int32_t bottom, uint32_t width, uint32_t height );
        BEPOINTCLOUD_EXPORT static void        GetViewEyeMatrix( double *matrix );
        BEPOINTCLOUD_EXPORT static void        GetViewProjectionMatrix( double *matrix );

    private:
    };


END_BENTLEY_BEPOINTCLOUD_NAMESPACE
