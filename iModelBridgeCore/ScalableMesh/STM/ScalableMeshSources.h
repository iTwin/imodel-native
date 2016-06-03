/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshSources.h $
|    $RCSfile: ScalableMeshSources.h,v $
|   $Revision: 1.16 $
|       $Date: 2011/10/31 15:45:08 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <GeoCoord/BaseGeoCoord.h>
#include <ScalableMesh/IScalableMeshCreator.h>

#include <ScalableMesh/IScalableMeshSourceCollection.h>
#include <ScalableMesh/IScalableMeshSourceImportConfig.h>

#include "ScalableMeshEditListener.h"

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

struct SourceImportConfig;
typedef RefCountedPtr<SourceImportConfig>   SourceImportConfigCPtr;





/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct IDTMSource::Impl : public EditListener
    {
private:
    friend struct                       IDTMSource;
  
    EditListener*                       m_editListenerP;
    DTMSourceDataType                   m_sourceDataType;        
    Time                                m_lastModified;   


// TDORAY: Add a last modified check time
    SourceImportConfig                  m_config;

    virtual void                        _NotifyOfPublicEdit            () override;
    virtual void                        _NotifyOfLastEditUpdate        (Time                    updatedLastEditTime) override;

    virtual bool                        _IsReachable                   () const;

    virtual StatusInt                   _UpdateLastModified            () = 0;



protected:
    WString m_path;
    explicit                            Impl                           (DTMSourceDataType       sourceDataType, 
                                                                        const IMoniker*         monikerP);

                                Impl(DTMSourceDataType       sourceDataType,
                                             WCharCP         fullPath);

                                        Impl                           (const Impl&             rhs);
public:
    virtual                             ~Impl                          () = 0;        

    // Uses default copy behavior

    /*const IMoniker&                     GetMoniker                     () const { return *m_monikerPtr; }
    bool                                HasMoniker                     () const { return 0 != m_monikerPtr.get(); }*/

    WString                             GetPath                        () const { return m_path; }


    Time                                GetLastModified                () const;
    Time                                GetLastModifiedCheckTime       () const;

    void                                SetLastModified                (const Time&                 time);

    void                                ResetLastModified              ();

    void                                OnPublicEdit                   ();

    bool                                IsPartOfCollection             () const { return 0 != m_editListenerP; }

    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct IDTMLocalFileSource::Impl : public IDTMSource::Impl 
    {   
private:
    friend struct                       IDTMLocalFileSource;
    friend struct                       IDTMLocalFileSourceCreator;

    mutable WString                     m_localURL;        
    mutable bool                        m_fileFound;

    virtual StatusInt                   _UpdateLastModified            () override;

protected:
    explicit                            Impl                           (DTMSourceDataType       sourceDataType, 
                                                                        const IMoniker*         monikerP);
    explicit                            Impl(DTMSourceDataType       sourceDataType,
                                             const WCharCP         fullPath);
public:
    virtual                             ~Impl                          ();    

    // Uses default copy behavior

    LocalFileURL                        GetURL                         (StatusInt&              status) const;

    const WString&                      GetPath                        (StatusInt&              status) const;




};

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct IDTMDgnModelSource::Impl : public IDTMLocalFileSource::Impl
    {  
private:
    friend struct                       IDTMDgnModelSource;

    uint32_t                              m_modelID;
    WString                             m_modelName;
    WString                             m_rootToRefPersistentPath;

protected:
    explicit                            Impl                           (DTMSourceDataType               sourceDataType, 
                                                                        const IMoniker*                 monikerP,                                                  
                                                                        uint32_t                          modelID, 
                                                                        const WChar*                  modelName);

    explicit                            Impl(DTMSourceDataType               sourceDataType,
                                             const wchar_t*                 filePath,
                                             uint32_t                          modelID,
                                             const WChar*                  modelName);

    uint32_t                              GetModelID                     () const { return m_modelID; }
    const WString&                 GetModelName                   () const { return m_modelName; }

public:
    virtual                            ~Impl                           ();

    // Uses default copy behavior

    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct IDTMDgnReferenceSource::Impl : public IDTMDgnModelSource::Impl
    {  
private:
    friend struct                       IDTMDgnReferenceSource;

    WString                             m_rootToRefPersistentPath;
    WString                             m_referenceName;
    WString                             m_referenceModelName;

protected:
    explicit                            Impl                           (DTMSourceDataType               sourceDataType, 
                                                                        const IMoniker*                 monikerP,                                                  
                                                                        uint32_t                          modelID, 
                                                                        const WChar*                  modelName,
                                                                        const WChar*                     rootToRefPersistentPath,
                                                                        const WChar*                  referenceName,
                                                                        const WChar*                  referenceModelName);

    const WString&                  GetRootToRefPersistentPath     () const { return m_rootToRefPersistentPath; }
    const WString&                 GetReferenceName               () const { return m_referenceName; }

    const WString&                 GetReferenceModelName          () const { return m_referenceModelName; }

public:
    virtual                            ~Impl                           ();

    // Uses default copy behavior

    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct IDTMDgnLevelSource::Impl : public IDTMDgnModelSource::Impl

    {
private:
    friend struct                       IDTMDgnLevelSource;
    friend struct                       IDTMDgnLevelSourceCreator;

    uint32_t                              m_levelID;
    WString                        m_levelName;

protected:
    explicit                            Impl                           (DTMSourceDataType               sourceDataType, 
                                                                        const IMoniker*                 monikerP,                                                       
                                                                        uint32_t                          modelID, 
                                                                        const WChar*                  modelName,
                                                                        uint32_t                          levelID,
                                                                        const WChar*                  levelName);

    explicit                            Impl(DTMSourceDataType               sourceDataType,
                                             const wchar_t*                 filePath,
                                             uint32_t                          modelID,
                                             const WChar*                  modelName,
                                             uint32_t                          levelID,
                                             const WChar*                  levelName);

    uint32_t                              GetLevelID                     () const { return m_levelID; }
    const WString&                 GetLevelName                   () const { return m_levelName; }

public:
    virtual                             ~Impl                          ();

    // Uses default copy behavior

    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct IDTMDgnReferenceLevelSource::Impl : public IDTMDgnReferenceSource::Impl

    {
private:
    friend struct                       IDTMDgnReferenceLevelSource;
    friend struct                       IDTMDgnReferenceLevelSourceCreatorV0;
    friend struct                       IDTMDgnReferenceLevelSourceCreator;

    uint32_t                              m_levelID;
    WString                        m_levelName;

protected:
    explicit                            Impl                           (DTMSourceDataType               sourceDataType, 
                                                                        const IMoniker*                 monikerP,                                                       
                                                                        uint32_t                          modelID, 
                                                                        const WChar*                  modelName,
                                                                        const WChar*                     rootToRefPersistentPath,
                                                                        const WChar*                  referenceName,
                                                                        const WChar*                  referenceModelName,
                                                                        uint32_t                          levelID,
                                                                        const WChar*                  levelName);

    uint32_t                              GetLevelID                     () const { return m_levelID; }
    const WString&                 GetLevelName                   () const { return m_levelName; }

public:
    virtual                             ~Impl                          ();

    // Uses default copy behavior

    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct IDTMSourceGroup::Impl : public IDTMSource::Impl
    {     
private:
    friend struct                       IDTMSourceGroup;

    IDTMSourceCollection                m_sources;

    virtual bool                        _IsReachable                   () const;

    virtual StatusInt                   _UpdateLastModified            () override;
public : 
    explicit                            Impl                           ();
    virtual                             ~Impl                          ();

    // Uses default copy behavior

    const IDTMSourceCollection&         GetSources                     () const { return m_sources; }
    IDTMSourceCollection&               GetSources                     () { return m_sources; }
    };



END_BENTLEY_SCALABLEMESH_NAMESPACE
