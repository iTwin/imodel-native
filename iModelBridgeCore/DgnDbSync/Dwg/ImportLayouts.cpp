/*--------------------------------------------------------------------------------------+
|
|     $Source: Dwg/ImportLayouts.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    "DwgImportInternal.h"

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_DWGDB
USING_NAMESPACE_DGNDBSYNC_DWG

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
        m_importer.ReportError (DwgImporter::IssueCategory::UnexpectedData(), DwgImporter::Issue::CantOpenObject(), Utf8PrintfString("layout %ls", m_layout->GetName().c_str()).c_str());
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
        overallViewportId.SetNull ();

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

    ElementImportInputs     inputs (*sheetModel);
    inputs.SetClassId (this->_GetElementType(block));
    inputs.SetTransform (modelMap.GetTransform());
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
        DwgDbBlockChildIterator     entityIter = block.GetBlockChildIterator ();
        if (!entityIter.IsValid())
            return  BSIERROR;

        for (entityIter.Start(); !entityIter.Done(); entityIter.Step())
            {
            DwgDbObjectId   id = entityIter.GetEntityId ();
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

    // free memory
    m_dgndb->Memory().PurgeUntil(1024 * 1024);

    return  BSISUCCESS;
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
        if (modelspaceId == modelId || entry.GetMapping().GetSourceType() != DwgSyncInfo::ModelSourceType::ModelOrPaperSpace)
            continue;

        DwgDbBlockTableRecordPtr    block(modelId, DwgDbOpenMode::ForRead);
        if (block.IsNull())
            {
            this->ReportError (IssueCategory::UnexpectedData(), Issue::CantOpenObject(), "bad layout block or unsaved DgnModel for it!");
            return  BSIERROR;
            }

        if (block->IsLayout() && !block->IsExternalReference())
            {
            if (this->_ShouldSkipModel(entry))
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

    m_isProcessingDwgModelMap = false;

    return BSISUCCESS;
    }

