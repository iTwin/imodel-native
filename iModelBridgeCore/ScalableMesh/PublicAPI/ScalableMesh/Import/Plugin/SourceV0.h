/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/Import/Plugin/SourceV0.h $
|    $RCSfile: SourceV0.h,v $
|   $Revision: 1.24 $
|       $Date: 2012/03/21 18:37:04 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableMesh/Import/Definitions.h>

#include <ScalableMesh/Import/ContentDescriptor.h>
#include <ScalableMesh/Import/SourceReference.h>

#include <ScalableMesh/Import/Error/Source.h>

#include <ScalableMesh/Foundations/Log.h>
#include <ScalableMesh/Foundations/Exception.h>
#include <ScalableMesh/Foundations/Warning.h>

#include <ScalableMesh/Import/Plugin/SourceRegistry.h>


BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
struct Source;
struct SourceCreator;
END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE


BEGIN_BENTLEY_SCALABLEMESH_IMPORT_PLUGIN_VXX_NAMESPACE(0)

/*---------------------------------------------------------------------------------**//**
* @description  //TDORAY: Would better be named StoreBase
* @bsiclass                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct SourceBase : private Uncopyable
    {
protected:
    typedef const std::type_info*               ClassID;
private:
    friend struct                               Source;
    friend struct                               SourceCreatorBase;

    struct                                      Impl;   
    std::auto_ptr<Impl>                         m_pImpl;


    virtual ClassID                             _GetClassID                        () const = 0;
    IMPORT_DLLE virtual SourceBase&             _ResolveOriginal                   ();

    virtual void                                _Close                             () = 0;

    virtual ContentDescriptor                   _CreateDescriptor                  () const = 0;
    IMPORT_DLLE virtual void                    _ExtendDescription                 (ContentDescriptor&          description) const;

    virtual const WChar*                       _GetType                           () const = 0;


protected:
    struct                                      Handler;

    IMPORT_DLLE explicit                        SourceBase                         ();

public:
    IMPORT_DLLE virtual                         ~SourceBase                        () = 0;

    IMPORT_DLLE const ContentDescriptor&        GetDescriptor                      () const;
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct SourceBase::Handler
    {
    static ClassID                              GetClassID                         (const SourceBase&           instance);
    static SourceBase&                          ResolveOriginal                    (SourceBase&                 instance);

    static void                                 Close                              (SourceBase&                 instance);

    static ContentDescriptor                    CreateDescriptor                   (const SourceBase&           instance);
    static void                                 ExtendDescription                  (const SourceBase&           instance,
                                                                                    ContentDescriptor&          description);

    static const WChar*                      GetType                            (const SourceBase&           instance);
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename SourceT>
struct SourceMixinBase : public SourceBase
    {
private:
    struct UniqueTokenType {};

    virtual ClassID                             _GetClassID                        () const override
        {
        return s_GetClassID();
        }
public:
    static ClassID                              s_GetClassID                       () 
        {
        static ClassID CLASS_ID = &typeid(UniqueTokenType());
        return CLASS_ID;
        }
    };


/*---------------------------------------------------------------------------------**//**
* @description 
* @bsiclass                                                  Raymond.Gauthier   7/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct SourceCreatorBase : private Uncopyable
    {
private:
    friend struct                               SourceCreator;

    const void*                                 m_implP; // Reserve some space for further use

    virtual bool                                _Supports                          (const SourceRef&                    sourceRef) const = 0;
    virtual SourceBase*                         _Create                            (const SourceRef&                    sourceRef,
                                                                                    Log&                                log) const = 0;
protected:
    struct                                      SourceHandler;

    IMPORT_DLLE explicit                        SourceCreatorBase                  ();

public:
    typedef const SourceCreatorBase*            ID;

    IMPORT_DLLE virtual                         ~SourceCreatorBase                 () = 0;
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct SourceCreatorBase::SourceHandler : public SourceBase::Handler
    {
    };


typedef const WChar*                         ExtensionFilter;

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct LocalFileSourceCreatorBase : private Uncopyable
    {
private:
    friend struct                               LocalFileSourceCreator;

    struct                                      Impl; 
    std::auto_ptr<Impl>                         m_pImpl;

    virtual ExtensionFilter                     _GetExtensions                     () const = 0;

    virtual bool                                _Supports                          (const LocalFileSourceRef&           sourceRef) const = 0;
    virtual SourceBase*                         _Create                            (const LocalFileSourceRef&           sourceRef,
                                                                                    Log&                                log) const = 0;
protected:
    IMPORT_DLLE explicit                        LocalFileSourceCreatorBase         ();

    IMPORT_DLLE bool                            DefaultSupports                    (const LocalFileSourceRef&           sourceRef) const;

public:
    typedef const LocalFileSourceCreatorBase*   ID;

    IMPORT_DLLE virtual                         ~LocalFileSourceCreatorBase        () = 0;

    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct DGNElementSourceCreatorBase : private Uncopyable
    {
private:
    friend struct                               DGNElementSourceCreator;

    const void*                                 m_implP; // Reserve some space for further use

    virtual UInt                                _GetElementType                    () const = 0;
    virtual UInt                                _GetElementHandlerID               () const = 0;

    virtual bool                                _Supports                          (const DGNElementSourceRef&          sourceRef) const = 0;
    virtual SourceBase*                         _Create                            (const DGNElementSourceRef&          sourceRef,
                                                                                    Log&                                log) const = 0;
protected:
    IMPORT_DLLE explicit                        DGNElementSourceCreatorBase        ();
public:
    typedef const DGNElementSourceCreatorBase*  ID;

    IMPORT_DLLE virtual                         ~DGNElementSourceCreatorBase       () = 0;

    };


// TDORAY: Move to hpp?
inline SourceBase::ClassID SourceBase::Handler::GetClassID (const SourceBase& instance) 
    { return instance._GetClassID(); }

inline SourceBase& SourceBase::Handler::ResolveOriginal (SourceBase& instance) 
    { return instance._ResolveOriginal(); }

inline void SourceBase::Handler::Close (SourceBase& instance) 
    { return instance._Close(); }

inline ContentDescriptor SourceBase::Handler::CreateDescriptor (const SourceBase& instance)
    { return instance._CreateDescriptor(); }

inline void SourceBase::Handler::ExtendDescription (const SourceBase&   instance,
                                                    ContentDescriptor&  description)
    { return instance._ExtendDescription(description); }

inline const WChar* SourceBase::Handler::GetType (const SourceBase& instance)
    { return instance._GetType(); }

END_BENTLEY_SCALABLEMESH_IMPORT_PLUGIN_VXX_NAMESPACE
