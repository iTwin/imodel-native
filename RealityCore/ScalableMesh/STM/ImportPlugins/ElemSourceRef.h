/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

        

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE


struct DGNModelRefHolder;

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                Raymond.Gauthier     07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct DGNElemSourceRefBase : public BENTLEY_NAMESPACE_NAME::ScalableMesh::Import::Plugin::V0::DGNElementSourceRefBase
    {
private:
    struct Impl;
    std::auto_ptr<Impl>                 m_implP;

    explicit                            DGNElemSourceRefBase                   (const DGNElemSourceRefBase& rhs);

    virtual DGNElemSourceRefBase*       _Clone                                 () const override;


    virtual uint32_t                        _GetElementType                        () const override;
    virtual uint32_t                        _GetElementHandlerID                   () const override;

    virtual ElementRefP                  _GetElementRef                         () const override;

    virtual DgnModelRefP                _GetModelRef                           () const override;

    virtual const LocalFileSourceRef*   _GetLocalFileP                         () const override;


    explicit                            DGNElemSourceRefBase                   (Impl*                       implP);

protected:
    typedef BENTLEY_NAMESPACE_NAME::ScalableMesh::Import::SourceRef
                                        SourceRef;

    static SourceRef                    CreateFromImpl                         (uint32_t                        elemType,
                                                                                uint32_t                        elemHandlerID,
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
