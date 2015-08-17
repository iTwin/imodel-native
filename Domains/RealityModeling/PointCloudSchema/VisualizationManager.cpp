/*--------------------------------------------------------------------------------------+
|
|     $Source: PointCloudSchema/VisualizationManager.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <PointCloudSchemaInternal.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_BEPOINTCLOUD
USING_NAMESPACE_BENTLEY_POINTCLOUDSCHEMA

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
VisualizationManager::VisualizationManager ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    12/2008
+---------------+---------------+---------------+---------------+---------------+------*/
VisualizationManager::~VisualizationManager ()
    {
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     2/2015
//----------------------------------------------------------------------------------------
IProgressiveDisplay::Completion VisualizationManager::DrawToContext(ViewContextR context, PointCloudSceneCR pointCloudScene, DRange3dCR pointCloudRange)
    {
    //&&MM use PTint64 ptPtsLoadedInViewportSinceDraw( PThandle forScene ) to detect that we have a good amount of pixels to redraw
    //          if we returned because of context.CheckStop() I think it would not make sense. it make sense only when ptPtsToLoadInViewport != 0

    DrawPurpose drawPurpose = context.GetDrawPurpose ();

    // Get transformation to UOR (including transformation for the coordinate system)
    Transform pointCloudTransform;
    DRange3d rangeUOR;
    pointCloudScene.GetRange (rangeUOR, true);
    WString wktString(pointCloudScene.GetSurveyGeoreferenceMetaTag());
    PointCloudGcsFacility::GetTransformToUor(pointCloudTransform, wktString, rangeUOR, context.GetViewport()->GetViewController().GetTargetModel()->GetDgnDb());

    // Setup viewport
    // set the pointool viewport to use for this model, scene and view
    DRange3d pcRange;
    pointCloudTransform.Multiply(pcRange, pointCloudRange);

    PTViewportID viewportId(context.GetViewport()->GetViewController().GetTargetModel(), const_cast<PointCloudScene*>(&pointCloudScene), pcRange);

    PointCloudVortex::SetViewport(ModelViewportManager::Get ().GetViewport (viewportId));
    SetViewportInfo (context, pointCloudTransform, pcRange);
    // Get view settings
    RefCountedPtr<IPointCloudViewSettings> viewSettings = PointCloudViewSettings::GetPointCloudViewSettings(context);

    // POINTCLOUD_WIP_GR06 - calling SetUseRGB here is temporary. We set the display style to RGB until we have an application that can 
    //                       handle the view settings.
    viewSettings->SetUseRGB(true);
    viewSettings->SetNeedClassifBuffer(true);

    // May need to override view settings based on the scene characteristics and the view\print background color
    OverrideViewSettings(viewSettings, context, pointCloudScene);

    DgnViewportP viewport = context.GetViewport();

    // Examples how to use viewSettings:
    // Intensity
    // viewSettings->SetDisplayStyle(IPointCloudViewSettings::DisplayStyle_Intensity);
    // WCharCP rampName = PointCloudRamps::GetInstance().GetIntensityRampName(2).c_str();
    // viewSettings->SetIntensityRamp(rampName);
    //
    // Elevation
    // viewSettings->SetDisplayStyle(IPointCloudViewSettings::DisplayStyle_Location);
    // WCharCP planeRampName = PointCloudRamps::GetInstance().GetPlaneRamps().at(2).c_str();
    // viewSettings->SetPlaneRamp(planeRampName);
    // viewSettings->SetDistance(20);
    // viewSettings->SetClampIntensity(true);

    UpdateVortexShaders (viewSettings, *viewport);

    // POINTCLOUD_WIP_GR06 - For now, set density to 1.0. We can change that when the application will have a way to set the density.
    float density = 1.0;

    PtQueryDensity densityType = PtQueryDensity::QUERY_DENSITY_VIEW; // Get only points in memory for a view representation. Point still on disk will get loaded at a later time.

    PointCloudQueryHandlePtr queryHandle (pointCloudScene.GetFrustumQueryHandle());

    if (IsDrawPurposeValidForFastUpdate(drawPurpose))
        {
        // Use fast params
        // POINTCLOUD_WIP_GR06_Density - Use 0.10 for now. Eventually, we could maybe get this from user preferences.
        density = __min (density, 0.10f);
        }
    else if (IsDrawPurposeValidForCompleteUpdate(drawPurpose))
        {
        densityType = PtQueryDensity::QUERY_DENSITY_VIEW_COMPLETE;   // Get every points (memory and disk) needed for a view representation. (Display is not progressive).
        }

    PointCloudVortex::ResetQuery(queryHandle->GetHandle());
    PointCloudVortex::SetQueryDensity (queryHandle->GetHandle(), densityType, density);

    LasClassificationInfo const*   pClassifInfo = NULL;
    IPointCloudClassificationViewSettingsPtr pClassifSettings;

    // Get classification info if necessary
    if (viewSettings->GetNeedClassifBuffer())
        {
        //Use the context and the element model ref so we can use different style
        //when the element is from a reference attachement file
        //IPointCloudClassificationViewSettingsPtr pClassifSettings = (IPointCloudClassificationViewSettings::GetPointCloudClassificationViewSettings(viewport->GetViewNumber()));
        pClassifSettings = (IPointCloudClassificationViewSettings::GetPointCloudClassificationViewSettings(context));
        bool getClassifBuffer = false;

        //Make sure we need the classif channel
        if (    viewSettings->GetNeedClassifBuffer()    // Need classif buffer in the display style 
            || !pClassifSettings->GetUnclassState()     // Undefined Class is inactive in the view
            ||  viewSettings->GetDisplayStyle() == IPointCloudViewSettings::DisplayStyle_Classification // DS from SS3
            )
            {
            getClassifBuffer = true;
            }
        else
            {
            //as soon as one active class is disabled we need to get the classif buffer
            for (int i = 0; i < CLASSIFCATION_COUNT; i++) 
                {
                if (pClassifSettings->GetClassActiveState((unsigned char)i) && !pClassifSettings->GetClassificationState((unsigned char)i)) 
                    {
                    getClassifBuffer = true;
                    break;
                    }
                }
            }

        if (getClassifBuffer)
            pClassifInfo = dynamic_cast<PointCloudClassificationViewSettings*>(pClassifSettings.get())->GetLasClassificationInfo();
        }

    // Push transformation to UOR
    ContextTransform pushTransform(context, pointCloudTransform);

    PointCloudRenderer renderer(DRAW_OUTPUTCAPACITY);
    IProgressiveDisplay::Completion status = renderer.DrawPointCloud(context, pClassifInfo, pointCloudScene);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    01/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void VisualizationManager::SetViewportInfo (ViewContextR context, Transform const& sceneToUor, DRange3d const& range)
    {
    // TR #311525 with a cube pointcloud, 8 points, not all of the points are drawn as the rangeVec is cropping some out.
    // This happens only when the points fit precisely on the range.  So with an exact meter cubed or km cubed
    // To Fix this, inflate the range vector. Here I'm not sure what's the best way to do this, scale it with a percentage or translate by a single UOR 
    DRange3d scaledRange;
#if 0
    RotMatrix rotOrg; rotOrg.initFromScale (1-0.0001);
    rotOrg.Multiply (&scaledRange.org, &vector.org, 1);
    RotMatrix rotEnd; rotEnd.initFromScale (1+0.0001);
    rotEnd.Multiply (&scaledRange.end, &vector.end, 1);
#else
    Transform transOrg; transOrg.InitFrom (-1,-1,-1);
    transOrg.Multiply (&scaledRange.low, &range.low, 1);
    Transform transEnd; transEnd.InitFrom (1,1,1);
    transEnd.Multiply (&scaledRange.high, &range.high, 1);
#endif

    if(context.IsCameraOn())
        {
        DVec3d      npcPoints[8], viewPoints [8];

        GetNpcViewBox (npcPoints, context);
        context.NpcToWorld (viewPoints, npcPoints, 8);
        double scale = ComputeWorldScale(viewPoints);

        if (scale <= 0.999)
             return SetViewportProjectionFromSceneRange(context, sceneToUor, scaledRange);
        }

    SetViewportOrtho(context, sceneToUor, scaledRange);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  11/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void VisualizationManager::SetViewportProjectionFromSceneRange(ViewContextR context, Transform const& sceneToUor, DRange3d const& range)
    {
    Transform uorToScene;
    uorToScene.InverseOf(sceneToUor);

    // Get View Npc range
    DRange3d viewRange_npc; GetNpcViewRange(viewRange_npc, context);
    
    // Compute the point cloud Npc corners
    DRange3d pointCloudRange_local; pointCloudRange_local.InitFrom(range.low, range.high);
    DRange3d pointCloudRange_npc; 
    LocalRangeToNpcRange(pointCloudRange_npc, pointCloudRange_local, context);

    // intersect the point cloud with the view (Npc)
    DRange3d subViewRange_npc; subViewRange_npc.IntersectionOf(pointCloudRange_npc, viewRange_npc);
    DPoint3d subViewBox_npc[NPC_CORNER_COUNT];
    subViewRange_npc.Get8Corners(subViewBox_npc);

    // Convert the intersected box into view coordinates.
    DPoint3d subViewBox_view[NPC_CORNER_COUNT];
    context.NpcToView(&subViewBox_view[0], &subViewBox_npc[0], 8);
    DRange3d subViewRange_view; subViewRange_view.InitFrom(subViewBox_view, NPC_CORNER_COUNT);

    int horizontalPixels = (int)(subViewRange_view.high.x - subViewRange_view.low.x + 0.5);  //  + 1?
    int verticalPixels  =  (int)(subViewRange_view.high.y - subViewRange_view.low.y + 0.5);  //  + 1?

    PointCloudVortex::SetViewportSize ((int)(subViewRange_view.low.x + 0.5), (int)(subViewRange_view.low.y + 0.5), horizontalPixels, verticalPixels);
    
    DPoint3d subViewBox_local[NPC_CORNER_COUNT];
    context.ViewToLocal(subViewBox_local, subViewBox_view, NPC_CORNER_COUNT);
    DVec3d subViewBox_scene[NPC_CORNER_COUNT];
    uorToScene.Multiply (subViewBox_scene, subViewBox_local, NPC_CORNER_COUNT);

    PerspectiveViewParams subViewParams;
    ViewParametersFromWorld(subViewParams, subViewBox_scene);

    PointCloudVortex::SetViewEyeLookAt((double*)&subViewParams.cameraRoot, (double*)&subViewParams.targetRoot, (double*)&subViewParams.viewY );
    PointCloudVortex::SetViewProjectionFrustum( subViewParams.xmin, subViewParams.xmax, subViewParams.ymin, subViewParams.ymax, subViewParams.front, subViewParams.back );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  11/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void VisualizationManager::SetViewportOrtho(ViewContextR context, Transform const& sceneToUor, DRange3d const& vector)
    {
    // Get the 8 Npc corners of the view
    DPoint3d viewBox_npc[NPC_CORNER_COUNT];
    GetNpcViewBox(viewBox_npc, context);
    DRange3d viewRange_npc; viewRange_npc.InitFrom(viewBox_npc, NPC_CORNER_COUNT);

    // Compute the point cloud Npc corners
    DRange3d pointCloudRange_local; pointCloudRange_local.InitFrom(vector.low, vector.high);
    DPoint3d pointCloudBox[NPC_CORNER_COUNT];
    pointCloudRange_local.Get8Corners(pointCloudBox);
    context.LocalToView(&pointCloudBox[0], &pointCloudBox[0], NPC_CORNER_COUNT);
    context.ViewToNpc(&pointCloudBox[0], &pointCloudBox[0], NPC_CORNER_COUNT);

    // intersect the point cloud with the view
    DRange3d pointCloudRange_npc; pointCloudRange_npc.InitFrom(pointCloudBox, NPC_CORNER_COUNT);
    viewRange_npc.IntersectionOf(pointCloudRange_npc, viewRange_npc);
    viewRange_npc.Get8Corners(viewBox_npc);

    // Convert the intersected box into view coordinates.
    DPoint3d viewBox_view[NPC_CORNER_COUNT];
    context.NpcToView(&viewBox_view[0], &viewBox_npc[0], NPC_CORNER_COUNT);
    DRange3d viewRange_view; viewRange_view.InitFrom(viewBox_view, NPC_CORNER_COUNT);

    int horizontalPixels = (int)(viewRange_view.high.x - viewRange_view.low.x + 0.5);  //  + 1?
    int verticalPixels  =  (int)(viewRange_view.high.y - viewRange_view.low.y + 0.5);  //  + 1?

    PointCloudVortex::SetViewportSize ((int)(viewRange_view.low.x + 0.5), (int)(viewRange_view.low.y + 0.5), horizontalPixels, verticalPixels);

    DMatrix4d sceneToUorMat, localToView = context.GetLocalToView();
    bsiDMatrix4d_initFromTransform (&sceneToUorMat, &sceneToUor);

    // TR# 296246. - On viewlets (and the sheet/drawing parents) the "View" Z coordinates are somewhat 
    // arbitrary and sending very large values seems to confuse the vortex stuff.
    // So here we normalize the view Z values (in both the points and the matris) to the viewport Z range.   (RBB 05/2010).
//   REMOVE for TR 312595. it looks like the correction for 311221 has fixed this
//     if (context.InViewlet())
//         {
//         DPoint3d npcPoints[8], viewViewPoints[8];
// 
//         GetNpcViewBox (npcPoints, context);
//         context.NpcToView (viewViewPoints, npcPoints, 8);
//         normalizeViewZRange (viewViewPoints, localToView, context);
//         }

    // Now convert to a coordinate system with (0,0) at the bottom left
    DMatrix4d toOpenGL;
    GetUstnViewToOpenGLViewTransform (toOpenGL, viewRange_view.low.y, viewRange_view.high.y);

    DMatrix4d modelView;
    modelView.InitProduct (toOpenGL, localToView, sceneToUorMat);

    PointCloudVortex::SetViewEyeMatrix (modelView.coff [0], true /* Row Major */);
    // invert the low and high Z for openGL, we think!
    PointCloudVortex::SetViewProjectionOrtho (viewRange_view.low.x, viewRange_view.high.x, viewRange_view.low.y, viewRange_view.high.y, /*near*/-viewRange_view.low.z, /*far*/-viewRange_view.high.z);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void VisualizationManager::LocalRangeToNpcRange(DRange3d& npcRange, DRange3d const& localRange, ViewContextR context)
    {
    DPoint3d box[8];
    localRange.Get8Corners(box);

    if (SUCCESS == LocalToViewAccountingForPointBehindCamera(box, box, 8, context))
        {
        context.ViewToNpc(box, box, 8);
        npcRange.InitFrom(box, 8);
        }
    else
        {
        // Return Npc view range
        GetNpcViewRange(npcRange, context);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void VisualizationManager::ViewParametersFromWorld(PerspectiveViewParams& viewParams, DPoint3dCP world)
    {
    viewParams.viewX.NormalizedDifference (world[NPC_101], world[NPC_001]);
    viewParams.viewY.NormalizedDifference (world[NPC_011], world[NPC_001]);
    viewParams.viewZ.NormalizedCrossProduct (viewParams.viewX, viewParams.viewY);

    // Calculate camera position and target from frustum directly. - Note we cannot take these directly from the viewport as those values
    // are not correct when processing viewlets.
    double  compressionFraction = ComputeWorldScale(world);
    double  a = 1.0 / (1.0 - compressionFraction);  // compressionFaction tested < .999 before calling this method.
    
    DVec3d  worldZ; worldZ.DifferenceOf (world[NPC_001], world[NPC_000]);
    viewParams.cameraRoot.SumOf (world[NPC_000],worldZ, a);
    
    viewParams.xmin  = world[NPC_001].DotDifference (viewParams.cameraRoot, viewParams.viewX);
    viewParams.xmax  = world[NPC_101].DotDifference (viewParams.cameraRoot, viewParams.viewX);
    viewParams.ymin  = world[NPC_001].DotDifference (viewParams.cameraRoot, viewParams.viewY);
    viewParams.ymax  = world[NPC_011].DotDifference (viewParams.cameraRoot, viewParams.viewY);
    viewParams.front = -world[NPC_001].DotDifference (viewParams.cameraRoot, viewParams.viewZ);
    viewParams.back  = -world[NPC_000].DotDifference (viewParams.cameraRoot, viewParams.viewZ);

    // The "target" value is somewhat arbitrary - just calculate it at the back clipping plane.
    viewParams.targetRoot.SumOf (viewParams.cameraRoot,viewParams.viewZ, -viewParams.back);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    02/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void VisualizationManager::GetUstnViewToOpenGLViewTransform(DMatrix4dR result, double minY, double maxY)
    {
    double      yOffset = minY + (maxY - minY)/2;

    DPoint3d    scale;
    scale.Init (1, -1, 1);
    DPoint3d    offset;
    offset.Init (0, 2 * yOffset, 0);

    //  This equivalent to translating by -yOffset, scaling by -1,
    //  and then translating by yOffset.
    bsiDMatrix4d_initScaleAndTranslate(&result, &scale, &offset);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt VisualizationManager::LocalToViewAccountingForPointBehindCamera(DPoint3dP viewPoints, DPoint3dCP localPoints, uint32_t nbPoints, ViewContextR context)
    {
    for (uint32_t i=0; i<nbPoints; ++i)
        {
        DPoint4d point4d;
        static double   s_minW = .001;                  // Arbitrary front of eye lmit.

        point4d.Init (localPoints[i], 1.0);
        context.GetLocalToView().Multiply(point4d, point4d);

        if (point4d.w < s_minW)
            return ERROR;

        point4d.NormalizeWeightInPlace ();
        viewPoints[i].XyzOf(point4d);
        }

    return SUCCESS;
    }

//----------------------------------------------------------------------------------------
// This method try to determine if the point cloud will draw white on a white background.
// If so, it overrides the view settings to draw the point cloud in black.
// @bsimethod                                                      StephanePoulin  03/2010
//----------------------------------------------------------------------------------------
void VisualizationManager::OverrideViewSettings(RefCountedPtr<PointCloudSchema::IPointCloudViewSettings> settingsPtr, ViewContextR context, PointCloudSceneCR pointCloudScene)
    {
    IPointCloudViewSettings::PointCloudDisplayStyles displayStyle = settingsPtr->GetDisplayStyle();
    if (displayStyle == IPointCloudViewSettings::DisplayStyle_Intensity)
        {
        if (pointCloudScene.HasIntensityChannel())
            return;
        }
    else if (displayStyle == IPointCloudViewSettings::DisplayStyle_Location)
        {
        return; 
        }
    else if (displayStyle == IPointCloudViewSettings::DisplayStyle_Classification)
        {
        if (pointCloudScene.HasClassificationChannel())
            return; 
        }
    else if (displayStyle == IPointCloudViewSettings::DisplayStyle_Custom)
        {
        return;
        }
    else
        {
        BeAssert (displayStyle == IPointCloudViewSettings::DisplayStyle_None);
        if (!settingsPtr->GetUseRGB())
            return; // Point cloud should display using element color

        if (settingsPtr->GetUseRGB() && pointCloudScene.HasRGBChannel())
            return; // We are in Rgb display Style and the point cloud has a Rbg channel.
        }

    DgnViewportP contextVp = context.GetViewport();

    if (NULL == contextVp)
        return;

    ColorDef mediaColor;
    mediaColor = contextVp->GetWindowBgColor();

    ColorDef whiteColor = {255,255,255};

    // We get here because we are in an inconsistent state(i.e. Classification display style but no classification channel.) and we want to avoid to display 
    // white points on a white background.
    // The default point color is white, maybe we want to use the element color instead
    if(memcmp(&mediaColor, &whiteColor, sizeof(whiteColor)) == 0)
        {
        // the background color is white, adjust the view settings to draw the point cloud in black.
        settingsPtr->SetDisplayStyle(IPointCloudViewSettings::DisplayStyle_Location);
        settingsPtr->SetContrast(POINTCLOUD_DEFAULT_VIEW_CONTRAST);
        settingsPtr->SetBrightness(POINTCLOUD_DEFAULT_VIEW_BRIGHTNESS);
        settingsPtr->SetPlaneRamp(BLACK_RAMP);
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                   Simon.Normand                   11/2009
//----------------------------------------------------------------------------------------
void VisualizationManager::UpdateVortexShaders (RefCountedPtr<PointCloudSchema::IPointCloudViewSettings> const& settingsPtr, DgnViewportR viewport)
    {
    /* POINTCLOUD_WIP_GR06_PointCloudDisplay - adjust correctly with display params
    DisplayQueryParams& displayParams(GetDisplayParam());

    PtVortex::DynamicFrameRate (displayParams.fps);
    */

    // change the global density has no effect once points have been loaded already.
    // instead, alter the display query density 
    PointCloudVortex::GlobalDensity (1.0f);

    //POINTCLOUD_WIP_GR06_PointCloudDisplay (adjust) - PointCloudVortex::StaticOptimizer(displayParams.staticOptimizer);

    PointCloudVortex::SetEnabledState (PtEnable::RGB_SHADER, settingsPtr->GetUseRGB ());
    PointCloudVortex::SetEnabledState (PtEnable::FRONT_BIAS, settingsPtr->GetUseFrontBias ());
    PointCloudVortex::SetEnabledState (PtEnable::ADAPTIVE_POINT_SIZE, false);
    PointCloudVortex::SetEnabledState (PtEnable::LIGHTING, false);

    IPointCloudViewSettings::PointCloudDisplayStyles displayStyle = settingsPtr->GetDisplayStyle();
    if (displayStyle == IPointCloudViewSettings::DisplayStyle_Custom)

        {
        PointCloudVortex::SetEnabledState (PtEnable::INTENSITY_SHADER, settingsPtr->GetUseIntensity());
        PointCloudVortex::SetEnabledState (PtEnable::PLANE_SHADER, settingsPtr->GetUsePlane());
        if (settingsPtr->GetUseIntensity()) //if intensity set to true
            {
            PointCloudVortex::ShaderOptionf( PtShaderOptions::INTENSITY_SHADER_BRIGHTNESS, settingsPtr->GetBrightness () );
            PointCloudVortex::ShaderOptionf( PtShaderOptions::INTENSITY_SHADER_CONTRAST, settingsPtr->GetContrast () );
            PointCloudVortex::ShaderOptioni( PtShaderOptions::INTENSITY_SHADER_RAMP, settingsPtr->GetIntensityRampIndex ());
            BeAssert (settingsPtr->GetIntensityRampIndex() != INVALID_RAMP_INDEX);
            }

        if (settingsPtr->GetUsePlane()) //if elevation is set to true
            {
            PointCloudVortex::ShaderOptioni( PtShaderOptions::PLANE_SHADER_RAMP, settingsPtr->GetPlaneRampIndex ());
            BeAssert (settingsPtr->GetPlaneRampIndex() != INVALID_RAMP_INDEX);
            PointCloudVortex::ShaderOptionf (PtShaderOptions::PLANE_SHADER_DISTANCE, settingsPtr->GetDistance ());
            PointCloudVortex::ShaderOptionf (PtShaderOptions::PLANE_SHADER_OFFSET, settingsPtr->GetOffset ());

            //Clamp elevation value
            if (settingsPtr->GetClampIntensity())
                PointCloudVortex::ShaderOptioni (PtShaderOptions::PLANE_SHADER_EDGE, 0x01);
            else
                PointCloudVortex::ShaderOptioni (PtShaderOptions::PLANE_SHADER_EDGE, 0x00);

            float axis [] = {0,0,0};
            if (settingsPtr->GetACSAsPlaneAxis () && IACSManager::GetManager().GetActive (viewport))
                {
                RotMatrix rot;
                DVec3d direction;

                IACSManager::GetManager().GetActive (viewport)->GetRotation (rot);
                rot.GetRow(direction, 2);
                direction.Normalize ();
                axis[0]= (float)direction.x;
                axis[1]= (float)direction.y;
                axis[2]= (float)direction.z;
                }
            else
                axis [__min (2, settingsPtr->GetPlaneAxis ())] = 1.0f;

            PointCloudVortex::ShaderOptionfv( PtShaderOptions::PLANE_SHADER_VECTOR, axis ); 
            }
        }
    else if (displayStyle == IPointCloudViewSettings::DisplayStyle_Intensity)
        {
        PointCloudVortex::SetEnabledState (PtEnable::INTENSITY_SHADER, true);
        PointCloudVortex::SetEnabledState (PtEnable::PLANE_SHADER, false);
        PointCloudVortex::ShaderOptionf( PtShaderOptions::INTENSITY_SHADER_BRIGHTNESS, settingsPtr->GetBrightness () );
        PointCloudVortex::ShaderOptionf( PtShaderOptions::INTENSITY_SHADER_CONTRAST, settingsPtr->GetContrast () );
        PointCloudVortex::ShaderOptioni( PtShaderOptions::INTENSITY_SHADER_RAMP, settingsPtr->GetIntensityRampIndex ());
        BeAssert (settingsPtr->GetIntensityRampIndex() != INVALID_RAMP_INDEX);
        }
    else if (displayStyle == IPointCloudViewSettings::DisplayStyle_Location)
        {
        PointCloudVortex::SetEnabledState (PtEnable::INTENSITY_SHADER, false);
        PointCloudVortex::SetEnabledState (PtEnable::PLANE_SHADER, true);
        PointCloudVortex::ShaderOptioni( PtShaderOptions::PLANE_SHADER_RAMP, settingsPtr->GetPlaneRampIndex ());

        BeAssert (settingsPtr->GetPlaneRampIndex() != INVALID_RAMP_INDEX);

        //Clamp elevation value
        if (settingsPtr->GetClampIntensity())
            PointCloudVortex::ShaderOptioni (PtShaderOptions::PLANE_SHADER_EDGE, 0x01);
        else
            PointCloudVortex::ShaderOptioni (PtShaderOptions::PLANE_SHADER_EDGE, 0x00);

        PointCloudVortex::ShaderOptionf (PtShaderOptions::PLANE_SHADER_DISTANCE, settingsPtr->GetDistance ());
        PointCloudVortex::ShaderOptionf (PtShaderOptions::PLANE_SHADER_OFFSET, settingsPtr->GetOffset ());

        float axis [] = {0,0,0};
        if (settingsPtr->GetACSAsPlaneAxis () && IACSManager::GetManager().GetActive (viewport))
            {
            RotMatrix rot;
            DVec3d direction;
            IACSManager::GetManager().GetActive (viewport)->GetRotation (rot);
            rot.GetRow(direction,  2);
            direction.Normalize ();
            axis[0]= (float)direction.x;
            axis[1]= (float)direction.y;
            axis[2]= (float)direction.z;
            }
        else
            axis [__min (2, settingsPtr->GetPlaneAxis ())] = 1.0f;
        PointCloudVortex::ShaderOptionfv( PtShaderOptions::PLANE_SHADER_VECTOR, axis ); 
        }
    else
        {
        BeAssert (displayStyle == IPointCloudViewSettings::DisplayStyle_None || 
                    displayStyle == IPointCloudViewSettings::DisplayStyle_Classification);
        PointCloudVortex::SetEnabledState (PtEnable::INTENSITY_SHADER, false);
        PointCloudVortex::SetEnabledState (PtEnable::PLANE_SHADER, false);
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                    StephanePoulin  11/2009
//----------------------------------------------------------------------------------------
bool VisualizationManager::IsDrawPurposeValidForFastUpdate(DrawPurpose drawPurpose)
    {
    switch (drawPurpose)
        {
        case DrawPurpose::UpdateDynamic:
        case DrawPurpose::Dynamics:
            {
            return true;
            }
        }
    return false;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                    StephanePoulin  11/2009
//----------------------------------------------------------------------------------------
bool VisualizationManager::IsDrawPurposeValidForCompleteUpdate(DrawPurpose drawPurpose)
    {
    switch (drawPurpose)
        {
        case DrawPurpose::Plot:
        case DrawPurpose::GenerateThumbnail:
        case DrawPurpose::ModelFacet:
            {
            return true;
            }
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void VisualizationManager::GetNpcViewRange (DRange3d& npcRange, ViewContextR context)
    {
//     ViewContext::ModelRefMark       modelRefMark (context);        
// 
//     if (modelRefMark.m_useNpcSubRange)
//         npcRange        = modelRefMark.m_npcSubRange;
//     else
        npcRange.InitFrom (0.0, 0.0, 0.0, 1.0, 1.0, 1.0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void VisualizationManager::GetNpcViewBox (DPoint3d npcPoints[8], ViewContextR context)
    {
    //ViewContext::ModelRefMark       modelRefMark (context);
    DRange3d                        npcRange;
    GetNpcViewRange(npcRange, context);

    npcRange.Get8Corners (npcPoints);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  11/2009
+---------------+---------------+---------------+---------------+---------------+------*/
double VisualizationManager::ComputeWorldScale(DPoint3dCP world)
    {
    DVec3d backViewX, frontViewX;
    backViewX.DifferenceOf (world[NPC_100], world[NPC_000]);
    frontViewX.DifferenceOf (world[NPC_101], world[NPC_001]);

    return  frontViewX.Magnitude () / backViewX.Magnitude ();
    }

/*---------------------------------------------------------------------------------**//**
* Management of the VisualizationManager singleton
+---------------+---------------+---------------+---------------+---------------+------*/
// static VisualizationManager instance
VisualizationManager* VisualizationManager::s_instance=NULL;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  11/2009
+---------------+---------------+---------------+---------------+---------------+------*/
VisualizationManager& VisualizationManager::GetInstance()
    {
    if (NULL == s_instance)
        s_instance = new VisualizationManager();

    return *s_instance;
    }

//========================================================================================
// This class deletes the static VisualizationManager pointer in its destructor.
// @bsiclass                                                        Eric.Paquet     1/2015
//========================================================================================
BEGIN_BENTLEY_POINTCLOUDSCHEMA_NAMESPACE
struct VisualizationManagerDestroyer
    {
    public:
        VisualizationManagerDestroyer() {}
        ~VisualizationManagerDestroyer()
            {
            if (VisualizationManager::s_instance!=NULL)
                delete VisualizationManager::s_instance;
            }
    private:
        VisualizationManagerDestroyer(VisualizationManagerDestroyer const&);  //disabled
        VisualizationManagerDestroyer& operator=(VisualizationManagerDestroyer const&);   //disabled
    };
END_BENTLEY_POINTCLOUDSCHEMA_NAMESPACE

VisualizationManagerDestroyer s_visualizationManagerDestroyer;
