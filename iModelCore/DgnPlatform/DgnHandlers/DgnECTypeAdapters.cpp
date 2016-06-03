/*--------------------------------------------------------------------------------------+
|
|  $Source: DgnHandlers/DgnECTypeAdapters.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>
#include    <DgnPlatform/DgnGeoCoord.h>
#include    "DgnECTypeAdapters.h"
#include    "DgnECInteropStringFormatter.h"

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_DGNPLATFORM

#define RSC_STR(X) DgnCoreL10N::GetString(DgnCoreL10N::ECTYPEADAPTER_##X())

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

static const Utf8CP    s_fmtInteger        = "%d",
                        s_fmtLong           = "%lld",
                        s_fmtDouble         = "%lf";
static const int32_t    USE_ACTIVE          = -1;                   // for formatter instance properties to indicate active settings should be applied.

//  We use this magic internal code USE_ACRES because we don't want
//  to try to squeeze square units into our units system.
static const int32_t USE_ACTIVE_MASTER_UNITS          = -1,
                    USE_ACTIVE_SECONDARY_UNITS       = -2,
                    USE_ACRES                        = -3;

// as defined in Units_Schema.01.00.ecschema.xml
#define BASEUNIT_METER          "METRE"
#define BASEUNIT_METER2         "METRE_SQUARED"
#define BASEUNIT_METER3         "METRE_CUBED"
#define BASEUNIT_RADIAN         "RADIAN"

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
static bool getValueFromFormatter (ECValueR v, IECInstanceCP fmtr, Utf8CP accessString, PrimitiveType type)
    {
    return (NULL != fmtr && ECObjectsStatus::Success == fmtr->GetValue(v, accessString) && !v.IsNull() && v.IsPrimitive() && v.GetPrimitiveType() == type);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
static int32_t getIntOrDefault (IECInstanceCP fmtr, Utf8CP accessString, int32_t def)
    {
    ECValue v;
    return getValueFromFormatter (v, fmtr, accessString, PRIMITIVETYPE_Integer) ? v.GetInteger() : def;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
static double getDoubleOrDefault (IECInstanceCP fmtr, Utf8CP accessString, double def)
    {
    ECValue v;
    return getValueFromFormatter (v, fmtr, accessString, PRIMITIVETYPE_Double) ? v.GetDouble() : def;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String getStringOrDefault (IECInstanceCP opts, Utf8CP accessString, Utf8CP def)
    {
    ECValue v;
    return getValueFromFormatter(v, opts, accessString, PRIMITIVETYPE_String) ? v.GetUtf8CP() : def;
    }

/*---------------------------------------------------------------------------------**//**
* Some boolean properties are declared as integers in the schema.
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
static bool getBoolOrDefault (IECInstanceCP fmtr, Utf8CP accessString, bool def)
    {
    ECValue v;
    if (NULL != fmtr && ECObjectsStatus::Success == fmtr->GetValue(v, accessString) && v.IsPrimitive() && !v.IsNull())
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
static bool getBoolOrDefault (IECInstanceCP fmtr, Utf8CP accessString, bool defIfInstanceNull, bool defIfPropertyNull)
    {
    return NULL == fmtr ? defIfInstanceNull : getBoolOrDefault (fmtr, accessString, defIfPropertyNull);
    }

/////////////////////////////////////////////////////
//          Units Serialization
////////////////////////////////////////////////////

static const Utf8CP    s_unitsAccessors[3][6] =
    {
        { "Units", "Base", "System", "Numerator", "Denominator", "Label" },
        { "MasterUnits", "MasterBase", "MasterSystem", "MasterNumerator", "MasterDenominator", "MasterLabel" },
        { "SecondaryUnits", "SecondaryBase", "SecondarySystem", "SecondaryNumerator", "SecondaryDenominator", "SecondaryLabel" }
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
        if (!visitor.Visit ((int32_t)entry.GetNumber(), entry.GetName().c_str(), unitDef))
            break;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/14
+---------------+---------------+---------------+---------------+---------------+------*/
static void createUnitOptions (UnitIteratorOptionsR opts, IDgnECTypeAdapterContextCP context)
    {
    if (NULL != context)
        {
        ECPropertyCP prop = context->GetProperty();
        IECInstanceCP attr = nullptr != prop ? prop->GetCustomAttribute ("DgnCustomAttributes", "UnitDefinitionExtendedTypeOptions").get() : nullptr;

        if (getBoolOrDefault(attr, "SupportsBaseMeter", true))
            opts.SetAllowAdditionalBase(UnitBase::Meter);
        if (getBoolOrDefault(attr, "SupportsBaseDegree", false))
            opts.SetAllowAdditionalBase(UnitBase::Degree);
        if (getBoolOrDefault(attr, "SupportsBaseNone", false))
            opts.SetAllowAdditionalBase(UnitBase::None);

        if (getBoolOrDefault(attr, "SupportsSystemMetric", true))
            opts.SetAllowAdditionalSystem(UnitSystem::Metric);
        if (getBoolOrDefault(attr, "SupportsSystemEnglish", true))
            opts.SetAllowAdditionalSystem(UnitSystem::English);
        if (getBoolOrDefault(attr, "SupportsSystemUSSurvey", true))
            opts.SetAllowAdditionalSystem(UnitSystem::USSurvey);
        if (getBoolOrDefault(attr, "SupportsSystemUndefined", true))
            opts.SetAllowAdditionalSystem(UnitSystem::Undefined);
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
        int32_t             m_num;
        bool                m_found;

        bool Visit (int32_t number, Utf8CP name, UnitDefinitionCR unitDef)
            {
            if (number == m_num)
                {
                m_found = true;
                return false;
                }
            return true;
            }

        Validater (int32_t num) : m_num(num), m_found(false) { }
        };

    struct ToStringConverter
        {
        int32_t         m_num;
        Utf8StringR        m_name;
        bool            m_found;

        bool Visit (int32_t number, Utf8CP name, UnitDefinitionCR unitDef)
            {
            if (number == m_num)
                {
                m_name = name;
                m_found = true;
                return false;
                }
            return true;
            }

        ToStringConverter(int32_t num, Utf8StringR name) : m_num(num), m_name(name), m_found(false) {}
        };

    struct FirstUnitFinder
        {
        ECValueR        m_value;

        bool Visit(int32_t number, Utf8CP name, UnitDefinitionCR unitDef)
            {
            m_value.SetInteger(number);
            return false;
            }

        FirstUnitFinder(ECValueR v) : m_value(v)
            {
            v.Clear();
            }
        };

    struct StandardValuesCollector
        {
        IDgnECTypeAdapter::StandardValuesCollection&   m_values;

        bool Visit (int32_t number, Utf8CP name, UnitDefinitionCR unitDef)
            {
            m_values.push_back (name);
            return true;
            }

        StandardValuesCollector (IDgnECTypeAdapter::StandardValuesCollection& vals) : m_values (vals) { }
        };

    struct UnitMatcher
        {
        bool            m_ignoreSystem, m_ignoreLabel;
        int32_t         m_base, m_system;
        double          m_num, m_denom;
        Utf8CP         m_label;
        int32_t         m_unitNumber;

        UnitMatcher (int32_t base, int32_t system, double num, double denom, Utf8CP label)
            : m_ignoreSystem(false), m_ignoreLabel(false), m_base(base), m_system(system), m_num(num), m_denom(denom), m_label(label), m_unitNumber(USE_ACTIVE_MASTER_UNITS) { }

        bool            Matched() const         { return USE_ACTIVE_MASTER_UNITS != m_unitNumber; }
        int32_t         GetUnitNumber() const   { return m_unitNumber; }
        void            IgnoreBaseAndSystem()   { m_ignoreSystem = true; }
        void            IgnoreLabel()           { m_ignoreLabel = true; }

        bool            Visit (int32_t number, Utf8CP name, UnitDefinitionCR unitDef)
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
static int32_t findClosestUnits (int32_t base, int32_t system, double num, double denom, Utf8CP label)
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
static void setIntOrNull (IECInstanceR opts, Utf8CP accessString, int32_t actual, int32_t def)
    {
    ECValue v;
    if (actual != def)
        v.SetInteger (actual);

    opts.SetValue (accessString, v);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
static void setDoubleOrNull (IECInstanceR opts, Utf8CP accessString, double actual, double def)
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
    const Utf8CP* accessors = s_unitsAccessors[prefix];
    ECValue v;
    int32_t unitNumber = getIntOrDefault (&fmtr, accessors[Accessor_Units], USE_ACTIVE_MASTER_UNITS);
    UnitDefinition units;
    if (USE_ACRES == unitNumber)
        {
        units = UnitDefinition::GetStandardUnit (StandardUnit::EnglishFeet);
        units.Init (units.GetBase(), units.GetSystem(), units.GetNumerator() / sqrt(43560.0), units.GetDenominator(), "ACRES");    // do not translate
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

    setIntOrNull (fmtr, accessors[Accessor_Base], (int32_t)units.GetBase(), s_DefaultBase);
    setIntOrNull (fmtr, accessors[Accessor_System], (int32_t)units.GetSystem(), s_DefaultSystem);
    setDoubleOrNull (fmtr, accessors[Accessor_Numerator], units.GetNumerator(), s_DefaultNumerator);
    setDoubleOrNull (fmtr, accessors[Accessor_Denominator], units.GetDenominator(), s_DefaultDenominator);
    
    v.SetUtf8CP(units.GetLabelCP());
    fmtr.SetValue (accessors[Accessor_Label], v);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
void TypeAdapterUnitsSerialization::PostProcessUnits (IECInstanceR fmtr, AccessorPrefix prefix)
    {
    const Utf8CP* accessors = s_unitsAccessors[prefix];

    ECValue v;
    if (ECObjectsStatus::Success == fmtr.GetValue (v, accessors[Accessor_Units]) && !v.IsNull())
        {
        if (v.GetInteger() == USE_ACTIVE_MASTER_UNITS || v.GetInteger() == USE_ACTIVE_SECONDARY_UNITS)
            return;
        }

    int32_t base = getIntOrDefault (&fmtr, accessors[Accessor_Base], s_DefaultBase);
    int32_t system = getIntOrDefault (&fmtr, accessors[Accessor_System], s_DefaultSystem);
    double numerator = getDoubleOrDefault (&fmtr, accessors[Accessor_Numerator], (double)s_DefaultNumerator);
    double denominator = getDoubleOrDefault (&fmtr, accessors[Accessor_Denominator], (double)s_DefaultDenominator);
    Utf8String label = getStringOrDefault (&fmtr, accessors[Accessor_Label], "");

    int32_t unitNumber = label.Equals ("ACRES") ? USE_ACRES : findClosestUnits (base, system, numerator, denominator, label.c_str());

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
//          String case conversion
////////////////////////////////////////////////////

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
static void makeUpper (Utf8StringR str)
    {
    for (size_t i = 0; i < str.length(); i++)
        if (islower (str[i]))
            str[i] = (char) toupper (str[i]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
static void makeLower (Utf8StringR str)
    {
    for (size_t i = 0; i < str.length(); i++)
        if (isupper (str[i]))
            str[i] = (char) tolower(str[i]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
static void capFirst (Utf8StringR str)
    {
    size_t i = 0;
    for ( ; i < str.length(); i++)
        {
        Utf8Char ch = str[i];
        if (!isspace (ch) && !ispunct (ch))
            {
            str[i] = (char) toupper(ch);
            break;
            }
        }

    for (i++; i < str.length(); i++)
        if (isupper (str[i]))
            str[i] = (char) tolower(str[i]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
static size_t makeWordTitleCase (Utf8StringR str, size_t i)
    {
    // skip white/punct
    for ( ; i < str.length(); i++)
        {
        if (!ispunct (str[i]) && !isspace (str[i]))
            break;
        }

    // check if all-caps; if so don't modify
    bool foundLower = false;
    size_t start = i;
    for ( ; i < str.length(); i++)
        {
        if (!foundLower && islower (str[i]))
            foundLower = true;
        else if (ispunct (str[i]) || isspace (str[i]))
            break;
        }
        
    size_t end = i;
    if (foundLower)
        {
        // make title case
        if (start < end && islower (str[start]))
            str[start] = (char) toupper (str[start]);

        for (i = start+1; i < end; i++)
            if (isupper (str[i]))
                str[i] = (char) tolower(str[i]);
        }

    return end;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
static void makeTitleCase (Utf8StringR str)
    {
    // Title Case:
    //  "ABC DEF GHI" => "Abc Def Ghi"  <-- if all chars in string uppercase, perform normal title case
    //  "ABC def gHi" => "ABC Def Ghi"  <-- if all chars in a single word uppercase, leave that word alone

    bool foundLower = false;
    for (size_t cur = 0; cur < str.length(); cur++)
        {
        if (islower (str[cur]))
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
void StringFormatTypeAdapter::ConvertCase (Utf8StringR str, IECInstanceCP fmtr)
    {
    int32_t caseSpec = getIntOrDefault (fmtr, "Case", CaseSpec_None);
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
bool StringFormatTypeAdapter::_ConvertToString (Utf8StringR str, ECValueCR v, IDgnECTypeAdapterContextCR context, IECInstanceCP fmtr) const
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
    opts.SetValue ("Case", ECValue ((int32_t)CaseSpec_None));
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
Utf8CP BooleanFormatTypeAdapter::GetResourceString (BoolWord type, bool value) const
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
bool BooleanFormatTypeAdapter::_ConvertToString (Utf8StringR str, ECValueCR v, IDgnECTypeAdapterContextCR context, IECInstanceCP fmtr) const
    {
    if (v.IsNull())
        {
        str = GetParensNullString();
        return true;
        }
    else if (!v.IsBoolean())
        return false;

    BoolWord boolWord = (BoolWord)getIntOrDefault (fmtr, "BoolWord", BoolWord_Default);
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
bool BooleanFormatTypeAdapter::_ConvertFromString (ECValueR v, Utf8CP str, IDgnECTypeAdapterContextCR context) const
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
    Utf8String str;
    if (!ConvertToString (str, v, context, opts))
        return false;

    v.SetUtf8CP (str.c_str());
    return true;
    }


///////////////////////////////////
//      Boolean
//////////////////////////////////

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String BooleanTypeAdapter::GetBooleanString (bool b, IDgnECTypeAdapterContextCR) const
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
bool DoubleTypeAdapter::_ConvertFromString (ECValueR v, Utf8CP str, IDgnECTypeAdapterContextCR context) const
    {
    double d;
    if (1 == BE_STRING_UTILITIES_UTF8_SSCANF(str, s_fmtDouble, &d))
        {
        v.SetDouble (d);
        return true;
        }
    
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool DoubleTypeAdapter::_ConvertToString (Utf8StringR str, ECValueCR v, IDgnECTypeAdapterContextCR context, IECInstanceCP opts) const
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
    double conversionFactor = getDoubleOrDefault (opts, "CF", 1.0);
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
    double conversionFactor = getDoubleOrDefault (opts, "CF", 1.0);
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

    int32_t nDecimalPlaces = 4;
    int32_t accuracy = getIntOrDefault (opts, "Accuracy", USE_ACTIVE);
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
    v.SetInteger (USE_ACTIVE);      opts.SetValue ("Accuracy",        v);
    v.SetUtf8CP("");      opts.SetValue("PX", v);
                            opts.SetValue ("SX",              v);
    v.SetInteger ('.');     opts.SetValue ("DecimalSeparator",v);
    v.SetInteger (1);       opts.SetValue ("LeadingZero",     v);
                            opts.SetValue ("TrailingZeros",   v);

    // Area, Coordinate, Double, and Volume support TS
    if (ECObjectsStatus::Success == opts.GetValue (v, "TS"))
        { v.SetInteger (0); opts.SetValue ("TS", v); }                // 'none'

    // Area, Volume, and Double support CF
    if (ECObjectsStatus::Success == opts.GetValue (v, "CF"))
        { v.SetDouble (1.0);    opts.SetValue ("CF", v); }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8Char getSeparatorOrDefault(IECInstanceCP opts, Utf8CP accessString, Utf8Char def)
    {
    int32_t i = getIntOrDefault(opts, accessString, 0);
    switch (i)
        {
        // known supported separators
        case '.':
        case ',':
        case ' ':
        case '/':
        case ';':
            def = (Utf8Char) i;
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
    Utf8Char thousSep = getSeparatorOrDefault (opts, "TS", 0);
    if (0 != thousSep)
        {
        fmtr.SetInsertThousandsSeparator (true);
        fmtr.SetThousandsSeparator (thousSep);
        }

    Utf8Char decSep = getSeparatorOrDefault(opts, "DecimalSeparator", '.');
    fmtr.SetDecimalSeparator (decSep);

    fmtr.SetLeadingZero (getBoolOrDefault (opts, "LeadingZero", true));
    fmtr.SetTrailingZeros (getBoolOrDefault (opts, "TrailingZeros", false));

    int32_t accuracy = getIntOrDefault (opts, "Accuracy", USE_ACTIVE);
    if (0 <= accuracy)
        {
        //  accu             prec
        //  0..8            100..108        decimal
        //  10..18          200..208        fractional
        //  20..28          300..308        scientific
        int32_t precInt = 100 * (accuracy/10) + 100;
        precInt += (accuracy - (accuracy/10));
        fmtr.SetPrecision ((PrecisionFormat)precInt);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DoubleTypeAdapter::ApplyPrefixSuffix (Utf8StringR str, IECInstanceCP opts)
    {
    Utf8String prefix = getStringOrDefault (opts, "PX", "");
    if (!prefix.empty())
        str.insert (0, prefix);

    Utf8String suffix = getStringOrDefault (opts, "SX", "");
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
bool IntegerTypeAdapter::_ConvertFromString (ECValueR v, Utf8CP str, IDgnECTypeAdapterContextCR context) const
    {
    int32_t i;
    if (1 == BE_STRING_UTILITIES_UTF8_SSCANF(str, s_fmtInteger, &i))
        {
        v.SetInteger (i);
        return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool IntegerTypeAdapter::_ConvertToString (Utf8StringR str, ECValueCR v, IDgnECTypeAdapterContextCR context, IECInstanceCP opts) const
    {
    str.clear();
    if (v.IsNull())
        {
        str = GetParensNullString();
        return true;
        }
    else if (!v.IsInteger())
        return false;

    Utf8String fmt;
    ExtractFormatString (fmt, opts);
    str.Sprintf (fmt.c_str(), v.GetInteger());
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* Returns a native format string.
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void IntegerTypeAdapter::ExtractFormatString (Utf8StringR fmt, IECInstanceCP opts) const
    {
    if (NULL == opts)
        fmt = s_fmtInteger;
    else
        {
        int32_t minWidth = getIntOrDefault (opts, "MinimumWidth", 1);
        if (1 == minWidth)
            fmt = s_fmtInteger;     // other properties can have no effect
        else
            {
            bool rightAligned = getBoolOrDefault (opts, "RightAligned", true);
            bool leadingZeros = getBoolOrDefault (opts, "LeadingZeros", false);

            Utf8CP widthFmt = "%%%dd";
            if (!rightAligned)
                widthFmt = "%%-%dd";
            else if (leadingZeros)
                widthFmt = "%%0%dd";

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
bool StringTypeAdapter::_ConvertFromString (ECValueR v, Utf8CP str, IDgnECTypeAdapterContextCR context) const
    {
    if (NULL != str)
        v.SetUtf8CP(str);
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
bool StringTypeAdapter::GetUnformattedStringValue (Utf8StringR str, ECValueCR v, IDgnECTypeAdapterContextCR context) const
    {
    if (v.IsNull())
        str = GetParensNullString();
    else if (!v.IsString())
        return false;
    else
        str = v.GetUtf8CP();

    return true;
    }

//////////////////////////////////////////////////////
//          DateTime
/////////////////////////////////////////////////////

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct DateTimeSettings : DgnHost::IHostObject
    {
    struct StandardFormat
        {
        Utf8String         name;
        Utf8String         format;
        StandardFormat (Utf8CP n, Utf8CP f) : name(n), format(f) { }
        };
    typedef bvector<StandardFormat>             FormatList;
private:
    DateTimeSettings() { }

    static const int64_t TICKADJUSTMENT = 504911232000000000LL;     // ticks between 01/01/01 and 01/01/1601
    static const uint64_t SAMPLETIME    = 1126120082463LL;

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
    typedef bmap<Utf8Char, PartPair>               InstructionFormatConversionMap;
    typedef bmap<Utf8String, DateTimeFormatPart>   DotNetConversionMap;
    
    InstructionFormatConversionMap  m_instructionMap;
    DotNetConversionMap             m_dotNetMap;
    FormatList                      m_standardDateTimeFormats;      // from config file

    void                            PopulateConversionMaps();
    void                            PopulateStandardFormats();
    static bool                     ReadFormatsFromXml (bvector<Utf8String>& formats, bvector<Utf8String>& descriptions);
    static void                     GetDefaultFormats (bvector<Utf8String>& formats);
public:
    DateTimeFormatPart              ConvertInstructionSpecifier (Utf8Char spec, bool useAlternate) const;      // Returns DATETIME_PART_END if not found
    DateTimeFormatPart              ConvertDotNetSpecifier (Utf8StringCR spec) const;                          // Returns DATETIME_PART_END if not found
    FormatList const&               GetStandardFormats() const   { return m_standardDateTimeFormats; }
    void                            AddStandardFormat (Utf8CP fmt);

    static DateTime                 GetSampleTime()
        {
        static const uint64_t SAMPLETIME = 1126120082463LL;
        DateTime dt;
        DateTime::FromUnixMilliseconds(dt, SAMPLETIME);
        return dt;
        }

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
DateTimeFormatPart DateTimeSettings::ConvertInstructionSpecifier (Utf8Char spec, bool useAlternate) const
    {
    InstructionFormatConversionMap::const_iterator iter = m_instructionMap.find (spec);
    if (iter != m_instructionMap.end())
        return useAlternate ? iter->second.alternatePart : iter->second.primaryPart;

    return DATETIME_PART_END;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
DateTimeFormatPart DateTimeSettings::ConvertDotNetSpecifier (Utf8StringCR spec) const
    {
    DotNetConversionMap::const_iterator iter = m_dotNetMap.find (spec);
    return iter != m_dotNetMap.end() ? iter->second : DATETIME_PART_END;
    }

/*---------------------------------------------------------------------------------**//*
* @bsimethod                                                    Paul.Connelly   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool DateTimeSettings::ReadFormatsFromXml (bvector<Utf8String>& formats, bvector<Utf8String>& descriptions)
    {
#if defined (WIP_CFGVAR) // MS_DATETIMEFORMATS
    Utf8String     xmlPath;
    BeXmlDomPtr xmlDom;
    BeXmlStatus xmlStatus;
    if (SUCCESS != ConfigurationManager::GetVariable (xmlPath, "MS_DATETIMEFORMATS") || (xmlDom = BeXmlDom::CreateAndReadFromFile (xmlStatus, xmlPath.c_str())).IsNull())
        return false;

    BeXmlDom::IterableNodeSet nodes;
    xmlDom->SelectNodes (nodes, "/DateTimeFormats/SupportedFormats/Format", NULL);
    FOR_EACH (BeXmlNodeP node, nodes)
        {
        Utf8String fmt, desc;
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
void DateTimeSettings::GetDefaultFormats (bvector<Utf8String>& formats)
    {
    static const Utf8String s_defaults[] = { "%c", "%#c", "%x", "%#x", "dddd, MMMM dd, yyyy", "HH:mm:ss", "HH:mm", "h:mm:ss tt", "h:mm tt",
        "M/d/yyyy h:mm:ss tt", "M/d/yyyy h:mm tt", "MMM-yy", "MMMM yy", "yyyy-M-d", "yyyy/MM/dd", "dd/MM/yyyy", "dd.MM.yyyy", "d MMMM yyyy",
        "MMM. d, yy", "d-MMM-yy", "yyyy-MM-dd", "M/d/yy", "MMMM d, yyyy", "M/d/yyyy", "M.d.yyyy", "%X"
        };

    formats.insert (formats.begin(), s_defaults, s_defaults + _countof (s_defaults));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DateTimeSettings::PopulateStandardFormats()
    {
    bvector<Utf8String> formats, descriptions;
    if (!ReadFormatsFromXml (formats, descriptions))
        GetDefaultFormats (formats);

    // Sample time 09/07/2005 15:08:02.463 chosen so 24-hour clock, leading zeros, etc are obvious
    
    for (size_t i = 0; i < formats.size(); i++)
        {
        // use the format string to format the sample time
        Utf8String formattedSampleTime;
        DateTime sampleDT;
        DateTime::FromUnixMilliseconds(sampleDT, SAMPLETIME);
        DateTimeTypeAdapter::FormatDateTime(formattedSampleTime, formats[i], sampleDT);

        Utf8String name = formattedSampleTime;
        if (0 != descriptions.size() && 0 < descriptions[i].length())
            {
            // append the description in parentheses
            name.append (" (");
            name.append (descriptions[i]);
            name.append (")");
            }

        m_standardDateTimeFormats.push_back (StandardFormat (name.c_str(), formats[i].c_str()));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DateTimeSettings::AddStandardFormat (Utf8CP fmt)
    {
    Utf8String formatted;
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
    m_dotNetMap["d"] = DATETIME_PART_D;
    m_dotNetMap["dd"] = DATETIME_PART_DD;
    m_dotNetMap["ddd"] = DATETIME_PART_DoW;
    m_dotNetMap["dddd"] = DATETIME_PART_DayOfWeek;
    // ffffff special cases
    // g, gg for era - not supported (times B.C. not supported....)
    m_dotNetMap["h"] = DATETIME_PART_h;
    m_dotNetMap["hh"] = DATETIME_PART_hh;
    m_dotNetMap["H"] = DATETIME_PART_H;
    m_dotNetMap["HH"] = DATETIME_PART_HH;
    m_dotNetMap["m"] = DATETIME_PART_m;
    m_dotNetMap["mm"] = DATETIME_PART_mm;
    m_dotNetMap["M"] = DATETIME_PART_M;
    m_dotNetMap["MM"] = DATETIME_PART_MM;
    m_dotNetMap["MMM"] = DATETIME_PART_Mon;
    m_dotNetMap["MMMM"] = DATETIME_PART_Month;
    m_dotNetMap["s"] = DATETIME_PART_s;
    m_dotNetMap["ss"] = DATETIME_PART_ss;
    m_dotNetMap["t"] = DATETIME_PART_AP;
    m_dotNetMap["tt"] = DATETIME_PART_AMPM;
    m_dotNetMap["y"] = DATETIME_PART_Y;
    m_dotNetMap["yy"] = DATETIME_PART_YY;
    m_dotNetMap["yyy"] = DATETIME_PART_YYY;
    m_dotNetMap["yyyy"] = DATETIME_PART_YYYY;
    m_dotNetMap["yyyyy"] = DATETIME_PART_YYYYY;
    m_dotNetMap["z"] = DATETIME_PART_U;
    m_dotNetMap["zz"] = DATETIME_PART_UU;
    m_dotNetMap["zzz"] = DATETIME_PART_UU_UU;
    m_dotNetMap[":"] = DATETIME_PART_TimeSeparator;
    m_dotNetMap["/"] = DATETIME_PART_DateSeparator;
    m_dotNetMap["."] = DATETIME_PART_DecimalSeparator;
    m_dotNetMap[","] = DATETIME_PART_Comma;
    m_dotNetMap[" "] = DATETIME_PART_Space;
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
bool DateTimeTypeAdapter::_ConvertToString (Utf8StringR str, ECValueCR v, IDgnECTypeAdapterContextCR context, IECInstanceCP opts) const
    {
    str.clear();
    if (v.IsNull())
        str = GetParensNullString();
    else if (!v.IsDateTime())
        return false;
    else
        {
        Utf8String fmt = getStringOrDefault (opts, "DTFmtStr", "");
        FormatDateTime(str, fmt, v.GetDateTime());
        }

    StringFormatTypeAdapter::ConvertCase (str, opts);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DateTimeTypeAdapter::FormatDateTime (Utf8StringR str, Utf8StringCR fmt, DateTimeCR dt)
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
void DateTimeTypeAdapter::FormatWithInstructions(Utf8StringR str, Utf8StringCR fmt, DateTimeCR dt)
    {
    DateTimeFormatterPtr fmtr = DateTimeFormatter::Create();
    fmtr->SetConvertToLocalTime (false);
    size_t len = fmt.length();
    DateTimeSettings const& dtSettings = DateTimeSettings::GetSettings();
    for (size_t index = 0; index < len; index++)
        {
        Utf8Char c = fmt[index];
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
Utf8String extractLiteralString (Utf8StringCR fmt, Utf8Char quoteChar, size_t quoteStart)
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
void DateTimeTypeAdapter::FormatWithPicture(Utf8StringR str, Utf8StringCR fmt, DateTimeCR dt)
    {
    DateTimeFormatterPtr fmtr = DateTimeFormatter::Create();
    fmtr->SetConvertToLocalTime (false);
    size_t len = fmt.length();
    DateTimeSettings const& dtSettings = DateTimeSettings::GetSettings();

    for (size_t index = 0; index < len; index++)
        {
        Utf8Char c = fmt[index];
        if ('\\' == c)                   // escape character - treat next as literal
            {
            if (++index < len)
                str.append (1, fmt[index]);
            }
        else if ('"' == c || '\'' == c)
            {
            Utf8String literalString = extractLiteralString (fmt, c, index);
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

            Utf8String sequence (matchEnd-index, c);
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
                    fmtr->SetFractionalSecondPrecision ((uint8_t)sequence.length());
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
bool DateTimeFormatTypeAdapter::_ConvertFromString (ECValueR v, Utf8CP str, IDgnECTypeAdapterContextCR) const
    {
    DateTimeSettings::FormatList const& fmts = DateTimeSettings::GetSettings().GetStandardFormats();
    for (size_t i = 0; i < fmts.size(); i++)
        {
        if (fmts[i].name.Equals (str))
            {
            v.SetUtf8CP(fmts[i].format.c_str());
            return true;
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool DateTimeFormatTypeAdapter::_ConvertToString (Utf8StringR str, ECValueCR v, IDgnECTypeAdapterContextCR, IECInstanceCP opts) const
    {
    if (v.IsString() && !v.IsNull())
        {
        Utf8CP fmt = v.GetUtf8CP();
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
int32_t DateTimeFormatTypeAdapter::LookupFormatIndex (Utf8CP formatString)
    {
    DateTimeSettings::FormatList const& fmts = DateTimeSettings::GetSettings().GetStandardFormats();
    for (size_t i = 0; i < fmts.size(); i++)
        {
        DateTimeSettings::StandardFormat const& fmt = fmts[i];
        if (fmt.format.Equals (formatString))
            return (int32_t)i;
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
bool StructTypeAdapter::_ConvertToString (Utf8StringR str, ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP formatter) const
    {
    // we could return the instance ID, but would that really be useful?
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool StructTypeAdapter::_ConvertFromString (ECValueR v, Utf8CP str, IDgnECTypeAdapterContextCR context) const
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
    m_instance = prop.GetCustomAttribute ("StandardValues");
    if (m_instance.IsValid())
        {
        ECValue v;
        if (ECObjectsStatus::Success == m_instance->GetValue (v, "ValueMap") && v.IsArray() && !v.IsNull())
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
    if (ECObjectsStatus::Success == m_instance->GetValue (v, "ValueMap", m_currentIndex) && v.IsStruct() && !v.IsNull())
        {
        IECInstancePtr structInstance = v.GetStruct();
        if (ECObjectsStatus::Success == structInstance->GetValue (v, "DisplayString"))
            {
            m_name = v.IsNull() ? "" : v.GetUtf8CP();
            if (ECObjectsStatus::Success == structInstance->GetValue (v, "Value"))
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
        int32_t intVal = v.GetInteger();
        for (ValueMapIterator iter (*context.GetProperty()); !iter.IsEnd(); iter.MoveNext())
            if (iter.GetValue() == intVal)
                return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool StandardValuesTypeAdapter::GetUnformattedStringValue (Utf8StringR valueAsString, ECValueCR v, IDgnECTypeAdapterContextCR context) const
    {
    if (PRIMITIVETYPE_Integer == v.GetPrimitiveType())
        {
        int32_t intVal = v.GetInteger();
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
bool StandardValuesTypeAdapter::_ConvertFromString (ECValueR v, Utf8CP stringVal, IDgnECTypeAdapterContextCR context) const
    {
    for (ValueMapIterator iter (*context.GetProperty()); !iter.IsEnd(); iter.MoveNext())
        {
        if (0 == strcmp (stringVal, iter.GetName()))
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool StandardValuesTypeAdapter::_GetPlaceholderValue (ECValueR v, IDgnECTypeAdapterContextCR context) const
    {
    auto prop = context.GetProperty();
    if (nullptr != prop)
        {
        ValueMapIterator iter (*prop);
        if (!iter.IsEnd())
            {
            v.SetInteger (iter.GetValue());
            return true;
            }
        }

    return false;
    }

///////////////////////////////////////////
//  BooleanDisplayTypeAdapter
//////////////////////////////////////////

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr BooleanDisplayTypeAdapter::GetCustomAttributeInstance (IDgnECTypeAdapterContextCR context) const
    {
    return context.GetProperty()->GetCustomAttribute ("BooleanDisplay");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String BooleanDisplayTypeAdapter::GetBooleanString (bool boolVal, IDgnECTypeAdapterContextCR context) const
    {
    IECInstancePtr customAttr;
    if ((customAttr = GetCustomAttributeInstance (context)).IsValid())
        {
        ECValue v;
        if (ECObjectsStatus::Success == customAttr->GetValue (v, boolVal ? "TrueString" : "FalseString"))
            return v.GetUtf8CP();
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
        if (ECObjectsStatus::Success != customAttr->GetValue (v, "TrueString"))
            { BeAssert (false); return false; }

        values.push_back(v.GetUtf8CP());
        if (ECObjectsStatus::Success != customAttr->GetValue (v, "FalseString"))
            { BeAssert (false); return false; }

        values.push_back(v.GetUtf8CP());

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
bool PrimitiveAdapterBase::_ConvertToString (Utf8StringR str, ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP formatter) const
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
bool MissingExtendTypeAdapter::_ConvertToString (Utf8StringR str, ECValueCR v, IDgnECTypeAdapterContextCR context, IECInstanceCP opts) const
    {
    // without access to the real type adapter, we can't provide much formatting.
    str = v.IsNull() ? GetParensNullString() : v.ToString();
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool LongTypeAdapter::_ConvertFromString (ECValueR v, Utf8CP str, IDgnECTypeAdapterContextCR context) const
    {
    int64_t l;
    if (1 == BE_STRING_UTILITIES_UTF8_SSCANF(str, s_fmtLong, &l))
        {
        v.SetLong (l);
        return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool BinaryTypeAdapter::_ConvertFromString (ECValueR v, Utf8CP str, IDgnECTypeAdapterContextCR context) const
    {
    return false;
    }

////////////////////////////////////////////////////////////////////////////
//  UnitlessPointTypeAdapter
//  Managed code can supply a coordinate index in the context for these
//  methods. The ECValue should always be a 3D point, though.
////////////////////////////////////////////////////////////////////////////

#ifdef UNUSED_FUNCTION
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
static double& getPointComponent (DPoint3dR point, uint32_t index)
    {
    switch (index)
        {
    case 0:
        return point.x;
    case 1:
        return point.y;
    case 2:
        return point.z;
    default:
        BeAssert (false);
        return point.x;
        }
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool PointFormatTypeAdapter::_GetPlaceholderValue (ECValueR v, IDgnECTypeAdapterContextCR context) const
    {
    if (_Is2d())
        v.SetPoint2D (DPoint2d::From (12.34, 56.78));
    else
        v.SetPoint3D (DPoint3d::FromXYZ (12.34, 56.78, 0.9));

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool PointFormatTypeAdapter::_Validate (ECValueCR v, IDgnECTypeAdapterContextCR context) const
    {
    if (-1 == context.GetComponentIndex())
        {
        if (v.IsDouble())
            return _AllowDoubleType();
        else if (v.IsPoint2D())
            return _Is2d();
        else
            return v.IsPoint3D() && !_Is2d();
        }
    else
        {
        return v.IsDouble() && (context.GetComponentIndex() < 2 || !_Is2d());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
ECValue PointFormatTypeAdapter::FromStorageType (ECValueCR in) const
    {
    ECValue out = in;
    if (in.IsPoint2D())
        out.SetPoint3D (DPoint3d::FromXYZ (in.GetPoint2D().x, in.GetPoint2D().y, 0.0));

    return out;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
ECValue PointFormatTypeAdapter::ToStorageType (ECValueCR in) const
    {
    ECValue out = in;
    if (in.IsPoint3D() && _Is2d())
        out.SetPoint2D (DPoint2d::From (in.GetPoint3D().x, in.GetPoint3D().y));

    return out;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool PointFormatTypeAdapter::ExtractOptions(DPoint3d& pt, bool& useX, bool& useY, bool& useZ, Utf8Char& separator, ECValueCR inputV, ECN::IECInstanceCP opts, IDgnECTypeAdapterContextCR context) const
    {
    ECValue v = FromStorageType (inputV);

    uint32_t componentIndex = context.GetComponentIndex();
    if (componentIndex == -1 && v.IsDouble() && _AllowDoubleType())
        {
        // dumb special case
        componentIndex = 2;
        }

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
    case (uint32_t)-1:
        {
        int32_t coordOpts = getIntOrDefault (opts, "Coordinates", 0);
        if (coordOpts == 0)     // default
            coordOpts = 0xFFFFFFFF;

        useX = 0 != (coordOpts & 1);
        useY = 0 != (coordOpts & 2);
        useZ = 0 != (coordOpts & 4) && !_Is2d();
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

    separator = getSeparatorOrDefault (opts, "LS", L',');

    if (UseGlobalOrigin())
        {
        DgnModelP model = context.GetDgnModel();
        if (model)
            pt.Subtract (model->GetDgnDb().Units().GetGlobalOrigin());
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void PointFormatTypeAdapter::InitOptions (IECInstanceR opts) const
    {
    ECValue v;
    v.SetInteger ((int32_t)L',');     opts.SetValue ("LS",           v);
    v.SetInteger (0);               opts.SetValue ("Coordinates",  v);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool PointFormatTypeAdapter::Parse (ECValueR v, Utf8CP str, IDgnECTypeAdapterContextCR context) const
    {
    if (context.GetComponentIndex() != -1)
        {
        double d;
        if (1 == sscanf (str, s_fmtDouble, &d))
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
        v = ToStorageType (v);
        return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool UnitlessPointTypeAdapter::_ConvertToString (Utf8StringR str, ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP formatter) const
    {
    str.clear();
    DoubleFormatterPtr fmtr = DoubleFormatter::Create();
    DoubleTypeAdapter::InitFormatter (*fmtr, formatter);
    return Format (str, v, *fmtr, context, formatter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool UnitlessPointTypeAdapter::_ConvertFromString (ECValueR v, Utf8CP str, IDgnECTypeAdapterContextCR context) const
    {
    return Parse (v, str, context);
    }

/////////////////////////////////
//  FileSizeTypeAdapter
////////////////////////////////

static const int64_t    s_kilobyte          = 0x400;
static const int64_t    s_megabyte          = 0x100000;
static const int64_t    s_gigabyte          = 0x40000000;


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
int32_t FileSizeTypeAdapter::LookupFormatIndex (Utf8CP formatString)
    {
    if (NULL == formatString || 0 == strlen (formatString))
        return 0;
    else if (0 == strcmp (formatString, "%ld%by1"))
        return 1;
    else if (0 == strcmp(formatString, "%.2f%by2"))
        return 2;
    else if (0 == strcmp(formatString, "%.2f%by3"))
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
bool FileSizeTypeAdapter::_ConvertToString (Utf8StringR strVal, ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP formatter) const
    {
    if (!v.IsLong())
        return false;

    int64_t nBytes = v.GetLong();
    double nKb   = ((double)nBytes) / (double)s_kilobyte;

    InteropStringFormatter::GetInstance().FormatValue (strVal, "N0", ECValue (nKb));  // integer format with thousands separators
    strVal.append (1, ' ');
    strVal.append (GetKBString());
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool FileSizeTypeAdapter::_ConvertFromString (ECValueR v, Utf8CP str, IDgnECTypeAdapterContextCR context) const
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

#define RMAXUI4 4294967295.0

/*---------------------------------------------------------------------------------**//**
* Produces a string like "40.50 KB (41,472 bytes)"
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool FileSizeLongTypeAdapter::_ConvertToString (Utf8StringR strVal, ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP formatter) const
    {
    if (!v.IsLong())
        return false;

    int64_t nBytes = v.GetLong();
    Utf8CP units;
    Utf8CP fmt = "N2";
    double size = (double)nBytes;

    if (nBytes < s_kilobyte)
        {
        units = m_BytesString.c_str();
        fmt = "N0";
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
    Utf8String bytesString;
    fmtr.FormatValue (strVal, fmt, ECValue (size));
    strVal.append (1, ' ');
    strVal.append (units);
    strVal.append (" (");
    fmtr.FormatValue (bytesString, RMAXUI4 > (double)nBytes ? "N0" : "E12", ECValue (nBytes));    // DoubleFormatter will convert to scientific notation anyway if nBytes exceeds RMAXUI4, so increase precision
    strVal.append (bytesString);
    strVal.append (1, ' ');
    strVal.append (m_BytesString);
    strVal.append (1, ')');
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool FileSizeLongTypeAdapter::_ConvertFromString (ECValueR v, Utf8CP strVal, IDgnECTypeAdapterContextCR context) const
    {
    Utf8CP byteStr = ::strchr (strVal, '(');
    if (NULL != byteStr)
        {
        Utf8String toParse (byteStr + 1);
        if (Utf8String::npos == toParse.find ('.'))
            {
            toParse.erase(std::remove(toParse.begin(), toParse.end(), ','), toParse.end());

            int64_t nBytes;
            if (1 == BE_STRING_UTILITIES_UTF8_SSCANF(toParse.c_str(), "%lld", &nBytes))
                {
                v.SetLong (nBytes);
                return true;
                }
            }
        else
            {
            double nBytes;
            if (1 == BE_STRING_UTILITIES_UTF8_SSCANF(toParse.c_str(), "%lf", &nBytes))
                {
                v.SetLong ((int64_t)nBytes);
                return true;
                }
            }
        }

    return false;
    }

////////////////////////////////////////////////
//  Units-based adapters.
//  ModelRef optional; if supplied, units are
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
    opts.SetValue ("Format", ECValue ((int32_t)AngleFormatVals::Active));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void AngleFormatTypeAdapter::InitFormatter (AngleFormatter& fmtr, IECInstanceCP opts)
    {
    bool leadingZero = getBoolOrDefault (opts, "LeadingZero", true);
    bool trailingZeros = getBoolOrDefault (opts, "TrailingZeros", true);
   //  Utf8Char decSep = getSeparatorOrDefault (opts, "DecimalSeparator", '.'); ###TODO: AngleFormatter does not support custom decimal separator...

    int32_t angleFormat = getIntOrDefault (opts, "Format", (int32_t)AngleFormatVals::Active);
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

    int32_t accuracy = getIntOrDefault (opts, "Accuracy", USE_ACTIVE);
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
    int32_t nDecimalPlaces = 4;

    GeometricModelCP model = context.GetDgnModel() ? context.GetDgnModel()->ToGeometricModel() : nullptr;
    if (model)
        {
        mode = model->GetDisplayInfo().GetAngularMode();
        nDecimalPlaces = (int32_t) model->GetDisplayInfo().GetAngularPrecision();
        }
    
    // DirectionFormatter doesn't have the "Format" property. Will use degrees by default.
    switch (getIntOrDefault (opts, "Format", (int32_t)AngleFormatVals::Active))
        {
    case USE_ACTIVE:
        break;
    case (int32_t) AngleFormatVals::Centesimal:
        mode = AngleMode::Centesimal;
        break;
    case (int32_t) AngleFormatVals::Radians:
        mode = AngleMode::Radians;
        break;
    default:
        mode = AngleMode::Degrees;
        break;
        }

    int32_t accuracy = getIntOrDefault (opts, "Accuracy", USE_ACTIVE);
    if (0 <= accuracy)
        nDecimalPlaces = accuracy;

    double angle = v.GetDouble();
    angle *= msGeomConst_degreesPerRadian;
    bool negative = false;
    if (getBoolOrDefault (opts, "AllowNegative", true) && angle < 0.0)
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
bool AngleTypeAdapter::_ConvertToString (Utf8StringR str, ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP opts) const
    {
    if (v.IsNull())
        str = GetParensNullString();
    else if (!v.IsDouble())
        return false;
    else
        {
        GeometricModelCP model = context.GetDgnModel() ? context.GetDgnModel()->ToGeometricModel() : nullptr;
        AngleFormatterPtr fmtr = model ? AngleFormatter::Create(*model) : AngleFormatter::Create();
        InitFormatter (*fmtr, opts);

        fmtr->SetAllowNegative (getBoolOrDefault (opts, "AllowNegative", true));
        str = fmtr->ToString (v.GetDouble());

        int32_t angleFormat = getIntOrDefault (opts, "Format", (int32_t)AngleFormatVals::Active);
#if defined (_MSC_VER)
    #pragma warning (disable : 4428)   // Workaround incorrect -W4 compiler warning - http://msdn.microsoft.com/en-us/library/ttw8abkd.aspx
#endif // defined (_MSC_VER)
        // Cannot compare character values over 127 on a UTF-8 encoded string.
        // It should be sufficient to use a WString here; the value in question will fit in all known WString encodings,
        //  and if it does match, interchanging str and strW for purposes of using the last character should also be safe.
        WString strW(str.c_str(), BentleyCharEncoding::Utf8);
        if (-2 == angleFormat && !strW.empty() && L'\u00B0' == strW[strW.length()-1])
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
bool AngleTypeAdapter::_ConvertFromString (ECValueR v, Utf8CP str, IDgnECTypeAdapterContextCR context) const
    {
    AngleParserPtr parser = AngleParser::Create();
    GeometricModelCP model = context.GetDgnModel() ? context.GetDgnModel()->ToGeometricModel() : nullptr;
    if (model)
        parser->SetAngleMode(model->GetDisplayInfo().GetAngularMode());

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
bool DirectionAngleTypeAdapter::_ConvertToString (Utf8StringR str, ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP opts) const
    {
    if (v.IsNull())
        str = GetParensNullString();
    else if (!v.IsDouble())
        return false;
    else
        {
        GeometricModelCP model = context.GetDgnModel() ? context.GetDgnModel()->ToGeometricModel() : nullptr;
        DirectionFormatterPtr fmtr = model ? DirectionFormatter::Create(*model) : DirectionFormatter::Create();
        InitFormatter (fmtr->GetAngleFormatter(), opts);

        fmtr->SetAddTrueNorth (false);
        int32_t mode = getIntOrDefault (opts, "Mode", USE_ACTIVE);
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
bool DirectionAngleTypeAdapter::_ConvertFromString (ECValueR v, Utf8CP str, IDgnECTypeAdapterContextCR context) const
    {
    DirectionParserPtr parser = DirectionParser::Create();

    GeometricModelCP model = context.GetDgnModel() ? context.GetDgnModel()->ToGeometricModel() : nullptr;
    if (model)
        {
        GeometricModel::DisplayInfo const& displayInfo = model->GetDisplayInfo();

        parser->SetDirectionMode(displayInfo.GetDirectionMode());
        parser->SetBaseDirection(displayInfo.GetDirectionBaseDir());
        parser->SetClockwise(displayInfo.GetDirectionClockwise());
        parser->GetAngleParser().SetAngleMode(displayInfo.GetAngularMode());
        DgnGCS* gcs = model->GetDgnDb().Units().GetDgnGCS();
        if (gcs)
            parser->SetTrueNorthValue(gcs->GetAzimuth());

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
bool XyzRotationsTypeAdapter::_ConvertToString (Utf8StringR str, ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP formatter) const
    {
    // Note no special formatting options are supported for this type
    str.clear();
    GeometricModelCP model = context.GetDgnModel() ? context.GetDgnModel()->ToGeometricModel() : nullptr;
    AngleFormatterPtr fmtr = model ? AngleFormatter::Create(*model) : AngleFormatter::Create();
    return Format (str, v, *fmtr, context, formatter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool XyzRotationsTypeAdapter::_ConvertFromString (ECValueR v, Utf8CP str, IDgnECTypeAdapterContextCR context) const
    {
    if (!v.IsDouble())
        return false;   // can only set individual components from string value...

    GeometricModelCP model = context.GetDgnModel() ? context.GetDgnModel()->ToGeometricModel() : nullptr;
    AngleParserPtr parser = AngleParser::Create();
    if (model)
        parser->SetAngleMode(model->GetDisplayInfo().GetAngularMode());

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
//        GeometricModelCP model = context.GetDgnModel() ? context.GetDgnModel()->ToGeometricModel() : nullptr;
//        if (nullptr != model)
//            {
//            GeometricModel::DisplayInfo const& displayInfo = model->GetDisplayInfo();
//            PrecisionFormat prec = displayInfo.GetLinearPrecision();
//            m_nDecimalPlaces = (((int32_t)prec) & 0x0000000F);
//
//            unitFmt = displayInfo.GetLinearUnitMode();
//
//            m_masterUnit = displayInfo.GetMasterUnit();
//            m_subUnit = displayInfo.GetSubUnit();
//            m_stgUnit = displayInfo.GetStorageUnit();
//            m_uorPerStg = displayInfo.GetUorPerStorage();
//            }
//        else
//            {
//            m_masterUnit.Init (UnitBase::Meter, UnitSystem::Metric, 1.0, 1.0, NULL);
//            m_subUnit = m_masterUnit;
//            m_stgUnit = m_subUnit;
//            }
//
//        // options can override accuracy
//        int32_t accuracy = getIntOrDefault (opts, "Accuracy", USE_ACTIVE);
//        if (0 <= accuracy)
//            m_nDecimalPlaces = (accuracy & 0x0000000F);
//
//        // all but distance can apply multiplier to converted value
//        m_conversionFactor = getDoubleOrDefault (opts, "CF", 1.0);
//
//        // options can override master and sub units
//        if (isDistance)
//            {
//            int32_t masterUnitId = getIntOrDefault (opts, "MasterUnits", USE_ACTIVE);
//            UnitDefinition tmpUnit;
//            if (USE_ACTIVE != masterUnitId)
//                {
//                tmpUnit = UnitDefinition::GetStandardUnit ((StandardUnit)masterUnitId);
//                if (tmpUnit.IsValid())
//                    m_masterUnit = tmpUnit;
//                }
//
//            int32_t subUnitId = getIntOrDefault (opts, "SecondaryUnits", USE_ACTIVE);
//            if (USE_ACTIVE != subUnitId && (tmpUnit = UnitDefinition::GetStandardUnit ((StandardUnit)subUnitId)).IsValid())
//                m_subUnit = tmpUnit;
//
//            int32_t labelFormat = getIntOrDefault (opts, "LabelFormat", USE_ACTIVE);
//            if (DistanceTypeAdapter::Format_SU == labelFormat || DistanceTypeAdapter::Format_SU_Label == labelFormat)
//                unitFmt = DgnUnitFormat::SU;
//
//            // sub-units instead of master?
//            if (DgnUnitFormat::SU == unitFmt)
//                m_masterUnit = m_subUnit;
//            }
//        else
//            {
//            int32_t unitInt = getIntOrDefault (opts, "Units", USE_ACTIVE);
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
bool DistanceTypeAdapter::_ConvertToString (Utf8StringR str, ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP opts) const
    {
    if (v.IsNull() || !v.IsDouble())
        return false;

    GeometricModelCP model = context.GetDgnModel() ? context.GetDgnModel()->ToGeometricModel() : nullptr;
    DistanceFormatterPtr fmtr = model ? DistanceFormatter::Create(*model) : DistanceFormatter::Create();
    InitFormatter(*fmtr, model, opts);

    double storedValue = v.GetDouble();
    str = fmtr->ToString (storedValue);
    DoubleTypeAdapter::ApplyPrefixSuffix (str, opts);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool DistanceTypeAdapter::_ConvertFromString (ECValueR v, Utf8CP str, IDgnECTypeAdapterContextCR context) const
    {
    GeometricModelCP model = context.GetDgnModel() ? context.GetDgnModel()->ToGeometricModel() : nullptr;
    DistanceParserPtr parser = model ? DistanceParser::Create(*model) : DistanceParser::Create();

    double  distance;
    if (SUCCESS != parser->ToValue (distance, str))
        return false;

    v.SetDouble (distance);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DistanceTypeAdapter::InitFormatter(DistanceFormatterR fmtr, GeometricModelCP model, ECN::IECInstanceCP opts)
    {
    DoubleTypeAdapter::InitFormatter (fmtr, opts);
    int32_t labelFormat = getIntOrDefault (opts, "LabelFormat", USE_ACTIVE);

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
    int32_t masterUnitInt = getIntOrDefault (opts, "MasterUnits", USE_ACTIVE);
    if (USE_ACTIVE != masterUnitInt)
        masterUnits = UnitDefinition::GetStandardUnit ((StandardUnit)masterUnitInt);

    int32_t subUnitInt = getIntOrDefault (opts, "SecondaryUnits", USE_ACTIVE);
    if (USE_ACTIVE != subUnitInt)
        subUnits = UnitDefinition::GetStandardUnit ((StandardUnit)subUnitInt);
    else
        {
        // Note: if we pass NULL to SetWorkingUnits() below, it will set subunits equal to masterunits, which is not what we want.
        subUnits = fmtr.GetSubUnits();
        }

    if (masterUnits.IsValid())
        fmtr.SetUnits(masterUnits, subUnits.IsValid() ? &subUnits : NULL);

    // Shared by DWG formatting logic and ordinary formatting logic
    bool showZeroMasterUnits = getBoolOrDefault (opts, "ZMU", true);
    fmtr.SetSuppressZeroMasterUnits (!showZeroMasterUnits);
    bool showZeroSubUnits = getBoolOrDefault (opts, "ZSU", true);
    fmtr.SetSuppressZeroSubUnits (!showZeroSubUnits);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DistanceTypeAdapter::InitOptions (IECInstanceR opts)
    {
    DoubleTypeAdapter::InitOptions (opts);
    ECValue v;
    v.SetInteger (USE_ACTIVE);      opts.SetValue ("LabelFormat",      v);
                                    opts.SetValue ("MasterUnits",      v);
                                    opts.SetValue ("SecondaryUnits",   v);
    v.SetInteger (1);               opts.SetValue ("ZMU",              v);
    v.SetInteger (1);               opts.SetValue ("ZSU",              v);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool CoordinatesTypeAdapter::_ConvertToString (Utf8StringR str, ECValueCR v, IDgnECTypeAdapterContextCR context, ECN::IECInstanceCP opts) const
    {
    str.clear();
    if (v.IsNull())
        return false;

    GeometricModelCP model = context.GetDgnModel() ? context.GetDgnModel()->ToGeometricModel() : nullptr;
    DistanceFormatterPtr fmtr = model ? DistanceFormatter::Create(*model) : DistanceFormatter::Create();
    DistanceTypeAdapter::InitFormatter(*fmtr, model, opts);
    
    return Format(str, v, *fmtr, context, opts);
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
bool CoordinatesTypeAdapter::_ConvertFromString (ECValueR v, Utf8CP str, IDgnECTypeAdapterContextCR context) const
    {
    GeometricModelCP model = context.GetDgnModel() ? context.GetDgnModel()->ToGeometricModel() : nullptr;
    DPoint3d globalOrigin = model ? model->GetDgnDb().Units().GetGlobalOrigin() : DPoint3d::FromXYZ(0, 0, 0);

    DPoint3d pt;
    uint32_t componentIndex = context.GetComponentIndex();
    if (-1 == componentIndex && _AllowDoubleType())
        {
        // dumb special case
        auto prop = context.GetProperty();
        auto primProp = nullptr != prop ? prop->GetAsPrimitiveProperty() : nullptr;
        if (nullptr != primProp && PRIMITIVETYPE_Double == primProp->GetType())
            componentIndex = 2;
        }

    if (-1 != componentIndex)
        {
        DistanceParserPtr parser = model ? DistanceParser::Create(*model) : DistanceParser::Create();
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
        PointParserPtr parser = model ? PointParser::Create(*model) : PointParser::Create();

        parser->SetIs3d (context.Is3d());

        if (SUCCESS != parser->ToValue (pt, str))
            return false;
        pt.Add (globalOrigin);
        }

    DPoint3d ptForStorage = DPoint3d::FromXYZ (pt.x, pt.y, context.Is3d() ? pt.z : 0.0);
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
bool AreaOrVolumeTypeAdapter::_ConvertFromString (ECValueR v, Utf8CP str, IDgnECTypeAdapterContextCR context) const
    {
    // Area/Volume are calculated properties, thus read-only
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void AreaOrVolumeTypeAdapter::InitFormatter(AreaOrVolumeFormatterBase& fmtr, GeometricModelCP model, IECInstanceCP opts) const
    {
    static const double FEET_IN_ACRE = 43560.0;
    static const double SQRT_ACRE = sqrt (FEET_IN_ACRE);

    DoubleTypeAdapter::InitFormatter (fmtr, opts);

    double conversionFactor = getDoubleOrDefault (opts, "CF", 1.0);
    fmtr.SetScaleFactor (conversionFactor);

    bool useAcres = false;
    int32_t unitInt = getIntOrDefault (opts, "Units", USE_ACTIVE);
    if (unitInt >= 0)
        {
        UnitDefinition masterUnit = UnitDefinition::GetStandardUnit ((StandardUnit)unitInt);
        if (masterUnit.IsValid())
            fmtr.SetMasterUnit (masterUnit);
        }
    else if (USE_ACRES == unitInt)
        {
        UnitDefinition masterUnit = UnitDefinition::GetStandardUnit (StandardUnit::EnglishFeet);
        UnitDefinition setUnit;
        setUnit.Init(masterUnit.GetBase(), masterUnit.GetSystem(), masterUnit.GetNumerator() / SQRT_ACRE, masterUnit.GetDenominator(), masterUnit.GetLabelCP());
        fmtr.SetMasterUnit(setUnit);
        useAcres = true;
        }
    else if (USE_ACTIVE_SUBUNITS == unitInt && NULL != model)
        {
        UnitDefinitionCR subUnit = model->GetDisplayInfo().GetSubUnits();
        fmtr.SetMasterUnit (subUnit);
        }

    bool showLabel = getBoolOrDefault (opts, "ShowLabel", false, true);  // Note the default value is 'true' for the formatting IECInstance, but false if we have no IECInstance
    if (!useAcres)
        fmtr.SetShowUnitLabel (showLabel);

    fmtr.SetLabelDecoratorAsSuffix (getBoolOrDefault (opts, "UnitDecorator", true));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void AreaOrVolumeTypeAdapter::ApplyFormatting (Utf8StringR str, IECInstanceCP opts) const
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
    v.SetInteger (USE_ACTIVE);  opts.SetValue ("Units",       v);
    v.SetInteger (1);           opts.SetValue ("ShowLabel",   v);
                                opts.SetValue ("UnitDecorator",   v);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool AreaTypeAdapter::_ConvertToString (Utf8StringR str, ECValueCR v, IDgnECTypeAdapterContextCR context, IECInstanceCP opts) const
    {
    if (v.IsNull())
        {
        str = GetParensNullString();
        return true;
        }
    else if (!v.IsDouble())
        return false;

    GeometricModelCP model = context.GetDgnModel() ? context.GetDgnModel()->ToGeometricModel() : nullptr;
    AreaFormatterPtr fmtr = model ? AreaFormatter::Create(*model) : AreaFormatter::Create();
    double area = v.GetDouble();
   
    InitFormatter(*fmtr, model, opts);
    str = fmtr->ToString (area);

    if (USE_ACRES == getIntOrDefault (opts, "Units", 0))
        {
        if (getBoolOrDefault (opts, "ShowLabel", false, true))
            {
            str.append (" ");
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
bool VolumeTypeAdapter::_ConvertToString (Utf8StringR str, ECValueCR v, IDgnECTypeAdapterContextCR context, IECInstanceCP opts) const
    {
    if (v.IsNull())
        {
        str = GetParensNullString();
        return true;
        }
    else if (!v.IsDouble())
        return false;

    GeometricModelCP model = context.GetDgnModel() ? context.GetDgnModel()->ToGeometricModel() : nullptr;
    VolumeFormatterPtr fmtr = model ? VolumeFormatter::Create(*model) : VolumeFormatter::Create();
    InitFormatter(*fmtr, model, opts);

    double vol = v.GetDouble();
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
* @bsimethod                                                    Paul.Connelly   07/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool UnitDefinitionTypeAdapter::_GetPlaceholderValue (ECValueR v, IDgnECTypeAdapterContextCR context) const
    {
    UnitVisitors::FirstUnitFinder vis (v);
    UnitIteratorOptions options;
    createUnitOptions(options, &context);
    visitUnits <StandardUnitCollection::Entry const&>(vis, StandardUnitCollection(options));
    return !v.IsNull();
    }

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
bool UnitDefinitionTypeAdapter::_ConvertFromString (ECValueR v, Utf8CP str, IDgnECTypeAdapterContextCR context) const
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
        int32_t number;
        if (def.IsValid() && (int32_t)StandardUnit::Custom != (number = (int32_t)def.IsStandardUnit()))
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
bool UnitDefinitionTypeAdapter::GetUnformattedStringValue (Utf8StringR str, ECValueCR v, IDgnECTypeAdapterContextCR context) const
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
bool FormatStringTypeAdapter::GetUnformattedStringValue (Utf8StringR valueAsString, ECN::ECValueCR v, IDgnECTypeAdapterContextCR context) const
    {
    struct ValueList : IECInteropStringFormatter::IECValueList
        {
        ECValueCR m_value;
        ValueList (ECValueCR v) : m_value (v) { }

        virtual uint32_t    GetCount() const override               { return 1; }
        virtual ECValueCP   operator[](uint32_t index) const override { return index == 0 ? &m_value : NULL; }
        };

    IECInstancePtr attr = NULL != context.GetProperty() ? context.GetProperty()->GetCustomAttribute ("Format") : NULL;
    Utf8CP fmtString = "{0}";
    ECValue fmtStringVal;
    if (attr.IsValid() && ECObjectsStatus::Success == attr->GetValue (fmtStringVal, "FormatString") && fmtStringVal.IsString() && !fmtStringVal.IsNull())
        fmtString = fmtStringVal.GetUtf8CP();

    return InteropStringFormatter::GetInstance().Format (valueAsString, fmtString, ValueList (v));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool FormatStringTypeAdapter::_ConvertFromString (ECN::ECValueR v, Utf8CP stringValue, IDgnECTypeAdapterContextCR context) const
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
bool ECUnitsTypeAdapter::_ConvertToString (Utf8StringR valueAsString, ECValueCR inputVal, IDgnECTypeAdapterContextCR context, IECInstanceCP fmtr) const
    {
    ECValue v (inputVal);
    Unit storedUnit, displayUnit;
    ECPropertyCP ecprop = context.GetProperty();
    IECClassLocaterR unitsECClassLocater = context.GetUnitsECClassLocater();

    if (v.IsNull() || NULL == ecprop)
        {
        valueAsString = "";
        return true;
        }

    if(!Unit::GetUnitForECProperty (storedUnit, *ecprop, unitsECClassLocater))
        return false;

    if (context.GetDgnModel())
        ApplyUnitLabelCustomization (storedUnit, *context.GetDgnModel());

    Utf8String fmt;
    Utf8CP label = storedUnit.GetShortLabel();

    if (Unit::GetDisplayUnitAndFormatForECProperty(displayUnit, fmt, storedUnit, *ecprop, unitsECClassLocater))
        {
        if (!v.ConvertToPrimitiveType (PRIMITIVETYPE_Double))
            return false;

        double displayValue = v.GetDouble();
        if (displayUnit.IsCompatible(storedUnit) && storedUnit.ConvertTo(displayValue, displayUnit))
            {
            if (context.GetDgnModel())
                ApplyUnitLabelCustomization(displayUnit, *context.GetDgnModel());

            v.SetDouble(displayValue);
            label = displayUnit.GetShortLabel();
            }
        else
            {
            // Note: Make this an error once TFS#20403 is resolved.
            BeAssert (false && "Cannot convert between stored and displayed units!!");
            }

        }

    Utf8CP fmtCP = fmt.empty() ? "f" : fmt.c_str();
    if (InteropStringFormatter::GetInstance().FormatValue (valueAsString, fmtCP, v) && NULL != label)
        {
        if (!valueAsString.empty())
            valueAsString.append (1, ' ').append (label);
        return true;
        }
    else
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
    Utf8String unusedFmt;
    if (!v.IsNull() && NULL != ecprop && Unit::GetUnitForECProperty (storedUnit, *ecprop) && Unit::GetDisplayUnitAndFormatForECProperty(displayUnit, unusedFmt, storedUnit, *ecprop))
        {
        double displayValue = v.GetDouble();
        if (!displayUnit.IsCompatible (storedUnit) || !storedUnit.ConvertTo (displayValue, displayUnit))
            return false;

        v.SetDouble (displayValue);
        }

    v.SetDouble (v.GetDouble() * getDoubleOrDefault (opts, "CF", 1.0));
    return DoubleTypeAdapter::ConvertToDisplayType (v, opts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECUnitsTypeAdapter::_ConvertFromString (ECValueR v, Utf8CP stringValue, IDgnECTypeAdapterContextCR context) const
    {
    // Note managed implementation expects input value will be in display units - does not check if user provided a different suffix to indicate different units
    ECPropertyCP ecprop = context.GetProperty();
    IECClassLocaterR unitsECClassLocater = context.GetUnitsECClassLocater();
    if (NULL != ecprop && InteropStringFormatter::GetInstance().Parse (v, stringValue, PRIMITIVETYPE_Double))
        {
        Unit storedUnit;
        if (!Unit::GetUnitForECProperty(storedUnit, *ecprop, unitsECClassLocater))
            return false;
        
        Utf8String fmt;
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
void ECUnitsTypeAdapter::ApplyUnitLabelCustomization (UnitR unit, DgnModelR model)
    {
    BeAssert ("ApplyUnitLabelCustomization has not been implemented in Graphite");
        // TODO: Need to discover schemas from the DgnDb in Graphite. 

    //static const Utf8Char s_prefix[] = "UnitsCustomization";
    //static const size_t s_prefixLen = _countof(s_prefix) - 1;
    //static const size_t s_underBarPos = s_prefixLen + 3;

    //bvector<SchemaInfo> schemaInfos;
    //bvector<ECSchemaPtr> schemas;

    //DgnECManagerR mgr = DgnECManager::GetManager();
    //mgr.DiscoverSchemas (schemaInfos, model);

    //size_t i = 0;

    //FOR_EACH (SchemaInfo& schemaInfo, schemaInfos)
    //    {
    //    Utf8CP schemaName = schemaInfo.GetSchemaName();
    //    Utf8CP underBarPos;
    //    if (schemaName == ::strstr (schemaName, s_prefix) && NULL != (underBarPos = ::strstr (schemaName, '_')) && underBarPos == schemaName + s_underBarPos)
    //        {
    //        ECSchemaPtr schema = mgr.LocateSchemaInDgnFile (schemaInfo, SCHEMAMATCHTYPE_Exact);
    //        if (schema.IsValid())
    //            {
    //            // Insert based on priority
    //            size_t insertPos;
    //            for (insertPos = 0; insertPos < schemas.size(); insertPos++)
    //                {
    //                Utf8CP otherSchemaName = schemas[i]->GetName().c_str();
    //                if (0 > strcmp (schemaName + s_prefixLen, otherSchemaName + s_prefixLen))
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
    //    if (NULL != unitClass && (unitAttr = unitClass->GetCustomAttribute ("Unit_Attributes")).IsValid()
    //        && ECObjectsStatus::Success == unitAttr->GetValue (v, "ShortLabel") && !v.IsNull())
    //        {
    //        unit.SetShortLabel (v.GetUtf8CP());
    //        break;
    //        }
    //    }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/15
+---------------+---------------+---------------+---------------+---------------+------*/
static bool getPlaceholderFromPropertyLabel (ECValueR v, IDgnECTypeAdapterContextCR context)
    {
    auto prop = context.GetProperty();
    v.SetUtf8CP(nullptr != prop ? prop->GetDisplayLabel().c_str() : "");
    return true;
    }

#define PLACEHOLDER_PROPERTY_LABEL return getPlaceholderFromPropertyLabel (v, context);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/15
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> bool getPlaceholderFromString (ECValueR v, T const& adapter, Utf8CP str, IDgnECTypeAdapterContextCR context)
    {
    return adapter.ConvertFromString (v, str, context);
    }

#define PLACEHOLDER_PARENS_NONE return getPlaceholderFromString (v, *this, GetParensNoneString(), context);
#define PLACEHOLDER_FROM_STRING(STR) return getPlaceholderFromString (v, *this, STR, context);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool BooleanFormatTypeAdapter::_GetPlaceholderValue (ECValueR v, IDgnECTypeAdapterContextCR context) const { v.SetBoolean (true); return true; }
bool AngleFormatTypeAdapter::_GetPlaceholderValue (ECValueR v, IDgnECTypeAdapterContextCR context) const { v.SetDouble (27.5*msGeomConst_radiansPerDegree); return true; }
bool DateTimeTypeAdapter::_GetPlaceholderValue (ECValueR v, IDgnECTypeAdapterContextCR context) const { v.SetDateTime (DateTimeSettings::GetSampleTime()); return true; }
bool DateTimeFormatTypeAdapter::_GetPlaceholderValue(ECValueR v, IDgnECTypeAdapterContextCR context) const { v.SetUtf8CP("M/d/yyyy h:mm:ss tt"); return true; }
bool StringTypeAdapter::_GetPlaceholderValue (ECValueR v, IDgnECTypeAdapterContextCR context) const { PLACEHOLDER_PROPERTY_LABEL }
bool FormatStringTypeAdapter::_GetPlaceholderValue (ECValueR v, IDgnECTypeAdapterContextCR context) const { return false; }
bool FileSizeTypeAdapter::_GetPlaceholderValue (ECValueR v, IDgnECTypeAdapterContextCR context) const { v.SetLong (12345678); return true; }
bool DistanceTypeAdapter::_GetPlaceholderValue (ECValueR v, IDgnECTypeAdapterContextCR context) const { PLACEHOLDER_FROM_STRING ("12.34") }
bool AreaOrVolumeTypeAdapter::_GetPlaceholderValue (ECValueR v, IDgnECTypeAdapterContextCR context) const { PLACEHOLDER_FROM_STRING ("12.34") }
bool CoordinatesTypeAdapter::_GetPlaceholderValue (ECValueR v, IDgnECTypeAdapterContextCR context) const { PLACEHOLDER_FROM_STRING ("12.34, 56.78, 0.9") }
bool ECUnitsTypeAdapter::_GetPlaceholderValue (ECValueR v, IDgnECTypeAdapterContextCR context) const { PLACEHOLDER_FROM_STRING ("123.45") }

END_BENTLEY_DGNPLATFORM_NAMESPACE

#undef RSC_STR
