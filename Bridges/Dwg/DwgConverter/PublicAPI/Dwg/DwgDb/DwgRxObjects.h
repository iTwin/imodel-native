/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include    <Dwg/DwgDb/DwgDbCommon.h>


// Protocol Extension Class name
#define NAME_DwgProtocolExtension   L"DwgProtocolExtension"

#ifdef DWGTOOLKIT_OpenDwg
typedef OdPseudoConstructorType     PseudoConstructor;
#elif DWGTOOLKIT_RealDwg
typedef AcRxObject*                 (*PseudoConstructor)();
#endif

BEGIN_DWGDB_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          06/16
+===============+===============+===============+===============+===============+======*/
class DwgRxObject : public DWGRX_EXTENDCLASS(Object)
    {
public:
    DEFINE_T_SUPER (DWGROOT_SUPER_CONSTRUCTOR(RxObject))
    DWGRX_DECLARE_MEMBERS(DwgRxObject)

    DWGDB_EXPORT DwgRxObject () : T_Super() { }
    
    DWGDB_EXPORT static DWG_TypeP(RxObject) CreateObject ();
    DWGDB_EXPORT static DWG_TypeP(RxClass)  Desc ();
    DWGDB_EXPORT static DwgRxObjectP        Cast (DWG_TypeCP(RxObject) rxObj);
    DWGDB_EXPORT static void                RxInit ();

    DWGDB_EXPORT virtual DwgDbStatus        CopyFrom (DwgRxObjectCP other);
    DWGDB_EXPORT virtual bool               IsEqualTo (DwgRxObjectCP other) const;
    DWGDB_EXPORT bool                       IsKindOf (DWG_TypeCP(RxClass) other) const;

//__PUBLISH_SECTION_END__
private:
    DWGDB_EXPORT virtual DwgDbBool              isEqualTo (DWG_TypeCP(RxObject) other) const override;
    DWGDB_EXPORT virtual DWG_Type(Rx::Ordering) comparedTo (DWG_TypeCP(RxObject) other) const override;
#ifdef DWGTOOLKIT_OpenDwg
    DWGDB_EXPORT virtual OdRxObjectPtr          clone () const override;
    DWGDB_EXPORT virtual void                   copyFrom (const OdRxObject* other) override;
#elif DWGTOOLKIT_RealDwg
    DWGDB_EXPORT virtual AcRxObject*            clone () const override;
    DWGDB_EXPORT virtual Acad::ErrorStatus      copyFrom (const AcRxObject* other) override;
    DWGDB_EXPORT virtual AcRxObject*            subQueryX (const AcRxClass* protocolClass) const override;
#endif  // DWGTOOLKIT_
//__PUBLISH_SECTION_START__
    };  // DwgRxObject

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          06/16
+===============+===============+===============+===============+===============+======*/
struct DwgRxClass
    {
    DWGDB_EXPORT static DWG_TypeP(RxClass) QueryClassFromDictionary (WCharCP name);
    DWGDB_EXPORT static DWG_TypeP(RxClass) NewDwgRxClass (WCharCP className, WCharCP parentName, int proxyFlags = 0, PseudoConstructor constr = nullptr);
    DWGDB_EXPORT static DwgRxObjectP       AddProtocolExtension (DWG_TypeP(RxClass) toClass, DWG_TypeP(RxClass) protocalClass, DWG_TypeP(RxObject) protocalObject);
    DWGDB_EXPORT static void               BuildClassHierarchy ();
    };

END_DWGDB_NAMESPACE
//__PUBLISH_SECTION_END__
