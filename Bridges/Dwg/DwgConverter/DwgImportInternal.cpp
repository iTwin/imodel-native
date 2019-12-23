/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
// quiet static analysis warnings through toolkit's header files
#pragma warning(disable:6001)
#pragma warning(disable:6011)
#pragma warning(disable:6244)
#pragma warning(disable:6387)

#include "DwgImportInternal.h"

#pragma warning(default:6001)
#pragma warning(default:6011)
#pragma warning(default:6244)
#pragma warning(default:6387)

USING_NAMESPACE_DWG

// define entity protocol extension member
DWG_PROTOCOLEXT_DEFINE_MEMBERS(DwgBlockReferenceExt)
DWG_PROTOCOLEXT_DEFINE_MEMBERS(DwgBrepExt)
DWG_PROTOCOLEXT_DEFINE_MEMBERS(DwgHatchExt)
DWG_PROTOCOLEXT_DEFINE_MEMBERS(DwgLightExt)
DWG_PROTOCOLEXT_DEFINE_MEMBERS(DwgPointCloudExExt)
DWG_PROTOCOLEXT_DEFINE_MEMBERS(DwgRasterImageExt)
DWG_PROTOCOLEXT_DEFINE_MEMBERS(DwgViewportExt)


#ifdef DWGTOOLKIT_OpenDwg

#define DWG_STATIC_RXOBJECT(__className__)  OdStaticRxObject<__className__>
    
DWGRX_NO_CONS_DEFINE_MEMBERS(DwgProtocolExtension, DwgRxObject)

DWG_TypeP(RxClass)  DwgProtocolExtension::Desc() { return DwgProtocolExtension::desc(); }
DwgProtocolExtension* DwgProtocolExtension::Cast(DWG_TypeCP(RxObject) obj) { return (DwgProtocolExtension*)obj; }
void    DwgProtocolExtension::RxInit() { DwgProtocolExtension::rxInit(); }
void    DwgProtocolExtension::RxUnInit() { DwgProtocolExtension::rxUninit(); }

#else   // DWGTOOLKIT_RealDwg

#define DWG_STATIC_RXOBJECT(__className__)  __className__

DWG_TypeP(RxClass)  DwgProtocolExtension::gpDesc = nullptr;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/16
+---------------+---------------+---------------+---------------+---------------+------*/
DWG_TypeP(RxClass)  DwgProtocolExtension::desc ()
    {
    if (gpDesc != nullptr)
        return gpDesc;

    return gpDesc = DwgRxClass::QueryClassFromDictionary (NAME_DwgProtocolExtension);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/16
+---------------+---------------+---------------+---------------+---------------+------*/
DWG_TypeP(RxClass)  DwgProtocolExtension::Desc ()
    {
    return  DwgProtocolExtension::desc ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgProtocolExtension* DwgProtocolExtension::Cast (DWG_TypeCP(RxObject) obj)
    {
    DwgProtocolExtension* extensionObj = (DwgProtocolExtension*)obj;
    if (nullptr != extensionObj)
        {
        if (!extensionObj->IsKindOf(DwgProtocolExtension::Desc()))
            return  nullptr;
        }
        
    return extensionObj;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/16
+---------------+---------------+---------------+---------------+---------------+------*/
DWG_TypeP(RxClass)  DwgProtocolExtension::isA () const
    {
    if (gpDesc != nullptr)
        return gpDesc;

    return gpDesc = DwgRxClass::QueryClassFromDictionary (NAME_DwgProtocolExtension);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/16
+---------------+---------------+---------------+---------------+---------------+------*/
void        DwgProtocolExtension::rxInit ()
    {
    if (gpDesc != nullptr)
        {
        DWG_TypeP(RxClass) pClass = DwgRxClass::QueryClassFromDictionary (NAME_DwgProtocolExtension);
        if (pClass)
            {
            if (gpDesc == pClass)
                return;
            else
                BeAssert (false && L"DwgProtocolExtension class mismatch!");
            }
        }
    gpDesc = DwgRxClass::NewDwgRxClass (NAME_DwgProtocolExtension, L"DwgRxObject");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/16
+---------------+---------------+---------------+---------------+---------------+------*/
void        DwgProtocolExtension::RxInit ()
    {
    DwgProtocolExtension::rxInit ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/16
+---------------+---------------+---------------+---------------+---------------+------*/
void        DwgProtocolExtension::RxUnInit ()
    {
    }
#endif  // DWGTOOLKIT_


/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          12/19
+===============+===============+===============+===============+===============+======*/
struct DwgProtocolExtensionRegister
{
public:
    DWG_STATIC_RXOBJECT(DwgBlockReferenceExt)   m_dwgBlockReferenceExt;
    DWG_STATIC_RXOBJECT(DwgRasterImageExt)      m_dwgRasterImageExt;
    DWG_STATIC_RXOBJECT(DwgPointCloudExExt)     m_dwgPointCloudExExt;
    DWG_STATIC_RXOBJECT(DwgViewportExt)         m_dwgViewportExt;
    DWG_STATIC_RXOBJECT(DwgLightExt)            m_dwgLightExt;
    DWG_STATIC_RXOBJECT(DwgBrepExt)             m_dwgBrepExt;
    DWG_STATIC_RXOBJECT(DwgHatchExt)            m_dwgHatchExt;

// the constructor
DwgProtocolExtensionRegister ()
    {
    DwgProtocolExtension::RxInit ();
    DwgRasterImageExt::RxInit ();
    DwgPointCloudExExt::RxInit ();
    DwgViewportExt::RxInit ();
    DwgLightExt::RxInit ();
    DwgBrepExt::RxInit ();
    DwgBlockReferenceExt::RxInit ();
    DwgHatchExt::RxInit ();

    DwgRxClass::BuildClassHierarchy ();

    DWG_TypeP(RxClass) protocolClass = DwgProtocolExtension::Desc ();
    if (nullptr == protocolClass)
        {
        BeAssert (false && L"Protocol extension class not initialized!");
        return;
        }

    // add our protocal extensions to toolkit's classes(hence "SuperDesc"):
    DwgRxClass::AddProtocolExtension (DwgDbRasterImage::SuperDesc(), protocolClass, &m_dwgRasterImageExt);
    DwgRxClass::AddProtocolExtension (DwgDbPointCloudEx::SuperDesc(), protocolClass, &m_dwgPointCloudExExt);
    DwgRxClass::AddProtocolExtension (DwgDbViewport::SuperDesc(), protocolClass, &m_dwgViewportExt);
    DwgRxClass::AddProtocolExtension (DwgDbLight::SuperDesc(), protocolClass, &m_dwgLightExt);
    DwgRxClass::AddProtocolExtension (DwgDb3dSolid::SuperDesc(), protocolClass, &m_dwgBrepExt);
    DwgRxClass::AddProtocolExtension (DwgDbRegion::SuperDesc(), protocolClass, &m_dwgBrepExt);
    DwgRxClass::AddProtocolExtension (DwgDbBody::SuperDesc(), protocolClass, &m_dwgBrepExt);
    DwgRxClass::AddProtocolExtension (DwgDbSurface::SuperDesc(), protocolClass, &m_dwgBrepExt);
    DwgRxClass::AddProtocolExtension (DwgDbExtrudedSurface::SuperDesc(), protocolClass, &m_dwgBrepExt);
    DwgRxClass::AddProtocolExtension (DwgDbLoftedSurface::SuperDesc(), protocolClass, &m_dwgBrepExt);
    DwgRxClass::AddProtocolExtension (DwgDbNurbSurface::SuperDesc(), protocolClass, &m_dwgBrepExt);
    DwgRxClass::AddProtocolExtension (DwgDbRevolvedSurface::SuperDesc(), protocolClass, &m_dwgBrepExt);
    DwgRxClass::AddProtocolExtension (DwgDbPlaneSurface::SuperDesc(), protocolClass, &m_dwgBrepExt);
    DwgRxClass::AddProtocolExtension (DwgDbSweptSurface::SuperDesc(), protocolClass, &m_dwgBrepExt);
    DwgRxClass::AddProtocolExtension (DwgDbBlockReference::SuperDesc(), protocolClass, &m_dwgBlockReferenceExt);
    DwgRxClass::AddProtocolExtension (DwgDbHatch::SuperDesc(), protocolClass, &m_dwgHatchExt);
    }

// the destructor
~DwgProtocolExtensionRegister ()
    {
    DWG_TypeP(RxClass) protocolClass = DwgProtocolExtension::Desc ();
    DwgRxClass::DeleteProtocolExtension (DwgDbRasterImage::SuperDesc(), protocolClass);
    DwgRxClass::DeleteProtocolExtension (DwgDbPointCloudEx::SuperDesc(), protocolClass);
    DwgRxClass::DeleteProtocolExtension (DwgDbViewport::SuperDesc(), protocolClass);
    DwgRxClass::DeleteProtocolExtension (DwgDbLight::SuperDesc(), protocolClass);
    DwgRxClass::DeleteProtocolExtension (DwgDb3dSolid::SuperDesc(), protocolClass);
    DwgRxClass::DeleteProtocolExtension (DwgDbRegion::SuperDesc(), protocolClass);
    DwgRxClass::DeleteProtocolExtension (DwgDbBody::SuperDesc(), protocolClass);
    DwgRxClass::DeleteProtocolExtension (DwgDbSurface::SuperDesc(), protocolClass);
    DwgRxClass::DeleteProtocolExtension (DwgDbExtrudedSurface::SuperDesc(), protocolClass);
    DwgRxClass::DeleteProtocolExtension (DwgDbLoftedSurface::SuperDesc(), protocolClass);
    DwgRxClass::DeleteProtocolExtension (DwgDbNurbSurface::SuperDesc(), protocolClass);
    DwgRxClass::DeleteProtocolExtension (DwgDbRevolvedSurface::SuperDesc(), protocolClass);
    DwgRxClass::DeleteProtocolExtension (DwgDbPlaneSurface::SuperDesc(), protocolClass);
    DwgRxClass::DeleteProtocolExtension (DwgDbSweptSurface::SuperDesc(), protocolClass);
    DwgRxClass::DeleteProtocolExtension (DwgDbBlockReference::SuperDesc(), protocolClass);
    DwgRxClass::DeleteProtocolExtension (DwgDbHatch::SuperDesc(), protocolClass);

    DwgProtocolExtension::RxUnInit ();
    DwgRasterImageExt::RxUnInit ();
    DwgPointCloudExExt::RxUnInit ();
    DwgViewportExt::RxUnInit ();
    DwgLightExt::RxUnInit ();
    DwgBrepExt::RxUnInit ();
    DwgBlockReferenceExt::RxUnInit ();
    DwgHatchExt::RxUnInit ();
    }
};  // DwgProtocolExtensionRegister
static DwgProtocolExtensionRegister*   s_dwgProtocolExtensionRegister;


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/16
+---------------+---------------+---------------+---------------+---------------+------*/
void        DwgImporter::RegisterProtocolExtensions ()
    {
    s_dwgProtocolExtensionRegister = new DwgProtocolExtensionRegister ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/16
+---------------+---------------+---------------+---------------+---------------+------*/
void        DwgImporter::UnRegisterProtocolExtensions ()
    {
    if (s_dwgProtocolExtensionRegister != nullptr)
        {
        delete s_dwgProtocolExtensionRegister;
        s_dwgProtocolExtensionRegister = nullptr;
        }
    }
