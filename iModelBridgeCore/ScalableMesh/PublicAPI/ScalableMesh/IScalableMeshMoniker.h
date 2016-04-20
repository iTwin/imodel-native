/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/IScalableMeshMoniker.h $
|    $RCSfile: IScalableMeshMoniker.h,v $
|   $Revision: 1.19 $
|       $Date: 2011/10/21 17:32:50 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <GeoCoord/BaseGeoCoord.h>
#include <TerrainModel/TerrainModel.h>
#include <TerrainModel/Core/IDTM.h>
#include <ScalableMesh/IScalableMesh.h>
#include <ScalableMesh/ScalableMeshDefs.h>
#include <ScalableMesh/IScalableMeshDocumentEnv.h>
#include <ScalableMesh/IScalableMeshURL.h>
#include <Bentley/RefCounted.h>
#include <ScalableMesh/Import/DataSQLite.h>

namespace BENTLEY_NAMESPACE_NAME
    {
    namespace DgnPlatform
        {

struct                                  DgnDocumentMoniker;
typedef RefCountedPtr<DgnDocumentMoniker>
                                        MrDtmDgnDocumentMonikerPtr;

}}

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

struct IMoniker;
struct ILocalFileMoniker;
struct IMonikerCreator;

struct BinaryIStream;
struct BinaryOStream;

//Support for the Document Manager API's moniker. 
typedef RefCountedPtr<IMoniker>          IMonikerPtr;
typedef RefCountedPtr<ILocalFileMoniker> ILocalFileMonikerPtr;
typedef RefCountedPtr<IMonikerCreator>   IMonikerCreatorPtr;


struct IMonikerVisitor
    {
    virtual                             ~IMonikerVisitor               () = 0 {}

    virtual void                        _Visit                         (const IMoniker&                     moniker) { /*Do nothing*/ }

    virtual void                        _Visit                         (const ILocalFileMoniker&            moniker) = 0;
    };


struct IMoniker : public RefCountedBase
    {
private:   
    const void*                         m_implP; // Reserved some space for further use

    // Disable copy
                                        IMoniker                       (const IMoniker&);
    IMoniker&                           operator=                      (const IMoniker&);

    virtual void                        _Accept                        (IMonikerVisitor&                    visitor) const = 0;

    virtual DTMSourceMonikerType        _GetType                       () const = 0;    

    virtual bool                        _IsTargetReachable             () const = 0;

    virtual StatusInt                   _Serialize(Import::SourceDataSQLite&                      sourceData,
        const DocumentEnv&                  env) const = 0;

       
protected:
    BENTLEY_SM_EXPORT explicit                IMoniker                       ();
public: 
    BENTLEY_SM_EXPORT virtual                 ~IMoniker                      () = 0;
    
    void                                Accept                         (IMonikerVisitor&                    visitor) const;

    DTMSourceMonikerType                GetType                        () const;    

    bool                                IsTargetReachable              () const;


    StatusInt                           Serialize(Import::SourceDataSQLite&                      sourceData,
        const DocumentEnv&                  env) const;


    };




struct ILocalFileMoniker : public IMoniker
    {
private:
    const void*                         m_implP; // Reserved some space for further use

    BENTLEY_SM_EXPORT virtual void            _Accept                        (IMonikerVisitor&                    visitor) const;

    virtual LocalFileURL                _GetURL                        (StatusInt&                          status) const = 0;

protected:
    BENTLEY_SM_EXPORT explicit                ILocalFileMoniker              ();

public: 
    BENTLEY_SM_EXPORT virtual                 ~ILocalFileMoniker             () = 0;

    LocalFileURL                        GetURL                         (StatusInt&                          status) const;
    LocalFileURL                        GetURL                         () const;
    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct ILocalFileMonikerCreator
    {
private:
    friend struct                       ILocalFileMonikerFactory;

    const void*                         m_implP; // Reserved some space for further use

    // Disable copy
                                        ILocalFileMonikerCreator       (const ILocalFileMonikerCreator&);
    ILocalFileMonikerCreator&           operator=                      (const ILocalFileMonikerCreator&);

    virtual ILocalFileMonikerPtr        _Create                        (const BENTLEY_NAMESPACE_NAME::DgnPlatform::MrDtmDgnDocumentMonikerPtr&         
                                                                                                            msMoniker,
                                                                        StatusInt&                          status) const = 0;

    virtual ILocalFileMonikerPtr        _Create                        (const WChar*                      fullPath,
                                                                        StatusInt&                          status) const = 0;

protected:
    BENTLEY_SM_EXPORT explicit                ILocalFileMonikerCreator       ();
public:
    BENTLEY_SM_EXPORT virtual                 ~ILocalFileMonikerCreator      () = 0;

    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct ILocalFileMonikerFactory
    {
private:
    struct                              Impl;
    std::auto_ptr<Impl>                 m_implP;

    explicit                            ILocalFileMonikerFactory       ();
                                        ~ILocalFileMonikerFactory      ();

    // Disable copy
                                        ILocalFileMonikerFactory       (const ILocalFileMonikerFactory&);
    ILocalFileMonikerFactory&           operator=                      (const ILocalFileMonikerFactory&);
    
public:
    typedef const ILocalFileMonikerCreator*
                                        CreatorID;

    BENTLEY_SM_EXPORT static ILocalFileMonikerFactory&
                                        GetInstance                    ();

    BENTLEY_SM_EXPORT CreatorID               Register                       (const ILocalFileMonikerCreator&     creator);
    BENTLEY_SM_EXPORT void                    Unregister                     (CreatorID                           id);


    BENTLEY_SM_EXPORT ILocalFileMonikerPtr    Create                         (const BENTLEY_NAMESPACE_NAME::DgnPlatform::MrDtmDgnDocumentMonikerPtr&         
                                                                                                            msMoniker) const;

    BENTLEY_SM_EXPORT ILocalFileMonikerPtr    Create                         (const WChar*                      fullPath) const;
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct IMonikerBinStreamCreator
    {
private :   
    friend struct                       IMonikerFactory;
    const void*                         m_implP; // Reserved some space for further use

    // Disable copy
                                        IMonikerBinStreamCreator       
                                                                       (const IMonikerBinStreamCreator&);
    IMonikerBinStreamCreator&           operator=                      (const IMonikerBinStreamCreator&);

    virtual DTMSourceMonikerType        _GetSupportedType              () const = 0;


    virtual IMonikerPtr                 _Create(Import::SourceDataSQLite&                      sourceData,
        const DocumentEnv&                  env,
        StatusInt&                          status) const = 0;

    
protected:
    typedef IMoniker                    IMoniker; // Avoid name ambiguities

    BENTLEY_SM_EXPORT explicit                IMonikerBinStreamCreator       ();
public :         
    BENTLEY_SM_EXPORT virtual                 ~IMonikerBinStreamCreator      () = 0;
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct IMonikerFactory
    {
private:
    struct                              Impl;
    std::auto_ptr<Impl>                 m_implP;

    explicit                            IMonikerFactory                ();
                                        ~IMonikerFactory               ();

    // Disable copy
                                        IMonikerFactory                (const IMonikerFactory&);
    IMonikerFactory&                    operator=                      (const IMonikerFactory&);
    
public:
    typedef const IMonikerBinStreamCreator*      
                                        BinStreamCreatorID;

    BENTLEY_SM_EXPORT static IMonikerFactory& GetInstance                    ();

    BENTLEY_SM_EXPORT BinStreamCreatorID      Register                       (const IMonikerBinStreamCreator&     creator);
    BENTLEY_SM_EXPORT void                    Unregister                     (BinStreamCreatorID                  id);


    BENTLEY_SM_EXPORT IMonikerPtr             Create(Import::SourceDataSQLite&                      sourceData,
        const DocumentEnv&                  env);

    };

END_BENTLEY_SCALABLEMESH_NAMESPACE
