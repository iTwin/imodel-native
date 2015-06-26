/*--------------------------------------------------------------------------------------+
|
|     $Source: PointCloudSchema/VisualizationManager.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_POINTCLOUDSCHEMA_NAMESPACE

struct VisualizationManagerDestroyer;
struct PerspectiveViewParams;

//========================================================================================
// @bsiclass                                                        Eric.Paquet     1/2015
//========================================================================================
struct VisualizationManager
    {
    private:
        friend VisualizationManagerDestroyer;

        static      VisualizationManager* s_instance;

        void        SetViewportInfo(ViewContextR context, Transform const& sceneToUor, DRange3d const& range);
        void        OverrideViewSettings(RefCountedPtr<IPointCloudViewSettings> settingsPtr, Dgn::ViewContextR context, BePointCloud::PointCloudSceneCR pointCloudScene);
        void        UpdateVortexShaders(RefCountedPtr<IPointCloudViewSettings> const& settingsPtr, DgnViewportR viewport);

        bool        IsDrawPurposeValidForFastUpdate(Dgn::DrawPurpose drawPurpose);
        bool        IsDrawPurposeValidForCompleteUpdate(Dgn::DrawPurpose drawPurpose);
        void        SetEnabledState(PTenum option, int32_t value);
        void        GetNpcViewRange(DRange3d& npcRange, ViewContextR context);
        void        GetNpcViewBox(DPoint3d npcPoints[8], ViewContextR context);
        double      ComputeWorldScale(DPoint3dCP world);
        void        SetViewportProjectionFromSceneRange(ViewContextR context, Transform const& sceneToUor, DRange3d const& range);
        void        SetViewportOrtho(ViewContextR context, Transform const& sceneToUor, DRange3d const& vector);
        void        LocalRangeToNpcRange(DRange3d& npcRange, DRange3d const& localRange, ViewContextR context);
        void        ViewParametersFromWorld(PerspectiveViewParams& viewParams, DPoint3dCP world);
        void        GetUstnViewToOpenGLViewTransform(DMatrix4dR result, double minY, double maxY);
        StatusInt   LocalToViewAccountingForPointBehindCamera(DPoint3dP viewPoints, DPoint3dCP localPoints, uint32_t nbPoints, ViewContextR context);

    public:
        VisualizationManager();
        ~VisualizationManager();

        Dgn::IProgressiveDisplay::Completion DrawToContext(Dgn::ViewContextR context, BePointCloud::PointCloudSceneCR pointCloudScene, DRange3dCR pointCloudRange);

        static  VisualizationManager& GetInstance();
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

//=======================================================================================
// Class responsible to construct / destroy the transformation pushed to the context.
// @bsiclass                                                    Eric.Paquet         02/15
//=======================================================================================
struct ContextTransform 
    {
    private:
        ViewContextR    m_viewContext;

    public:
        ContextTransform(ViewContextR context, Transform trf) : m_viewContext(context)
            {
            m_viewContext.PushTransform(trf);
            }

        ~ContextTransform()
            {
            m_viewContext.PopTransformClip();
            }
    };

END_BENTLEY_POINTCLOUDSCHEMA_NAMESPACE
