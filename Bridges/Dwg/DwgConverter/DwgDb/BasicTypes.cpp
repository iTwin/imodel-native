/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "DwgDbInternal.h"

USING_NAMESPACE_DWGDB


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool        DwgString::StartsWithI (WCharCP suffix) const
    {
    if (nullptr == suffix || T_Super::isEmpty())
        return  false;

    size_t  numChars = std::min (wcslen(suffix), (size_t)this->GetLength());

    for (int i = 0; i < numChars; i++)
        {
        if (0 == suffix[i])
            break;
        else if (T_Super::getAt(i) != suffix[i])
            return  false;
        }
    return  true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool        DwgString::IsAscii () const
    {
#ifdef DWGTOOLKIT_OpenDwg
    auto nChars = T_Super::getLength ();
    auto* chars = T_Super::c_str ();
    if (nChars < 1 || nullptr == chars || 0 == chars[0])
        return  false;

    for (int i = 0; i < nChars; i++)
        {
        if (chars[i] < 0x20 || chars[i] > 0x7F)
            return  false;
        }
    return  true;
#elif DWGTOOLKIT_RealDwg
    return T_Super::isAscii ();
#endif
    }
int         DwgString::GetLength () const   { return DWGDB_CALLSDKMETHOD(getLength, length)(); }
bool        DwgString::IsEmpty () const     { return DWGDB_CALLSDKMETHOD(isEmpty, isEmpty) (); }
bool        DwgString::EqualsI (WCharCP o) const { return DWGDB_CALLSDKMETHOD(0==T_Super::iCompare, 0==T_Super::compareNoCase) (o); }
void        DwgString::Empty ()             { DWGDB_CALLSDKMETHOD(empty, setEmpty) (); }
WCharCP     DwgString::GetWCharCP () const  { return reinterpret_cast<WCharCP>(DWGDB_CALLSDKMETHOD(T_Super::c_str(), T_Super::kwszPtr())); }
WCharCP     DwgString::c_str () const       { return reinterpret_cast<WCharCP>(DWGDB_CALLSDKMETHOD(T_Super::c_str(), T_Super::kwszPtr())); }
void        DwgString::Assign (WCharCP chars) { DWGDB_CALLSDKMETHOD(T_Super::empty();T_Super::insert(0,chars), T_Super::assign(chars)); }
void        DwgString::Append (WChar c)     { DWGDB_CALLSDKMETHOD(T_Super::operator+=(c), T_Super::append(c)); }
int         DwgString::Insert (int i, WCharCP chars) { return DWGDB_CALLSDKMETHOD(T_Super::insert(i, chars), T_Super::insert(i, chars).length()); }
int         DwgString::Find (WCharCP s) const { return T_Super::find(s); }
void const* DwgString::AsBufferPtr () const { return reinterpret_cast<void const*>(DWGDB_CALLSDKMETHOD(T_Super::c_str(), T_Super::constPtr())); }
size_t      DwgString::GetBufferSize () const { return this->GetLength()*sizeof(DWGDB_SDKNAME(OdChar,ACHAR)); }
DwgStringR  DwgString::operator             = (DwgStringCR str) { this->Assign(str.c_str()); return *this; }
            DwgString::operator             const wchar_t* () const { return reinterpret_cast<const wchar_t*>(DWGDB_CALLSDKMETHOD(T_Super::c_str(), T_Super::kwszPtr())); }
DwgString::DwgString (WCharCP chars)        { this->Assign(chars); }
DwgString::DwgString (Utf8StringCR in) : DWGDB_CALLSDKMETHOD(T_Super(in.c_str(),OdCodePageId::CP_CNT),T_Super(in.c_str(),Encoding::Utf8)) {}
DwgString::~DwgString ()                    { ; }

uint32_t    DwgCmColor::GetMRGB () const        { return static_cast<uint32_t>(T_Super::color()); }
uint32_t    DwgCmColor::GetRGB () const         { return static_cast<uint32_t>(T_Super::color() & 0x00FFFFFF); }
int16_t     DwgCmColor::GetIndex () const       { return static_cast<int16_t>(T_Super::colorIndex()); }
Byte        DwgCmColor::GetRed () const         { return static_cast<Byte>(T_Super::red()); }
Byte        DwgCmColor::GetGreen () const       { return static_cast<Byte>(T_Super::green()); }
Byte        DwgCmColor::GetBlue () const        { return static_cast<Byte>(T_Super::blue()); }
bool        DwgCmColor::IsByLayer () const      { return T_Super::isByLayer(); }
bool        DwgCmColor::IsByBlock () const      { return T_Super::isByBlock(); }
bool        DwgCmColor::IsByACI () const        { return T_Super::isByACI(); }
bool        DwgCmColor::IsByColor () const      { return T_Super::isByColor(); }
bool        DwgCmColor::IsForeground () const   { return T_Super::isForeground(); }
bool        DwgCmColor::IsNone () const         { return T_Super::isNone(); }
DwgCmColor::DwgCmColor (DwgCmEntityColorCR ec)  { T_Super::setColor(ec.color()); }
DwgCmColor::~DwgCmColor ()                      { ; }
DwgCmEntityColor DwgCmColor::GetEntityColor () const { return static_cast<DwgCmEntityColor>(T_Super::entityColor()); }
DwgCmEntityColor::Method DwgCmColor::GetColorMethod () const { return static_cast<DwgCmEntityColor::Method>(T_Super::colorMethod()); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus DwgCmColor::SetColorIndex (uint16_t color)
    {
#ifdef DWGTOOLKIT_OpenDwg
    T_Super::setColorIndex (color);
    return  DwgDbStatus::Success;
#elif DWGTOOLKIT_RealDwg
    return  ToDwgDbStatus (T_Super::setColorIndex(color));
#endif
    }

uint32_t    DwgCmEntityColor::GetRGBFromIndex (uint16_t index) { return static_cast<uint32_t>(DWGCM_Type(EntityColor)::lookUpRGB(static_cast<uint8_t>(index))); }
uint32_t    DwgCmEntityColor::GetMRGB () const    { return static_cast<uint32_t>(T_Super::color()); }
uint32_t    DwgCmEntityColor::GetRGB () const     { return static_cast<uint32_t>(DWGDB_CALLSDKMETHOD(T_Super::color,T_Super::trueColor)() & 0x00FFFFFF); }
int16_t     DwgCmEntityColor::GetIndex () const   { return static_cast<int16_t>(T_Super::colorIndex()); }
Byte        DwgCmEntityColor::GetRed () const     { return static_cast<Byte>(T_Super::red()); }
Byte        DwgCmEntityColor::GetGreen () const   { return static_cast<Byte>(T_Super::green()); }
Byte        DwgCmEntityColor::GetBlue () const    { return static_cast<Byte>(T_Super::blue()); }
bool        DwgCmEntityColor::IsByLayer () const  { return T_Super::isByLayer(); }
bool        DwgCmEntityColor::IsByBlock () const  { return T_Super::isByBlock(); }
bool        DwgCmEntityColor::IsByACI () const    { return T_Super::isByACI(); }
bool        DwgCmEntityColor::IsByColor () const  { return T_Super::isByColor(); }
bool        DwgCmEntityColor::IsForeground () const { return T_Super::isForeground(); }
bool        DwgCmEntityColor::IsNone () const     { return T_Super::isNone(); }
DwgCmEntityColor::~DwgCmEntityColor ()            { ; }
DwgCmEntityColor::Method DwgCmEntityColor::GetColorMethod () const { return static_cast<DwgCmEntityColor::Method>(T_Super::colorMethod()); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbStatus DwgCmEntityColor::SetColorIndex (uint16_t color)
    {
#ifdef DWGTOOLKIT_OpenDwg
    T_Super::setColorIndex (color);
    return  DwgDbStatus::Success;
#elif DWGTOOLKIT_RealDwg
    return  ToDwgDbStatus (T_Super::setColorIndex(color));
#endif
    }

uint8_t     DwgTransparency::GetAlpha () const  { return static_cast<uint8_t>(T_Super::alpha()); }
bool        DwgTransparency::IsByLayer () const { return T_Super::isByLayer(); }
bool        DwgTransparency::IsByBlock () const { return T_Super::isByBlock(); }
bool        DwgTransparency::IsByAlpha () const { return T_Super::isByAlpha(); }
bool        DwgTransparency::IsClear () const   { return T_Super::isClear(); }
uint32_t    DwgTransparency::SerializeOut () const { return T_Super::serializeOut(); }
DwgTransparency::DwgTransparency () : DWGCM_Type(Transparency)() { ; }
DwgTransparency::~DwgTransparency ()            { ; }

double      DwgDbDate::GetJulianFraction () const { return T_Super::julianFraction(); }
void        DwgDbDate::GetDate (int16_t& month, int16_t& day, int16_t& year) const { T_Super::getDate(month, day, year); }
void        DwgDbDate::GetTime (int16_t& h, int16_t& m, int16_t& s, int16_t& ms) const { T_Super::getTime(h, m, s, ms); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgBinaryData::DwgBinaryData ()
    {
    m_buffer.clear ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgBinaryData::Set (uint8_t const* buf, uint32_t nBytes)
    {
    m_buffer.clear ();
    for (size_t i = 0; i < nBytes; i++)
        m_buffer.push_back (buf[i]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgBinaryData::Set (char const* buf, uint32_t nBytes)
    {
    m_buffer.clear ();
    for (size_t i = 0; i < nBytes; i++)
        m_buffer.push_back (static_cast<uint8_t>(buf[i]));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/16
+---------------+---------------+---------------+---------------+---------------+------*/
size_t          DwgBinaryData::GetSize () const
    {
    return  m_buffer.size ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/16
+---------------+---------------+---------------+---------------+---------------+------*/
void const*     DwgBinaryData::GetBuffer () const
    {
    return  &m_buffer.front();
    }
