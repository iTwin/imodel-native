/*--------------------------------------------------------------------------------------+
|
|     $Source: Dwg/DwgDb/RxObjects.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    "DwgDbInternal.h"

USING_NAMESPACE_DWGDB

#ifdef DWGTOOLKIT_OpenDwg
ODRX_CONS_DEFINE_MEMBERS (DwgRxObject, OdRxObject, RXIMPL_CONSTR)

void                DwgRxObject::copyFrom (DWG_TypeCP(RxObject) other) { T_Super::copyFrom(other); }
OdRxObjectPtr       DwgRxObject::clone () const { return T_Super::clone(); }
OdRxObject*         DwgRxObject::CreateObject () { return DwgRxObject::createObject(); }

#elif DWGTOOLKIT_RealDwg
ACRX_CONS_DEFINE_MEMBERS (DwgRxObject, AcRxObject, 1)

Acad::ErrorStatus   DwgRxObject::copyFrom (DWG_TypeCP(RxObject) other) { return T_Super::copyFrom(other); }
AcRxObject*         DwgRxObject::clone () const { return T_Super::clone(); }
AcRxObject*         DwgRxObject::subQueryX (const AcRxClass* protocolClass) const { return T_Super::subQueryX(protocolClass); }
AcRxObject*         DwgRxObject::CreateObject () { return nullptr!=DwgRxObject::desc() ? DwgRxObject::desc()->create() : nullptr; }
#endif
DWG_Type(Rx::Ordering)  DwgRxObject::comparedTo (DWG_TypeCP(RxObject) other) const { return T_Super::comparedTo(other); }
DwgDbBool           DwgRxObject::isEqualTo (DWG_TypeCP(RxObject) other) const { return T_Super::isEqualTo(other); }

void                DwgRxObject::RxInit () { DwgRxObject::rxInit(); }
bool                DwgRxObject::IsEqualTo (DwgRxObjectCP other) const { return this->isEqualTo(static_cast<DWG_TypeCP(RxObject)>(other)); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool                DwgRxObject::IsKindOf (DWG_TypeCP(RxClass) other) const
    {
    if (other == DwgRxObject::desc())
        return true;
    return T_Super::isKindOf(static_cast<DWG_TypeCP(RxClass)>(other));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/16
+---------------+---------------+---------------+---------------+---------------+------*/
DWG_TypeP(RxClass)  DwgRxObject::Desc ()
    {
#ifdef DWGTOOLKIT_OpenDwg
    return g_pDesc;

#elif DWGTOOLKIT_RealDwg
    if (gpDesc != nullptr)
        return gpDesc;
    return gpDesc = (AcRxClass*)((AcRxDictionary*)acrxSysRegistry()->at(ACRX_CLASS_DICTIONARY))->at(L"DwgRxObject");
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus     DwgRxObject::CopyFrom (DwgRxObjectCP other)
    {
    DwgDbStatus status = DwgDbStatus::NotSupported;
#ifdef DWGTOOLKIT_OpenDwg

    T_Super::copyFrom (static_cast<const OdRxObject*>(other));
    status = DwgDbStatus::Success;

#elif DWGTOOLKIT_RealDwg

    Acad::ErrorStatus   es = T_Super::copyFrom (static_cast<const AcRxObject*>(other));
    status = ToDwgDbStatus(es);
#endif
    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/16
+---------------+---------------+---------------+---------------+---------------+------*/
DWG_TypeP(RxClass) DwgRxClass::QueryClassFromDictionary (WCharCP name)
    {
    DWG_TypeP(RxClass)  rxc = nullptr;
#ifdef DWGTOOLKIT_OpenDwg
    rxc = (OdRxClass*)::odrxClassDictionary()->getAt(name).get ();
#elif DWGTOOLKIT_RealDwg
    rxc = (AcRxClass*)((AcRxDictionary*)::acrxSysRegistry()->at(ACRX_CLASS_DICTIONARY))->at (name);
#endif
    return  rxc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/16
+---------------+---------------+---------------+---------------+---------------+------*/
DWG_TypeP(RxClass) DwgRxClass::NewDwgRxClass (WCharCP className, WCharCP parentName, int proxyFlags, PseudoConstructor constr)
    {
    DWG_TypeP(RxClass)  rxc = nullptr;
#ifdef DWGTOOLKIT_OpenDwg
    OdRxClass*  parentClass = (OdRxClass*)::odrxClassDictionary()->getAt(parentName).get ();
    if (nullptr != parentClass)
        rxc = ::newOdRxClass (className, parentClass, constr, 0, 0, proxyFlags);
#elif DWGTOOLKIT_RealDwg
    rxc = ::newAcRxClass (className, parentName, proxyFlags, constr);
#endif
    return  rxc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgRxObjectP    DwgRxClass::AddProtocolExtension (DWG_TypeP(RxClass) toClass, DWG_TypeP(RxClass) protocalClass, DWG_TypeP(RxObject) protocalObject)
    {
    if (nullptr != toClass && nullptr != protocalClass && nullptr != protocalObject)
        {
        DWG_TypeP(RxObject) priorAdded = toClass->addX (protocalClass, protocalObject);
        return  DwgRxObject::cast (priorAdded);
        }
    return  nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/16
+---------------+---------------+---------------+---------------+---------------+------*/
void    DwgRxClass::BuildClassHierarchy ()
    {
#ifdef DWGTOOLKIT_RealDwg
    acrxBuildClassHierarchy ();
#endif
    }
