/*--------------------------------------------------------------------------------------+
|
|     $Source: PointCloudSchema/VisualizationManager.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_POINTCLOUD_NAMESPACE

struct PerspectiveViewParams;

//========================================================================================
// @bsiclass                                                        Eric.Paquet     1/2015
//========================================================================================
struct VisualizationManager
{
private:
    static void        SetEnabledState(PTenum option, int32_t value);
    static void        GetNpcViewRange(DRange3d& npcRange, ViewContextR context);
    static void        GetNpcViewBox(DPoint3d npcPoints[8], ViewContextR context);
    static double      ComputeWorldScale(DPoint3dCP world);
    static void        SetViewportProjectionFromSceneRange(ViewContextR context, Transform const& sceneToUor, DRange3d const& range);
    static void        SetViewportOrtho(ViewContextR context, Transform const& sceneToUor, DRange3d const& vector);
    static void        LocalRangeToNpcRange(DRange3d& npcRange, DRange3d const& localRange, ViewContextR context);
    static void        ViewParametersFromWorld(PerspectiveViewParams& viewParams, DPoint3dCP world);
    static void        GetUstnViewToOpenGLViewTransform(DMatrix4dR result, double minY, double maxY);
    static StatusInt   LocalToViewAccountingForPointBehindCamera(DPoint3dP viewPoints, DPoint3dCP localPoints, uint32_t nbPoints, ViewContextR context);
public:
    static void        SetViewportInfo(ViewContextR context, Transform const& sceneToUor, DRange3d const& range);
};

//=======================================================================================
// @bsiclass                                                    Eric.Paquet         02/15
//=======================================================================================
struct PerspectiveViewParams
    {
    double      xmin; 
    double      xmax; 
    double      ymin; 
    double      ymax; 
    double      front;
    double      back;
    DVec3d      viewX;
    DVec3d      viewY;
    DVec3d      viewZ;
    DPoint3d    cameraRoot;
    DPoint3d    targetRoot;
    };


END_BENTLEY_POINTCLOUD_NAMESPACE
