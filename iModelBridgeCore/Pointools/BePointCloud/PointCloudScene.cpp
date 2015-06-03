/*--------------------------------------------------------------------------------------+
|
|     $Source: PointCloudScene.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <BePointCloudInternal.h>

USING_NAMESPACE_BENTLEY_BEPOINTCLOUD

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     1/2015
//----------------------------------------------------------------------------------------
PointCloudScenePtr PointCloudScene::Create(WCharCP fileName)
    {
    try
        {
        return new PointCloudScene(fileName);
        }
    catch (PointCloudSceneException)
        {
        return  nullptr;
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     1/2015
//----------------------------------------------------------------------------------------
PointCloudScene::PointCloudScene(WCharCP fileName) 
: m_frustumQueryHandle(NULL)
    {
    PThandle pcFileHandle = ptOpenPOD(fileName);
    if (pcFileHandle == 0)
        {
        PTres errorCode = ptGetLastErrorCode();
        throw(PointCloudSceneException(errorCode));
        }

    m_sceneHandle = PointCloudSceneHandle::Create(pcFileHandle);

    m_colorChannelHandle = 0;
    m_metadataHandle = 0;
    m_isPWfileInitialized = false;
    m_isPWFile            = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
PointCloudScene::~PointCloudScene ()
    {
    m_frustumQueryHandle = NULL;

// POINTCLOUD_WIP_GR06 - ModelViewportManager
//    ModelViewportManager::Get().Remove (this);

    m_metadataHandle = 0;
    m_colorChannelHandle = 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Daniel.McKenzie  06.2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool PointCloudScene::IsPwFile()
    {

    if (!m_isPWfileInitialized)
        {
        //Check if the file is a tempFile = PW file
        //TODO: This chunk of code should change in the future for a simple call to Pointool
        WString temp = GetResolvedPath();  
        char fileNameChar[1024];
        temp.ConvertToLocaleChars(&fileNameChar[0], _countof(fileNameChar));

        FILE* pFile = fopen(&fileNameChar[0], "rb");
        if (pFile != 0)
            {
            char FileIdentification[8];

            if (fread(FileIdentification, 1, 8, pFile) == 8 && strncmp(FileIdentification, "PTPODTMP", 8) == 0)
                m_isPWFile = true;    // it's a fake pod file
            fclose(pFile);
            }
        m_isPWfileInitialized = true;
        }
    
    return m_isPWFile;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WString PointCloudScene::GetSurveyGeoreferenceMetaTag() const
    {
    WString wktString;

    if (SUCCESS == GetMetaTag(WString(L"Survey.GeoReference"), wktString))
        return wktString;

    return WString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PointCloudScene::SetSurveyGeoreferenceMetaTag(WStringCR tagValue)
    {
    SetMetaTag(WString(L"Survey.GeoReference"), tagValue);
    WriteMetaTags();

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
PThandle PointCloudScene::GetMetadataHandle () const
    {
    if (m_metadataHandle == NULL)
        {
        m_metadataHandle = PointCloudMetaDataHandle::Create(ptGetMetaDataHandle (GetSceneHandle()));
        }

    return m_metadataHandle->GetHandle ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
PThandle PointCloudScene::GetSceneHandle () const
    {
    return m_sceneHandle->GetHandle();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
PThandle PointCloudScene::GetColorChannelHandle () const
    {
    if (m_colorChannelHandle == NULL)
        return 0;

    return m_colorChannelHandle->GetHandle();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudScene::GetRange (DRange3d& range, bool convertToUor) const
    {
    //  Get the Pointools range
    range.low.x  = range.low.y  = range.low.z  = DBL_MAX;
    range.high.x = range.high.y = range.high.z = -DBL_MAX;

    float lower [3], upper [3];
    ptSceneBounds (GetSceneHandle(), lower, upper);

    double multFactor = 1.0;
    if (convertToUor)
        multFactor = UOR_PER_METER;
        
    range.low.x  = lower [0] * multFactor;
    range.low.y  = lower [1] * multFactor;
    range.low.z  = lower [2] * multFactor;
    range.high.x = upper [0] * multFactor;
    range.high.y = upper [1] * multFactor;
    range.high.z = upper [2] * multFactor;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudScene::SetFrustumQueryHandle(PointCloudQueryHandleP handle)
    {
    m_frustumQueryHandle = handle;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
PointCloudQueryHandleP PointCloudScene::GetFrustumQueryHandle() const 
    {
    if (m_frustumQueryHandle.IsNull())
        {
        m_frustumQueryHandle = PointCloudQueryHandle::Create(ptCreateFrustumPointsQuery());
        ptSetQueryRGBMode (m_frustumQueryHandle->GetHandle(), PT_QUERY_RGB_MODE_SHADER); /*PointCloudCore::QUERY_RGB_MODE_ACTUAL, ;*/
        ptSetQueryScope(m_frustumQueryHandle->GetHandle(), GetSceneHandle());
        }

    return m_frustumQueryHandle.get(); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
PointCloudQueryHandleP PointCloudScene::GetVisiblePointsQueryHandle() const 
    {
    if (m_visiblePointsQueryHandle.IsNull())
        {
        m_visiblePointsQueryHandle = PointCloudQueryHandle::Create(ptCreateVisPointsQuery());
        ptSetQueryRGBMode (m_visiblePointsQueryHandle->GetHandle(), PT_QUERY_RGB_MODE_SHADER); /*PointCloudCore::QUERY_RGB_MODE_ACTUAL, ;*/
        ptSetQueryScope(m_visiblePointsQueryHandle->GetHandle(), GetSceneHandle());
        }

    return m_visiblePointsQueryHandle.get(); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP PointCloudScene::_GetFileName() const
    {
    return ptSceneFile(GetSceneHandle());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t PointCloudScene::_GetNumberOfPoints() const
    {
    int32_t numClouds;
    uint32_t numPoints, specification;
    bool loaded, visible;
    WChar name [256];

    if (PTV_SUCCESS == ptSceneInfo (GetSceneHandle(), name, numClouds, numPoints, specification, loaded, visible))
        return numPoints;
        
    return 0;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t PointCloudScene::_GetNumberOfClouds() const
    {
    int32_t numClouds;
    uint32_t numPoints, specification;
    bool loaded, visible;
    WChar name [256];

    if (PTV_SUCCESS == ptSceneInfo (GetSceneHandle(), name, numClouds, numPoints, specification, loaded, visible))
        return numClouds;

    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool PointCloudScene::_HasIntensityChannel() const
    {
    int32_t numClouds;
    uint32_t numPoints, specification;
    bool loaded, visible;
    WChar name [256];

    if (PTV_SUCCESS == ptSceneInfo (GetSceneHandle(), name, numClouds, numPoints, specification, loaded, visible))
        return TO_BOOL(specification & HAS_INTENSITY);
    
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool PointCloudScene::_HasClassificationChannel() const
    {
    int32_t numClouds;
    uint32_t numPoints, specification;
    bool loaded, visible;
    WChar name [256];
    
    if (PTV_SUCCESS == ptSceneInfo (GetSceneHandle(), name, numClouds, numPoints, specification, loaded, visible))
        return TO_BOOL(specification & HAS_CLASSIFICATION);
        
    return false;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool PointCloudScene::_HasRGBChannel() const
    {
    int32_t numClouds;
    uint32_t numPoints, specification;
    bool loaded, visible;
    WChar name [256];

    if (PTV_SUCCESS == ptSceneInfo (GetSceneHandle(), name, numClouds, numPoints, specification, loaded, visible))
        return TO_BOOL(specification & HAS_RGB);
    
    return false;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool PointCloudScene::_HasNormalChannel() const
    {
    int32_t numClouds;
    uint32_t numPoints, specification;
    bool loaded, visible;
    WChar name [256];

    if (PTV_SUCCESS == ptSceneInfo (GetSceneHandle(), name, numClouds, numPoints, specification, loaded, visible))
        return TO_BOOL(specification & HAS_NORMAL);

    return false;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   PointCloudScene::_GetMetaData (WStringR name, uint32_t& numClouds, uint64_t& numPoints, DPoint3dP lowerBound, DPoint3dP upperBound) const
    {
    wchar_t buffer [PT_MAX_META_STR_LEN];
    int32_t numberofClouds;
    uint32_t scene_spec;

    ptGetMetaData ( GetMetadataHandle (), buffer, numberofClouds, numPoints, scene_spec, (double *)lowerBound, (double *)upperBound );

    numClouds = numberofClouds;
    name = WString (buffer);

    return SUCCESS;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   PointCloudScene::_GetMetaTag(WStringCR tagName, WStringR value) const
    {
    wchar_t buffer[ PT_MAX_META_STR_LEN ];

    if (false == ptGetMetaTag ( GetMetadataHandle (),  tagName.GetWCharCP (), buffer))
        return ERROR;

    value =  WString(buffer);

    return SUCCESS;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t     PointCloudScene::_GetNumUserMetaSections() const
    {
    return ptNumUserMetaSections (GetMetadataHandle ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t     PointCloudScene::_GetNumUserMetaTagsInSection(int32_t sectionIndex) const
    {
    return ptNumUserMetaTagsInSection(GetMetadataHandle (), sectionIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   PointCloudScene::_GetUserMetaSectionName (int32_t sectionIndex, WStringR name) const
    {
    name = ptUserMetaSectionName(GetMetadataHandle (), sectionIndex);

    return SUCCESS;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   PointCloudScene::_GetUserMetaTagByIndex(int32_t sectionIndex, int32_t tagIndex, WStringR tagName, WStringR tagValue) const
    {
    wchar_t namebuffer[ 64 ];
    wchar_t valuebuffer[ PT_MAX_META_STR_LEN ];

    if (false == ptGetUserMetaTagByIndex(GetMetadataHandle (), sectionIndex, tagIndex,namebuffer, valuebuffer ))
        return ERROR;

    tagName = namebuffer;
    tagValue = valuebuffer;

    return SUCCESS;

    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   PointCloudScene::_GetUserMetaTagByName(WStringCR tagName, WStringR tagValue) const
    {
    wchar_t buffer[ PT_MAX_META_STR_LEN ];

    if (false == ptGetUserMetaTagByName(GetMetadataHandle (), tagName.GetWCharCP(), buffer))
        return ERROR;

    tagValue = buffer;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Daniel.McKenzie                   10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PointCloudScene::_SetMetaTag(WStringCR tagName, WStringCR tagValue)
    { 
    return ptSetMetaTag(GetMetadataHandle (),tagName.GetWCharCP(), tagValue.GetWCharCP()); 
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Daniel.McKenzie                   10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PointCloudScene::_WriteMetaTags()
    { 
    return ptWriteMetaTags(GetMetadataHandle ()); 
    }

