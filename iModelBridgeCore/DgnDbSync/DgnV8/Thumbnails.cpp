/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnV8/Thumbnails.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ConverterInternal.h"

BEGIN_DGNDBSYNC_DGNV8_NAMESPACE

enum class DgnViewType {None=0, Physical=1<<0, Drawing=1<<1 };

ENUM_IS_FLAGS(DgnViewType);

//=======================================================================================
// @bsiclass                                                    Bern.McCarty   12/2012
//=======================================================================================
struct ThumbnailConfig
    {
    private:
        int m_resolution;
        DgnViewType m_viewTypes;
        RenderMode m_renderModeOverride;
        bool m_useRenderModeOverride;

    public:
        ThumbnailConfig(Converter::Config&);
        int GetResolution() {return m_resolution;}
        DgnViewType GetViewTypes() {return m_viewTypes;}
        RenderMode GetRenderModeOverride() {return m_renderModeOverride;}
        DgnViewType GetViewTypeFromString(Utf8StringCR viewStr);
        bool GetRenderModeOverrideFromString(RenderMode& mode, Utf8StringCR str);
        bool GetUseRenderModeOverride() {return m_useRenderModeOverride;}
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Bern.McCarty    12/2012
//---------------------------------------------------------------------------------------
DgnViewType ThumbnailConfig::GetViewTypeFromString(Utf8StringCR viewStr)
    {
    if (viewStr.empty())
        return DgnViewType::None;

    Utf8CP tokenP = ::strtok(const_cast<Utf8P>(viewStr.c_str()), " \t\n");
    int dgnViewType = 0;
    while (NULL != tokenP)
        {
        if (0 == strcmp("Physical", tokenP))
            dgnViewType |= (int)DgnViewType::Physical;
        else if (0 == strcmp("Drawing", tokenP))
            dgnViewType |= (int)DgnViewType::Drawing;
        tokenP = ::strtok(NULL, " \t\n");
        }

    return (DgnViewType) dgnViewType;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Bern.McCarty    12/2012
//---------------------------------------------------------------------------------------
bool ThumbnailConfig::GetRenderModeOverrideFromString(RenderMode& mode, Utf8StringCR str)
    {
    if (str.empty())
        return false;

    Utf8P tokenP = ::strtok(const_cast<Utf8P>(str.c_str()), " \t\n");

    if (0 == strcmp("DgnRenderMode::Wireframe", tokenP))
        {
        mode = RenderMode::Wireframe;
        return true;
        }

    if (0 == strcmp("DgnRenderMode::HiddenLine", tokenP))
        {
        mode = RenderMode::HiddenLine;
        return true;
        }

    if (0 == strcmp("DgnRenderMode::SolidFill", tokenP))
        {
        mode = RenderMode::SolidFill;
        return true;
        }

    if (0 == strcmp("DgnRenderMode::SmoothShade", tokenP))
        {
        mode = RenderMode::SmoothShade;
        return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
ThumbnailConfig::ThumbnailConfig(Converter::Config& config)
    {
    m_resolution = static_cast <int> (config.GetXPathInt64("/ImportConfig/Thumbnails/@pixelResolution", 768));
    if (m_resolution < 64 || m_resolution > 1600)
        m_resolution = 768;

    Utf8String viewTypes = config.GetXPathString("/ImportConfig/Thumbnails/@viewTypes", "");
    m_viewTypes = GetViewTypeFromString(viewTypes);

    Utf8String renderOverrides = config.GetXPathString("/ImportConfig/Thumbnails/@renderModeOverride", "");
    m_useRenderModeOverride = GetRenderModeOverrideFromString(m_renderModeOverride, renderOverrides);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Converter::GenerateThumbnails()
    {
#define ELEMENT_TILE_GENERATE_THUMBNAILS
#if defined(ELEMENT_TILE_GENERATE_THUMBNAILS)
    ThumbnailConfig thumbnailConfig(m_config);

    if ((DgnViewType::None == thumbnailConfig.GetViewTypes()) || !_GetParams().WantThumbnails())
        return BSISUCCESS;

    SetStepName(ProgressMessage::STEP_CREATE_THUMBNAILS());
    AddTasks((int32_t)ViewDefinition::QueryCount(*m_dgndb));

    // Initalize the graphics subsystem, to produce thumbnails.
    DgnViewLib::GetHost().GetViewManager().Startup();

    bool wantSpatial = DgnViewType::Physical == (thumbnailConfig.GetViewTypes() & DgnViewType::Physical);
    bool wantDrawing = DgnViewType::Drawing == (thumbnailConfig.GetViewTypes() & DgnViewType::Drawing);
    for (auto const& entry : ViewDefinition::MakeIterator(*m_dgndb))
        {
        auto view = ViewDefinition::Get(*m_dgndb, entry.GetId());
        if (!view.IsValid() || view->IsPrivate())
            continue;

        if ((!wantSpatial && view->IsSpatialView()) || (!wantDrawing && view->IsDrawingView()))
            continue;

        BeDuration timeout = BeDuration::FromSeconds(m_config.GetOptionValueDouble("ThumbnailTimeout", 30));
        if (_GetParams().GetThumbnailTimeout() != 0)
            timeout = _GetParams().GetThumbnailTimeout();

        SetTaskName(ProgressMessage::TASK_CREATING_THUMBNAIL(), entry.GetName());
        Render::RenderMode mode = thumbnailConfig.GetRenderModeOverride();
        Point2d size = {thumbnailConfig.GetResolution(), thumbnailConfig.GetResolution()};

        view->RenderAndSaveThumbnail(size, thumbnailConfig.GetUseRenderModeOverride() ? &mode : nullptr, timeout);
        ReportProgress();
        if (WasAborted())
            break;
        }
#endif

    return BSISUCCESS;
    }

END_DGNDBSYNC_DGNV8_NAMESPACE
