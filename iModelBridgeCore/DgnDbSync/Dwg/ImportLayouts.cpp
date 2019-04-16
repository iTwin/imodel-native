/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include    "DwgImportInternal.h"

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_DWGDB
USING_NAMESPACE_DWG

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/17
+---------------+---------------+---------------+---------------+---------------+------*/
LayoutFactory::LayoutFactory (DwgImporter& importer, DwgDbObjectId layoutId) : m_importer(importer)
    {
    m_layout.OpenObject (layoutId, DwgDbOpenMode::ForRead);
    m_isValid = m_layout.OpenStatus() == DwgDbStatus::Success;
    BeAssert (m_isValid && "cannot open DwgDgbLayout!");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/17
+---------------+---------------+---------------+---------------+---------------+------*/
double          LayoutFactory::GetUserScale () const
    {
    if (this->IsValid())
        return m_layout->IsStandardScale() ? m_layout->GetStandardScale() : m_layout->GetCustomScale();
    return  1.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   LayoutFactory::CalculateSheetSize (DPoint2dR sheetSize) const
    {
    sheetSize.Init (1.0, 1.0);
    if (!this->IsValid())
        return  BSIERROR;

    // read layout's paper size
    if (DwgDbStatus::Success != m_layout->GetPlotPaperSize(sheetSize))
        {
        // failed reading layout data!
        m_importer.ReportError (IssueCategory::UnexpectedData(), Issue::CantOpenObject(), Utf8PrintfString("layout %ls", m_layout->GetName().c_str()).c_str());
        // default to 8.5 x 11.0
        sheetSize.Init (0.216, 0.279);
        return  BSIERROR;
        }

    // scale millimeter paper size to meters
    sheetSize.Scale (0.001);

    // convert upward potrait & landscape orientations to a rotated paper:
    DwgDbLayout::PaperOrientation   orientation = m_layout->GetPaperOrientation ();
    if (DwgDbLayout::RotateBy90 == orientation || DwgDbLayout::RotateBy270 == orientation)
        {
        DPoint2d    swapSize = sheetSize;
        sheetSize.Init (swapSize.y, swapSize.x);
        }

    return  BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   LayoutFactory::AlignSheetToPaperOrigin (TransformR transform) const
    {
    /*-----------------------------------------------------------------------------------
    Currently, Sheet::Border can only be placed at 0,0 in the sheet model.  To make the
    sheet geometry & view attachments appear relative to the border correctly, we have to
    relocate the geometry & viewports.  This method aligns DWG paper origin to Sheet::Border
    origin by adding a translation in the output transformation.
    
    In the future if/when Sheet::Border can be moved, we shall set the border origin from 
    the paper origin, instead of moving geometry.

    The way to find the DWG paper origin turns out to be simple: it is the lower-left corner
    of the LIMITS of the layout.  Apparently the paper origin is the final product of margins,
    plot origin, scale and all other settings from the Page Setup Manager in ACAD.  It is
    different than printable origin, which varies with those parameters.
    -----------------------------------------------------------------------------------*/
    if (!this->IsValid())
        return  BSIERROR;

    // get the paper range, in layout units:
    DRange2d    limits = m_layout->GetLimits ();
    if (limits.IsEmpty())
        return  BSIERROR;

    // the lower-left cornor is the paper origin, regardless how the layout gets set up:
    DPoint2d    paperOrigin = limits.low;
    if (paperOrigin.MaxAbs() > 1.e-4)
        {
        // apply the translation to the sheet transformation:
        DPoint3d    offset = DPoint3d::From (paperOrigin);
        transform.MultiplyMatrixOnly (offset);

        DPoint3d    translation;
        transform.GetTranslation (translation);

        translation.Subtract (offset);
        transform.SetTranslation (translation);
        }

    return  BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/17
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbObjectId   LayoutFactory::FindOverallViewport (DwgDbBlockTableRecordCR block)
    {
    // an inactive layout does not have viewport, so we have to iterate through layout block and find the first viewport:
    DwgDbObjectId   firstViewportId;
    auto iter = block.GetBlockChildIterator ();
    if (iter.IsValid() || !iter->IsValid())
        {
        for (iter->Start(); !iter->Done(); iter->Step())
            {
            auto id = iter->GetEntityId ();
            if (id.IsValid() && id.GetDwgClass() == DwgDbViewport::SuperDesc())
                {
                firstViewportId = id;
                break;
                }
            }
        }
    return  firstViewportId;
    }
    


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/18
+---------------+---------------+---------------+---------------+---------------+------*/
void    DwgImporter::AlignSheetToPaperOrigin (TransformR trans, DwgDbObjectIdCR layoutId)
    {
    // WIP - when/if Sheet::Border supports location in the future, switching code to set the origin, instead of moving geometry:
    LayoutFactory   factory(*this, layoutId);
    factory.AlignSheetToPaperOrigin (trans);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::_ImportLayout (ResolvedModelMapping& modelMap, DwgDbBlockTableRecordR block, DwgDbLayoutCR layout)
    {
    // set the layout block as the current space being processed
    m_currentspaceId = block.GetObjectId ();

    GeometryOptions&    currOptions = this->_GetCurrentGeometryOptions ();
    DwgDbObjectId       overallViewportId;
    DwgDbObjectIdArray  viewports;
    if (layout.GetViewports(viewports) > 0)
        overallViewportId = viewports.front ();
    else
        overallViewportId = LayoutFactory::FindOverallViewport (block);

    // update current viewportID for geometry drawing:
    currOptions.SetViewportId (overallViewportId);

    double  currentPDSIZE = m_dwgdb->GetPDSIZE ();
    bool    resetPDSIZE = false;

    DwgDbViewportPtr    viewport(overallViewportId, DwgDbOpenMode::ForRead);
    if (DwgDbStatus::Success == viewport.OpenStatus())
        {
        // update current viewport range:
        currOptions.SetViewportRange (DwgHelper::GetRangeFrom(viewport->GetViewCenter(), viewport->GetWidth(), viewport->GetHeight()));

        // change percentage PDSIZE to an absolute size to get a desired PDMODE geometry:
        if (m_dwgdb->GetPDMODE() != 0 && currentPDSIZE <= 0.0)
            {
            m_dwgdb->SetPDSIZE (DwgHelper::GetAbsolutePDSIZE(currentPDSIZE, viewport->GetHeight()));
            resetPDSIZE = true;
            }
        viewport.CloseObject ();
        }

    DgnModelP   sheetModel = modelMap.GetModel ();
    if (nullptr == sheetModel)
        return  BSIERROR;

    this->SetTaskName (ProgressMessage::TASK_IMPORTING_MODEL(), sheetModel->GetName().c_str());
    this->Progress ();

    this->SetStepName (ProgressMessage::STEP_IMPORTING_ENTITIES());

    // import the overall paperspace viewport
    this->_ImportPaperspaceViewport (*sheetModel, modelMap.GetTransform(), layout);

    auto trans = modelMap.GetTransform ();

    ElementImportInputs     inputs (*sheetModel);
    inputs.SetClassId (this->_GetElementType(block));
    inputs.SetTransform (trans);
    inputs.SetSpatialFilter (nullptr);
    inputs.SetModelMapping (modelMap);

    // SortEnts table
    DwgDbSortentsTablePtr   sortentsTable;
    if (DwgDbStatus::Success == block.OpenSortentsTable(sortentsTable, DwgDbOpenMode::ForRead))
        {
        // import entities in sorted order:
        DwgDbObjectIdArray  entities;
        if (DwgDbStatus::Success == sortentsTable->GetFullDrawOrder(entities))
            {
            for (DwgDbObjectIdCR id : entities)
                {
                // import all entities except for the paperspace viewport itself
                if (id != overallViewportId)
                    {
                    inputs.SetEntityId (id);
                    this->OpenAndImportEntity (inputs);
                    }
                }
            }
        }
    else
        {
        // import entities in database order:
        DwgDbBlockChildIteratorPtr  entityIter = block.GetBlockChildIterator ();
        if (!entityIter.IsValid() || !entityIter->IsValid())
            return  BSIERROR;

        for (entityIter->Start(); !entityIter->Done(); entityIter->Step())
            {
            DwgDbObjectId   id = entityIter->GetEntityId ();
            // import all entities except for the paperspace viewport itself
            if (id != overallViewportId)
                {
                inputs.SetEntityId (id);
                this->OpenAndImportEntity (inputs);
                }
            }
        }

    // invalidate current layout information:
    m_currentspaceId.SetNull ();
    currOptions.SetViewportId (DwgDbObjectId());

    // restore PDSIZE
    if (resetPDSIZE)
        m_dwgdb->SetPDSIZE (currentPDSIZE);

    return  BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool    DwgImporter::ShouldSkipAllXrefs (ResolvedModelMapping const& ownerModel, DwgDbObjectIdCR ownerSpaceId)
    {
    // check all xRef's attached to the modelspace or a paperspace and return true if none is changed.
    if (!this->IsUpdating())
        return  false;

    bool isModelspace = ownerSpaceId == m_modelspaceId;

    // walk through all xref's we have cached
    for (auto xref : m_loadedXrefFiles)
        {
        DwgDbDatabaseCP xrefDwg = nullptr;
        if (isModelspace)
            {
            // lookup modelspace xref cache
            for (auto xrefModelId : m_modelspaceXrefs)
                {
                if (xref.HasDgnModel(xrefModelId))
                    {
                    xrefDwg = xref.GetDatabaseP ();
                    break;
                    }
                }
            }
        else
            {
            // lookup paperspace xref cache:
            for (auto layoutXref : m_paperspaceXrefs)
                {
                if (ownerSpaceId == layoutXref.GetPaperSpaceId())
                    {
                    xrefDwg = xref.GetDatabaseP ();
                    break;
                    }
                }
            }
        if (nullptr == xrefDwg)
            continue;

        // if an xref is detected to have been changed, do NOT skip the owner model:
        if (!this->_GetChangeDetector()._ShouldSkipModel(*this, ownerModel, xrefDwg))
            return  false;
        }

    // either no xRef is attached, or no change is detected in any xref file, we can skip the owner space.
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::_ImportLayouts ()
    {
    // import import entities in the layout blocks we have previsouly saved into the DgnModel list:
    DwgDbObjectId   modelspaceId = m_dwgdb->GetModelspaceId ();

    // save current model/layout information
    DwgDbObjectId   savedspaceId = m_currentspaceId;
    DwgDbObjectId   savedviewportId = this->_GetCurrentGeometryOptions().GetViewportId ();
    IDwgChangeDetector& changeDetector = this->_GetChangeDetector ();

    DwgDbObjectId   savedLayoutId;
    auto layoutManager = DwgImportHost::GetHost().GetLayoutManager ();
    if (layoutManager.IsValid())
        savedLayoutId =layoutManager->FindActiveLayout (m_dwgdb.get());

    // begin processing DwgModelMap - do not add layout's xref models into m_dwgModelMap, add them in m_paperspaceXrefs!
    m_isProcessingDwgModelMap = true;

    for (auto& entry : m_dwgModelMap)
        {
        if (!entry.IsValid() || entry.GetModel() == nullptr)
            {
            BeAssert (false && "Invalid DwgModelMap!");
            continue;
            }

        // get the model ID
        DwgDbObjectId   modelId = entry.GetModelInstanceId ();
        // skip all models expcept for paperspaces:
        if (modelspaceId == modelId || entry.GetMapping().GetSourceType() != DwgSyncInfo::ModelSourceType::PaperSpace)
            continue;

        DwgDbBlockTableRecordPtr    block(modelId, DwgDbOpenMode::ForRead);
        if (block.IsNull())
            {
            this->ReportError (IssueCategory::UnexpectedData(), Issue::CantOpenObject(), "bad layout block or unsaved DgnModel for it!");
            return  BSIERROR;
            }

        if (block->IsLayout() && !block->IsExternalReference())
            {
            // activate the layout such that its viewports etc can work correctly (specifically DwgDbLayout::GetViewports() fails otherwise):
            if (layoutManager.IsValid())
                {
                // must close & re-open block as otherwise RealDWG will fail:
                auto layoutId = block->GetLayoutId ();
                block.CloseObject ();
                layoutManager->ActivateLayout (layoutId);
                block.OpenObject (modelId, DwgDbOpenMode::ForRead);
                BeAssert (block.OpenStatus() == DwgDbStatus::Success);
                }

            if (changeDetector._ShouldSkipModel(*this, entry))
                {
                // no entity change is detected in this layout - check all xRef files attached to it:
                if (this->ShouldSkipAllXrefs(entry, modelId))
                    {
                    // before we skip this layout, record its viewport as seen:
                    DwgDbObjectIdArray  ids;
                    DwgDbLayoutPtr  layout (block->GetLayoutId(), DwgDbOpenMode::ForRead);
                    if (layout.OpenStatus() == DwgDbStatus::Success && layout->GetViewports(ids) > 0)
                        changeDetector._OnViewSeen (*this, this->GetSyncInfo().FindView(ids.front(), DwgSyncInfo::View::Type::PaperspaceViewport));
                    continue;
                    }
                }

            // don't import an empty layout:
            auto iter = block->GetBlockChildIterator ();
            if (!iter.IsValid() || !iter->IsValid())
                continue;
            iter->Start ();
            if (iter->Done())
                continue;

            // even if the overall viewport exists, and it's the only entity in the block, the layout is treated as empty:
            iter->Step ();
            if (iter->Done() && !iter->GetEntityId().IsValid())
                continue;

            DwgDbLayoutPtr  layout (block->GetLayoutId(), DwgDbOpenMode::ForRead);
            if (layout.OpenStatus() == DwgDbStatus::Success)
                this->_ImportLayout (entry, *block.get(), *layout.get());
            else
                this->ReportError (IssueCategory::UnexpectedData(), Issue::CantOpenObject(), Utf8PrintfString("unable to open & import layout %ls!", block->GetName().c_str()).c_str());
            }
        }

    // restore current model/layout information
    m_currentspaceId = savedspaceId;
    this->_GetCurrentGeometryOptions().SetViewportId (savedviewportId);

    if (layoutManager.IsValid() && savedLayoutId.IsValid())
        layoutManager->ActivateLayout (savedLayoutId);

    m_isProcessingDwgModelMap = false;

    return BSISUCCESS;
    }

