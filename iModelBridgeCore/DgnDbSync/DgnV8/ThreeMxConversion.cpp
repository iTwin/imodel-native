/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnV8/ThreeMxConversion.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ConverterInternal.h"

static bool isHttp(WCharCP str){return (0 == wcsncmp(L"http:", str, 5) || 0 == wcsncmp(L"https:", str, 6));}

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyApi::Utf8String attachNameFromMoniker(DgnV8Api::DgnDocumentMoniker const& moniker)
    {
    auto name = moniker.GetSavedFileName();
    if (isHttp(name.c_str()))
        return BentleyApi::Utf8String(name.c_str());

    StatusInt resolveStatus;
    BentleyApi::BeFileName fileName = BentleyApi::BeFileName(moniker.ResolveFileName(&resolveStatus).c_str());

    if (SUCCESS != resolveStatus)
        fileName = BentleyApi::BeFileName(moniker.GetSavedFileName().c_str());

    return BentleyApi::Utf8String(fileName);
    }

enum MRMeshMinorXAttributeId
{
    MRMeshMinorXAttributeId_PrimaryData = 0,
    MRMeshMinorXAttributeId_Clip = 1,
    MRMeshMinorXAttributeId_ClipElement = 2,
    MRMeshMinorXAttributeId_Link_Classifier = 3,
    MRMeshMinorXAttributeId_Link_ClassifierId = 4,
    MRMeshMinorXAttributeId_SecondaryData = 5,
};

enum    LinkStorageFlags
    {
    LinkStorage_ModelPEP              =  0x0001 << 0,
    LinkStorage_Name                  =  0x0001 << 1,
    LinkStorage_NamedGroupName        =  0x0001 << 2,
    LinkStorage_LevelName             =  0x0001 << 3,
    LinkStorage_ElementPEP            =  0x0001 << 4,
    };


enum ClassifierType
    {
    CLASSIFIER_TYPE_Model = 0,
    CLASSIFIER_TYPE_Element = 1,
    CLASSIFIER_TYPE_NamedGroup = 2,
    CLASSIFIER_TYPE_Level = 3,
    };

struct DgnV8ThreeMxClipFlags
{
    unsigned        m_isMask:1;
    unsigned        m_clipZLow:1;
    unsigned        m_clipZHigh:1;
    unsigned        m_reserved:29;
};

struct DgnV8ClassificationFlags
{
    unsigned    m_type:5;
    unsigned    m_outsideMode:5;
    unsigned    m_insideMode:5;
    unsigned    m_selectedMode:5;
    unsigned    m_unused:12;
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
ConvertToDgnDbElementExtension::Result ConvertThreeMxAttachment::_PreConvertElement(DgnV8EhCR v8el, Converter& converter, ResolvedModelMapping const& v8mm)
    {
    DgnV8Api::ElementHandle::XAttributeIter xai(v8el, DgnV8Api::XAttributeHandlerId(ThreeMxElementHandler::XATTRIBUTEID_ThreeMxAttachment, 0), 0);
    if (!xai.IsValid())
        return Result::SkipElement;

    Bentley::DataInternalizer source((byte*)xai.PeekData(), xai.GetSize());
    UInt32 version;
    source.get(&version);

    Transform location;
    for (size_t i=0; i<3; i++)
        {
        for (size_t j=0; j<4; j++)
            source.get(&location.form3d[i][j]);
        }

    location = Transform::FromProduct(v8mm.GetTransform(), location);
    
    ClipVectorPtr clipVector = ClipVector::Create();

    for (UInt32 index=0; true; index++)
        {
        DgnV8Api::ElementHandle::XAttributeIter xai(v8el, DgnV8Api::XAttributeHandlerId(ThreeMxElementHandler::XATTRIBUTEID_ThreeMxAttachment, (int) MRMeshMinorXAttributeId_Clip), index);

        if (!xai.IsValid())
            break;

        Bentley::DataInternalizer source((byte*) xai.PeekData(), xai.GetSize());

        DgnV8ThreeMxClipFlags flags;
        source.get((byte*) &flags, sizeof (flags));

        double zLow, zHigh;
        source.get(&zLow);
        source.get(&zHigh);

        Transform transformFromClip;
        for (int i=0; i<3; i++)
            {
            for (int j=0; j<4; j++)
                source.get(&transformFromClip.form3d[i][j]);
            }

        UInt32 nPoints;
        source.get(&nPoints);

        if  (nPoints > 0xffff)
            {
            BeAssert(false);
            continue;
            }

        bvector<DPoint2d> points;
        points.resize(nPoints);
        for (size_t i=0; i<nPoints; i++)
            {
            source.get(&points[i].x);
            source.get(&points[i].y);
            }

        clipVector->push_back(ClipPrimitive::CreateFromShape(&points.front(), points.size(), flags.m_isMask, flags.m_clipZLow ? &zLow : NULL, flags.m_clipZHigh ? &zHigh : NULL, &transformFromClip));
        }

    for (UInt32 index=0; true; index++)
        {
        DgnV8Api::ElementHandle::XAttributeIter xai(v8el, DgnV8Api::XAttributeHandlerId(ThreeMxElementHandler::XATTRIBUTEID_ThreeMxAttachment, (int)MRMeshMinorXAttributeId_ClipElement), index);

        if (!xai.IsValid())
            break;

        Bentley::DataInternalizer source((byte*) xai.PeekData(), xai.GetSize());
        DgnV8Api::PersistentElementPath pep;
        DgnV8Api::ElementHandle clipEh;
        Bentley::ClipVectorPtr thisClip;
        DgnV8ThreeMxClipFlags flags;

        source.get((byte*) &flags, sizeof (flags));
        
        if (SUCCESS == pep.Load(source) &&
            (clipEh = pep.EvaluateElementFromHost(v8el)).IsValid() &&
            (thisClip = DgnV8Api::ClipVector::CreateFromElement(clipEh, v8el.GetModelRef()->AsDgnModelP(), nullptr, flags.m_isMask ? DgnV8Api::ClipVolumePass::Outside : DgnV8Api::ClipVolumePass::Inside)).IsValid())
            {
            for (auto& clipPrimitive : *thisClip)
                {
                if (nullptr != clipPrimitive->GetClipPlanes())
                    clipVector->push_back(ClipPrimitive::CreateFromClipPlanes(reinterpret_cast<ClipPlaneSetCR>(*clipPrimitive->GetClipPlanes())));
                }
            }
        }

    // Classifiers.
    ModelSpatialClassifiers         classifiers;
    for (UInt32 index=0; true; index++)
        {
        DgnV8Api::ElementHandle::XAttributeIter xai(v8el, DgnV8Api::XAttributeHandlerId(ThreeMxElementHandler::XATTRIBUTEID_ThreeMxAttachment, (int)MRMeshMinorXAttributeId_Link_Classifier), index);

        if (!xai.IsValid())
            break;


        Bentley::DataInternalizer       source ((byte*) xai.PeekData(), xai.GetSize());
        DgnV8ClassificationFlags        v8Flags;
        double                          unused, expandDistance;
        UInt32                          optionalStorageFlags = 0;
        Bentley::WString                wName;
        DgnV8Api::PersistentElementPath pep;

        source.get ((byte*) &v8Flags, sizeof(v8Flags));
        source.get (&unused);
        source.get (&expandDistance);
        source.get (&optionalStorageFlags);
    
        if (0 == (optionalStorageFlags & LinkStorage_ModelPEP) ||
            SUCCESS != pep.Load (source))
            {
            BeAssert(false && "unable to load classifier PEP");
            break;
            }

        if (0 != (optionalStorageFlags & LinkStorage_Name))
            source.get (wName);

        switch (v8Flags.m_type)
            {
            case CLASSIFIER_TYPE_Model:
                {
                
                DgnV8Api::DgnModelRef*        prefix;
                DgnV8Api::DgnModelRef*        modelRef = nullptr;
                DgnV8Api::ElementHandle       refAttachEh;

                converter.GetAttachments(*v8el.GetModelRef());          // Force attachments to load.

                if (SUCCESS != pep.EvaluateReferenceAttachmentPrefix (prefix, v8el.GetModelRef()) ||
                    !(refAttachEh = pep.EvaluateReferenceAttachment(prefix)).IsValid() ||
                    NULL == (modelRef = prefix->FindDgnAttachmentByElementId (refAttachEh.GetElementId())) ||
                    NULL == modelRef->GetDgnModelP() ||
                    NULL == modelRef->AsDgnAttachmentCP())
                    {
                    BeAssert(false && "Unable to evaluate classifier attachment.");
                    break;
                    }

                auto    refTrans = converter.ComputeAttachmentTransform(v8mm.GetTransform(), *modelRef->AsDgnAttachmentCP());
                auto    modelMapping = converter.FindModelForDgnV8Model(*modelRef->GetDgnModelP(), refTrans);

                if (!modelMapping.IsValid())
                    {
                    BeAssert (false && "Unable to evaluate classifier model");
                    break;
                    }
                
                ModelSpatialClassifier::Flags   classifierFlags((ModelSpatialClassifier::Type) v8Flags.m_type, (ModelSpatialClassifier::Display) v8Flags.m_outsideMode, (ModelSpatialClassifier::Display) v8Flags.m_insideMode, (ModelSpatialClassifier::Display) v8Flags.m_selectedMode);
                classifiers.push_back(ModelSpatialClassifier(modelMapping.GetDgnModel().GetModelId(), classifierFlags, Utf8String(wName.c_str()), expandDistance));

                break;
                }

            default:
                BeAssert(false && "Unsupported classifier type");
                break;
            }
        }

    clipVector->TransformInPlace(v8mm.GetTransform());

    Bentley::WString monikerString;
    source.get(monikerString);

    DgnDocumentMonikerPtr moniker;
    if (0 == version)
        moniker = DgnV8Api::DgnDocumentMoniker::CreateFromFileName(monikerString.c_str());
    else
        moniker = DgnV8Api::DgnDocumentMoniker::Create(monikerString.c_str());

    moniker->SetParentSearchPath(v8el.GetDgnFileP()->GetFileName().c_str());

    Utf8String rootUrl = attachNameFromMoniker(*moniker);
    Utf8String linkName(BeFileName(rootUrl).GetFileNameWithoutExtension());

    if (true)
        {
        ThreeMx::Scene scene(converter.GetDgnDb(), location, rootUrl.c_str(), nullptr);
        if (SUCCESS == scene.ReadSceneFile())
            linkName = scene.GetSceneInfo().m_sceneName;
        }

    DgnDbR db = converter.GetDgnDb();
    RepositoryLinkPtr repositoryLink = RepositoryLink::Create(*db.GetRealityDataSourcesModel(), rootUrl.c_str(), linkName.c_str());
    if (!repositoryLink.IsValid() || !repositoryLink->Insert().IsValid())
        return Result::SkipElement;

    // In DgnV8, ThreeMx attachments are elements, and their visibility is determined by their level. In DgnDb they are DgnModels. 
    // For every spatial view in the DgnDb, determine whether the category of the element's original level is on, and if so
    // add the new ThreeMxModel to the list of viewed models.
    DgnModelId modelId = ThreeMx::ModelHandler::CreateModel(*repositoryLink, rootUrl.c_str(), &location, clipVector.get(), &classifiers);
    DgnCategoryId category = converter.GetSyncInfo().GetCategory(v8el, v8mm);

    for (auto const& entry : ViewDefinition::MakeIterator(db))
        {
        auto viewController = ViewDefinition::LoadViewController(entry.GetId(), db);
        if (!viewController.IsValid() || !viewController->IsSpatialView() || !viewController->GetViewDefinitionR().GetCategorySelector().IsCategoryViewed(category))
            continue;

        auto& modelSelector = viewController->ToSpatialViewP()->GetSpatialViewDefinition().GetModelSelector();
        modelSelector.AddModel(modelId);
        modelSelector.Update();
        }

    return Result::SkipElement;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ConvertThreeMxAttachment::Register()
    {
    ConvertThreeMxAttachment* instance = new ConvertThreeMxAttachment();
    RegisterExtension(ThreeMxElementHandler::GetInstance(), *instance);
    }


