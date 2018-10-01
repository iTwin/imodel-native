/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Dwg/DwgDb/BasicTypes.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include    <Dwg/DwgDb/DwgDbCommon.h>

#ifdef DWGTOOLKIT_OpenDwg
#include    <Teigha/Kernel/Include/SmartPtr.h>
#include    <Teigha/Kernel/Include/OdString.h>
#elif DWGTOOLKIT_RealDwg
#include    <RealDwg/Base/dbobjptr2.h>
#include    <RealDwg/Base/AcString.h>
#else
    #error  "Must define DWGTOOLKIT!"
#endif

#include    <Bentley/WString.h>
#include    <Bentley/RefCounted.h>
#include    <Bentley/bvector.h>
#include    <Geom/GeomApi.h>

#if defined (BENTLEYCONFIG_PARASOLID)
#include    <PSolid/parasolid_kernel.h>
#include    <PSolid/parasolid_debug.h>
#endif

BEGIN_DWGDB_NAMESPACE

class DwgDbObjectId;
class DwgCmColor;

//! common arrays
typedef bvector <BentleyApi::DPoint2d>  DPoint2dArray;
typedef bvector <BentleyApi::DPoint3d>  DPoint3dArray;
typedef bvector <BentleyApi::DVec3d>    DVector3dArray;
typedef bvector <int32_t>               DwgDbIntArray;
typedef bvector <double>                DwgDbDoubleArray;
typedef bvector <DwgDbObjectId>         DwgDbObjectIdArray;
typedef bvector <DwgDbObjectP>          DwgDbObjectPArray;
typedef bvector <DwgDbEntityP>          DwgDbEntityPArray;
typedef bvector <DwgCmColor>            DwgColorArray;
DEFINE_NO_NAMESPACE_TYPEDEFS (DPoint2dArray)
DEFINE_NO_NAMESPACE_TYPEDEFS (DPoint3dArray)
DEFINE_NO_NAMESPACE_TYPEDEFS (DVector3dArray)
DEFINE_NO_NAMESPACE_TYPEDEFS (DwgDbIntArray)
DEFINE_NO_NAMESPACE_TYPEDEFS (DwgDbDoubleArray)
DEFINE_NO_NAMESPACE_TYPEDEFS (DwgDbObjectIdArray)
DEFINE_NO_NAMESPACE_TYPEDEFS (DwgDbObjectPArray)
DEFINE_NO_NAMESPACE_TYPEDEFS (DwgDbEntityPArray)
DEFINE_NO_NAMESPACE_TYPEDEFS (DwgColorArray)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          11/15
+===============+===============+===============+===============+===============+======*/
template <class T> class IDwgDbSmartPtr
    {
protected:
    DwgDbStatus         m_openStatus;

public:
    IDwgDbSmartPtr () : m_openStatus(DwgDbStatus::UnknownError) { ; }

    DwgDbStatus         OpenStatus () { return m_openStatus; }

    virtual bool        _IsNull () const = 0;
    virtual T const*    _Get () const = 0;
    virtual T*          _Get () = 0;
    virtual DwgDbStatus _OpenObject (DwgDbObjectId id, DwgDbOpenMode mode, bool openErased, bool openLocked) = 0;
    virtual DwgDbStatus _AcquireObject (T *& obj) = 0;
    virtual DwgDbStatus _CreateObject () = 0;
    virtual DwgDbStatus _CloseObject () = 0;
    };  // IDwgDbSmartPtr

/*=================================================================================**//**
* Use this to workaround DEFINE_BENTLEY_NEW_DELETE_OPERATORS conflict when extending RealDWG's.
* @bsiclass                                                     Don.Fu          11/15
+===============+===============+===============+===============+===============+======*/
struct  IDwgDbRefCounted
    {
protected:
    virtual ~IDwgDbRefCounted() {}

public:
    virtual uint32_t AddRef() const = 0;
    virtual uint32_t Release() const = 0;
    };  //  IDwgDbRefCounted

template <class T> class DwgDbRefCounted : public T
    {
private:
    mutable BeAtomic<uint32_t> m_refCount;

public:
    uint32_t AddRef() const { return m_refCount.IncrementAtomicPre(); }
    uint32_t Release() const
        {
        uint32_t    countWas = m_refCount.DecrementAtomicPost();
        if (1 == countWas)
            delete this;
        return  countWas - 1;
        }

protected:
    virtual ~DwgDbRefCounted() {}

public:
    DwgDbRefCounted() { m_refCount.store(0); }
    DwgDbRefCounted(DwgDbRefCounted const& rhs) { m_refCount.store(0); }
    DwgDbRefCounted& operator=(DwgDbRefCounted const& rhs)
        {
        if (this != &rhs)
            {
            T::operator=(rhs);
            } 
        return *this;
        }
    uint32_t GetRefCount() const
        {
        return m_refCount.load();
        }
    };  // DwgDbRefCounted

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          11/15
+===============+===============+===============+===============+===============+======*/
class DwgString : public DWGROOTCLASS_EXTEND(String)
    {
    DEFINE_T_SUPER (DWGROOT_SUPER_CONSTRUCTOR(String))
public:
    DWGROOTCLASS_ADD_CONSTRUCTORS (String)

    DWGDB_EXPORT DwgString (WCharCP chars);
    DWGDB_EXPORT DwgString (DwgString const& in) : DwgString(in.c_str()) {}
    DWGDB_EXPORT DwgString (Utf8StringCR in);
    DWGDB_EXPORT ~DwgString ();

    DWGDB_EXPORT void       Assign (WCharCP chars);
    DWGDB_EXPORT void       Append (WChar oneChar);
    DWGDB_EXPORT int        Insert (int index, WCharCP chars);
    DWGDB_EXPORT int        GetLength () const;
    DWGDB_EXPORT int        Find (WCharCP chars) const;
    DWGDB_EXPORT bool       IsEmpty () const;
    DWGDB_EXPORT bool       IsAscii () const;
    DWGDB_EXPORT bool       EqualsI (WCharCP other) const;
    DWGDB_EXPORT bool       StartsWithI (WCharCP suffix) const;
    DWGDB_EXPORT void       Empty ();
    DWGDB_EXPORT WCharCP    GetWCharCP () const;
    DWGDB_EXPORT WCharCP    c_str () const;
    DWGDB_EXPORT void const* AsBufferPtr () const;
    DWGDB_EXPORT size_t     GetBufferSize () const;
    DWGDB_EXPORT DwgStringR operator = (DwgStringCR str);
    DWGDB_EXPORT operator   const wchar_t* () const;
    };  // DwgString

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          04/16
+===============+===============+===============+===============+===============+======*/
struct DwgBinaryData
    {
private:
    bvector<uint8_t>        m_buffer;

public:
    DWGDB_EXPORT DwgBinaryData ();
    DWGDB_EXPORT DwgBinaryData (uint8_t const* buf, uint32_t nBytes) { Set(buf, nBytes); }
    DWGDB_EXPORT DwgBinaryData (char const* buf, uint32_t nBytes) { Set(buf, nBytes); }
    DWGDB_EXPORT void           Set (uint8_t const* buf, uint32_t nBytes);
    DWGDB_EXPORT void           Set (char const* buf, uint32_t nBytes);
    DWGDB_EXPORT void           Clear () { m_buffer.clear(); }
    DWGDB_EXPORT size_t         GetSize () const;
    DWGDB_EXPORT void const*    GetBuffer () const;
    };  // DwgBinaryData
DEFINE_NO_NAMESPACE_TYPEDEFS (DwgBinaryData)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          11/15
+===============+===============+===============+===============+===============+======*/
class DwgCmEntityColor : public DWGROOTCLASS_EXTEND(CmEntityColor)
    {
public:
    DEFINE_T_SUPER (DWGROOT_SUPER_CONSTRUCTOR(CmEntityColor))
    DWGROOTCLASS_ADD_CONSTRUCTORS (CmEntityColor)

    enum Method
        {
        ByLayer     = 0xC0,
        ByBlock,
        ByColor,
        ByACI,
        ByPen,
        Foreground,
        LayerOff,
        //! RealDWG has this as LayerFrozen(nonsense) and OpenDWG overrides it as ByDgnIndex.
        ByDgnIndex,
        None
        };  // Method

    DWGDB_EXPORT ~DwgCmEntityColor ();

    //! @return An interger with the high byte to be the color method, and the low bytes to be RGB values.
    DWGDB_EXPORT uint32_t   GetMRGB () const;
    //! @return An interger containing only the RGB bytes.
    DWGDB_EXPORT uint32_t   GetRGB () const;
    //! @return The ACI vallue.
    DWGDB_EXPORT int16_t    GetIndex () const;
    DWGDB_EXPORT Byte       GetRed () const;
    DWGDB_EXPORT Byte       GetGreen () const;
    DWGDB_EXPORT Byte       GetBlue () const;
    DWGDB_EXPORT bool       IsByLayer () const;
    DWGDB_EXPORT bool       IsByBlock () const;
    DWGDB_EXPORT bool       IsByACI () const;
    DWGDB_EXPORT bool       IsByColor () const;
    DWGDB_EXPORT bool       IsForeground () const;
    DWGDB_EXPORT bool       IsNone () const;
    //! Get the color method
    DWGDB_EXPORT Method     GetColorMethod () const;

    DWGDB_EXPORT DwgDbStatus        SetColorIndex (uint16_t color);
    static DWGDB_EXPORT uint32_t    GetRGBFromIndex (uint16_t color);
    };
DEFINE_NO_NAMESPACE_TYPEDEFS (DwgCmEntityColor)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          11/15
+===============+===============+===============+===============+===============+======*/
class DwgCmColor : public DWGROOTCLASS_EXTEND(CmColor)
    {
public:
    DEFINE_T_SUPER (DWGROOT_SUPER_CONSTRUCTOR(CmColor))
    DWGROOTCLASS_ADD_CONSTRUCTORS (CmColor)

    DWGDB_EXPORT DwgCmColor (DwgCmEntityColorCR entityColor);
    DWGDB_EXPORT ~DwgCmColor ();

    //! @return An interger with the high byte to be the color method, and the low bytes to be RGB values.
    DWGDB_EXPORT uint32_t   GetMRGB () const;
    //! @return An interger containing only the RGB bytes.
    DWGDB_EXPORT uint32_t   GetRGB () const;
    //! @return The ACI vallue.
    DWGDB_EXPORT int16_t    GetIndex () const;
    DWGDB_EXPORT Byte       GetRed () const;
    DWGDB_EXPORT Byte       GetGreen () const;
    DWGDB_EXPORT Byte       GetBlue () const;
    DWGDB_EXPORT bool       IsByLayer () const;
    DWGDB_EXPORT bool       IsByBlock () const;
    DWGDB_EXPORT bool       IsByACI () const;
    DWGDB_EXPORT bool       IsByColor () const;
    DWGDB_EXPORT bool       IsForeground () const;
    DWGDB_EXPORT bool       IsNone () const;

    //! Get ACI value
    DWGDB_EXPORT DwgDbStatus        SetColorIndex (uint16_t color);
    //! Get this DwgCmColor as a DwgEntityColor
    DWGDB_EXPORT DwgCmEntityColor   GetEntityColor () const;
    //! Get the color method
    DWGDB_EXPORT DwgCmEntityColor::Method GetColorMethod () const;
    };
DEFINE_NO_NAMESPACE_TYPEDEFS (DwgCmColor)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          11/15
+===============+===============+===============+===============+===============+======*/
class DwgTransparency  : public DWGROOTCLASS_EXTEND(CmTransparency)
    {
    DEFINE_T_SUPER (DWGROOT_SUPER_CONSTRUCTOR(CmTransparency))
#ifdef DWGTOOLKIT_OpenDwg
    DwgTransparency (OdCmTransparency const& trans) : OdCmTransparency(trans) {}
#elif DWGTOOLKIT_RealDwg
    DwgTransparency (AcCmTransparency const& trans) : AcCmTransparency(trans) {}
#endif
    DWGDB_EXPORT DwgTransparency ();
    DWGDB_EXPORT ~DwgTransparency ();

    DWGDB_EXPORT uint8_t    GetAlpha () const;
    DWGDB_EXPORT bool       IsByLayer () const;
    DWGDB_EXPORT bool       IsByBlock () const;
    DWGDB_EXPORT bool       IsByAlpha () const;
    DWGDB_EXPORT bool       IsClear () const;
    DWGDB_EXPORT uint32_t   SerializeOut () const;
    };
DEFINE_NO_NAMESPACE_TYPEDEFS (DwgTransparency)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          11/15
+===============+===============+===============+===============+===============+======*/
class DwgDbDate : public DWGDB_EXTENDCLASS(Date)
    {
    DEFINE_T_SUPER (DWGDB_SUPER_CONSTRUCTOR(Date))
public:
    DWGDB_ADD_CONSTRUCTORS (Date)

    DWGDB_EXPORT double     GetJulianFraction () const;
    DWGDB_EXPORT void       GetDate (int16_t& month, int16_t& day, int16_t& year) const;
    DWGDB_EXPORT void       GetTime (int16_t& hour, int16_t& minute, int16_t& second, int16_t& minisecond) const;
    };
DEFINE_NO_NAMESPACE_TYPEDEFS (DwgDbDate)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          07/18
+===============+===============+===============+===============+===============+======*/
struct UtilsLib
    {
#ifdef BENTLEYCONFIG_PARASOLID
    //! Converts an Autodesk ShapeManager entity to a Parasolid body. Valid ASM types are SOLID3D, BODY, REGION, and SURFACE.
    //! @param[out] results Output Parasolid body
    //! @param[out] trans   Output transform for the Parasolid body
    //! @param[in] entity   An input ASM entity
    //! @return BSISUCCESS on success conversion, in which case the caller is responsible to free Parasolid memory in the output results.
    //! @note A caller is responsible for managing the Parasolid session, which must be started before this method is called.
    DWGDB_EXPORT static BentleyStatus   ConvertAsmToParasolid (PK_BODY_create_topology_2_r_t& results, TransformR trans, DwgDbEntityCP entity);
#endif
    //! Check if an entity is an Autodesk ShapeManager object.
    DWGDB_EXPORT static bool IsAsmEntity (DwgDbEntityCP entity);
    };  // UtilsLib


END_DWGDB_NAMESPACE
//__PUBLISH_SECTION_END__
