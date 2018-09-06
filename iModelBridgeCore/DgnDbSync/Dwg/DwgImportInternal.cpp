/*--------------------------------------------------------------------------------------+
|
|     $Source: Dwg/DwgImportInternal.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DwgImportInternal.h"

USING_NAMESPACE_DGNDBSYNC_DWG

#ifdef DWGTOOLKIT_OpenDwg
#define gpDesc      g_pDesc
#endif

DWG_TypeP(RxClass)  DwgProtocalExtension::gpDesc = nullptr;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/16
+---------------+---------------+---------------+---------------+---------------+------*/
DWG_TypeP(RxClass)  DwgProtocalExtension::desc ()
    {
    if (gpDesc != nullptr)
        return gpDesc;

    return gpDesc = DwgRxClass::QueryClassFromDictionary (NAME_DwgProtocalExtension);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/16
+---------------+---------------+---------------+---------------+---------------+------*/
DWG_TypeP(RxClass)  DwgProtocalExtension::Desc ()
    {
    return  DwgProtocalExtension::desc ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgProtocalExtension* DwgProtocalExtension::Cast (DWG_TypeCP(RxObject) obj)
    {
    DwgProtocalExtension* extensionObj = (DwgProtocalExtension*)obj;
    if (nullptr != extensionObj)
        {
#ifdef DWGTOOLKIT_RealDwg
        if (!extensionObj->IsKindOf(DwgProtocalExtension::Desc()))
            return  nullptr;
#endif
        }
        
    return extensionObj;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/16
+---------------+---------------+---------------+---------------+---------------+------*/
DWG_TypeP(RxClass)  DwgProtocalExtension::isA () const
    {
    if (gpDesc != nullptr)
        return gpDesc;

    return gpDesc = DwgRxClass::QueryClassFromDictionary (NAME_DwgProtocalExtension);
    }

#ifdef DWGTOOLKIT_OpenDwg
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/16
+---------------+---------------+---------------+---------------+---------------+------*/
DWG_TypeP(RxObject) DwgProtocalExtension::queryX (DWG_TypeCP(RxClass) obj) const
    {
    return obj->queryX (DwgProtocalExtension::desc());
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/16
+---------------+---------------+---------------+---------------+---------------+------*/
void        DwgProtocalExtension::rxInit ()
    {
    if (gpDesc != nullptr)
        {
        DWG_TypeP(RxClass) pClass = DwgRxClass::QueryClassFromDictionary (NAME_DwgProtocalExtension);
        if (pClass)
            {
            if (gpDesc == pClass)
                return;
            else
                BeAssert (false && L"DwgProtocalExtension class mismatch!");
            }
        }
    gpDesc = DwgRxClass::NewDwgRxClass (NAME_DwgProtocalExtension, L"DwgRxObject");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/16
+---------------+---------------+---------------+---------------+---------------+------*/
void        DwgProtocalExtension::RxInit ()
    {
    DwgProtocalExtension::rxInit ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/16
+---------------+---------------+---------------+---------------+---------------+------*/
void        DwgImporter::RegisterProtocalExtensions ()
    {
    DwgProtocalExtension::RxInit ();
    DwgRasterImageExt::RxInit ();
    DwgPointCloudExExt::RxInit ();
    DwgViewportExt::RxInit ();
    DwgLightExt::RxInit ();
    DwgBrepExt::RxInit ();

    DwgRxClass::BuildClassHierarchy ();

    DWG_TypeP(RxClass) protocalClass = DwgProtocalExtension::Desc ();
    if (nullptr == protocalClass)
        {
        BeAssert (false && L"Protocal extension class not initialized!");
        return;
        }

    // add our protocal extensions to toolkit's classes(hence "SuperDesc"):
    DwgRxClass::AddProtocolExtension (DwgDbRasterImage::SuperDesc(), protocalClass, DwgRasterImageExt::CreateObject());
    DwgRxClass::AddProtocolExtension (DwgDbPointCloudEx::SuperDesc(), protocalClass, DwgPointCloudExExt::CreateObject());
    DwgRxClass::AddProtocolExtension (DwgDbViewport::SuperDesc(), protocalClass, DwgViewportExt::CreateObject());
    DwgRxClass::AddProtocolExtension (DwgDbLight::SuperDesc(), protocalClass, DwgLightExt::CreateObject());
    DwgRxClass::AddProtocolExtension (DwgDb3dSolid::SuperDesc(), protocalClass, DwgBrepExt::CreateObject());
    DwgRxClass::AddProtocolExtension (DwgDbRegion::SuperDesc(), protocalClass, DwgBrepExt::CreateObject());
    DwgRxClass::AddProtocolExtension (DwgDbBody::SuperDesc(), protocalClass, DwgBrepExt::CreateObject());
    // WIP - add surface entities
    //DwgRxClass::AddProtocolExtension (DwgDbSurface::SuperDesc(), protocalClass, DwgBrepExt::CreateObject());
    }

