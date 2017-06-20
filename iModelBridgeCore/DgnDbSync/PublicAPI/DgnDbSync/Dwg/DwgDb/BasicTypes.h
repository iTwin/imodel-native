/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbSync/Dwg/DwgDb/BasicTypes.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include    <DgnDbSync/Dwg/DwgDb/DwgDbCommon.h>

#ifdef DWGTOOLKIT_OpenDwg
#include    <Teigha/Core/Include/SmartPtr.h>
#include    <Teigha/Core/Include/OdString.h>
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

BEGIN_DWGDB_NAMESPACE

class DwgDbObjectId;
class DwgCmColor;

//! common arrays
typedef bvector <BentleyApi::DPoint2d>  DPoint2dArray;
typedef bvector <BentleyApi::DPoint3d>  DPoint3dArray;
typedef bvector <int32_t>               DwgDbIntArray;
typedef bvector <double>                DwgDbDoubleArray;
typedef bvector <DwgDbObjectId>         DwgDbObjectIdArray;
typedef bvector <DwgCmColor>            DwgColorArray;
DEFINE_NO_NAMESPACE_TYPEDEFS (DPoint2dArray)
DEFINE_NO_NAMESPACE_TYPEDEFS (DPoint3dArray)
DEFINE_NO_NAMESPACE_TYPEDEFS (DwgDbIntArray)
DEFINE_NO_NAMESPACE_TYPEDEFS (DwgDbDoubleArray)
DEFINE_NO_NAMESPACE_TYPEDEFS (DwgDbObjectIdArray)
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
    DWGDB_EXPORT ~DwgString ();

    DWGDB_EXPORT void       Assign (WCharCP chars);
    DWGDB_EXPORT void       Append (WChar oneChar);
    DWGDB_EXPORT int        GetLength () const;
    DWGDB_EXPORT bool       IsEmpty () const;
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
class DwgCmColor : public DWGROOTCLASS_EXTEND(CmColor)
    {
    DEFINE_T_SUPER (DWGROOT_SUPER_CONSTRUCTOR(CmColor))
    DWGROOTCLASS_ADD_CONSTRUCTORS (CmColor)

    DWGDB_EXPORT ~DwgCmColor ();

    DWGDB_EXPORT uint32_t   GetRGB () const;
    DWGDB_EXPORT int16_t    GetIndex () const;
    DWGDB_EXPORT Byte       GetRed () const;
    DWGDB_EXPORT Byte       GetGreen () const;
    DWGDB_EXPORT Byte       GetBlue () const;
    DWGDB_EXPORT bool       IsByLayer () const;
    DWGDB_EXPORT bool       IsByBlock () const;
    DWGDB_EXPORT bool       IsByACI () const;
    DWGDB_EXPORT bool       IsByColor () const;

    DWGDB_EXPORT DwgDbStatus        SetColorIndex (uint16_t color);
    };
DEFINE_NO_NAMESPACE_TYPEDEFS (DwgCmColor)

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          11/15
+===============+===============+===============+===============+===============+======*/
class DwgCmEntityColor : public DWGROOTCLASS_EXTEND(CmEntityColor)
    {
    DEFINE_T_SUPER (DWGROOT_SUPER_CONSTRUCTOR(CmEntityColor))
    DWGROOTCLASS_ADD_CONSTRUCTORS (CmEntityColor)

    DWGDB_EXPORT ~DwgCmEntityColor ();

    DWGDB_EXPORT uint32_t   GetRGB () const;
    DWGDB_EXPORT int16_t    GetIndex () const;
    DWGDB_EXPORT Byte       GetRed () const;
    DWGDB_EXPORT Byte       GetGreen () const;
    DWGDB_EXPORT Byte       GetBlue () const;
    DWGDB_EXPORT bool       IsByLayer () const;
    DWGDB_EXPORT bool       IsByBlock () const;
    DWGDB_EXPORT bool       IsByACI () const;
    DWGDB_EXPORT bool       IsByColor () const;

    DWGDB_EXPORT DwgDbStatus        SetColorIndex (uint16_t color);
    static DWGDB_EXPORT uint32_t    GetRGBFromIndex (uint16_t color);
    };
DEFINE_NO_NAMESPACE_TYPEDEFS (DwgCmEntityColor)

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

END_DWGDB_NAMESPACE
//__PUBLISH_SECTION_END__
