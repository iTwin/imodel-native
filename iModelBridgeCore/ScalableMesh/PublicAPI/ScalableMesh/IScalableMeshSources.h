/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/IScalableMeshSources.h $
|    $RCSfile: IScalableMeshSources.h,v $
|   $Revision: 1.17 $
|       $Date: 2011/12/01 18:51:39 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/


#include <TerrainModel/TerrainModel.h>
#include <ScalableMesh/ScalableMeshDefs.h>
#include <Bentley/RefCounted.h>
#include <ScalableMesh/IScalableMeshMoniker.h>
#include <ScalableMesh/IScalableMeshTime.h>
#include <ScalableMesh/IScalableMeshURL.h>
#include <ScalableMesh/IScalableMeshSourceCollection.h>
#include <ScalableMesh/GeoCoords/GCS.h>

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

struct SourceImportConfig;

struct IDTMSource;
struct IDTMSourceGroup;
struct IDTMLocalFileSource;
struct IDTMDgnLevelSource;
struct IDTMDgnReferenceLevelSource;

//struct BinaryIStream;
//struct BinaryOStream;


typedef RefCountedPtr<IDTMSource>           IDTMSourcePtr;
typedef RefCountedPtr<IDTMSourceGroup>      IDTMSourceGroupPtr;

//Different type of data source.
typedef RefCountedPtr<IDTMLocalFileSource>  IDTMLocalFileSourcePtr;                                         
typedef RefCountedPtr<IDTMDgnLevelSource>   IDTMDgnLevelSourcePtr;
typedef RefCountedPtr<IDTMDgnReferenceLevelSource>   
                                            IDTMDgnReferenceLevelSourcePtr;


/*__PUBLISH_SECTION_END__*/
struct                                      IDTMSourceVisitor;
struct                                      EditListener;
/*__PUBLISH_SECTION_START__*/

/***************************************************************************************
READ THIS BEFORE ADDING A NEW SOURCE : 

What must be modify : 

In the platform (might not be required if an current base source can already do the trick, for example trying to support a new local source file): 

- Add a new source in PublicAPI\ScalableMesh\IScalableMeshSources.h 
- Add a new source reference in PublicAPI\ScalableMesh\Import\SourceReference.h 
- Add a new source base in PublicAPI\ScalableMesh\Import\Plugin\SourceV0.h
- Add a new source serializer (e.g. : IDTMLocalFileSourceSerializer) in TerrainModel\ScalableTerrainModel\STM\ScalableMeshSourcePersistance.cpp

In the DLL where the plugin code will be located (e.g. : Descartes) : 

- Create a new file for the source definition (e.g. : DEMRasterSource) and source extractor definition (e.g. : DEMRasterPointExtractor) 
- Create a source creator (e.g. : DEMRasterFileSourceCreator) and source extractor creator (e.g. : DEMRasterPointExtractorCreator)
- In the source creator create a _Supports method taking in parameter the related source reference created in SourceReference.
- In the source extractor creator a _Supports method that specify the type of data supported by the extractor. 
- Register a source creator and the source extractor creator (e.g. : RegisterDEMImportPlugin).
****************************************************************************************/

/***************************************************************************************
READ THIS BEFORE ADDING A NEW CONTENT CONFIG (usually a config object should pertain to all kind of source) : 

What must be modify : 

In the platform : 

- New config object in Import\ContentConfigs.cpp.
- New visitor function for the new config object in Import\ContentDescriptor.cpp.
- New public function for setting the new config object info in IScalableMeshSourceImportConfig.h.
- New ConfigComponentSerializationID in ScalableMeshContentConfigPersistance.cpp (Please add the new ID just before CCSID_QTY). 
- New config serializer and deserializer in ScalableMeshContentConfigPersistance.cpp.


In the application where the source selection GUI is (e.g. : Descartes) : 

- Just call the new public method in IScalableMeshSourceImportConfig.h for configuring the new config object.
****************************************************************************************/

/***************************************************************************************
READ THIS BEFORE ADDING A NEW SEQUENCE CONFIG (usually a sequence config object can be created for a handling a new type of data, 
which could also probably be done by adding a new source) : 

What must be modify : 

In the platform :

- Add a new type family creator (e.g. LinearTypeFamilyCreator) in PublicAPI\ScalableMesh\Type\IScalableMeshLinear.h.
- Create all the stuffs required base on one existing family creator.
- Add a new entry in the enum DTMSourceDataType.

In the application where the source selection GUI is (e.g. : Descartes) : 

- Add this new type family creator to the source sequence.
- Add the new data type related to the type family creator to the _CreateDescriptor of the source definition.
- Create and register a new source extractor creator supporting this new type family creator.
****************************************************************************************/

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct IDTMSource : public RefCountedBase
    {
    private: 
        // Disable copy
                                            IDTMSource                 (const IDTMSource&);
        IDTMSource&                         operator=                  (const IDTMSource&);

/*__PUBLISH_SECTION_END__*/
    public:



        struct                              Impl;

        void                                RegisterEditListener       (EditListener&               listener);
        void                                UnregisterEditListener     (const EditListener&         listener);

        bool                                IsPartOfCollection         () const;

        StatusInt                           InternalUpdateLastModified ();

    private:        
        std::auto_ptr<Impl>                 m_implP;
        
        virtual void                        _Accept                    (IDTMSourceVisitor&          visitor) const = 0;
        virtual IDTMSource*                 _Clone                     () const = 0;

    protected:
        explicit                            IDTMSource                 (Impl*                       implP);

/*__PUBLISH_SECTION_START__*/

    public: 
        BENTLEY_SM_EXPORT virtual                 ~IDTMSource                () = 0;
       
        BENTLEY_SM_EXPORT bool                    IsReachable                () const;

//        BENTLEY_SM_EXPORT const IMoniker&         GetMoniker                 () const;

        BENTLEY_SM_EXPORT WString                 GetPath                    () const;

        BENTLEY_SM_EXPORT DTMSourceDataType       GetSourceType              () const;

        BENTLEY_SM_EXPORT Time                    GetLastModified            () const;
        BENTLEY_SM_EXPORT Time                    GetLastModifiedCheckTime   () const;

        BENTLEY_SM_EXPORT void                    SetLastModified            (const Time&                 time);

        BENTLEY_SM_EXPORT void                    ResetLastModified          ();
        BENTLEY_SM_EXPORT StatusInt               UpdateLastModified         ();
        // TDORAY: Consider adding a versions that takes as argument the minimum last time checked for
        // which it is not worth updating.
        

        BENTLEY_SM_EXPORT SourceImportConfig&     EditConfig                 ();
        BENTLEY_SM_EXPORT const SourceImportConfig&   
                                            GetConfig                  () const;

        BENTLEY_SM_EXPORT IDTMSourcePtr           Clone                      () const;


        

        

/*__PUBLISH_SECTION_END__*/

        void                                Accept                     (IDTMSourceVisitor&          visitor) const;
/*__PUBLISH_SECTION_START__*/
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct IDTMLocalFileSource : public IDTMSource
    {            

/*__PUBLISH_SECTION_END__*/
    friend struct                           IDTMLocalFileSourceCreator;

    public:
        struct                              Impl;
    private:
        Impl&                               m_impl;

        virtual void                        _Accept                    (IDTMSourceVisitor&          visitor) const override;
        virtual IDTMSource*                 _Clone                     () const override;
    protected:
        explicit                            IDTMLocalFileSource    (Impl*                       implP);
/*__PUBLISH_SECTION_START__*/

    public : 
        BENTLEY_SM_EXPORT virtual                 ~IDTMLocalFileSource   ();

        BENTLEY_SM_EXPORT LocalFileURL            GetURL                     () const;
        BENTLEY_SM_EXPORT LocalFileURL            GetURL                     (StatusInt&                  status) const;

        BENTLEY_SM_EXPORT const WChar*          GetPath                    () const;
        BENTLEY_SM_EXPORT const WChar*          GetPath                    (StatusInt&                  status) const;
 
        BENTLEY_SM_EXPORT static IDTMLocalFileSourcePtr 
                                            Create                     (DTMSourceDataType           sourceDataType, 
                                                                        const ILocalFileMonikerPtr& localFileMonikerPtr);   
        BENTLEY_SM_EXPORT static IDTMLocalFileSourcePtr
                                            Create(DTMSourceDataType           sourceDataType,
                                                    const WCharCP fullPath);
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct IDTMDgnModelSource : public IDTMLocalFileSource
    {
/*__PUBLISH_SECTION_END__*/
    friend struct                           IDTMDgnModelSourceCreator;

    public:
        struct                              Impl;
    private: 
        Impl&                               m_impl;

        virtual void                        _Accept                    (IDTMSourceVisitor&          visitor) const override;
        virtual IDTMSource*                 _Clone                     () const override;
    protected:
        explicit                            IDTMDgnModelSource     (Impl*                       implP);
                
/*__PUBLISH_SECTION_START__*/

    public :                                        
        BENTLEY_SM_EXPORT virtual                 ~IDTMDgnModelSource    ();
        
        BENTLEY_SM_EXPORT uint32_t                  GetModelID                 () const;
        BENTLEY_SM_EXPORT const WChar*          GetModelName               () const;

        BENTLEY_SM_EXPORT void                    UpdateModelName            (const WChar*              name) const;
    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct IDTMDgnReferenceSource : public IDTMDgnModelSource
    {
/*__PUBLISH_SECTION_END__*/
    friend struct                           IDTMDgnReferenceSourceCreator;


    public:
        struct                              Impl;
    private: 
        Impl&                               m_impl;

        virtual void                        _Accept                    (IDTMSourceVisitor&          visitor) const override;
        virtual IDTMSource*                 _Clone                     () const override;
    protected:
        explicit                            IDTMDgnReferenceSource     (Impl*                       implP);
                
/*__PUBLISH_SECTION_START__*/

    public :                                        
        BENTLEY_SM_EXPORT virtual                 ~IDTMDgnReferenceSource    ();

        BENTLEY_SM_EXPORT WCharCP             GetRootToRefPersistentPath () const;

        BENTLEY_SM_EXPORT const WChar*          GetReferenceName           () const;
        BENTLEY_SM_EXPORT void                    UpdateReferenceName        (const WChar*              name) const;

        BENTLEY_SM_EXPORT const WChar*          GetReferenceModelName      () const;
        BENTLEY_SM_EXPORT void                    UpdateReferenceModelName   (const WChar*              name) const;
    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct IDTMDgnLevelSource : public IDTMDgnModelSource
    {
/*__PUBLISH_SECTION_END__*/
    friend struct                           IDTMDgnLevelSourceCreator;


    public:
        struct                              Impl;
    private: 
        Impl&                               m_impl;        

        virtual void                        _Accept                    (IDTMSourceVisitor&          visitor) const override;
        virtual IDTMSource*                 _Clone                     () const override;
    protected:
        explicit                            IDTMDgnLevelSource         (Impl*                       implP);
/*__PUBLISH_SECTION_START__*/

    public :          
        BENTLEY_SM_EXPORT virtual                 ~IDTMDgnLevelSource        ();

        BENTLEY_SM_EXPORT uint32_t                  GetLevelID                 () const;
        BENTLEY_SM_EXPORT const WChar*          GetLevelName               () const;


        BENTLEY_SM_EXPORT void                    UpdateLevelName            (const WChar*              name) const;

        BENTLEY_SM_EXPORT static IDTMDgnLevelSourcePtr 
                                            Create                     (DTMSourceDataType           sourceDataType, 
                                                                        const ILocalFileMonikerPtr& dgnFileMonikerPtr,
                                                                        uint32_t                      modelID, 
                                                                        const WChar*              modelName,
                                                                        uint32_t                      levelID,
                                                                        const WChar*              levelName);

                BENTLEY_SM_EXPORT static IDTMDgnLevelSourcePtr 
                                            Create                     (DTMSourceDataType           sourceDataType, 
                                                                        const wchar_t* dgnFileMonikerPtr,
                                                                        uint32_t                      modelID, 
                                                                        const WChar*              modelName,
                                                                        uint32_t                      levelID,
                                                                        const WChar*              levelName);
    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct IDTMDgnReferenceLevelSource : public IDTMDgnReferenceSource
    {
/*__PUBLISH_SECTION_END__*/
    friend struct                           IDTMDgnReferenceLevelSourceCreatorV0;
    friend struct                           IDTMDgnReferenceLevelSourceCreator;


    public:
        struct                              Impl;
    private: 
        Impl&                               m_impl;        

        virtual void                        _Accept                    (IDTMSourceVisitor&          visitor) const override;
        virtual IDTMSource*                 _Clone                     () const override;
    protected:
        explicit                            IDTMDgnReferenceLevelSource(Impl*                       implP);
/*__PUBLISH_SECTION_START__*/

    public :          
        BENTLEY_SM_EXPORT virtual                 ~IDTMDgnReferenceLevelSource
                                                                       ();

        BENTLEY_SM_EXPORT uint32_t                  GetLevelID                 () const;
        BENTLEY_SM_EXPORT const WChar*          GetLevelName               () const;


        BENTLEY_SM_EXPORT void                    UpdateLevelName            (const WChar*              name) const;


        BENTLEY_SM_EXPORT static IDTMDgnReferenceLevelSourcePtr 
                                            Create                     (DTMSourceDataType           sourceDataType, 
                                                                        const ILocalFileMonikerPtr& rootDgnFileMonikerPtr,
                                                                        uint32_t                      rootModelID, 
                                                                        const WChar*              rootModelName,
                                                                        const WChar*                 rootToRefPersistentPath,
                                                                        const WChar*              referenceName,
                                                                        const WChar*              referenceModelName,
                                                                        uint32_t                      referenceLevelID,
                                                                        const WChar*              referenceLevelName);

    };




/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct IDTMSourceGroup : public IDTMSource
    {
/*__PUBLISH_SECTION_END__*/
    public:
        struct                              Impl;
        const Impl&                         GetImpl                    () const { return m_impl; }
        Impl&                               GetImpl                    () { return m_impl; }

    private:             
        Impl&                               m_impl;   

        virtual void                        _Accept                    (IDTMSourceVisitor&          visitor) const override;
        virtual IDTMSource*                 _Clone                     () const override;
    protected:
        explicit                            IDTMSourceGroup        (Impl*                       implP);
          
/*__PUBLISH_SECTION_START__*/

    public : 
        BENTLEY_SM_EXPORT virtual                 ~IDTMSourceGroup       ();

        BENTLEY_SM_EXPORT static IDTMSourceGroupPtr 
                                            Create                     ();   

        BENTLEY_SM_EXPORT const IDTMSourceCollection&
                                            GetSources                 () const;
        BENTLEY_SM_EXPORT IDTMSourceCollection&   EditSources                ();
    };



END_BENTLEY_SCALABLEMESH_NAMESPACE
