/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnV8/DrawingConverter.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ConverterInternal.h"

BEGIN_DGNDBSYNC_DGNV8_NAMESPACE

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct DrawingHoldsProxyGraphics : Converter::ProxyGraphicsDrawingFactory
    {
    ResolvedModelMapping const& m_drawingModelMapping;

    DrawingHoldsProxyGraphics(ResolvedModelMapping const& v8mm) : m_drawingModelMapping(v8mm) {}
    
    ResolvedModelMappingWithElement _CreateAndInsertDrawing(DgnAttachmentCR v8Attachment, ResolvedModelMapping const& parentModel, Converter& converter) override
        {
        DgnV8Api::EditElementHandle v8eh(v8Attachment.GetElementId(), &parentModel.GetV8Model());
        return ResolvedModelMappingWithElement(parentModel, converter.RecordMappingInSyncInfo(parentModel.GetDgnModel().GetModeledElementId(), v8eh, parentModel));
        }

    ResolvedModelMapping _GetDrawing(DgnV8Api::ElementId, ResolvedModelMapping const& parentModel, Converter&) override
        {
        return parentModel;
        }

    bool _UseProxyGraphicsFor(DgnAttachmentCR attachment, Converter&) override {return attachment.Is3d() && !attachment.IsTemporary();}
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct DrawingViewFactory : ViewFactory
    {
    ResolvedModelMapping const& m_drawingModelMapping;
    DrawingViewFactory(ResolvedModelMapping const& v8mm) : m_drawingModelMapping(v8mm) {}

    ViewDefinitionPtr _MakeView(Converter& converter, ViewDefinitionParams const&) override;
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
        if (v8mm.GetDgnModel().IsDrawingModel())
            drawings.insert(v8mm);
        }

    AddTasks((uint32_t)drawings.size());

    for (auto v8mm : drawings)
        {
        SetTaskName(Converter::ProgressMessage::TASK_CONVERTING_MODEL(), v8mm.GetDgnModel().GetName().c_str());
    
        DrawingsConvertModelAndViews(v8mm);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/16
+---------------+---------------+---------------+---------------+---------------+------*/
bpair<ResolvedModelMapping,bool> Converter::Import2dModel(DgnV8ModelR v8model)
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
        _KeepFileAlive(thisV8File);     // keep the file alive
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
            for (DgnV8Api::ModelIndexItem const& item : v8File->GetModelIndex())
                {
                cb(*v8File, item);
                }
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

    auto importres = Import2dModel(v8model);
    
    ResolvedModelMapping const& resolvedMapping = importres.first;
    if (!resolvedMapping.IsValid()) // if the import failed,
        return;                     // there is nothing more we can do

    // Since there is no such thing as an attachment to a drawing, we must merge any V8 attachments
    // into the imported parent drawing model.
    DrawingRegisterAttachmentsToBeMerged(v8model, importres.first);
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

    if (0 != BeNumerical::Compare(requiredEffectiveAnnotationScale, refAnnotationScale))
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
void            Converter::MakeAttachmentsMatchRootAnnotationScale(DgnModelRefR parent)
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
bvector<DgnV8Api::ElementId> Converter::GetAttachmentsToBeUnnested(DgnV8ModelR sheetModel)
    {
    bvector<DgnV8Api::ElementId> attachmentIds;

    auto attachments = sheetModel.GetDgnAttachmentsP();
    if (nullptr == attachments)
        return attachmentIds;

    for (auto attachment : *attachments)
        {
        if (nullptr == attachment->GetDgnModelP())
            continue;
        if (attachment->Is3d())
            continue;
            
        attachmentIds.push_back(attachment->GetElementId());
        }

    return attachmentIds;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::UnnestAttachments(DgnV8ModelR parentModel)
    {
    parentModel.SetReadOnly(false);
    
    //  First, unnest all 2D attachments
    auto vg = FindFirstViewGroupShowing(parentModel);
    for (auto attachmentId : GetAttachmentsToBeUnnested(parentModel))
        {
        auto attachment = DgnV8Api::DgnAttachment::FindByElementId(&parentModel, attachmentId);
        if (nullptr == attachment)
            {
            BeAssert(false);
            continue;
            }
        if (0 != attachment->MoveNestedAttachmentsToParent(-1, vg.get()))
            {
            BeAssert(false);
            }
        }

    auto attachments = GetAttachments(parentModel);
    if (nullptr == attachments)
        return;

    // Make sure all (new) attachments are classified.
    for (auto attachment : *attachments)
        {
        if (attachment->GetDgnModelP())
            Classify2dModelIfNormal(*attachment->GetDgnModelP(), nullptr);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::TransformSheetAttachmentsToDrawings(DgnV8ModelR parentModel)
    {
    auto attachments = GetAttachments(parentModel);
    if (nullptr == attachments)
        return;

    for (auto attachment : *attachments)
        {
        if ((nullptr == attachment->GetDgnModelP()) || !attachment->IsSheet())
            continue;

        auto attachedModelAsDrawing = CopyAndChangeSheetToDrawing(attachment->GetDgnModelP());
        if (!attachedModelAsDrawing.IsValid())
            continue;

        parentModel.SetReadOnly(false);

        attachment->SetDgnModel(attachedModelAsDrawing.get()); // attachment will add a reference to the new model, keeping it alive.
        GetAttachments(*attachment); // SetDgnModel deleted the nested attachment array. Rebuild it.
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
    
    UnnestAttachments(v8ParentModel);
    
    TransformSheetAttachmentsToDrawings(v8ParentModel);
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

    TransformDrawingAttachmentsToCopies(*v8model, [this] (DgnV8Api::DgnAttachment const& attachment)    // See "Why do sheet need to make copies of their attachments?"
        {
        if (this->_GetParams().GetConvertViewsOfAllDrawings())          // 
            return true;
        return (attachment.GetDgnModelP() == this->GetRootModelP());
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

    DgnDbR db = converter.GetDgnDb();

    converter._TurnOnExtractionCategories(*parms.m_categories);

    DrawingViewDefinitionPtr view = new DrawingViewDefinition(*converter.GetJobDefinitionModel(), parms.m_name, parms.GetDgnModel().GetModelId(), *parms.m_categories, *parms.m_dstyle->ToDisplayStyle2dP());

    parms.Apply(*view);

    converter.ConvertLevelMask(*view, parms.m_viewInfo, &parms.GetV8Model());

    return view;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::DoConvertDrawingElementsInModel(ResolvedModelMapping const& v8mm)
    {
    if (GetChangeDetector()._AreContentsOfModelUnChanged(*this, v8mm))
        return;

    v8mm.GetV8Model().FillSections(DgnV8Api::DgnModelSections::Model);

    DgnV8Api::PersistentElementRefList* controlElements = v8mm.GetV8Model().GetControlElementsP();
    if (nullptr != controlElements)
        {
        for (DgnV8Api::PersistentElementRef* v8Element : *controlElements)
            {
            DgnV8Api::EditElementHandle v8eh(v8Element);
            ElementConversionResults results;
            _ConvertControlElement(results, v8eh, v8mm);
            }
        }

    DgnV8Api::PersistentElementRefList* graphicElements = v8mm.GetV8Model().GetGraphicElementsP();
    if (nullptr != graphicElements)
        {
        for (DgnV8Api::PersistentElementRef* v8Element : *graphicElements)
            {
            DgnV8Api::EditElementHandle v8eh(v8Element);
            _ConvertDrawingElement(v8eh, v8mm);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::DrawingRegisterModelToBeMerged(bvector<ResolvedModelMapping>& tbm, DgnAttachmentR v8DgnAttachment, DgnModelR targetBimModel, TransformCR transformToParent)
    {
    auto attachedV8model = v8DgnAttachment.GetDgnModelP();

    auto& referenceFile = *attachedV8model->GetDgnFileP();
    SyncInfo::V8FileSyncInfoId v8FileId = GetV8FileSyncInfoId(referenceFile);
    if (!v8FileId.IsValid())
        {
        BeAssert(false);
        ReportError(Converter::IssueCategory::Unknown(), Converter::Issue::ConvertFailure(), "reference file not found");
        return;
        }

    auto refTrans = ComputeAttachmentTransform(transformToParent, v8DgnAttachment);

    SyncInfo::V8ModelMapping refSyncInfoMapping;
    if (BSISUCCESS != GetSyncInfo().InsertModel(refSyncInfoMapping, targetBimModel.GetModelId(), *attachedV8model, refTrans))
        {
        BeAssert(false);
        LOG.infov("DrawingRegisterModelToBeMerged %s -> %s failed - duplicate attachments??", IssueReporter::FmtModel(*attachedV8model).c_str(), IssueReporter::FmtModel(targetBimModel).c_str());
        return;
        }

    ResolvedModelMapping mergedModelMapping(targetBimModel, *attachedV8model, refSyncInfoMapping, &v8DgnAttachment);

    tbm.push_back(mergedModelMapping);

    if (LOG_IS_SEVERITY_ENABLED(LOG_TRACE))
        LOG.tracev("DrawingRegisterModelToBeMerged %s -> %s", IssueReporter::FmtModel(*attachedV8model).c_str(), IssueReporter::FmtModel(targetBimModel).c_str());

    // recurse over nested attachments -- NEEDS WORK: All nested attachments should have been un-nested before this function is called!
    DrawingRegisterAttachmentsToBeMerged(tbm, v8DgnAttachment, targetBimModel, refTrans);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::DrawingRegisterAttachmentsToBeMerged(bvector<ResolvedModelMapping>& tbm, DgnV8ModelRefR v8modelRef, DgnModelR targetBimModel, TransformCR transformToParent)
    {
    auto attachments = GetAttachments(v8modelRef);
    if (nullptr == attachments)
        return;

    for (auto attachment : *attachments)
        {
        if (!attachment->Is3d() && !attachment->IsTemporary())
            {
            DrawingRegisterModelToBeMerged(tbm, *attachment, targetBimModel, transformToParent);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::DrawingRegisterAttachmentsToBeMerged(DgnV8ModelRefR v8modelRef, ResolvedModelMapping const& drawingModelMapping)
    {
    bvector<ResolvedModelMapping> tbm;
    DrawingRegisterAttachmentsToBeMerged(tbm, v8modelRef, drawingModelMapping.GetDgnModel(), drawingModelMapping.GetTransform());
    if (!tbm.empty())
        ModelsToBeMerged::AddTo(*v8modelRef.GetDgnModelP(), tbm);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::DrawingsConvertModelAndViews(ResolvedModelMapping const& v8mm)
    {
    if (!v8mm.GetDgnModel().IsDrawingModel())
        {
        BeAssert(false);
        return;
        }

    DgnV8ModelR v8model = v8mm.GetV8Model();

    if (LOG_IS_SEVERITY_ENABLED(LOG_TRACE))
        LOG.tracev("DrawingsConvertModelAndViews %s -> %s", IssueReporter::FmtModel(v8model).c_str(), IssueReporter::FmtModel(v8mm.GetDgnModel()).c_str());

    // Convert the elements in the drawing. This also converts the levels that are actually used by the elements.
    DoConvertDrawingElementsInModel(v8mm);
    if (WasAborted())
        return;


    //  We do not merge anything into a drawing and we do not convert any views of a drawing
    //  except in the special case where it is an "orphan", that is, it is not shown on any sheet.
    if (SheetAttachmentMarker::IsFoundOn(v8model))
        return;

    // Convert the elements in the drawing's attachments, merging them into the drawing.
    auto tbm = ModelsToBeMerged::GetFrom(v8model);
    if (nullptr != tbm)
        {
        for (auto mergemm : tbm->m_models)
            {
            DoConvertDrawingElementsInModel(mergemm);
            if (WasAborted())
                return;
            }
        }

    //  Now that we know levels and styles ...

    //  Convert all views of this model
    DgnV8Api::ViewInfoPtr firstViewInfo;
    for (DgnV8Api::ViewGroupPtr const& vg : v8model.GetDgnFileP()->GetViewGroups())
        {
        DrawingViewFactory svf(v8mm);
        Convert2dViewsOf(firstViewInfo, *vg, v8model, svf); 
        }

    //  Now that we have a view ...

    //  Convert proxy graphics on attachments in the drawing
        {
        DrawingHoldsProxyGraphics intoDrawing(v8mm);
        ConvertExtractionAttachments(v8mm, intoDrawing, firstViewInfo.get());
        }

    v8model.Empty();
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
//      *content* of the attached models into the root drawing model later, during the element conversion phase, as follows:
//      2d attachment: 
//          We will copy the elements from all 2D attachments (recursively) into the root drawing model. 
//          While it's true that we will convert the same models in their own right, but the converted models will be unrelated to the 
//          graphics copied from the attachments to them.
//      3d attachment:
//          We will capture V8's view of the 3D attachment and all nested attachments as 2D proxy graphics and copy them into the root drawing model.
//
//  Root model = sheet. 
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


// "Why do sheet need to make copies of their attachments?"
// Some of the cases are explained above, for example, when a sheet needs a re-scaled copy of a drawing.
//
// Why in other cases? To avoid duplicate graphics in the case where a drawing has nested attachments of its own.
// When a drawing is attached to a sheet, the nested attachments are un-nested and become direct attachments of the sheet. 
// When a drawing is displayed in its own right in its own view, then its nested attachments must be merged into it.
// So, obviously, the bridge can apply those two different transformations to the same model. (Note that these transformations are done on the V8 models (in memory) before the conversion really starts.)
//
// There's another issue. Sheets always assert ownership over the drawings that are attached to them. By default such drawings are not considered to have
// an independent existence. So, the converter will not converter views of such drawings. If that is not what we want for a given drawing (or for all drawings),
// then we have to tell sheets to make their own private copies and assert ownership over them, so that the original drawings and their will be converted independently of the sheets.
//
// Finally, one more issue: If the root model is a drawing, then we must not fail to convert views of it. We cannot allow a sheet to assert private ownership over it.
// Therefore, sheets that attach a drawing that is the root model must attach a copy of it.
// 