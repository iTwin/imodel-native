/*--------------------------------------------------------------------------------------+
|
|     $Source: PointCloudSchema/PointCloudRenderer.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <PointCloudSchemaInternal.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_POINTCLOUDSCHEMA
USING_NAMESPACE_BENTLEY_BEPOINTCLOUD

//----------------------------------------------------------------------------------------
// @bsimethod                                    Simon.Normand                   04/2012
//----------------------------------------------------------------------------------------
#define GETREDVALUE(colorRef)                          ((Byte)(colorRef & 0x000000FF))
#define GETGREENVALUE(colorRef)                        ((Byte)((colorRef>>8) & 0x00000FF))
#define GETBLUEVALUE(colorRef)                         ((Byte)((colorRef>>16) & 0x00000FF))

void halfBlend (BePointCloud::PointCloudColorDef& color1, BePointCloud::PointCloudColorDef const& color2)
    {
    uint32_t c1 = (((uint32_t)color1.GetBlue())<<16 | ((uint32_t)color1.GetGreen())<<8 | (uint32_t)color1.GetRed());
    uint32_t c2 = (((uint32_t)color2.GetBlue())<<16 | ((uint32_t)color2.GetGreen())<<8 | (uint32_t)color2.GetRed());
    uint32_t res = ((c1&0xfefeff)+(c2&0xfefeff))>>1;
    color1.SetRed(GETREDVALUE(res));
    color1.SetGreen(GETGREENVALUE(res));
    color1.SetBlue(GETBLUEVALUE(res));
    }
    
#define COLORREF_FROM_RGBCOLORDEF(colorRgb)             RGB(colorRgb.GetRed(), colorRgb.GetGreen(), colorRgb.GetBlue())
#define RGBCOLORDEF_FROM_COLORREF(colorRgb, colorRef)   colorRgb.GetRed() = GETREDVALUE(colorRef); colorRgb.GetGreen() = GETGREENVALUE(colorRef); colorRgb.GetBlue() = GETBLUEVALUE(colorRef)

// Macro version of halfBlend (RGBColorDef& color1, RGBColorDef const& color2)
#define HALFBLEND_MASKVALUE                             (0xfefeff)
#define HALFBLEND_COLOR_MASK(color)                     ((color)&HALFBLEND_MASKVALUE)
#define HALFBLEND_COLORREF(colorRef1, colorRef2)        ((HALFBLEND_COLOR_MASK(colorRef1) + HALFBLEND_COLOR_MASK(colorRef2)) >> 1)
#define HALFBLEND_RGB(colorRgb1, colorRgb2)             {COLORREF colorRef = HALFBLEND_COLORREF(COLORREF_FROM_RGBCOLORDEF(colorRgb1), COLORREF_FROM_RGBCOLORDEF(colorRgb2));\
                                                        RGBCOLORDEF_FROM_COLORREF(colorRgb1, colorRef);}

// halfBlend with color2 pre-computed
#define HALFBLEND_PRECOMPUTE(colorRgb)                  HALFBLEND_COLOR_MASK(COLORREF_FROM_RGBCOLORDEF(colorRgb))                         
#define HALFBLEND_COLORREF_1(colorRef1, colorRef2)      ((HALFBLEND_COLOR_MASK(colorRef1) + (colorRef2)) >> 1)
#define HALFBLEND_RGB_1(color1, colorRef2)              {COLORREF colorRef = HALFBLEND_COLORREF_1(COLORREF_FROM_RGBCOLORDEF(color1), (colorRef2));\
                                                        RGBCOLORDEF_FROM_COLORREF(color1, colorRef);}

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     2/2015
//----------------------------------------------------------------------------------------
PointCloudRenderer::PointCloudRenderer (uint32_t outputCapacity)
    : m_outputCapacity(outputCapacity)
    {
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     2/2015
//----------------------------------------------------------------------------------------
PointCloudRenderer::~PointCloudRenderer()
    {
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     2/2015
//----------------------------------------------------------------------------------------
ProgressiveTask::Completion PointCloudRenderer::DrawPointCloud(ViewContextR context, PointCloudClassificationSettings const* pClassifInfo, PointCloudSceneCR pointCloudScene)
    {
        bool sceneHasClassif = pointCloudScene.HasClassificationChannel();

        uint32_t channelFlags = (uint32_t)PointCloudChannelId::Xyz;
    
        if (pointCloudScene._HasRGBChannel ())
            channelFlags |= (uint32_t)PointCloudChannelId::Rgb;

        if (pClassifInfo && sceneHasClassif)
            {
            channelFlags |= (uint32_t)PointCloudChannelId::Rgb;
            channelFlags |= (uint32_t)PointCloudChannelId::Filter;
            channelFlags |= (uint32_t)PointCloudChannelId::Classification;
            }

        PointCloudVortex::StartDrawFrameMetrics();

        PThandle queryHandle = pointCloudScene.GetFrustumQueryHandle()->GetHandle();
        PointCloudVortex::ResetQuery(queryHandle);

        // POINTCLOUD_WIP_GR06_PointCloudDisplay - to do change density(or whatever they did in v8i) when DrawPurpose::UpdateDynamic.
        // ??required?? PtVortex::DynamicFrameRate (displayParams.fps);
        PointCloudVortex::SetQueryDensity (queryHandle, PtQueryDensity::QUERY_DENSITY_VIEW_COMPLETE, context.GetDrawPurpose() == DrawPurpose::Dynamics ? 0.2f : 1.0f);

        uint64_t numQueryPoints = 0;

        while(1)
            {
            if (context.CheckStop())
                {
                return ProgressiveTask::Completion::Aborted;
                }
            //we need to create buffers for each iteration as QV will reference it.
            PointCloudQueryBuffersPtr queryBuffers = PointCloudQueryBuffers::Create(DRAW_QUERYCAPACITY, channelFlags);
            numQueryPoints = queryBuffers->GetPoints(queryHandle);

            if (queryBuffers->GetNumPoints() == 0)
                break;

            if (pClassifInfo && sceneHasClassif)
                {
                ApplyClassification(*queryBuffers, pClassifInfo, context);
                }

            // Drop channels
            queryBuffers->SetIntensityChannel(NULL);
            queryBuffers->SetClassificationChannel(NULL);
            queryBuffers->SetNormalChannel(NULL);

#if defined (NEEDS_WORK_POINT_CLOUD)
            // Create buffers for drawing. These are the buffers that are eventually used by QV.
            RefCountedPtr<PointCloudDrawParams> buffer = PointCloudDrawParams::Create (queryBuffers->GetXyzChannel(), queryBuffers->GetRgbChannel());
            if (buffer.IsNull())
                break;

            DrawPointBuffer(context, *buffer);
#endif
            }

        int64_t ptsToLoad = PointCloudVortex::PtsToLoadInViewport(pointCloudScene.GetSceneHandle(), true/*recompute*/);

        PointCloudVortex::EndDrawFrameMetrics();

        return 0 == ptsToLoad ? ProgressiveTask::Completion::Finished : ProgressiveTask::Completion::Aborted;
    }

#ifdef NEEDS_WORK_CONTINUOUS_RENDER
//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     3/2015
//----------------------------------------------------------------------------------------
void PointCloudRenderer::DrawPointBuffer(ViewContextR context, PointCloudDrawParams& buffer) const
    {
    uint32_t numPoints = buffer.GetNumPoints();

    if (numPoints > m_outputCapacity)
        {
        for(uint32_t i(0); i<numPoints; i+=m_outputCapacity)
            {
            RefCountedPtr<PointCloudXyzChannel> pXyzChannel;
            RefCountedPtr<PointCloudRgbChannel> pRgbChannel;

            if(NULL != buffer->GetPointChannel())
                pXyzChannel = PointCloudXyzChannel::Create(buffer->GetPointChannel()->GetCapacity());
            if(NULL != buffer->GetRgbChannel())
                pRgbChannel = PointCloudRgbChannel::Create(buffer->GetRgbChannel()->GetCapacity());

            RefCountedPtr<PointCloudDrawParams> tmpBuffer = PointCloudDrawParams::Create (pXyzChannel.get(), pRgbChannel.get());

            if (tmpBuffer.IsNull())
                return;

            uint32_t end = std::min(numPoints, i + m_outputCapacity);
            tmpBuffer->InitFrom(i, end, NULL, buffer);

            context.GetIViewDraw().DrawPointCloud (tmpBuffer.get());

            if (context.CheckStop())
                break;
            }
        }
    else

      //temporary.  
        {
        Render::GraphicPtr pGraphic = context.CreateGraphic(Render::Graphic::CreateParams(context.GetViewport()));

        Render::GraphicParams graphicParams;
        graphicParams.SetLineColor(ColorDef::White());
        graphicParams.SetFillColor(ColorDef::White());
        graphicParams.SetWidth(1);
        pGraphic->ActivateGraphicParams(graphicParams);
        pGraphic->AddPointString(buffer._GetNumPoints(), buffer._GetDPoints());

        context.OutputGraphic(*pGraphic, nullptr);
        }

    }
#endif        

//----------------------------------------------------------------------------------------
// @bsimethod                                    Simon.Normand                   05/2014
//----------------------------------------------------------------------------------------
void classifyPoints (PointCloudClassificationSettings const* pClassifInfo, unsigned char* filterBuffer, unsigned char* pClassifChannel, ColorDef const* pClassifColors, BePointCloud::PointCloudColorDef* pRgbChannel, uint32_t nPoints, bool hasRgb, bool hasFilter)
    {
    bool bUnclassState = pClassifInfo->GetUnclassVisible();
    bool bBlendColor   = pClassifInfo->GetBlendColor();
    bool bUseBaseColor = pClassifInfo->GetUseBaseColor();
    ColorDef unclassCol = pClassifInfo->GetUnclassColor();
    BePointCloud::PointCloudColorDef rgbColorUnclass (unclassCol.GetRed(), unclassCol.GetGreen(), unclassCol.GetBlue());

    // Convert ColorDef to PointCloudColorDef
    PointCloudColorDef pointCloudClassifColor[CLASSIFICATION_COUNT];
    for (uint32_t i = 0; i < CLASSIFICATION_COUNT; i++)
        {
        pointCloudClassifColor[i].SetRed(pClassifColors[i].GetRed());
        pointCloudClassifColor[i].SetGreen(pClassifColors[i].GetGreen());
        pointCloudClassifColor[i].SetBlue(pClassifColors[i].GetBlue());
        }

    for (uint32_t ptIdx = 0; ptIdx < nPoints; ++ptIdx)
        {
        if (!PointCloudChannels_Is_Point_Hidden(filterBuffer[ptIdx]))
            {
            // Get classification value (the five least significant bits)
            const unsigned char classifValue = (pClassifChannel[ptIdx]);
            bool bClassState = pClassifInfo->GetVisibleState(classifValue);
            bool bClassActive = pClassifInfo->GetActiveState(classifValue);

            // Is classification visible?
            if ((bClassState && bClassActive) || 
                (!bClassActive && bUnclassState))       
                {
                // don't change the color of a selected point
                if (hasRgb && (!hasFilter || (hasFilter && !PointCloudChannels_Is_Point_Selected(filterBuffer[ptIdx]))))
                    {
                    // Assign classification color to point
                    if (!bClassActive && !bUnclassState)
                        PointCloudChannels_Hide_Point(filterBuffer[ptIdx]);

                    if (bBlendColor)
                        {
                        if(bClassActive)
                            {
                            halfBlend (pRgbChannel[ptIdx], pointCloudClassifColor[classifValue]);
                            }
                        else
                            {
                            halfBlend (pRgbChannel[ptIdx], rgbColorUnclass);
                            }
                        }
                    else if (!bUseBaseColor)
                        {
                        pRgbChannel[ptIdx] = bClassActive ? pointCloudClassifColor[classifValue] : rgbColorUnclass;
                        }
                    }
                }
            else //If classif not visible hide it
                PointCloudChannels_Hide_Point(filterBuffer[ptIdx]);
            }
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                    Stephane.Poulin                 10/2012
//----------------------------------------------------------------------------------------
void PointCloudRenderer::ApplyClassification(PointCloudQueryBuffers& channels, PointCloudClassificationSettings const* pClassifInfo, ViewContextR context) const
    {
    BeAssert(NULL != channels.GetClassificationChannel());
    BeAssert(NULL != channels.GetXyzChannel());

    // Fix for TFS#8078
    // Verify if background is white. If it is the case, when a classification color is white, change it to black, 
    // so that this class is visible even on a white background.
    bool backgroundIsWhite = false;
    ColorDef mediaColor;
    DgnViewportP contextVp = context.GetViewport();

    ColorDef whiteColor = {255,255,255};
    PointCloudClassificationSettings adjustedClassification;
    if (contextVp != NULL)
        {
        mediaColor = contextVp->GetBackgroundColor();
        if(memcmp(&mediaColor, &whiteColor, sizeof(whiteColor)) == 0)
            backgroundIsWhite = true;
        }
    ColorDef const* pClassifColors;
    if (!backgroundIsWhite)
        {
        // Background is not white. No need to modify classification colors.
        pClassifColors = pClassifInfo->GetClassificationColors();
        }
    else
        {
        // Background is white. Change white color to black.
        ColorDef blackColor = ColorDef::Black();

        // Take a copy of LasClassification, so that original classification is not altered. The color change will only be applied to display.
        memcpy (&adjustedClassification, pClassifInfo, sizeof (PointCloudClassificationSettings));

        ColorDef color;
        for (int idxClassif = 0; idxClassif < CLASSIFICATION_COUNT; idxClassif++)
            {
            color = adjustedClassification.GetClassificationColor ((unsigned char) idxClassif);
            if(color.GetRed() == whiteColor.GetRed() && color.GetGreen() == whiteColor.GetGreen() && color.GetBlue() == whiteColor.GetBlue())
                {
                // Original class color is white. Change it to black.
                adjustedClassification.SetClassificationColor(blackColor, (unsigned char)idxClassif);
                }
            }
        pClassifColors = adjustedClassification.GetClassificationColors();
        }
    
    // iterate over each points in the classification buffer
    bool hasRgb                     = channels.HasRgb();
    bool hasFilter                  = channels.HasFilter();
    unsigned char*  filterBuffer    = channels.GetFilterChannel()->GetChannelBuffer();
    unsigned char*  pClassifChannel = channels.GetClassificationChannel()->GetChannelBuffer();
    BePointCloud::PointCloudColorDef* pRgbChannel = hasRgb ? channels.GetRgbChannel()->GetChannelBuffer() : NULL;
    uint32_t        nPoints         = channels.GetNumPoints();

    classifyPoints (pClassifInfo, filterBuffer, pClassifChannel, pClassifColors, pRgbChannel, nPoints, hasRgb, hasFilter);
    }


