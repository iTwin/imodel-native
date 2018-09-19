/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnV8/DrawingConverter.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ConverterInternal.h"
#include <VersionedDgnV8Api/PSolid/PSolidCore.h>
#include <VersionedDgnV8Api/VisEdgesLib/VisEdgesLib.h>
#include <VersionedDgnV8Api/DgnPlatform/CVEHandler.h>
#include <DgnPlatform/AutoRestore.h>


BEGIN_DGNDBSYNC_DGNV8_NAMESPACE

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct DrawingViewFactory : ViewFactory
    {
    ResolvedModelMapping const& m_drawingModelMapping;
    DrawingViewFactory(ResolvedModelMapping const& v8mm) : m_drawingModelMapping(v8mm) {}

    ViewDefinitionPtr _MakeView(Converter& converter, ViewDefinitionParams const&) override;
    ViewDefinitionPtr _UpdateView(Converter& converter, ViewDefinitionParams const&, DgnViewId viewId) override;
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ScaledCopyMarker : DgnV8Api::DgnModelAppData
    {
    static Key s_key;
    void _OnCleanup (DgnV8ModelR host) override {delete this;}
    static bool IsFoundOn(DgnV8ModelCR model) {return nullptr != model.FindAppData(s_key);}
    static void AddTo(DgnV8ModelR model) {model.AddAppData(s_key, new ScaledCopyMarker);}
    };

ScaledCopyMarker::Key ScaledCopyMarker::s_key;

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct SheetAttachmentMarker : DgnV8Api::DgnModelAppData
    {
    static Key s_key;
    void _OnCleanup (DgnV8ModelR host) override {delete this;}
    static bool IsFoundOn(DgnV8ModelCR model) {return nullptr != model.FindAppData(s_key);}
    static void AddTo(DgnV8ModelR model) {model.AddAppData(s_key, new SheetAttachmentMarker);}
    };

SheetAttachmentMarker::Key SheetAttachmentMarker::s_key;

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ConverterMadeCopyMarker : DgnV8Api::DgnModelAppData
    {
    static Key s_key;
    void _OnCleanup (DgnV8ModelR host) override {delete this;}
    static bool IsFoundOn(DgnV8ModelCR model) {return nullptr != model.FindAppData(s_key);}
    static void AddTo(DgnV8ModelR model) {model.AddAppData(s_key, new ConverterMadeCopyMarker);}
    };

ConverterMadeCopyMarker::Key ConverterMadeCopyMarker::s_key;

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ModelsToBeMerged : DgnV8Api::DgnModelAppData
    {
    static Key s_key;
    bvector<ResolvedModelMapping> m_models;

    ModelsToBeMerged(bvector<ResolvedModelMapping> const& v) : m_models(v) {}
    void _OnCleanup (DgnV8ModelR host) override {delete this;}
    static void AddTo(DgnV8ModelR model, bvector<ResolvedModelMapping> const& tbm) {model.AddAppData(s_key, new ModelsToBeMerged(tbm));}
    static ModelsToBeMerged* GetFrom(DgnV8ModelCR model) {return dynamic_cast<ModelsToBeMerged*>(model.FindAppData(s_key));}
    };

ModelsToBeMerged::Key ModelsToBeMerged::s_key;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/16
+---------------+---------------+---------------+---------------+---------------+------*/
void RootModelConverter::_ImportDrawingAndSheetModels(ResolvedModelMapping& rootModelMapping)
    {
#ifndef NDEBUG
    bset<DgnV8ModelP> seen;
#endif

    // Pass 1: sheets and attached drawings. IMPORTANT! See "DgnModel objects and Sheet attachments" for why this MUST BE DONE FIRST.
    for (auto v8Model : m_nonSpatialModelsInModelIndexOrder)
        {
        BeAssert(seen.insert(v8Model).second && " no dups expected in m_nonSpatialModelsInModelIndexOrder");

        if (v8Model->IsSheet())
            ImportSheetModel(*v8Model, m_isRootModelSpatial);
        }

    // Pass 2: drawings that are not referenced by sheets
    for (auto v8Model : m_nonSpatialModelsInModelIndexOrder)
        {
        if (!v8Model->IsSheet() && !SheetAttachmentMarker::IsFoundOn(*v8Model))
            ImportDrawingModel(rootModelMapping, *v8Model);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/16
+---------------+---------------+---------------+---------------+---------------+------*/
void RootModelConverter::_OnDrawingModelFound(DgnV8ModelR v8model)
    {
    // See "Keeping sheet and drawing models alive" comment below.
    m_drawingModelsKeepAlive.push_back(&v8model);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/16
+---------------+---------------+---------------+---------------+---------------+------*/
void RootModelConverter::_KeepFileAlive(DgnV8FileR v8file)
    {
    // See "Keeping sheet and drawing models alive" comment below.
    if (std::find(m_filesKeepAlive.begin(), m_filesKeepAlive.end(), &v8file) == m_filesKeepAlive.end())
        m_filesKeepAlive.push_back(&v8file);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/16
+---------------+---------------+---------------+---------------+---------------+------*/
void RootModelConverter::_ConvertDrawings()
    {
    SetStepName(Converter::ProgressMessage::STEP_CONVERTING_DRAWINGS());

    bmultiset<ResolvedModelMapping> drawings;
    for (auto v8mm : m_v8ModelMappings)
        {
        if (!IsFileAssignedToBridge(*v8mm.GetV8Model().GetDgnFileP()))
            continue;
        if (v8mm.GetDgnModel().IsDrawingModel())
            drawings.insert(v8mm);
        }

    AddTasks((uint32_t)drawings.size());

    // Just in case the session hasn't started
    if (!DgnV8Api::PSolidKernelManager::IsSessionStarted())
        DgnV8Api::PSolidKernelManager::StartSession();

    for (auto v8mm : drawings)
        {
        SetTaskName(Converter::ProgressMessage::TASK_CONVERTING_MODEL(), v8mm.GetDgnModel().GetName().c_str());
        DrawingsConvertModelAndViews(v8mm);
        // TFS#661407: Reset parasolid session to avoid running out of tags on long processing of VisEdgesLib
        DgnV8Api::PSolidKernelManager::StopSession();
        DgnV8Api::PSolidKernelManager::StartSession();
        }

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/16
+---------------+---------------+---------------+---------------+---------------+------*/
bpair<ResolvedModelMapping, bool> Converter::Import2dModel(DgnV8ModelR v8model)
    {
    BeAssert(GetV8FileSyncInfoIdFromAppData(*v8model.GetDgnFileP()).IsValid() && "All V8 files should have been found discovered by _InitRootModel");

    if (LOG_IS_SEVERITY_ENABLED(LOG_TRACE))
        LOG.tracev("Import2dModel %s", IssueReporter::FmtModel(v8model).c_str());

    // Note: We do not differentiate 2d attachments by transform. We handle attachments with positioning transforms specially in drawings and sheets elsewhere.
    auto toMeters = ComputeUnitsScaleTransform(v8model);

    auto foundmm = FindModelForDgnV8Model(v8model, toMeters);
    if (foundmm.IsValid())
        {
        if (LOG_IS_SEVERITY_ENABLED(LOG_TRACE))
            LOG.tracev(" %s was previously converted to %s", IssueReporter::FmtModel(v8model).c_str(), IssueReporter::FmtModel(foundmm.GetDgnModel()).c_str());

        return make_bpair(foundmm, false); // v8model has already been imported
        }
	
    WriteRepositoryLink(*v8model.GetDgnFileP());    // The old converter created a repository link for each sheet file, so we will do it, too. This is harmless if there's already a link for this file.

    _OnDrawingModelFound(v8model);	// We may have reached a new drawing model (e.g., in the case where we *generated* the model to adjust for annotation scale.)

    auto bimModel = _GetModelForDgnV8Model(v8model, toMeters); // adds to m_v8ModelMappings
    if (!bimModel.IsValid())
        return make_bpair(bimModel, false);
    
    BeAssert(bimModel.GetDgnModel().Is2dModel());

    return make_bpair(bimModel, true); 

    // NB: Do not recurse! The caller handles that!
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void RootModelConverter::RegisterNonSpatialModel(DgnV8ModelR thisV8Model)
    {
    DgnV8FileR thisV8File = *thisV8Model.GetDgnFileP();

    if (!IsFileAssignedToBridge(thisV8File))
        return;

    if (ShouldConvertToPhysicalModel(thisV8Model))          // Not going to convert this to a sheet or drawing?
        return;

    if (!m_nonSpatialModelsSeen.insert(&thisV8Model).second)   // Already seen this model?
        return;

    // Build up a list of non-spatial models in the order in which they were found. This is 
    // so that we will (later) import those models in that same order. That is necessary so that
    // the converter will produce the same results -- the same rows in the same order -- as the older converter did.
    // And that is necessary so that we can verify the new converter by matching its results with the results of the old converter.
    m_nonSpatialModelsInModelIndexOrder.push_back(&thisV8Model);
    
    _OnDrawingModelFound(thisV8Model);  // keep this model alive

    if (!GetV8FileSyncInfoIdFromAppData(thisV8File).IsValid())  // Register this FILE in syncinfo.
        {
        GetV8FileSyncInfoId(thisV8File); // populates m_v8files
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/16
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnV8FileP findOpenV8FileByName(bvector<DgnV8FileP> const& files, BeFileNameCR fn)
    {
    for (auto v8File : files)
        {
        if (fn.EqualsI(v8File->GetFileName().c_str()))
            return v8File;
        }
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/17
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename CB>
static void processModelIndex(bvector<DgnV8FileP>& filesToSearch, bvector<DgnV8FileP> const& allV8Files, CB cb)
    {
    bvector<DgnV8FileP> filesToSearchThisTime(filesToSearch);
    while (!filesToSearchThisTime.empty())
        {
        for (auto v8File : filesToSearchThisTime)
            {
            // The callback may create copies of model which invalidates the iterator - so iterate through a temporary list of indices.
            bvector<DgnV8Api::ModelIndexItem const*> indexItems;
            bset <DgnV8Api::ModelIndexItem const*> originals;
    
            for (DgnV8Api::ModelIndexItem const& item : v8File->GetModelIndex())
                {
                indexItems.push_back(&item);
                originals.insert(&item);
                }

            // First process the items that were originally in the index.
            for (auto& pItem : indexItems)      
                cb(*v8File, *pItem);

            /// Then process any items that were added in the original pass.
            for (DgnV8Api::ModelIndexItem const& item : v8File->GetModelIndex())
                if (originals.find(&item) == originals.end())
                    cb(*v8File, item);

            }

        filesToSearchThisTime.clear();
        for (auto f : allV8Files)        // See if RegisterNonSpatialModel discovered new files while following 2-D attachments.
            {
            if (std::find(filesToSearch.begin(), filesToSearch.end(), f) == filesToSearch.end())
                filesToSearchThisTime.push_back(f);
            }

        filesToSearch.insert(filesToSearch.end(), filesToSearchThisTime.begin(), filesToSearchThisTime.end());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void RootModelConverter::FindV8DrawingsAndSheets()
    {
    //  Build a list of dgn files to search for sheets and drawings 
    bvector<DgnFilePtr> tempKeepAlive;
    bvector<DgnV8FileP> filesToSearch;

    for (auto v8 : m_v8Files) // start with the files that we already found when we searched for spatial models
        {
        if (IsFileAssignedToBridge(*v8))
            filesToSearch.push_back(v8);
        }

    for (auto const& fn : m_params.GetDrawingAndSheetFiles()) // include the list of other files to search for drawings and sheets
        {
        if (nullptr != findOpenV8FileByName(filesToSearch, fn))
            continue;

        DgnV8Api::DgnFileStatus openStatus;
        Bentley::DgnFilePtr v8File = OpenDgnV8File(openStatus, fn); // Just open it. Don't register it in syncinfo. We'll do that in RegisterNonSpatialModel if we actually find a drawing or sheet in there.
        if (v8File.IsValid() && IsFileAssignedToBridge(*v8File))
            {
            tempKeepAlive.push_back(v8File);
            filesToSearch.push_back(v8File.get());
            }
        }

    for (auto fileToSearch : filesToSearch)
        ClassifyNormal2dModels (*fileToSearch); // This tells us whether a given 2d design model should become a drawing or a spatial model

    // *** EXTREMELY TRICKY: See "DgnModel objects and Sheet attachments" for why we need TWO PASSES
    processModelIndex(filesToSearch, m_v8Files, [&](DgnV8FileR v8File, DgnV8Api::ModelIndexItem const& item) {RegisterSheetModel(v8File, item);});
    processModelIndex(filesToSearch, m_v8Files, [&](DgnV8FileR v8File, DgnV8Api::ModelIndexItem const& item) {RegisterDrawingModel(v8File, item);});
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/17
+---------------+---------------+---------------+---------------+---------------+------*/
void RootModelConverter::RegisterDrawingModel(DgnV8FileR v8File, DgnV8Api::ModelIndexItem const& item)
    {
    if (!IsV8DrawingModel(v8File, item))
        return;
    
    auto v8model = v8File.LoadModelById(item.GetModelId());
    if (!v8model.IsValid())
        return;

    // Already found as a sheet attachment? Move on. See "DgnModel objects and Sheet attachments"
    if (SheetAttachmentMarker::IsFoundOn(*v8model))
        return;

    Transform2dAttachments(*v8model);

    RegisterNonSpatialModel(*v8model);

    if (GetAttachments(*v8model))
        {
        for (auto attachment : *GetAttachments(*v8model))
            {
            auto attachedModel = attachment->GetDgnModelP();
            if (nullptr == attachedModel)
                continue;

            RegisterNonSpatialModel(*attachedModel);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::ImportDrawingModel(ResolvedModelMapping& rootModelMapping, DgnV8ModelR v8model)
    {
    BeAssert(IsV8DrawingModel(v8model));

    Import2dModel(v8model);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/17
+---------------+---------------+---------------+---------------+---------------+------*/
static void fixV8ModelName(WStringR mname)
    {
    for (size_t i=0, n=mname.size(); i<n; ++i)
        {
        if (!DgnV8Api::ModelInfo::IsNameCharValid(mname[i]))
            mname[i] = '_';
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/17
+---------------+---------------+---------------+---------------+---------------+------*/
static WString computeFullV8ModelName(DgnV8ModelR v8Model, wchar_t const* suffix)
    {
    auto v8file = v8Model.GetDgnFileP();
    WString newModelName(L".");
    newModelName.append(v8file->GetFileName().c_str());
    newModelName.append(L"/");
    newModelName.append(v8Model.GetModelName());
    newModelName.append(suffix);
    fixV8ModelName(newModelName);
    return newModelName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/16
+---------------+---------------+---------------+---------------+---------------+------*/
Bentley::RefCountedPtr<DgnV8Api::DgnModel> Converter::CopyAndChangeAnnotationScale(DgnV8ModelP v8Model, double newAnnotationScale)
    {
    DgnV8Api::DependencyManager::SetProcessingDisabled(false);   // must allow dependency mgr to "process" as that is how V8 remaps IDs. See below.

    auto v8file = v8Model->GetDgnFileP();

    // Generate a new name for the model based on the attachment and the new scale
    WString newModelName = computeFullV8ModelName(*v8Model, WPrintfString(L" (annotation scale=%lf) DgnV8Converter", newAnnotationScale).c_str());

    //  See if we already made such a copy. This can happen if the same drawing is attached to many different sheets 
    //  or to the same sheet, each time with a different clip
    if (true)
        {
        DgnV8ModelP existingModel = v8file->FindLoadedModelById(v8file->FindModelIdByName(newModelName.c_str()));
        if (nullptr != existingModel)
            return existingModel;
        }

    // *** TRICKY: V8's ITxn::CreateNewModel function always sets up the new model as a rootmodel. V8 requires
    //              that there be a non-zero refcount on a DgnFile that owns rootmodels. So, we must make sure that
    //              there is such a ref.
    _KeepFileAlive(*v8file);

    //  Make a copy of the model
    auto newModelInfo = v8Model->GetModelInfo().MakeCopy();
    newModelInfo->SetName(newModelName.c_str());
    newModelInfo->SetAnnotationScaleFactor(newAnnotationScale);
    newModelInfo->SetIsHidden(true);
    if (newModelInfo->GetModelType() == DgnV8Api::DgnModelType::Sheet)
        newModelInfo->SetModelType(DgnV8Api::DgnModelType::Drawing);

    DgnV8ModelP newModel = DgnV8Api::ITxnManager::GetCurrentTxn().CreateNewModel(nullptr, *v8file, *newModelInfo);
    if (nullptr == newModel)
        return nullptr;

    v8file->CopyModelContents(*newModel, *v8Model, NULL);

    CopyEffectiveModelType(*newModel, *v8Model);

    //  Change the annotation scale of the new model
    DgnV8Api::ChangeAnnotationScale changeContext (newModel);
    changeContext.SetTraverseElementContext (DgnV8Api::CASTraverseElementContext::All, 0, NULL);
    changeContext.SetAction(DgnV8Api::AnnotationScaleAction::Update, newAnnotationScale);
    changeContext.DoChange();

    ScaledCopyMarker::AddTo(*newModel);
    ConverterMadeCopyMarker::AddTo(*newModel);

    DgnV8Api::DependencyManager::ProcessAffected(); // remap element-element pointers and clear remap tables. If we don't do this, then second call to CopyModelContents on the same model will do nothing.

    return newModel;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/16
+---------------+---------------+---------------+---------------+---------------+------*/
Bentley::RefCountedPtr<DgnV8Api::DgnModel> Converter::CopyModel(DgnV8ModelP v8Model, WCharCP newNameSuffix)
    {
    DgnV8Api::DependencyManager::SetProcessingDisabled(false);   // must allow dependency mgr to "process" as that is how V8 remaps IDs. See below.

    auto v8file = v8Model->GetDgnFileP();

    // Generate a new name for the copied model based on the attachment
    WString newModelName = computeFullV8ModelName(*v8Model, newNameSuffix);

    //  See if we already made such a copy. This can happen if the same drawing is attached to many different sheets 
    //  or to the same sheet, each time with a different clip
    if (true)
        {
        DgnV8ModelP existingModel = v8file->FindLoadedModelById(v8file->FindModelIdByName(newModelName.c_str()));
        if (nullptr != existingModel)
            return existingModel;
        }

    // *** TRICKY: V8's ITxn::CreateNewModel function always sets up the new model as a rootmodel. V8 requires
    //              that there be a non-zero refcount on a DgnFile that owns rootmodels. So, we must make sure that
    //              there is such a ref.
    _KeepFileAlive(*v8file);

    //  Make a copy of the model
    auto newModelInfo = v8Model->GetModelInfo().MakeCopy();
    newModelInfo->SetName(newModelName.c_str());
    newModelInfo->SetIsHidden(true);

    DgnV8ModelP newModel = DgnV8Api::ITxnManager::GetCurrentTxn().CreateNewModel(nullptr, *v8file, *newModelInfo);
    if (nullptr == newModel)
        return nullptr;

    v8file->CopyModelContents(*newModel, *v8Model, NULL);

    CopyEffectiveModelType(*newModel, *v8Model);

    DgnV8Api::DependencyManager::ProcessAffected(); // remap element-element pointers and clear remap tables. If we don't do this, then second call to CopyModelContents on the same model will do nothing.

    DgnV8Api::DependencyManager::SetProcessingDisabled(true);

    ConverterMadeCopyMarker::AddTo(*newModel);

    return newModel;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/16
+---------------+---------------+---------------+---------------+---------------+------*/
Bentley::RefCountedPtr<DgnV8Api::DgnModel> Converter::CopyAndChangeSheetToDrawing(DgnV8ModelP v8Model)
    {
    DgnV8Api::DependencyManager::SetProcessingDisabled(false);   // must allow dependency mgr to "process" as that is how V8 remaps IDs. See below.

    auto v8file = v8Model->GetDgnFileP();

    BeAssert(v8Model->GetModelType() == DgnV8Api::DgnModelType::Sheet);

    // Generate a new name for the drawing model based on the attachment
    WString newModelName = computeFullV8ModelName(*v8Model, L" (drawing) DgnV8Converter");

    //  See if we already made such a copy. This can happen if the same drawing is attached to many different sheets 
    //  or to the same sheet, each time with a different clip
    if (true)
        {
        DgnV8ModelP existingModel = v8file->FindLoadedModelById(v8file->FindModelIdByName(newModelName.c_str()));
        if (nullptr != existingModel)
            return existingModel;
        }

    // *** TRICKY: V8's ITxn::CreateNewModel function always sets up the new model as a rootmodel. V8 requires
    //              that there be a non-zero refcount on a DgnFile that owns rootmodels. So, we must make sure that
    //              there is such a ref.
    _KeepFileAlive(*v8file);

    //  Make a copy of the model
    auto newModelInfo = v8Model->GetModelInfo().MakeCopy();
    newModelInfo->SetName(newModelName.c_str());
    newModelInfo->SetModelType(DgnV8Api::DgnModelType::Drawing);
    newModelInfo->SetIsHidden(true);

    DgnV8ModelP newModel = DgnV8Api::ITxnManager::GetCurrentTxn().CreateNewModel(nullptr, *v8file, *newModelInfo);
    if (nullptr == newModel)
        return nullptr;

    v8file->CopyModelContents(*newModel, *v8Model, NULL);

    SetEffectiveModelType(*newModel, newModel->GetModelType());

    DgnV8Api::DependencyManager::ProcessAffected(); // remap element-element pointers and clear remap tables. If we don't do this, then second call to CopyModelContents on the same model will do nothing.

    DgnV8Api::DependencyManager::SetProcessingDisabled(true);

    ConverterMadeCopyMarker::AddTo(*newModel);

    return newModel;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 05/07
+---------------+---------------+---------------+---------------+---------------+------*/
static double   getModelAnnotationScale (DgnV8ModelRefCP modelRef)
    {
    ModelInfoCP minfo = modelRef->GetModelInfoCP ();

    return (minfo ? minfo->GetAnnotationScaleFactor () : 1.0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/2008
+---------------+---------------+---------------+---------------+---------------+------*/
void            Converter::MakeAttachedModelMatchRootAnnotationScale(DgnAttachmentR ref)
    {
    // See "Annotation Scale and V8 drawings and Sheets" comment below

    DgnModelRefP attachedModel = ref.GetDgnModelP();
    if (nullptr == attachedModel)
        return;

    // If the target of the reference does not propagate annScale, treat as if
    // the 'useAnnotationScale' flag is off.
    if (nullptr == attachedModel ||
        attachedModel->GetModelFlag (DgnV8Api::MODELFLAG_NO_PROPAGATE_ANNSCALE))
        return;

    DgnAttachmentCR rootRef = ref.GetBaseDgnAttachment();

    // Differentiate between using annotation scale of 1.0 and no scale
    if (!rootRef.UseAnnotationScale())
        return;

    DgnModelRefP    rootModelRef = rootRef.GetRoot();

    double rootAnnotationScale = getModelAnnotationScale (rootModelRef);        // the AS of the sheet.
    double refAnnotationScale  = getModelAnnotationScale (&ref);                // the AS of the drawing that is attached to the sheet
    double refDisplayScale     = DgnV8Api::AnnotationScale::GetUserScaleFromRefToRoot (ref); // the geometric transform that will be applied to the attachment. This is a small number, e.g., 0.02 for 1:50 scaling.

    //  The sheet requires the attachment's annotations to be at rootAnnotationScale.
    //  The sheet view attachment will *also* apply a scale factor equal to refDisplayScale when displaying the attached model.
    //  So, the effective attachment annotation scale is the attachment annotation scale times refDisplayScale.
    //  Modify the attachment annotation scale to make the effective scale what we want to see.
    double requiredEffectiveAnnotationScale = rootAnnotationScale / refDisplayScale;

    if (fabs(requiredEffectiveAnnotationScale - refAnnotationScale) > 10.E-8  * std::max(requiredEffectiveAnnotationScale, refAnnotationScale))
        {
        auto scaledChild = CopyAndChangeAnnotationScale(ref.GetDgnModelP(), requiredEffectiveAnnotationScale);
        if (scaledChild.IsValid())
            {
            ref.SetDgnModel(scaledChild.get()); // v8DgnAttachment will add a reference to the new model, keeping it alive.
            attachedModel = ref.GetDgnModelP();
            GetAttachments(ref); // SetDgnModel deleted the nested attachment array. Rebuild it.
            }
        }

    // Recurse over nested attachments, to make sure they all match the sheet's annotation scale.
    MakeAttachmentsMatchRootAnnotationScale(ref);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/2008
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::MakeAttachmentsMatchRootAnnotationScale(DgnModelRefR parent)
    {
    auto attachments = parent.GetDgnAttachmentsP(); // NB: Don't try to load attachments. "parent" itself might be an attachment, and we must work with the attachment hierarchy that we are given.
    if (nullptr == attachments)
        return;

    for (auto attachment : *attachments)
        {
        if (!attachment->Is3d())
            MakeAttachedModelMatchRootAnnotationScale(*attachment);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::TransformDrawingAttachmentsToCopies(DgnV8ModelR parentModel, T_AttachmentCopyFilter filter)
    {
    auto attachments = GetAttachments(parentModel);
    if (nullptr == attachments)
        return;

    for (auto attachment : *attachments)
        {
        auto v8Model = attachment->GetDgnModelP();
        if (nullptr == v8Model)
            continue;

        // We only make private copies of attached drawings, not spatial/3d models!
        if (ShouldConvertToPhysicalModel(*v8Model))
            continue;
            
        // If this is a model that we already copied in a previous step (or for another sheet), there is no need to copy it again.
        if (ConverterMadeCopyMarker::IsFoundOn(*v8Model))
            continue;

        // See if the caller really wants me to make a copy of this particular attachment
        if (!filter(*attachment))
            continue;

        // Give the parent a hidden copy of the attached model.
        auto copyOfAttached = CopyModel(attachment->GetDgnModelP(), L" (attachment) DgnV8Converter");
        if (!copyOfAttached.IsValid())
            continue;

        parentModel.SetReadOnly(false);

        attachment->SetDgnModel(copyOfAttached.get()); // attachment will add a reference to the new model, keeping it alive.
        GetAttachments(*attachment); // SetDgnModel deleted the nested attachment array. Rebuild it.
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/17
+---------------+---------------+---------------+---------------+---------------+------*/
void RootModelConverter::Transform2dAttachments(DgnV8ModelR v8ParentModel)
    {
    GetAttachments(v8ParentModel); // make sure attachment hierarchy is created
    
    if (!ScaledCopyMarker::IsFoundOn(v8ParentModel))    // *** NEEDS WORK: not sure why we need this check
        MakeAttachmentsMatchRootAnnotationScale(v8ParentModel);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/17
+---------------+---------------+---------------+---------------+---------------+------*/
void RootModelConverter::RegisterSheetModel(DgnV8FileR v8File, DgnV8Api::ModelIndexItem const& item)
    {
    if (DgnV8Api::DgnModelType::Sheet != item.GetModelType())
        return;
    
    auto v8model = v8File.LoadModelById(item.GetModelId());
    if (!v8model.IsValid())
        return;

    Transform2dAttachments(*v8model);

    TransformDrawingAttachmentsToCopies(*v8model, [this] (DgnV8Api::DgnAttachment const& attachment)    // See "Why do sheets need to make copies of their attachments?"
        {
        if (nullptr == attachment.GetDgnModelP())
            return false;

        // If this attachment is the root model OR the drawing has rendered view attachments (which will be merged into sheet) then make a copy.
        return (attachment.GetDgnModelP() == this->GetRootModelP() || HasRenderedViewAttachments(*attachment.GetDgnModelP()));
        });

    RegisterNonSpatialModel(*v8model);

    if (GetAttachments(*v8model))
        {
        for (auto attachment : *GetAttachments(*v8model))
            {
            auto attachedModel = attachment->GetDgnModelP();
            if (nullptr == attachedModel)
                continue;

            RegisterNonSpatialModel(*attachedModel);
    
            // Mark the sheet's attachments. This will tell downstream conversion logic: "hands off!"
            SheetAttachmentMarker::AddTo(*attachedModel);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::ImportSheetModel(DgnV8ModelR v8model, bool isRootModelSpatial)
    {
    BeAssert(v8model.IsSheet());

    Import2dModel(v8model);

    //  Import direct attachments. See "DgnModel objects and Sheet attachments" comment below.
    auto attachments = GetAttachments(v8model);
    if (nullptr == attachments)
        return;

    for (auto attachment : *attachments)
        {
        if (nullptr == attachment->GetDgnModelP())
            continue;

        if (!attachment->Is3d())
            {
            Import2dModel(*attachment->GetDgnModelP());
            }
        else
            {
            if (!isRootModelSpatial)
                {
                OnFatalError(IssueCategory::Compatibility(), Issue::Detected3dViaAttachment(), Converter::IssueReporter::FmtAttachment(*attachment).c_str());
                BeAssert(false);
                return;
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ViewDefinitionPtr DrawingViewFactory::_MakeView(Converter& converter, ViewDefinitionParams const& parms)
    {
    if (!parms.GetDgnModel().Is2dModel() || parms.GetDgnModel().IsSheetModel())
        return nullptr;

    converter._TurnOnExtractionCategories(*parms.m_categories);

    DrawingViewDefinitionPtr view = new DrawingViewDefinition(*converter.GetJobDefinitionModel(), parms.m_name, parms.GetDgnModel().GetModelId(), *parms.m_categories, *parms.m_dstyle->ToDisplayStyle2dP());

    parms.Apply(*view);

    converter.ConvertLevelMask(*view, parms.m_viewInfo, &parms.GetV8Model());

    return view;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            07/2018
//---------------+---------------+---------------+---------------+---------------+-------
ViewDefinitionPtr DrawingViewFactory::_UpdateView(Converter& converter, ViewDefinitionParams const& params, DgnViewId viewId)
    {
    ViewDefinitionPtr newDef = _MakeView(converter, params);
    if (!newDef.IsValid())
        return newDef;

    DrawingViewDefinition* drawing = newDef->ToDrawingViewP();

    DgnDbStatus stat;
    // need to update the DisplayStyle and CategorySelector.
    DrawingViewDefinitionPtr existingDef = converter.GetDgnDb().Elements().GetForEdit<DrawingViewDefinition>(viewId);

    DisplayStyleR newDisplayStyle = drawing->GetDisplayStyle();
    newDisplayStyle.ForceElementIdForInsert(existingDef->GetDisplayStyleId());
    newDisplayStyle.Update(&stat);
    if (DgnDbStatus::Success != stat)
        return nullptr;
    drawing->SetDisplayStyle(newDisplayStyle);

    CategorySelectorR newCategorySelector = drawing->GetCategorySelector();
    newCategorySelector.ForceElementIdForInsert(existingDef->GetCategorySelectorId());
    newCategorySelector.Update(&stat);
    if (DgnDbStatus::Success != stat)
        return nullptr;
    drawing->SetCategorySelector(newCategorySelector);

    newDef->ForceElementIdForInsert(viewId);
    return newDef;
    }

static BentleyApi::TransformCR DoInterop(Bentley::Transform const&source) { return (BentleyApi::TransformCR)source; }
static BentleyApi::DMatrix4dCR DoInterop(Bentley::DMatrix4d const&source) { return (BentleyApi::DMatrix4dCR)source; }
/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     05/2018
+===============+===============+===============+===============+===============+======*/
struct MergeProxyGraphicsDrawGeom : public DgnV8Api::SimplifyViewDrawGeom
{
    typedef
        bmap<Bentley::DgnModelRefP,                            
            bmap<DgnV8Api::ElementId,                          
                bmap<DgnCategoryId, GeometryBuilderPtr>>>    T_BuilderMap;


    struct ModelRefInfo
        {
        DgnCategoryId                           m_categoryId;
        bool                                    m_attachmentChanged = false;
        bool                                    m_modelContentsChanged = false;
        bool                                    m_useMap = false;
        DMatrix4d                               m_map;
        SyncInfo::V8ElementMapping              m_attachElementMapping;
        ResolvedModelMapping                    m_modelMapping;
        };

    typedef bmap<DgnModelRefP, ModelRefInfo>    T_ModelRefInfoMap;

    DEFINE_T_SUPER(SimplifyViewDrawGeom)

    T_BuilderMap                                m_builders;
    ResolvedModelMapping const&                 m_parentModelMapping;
    Bentley::DgnModelRefP                       m_currentModelRef;
    ModelRefInfo                                m_currentModelInfo;
    Converter&                                  m_converter;
    SyncInfo::T_V8ElementSourceSet              m_attachmentsUnchanged;

    T_ModelRefInfoMap                           m_modelRefInfoMap;
    bool                                        m_doClip = true;
    bool                                        m_processingProxy = false;
    bool                                        m_processingText = false;

protected:
    virtual bool        _DoClipping () const override {return !m_processingProxy; }         // Proxy is already clipped and clipping is expensive.
    virtual bool        _DoTextGeometry () const override {return true;}
    virtual bool        _DoSymbolGeometry () const override {return true;}
    virtual bool        _ProcessAsFacets (bool isPolyface) const override {return false; }
    virtual bool        _ProcessAsBody (bool isCurved) const override {return false; }
    virtual StatusInt   _ProcessSolidPrimitive (Bentley::ISolidPrimitiveCR primitive) override  { return Bentley::BSIERROR;}
    virtual StatusInt   _ProcessSurface (Bentley::MSBsplineSurfaceCR surface) override  { return Bentley::BSIERROR;}
    virtual StatusInt   _ProcessFacetSet (Bentley::PolyfaceQueryCR facets, bool filled) override { return Bentley::BSIERROR;}
    virtual StatusInt   _ProcessBody (Bentley::ISolidKernelEntityCR entity, Bentley::IFaceMaterialAttachmentsCP attachments) override  { return Bentley::BSIERROR;}


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Transform   GetCurrentTransform()
    {
    return (nullptr == m_context->GetCurrLocalToFrustumTransformCP()) ? Transform::FromIdentity() : DoInterop(*m_context->GetCurrLocalToFrustumTransformCP());
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool InitGeometryParams(Render::GeometryParams& params)
    {
    DgnV8Api::ProxyDisplayHitInfo const*    proxyInfo = nullptr;
    DgnCategoryId                           categoryId;
    DgnSubCategoryId                        subCategoryId;

    if (nullptr != m_context->GetDisplayStyleHandler() &&
        nullptr != (proxyInfo = m_converter.GetProxyDisplayHitInfo(*m_context)))
        {
        categoryId = m_currentModelInfo.m_categoryId;
        subCategoryId = m_converter.GetExtractionSubCategoryId(m_currentModelInfo.m_categoryId, proxyInfo->m_viewHandlerPass.m_pass, proxyInfo->m_graphicsType);
        if (!subCategoryId.IsValid())
            {
            BeAssert(false);
            return false;
            }
        }
    else
        {
        auto   v8Model = m_currentModelRef->GetDgnModelP();
        if (nullptr == v8Model || nullptr == v8Model->GetDgnFileP())
            {
            BeAssert(false);
            }
        else
            {
            auto    levelId = m_context->GetCurrentDisplayParams()->GetLevel();

            categoryId =    m_converter.ConvertDrawingLevel(*v8Model->GetDgnFileP(), levelId);
            subCategoryId = DgnCategory::GetDefaultSubCategoryId(categoryId);
            }
        }

    params.SetCategoryId(categoryId);
    params.SetSubCategoryId(subCategoryId);
    m_converter.InitGeometryParams(params, *m_context->GetCurrentDisplayParams(), *m_context, false, m_parentModelMapping.GetV8ModelSource());

    if (m_processingText)
        {
        params.SetFillDisplay(FillDisplay::Always);
        params.SetFillColor(params.GetLineColor());
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void _DrawTextString (Bentley::TextStringCR v8Text, double* zDepth)  override
    {
    InitCurrentModel(m_context->GetCurrentModel());
    AutoRestore<bool>   saveInText(&m_processingText, true);

    DgnAttachmentCP     currentAttachment = m_currentModelRef->AsDgnAttachmentCP();
    Transform           currentTransform = GetCurrentTransform();

    if (nullptr != currentAttachment && currentAttachment->IsCameraOn())
        return T_Super::_DrawTextString(v8Text, zDepth);

    Bentley::DVec3d         zVector, zAxis = Bentley::DVec3d::From(0.0, 0.0, 1.0);
    v8Text.GetRotMatrix().GetColumn(zVector, 2);
    currentTransform.MultiplyMatrixOnly(*((DPoint3dP) &zVector));

    if (!zVector.IsParallelTo(zAxis))
        return T_Super::_DrawTextString(v8Text, zDepth);

    m_converter.ShowProgress();

    Render::GeometryParams geometryParams;

    if (!InitGeometryParams(geometryParams) || nullptr == m_currentModelRef->GetDgnFileP())
        return T_Super::_DrawTextString(v8Text, zDepth);

    TextStringPtr textString;
    Converter::ConvertTextString(textString, v8Text, *m_currentModelRef->GetDgnFileP(), m_converter);
    
    // Flatten 3D -> 2D
    Transform   flattenTrans;
    flattenTrans.InitIdentity();
    flattenTrans.form3d[2][2] = 0.0;

    textString->ApplyTransform(Transform::FromProduct(flattenTrans, m_parentModelMapping.GetTransform(), currentTransform));

    auto primitive = GeometricPrimitive::Create(*textString);
    AddToBuilder(primitive, geometryParams);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Bentley::StatusInt _ProcessCurveVector(Bentley::CurveVectorCR v8curves, bool isFilled) override
    {
    InitCurrentModel(m_context->GetCurrentModel());
    m_converter.ShowProgress();

    Render::GeometryParams geometryParams;

    if (!InitGeometryParams(geometryParams))
        return Bentley::SUCCESS;

    Bentley::Transform  currentTransform = (nullptr == m_context->GetCurrLocalToFrustumTransformCP()) ? Bentley::Transform::FromIdentity() : *m_context->GetCurrLocalToFrustumTransformCP();

    CurveVectorPtr bimcurves;
    Converter::ConvertCurveVector(bimcurves, v8curves, &m_parentModelMapping.GetTransform());   

    // Flatten 3D -> 2D
    Transform       flattenTrans;
    flattenTrans.InitIdentity();
    flattenTrans.form3d[2][2] = 0.0;

    Transform       parentAndFlatten = Transform::FromProduct(flattenTrans, m_parentModelMapping.GetTransform());

    if (m_currentModelInfo.m_useMap)
        {
        DMatrix4d     current, composite;

        composite.InitProduct(DMatrix4d::From(parentAndFlatten), m_currentModelInfo.m_map, DMatrix4d::From(DoInterop(currentTransform)));
        bimcurves = bimcurves->Clone (composite);          
        } 
    else
        {
        bimcurves->TransformInPlace(Transform::FromProduct(parentAndFlatten, DoInterop(currentTransform)));
        }

    auto    primitive = GeometricPrimitive::Create(bimcurves);
    return AddToBuilder(primitive, geometryParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Bentley::StatusInt  AddToBuilder(GeometricPrimitivePtr& primitive, Render::GeometryParams& geometryParams)
    {
    if (!primitive.IsValid())
        return Bentley::BSIERROR;

    auto&               byattachment = m_builders[m_currentModelRef];
    auto                elementRef = m_context->GetCurrDisplayPath()->GetHeadElem();
    auto&               byelement = byattachment[nullptr == elementRef ? 0 : elementRef->GetElementId()];
    GeometryBuilderPtr& builder = byelement[geometryParams.GetCategoryId()];
    if (!builder.IsValid())
        {
        builder = GeometryBuilder::Create(m_parentModelMapping.GetDgnModel(), geometryParams.GetCategoryId(), DPoint2d::FromZero());
        if (!builder.IsValid())
            return Bentley::SUCCESS;
        }

    builder->Append(geometryParams);
    builder->Append(*primitive, GeometryBuilder::CoordSystem::World);
    return Bentley::BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void _DrawShape3d (int numPoints, Bentley::DPoint3dCP points, bool filled, Bentley::DPoint3dCP range) override
    {
    if (!filled)
        T_Super::_DrawLineString3d(numPoints, points, range);       // Avoid clipping with parasolid and potentially exposing facets if nonplanar (Civil profiles).
    else
        T_Super::_DrawShape3d(numPoints, points,filled, range);
    }
public:

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool  IsElementChanged(SyncInfo::V8ElementMapping& elementMapping, DgnV8Api::EditElementHandle& v8eh)
    {
    IChangeDetector::SearchResults      syncInfoSearch;

    if (! m_converter.GetChangeDetector()._IsElementChanged(syncInfoSearch, m_converter, v8eh, m_parentModelMapping, nullptr))
        {
        elementMapping = syncInfoSearch.m_v8ElementMapping;
        m_converter.GetChangeDetector()._OnElementSeen(m_converter, syncInfoSearch.GetExistingElementId());
        return false;
        }
    else if (IChangeDetector::ChangeType::Update == syncInfoSearch.m_changeType)
        {
        m_converter.UpdateMappingInSyncInfo(syncInfoSearch.GetExistingElementId(), v8eh, m_parentModelMapping);
        elementMapping = syncInfoSearch.m_v8ElementMapping;
        m_converter.GetChangeDetector()._OnElementSeen(m_converter, syncInfoSearch.GetExistingElementId());
        }
    else
        {
        elementMapping = m_converter.RecordMappingInSyncInfo(m_parentModelMapping.GetDgnModel().GetModeledElementId(), v8eh, m_parentModelMapping);
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void InitCurrentModel(DgnModelRefP modelRef)
    {
    m_currentModelRef = modelRef;
    auto  foundInfo = m_modelRefInfoMap.find(m_currentModelRef);
    
    if (foundInfo != m_modelRefInfoMap.end())
        {
        m_currentModelInfo = foundInfo->second;
        return;
        }
    
    ModelRefInfo info;

    auto    attachment = m_currentModelRef->AsDgnAttachmentCP();
    if (nullptr == attachment)
        {
        // We are processing elements in the master DrawingModel itself.
        info.m_modelContentsChanged = !m_converter.GetChangeDetector()._AreContentsOfModelUnChanged(m_converter, m_parentModelMapping);
        info.m_modelMapping = m_parentModelMapping;
        m_modelRefInfoMap[m_currentModelRef] = info;
        m_currentModelInfo = info;
        m_attachmentsUnchanged.insert(SyncInfo::V8ElementSource (0, m_parentModelMapping.GetV8ModelSyncInfoId()));

        return;
        }
    
    if (nullptr != attachment->GetDgnModelP())
        {
        info.m_modelMapping = m_converter._FindFirstModelMappedTo(*attachment->GetDgnModelP());
        info.m_modelContentsChanged = info.m_modelMapping.IsValid() ? !m_converter.GetChangeDetector()._AreContentsOfModelUnChanged(m_converter, info.m_modelMapping) : true;        // If no model mapping can't tell whether unchanged...
        }


    for (auto thisAttachment = attachment; nullptr != thisAttachment; thisAttachment = thisAttachment->GetParentModelRefP()->AsDgnAttachmentCP())
        {
        DgnV8Api::EditElementHandle         v8eh(thisAttachment->GetElementId(), thisAttachment->GetParent().GetDgnModelP());
        SyncInfo::V8ElementMapping          attachMapping, clipMapping, maskMapping;


        if (IsElementChanged(attachMapping, v8eh))
            info.m_attachmentChanged = true;

        if (attachment == thisAttachment)
            info.m_attachElementMapping = attachMapping;

        auto                            dvSettings = thisAttachment->GetDynamicViewSettingsCR();
        DgnV8Api::EditElementHandle     clipEh, maskEh;


        if ((SUCCESS == dvSettings.GetClipBoundElemHandle(clipEh, thisAttachment->GetParentModelRefP()) && IsElementChanged (clipMapping, clipEh)) ||
            (SUCCESS == dvSettings.GetClipMaskElemHandle(maskEh, thisAttachment->GetParentModelRefP()) && IsElementChanged (maskMapping, maskEh)))
            info.m_attachmentChanged = true;

        if (thisAttachment->IsDirectDgnAttachment() && thisAttachment->IsCameraOn())
            {
            Bentley::DMap4d     toParentMap;
            Bentley::Transform  fromParentTransform;

            thisAttachment->GetTransformFromParent (fromParentTransform, false);
            thisAttachment->GetMapToParent(toParentMap, false);

            info.m_useMap = true;
            info.m_map.InitProduct(DoInterop(toParentMap.M0), DMatrix4d::From(DoInterop(fromParentTransform)));
            }
        }
    
    if (!info.m_attachmentChanged && !info.m_modelContentsChanged)
        m_attachmentsUnchanged.insert(info.m_attachElementMapping);
        
    if (nullptr != attachment->FindProxyHandler(nullptr, m_context->GetViewport()))     // This works on children (searches through parents)...
        info.m_categoryId = m_converter.GetExtractionCategoryId(*attachment);

    m_modelRefInfoMap[m_currentModelRef] = info;
    m_currentModelInfo = info;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool CreateOrUpdateDrawingGraphics()
    {
    bool        modified = false;
    SyncInfo::T_V8ElementMapOfV8ElementSourceSet v8OriginalsSeen;

    for (auto& byModelRef : m_builders)
        {
        auto                    modelRef = byModelRef.first;
        auto&                   modelRefInfo = m_modelRefInfoMap[modelRef];

        SyncInfo::V8ElementSource v8AttachmentSource = modelRefInfo.m_attachElementMapping;
        if (!v8AttachmentSource.IsValid())   // See "MergeProxyGraphicsDrawGeom handles both the drawing and attachments to the drawing"
            v8AttachmentSource = SyncInfo::V8ElementSource(0, m_parentModelMapping.GetV8ModelSyncInfoId());

        auto& v8ElementsByAttachment = v8OriginalsSeen[v8AttachmentSource];

        for (auto& byElement : byModelRef.second)
            {
            SyncInfo::V8ElementMapping originalElementMapping;
            
            if (nullptr != modelRef && nullptr != modelRef->GetDgnModelP())
                originalElementMapping = m_converter.FindFirstElementMappedTo(*modelRef->GetDgnModelP(), byElement.first);

            if (!originalElementMapping.IsValid())
                {
                // See "MergeProxyGraphicsDrawGeom handles both the drawing and attachments to the drawing"
                DgnV8Api::ElementHandle v8eh(modelRef->GetDgnModelP()->FindElementByID(byElement.first));
                SyncInfo::ElementProvenance lmt(v8eh, m_converter.GetSyncInfo(), m_converter.GetCurrentIdPolicy());
                originalElementMapping = SyncInfo::V8ElementMapping(DgnElementId(), byElement.first, m_parentModelMapping.GetV8ModelSyncInfoId(), lmt);
                }

            if (m_converter.IsUpdating())
                v8ElementsByAttachment.insert(originalElementMapping);
            
            bset<DgnCategoryId>     seenCategories;
            for (auto& bycategory : byElement.second)
                {
                modified = true;
                seenCategories.insert(bycategory.first);
                DgnDbStatus status = m_converter._CreateAndInsertExtractionGraphic(m_parentModelMapping, v8AttachmentSource, originalElementMapping, bycategory.first, *bycategory.second);
                if (DgnDbStatus::Success != status)
                    {
                    BeAssert((DgnDbStatus::LockNotHeld != status) && "Failed to get or retain necessary locks");
                    m_converter.ReportError(Converter::IssueCategory::Unknown(), Converter::Issue::ConvertFailure(), "drawing extraction");
                    }
                }
            if (m_converter.IsUpdating() &&
                m_converter._DetectedDeletedExtractionGraphicsCategories(v8AttachmentSource, originalElementMapping, seenCategories))
                modified = true;
            }
        }

    if (m_converter.IsUpdating() &&
        m_converter._DetectDeletedExtractionGraphics(m_parentModelMapping, v8OriginalsSeen, m_attachmentsUnchanged))
        modified = true;

    return !m_converter.IsUpdating() || modified;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
MergeProxyGraphicsDrawGeom(Converter& converter, ResolvedModelMapping const& parentModelMapping)
                           : m_converter(converter), m_parentModelMapping(parentModelMapping)
    { 
    }

}; // MergeProxyGraphicsDrawGeom

/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     05/2018
+===============+===============+===============+===============+===============+======*/
struct MergeDrawingContext : public DgnV8Api::NullContext
{
    DEFINE_T_SUPER(NullContext)

protected:

MergeProxyGraphicsDrawGeom&             m_output;
Converter&                              m_converter;
bool                                    m_mergeGraphicsForRenderedViews;

virtual bool _WantUndisplayed () { return false; }
virtual bool _HandleRefAsViewlet (DgnAttachmentCR thisRef) override  { return false; }


virtual void _SetupOutputs () override {SetIViewDraw (m_output);}

public:

MergeDrawingContext(MergeProxyGraphicsDrawGeom& output, Converter& converter, bool mergeGraphicsForRenderedViews) : m_output (output), m_converter(converter), m_mergeGraphicsForRenderedViews(mergeGraphicsForRenderedViews)
    {
    m_setupScan = true;
    m_purpose = DgnV8Api::DrawPurpose::DgnDbConvert;
    m_wantMaterials = true; 

    SetBlockAsynchs (true);
    _SetupOutputs ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _DrawTextString (DgnV8Api::TextString const& text) override
    {
    // NOTE: When IElementGraphicsProcessor handles TextString we don't want to spew background/adornment geometry!
    text.GetGlyphSymbology(*GetCurrentDisplayParams());
    CookDisplayParams();

    double priority = GetDisplayPriority();

    GetIDrawGeom().DrawTextString (text, text.Is3d() ? NULL : &priority);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/12
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _DrawSymbol (DgnV8Api::IDisplaySymbol* symbolDef, Bentley::Transform const* trans, Bentley::ClipPlaneSet* clipPlanes, bool ignoreColor, bool ignoreWeight) override
    {
    // Pass along any symbol that is drawn from _ExpandPatterns/_ExpandLineStyles, etc.
    m_output.ClipAndProcessSymbol (symbolDef, trans, clipPlanes, ignoreColor, ignoreWeight);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/13
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _CookDisplayParams (ElemDisplayParamsR elParams, ElemMatSymbR elMatSymb) override
    {
    DgnV8Api::ElemDisplayParams tmpElParams(elParams);

    tmpElParams.Resolve (*this);
    elMatSymb.FromResolvedElemDisplayParams (tmpElParams, *this, m_startTangent, m_endTangent);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
virtual Bentley::StatusInt       _OnNewModelRef (DgnModelRefP modelRef) override
    {
    Bentley::StatusInt   status = T_Super::_OnNewModelRef(modelRef);
    auto        viewFlags = *GetViewFlags();

    viewFlags.text_nodes = false;
    SetViewFlags(&viewFlags);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void _DrawModelRef (DgnV8Api::DgnModelRef* baseModelRef, DgnV8Api::DgnModelRefList* includeList, bool useUpdateSequence, bool includeRefs) override
    {
    if (nullptr == baseModelRef->GetDgnModelP())
        return;

    m_converter.GetV8FileSyncInfoId(*baseModelRef->GetDgnModelP()->GetDgnFileP());  // May be the first we've encountered this file. Map it in now, so that downstream conversion mappings can refer to it.
    
    // Rendered views are handled as direct sheet attachments -- so we ignore them if processing a drawing that will be viewed through a sheet.
    if (!m_mergeGraphicsForRenderedViews &&
        nullptr != baseModelRef->AsDgnAttachmentP() &&
        m_converter._UseRenderedViewAttachmentFor(*baseModelRef->AsDgnAttachmentP()))
        return;

    
    m_output.InitCurrentModel(baseModelRef);
    bool        drawThisModel = (m_output.m_currentModelInfo.m_modelContentsChanged || (m_output.m_currentModelRef->IsDgnAttachment() && m_output.m_currentModelInfo.m_attachmentChanged));
    m_viewHandler->DrawModelRef (*this, baseModelRef, includeList, useUpdateSequence, drawThisModel, includeRefs);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   _VisitElemHandle (DgnV8Api::ElementHandle const& inEl, bool checkRange, bool checkScanCriteria) override
    {
    if (m_converter.IsUpdating() && !m_output.m_currentModelInfo.m_attachmentChanged)
        {
        IChangeDetector::SearchResults      syncInfoSearch;
        if (!m_converter.GetChangeDetector()._IsElementChanged(syncInfoSearch, m_converter, inEl, m_output.m_currentModelInfo.m_modelMapping))
            {
            // The existing V8 attachment is unchanged, so there's no need to re-generate the proxy graphics. Just record the fact
            // that we have found a mapping to the BIM Drawing element.
            m_converter.GetChangeDetector()._OnElementSeen(m_converter, syncInfoSearch.GetExistingElementId());

            // Add an empty entry so that that this is recorded as seen.
            m_output.m_builders[GetCurrentModel()][inEl.GetElementId()] = bmap<DgnCategoryId, GeometryBuilderPtr>();

            return SUCCESS;
            }
        }

    // *** TRICKY: For drawings, we do not convert all levels ahead of time. We wait until we see which ones are used.
    //              That's how we tell which should be DrawingCategories instead of SpatialCategories. We must therefore
    //              visit complex children and ensure that their levels are converted.
    m_converter.ConvertLevels(inEl);

    return T_Super::_VisitElemHandle(inEl, checkRange, checkScanCriteria);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void Process(ViewportR viewport, DgnV8Api::DgnModelRef* baseModelRef)
    {
    Attach (&viewport,  DgnV8Api::DrawPurpose::DgnDbConvert);
    InitScanRangeAndPolyhedron ();
    GetCurrentDisplayParams()->Init ();
    SetScanReturn();

    if (SUCCESS == PushModelRef(baseModelRef, true))
        _DrawModelRef(baseModelRef, nullptr, false, true);

    Detach ();
    }

}; // MergeDrawingContext


/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     02/2013
+===============+===============+===============+===============+===============+======*/
struct CreateCveMeter : DgnV8Api::VisEdgesProgressMeter
{
    Converter&              m_converter;
    DgnProgressMeter::Abort m_aborted;

    CreateCveMeter(Converter& c) : m_converter(c), m_aborted(DgnProgressMeter::ABORT_No) {}

    virtual void            _SetTaskName (WCharCP taskName)  override       { m_aborted = m_converter.GetProgressMeter().ShowProgress(); }
    virtual void            _IndicateProgress (double fraction) override    { m_aborted = m_converter.GetProgressMeter().ShowProgress(); }
    virtual void            _Terminate () override                          {}
    virtual bool            _WasAborted () override                         { return DgnProgressMeter::ABORT_Yes == m_aborted; }
    virtual void            _DisplayMessageCenter (DgnV8Api::OutputMessagePriority priority, DgnV8Api::OutputMessageAlert openAlert, Bentley::WStringCR brief, Bentley::WStringCR detailed) override
        {
        Utf8String msg(brief.c_str());
        msg.append(" - ");
        msg.append(Utf8String(detailed.c_str()));

        Converter::IssueSeverity sev = (DgnV8Api::OutputMessagePriority::Error == priority || DgnV8Api::OutputMessagePriority::Fatal == priority)?
                                        Converter::IssueSeverity::Error: 
                                        (DgnV8Api::OutputMessagePriority::Warning == priority)?
                                        Converter::IssueSeverity::Warning:
                                        Converter::IssueSeverity::Info;

        m_converter.ReportIssue(sev, Converter::IssueCategory::Unknown(), Converter::Issue::ConvertFailure(), msg.c_str());
        }
};  

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt generateCve (DgnAttachmentP refP, ViewportP viewport, CachedVisibleEdgeOptionsCP pInputOptions, CreateCveMeter* meter, bool setAlwaysValid)
    {
    DgnV8Api::VisibleEdgeCache*           currentCache;

    if (NULL == refP ||
        NULL == viewport)
        {
        BeAssert(false);
        return ERROR;
        }
    DgnModelRefP modelRef = refP;

    if (NULL != (currentCache = dynamic_cast <DgnV8Api::VisibleEdgeCache*> (refP->GetProxyCache())) && (NULL != viewport && currentCache->IsValidForViewport (*viewport)))
        return SUCCESS;

    BeAssert (refP->IsDisplayedInViewport (*viewport, true));

    DgnV8Api::CachedVisibleEdgeOptions    options;

    // if we got options passed in, we use those.
    DgnV8Api::Tcb const* tcb = modelRef->GetDgnFileP()->GetPersistentTcb();
    if (NULL == pInputOptions)
        {
        if (NULL == currentCache)
            options = DgnV8Api::CachedVisibleEdgeOptions (*tcb, modelRef);
        else
            options = currentCache->GetOptions();
        }
    else
        {
        options = *pInputOptions;
        }

    DgnV8Api::VisibleEdgeCache*           proxyCache = DgnV8Api::VisibleEdgeCache::Create (modelRef, options);
                              
    if (SUCCESS ==  DgnV8Api::VisibleEdgesLib::CreateProxyCache (*proxyCache, NULL, modelRef, viewport, meter))
        {
        proxyCache->Resolve ();

        if (!setAlwaysValid)
            proxyCache->ComputeHash (modelRef, viewport);
        else
            proxyCache->ClearElementModifiedTimes (false);
        }


    // it's valid for the view we created it for. It might be valid for other views, depending on what levels, etc., they are displaying. We have to run the test.
    //UInt32  thisView = viewport->GetViewNumber();
    UInt32  thisView = 0; // *** we always use a fake view
    proxyCache->SetValidForView (thisView, true);

    refP->SetProxyCache (proxyCache, DgnV8Api::ProxyDgnAttachmentHandlerManager::GetManager().GetHandler (DgnV8Api::CachedVisibleEdgeHandlerId()));

    // write the proxy cache as XAttributes to the reference element.
    return proxyCache->Save (modelRef);

    // Note: It is up to the caller to make sure that the reference element itself is saved.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool Converter::HasRenderedViewAttachments(DgnV8ModelR v8ParentModel)
    {
    auto                    attachments = GetAttachments(v8ParentModel);

    if (nullptr != attachments)
        for (auto attachment : *attachments)
            if (_UseRenderedViewAttachmentFor(*attachment))
                return true;

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool Converter::_UseRenderedViewAttachmentFor(DgnAttachmentR ref)
    {
    if (!ref.Is3d() || ref.IsTemporary() || IsSimpleWireframeAttachment(ref) || nullptr != ref.GetProxyCache() || nullptr == ref.GetDgnModelP())
        return false;

    auto const&                 dynamicViewSettings = ref.GetDynamicViewSettingsCR();

    if (nullptr !=  dynamicViewSettings.GetClipBoundElementRef(&ref) &&
        dynamicViewSettings.ShouldDisplayCut())
        {
        return false;
        }
        
    for (int i=0; i<8; i++)
        if (ref.GetViewFlags(i).renderMode  >= (UInt32) DgnV8Api::MSRenderMode::SmoothShade)
            return true;

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool Converter::IsSimpleWireframeAttachment(DgnAttachmentCR ref)
    {
    static bool s_forceDisable = false;

    if (s_forceDisable)
        return false;

    if (!ref.Is3d() || ref.IsTemporary())
        return false;

    if ((nullptr != ref.GetDynamicViewSettingsCR().GetClipBoundElementRef(ref.GetDgnModelP()) ||
         nullptr != ref.GetDynamicViewSettingsCR().GetClipMaskElementRef(ref.GetDgnModelP())) &&
         ref.GetDynamicViewSettingsCR().GetMaximumClipRenderMode(ref.GetParentModelRefP()) > (UInt32) DgnV8Api::MSRenderMode::Wireframe) 
        return false;

    for (int i=0; i<8; i++)
        if (ref.GetViewFlags(i).renderMode != (UInt32) DgnV8Api::MSRenderMode::Wireframe)
            return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool Converter::_GenerateProxyCacheFor(DgnAttachmentR ref)
    {
    if (!ref.Is3d() || ref.IsTemporary() || IsSimpleWireframeAttachment(ref) || _UseRenderedViewAttachmentFor(ref)) 
        return false;
    
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static void  createProxyGraphics (Converter& converter, DgnModelRefR modelRef, ViewportR viewport, bool createProxyGraphicsForRenderedViews)
    {
    auto attachedV8Model = modelRef.GetDgnModelP();

    if (attachedV8Model == nullptr)
        return;

    DgnAttachmentP      attachment = modelRef.AsDgnAttachmentP();
    if (nullptr  != attachment)
        {
        if((converter._GenerateProxyCacheFor(*attachment) || (createProxyGraphicsForRenderedViews && converter._UseRenderedViewAttachmentFor(*attachment))) && !converter.HasProxyGraphicsCache(*attachment, &viewport))
            {
            CreateCveMeter meter(converter);

            attachedV8Model->SetReadOnly(false);
            if (SUCCESS == generateCve(attachment, &viewport, nullptr, &meter, true))
                {
                attachment->SetProxyCachingOption(DgnV8Api::ProxyCachingOption::Cached);
                attachment->Rewrite(true, true);
                return;
                }
            }
        }
    DgnAttachmentArrayP childAttachments = converter.GetAttachments(modelRef);
    if (nullptr != childAttachments)
        for (auto& childAttachment : *childAttachments)
                createProxyGraphics (converter, *childAttachment, viewport, createProxyGraphicsForRenderedViews);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static bool createOrUpdateDrawingGraphics(Converter& converter, Bentley::DgnModelRefR baseModelRef, ResolvedModelMapping const& v8mm, ViewportR viewport, bool mergeGraphicsForRenderedViews)
    {
    MergeProxyGraphicsDrawGeom      drawGeom(converter, v8mm);
    MergeDrawingContext             mergeDrawingContext(drawGeom, converter, mergeGraphicsForRenderedViews);

    drawGeom.SetViewContext(&mergeDrawingContext);
    mergeDrawingContext.Process(viewport, &baseModelRef);
    return drawGeom.CreateOrUpdateDrawingGraphics();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnV8Api::ViewInfoPtr  createFakeViewInfo(DgnV8ModelR v8ParentModel, Converter& converter)
    {
    Bentley::DRange3d sheetModelRange;

    v8ParentModel.GetRange(sheetModelRange);
    if (sheetModelRange.IsEmpty() || sheetModelRange.IsNull() || sheetModelRange.IsPoint())
        sheetModelRange = Bentley::DRange3d::FromMinMax(-1000000, 1000000);

    return converter.CreateV8ViewInfo(v8ParentModel, sheetModelRange);
    }

struct FakeViewport : DgnV8Api::NonVisibleViewport
    {
    FakeViewport(DgnV8Api::ViewInfo& viewInfo) :  DgnV8Api::NonVisibleViewport(viewInfo) { m_viewNumber = 0; m_backgroundColor.m_int = 0xffffff; }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::DrawingsConvertModelAndViews(ResolvedModelMapping const& v8mm)
    {
    if (!v8mm.GetDgnModel().IsDrawingModel())
        {
        BeAssert(false);
        return;
        }
    DgnV8ModelR v8ParentModel = v8mm.GetV8Model();

    if (LOG_IS_SEVERITY_ENABLED(LOG_TRACE))
        LOG.tracev("DrawingsConvertModelAndViews %s -> %s", IssueReporter::FmtModel(v8ParentModel).c_str(), IssueReporter::FmtModel(v8mm.GetDgnModel()).c_str());

    // NB: We must be sure that we have all of the attachments loaded and filled if we want to create proxy graphics. Don't leave
    //      that to chance. And don't assume that if v8ParentModel.GetDgnAttachmentsP() returns non-null then we have all the attachments.
    //      we might not have all nested refs loaded, and we might not have them filled. They must all be loaded and filled!
        {
        DgnV8Api::DgnAttachmentLoadOptions loadOptions;
        loadOptions.SetTopLevelModel(true);
        loadOptions.SetShowProgressMeter(false); // turn this off for now. It seems to increment the task count for every ref, but it doesn't decrement the count afterward.
        v8ParentModel.ReadAndLoadDgnAttachments(loadOptions);
        }

    
    DgnV8Api::ViewInfo const*   foundViewInfo = nullptr;
    auto                        vg = FindFirstViewGroupShowing(v8ParentModel);
    if (vg.IsValid())
        {
        for (int i=0; i<DgnV8Api::MAX_VIEWS; ++i)
            {
            if (vg->GetViewInfo(i).GetRootModelId() == v8ParentModel.GetModelId())
                {
                foundViewInfo = &vg->GetViewInfo(i);
                break;
                }
            }
        }

    DgnV8Api::ViewInfoPtr       viewInfo = (nullptr == foundViewInfo) ? createFakeViewInfo(v8ParentModel, *this):  DgnV8Api::ViewInfo::CopyFrom(*foundViewInfo, true, true, true);
    FakeViewport                fakeVp(*viewInfo);

    if (LOG_IS_SEVERITY_ENABLED(LOG_TRACE))
        LOG.tracev("DrawingsConvertModelAndViews %s -> %s", IssueReporter::FmtModel(v8ParentModel).c_str(), IssueReporter::FmtModel(v8mm.GetDgnModel()).c_str());

    bool isThisIndependentDrawing = !ConverterMadeCopyMarker::IsFoundOn(v8ParentModel);

    createProxyGraphics(*this, v8ParentModel, fakeVp, isThisIndependentDrawing /* If independent generate proxy for rendered views */);
    if (!createOrUpdateDrawingGraphics(*this, v8ParentModel, v8mm, fakeVp, isThisIndependentDrawing /* If independent merge proxy for rendered views */))
        m_unchangedModels.insert(v8mm.GetDgnModel().GetModelId());

    //  Convert all views of this model
    for (DgnV8Api::ViewGroupPtr const& vg : v8ParentModel.GetDgnFileP()->GetViewGroups())
        {
        DrawingViewFactory svf(v8mm);
        Convert2dViewsOf(viewInfo, *vg, v8ParentModel, svf); 
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::CreateSheetExtractionAttachments(ResolvedModelMapping const& v8SheetModelMapping, ProxyGraphicsDrawingFactory& drawingGenerator, Bentley::ViewInfoCP v8SheetView)
    {
    DgnV8ModelR             v8ParentModel = v8SheetModelMapping.GetV8Model();
    auto                    attachments = GetAttachments(v8ParentModel);
    DgnV8Api::ViewInfoPtr   viewInfo = (nullptr == v8SheetView) ? createFakeViewInfo(v8ParentModel, *this):  DgnV8Api::ViewInfo::CopyFrom(*v8SheetView, true, true, true);
    FakeViewport            fakeVp(*viewInfo);
    
    if (nullptr != attachments)
        {
        for (auto attachment : *attachments)
            {
            if (attachment->Is3d() && !_UseRenderedViewAttachmentFor(*attachment) && nullptr != attachment->GetDgnModelP())             // NEEDS_WORK -- rendered attachment.
                {
                auto                createdDrawing = drawingGenerator._CreateAndInsertDrawing(*attachment, v8SheetModelMapping, *this);

                createdDrawing.SetTransform(v8SheetModelMapping.GetTransform());
                createProxyGraphics(*this, *attachment, fakeVp, false);
                createOrUpdateDrawingGraphics(*this, *attachment, createdDrawing, fakeVp, false);
                }
            }
        }
    }


END_DGNDBSYNC_DGNV8_NAMESPACE

// "Attachments to V8 drawings and sheets"
//
//  Terminology:
//  "Importing" (aka "mapping") a model means creating an empty BIM model to represent the V8 model.
//  "Converting" a model means creating elements and other BIM content from the contents of the V8 model.
//  Importing/mapping is done as the very first phase of the converter.
//  Converting is done much later, after styles and views and ECSchemas have been converted.
//
//  Note that we ignore 3D attachments to 2d parents entirely at *model import* time. 
//  3d models are mapped into the BIM only when found via attachments to the spatial rootmodel.
//  3d content is pulled into drawings and sheets via conversion to 2d proxy graphics, as explained below.
//
//  Here is how attachments are handled by the converter when the root model is a V8 drawing or sheet:
//
//  Root model = drawing. 
//      We do not use attachments found in a V8 drawing model to map any other models in to the BIM. That is because we will copy the
//      *content* of the attached models into the root drawing model later by visiting the drawing with a V8 view context and 
//      recording the graphics as it would be displayed.
////  Root model = sheet. 
//      2d attachment: 
//          We map the *directly* attached drawings into BIM. See the comment "DgnModel objects and Sheet attachments" below for why we *must* map in 2d models found by 
//          direct attachments to sheets. 
//          Later, we will convert the drawing's contents as described in the case above.
//          Finally, we will create a view of the mapped DrawingModel.
//      3d attachment:
//          We will generate a BIM DrawingModel that captures the attached 3D content as described above.
//          We then attach a view of that generated drawing to the sheet.
//      Note that a direct attachment to a sheet must be to a model that has been mapped into the BIM, even if it's a generated model. That is because
//      The sheet wants to attach a BIM view of that model, so obviously the model itself must exist in the BIM.


//  "Annotation Scale and V8 drawings and Sheets"
//  
//  When a sheet or drawing has a reference to a sheet or drawing, the attachment may specify that the annotations (text, dims, etc.) in the attached
//  model should be displayed at a different scale. Changing the "annotation scale" of an annotation element is not the same as applying a simple
//  scale factor to its geometry. In V8, this annotation re-scaling is done at draw time (in a very complicated way that meshes with the reference transform). 
//  BIM does not have any concept of of reference attachments.
//  BIM does not have any real concept of annotation scale. 
//  In BIM, a drawing or sheet are always full size, and so the text, dims, etc. in the model are to be placed in the size ultimately intended for plotting.
//  Change the scale of a drawing or sheet means changing the geometry of the elements in it.
//  
//  The converter handles an attachment that implies annotation re-scaling by making a copy of the attached model and changing its contents to the required scale.
//  This transformation is applied recursively to all nested attachments.
//  The result is a new, temporary copy of the original V8 DgnModel, and the converter converts the content of the copy, not the original, when processing
//  the DgnAttachment.
//  Note that this transform is done by making a copy of the V8 DgnModel and transforming it. We do not make a copy of the BIM model and try to transform it.
//  Only V8 has annotation re-scaling logic.
//
//  This transform is done at *model import* time. That is, we are tweaking the V8 *input* to the converter. 
//  The later phases of the converter, such as where we create views of drawings attached to sheets, see the transformed copies of the
//  V8 models. The rules about processing attachments to 2d root models described above apply to these transformed copies just the same as they apply to other V8 attachments.


//  "DgnModel objects and Sheet attachments"
//
// **** Make sure our m_v8ModelMappings points to the the same set of DgnModels that V8 knows about! ****
// We use 2 passes so that the drawings that are referenced by sheets are found in the first pass, by ImportSheetModelsInFile, 
// and not in the second pass by ImportDrawingModelsInFile. That is the way to avoid creating duplicate V8 DgnModel objects in memory. 
// Specifically, when we tell V8 to load the reference attachments for a given sheet model, it will create V8 DgnModel objects in memory 
// for the drawings attached to the sheet, and those are the V8 DgnModel objects that we will see later on when we process the sheet's attachments 
// in order to create ViewAttachments. We want to make sure that those are the V8 DgnModel objects that are mapped to BIM models. 
// If, instead, we had discovered those *same* V8 drawing models in the second pass (in ImportDrawingModelsInFile), we would create
// *new* V8 DgnModel objects for them. The new objects would be distinct from but duplicates of the V8 DgnModel objects that the sheet's attachments point to. 
// That would lead to mass confusion. We would think that a given V8 DgnModel object represents a given V8 model, but V8 would think that some other
// V8 DgnModel object represents that model. We would map the one into BIM, and the duplicate would have no mapping. We would fill the one model, and the 
// duplicate would remain unfilled, and so on.

//  "Keeping sheet and drawing models alive"
//
// Sheets are never kept alive by references, and drawings are also often un-referenced.
// In this converter, sheets and drawings are never treated as V8 "root models" (since their containing files may in fact be reference files).
// So, we have to keep sheet and drawing models alive by holding a reference to them explicitly.
// Note that holding a reference to a model also keeps the containing DgnFile alive (even though its fileref remains zero). 
// But that is not enough. We must also hold a reference to the file. That is to prevent this callstack from freeing the attachments
// that we set up so carefully for scale-specific or other manipulated drawings and sheets:
// 	DgnPlatform5.dll!Bentley::DgnPlatform::DgnModelRef::internal_DeleteDgnAttachmentList() Line 3543	C++
// 	DgnPlatform5.dll!Bentley::DgnPlatform::DgnFile::DeleteRootModelList() Line 4153	C++
// 	DgnPlatform5.dll!Bentley::DgnPlatform::DgnFile::_ReleaseFileRef() Line 706	C++
// 	DgnV8ConverterB02.dll!Bentley::RefCountedPtr<Bentley::DgnPlatform::DgnFile>::~RefCountedPtr<Bentley::DgnPlatform::DgnFile>() Line 167	C++
// 	DgnV8ConverterB02.dll!BentleyB0200::Dgn::DgnDbSync::DgnV8::Converter::Convert2dViewsOf(Bentley::RefCountedPtr<Bentley::DgnPlatform::ViewInfo> & firstViewInfo, Bentley::DgnPlatform::ViewGroup & vg, Bentley::DgnPlatform::DgnModel & rootModel, BentleyB0200::Dgn::DgnDbSync::DgnV8::ViewFactory & fac) Line 747	C++
// 	DgnV8ConverterB02.dll!BentleyB0200::Dgn::DgnDbSync::DgnV8::Converter::SheetsConvertModelAndViews(const BentleyB0200::Dgn::DgnDbSync::DgnV8::ResolvedModelMapping & v8mm, BentleyB0200::Dgn::DgnDbSync::DgnV8::ViewFactory & nvvf) Line 123	C++
// 	DgnV8ConverterB02.dll!BentleyB0200::Dgn::DgnDbSync::DgnV8::RootModelConverter::_ConvertSheets() Line 74	C++
// 	DgnV8ConverterB02.dll!BentleyB0200::Dgn::DgnDbSync::DgnV8::RootModelConverter::Process() Line 2135	C++


// "Why do sheets need to make copies of their attachments?"
// Some of the cases are explained above, for example, when a sheet needs a re-scaled copy of a drawing.
//
// Why in other cases? To avoid duplicate graphics in the case where a drawing has nested attachments of its own.
// When a drawing is attached to a sheet, the nested 3d attachments are un-nested and become direct attachments of the sheet. 
// When a drawing is displayed in its own right in its own view, then its nested attachments must be merged into it.
// So, obviously, the bridge cannot apply those two different transformations to the same model. (Note that these transformations are done on the V8 models (in memory) before the conversion really starts.)
//

// "MergeProxyGraphicsDrawGeom handles both the drawing and attachments to the drawing"
// MergeProxyGraphicsDrawGeom captures all graphics that appear in a V8 drawing, including
// graphics generated from elements in the v8 drawing model itself, as well as graphics
// generated from elements in attachments to the v8 drawing model. In some cases, the 
// referenced graphics will have been generated by CVE, e.g., for section cuts, but that is a detail.
// In some cases, the graphics for elements in the drawing itself will be clipped or otherwise
// different from the original elements. That is also a detail. In all cases, the graphics
// are proxies for the original elements.
//
// In order to support updates, where the converter detects changes and converts only what has changed,
// the converter keeps track of the mapping from the original V8 elements to the generated
// DrawingGraphic elements in the SyncInfo db. On an update, Converter::_CreateAndInsertExtractionGraphic looks up
// the original V8 elements and updates them if they exist. Otherwise, it inserts new DrawingGraphic
// elements.
//
// SyncInfo does not use the normal element mapping table for proxy graphics. It has dedicated table called v8sync_ExtractedGraphic.
// That table is defined to handle the complicated case of CVE, where a single 3d element can throw off
// many proxy graphics, where the graphics are assigned to various categories, and where all of that
// is specific to a given DgnAttachment. The same v8sync_ExtractedGraphic must also handle the simpler case
// where the proxy graphics are generated from elements in the drawing model itself. In that case,
// the attachment-specific columns will be 0 and OriginalV8ModelSyncInfoId will be the same as DrawingV8ModelSyncInfoId.
//
// MergeProxyGraphicsDrawGeom::CreateOrUpdateDrawingGraphics handles the non-attachment case by generating a dummy object to represent
// missing attachment.
