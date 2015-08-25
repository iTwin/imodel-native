/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ImportPlugins/ElemSourceRef.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

        

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE


struct DGNModelRefHolder;

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                Raymond.Gauthier     07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct DGNElemSourceRefBase : public Bentley::ScalableMesh::Import::Plugin::V0::DGNElementSourceRefBase
    {
private:
    struct Impl;
    std::auto_ptr<Impl>                 m_implP;

    explicit                            DGNElemSourceRefBase                   (const DGNElemSourceRefBase& rhs);

    virtual DGNElemSourceRefBase*       _Clone                                 () const override;


    virtual UInt                        _GetElementType                        () const override;
    virtual UInt                        _GetElementHandlerID                   () const override;

    virtual ElementRefP                  _GetElementRef                         () const override;

    virtual DgnModelRefP                _GetModelRef                           () const override;

    virtual const LocalFileSourceRef*   _GetLocalFileP                         () const override;


    explicit                            DGNElemSourceRefBase                   (Impl*                       implP);

protected:
    typedef Bentley::ScalableMesh::Import::SourceRef
                                        SourceRef;

    static SourceRef                    CreateFromImpl                         (UInt                        elemType,
                                                                                UInt                        elemHandlerID,
                                                                                ElementRefP                  elemRef,
                                                                                const DGNModelRefHolder&    modelRef,
                                                                                LocalFileSourceRef*         localFileRefP);

    virtual                             ~DGNElemSourceRefBase                  ();
    };


/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                Jean-Francois.Cote   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct RasterElemSourceRef : private DGNElemSourceRefBase
    {
public:
    static SourceRef                    CreateFrom                             (const ElementRefP&           elemRef,
                                                                                const DGNModelRefHolder&    modelRef);
    };


/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                Jean-Francois.Cote   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct CivilElemSourceRef : private DGNElemSourceRefBase
    {
public:
    static SourceRef                    CreateFrom                             (const ElementRefP&           elemRef,
                                                                                const DGNModelRefHolder&    modelRef);
    };


/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                Jean-Francois.Cote   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct STMElemSourceRef : private DGNElemSourceRefBase
    {
public:
    static SourceRef                    CreateFrom                             (const ElementRefP&           elemRef,
                                                                                const DGNModelRefHolder&    modelRef);

    };


/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                Jean-Francois.Cote   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct PODElemSourceRef : private DGNElemSourceRefBase
    {
public:
    static SourceRef                    CreateFrom                             (const ElementRefP&           elemRef,
                                                                                const DGNModelRefHolder&    modelRef);

    };

END_BENTLEY_SCALABLEMESH_NAMESPACE
