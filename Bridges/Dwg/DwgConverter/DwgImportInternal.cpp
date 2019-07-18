/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
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

#ifdef DWGTOOLKIT_OpenDwg
#define gpDesc      g_pDesc
#endif

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
#ifdef DWGTOOLKIT_RealDwg
        if (!extensionObj->IsKindOf(DwgProtocolExtension::Desc()))
            return  nullptr;
#endif
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

#ifdef DWGTOOLKIT_OpenDwg
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/16
+---------------+---------------+---------------+---------------+---------------+------*/
DWG_TypeP(RxObject) DwgProtocolExtension::queryX (DWG_TypeCP(RxClass) obj) const
    {
    return obj->queryX (DwgProtocolExtension::desc());
    }
#endif

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
void        DwgImporter::RegisterProtocolExtensions ()
    {
    DwgProtocolExtension::RxInit ();
    DwgRasterImageExt::RxInit ();
    DwgPointCloudExExt::RxInit ();
    DwgViewportExt::RxInit ();
    DwgLightExt::RxInit ();
    DwgBrepExt::RxInit ();
    DwgBlockReferenceExt::RxInit ();

    DwgRxClass::BuildClassHierarchy ();

    DWG_TypeP(RxClass) protocolClass = DwgProtocolExtension::Desc ();
    if (nullptr == protocolClass)
        {
        BeAssert (false && L"Protocol extension class not initialized!");
        return;
        }

    // add our protocal extensions to toolkit's classes(hence "SuperDesc"):
    DwgRxClass::AddProtocolExtension (DwgDbRasterImage::SuperDesc(), protocolClass, DwgRasterImageExt::CreateObject());
    DwgRxClass::AddProtocolExtension (DwgDbPointCloudEx::SuperDesc(), protocolClass, DwgPointCloudExExt::CreateObject());
    DwgRxClass::AddProtocolExtension (DwgDbViewport::SuperDesc(), protocolClass, DwgViewportExt::CreateObject());
    DwgRxClass::AddProtocolExtension (DwgDbLight::SuperDesc(), protocolClass, DwgLightExt::CreateObject());
    DwgRxClass::AddProtocolExtension (DwgDb3dSolid::SuperDesc(), protocolClass, DwgBrepExt::CreateObject());
    DwgRxClass::AddProtocolExtension (DwgDbRegion::SuperDesc(), protocolClass, DwgBrepExt::CreateObject());
    DwgRxClass::AddProtocolExtension (DwgDbBody::SuperDesc(), protocolClass, DwgBrepExt::CreateObject());
    DwgRxClass::AddProtocolExtension (DwgDbSurface::SuperDesc(), protocolClass, DwgBrepExt::CreateObject());
    DwgRxClass::AddProtocolExtension (DwgDbExtrudedSurface::SuperDesc(), protocolClass, DwgBrepExt::CreateObject());
    DwgRxClass::AddProtocolExtension (DwgDbLoftedSurface::SuperDesc(), protocolClass, DwgBrepExt::CreateObject());
    DwgRxClass::AddProtocolExtension (DwgDbNurbSurface::SuperDesc(), protocolClass, DwgBrepExt::CreateObject());
    DwgRxClass::AddProtocolExtension (DwgDbRevolvedSurface::SuperDesc(), protocolClass, DwgBrepExt::CreateObject());
    DwgRxClass::AddProtocolExtension (DwgDbPlaneSurface::SuperDesc(), protocolClass, DwgBrepExt::CreateObject());
    DwgRxClass::AddProtocolExtension (DwgDbSweptSurface::SuperDesc(), protocolClass, DwgBrepExt::CreateObject());
    DwgRxClass::AddProtocolExtension (DwgDbBlockReference::SuperDesc(), protocolClass, DwgBlockReferenceExt::CreateObject());
    }

