/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/DgnEC/DgnECTypeAdapters.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

//! @cond DONTINCLUDEINDOC
#if defined (_MSC_VER)
    #pragma managed(push, off)
#endif // defined (_MSC_VER)

#include <DgnPlatform/DgnHandlers/DgnECTypes.h>
#include <ECUnits/Units.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//  To ease caching of commonly-accessed string resources
#define DEFINE_TYPE_ADAPTER_RESOURCE_CACHE(STRID)  \
    struct Has ## STRID ## String \
        { \
    private:    WString     m_ ## STRID ## String ; \
    protected:  /*ctor*/    Has ## STRID ## String() \
        { \
        m_ ## STRID ## String = DgnHandlersMessage::GetStringW(DgnHandlersMessage::MSGID_ECTYPEADAPTER_##STRID); \
        } \
    protected:  WCharCP     Get ## STRID ## String() const { return m_ ## STRID ## String.c_str(); } \
        };

DEFINE_TYPE_ADAPTER_RESOURCE_CACHE(ParensNull)
DEFINE_TYPE_ADAPTER_RESOURCE_CACHE(ParensNone)
DEFINE_TYPE_ADAPTER_RESOURCE_CACHE(ByLevel)
DEFINE_TYPE_ADAPTER_RESOURCE_CACHE(ByCell)
DEFINE_TYPE_ADAPTER_RESOURCE_CACHE(SheetScaleCustom)
DEFINE_TYPE_ADAPTER_RESOURCE_CACHE(KB);
DEFINE_TYPE_ADAPTER_RESOURCE_CACHE(True);
DEFINE_TYPE_ADAPTER_RESOURCE_CACHE(False);

#undef DEFINE_TYPE_ADAPTER_RESOURCE_CACHE

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct TypeAdapterUnitsSerialization
    {
private:
    enum Accessor               { Accessor_Units, Accessor_Base, Accessor_System, Accessor_Numerator, Accessor_Denominator, Accessor_Label };
    enum AccessorPrefix         { AccessorPrefix_None, AccessorPrefix_Master, AccessorPrefix_Secondary };

    static const Int32          s_DefaultBase               = 1,
                                s_DefaultSystem             = 1,
                                s_DefaultNumerator          = 254,
                                s_DefaultDenominator        = 10000;

    static void                 PreprocessUnits (ECN::IECInstanceR fmtr, AccessorPrefix prefix);
    static void                 PostProcessUnits (ECN::IECInstanceR fmtr, AccessorPrefix prefix);
protected:
    virtual bool                _SupportsSecondaryUnits() const = 0;
    
    void                        Process (ECN::IECInstanceR fmtr, bool post, IDgnECTypeAdapter const& dwgSupport) const;
    };

//////////////////////////////////////////////////////////////////
//
//  Base adapters providing common formatting functionality
//
/////////////////////////////////////////////////////////////////

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct StringFormatTypeAdapter : IDgnECTypeAdapter, HasParensNullString
    {
    enum CaseSpecification
        { CaseSpec_None, CaseSpec_Upper, CaseSpec_Lower, CaseSpec_First, CaseSpec_Title };
protected:
    StringFormatTypeAdapter() { }

    virtual bool                _ConvertToString (WStringR valueAsString, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP formatter) const override;
    virtual bool                _IsTreatedAsString() const override { return true; }
    virtual bool                GetUnformattedStringValue (WStringR str, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) const = 0;
public:
    static void ConvertCase (WStringR str, IECInstanceCP fmtr);
    static void InitOptions (ECN::IECInstanceR opts);
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct BooleanFormatTypeAdapter : IDgnECTypeAdapter, HasParensNullString
    {
    enum BoolWord
        { BoolWord_Default, BoolWord_TrueFalse, BoolWord_YesNo, BoolWord_OnOff, BoolWord_EnabledDisabled };
private:
    WString         m_trueFalseStrings[4][2];
protected:
    BooleanFormatTypeAdapter();

    virtual bool                _Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) const override;
    virtual bool                _ConvertFromString (ECN::ECValueR v, WCharCP stringValue, IDgnECTypeAdapterContextCR context) const override;
    virtual bool                _ConvertToString (WStringR str, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP formatter) const override;
    virtual bool                _GetDisplayType (ECN::PrimitiveType& primType) const override { primType = ECN::PRIMITIVETYPE_String; return true; }
    virtual bool                _ConvertToDisplayType (ECN::ECValueR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP opts) const override;

    virtual WString             GetBooleanString (bool booleanValue, IDgnECTypeAdapterContextCR context) const = 0;

    WCharCP                     GetResourceString (BoolWord type, bool value) const;
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct AngleFormatTypeAdapter : IDgnECTypeAdapter, HasParensNullString
    {
protected:
    AngleFormatTypeAdapter() { }

    virtual bool                _CanConvertFromString (IDgnECTypeAdapterContextCR context) const override { return false; } // WIP_TOPAZ_MERGE: This needs an actual implementation.  The merge from topaz doesn't actually work on graphite
    virtual bool                _Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) const override;
    virtual bool                _GetDisplayType (ECN::PrimitiveType& type) const override { type = ECN::PRIMITIVETYPE_Double; return true; }
    virtual bool                _ConvertToDisplayType (ECN::ECValueR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP opts) const override;
    virtual bool                _SupportsUnits() const override { return true; }
    virtual bool                _GetUnits (ECN::UnitSpecR units, IDgnECTypeAdapterContextCR context) const override;
    static void                 InitOptions (ECN::IECInstanceR opts);
    static void                 InitFormatter (AngleFormatter& fmtr, ECN::IECInstanceCP opts);
    };

/*---------------------------------------------------------------------------------**//**
* Base class for simple TypeAdapters.
* No special formatting behavior.
* @bsistruct                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct PrimitiveAdapterBase : IDgnECTypeAdapter
    {
private:
    ECN::PrimitiveType           m_primitiveType;
protected:
    PrimitiveAdapterBase (ECN::PrimitiveType type) : m_primitiveType (type) { }

    virtual bool            _Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) const override;
    virtual bool            _ConvertToString (WStringR valueAsString, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP formatter) const override;

    virtual bool            _GetDisplayType (ECN::PrimitiveType& type) const override { type = m_primitiveType; return true; }
    virtual bool            _ConvertToDisplayType (ECN::ECValueR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP opts) const override { return true; }
    };

////////////////////////////////////////////////////////////////
//
//  TypeAdapters for primitive/unextended types
//
////////////////////////////////////////////////////////////////

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct BooleanTypeAdapter : BooleanFormatTypeAdapter
    {
private: virtual WString             GetBooleanString (bool booleanValue, IDgnECTypeAdapterContextCR context) const override;
protected: BooleanTypeAdapter() { }
public:
    static IDgnECTypeAdapterPtr Create () { return new BooleanTypeAdapter(); }
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct DoubleTypeAdapter : IDgnECTypeAdapter, HasParensNullString
    {
protected:
    DoubleTypeAdapter() { }

    virtual bool                _Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) const override;
    virtual bool                _ConvertFromString (ECN::ECValueR v, WCharCP str, IDgnECTypeAdapterContextCR context) const override;
    virtual bool                _ConvertToString (WStringR str, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP formatter) const override;
    virtual bool                _GetDisplayType (ECN::PrimitiveType& type) const override { type = ECN::PRIMITIVETYPE_Double; return true; }
    virtual bool                _ConvertToDisplayType (ECN::ECValueR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP opts) const override;
public:
    static void                 InitFormatter (DoubleFormatterBase& fmtr, ECN::IECInstanceCP options);
    static void                 InitOptions (ECN::IECInstanceR options);
    static void                 ApplyPrefixSuffix (WStringR str, ECN::IECInstanceCP options);
    static bool                 ConvertToDisplayType (ECN::ECValueR v, ECN::IECInstanceCP opts);

    static IDgnECTypeAdapterPtr Create() { return new DoubleTypeAdapter(); }
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct IntegerTypeAdapter : IDgnECTypeAdapter, HasParensNullString
    {
private:
    IntegerTypeAdapter() { }

    virtual bool                _Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) const override;
    virtual bool                _ConvertFromString (ECN::ECValueR v, WCharCP str, IDgnECTypeAdapterContextCR context) const override;
    virtual bool                _ConvertToString (WStringR str, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP opts) const override;
    virtual bool                _GetDisplayType (ECN::PrimitiveType& type) const override { type = ECN::PRIMITIVETYPE_Integer; return true; }
    virtual bool                _ConvertToDisplayType (ECN::ECValueR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP opts) const override { return true; }

    void                        ExtractFormatString (WStringR fmt, ECN::IECInstanceCP opts) const;
public:
    static IDgnECTypeAdapterPtr Create() { return new IntegerTypeAdapter(); }
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct StringTypeAdapter : StringFormatTypeAdapter
    {
private:
    StringTypeAdapter() { }

    virtual bool                _ConvertFromString (ECN::ECValueR v, WCharCP str, IDgnECTypeAdapterContextCR context) const override;
    virtual bool                _Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) const override;
    virtual bool                GetUnformattedStringValue (WStringR str, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) const override;
public:
    static IDgnECTypeAdapterPtr Create () { return new StringTypeAdapter(); }
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct DateTimeTypeAdapter : IDgnECTypeAdapter, HasParensNullString
    {
private:
    DateTimeTypeAdapter() { }

    virtual bool                _Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) const override;
    virtual bool                _CanConvertFromString (IDgnECTypeAdapterContextCR context) const override { return false; }
    virtual bool                _ConvertFromString (ECN::ECValueR v, WCharCP str, IDgnECTypeAdapterContextCR context) const override   { return false; }
    virtual bool                _ConvertToString (WStringR str, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP opts) const override;

    static void                 FormatWithInstructions(WStringR str, WStringCR fmt, DateTimeCR);
    static void                 FormatWithPicture(WStringR str, WStringCR fmt, DateTimeCR);
public:
    static void                 FormatDateTime(WStringR str, WStringCR fmt, DateTimeCR);

    static IDgnECTypeAdapterPtr Create() { return new DateTimeTypeAdapter(); }
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct DateTimeFormatTypeAdapter : IDgnECTypeAdapter
    {
private:
    DateTimeFormatTypeAdapter() { }

    virtual bool                _Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) const override;
    virtual bool                _ConvertFromString (ECN::ECValueR v, WCharCP str, IDgnECTypeAdapterContextCR context) const override;
    virtual bool                _ConvertToString (WStringR str, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP opts) const override;
    virtual bool                _HasStandardValues() const override { return true; }
    virtual bool                _GetStandardValues (StandardValuesCollection& values, IDgnECTypeAdapterContextCR context) const override;
public:
    static Int32                LookupFormatIndex (WCharCP dateTimeFormatString);
    static IDgnECTypeAdapterPtr Create() { return new DateTimeFormatTypeAdapter(); }
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct Point3DTypeAdapter : PrimitiveAdapterBase
    {
private:
    Point3DTypeAdapter() : PrimitiveAdapterBase (PRIMITIVETYPE_Point3D) { }

    virtual bool        _ConvertFromString (ECN::ECValueR v, WCharCP str, IDgnECTypeAdapterContextCR context) const override;
public:
    static IDgnECTypeAdapterPtr Create() { return new Point3DTypeAdapter(); }
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct Point2DTypeAdapter : PrimitiveAdapterBase
    {
private:
    Point2DTypeAdapter() : PrimitiveAdapterBase (PRIMITIVETYPE_Point2D) { }

    virtual bool        _ConvertFromString (ECN::ECValueR v, WCharCP str, IDgnECTypeAdapterContextCR context) const override;
public:
    static IDgnECTypeAdapterPtr Create() { return new Point2DTypeAdapter(); }
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct LongTypeAdapter : PrimitiveAdapterBase
    {
private:
    LongTypeAdapter() : PrimitiveAdapterBase (PRIMITIVETYPE_Long) { }

    virtual bool        _ConvertFromString (ECN::ECValueR v, WCharCP str, IDgnECTypeAdapterContextCR context) const override;
public:
    static IDgnECTypeAdapterPtr Create() { return new LongTypeAdapter(); }
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct BinaryTypeAdapter : PrimitiveAdapterBase
    {
private:
    BinaryTypeAdapter() : PrimitiveAdapterBase (PRIMITIVETYPE_Binary) { }

    virtual bool        _ConvertFromString (ECN::ECValueR v, WCharCP str, IDgnECTypeAdapterContextCR context) const override;
    virtual bool        _GetDisplayType (ECN::PrimitiveType& type) const override { return false; }
    virtual bool        _ConvertToDisplayType (ECN::ECValueR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP opts) const override { return false; }
public:
    static IDgnECTypeAdapterPtr Create() { return new BinaryTypeAdapter(); }
    };

/*---------------------------------------------------------------------------------**//**
* Type adapter for struct array elements.
* @bsistruct                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct StructTypeAdapter : IDgnECTypeAdapter
    {
protected:
    StructTypeAdapter () { }

    virtual bool            _Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) const override;
    virtual bool            _ConvertToString (WStringR valueAsString, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP formatter) const override;
    virtual bool            _ConvertFromString (ECN::ECValueR v, WCharCP stringValue, IDgnECTypeAdapterContextCR context) const override;
    virtual bool            _CanConvertToString (IDgnECTypeAdapterContextCR context) const override { return false; }
    virtual bool            _CanConvertFromString (IDgnECTypeAdapterContextCR context) const override { return false; }
    virtual bool            _IsStruct() const override { return true; }
public:
    static IDgnECTypeAdapterPtr Create () { return new StructTypeAdapter(); }
    };

/*---------------------------------------------------------------------------------**//**
* TypeAdapter for an ExtendedType for which a specialized TypeAdapter could not be
* located.
* Conversion from string and validation will always fail, because it cannot know how to
* interpret its underlying value; this effectively makes the associated property read-only.
* @bsistruct                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct MissingExtendTypeAdapter : IDgnECTypeAdapter, HasParensNullString
    {
private:
    MissingExtendTypeAdapter () { }

    virtual bool            _Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) const override { return false; }
    virtual bool            _ConvertFromString (ECN::ECValueR v, WCharCP stringValue, IDgnECTypeAdapterContextCR context) const override { return false; }
    virtual bool            _CanConvertFromString (IDgnECTypeAdapterContextCR context) const override { return false; }
    virtual bool            _ConvertToString (WStringR str, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP opts) const override;
public:
    static IDgnECTypeAdapterPtr Create () { return new MissingExtendTypeAdapter (); }
    };

/*---------------------------------------------------------------------------------**//**
* Provides standard values and int<->string conversion for properties with
* StandardValues custom attribute.
* @bsistruct                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct StandardValuesTypeAdapter : StringFormatTypeAdapter
    {
    typedef bvector<Int32>  UnderlyingValuesCollection;
private:

    struct ValueMapIterator
        {
    private:
        ECN::IECInstancePtr      m_instance;
        UInt32                  m_count;
        UInt32                  m_currentIndex;
        WString                 m_name;
        Int32                   m_value;

        void Populate ();
        void SetAtEnd()         { m_count = m_currentIndex = m_value = 0; m_name = L""; }
    public:
        ValueMapIterator (ECN::ECPropertyCR ecprop);

        WCharCP                 GetName() const         { return m_name.c_str(); }
        Int32                   GetValue() const        { return m_value; }
        bool                    IsEnd() const           { return m_instance.IsNull() || m_currentIndex >= m_count; }
        void                    MoveNext();
        };

    virtual bool            GetUnformattedStringValue (WStringR valueAsString, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) const override;
    virtual bool            _ConvertFromString (ECN::ECValueR v, WCharCP stringValue, IDgnECTypeAdapterContextCR context) const override;
    virtual bool            _HasStandardValues() const override { return true; }
    virtual bool            _GetStandardValues (StandardValuesCollection& values, IDgnECTypeAdapterContextCR context) const override;
protected:
    StandardValuesTypeAdapter() { }

    virtual bool            _Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) const override;
    virtual bool            _IsTreatedAsString() const override { return true; }
public:
    static IDgnECTypeAdapterPtr   Create()    { return new StandardValuesTypeAdapter(); }
    };

/*---------------------------------------------------------------------------------**//**
* TypeAdapter for properties of any primitive type with a FormatString custom attribute.
* The custom attribute contains a .NET-style format string used to format the value.
* @bsistruct                                                    Paul.Connelly   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct FormatStringTypeAdapter : StringFormatTypeAdapter, HasTrueString, HasFalseString
    {
private:
    FormatStringTypeAdapter() { }

    virtual bool            _Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) const override;
    virtual bool            GetUnformattedStringValue (WStringR valueAsString, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) const override;
    virtual bool            _ConvertFromString (ECN::ECValueR v, WCharCP stringValue, IDgnECTypeAdapterContextCR context) const override;
    virtual bool            _HasStandardValues() const override     { return false; }
public:
    static IDgnECTypeAdapterPtr   Create()        { return new FormatStringTypeAdapter(); }
    };

/*---------------------------------------------------------------------------------**//**
* TypeAdapter for boolean properties with BooleanDisplay custom attribute.
* @bsistruct                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct BooleanDisplayTypeAdapter : BooleanFormatTypeAdapter
    {
private:
    BooleanDisplayTypeAdapter() { }

    virtual bool            _IsTreatedAsString() const override         { return true; }
    virtual bool            _HasStandardValues () const override        { return true; }
    virtual bool            _GetStandardValues (StandardValuesCollection& values, IDgnECTypeAdapterContextCR context) const override;
    virtual WString         GetBooleanString (bool booleanValue, IDgnECTypeAdapterContextCR context) const override;

    ECN::IECInstancePtr      GetCustomAttributeInstance (IDgnECTypeAdapterContextCR context) const;
public:
    static IDgnECTypeAdapterPtr   Create()        { return new BooleanDisplayTypeAdapter; }
    };

///////////////////////////////////////////////////////////////////////////
//
//  TypeAdapters for extended types.
//      These can be used by managed code, unless they represent structs
//
///////////////////////////////////////////////////////////////////////////

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct PointFormatTypeAdapter : IDgnECTypeAdapter
    {
private:
    virtual bool                _Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) const override;

    bool                        ExtractOptions (DPoint3d& pt, bool& useX, bool& useY, bool& useZ, WChar& separator, ECValueCR v, ECN::IECInstanceCP opts, IDgnECTypeAdapterContextCR context) const;
protected:
    PointFormatTypeAdapter() { }

    void                        InitOptions (ECN::IECInstanceR opts) const;
    
    virtual bool                UseGlobalOrigin() const = 0;

    template <typename FMTR>
    bool Format (WStringR str, ECN::ECValueCR v, FMTR const& fmtr, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP opts) const
        {
        DPoint3d pt;
        bool useX, useY, useZ;
        WChar sepChar;

        if (v.IsNull())
            {
            str = DgnHandlersMessage::GetStringW(DgnHandlersMessage::MSGID_ECTYPEADAPTER_ParensNull);
            return true;
            }
        else if (!ExtractOptions (pt, useX, useY, useZ, sepChar, v, opts, context))
            return false;

        WChar sep[3] = L"@ ";
        sep[0] = sepChar;
        if (useX)
            {
            str.append (fmtr.ToString (pt.x));
            if (useY || useZ)
                str.append (sep);
            }
        if (useY)
            {
            str.append (fmtr.ToString (pt.y));
            if (useZ)
                str.append (sep);
            }
        if (useZ)
            str.append (fmtr.ToString (pt.z));

        if (-1 == context.GetComponentIndex())
            DoubleTypeAdapter::ApplyPrefixSuffix (str, opts);

        StringFormatTypeAdapter::ConvertCase (str, opts);
        return true;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct UnitlessPointTypeAdapter : PointFormatTypeAdapter
    {
private:
    UnitlessPointTypeAdapter() { }

    virtual bool                _ConvertToString (WStringR valueAsString, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP formatter) const override;
    virtual bool                _ConvertFromString (ECN::ECValueR v, WCharCP stringValue, IDgnECTypeAdapterContextCR context) const override;
    virtual bool                UseGlobalOrigin() const override { return false; }
public:
    static IDgnECTypeAdapterPtr     Create()        { return new UnitlessPointTypeAdapter(); }
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct FileSizeTypeAdapter : IDgnECTypeAdapter, HasKBString
    {
protected:
    FileSizeTypeAdapter() { }

    virtual bool            _Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) const override;
    virtual bool            _ConvertToString (WStringR valueAsString, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP formatter) const override;
    virtual bool            _ConvertFromString (ECN::ECValueR v, WCharCP stringValue, IDgnECTypeAdapterContextCR context) const override;
    virtual bool            _CanConvertFromString (IDgnECTypeAdapterContextCR context) const override { return false; }
    virtual bool            _GetDisplayType (ECN::PrimitiveType& type) const override { type = ECN::PRIMITIVETYPE_Long; return true; }
    virtual bool            _ConvertToDisplayType (ECN::ECValueR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP opts) const override { return true; }
public:
    static Int32                  LookupFormatIndex (WCharCP formatString);
    static IDgnECTypeAdapterPtr   Create()        { return new FileSizeTypeAdapter(); }
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct FileSizeLongTypeAdapter : FileSizeTypeAdapter
    {
private:
    WString     m_BytesString;
    WString     m_MBString;
    WString     m_GBString;

    FileSizeLongTypeAdapter();

    virtual bool            _ConvertToString (WStringR valueAsString, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP formatter) const override;
    virtual bool            _ConvertFromString (ECN::ECValueR v, WCharCP stringValue, IDgnECTypeAdapterContextCR context) const override;
    virtual bool            _CanConvertFromString (IDgnECTypeAdapterContextCR context) const override { return true; }
public:
    static IDgnECTypeAdapterPtr   Create()        { return new FileSizeLongTypeAdapter(); }
    };

/*---------------------------------------------------------------------------------**//**
* Supports conversion to string.
* @bsistruct                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct ReadonlyTypeAdapter : StringFormatTypeAdapter
    {
protected:
    ReadonlyTypeAdapter() { }

    virtual bool            _Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) const override { return false; }
    virtual bool            _ConvertFromString (ECN::ECValueR v, WCharCP stringValue, IDgnECTypeAdapterContextCR context) const override { return false; }
    virtual bool            _CanConvertFromString (IDgnECTypeAdapterContextCR context) const override { return false; }
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct AngleTypeAdapter : AngleFormatTypeAdapter
    {
protected:
    AngleTypeAdapter () { }

    virtual bool                _ConvertToString (WStringR valueAsString, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP formatter) const override;
    virtual bool                _ConvertFromString (ECN::ECValueR v, WCharCP stringValue, IDgnECTypeAdapterContextCR context) const override;
public:
    static IDgnECTypeAdapterPtr Create () { return new AngleTypeAdapter(); }
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct DirectionAngleTypeAdapter : AngleFormatTypeAdapter
    {
protected:
    DirectionAngleTypeAdapter () { }

    virtual bool                _ConvertToString (WStringR valueAsString, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP formatter) const override;
    virtual bool                _ConvertFromString (ECN::ECValueR v, WCharCP stringValue, IDgnECTypeAdapterContextCR context) const override;
public:
    static IDgnECTypeAdapterPtr Create () { return new DirectionAngleTypeAdapter(); }
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct DistanceTypeAdapter : IDgnECTypeAdapter, TypeAdapterUnitsSerialization
    {
protected:
    DistanceTypeAdapter () { }

    enum { Format_MU = 0, Format_MU_Label = 1, Format_SU = 2, Format_SU_Label = 3, Format_MUSU = 4, Format_MUSU_Label = 5 };

    virtual bool                _Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) const override;
    virtual bool                _ConvertToString (WStringR valueAsString, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP formatter) const override;
    virtual bool                _ConvertFromString (ECN::ECValueR v, WCharCP stringValue, IDgnECTypeAdapterContextCR context) const override;
    virtual void                _PreprocessFormatterForSerialization (ECN::IECInstanceR formatter) const override    { return TypeAdapterUnitsSerialization::Process (formatter, false, *this); }
    virtual void                _PostProcessDeserializedFormatter (ECN::IECInstanceR formatter) const override       { return TypeAdapterUnitsSerialization::Process(formatter, true, *this); }

    virtual bool                _SupportsSecondaryUnits() const override { return true; }
public:
    static void             InitOptions (ECN::IECInstanceR opts);
    static void             InitFormatter (DistanceFormatterR fmtr, DgnModelP model, ECN::IECInstanceCP opts);

    static IDgnECTypeAdapterPtr Create () { return new DistanceTypeAdapter(); }
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct AreaOrVolumeTypeAdapter : IDgnECTypeAdapter, HasParensNullString, TypeAdapterUnitsSerialization
    {
private:
    virtual bool                _Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) const override;
    virtual bool                _ConvertFromString (ECN::ECValueR v, WCharCP str, IDgnECTypeAdapterContextCR context) const override;
    virtual bool                _CanConvertFromString (IDgnECTypeAdapterContextCR context) const override { return false; }
    virtual void                _PreprocessFormatterForSerialization (ECN::IECInstanceR formatter) const override    { return TypeAdapterUnitsSerialization::Process (formatter, false, *this); }
    virtual void                _PostProcessDeserializedFormatter (ECN::IECInstanceR formatter) const override       { return TypeAdapterUnitsSerialization::Process(formatter, true, *this); }

    virtual bool                _SupportsSecondaryUnits() const override { return false; }
protected:
    AreaOrVolumeTypeAdapter() { }

    enum { USE_ACRES = -3, USE_ACTIVE_SUBUNITS = -2 };

    virtual WCharCP GetClassName() const = 0;

    void            InitFormatter (AreaOrVolumeFormatterBase& fmtr, ECN::IECInstanceCP opts, DgnModelP model) const;
    void            ApplyFormatting (WStringR str, ECN::IECInstanceCP opts) const;
    void            InitOptions (ECN::IECInstanceR opts) const;
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct AreaTypeAdapter : AreaOrVolumeTypeAdapter
    {
private:
    AreaTypeAdapter() { }

    virtual bool                _ConvertToString (WStringR str, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP formatter) const override;
    virtual bool                _ConvertToDisplayType (ECN::ECValueR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP opts) const override;
    virtual WCharCP             GetClassName() const override { return L"AreaClass"; }
    virtual bool                _GetUnits (ECN::UnitSpecR units, IDgnECTypeAdapterContextCR context) const override;
public:
    static IDgnECTypeAdapterPtr Create() { return new AreaTypeAdapter(); }
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct VolumeTypeAdapter : AreaOrVolumeTypeAdapter
    {
private:
    VolumeTypeAdapter() { }

    virtual bool                _ConvertToString (WStringR str, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP formatter) const override;
    virtual bool                _ConvertToDisplayType (ECN::ECValueR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP opts) const override;
    virtual WCharCP             GetClassName() const override { return L"Vol"; }
    virtual bool                _GetUnits (ECN::UnitSpecR units, IDgnECTypeAdapterContextCR context) const override;
public:
    static IDgnECTypeAdapterPtr Create() { return new VolumeTypeAdapter(); }
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct CoordinatesTypeAdapter : PointFormatTypeAdapter, TypeAdapterUnitsSerialization
    {
protected:
    CoordinatesTypeAdapter () { }

    virtual bool                _ConvertToString (WStringR valueAsString, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP formatter) const override;
    virtual bool                _ConvertFromString (ECN::ECValueR v, WCharCP stringValue, IDgnECTypeAdapterContextCR context) const override;
    virtual bool                UseGlobalOrigin() const override { return true; }
    virtual void                _PreprocessFormatterForSerialization (ECN::IECInstanceR formatter) const override    { return TypeAdapterUnitsSerialization::Process (formatter, false, *this); }
    virtual void                _PostProcessDeserializedFormatter (ECN::IECInstanceR formatter) const override       { return TypeAdapterUnitsSerialization::Process(formatter, true, *this); }

    virtual bool                _SupportsUnits() const override { return true; }
    virtual bool                _SupportsSecondaryUnits() const override { return true; }
    virtual bool                _GetUnits (ECN::UnitSpecR units, IDgnECTypeAdapterContextCR context) const override;
public:
    static IDgnECTypeAdapterPtr Create () { return new CoordinatesTypeAdapter(); }
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct XyzRotationsTypeAdapter : PointFormatTypeAdapter
    {
private:
    XyzRotationsTypeAdapter () { }

    virtual bool            _ConvertToString (WStringR valueAsString, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP formatter) const override;
    virtual bool            _ConvertFromString (ECN::ECValueR v, WCharCP stringValue, IDgnECTypeAdapterContextCR context) const override;
    virtual bool            UseGlobalOrigin() const override { return false; }
public:
    static IDgnECTypeAdapterPtr Create () { return new XyzRotationsTypeAdapter(); }
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct UnitDefinitionTypeAdapter : StringFormatTypeAdapter, HasSheetScaleCustomString
    {
protected:
    UnitDefinitionTypeAdapter() { }

    virtual bool                _Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) const override;
    virtual bool                _ConvertFromString (ECN::ECValueR v, WCharCP str, IDgnECTypeAdapterContextCR context) const override;
    virtual bool                _HasStandardValues() const override { return true; }
    virtual bool                _GetStandardValues (StandardValuesCollection& values, IDgnECTypeAdapterContextCR context) const override;
    virtual bool                GetUnformattedStringValue (WStringR str, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) const override;
public:
    static IDgnECTypeAdapterPtr Create() { return new UnitDefinitionTypeAdapter(); }
    };
    
/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
template <class T_BaseAdapter>
struct VariableTypeAdapter : T_BaseAdapter
    {
private:
    virtual bool                _Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) const override
        { return (!v.IsNull() && IsSigil (v)) ? false : T_BaseAdapter::_Validate (ToBase (v), context); }
    virtual bool                _ConvertToString (WStringR str, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP opts) const override
        {
        if (!v.IsNull() && IsSigil (v))     { str = DgnHandlersMessage::GetStringW(DgnHandlersMessage::MSGID_ECTYPEADAPTER_VariesAcross); return true; }
        else                                { return T_BaseAdapter::_ConvertToString (str, !v.IsNull() ? ToBase (v) : v, context, opts); };
        }
protected:
    typedef VariableTypeAdapter<T_BaseAdapter> T_SUPER;
    VariableTypeAdapter() : T_BaseAdapter() { }

    virtual ECN::ECValue ToBase (ECN::ECValueCR v) const { return v; }
    virtual bool        IsSigil (ECN::ECValueCR v) const = 0;
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct VariableAngleTypeAdapter : VariableTypeAdapter<AngleTypeAdapter>
    {
private: VariableAngleTypeAdapter() : T_SUPER() { } 
private: bool IsSigil (ECN::ECValueCR v) const { return v.IsDouble() && v.GetDouble() == GetSigil(); }
public: static double GetSigil() { return DISCONNECT; }
public: static IDgnECTypeAdapterPtr Create() { return new VariableAngleTypeAdapter(); }
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct VariableDistanceTypeAdapter : VariableTypeAdapter<DistanceTypeAdapter>
    {
private: VariableDistanceTypeAdapter() : T_SUPER() { }
private: bool IsSigil (ECN::ECValueCR v) const { return v.IsDouble() && v.GetDouble() == GetSigil(); }
public: static double GetSigil() { return DISCONNECT; }
public: static IDgnECTypeAdapterPtr Create() { return new VariableDistanceTypeAdapter(); }
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct VariableDoubleTypeAdapter : VariableTypeAdapter<DoubleTypeAdapter>
    {
private: VariableDoubleTypeAdapter() : T_SUPER() { }
private: bool IsSigil (ECN::ECValueCR v) const { return v.IsDouble() && v.GetDouble() == GetSigil(); }
public: static double GetSigil() { return DISCONNECT; }
public: static IDgnECTypeAdapterPtr Create() { return new VariableDoubleTypeAdapter(); }
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct VariableBooleanTypeAdapter : VariableTypeAdapter<BooleanTypeAdapter>
    {
private: VariableBooleanTypeAdapter() : T_SUPER() { }
private:
    bool        IsSigil (ECN::ECValueCR v) const { return v.IsInteger() && GetSigil() == v.GetInteger(); }
    ECN::ECValue ToBase (ECN::ECValueCR v) const
        {
        ECN::ECValue outVal;
        if (v.IsInteger())
            {
            if (v.GetInteger() == 0)        outVal.SetBoolean (false);
            else if (v.GetInteger() == 1)   outVal.SetBoolean (true);
            }
        return outVal;
        }
public: static Int32 GetSigil() { return 0xFF; }
public: static IDgnECTypeAdapterPtr Create() { return new VariableBooleanTypeAdapter(); }
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct VariableEnumTypeAdapter : VariableTypeAdapter<StandardValuesTypeAdapter>
    {
private: VariableEnumTypeAdapter() : T_SUPER() { }
private: bool IsSigil (ECN::ECValueCR v) const { return v.IsInteger() && GetSigil() == v.GetInteger(); }
public: static Int32 GetSigil() { return -999; }
public: static IDgnECTypeAdapterPtr Create() { return new VariableEnumTypeAdapter(); }
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECUnitsTypeAdapter : IDgnECTypeAdapter
    {
private:
    ECUnitsTypeAdapter() { }

    virtual bool                _Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) const override;
    virtual bool                _ConvertToString (WStringR valueAsString, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP formatter) const override;
    virtual bool                _ConvertFromString (ECN::ECValueR v, WCharCP stringValue, IDgnECTypeAdapterContextCR context) const override;
    virtual bool                _GetDisplayType (ECN::PrimitiveType& type) const override { type = ECN::PRIMITIVETYPE_Double; return true; }
    virtual bool                _ConvertToDisplayType (ECN::ECValueR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP opts) const override;
    virtual bool                _SupportsUnits() const override { return true; }
    virtual bool                _GetUnits (ECN::UnitSpecR units, IDgnECTypeAdapterContextCR context) const override;
public:
    static void                 ApplyUnitLabelCustomization (UnitR unit, DgnProjectR dgnFile);

    static IDgnECTypeAdapterPtr Create() { return new ECUnitsTypeAdapter(); }
    };

END_BENTLEY_DGNPLATFORM_NAMESPACE

#if defined (_MSC_VER)
    #pragma managed(pop)
#endif // defined (_MSC_VER)

//! @endcond
