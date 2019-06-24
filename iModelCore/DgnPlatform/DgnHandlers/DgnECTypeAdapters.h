/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

//! @cond DONTINCLUDEINDOC
#if defined (_MSC_VER)
    #pragma managed(push, off)
#endif // defined (_MSC_VER)

#include <DgnPlatform/DgnECTypes.h>
#include <ECUnits/Units.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//  To ease caching of commonly-accessed string resources
#define DEFINE_TYPE_ADAPTER_RESOURCE_CACHE(STRID)  \
    struct Has ## STRID ## String \
                { \
    private:    Utf8String     m_ ## STRID ## String ; \
    protected:  /*ctor*/    Has ## STRID ## String() \
                { \
        m_ ## STRID ## String = DgnCoreL10N::GetString(DgnCoreL10N::ECTYPEADAPTER_##STRID()); \
                } \
    protected:  Utf8CP     Get ## STRID ## String() const { return m_ ## STRID ## String.c_str(); } \
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

    static const int32_t        s_DefaultBase               = 1,
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

    bool                _ConvertToString (Utf8StringR valueAsString, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP formatter) const override;
    bool                _IsTreatedAsString() const override { return true; }
    virtual bool                GetUnformattedStringValue(Utf8StringR str, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) const = 0;
public:
    static void ConvertCase(Utf8StringR str, IECInstanceCP fmtr);
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
    Utf8String         m_trueFalseStrings[4][2];
protected:
    BooleanFormatTypeAdapter();

    bool                _Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) const override;
    bool                _ConvertFromString (ECN::ECValueR v, Utf8CP stringValue, IDgnECTypeAdapterContextCR context) const override;
    bool                _ConvertToString(Utf8StringR str, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP formatter) const override;
    bool                _GetDisplayType (ECN::PrimitiveType& primType) const override { primType = ECN::PRIMITIVETYPE_String; return true; }
    bool                _ConvertToDisplayType (ECN::ECValueR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP opts) const override;
    bool                _GetPlaceholderValue(ECN::ECValueR v, IDgnECTypeAdapterContextCR context) const override;

    virtual Utf8String             GetBooleanString (bool booleanValue, IDgnECTypeAdapterContextCR context) const = 0;

    Utf8CP                     GetResourceString (BoolWord type, bool value) const;
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct AngleFormatTypeAdapter : IDgnECTypeAdapter, HasParensNullString
    {
protected:
    AngleFormatTypeAdapter() { }

    bool                _CanConvertFromString (IDgnECTypeAdapterContextCR context) const override { return false; } // WIP_TOPAZ_MERGE: This needs an actual implementation.  The merge from topaz doesn't actually work on graphite
    bool                _Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) const override;
    bool                _GetDisplayType (ECN::PrimitiveType& type) const override { type = ECN::PRIMITIVETYPE_Double; return true; }
    bool                _ConvertToDisplayType (ECN::ECValueR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP opts) const override;
    bool                _SupportsUnits() const override { return true; }
    bool                _GetUnits (ECN::UnitSpecR units, IDgnECTypeAdapterContextCR context) const override;
    bool                _GetPlaceholderValue(ECN::ECValueR v, IDgnECTypeAdapterContextCR context) const override;
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

    bool            _Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) const override;
    bool            _ConvertToString(Utf8StringR valueAsString, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP formatter) const override;

    bool            _GetDisplayType (ECN::PrimitiveType& type) const override { type = m_primitiveType; return true; }
    bool            _ConvertToDisplayType (ECN::ECValueR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP opts) const override { return true; }
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
private: Utf8String             GetBooleanString (bool booleanValue, IDgnECTypeAdapterContextCR context) const override;
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

    bool                _Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) const override;
    bool                _ConvertFromString (ECN::ECValueR v, Utf8CP str, IDgnECTypeAdapterContextCR context) const override;
    bool                _ConvertToString(Utf8StringR str, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP formatter) const override;
    bool                _GetDisplayType (ECN::PrimitiveType& type) const override { type = ECN::PRIMITIVETYPE_Double; return true; }
    bool                _ConvertToDisplayType (ECN::ECValueR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP opts) const override;
    bool                _GetPlaceholderValue(ECN::ECValueR v, IDgnECTypeAdapterContextCR context) const override { v.SetDouble(123.45); return true; }
public:
    static void                 InitFormatter (DoubleFormatterBase& fmtr, ECN::IECInstanceCP options);
    static void                 InitOptions (ECN::IECInstanceR options);
    static void                 ApplyPrefixSuffix(Utf8StringR str, ECN::IECInstanceCP options);
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

    bool                _Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) const override;
    bool                _ConvertFromString (ECN::ECValueR v, Utf8CP str, IDgnECTypeAdapterContextCR context) const override;
    bool                _ConvertToString(Utf8StringR str, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP opts) const override;
    bool                _GetDisplayType (ECN::PrimitiveType& type) const override { type = ECN::PRIMITIVETYPE_Integer; return true; }
    bool                _ConvertToDisplayType (ECN::ECValueR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP opts) const override { return true; }
    bool                _GetPlaceholderValue(ECN::ECValueR v, IDgnECTypeAdapterContextCR context) const override { v.SetInteger(12345); return true; }

    void                        ExtractFormatString(Utf8StringR fmt, ECN::IECInstanceCP opts) const;
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

    bool                _ConvertFromString (ECN::ECValueR v, Utf8CP str, IDgnECTypeAdapterContextCR context) const override;
    bool                _Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) const override;
    bool                _GetPlaceholderValue(ECN::ECValueR v, IDgnECTypeAdapterContextCR context) const override;
    bool                GetUnformattedStringValue(Utf8StringR str, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) const override;
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

    bool                _Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) const override;
    bool                _CanConvertFromString (IDgnECTypeAdapterContextCR context) const override { return false; }
    bool                _ConvertFromString (ECN::ECValueR v, Utf8CP str, IDgnECTypeAdapterContextCR context) const override   { return false; }
    bool                _ConvertToString(Utf8StringR str, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP opts) const override;
    bool                _IsOrdinalType () const override { return true; };
    bool                _GetPlaceholderValue(ECN::ECValueR v, IDgnECTypeAdapterContextCR context) const override;

    static void                 FormatWithInstructions(Utf8StringR str, Utf8StringCR fmt, DateTimeCR);
    static void                 FormatWithPicture(Utf8StringR str, Utf8StringCR fmt, DateTimeCR);
public:
    static void                 FormatDateTime(Utf8StringR str, Utf8StringCR fmt, DateTimeCR);

    static IDgnECTypeAdapterPtr Create() { return new DateTimeTypeAdapter(); }
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct DateTimeFormatTypeAdapter : IDgnECTypeAdapter
    {
private:
    DateTimeFormatTypeAdapter() { }

    bool                _Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) const override;
    bool                _ConvertFromString (ECN::ECValueR v, Utf8CP str, IDgnECTypeAdapterContextCR context) const override;
    bool                _ConvertToString(Utf8StringR str, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP opts) const override;
    bool                _HasStandardValues() const override { return true; }
    bool                _GetStandardValues (StandardValuesCollection& values, IDgnECTypeAdapterContextCR context) const override;
    bool                _GetPlaceholderValue(ECN::ECValueR v, IDgnECTypeAdapterContextCR context) const override;
public:
    static int32_t              LookupFormatIndex (Utf8CP dateTimeFormatString);
    static IDgnECTypeAdapterPtr Create() { return new DateTimeFormatTypeAdapter(); }
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct PointFormatTypeAdapter : IDgnECTypeAdapter
    {
protected:
    virtual bool                _Is2d() const       { return false; }

    bool                _Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) const override;

    ECN::ECValue                FromStorageType (ECN::ECValueCR v) const;
    ECN::ECValue                ToStorageType (ECN::ECValueCR v) const;
    bool                        ExtractOptions (DPoint3d& pt, bool& useX, bool& useY, bool& useZ, Utf8Char& separator, ECN::ECValueCR v, ECN::IECInstanceCP opts, IDgnECTypeAdapterContextCR context) const;
    PointFormatTypeAdapter() { }

    void                        InitOptions (ECN::IECInstanceR opts) const;
    
    virtual bool                UseGlobalOrigin() const = 0;
    virtual bool                _AllowDoubleType() const { return false; }   // Dumb special-case...people apply Coordinates extended type to double-type properties and 8.11.9 happens to allow it.
    bool                _IsOrdinalType () const override { return true; };
    bool                _GetPlaceholderValue (ECN::ECValueR v, IDgnECTypeAdapterContextCR context) const override;

    static void                 ConvertPointSpecToDwgFormatString (Utf8StringR formatString, ECN::IECInstanceCR fmtr);

    bool Parse (ECN::ECValueR v, Utf8CP str, IDgnECTypeAdapterContextCR context) const;

    template <typename FMTR>
    bool Format (Utf8StringR str, ECN::ECValueCR v, FMTR const& fmtr, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP opts) const
        {
        DPoint3d pt;
        bool useX, useY, useZ;
        Utf8Char sepChar;

        if (v.IsNull())
            {
            str = DgnCoreL10N::GetString(DgnCoreL10N::ECTYPEADAPTER_ParensNull()); 
            return true;
            }
        else if (!ExtractOptions (pt, useX, useY, useZ, sepChar, v, opts, context))
            return false;

        Utf8Char sep[3] = "@ ";
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
protected:
    UnitlessPointTypeAdapter() { }

    bool                _ConvertToString (Utf8StringR valueAsString, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP formatter) const override;
    bool                _ConvertFromString (ECN::ECValueR v, Utf8CP stringValue, IDgnECTypeAdapterContextCR context) const override;
    bool                UseGlobalOrigin() const override { return false; }
public:
    static IDgnECTypeAdapterPtr     Create()        { return new UnitlessPointTypeAdapter(); }
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct Point2DTypeAdapter : UnitlessPointTypeAdapter
    {
private:
    Point2DTypeAdapter() { }

    bool                _Is2d() const override  { return true; }
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

    bool        _ConvertFromString (ECN::ECValueR v, Utf8CP str, IDgnECTypeAdapterContextCR context) const override;
    bool        _GetPlaceholderValue(ECN::ECValueR v, IDgnECTypeAdapterContextCR context) const override { v.SetLong(1234567890L); return true; }
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

    bool        _ConvertFromString (ECN::ECValueR v, Utf8CP str, IDgnECTypeAdapterContextCR context) const override;
    bool        _GetDisplayType (ECN::PrimitiveType& type) const override { return false; }
    bool        _ConvertToDisplayType (ECN::ECValueR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP opts) const override { return false; }
    bool        _GetPlaceholderValue(ECN::ECValueR v, IDgnECTypeAdapterContextCR context) const override { return false; }
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

    bool            _Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) const override;
    bool            _ConvertToString(Utf8StringR valueAsString, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP formatter) const override;
    bool            _ConvertFromString (ECN::ECValueR v, Utf8CP stringValue, IDgnECTypeAdapterContextCR context) const override;
    bool            _CanConvertToString (IDgnECTypeAdapterContextCR context) const override { return false; }
    bool            _CanConvertFromString (IDgnECTypeAdapterContextCR context) const override { return false; }
    bool            _IsStruct() const override { return true; }
    bool            _GetPlaceholderValue(ECN::ECValueR v, IDgnECTypeAdapterContextCR context) const override { return false; }
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

    bool            _Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) const override { return false; }
    bool            _ConvertFromString (ECN::ECValueR v, Utf8CP stringValue, IDgnECTypeAdapterContextCR context) const override { return false; }
    bool            _CanConvertFromString (IDgnECTypeAdapterContextCR context) const override { return false; }
    bool            _ConvertToString(Utf8StringR str, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP opts) const override;
    bool            _GetPlaceholderValue(ECN::ECValueR v, IDgnECTypeAdapterContextCR context) const override { return false; }
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
    typedef bvector<int32_t>  UnderlyingValuesCollection;
private:

    struct ValueMapIterator
        {
    private:
        ECN::IECInstancePtr     m_instance;
        uint32_t                m_count;
        uint32_t                m_currentIndex;
        Utf8String              m_name;
        int32_t                 m_value;

        void Populate ();
        void SetAtEnd()         { m_count = m_currentIndex = m_value = 0; m_name = ""; }
    public:
        ValueMapIterator (ECN::ECPropertyCR ecprop);

        Utf8CP                 GetName() const         { return m_name.c_str(); }
        int32_t                 GetValue() const        { return m_value; }
        bool                    IsEnd() const           { return m_instance.IsNull() || m_currentIndex >= m_count; }
        void                    MoveNext();
        };

    bool            GetUnformattedStringValue(Utf8StringR valueAsString, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) const override;
    bool            _ConvertFromString (ECN::ECValueR v, Utf8CP stringValue, IDgnECTypeAdapterContextCR context) const override;
    bool            _HasStandardValues() const override { return true; }
    bool            _GetStandardValues (StandardValuesCollection& values, IDgnECTypeAdapterContextCR context) const override;
    bool            _GetPlaceholderValue(ECN::ECValueR v, IDgnECTypeAdapterContextCR context) const override;
protected:
    StandardValuesTypeAdapter() { }

    bool            _Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) const override;
    bool            _IsTreatedAsString() const override { return true; }
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

    bool            _Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) const override;
    bool            GetUnformattedStringValue(Utf8StringR valueAsString, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) const override;
    bool            _ConvertFromString (ECN::ECValueR v, Utf8CP stringValue, IDgnECTypeAdapterContextCR context) const override;
    bool            _HasStandardValues() const override     { return false; }
    bool            _GetPlaceholderValue(ECN::ECValueR v, IDgnECTypeAdapterContextCR context) const override;
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

    bool            _IsTreatedAsString() const override         { return true; }
    bool            _HasStandardValues () const override        { return true; }
    bool            _GetStandardValues (StandardValuesCollection& values, IDgnECTypeAdapterContextCR context) const override;
    Utf8String         GetBooleanString (bool booleanValue, IDgnECTypeAdapterContextCR context) const override;

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
struct FileSizeTypeAdapter : IDgnECTypeAdapter, HasKBString
    {
protected:
    FileSizeTypeAdapter() { }

    bool            _Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) const override;
    bool            _ConvertToString(Utf8StringR valueAsString, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP formatter) const override;
    bool            _ConvertFromString (ECN::ECValueR v, Utf8CP stringValue, IDgnECTypeAdapterContextCR context) const override;
    bool            _CanConvertFromString (IDgnECTypeAdapterContextCR context) const override { return false; }
    bool            _GetDisplayType (ECN::PrimitiveType& type) const override { type = ECN::PRIMITIVETYPE_Long; return true; }
    bool            _ConvertToDisplayType (ECN::ECValueR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP opts) const override { return true; }
    bool            _GetPlaceholderValue(ECN::ECValueR v, IDgnECTypeAdapterContextCR context) const override;
public:
    static int32_t                LookupFormatIndex (Utf8CP formatString);
    static IDgnECTypeAdapterPtr   Create()        { return new FileSizeTypeAdapter(); }
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct FileSizeLongTypeAdapter : FileSizeTypeAdapter
    {
private:
    Utf8String     m_BytesString;
    Utf8String     m_MBString;
    Utf8String     m_GBString;

    FileSizeLongTypeAdapter();

    bool            _ConvertToString(Utf8StringR valueAsString, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP formatter) const override;
    bool            _ConvertFromString (ECN::ECValueR v, Utf8CP stringValue, IDgnECTypeAdapterContextCR context) const override;
    bool            _CanConvertFromString (IDgnECTypeAdapterContextCR context) const override { return true; }
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

    bool            _Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) const override { return false; }
    bool            _ConvertFromString (ECN::ECValueR v, Utf8CP stringValue, IDgnECTypeAdapterContextCR context) const override { return false; }
    bool            _CanConvertFromString (IDgnECTypeAdapterContextCR context) const override { return false; }
    bool            _GetPlaceholderValue(ECN::ECValueR v, IDgnECTypeAdapterContextCR context) const override { return false; }
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct AngleTypeAdapter : AngleFormatTypeAdapter
    {
protected:
    AngleTypeAdapter () { }

    bool                _ConvertToString(Utf8StringR valueAsString, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP formatter) const override;
    bool                _ConvertFromString (ECN::ECValueR v, Utf8CP stringValue, IDgnECTypeAdapterContextCR context) const override;
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

    bool                _ConvertToString(Utf8StringR valueAsString, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP formatter) const override;
    bool                _ConvertFromString (ECN::ECValueR v, Utf8CP stringValue, IDgnECTypeAdapterContextCR context) const override;
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

    bool                _Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) const override;
    bool                _ConvertToString(Utf8StringR valueAsString, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP formatter) const override;
    bool                _ConvertFromString (ECN::ECValueR v, Utf8CP stringValue, IDgnECTypeAdapterContextCR context) const override;
    void                _PreprocessFormatterForSerialization (ECN::IECInstanceR formatter) const override    { return TypeAdapterUnitsSerialization::Process (formatter, false, *this); }
    void                _PostProcessDeserializedFormatter (ECN::IECInstanceR formatter) const override       { return TypeAdapterUnitsSerialization::Process(formatter, true, *this); }

    bool                _SupportsSecondaryUnits() const override { return true; }
    bool                _GetPlaceholderValue(ECN::ECValueR v, IDgnECTypeAdapterContextCR context) const override;
public:
    static void             InitOptions (ECN::IECInstanceR opts);
    static void             InitFormatter (DistanceFormatterR fmtr, GeometricModelCP model, ECN::IECInstanceCP opts);

    static IDgnECTypeAdapterPtr Create () { return new DistanceTypeAdapter(); }
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct AreaOrVolumeTypeAdapter : IDgnECTypeAdapter, HasParensNullString, TypeAdapterUnitsSerialization
    {
private:
    bool                _Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) const override;
    bool                _ConvertFromString (ECN::ECValueR v, Utf8CP str, IDgnECTypeAdapterContextCR context) const override;
    bool                _CanConvertFromString (IDgnECTypeAdapterContextCR context) const override { return false; }
    void                _PreprocessFormatterForSerialization (ECN::IECInstanceR formatter) const override    { return TypeAdapterUnitsSerialization::Process (formatter, false, *this); }
    void                _PostProcessDeserializedFormatter (ECN::IECInstanceR formatter) const override       { return TypeAdapterUnitsSerialization::Process(formatter, true, *this); }

    bool                _SupportsSecondaryUnits() const override { return false; }
    bool                _GetPlaceholderValue(ECN::ECValueR v, IDgnECTypeAdapterContextCR context) const override;
protected:
    AreaOrVolumeTypeAdapter() { }

    enum { USE_ACRES = -3, USE_ACTIVE_SUBUNITS = -2 };

    virtual Utf8CP GetClassName() const = 0;

    void            InitFormatter(AreaOrVolumeFormatterBase& fmtr, GeometricModelCP model, ECN::IECInstanceCP opts) const;
    void            ApplyFormatting(Utf8StringR str, ECN::IECInstanceCP opts) const;
    void            InitOptions (ECN::IECInstanceR opts) const;
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct AreaTypeAdapter : AreaOrVolumeTypeAdapter
    {
private:
    AreaTypeAdapter() { }

    bool                _ConvertToString(Utf8StringR str, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP formatter) const override;
    bool                _ConvertToDisplayType (ECN::ECValueR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP opts) const override;
    Utf8CP              GetClassName() const override { return "AreaClass"; }
    bool                _GetUnits (ECN::UnitSpecR units, IDgnECTypeAdapterContextCR context) const override;
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

    bool                _ConvertToString(Utf8StringR str, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP formatter) const override;
    bool                _ConvertToDisplayType (ECN::ECValueR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP opts) const override;
    Utf8CP              GetClassName() const override { return "Vol"; }
    bool                _GetUnits (ECN::UnitSpecR units, IDgnECTypeAdapterContextCR context) const override;
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

    bool                _ConvertToString(Utf8StringR valueAsString, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP formatter) const override;
    bool                _ConvertFromString (ECN::ECValueR v, Utf8CP stringValue, IDgnECTypeAdapterContextCR context) const override;
    bool                UseGlobalOrigin() const override { return true; }
    bool                _AllowDoubleType() const override { return true; }
    void                _PreprocessFormatterForSerialization (ECN::IECInstanceR formatter) const override    { return TypeAdapterUnitsSerialization::Process (formatter, false, *this); }
    void                _PostProcessDeserializedFormatter (ECN::IECInstanceR formatter) const override       { return TypeAdapterUnitsSerialization::Process(formatter, true, *this); }
    bool                _GetPlaceholderValue(ECN::ECValueR v, IDgnECTypeAdapterContextCR context) const override;

    bool                _SupportsUnits() const override { return true; }
    bool                _SupportsSecondaryUnits() const override { return true; }
    bool                _GetUnits (ECN::UnitSpecR units, IDgnECTypeAdapterContextCR context) const override;
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

    bool            _ConvertToString(Utf8StringR valueAsString, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP formatter) const override;
    bool            _ConvertFromString (ECN::ECValueR v, Utf8CP stringValue, IDgnECTypeAdapterContextCR context) const override;
    bool            UseGlobalOrigin() const override { return false; }
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

    bool                _Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) const override;
    bool                _ConvertFromString (ECN::ECValueR v, Utf8CP str, IDgnECTypeAdapterContextCR context) const override;
    bool                _HasStandardValues() const override { return true; }
    bool                _GetStandardValues (StandardValuesCollection& values, IDgnECTypeAdapterContextCR context) const override;
    bool                _GetPlaceholderValue(ECN::ECValueR v, IDgnECTypeAdapterContextCR context) const override;
    bool                GetUnformattedStringValue(Utf8StringR str, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) const override;
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
    bool                _Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) const override
        { return (!v.IsNull() && IsSigil (v)) ? false : T_BaseAdapter::_Validate (ToBase (v), context); }
    bool                _ConvertToString(Utf8StringR str, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP opts) const override
        {
        if (!v.IsNull() && IsSigil(v)) { str = DgnCoreL10N::GetString(DgnCoreL10N::ECTYPEADAPTER_VariesAcross()); return true; }
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
public: static int32_t GetSigil() { return 0xFF; }
public: static IDgnECTypeAdapterPtr Create() { return new VariableBooleanTypeAdapter(); }
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct VariableEnumTypeAdapter : VariableTypeAdapter<StandardValuesTypeAdapter>
    {
private: VariableEnumTypeAdapter() : T_SUPER() { }
private: bool IsSigil (ECN::ECValueCR v) const { return v.IsInteger() && GetSigil() == v.GetInteger(); }
public: static int32_t GetSigil() { return -999; }
public: static IDgnECTypeAdapterPtr Create() { return new VariableEnumTypeAdapter(); }
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECUnitsTypeAdapter : IDgnECTypeAdapter
    {
private:
    ECUnitsTypeAdapter() { }

    bool                _Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) const override;
    bool                _ConvertToString(Utf8StringR valueAsString, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP formatter) const override;
    bool                _ConvertFromString (ECN::ECValueR v, Utf8CP stringValue, IDgnECTypeAdapterContextCR context) const override;
    bool                _GetDisplayType (ECN::PrimitiveType& type) const override { type = ECN::PRIMITIVETYPE_Double; return true; }
    bool                _ConvertToDisplayType (ECN::ECValueR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP opts) const override;
    bool                _SupportsUnits() const override { return true; }
    bool                _GetUnits (ECN::UnitSpecR units, IDgnECTypeAdapterContextCR context) const override;
    bool                _GetPlaceholderValue(ECN::ECValueR v, IDgnECTypeAdapterContextCR context) const override;
public:
    static void                 ApplyUnitLabelCustomization (UnitR unit, DgnModelR dgnFile);

    static IDgnECTypeAdapterPtr Create() { return new ECUnitsTypeAdapter(); }
    };

END_BENTLEY_DGNPLATFORM_NAMESPACE

#if defined (_MSC_VER)
    #pragma managed(pop)
#endif // defined (_MSC_VER)

//! @endcond
