/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnV8/PointCloudConversion.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ConverterInternal.h"

#include <PointCloud/PointCloudApi.h>
#include <PointCloud/PointCloudHandler.h>
#include <PointCloud/PointCloudRamps.h>
#include <PointCloud/PointCloudSettings.h>

USING_NAMESPACE_BENTLEY_POINTCLOUD

BEGIN_DGNDBSYNC_DGNV8_NAMESPACE

#define STATE_SIZE_V8               32    

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2016
//----------------------------------------------------------------------------------------
static void computeSceneToWorldFromV8props(TransformR sceneToWorld, DgnV8Api::PointCloudProperties const& props, DgnV8Api::DgnModel& model)
    {
    Bentley::DPoint3d origin = props.GetGlobalOrigin();
    double factor = props.GetUorPerMeter();

    Transform toUOR;
    toUOR.InitFromRowValues(factor, 0.0, 0.0, origin.x,
                            0.0, factor, 0.0, origin.y,
                            0.0, 0.0, factor, origin.z);

    Transform transform = (TransformCR)props.GetTransform();

    // DgnDb world is in meters so we must convert v8 uor to meter.
    DgnV8Api::ModelInfo const& v8ModelInfo = model.GetModelInfo();
    double uorPerMeter = DgnV8Api::ModelInfo::GetUorPerMeter(&v8ModelInfo);
    Transform toMeter;
    toMeter.InitFromScaleFactors(1.0 / uorPerMeter, 1.0 / uorPerMeter, 1.0 / uorPerMeter);
    
    sceneToWorld = Transform::FromProduct(toMeter, transform, toUOR);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  4/2016
//----------------------------------------------------------------------------------------
static void setPointCloudSymbology(PointCloud::PointCloudModel& pointCloudModel, DgnV8EhCR v8eh, SpatialConverterBase& converter)
    {
    // This is a very basic resolution of the effective color/weight. Do more as needed..

    // Color
    UInt32 v8Color = v8eh.GetElementCP()->hdr.dhdr.symb.color;
    if (DgnV8Api::COLOR_BYLEVEL == v8Color)
        {
        DgnV8Api::LevelCache& levelCache = (v8eh.GetModelRef() != NULL) ? v8eh.GetModelRef()->GetLevelCacheR() : v8eh.GetDgnModelP()->GetLevelCacheR();
        DgnV8Api::LevelHandle level = levelCache.GetLevel(v8eh.GetElementCP()->ehdr.level);
        if (level.IsValid())
            {
            DgnV8Api::LevelDefinitionColor byLevelColor = level.GetByLevelColor();
            DgnV8Api::IntColorDef intColorDef;
            if (SUCCESS == DgnV8Api::DgnColorMap::ExtractElementColorInfo(&intColorDef, NULL, NULL, NULL, NULL, byLevelColor.GetColor(), *byLevelColor.GetDefinitionFile()))
                pointCloudModel.SetColor(ColorDef(intColorDef.m_int));
            }
        }
    else
        {
        DgnV8Api::IntColorDef intColorDef;
        if (SUCCESS == DgnV8Api::DgnColorMap::ExtractElementColorInfo(&intColorDef, nullptr, nullptr, nullptr, nullptr, v8Color, *v8eh.GetDgnModelP()->GetDgnFileP()))
            pointCloudModel.SetColor(ColorDef(intColorDef.m_int));
        }

    // Weight
    UInt32 v8Weight = v8eh.GetElementCP()->hdr.dhdr.symb.weight;
    if (DgnV8Api::WEIGHT_BYLEVEL == v8Weight)
        {
        DgnV8Api::LevelCache& levelCache = (v8eh.GetModelRef() != NULL) ? v8eh.GetModelRef()->GetLevelCacheR() : v8eh.GetDgnModelP()->GetLevelCacheR();
        DgnV8Api::LevelHandle level = levelCache.GetLevel(v8eh.GetElementCP()->ehdr.level);
        if (level.IsValid())
            pointCloudModel.SetWeight(level.GetByLevelWeight());
        }
    else
        {
        pointCloudModel.SetWeight(v8Weight);
        }
    }

//-----------------------------------------------------------------------------------------
// Return true if sourceFile is newer than outputFile
// @bsimethod                                                   Eric.Paquet         10/2016
//-----------------------------------------------------------------------------------------
extern bool SourceFileIsNewer(BeFileNameCR sourceFileName, BeFileNameCR outputFileName); // Defined in RasterConversion.cpp

//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Eric.Paquet         10/2016
//-----------------------------------------------------------------------------------------
BentleyStatus CopyPointCloudToBimLocation(BeFileNameR outputFileName, DgnDbCR db, BeFileNameCR sourceFileName)
    {
    // Copy the point cloud at the same location than the Bim file
    BeFileName dbFileName(db.GetDbFileName());
    WString fileName = sourceFileName.GetFileNameAndExtension();
    outputFileName = dbFileName.GetDirectoryName();
    outputFileName.AppendToPath(fileName.c_str());
    if (outputFileName.DoesPathExist() && !SourceFileIsNewer(sourceFileName, outputFileName))
        {
        // The output file already exists and it is newer (or equal time). No need to copy.
        return SUCCESS;
        }

    // Don't try to copy file if source == destination
    if ((0 != BeStringUtilities::Wcsicmp(sourceFileName.c_str(), outputFileName.c_str())))
        {
        BeFileNameStatus fileStatus = BeFileName::BeCopyFile(sourceFileName, outputFileName);
        if (fileStatus != BeFileNameStatus::Success && fileStatus != BeFileNameStatus::AlreadyExists)
            {
            return ERROR;
            }
        }
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
//! Create a PointCloud model into the dgnDb.
// @bsimethod                                                   Eric.Paquet         1/2016
//-----------------------------------------------------------------------------------------
PointCloud::PointCloudModelPtr CreatePointCloudModel(Utf8StringCR fileName, DgnDbR db)
    {
    Utf8String fileUri;
    if (T_HOST.GetPointCloudAdmin()._CreateFileUri(fileUri, fileName) == SUCCESS)
        {
        RepositoryLinkPtr repositoryLink = RepositoryLink::Create(*db.GetRealityDataSourcesModel(), fileUri.c_str(), fileUri.c_str());
        if (!repositoryLink.IsValid() || !repositoryLink->Insert().IsValid())
            return nullptr;

        return PointCloud::PointCloudModelHandler::CreatePointCloudModel(PointCloud::PointCloudModel::CreateParams(db, repositoryLink->GetElementId(), fileUri));
        }
    
    return nullptr;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Eric.Paquet         3/2016
//-----------------------------------------------------------------------------------------
BentleyStatus SpatialConverterBase::_ConvertPointCloudElement(DgnV8EhCR v8eh, ResolvedModelMapping const& v8mm, bool copyPointCloud, bool isNewElement)
    {
    //WIP_RASTER_CONVERTER need to think about 
    //  - output param.  we probably should return the created model. ElementConversionResults is not a good fit.

    bool importPointCloudAttachments = m_config.GetXPathBool("/ImportConfig/PointCloud/@importAttachments", false);
    if (!importPointCloudAttachments)
        {
        // Point cloud import is disabled in the configuration file
        return SUCCESS;
        }

    DgnElementId existingId;
    IChangeDetector::SearchResults changeInfo;
    if (GetChangeDetector()._IsElementChanged(changeInfo, *this, v8eh, v8mm) && IChangeDetector::ChangeType::Update == changeInfo.m_changeType)
        {
        existingId = changeInfo.GetExistingElementId();
        }

    DgnV8Api::IPointCloudQuery* pQuery = dynamic_cast<DgnV8Api::IPointCloudQuery*>(&v8eh.GetHandler());
    if (nullptr == pQuery)
        return ERROR;
    DgnV8Api::PointCloudPropertiesPtr propsP = pQuery->GetPointCloudProperties(v8eh);
    
    DgnDocumentMonikerPtr fileMonikerPtr = DgnV8Api::DgnDocumentMoniker::Clone(propsP->GetFileMoniker());

    // Set a search path if there is none.
    if (fileMonikerPtr->GetParentSearchPath().empty())
        fileMonikerPtr->SetParentSearchPath(DgnV8Api::PointCloudHandler::GetSearchPath(v8eh.GetDgnModelP()).c_str());

    // Resolve local path using moniker search logic. 
    BeFileName filename(fileMonikerPtr->ResolveFileName().c_str());
    if (filename.empty())
        {
        // Could not resolve file path. Do not import.
        Utf8String filenameU(fileMonikerPtr->GetPortableName().c_str());
        ReportIssueV(Converter::IssueSeverity::Warning, Converter::IssueCategory::MissingData(), Converter::Issue::PointCloudFile(), nullptr, filenameU.c_str());
        return ERROR;
        }

    // Write progress information to output window
    SetTaskName (Converter::ProgressMessage::TASK_CONVERTING_POINTCLOUD(), Utf8String(filename).c_str());

    // Copy point cloud to the Bim's location.
    if (copyPointCloud)
        {
        BeFileName outputFileName;
        if (SUCCESS != CopyPointCloudToBimLocation(outputFileName, GetDgnDb(), filename))
            {
            ReportError(Converter::IssueCategory::Unknown(), Converter::Issue::PointCloudCreationError(), outputFileName.GetNameUtf8().c_str());
            return ERROR;
            }
        }
    
    PointCloud::PointCloudModelPtr pPointCloudModel;
    
    if (!existingId.IsValid())
        pPointCloudModel = CreatePointCloudModel(filename.GetNameUtf8(), GetDgnDb());
    else
        pPointCloudModel = GetDgnDb().Models().Get<PointCloud::PointCloudModel>(DgnModelId(existingId.GetValue()));

    if (pPointCloudModel.IsNull())
        {
        ReportError(Converter::IssueCategory::Unknown(), Converter::Issue::ConvertFailure(), Utf8String(filename.c_str()).c_str());
        return ERROR;
        }

    // Set model properties
    Utf8String description(propsP->GetDescription().c_str());
    pPointCloudModel->SetDescription(description.c_str());

    if (propsP->HasSpatialReferenceWkt())
        {
        Utf8String wkt(propsP->GetSpatialReferenceWkt().c_str());
        pPointCloudModel->SetSpatialReferenceWkt(wkt.c_str());
        }
    else
        {
        pPointCloudModel->SetSpatialReferenceWkt("");
        }

    pPointCloudModel->SetViewDensity(propsP->GetViewDensity());

    Transform sceneToWorld;
    computeSceneToWorldFromV8props(sceneToWorld, *propsP, *v8eh.GetDgnModelP());
    pPointCloudModel->SetSceneToWorld(sceneToWorld);

    // Extract color and weight from element symbology.
    setPointCloudSymbology(*pPointCloudModel, v8eh, *this);
    
    if (!existingId.IsValid())
        {
        auto modelStatus = pPointCloudModel->Insert();
        if (modelStatus != DgnDbStatus::Success)
            {
            BeAssert((DgnDbStatus::LockNotHeld != modelStatus) && "Failed to get or retain necessary locks");
            ReportError(Converter::IssueCategory::Unknown(), Converter::Issue::ConvertFailure(), Utf8String(filename.c_str()).c_str());
            return ERROR;
            }

        _GetChangeDetector()._OnElementSeen(*this, pPointCloudModel->GetModeledElementId());
        }
    else
        {
        DgnDbStatus updateStatus;
        if (DgnDbStatus::Success != (updateStatus = pPointCloudModel->Update()))
            {
            BeAssert((DgnDbStatus::LockNotHeld != updateStatus) && "Failed to get or retain necessary locks");
            ReportError(Converter::IssueCategory::Unknown(), Converter::Issue::ConvertFailure(), filename.GetNameUtf8().c_str());
            return ERROR;
            }

        _OnElementConverted(existingId, &v8eh, Converter::ChangeOperation::Update);
        }

    WriteV8ElementExternalSourceAspect(pPointCloudModel->GetModeledElementId(), v8eh, GetDgnDb().GetRealityDataSourcesModel()->GetModelId());

    // Schedule reality model tileset creation.
    AddModelRequiringRealityTiles(pPointCloudModel->GetModelId(), filename.GetNameUtf8(), GetRepositoryLinkId(*v8eh.GetDgnFileP()));

        
    // Display the point cloud (or not) in the DgnDb views
    DgnCategoryId category = GetSyncInfo().GetCategory(v8eh, v8mm);
    for (auto const& entry : ViewDefinition::MakeIterator(GetDgnDb()))
        {
        auto viewNumberItr = m_viewNumberMap.find(entry.GetId());
        if (viewNumberItr == m_viewNumberMap.end() || !propsP->GetViewState(viewNumberItr->second))
            continue;

        auto viewController = ViewDefinition::LoadViewController(entry.GetId(), GetDgnDb());
        if (!viewController.IsValid() || !viewController->IsSpatialView() || !viewController->GetViewDefinitionR().GetCategorySelector().IsCategoryViewed(category))
            continue;

        auto& models = viewController->ToSpatialViewP()->GetSpatialViewDefinition().GetModelSelector();
        models.AddModel(pPointCloudModel->GetModelId());
        models.Update();
        }

    return SUCCESS;
    }

//=========================================================================================
// @bsiclass                                                    Eric.Paquet         4/2016
//=========================================================================================
struct V8ViewSettingsData
    {
    uint32_t    m_flags;
    float       m_contrast;
    float       m_brightness;
    float       m_dist;
    float       m_off;
    int         m_adaptivePointSize;
    uint16_t    m_intensityRampIdx;
    uint16_t    m_planeRampIdx;
    uint16_t    m_planeAxis; //{x,y,z} index

    Dgn::PointCloudViewSettings::DisplayStyle    m_displayStyle;

    Bentley::WString    m_planeRamp;
    Bentley::WString    m_intensityRamp;
    bool                m_useACSAsPlaneAxis;

    //Advanced Settings from SS4
    bool                m_clampIntensity;
    bool                m_NeedClassifBuffer;
    Bentley::WString    m_DisplayStyleName;
    int                 m_dsIdx;

    //-----------------------------------------------------------------------------------------
    // Constructor
    // @bsimethod                                                   Eric.Paquet         4/2016
    //-----------------------------------------------------------------------------------------
    V8ViewSettingsData()
        {
        // init with default values
        m_flags = Dgn::PointCloudViewSettings::VIEWSETTINGS_FRONTBIAS_MASK;
        m_contrast = Dgn::PointCloudViewSettings::GetDefaultViewContrast();
        m_brightness = Dgn::PointCloudViewSettings::GetDefaultViewBrightness();
        m_dist = 10.0f;
        m_off = 0;
        m_adaptivePointSize = 0;
        m_intensityRampIdx = 0;
        m_planeRampIdx = 1;
        m_planeAxis = 2;
        m_useACSAsPlaneAxis = false;
        m_displayStyle = Dgn::PointCloudViewSettings::DisplayStyle::None;
        m_planeRamp     = L"";
        m_intensityRamp = L"";

        //Advanced Settings for SS4
        m_clampIntensity = false;
        m_NeedClassifBuffer = false;
        m_DisplayStyleName = L"";
        m_dsIdx = -1; //Set it to -1 it means display style none
        }

    //-----------------------------------------------------------------------------------------
    // @bsimethod                                                   Eric.Paquet         4/2016
    //-----------------------------------------------------------------------------------------
    void Init(DataInternalizer& dataInternalizer)
        {
        dataInternalizer.get(&m_flags);
        dataInternalizer.get(&m_contrast);
        dataInternalizer.get(&m_brightness);
        dataInternalizer.get(&m_dist);
        dataInternalizer.get(&m_off);
        dataInternalizer.get(&m_adaptivePointSize);
        dataInternalizer.get(&m_intensityRampIdx);
        dataInternalizer.get(&m_planeRampIdx);
        dataInternalizer.get(&m_planeAxis);

        uint16_t displayStyle;
        dataInternalizer.get(&displayStyle);
        m_displayStyle = (Dgn::PointCloudViewSettings::DisplayStyle)displayStyle;

        // Point cloud beta don't have ramp names.
        WString rampName;
        if (!dataInternalizer.AtOrBeyondEOS())
            {
            dataInternalizer.get(m_planeRamp);
            dataInternalizer.get(m_intensityRamp);

            //TR #346761 if the file is corrupted - get the good ramp for the index already set
            if (PointCloudRamps::GetInstance().GetPlaneRampIndex(WString(m_planeRamp.c_str())) == INVALID_RAMP_INDEX)
                {
                rampName = PointCloudRamps::GetInstance().GetPlaneRampName(m_planeRampIdx);
                m_planeRamp = Bentley::WString(rampName.c_str());
                }
            if (PointCloudRamps::GetInstance().GetIntensityRampIndex(WString(m_intensityRamp.c_str())) == INVALID_RAMP_INDEX)
                {
                rampName = PointCloudRamps::GetInstance().GetIntensityRampName(m_intensityRampIdx);
                m_intensityRamp = Bentley::WString(rampName.c_str());
                }
            }
        else
            {
            // This could only happen if created with the first beta.
            rampName = PointCloudRamps::GetInstance().GetPlaneRampName(m_planeRampIdx);
            m_planeRamp = Bentley::WString(rampName.c_str());
            rampName = PointCloudRamps::GetInstance().GetIntensityRampName(m_intensityRampIdx);
            m_intensityRamp = Bentley::WString(rampName.c_str());
            }

        BeAssert(PointCloudRamps::GetInstance().GetPlaneRampIndex(WString(m_planeRamp.c_str())) != INVALID_RAMP_INDEX);
        BeAssert(PointCloudRamps::GetInstance().GetIntensityRampIndex(WString(m_intensityRamp.c_str())) != INVALID_RAMP_INDEX);

        // Now we have valid ramp names. Set the index from it.
        m_planeRampIdx = PointCloudRamps::GetInstance().GetPlaneRampIndex(WString(m_planeRamp.c_str()));
        m_intensityRampIdx = PointCloudRamps::GetInstance().GetIntensityRampIndex(WString(m_intensityRamp.c_str()));

        if (!dataInternalizer.AtOrBeyondEOS())
            {
            int val;
            dataInternalizer.get(&val);
            m_useACSAsPlaneAxis = val ? true : false;
            }
        else
            m_useACSAsPlaneAxis = false;

        //Advanced Settings from SS4
        if (!dataInternalizer.AtOrBeyondEOS())
            {
            int32_t val;
            dataInternalizer.get(&val);
            m_clampIntensity = val ? true : false;

            dataInternalizer.get(&val);
            m_NeedClassifBuffer = val ? true : false;
        
            dataInternalizer.get(m_DisplayStyleName);
        
            dataInternalizer.get(&m_dsIdx);
            }
        else
            {
            m_clampIntensity = false;
            m_NeedClassifBuffer = false;
            m_DisplayStyleName = L"";
            m_dsIdx = -1;
            }     
        }
    };

//=========================================================================================
// @bsiclass                                                    Eric.Paquet         4/2016
//=========================================================================================
struct V8PointCloudColorDef
    {
    Byte    m_red;
    Byte    m_green;
    Byte    m_blue;
    };

//=========================================================================================
// This is the template of the struct from SS3. In SS3, this struct was smaller than the
// one in subsequent versions.
// @bsiclass                                                    Eric.Paquet         4/2016
//=========================================================================================
struct V8LasClassificationInfoSS3
    {
    uint32_t                m_visibleState;
    V8PointCloudColorDef    m_classificationColor[CLASSIFICATION_COUNT];

    //-----------------------------------------------------------------------------------------
    // @bsimethod                                                   Eric.Paquet         4/2016
    //-----------------------------------------------------------------------------------------
    uint32_t GetClassificationStates() const 
        { 
        return m_visibleState; 
        }

    //-----------------------------------------------------------------------------------------
    // @bsimethod                                                   Eric.Paquet         4/2016
    //-----------------------------------------------------------------------------------------
    V8PointCloudColorDef const * GetClassificationColors() const 
        {
        return m_classificationColor; 
        }

    };

//=========================================================================================
// @bsiclass                                                    Eric.Paquet         4/2016
//=========================================================================================
struct V8LasClassificationInfo
    {
    uint32_t                m_visibleState;
    V8PointCloudColorDef    m_classificationColor[CLASSIFICATION_COUNT];
    bool                    m_blendColor;
    bool                    m_useBasecolor;
    unsigned char           m_visibleStateAdv[STATE_SIZE_V8];
    unsigned char           m_activeStateAdv[STATE_SIZE_V8];
    V8PointCloudColorDef    m_unclassColor;
    bool                    m_unclassVisible;
    };

//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Eric.Paquet         4/2016
//-----------------------------------------------------------------------------------------
static void convertV8ClassifColors(Dgn::PointCloudClassificationSettings& pcClassifViewSettings, V8PointCloudColorDef const * v8ClassifColors)
    {
    for (uint32_t i = 0; i < CLASSIFICATION_COUNT; i++)
        {
        ColorDef col = ColorDef(v8ClassifColors[i].m_red, v8ClassifColors[i].m_green, v8ClassifColors[i].m_blue);
        pcClassifViewSettings.SetClassificationColor(col, i);
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Eric.Paquet         4/2016
//-----------------------------------------------------------------------------------------
static void convertV8VisibleStates(Dgn::PointCloudClassificationSettings& pcClassifViewSettings, const unsigned char visibleStates[STATE_SIZE_V8])
    {
    uint32_t    arrayPosition;
    uint32_t    positionBitField;
    bool        visibleState;

    for (uint32_t i = 0; i < CLASSIFICATION_STATES; i++)
        {
        arrayPosition = i / 8;
        positionBitField = i % 8;
        visibleState = TO_BOOL(visibleStates[arrayPosition] & 0x00000001 << positionBitField);
        pcClassifViewSettings.SetVisibleState(visibleState, i);
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Eric.Paquet         4/2016
//-----------------------------------------------------------------------------------------
static void convertV8ActiveStates(Dgn::PointCloudClassificationSettings& pcClassifViewSettings, const unsigned char activeStates[STATE_SIZE_V8])
    {
    uint32_t    arrayPosition;
    uint32_t    positionBitField;
    bool        activeState;

    for (uint32_t i = 0; i < CLASSIFICATION_STATES; i++)
        {
        arrayPosition = i / 8;
        positionBitField = i % 8;
        activeState = TO_BOOL(activeStates[arrayPosition] & 0x00000001 << positionBitField);
        pcClassifViewSettings.SetActiveState(activeState, i);
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Eric.Paquet         4/2016
//-----------------------------------------------------------------------------------------
static void convertV8PointCloudClassificationViewSettings(SpatialViewControllerR spatial, DgnV8ViewInfoCR viewInfo)
    {
    //int displayStyleIndex = viewInfo.GetDynamicViewSettings ().GetPointCloudDisplayStyleIndex();
    Bentley::DgnPlatform::XAttributeHandlerId handlerClassifId(DgnV8Api::XATTRIBUTEID_PointCloudHandler, DgnV8Api::PointCloudMinorId_ClassificationViewSettings);
    int dataSize = 0;
    //void const* savedClassifData = viewInfo.GetDynamicViewSettings ().GetXAttributesHolderCR().GetXAttribute (&dataSize, handlerClassifId, 0);
    V8LasClassificationInfo const* v8SavedClassifData = reinterpret_cast <V8LasClassificationInfo const*> (viewInfo.GetDynamicViewSettings().GetXAttributesHolderCR().GetXAttribute(&dataSize, handlerClassifId, 0));

    if (nullptr == v8SavedClassifData)
        return;

    PointCloudClassificationSettings& pcClassifViewSettings = PointCloudClassificationSettings::FromView(spatial);
    if (dataSize == sizeof (V8LasClassificationInfoSS3))
        {
        // In SS3, the V8LasClassificationInfoSS3 structure is smaller than the versions > SS3.
        // So the XAttribute may contain classification data with the size of the SS3 structure.
        // We process this case here. We initialize the V8LasClassificationInfo struct with default values and only copy
        // significant data from the SS3 struct (That's how it is processed in MicroStation as well).

        V8LasClassificationInfoSS3 const* v8SavedClassifDataSS3 = reinterpret_cast <V8LasClassificationInfoSS3 const*> (v8SavedClassifData);

        // Convert classification colors
        convertV8ClassifColors(pcClassifViewSettings, v8SavedClassifDataSS3->GetClassificationColors());

        // Convert SS3 visible states (32 states) to DgnDb (256 states)

        // 1 bit for each state (256 states)
        unsigned char   tempState[STATE_SIZE_V8];

        // Set all 256 states to visible
        memset(&tempState[0], 0xFFFFFFFF, STATE_SIZE_V8 * sizeof(unsigned char));

        //Get the states from SS3 and copy them into the settings in SS4 (SS3 only has 32 states)
        uint32_t temp = v8SavedClassifDataSS3->GetClassificationStates();
        memcpy(tempState, &temp, sizeof (uint32_t));
        convertV8VisibleStates(pcClassifViewSettings, tempState);
        return;
        }

    // Process the structure version > SS3

    // Convert classification colors
    convertV8ClassifColors(pcClassifViewSettings, v8SavedClassifData->m_classificationColor);

    // Convert visible states
    // First, copy m_visibleStateAdv structure
    unsigned char   tempState[STATE_SIZE_V8];
    memcpy(tempState, v8SavedClassifData->m_visibleStateAdv, STATE_SIZE_V8);

    // 2nd, copy m_visibleState, which overrides first 4 bytes of m_visibleStateAdv
    uint32_t temp = v8SavedClassifData->m_visibleState;
    memcpy(tempState, &temp, sizeof (uint32_t));
    convertV8VisibleStates(pcClassifViewSettings, tempState);

    pcClassifViewSettings.SetBlendColor(v8SavedClassifData->m_blendColor);
    pcClassifViewSettings.SetUseBaseColor(v8SavedClassifData->m_useBasecolor);
    convertV8ActiveStates(pcClassifViewSettings, v8SavedClassifData->m_activeStateAdv);
    ColorDef col = ColorDef(v8SavedClassifData->m_unclassColor.m_red, v8SavedClassifData->m_unclassColor.m_green, v8SavedClassifData->m_unclassColor.m_blue);
    pcClassifViewSettings.SetUnclassColor(col);
    pcClassifViewSettings.SetUnclassVisible(v8SavedClassifData->m_unclassVisible);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Eric.Paquet         4/2016
//-----------------------------------------------------------------------------------------
void SpatialConverterBase::ConvertV8PointCloudViewSettings(SpatialViewControllerR spatial, DgnV8ViewInfoCR viewInfo)
    {
    // Read V8 Point Cloud settings
    DgnV8Api::XAttributeHandlerId handlerId(DgnV8Api::XATTRIBUTEID_PointCloudHandler, DgnV8Api::PointCloudMinorId_ViewSettings);

    int dataSize = 0;
    void const* savedData = viewInfo.GetDynamicViewSettings().GetXAttributesHolderCR().GetXAttribute(&dataSize, handlerId, 0);
    if (nullptr != savedData)
        {
        DataInternalizer dataInternalizer((byte*)savedData, dataSize);
        V8ViewSettingsData v8ViewSettingsData;
        v8ViewSettingsData.Init(dataInternalizer);

        PointCloudViewSettings& pcViewSettings = PointCloudViewSettings::FromView(spatial);
        pcViewSettings.SetFlags(v8ViewSettingsData.m_flags);
        pcViewSettings.SetContrast(v8ViewSettingsData.m_contrast);
        pcViewSettings.SetBrightness(v8ViewSettingsData.m_brightness);
        pcViewSettings.SetDistance(v8ViewSettingsData.m_dist);
        pcViewSettings.SetOffset(v8ViewSettingsData.m_off);
        pcViewSettings.SetAdaptivePointSize(v8ViewSettingsData.m_adaptivePointSize);
        pcViewSettings.SetIntensityRampIdx(v8ViewSettingsData.m_intensityRampIdx);
        pcViewSettings.SetPlaneRampIdx(v8ViewSettingsData.m_planeRampIdx);
        pcViewSettings.SetPlaneAxis(v8ViewSettingsData.m_planeAxis);
        pcViewSettings.SetDisplayStyle(v8ViewSettingsData.m_displayStyle);
        pcViewSettings.SetPlaneRamp(Utf8String(v8ViewSettingsData.m_planeRamp.c_str()));
        pcViewSettings.SetIntensityRamp(Utf8String(v8ViewSettingsData.m_intensityRamp.c_str()));
        pcViewSettings.SetUseACSAsPlaneAxis(v8ViewSettingsData.m_useACSAsPlaneAxis);
        pcViewSettings.SetClampIntensity(v8ViewSettingsData.m_clampIntensity);
        pcViewSettings.SetNeedClassifBuffer(v8ViewSettingsData.m_NeedClassifBuffer);
        pcViewSettings.SetDisplayStyleName(Utf8String(v8ViewSettingsData.m_DisplayStyleName.c_str()));
        pcViewSettings.SetDisplayStyleIndex(v8ViewSettingsData.m_dsIdx);
        }

    convertV8PointCloudClassificationViewSettings(spatial, viewInfo);
    }

END_DGNDBSYNC_DGNV8_NAMESPACE
