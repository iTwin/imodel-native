/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/BePointCloud/PointCloudScene.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__

// POINTCLOUD_WIP_GR06 - When ExternalModelHandler will be implemented, maybe PointCloudScene.h should be private (not in PublicApi)
//                       and have no exported methods, of course. Reevealuate when we have ExternalModelHandler.

#include <BePointCloud/IPointCloudFileEdit.h>

BEPOINTCLOUD_TYPEDEFS(PointCloudScene);

BEGIN_BENTLEY_BEPOINTCLOUD_NAMESPACE

//forward declarations
/* POINTCLOUD_WIP_GR06 - SceneHandleDescr
struct SceneHandleDescr;
*/

struct PointCloudManager;

struct PointCloudScene;
typedef RefCountedPtr<PointCloudScene>       PointCloudScenePtr;

enum PtPointAttributeFlags
    {
    HAS_INTENSITY       = 0x01,
    HAS_RGB             = 0x02,
    HAS_NORMAL          = 0x04,
    HAS_FILTER          = 0x08,
    HAS_CLASSIFICATION  = 0x10
    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct PointCloudScene : public RefCounted<IPointCloudFileEdit>
    {
    friend PointCloudManager;

    public:
        BEPOINTCLOUD_EXPORT     static  PointCloudScenePtr   Create(WCharCP fileName);
        BEPOINTCLOUD_EXPORT     virtual ~PointCloudScene ();

        BEPOINTCLOUD_EXPORT     PThandle                GetMetadataHandle () const;
        BEPOINTCLOUD_EXPORT     PThandle                GetSceneHandle () const;
        BEPOINTCLOUD_EXPORT     PThandle                GetColorChannelHandle() const;
        BEPOINTCLOUD_EXPORT     void                    GetRange (DRange3d& range) const;

        BEPOINTCLOUD_EXPORT     void                    SetFrustumQueryHandle(PointCloudQueryHandleP handle);  
        BEPOINTCLOUD_EXPORT     PointCloudQueryHandleP  GetFrustumQueryHandle() const;
        BEPOINTCLOUD_EXPORT     PointCloudQueryHandleP  PeekFrustumQueryHandle() const { return m_frustumQueryHandle.get(); }

        BEPOINTCLOUD_EXPORT     PointCloudQueryHandleP  GetVisiblePointsQueryHandle() const;
        BEPOINTCLOUD_EXPORT     PointCloudQueryHandleP  PeekVisiblePointsQueryHandle() const { return m_visiblePointsQueryHandle.get(); }

/* POINTCLOUD_WIP_GR06 - SceneHandleDescr
        RefCountedPtr<SceneHandleDescr> GetSceneHandleDescr() const;
*/
        BEPOINTCLOUD_EXPORT     WString                 GetResolvedPath() const        { return m_resolvedPath; }
        
        BEPOINTCLOUD_EXPORT     bool                    IsPwFile();
        BEPOINTCLOUD_EXPORT     WString                 GetSurveyGeoreferenceMetaTag() const;
        BEPOINTCLOUD_EXPORT     StatusInt               SetSurveyGeoreferenceMetaTag(WStringCR tagValue);

        // IPointCloudFileQuery
        virtual WCharCP     _GetFileName() const override;
        virtual uint64_t    _GetNumberOfPoints() const override;
        virtual uint32_t    _GetNumberOfClouds() const override;
        virtual bool        _HasIntensityChannel() const override;
        virtual bool        _HasClassificationChannel() const override;
        virtual bool        _HasRGBChannel() const override;
        virtual bool        _HasNormalChannel() const override;
        virtual StatusInt   _GetMetaTag(WStringCR tagName, WStringR value) const  override;
        virtual uint32_t    _GetNumUserMetaSections() const  override;
        virtual StatusInt   _GetMetaData (WStringR name, uint32_t& numClouds, uint64_t& numPoints, DPoint3dP lowerBound, DPoint3dP upperBound) const  override;
        virtual uint32_t    _GetNumUserMetaTagsInSection(int32_t sectionIndex) const override;
        virtual StatusInt   _GetUserMetaSectionName (int32_t sectionIndex, WStringR name) const override;
        virtual StatusInt   _GetUserMetaTagByIndex(int32_t sectionIndex, int32_t tagIndex, WStringR tagName, WStringR tagValue) const override;
        virtual StatusInt   _GetUserMetaTagByName(WStringCR tagName, WStringR tagValue) const override;

        //IPointCloudFileEdit
        virtual StatusInt   _SetMetaTag(WStringCR tagName, WStringCR tagValue) override;
        virtual StatusInt   _WriteMetaTags() override;

    private:
        PointCloudScene(WCharCP fileName) ;

        // runtime information
        PointCloudChannelHandlePtr          m_colorChannelHandle;
        mutable PointCloudMetaDataHandlePtr m_metadataHandle;
        WString                                 m_resolvedPath;
/* POINTCLOUD_WIP_GR06 - SceneHandleDescr
        RefCountedPtr<SceneHandleDescr>         m_pSceneDscr;
*/
        mutable PointCloudSceneHandlePtr        m_sceneHandle;
        mutable PointCloudQueryHandlePtr        m_frustumQueryHandle;
        mutable PointCloudQueryHandlePtr        m_visiblePointsQueryHandle;
        bool                                    m_isPWFile;
        bool                                    m_isPWfileInitialized;
    };

//========================================================================================
// @bsiclass                                                        Eric.Paquet     1/2015
//========================================================================================
struct PointCloudSceneException
    {
    PTres       m_errorCode;
    PointCloudSceneException (PTres errorCode){m_errorCode = errorCode;}
    };

END_BENTLEY_BEPOINTCLOUD_NAMESPACE
