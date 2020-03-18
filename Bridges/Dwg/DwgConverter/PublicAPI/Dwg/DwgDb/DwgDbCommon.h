/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
//__PUBLISH_SECTION_START__
#if !defined (DWGTOOLKIT_OpenDwg) && !defined (DWGTOOLKIT_RealDwg)
    #error  "Must define a valid DWGTOOLKIT(i.e. OpenDwg or RealDwg)!!!"
#endif

#pragma once

#include    <stdarg.h>                      // va_list
#include    <iostream>

#include    <Bentley/Bentley.h>

#define DWGDB_NAMESPACE_NAME    DwgDb
#define BEGIN_DWGDB_NAMESPACE   namespace BENTLEY_NAMESPACE_NAME { namespace DWGDB_NAMESPACE_NAME {
#define END_DWGDB_NAMESPACE     } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_DWGDB   using namespace BentleyApi::DwgDb;


#ifndef UNICODE
    #define UNICODE
#endif

#ifdef DWGTOOLKIT_OpenDwg

// OdPlatform.h expects WIN64 defined or GiContextualColors.h would fail!
#if defined(_X64_) && defined(WIN32) && !defined(WIN64)
    #define WIN64
#endif
// OdPlatform.h explicitly defines WIN32_LEAN_AND_MEAN since v2021.x
#if defined(WIN32_LEAN_AND_MEAN)
    #undef WIN32_LEAN_AND_MEAN
#endif

#include    <Teigha/KernelBase/Include/OdaCommon.h>
#include    <Teigha/Drawing/Include/DbClass.h>
#include    <Teigha/Drawing/Include/DbObjectId.h>
#include    <Teigha/Drawing/Include/DbDatabase.h>
#include    <Teigha/Drawing/Include/DbLayoutManager.h>
#include    <Teigha/Drawing/Include/DbGroup.h>
#include    <Teigha/Drawing/Include/DbXrecord.h>
#include    <Teigha/Drawing/Include/DbXrefGraph.h>
#include    <Teigha/Drawing/Include/DbExtrudedSurface.h>
#include    <Teigha/Drawing/Include/DbLoftedSurface.h>
#include    <Teigha/Drawing/Include/DbNurbSurface.h>
#include    <Teigha/Drawing/Include/DbPlaneSurface.h>
#include    <Teigha/Drawing/Include/DbRevolvedSurface.h>
#include    <Teigha/Drawing/Include/DbSweptSurface.h>
#include    <Teigha/Drawing/Include/ModelDocObj/DbViewBorder.h>
#include    <Teigha/Drawing/Include/ModelDocObj/DbViewRepBlockReference.h>
#include    <Teigha/KernelBase/Include/OdString.h>
#include    <Teigha/Kernel/Include/RxObject.h>
#include    <Teigha/Drawing/Extensions/ExServices/ExHostAppServices.h>
#include    <Teigha/Kernel/Extensions/ExServices/ExSystemServices.h>

#elif DWGTOOLKIT_RealDwg

#define     _AFXDLL
#include    <afxole.h>                  // UNREFERENCED_PARAMETER
#include    <Wingdi.h>                  // BITMAPINFO

#include    <Realdwg/Base/adesk.h>
#include    <Realdwg/Base/acadstrc.h>
#include    <Realdwg/Base/dbmain.h>
#include    <Realdwg/Base/dbents.h>
#include    <RealDwg/Base/dbbody.h>
#include    <RealDwg/Base/dbsol3d.h>
#include    <RealDwg/Base/dbregion.h>
#include    <RealDwg/Base/dbgroup.h>
#include    <RealDwg/Base/dbextrudedsurf.h>
#include    <RealDwg/Base/dbloftedsurf.h>
#include    <RealDwg/Base/dbnurbsurf.h>
#include    <RealDwg/Base/dbrevolvedsurf.h>
#include    <RealDwg/Base/dbplanesurf.h>
#include    <RealDwg/Base/dbsweptsurf.h>
#include    <Realdwg/Base/dbapserv.h>
#include    <Realdwg/Base/rxclass.h>
#include    <Realdwg/Base/rxobject.h>
#include    <Realdwg/Base/rxevent.h>
#include    <Realdwg/Base/dbsymtb.h>
#include    <Realdwg/Base/dbsymutl.h>
#include    <RealDwg/Base/dbboiler.h>
#include    <Realdwg/Base/dbobjptr2.h>
#include    <Realdwg/Base/dbxrecrd.h>
#include    <RealDwg/Base/dbViewBorder.h>
#include    <RealDwg/Base/dbViewRepBlockReference.h>
#include    <RealDwg/Base/AcDbLMgr.h>
#include    <RealDwg/Base/AcString.h>
#include    <RealDwg/Base/idver.h>

#else
    #error  "Must define DWGTOOLKIT!!!"
#endif


#ifndef DWGDB_EXPORT
    #ifdef __DWGDB_BUILD__
    #define DWGDB_EXPORT    EXPORT_ATTRIBUTE
    #endif
#endif

#ifndef DWGDB_EXPORT
    #define DWGDB_EXPORT    IMPORT_ATTRIBUTE
#endif


#ifdef DWGTOOLKIT_OpenDwg

    #define DWGDB_SDKNAME(_opendwgName_,_realdwgName_)      _opendwgName_

    // extend OpenDwg classes to DwgDb classes
    #define DWGDB_SDKCLASSNAME(_classSuffix_) OdDb##_classSuffix_
    #define DWGDB_EXTENDCLASS(_classSuffix_) DWGDB_SDKCLASSNAME(_classSuffix_)
    #define DWGDB_EXTENDCLASS2(_opendwgSuffix_, _realdwgSuffix_) DWGDB_EXTENDCLASS(_opendwgSuffix_)
    #define DWGRX_EXTENDCLASS(_classSuffix_) OdRx##_classSuffix_
    #define DWGGI_EXTENDCLASS(_classSuffix_) OdGi##_classSuffix_
    #define DWGROOTCLASS_EXTEND(_classSuffix_) Od##_classSuffix_
    #define DWGROOTCLASS_EXTEND2(_opendwgSuffix_, _realdwgSuffix_) DWGROOTCLASS_EXTEND(_opendwgSuffix_)
    #define DWGDB_EXTENDSMARTPTR(_classSuffix_) OdSmartPtr<Dwg##_classSuffix_##>
    #define DWGDB_EXTENDSMARTPTR2(_opendwgSuffix_, _realdwgSuffix_) DWGDB_EXTENDSMARTPTR(_opendwgSuffix_)
    #define DWGROOT_SUPER_CONSTRUCTOR(_classSuffix_) Od##_classSuffix_
    #define DWGDB_SDKENUM_DB(_name_) OdDb::##_name_

    #define DWG_Type(_type_)                Od##_type_
    #define DWGCM_Type(_cmType_)            OdCm##_cmType_
    #define DWGDB_Type(_dbType_)            OdDb##_dbType_
    #define DWGGE_Type(_geomType_)          OdGe##_geomType_
    #define DWGGI_Type(_giType_)            OdGi##_giType_
    #define DWGSTR_NAME(_name_)             ODDB_##_name_

    // call OpenDwg method
    #define DWGDB_CALLSDKMETHOD(_opendwgMethod_, _realdwgMethod_) _opendwgMethod_

    // constructors needed to convert from the super class to the DwgDb class:
    #define DWGROOTCLASS_ADD_CONSTRUCTORS(_classSuffix_)                                        \
        DWGDB_EXPORT Dwg##_classSuffix_## () : Od##_classSuffix_## () { ; }                     \
        DWGDB_EXPORT Dwg##_classSuffix_## (const Od##_classSuffix_##& _arg_) : Od##_classSuffix_##(_arg_) { ; }
    #define DWGDB_ADD_CONSTRUCTORS(_classSuffix_)   DWGROOTCLASS_ADD_CONSTRUCTORS(Db##_classSuffix_)
    #define DWGDB_ADD_SMARTPTR_CONSTRUCTORS(_classSuffix_)                                      \
        DWGDB_EXPORT Dwg##_classSuffix_##Ptr () : OdSmartPtr<Dwg##_classSuffix_##> () { ; }     \
        DWGDB_EXPORT Dwg##_classSuffix_##Ptr (const Od##_classSuffix_##Ptr& _arg_) : OdSmartPtr<Dwg##_classSuffix_##>(##_arg_) { ; } \
        DWGDB_EXPORT Dwg##_classSuffix_##Ptr (const Od##_classSuffix_##* _arg_) : OdSmartPtr<Dwg##_classSuffix_##>(##_arg_) { ; }

    #define DWGDB_DECLARE_MEMBERS(_className_)          ODDB_DECLARE_MEMBERS(##_className_##);
    #define DWGDB_PSEUDO_DECLARE_MEMBERS(_className_)   ODDB_PSEUDO_DECLARE_MEMBERS(##_className_##);

    #define DWGRX_DECLARE_MEMBERS(_className_)          ODRX_DECLARE_MEMBERS(##_className_##);
    #define DWGRX_DECLARE_MEMBERS_EXPIMP(_className_,_expimp_)              \
        public:                                                             \
        _expimp_ static OdSmartPtr<_className_>cast(const OdRxObject* obj)  \
            {                                                               \
            if (obj != nullptr)                                             \
                return OdSmartPtr<_className_>(((_className_*)obj->queryX(_className_::desc())), kOdRxObjAttach);   \
            return (_className_*)nullptr;                                   \
            }                                                               \
        static OdRxClass* g_pDesc;                                          \
        _expimp_ static OdRxClass* desc();                                  \
        _expimp_ virtual OdRxClass* isA() const;                            \
        _expimp_ virtual OdRxObject* queryX(const OdRxClass* protocolClass) const;  \
        _expimp_ static OdRxObjectPtr pseudoConstructor();                  \
        _expimp_ static OdSmartPtr<_className_> createObject()              \
            {                                                               \
                if (!desc())                                                \
                    throw OdError(eNotInitializedYet);                      \
                return desc()->create();                                    \
            }                                                               \
        _expimp_ static void rxInit();                                      \
        _expimp_ static void rxUninit();                                    \
        _expimp_ static void rxInit(AppNameChangeFuncPtr appNameChangeFunc);

    #define DWGRX_DECLARE_RX_MEMBERS(_className_)                               \
        DWGDB_EXPORT static OdRxClass*      Desc ();                            \
        DWGDB_EXPORT static OdRxClass*      SuperDesc ();                       \
        DWGDB_EXPORT static OdRxObjectPtr   CreateObject ();                    \
        DWGDB_EXPORT static _className_*    Cast (OdRxObject const* rxObj);     \
        DWGDB_EXPORT OdRxObject*            QueryX (OdRxClass const* c) const;  \
        DWGDB_EXPORT OdRxObject*            GetX (OdRxClass const* c) const;    \
        DWGDB_EXPORT OdRxClass*             IsA () const;

    #define DWGRX_DEFINE_SMARTPTR_BASE()    \
        void addRef() {;}                   \
        void release() {;}
    #define DWGRX_CONS_DEFINE_MEMBERS(_class_,_parent_)     ODRX_CONS_DEFINE_MEMBERS(##_class_##,##_parent_##,RXIMPL_CONSTR)
    #define DWGRX_NO_CONS_DEFINE_MEMBERS(_class_,_parent_)  ODRX_NO_CONS_DEFINE_MEMBERS(##_class_##,##_parent_##)

    #define DWGDB_IsTrue(_boolValue_)       _boolValue_

    // major release number, e.g. 3, 4, 19 etc
    #define DWGDB_ToolkitMajorRelease       TD_MAJOR_VERSION
    // manior release number, e.g. 3, 8, etc
    #define DWGDB_ToolkitMinorRelease       TD_MINOR_VERSION
    // ODA DLL version suffix, e.g. "4.3_1", "19.8_15", etc
    #define DWGDB_ToolkitDllSuffix          TD_DLL_VERSION_SUFFIX_STR

#elif DWGTOOLKIT_RealDwg

    #define DWGDB_SDKNAME(_opendwgName_,_realdwgName_)      _realdwgName_

    // extend RealDwg classes to DwgDb classes
    #define DWGDB_SDKCLASSNAME(_classSuffix_) AcDb##_classSuffix_
    #define DWGDB_EXTENDCLASS(_classSuffix_) DWGDB_SDKCLASSNAME(_classSuffix_)
    #define DWGDB_EXTENDCLASS2(_opendwgSuffix_, _realdwgSuffix_) DWGDB_EXTENDCLASS(_realdwgSuffix_)
    #define DWGRX_EXTENDCLASS(_classSuffix_) AcRx##_classSuffix_
    #define DWGGI_EXTENDCLASS(_classSuffix_) AcGi##_classSuffix_
    #define DWGROOTCLASS_EXTEND(_classSuffix_) Ac##_classSuffix_
    #define DWGROOTCLASS_EXTEND2(_opendwgSuffix_, _realdwgSuffix_) DWGROOTCLASS_EXTEND(_realdwgSuffix_)
    #define DWGDB_EXTENDSMARTPTR(_classSuffix_) AcDbSmartObjectPointer<Dwg##_classSuffix_##>
    #define DWGDB_EXTENDSMARTPTR2(_opendwgSuffix_, _realdwgSuffix_) DWGDB_EXTENDSMARTPTR(_realdwgSuffix_)
    #define DWGROOT_SUPER_CONSTRUCTOR(_classSuffix_) Ac##_classSuffix_
    #define DWGDB_SDKENUM_DB(_name_) AcDb::##_name_

    #define DWG_Type(_type_)                Ac##_type_
    #define DWGCM_Type(_cmType_)            AcCm##_cmType_
    #define DWGDB_Type(_dbType_)            AcDb##_dbType_
    #define DWGGE_Type(_geomType_)          AcGe##_geomType_
    #define DWGGI_Type(_giType_)            AcGi##_giType_
    #define DWGSTR_NAME(_name_)             ACDB_##_name_

    // call RealDwg method
    #define DWGDB_CALLSDKMETHOD(_opendwgMethod_, _realdwgMethod_) _realdwgMethod_

    // constructors needed to convert from the super class to the DwgDb class:
    #define DWGROOTCLASS_ADD_CONSTRUCTORS(_classSuffix_)                                        \
        DWGDB_EXPORT Dwg##_classSuffix_## () : Ac##_classSuffix_## () { ; }                     \
        DWGDB_EXPORT Dwg##_classSuffix_## (const Ac##_classSuffix_##& _arg_) : Ac##_classSuffix_##(_arg_) { ; }
    #define DWGDB_ADD_CONSTRUCTORS(_classSuffix_)   DWGROOTCLASS_ADD_CONSTRUCTORS(Db##_classSuffix_)
    // copy/assignment is not allowed by AcDbObjectPointer!
    #define DWGDB_ADD_SMARTPTR_CONSTRUCTORS(_classSuffix_)                                      \
        DWGDB_EXPORT Dwg##_classSuffix_##Ptr () : AcDbSmartObjectPointer<Dwg##_classSuffix_##> () { ; }

    #define DWGDB_DECLARE_MEMBERS(_className_)          ACDB_DECLARE_MEMBERS(##_className_##);

    #define DWGDB_PSEUDO_DECLARE_MEMBERS(_className_)                   \
        private:                                                        \
            friend class AcDbSystemInternals;                           \
        protected:                                                      \
            ##_className_##(AcDbSystemInternals*);                      \
        public:                                                         \
            ACRX_DECLARE_MEMBERS_EXPIMP(##_className_##,DWGDB_EXPORT);

    #define DWGRX_DECLARE_MEMBERS(_className_)          ACRX_DECLARE_MEMBERS(##_className_##);
    #define DWGRX_DECLARE_MEMBERS_EXPIMP(_className_,_expimp_)  ACRX_DECLARE_MEMBERS_EXPIMP(##_className_##,##_expimp_##);

    #define DWGRX_DECLARE_RX_MEMBERS(_className_)                               \
        DWGDB_EXPORT static AcRxClass*      Desc ();                            \
        DWGDB_EXPORT static AcRxClass*      SuperDesc ();                       \
        DWGDB_EXPORT static AcRxObject*     CreateObject ();                    \
        DWGDB_EXPORT static _className_*    Cast (AcRxObject const* rxObj);     \
        DWGDB_EXPORT AcRxObject*            QueryX (AcRxClass const* c) const;  \
        DWGDB_EXPORT AcRxObject*            GetX (AcRxClass const* c) const;    \
        DWGDB_EXPORT AcRxClass*             IsA () const;
    // OpenDWG specific
    #define DWGRX_DEFINE_SMARTPTR_BASE()
    #define DWGRX_CONS_DEFINE_MEMBERS(_class_,_parent_)     ACRX_CONS_DEFINE_MEMBERS(##_class_##,##_parent_##,1);
    #define DWGRX_NO_CONS_DEFINE_MEMBERS(_class_,_parent_)  ACRX_NO_CONS_DEFINE_MEMBERS(##_class_##,##_parent_##);

    #define DWGDB_IsTrue(_boolValue_)       Adesk::kTrue == ##_boolValue_

    // major release number, e.g. 22, 23, etc
    #define DWGDB_ToolkitMajorRelease       ACADV_RELMAJOR
    // manior release number, e.g. 0, 1, etc
    #define DWGDB_ToolkitMinorRelease       ACADV_RELMINOR
    // RealDWG core DLL version suffix, e.g. "22", "23", etc
    #define DWGDB_ToolkitDllSuffix          ACRX_T(ID2STR(ACADV_RELMAJOR))

#endif  // DWGTOOLKIT


// declare super constructor of a DwgDbXxxx class
#define DWGDB_SUPER_CONSTRUCTOR(_classSuffix_)          DWGROOT_SUPER_CONSTRUCTOR(Db##_classSuffix_##)

// declare SmartPtr access methods
#define DWGDB_OVERRIDE_SMARTPTR_INTERFACE(_classSuffix_)                                                                        \
    virtual bool _IsNull () const override;                                                                                     \
    virtual Dwg##_classSuffix_##CP _Get () const override;                                                                      \
    virtual Dwg##_classSuffix_##P _Get () override;                                                                             \
    virtual DwgDbStatus _OpenObject(DwgDbObjectId id,DwgDbOpenMode mode,bool openErased=false, bool openLocked=false) override; \
    virtual DwgDbStatus _AcquireObject (Dwg##_classSuffix_## *& obj) override;                                                  \
    virtual DwgDbStatus _CreateObject () override;                                                                              \
    virtual DwgDbStatus _CloseObject () override;

#define DWGDB_PUBLIC_SMARTPTR_METHODS(_classSuffix_)                                                                            \
    DWGDB_EXPORT bool IsNull () const;                                                                                          \
    DWGDB_EXPORT Dwg##_classSuffix_##CP get () const;                                                                           \
    DWGDB_EXPORT Dwg##_classSuffix_##P get ();                                                                                  \
    DWGDB_EXPORT DwgDbStatus OpenObject (DwgDbObjectId id,DwgDbOpenMode mode,bool openErased=false, bool openLocked=false);     \
    DWGDB_EXPORT DwgDbStatus AcquireObject (Dwg##_classSuffix_## *& obj);                                                       \
    DWGDB_EXPORT DwgDbStatus CloseObject ();

// declare common methods of a DbObject derivative which may be used by our consumers:
#define DWGDB_OBJECT_DECLARE_MEMBERS(_className_)                                       \
    DWGRX_DECLARE_RX_MEMBERS(##_className_##)                                           \
    DWGDB_EXPORT _className_ ();                                                        \
    DWGDB_EXPORT DwgDbObjectId      GetObjectId () const;                               \
    DWGDB_EXPORT DwgDbObjectId      GetOwnerId () const;                                \
    DWGDB_EXPORT DwgDbDatabasePtr   GetDatabase () const;                               \
    DWGDB_EXPORT DwgDbStatus        UpgradeOpen ();                                     \
    DWGDB_EXPORT DwgDbStatus        DowngradeOpen ();                                   \
    DWGDB_EXPORT DwgDbStatus        Close ();                                           \
    DWGDB_EXPORT DwgDbStatus        Erase ();                                           \
    DWGDB_EXPORT DwgString          GetDxfName () const;                                \
    DWGDB_EXPORT DwgString          GetDwgClassName () const;                           \
    DWGDB_EXPORT DwgResBufIterator  GetXData (DwgStringCR name=DwgString()) const;      \
    DWGDB_EXPORT DwgDbStatus        DxfOutFields (IDxfFilerR filer) const;              \
    DWGDB_EXPORT DwgDbStatus        DxfOut (IDxfFilerR filer) const;                    \
    DWGDB_EXPORT DwgDbStatus        CreateExtensionDictionary ();                       \
    DWGDB_EXPORT DwgDbStatus        ReleaseExtensionDictionary ();                      \
    DWGDB_EXPORT DwgDbObjectId      GetExtensionDictionary () const;                    \
    DWGDB_EXPORT bool               HasPersistentReactor (DwgDbObjectIdCR id) const;    \
    DWGDB_EXPORT DwgDbObjectIdArray GetPersistentReactors () const;                     \
    DWGDB_EXPORT DwgDbStatus        SetBinaryData (DwgStringCR, size_t, const char*);   \
    DWGDB_EXPORT DwgDbStatus        GetBinaryData (DwgStringCR, size_t&, char*&) const;

// declare static method Create, which instantiates a super new DbObject for the purpose of saving to DWG
#define DWGDB_OBJECT_DECLARE_CREATE(_classSuffix_)                                      \
    DWGDB_EXPORT static DwgDb##_classSuffix_##* Create ();

// declare all common DwgDbXxxxx members derived from toolkit's DbObjects, but no static Create method
#define DWGDB_DECLARE_BASECLASS_MEMBERS(_classSuffix_)                                  \
    DEFINE_T_SUPER (DWGDB_SUPER_CONSTRUCTOR(##_classSuffix_##))                         \
    DWGDB_PSEUDO_DECLARE_MEMBERS (DwgDb##_classSuffix_##)                               \
    DWGDB_OBJECT_DECLARE_MEMBERS (DwgDb##_classSuffix_##)

// declare all common DwgDbXxxxx members derived from toolkit's DbObjects, with the static Create method
#define DWGDB_DECLARE_COMMON_MEMBERS(_classSuffix_)                                     \
    DWGDB_DECLARE_BASECLASS_MEMBERS(_classSuffix_)                                      \
    DWGDB_OBJECT_DECLARE_CREATE(_classSuffix_)

// define a common DwgDbXxxxPtr
#define DWGDB_DEFINE_OBJECTPTR(_classSuffix_)                                                                                           \
    class DwgDb##_classSuffix_##Ptr : public DWGDB_EXTENDSMARTPTR(Db##_classSuffix_##), public IDwgDbSmartPtr<DwgDb##_classSuffix_##>   \
        {                                                                                                                               \
        friend class DwgDbObjectId;                                                                                                     \
        protected:                                                                                                                      \
        DWGDB_OVERRIDE_SMARTPTR_INTERFACE (Db##_classSuffix_##)                                                                         \
        public:                                                                                                                         \
        DWGDB_PUBLIC_SMARTPTR_METHODS (Db##_classSuffix_##)                                                                             \
        DWGDB_ADD_SMARTPTR_CONSTRUCTORS (Db##_classSuffix_##)                                                                           \
        DWGDB_EXPORT DwgDb##_classSuffix_##Ptr (DwgDbObjectId id, DwgDbOpenMode mode = DwgDbOpenMode::ForRead, bool openErased = false, \
                                                bool openLocked = false);                                                               \
        DWGDB_EXPORT DwgDb##_classSuffix_##Ptr (DwgDb##_classSuffix_##* obj);                                                           \
        DWGDB_EXPORT DwgDb##_classSuffix_##Ptr& operator = (DwgDb##_classSuffix_##* obj);                                               \
        };

// type conversion casts
#define DWGDB_DOWNWARDCAST(_classSuffix_)               static_cast <DWGDB_SDKCLASSNAME(_classSuffix_)>
#define DWGDB_UPWARDCAST(_classSuffix_)                 static_cast <DwgDb##_classSuffix_##>
#define DWGDB_CASTTOENUM_DB(_enumName_)                 static_cast <DWGDB_SDKENUM_DB(##_enumName_##)>
#define DWGDB_CASTFROMENUM_DB(_enumName_)               DWGDB_UPWARDCAST(##_enumName_##)

#define DEFINE_NO_NAMESPACE_TYPEDEFS(_className_)                           \
    typedef _className_* _className_##P, &_className_##R;                   \
    typedef _className_ const* _className_##CP;                             \
    typedef _className_ const& _className_##CR;

#define DEFINE_DWGDB_TYPEDEFS(_className_)                                  \
    BEGIN_BENTLEY_NAMESPACE namespace DWGDB_NAMESPACE_NAME {                \
    class _className_;                                                      \
    DEFINE_NO_NAMESPACE_TYPEDEFS (##_className_##)                          \
    END_DWGDB_NAMESPACE

DEFINE_DWGDB_TYPEDEFS (DwgString)
DEFINE_DWGDB_TYPEDEFS (DwgCmColor)
DEFINE_DWGDB_TYPEDEFS (DwgCmEntityColor)
DEFINE_DWGDB_TYPEDEFS (DwgTransparency)
DEFINE_DWGDB_TYPEDEFS (DwgRxObject)
DEFINE_DWGDB_TYPEDEFS (DwgDbHandle)
DEFINE_DWGDB_TYPEDEFS (DwgDbObjectId)
DEFINE_DWGDB_TYPEDEFS (DwgDbObject)
DEFINE_DWGDB_TYPEDEFS (DwgDbObjectIterator)
DEFINE_DWGDB_TYPEDEFS (DwgDbDictionaryIterator)
DEFINE_DWGDB_TYPEDEFS (DwgDbDictionary)
DEFINE_DWGDB_TYPEDEFS (DwgDbEntity)
DEFINE_DWGDB_TYPEDEFS (DwgDbEllipse)
DEFINE_DWGDB_TYPEDEFS (DwgDbLine)
DEFINE_DWGDB_TYPEDEFS (DwgDbArc)
DEFINE_DWGDB_TYPEDEFS (DwgDbCircle)
DEFINE_DWGDB_TYPEDEFS (DwgDb3dSolid)
DEFINE_DWGDB_TYPEDEFS (DwgDbBlockReference)
DEFINE_DWGDB_TYPEDEFS (DwgDbViewRepBlockReference)
DEFINE_DWGDB_TYPEDEFS (DwgDbBody)
DEFINE_DWGDB_TYPEDEFS (DwgDbCamera)
DEFINE_DWGDB_TYPEDEFS (DwgDbCurve)
DEFINE_DWGDB_TYPEDEFS (DwgDbDimension)
DEFINE_DWGDB_TYPEDEFS (DwgDbFace)
DEFINE_DWGDB_TYPEDEFS (DwgDbFaceRecord)
DEFINE_DWGDB_TYPEDEFS (DwgDbFcf)
DEFINE_DWGDB_TYPEDEFS (DwgDbFrame)
DEFINE_DWGDB_TYPEDEFS (DwgDbGeoPositionMarker)
DEFINE_DWGDB_TYPEDEFS (DwgDbGroup)
DEFINE_DWGDB_TYPEDEFS (DwgDbHatch)
DEFINE_DWGDB_TYPEDEFS (DwgDbImage)
DEFINE_DWGDB_TYPEDEFS (DwgDbLayout)
DEFINE_DWGDB_TYPEDEFS (DwgDbLight)
DEFINE_DWGDB_TYPEDEFS (DwgDbMaterial)
DEFINE_DWGDB_TYPEDEFS (DwgDbMLeader)
DEFINE_DWGDB_TYPEDEFS (DwgDbMline)
DEFINE_DWGDB_TYPEDEFS (DwgDbMPolygon)
DEFINE_DWGDB_TYPEDEFS (DwgDbMText)
DEFINE_DWGDB_TYPEDEFS (DwgDbAttribute)
DEFINE_DWGDB_TYPEDEFS (DwgDbAttributeDefinition)
DEFINE_DWGDB_TYPEDEFS (DwgDbPoint)
DEFINE_DWGDB_TYPEDEFS (DwgDbPointCloudEx)
DEFINE_DWGDB_TYPEDEFS (DwgDbPolyline)
DEFINE_DWGDB_TYPEDEFS (DwgDb2dPolyline)
DEFINE_DWGDB_TYPEDEFS (DwgDb3dPolyline)
DEFINE_DWGDB_TYPEDEFS (DwgDbPolyFaceMesh)
DEFINE_DWGDB_TYPEDEFS (DwgDbPolygonMesh)
DEFINE_DWGDB_TYPEDEFS (DwgDbProxyEntity)
DEFINE_DWGDB_TYPEDEFS (DwgDbRasterImage)
DEFINE_DWGDB_TYPEDEFS (DwgDbRasterImageDef)
DEFINE_DWGDB_TYPEDEFS (DwgDbRegion)
DEFINE_DWGDB_TYPEDEFS (DwgDbSection)
DEFINE_DWGDB_TYPEDEFS (DwgDbSequenceEnd)
DEFINE_DWGDB_TYPEDEFS (DwgDbShape)
DEFINE_DWGDB_TYPEDEFS (DwgDbSolid)
DEFINE_DWGDB_TYPEDEFS (DwgDbSpline)
DEFINE_DWGDB_TYPEDEFS (DwgDbSubDMesh)
DEFINE_DWGDB_TYPEDEFS (DwgDbSurface)
DEFINE_DWGDB_TYPEDEFS (DwgDbExtrudedSurface)
DEFINE_DWGDB_TYPEDEFS (DwgDbLoftedSurface)
DEFINE_DWGDB_TYPEDEFS (DwgDbNurbSurface)
DEFINE_DWGDB_TYPEDEFS (DwgDbRevolvedSurface)
DEFINE_DWGDB_TYPEDEFS (DwgDbPlaneSurface)
DEFINE_DWGDB_TYPEDEFS (DwgDbSweptSurface)
DEFINE_DWGDB_TYPEDEFS (DwgDbText)
DEFINE_DWGDB_TYPEDEFS (DwgDbTrace)
DEFINE_DWGDB_TYPEDEFS (DwgDbUnderlayReference)
DEFINE_DWGDB_TYPEDEFS (DwgDbVertex)
DEFINE_DWGDB_TYPEDEFS (DwgDbPolyFaceMeshVertex)
DEFINE_DWGDB_TYPEDEFS (DwgDbPolygonMeshVertex)
DEFINE_DWGDB_TYPEDEFS (DwgDbViewBorder)
DEFINE_DWGDB_TYPEDEFS (DwgDbViewport)
DEFINE_DWGDB_TYPEDEFS (DwgDbViewSymbol)
DEFINE_DWGDB_TYPEDEFS (DwgDbDatabase)
DEFINE_DWGDB_TYPEDEFS (DwgDbSymbolTable)
DEFINE_DWGDB_TYPEDEFS (DwgDbSymbolTableRecord)
DEFINE_DWGDB_TYPEDEFS (DwgDbBlockTable)
DEFINE_DWGDB_TYPEDEFS (DwgDbBlockTableRecord)
DEFINE_DWGDB_TYPEDEFS (DwgDbLayerTable)
DEFINE_DWGDB_TYPEDEFS (DwgDbLayerTableRecord)
DEFINE_DWGDB_TYPEDEFS (DwgDbTextStyleTable)
DEFINE_DWGDB_TYPEDEFS (DwgDbTextStyleTableRecord)
DEFINE_DWGDB_TYPEDEFS (DwgDbLinetypeTable)
DEFINE_DWGDB_TYPEDEFS (DwgDbLinetypeTableRecord)
DEFINE_DWGDB_TYPEDEFS (DwgDbDimstyleTable)
DEFINE_DWGDB_TYPEDEFS (DwgDbDimstyleTableRecord)
DEFINE_DWGDB_TYPEDEFS (DwgDbViewTable)
DEFINE_DWGDB_TYPEDEFS (DwgDbViewTableRecord)
DEFINE_DWGDB_TYPEDEFS (DwgDbViewportTable)
DEFINE_DWGDB_TYPEDEFS (DwgDbViewportTableRecord)
DEFINE_DWGDB_TYPEDEFS (DwgDbVisualStyle)
DEFINE_DWGDB_TYPEDEFS (DwgDbUCSTable)
DEFINE_DWGDB_TYPEDEFS (DwgDbUCSTableRecord)
DEFINE_DWGDB_TYPEDEFS (DwgDbRegAppTable)
DEFINE_DWGDB_TYPEDEFS (DwgDbRegAppTableRecord)
DEFINE_DWGDB_TYPEDEFS (DwgDbSpatialFilter)
DEFINE_DWGDB_TYPEDEFS (DwgDbSpatialIndex)
DEFINE_DWGDB_TYPEDEFS (DwgDbSpatialIndexIterator)
DEFINE_DWGDB_TYPEDEFS (DwgDbSortentsTable)
DEFINE_DWGDB_TYPEDEFS (DwgDbXrecord)
DEFINE_DWGDB_TYPEDEFS (DwgDbDwgFiler)
DEFINE_DWGDB_TYPEDEFS (DwgDbDxfFiler)
DEFINE_DWGDB_TYPEDEFS (DwgDbSun)
DEFINE_DWGDB_TYPEDEFS (DwgDbBackground)
DEFINE_DWGDB_TYPEDEFS (DwgDbGradientBackground)
DEFINE_DWGDB_TYPEDEFS (DwgDbGroundPlaneBackground)
DEFINE_DWGDB_TYPEDEFS (DwgDbIBLBackground)
DEFINE_DWGDB_TYPEDEFS (DwgDbImageBackground)
DEFINE_DWGDB_TYPEDEFS (DwgDbSkyBackground)
DEFINE_DWGDB_TYPEDEFS (DwgDbSolidBackground)
DEFINE_DWGDB_TYPEDEFS (DwgDbXrefGraph)
DEFINE_DWGDB_TYPEDEFS (DwgDbXrefGraphNode)


enum class DwgDbStatus
    {
    Success                     = 0,

    // 1-FirstDwgDbError will be casted from either OdResult or Acad::ErrorStatus!
    FirstDwgDbError             = 50000,
    UnknownError,
    ObjectNotOpenYet,
    FileNotFound,
    NotFound,
    InvalidData,
    MemoryError,
    InvalidInput,
    FilerError,
    NotSupported,
    UrlCacheError,
    NotPersistentObject,
    };  // DwgDbStatus
#define ToDwgDbStatus(status)   static_cast<DwgDbStatus> (##status)

enum class DwgFileVersion
    {
    Invalid                     = -2,   // Not a DWG version at all
    Newer                       = -1,   // A newer version not currently supported
    Unknown                     = 0,    // An old version not known to us but can potentially open
    R2_5                        = 1,
    R2_6                        = 2,
    R9                          = 3,
    R10                         = 4,
    R11                         = 5,
    R13                         = 6,
    R14                         = 7,
    R2000                       = 8,
    R2004                       = 9,
    R2007                       = 10,
    R2010                       = 11,
    R2013                       = 12,
    R2018                       = 13,
    MAX                         = R2018,
    Current                     = 0xFF
    };  // DwgFileVersion

enum class DwgDbLineWeight
    { 
    Weight000                   = 0,
    Weight005                   = 5,
    Weight009                   = 9,
    Weight013                   = 13,
    Weight015                   = 15,
    Weight018                   = 18,
    Weight020                   = 20,
    Weight025                   = 25,
    Weight030                   = 30,
    Weight035                   = 35,
    Weight040                   = 40,
    Weight050                   = 50,
    Weight053                   = 53,
    Weight060                   = 60,
    Weight070                   = 70,
    Weight080                   = 80,
    Weight090                   = 90,
    Weight100                   = 100,
    Weight106                   = 106,
    Weight120                   = 120,
    Weight140                   = 140,
    Weight158                   = 158,
    Weight200                   = 200,
    Weight211                   = 211,
    WeightByLayer               = -1,
    WeightByBlock               = -2,
    WeightByDefault             = -3
    };  // DwgDbLineWeight

enum class DwgDbOpenMode
    {
    ForRead                     = 0,
    ForWrite                    = 1,
    ForNotify                   = 2
    };  // DwgDbOpenMode
#define ToDwgDbOpenMode(mode)   DWGDB_CASTFROMENUM_DB(OpenMode)(mode)   // from Toolkit
#define FromDwgDbOpenMode(mode) DWGDB_CASTTOENUM_DB(OpenMode)(mode)     // to Toolkit

enum class DwgDbVisibility
    {
    Visible                     = 0,
    Invisible                   = 1
    };  // DwgDbVisibility

enum class DwgDbUnits           // == AcDb::UnitsValue
    {
    Undefined                   = 0,
    Inches                      = 1,
    Feet                        = 2,
    Miles                       = 3,
    Millimeters                 = 4,
    Centimeters                 = 5,
    Meters                      = 6,
    Kilometers                  = 7,
    Microinches                 = 8,
    Mils                        = 9,
    Yards                       = 10,
    Angstroms                   = 11,
    Nanometers                  = 12,
    Microns                     = 13,
    Decimeters                  = 14,
    Dekameters                  = 15,
    Hectometers                 = 16,
    Gigameters                  = 17,
    Astronomical                = 18,
    LightYears                  = 19,
    Parsecs                     = 20,
    USSurveyFeet                = 21,       // R2017
    USSurveyInch                = 22,       // R2017
    USSurveyYard                = 23,       // R2017
    USSurveyMile                = 24,       // R2017
    Max                         = USSurveyMile
    };  // DwgDbUnits

enum class DwgDbLUnitFormat
    {
    Scientific                  = 1,
    Decimal                     = 2,
    Engineering                 = 3,
    Architectural               = 4,
    Fractional                  = 5,
    WindowsDesktop              = 6,            // use Windows control panel settings.
    };  // DwgDbLUnitFormat

enum class DwgDbAngularUnits
    {
    DecimalDegrees              = 0,
    DegreesMinutesSeconds       = 1,
    Gradians                    = 2,
    Radians                     = 3,
    Bearing                     = 4,
    };  // DwgDbAngularUnits

enum class DwgDbLightingUnits
    {
    None                        = 0,
    American                    = 1,    // US lighting units (foot-candles)
    International               = 2,    // International lighting units (lux)
    };  // DwgDbLightingUnits

enum class DwgDbPlotStyleNameType
    {
    ByLayer                     = 0,
    ByBlock                     = 1,
    IsDictDefault               = 2,
    ById                        = 3,
    };  // DwgDbPlotStyleNameType

enum class DwgFilerType
    {
    FileFiler,
    CopyFiler,
    UndoFiler,
    BagFiler,
    IdXlateFiler,
    PageFiler,
    DeepCloneFiler,
    IdFiler,
    PurgeFiler,
    WblockCloneFiler
    };  // DwgFilerType

enum class DxfGroupCode
    {
    Invalid                 = -9999,
    XDictionary             = -6,
    PReactors               = -5,
    Operator                = -4,
    XDataStart              = -3,
    HeaderId                = -2,
    FirstEntId              = -2,
    End                     = -1,
    Start                   = 0,
    Text                    = 1,
    XRefPath                = 1,
    ShapeName               = 2,
    BlockName               = 2,
    AttributeTag            = 2,
    SymbolTableName         = 2,
    MstyleName              = 2,
    SymTableRecName         = 2,
    AttributePrompt         = 3,
    DimStyleName            = 3,
    LinetypeProse           = 3,
    TextFontFile            = 3,
    Description             = 3,
    DimPostStr              = 3,
    TextBigFontFile         = 4,
    DimAPostStr             = 4,
    CLShapeName             = 4,
    SymTableRecComments     = 4,
    Handle                  = 5,
    DimBlk                  = 5,
    DimBlk1                 = 6,
    LinetypeName            = 6,
    DimBlk2                 = 7,
    TextStyleName           = 7,
    LayerName               = 8,
    CLShapeText             = 9,
    XCoord                  = 10,
    YCoord                  = 20,
    ZCoord                  = 30,
    Elevation               = 38,
    Thickness               = 39,
    Real                    = 40,
    ViewportHeight          = 40,
    TxtSize                 = 40,
    TxtStyleXScale          = 41,
    ViewWidth               = 41,
    ViewportAspect          = 41,
    TxtStylePSize           = 42,
    ViewLensLength          = 42,
    ViewFrontClip           = 43,
    ViewBackClip            = 44,
    ShapeXOffset            = 44,
    ShapeYOffset            = 45,
    ViewHeight              = 45,
    ShapeScale              = 46,
    PixelScale              = 47,
    LinetypeScale           = 48,
    DashLength              = 49,
    MlineOffset             = 49,
    LinetypeElement         = 49,
    Angle                   = 50,
    ViewportSnapAngle       = 50, // deprecated
    ViewportTwist           = 51,
    Visibility              = 60,
    ViewportGridDisplay     = 60,
    LayerLinetype           = 61,
    ViewportGridMajor       = 61,
    Color                   = 62,
    // Removed codes intended only for internal use:  63-65
    HasSubentities          = 66,
    ViewportVisibility      = 67,
    ViewportActive          = 68,
    ViewportNumber          = 69,
    Int16                   = 70,
    ViewMode                = 71,
    CircleSides             = 72,
    ViewportZoom            = 73,
    ViewportIcon            = 74,
    ViewportSnap            = 75,
    ViewportGrid            = 76,
    ViewportSnapStyle       = 77,
    ViewportSnapPair        = 78,
    RegAppFlags             = 71,
    TxtStyleFlags           = 71,
    LinetypeAlign           = 72,
    LinetypePDC             = 73,
    Int32                   = 90,
    VertexIdentifier        = 91,
    // Subclass Section Marker to be followed by subclass name.
    Subclass                = 100,
    EmbeddedObjectStart     = 101,
    ControlString           = 102,
    // DimVarTableRecords have been using 5 for a string value.  With R13, they get a handle
    // value as well.  Since 5 is already in use, we use 105 for this special case.
    DimVarHandle            = 105,
    UCSOrg                  = 110,
    UCSOriX                 = 111,
    UCSOriY                 = 112,
    XReal                   = 140,
    ViewBrightness          = 141,
    ViewContrast            = 142,
    // 64-bit integers can only be used with R2010 and higher.
    Int64                   = 160,
    XInt16                  = 170,
    // 180 - 189 cannot be used and 190-199 are invalid
    NormalX                 = 210,
    NormalY                 = 220,
    NormalZ                 = 230,
    // 260-269 are invalid
    XXInt16                 = 270,
    Int8                    = 280,
    RenderMode              = 281,
    DefaultLightingType     = 282,
    ShadowFlags             = 284,
    Bool                    = 290,
    DefaultLightingOn       = 292,
    //  More string values 300-309
    XTextString             = 300,
    //  Arbitrary Binary Chunks 310-319
    BinaryChunk             = 310,
    //  Arbitrary Object Handles 320-329
    ArbHandle               = 320,
    SoftPointerId           = 330, // 330-339
    ViewBackgroundId        = 332, // softPointer to background of viewport and viewporttable record
    ShadePlotId             = 333, // softPointer to shade plot visual style or render preset
    LiveSectionId           = 334, // softPointer to LiveSection of view, viewport and viewporttable record
    LiveSectionName         = 309, // LiveSection Name
    HardPointerId           = 340, // 340-349
    ObjVisualStyleId        = 345,
    VpVisualStyleId         = 346,
    MaterialId              = 347, // hardpointer reference to AcDbMaterial
    VisualStyleId           = 348, // hardpointer reference to visual style
    DragVisualStyleId       = 349, // hardpointer reference to visual style
    SoftOwnershipId         = 350, // 350-359
    HardOwnershipId         = 360, // 360-369
    SunId                   = 361, // hardownership reference to sun object
    // Lineweight is either an integer or "BYLAYER" or "BYBLOCK"
    LineWeight              = 370,
    PlotStyleNameType       = 380,
    PlotStyleNameId         = 390,
    XXXInt16                = 400,
    LayoutName              = 410,
    // Extended color information for base entities
    ColorRGB                = 420,
    ColorName               = 430,
    // New base entity property Alpha is an integer
    Alpha                   = 440,
    GradientObjType         = 450,
    GradientPatType         = 451,
    GradientTintType        = 452,
    GradientColCount        = 453,
    GradientAngle           = 460,
    GradientShift           = 461,
    GradientTintVal         = 462,
    GradientColVal          = 463,
    GradientName            = 470,
    FaceStyleId             = 480,
    EdgeStyleId             = 481,
    Comment                 = 999,
    XdAsciiString           = 1000,
    RegAppName              = 1001,
    XdControlString         = 1002,
    XdLayerName             = 1003,
    XdBinaryChunk           = 1004,
    XdHandle                = 1005,
    XdXCoord                = 1010,
    XdYCoord                = 1020,
    XdZCoord                = 1030,
    XdWorldXCoord           = 1011,
    XdWorldYCoord           = 1021,
    XdWorldZCoord           = 1031,
    XdWorldXDisp            = 1012,
    XdWorldYDisp            = 1022,
    XdWorldZDisp            = 1032,
    XdWorldXDir             = 1013,
    XdWorldYDir             = 1023,
    XdWorldZDir             = 1033,
    XdReal                  = 1040,
    XdDist                  = 1041,
    XdScale                 = 1042,
    XdInteger16             = 1070,
    XdInteger32             = 1071,
    // This enum value should always be set to whatever the highest enum value is.
    XdMax                   =  XdInteger32
    };  // DxfGroupCode

enum class SnapIsoPair
    {
    LeftIsoplane            = 0,
    TopIsoplane             = 1,
    RightIsoplane           = 2
    };  // SnapIsoPair

enum class DwgDbXrefStatus
    {
    NotAnXref               = DWGDB_SDKENUM_DB(kXrfNotAnXref),
    Resolved                = DWGDB_SDKENUM_DB(kXrfResolved),
    Unloaded                = DWGDB_SDKENUM_DB(kXrfUnloaded),
    Unreferenced            = DWGDB_SDKENUM_DB(kXrfUnreferenced),
    FileNotFound            = DWGDB_SDKENUM_DB(kXrfFileNotFound ),
    Unresolved              = DWGDB_SDKENUM_DB(kXrfUnresolved),
    };  // DwgDbXrefStatus

// platform types commonly used across OpenDWG & RealDWG
typedef DWGDB_SDKNAME(bool,     Adesk::Boolean)     DwgDbBool;
typedef DWGDB_SDKNAME(OdChar,   ACHAR)              DwgDbChar;
typedef DWGDB_SDKNAME(OdUInt8,  Adesk::UInt8)       DwgDbUInt8;
typedef DWGDB_SDKNAME(OdInt16,  Adesk::Int16)       DwgDbInt16;
typedef DWGDB_SDKNAME(OdUInt16, Adesk::UInt16)      DwgDbUInt16;
typedef DWGDB_SDKNAME(OdInt32,  Adesk::Int32)       DwgDbInt32;
typedef DWGDB_SDKNAME(OdUInt32, Adesk::UInt32)      DwgDbUInt32;
typedef DWGDB_SDKNAME(OdUInt64, Adesk::UInt64)      DwgDbUInt64;
typedef DWGDB_SDKNAME(OdIntPtr, Adesk::LongPtr)     DwgDbLongPtr;
DEFINE_NO_NAMESPACE_TYPEDEFS (DwgDbBool)
DEFINE_NO_NAMESPACE_TYPEDEFS (DwgDbChar)
DEFINE_NO_NAMESPACE_TYPEDEFS (DwgDbUInt8)
DEFINE_NO_NAMESPACE_TYPEDEFS (DwgDbUInt16)
DEFINE_NO_NAMESPACE_TYPEDEFS (DwgDbUInt32)
DEFINE_NO_NAMESPACE_TYPEDEFS (DwgDbInt32)
DEFINE_NO_NAMESPACE_TYPEDEFS (DwgDbUInt64)

// common Od/AcXxxx root classes
#define DWG_TypeR(_type_)               DWG_Type(_type_) &
#define DWG_TypeCR(_type_)              DWG_Type(_type_) const&
#define DWG_TypeCP(_type_)              DWG_Type(_type_) const*
#define DWG_TypeP(_type_)               DWG_Type(_type_) *

// common Od/AcCmXxxx classes
#define DWGCM_TypeR(_cmType_)           DWGCM_Type(_cmType_) &
#define DWGCM_TypeCR(_cmType_)          DWGCM_Type(_cmType_) const&
#define DWGCM_TypeCP(_cmType_)          DWGCM_Type(_cmType_) const*
#define DWGCM_TypeP(_cmType_)           DWGCM_Type(_cmType_) *

// common Od/AcDbXxxx classes
#define DWGDB_TypeR(_dbType_)           DWGDB_Type(_dbType_) &
#define DWGDB_TypeCR(_dbType_)          DWGDB_Type(_dbType_) const&
#define DWGDB_TypeCP(_dbType_)          DWGDB_Type(_dbType_) const*
#define DWGDB_TypeP(_dbType_)           DWGDB_Type(_dbType_) *

// common Od/AcGeXxxx classes
#define DWGGE_TypeR(_geomType_)         DWGGE_Type(_geomType_) &
#define DWGGE_TypeCR(_geomType_)        DWGGE_Type(_geomType_) const&
#define DWGGE_TypeCP(_geomType_)        DWGGE_Type(_geomType_) const*
#define DWGGE_TypeP(_geomType_)         DWGGE_Type(_geomType_) *

//! common Od/AcGiXxxx classes
#define DWGGI_TypeR(_giType_)           DWGGI_Type(_giType_)&
#define DWGGI_TypeCR(_giType_)          DWGGI_Type(_giType_) const&
#define DWGGI_TypeCP(_giType_)          DWGGI_Type(_giType_) const*
#define DWGGI_TypeP(_giType_)           DWGGI_Type(_giType_) *

//__PUBLISH_SECTION_END__
