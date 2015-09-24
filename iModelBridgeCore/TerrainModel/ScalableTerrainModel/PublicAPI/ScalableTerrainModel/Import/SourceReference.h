/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/PublicAPI/ScalableTerrainModel/Import/SourceReference.h $
|    $RCSfile: SourceReference.h,v $
|   $Revision: 1.19 $
|       $Date: 2012/02/23 18:20:09 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableTerrainModel/Import/Definitions.h>

BEGIN_BENTLEY_NAMESPACE

namespace DgnPlatform {
struct DgnModelRef;
struct ElementRefBase;
}
END_BENTLEY_NAMESPACE
    
typedef struct Bentley::DgnPlatform::ElementRefBase* ElementReferenceP;
typedef struct Bentley::DgnPlatform::DgnModelRef*  DgnModelReferenceP;

BEGIN_BENTLEY_MRDTM_IMPORT_PLUGIN_VXX_NAMESPACE(0)
struct DGNElementSourceRefBase;
END_BENTLEY_MRDTM_IMPORT_PLUGIN_VXX_NAMESPACE

BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE
struct ISourceRefVisitor;
struct SourceRefBase;


/*---------------------------------------------------------------------------------**//**
* @description  TDORAY: Source should not be part of the Ref name as it may also refer
*                       to a sink..... Find more suitable name.
* @bsiclass                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct SourceRef
    {
    typedef const std::type_info*           ClassID;
private:
    
    friend struct                           SourceRefBase;
    typedef SharedPtrTypeTrait<const SourceRefBase>::type
                                            BaseCPtr;   

    BaseCPtr                                m_basePtr;
    ClassID                                 m_classID;

    // Source reference should always be created via the Create method of a specialized SourceRef.
    explicit                                SourceRef                              (const SourceRefBase*        sourceRefP);
public:
    // Implicitly constructed from a base
    IMPORT_DLLE                             SourceRef                              (const SourceRefBase&        sourceRef);

    IMPORT_DLLE                             ~SourceRef                             ();

    IMPORT_DLLE                             SourceRef                              (const SourceRef&            rhs);    
    IMPORT_DLLE SourceRef&                  operator=                              (const SourceRef&            rhs);    

    ClassID                                 GetClassID                             () const { return m_classID; } 

    IMPORT_DLLE void                        Accept                                 (ISourceRefVisitor&          visitor) const;


    };


/*---------------------------------------------------------------------------------**//**
* @description  
* // TDORAY: Make source ref copyable via Clone pattern?
* @bsiclass                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct SourceRefBase : public ShareableObjectTypeTrait<SourceRefBase>::type
    {
    typedef const std::type_info*           ClassID;
private:
    friend struct                           SourceRef;

    void*                                   m_implP; // Reserved some space for further use

    virtual ClassID                         _GetClassID                            () const = 0;
    virtual void                            _Accept                                (ISourceRefVisitor&      visitor) const = 0;
    virtual SourceRefBase*                  _Clone                                 () const = 0;

protected:
    IMPORT_DLLE static SourceRef            CreateFromBase                         (const SourceRefBase*    sourceRefP);

    IMPORT_DLLE explicit                    SourceRefBase                          ();
    IMPORT_DLLE virtual                     ~SourceRefBase                         () = 0;

    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename BaseT>
struct SourceRefMixinBase : public SourceRefBase
    {
private:
    struct UniqueTokenType {};

    virtual ClassID                         _GetClassID                            () const override;
    virtual void                            _Accept                                (ISourceRefVisitor&      visitor) const override;
    virtual SourceRefBase*                  _Clone                                 () const override;
protected:
    typedef SourceRefMixinBase<BaseT>       super_class;

    explicit                                SourceRefMixinBase                     ();
    virtual                                 ~SourceRefMixinBase                    () = 0;

    static ClassID                          s_GetClassID                           ();
    };


// TDORAY: Add new source ref types for expressing invalid/unsupported/not found source refs. These source
//         refs will be intercepted by the importer when visiting and then provide an opportunity
//         for postponing error reporting. A new method can be added to SourceRef class mentioning
//         whether source ref is valid. An invalid source ref could still hold data in order to
//         enhance error messages (the invalid path, etc).

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct LocalFileSourceRef : public SourceRefMixinBase<LocalFileSourceRef>
    {
private:
    struct Impl;
    std::auto_ptr<Impl>                     m_pImpl;

public:
    IMPORT_DLLE static ClassID              s_GetClassID                           ();

    IMPORT_DLLE static SourceRef            CreateFrom                             (const WChar*              path);
   
    IMPORT_DLLE explicit                    LocalFileSourceRef                     (const WChar*              path);

    IMPORT_DLLE                             LocalFileSourceRef                     (const LocalFileSourceRef&   rhs);

    IMPORT_DLLE virtual                     ~LocalFileSourceRef                    ();

    IMPORT_DLLE friend bool                 operator==                             (const LocalFileSourceRef&   lhs,
                                                                                    const LocalFileSourceRef&   rhs);

    IMPORT_DLLE bool                        HasExtension                           (const WString&           extension) const;
    IMPORT_DLLE bool                        HasExtension                           (const WChar*             extension) const;

    IMPORT_DLLE const WString&           GetPath                                () const;
    IMPORT_DLLE const WChar*             GetPathCStr                            () const;

    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct DGNLevelByNameSourceRef : public SourceRefMixinBase<DGNLevelByNameSourceRef>
    {
private:
    struct Impl;
    std::auto_ptr<Impl>                     m_pImpl;

public:
    IMPORT_DLLE static ClassID              s_GetClassID                           ();


    IMPORT_DLLE static SourceRef            CreateFrom                             (const WChar*              dgnPath,
                                                                                    const WChar*              modelName,
                                                                                    const WChar*              levelName);

    IMPORT_DLLE explicit                    DGNLevelByNameSourceRef                (const WChar*              dgnPath,
                                                                                    const WChar*              modelName,
                                                                                    const WChar*              levelName);

    IMPORT_DLLE                             DGNLevelByNameSourceRef                (const DGNLevelByNameSourceRef&    rhs);

    IMPORT_DLLE virtual                     ~DGNLevelByNameSourceRef               ();

    IMPORT_DLLE const WString&           GetDGNPath                             () const;
    IMPORT_DLLE const WChar*             GetDGNPathCStr                         () const;

    IMPORT_DLLE const WString&           GetModelName                           () const;
    IMPORT_DLLE const WChar*             GetModelNameCStr                       () const;

    IMPORT_DLLE const WString&           GetLevelName                           () const;
    IMPORT_DLLE const WChar*             GetLevelNameCStr                       () const;
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct DGNReferenceLevelByNameSourceRef : public SourceRefMixinBase<DGNReferenceLevelByNameSourceRef>
    {
private:
    struct Impl;
    std::auto_ptr<Impl>                     m_pImpl;

public:
    IMPORT_DLLE static ClassID              s_GetClassID                           ();


    IMPORT_DLLE static SourceRef            CreateFrom                             (const WChar*              dgnPath,
                                                                                    const WChar*              rootModelName,
                                                                                    const WChar*                 rootToRefPersistentPath,
                                                                                    const WChar*              referenceLevelName);

    IMPORT_DLLE explicit                    DGNReferenceLevelByNameSourceRef       (const WChar*              dgnPath,
                                                                                    const WChar*              rootModelName,
                                                                                    const WChar*                 rootToRefPersistentPath,
                                                                                    const WChar*              referenceLevelName);

    IMPORT_DLLE                             DGNReferenceLevelByNameSourceRef       (const DGNReferenceLevelByNameSourceRef&
                                                                                                                rhs);

    IMPORT_DLLE virtual                     ~DGNReferenceLevelByNameSourceRef      ();

    IMPORT_DLLE const WString&           GetDGNPath                             () const;
    IMPORT_DLLE const WChar*             GetDGNPathCStr                         () const;

    IMPORT_DLLE const WString&           GetRootModelName                       () const;
    IMPORT_DLLE const WChar*             GetRootModelNameCStr                   () const;

    IMPORT_DLLE const WString&          GetRootToRefPersistentPath             () const;
    IMPORT_DLLE const WChar*            GetRootToRefPersistentPathCStr         () const;

    IMPORT_DLLE const WString&           GetLevelName                           () const;
    IMPORT_DLLE const WChar*             GetLevelNameCStr                       () const;
    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct DGNLevelByIDSourceRef : public SourceRefMixinBase<DGNLevelByIDSourceRef>
    {
private:
    struct Impl;
    std::auto_ptr<Impl>                     m_pImpl;

public:
    IMPORT_DLLE static ClassID              s_GetClassID                           ();


    IMPORT_DLLE static SourceRef            CreateFrom                             (const WChar*              dgnPath,
                                                                                    uint32_t                      modelID,
                                                                                    uint32_t                      levelID);

    IMPORT_DLLE explicit                    DGNLevelByIDSourceRef                  (const WChar*              dgnPath,
                                                                                    uint32_t                      modelID,
                                                                                    uint32_t                      levelID);

    IMPORT_DLLE                             DGNLevelByIDSourceRef                  (const DGNLevelByIDSourceRef&
                                                                                                                rhs);

    IMPORT_DLLE virtual                     ~DGNLevelByIDSourceRef                 ();

    IMPORT_DLLE const WString&           GetDGNPath                             () const;
    IMPORT_DLLE const WChar*             GetDGNPathCStr                         () const;

    IMPORT_DLLE uint32_t                      GetModelID                             () const;
    
    IMPORT_DLLE uint32_t                      GetLevelID                             () const;
    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct DGNReferenceLevelByIDSourceRef : public SourceRefMixinBase<DGNReferenceLevelByIDSourceRef>
    {
private:
    struct Impl;
    std::auto_ptr<Impl>                     m_pImpl;

public:
    IMPORT_DLLE static ClassID              s_GetClassID                           ();

    IMPORT_DLLE static SourceRef            CreateFrom                             (const WChar*              dgnPath,
                                                                                    uint32_t                      rootModelID,
                                                                                    const WChar*                 rootToRefPersistentPath,
                                                                                    uint32_t                      referenceLevelID);

    IMPORT_DLLE explicit                    DGNReferenceLevelByIDSourceRef         (const WChar*              dgnPath,
                                                                                    uint32_t                      rootModelID,
                                                                                    const WChar*                 rootToRefPersistentPath,
                                                                                    uint32_t                      referenceLevelID);

    IMPORT_DLLE                             DGNReferenceLevelByIDSourceRef         (const DGNReferenceLevelByIDSourceRef&
                                                                                                                rhs);

    IMPORT_DLLE virtual                     ~DGNReferenceLevelByIDSourceRef        ();

    IMPORT_DLLE const WString&           GetDGNPath                             () const;
    IMPORT_DLLE const WChar*             GetDGNPathCStr                         () const;

    IMPORT_DLLE uint32_t                      GetRootModelID                         () const;
    
    IMPORT_DLLE const WString&          GetRootToRefPersistentPath             () const;
    IMPORT_DLLE const WChar*                 GetRootToRefPersistentPathCStr         () const;

    IMPORT_DLLE uint32_t                      GetLevelID                             () const;
    };



/*---------------------------------------------------------------------------------**//**
* @description  
* NTERAY: Elements could also be referred to via a path, a model id + a persistent element path
*         we should probably rename this class in order to enables us to have a serialize-able
*         ref to a element.
* @bsiclass                                                Jean-Francois.Cote   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct DGNElementSourceRef : public SourceRefMixinBase<DGNElementSourceRef>
    {
    typedef struct Plugin::V0::DGNElementSourceRefBase
                                            Base;
private:
    std::auto_ptr<Base>                     m_baseP;
public:
    IMPORT_DLLE static ClassID              s_GetClassID                           ();

    IMPORT_DLLE static SourceRef            CreateFrom                             (Base*                       baseP);

    IMPORT_DLLE explicit                    DGNElementSourceRef                    (Base*                       baseP);
    IMPORT_DLLE                             DGNElementSourceRef                    (const DGNElementSourceRef&     rhs);

    IMPORT_DLLE virtual                     ~DGNElementSourceRef                   ();

    IMPORT_DLLE UInt                        GetElementType                         () const;
    IMPORT_DLLE UInt                        GetElementHandlerID                    () const;
    
    IMPORT_DLLE ElementReferenceP           GetElementRef                          () const;
    IMPORT_DLLE DgnModelReferenceP          GetModelRef                            () const;    

    IMPORT_DLLE const LocalFileSourceRef*   GetLocalFileP                          () const;
    };


END_BENTLEY_MRDTM_IMPORT_NAMESPACE