/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "ConverterInternal.h"
#if defined(BENTLEY_WIN32)
    #include <windows.h>
#endif

BEGIN_DGNDBSYNC_DGNV8_NAMESPACE

enum class DgnViewType {None=0, Physical=1<<0, Drawing=1<<1, Root=1<<2, };

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
        else if (0 == strcmp("Root", tokenP))
            dgnViewType |= (int)DgnViewType::Root;
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
* @bsimethod                                                    Ray.Bentley     06/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool Converter::ThumbnailUpdateRequired(ViewDefinition const& view)
    {
    if (!view.HasThumbnail())
        return true;

    auto    view2d  = view.ToView2d();
    if (nullptr != view2d)
        {
        if (m_unchangedModels.find(view2d->GetBaseModelId()) == m_unchangedModels.end())
            return true;

        if (view.IsSheetView())
            {
            auto stmt = GetDgnDb().GetPreparedECSqlStatement("SELECT ECInstanceId FROM " BIS_SCHEMA(BIS_CLASS_ViewAttachment) " WHERE Model.Id=?");
            stmt->BindId(1, view2d->GetBaseModelId());
            while (BE_SQLITE_ROW == stmt->Step())
                {
                auto attachId = stmt->GetValueId<DgnElementId>(0);
                auto attach = GetDgnDb().Elements().Get<Sheet::ViewAttachment>(attachId);
                if (attach.IsValid())
                    {
                    auto attachedView = m_dgndb->Elements().Get<ViewDefinition>(attach->GetAttachedViewId());
                    if (attachedView.IsValid() && !attachedView->IsPrivate() && ThumbnailUpdateRequired(*attachedView))
                        return true;
                    }
                }
            }
        return false;
        }
    auto    spatialView  = view.ToSpatialView();
    if (nullptr != spatialView)
        {
        auto modelSelector = m_dgndb->Elements().Get<ModelSelector>(spatialView->GetModelSelectorId());

        if (modelSelector.IsValid())
            for (auto& model : modelSelector->GetModels())
                if (m_unchangedModels.find(model) == m_unchangedModels.end())
                    return true;

        return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/18
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::GenerateThumbnailsWithExceptionHandling()
    {
//  __try    
        {
        GenerateThumbnails();
        }
//    __except(EXCEPTION_EXECUTE_HANDLER)
//        {
//        ReportFailedThumbnails();
//        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Converter::GenerateThumbnail(ViewDefinition const& view)
    {
    if (IsUpdating() && !ThumbnailUpdateRequired (view))
        return BSISUCCESS;

    // VSTS#54728 - Converter hangs on Windows 7 machines due to a bug in folly, which is used during tile generation.
    // Rather than spend time trying to workaround that, we're just turning off thumbnail generation for Windows 7 and earlier.
#if defined(BENTLEY_WIN32)
    DWORD version = GetVersion();
    DWORD major = (DWORD) (LOBYTE(LOWORD(version)));
    DWORD minor = (DWORD) (HIBYTE(LOWORD(version)));
    if ((major < 6) || ((major == 6) && (minor <= 1)))
        return BSISUCCESS;
#endif

    ThumbnailConfig thumbnailConfig(m_config);

    BeDuration timeout = BeDuration::FromSeconds(m_config.GetOptionValueDouble("ThumbnailTimeout", 30));
    if (_GetParams().GetThumbnailTimeout() != 0)
        timeout = _GetParams().GetThumbnailTimeout();

    SetTaskName(ProgressMessage::TASK_CREATING_THUMBNAIL(), view.GetCode().GetValueUtf8CP());
    Render::RenderMode mode = thumbnailConfig.GetRenderModeOverride();
    Point2d size = {thumbnailConfig.GetResolution(), thumbnailConfig.GetResolution()};

    view.RenderAndSaveThumbnail(size, thumbnailConfig.GetUseRenderModeOverride() ? &mode : nullptr, timeout);
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Converter::GenerateThumbnails()
    {
#define ELEMENT_TILE_GENERATE_THUMBNAILS
#if defined(ELEMENT_TILE_GENERATE_THUMBNAILS)
    ThumbnailConfig thumbnailConfig(m_config);

    if ((DgnViewType::None == thumbnailConfig.GetViewTypes()) || !_GetParams().WantThumbnails() || m_haveCreatedThumbnails)
        return BSISUCCESS;

    bool wantRoot    = DgnViewType::Root     == (thumbnailConfig.GetViewTypes() & DgnViewType::Root);
    bool wantSpatial = DgnViewType::Physical == (thumbnailConfig.GetViewTypes() & DgnViewType::Physical);
    bool wantDrawing = DgnViewType::Drawing  == (thumbnailConfig.GetViewTypes() & DgnViewType::Drawing);

    SetStepName(ProgressMessage::STEP_CREATE_THUMBNAILS());

    // Initalize the graphics subsystem, to produce thumbnails.
    DgnViewLib::GetHost().GetViewManager().Startup();

    if (wantRoot)
        {
        auto mainViewGroup =_GetMainViewGroup();
        if (nullptr != mainViewGroup)
            {
            AddTasks(DgnV8Api::MAX_VIEWS);

            DgnElementId repositoryLinkId = GetRepositoryLinkId(*mainViewGroup->GetDgnFileP());

            for (int iView=0; iView<DgnV8Api::MAX_VIEWS; ++iView)
                {
                ReportProgress();
                DgnViewId viewId;
                SyncInfo::ViewDefinitionExternalSourceAspect aspect(nullptr);
                std::tie(aspect, viewId) = SyncInfo::ViewDefinitionExternalSourceAspect::GetAspectBySourceId(repositoryLinkId, mainViewGroup->GetViewInfo(iView), GetDgnDb());
                if (!viewId.IsValid())
                    continue;
                auto view = ViewDefinition::Get(*m_dgndb, viewId);
                if (!view.IsValid())
                    continue;

                GenerateThumbnail(*view);

                if (WasAborted())
                    break;
                }
            }
        }

    if (wantSpatial || wantDrawing)
        {
        AddTasks((int32_t)ViewDefinition::QueryCount(*m_dgndb));
    
        for (auto const& entry : ViewDefinition::MakeIterator(*m_dgndb))
            {
            ReportProgress();
            auto view = ViewDefinition::Get(*m_dgndb, entry.GetId());
            if (!view.IsValid() || view->IsPrivate())
                continue;

            if ((!wantSpatial && view->IsSpatialView()) || (!wantDrawing && view->IsDrawingView()))
                continue;
    
            GenerateThumbnail(*view);

            if (WasAborted())
                break;
            }
        }

    m_haveCreatedThumbnails = true;
#endif

    return BSISUCCESS;
    }

END_DGNDBSYNC_DGNV8_NAMESPACE
