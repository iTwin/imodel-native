/*--------------------------------------------------------------------------------------+
|
|  $Source: DgnHandlers/DgnEC/DgnECTypeAdapters.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>
#include    "DgnECTypeAdapters.h"
#include    "DgnECInteropStringFormatter.h"

#include    <DgnPlatform/DgnCore/ScaleDefinition.h>
#include    <boost/algorithm/string/erase.hpp>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_EC
USING_NAMESPACE_BENTLEY_DGNPLATFORM

#define RSC_STR(X) DgnHandlersMessage::GetStringW(DgnHandlersMessage::MSGID_ECTYPEADAPTER_##X)

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

static const WCharCP    s_fmtInteger        = L"%d",
                        s_fmtLong           = L"%lld",
                        s_fmtDouble         = L"%lf";
static const Int32      USE_ACTIVE          = -1;                   // for formatter instance properties to indicate active settings should be applied.

//  We use this magic internal code USE_ACRES because we don't want
//  to try to squeeze square units into our units system.
static const Int32  USE_ACTIVE_MASTER_UNITS          = -1,
                    USE_ACTIVE_SECONDARY_UNITS       = -2,
                    USE_ACRES                        = -3;

// as defined in Units_Schema.01.00.ecschema.xml
#define BASEUNIT_METER          L"METRE"
#define BASEUNIT_METER2         L"METRE_SQUARED"
#define BASEUNIT_METER3         L"METRE_CUBED"
#define BASEUNIT_RADIAN         L"RADIAN"

/*---------------------------------------------------------------------------------**//**
* VC 2012 still does not implement std::round....
* @bsimethod                                                    Paul.Connelly   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
static double std_round (double d)
    {
    return d < 0.0 ? ceil(d - 0.5) : floor(d + 0.5);
    }

/////////////////////////////////////////////////////////////
//  Convenience routines for extracting formatting options
////////////////////////////////////////////////////////////

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
static bool getValueFromFormatter (ECValueR v, IECInstanceCP fmtr, WCharCP accessString, PrimitiveType type)
    {
    return (NULL != fmtr && ECOBJECTS_STATUS_Success == fmtr->GetValue (v, accessString) && !v.IsNull() && v.IsPrimitive() && v.GetPrimitiveType() == type);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
static Int32 getIntOrDefault (IECInstanceCP fmtr, WCharCP accessString, Int32 def)
    {
    ECValue v;
    return getValueFromFormatter (v, fmtr, accessString, PRIMITIVETYPE_Integer) ? v.GetInteger() : def;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
static double getDoubleOrDefault (IECInstanceCP fmtr, WCharCP accessString, double def)
    {
    ECValue v;
    return getValueFromFormatter (v, fmtr, accessString, PRIMITIVETYPE_Double) ? v.GetDouble() : def;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
static WString getStringOrDefault (IECInstanceCP opts, WCharCP accessString, WCharCP def)
    {
    ECValue v;
    return getValueFromFormatter (v, opts, accessString, PRIMITIVETYPE_String) ? v.GetString() : def;
    }

/*---------------------------------------------------------------------------------**//**
* Some boolean properties are declared as integers in the schema.
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
static bool getBoolOrDefault (IECInstanceCP fmtr, WCharCP accessString, bool def)
    {
    ECValue v;
    if (NULL != fmtr && ECOBJECTS_STATUS_Success == fmtr->GetValue (v, accessString) && v.IsPrimitive() && !v.IsNull())
        {
        if (v.IsInteger())
            {
            if (v.GetInteger() == 0)
                return false;
            else if (v.GetInteger() == 1)
                return true;
            else
                BeAssert (false);
            }
        else if (v.IsBoolean())
            return v.GetBoolean();
        }

    return def;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
static bool getBoolOrDefault (IECInstanceCP fmtr, WCharCP accessString, bool defIfInstanceNull, bool defIfPropertyNull)
    {
    return NULL == fmtr ? defIfInstanceNull : getBoolOrDefault (fmtr, accessString, defIfPropertyNull);
    }

/////////////////////////////////////////////////////
//          Units Serialization
////////////////////////////////////////////////////

static const WCharCP    s_unitsAccessors[3][6] =
    {
        { L"Units", L"Base", L"System", L"Numerator", L"Denominator", L"Label" },
        { L"MasterUnits", L"MasterBase", L"MasterSystem", L"MasterNumerator", L"MasterDenominator", L"MasterLabel" },
        { L"SecondaryUnits", L"SecondaryBase", L"SecondarySystem", L"SecondaryNumerator", L"SecondaryDenominator", L"SecondaryLabel" }
    };
        

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T_ENTRY, typename T_VISITOR, typename T_COLLECTION> static void visitUnits (T_VISITOR& visitor, T_COLLECTION const& collection)
    {
    for (typename T_COLLECTION::const_iterator iter = collection.begin(); iter != collection.end(); ++iter)
        {
        T_ENTRY entry = *iter;
        UnitDefinition unitDef = entry.GetUnitDef();
        if (!visitor.Visit ((Int32)entry.GetNumber(), entry.GetName().c_str(), unitDef))
            break;
        }
    }

/*---------------------------------------------------------------------------------**//**
* Iterate over user units if defined, else iterate over standard units
* @bsimethod                                                    Paul.Connelly   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T_VISITOR> static void visitUnits (T_VISITOR& visitor)
    {
    UnitIteratorOptions opts;
    UserUnitCollection coll (opts);
    if (coll.begin() != coll.end())
        visitUnits <UserUnitCollection::Entry>(visitor, coll);
    else
        visitUnits <StandardUnitCollection::Entry const&> (visitor, StandardUnitCollection (opts));
    }

namespace UnitVisitors
    {
    struct Validater
        {
        Int32               m_num;
        bool                m_found;

        bool Visit (Int32 number, WCharCP name, UnitDefinitionCR unitDef)
            {
            if (number == m_num)
                {
                m_found = true;
                return false;
                }
            return true;
            }

        Validater (Int32 num) : m_num(num), m_found(false) { }
        };

    struct ToStringConverter
        {
        Int32           m_num;
        WStringR        m_name;
        bool            m_found;

        bool Visit (Int32 number, WCharCP name, UnitDefinitionCR unitDef)
            {
            if (number == m_num)
                {
                m_name = name;
                m_found = true;
                return false;
                }
            return true;
            }

        ToStringConverter (Int32 num, WStringR name) : m_num(num), m_name(name), m_found(false) { }
        };

    struct StandardValuesCollector
        {
        IDgnECTypeAdapter::StandardValuesCollection&   m_values;

        bool Visit (Int32 number, WCharCP name, UnitDefinitionCR unitDef)
            {
            m_values.push_back (name);
            return true;
            }

        StandardValuesCollector (IDgnECTypeAdapter::StandardValuesCollection& vals) : m_values (vals) { }
        };

    struct UnitMatcher
        {
        bool            m_ignoreSystem, m_ignoreLabel;
        Int32           m_base, m_system;
        double          m_num, m_denom;
        WCharCP         m_label;
        Int32           m_unitNumber;

        UnitMatcher (Int32 base, Int32 system, double num, double denom, WCharCP label)
            : m_ignoreSystem(false), m_ignoreLabel(false), m_base(base), m_system(system), m_num(num), m_denom(denom), m_label(label), m_unitNumber(USE_ACTIVE_MASTER_UNITS) { }

        bool            Matched() const         { return USE_ACTIVE_MASTER_UNITS != m_unitNumber; }
        Int32           GetUnitNumber() const   { return m_unitNumber; }
        void            IgnoreBaseAndSystem()   { m_ignoreSystem = true; }
        void            IgnoreLabel()           { m_ignoreLabel = true; }

        bool            Visit (Int32 number, WCharCP name, UnitDefinitionCR unitDef)
            {
            if (!DoubleOps::AlmostEqual (m_num, unitDef.GetNumerator()) || !DoubleOps::AlmostEqual (m_denom, unitDef.GetDenominator()))
                return true;    // continue
            else if (!m_ignoreSystem && (unitDef.GetSystem() != (UnitSystem)m_system || unitDef.GetBase() != (UnitBase)m_base))
                return true;    // continue
            else if (!m_ignoreLabel && !unitDef.GetLabel().Equals (m_label))
                return true;    // continue;

            m_unitNumber = number;
            return false;       // stop, found a match
            }
        };
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
static Int32 findClosestUnits (Int32 base, Int32 system, double num, double denom, WCharCP label)
    {
    UnitVisitors::UnitMatcher matcher (base, system, num, denom, label);
    visitUnits (matcher);
    if (!matcher.Matched())
        {
        matcher.IgnoreBaseAndSystem();
        visitUnits (matcher);
        if (!matcher.Matched())
            {
            matcher.IgnoreLabel();
            visitUnits (matcher);
            }
        }

    return matcher.GetUnitNumber();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
void TypeAdapterUnitsSerialization::Process (ECN::IECInstanceR fmtr, bool post, IDgnECTypeAdapter const& dwgSupport) const
    {
    void (* processFunc)(ECN::IECInstanceR, AccessorPrefix) = post ? PostProcessUnits : PreprocessUnits;
    
    bool supportsSecondary = _SupportsSecondaryUnits();
    processFunc (fmtr, supportsSecondary ? AccessorPrefix_Master : AccessorPrefix_None);

    if (supportsSecondary)
        processFunc (fmtr, AccessorPrefix_Secondary);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
static void setIntOrNull (IECInstanceR opts, WCharCP accessString, Int32 actual, Int32 def)
    {
    ECValue v;
    if (actual != def)
        v.SetInteger (actual);

    opts.SetValue (accessString, v);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
static void setDoubleOrNull (IECInstanceR opts, WCharCP accessString, double actual, double def)
    {
    ECValue v;
    if (!DoubleOps::AlmostEqual (actual, def))
        v.SetDouble (actual);

    opts.SetValue (accessString, v);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
void TypeAdapterUnitsSerialization::PreprocessUnits (IECInstanceR fmtr, AccessorPrefix prefix)
    {
    const WCharCP* accessors = s_unitsAccessors[prefix];
    ECValue v;
    Int32 unitNumber = getIntOrDefault (&fmtr, accessors[Accessor_Units], USE_ACTIVE_MASTER_UNITS);
    UnitDefinition units;
    if (USE_ACRES == unitNumber)
        {
        units = UnitDefinition::GetStandardUnit (StandardUnit::EnglishFeet);
        units.Init (units.GetBase(), units.GetSystem(), units.GetNumerator() / sqrt(43560.0), units.GetDenominator(), L"ACRES");    // do not translate
        }
    else if (0 <= unitNumber)
        {
        UnitIteratorOptions opts;
        UserUnitCollection userUnits (opts);
        units = UnitDefinition::GetByNumber (unitNumber, userUnits.begin() == userUnits.end());
        }

    if (!units.IsValid())
        {
        v.SetInteger (unitNumber < 0 ? unitNumber : USE_ACTIVE_MASTER_UNITS);
        fmtr.SetValue (accessors[Accessor_Units], v);
        return;
        }

    v.Clear();
    fmtr.SetValue (accessors[Accessor_Units], v);

    setIntOrNull (fmtr, accessors[Accessor_Base], (Int32)units.GetBase(), s_DefaultBase);
    setIntOrNull (fmtr, accessors[Accessor_System], (Int32)units.GetSystem(), s_DefaultSystem);
    setDoubleOrNull (fmtr, accessors[Accessor_Numerator], units.GetNumerator(), s_DefaultNumerator);
    setDoubleOrNull (fmtr, accessors[Accessor_Denominator], units.GetDenominator(), s_DefaultDenominator);
    
    v.SetString (units.GetLabelCP());
    fmtr.SetValue (accessors[Accessor_Label], v);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
void TypeAdapterUnitsSerialization::PostProcessUnits (IECInstanceR fmtr, AccessorPrefix prefix)
    {
    const WCharCP* accessors = s_unitsAccessors[prefix];

    ECValue v;
    if (ECOBJECTS_STATUS_Success == fmtr.GetValue (v, accessors[Accessor_Units]) && !v.IsNull())
        {
        if (v.GetInteger() == USE_ACTIVE_MASTER_UNITS || v.GetInteger() == USE_ACTIVE_SECONDARY_UNITS)
            return;
        }

    Int32 base = getIntOrDefault (&fmtr, accessors[Accessor_Base], s_DefaultBase);
    Int32 system = getIntOrDefault (&fmtr, accessors[Accessor_System], s_DefaultSystem);
    double numerator = getDoubleOrDefault (&fmtr, accessors[Accessor_Numerator], (double)s_DefaultNumerator);
    double denominator = getDoubleOrDefault (&fmtr, accessors[Accessor_Denominator], (double)s_DefaultDenominator);
    WString label = getStringOrDefault (&fmtr, accessors[Accessor_Label], L"");

    Int32 unitNumber = label.Equals (L"ACRES") ? USE_ACRES : findClosestUnits (base, system, numerator, denominator, label.c_str());

    v.SetInteger (unitNumber);
    fmtr.SetValue (accessors[Accessor_Units], v);

    v.Clear();
    fmtr.SetValue (accessors[Accessor_Base], v);
    fmtr.SetValue (accessors[Accessor_System], v);
    fmtr.SetValue (accessors[Accessor_Numerator], v);
    fmtr.SetValue (accessors[Accessor_Denominator], v);
    fmtr.SetValue (accessors[Accessor_Label], v);
    }

/////////////////////////////////////////////////////
//          Working units conversions
// Properties can declare an extended type like Distance, Area, etc which indicate they use the working units
// for the instance's containing model. For intrinsic instances (e.g. element properties), we get and set these values in UORs.
// However extrinsic instances persisted to the file will have stored these values in meters. We must continue to support this.
//
// So, by default we convert stored values from meters to UORs before formatting, and from UORs to meters after parsing.
// This can be overridden in a schema by defining "StoresUnitsAsUors" custom attribute.
// 
// Also, if the context does not provide a DgnModel, we default to meters for formatting and parsing.
////////////////////////////////////////////////////

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   11/12
+---------------+---------------+---------------+---------------+---------------+------*/
template <int DIMENSION> struct UorMeterConverter
    {
protected:
    double          m_dimensionedUorPerMeter;
public:
    UorMeterConverter (IDgnECTypeAdapterContextCR context) : m_dimensionedUorPerMeter(1.0)
        {
        BeAssert (0 < DIMENSION && DIMENSION < 4);

        ECSchemaCP schema = NULL;
        ECPropertyCP ecprop = context.GetProperty();
        if (NULL != ecprop)
            schema = &ecprop->GetClass().GetSchema();
        else
            {
            IECInstanceCP instance = context.GetECInstance();
            if (NULL != instance)
                schema = &instance->GetEnabler().GetClass().GetSchema();
            }

        if (NULL == schema || schema->GetCustomAttribute (L"StoresUnitsAsUors").IsNull())
            {
            DgnModelP modelRef = context.GetDgnModel();
            if (NULL != modelRef)
                {
                double uorPerMeter = 1000.;
                m_dimensionedUorPerMeter = pow (uorPerMeter, DIMENSION);
                }
            }
        }

    // Incoming value is uors for intrinsic instances (schema defines StoresUnitsAsUors attribute)
    // Is in meters otherwise - convert to UORs to pass to some formatter object
    void ConvertForDisplay (double& storedValue) const      { storedValue *= m_dimensionedUorPerMeter; }

    // Incoming value is in uors (from some parser object)
    // If schema does not define StoresUnitsAsUors, store as meters
    void ConvertForStorage (double& uors) const             { uors /= m_dimensionedUorPerMeter; }
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   11/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct CoordinateConverter : UorMeterConverter<1>
    {
public:
    CoordinateConverter (IDgnECTypeAdapterContextCR context) : UorMeterConverter<1> (context) { }

    void ConvertForDisplay (DPoint3d& storedPt) const           { storedPt.Scale (m_dimensionedUorPerMeter); }
    void ConvertForStorage (DPoint3d& uorPt) const              { uorPt.Scale (1.0 / m_dimensionedUorPerMeter); }
    };

///*---------------------------------------------------------------------------------**//**
//* @bsistruct                                                    Paul.Connelly   06/14
//+---------------+---------------+---------------+---------------+---------------+------*/
//static bool getWorkingUnitsSpec (UnitSpecR spec, IDgnECTypeAdapterContextCR context, int dimension)
//    {
//    auto modelRef = context.GetDgnModel();
//    ModelInfoCR modelInfo = modelRef->GetModelInfo();
//
//    WCharCP baseUnitName = nullptr;
//    switch (dimension)
//        {
//        case 1:     baseUnitName = BASEUNIT_METER; break;
//        case 2:     baseUnitName = BASEUNIT_METER2; break;
//        case 3:     baseUnitName = BASEUNIT_METER3; break;
//        default:    return false;
//        }
//
//    double factor = ModelInfo::GetUorPerMeter (modelInfo); // WIP_TOPAZ_MERGE: this method doesn't exist on graphite
//    UnitConverter converter;    // identity
//    if (!DoubleOps::AlmostEqual (factor, 1.0))
//        {
//        if (0.0 == factor)
//            return false;
//        else
//            converter = UnitConverter (factor);
//        }
//
//    spec = UnitSpec (baseUnitName, converter);
//    return true;
//    }

/////////////////////////////////////////////////////
//          String case conversion
////////////////////////////////////////////////////

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
static void makeUpper (WStringR str)
    {
    for (size_t i = 0; i < str.length(); i++)
        if (iswlower (str[i]))
            str[i] = towupper (str[i]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
static void makeLower (WStringR str)
    {
    for (size_t i = 0; i < str.length(); i++)
        if (iswupper (str[i]))
            str[i] = towlower (str[i]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
static void capFirst (WStringR str)
    {
    size_t i = 0;
    for ( ; i < str.length(); i++)
        {
        WChar ch = str[i];
        if (!iswspace (ch) && !iswpunct (ch))
            {
            str[i] = towupper (ch);
            break;
            }
        }

    for (i++; i < str.length(); i++)
        if (iswupper (str[i]))
            str[i] = towlower (str[i]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
static size_t makeWordTitleCase (WStringR str, size_t i)
    {
    // skip white/punct
    for ( ; i < str.length(); i++)
        {
        if (!iswpunct (str[i]) && !iswspace (str[i]))
            break;
        }

    // check if all-caps; if so don't modify
    bool foundLower = false;
    size_t start = i;
    for ( ; i < str.length(); i++)
        {
        if (!foundLower && iswlower (str[i]))
            foundLower = true;
        else if (iswpunct (str[i]) || iswspace (str[i]))
            break;
        }
        
    size_t end = i;
    if (foundLower)
        {
        // make title case
        if (start < end && iswlower (str[start]))
            str[start] = towupper (str[start]);

        for (i = start+1; i < end; i++)
            if (iswupper (str[i]))
                str[i] = towlower (str[i]);
        }

    return end;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
static void makeTitleCase (WStringR str)
    {
    // Title Case:
    //  "ABC DEF GHI" => "Abc Def Ghi"  <-- if all chars in string uppercase, perform normal title case
    //  "ABC def gHi" => "ABC Def Ghi"  <-- if all chars in a single word uppercase, leave that word alone

    bool foundLower = false;
    for (size_t cur = 0; cur < str.length(); cur++)
        {
        if (iswlower (str[cur]))
            {
            foundLower = true;
            break;
            }
        }

    if (!foundLower)
        makeLower (str);

    size_t start = 0;
    while (start < str.length())
        start = makeWordTitleCase (str, start);    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void StringFormatTypeAdapter::ConvertCase (WStringR str, IECInstanceCP fmtr)
    {
    Int32 caseSpec = getIntOrDefault (fmtr, L"Case", CaseSpec_None);
    switch (caseSpec)
        {
    case CaseSpec_None:
        break;
    case CaseSpec_Upper:
        makeUpper (str);
        break;
    case CaseSpec_Lower:
        makeLower (str);
        break;
    case CaseSpec_Title:
        makeTitleCase (str);
        break;
    case CaseSpec_First:
        capFirst (str);
        break;
    default:
        BeAssert (false);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool StringFormatTypeAdapter::_ConvertToString (WStringR str, ECValueCR v, IDgnECTypeAdapterContextCR context, IECInstanceCP fmtr) const
    {
    // Note derived classes can choose how to handle case where v.IsNull()
    str.clear();
    if (!GetUnformattedStringValue (str, v, context))
        return false;

    ConvertCase (str, fmtr);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void StringFormatTypeAdapter::InitOptions (IECInstanceR opts)
    {
    opts.SetValue (L"Case", ECValue ((Int32)CaseSpec_None));
    }

///////////////////////////////////////////
//          BooleanFormat
///////////////////////////////////////////
        
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/12
+---------------+---------------+---------------+---------------+---------------+------*/
BooleanFormatTypeAdapter::BooleanFormatTypeAdapter()
    {
    m_trueFalseStrings[BoolWord_TrueFalse-1][0] = RSC_STR (False);
    m_trueFalseStrings[BoolWord_TrueFalse-1][1] = RSC_STR (True);
    m_trueFalseStrings[BoolWord_YesNo-1][0] = RSC_STR (No);
    m_trueFalseStrings[BoolWord_YesNo-1][1] = RSC_STR (Yes);
    m_trueFalseStrings[BoolWord_OnOff-1][0] = RSC_STR (Off);
    m_trueFalseStrings[BoolWord_OnOff-1][1] = RSC_STR (On);
    m_trueFalseStrings[BoolWord_EnabledDisabled-1][0] = RSC_STR (Disabled);
    m_trueFalseStrings[BoolWord_EnabledDisabled-1][1] = RSC_STR (Enabled);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/12
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP BooleanFormatTypeAdapter::GetResourceString (BoolWord type, bool value) const
    {
    return BoolWord_Default != type ? m_trueFalseStrings[type-1][value ? 1 : 0].c_str() : NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool BooleanFormatTypeAdapter::_Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) const
    {
    return v.IsBoolean() && !v.IsNull();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool BooleanFormatTypeAdapter::_ConvertToString (WStringR str, ECValueCR v, IDgnECTypeAdapterContextCR context, IECInstanceCP fmtr) const
    {
    if (v.IsNull())
        {
        str = GetParensNullString();
        return true;
        }
    else if (!v.IsBoolean())
        return false;

    BoolWord boolWord = (BoolWord)getIntOrDefault (fmtr, L"BoolWord", BoolWord_Default);
    switch (boolWord)
        {
    case BoolWord_Default:
        str = GetBooleanString (v.GetBoolean(), context);
        break;
    case BoolWord_TrueFalse:
    case BoolWord_YesNo:
    case BoolWord_OnOff:
    case BoolWord_EnabledDisabled:
        str = GetResourceString (boolWord, v.GetBoolean());
        break;
    default:
        BeAssert (false);
        return false;
        }
    
    StringFormatTypeAdapter::ConvertCase (str, fmtr);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool BooleanFormatTypeAdapter::_ConvertFromString (ECValueR v, WCharCP str, IDgnECTypeAdapterContextCR context) const
    {
    if (GetBooleanString (false, context).Equals (str))
        v.SetBoolean (false);
    else if (GetBooleanString (true, context).Equals (str))
        v.SetBoolean (true);
    else
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool BooleanFormatTypeAdapter::_ConvertToDisplayType (ECValueR v, IDgnECTypeAdapterContextCR context, IECInstanceCP opts) const
    {
    WString str;
    if (!ConvertToString (str, v, context, opts))
        return false;

    v.SetString (str.c_str());
    return true;
    }


///////////////////////////////////
//      Boolean
//////////////////////////////////

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
WString BooleanTypeAdapter::GetBooleanString (bool b, IDgnECTypeAdapterContextCR) const
    {
    return GetResourceString (BoolWord_TrueFalse, b);
    }

////////////////////////////////////////////////////
//      Double
///////////////////////////////////////////////////

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool DoubleTypeAdapter::_Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) const
    {
    return v.IsDouble() || v.IsNull();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool DoubleTypeAdapter::_ConvertFromString (ECValueR v, WCharCP str, IDgnECTypeAdapterContextCR context) const
    {
    double d;
    if (1 == BE_STRING_UTILITIES_SWSCANF (str, s_fmtDouble, &d))
        {
        v.SetDouble (d);
        return true;
        }
    
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool DoubleTypeAdapter::_ConvertToString (WStringR str, ECValueCR v, IDgnECTypeAdapterContextCR context, IECInstanceCP opts) const
    {
    if (v.IsNull())
        {
        str = GetParensNullString();
        return true;
        }
    else if (!v.IsDouble())
        return false;

    DoubleFormatterPtr fmtr = DoubleFormatter::Create();
    InitFormatter (*fmtr, opts);
    
    double dval = v.GetDouble();
    double conversionFactor = getDoubleOrDefault (opts, L"CF", 1.0);
    dval *= conversionFactor;

    str = fmtr->ToString (dval);
    ApplyPrefixSuffix (str, opts);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool DoubleTypeAdapter::_ConvertToDisplayType (ECValueR v, IDgnECTypeAdapterContextCR, IECInstanceCP opts) const
    {
    if (v.IsNull() || !v.IsDouble())
        return false;

    double d = v.GetDouble();
    double conversionFactor = getDoubleOrDefault (opts, L"CF", 1.0);
    d *= conversionFactor;
    v.SetDouble (d);

    return ConvertToDisplayType (v, opts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool DoubleTypeAdapter::ConvertToDisplayType (ECValueR v, IECInstanceCP opts)
    {
    if (v.IsNull() || !v.IsDouble())
        return false;

    double d = v.GetDouble();

    Int32 nDecimalPlaces = 4;
    Int32 accuracy = getIntOrDefault (opts, L"Accuracy", USE_ACTIVE);
    if (0 <= accuracy)
        nDecimalPlaces = (accuracy & 0x0000000F);

    double fac = pow (10.0, nDecimalPlaces);
    double rounded = std_round (d * fac) / fac;
    
    v.SetDouble (rounded);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* Fill out instance with the default values common to many formatting classes
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DoubleTypeAdapter::InitOptions (IECInstanceR opts)
    {
    ECValue v;
    v.SetInteger (USE_ACTIVE);      opts.SetValue (L"Accuracy",        v);
    v.SetString (L"");      opts.SetValue (L"PX",              v);
                            opts.SetValue (L"SX",              v);
    v.SetInteger ('.');     opts.SetValue (L"DecimalSeparator",v);
    v.SetInteger (1);       opts.SetValue (L"LeadingZero",     v);
                            opts.SetValue (L"TrailingZeros",   v);

    // Area, Coordinate, Double, and Volume support TS
    if (ECOBJECTS_STATUS_Success == opts.GetValue (v, L"TS"))
        { v.SetInteger (0); opts.SetValue (L"TS", v); }                // 'none'

    // Area, Volume, and Double support CF
    if (ECOBJECTS_STATUS_Success == opts.GetValue (v, L"CF"))
        { v.SetDouble (1.0);    opts.SetValue (L"CF", v); }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
static WChar getSeparatorOrDefault (IECInstanceCP opts, WCharCP accessString, WChar def)
    {
    Int32 i = getIntOrDefault (opts, accessString, 0);
    switch (i)
        {
    // known supported separators
    case '.':
    case ',':
    case ' ':
    case '/':
    case ';':
        def = (WChar)i;
        break;
        }

    return def;
    }

/*---------------------------------------------------------------------------------**//**
* Init DoubleFormatterBase from IECInstance
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DoubleTypeAdapter::InitFormatter (DoubleFormatterBase& fmtr, IECInstanceCP opts)
    {
    WChar thousSep = getSeparatorOrDefault (opts, L"TS", 0);
    if (0 != thousSep)
        {
        fmtr.SetInsertThousandsSeparator (true);
        fmtr.SetThousandsSeparator (thousSep);
        }

    WChar decSep = getSeparatorOrDefault (opts, L"DecimalSeparator", '.');
    fmtr.SetDecimalSeparator (decSep);

    fmtr.SetLeadingZero (getBoolOrDefault (opts, L"LeadingZero", true));
    fmtr.SetTrailingZeros (getBoolOrDefault (opts, L"TrailingZeros", false));

    Int32 accuracy = getIntOrDefault (opts, L"Accuracy", USE_ACTIVE);
    if (0 <= accuracy)
        {
        //  accu             prec
        //  0..8            100..108        decimal
        //  10..18          200..208        fractional
        //  20..28          300..308        scientific
        Int32 precInt = 100 * (accuracy/10) + 100;
        precInt += (accuracy - (accuracy/10));
        fmtr.SetPrecision ((PrecisionFormat)precInt);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DoubleTypeAdapter::ApplyPrefixSuffix (WStringR str, IECInstanceCP opts)
    {
    WString prefix = getStringOrDefault (opts, L"PX", L"");
    if (!prefix.empty())
        str.insert (0, prefix);

    WString suffix = getStringOrDefault (opts, L"SX", L"");
    if (!suffix.empty())
        str.append (suffix);
    }

///////////////////////////////////////
//      Integer
//////////////////////////////////////

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool IntegerTypeAdapter::_Validate (ECValueCR v, IDgnECTypeAdapterContextCR context) const
    {
    return v.IsNull() || v.IsInteger();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool IntegerTypeAdapter::_ConvertFromString (ECValueR v, WCharCP str, IDgnECTypeAdapterContextCR context) const
    {
    Int32 i;
    if (1 == BE_STRING_UTILITIES_SWSCANF (str, s_fmtInteger, &i))
        {
        v.SetInteger (i);
        return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool IntegerTypeAdapter::_ConvertToString (WStringR str, ECValueCR v, IDgnECTypeAdapterContextCR context, IECInstanceCP opts) const
    {
    str.clear();
    if (v.IsNull())
        {
        str = GetParensNullString();
        return true;
        }
    else if (!v.IsInteger())
        return false;

    WString fmt;
    ExtractFormatString (fmt, opts);
    str.Sprintf (fmt.c_str(), v.GetInteger());
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* Returns a native format string.
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void IntegerTypeAdapter::ExtractFormatString (WStringR fmt, IECInstanceCP opts) const
    {
    if (NULL == opts)
        fmt = s_fmtInteger;
    else
        {
        Int32 minWidth = getIntOrDefault (opts, L"MinimumWidth", 1);
        if (1 == minWidth)
            fmt = s_fmtInteger;     // other properties can have no effect
        else
            {
            bool rightAligned = getBoolOrDefault (opts, L"RightAligned", true);
            bool leadingZeros = getBoolOrDefault (opts, L"LeadingZeros", false);

            WCharCP widthFmt = L"%%%dd";
            if (!rightAligned)
                widthFmt = L"%%-%dd";
            else if (leadingZeros)
                widthFmt = L"%%0%dd";

            fmt.Sprintf (widthFmt, minWidth);
            }
        }
    }

/////////////////////////////////////////////////
//          String
////////////////////////////////////////////////

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool StringTypeAdapter::_ConvertFromString (ECValueR v, WCharCP str, IDgnECTypeAdapterContextCR context) const
    {
    if (NULL != str)
        v.SetString (str);
    else
        v.SetToNull();

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool StringTypeAdapter::_Validate (ECValueCR v, IDgnECTypeAdapterContextCR context) const
    {
    return v.IsNull() || v.IsString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool StringTypeAdapter::GetUnformattedStringValue (WStringR str, ECValueCR v, IDgnECTypeAdapterContextCR context) const
    {
    if (v.IsNull())
        str = GetParensNullString();
    else if (!v.IsString())
        return false;
    else
        str = v.GetString();

    return true;
    }

//////////////////////////////////////////////////////
//          DateTime
/////////////////////////////////////////////////////

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct DateTimeSettings : DgnHost::HostObjectBase
    {
    struct StandardFormat
        {
        WString         name;
        WString         format;
        StandardFormat (WCharCP n, WCharCP f) : name(n), format(f) { }
        };
    typedef bvector<StandardFormat>             FormatList;
private:
    DateTimeSettings() { }

    static const Int64 TICKADJUSTMENT = 504911232000000000LL;     // ticks between 01/01/01 and 01/01/1601
    static const UInt64 SAMPLETIME    = 1126120082463LL;

    // For use in converting persisted 'instruction format' strings
    struct PartPair
        {
        DateTimeFormatPart          primaryPart;
        DateTimeFormatPart          alternatePart;      // when preceded by a '#'
        
        PartPair() : primaryPart(DATETIME_PART_END), alternatePart(DATETIME_PART_END) { }
        PartPair (DateTimeFormatPart p) : primaryPart(p), alternatePart(DATETIME_PART_END) { }
        PartPair (DateTimeFormatPart p, DateTimeFormatPart a) : primaryPart(p), alternatePart(a) { }
        };
    
    // Maps a % character specifier from a .NET date format string to a DateTimeFormatPart
    typedef bmap<WChar, PartPair>               InstructionFormatConversionMap;
    typedef bmap<WString, DateTimeFormatPart>   DotNetConversionMap;
    
    InstructionFormatConversionMap  m_instructionMap;
    DotNetConversionMap             m_dotNetMap;
    FormatList                      m_standardDateTimeFormats;      // from config file

    void                            PopulateConversionMaps();
    void                            PopulateStandardFormats();
    static bool                     ReadFormatsFromXml (bvector<WString>& formats, bvector<WString>& descriptions);
    static void                     GetDefaultFormats (bvector<WString>& formats);
public:
    DateTimeFormatPart              ConvertInstructionSpecifier (WChar spec, bool useAlternate) const;      // Returns DATETIME_PART_END if not found
    DateTimeFormatPart              ConvertDotNetSpecifier (WStringCR spec) const;                          // Returns DATETIME_PART_END if not found
    FormatList const&               GetStandardFormats() const   { return m_standardDateTimeFormats; }
    void                            AddStandardFormat (WCharCP fmt);

    static DateTimeSettings&        GetSettings();
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
DateTimeSettings& DateTimeSettings::GetSettings()
    {
    static DgnHost::Key key;
    DateTimeSettings* settings = static_cast<DateTimeSettings*> (T_HOST.GetHostObject (key));
    if (NULL == settings)
        {
        settings = new DateTimeSettings();
        T_HOST.SetHostObject (key, settings);
        settings->PopulateConversionMaps();
        settings->PopulateStandardFormats();
        }

    return *settings;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
DateTimeFormatPart DateTimeSettings::ConvertInstructionSpecifier (WChar spec, bool useAlternate) const
    {
    InstructionFormatConversionMap::const_iterator iter = m_instructionMap.find (spec);
    if (iter != m_instructionMap.end())
        return useAlternate ? iter->second.alternatePart : iter->second.primaryPart;

    return DATETIME_PART_END;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
DateTimeFormatPart DateTimeSettings::ConvertDotNetSpecifier (WStringCR spec) const
    {
    DotNetConversionMap::const_iterator iter = m_dotNetMap.find (spec);
    return iter != m_dotNetMap.end() ? iter->second : DATETIME_PART_END;
    }

/*---------------------------------------------------------------------------------**//*
* @bsimethod                                                    Paul.Connelly   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool DateTimeSettings::ReadFormatsFromXml (bvector<WString>& formats, bvector<WString>& descriptions)
    {
#if defined (WIP_CFGVAR) // MS_DATETIMEFORMATS
    WString     xmlPath;
    BeXmlDomPtr xmlDom;
    BeXmlStatus xmlStatus;
    if (SUCCESS != ConfigurationManager::GetVariable (xmlPath, L"MS_DATETIMEFORMATS") || (xmlDom = BeXmlDom::CreateAndReadFromFile (xmlStatus, xmlPath.c_str())).IsNull())
        return false;

    BeXmlDom::IterableNodeSet nodes;
    xmlDom->SelectNodes (nodes, "/DateTimeFormats/SupportedFormats/Format", NULL);
    FOR_EACH (BeXmlNodeP node, nodes)
        {
        WString fmt, desc;
        if (SUCCESS == node->GetAttributeStringValue (fmt, "FormatString") && !fmt.empty())
            {
            node->GetAttributeStringValue (desc, "Description");
            formats.push_back (fmt);
            descriptions.push_back (desc);
            }
        }

    return descriptions.size() > 0;
#endif
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DateTimeSettings::GetDefaultFormats (bvector<WString>& formats)
    {
    static const WString s_defaults[] = { L"%c", L"%#c", L"%x", L"%#x", L"dddd, MMMM dd, yyyy", L"HH:mm:ss", L"HH:mm", L"h:mm:ss tt", L"h:mm tt",
        L"M/d/yyyy h:mm:ss tt", L"M/d/yyyy h:mm tt", L"MMM-yy", L"MMMM yy", L"yyyy-M-d", L"yyyy/MM/dd", L"dd/MM/yyyy", L"dd.MM.yyyy", L"d MMMM yyyy",
        L"MMM. d, yy", L"d-MMM-yy", L"yyyy-MM-dd", L"M/d/yy", L"MMMM d, yyyy", L"M/d/yyyy", L"M.d.yyyy", L"%X"
        };

    formats.insert (formats.begin(), s_defaults, s_defaults + _countof (s_defaults));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DateTimeSettings::PopulateStandardFormats()
    {
    bvector<WString> formats, descriptions;
    if (!ReadFormatsFromXml (formats, descriptions))
        GetDefaultFormats (formats);

    // Sample time 09/07/2005 15:08:02.463 chosen so 24-hour clock, leading zeros, etc are obvious
    
    for (size_t i = 0; i < formats.size(); i++)
        {
        // use the format string to format the sample time
        WString formattedSampleTime;
        DateTime sampleDT;
        DateTime::FromUnixMilliseconds(sampleDT, SAMPLETIME);
        DateTimeTypeAdapter::FormatDateTime(formattedSampleTime, formats[i], sampleDT);

        WString name = formattedSampleTime;
        if (0 != descriptions.size() && 0 < descriptions[i].length())
            {
            // append the description in parentheses
            name.append (L" (");
            name.append (descriptions[i]);
            name.append (L")");
            }

        m_standardDateTimeFormats.push_back (StandardFormat (name.c_str(), formats[i].c_str()));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DateTimeSettings::AddStandardFormat (WCharCP fmt)
    {
    WString formatted;
    DateTime sampleDT;
    DateTime::FromUnixMilliseconds(sampleDT, SAMPLETIME);
    DateTimeTypeAdapter::FormatDateTime(formatted, fmt, sampleDT);
    m_standardDateTimeFormats.push_back (StandardFormat (formatted.c_str(), fmt));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DateTimeSettings::PopulateConversionMaps()
    {
    // Instruction format
    m_instructionMap['a'] = PartPair (DATETIME_PART_DoW);
    m_instructionMap['b'] = PartPair (DATETIME_PART_Mon);
    m_instructionMap['c'] = PartPair (DATETIME_PART_General, DATETIME_PART_Full);
    m_instructionMap['d'] = PartPair (DATETIME_PART_DD, DATETIME_PART_D);
    m_instructionMap['H'] = PartPair (DATETIME_PART_HH, DATETIME_PART_H);
    m_instructionMap['m'] = PartPair (DATETIME_PART_MM, DATETIME_PART_M);
    m_instructionMap['M'] = PartPair (DATETIME_PART_mm, DATETIME_PART_m);
    m_instructionMap['p'] = PartPair (DATETIME_PART_AMPM);
    // w => ignore
    m_instructionMap['x'] = PartPair (DATETIME_PART_M_D_YYYY, DATETIME_PART_Day_Month_D_YYYY);
    m_instructionMap['X'] = PartPair (DATETIME_PART_h_mm_ss_AMPM, DATETIME_PART_h_mm_AMPM);
    m_instructionMap['y'] = PartPair (DATETIME_PART_YY, DATETIME_PART_Y);
    m_instructionMap['z'] = PartPair (DATETIME_PART_U_UU);
    m_instructionMap['j'] = PartPair (DATETIME_PART_ddd, DATETIME_PART_d);

    // .NET
    // Note managed code treated everything as a custom format.
    // ex: (date.ToString("d") => M/d/yyyy, date.ToString("%d") => dayOfMonth)
    m_dotNetMap[L"d"] = DATETIME_PART_D;
    m_dotNetMap[L"dd"] = DATETIME_PART_DD;
    m_dotNetMap[L"ddd"] = DATETIME_PART_DoW;
    m_dotNetMap[L"dddd"] = DATETIME_PART_DayOfWeek;
    // ffffff special cases
    // g, gg for era - not supported (times B.C. not supported....)
    m_dotNetMap[L"h"] = DATETIME_PART_h;
    m_dotNetMap[L"hh"] = DATETIME_PART_hh;
    m_dotNetMap[L"H"] = DATETIME_PART_H;
    m_dotNetMap[L"HH"] = DATETIME_PART_HH;
    m_dotNetMap[L"m"] = DATETIME_PART_m;
    m_dotNetMap[L"mm"] = DATETIME_PART_mm;
    m_dotNetMap[L"M"] = DATETIME_PART_M;
    m_dotNetMap[L"MM"] = DATETIME_PART_MM;
    m_dotNetMap[L"MMM"] = DATETIME_PART_Mon;
    m_dotNetMap[L"MMMM"] = DATETIME_PART_Month;
    m_dotNetMap[L"s"] = DATETIME_PART_s;
    m_dotNetMap[L"ss"] = DATETIME_PART_ss;
    m_dotNetMap[L"t"] = DATETIME_PART_AP;
    m_dotNetMap[L"tt"] = DATETIME_PART_AMPM;
    m_dotNetMap[L"y"] = DATETIME_PART_Y;
    m_dotNetMap[L"yy"] = DATETIME_PART_YY;
    m_dotNetMap[L"yyy"] = DATETIME_PART_YYY;
    m_dotNetMap[L"yyyy"] = DATETIME_PART_YYYY;
    m_dotNetMap[L"yyyyy"] = DATETIME_PART_YYYYY;
    m_dotNetMap[L"z"] = DATETIME_PART_U;
    m_dotNetMap[L"zz"] = DATETIME_PART_UU;
    m_dotNetMap[L"zzz"] = DATETIME_PART_UU_UU;
    m_dotNetMap[L":"] = DATETIME_PART_TimeSeparator;
    m_dotNetMap[L"/"] = DATETIME_PART_DateSeparator;
    m_dotNetMap[L"."] = DATETIME_PART_DecimalSeparator;
    m_dotNetMap[L","] = DATETIME_PART_Comma;
    m_dotNetMap[L" "] = DATETIME_PART_Space;
    // %, \, ", ' special handling
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool DateTimeTypeAdapter::_Validate (ECValueCR v, IDgnECTypeAdapterContextCR context) const
    {
    return v.IsNull() || v.IsDateTime();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool DateTimeTypeAdapter::_ConvertToString (WStringR str, ECValueCR v, IDgnECTypeAdapterContextCR context, IECInstanceCP opts) const
    {
    str.clear();
    if (v.IsNull())
        str = GetParensNullString();
    else if (!v.IsDateTime())
        return false;
    else
        {
        WString fmt = getStringOrDefault (opts, L"DTFmtStr", L"");
        FormatDateTime(str, fmt, v.GetDateTime());
        }

    StringFormatTypeAdapter::ConvertCase (str, opts);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DateTimeTypeAdapter::FormatDateTime (WStringR str, WStringCR fmt, DateTimeCR dt)
    {
    if (fmt.empty())
        {
        DateTimeFormatterPtr fmtr = DateTimeFormatter::Create();
        fmtr->SetConvertToLocalTime (false);
        str = fmtr->ToString (dt);

        }
    else if (-1 != fmt.find ('%'))
        FormatWithInstructions(str, fmt, dt);
    else
        FormatWithPicture(str, fmt, dt);
    }

/*---------------------------------------------------------------------------------**//**
* DateTime format strings are persisted to the dgn in a format intended for .NET's
* DateTime.ToString() method.
* We have to parse them and produce something usable in native code. This is complicated
* by the additional custom formatting supported by our managed DateTimeFormatProvider.
* To simplify things somewhat, we create a new DateTimeFormatter for each format
* specifier, as otherwise we would need to make sure we write and reset the formatter
* as options change or non-datetime data is parsed.
* @bsimethod                                                    Paul.Connelly   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DateTimeTypeAdapter::FormatWithInstructions(WStringR str, WStringCR fmt, DateTimeCR dt)
    {
    DateTimeFormatterPtr fmtr = DateTimeFormatter::Create();
    fmtr->SetConvertToLocalTime (false);
    size_t len = fmt.length();
    DateTimeSettings const& dtSettings = DateTimeSettings::GetSettings();
    for (size_t index = 0; index < len; index++)
        {
        WChar c = fmt[index];
        if ('%' != c)
            {
            str.append (1, c);
            continue;
            }
        else if (++index >= len)
            break;

        c = fmt[index];
        bool  havePound = ('#' == c);
        if (havePound)
            {
            if (++index >= len)
                break;
            c = fmt[index];
            }

        DateTimeFormatPart part = dtSettings.ConvertInstructionSpecifier (c, havePound);
        if (DATETIME_PART_END != part)
            {
            fmtr->Reset();
            fmtr->SetConvertToLocalTime (false);
            fmtr->AppendFormatPart (part);
            str.append(fmtr->ToString(dt));
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
WString extractLiteralString (WStringCR fmt, WChar quoteChar, size_t quoteStart)
    {
    size_t quoteEnd = fmt.find (quoteChar, quoteStart+1);
    if (-1 == quoteEnd)
        quoteEnd = fmt.length() + 1;

    return fmt.substr (quoteStart+1, quoteEnd-quoteStart-1);
    }

/*---------------------------------------------------------------------------------**//**
* This basically tries to match .NET's DateTime formatting logic.
* @bsimethod                                                    Paul.Connelly   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DateTimeTypeAdapter::FormatWithPicture(WStringR str, WStringCR fmt, DateTimeCR dt)
    {
    DateTimeFormatterPtr fmtr = DateTimeFormatter::Create();
    fmtr->SetConvertToLocalTime (false);
    size_t len = fmt.length();
    DateTimeSettings const& dtSettings = DateTimeSettings::GetSettings();

    for (size_t index = 0; index < len; index++)
        {
        WChar c = fmt[index];
        if ('\\' == c)                   // escape character - treat next as literal
            {
            if (++index < len)
                str.append (1, fmt[index]);
            }
        else if ('"' == c || '\'' == c)
            {
            WString literalString = extractLiteralString (fmt, c, index);
            index += literalString.length()+1;
            str.append (literalString);
            }
        else if ('%' == c)
            continue;
        else
            {
            size_t matchEnd = index+1;
            while (matchEnd < len && fmt[matchEnd] == c)
                ++matchEnd;

            WString sequence (matchEnd-index, c);
            index = matchEnd-1;
            fmtr->Reset();
            DateTimeFormatPart part = dtSettings.ConvertDotNetSpecifier (sequence);
            if (DATETIME_PART_END != part)
                {
                fmtr->AppendFormatPart (part);
                str.append(fmtr->ToString(dt));
                }
            else
                {
                // special cases
                switch (c)
                    {
                case 'f':
                case 'F':
                    fmtr->SetFractionalSecondPrecision ((UInt8)sequence.length());
                    fmtr->SetTrailingZeros ('f' == c);
                    part = DATETIME_PART_FractionalSeconds;
                    break;
                case 'z':
                case 'g':
                    // unsupported
                    sequence.clear();
                    }

                if (DATETIME_PART_END != part)
                    {
                    fmtr->AppendFormatPart (part);
                    str.append(fmtr->ToString(dt));
                    }
                else
                    str.append (sequence);
                }
            }
        }
    }

///////////////////////////////////////
//  DateTimeFormat
///////////////////////////////////////

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool DateTimeFormatTypeAdapter::_Validate (ECValueCR v, IDgnECTypeAdapterContextCR) const
    {
    return v.IsString() && !v.IsNull();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool DateTimeFormatTypeAdapter::_ConvertFromString (ECValueR v, WCharCP str, IDgnECTypeAdapterContextCR) const
    {
    DateTimeSettings::FormatList const& fmts = DateTimeSettings::GetSettings().GetStandardFormats();
    for (size_t i = 0; i < fmts.size(); i++)
        {
        if (fmts[i].name.Equals (str))
            {
            v.SetString (fmts[i].format.c_str());
            return true;
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool DateTimeFormatTypeAdapter::_ConvertToString (WStringR str, ECValueCR v, IDgnECTypeAdapterContextCR, IECInstanceCP opts) const
    {
    if (v.IsString() && !v.IsNull())
        {
        WCharCP fmt = v.GetString();
        DateTimeSettings::FormatList const& fmts = DateTimeSettings::GetSettings().GetStandardFormats();
        for (size_t i = 0; i < fmts.size(); i++)
            {
            if (fmts[i].format.Equals (fmt))
                {
                str = fmts[i].name;
                return true;
                }
            }

        // We may have a format string that was persisted in one session, and is not present in the current session's config
        // Add it to our list of standard formats
        DateTimeSettings::GetSettings().AddStandardFormat (fmt);
        return true;
        }
    else
        return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
Int32 DateTimeFormatTypeAdapter::LookupFormatIndex (WCharCP formatString)
    {
    DateTimeSettings::FormatList const& fmts = DateTimeSettings::GetSettings().GetStandardFormats();
    for (size_t i = 0; i < fmts.size(); i++)
        {
        DateTimeSettings::StandardFormat const& fmt = fmts[i];
        if (fmt.format.Equals (formatString))
            return (Int32)i;
        }

    return -1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool DateTimeFormatTypeAdapter::_GetStandardValues (StandardValuesCollection& values, IDgnECTypeAdapterContextCR) const
    {
    DateTimeSettings::FormatList const& fmts = DateTimeSettings::GetSettings().GetStandardFormats();
    FOR_EACH (DateTimeSettings::StandardFormat const& fmt, fmts)
        values.push_back (fmt.name);

    return true;
    }

///////////////////////////////////////
//  StructTypeAdapter
//////////////////////////////////////

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool StructTypeAdapter::_Validate (ECValueCR v, IDgnECTypeAdapterContextCR context) const
    {
    if (v.IsStruct())
        {
        IECInstancePtr structInstance = v.GetStruct();
        return structInstance.IsNull() || structInstance->GetEnabler().GetClass().Is (&context.GetProperty()->GetAsStructProperty()->GetType());
        }
    else
        return v.IsNull();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool StructTypeAdapter::_ConvertToString (WStringR str, ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP formatter) const
    {
    // we could return the instance ID, but would that really be useful?
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool StructTypeAdapter::_ConvertFromString (ECValueR v, WCharCP str, IDgnECTypeAdapterContextCR context) const
    {
    return false;
    }

//////////////////////////////////////////////
//  StandardValuesTypeAdapter
/////////////////////////////////////////////

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
StandardValuesTypeAdapter::ValueMapIterator::ValueMapIterator (ECPropertyCR prop)
: m_count(0), m_currentIndex(0), m_value(0)
    {
    m_instance = prop.GetCustomAttribute (L"StandardValues");
    if (m_instance.IsValid())
        {
        ECValue v;
        if (ECOBJECTS_STATUS_Success == m_instance->GetValue (v, L"ValueMap") && v.IsArray() && !v.IsNull())
            {
            m_count = v.GetArrayInfo().GetCount();
            if (!IsEnd())
                Populate();
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
void StandardValuesTypeAdapter::ValueMapIterator::MoveNext()
    {
    BeAssert (!IsEnd());
    ++m_currentIndex;
    IsEnd() ? SetAtEnd() : Populate();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
void StandardValuesTypeAdapter::ValueMapIterator::Populate()
    {
    BeAssert (!IsEnd());
    ECValue v;
    if (ECOBJECTS_STATUS_Success == m_instance->GetValue (v, L"ValueMap", m_currentIndex) && v.IsStruct() && !v.IsNull())
        {
        IECInstancePtr structInstance = v.GetStruct();
        if (ECOBJECTS_STATUS_Success == structInstance->GetValue (v, L"DisplayString"))
            {
            m_name = v.IsNull() ? L"" : v.GetString();
            if (ECOBJECTS_STATUS_Success == structInstance->GetValue (v, L"Value"))
                {
                m_value = v.GetInteger();
                return;
                }
            }
        }

    // Something went wrong.
    SetAtEnd();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool StandardValuesTypeAdapter::_Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) const
    {
    if (!v.IsNull() &&v.IsInteger())
        {
        Int32 intVal = v.GetInteger();
        for (ValueMapIterator iter (*context.GetProperty()); !iter.IsEnd(); iter.MoveNext())
            if (iter.GetValue() == intVal)
                return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool StandardValuesTypeAdapter::GetUnformattedStringValue (WStringR valueAsString, ECValueCR v, IDgnECTypeAdapterContextCR context) const
    {
    if (PRIMITIVETYPE_Integer == v.GetPrimitiveType())
        {
        Int32 intVal = v.GetInteger();
        for (ValueMapIterator iter (*context.GetProperty()); !iter.IsEnd(); iter.MoveNext())
            {
            if (iter.GetValue() == intVal)
                {
                valueAsString = iter.GetName();
                return true;
                }
            }
        }
    
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool StandardValuesTypeAdapter::_ConvertFromString (ECValueR v, WCharCP stringVal, IDgnECTypeAdapterContextCR context) const
    {
    for (ValueMapIterator iter (*context.GetProperty()); !iter.IsEnd(); iter.MoveNext())
        {
        if (0 == wcscmp (stringVal, iter.GetName()))
            {
            v.SetInteger (iter.GetValue());
            return true;
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool StandardValuesTypeAdapter::_GetStandardValues (StandardValuesCollection& values, IDgnECTypeAdapterContextCR context) const
    {
    for (ValueMapIterator iter (*context.GetProperty()); !iter.IsEnd(); iter.MoveNext())
        values.push_back (iter.GetName());

    return true;
    }

///////////////////////////////////////////
//  BooleanDisplayTypeAdapter
//////////////////////////////////////////

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr BooleanDisplayTypeAdapter::GetCustomAttributeInstance (IDgnECTypeAdapterContextCR context) const
    {
    return context.GetProperty()->GetCustomAttribute (L"BooleanDisplay");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
WString BooleanDisplayTypeAdapter::GetBooleanString (bool boolVal, IDgnECTypeAdapterContextCR context) const
    {
    IECInstancePtr customAttr;
    if ((customAttr = GetCustomAttributeInstance (context)).IsValid())
        {
        ECValue v;
        if (ECOBJECTS_STATUS_Success == customAttr->GetValue (v, boolVal ? L"TrueString" : L"FalseString"))
            return v.GetString();
        }

    BeAssert (false);
    return GetResourceString (BoolWord_TrueFalse, boolVal);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool BooleanDisplayTypeAdapter::_GetStandardValues (StandardValuesCollection& values, IDgnECTypeAdapterContextCR context) const
    {
    IECInstancePtr customAttr = GetCustomAttributeInstance (context);
    if (customAttr.IsValid())
        {
        ECValue v;
        if (ECOBJECTS_STATUS_Success != customAttr->GetValue (v, L"TrueString"))
            { BeAssert (false); return false; }

        values.push_back (v.GetString());
        if (ECOBJECTS_STATUS_Success != customAttr->GetValue (v, L"FalseString"))
            { BeAssert (false); return false; }

        values.push_back (v.GetString());
        
        return true;
        }

    return false;
    }

/////////////////////////////////////////////////
//  PrimitiveAdapterBase
/////////////////////////////////////////////////

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool PrimitiveAdapterBase::_Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) const
    {
    if (v.IsNull())
        return context.GetComponentIndex() == -1;   // can't accept NULL for a component value
    else if (!v.IsPrimitive())
        return false;
    else if (-1 != context.GetComponentIndex())
        return (m_primitiveType == PRIMITIVETYPE_Point3D || m_primitiveType == PRIMITIVETYPE_Point2D) && v.IsDouble();
    else
        return v.GetPrimitiveType() == m_primitiveType;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool PrimitiveAdapterBase::_ConvertToString (WStringR str, ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP formatter) const
    {
    str = v.ToString();
    return true;
    }

////////////////////////////////////////////////////
//      Barebones primitive types
///////////////////////////////////////////////////

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool MissingExtendTypeAdapter::_ConvertToString (WStringR str, ECValueCR v, IDgnECTypeAdapterContextCR context, IECInstanceCP opts) const
    {
    // without access to the real type adapter, we can't provide much formatting.
    str = v.IsNull() ? GetParensNullString() : v.ToString();
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool LongTypeAdapter::_ConvertFromString (ECValueR v, WCharCP str, IDgnECTypeAdapterContextCR context) const
    {
    Int64 l;
    if (1 == BE_STRING_UTILITIES_SWSCANF (str, s_fmtLong, &l))
        {
        v.SetLong (l);
        return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool BinaryTypeAdapter::_ConvertFromString (ECValueR v, WCharCP str, IDgnECTypeAdapterContextCR context) const
    {
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool Point3DTypeAdapter::_ConvertFromString (ECValueR v, WCharCP str, IDgnECTypeAdapterContextCR context) const
    {
    // Point properties should always have an extended type (E.g. Coordinates or UnitlessPoint).
    // Without it, we have no context within which to interpret the point - so treat as unitless
    if (-1 != context.GetComponentIndex())
        {
        DistanceParserPtr   parser = DistanceParser::Create();

        double              distance;
        if (SUCCESS != parser->ToValue (distance, str))
            return false;

        v.SetDouble (distance);
        return true;
        }
    else
        {
        PointParserPtr      parser = PointParser::Create();
        parser->SetIs3d (context.Is3d());

        DPoint3d            point;
        if (SUCCESS != parser->ToValue (point, str))
            return false;

        v.SetPoint3D (point);
        return true;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool Point2DTypeAdapter::_ConvertFromString (ECValueR v, WCharCP str, IDgnECTypeAdapterContextCR context) const
    {
    if (-1 != context.GetComponentIndex())
        {
        double              distance;
        DistanceParserPtr   parser = DistanceParser::Create();

        if (SUCCESS != parser->ToValue (distance, str))
            return false;

        v.SetDouble (distance);
        return true;
        }
    else
        {
        DPoint3d            pt3d;
        PointParserPtr      parser = PointParser::Create();
        parser->SetIs3d (false);

        if (SUCCESS != parser->ToValue (pt3d, str))
            return false;

        DPoint2d pt2d = DPoint2d::From (pt3d.x, pt3d.y);
        v.SetPoint2D (pt2d);
        return true;
        }
    }

////////////////////////////////////////////////////////////////////////////
//  UnitlessPointTypeAdapter
//  Managed code can supply a coordinate index in the context for these
//  methods. The ECValue should always be a 3D point, though.
////////////////////////////////////////////////////////////////////////////

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool PointFormatTypeAdapter::_Validate (ECValueCR v, IDgnECTypeAdapterContextCR context) const
    {
    return -1 == context.GetComponentIndex() ? (v.IsPoint3D() || v.IsNull()) : v.IsDouble();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool PointFormatTypeAdapter::ExtractOptions (DPoint3d& pt, bool& useX, bool& useY, bool& useZ, WChar& separator, ECValueCR v, ECN::IECInstanceCP opts, IDgnECTypeAdapterContextCR context) const
    {
    UInt32 componentIndex = context.GetComponentIndex();
    if ((componentIndex == -1 && !v.IsPoint3D()) || (componentIndex != -1 && !v.IsDouble()))
        return false;

    switch (componentIndex)
        {
    case 0:
        useX = true;
        useY = useZ = false;
        pt = DPoint3d::FromXYZ (v.GetDouble(), 0, 0);
        break;
    case 1:
        useY = true;
        useX = useZ = false;
        pt = DPoint3d::FromXYZ (0, v.GetDouble(), 0);
        break;
    case 2:
        useZ = true;
        useX = useY = false;
        pt = DPoint3d::FromXYZ (0, 0, v.GetDouble());
        break;
    case (UInt32)-1:
        {
        Int32 coordOpts = getIntOrDefault (opts, L"Coordinates", 0);
        if (coordOpts == 0)     // default
            coordOpts = 0xFFFFFFFF;

        useX = 0 != (coordOpts & 1);
        useY = 0 != (coordOpts & 2);
        useZ = 0 != (coordOpts & 4);
        if (useZ && !context.Is3d())
            {
            // by default we hide the z - only show it if user specifically requested
            useZ = coordOpts != 0xffffffff;
            }

        pt = v.GetPoint3D();
        }
        break;
    default:
        BeAssert (false);
        }

    separator = getSeparatorOrDefault (opts, L"LS", L',');

    if (UseGlobalOrigin())
        {
        DgnModelP modelRef = context.GetDgnModel();
        if (modelRef)
            pt.Subtract (modelRef->GetGlobalOrigin());
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void PointFormatTypeAdapter::InitOptions (IECInstanceR opts) const
    {
    ECValue v;
    v.SetInteger ((Int32)L',');     opts.SetValue (L"LS",           v);
    v.SetInteger (0);               opts.SetValue (L"Coordinates",  v);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool UnitlessPointTypeAdapter::_ConvertToString (WStringR str, ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP formatter) const
    {
    str.clear();
    DoubleFormatterPtr fmtr = DoubleFormatter::Create();
    DoubleTypeAdapter::InitFormatter (*fmtr, formatter);
    return Format (str, v, *fmtr, context, formatter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool UnitlessPointTypeAdapter::_ConvertFromString (ECValueR v, WCharCP str, IDgnECTypeAdapterContextCR context) const
    {
    if (context.GetComponentIndex() != -1)
        {
        double d;
        if (1 == BE_STRING_UTILITIES_SWSCANF (str, s_fmtDouble, &d))
            {
            v.SetDouble(d);
            return true;
            }
        }
    else
        {
        DPoint3d        point;
        PointParserPtr  parser = PointParser::Create();
        parser->SetIs3d (context.Is3d());

        if (SUCCESS != parser->ToValue (point, str))
            return false;

        v.SetPoint3D (point);
        return true;
        }

    return false;
    }

/////////////////////////////////
//  FileSizeTypeAdapter
////////////////////////////////

static const Int64      s_kilobyte          = 0x400;
static const Int64      s_megabyte          = 0x100000;
static const Int64      s_gigabyte          = 0x40000000;


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
Int32 FileSizeTypeAdapter::LookupFormatIndex (WCharCP formatString)
    {
    if (NULL == formatString || 0 == wcslen (formatString))
        return 0;
    else if (0 == wcscmp (formatString, L"%ld%by1"))
        return 1;
    else if (0 == wcscmp (formatString, L"%.2f%by2"))
        return 2;
    else if (0 == wcscmp (formatString, L"%.2f%by3"))
        return 3;
    else
        return -1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool FileSizeTypeAdapter::_Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) const
    {
    return v.IsLong() && !v.IsNull();
    }

/*---------------------------------------------------------------------------------**//**
* Give as kilobytes rounded.
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool FileSizeTypeAdapter::_ConvertToString (WStringR strVal, ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP formatter) const
    {
    if (!v.IsLong())
        return false;

    Int64 nBytes = v.GetLong();
    double nKb   = ((double)nBytes) / (double)s_kilobyte;

    InteropStringFormatter::GetInstance().FormatValue (strVal, L"N0", ECValue (nKb));  // integer format with thousands separators
    strVal.append (1, ' ');
    strVal.append (GetKBString());
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool FileSizeTypeAdapter::_ConvertFromString (ECValueR v, WCharCP str, IDgnECTypeAdapterContextCR context) const
    {
    // It is not possible to accurately convert the rounded value back to the original size in bytes
    return false;
    }

///////////////////////////////////////
//  FileSizeLongTypeAdapter
///////////////////////////////////////

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/12
+---------------+---------------+---------------+---------------+---------------+------*/
FileSizeLongTypeAdapter::FileSizeLongTypeAdapter()
    {
    m_BytesString = RSC_STR (Bytes);
    m_MBString = RSC_STR (MB);
    m_GBString = RSC_STR (GB);
    }

/*---------------------------------------------------------------------------------**//**
* Produces a string like "40.50 KB (41,472 bytes)"
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool FileSizeLongTypeAdapter::_ConvertToString (WStringR strVal, ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP formatter) const
    {
    if (!v.IsLong())
        return false;

    Int64 nBytes = v.GetLong();
    WCharCP units;
    WCharCP fmt   = L"N2";
    double size = (double)nBytes;

    if (nBytes < s_kilobyte)
        {
        units = m_BytesString.c_str();
        fmt = L"N0";
        }
    else if (nBytes < s_megabyte)
        {
        units = GetKBString();
        size /= (double)s_kilobyte;
        }
    else if (nBytes < s_gigabyte)
        {
        units = m_MBString.c_str();
        size /= (double)s_megabyte;
        }
    else
        {
        units = m_GBString.c_str();
        size /= (double)s_gigabyte;
        }

    BeAssert (NULL != fmt && NULL != units);

    IECInteropStringFormatter const& fmtr = InteropStringFormatter::GetInstance();
    WString bytesString;
    fmtr.FormatValue (strVal, fmt, ECValue (size));
    strVal.append (1, ' ');
    strVal.append (units);
    strVal.append (L" (");
    fmtr.FormatValue (bytesString, RMAXUI4 > (double)nBytes ? L"N0" : L"E12", ECValue (nBytes));    // DoubleFormatter will convert to scientific notation anyway if nBytes exceeds RMAXUI4, so increase precision
    strVal.append (bytesString);
    strVal.append (1, ' ');
    strVal.append (m_BytesString);
    strVal.append (1, ')');
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool FileSizeLongTypeAdapter::_ConvertFromString (ECValueR v, WCharCP strVal, IDgnECTypeAdapterContextCR context) const
    {
    WCharCP byteStr = ::wcschr (strVal, '(');
    if (NULL != byteStr)
        {
        WString toParse (byteStr + 1);
        if (WString::npos == toParse.find ('.'))
            {
            boost::algorithm::erase_all (toParse, L",");
        
            Int64 nBytes;
            if (1 == BE_STRING_UTILITIES_SWSCANF (toParse.c_str(), L"%lld", &nBytes))
                {
                v.SetLong (nBytes);
                return true;
                }
            }
        else
            {
            double nBytes;
            if (1 == BE_STRING_UTILITIES_SWSCANF (toParse.c_str(), L"%lf", &nBytes))
                {
                v.SetLong ((Int64)nBytes);
                return true;
                }
            }
        }

    return false;
    }

////////////////////////////////////////////////
//  Units-based adapters.
//  DgnModel optional; if supplied, units are
//  taken from model.
////////////////////////////////////////////////

///////////////////////////////////
//      AngleFormat
//////////////////////////////////

///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                                    Paul.Connelly   03/14
//+---------------+---------------+---------------+---------------+---------------+------*/
//bool AngleFormatTypeAdapter::_CanConvertFromString (IDgnECTypeAdapterContextCR context) const
//    {
//    return !hasPropertyDependency (context);
//    }
//
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool AngleFormatTypeAdapter::_Validate (ECValueCR v, IDgnECTypeAdapterContextCR context) const
    {
    return v.IsNull() || v.IsDouble();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void AngleFormatTypeAdapter::InitOptions (IECInstanceR opts)
    {
    DoubleTypeAdapter::InitOptions (opts);
    opts.SetValue (L"Format", ECValue ((Int32)AngleFormatVals::Active));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void AngleFormatTypeAdapter::InitFormatter (AngleFormatter& fmtr, IECInstanceCP opts)
    {
    bool leadingZero = getBoolOrDefault (opts, L"LeadingZero", true);
    bool trailingZeros = getBoolOrDefault (opts, L"TrailingZeros", true);
   //  WChar decSep = getSeparatorOrDefault (opts, L"DecimalSeparator", '.'); ###TODO: AngleFormatter does not support custom decimal separator...

    Int32 angleFormat = getIntOrDefault (opts, L"Format", (Int32)AngleFormatVals::Active);
    switch ((AngleFormatVals)angleFormat)
        {
    case AngleFormatVals::Degrees:
        fmtr.SetAngleMode (AngleMode::Degrees); break;
    case AngleFormatVals::DegMinSec:
        fmtr.SetAngleMode (AngleMode::DegMinSec); break;
    case AngleFormatVals::Centesimal:
        fmtr.SetAngleMode (AngleMode::Centesimal); break;
    case AngleFormatVals::Radians:
        fmtr.SetAngleMode (AngleMode::Radians); break;
    case AngleFormatVals::DegMin:
        fmtr.SetAngleMode (AngleMode::DegMin); break;
    default:
        // use active/default
        break;
        }

    fmtr.SetLeadingZero (leadingZero);
    fmtr.SetTrailingZeros (trailingZeros);

    Int32 accuracy = getIntOrDefault (opts, L"Accuracy", USE_ACTIVE);
    if (0 <= accuracy)
        fmtr.SetAnglePrecision ((AnglePrecision)accuracy);               // should be in range 0..6, fmtr accepts 0..8
    }

/*---------------------------------------------------------------------------------**//**
* Used for Angle and Direction. Convert to degrees for display.
* @bsimethod                                                    Paul.Connelly   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool AngleFormatTypeAdapter::_ConvertToDisplayType (ECValueR v, IDgnECTypeAdapterContextCR context, IECInstanceCP opts) const
    {
    AngleMode mode = AngleMode::Degrees;
    Int32 nDecimalPlaces = 4;
    DgnModelP modelRef = context.GetDgnModel();
    ModelInfoCR modelInfo = modelRef->GetModelInfo();
    mode = modelInfo.GetAngularMode();
    nDecimalPlaces = (Int32)modelInfo.GetAngularPrecision();

    // DirectionFormatter doesn't have the "Format" property. Will use degrees by default.
    switch (getIntOrDefault (opts, L"Format", (Int32)AngleFormatVals::Active))
        {
    case USE_ACTIVE:
        break;
    case (Int32) AngleFormatVals::Centesimal:
        mode = AngleMode::Centesimal;
        break;
    case (Int32) AngleFormatVals::Radians:
        mode = AngleMode::Radians;
        break;
    default:
        mode = AngleMode::Degrees;
        break;
        }

    Int32 accuracy = getIntOrDefault (opts, L"Accuracy", USE_ACTIVE);
    if (0 <= accuracy)
        nDecimalPlaces = accuracy;

    double angle = v.GetDouble();
    angle *= msGeomConst_degreesPerRadian;
    bool negative = false;
    if (getBoolOrDefault (opts, L"AllowNegative", true) && angle < 0.0)
        {
        angle = fabs (angle);
        negative = true;
        }

    // clamp
    if (fabs (angle) > 1.0e+10) angle = 0.0;
    while (angle < 0.0)         angle += 360.00;
    while (angle > 360.0)       angle -= 360.00;

    switch (mode)
        {
    case AngleMode::Centesimal: angle *=  100.0 / 90.0; break;
    case AngleMode::Radians:    angle *= msGeomConst_radiansPerDegree; break;
        }

    double fac = pow (10.0, nDecimalPlaces);
    angle = std_round (angle * fac) / fac;

    if (negative)
        angle = -angle;

    v.SetDouble (angle);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool AngleFormatTypeAdapter::_GetUnits (UnitSpecR spec, IDgnECTypeAdapterContextCR context) const
    {
    spec = UnitSpec (BASEUNIT_RADIAN, UnitConverter (/*Identity*/));
    return true;
    }

///////////////////////////////////////////
//          Angle
//////////////////////////////////////////

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool AngleTypeAdapter::_ConvertToString (WStringR str, ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP opts) const
    {
    if (v.IsNull())
        str = GetParensNullString();
    else if (!v.IsDouble())
        return false;
    else
        {
        DgnModelP modelRef = context.GetDgnModel();
        AngleFormatterPtr fmtr = modelRef ? AngleFormatter::Create (*modelRef) : AngleFormatter::Create();
        InitFormatter (*fmtr, opts);

        fmtr->SetAllowNegative (getBoolOrDefault (opts, L"AllowNegative", true));
        str = fmtr->ToString (v.GetDouble());

        Int32 angleFormat = getIntOrDefault (opts, L"Format", (Int32)AngleFormatVals::Active);
#if defined (_MSC_VER)
    #pragma warning (disable : 4428)   // Workaround incorrect -W4 compiler warning - http://msdn.microsoft.com/en-us/library/ttw8abkd.aspx
#endif // defined (_MSC_VER)
        if (-2 == angleFormat && !str.empty() && L'\u00B0' == str[str.length()-1])
#if defined (_MSC_VER)
    #pragma warning (default: 4428)
#endif // defined (_MSC_VER)
            {
            // special case - do not include the degree symbol
            str.erase (str.length()-1);
            }

        DoubleTypeAdapter::ApplyPrefixSuffix (str, opts);
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool AngleTypeAdapter::_ConvertFromString (ECValueR v, WCharCP str, IDgnECTypeAdapterContextCR context) const
    {
    AngleParserPtr parser = AngleParser::Create();
    DgnModelP modelRef = context.GetDgnModel();
    if (modelRef)
        parser->SetAngleMode (modelRef->GetModelInfo().GetAngularMode());

    double  angleVal;
    if (SUCCESS != parser->ToValue (angleVal, str))
        return false;

    v.SetDouble (angleVal);
    return true;
    }

//////////////////////////////
//      Direction
/////////////////////////////

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool DirectionAngleTypeAdapter::_ConvertToString (WStringR str, ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP opts) const
    {
    if (v.IsNull())
        str = GetParensNullString();
    else if (!v.IsDouble())
        return false;
    else
        {
        DgnModelP modelRef = context.GetDgnModel();
        DirectionFormatterPtr fmtr = modelRef ? DirectionFormatter::Create (*modelRef) : DirectionFormatter::Create();
        InitFormatter (fmtr->GetAngleFormatter(), opts);

        fmtr->SetAddTrueNorth (false);
        Int32 mode = getIntOrDefault (opts, L"Mode", USE_ACTIVE);
        if (USE_ACTIVE != mode)
            fmtr->SetDirectionModeFromLegacy (mode);

        str = fmtr->ToString (v.GetDouble());
        DoubleTypeAdapter::ApplyPrefixSuffix (str, opts);
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool DirectionAngleTypeAdapter::_ConvertFromString (ECValueR v, WCharCP str, IDgnECTypeAdapterContextCR context) const
    {
    DirectionParserPtr parser = DirectionParser::Create();

    DgnModelP modelRef = context.GetDgnModel();
    if (modelRef)
        {
        ModelInfoCR modelInfo = modelRef->GetModelInfo();

        parser->SetDirectionMode (modelInfo.GetDirectionMode());
        parser->SetTrueNorthValue (context.GetDgnModel()->GetDgnProject().Units().GetAzimuth());
        parser->SetBaseDirection (modelInfo.GetDirectionBaseDir());
        parser->SetClockwise (modelInfo.GetDirectionClockwise());

        parser->GetAngleParser().SetAngleMode (modelInfo.GetAngularMode());
        }

    double  dirValue;
    if (SUCCESS != parser->ToValue (dirValue, str))
        return false;

    v.SetDouble (dirValue);
    return true;
    }

//////////////////////////////////////
//          XyzRotations
/////////////////////////////////////

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool XyzRotationsTypeAdapter::_ConvertToString (WStringR str, ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP formatter) const
    {
    // Note no special formatting options are supported for this type
    str.clear();
    DgnModelP modelRef = context.GetDgnModel();
    AngleFormatterPtr fmtr = modelRef ? AngleFormatter::Create (*modelRef) : AngleFormatter::Create();
    return Format (str, v, *fmtr, context, formatter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool XyzRotationsTypeAdapter::_ConvertFromString (ECValueR v, WCharCP str, IDgnECTypeAdapterContextCR context) const
    {
    if (!v.IsDouble())
        return false;   // can only set individual components from string value...

    DgnModelP modelRef = context.GetDgnModel();
    AngleParserPtr parser = AngleParser::Create();
    if (modelRef)
        parser->SetAngleMode (modelRef->GetModelInfo().GetAngularMode());

    double  angleVal;
    if (SUCCESS != parser->ToValue (angleVal, str))
        return false;

    v.SetDouble (angleVal);
    return true;
    }

// WIP_TOPAZ_MERGE: brought over from Topaz, but it needs to be updated for graphite
//static const double FEET_IN_ACRE = 43560.0;
//static const double SQRT_ACRE = sqrt (FEET_IN_ACRE);
//
///*---------------------------------------------------------------------------------**//**
//* @bsistruct                                                    Paul.Connelly   12/13
//+---------------+---------------+---------------+---------------+---------------+------*/
//struct LinearUnitsConverter
//    {
//private:
//    UnitDefinition          m_masterUnit, m_subUnit, m_stgUnit;
//    double                  m_uorPerStg;
//    Int8                    m_nDecimalPlaces;
//    double                  m_conversionFactor;
//
//    double                  GetSubPerMaster() const { double spm; m_subUnit.ConvertDistanceFrom (spm, 1.0, m_masterUnit); return spm; }
//    double                  GetPosPerSub() const    { double pps; m_stgUnit.ConvertDistanceFrom (pps, m_uorPerStg, m_subUnit); return pps; }
//    double                  ApplyAccuracy (double d) const
//        {
//        double fac = pow (10.0, m_nDecimalPlaces);
//        return std_round (d * fac) / fac;
//        }
//public:
//    LinearUnitsConverter (IDgnECTypeAdapterContextCR context, IECInstanceCP opts, bool isDistance) : m_uorPerStg (1.0), m_nDecimalPlaces (4)
//        {
//        // init defaults or get active settings from model
//        DgnUnitFormat unitFmt = DgnUnitFormat::MU;
//        DgnModelP modelRef = context.GetDgnModel();
//        if (nullptr != modelRef)
//            {
//            ModelInfoCR modelInfo = modelRef->GetModelInfo();
//            PrecisionFormat prec = modelInfo.GetLinearPrecision();
//            m_nDecimalPlaces = (((Int32)prec) & 0x0000000F);
//
//            unitFmt = modelInfo.GetLinearUnitMode();
//
//            m_masterUnit = modelInfo.GetMasterUnit();
//            m_subUnit = modelInfo.GetSubUnit();
//            m_stgUnit = modelInfo.GetStorageUnit();
//            m_uorPerStg = modelInfo.GetUorPerStorage();
//            }
//        else
//            {
//            m_masterUnit.Init (UnitBase::Meter, UnitSystem::Metric, 1.0, 1.0, NULL);
//            m_subUnit = m_masterUnit;
//            m_stgUnit = m_subUnit;
//            }
//
//        // options can override accuracy
//        Int32 accuracy = getIntOrDefault (opts, L"Accuracy", USE_ACTIVE);
//        if (0 <= accuracy)
//            m_nDecimalPlaces = (accuracy & 0x0000000F);
//
//        // all but distance can apply multiplier to converted value
//        m_conversionFactor = getDoubleOrDefault (opts, L"CF", 1.0);
//
//        // options can override master and sub units
//        if (isDistance)
//            {
//            Int32 masterUnitId = getIntOrDefault (opts, L"MasterUnits", USE_ACTIVE);
//            UnitDefinition tmpUnit;
//            if (USE_ACTIVE != masterUnitId)
//                {
//                tmpUnit = UnitDefinition::GetStandardUnit ((StandardUnit)masterUnitId);
//                if (tmpUnit.IsValid())
//                    m_masterUnit = tmpUnit;
//                }
//
//            Int32 subUnitId = getIntOrDefault (opts, L"SecondaryUnits", USE_ACTIVE);
//            if (USE_ACTIVE != subUnitId && (tmpUnit = UnitDefinition::GetStandardUnit ((StandardUnit)subUnitId)).IsValid())
//                m_subUnit = tmpUnit;
//
//            Int32 labelFormat = getIntOrDefault (opts, L"LabelFormat", USE_ACTIVE);
//            if (DistanceTypeAdapter::Format_SU == labelFormat || DistanceTypeAdapter::Format_SU_Label == labelFormat)
//                unitFmt = DgnUnitFormat::SU;
//
//            // sub-units instead of master?
//            if (DgnUnitFormat::SU == unitFmt)
//                m_masterUnit = m_subUnit;
//            }
//        else
//            {
//            Int32 unitInt = getIntOrDefault (opts, L"Units", USE_ACTIVE);
//            if (unitInt >= 0)
//                {
//                UnitDefinition masterUnit = UnitDefinition::GetStandardUnit ((StandardUnit)unitInt);
//                if (masterUnit.IsValid())
//                    m_masterUnit = masterUnit;
//                }
//            else if (USE_ACRES == unitInt)
//                {
//                m_masterUnit = UnitDefinition::GetStandardUnit (StandardUnit::EnglishFeet);
//                UnitInfo unitInfo = m_masterUnit.ToUnitInfo();
//                unitInfo.numerator /= SQRT_ACRE;
//                m_masterUnit.Init (unitInfo);
//                }
//            else if (AreaOrVolumeTypeAdapter::USE_ACTIVE_SUBUNITS == unitInt)
//                {
//                m_masterUnit = m_subUnit;
//                }
//            }
//        }
//
//    double      ConvertDistance (double uors) const
//        {
//        double subPerMast = GetSubPerMaster(), posPerSub = GetPosPerSub();
//        double dist = uors / (subPerMast * posPerSub);
//        return ApplyAccuracy (dist);
//        }
//    double      ConvertArea (double uors) const
//        {
//        UnitDefinition sqMast = m_masterUnit, sqStg = m_stgUnit;
//        sqMast.Square();
//        sqStg.Square();
//
//        double sqStgPerMast;
//        sqStg.GetConversionFactorFrom (sqStgPerMast, sqMast);
//
//        double sqUorsPerMast = sqStgPerMast * (m_uorPerStg * m_uorPerStg);
//        double area = uors / sqUorsPerMast;
//        return ApplyAccuracy (area * m_conversionFactor);
//        }
//    double      ConvertVolume (double uors) const
//        {
//        UnitDefinition cuMast = m_masterUnit, cuStg = m_stgUnit;
//        cuMast.Cube();
//        cuStg.Cube();
//
//        double cuStgPerMast;
//        cuStg.GetConversionFactorFrom (cuStgPerMast, cuMast);
//
//        double cuUorsPerMast = cuStgPerMast * (m_uorPerStg * m_uorPerStg * m_uorPerStg);
//        double volume = uors / cuUorsPerMast;
//        return ApplyAccuracy (volume * m_conversionFactor);
//        }
//    };
//

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool DistanceTypeAdapter::_Validate (ECValueCR v, IDgnECTypeAdapterContextCR context) const
    {
    return v.IsDouble() || v.IsNull();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool DistanceTypeAdapter::_ConvertToString (WStringR str, ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP opts) const
    {
    if (v.IsNull() || !v.IsDouble())
        return false;

    DgnModelP modelRef = context.GetDgnModel();
    DistanceFormatterPtr fmtr = modelRef ? DistanceFormatter::Create (*modelRef) : DistanceFormatter::Create();
    InitFormatter (*fmtr, modelRef, opts);

    double storedValue = v.GetDouble();
    UorMeterConverter<1> (context).ConvertForDisplay (storedValue);
    
    str = fmtr->ToString (storedValue);
    DoubleTypeAdapter::ApplyPrefixSuffix (str, opts);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool DistanceTypeAdapter::_ConvertFromString (ECValueR v, WCharCP str, IDgnECTypeAdapterContextCR context) const
    {
    DgnModelP modelRef = context.GetDgnModel();

    DistanceParserPtr   parser;

    if (modelRef)
        parser = DistanceParser::Create(*modelRef);
    else
        parser = DistanceParser::Create();

    double  distance;
    if (SUCCESS != parser->ToValue (distance, str))
        return false;

    UorMeterConverter<1> (context).ConvertForStorage (distance);
    v.SetDouble (distance);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DistanceTypeAdapter::InitFormatter (DistanceFormatterR fmtr, DgnModelP model, ECN::IECInstanceCP opts)
    {
    DoubleTypeAdapter::InitFormatter (fmtr, opts);
    Int32 labelFormat = getIntOrDefault (opts, L"LabelFormat", USE_ACTIVE);

    // Temporary for graphite only - always set up to show the units explicitly.
    // Note: Need a better way to pass this in - needs coordination with the Topaz team. 
    if (opts == NULL)
        fmtr.SetUnitLabelFlag (true);

    if (USE_ACTIVE != labelFormat)
        {
        fmtr.SetUnitLabelFlag (true);
        switch (labelFormat)
            {
            case Format_MU:
                fmtr.SetUnitLabelFlag (false);  /* and fall through */
            case Format_MU_Label:
                fmtr.SetUnitFormat (DgnUnitFormat::MU);
                break;
            case Format_SU:
                fmtr.SetUnitLabelFlag (false);  /* and fall through */
            case Format_SU_Label:
                fmtr.SetUnitFormat (DgnUnitFormat::SU);
                break;
            case Format_MUSU:
                fmtr.SetUnitLabelFlag (false);  /* and fall through */
            case Format_MUSU_Label:
                fmtr.SetUnitFormat (DgnUnitFormat::MUSU);
                break;
            default:
                BeAssert (false);
            }
        }

    UnitDefinition masterUnits, subUnits;
    Int32 masterUnitInt = getIntOrDefault (opts, L"MasterUnits", USE_ACTIVE);
    if (USE_ACTIVE != masterUnitInt)
        masterUnits = UnitDefinition::GetStandardUnit ((StandardUnit)masterUnitInt);

    Int32 subUnitInt = getIntOrDefault (opts, L"SecondaryUnits", USE_ACTIVE);
    if (USE_ACTIVE != subUnitInt)
        subUnits = UnitDefinition::GetStandardUnit ((StandardUnit)subUnitInt);
    else
        {
        // Note: if we pass NULL to SetWorkingUnits() below, it will set subunits equal to masterunits, which is not what we want.
        subUnits = fmtr.GetSubUnit();
        }

    if (masterUnits.IsValid())
        fmtr.SetWorkingUnits (masterUnits, subUnits.IsValid() ? &subUnits : NULL);

    // Shared by DWG formatting logic and ordinary formatting logic
    bool showZeroMasterUnits = getBoolOrDefault (opts, L"ZMU", true);
    fmtr.SetSuppressZeroMasterUnits (!showZeroMasterUnits);
    bool showZeroSubUnits = getBoolOrDefault (opts, L"ZSU", true);
    fmtr.SetSuppressZeroSubUnits (!showZeroSubUnits);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DistanceTypeAdapter::InitOptions (IECInstanceR opts)
    {
    DoubleTypeAdapter::InitOptions (opts);
    ECValue v;
    v.SetInteger (USE_ACTIVE);      opts.SetValue (L"LabelFormat",      v);
                                    opts.SetValue (L"MasterUnits",      v);
                                    opts.SetValue (L"SecondaryUnits",   v);
    v.SetInteger (1);               opts.SetValue (L"ZMU",              v);
    v.SetInteger (1);               opts.SetValue (L"ZSU",              v);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool CoordinatesTypeAdapter::_ConvertToString (WStringR str, ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP opts) const
    {
    str.clear();
    if (v.IsNull())
        return false;

    DgnModelP modelRef = context.GetDgnModel();
    DistanceFormatterPtr fmtr = modelRef ? DistanceFormatter::Create (*modelRef) : DistanceFormatter::Create();
    DistanceTypeAdapter::InitFormatter (*fmtr, modelRef, opts);
    
    ECValue valueForDisplay;
    if (v.IsDouble())
        {
        double d = v.GetDouble();
        UorMeterConverter<1> (context).ConvertForDisplay (d);
        valueForDisplay.SetDouble (d);
        }
    else
        {
        DPoint3d ptForDisplay = v.GetPoint3D();
        CoordinateConverter (context).ConvertForDisplay (ptForDisplay);
        valueForDisplay.SetPoint3D (ptForDisplay);
        }

    return Format (str, valueForDisplay, *fmtr, context, opts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool CoordinatesTypeAdapter::_GetUnits (UnitSpecR units, IDgnECTypeAdapterContextCR context) const
    {
    return false; // WIP_TOPAZ_MERGE: This needs a real implementation on graphite
//    return getWorkingUnitsSpec (units, context, 1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool CoordinatesTypeAdapter::_ConvertFromString (ECValueR v, WCharCP str, IDgnECTypeAdapterContextCR context) const
    {
    DgnModelP modelRef = context.GetDgnModel();
    DPoint3d globalOrigin = modelRef ? modelRef->GetGlobalOrigin() : DPoint3d::FromXYZ (0, 0, 0);

    DPoint3d pt;
    UInt32 componentIndex = context.GetComponentIndex();
    if (-1 != componentIndex)
        {

        DistanceParserPtr   parser;

        if (modelRef)
            parser = DistanceParser::Create(*modelRef);
        else
            parser = DistanceParser::Create();

        double  distance;
        if (SUCCESS != parser->ToValue (distance, str))
            return false;

        // apply global origin
        double offset = (0==componentIndex) ? globalOrigin.x : ((1==componentIndex) ? globalOrigin.y : globalOrigin.z);
        v.SetDouble (distance + offset);
        return true;
        }
    else
        {
        PointParserPtr parser;

        if (modelRef)
             parser = PointParser::Create(*modelRef);
        else
             parser = PointParser::Create();

        parser->SetIs3d (context.Is3d());

        if (SUCCESS != parser->ToValue (pt, str))
            return false;
        pt.Add (globalOrigin);
        }

    DPoint3d ptForStorage = DPoint3d::FromXYZ (pt.x, pt.y, context.Is3d() ? pt.z : 0.0);
    CoordinateConverter (context).ConvertForStorage (ptForStorage);
    v.SetPoint3D (ptForStorage);
    return true;
    }

///////////////////////////////////////////////////////
//      Area/Volume
//////////////////////////////////////////////////////

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool AreaOrVolumeTypeAdapter::_Validate (ECValueCR v, IDgnECTypeAdapterContextCR context) const
    {
    // Area/Volume are calculated properties, thus read-only
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool AreaOrVolumeTypeAdapter::_ConvertFromString (ECValueR v, WCharCP str, IDgnECTypeAdapterContextCR context) const
    {
    // Area/Volume are calculated properties, thus read-only
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void AreaOrVolumeTypeAdapter::InitFormatter (AreaOrVolumeFormatterBase& fmtr, IECInstanceCP opts, DgnModelP model) const
    {
    static const double FEET_IN_ACRE = 43560.0;
    static const double SQRT_ACRE = sqrt (FEET_IN_ACRE);

    DoubleTypeAdapter::InitFormatter (fmtr, opts);

    double conversionFactor = getDoubleOrDefault (opts, L"CF", 1.0);
    fmtr.SetScaleFactor (conversionFactor);

    bool useAcres = false;
    Int32 unitInt = getIntOrDefault (opts, L"Units", USE_ACTIVE);
    if (unitInt >= 0)
        {
        UnitDefinition masterUnit = UnitDefinition::GetStandardUnit ((StandardUnit)unitInt);
        if (masterUnit.IsValid())
            fmtr.SetMasterUnit (masterUnit);
        }
    else if (USE_ACRES == unitInt)
        {
        UnitDefinition masterUnit = UnitDefinition::GetStandardUnit (StandardUnit::EnglishFeet);
        UnitInfo unitInfo = masterUnit.ToUnitInfo();
        unitInfo.numerator /= SQRT_ACRE;
        masterUnit.Init (unitInfo);

        fmtr.SetMasterUnit (masterUnit);
        useAcres = true;
        }
    else if (USE_ACTIVE_SUBUNITS == unitInt && NULL != model)
        {
        UnitDefinitionCR subUnit = model->GetModelInfo().GetSubUnit();
        fmtr.SetMasterUnit (subUnit);
        }

    bool showLabel = getBoolOrDefault (opts, L"ShowLabel", false, true);  // Note the default value is 'true' for the formatting IECInstance, but false if we have no IECInstance
    if (!useAcres)
        fmtr.SetShowUnitLabel (showLabel);

    fmtr.SetLabelDecoratorAsSuffix (getBoolOrDefault (opts, L"UnitDecorator", true));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void AreaOrVolumeTypeAdapter::ApplyFormatting (WStringR str, IECInstanceCP opts) const
    {
    StringFormatTypeAdapter::ConvertCase (str, opts);
    DoubleTypeAdapter::ApplyPrefixSuffix (str, opts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
void AreaOrVolumeTypeAdapter::InitOptions (IECInstanceR opts) const
    {
    StringFormatTypeAdapter::InitOptions (opts);
    DoubleTypeAdapter::InitOptions (opts);

    ECValue v;
    v.SetInteger (USE_ACTIVE);  opts.SetValue (L"Units",       v);
    v.SetInteger (1);           opts.SetValue (L"ShowLabel",   v);
                                opts.SetValue (L"UnitDecorator",   v);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool AreaTypeAdapter::_ConvertToString (WStringR str, ECValueCR v, IDgnECTypeAdapterContextCR context, IECInstanceCP opts) const
    {
    if (v.IsNull())
        {
        str = GetParensNullString();
        return true;
        }
    else if (!v.IsDouble())
        return false;

    DgnModelP modelRef = context.GetDgnModel();
    AreaFormatterPtr fmtr = modelRef ? AreaFormatter::Create (*modelRef) : AreaFormatter::Create();
    double area = v.GetDouble();
    UorMeterConverter<2> (context).ConvertForDisplay (area);
    //area *= fmtr->GetUorPerStorageUnit();

    InitFormatter (*fmtr, opts, modelRef);
    str = fmtr->ToString (area);

    if (USE_ACRES == getIntOrDefault (opts, L"Units", 0))
        {
        if (getBoolOrDefault (opts, L"ShowLabel", false, true))
            {
            str.append (L" ");
            str.append (RSC_STR (Acres).c_str());
            }
        }

    ApplyFormatting (str, opts);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool AreaTypeAdapter::_GetUnits (UnitSpecR units, IDgnECTypeAdapterContextCR context) const
    {
    return false; // WIP_TOPAZ_MERGE: This needs a real implementation on graphite
//    return getWorkingUnitsSpec (units, context, 2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool AreaTypeAdapter::_ConvertToDisplayType (ECValueR v, IDgnECTypeAdapterContextCR context, IECInstanceCP opts) const
    {
    //if (v.IsNull() || !v.IsDouble())
    //    return false;

    //LinearUnitsConverter cvtr (context, opts, false);
    //double area = cvtr.ConvertArea (v.GetDouble());
    //v.SetDouble (area);
    //return true;

    return false; // WIP_TOPAZ_MERGE: This needs a real implementation on Graphite.  LinearUnitsConverter needs to be updated
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool VolumeTypeAdapter::_ConvertToString (WStringR str, ECValueCR v, IDgnECTypeAdapterContextCR context, IECInstanceCP opts) const
    {
    if (v.IsNull())
        {
        str = GetParensNullString();
        return true;
        }
    else if (!v.IsDouble())
        return false;

    DgnModelP modelRef = context.GetDgnModel();
    VolumeFormatterPtr fmtr = modelRef ? VolumeFormatter::Create (*modelRef) : VolumeFormatter::Create();
    InitFormatter (*fmtr, opts, modelRef);

    double vol = v.GetDouble();
    UorMeterConverter<3> (context).ConvertForDisplay (vol);
    //double cf = fmtr->GetUorPerStorageUnit();
    str = fmtr->ToString (vol/* * cf * cf*/);
    ApplyFormatting (str, opts);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool VolumeTypeAdapter::_GetUnits (UnitSpecR units, IDgnECTypeAdapterContextCR context) const
    {
    return false; // WIP_TOPAZ_MERGE: This needs a real implementation on graphite
//    return getWorkingUnitsSpec (units, context, 3);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool VolumeTypeAdapter::_ConvertToDisplayType (ECValueR v, IDgnECTypeAdapterContextCR context, IECInstanceCP opts) const
    {
    //if (v.IsNull() || !v.IsDouble())
    //    return false;

    //LinearUnitsConverter cvtr (context, opts, false);
    //double vol = cvtr.ConvertVolume (v.GetDouble());
    //v.SetDouble (vol);
    //return true;

    return false; // WIP_TOPAZ_MERGE: This needs a real implementation on Graphite.  LinearUnitsConverter needs to be updated
    }

////////////////////////////////////////////////////
//              UnitDefinition
///////////////////////////////////////////////////

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool UnitDefinitionTypeAdapter::_Validate (ECValueCR v, IDgnECTypeAdapterContextCR context) const
    {
    if (v.IsNull() || !v.IsInteger())
        return false;

    UnitVisitors::Validater vis (v.GetInteger());
    visitUnits (vis);
    return vis.m_found;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool UnitDefinitionTypeAdapter::_ConvertFromString (ECValueR v, WCharCP str, IDgnECTypeAdapterContextCR context) const
    {
    UnitIteratorOptions opts;
    UserUnitCollection userUnits (opts);
    if (userUnits.begin() != userUnits.end())
        {
        UnitDefinition def = UnitDefinition::GetByName (str);
        if (def.IsValid())
            {
            v.SetInteger (def.GetNumber());
            return true;
            }
        }
    else
        {
        UnitDefinition def = UnitDefinition::GetStandardUnitByName (str);
        Int32 number;
        if (def.IsValid() && (Int32)StandardUnit::Custom != (number = (Int32)def.IsStandardUnit()))
            {
            v.SetInteger (number);
            return true;
            }
        }

    return false;
    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool UnitDefinitionTypeAdapter::GetUnformattedStringValue (WStringR str, ECValueCR v, IDgnECTypeAdapterContextCR context) const
    {    
    if (!v.IsNull() && v.IsInteger())
        {
        str = GetSheetScaleCustomString();
        UnitVisitors::ToStringConverter vis (v.GetInteger(), str);
        visitUnits (vis);
        return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool UnitDefinitionTypeAdapter::_GetStandardValues (StandardValuesCollection& values, IDgnECTypeAdapterContextCR context) const
    {
    UnitVisitors::StandardValuesCollector vis (values);
    visitUnits (vis);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool FormatStringTypeAdapter::_Validate (ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) const
    {
    ECPropertyCP ecprop = context.GetProperty();
    PrimitiveECPropertyCP primProp = ecprop != NULL ? ecprop->GetAsPrimitiveProperty() : NULL;
    if (NULL == primProp)
        return v.IsNull() || (v.IsPrimitive() && v.GetPrimitiveType() == primProp->GetType());
    else
        return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool FormatStringTypeAdapter::GetUnformattedStringValue (WStringR valueAsString, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) const
    {
    struct ValueList : IECInteropStringFormatter::IECValueList
        {
        ECValueCR m_value;
        ValueList (ECValueCR v) : m_value (v) { }

        virtual UInt32      GetCount() const override               { return 1; }
        virtual ECValueCP   operator[](UInt32 index) const override { return index == 0 ? &m_value : NULL; }
        };

    IECInstancePtr attr = NULL != context.GetProperty() ? context.GetProperty()->GetCustomAttribute (L"Format") : NULL;
    WCharCP fmtString = L"{0}";
    ECValue fmtStringVal;
    if (attr.IsValid() && ECOBJECTS_STATUS_Success == attr->GetValue (fmtStringVal, L"FormatString") && fmtStringVal.IsString() && !fmtStringVal.IsNull())
        fmtString = fmtStringVal.GetString();

    return InteropStringFormatter::GetInstance().Format (valueAsString, fmtString, ValueList (v));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool FormatStringTypeAdapter::_ConvertFromString (ECN::ECValueR v, WCharCP stringValue, IDgnECTypeAdapterContextCR context) const
    {
    ECPropertyCP ecprop = context.GetProperty();
    PrimitiveECPropertyCP primProp = NULL != ecprop ? ecprop->GetAsPrimitiveProperty() : NULL;
    if (NULL == primProp)
        return false;

    return InteropStringFormatter::GetInstance().Parse (v, stringValue, primProp->GetType());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECUnitsTypeAdapter::_Validate (ECValueCR v, IDgnECTypeAdapterContextCR context) const
    {
    return v.IsNull() || v.IsDouble() || v.IsLong() || v.IsInteger();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECUnitsTypeAdapter::_GetUnits (UnitSpecR spec, IDgnECTypeAdapterContextCR context) const
    {
    Unit ecunit;
    ECPropertyCP ecprop = context.GetProperty();
    if (nullptr == ecprop || !Unit::GetUnitForECProperty (ecunit, *ecprop))
        return false;

    spec = ecunit;
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECUnitsTypeAdapter::_ConvertToString (WStringR valueAsString, ECValueCR inputVal, IDgnECTypeAdapterContextCR context, IECInstanceCP fmtr) const
    {
    ECValue v (inputVal);
    Unit storedUnit, displayUnit;
    ECPropertyCP ecprop = context.GetProperty();
    IECClassLocaterR unitsECClassLocater = context.GetUnitsECClassLocater();

    if (v.IsNull() || NULL == ecprop || !Unit::GetUnitForECProperty (storedUnit, *ecprop, unitsECClassLocater))
        return false;

    if (NULL != context.GetDgnProject())
        ApplyUnitLabelCustomization (storedUnit, *context.GetDgnProject());

    WString fmt;
    WCharCP label = storedUnit.GetShortLabel();

    if (Unit::GetDisplayUnitAndFormatForECProperty(displayUnit, fmt, storedUnit, *ecprop, unitsECClassLocater))
        {
        if (!v.ConvertToPrimitiveType (PRIMITIVETYPE_Double))
            return false;

        double displayValue = v.GetDouble();
        if (!displayUnit.IsCompatible (storedUnit) || !storedUnit.ConvertTo (displayValue, displayUnit))
            return false;

        if (NULL != context.GetDgnProject())
            ApplyUnitLabelCustomization (displayUnit, *context.GetDgnProject());

        v.SetDouble (displayValue);
        label = displayUnit.GetShortLabel();
        }

    WCharCP fmtCP = fmt.empty() ? L"f" : fmt.c_str();
    if (InteropStringFormatter::GetInstance().FormatValue (valueAsString, fmtCP, v) && NULL != label)
        {
        if (!valueAsString.empty())
            valueAsString.append (1, ' ').append (label);
        return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* Convert to display unit if one specified, else use storage unit.
* @bsimethod                                                    Paul.Connelly   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECUnitsTypeAdapter::_ConvertToDisplayType (ECValueR v, IDgnECTypeAdapterContextCR context, IECInstanceCP opts) const
    {
    if (!v.ConvertToPrimitiveType (PRIMITIVETYPE_Double))
        return false;

    Unit storedUnit, displayUnit;
    ECPropertyCP ecprop = context.GetProperty();
    WString unusedFmt;
    if (!v.IsNull() && NULL != ecprop && Unit::GetUnitForECProperty (storedUnit, *ecprop) && Unit::GetDisplayUnitAndFormatForECProperty(displayUnit, unusedFmt, storedUnit, *ecprop))
        {
        double displayValue = v.GetDouble();
        if (!displayUnit.IsCompatible (storedUnit) || !storedUnit.ConvertTo (displayValue, displayUnit))
            return false;

        v.SetDouble (displayValue);
        }

    v.SetDouble (v.GetDouble() * getDoubleOrDefault (opts, L"CF", 1.0));
    return DoubleTypeAdapter::ConvertToDisplayType (v, opts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECUnitsTypeAdapter::_ConvertFromString (ECValueR v, WCharCP stringValue, IDgnECTypeAdapterContextCR context) const
    {
    // Note managed implementation expects input value will be in display units - does not check if user provided a different suffix to indicate different units
    ECPropertyCP ecprop = context.GetProperty();
    IECClassLocaterR unitsECClassLocater = context.GetUnitsECClassLocater();
    if (NULL != ecprop && InteropStringFormatter::GetInstance().Parse (v, stringValue, PRIMITIVETYPE_Double))
        {
        Unit storedUnit;
        if (!Unit::GetUnitForECProperty(storedUnit, *ecprop, unitsECClassLocater))
            return false;
        
        WString fmt;
        Unit displayUnit;
        if (Unit::GetDisplayUnitAndFormatForECProperty(displayUnit, fmt, storedUnit, *ecprop, unitsECClassLocater))
            {
            // Convert to storage units
            double value = v.GetDouble();
            if (!storedUnit.IsCompatible (displayUnit) 
                || !displayUnit.ConvertTo (value, storedUnit))
                return false;
            
            v.SetDouble (value);
            }

        // Return as correct primitive type for ECProperty
        return v.ConvertToPrimitiveType (ecprop->GetAsPrimitiveProperty()->GetType());
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* From managed ECUnits: Apps/users can customize unit labels by importing 0 or more
* schemas matching the name "UnitsCustomization###_*" into a dgnfile. The schema contains
* overrides for Units' ShortLabel properties.
* The 3 digits in the schema name indicate priority (lower number == higher priority).
* @bsimethod                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
void ECUnitsTypeAdapter::ApplyUnitLabelCustomization (UnitR unit, DgnProjectR dgnFile)
    {
    BeAssert ("ApplyUnitLabelCustomization has not been implemented in Graphite");
        // TODO: Need to discover schemas from the DgnDb in Graphite. 

    //static const WChar s_prefix[] = L"UnitsCustomization";
    //static const size_t s_prefixLen = _countof(s_prefix) - 1;
    //static const size_t s_underBarPos = s_prefixLen + 3;

    //bvector<SchemaInfo> schemaInfos;
    //bvector<ECSchemaPtr> schemas;

    //DgnECManagerR mgr = DgnECManager::GetManager();
    //mgr.DiscoverSchemas (schemaInfos, dgnFile);

    //size_t i = 0;

    //FOR_EACH (SchemaInfo& schemaInfo, schemaInfos)
    //    {
    //    WCharCP schemaName = schemaInfo.GetSchemaName();
    //    WCharCP underBarPos;
    //    if (schemaName == ::wcsstr (schemaName, s_prefix) && NULL != (underBarPos = ::wcschr (schemaName, '_')) && underBarPos == schemaName + s_underBarPos)
    //        {
    //        ECSchemaPtr schema = mgr.LocateSchemaInDgnFile (schemaInfo, SCHEMAMATCHTYPE_Exact);
    //        if (schema.IsValid())
    //            {
    //            // Insert based on priority
    //            size_t insertPos;
    //            for (insertPos = 0; insertPos < schemas.size(); insertPos++)
    //                {
    //                WCharCP otherSchemaName = schemas[i]->GetName().c_str();
    //                if (0 > wcscmp (schemaName + s_prefixLen, otherSchemaName + s_prefixLen))
    //                    break;
    //                }

    //            schemas.insert (schemas.begin() + insertPos, schema);
    //            }
    //        }
    //    }

    //FOR_EACH (ECSchemaPtr const& schema, schemas)
    //    {
    //    ECClassCP unitClass = schema->GetClassP (unit.GetName());
    //    ECValue v;
    //    IECInstancePtr unitAttr;
    //    if (NULL != unitClass && (unitAttr = unitClass->GetCustomAttribute (L"Unit_Attributes")).IsValid()
    //        && ECOBJECTS_STATUS_Success == unitAttr->GetValue (v, L"ShortLabel") && !v.IsNull())
    //        {
    //        unit.SetShortLabel (v.GetString());
    //        break;
    //        }
    //    }
    }

END_BENTLEY_DGNPLATFORM_NAMESPACE

#undef RSC_STR
