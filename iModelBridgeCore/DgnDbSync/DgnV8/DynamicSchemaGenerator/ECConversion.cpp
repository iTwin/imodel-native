/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnV8/DynamicSchemaGenerator/ECConversion.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ConverterInternal.h"
#include "ECConversion.h"
#include "ECDiff.h"
#include <regex>
#include <ECObjects/StandardCustomAttributeHelper.h>
#include <Units/Units.h>
#include <Formatting/FormattingApi.h>

#define TEMPTABLE_ATTACH(name) "temp." name

BEGIN_DGNDBSYNC_DGNV8_NAMESPACE

using namespace BeSQLite::EC;

static bvector<Utf8CP> s_dgnV8DeliveredSchemas = {
    "BaseElementSchema",
    "BentleyDesignLinksPersistence",
    "BentleyDesignLinksPresetnation",
    "BentleyDrawingLinksPersistence",
    "DetailSymbolExtender",
    "DgnComponentSchema",
    "DgnContentRelationshipSchema",
    "DgnCustomAttributes",
    "DgnElementSchema",
    "RfaElementSchema",
    "DgnFileSchema",
    "DgnindexQueryschema",
    "DgnLevelSchema",
    "DgnModelSchema",
    "DgnPointCloudSchema",
    "DgnTextStyleObjSchema",
    "DgnVisualizationObjSchema",
    "ExtendedElementSchema",
    "MstnPropertyFormatter",
    "Ustn_ElementParams",
    "DTMElement_TemplateExtender_Schema",
    "dgn",
    "ECDbSystem",
    "ECDb_FileInfo"
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
static bool anyTxnsInFile(DgnDbR db)
    {
    Statement stmt;
    stmt.Prepare(db, "SELECT Id FROM " DGN_TABLE_Txns " LIMIT 1");
    return (BE_SQLITE_ROW == stmt.Step());
    }

static Utf8CP const EXTEND_TYPE = "ExtendType";
static Utf8CP const Units_SchemaName = "Units";
static Utf8CP const Formats_SchemaName = "Formats";

//****************************************************************************************
// ExtendTypeConverter
//****************************************************************************************
struct ExtendTypeConverter : ECN::IECCustomAttributeConverter
    {
    private:
        ECN::ECSchemaPtr m_unitsStandardSchema; // cache of the units schema
        ECN::ECSchemaPtr m_formatsStandardSchema; // cache of the units schema
        DgnV8Api::StandardUnit m_standardUnit;
        DgnV8Api::AngleMode m_angle;

        ECN::ECObjectsStatus ReplaceWithKOQ(ECN::ECSchemaR schema, ECN::ECPropertyP prop, Utf8String koqName, Utf8CP persistenceUnitName, Utf8CP presentationUnitName, ECN::ECSchemaReadContextR context);
    public:
        ECN::ECObjectsStatus Convert(ECN::ECSchemaR schema, ECN::IECCustomAttributeContainerR container, ECN::IECInstanceR instance, ECN::ECSchemaReadContextP context);
        ExtendTypeConverter(DgnV8Api::StandardUnit standard, DgnV8Api::AngleMode angle) : m_standardUnit(standard), m_angle(angle) {}
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            03/2018
//---------------+---------------+---------------+---------------+---------------+-------
ECN::ECObjectsStatus ExtendTypeConverter::ReplaceWithKOQ(ECN::ECSchemaR schema, ECN::ECPropertyP prop, Utf8String koqName, Utf8CP persistenceUnitName, Utf8CP presentationUnitName, ECN::ECSchemaReadContextR context)
    {
    ECN::KindOfQuantityP koq = schema.GetKindOfQuantityP(koqName.c_str());
    if (nullptr == koq)
        {
        schema.CreateKindOfQuantity(koq, koqName.c_str());

        if (!m_unitsStandardSchema.IsValid())
            {
            static ECN::SchemaKey key("Units", 1, 0, 0);
            m_unitsStandardSchema = ECN::ECSchema::LocateSchema(key, context);
            if (!m_unitsStandardSchema.IsValid())
                {
                BeAssert(false);
                return ECN::ECObjectsStatus::SchemaNotFound;
                }
            }

        if (!m_formatsStandardSchema.IsValid())
            {
            static ECN::SchemaKey key("Formats", 1, 0, 0);
            m_formatsStandardSchema = ECN::ECSchema::LocateSchema(key, context);
            if (!m_formatsStandardSchema.IsValid())
                {
                BeAssert(false);
                return ECN::ECObjectsStatus::SchemaNotFound;
                }
            }

        // Locate persistence Unit within Format Schema
        ECN::ECUnitCP persistenceUnit = m_unitsStandardSchema->GetUnitCP(persistenceUnitName);
        if (nullptr == persistenceUnit)
            return ECN::ECObjectsStatus::Error;

        // Check if Units Schema is referenced
        if (!ECN::ECSchema::IsSchemaReferenced(schema, *m_unitsStandardSchema) && ECN::ECObjectsStatus::Success != schema.AddReferencedSchema(*m_unitsStandardSchema))
            {
            LOG.errorv("Unable to add the %s schema as a reference to %s.", m_unitsStandardSchema->GetFullSchemaName().c_str(), schema.GetName().c_str());
            return ECN::ECObjectsStatus::SchemaNotFound;
            }

        koq->SetPersistenceUnit(*persistenceUnit);

        // Locate presentation Unit within Format Schema
        ECN::ECUnitCP presUnit = m_unitsStandardSchema->GetUnitCP(presentationUnitName);
        if (nullptr == presUnit)
            {
            BeAssert(false);
            return ECN::ECObjectsStatus::Error;
            }

        // Check if Units Schema is referenced
        if (!ECN::ECSchema::IsSchemaReferenced(schema, *m_formatsStandardSchema) && ECN::ECObjectsStatus::Success != schema.AddReferencedSchema(*m_formatsStandardSchema))
            {
            LOG.errorv("Unable to add the %s schema as a reference to %s.", m_formatsStandardSchema->GetFullSchemaName().c_str(), schema.GetName().c_str());
            return ECN::ECObjectsStatus::SchemaNotFound;
            }

        ECN::ECFormatCP format = schema.LookupFormat("f:DefaultRealU");
        if (nullptr == format)
            {
            BeAssert(false);
            return ECN::ECObjectsStatus::Error;
            }

        // No need to check if the Units schema is referenced, checked for persistence Unit
        koq->AddPresentationFormatSingleUnitOverride(*format, nullptr, presUnit);
        koq->SetRelativeError(1e-4);
        }
    prop->SetKindOfQuantity(koq);
    prop->RemoveCustomAttribute("EditorCustomAttributes", EXTEND_TYPE);
    prop->RemoveSupplementedCustomAttribute("EditorCustomAttributes", EXTEND_TYPE);
    return ECN::ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            04/2018
//---------------+---------------+---------------+---------------+---------------+-------
Utf8CP getLinearUnitName(DgnV8Api::StandardUnit standard)
    {
    switch (standard)
        {
        case StandardUnit::EnglishMiles:
            return "MILE";
        case StandardUnit::EnglishYards:
            return "YRD";
        case StandardUnit::EnglishFeet:
            return "FT";
        case StandardUnit::EnglishInches:
            return "IN";
        case StandardUnit::EnglishMicroInches:
            return "MICROINCH";
        case StandardUnit::EnglishMils:
            return "MILLIINCH";
        case StandardUnit::EnglishSurveyMiles:
            return "US_SURVEY_MILE";
        case StandardUnit::EnglishSurveyFeet:
            return "US_SURVEY_FT";
        case StandardUnit::EnglishSurveyInches:
            return "US_SURVEY_IN";
        case StandardUnit::MetricKilometers:
            return "KM";
        case StandardUnit::MetricMeters:
            return "M";
        case StandardUnit::MetricCentimeters:
            return "CM";
        case StandardUnit::MetricMillimeters:
            return "MM";
        case StandardUnit::MetricMicrometers:
            return "MU";
        case StandardUnit::NoSystemNauticalMiles:
            return "NAUT_MILE";
        default:
            return "M";
        };
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            04/2018
//---------------+---------------+---------------+---------------+---------------+-------
Utf8CP getAreaUnitName(DgnV8Api::StandardUnit standard)
    {
    switch (standard)
        {
        case StandardUnit::EnglishMiles:
            return "SQ_MILE";
        case StandardUnit::EnglishYards:
            return "SQ_YRD";
        case StandardUnit::EnglishFeet:
            return "SQ_FT";
        case StandardUnit::EnglishInches:
        case StandardUnit::EnglishMicroInches: // There is no equivalent in the new system for square microinches
        case StandardUnit::EnglishMils: // There is no equivalent in the new system for square milliinches
            return "SQ_IN";
        case StandardUnit::EnglishSurveyMiles:
            return "SQ_US_SURVEY_MILE";
        case StandardUnit::EnglishSurveyFeet:
            return "SQ_US_SURVEY_FT";
        case StandardUnit::EnglishSurveyInches:
            return "SQ_US_SURVEY_IN";
        case StandardUnit::MetricKilometers:
            return "SQ_KM";
        case StandardUnit::MetricMeters:
            return "SQ_M";
        case StandardUnit::MetricCentimeters:
            return "SQ_CM";
        case StandardUnit::MetricMillimeters:
            return "SQ_MM";
        case StandardUnit::MetricMicrometers:
            return "SQ_MU";
        case StandardUnit::NoSystemNauticalMiles:
            return "SQ_MILE"; // There is no equivalent in the new system for Square Nautical miles
        default:
            return "SQ_M";
    };
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            04/2018
//---------------+---------------+---------------+---------------+---------------+-------
Utf8CP getVolumeUnitName(DgnV8Api::StandardUnit standard)
    {
    switch (standard)
        {
        case StandardUnit::EnglishMiles:
            return "CUB_MILE";
        case StandardUnit::EnglishYards:
            return "CUB_YRD";
        case StandardUnit::EnglishFeet:
            return "CUB_FT";
        case StandardUnit::EnglishInches:
        case StandardUnit::EnglishMicroInches: // There is no equivalent in the new system for cubic microinches
        case StandardUnit::EnglishMils: // There is no equivalent in the new system for cubic milliinches
            return "CUB_IN";
        case StandardUnit::EnglishSurveyMiles:  // There is no equivalent in the new system for cubic survey miles
            return "CUB_MILE";
        case StandardUnit::EnglishSurveyFeet:  // There is no equivalent in the new system for cubic survey feet
            return "CUB_FT";
        case StandardUnit::EnglishSurveyInches: // There is no equivalent in the new system for cubic survey inches
            return "CUB_IN";
        case StandardUnit::MetricKilometers:
            return "CUB_KM";
        case StandardUnit::MetricMeters:
            return "CUB_M";
        case StandardUnit::MetricCentimeters:
            return "CUB_CM";
        case StandardUnit::MetricMillimeters:
            return "CUB_MM";
        case StandardUnit::MetricMicrometers:
            return "CUB_MU";
        case StandardUnit::NoSystemNauticalMiles:
            return "CUB_MILE"; // There is no equivalent in the new system for cubic Nautical miles
        default:
            return "CUB_M";
        };
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            04/2018
//---------------+---------------+---------------+---------------+---------------+-------
Utf8CP getAngleUnitName(DgnV8Api::AngleMode angle)
    {
    switch (angle)
        {
        case AngleMode::Degrees:
            return "ARC_DEG";
        case AngleMode::DegMin:
            return "ARC_MINUTE";
        case AngleMode::DegMinSec:
            return "ARC_SECOND";
        case AngleMode::Centesimal:
            return "GRAD";
        default:
            return "RAD";
        };
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            03/2018
//---------------+---------------+---------------+---------------+---------------+-------
ECN::ECObjectsStatus ExtendTypeConverter::Convert(ECN::ECSchemaR schema, ECN::IECCustomAttributeContainerR container, ECN::IECInstanceR instance, BECN::ECSchemaReadContextP context)
    {
    ECN::ECPropertyP prop = dynamic_cast<ECN::ECPropertyP> (&container);
    if (prop == nullptr)
        {
        LOG.warningv("Found ExtendType custom attribute on a container which is not a property, removing.  Container is %s", container.GetContainerName());
        container.RemoveCustomAttribute("EditorCustomAttributes", EXTEND_TYPE);
        container.RemoveSupplementedCustomAttribute("EditorCustomAttributes", EXTEND_TYPE);
        return ECN::ECObjectsStatus::Success;
        }

    ECN::ECValue standardValue;
    ECN::ECObjectsStatus status = instance.GetValue(standardValue, "Standard");
    if (ECN::ECObjectsStatus::Success != status || standardValue.IsNull())
        {
        LOG.infov("Found an ExtendType custom attribute on an ECProperty, '%s.%s', but it did not contain a Standard value. Dropping custom attribute....",
                     prop->GetClass().GetFullName(), prop->GetName().c_str());
        container.RemoveCustomAttribute("EditorCustomAttributes", EXTEND_TYPE);
        container.RemoveSupplementedCustomAttribute("EditorCustomAttributes", EXTEND_TYPE);
        return ECN::ECObjectsStatus::Success;
        }

    if (nullptr == context)
        {
        BeAssert(true);
        LOG.error("Missing ECSchemaReadContext, it is necessary to perform conversion on a ExtendType custom attribute.");
        return ECN::ECObjectsStatus::Error;
        }

    int standard = standardValue.GetInteger();
    switch (standard)
        {
        // Coordinates
        //case 7:
        //    ReplaceWithKOQ(schema, prop, "COORDINATE", "Coord", 0.0001);
        //    break;
        // Distance
        case 8:
            ReplaceWithKOQ(schema, prop, "DISTANCE", "M", getLinearUnitName(m_standardUnit), *context);
            break;
            // Area
        case 9:
            ReplaceWithKOQ(schema, prop, "AREA", "SQ_M", getAreaUnitName(m_standardUnit), *context);
            break;
            // Volume
        case 10:
            ReplaceWithKOQ(schema, prop, "VOLUME", "CUB_M", getVolumeUnitName(m_standardUnit), *context);
            break;
            // Angle
        case 11:
            ReplaceWithKOQ(schema, prop, "ANGLE", "RAD", getAngleUnitName(m_angle), *context);
            break;
        default:
            LOG.warningv("Found an ExtendType custom attribute on an ECProperty, '%s.%s', with an unknown standard value %d.  Only values 7-11 are supported.",
                         prop->GetClass().GetFullName(), prop->GetName().c_str());
            break;
        }

    // Need to clear the cache of the units and formats schema.
    m_unitsStandardSchema = nullptr;
    m_formatsStandardSchema = nullptr;

    return ECN::ECObjectsStatus::Success;
    }

//****************************************************************************************
// ECClassName
//****************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     02/2015
//---------------------------------------------------------------------------------------
Utf8String ECClassName::GetClassFullName() const
    {
    Utf8String fullName(GetSchemaName());
    fullName.append(".").append(GetClassName());
    return fullName;
    }

//****************************************************************************************
// BisConversionRuleHelper
//****************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     02/2015
//---------------------------------------------------------------------------------------
//static
BisConversionRule BisConversionRuleHelper::ConvertToBisConversionRule(V8ElementType v8ElementType, BisConversionTargetModelInfoCR targetModelInfo, const bool namedGroupOwnsMembersFlag, bool isSecondaryInstancesClass)
    {
    if (isSecondaryInstancesClass)
        return BisConversionRule::ToAspectOnly;

    if (v8ElementType == V8ElementType::NamedGroup)
        {
        if (!namedGroupOwnsMembersFlag)
            {
            return BisConversionRule::ToGroup;
            }
        }

#ifdef WIP_COMPONENT_MODEL // *** Pending redesign
    return is3d.IsSpatialModel() || nullptr != dynamic_cast<ComponentModel*>(&is3d) ? BisConversionRule::ToPhysicalElement : BisConversionRule::ToDrawingGraphic;
#endif
    return !targetModelInfo.IsDictionary() && targetModelInfo.Is2d() ? BisConversionRule::ToDrawingGraphic : BisConversionRule::ToPhysicalElement;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     07/2015
//---------------------------------------------------------------------------------------
//static
BentleyStatus BisConversionRuleHelper::ConvertToBisConversionRule(BisConversionRule& rule, BECN::ECClassCR v8ECClass)
    {
    //relationships and structs/CAs that are not domain classes remain as is.
    BECN::ECRelationshipClass const* v8RelClass = v8ECClass.GetRelationshipClassCP();
    if (v8RelClass != nullptr)
        rule = BisConversionRule::TransformedUnbisified;
    else if (v8ECClass.IsStructClass() || v8ECClass.IsCustomAttributeClass())
        rule = BisConversionRule::TransformedUnbisifiedAndIgnoreInstances;
    else
        rule = BisConversionRule::ToDefaultBisBaseClass;


    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     02/2015
//---------------------------------------------------------------------------------------
//static
BentleyStatus BisConversionRuleHelper::TryDetermineElementAspectKind(ElementAspectKind& kind, BisConversionRule rule)
    {
    bool success = true;
    switch (rule)
        {
            case BisConversionRule::ToAspectOnly:
                kind = ElementAspectKind::ElementMultiAspect;
                break;

            default:
                success = false;
                break;
        }

    return success ? BSISUCCESS : BSIERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     02/2015
//---------------------------------------------------------------------------------------
//static
bool BisConversionRuleHelper::IsSecondaryInstance(BisConversionRule rule)
    {
    return rule == BisConversionRule::ToAspectOnly;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     02/2015
//---------------------------------------------------------------------------------------
//static
bool BisConversionRuleHelper::IgnoreInstance(BisConversionRule rule)
    {
    switch (rule)
        {
            case BisConversionRule::Ignored:
            case BisConversionRule::IgnoredPolymorphically:
            case BisConversionRule::TransformedUnbisifiedAndIgnoreInstances:
                return true;

            default:
                return false;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     07/2015
//---------------------------------------------------------------------------------------
//static
bool BisConversionRuleHelper::ClassNeedsBisification(BisConversionRule conversionRule)
    {
    switch (conversionRule)
        {
            case BisConversionRule::ToAspectOnly:
            case BisConversionRule::ToDrawingGraphic:
            case BisConversionRule::ToGroup:
            case BisConversionRule::ToGenericGroup:
            case BisConversionRule::ToPhysicalElement:
            case BisConversionRule::ToPhysicalObject:
            case BisConversionRule::ToDefaultBisClass:
            case BisConversionRule::ToDefaultBisBaseClass:
                return true;

            default:
                return false;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     02/2015
//---------------------------------------------------------------------------------------
//static
Utf8CP BisConversionRuleHelper::ToString(BisConversionRule rule)
    {
    switch (rule)
        {
            case BisConversionRule::Ignored:
                return "Ignored";
            case BisConversionRule::TransformedUnbisified:
                return "TransformedUnbisified";
            case BisConversionRule::TransformedUnbisifiedAndIgnoreInstances:
                return "TransformedUnbisifiedAndIgnoreInstances";
            case BisConversionRule::ToAspectOnly:
                return "ToAspectOnly";
            case BisConversionRule::ToDrawingGraphic:
                return "ToDrawingGraphic";
            case BisConversionRule::ToGroup:
                return "ToGroup";
            case BisConversionRule::ToGenericGroup:
                return "ToGenericGroup";
            case BisConversionRule::ToPhysicalElement:
                return "ToPhysicalElement";
            case BisConversionRule::ToPhysicalObject:
                return "ToPhysicalObject";
            case BisConversionRule::ToDefaultBisBaseClass:
                return "ToDefaultBisBaseClass";
            case BisConversionRule::ToDefaultBisClass:
                return "ToDefaultBisClass";
            default:
                BeAssert(false && "Please update V8ECClassInfo::ToString for new value of the BisConversionRule enum.");
                return "";
        }
    }

//****************************************************************************************
// V8ElementTypeHelper
//****************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     04/2015
//---------------------------------------------------------------------------------------
//static
V8ElementType V8ElementTypeHelper::GetType(DgnV8EhCR v8eh)
    {
    return GetType(*v8eh.GetElementRef());

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   04/2015
//---------------------------------------------------------------------------------------
//static
V8ElementType V8ElementTypeHelper::GetType(DgnV8Api::ElementRefBase const& v8El)
    {
    if (v8El.IsGraphics())
        return V8ElementType::Graphical;

    const bool isNamedGroupElement = v8El.GetHandler() == &DgnV8Api::NamedGroupHandler::GetInstance();
    return isNamedGroupElement ? V8ElementType::NamedGroup : V8ElementType::NonGraphical;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     04/2015
//---------------------------------------------------------------------------------------
//static
Utf8CP V8ElementTypeHelper::ToString(V8ElementType v8ElementType)
    {
    switch (v8ElementType)
        {
            case V8ElementType::Graphical:
                return "graphical";
            case V8ElementType::NonGraphical:
                return "non-graphical";
            case V8ElementType::NamedGroup:
                return "Named Group";
            default:
                BeAssert(false);
                return "";
        }
    }

//****************************************************************************************
// V8ECClassInfo
//****************************************************************************************
#define V8ECCLASS_TABLE SYNCINFO_TABLE("V8ECClass")

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     02/2015
//---------------------------------------------------------------------------------------
//static
bool V8ECClassInfo::TryFind(BECN::ECClassId& v8ClassId, BisConversionRule& rule, DgnDbR db, ECClassName const& v8ClassName, bool& hasSecondary)
    {
    if (!v8ClassName.IsValid())
        {
        BeAssert(false);
        return false;
        }

    CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement(stmt, "SELECT V8ClassId, BisConversionRule, HasSecondary FROM " SYNCINFO_ATTACH(V8ECCLASS_TABLE) " WHERE V8SchemaName=? AND V8ClassName=?");
    if (stat != BE_SQLITE_OK)
        {
        BeAssert(false && "Could not retrieve cached statement.");
        return false;
        }

    stmt->BindText(1, v8ClassName.GetSchemaName(), Statement::MakeCopy::No);
    stmt->BindText(2, v8ClassName.GetClassName(), Statement::MakeCopy::No);

    while (BE_SQLITE_ROW == stmt->Step())
        {
        BeAssert(!stmt->IsColumnNull(0) && !stmt->IsColumnNull(1));

        v8ClassId = stmt->GetValueId<BECN::ECClassId>(0);
        rule = (BisConversionRule) stmt->GetValueInt(1);
        hasSecondary = stmt->GetValueBoolean(2);

        return true;
        }

    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     02/2015
//---------------------------------------------------------------------------------------
//static
BentleyStatus V8ECClassInfo::Insert(DynamicSchemaGenerator& converter, DgnV8EhCR v8Eh, ECClassName const& v8ClassName, bool namedGroupOwnsMembers, bool isSecondaryInstancesClass, BisConversionTargetModelInfoCR targetModelInfo)
    {
    const V8ElementType v8ElementType = V8ElementTypeHelper::GetType(v8Eh);

    BisConversionRule existingRule;
    BECN::ECClassId existingClassId;
    bool hasSecondary;
    const bool classInfoFound = TryFind(existingClassId, existingRule, converter.GetDgnDb(), v8ClassName, hasSecondary);

    BisConversionRule rule = BisConversionRule::ToDefaultBisBaseClass;
    ConvertToDgnDbElementExtension* upx = ConvertToDgnDbElementExtension::Cast(v8Eh.GetHandler());
    if (nullptr != upx)
        rule = upx->_DetermineBisConversionRule(v8Eh, converter.GetDgnDb(), targetModelInfo);
    for (auto xdomain : XDomainRegistry::s_xdomains)
        xdomain->_DetermineBisConversionRule(rule, v8Eh, converter.GetDgnDb(), targetModelInfo);
    if (BisConversionRule::ToDefaultBisBaseClass == rule)
        rule = BisConversionRuleHelper::ConvertToBisConversionRule(v8ElementType, targetModelInfo, namedGroupOwnsMembers, isSecondaryInstancesClass);

    ECDiagnostics::LogV8InstanceDiagnostics(v8Eh, v8ElementType, v8ClassName, isSecondaryInstancesClass, rule);

    if (classInfoFound)
        {
        if (existingRule == rule)
            return BSISUCCESS;

        if (existingRule == BisConversionRule::ToAspectOnly)
            {
            Utf8PrintfString info("Multiple rules for ECClass '%s'. Using %s and creating a secondary Aspect class.", v8ClassName.GetClassFullName().c_str(), BisConversionRuleHelper::ToString(rule));
            converter.ReportIssue(Converter::IssueSeverity::Info, Converter::IssueCategory::Sync(), Converter::Issue::Message(), info.c_str());
            Update(converter, existingClassId, rule, true);
            }
        else if (rule == BisConversionRule::ToAspectOnly)
            {
            Utf8PrintfString info("Multiple rules for ECClass '%s'. Using %s and creating a secondary Aspect class.", v8ClassName.GetClassFullName().c_str(), BisConversionRuleHelper::ToString(existingRule));
            converter.ReportIssue(Converter::IssueSeverity::Info, Converter::IssueCategory::Sync(), Converter::Issue::Message(), info.c_str());
            Update(converter, existingClassId, existingRule, true);
            }
        else
            {
            Utf8PrintfString info("Ambiguous conversion rules found for ECClass '%s': %s versus %s. Keeping the previous one", v8ClassName.GetClassFullName().c_str(), BisConversionRuleHelper::ToString(existingRule), BisConversionRuleHelper::ToString(rule));
            converter.ReportIssue(Converter::IssueSeverity::Info, Converter::IssueCategory::Sync(), Converter::Issue::Message(), info.c_str());
            }
        return BSISUCCESS;
        }

    return DoInsert(converter.GetDgnDb(), v8ClassName, rule);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     02/2015
//---------------------------------------------------------------------------------------
//static
BentleyStatus V8ECClassInfo::InsertOrUpdate(DynamicSchemaGenerator& converter, ECClassName const& v8ClassName, BisConversionRule rule)
    {
    BisConversionRule existingRule;
    BECN::ECClassId existingClassId;
    bool hasSecondary;
    if (TryFind(existingClassId, existingRule, converter.GetDgnDb(), v8ClassName, hasSecondary))
        return Update(converter, existingClassId, rule, hasSecondary);

    return Insert(converter, v8ClassName, rule);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     07/2015
//---------------------------------------------------------------------------------------
//static
BentleyStatus V8ECClassInfo::Insert(DynamicSchemaGenerator& converter, ECClassName const& v8ClassName, BisConversionRule rule)
    {
    return DoInsert(converter.GetDgnDb(), v8ClassName, rule);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     03/2015
//---------------------------------------------------------------------------------------
//static
BentleyStatus V8ECClassInfo::DoInsert(DgnDbR db, ECClassName const& v8ClassName, BisConversionRule rule)
    {
    if (!v8ClassName.IsValid())
        {
        BeAssert(false);
        return BSIERROR;
        }

    CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement(stmt, "INSERT INTO " SYNCINFO_ATTACH(V8ECCLASS_TABLE) " (V8SchemaName,V8ClassName,BisConversionRule) VALUES (?,?,?)");
    if (stat != BE_SQLITE_OK)
        {
        BeAssert(false && "Could not retrieve cached statement.");
        return BSIERROR;
        }

    stmt->BindText(1, v8ClassName.GetSchemaName(), Statement::MakeCopy::No);
    stmt->BindText(2, v8ClassName.GetClassName(), Statement::MakeCopy::No);
    stmt->BindInt(3, (int) rule);

    return BE_SQLITE_DONE == stmt->Step() ? BSISUCCESS : BSIERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     02/2015
//---------------------------------------------------------------------------------------
//static
BentleyStatus V8ECClassInfo::Update(DynamicSchemaGenerator& converter, BECN::ECClassId v8ClassId, BisConversionRule rule, bool hasSecondary)
    {
    if (!v8ClassId.IsValid())
        {
        BeAssert(false);
        return BSIERROR;
        }

    CachedStatementPtr stmt = nullptr;
    DbResult stat = converter.GetDgnDb().GetCachedStatement(stmt, "UPDATE " SYNCINFO_ATTACH(V8ECCLASS_TABLE) " SET BisConversionRule=?, HasSecondary=? WHERE V8ClassId=?");
    if (stat != BE_SQLITE_OK)
        {
        BeAssert(false && "Could not retrieve cached statement.");
        return BSIERROR;
        }

    stmt->BindInt(1, (int) rule);
    stmt->BindBoolean(2, hasSecondary);
    stmt->BindId(3, v8ClassId);
    return BE_SQLITE_DONE == stmt->Step() ? BSISUCCESS : BSIERROR;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     02/2015
//---------------------------------------------------------------------------------------
//static
BentleyStatus V8ECClassInfo::CreateTable(DgnDbR db)
    {
    if (db.TableExists(SYNCINFO_ATTACH(V8ECCLASS_TABLE)))
        return BSISUCCESS;

    if (BE_SQLITE_OK != db.ExecuteSql("CREATE TABLE " SYNCINFO_ATTACH(V8ECCLASS_TABLE) " (V8ClassId INTEGER PRIMARY KEY, V8SchemaName TEXT NOT NULL, V8ClassName TEXT NOT NULL, BisConversionRule INTEGER NOT NULL, HasSecondary BOOL DEFAULT 0)"))
        return BSIERROR;

    if (BE_SQLITE_OK != db.ExecuteSql("CREATE UNIQUE INDEX " SYNCINFO_ATTACH(V8ECCLASS_TABLE) "_uix ON " V8ECCLASS_TABLE " (V8SchemaName, V8ClassName)"))
        return BSIERROR;

    return BSISUCCESS;
    }

//****************************************************************************************
// V8ElementECClassInfo
//****************************************************************************************
#define V8ELEMENT_SECONDARYCLASSMAPPING_TABLE SYNCINFO_TABLE("V8ElementSecondaryECClass")

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            02/2017
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus V8ElementSecondaryECClassInfo::CreateTable(DgnDbR db)
    {
    if (db.TableExists(SYNCINFO_ATTACH(V8ELEMENT_SECONDARYCLASSMAPPING_TABLE)))
        return BSISUCCESS;

    return db.ExecuteSql("CREATE TABLE " SYNCINFO_ATTACH(V8ELEMENT_SECONDARYCLASSMAPPING_TABLE) " (V8ElementId INTEGER NOT NULL, V8SchemaName, V8ClassName, "
                         " PRIMARY KEY (V8ElementId, V8SchemaName, V8ClassName))") == BE_SQLITE_OK ? BSISUCCESS : BSIERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            02/2017
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus V8ElementSecondaryECClassInfo::Insert(DgnDbR db, DgnV8EhCR el, ECClassName const& v8Class)
    {
    CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement(stmt, "INSERT INTO " SYNCINFO_ATTACH(V8ELEMENT_SECONDARYCLASSMAPPING_TABLE) " (V8ElementId, V8SchemaName, V8ClassName) VALUES (?, ?, ?)");
    if (stat != BE_SQLITE_OK)
        {
        BeAssert(false && "Could not retrieve cached statement for V8ElementECClassInfo::Insert");
        return BSIERROR;
        }

    stmt->BindInt64(1, el.GetElementId());
    stmt->BindText(2, v8Class.GetSchemaName(), Statement::MakeCopy::No);
    stmt->BindText(3, v8Class.GetClassName(), Statement::MakeCopy::No);
    stat = stmt->Step();

    return stat == BE_SQLITE_DONE ? BSISUCCESS : BSIERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            02/2017
//---------------+---------------+---------------+---------------+---------------+-------
bool V8ElementSecondaryECClassInfo::TryFind(DgnDbR db, DgnV8EhCR el, ECClassName const& ecClassName)
    {
    CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement(stmt, "SELECT V8SchemaName, V8ClassName FROM " SYNCINFO_ATTACH(V8ELEMENT_SECONDARYCLASSMAPPING_TABLE) " WHERE V8ElementId = ? AND V8SchemaName=? AND V8ClassName=?");
    if (stat != BE_SQLITE_OK)
        {
        BeAssert(false && "Could not retrieve cached statement for V8ElementECClassInfo::Find.");
        return false;
        }

    stmt->BindInt64(1, el.GetElementId());
    stmt->BindText(2, ecClassName.GetSchemaName(), Statement::MakeCopy::No);
    stmt->BindText(3, ecClassName.GetClassName(), Statement::MakeCopy::No);
    if (stmt->Step() != BE_SQLITE_ROW)
        return false;

    return true;
    }

//****************************************************************************************
// V8ECSchemaXmlInfo
//****************************************************************************************
#define V8ECSCHEMAXML_TABLE "V8ECSchemaXml"
//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     02/2015
//---------------------------------------------------------------------------------------
//static
BeSQLite::DbResult V8ECSchemaXmlInfo::Insert(DgnDbR db, BECN::ECSchemaId schemaId, Utf8CP schemaXml)
    {
    if (!schemaId.IsValid() || Utf8String::IsNullOrEmpty(schemaXml))
        {
        BeAssert(false);
        return BE_SQLITE_ERROR;
        }

    CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement(stmt, "INSERT INTO " TEMPTABLE_ATTACH(V8ECSCHEMAXML_TABLE) " (Id, Xml) VALUES (?, ?)");
    if (stat != BE_SQLITE_OK)
        {
        BeAssert(false && "Could not retrieve cached statement.");
        return stat;
        }

    stmt->BindId(1, schemaId);
    stmt->BindText(2, schemaXml, Statement::MakeCopy::No);
    stat = stmt->Step();

    return stat == BE_SQLITE_DONE ? BE_SQLITE_OK : stat;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     02/2015
//---------------------------------------------------------------------------------------
//static
BeSQLite::DbResult V8ECSchemaXmlInfo::CreateTable(DgnDbR db)
    {
    return db.ExecuteSql("CREATE TABLE " TEMPTABLE_ATTACH(V8ECSCHEMAXML_TABLE) " (Id INTEGER NOT NULL, Xml TEXT NOT NULL);");
    }

//****************************************************************************************
// V8ECSchemaXmlInfo::Iterable
//****************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     02/2015
//---------------------------------------------------------------------------------------
V8ECSchemaXmlInfo::Iterable::const_iterator V8ECSchemaXmlInfo::Iterable::begin() const
    {
    if (m_stmt == nullptr)
        m_db->GetCachedStatement(m_stmt, "SELECT s.V8Name,s.V8VersionMajor,s.V8VersionMinor,s.MappingType,x.Xml FROM " TEMPTABLE_ATTACH(V8ECSCHEMAXML_TABLE) " x, "
                                 SYNCINFO_ATTACH(SYNC_TABLE_ECSchema) " s WHERE x.Id = s.rowid");
    else
        m_stmt->Reset();

    return Entry(m_stmt.get(), BE_SQLITE_ROW == m_stmt->Step());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     02/2015
//---------------------------------------------------------------------------------------
BECN::SchemaKey V8ECSchemaXmlInfo::Iterable::Entry::GetSchemaKey() const
    {
    BECN::SchemaKey key(m_sql->GetValueText(0), (uint32_t) m_sql->GetValueInt(1), (uint32_t) m_sql->GetValueInt(2));
    Utf8String xml(GetSchemaXml());
    key.m_checksum = ECN::ECSchema::ComputeSchemaXmlStringCheckSum(xml.c_str(), xml.length());
    return key;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     02/2015
//---------------------------------------------------------------------------------------
Utf8CP V8ECSchemaXmlInfo::Iterable::Entry::GetSchemaXml() const
    {
    return m_sql->GetValueText(4);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     02/2015
//---------------------------------------------------------------------------------------
SyncInfo::ECSchemaMappingType V8ECSchemaXmlInfo::Iterable::Entry::GetMappingType() const
    {
    return (SyncInfo::ECSchemaMappingType) m_sql->GetValueInt(3);
    }


//****************************************************************************************
// ECSchemaXmlDeserializer
//****************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     02/2015
//---------------------------------------------------------------------------------------
BECN::ECSchemaPtr ECSchemaXmlDeserializer::_LocateSchema(BECN::SchemaKeyR key, BECN::SchemaMatchType matchType, BECN::ECSchemaReadContextR schemaContext)
    {
    BECN::ECSchemaP schema = m_schemaCache.GetSchema(key, matchType);
    if (schema != nullptr)
        return schema;

    for (auto kvPairs : m_schemaXmlMap)
        {
        auto schemaIter = kvPairs.second.begin();
        BECN::SchemaKey const& schemaKey = schemaIter->first;
        if (!schemaKey.Matches(key, matchType))
            continue;

        BECN::ECSchemaPtr leftSchema;
        if (BECN::SchemaReadStatus::Success != BECN::ECSchema::ReadFromXmlString(leftSchema, schemaIter->second.c_str(), schemaContext))
            return nullptr;

        if (kvPairs.second.size() == 1)
            {
            m_schemaCache.AddSchema(*leftSchema);
            return leftSchema;
            }

        if (key.GetName().Equals("EWR"))
            {
            leftSchema->SetName("EWR");
            return leftSchema;
            }

        m_converter.SetTaskName(Converter::ProgressMessage::TASK_MERGING_V8_ECSCHEMA(), kvPairs.first.c_str());
        schemaIter++;
        for (; schemaIter != kvPairs.second.end(); schemaIter++)
            {
//            ReportProgress();
            BECN::ECSchemaPtr rightSchema;
            if (BECN::SchemaReadStatus::Success != BECN::ECSchema::ReadFromXmlString(rightSchema, schemaIter->second.c_str(), schemaContext))
                continue;
            auto diff = ECDiff::Diff(*leftSchema, *rightSchema);
            if (diff->GetStatus() == DiffStatus::Success)
                {
                if (diff->IsEmpty())
                    continue;
                if (LOG.isSeverityEnabled(NativeLogging::SEVERITY::LOG_INFO))
                    {
                    Utf8String diffString;
                    diff->WriteToString(diffString, 1);
                    LOG.info("ECDiff: Legend [L] Added from left schema, [R] Added from right schema, [!] conflicting value");
                    LOG.info("=====================================[ECDiff Start]=====================================");
                    //LOG doesnt allow single large string
                    Utf8String eol = "\r\n";
                    Utf8String::size_type i = 0;
                    Utf8String::size_type j = diffString.find(eol, i);
                    while (j > i && j != Utf8String::npos)
                        {
                        Utf8String line = diffString.substr(i, j - i);
                        LOG.infov("> %s", line.c_str()); //print out the difference
                        i = j + eol.size();
                        j = diffString.find(eol, i);
                        }
                    LOG.info("=====================================[ECDiff End]=====================================");
                    }

                bmap<Utf8String, DiffNodeState> unitStates;
                if (diff->GetNodesState(unitStates, "*.CustomAttributes.Unit_Attributes:UnitSpecification") != DiffStatus::Success)
                    LOG.error("ECDiff: Error determining diff node state for UnitSpecification");
                if (diff->GetNodesState(unitStates, "*.CustomAttributes.Unit_Attributes:UnitSpecifications") != DiffStatus::Success)
                    LOG.error("ECDiff: Error determining diff node state for UnitSpecifications");
                ECN::ECSchemaPtr merged;
                if (diff->Merge(merged, CONFLICTRULE_TakeLeft) == MergeStatus::Success)
                    {
                    leftSchema = merged;
                    leftSchema->ComputeCheckSum();
                    LOG.infov("Merged two versions of ECSchema '%s' successfully. Updated checksum: %s", leftSchema->GetFullSchemaName().c_str(), leftSchema->GetSchemaKey().m_checksum.c_str());
                    }
                else
                    {
                    LOG.errorv("Merging two versions of ECSchema '%s' failed.", leftSchema->GetFullSchemaName().c_str());
                    continue;
                    }
                }
            }
        BECN::ECSchemaP match = nullptr;
        do
            {
            match = schemaContext.GetCache().GetSchema(schemaKey, BECN::SchemaMatchType::Latest);
            if (nullptr != match)
                schemaContext.GetCache().DropSchema(match->GetSchemaKey());
            } while (match != nullptr);

        schemaContext.AddSchema(*leftSchema);
        m_schemaCache.AddSchema(*leftSchema);
        return leftSchema;
        }

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            04/2017
//---------------+---------------+---------------+---------------+---------------+-------
void ECSchemaXmlDeserializer::AddSchemaXml(Utf8CP schemaName, ECN::SchemaKeyCR key, Utf8CP xml)
    {
    bvector<bpair<ECN::SchemaKey, Utf8String>> map = m_schemaXmlMap[schemaName];
    auto mapIter = map.begin();
    for (; mapIter != map.end(); mapIter++)
        {
        if (mapIter->first == key)
            return;
        }
    m_schemaXmlMap[schemaName].push_back({key, xml});
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     02/2015
//---------------------------------------------------------------------------------------
BentleyStatus ECSchemaXmlDeserializer::DeserializeSchemas(BECN::ECSchemaReadContextR schemaContext, BECN::SchemaMatchType matchType)
    {
    m_schemaCache.Clear();

    m_converter.AddTasks(m_schemaXmlMap.size());
    schemaContext.RemoveSchemaLocater(m_converter.GetDgnDb().GetSchemaLocater());
    //Prefer ECDb and standard schemas over ones embedded in DGN file.
    schemaContext.AddSchemaLocater(*this);
    schemaContext.AddSchemaLocater(m_converter.GetDgnDb().GetSchemaLocater());

    bvector<Utf8String> usedAliases;
    for (auto& kvPairs : m_schemaXmlMap)
        {
        auto schemaKey = kvPairs.second.begin()->first;
        auto schema = schemaContext.LocateSchema(schemaKey, matchType);
        if (schema == nullptr)
            {
            if (schemaKey.GetFullSchemaName().Contains("Supplemental"))
                {
                Utf8PrintfString warning("Failed to deserialize supplemental v8 schema '%s' - ignoring and continuing.", schemaKey.GetFullSchemaName().c_str());
                m_converter.ReportIssue(Converter::IssueSeverity::Warning, Converter::IssueCategory::Sync(), Converter::Issue::Message(), warning.c_str());
                continue;
                }
            Utf8String error;
            error.Sprintf("Failed to deserialize v8 ECSchema '%s'.", schemaKey.GetFullSchemaName().c_str());
            m_converter.ReportError(Converter::IssueCategory::Sync(), Converter::Issue::Message(), error.c_str());
            return BSIERROR;
            }

        if (std::find(usedAliases.begin(), usedAliases.end(), schema->GetAlias()) != usedAliases.end())
            {
            bool conflict = true;
            int32_t counter = 1;
            while (conflict)
                {
                Utf8PrintfString testNS("%s_%" PRId32 "", schema->GetAlias().c_str(), counter);
                if (std::find(usedAliases.begin(), usedAliases.end(), testNS) != usedAliases.end())
                    counter++;
                else
                    {
                    usedAliases.push_back(testNS.c_str());
                    schema->SetAlias(testNS.c_str());
                    conflict = false;
                    }
                }
            }
        else
            usedAliases.push_back(schema->GetAlias().c_str());
        }

    schemaContext.RemoveSchemaLocater(*this);
    return BSISUCCESS;
    }


//****************************************************************************************
// ECDiagnostics
//****************************************************************************************
//static initialization
bmap<ECDiagnostics::Category, NativeLogging::ILogger*> ECDiagnostics::s_loggerMap;

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     05/2015
//---------------------------------------------------------------------------------------
//static
void ECDiagnostics::LogV8InstanceDiagnostics(DgnV8EhCR v8eh, V8ElementType v8ElementType, ECClassName const& v8ClassName, bool isSecondaryInstancesClass, BisConversionRule conversionRule)
    {
    bool isFirstCall = false;
    NativeLogging::ILogger& logger = GetLogger(isFirstCall, ECDiagnostics::Category::V8Instances);

    if (isFirstCall)
        {
        //log header
        logger.message(s_severity, "V8 Element | V8 Element type | V8 Model | V8 ECClass | V8 primary or secondary instance class | BIS conversion rule");
        }

    if (logger.isSeverityEnabled(s_severity))
        {
        DgnV8ModelCP v8Model = v8eh.GetDgnModelP();
        Utf8String v8ModelStr = v8Model != nullptr ? Converter::IssueReporter::FmtModel(*v8Model) : "nullptr";

        logger.messagev(s_severity, "%s|%s|%s|%s|%s|%s",
                        Converter::IssueReporter::FmtElement(v8eh).c_str(),
                        V8ElementTypeHelper::ToString(v8ElementType),
                        v8ModelStr.c_str(),
                        v8ClassName.GetClassFullName().c_str(),
                        isSecondaryInstancesClass ? "Secondary instances class" : "Primary instances class",
                        BisConversionRuleHelper::ToString(conversionRule));
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     05/2015
//---------------------------------------------------------------------------------------
//static
void ECDiagnostics::LogV8RelationshipDiagnostics(DgnDbR dgndb, ECClassName const& v8RelClassName, V8ECInstanceKey const& sourceKey, bool sourceWasConverted, bool sourceConvertedToElement, V8ECInstanceKey const& targetKey, bool targetWasConverted, bool targetConvertedToElement)
    {
    bool isFirstCall = false;
    NativeLogging::ILogger& logger = GetLogger(isFirstCall, ECDiagnostics::Category::V8Relationships);

    if (isFirstCall)
        {
        //log header
        logger.message(s_severity, "V8 RelationshipClass | V8 Cardinality | V8 Source ECClass | v8 Source ECInstanceId | BIS type of converted V8 Target | V8 Target ECClass | V8 Target ECInstanceId | BIS type of converted V8 Target");
        }

    if (logger.isSeverityEnabled(s_severity))
        {
        //Utf8String sourceCardinalityStr, targetCardinalityStr;
        //
        //ECObjectsV8::RelationshipCardinality sourceCardinality;
        //ECObjectsV8::RelationshipCardinality targetCardinality;
        //if (V8ECRelationshipInfo::TryFindV8Cardinality(sourceCardinality, targetCardinality, dgndb, v8RelClassName))
        //    {
        //    sourceCardinalityStr = Utf8String(sourceCardinality.ToString().c_str());
        //    targetCardinalityStr = Utf8String(targetCardinality.ToString().c_str());
        //    }
        //else
        //    BeAssert(false && "Retrieving v8 relationship cardinality from V8ECRelationshipInfo should not fail.");

        logger.messagev(s_severity, "%s.%s|%s|%s|%s|%s|%s|%s",
                        v8RelClassName.GetSchemaName(), v8RelClassName.GetClassName(),
                        //sourceCardinalityStr.c_str(), targetCardinalityStr.c_str(),
                        sourceKey.GetClassName().GetClassFullName().c_str(),
                        sourceKey.GetInstanceId(),
                        ToBisTypeString(sourceWasConverted, sourceConvertedToElement),
                        targetKey.GetClassName().GetClassFullName().c_str(),
                        targetKey.GetInstanceId(),
                        ToBisTypeString(targetWasConverted, targetConvertedToElement)
        );
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     05/2015
//---------------------------------------------------------------------------------------
//static
Utf8CP ECDiagnostics::ToBisTypeString(bool isConverted, bool isElement)
    {
    if (!isConverted)
        return "Not converted";

    return isElement ? "Element" : "ElementAspect";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     05/2015
//---------------------------------------------------------------------------------------
//static
NativeLogging::ILogger& ECDiagnostics::GetLogger(bool& isFirstCallForCategory, Category category)
    {
    auto it = s_loggerMap.find(category);
    if (it == s_loggerMap.end())
        {
        isFirstCallForCategory = true;
        Utf8CP loggerName = nullptr;
        switch (category)
            {
                case ECDiagnostics::Category::V8Instances:
                    loggerName = "Diagnostics.DgnV8Converter.V8ECInstanceAnalysis";
                    break;
                case ECDiagnostics::Category::V8Relationships:
                    loggerName = "Diagnostics.DgnV8Converter.V8ECRelationshipAnalysis";
                    break;

                default:
                    BeAssert(false);
                    break;
            }

        NativeLogging::ILogger* logger = NativeLogging::LoggingManager::GetLogger(loggerName);
        s_loggerMap[category] = logger;
        return *logger;
        }

    isFirstCallForCategory = false;
    return *it->second;
    }

//=======================================================================================
// For case-insensitive UTF-8 string comparisons in STL collections that only use ASCII
// strings
// @bsistruct
//+===============+===============+===============+===============+===============+======
struct CompareIUtf8Ascii
    {
    bool operator()(Utf8CP s1, Utf8CP s2) const { return BeStringUtilities::StricmpAscii(s1, s2) < 0; }
    bool operator()(Utf8StringCR s1, Utf8StringCR s2) const { return BeStringUtilities::StricmpAscii(s1.c_str(), s2.c_str()) < 0; }
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   07/2015
//---------------------------------------------------------------------------------------
DynamicSchemaGenerator::SchemaConversionScope::SchemaConversionScope(DynamicSchemaGenerator& converter)
: m_converter(converter), m_succeeded(false)
    {
    m_converter.InitializeECSchemaConversion();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   07/2015
//---------------------------------------------------------------------------------------
DynamicSchemaGenerator::SchemaConversionScope::~SchemaConversionScope()
    {
    m_converter.FinalizeECSchemaConversion();
    if (!m_succeeded)
        {
        m_converter.SetEcConversionFailed();
        if (!m_converter.m_ecConversionFailedDueToLockingError)
            m_converter.ReportError(Converter::IssueCategory::Sync(), Converter::Issue::Error(), "Failed to transform the v8 ECSchemas to a BIS based ECSchema. Therefore EC content is not converted. See logs for details. Please try to adjust the v8 ECSchemas.");
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            06/2018
//---------------+---------------+---------------+---------------+---------------+-------
void RemoveDgnV8CustomAttributes(ECN::IECCustomAttributeContainerR container)
    {
    for (ECN::IECInstancePtr instance : container.GetCustomAttributes(false))
        {
        Utf8String v8SchemaName(instance->GetClass().GetSchema().GetName().c_str());
        auto found = std::find_if(s_dgnV8DeliveredSchemas.begin(), s_dgnV8DeliveredSchemas.end(), [v8SchemaName] (Utf8CP dgnv8) ->bool { return BeStringUtilities::StricmpAscii(v8SchemaName.c_str(), dgnv8) == 0; });
        if (found == s_dgnV8DeliveredSchemas.end())
            continue;
        container.RemoveCustomAttribute(instance->GetClass().GetSchema().GetName(), instance->GetClass().GetName());
        container.RemoveSupplementedCustomAttribute(instance->GetClass().GetSchema().GetName(), instance->GetClass().GetName());
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   11/2014
//---------------------------------------------------------------------------------------
BentleyApi::BentleyStatus DynamicSchemaGenerator::ConsolidateV8ECSchemas()
    {
    if (m_skipECContent)
        return BentleyApi::SUCCESS;

    ECSchemaXmlDeserializer schemaXmlDeserializer(*this);
    bset<Utf8String> targetSchemaNames;
//#define EXPORT_V8SCHEMA_XML 1
#ifdef EXPORT_V8SCHEMA_XML
    BeFileName bimFileName = GetDgnDb().GetFileName();
    BeFileName outFolder = bimFileName.GetDirectoryName().AppendToPath(bimFileName.GetFileNameWithoutExtension().AppendUtf8("_V8").c_str());
    if (!outFolder.DoesPathExist())
        BeFileName::CreateNewDirectory(outFolder.GetName());

#endif

    for (auto const& entry : V8ECSchemaXmlInfo::Iterable(GetDgnDb()))
        {
        BECN::SchemaKey key = entry.GetSchemaKey();
        Utf8String schemaName(key.GetName().c_str());
        if (0 == BeStringUtilities::Strnicmp("EWR", schemaName.c_str(), 3))
            {
            schemaName.AssignOrClear("EWR");
            key.m_schemaName = schemaName;
            }

        targetSchemaNames.insert(schemaName);
        Utf8CP schemaXml = entry.GetSchemaXml();
        if (entry.GetMappingType() == SyncInfo::ECSchemaMappingType::Dynamic)
            schemaXmlDeserializer.AddSchemaXml(schemaName.c_str(), key, schemaXml);
        else
            schemaXmlDeserializer.AddSchemaXml(key.GetFullSchemaName().c_str(), key, schemaXml);

#ifdef EXPORT_V8SCHEMA_XML
        WString fileName;
        fileName.AssignUtf8(key.GetFullSchemaName().c_str());
        fileName.append(L".ecschema.xml");

        BeFileName outPath(outFolder);
        outPath.AppendToPath(fileName.c_str());

        if (outPath.DoesPathExist())
            outPath.BeDeleteFile();
        BeFile outFile;
        outFile.Create(outPath.GetName());
        Utf8String xmlString(schemaXml);
        xmlString.ReplaceAll("UTF-16", "utf-8");
        outFile.Write(nullptr, xmlString.c_str(), static_cast<uint32_t>(xmlString.size()));
#endif

        }

     if (BentleyApi::SUCCESS != schemaXmlDeserializer.DeserializeSchemas(*m_schemaReadContext, ECN::SchemaMatchType::Latest))
         {
         ReportError(Converter::IssueCategory::Unknown(), Converter::Issue::ConvertFailure(), "Failed to merge dynamic V8 ECSchemas.");
         BeAssert(false && "Failed to merge dynamic V8 ECSchemas.");
         return BSIERROR;
         }

     if (BentleyApi::SUCCESS != SupplementV8ECSchemas())
         return BSIERROR;

     bvector<BECN::ECSchemaP> schemas;
     m_schemaReadContext->GetCache().GetSchemas(schemas);
     bvector<Utf8CP> schemasWithMultiInheritance = {"OpenPlant_3D", "BuildingDataGroup", "StructuralModelingComponents", "OpenPlant", "jclass", "pds", "group", 
         "ams", "bmf", "pid", "schematics", "OpenPlant_PID", "OpenPlant3D_PID", "speedikon", "autoplant_PIW", "ECXA_autoplant_PIW", "Bentley_Plant", "globals",
         "Electrical_RCM", "pid_ansi", "PDMx_Base", "TUAS", "HS2", "BMHDesigner"};
     bool needsFlattening = false;
     // It is possible that a schema will refer to one of the above schemas that needs flattening.  In such a situation, the reference needs to be updated to the flattened ref.  There is no
     // easy way to replace a referenced schema.  Therefore, if one of the schemas in the set needs to be flattened, we just flatten everything which automatically updates the references.
     for (BECN::ECSchemaP schema : schemas)
         {
         Utf8CP schemaName = schema->GetName().c_str();
         auto found = std::find_if(schemasWithMultiInheritance.begin(), schemasWithMultiInheritance.end(), [schemaName] (Utf8CP reserved) ->bool { return BeStringUtilities::StricmpAscii(schemaName, reserved) == 0; });
         if (found != schemasWithMultiInheritance.end())
             {
             needsFlattening = true;
             break;
             }
         else if (schema->GetName().StartsWithI("ECXA_"))
             {
             needsFlattening = true;
             break;
             }
         }

     if (needsFlattening)
         {
         bvector<BECN::ECSchemaP> schemasCopy;
         for (BECN::ECSchemaP schema : schemas)
             schemasCopy.push_back(schema);

         for (BECN::ECSchemaP schema : schemasCopy)
             if (BSISUCCESS != FlattenSchemas(schema))
                return BSIERROR;
         }

     schemas.clear();
     m_schemaReadContext->GetCache().GetSchemas(schemas);

//#define EXPORT_FLATTENEDECSCHEMAS 1
#ifdef EXPORT_FLATTENEDECSCHEMAS
     if (m_flattenedRefs.size() > 0)
         {
         BeFileName bimFileName = GetDgnDb().GetFileName();
         BeFileName outFolder = bimFileName.GetDirectoryName().AppendToPath(bimFileName.GetFileNameWithoutExtension().AppendUtf8("_flat").c_str());
         if (!outFolder.DoesPathExist())
             BeFileName::CreateNewDirectory(outFolder.GetName());

         for (const auto& sourceSchema : schemas)
             {
             WString fileName;
             fileName.AssignUtf8(sourceSchema->GetFullSchemaName().c_str());
             fileName.append(L".ecschema.xml");

             BeFileName outPath(outFolder);
             outPath.AppendToPath(fileName.c_str());

             if (outPath.DoesPathExist())
                 outPath.BeDeleteFile();

             sourceSchema->WriteToXmlFile(outPath.GetName());

             }
         }
#endif

     SpatialConverterBase* sc = dynamic_cast<SpatialConverterBase*>(&m_converter);
     DgnV8Api::StandardUnit standard = DgnV8Api::StandardUnit::MetricMeters;
     DgnV8Api::AngleMode angle = DgnV8Api::AngleMode::Degrees;
     if (nullptr != sc)
         {
         DgnV8ModelP rootModel = sc->GetRootModelP();
         DgnV8Api::ModelInfo const& info = rootModel->GetModelInfo();
         DgnV8Api::UnitDefinition masterUnit = info.GetMasterUnit();
         standard = masterUnit.IsStandardUnit();
         angle = info.GetAngularMode();
         }

     ECN::IECCustomAttributeConverterPtr extendType = new ExtendTypeConverter(standard, angle);
     ECN::ECSchemaConverter::AddConverter("EditorCustomAttributes", EXTEND_TYPE, extendType);

     for (BECN::ECSchemaP schema : schemas)
         {
         if (schema->IsSupplementalSchema())
             continue;
         if (BisClassConverter::SchemaConversionContext::ExcludeSchemaFromBisification(*schema))
             continue;
         if (!ECN::ECSchemaConverter::Convert(*schema, m_schemaReadContext.get(), false))
             {
             Utf8PrintfString error("Failed to run the schema converter on v8 ECSchema '%s'", schema->GetFullSchemaName().c_str());
             ReportError(Converter::IssueCategory::Sync(), Converter::Issue::Message(), error.c_str());
             return BentleyApi::BSIERROR;
             }
         RemoveDgnV8CustomAttributes(*schema);
         for (ECN::ECClassP ecClass : schema->GetClasses())
             {
             RemoveDgnV8CustomAttributes(*ecClass);
             for (ECN::ECPropertyP ecProp : ecClass->GetProperties())
                 RemoveDgnV8CustomAttributes(*ecProp);
             }
         }
     ECN::ECSchemaConverter::RemoveConverter(ECN::ECSchemaConverter::GetQualifiedClassName("EditorCustomAttributes", EXTEND_TYPE));

     return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2017
//---------------+---------------+---------------+---------------+---------------+-------
void DynamicSchemaGenerator::SwizzleOpenPlantSupplementals(bvector<BECN::ECSchemaPtr>& tmpSupplementals, BECN::ECSchemaP primarySchema, bvector<BECN::ECSchemaP> supplementalSchemas)
    {
    bool foundSupplemental = false;
    bvector<BECN::ECSchemaP> baseSupplementals;
    Utf8PrintfString suppName("%s_Supplemental", primarySchema->GetName().c_str());
    suppName.ReplaceAll("_3D", "");
    for (BECN::ECSchemaP supp : supplementalSchemas)
        {
        if (supp->GetName().StartsWithIAscii(suppName.c_str()))
            baseSupplementals.push_back(supp);
        }
    for (BECN::ECSchemaP supp : baseSupplementals)
        {
        BECN::ECSchemaPtr op3d;
        if (BECN::ECObjectsStatus::Success != supp->CopySchema(op3d))
            {
            Utf8String error;
            error.Sprintf("Failed to create an %s copy of the base supplemental schema '%s'; Supplemental information will be unavailable. See log file for details.", primarySchema->GetName().c_str(), (supp->GetName()).c_str());
            ReportIssue(Converter::IssueSeverity::Warning, Converter::IssueCategory::Sync(), Converter::Issue::Message(), error.c_str());
            continue;
            }

        Utf8String oldName(op3d->GetName().c_str());
        oldName.ReplaceAll("OpenPlant", primarySchema->GetName().c_str());
        op3d->SetName(oldName);
        BECN::SupplementalSchemaMetaDataPtr metaData;
        if (!BECN::SupplementalSchemaMetaData::TryGetFromSchema(metaData, *op3d))
            {
            Utf8String error;
            error.Sprintf("Failed to get supplemental metadata from supplemental schema '%s'; Supplemental information will be unavailable. See log file for details.", Utf8String(supp->GetName()).c_str());
            ReportIssue(Converter::IssueSeverity::Warning, Converter::IssueCategory::Sync(), Converter::Issue::Message(), error.c_str());
            continue;
            }
        BECN::IECInstancePtr instance = metaData->CreateCustomAttribute();
        op3d->RemoveCustomAttribute("Bentley_Standard_CustomAttributes", "SupplementalSchemaMetaData");
        Utf8String newName(metaData->GetPrimarySchemaName());
        newName.ReplaceAll("OpenPlant", primarySchema->GetName().c_str());
        BECN::SupplementalSchemaMetaDataPtr newMetaData = BECN::SupplementalSchemaMetaData::Create(newName.c_str(), metaData->GetPrimarySchemaReadVersion(), metaData->GetPrimarySchemaWriteVersion(),
                                                                                                    metaData->GetPrimarySchemaMinorVersion(), metaData->GetSupplementalSchemaPrecedence(), metaData->GetSupplementalSchemaPurpose().c_str());
        if (!ECN::ECSchema::IsSchemaReferenced(*op3d, instance->GetClass().GetSchema()))
            {
            BECN::ECClassP nonConstClass = const_cast<BECN::ECClassP>(&instance->GetClass());
            op3d->AddReferencedSchema(nonConstClass->GetSchemaR());
            }
        BECN::SupplementalSchemaMetaData::SetMetadata(*op3d, *newMetaData);
        tmpSupplementals.push_back(op3d);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   03/2015
//---------------------------------------------------------------------------------------
BentleyApi::BentleyStatus DynamicSchemaGenerator::SupplementV8ECSchemas()
    {
    DgnPlatformLib::Host* host = DgnPlatformLib::QueryHost();
    if (host == nullptr)
        {
        BeAssert(false && "Could not retrieve Graphite DgnPlatformLib Host.");
        return BSIERROR;
        }

    BeFileName supplementalECSchemasDir = GetParams().GetAssetsDir();
    supplementalECSchemasDir.AppendToPath(L"ECSchemas");
    supplementalECSchemasDir.AppendToPath(L"Supplemental");

    if (!supplementalECSchemasDir.DoesPathExist())
        {
        Utf8String error;
        error.Sprintf("Could not find deployed system supplemental ECSchemas.Directory '%s' does not exist.", supplementalECSchemasDir.GetNameUtf8().c_str());
        ReportIssue(Converter::IssueSeverity::Fatal, Converter::IssueCategory::Sync(), Converter::Issue::Message(), error.c_str());
        BeAssert(false && "Could not find deployed system supplemental ECSchemas.");
        return BSIERROR;
        }

    //Schemas from v8 file were retrieved via XML. In that case the primary schemas have not been supplemented by DgnECManager.
    //So we have to do it ourselves.
    bvector<BECN::ECSchemaP> primarySchemas;
    bvector<BECN::ECSchemaP> supplementalSchemas;
    bvector<BECN::ECSchemaP> schemas;
    m_schemaReadContext->GetCache().GetSchemas(schemas);
    for (BECN::ECSchemaP schema : schemas)
        {
        if (schema->IsSupplementalSchema())
            supplementalSchemas.push_back(schema);
        else if (!schema->IsStandardSchema())
            primarySchemas.push_back(schema);
        }

    BeFileName entryName;
    bool isDir = false;
    for (BeDirectoryIterator dirs(supplementalECSchemasDir); dirs.GetCurrentEntry(entryName, isDir) == SUCCESS; dirs.ToNext())
        {
        if (!isDir && FileNamePattern::MatchesGlob(entryName, L"*Supplemental*.ecschema.xml"))
            {
            BECN::ECSchemaPtr supplementalSchema = nullptr;
            if (BECN::SchemaReadStatus::Success != BECN::ECSchema::ReadFromXmlFile(supplementalSchema, entryName.GetName(), *m_schemaReadContext))
                {
                Utf8String error;
                error.Sprintf("Failed to read supplemental ECSchema from file '%s'.", entryName.GetNameUtf8().c_str());
                ReportIssue(Converter::IssueSeverity::Warning, Converter::IssueCategory::Sync(), Converter::Issue::Message(), error.c_str());
                continue;
                }

            supplementalSchemas.push_back(supplementalSchema.get ());
            }
        }

    bvector<BECN::ECSchemaPtr> tmpSupplementals;
    for (BECN::ECSchemaP primarySchema : primarySchemas)
        {
        if (primarySchema->IsSupplemented())
            {
            BeAssert(false && "V8 primary schemas are not expected to be supplemented already when deserialized from XML.");
            continue;
            }

        // Later versions of OP3D don't use a separate units schema for supplementation.  Instead, they share the OpenPlant version.  
        if (primarySchema->GetName().EqualsIAscii("OpenPlant_3D") || primarySchema->GetName().EqualsIAscii("OpenPlant_PID"))
            {
            SwizzleOpenPlantSupplementals(tmpSupplementals, primarySchema, supplementalSchemas);
            for (BECN::ECSchemaPtr supp : tmpSupplementals)
                supplementalSchemas.push_back(supp.get());
            }

        BECN::SupplementedSchemaBuilder builder;
        if (BECN::SupplementedSchemaStatus::Success != builder.UpdateSchema(*primarySchema, supplementalSchemas, false))
            {
            Utf8String error;
            error.Sprintf("Failed to supplement ECSchema '%s'. See log file for details.", Utf8String(primarySchema->GetName ()).c_str());
            ReportIssue(Converter::IssueSeverity::Warning, Converter::IssueCategory::Sync(), Converter::Issue::Message(), error.c_str());
            continue;
            }
        }

    //now remove the supp schemas from the read context as they are not needed anymore and as the read context will
    //be used for the ECDb schema import (where ECDb would attempt to supplement again if the supp schemas were still there)
    for (BECN::ECSchemaP suppSchema : supplementalSchemas)
        {
        m_schemaReadContext->GetCache().DropSchema(suppSchema->GetSchemaKey());
        }

    for (BECN::ECSchemaCP schema : schemas)
        {
        if (schema->GetName().EqualsIAscii("Unit_Attributes"))
            {
            m_schemaReadContext->GetCache().DropSchema(schema->GetSchemaKey());
            break;
            }
        }
    return BentleyApi::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2017
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus DynamicSchemaGenerator::CopyFlatCustomAttributes(ECN::IECCustomAttributeContainerR targetContainer, ECN::IECCustomAttributeContainerCR sourceContainer)
    {
    for (ECN::IECInstancePtr instance : sourceContainer.GetCustomAttributes(true))
        {
        if (instance->GetClass().GetName().Equals("CalculatedECPropertySpecification") && instance->GetClass().GetSchema().GetName().Equals("Bentley_Standard_CustomAttributes"))
            continue;

        ECN::ECSchemaPtr flatCustomAttributeSchema = m_flattenedRefs[instance->GetClass().GetSchema().GetName()];
        if (!flatCustomAttributeSchema.IsValid())
            {
            Utf8String error;
            error.Sprintf("Failed to find ECSchema '%s' for custom attribute '%'.  Skipping custom attribute.", instance->GetClass().GetFullName());
            ReportIssue(Converter::IssueSeverity::Warning, Converter::IssueCategory::Sync(), Converter::Issue::Message(), error.c_str());
            continue;
            }
        ECN::IECInstancePtr copiedCA = instance->CreateCopyThroughSerialization(*flatCustomAttributeSchema);
        if (!copiedCA.IsValid())
            {
            Utf8String error;
            error.Sprintf("Failed to copy custom attribute '%s'. Skipping custom attribute.", instance->GetClass().GetFullName());
            ReportIssue(Converter::IssueSeverity::Warning, Converter::IssueCategory::Sync(), Converter::Issue::Message(), error.c_str());
            continue;
            }
        if (!ECN::ECSchema::IsSchemaReferenced(*targetContainer.GetContainerSchema(), *flatCustomAttributeSchema))
            targetContainer.GetContainerSchema()->AddReferencedSchema(*flatCustomAttributeSchema);

        targetContainer.SetCustomAttribute(*copiedCA);
        }
    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2017
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus DynamicSchemaGenerator::CopyFlatConstraint(ECN::ECRelationshipConstraintR toRelationshipConstraint, ECN::ECRelationshipConstraintCR fromRelationshipConstraint)
    {
    if (fromRelationshipConstraint.IsRoleLabelDefined() && ECN::ECObjectsStatus::Success != toRelationshipConstraint.SetRoleLabel(fromRelationshipConstraint.GetInvariantRoleLabel().c_str()))
        return BSIERROR;
    if (ECN::ECObjectsStatus::Success != toRelationshipConstraint.SetMultiplicity(fromRelationshipConstraint.GetMultiplicity()))
        return BSIERROR;
    if (ECN::ECObjectsStatus::Success != toRelationshipConstraint.SetIsPolymorphic(fromRelationshipConstraint.GetIsPolymorphic()))
        return BSIERROR;

    ECN::ECSchemaP destSchema = const_cast<ECN::ECSchemaP>(&(toRelationshipConstraint.GetRelationshipClass().GetSchema()));

    for (auto constraintClass : fromRelationshipConstraint.GetConstraintClasses())
        {
        if (fromRelationshipConstraint.GetRelationshipClass().GetSchema().GetSchemaKey() != constraintClass->GetSchema().GetSchemaKey())
            {
            ECN::ECSchemaPtr flatBaseSchema = m_flattenedRefs[constraintClass->GetSchema().GetName()];
            if (ECN::ECObjectsStatus::Success != toRelationshipConstraint.AddClass(*flatBaseSchema->GetClassP(constraintClass->GetName().c_str())->GetEntityClassCP()))
                return BSIERROR;
            }
        else
            {
            ECN::ECClassP destConstraintClass = destSchema->GetClassP(constraintClass->GetName().c_str());
            if (nullptr == destConstraintClass)
                {
                // All classes should already have been created
                return BSIERROR;
                }
            if (ECN::ECObjectsStatus::Success != toRelationshipConstraint.AddClass(*destConstraintClass->GetEntityClassCP()))
                return BSIERROR;
            }
        }
    CopyFlatCustomAttributes(toRelationshipConstraint, fromRelationshipConstraint);
    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2017
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus DynamicSchemaGenerator::CreateFlatClass(ECN::ECClassP& targetClass, ECN::ECSchemaP flatSchema, ECN::ECClassCP sourceClass)
    {
    ECN::ECRelationshipClassCP sourceAsRelationshipClass = sourceClass->GetRelationshipClassCP();
    ECN::ECStructClassCP sourceAsStructClass = sourceClass->GetStructClassCP();
    ECN::ECCustomAttributeClassCP sourceAsCAClass = sourceClass->GetCustomAttributeClassCP();
    if (nullptr != sourceAsRelationshipClass)
        {
        ECN::ECRelationshipClassP newRelationshipClass;
        if (ECN::ECObjectsStatus::Success != flatSchema->CreateRelationshipClass(newRelationshipClass, sourceClass->GetName()))
            return BSIERROR;
        newRelationshipClass->SetStrength(sourceAsRelationshipClass->GetStrength());
        newRelationshipClass->SetStrengthDirection(sourceAsRelationshipClass->GetStrengthDirection());

        CopyFlatConstraint(newRelationshipClass->GetSource(), sourceAsRelationshipClass->GetSource());
        CopyFlatConstraint(newRelationshipClass->GetTarget(), sourceAsRelationshipClass->GetTarget());
        targetClass = newRelationshipClass;
        }
    else if (nullptr != sourceAsStructClass)
        {
        ECN::ECStructClassP newStructClass;
        if (ECN::ECObjectsStatus::Success != flatSchema->CreateStructClass(newStructClass, sourceClass->GetName()))
            return BSIERROR;
        targetClass = newStructClass;
        }
    else if (nullptr != sourceAsCAClass)
        {
        ECN::ECCustomAttributeClassP newCAClass;
        if (ECN::ECObjectsStatus::Success != flatSchema->CreateCustomAttributeClass(newCAClass, sourceClass->GetName()))
            return BSIERROR;
        newCAClass->SetContainerType(sourceAsCAClass->GetContainerType());
        targetClass = newCAClass;
        }
    else
        {
        ECN::ECEntityClassP newEntityClass;
        if (ECN::ECObjectsStatus::Success != flatSchema->CreateEntityClass(newEntityClass, sourceClass->GetName()))
            return BSIERROR;
        targetClass = newEntityClass;
        }

    if (sourceClass->GetIsDisplayLabelDefined())
        targetClass->SetDisplayLabel(sourceClass->GetInvariantDisplayLabel());
    targetClass->SetDescription(sourceClass->GetInvariantDescription());
    targetClass->SetClassModifier(sourceClass->GetClassModifier());

    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2017
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus DynamicSchemaGenerator::CopyFlatClass(ECN::ECClassP& targetClass, ECN::ECSchemaP flatSchema, ECN::ECClassCP sourceClass)
    {
    if (BSISUCCESS != CreateFlatClass(targetClass, flatSchema, sourceClass))
        return BSIERROR;

    for (ECN::ECPropertyCP sourceProperty : sourceClass->GetProperties(true))
        {
        if (BSISUCCESS != CopyFlattenedProperty(targetClass, sourceProperty))
            return BSIERROR;
        }

    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2017
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus DynamicSchemaGenerator::CopyFlattenedProperty(ECN::ECClassP targetClass, ECN::ECPropertyCP sourceProperty)
    {
    // Only copy properties either directly on the same class or on a base class that was dropped.  Don't copy properties from base classes that are still set
    if (0 != strcmp(targetClass->GetFullName(), sourceProperty->GetClass().GetFullName()))
        {
        ECN::ECClassP targetPropertyClass = nullptr;
        ECN::ECSchemaPtr flatSchema = m_flattenedRefs[sourceProperty->GetClass().GetSchema().GetName()];
        if (!flatSchema.IsValid())
            {
            if (Utf8String(sourceProperty->GetClass().GetSchema().GetName()).StartsWithIAscii("SP3D"))
                targetPropertyClass = targetClass->GetSchemaR().GetClassP(sourceProperty->GetClass().GetName().c_str());
            else
                return BSIERROR;
            }
        else
            targetPropertyClass = flatSchema->GetClassP(sourceProperty->GetClass().GetName().c_str());
        //if (targetClass->Is(targetPropertyClass))
        //    return BSISUCCESS;
        }

    ECN::ECPropertyP destProperty = nullptr;
    if (sourceProperty->GetIsPrimitive())
        {
        ECN::PrimitiveECPropertyP destPrimitive;
        ECN::PrimitiveECPropertyCP sourcePrimitive = sourceProperty->GetAsPrimitiveProperty();
        ECN::ECEnumerationCP enumeration = sourcePrimitive->GetEnumeration();
        if (nullptr != enumeration)
            {
            if (enumeration->GetSchema().GetSchemaKey() == sourceProperty->GetClass().GetSchema().GetSchemaKey())
                {
                ECN::ECEnumerationP destEnum = targetClass->GetSchemaR().GetEnumerationP(enumeration->GetName().c_str());
                if (nullptr == destEnum)
                    {
                    auto status = targetClass->GetSchemaR().CopyEnumeration(destEnum, *enumeration);
                    if (ECN::ECObjectsStatus::Success != status && ECN::ECObjectsStatus::NamedItemAlreadyExists != status)
                        return BSIERROR;
                    }
                targetClass->CreateEnumerationProperty(destPrimitive, sourceProperty->GetName(), *destEnum);
                }
            else
                {
                ECN::ECSchemaPtr flatBaseSchema = m_flattenedRefs[enumeration->GetSchema().GetName()];
                targetClass->CreateEnumerationProperty(destPrimitive, sourceProperty->GetName(), *flatBaseSchema->GetEnumerationP(enumeration->GetName().c_str()));
                }
            }
        else
            targetClass->CreatePrimitiveProperty(destPrimitive, sourceProperty->GetName(), sourcePrimitive->GetType());

        if (sourcePrimitive->IsMinimumValueDefined())
            {
            ECN::ECValue valueToCopy;
            sourcePrimitive->GetMinimumValue(valueToCopy);
            destPrimitive->SetMinimumValue(valueToCopy);
            }

        if (sourcePrimitive->IsMaximumValueDefined())
            {
            ECN::ECValue valueToCopy;
            sourcePrimitive->GetMaximumValue(valueToCopy);
            destPrimitive->SetMaximumValue(valueToCopy);
            }

        if (sourcePrimitive->IsMinimumLengthDefined())
            destPrimitive->SetMinimumLength(sourcePrimitive->GetMinimumLength());
        if (sourcePrimitive->IsMaximumLengthDefined())
            destPrimitive->SetMaximumLength(sourcePrimitive->GetMaximumLength());

        if (sourcePrimitive->IsExtendedTypeDefinedLocally())
            destPrimitive->SetExtendedTypeName(sourcePrimitive->GetExtendedTypeName().c_str());
        destProperty = destPrimitive;
        }
    else if (sourceProperty->GetIsStructArray())
        {
        ECN::StructArrayECPropertyP destArray;
        ECN::StructArrayECPropertyCP sourceArray = sourceProperty->GetAsStructArrayProperty();
        ECN::ECStructClassCR structElementType = sourceArray->GetStructElementType();
        if (structElementType.GetSchema().GetSchemaKey() == targetClass->GetSchema().GetSchemaKey())
            {
            ECN::ECClassP destClass = targetClass->GetSchemaR().GetClassP(structElementType.GetName().c_str());
            if (nullptr == destClass)
                {
                auto status = CopyFlatClass(destClass, &(targetClass->GetSchemaR()), &structElementType);
                if (BSISUCCESS != status)
                    return BSIERROR;
                }
            targetClass->CreateStructArrayProperty(destArray, sourceProperty->GetName(), *destClass->GetStructClassCP());
            }
        else
            {
            ECN::ECSchemaPtr flatBaseSchema = m_flattenedRefs[structElementType.GetSchema().GetName()];
            targetClass->CreateStructArrayProperty(destArray, sourceProperty->GetName(), *flatBaseSchema->GetClassP(structElementType.GetName().c_str())->GetStructClassP());
            }

        destArray->SetMaxOccurs(sourceArray->GetStoredMaxOccurs());
        destArray->SetMinOccurs(sourceArray->GetMinOccurs());
        destProperty = destArray;
        }
    else if (sourceProperty->GetIsPrimitiveArray())
        {
        ECN::PrimitiveArrayECPropertyP destArray;
        ECN::PrimitiveArrayECPropertyCP sourceArray = sourceProperty->GetAsPrimitiveArrayProperty();
        ECN::ECEnumerationCP enumeration = sourceArray->GetEnumeration();
        if (nullptr != enumeration)
            {
            if (enumeration->GetSchema().GetSchemaKey() == sourceProperty->GetClass().GetSchema().GetSchemaKey())
                {
                ECN::ECEnumerationP destEnum = targetClass->GetSchemaR().GetEnumerationP(enumeration->GetName().c_str());
                if (nullptr == destEnum)
                    {
                    auto status = targetClass->GetSchemaR().CopyEnumeration(destEnum, *enumeration);
                    if (ECN::ECObjectsStatus::Success != status && ECN::ECObjectsStatus::NamedItemAlreadyExists != status)
                        return BSIERROR;
                    }
                targetClass->CreatePrimitiveArrayProperty(destArray, sourceProperty->GetName(), *destEnum);
                }
            else
                {
                ECN::ECSchemaPtr flatBaseSchema = m_flattenedRefs[enumeration->GetSchema().GetName()];
                targetClass->CreatePrimitiveArrayProperty(destArray, sourceProperty->GetName(), *flatBaseSchema->GetEnumerationP(enumeration->GetName().c_str()));
                }
            }
        else
            targetClass->CreatePrimitiveArrayProperty(destArray, sourceProperty->GetName(), sourceArray->GetPrimitiveElementType());

        if (sourceArray->IsMinimumValueDefined())
            {
            ECN::ECValue valueToCopy;
            sourceArray->GetMinimumValue(valueToCopy);
            destArray->SetMinimumValue(valueToCopy);
            }

        if (sourceArray->IsMaximumValueDefined())
            {
            ECN::ECValue valueToCopy;
            sourceArray->GetMaximumValue(valueToCopy);
            destArray->SetMaximumValue(valueToCopy);
            }

        if (sourceArray->IsMinimumLengthDefined())
            destArray->SetMinimumLength(sourceArray->GetMinimumLength());
        if (sourceArray->IsMaximumLengthDefined())
            destArray->SetMaximumLength(sourceArray->GetMaximumLength());

        if (sourceArray->IsExtendedTypeDefinedLocally())
            destArray->SetExtendedTypeName(sourceArray->GetExtendedTypeName().c_str());

        destArray->SetMaxOccurs(sourceArray->GetStoredMaxOccurs());
        destArray->SetMinOccurs(sourceArray->GetMinOccurs());
        destProperty = destArray;
        }
    else if (sourceProperty->GetIsStruct())
        {
        ECN::StructECPropertyP destStruct;
        ECN::StructECPropertyCP sourceStruct = sourceProperty->GetAsStructProperty();
        ECN::ECStructClassCR sourceType = sourceStruct->GetType();
        if (sourceType.GetSchema().GetSchemaKey() == sourceProperty->GetClass().GetSchema().GetSchemaKey())
            {
            ECN::ECClassP destClass = targetClass->GetSchemaR().GetClassP(sourceType.GetName().c_str());
            if (nullptr == destClass)
                {
                auto status = CopyFlatClass(destClass, &(targetClass->GetSchemaR()), &sourceType);
                if (BSISUCCESS != status)
                    return BSIERROR;
                }
            targetClass->CreateStructProperty(destStruct, sourceProperty->GetName(), *destClass->GetStructClassP());
            }
        else
            {
            ECN::ECSchemaPtr flatBaseSchema = m_flattenedRefs[sourceType.GetSchema().GetName()];
            if (!ECN::ECSchema::IsSchemaReferenced(targetClass->GetSchema(), *flatBaseSchema))
                targetClass->GetSchemaR().AddReferencedSchema(*flatBaseSchema);
            targetClass->CreateStructProperty(destStruct, sourceProperty->GetName(), *flatBaseSchema->GetClassP(sourceType.GetName().c_str())->GetStructClassP());
            }
        destProperty = destStruct;
        }
    else if (sourceProperty->GetIsNavigation())
        {
        ECN::NavigationECPropertyP destNav;
        ECN::NavigationECPropertyCP sourceNav = sourceProperty->GetAsNavigationProperty();

        ECN::ECRelationshipClassCP sourceRelClass = sourceNav->GetRelationshipClass();
        if (sourceRelClass->GetSchema().GetSchemaKey() == sourceProperty->GetClass().GetSchema().GetSchemaKey())
            {
            ECN::ECClassP destClass = targetClass->GetSchemaR().GetClassP(sourceRelClass->GetName().c_str());
            if (nullptr == destClass)
                {
                auto status = CopyFlatClass(destClass, &(targetClass->GetSchemaR()), sourceRelClass);
                if (BSISUCCESS != status)
                    return BSIERROR;
                }
            targetClass->GetEntityClassP()->CreateNavigationProperty(destNav, sourceProperty->GetName(), *destClass->GetRelationshipClassCP(), sourceNav->GetDirection(), false);
            }
        else
            {
            ECN::ECSchemaPtr flatBaseSchema = m_flattenedRefs[sourceRelClass->GetSchema().GetName()];
            targetClass->GetEntityClassP()->CreateNavigationProperty(destNav, sourceProperty->GetName(), *flatBaseSchema->GetClassP(sourceRelClass->GetName().c_str())->GetRelationshipClassP(), sourceNav->GetDirection(), false);
            }
        destProperty = destNav;
        }

    destProperty->SetDescription(sourceProperty->GetInvariantDescription());
    if (sourceProperty->GetIsDisplayLabelDefined())
        destProperty->SetDisplayLabel(sourceProperty->GetInvariantDisplayLabel());
    destProperty->SetIsReadOnly(sourceProperty->IsReadOnlyFlagSet());
    destProperty->SetPriority(sourceProperty->GetPriority());

    if (sourceProperty->IsCategoryDefinedLocally())
        {
        ECN::PropertyCategoryCP sourcePropCategory = sourceProperty->GetCategory();
        if (sourcePropCategory->GetSchema().GetSchemaKey() == sourceProperty->GetClass().GetSchema().GetSchemaKey())
            {
            ECN::PropertyCategoryP destPropCategory = targetClass->GetSchemaR().GetPropertyCategoryP(sourcePropCategory->GetName().c_str());
            if (nullptr == destPropCategory)
                {
                auto status = targetClass->GetSchemaR().CopyPropertyCategory(destPropCategory, *sourcePropCategory);
                if (ECN::ECObjectsStatus::Success != status && ECN::ECObjectsStatus::NamedItemAlreadyExists != status)
                    return BSIERROR;
                }
            destProperty->SetCategory(destPropCategory);
            }
        else
            {
            ECN::ECSchemaPtr flatBaseSchema = m_flattenedRefs[sourcePropCategory->GetSchema().GetName()];
            destProperty->SetCategory(flatBaseSchema->GetPropertyCategoryP(sourcePropCategory->GetName().c_str()));
            }
        }

    if (sourceProperty->IsKindOfQuantityDefinedLocally())
        {
        ECN::KindOfQuantityCP sourceKoq = sourceProperty->GetKindOfQuantity();
        if (sourceKoq->GetSchema().GetSchemaKey() == sourceProperty->GetClass().GetSchema().GetSchemaKey())
            {
            ECN::KindOfQuantityP destKoq = targetClass->GetSchemaR().GetKindOfQuantityP(sourceKoq->GetName().c_str());
            if (nullptr == destKoq)
                {
                auto status = targetClass->GetSchemaR().CopyKindOfQuantity(destKoq, *sourceKoq);
                if (ECN::ECObjectsStatus::Success != status && ECN::ECObjectsStatus::NamedItemAlreadyExists != status)
                    return BSIERROR;
                }
            destProperty->SetKindOfQuantity(destKoq);
            }
        else
            {
            ECN::ECSchemaPtr flatBaseSchema = m_flattenedRefs[sourceKoq->GetSchema().GetName()];
            destProperty->SetKindOfQuantity(flatBaseSchema->GetKindOfQuantityP(sourceKoq->GetName().c_str()));
            }
        }

    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2017
//---------------+---------------+---------------+---------------+---------------+-------
void verifyDerivedClassesNotAbstract(ECN::ECClassP ecClass)
    {
    for (ECN::ECClassP derivedClass : ecClass->GetDerivedClasses())
        {
        if (ECN::ECClassModifier::Abstract == derivedClass->GetClassModifier())
            derivedClass->SetClassModifier(ECN::ECClassModifier::None);
        verifyDerivedClassesNotAbstract(derivedClass);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2017
//---------------+---------------+---------------+---------------+---------------+-------
void verifyBaseClassAbstract(ECN::ECClassP ecClass)
    {
    // There are cases out there where a base class is non-abstract and has instances, yet a derived class (generally in another schema) is set to abstract.  Therefore, instead of setting
    // the base classes Abstract, the derived class must be set as non-abstract
    for (ECN::ECClassP baseClass : ecClass->GetBaseClasses())
        {
        if (BisClassConverter::SchemaConversionContext::ExcludeSchemaFromBisification(baseClass->GetSchema()))
            continue;
        if (ECN::ECClassModifier::Abstract != baseClass->GetClassModifier() && ECN::ECClassModifier::Abstract == ecClass->GetClassModifier())
            {
            ecClass->SetClassModifier(ECN::ECClassModifier::None);
            verifyDerivedClassesNotAbstract(ecClass);
            }
        verifyBaseClassAbstract(baseClass);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2017
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus DynamicSchemaGenerator::FlattenSchemas(ECN::ECSchemaP ecSchema)
    {
    bvector<BECN::ECSchemaP> schemas;
    ecSchema->FindAllSchemasInGraph(schemas, true);


    for (ECN::ECSchemaP sourceSchema : schemas)
        {
        if (BisClassConverter::SchemaConversionContext::ExcludeSchemaFromBisification(*sourceSchema))
            {
            m_flattenedRefs[sourceSchema->GetName()] = sourceSchema;
            continue;
            }

        // SP3D schemas are processed later.  The only way we got here is if there are other schemas in the dgn that get flattened, like mixing OpenPlant and SP3d
        if (sourceSchema->GetName().StartsWithIAscii("SP3D"))
            {
            m_flattenedRefs[sourceSchema->GetName()] = sourceSchema;
            continue;
            }

        if (m_flattenedRefs.find(sourceSchema->GetName()) != m_flattenedRefs.end())
            continue;

        ECN::ECSchemaPtr flatSchema;
        ECN::ECSchema::CreateSchema(flatSchema, sourceSchema->GetName(), sourceSchema->GetAlias(), sourceSchema->GetVersionRead(), sourceSchema->GetVersionWrite(), sourceSchema->GetVersionMinor(), sourceSchema->GetECVersion());
        m_flattenedRefs[flatSchema->GetName()] = flatSchema.get();
        flatSchema->SetOriginalECXmlVersion(2, 0);

        ECN::ECSchemaReferenceListCR referencedSchemas = sourceSchema->GetReferencedSchemas();
        for (ECN::ECSchemaReferenceList::const_iterator it = referencedSchemas.begin(); it != referencedSchemas.end(); ++it)
            {
            ECN::ECSchemaPtr flatRefSchema = m_flattenedRefs[it->second->GetName()];
            flatSchema->AddReferencedSchema(*flatRefSchema);
            }

        bvector<ECN::ECClassCP> relationshipClasses;
        for (ECN::ECClassCP sourceClass : sourceSchema->GetClasses())
            {
            ECN::ECClassP targetClass = nullptr;
            if (sourceClass->IsRelationshipClass())
                {
                relationshipClasses.push_back(sourceClass);
                continue;
                }
            CreateFlatClass(targetClass, flatSchema.get(), sourceClass);
            }

        // Need to make sure that all constraint classes are already created
        for (ECN::ECClassCP sourceClass : relationshipClasses)
            {
            ECN::ECClassP targetClass = nullptr;
            CreateFlatClass(targetClass, flatSchema.get(), sourceClass);
            }

        ECN::IECInstancePtr flattenedInstance = ECN::ConversionCustomAttributeHelper::CreateCustomAttributeInstance("IsFlattened");
        if (flattenedInstance.IsValid())
            {
            if (!ECN::ECSchema::IsSchemaReferenced(flattenedInstance->GetClass().GetSchema(), *flatSchema))
                {
                ECN::ECClassCR constClass = flattenedInstance->GetClass();
                ECN::ECClassP nonConst = const_cast<ECN::ECClassP>(&constClass);
                flatSchema->AddReferencedSchema(nonConst->GetSchemaR());
                }
            flatSchema->SetCustomAttribute(*flattenedInstance);
            }

        for (ECN::ECClassCP sourceClass : sourceSchema->GetClasses())
            {
            ECN::ECClassP targetClass = flatSchema->GetClassP(sourceClass->GetName().c_str());

            const ECN::ECBaseClassesList& baseClasses = sourceClass->GetBaseClasses();
            int totalBaseClasses = 0;
            int baseClassesFromSchema = 0;
            for (ECN::ECClassP sourceBaseClass : baseClasses)
                {
                totalBaseClasses++;
                if (sourceBaseClass->GetSchema().GetName().EqualsIAscii(sourceClass->GetSchema().GetName().c_str()))
                    baseClassesFromSchema++;
                }

            if (totalBaseClasses == 1)
                {
                for (ECN::ECClassP sourceBaseClass : baseClasses)
                    {
                    ECN::ECSchemaPtr flatBaseSchema = m_flattenedRefs[sourceBaseClass->GetSchema().GetName()];
                    if (!flatBaseSchema.IsValid())
                        return BSIERROR;
                    targetClass->AddBaseClass(*flatBaseSchema->GetClassCP(sourceBaseClass->GetName().c_str()), false, false, false);
                    }
                }
            else if (baseClassesFromSchema < 2)
                {
                for (ECN::ECClassP sourceBaseClass : baseClasses)
                    {
                    if (sourceBaseClass->GetSchema().GetName().EqualsIAscii(sourceClass->GetSchema().GetName().c_str()))
                        {
                        targetClass->AddBaseClass(*flatSchema->GetClassCP(sourceBaseClass->GetName().c_str()), false, false, false);
                        }
                    else
                        {
                        ECN::ECSchemaPtr flatBaseSchema = m_flattenedRefs[sourceBaseClass->GetSchema().GetName()];
                        if (!flatBaseSchema.IsValid())
                            continue;
                        ECN::ECClassP flatBase = flatBaseSchema->GetClassP(sourceBaseClass->GetName().c_str());
                        BisClassConverter::AddDroppedDerivedClass(flatBase, targetClass);
                        }
                    }
                }
            else if (totalBaseClasses > 1)
                {
                for (ECN::ECClassP baseClass : baseClasses)
                    {
                    ECN::ECSchemaPtr flatBaseSchema = m_flattenedRefs[baseClass->GetSchema().GetName()];
                    if (!flatBaseSchema.IsValid())
                        continue;
                    ECN::ECClassP flatBase = flatBaseSchema->GetClassP(baseClass->GetName().c_str());
                    BisClassConverter::AddDroppedDerivedClass(flatBase, targetClass);
                    }
                }
            if (targetClass->GetClassModifier() == ECN::ECClassModifier::Abstract)
                verifyBaseClassAbstract(targetClass);
            }

        // This needs to happen after all of the baseclasses have been set.
        for (ECN::ECClassCP sourceClass : sourceSchema->GetClasses())
            {
            ECN::ECClassP targetClass = flatSchema->GetClassP(sourceClass->GetName().c_str());
            for (ECN::ECPropertyCP sourceProperty : sourceClass->GetProperties(true))
                {
                if (BSISUCCESS != CopyFlattenedProperty(targetClass, sourceProperty))
                    return BSIERROR;
                }
            }
        // This needs to happen after we have copied all of the properties for all of the classes as the custom attributes could be defined locally
        CopyFlatCustomAttributes(*flatSchema, *sourceSchema);

        for (ECN::ECClassCP sourceClass : sourceSchema->GetClasses())
            {
            ECN::ECClassP targetClass = flatSchema->GetClassP(sourceClass->GetName().c_str());
            CopyFlatCustomAttributes(*targetClass, *sourceClass);
            for (ECN::ECPropertyCP sourceProperty : sourceClass->GetProperties(true))
                {
                ECN::ECPropertyP targetProperty = targetClass->GetPropertyP(sourceProperty->GetName().c_str());
                CopyFlatCustomAttributes(*targetProperty, *sourceProperty);
                }
            }
        }

    for (ECN::ECSchemaP sourceSchema : schemas)
        {
        Utf8String sourceSchemaName(sourceSchema->GetName().c_str());
        m_schemaReadContext->GetCache().DropSchema(sourceSchema->GetSchemaKey());
        m_schemaReadContext->AddSchema(*m_flattenedRefs[sourceSchemaName]);
        }

    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2017
//---------------+---------------+---------------+---------------+---------------+-------
void DynamicSchemaGenerator::ProcessSP3DSchema(ECN::ECSchemaP schema, ECN::ECClassCP baseInterface, ECN::ECClassCP baseObject)
    {
    bool wasFlattened = false;
    // CopyFlattenedProperty looks in m_flattenedRefs, so we need to prepopulate that
    ECN::ECSchemaReferenceListCR referencedSchemas = schema->GetReferencedSchemas();
    for (ECN::ECSchemaReferenceList::const_iterator it = referencedSchemas.begin(); it != referencedSchemas.end(); ++it)
        {
        ECN::ECSchemaP refSchema = it->second.get();
        m_flattenedRefs[refSchema->GetName()] = refSchema;
        }

    for (BECN::ECClassP ecClass : schema->GetClasses())
        {
        BECN::ECEntityClassP entityClass = ecClass->GetEntityClassP();
        // Classes derived from BaseObject can have multiple BaseInterface-derived base classes, but only one BaseObject base class
        // Classes derived from BaseInterface can only have one base class
        if (nullptr == entityClass)
            continue;
        if (!ecClass->Is(baseInterface) && !ecClass->Is(baseObject))
            continue;
        else if (ecClass->HasBaseClasses())
            {
            bool isInterface = ecClass->Is(baseInterface) && !ecClass->Is(baseObject);
            int baseClassCounter = 0;
            bvector<ECN::ECClassP> toRemove;
            for (auto& baseClass : ecClass->GetBaseClasses())
                {
                ECN::ECEntityClassCP asEntity = baseClass->GetEntityClassCP();
                if (nullptr == asEntity)
                    continue;
                else if (!isInterface && !asEntity->Is(baseObject))
                    continue;
                baseClassCounter++;
                if (baseClassCounter > 1)
                    toRemove.push_back(baseClass);
                }
            for (auto& baseClass : toRemove)
                {
                ecClass->RemoveBaseClass(*baseClass);
                BisClassConverter::AddDroppedDerivedClass(baseClass, ecClass);
                for (ECN::ECPropertyCP sourceProperty : baseClass->GetProperties(true))
                    {
                    if (BisClassConverter::SchemaConversionContext::ExcludeSchemaFromBisification(sourceProperty->GetClass().GetSchema()))
                        continue;

                    if (nullptr != ecClass->GetPropertyP(sourceProperty->GetName().c_str(), true))
                        continue;
                    if (BSISUCCESS != CopyFlattenedProperty(ecClass, sourceProperty))
                        return;
                    }
                wasFlattened = true;
                }
            }

        if (ecClass->GetClassModifier() == ECN::ECClassModifier::Abstract)
            verifyBaseClassAbstract(ecClass);
        }

    if (wasFlattened)
        {
        ECN::IECInstancePtr flattenedInstance = ECN::ConversionCustomAttributeHelper::CreateCustomAttributeInstance("IsFlattened");
        if (flattenedInstance.IsValid())
            {
            if (!ECN::ECSchema::IsSchemaReferenced(flattenedInstance->GetClass().GetSchema(), *schema))
                {
                ECN::ECClassCR constClass = flattenedInstance->GetClass();
                ECN::ECClassP nonConst = const_cast<ECN::ECClassP>(&constClass);
                schema->AddReferencedSchema(nonConst->GetSchemaR());
                }
            schema->SetCustomAttribute(*flattenedInstance);
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Sam.Wilson                      07/14
//+---------------+---------------+---------------+---------------+---------------+------
void DynamicSchemaGenerator::AnalyzeECContent(DgnV8ModelR v8Model, BisConversionTargetModelInfoCR targetModelInfo)
    {
    if (m_skipECContent)
        return;

    uint64_t count = 0;

    DgnV8Api::PersistentElementRefList* graphicElements = v8Model.GetGraphicElementsP();
    if (nullptr != graphicElements)
        {
        for (DgnV8Api::PersistentElementRef* v8Element : *graphicElements)
            {
            if ((++count % 1000) == 0)
                ReportProgress();

            DgnV8Api::ElementHandle v8eh(v8Element);
            //TODO if (_FilterElement(v8eh))
            //TODO     continue;

            Analyze(v8eh, targetModelInfo);
            }
        }

    DgnV8Api::PersistentElementRefList* controlElems = v8Model.GetControlElementsP();
    if (nullptr != controlElems)
        {
        for (DgnV8Api::PersistentElementRef* v8Element : *controlElems)
            {
            if ((++count % 1000) == 0)
                ReportProgress();

            DgnV8Api::ElementHandle v8eh(v8Element);
            //TODO if (_FilterElement(v8eh))
            //TODO     continue;

            Analyze(v8eh, targetModelInfo);
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     03/2015
//---------------------------------------------------------------------------------------
BentleyApi::BentleyStatus DynamicSchemaGenerator::Analyze(DgnV8Api::ElementHandle const& v8Element, BisConversionTargetModelInfoCR targetModelInfo)
    {
    DoAnalyze(v8Element, targetModelInfo);
    //recurse into component elements (if the element has any)
    for (DgnV8Api::ChildElemIter childIt(v8Element); childIt.IsValid(); childIt = childIt.ToNext())
        {
        Analyze(childIt, targetModelInfo);
        }

    return BentleyApi::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     03/2015
//---------------------------------------------------------------------------------------
BentleyApi::BentleyStatus DynamicSchemaGenerator::DoAnalyze(DgnV8Api::ElementHandle const& v8Element, BisConversionTargetModelInfoCR targetModelInfo)
    {
    auto& v8ECManager = DgnV8Api::DgnECManager::GetManager();
    DgnV8Api::ElementECClassInfo classes;
    v8ECManager.FindECClassesOnElement(v8Element.GetElementRef(), classes);
    for (auto& ecClassInfo : classes)
        {
        auto& ecClass = ecClassInfo.first;
        bool isPrimary = ecClassInfo.second;
        
        Utf8String v8SchemaName(ecClass.m_schemaName.c_str());
        auto found = std::find_if(s_dgnV8DeliveredSchemas.begin(), s_dgnV8DeliveredSchemas.end(), [v8SchemaName] (Utf8CP dgnv8) ->bool { return BeStringUtilities::StricmpAscii(v8SchemaName.c_str(), dgnv8) == 0; });
        if (found != s_dgnV8DeliveredSchemas.end())
            continue;

        auto skipped = std::find_if(m_skippedSchemas.begin(), m_skippedSchemas.end(), [v8SchemaName] (Utf8StringCR dgnv8) ->bool { return BeStringUtilities::StricmpAscii(v8SchemaName.c_str(), dgnv8.c_str()) == 0; });
        if (skipped != m_skippedSchemas.end())
            continue;

        // We fabricate the DgnV8 Tag Set Definition schema at runtime during conversion; never allow instances of that schema to be considered primary.
        if (isPrimary && ecClass.m_schemaName.Equals(Converter::GetV8TagSetDefinitionSchemaName()))
            isPrimary = false;
        
        ECClassName v8ClassName(v8SchemaName.c_str(), Utf8String(ecClass.m_className.c_str()).c_str());
        ECN::SchemaKey conversionKey(Utf8String(v8ClassName.GetSchemaName()).append("_DgnDbSync").c_str(), 1, 0);
        ECN::ECSchemaPtr conversionSchema = m_syncReadContext->LocateSchema(conversionKey, ECN::SchemaMatchType::Latest);
        bool namedGroupOwnsMembers = false;
        if (conversionSchema.IsValid())
            {
            ECN::ECClassCP ecClass = conversionSchema->GetClassCP(v8ClassName.GetClassName());
            if (nullptr != ecClass)
                namedGroupOwnsMembers = ecClass->GetCustomAttribute("NamedGroupOwnsMembers") != nullptr;
            }
        if (0 == BeStringUtilities::Strnicmp("EWR", v8ClassName.GetSchemaName(), 3))
            v8ClassName = ECClassName("EWR", v8ClassName.GetClassName());

        if (BentleyApi::SUCCESS != V8ECClassInfo::Insert(*this, v8Element, v8ClassName, namedGroupOwnsMembers, !isPrimary, targetModelInfo))
            return BSIERROR;
        if (!isPrimary && (BentleyApi::SUCCESS != V8ElementSecondaryECClassInfo::Insert(GetDgnDb(), v8Element, v8ClassName)))
            return BSIERROR;
        m_hasECContent = true;
        }

    return BentleyApi::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   10/2014
//---------------------------------------------------------------------------------------
BentleyApi::BentleyStatus DynamicSchemaGenerator::ConvertToBisBasedECSchemas()
    {
    BisClassConverter::SchemaConversionContext context(*this, *m_schemaReadContext, *m_syncReadContext, GetConfig().GetOptionValueBool("AutoDetectMixinParams", true));

    if (BentleyApi::SUCCESS != BisClassConverter::PreprocessConversion(context))
        return BentleyApi::ERROR;

    bset<BECN::ECClassP> rootClasses;
    bset<BECN::ECRelationshipClassP> relationshipClasses;

    // SP3D schemas can have multi-inheritance in addition to the mixins.  We need to remove any subsequent base classes
    for (bpair<Utf8String, BECN::ECSchemaP> const& kvpair : context.GetSchemas())
        {
        BECN::ECSchemaP schema = kvpair.second;
        if (schema->GetName().StartsWithIAscii("SP3D"))
            {
            BisClassConverter::SchemaConversionContext::MixinContext* mixinContext = context.GetMixinContext(*schema);
            if (mixinContext == nullptr)
                continue;

            BECN::ECClassCP baseInterface = mixinContext->first;
            BECN::ECClassCP baseObject = mixinContext->second;
            ProcessSP3DSchema(schema, baseInterface, baseObject);
            }
        }

    for (bpair<Utf8String, BECN::ECSchemaP> const& kvpair : context.GetSchemas())
        {
        BECN::ECSchemaP schema = kvpair.second;
        //only interested in the domain schemas, so skip standard, system and supp schemas
        if (context.ExcludeSchemaFromBisification(*schema))
            continue;

        for (BECN::ECClassP ecClass : schema->GetClasses())
            {
            BECN::ECRelationshipClassP relClass = ecClass->GetRelationshipClassP();
            if (relClass != nullptr && !relClass->HasBaseClasses())
                relationshipClasses.insert(relClass);
            else if (ecClass->HasBaseClasses())
                continue;
            else
                rootClasses.insert(ecClass);
            }
        }

    for (BECN::ECClassP rootClass : rootClasses)
        {
        ECClassName rootClassName(*rootClass);
        if (BisClassConverter::ConvertECClass(context, rootClassName) != BentleyApi::SUCCESS)
            return BentleyApi::BSIERROR;
        }

    for (BECN::ECClassP rootClass : rootClasses)
        {
        BisClassConverter::CheckForMixinConversion(context, *rootClass);
        }

    if (BisClassConverter::FinalizeConversion(context) != BentleyApi::SUCCESS)
        return BentleyApi::BSIERROR;

    BisClassConverter::ECClassRemovalContext removeContext(context);
    for (BECN::ECRelationshipClassP relationshipClass : relationshipClasses)
        {
        if (BisClassConverter::ConvertECRelationshipClass(removeContext, *relationshipClass, m_syncReadContext.get()) != BentleyApi::SUCCESS)
            return BentleyApi::BSIERROR;
        }

    for (BECN::ECClassP droppedClass : removeContext.GetClasses())
        {
        if (BECN::ECObjectsStatus::Success != droppedClass->GetSchemaR().DeleteClass(*droppedClass))
            return BSIERROR;
        }

    for (bpair<Utf8String, BECN::ECSchemaP> const& kvpair : context.GetSchemas())
        {
        BECN::ECSchemaP schema = kvpair.second;
        if (context.ExcludeSchemaFromBisification(*schema))
            continue;

        if (!schema->Validate(true) || !schema->IsECVersion(ECN::ECVersion::V3_2))
            {
//#define EXPORT_FAILEDECSCHEMAS 1
#ifdef EXPORT_FAILEDECSCHEMAS
                    {
                    BeFileName bimFileName = GetDgnDb().GetFileName();
                    BeFileName outFolder = bimFileName.GetDirectoryName().AppendToPath(bimFileName.GetFileNameWithoutExtension().AppendUtf8("_failed").c_str());

                    if (!outFolder.DoesPathExist())
                        BeFileName::CreateNewDirectory(outFolder.GetName());

                    WString fileName;
                    fileName.AssignUtf8(schema->GetFullSchemaName().c_str());
                    fileName.append(L".ecschema.xml");

                    BeFileName outPath(outFolder);
                    outPath.AppendToPath(fileName.c_str());

                    if (outPath.DoesPathExist())
                        outPath.BeDeleteFile();

                    schema->WriteToXmlFile(outPath.GetName(), schema->GetECVersion());
                    }
#endif

            Utf8String errorMsg;
            errorMsg.Sprintf("Failed to validate ECSchema %s as an EC3.1 ECSchema.", schema->GetFullSchemaName().c_str());
            ReportIssue(Converter::IssueSeverity::Error, Converter::IssueCategory::Sync(), Converter::Issue::Message(), errorMsg.c_str());
            return BentleyApi::BSIERROR;
            }
        Bentley::WString schemaName(schema->GetName().c_str());
        if (DgnV8Api::ItemTypeLibrary::IsItemTypeSchema(schemaName) && !schema->IsDynamicSchema())
            {
            ECN::IECInstancePtr dynamicInstance = ECN::CoreCustomAttributeHelper::CreateCustomAttributeInstance("DynamicSchema");
            if (!ECN::ECSchema::IsSchemaReferenced(*schema, dynamicInstance->GetClass().GetSchema()))
                {
                ECN::ECClassCR dynClass = dynamicInstance->GetClass();
                ECN::ECClassP nonConst = const_cast<ECN::ECClassP>(&dynClass);
                schema->AddReferencedSchema(nonConst->GetSchemaR());
                }
            schema->SetCustomAttribute(*dynamicInstance);
            }
        }

    return BentleyApi::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* Returns true if thisSchema directly references possiblyReferencedSchema
* @bsimethod                                 Ramanujam.Raman                05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
static bool DirectlyReferences(ECN::ECSchemaCP thisSchema, ECN::ECSchemaCP possiblyReferencedSchema)
    {
    ECN::ECSchemaReferenceListCR referencedSchemas = thisSchema->GetReferencedSchemas();
    for (ECN::ECSchemaReferenceList::const_iterator it = referencedSchemas.begin(); it != referencedSchemas.end(); ++it)
        {
        if (it->second.get() == possiblyReferencedSchema)
            return true;
        }
    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Casey.Mullen      01/2013
//---------------------------------------------------------------------------------------
static bool DependsOn(ECN::ECSchemaCP thisSchema, ECN::ECSchemaCP possibleDependency)
    {
    if (DirectlyReferences(thisSchema, possibleDependency))
        return true;

    ECN::SupplementalSchemaMetaDataPtr metaData;
    if (ECN::SupplementalSchemaMetaData::TryGetFromSchema(metaData, *possibleDependency)
        && metaData.IsValid()
        && metaData->IsForPrimarySchema(thisSchema->GetName(), 0, 0, ECN::SchemaMatchType::Latest))
        {
        return true; // possibleDependency supplements thisSchema. possibleDependency must be imported before thisSchema
        }

    // Maybe possibleDependency supplements one of my references?
    ECN::ECSchemaReferenceListCR referencedSchemas = thisSchema->GetReferencedSchemas();
    for (ECN::ECSchemaReferenceList::const_iterator it = referencedSchemas.begin(); it != referencedSchemas.end(); ++it)
        {
        if (DependsOn(it->second.get(), possibleDependency))
            return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Ramanujam.Raman                07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
static void InsertSchemaInDependencyOrderedList(bvector<ECN::ECSchemaP>& schemas, ECN::ECSchemaP insertSchema)
    {
    if (std::find(schemas.begin(), schemas.end(), insertSchema) != schemas.end())
        return; // This (and its referenced ECSchemas) are already in the list

    bvector<ECN::ECSchemaP>::reverse_iterator rit;
    for (rit = schemas.rbegin(); rit < schemas.rend(); ++rit)
        {
        if (DependsOn(*rit, insertSchema))
            {
            schemas.insert(rit.base(), insertSchema); // insert right after the referenced schema in the list
            return;
            }
        }

    schemas.insert(schemas.begin(), insertSchema); // insert at the beginning
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Ramanujam.Raman                07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
static void BuildDependencyOrderedSchemaList(bvector<ECN::ECSchemaP>& schemas, ECN::ECSchemaP insertSchema)
    {
    InsertSchemaInDependencyOrderedList(schemas, insertSchema);
    ECN::ECSchemaReferenceListCR referencedSchemas = insertSchema->GetReferencedSchemas();
    for (ECN::ECSchemaReferenceList::const_iterator iter = referencedSchemas.begin(); iter != referencedSchemas.end(); ++iter)
        {
        ECN::ECSchemaR referencedSchema = *iter->second.get();
        BuildDependencyOrderedSchemaList(schemas, &referencedSchema);
        }
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   10/2014
//---------------------------------------------------------------------------------------
BentleyApi::BentleyStatus DynamicSchemaGenerator::ImportTargetECSchemas()
    {
    bvector<BECN::ECSchemaP> schemas;
    m_schemaReadContext->GetCache().GetSchemas(schemas);
    if (schemas.empty())
        return BentleyApi::SUCCESS; //no schemas to import

    // need to ensure there are no duplicated aliases.  Also need to remove unused references
    bset<Utf8String, CompareIUtf8Ascii> prefixes;
    for (ECN::ECSchemaP schema : schemas)
        {
        schema->RemoveUnusedSchemaReferences();

        auto it = prefixes.find(schema->GetAlias());
        if (it == prefixes.end())
            {
            prefixes.insert(schema->GetAlias());
            continue;
            }
        int subScript;
        Utf8String tryPrefix;
        for (subScript = 1; subScript < 500; subScript++)
            {
            Utf8Char temp[256];
            tryPrefix.clear();
            tryPrefix.Sprintf("%s%d", schema->GetAlias().c_str(), subScript);
            it = prefixes.find(tryPrefix);
            if (it == prefixes.end())
                {
                schema->SetAlias(tryPrefix);
                prefixes.insert(tryPrefix);
                break;
                }
            }
        }

    bvector<BECN::ECSchemaCP> constSchemas = m_schemaReadContext->GetCache().GetSchemas();
    auto removeAt = std::remove_if(constSchemas.begin(), constSchemas.end(), [&] (BECN::ECSchemaCP const& arg) { return arg->IsStandardSchema() || arg->IsSystemSchema(); });
    constSchemas.erase(removeAt, constSchemas.end());

    auto removeDgn = std::remove_if(constSchemas.begin(), constSchemas.end(), [&] (BECN::ECSchemaCP const& arg)
        {
        Utf8String v8SchemaName = arg->GetName();
        auto found = std::find_if(s_dgnV8DeliveredSchemas.begin(), s_dgnV8DeliveredSchemas.end(), [v8SchemaName] (Utf8CP dgnv8) ->bool { return BeStringUtilities::StricmpAscii(v8SchemaName.c_str(), dgnv8) == 0; });
        return (found != s_dgnV8DeliveredSchemas.end());
        });

    constSchemas.erase(removeDgn, constSchemas.end());
//#define EXPORT_BISIFIEDECSCHEMAS 1
#ifdef EXPORT_BISIFIEDECSCHEMAS
    {
    BeFileName bimFileName = GetDgnDb().GetFileName();
    BeFileName outFolder = bimFileName.GetDirectoryName().AppendToPath(bimFileName.GetFileNameWithoutExtension().c_str());
    if (!outFolder.DoesPathExist())
        BeFileName::CreateNewDirectory(outFolder.GetName());

    for (BECN::ECSchemaCP schema : constSchemas)
        {
        WString fileName;
        fileName.AssignUtf8(schema->GetFullSchemaName().c_str());
        fileName.append(L".ecschema.xml");

        BeFileName outPath(outFolder);
        outPath.AppendToPath(fileName.c_str());

        if (outPath.DoesPathExist())
            outPath.BeDeleteFile();

        schema->WriteToXmlFile(outPath.GetName());
        }
    }
#endif

    auto importStatus = GetDgnDb().ImportV8LegacySchemas(constSchemas);
    if (SchemaStatus::Success != importStatus)
        {
        //By design ECDb must not do transaction management itself. A failed schema import can have changed the dgndb though. 
        //So we must abandon these changes.
        //(Cannot use Savepoints, as the change tracker might be enabled)
        GetDgnDb().AbandonChanges();
        m_ecConversionFailedDueToLockingError = (SchemaStatus::SchemaLockFailed == importStatus);
        auto cat = Converter::IssueCategory::Briefcase();
        auto issue = (SchemaStatus::SchemaLockFailed == importStatus)           ? Converter::Issue::SchemaLockFailed():
                     (SchemaStatus::CouldNotAcquireLocksOrCodes == importStatus)? Converter::Issue::CouldNotAcquireLocksOrCodes():
                                                                                  Converter::Issue::ImportTargetECSchemas();
        ReportError(cat, issue, "");        // NB! This is NOT a fatal error! This should NOT abort the converter!
        return BentleyApi::ERROR;
        }

    m_anyImported = true;

    if (GetConfig().GetOptionValueBool("ValidateSchemas", false))
        {
        StopWatch timer(true);
        ValidateSchemas(constSchemas);
        ConverterLogging::LogPerformance(timer, "Convert Schemas> Validate V8 ECSchemas");
        }
    return BentleyApi::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            12/2017
//---------------+---------------+---------------+---------------+---------------+-------
void DynamicSchemaGenerator::ValidateSchemas(bvector<BECN::ECSchemaCP>& importedSchemas)
    {

    for (BECN::ECSchemaCP importedSchema : importedSchemas)
        {
        bmap<Utf8String, ECObjectsV8::ECSchemaPtr>::const_iterator it = m_v8Schemas.find(importedSchema->GetName());
        if (it == m_v8Schemas.end())
            continue;

        SchemaRemapper mapper(m_converter);

        for (auto& v8class : it->second->GetClasses())
            {
            ECObjectsV8::ECRelationshipClassCP relClass = v8class->GetRelationshipClassCP();
            if (nullptr != relClass)
                continue;
            BentleyApi::ECN::ECClassCP ecClass = importedSchema->GetClassCP(Utf8String(v8class->GetName().c_str()).c_str());
            if (nullptr == ecClass)
                {
                Utf8PrintfString aspectName("%s%s", Utf8String(v8class->GetName().c_str()).c_str(), BIS_CLASS_ElementAspect);
                ecClass = importedSchema->GetClassCP(aspectName.c_str());
                if (nullptr == ecClass)
                    {
                    LOG.warningv("Unable to find entity class %s in imported schema", Utf8String(v8class->GetFullName()).c_str());
                    continue;
                    }
                }
            for (auto& v8prop : v8class->GetProperties(true))
                {
                BentleyApi::ECN::ECPropertyP prop = ecClass->GetPropertyP(v8prop->GetName().c_str());
                if (nullptr == prop)
                    {
                    Utf8String v8PropName(v8prop->GetName().c_str());
                    if (mapper.ResolvePropertyName(v8PropName, *ecClass))
                        prop = ecClass->GetPropertyP(v8PropName);
                    if (nullptr == prop)
                        LOG.warningv("Schema Validation: Failed to find %s:%s", Utf8String(v8class->GetFullName()).c_str(), v8PropName.c_str());
                    }
                }
            }

        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   10/2014
//---------------------------------------------------------------------------------------
BentleyApi::BentleyStatus DynamicSchemaGenerator::RetrieveV8ECSchemas(DgnV8ModelR v8Model)
    {
    SetTaskName(Converter::ProgressMessage::TASK_READING_V8_ECSCHEMA(), Utf8String(v8Model.GetDgnFileP()->GetFileName().c_str()).c_str());
    if (BentleyApi::SUCCESS != RetrieveV8ECSchemas(v8Model, DgnV8Api::ECSCHEMAPERSISTENCE_Stored))
        return BentleyApi::ERROR;
    return RetrieveV8ECSchemas(v8Model, DgnV8Api::ECSCHEMAPERSISTENCE_External);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   10/2014
//---------------------------------------------------------------------------------------
BentleyApi::BentleyStatus DynamicSchemaGenerator::RetrieveV8ECSchemas(DgnV8ModelR v8Model, DgnV8Api::ECSchemaPersistence persistence)
    {
    auto& dgnv8EC = DgnV8Api::DgnECManager::GetManager();
    const DgnV8Api::ReferencedModelScopeOption modelScopeOption = DgnV8Api::REFERENCED_MODEL_SCOPE_None;

    Bentley::bvector<DgnV8Api::SchemaInfo> v8SchemaInfos;
    dgnv8EC.DiscoverSchemasForModel(v8SchemaInfos, v8Model, persistence, modelScopeOption);
    if (v8SchemaInfos.empty())
        return BentleyApi::SUCCESS;

    for (auto& v8SchemaInfo : v8SchemaInfos)
        {
        ReportProgress();

        ECObjectsV8::SchemaKey& schemaKey = v8SchemaInfo.GetSchemaKeyR();
        if (LOG.isSeverityEnabled(NativeLogging::SEVERITY::LOG_TRACE))
            LOG.tracev(L"Schema %ls - File: %ls - Location: %ls - %ls - Provider: %ls", schemaKey.GetFullSchemaName().c_str(),
                       v8SchemaInfo.GetDgnFile().GetFileName().c_str(),
                       v8SchemaInfo.GetLocation(),
                       v8SchemaInfo.IsStoredSchema() ? L"Stored" : L"External",
                       v8SchemaInfo.GetProviderName());

        //TODO: Need to filter out V8/MicroStation specific ECSchemas, not needed in Graphite

        Utf8String fullName(schemaKey.GetFullSchemaName().c_str());
        if (!m_converter.ShouldImportSchema(fullName, v8Model))
            {
            m_skippedSchemas.push_back(Utf8String(schemaKey.GetName().c_str()));
            continue;
            }

        Bentley::Utf8String schemaName(schemaKey.GetName());

        bool isDynamicSchema = false;
        Bentley::Utf8String schemaXml;
        if (v8SchemaInfo.IsStoredSchema())
            {
            Bentley::WString schemaXmlW;
            auto stat = dgnv8EC.LocateSchemaXmlInModel(schemaXmlW, v8SchemaInfo, ECObjectsV8::SCHEMAMATCHTYPE_Exact, v8Model, modelScopeOption);
            if (stat != BentleyApi::SUCCESS)
                {
                ReportError(Converter::IssueCategory::Unknown(), Converter::Issue::ConvertFailure(), "Could not read v8 ECSchema XML.");
                BeAssert(false && "Could not locate v8 ECSchema.");
                return BSIERROR;
                }

            schemaXml = Bentley::Utf8String(schemaXmlW);
            const size_t xmlByteSize = schemaXmlW.length() * sizeof(WChar);
            schemaKey.m_checkSum = ECObjectsV8::ECSchema::ComputeSchemaXmlStringCheckSum(schemaXmlW.c_str(), xmlByteSize);

            isDynamicSchema = IsDynamicSchema(schemaName, schemaXml);

            // If we're validating schemas, then we need to retrieve the actual V8 schema
            if (GetConfig().GetOptionValueBool("ValidateSchemas", false))
                {
                if (m_v8Schemas.find(schemaName.c_str()) == m_v8Schemas.end())
                    {
                    ECObjectsV8::ECSchemaPtr v8Schema = dgnv8EC.LocateSchemaInDgnFile(v8SchemaInfo, ECObjectsV8::SCHEMAMATCHTYPE_Exact, true);
                    if (v8Schema.IsValid())
                        m_v8Schemas[schemaName.c_str()] = v8Schema;
                    }
                }
            }
        else
            {
            if (0 != wcscmp(L"External", v8SchemaInfo.GetLocation())) // System schemas are returned by the 'External' persistence designation
                continue;
            // handle external schemas
            //TBD: DgnECManager doesn't seem to allow to just return the schema xml (Is this possible somehow?)
            //So we need to get the ECSchema and then serialize it to a string.
            //(we need the string anyways as this is the only way to marshal the schema from v8 to Graphite)
            auto externalSchema = dgnv8EC.LocateExternalSchema(v8SchemaInfo, ECObjectsV8::SCHEMAMATCHTYPE_LatestCompatible);
            if (externalSchema == nullptr)
                {
                Utf8PrintfString error("Could not locate external v8 ECSchema %s.  Instances of classes from this schema will not be converted.", schemaName.c_str());
                ReportIssueV(Converter::IssueSeverity::Warning, Converter::IssueCategory::MissingData(), Converter::Issue::Message(), nullptr, error.c_str());
                continue;
                }

            isDynamicSchema = IsDynamicSchema(*externalSchema);

            if (ECObjectsV8::SCHEMA_WRITE_STATUS_Success != externalSchema->WriteToXmlString(schemaXml))
                {
                Utf8PrintfString error("Could not serialize external v8 ECSchema %s.  Instances of classes from this schema will not be converted.", schemaName.c_str());
                ReportIssueV(Converter::IssueSeverity::Warning, Converter::IssueCategory::MissingData(), Converter::Issue::Message(), nullptr, error.c_str());
                continue;
                }
            if (BSIERROR == ProcessReferenceSchemasFromExternal(*externalSchema, v8Model))
                return BSIERROR;
            if (GetConfig().GetOptionValueBool("ValidateSchemas", false))
                {
                if (m_v8Schemas.find(schemaName.c_str()) == m_v8Schemas.end())
                    {
                    m_v8Schemas[schemaName.c_str()] = externalSchema;
                    }
                ECObjectsV8::ECSchemaReferenceListCR referencedSchemas = externalSchema->GetReferencedSchemas();
                for (ECObjectsV8::ECSchemaReferenceList::const_iterator it = referencedSchemas.begin(); it != referencedSchemas.end(); ++it)
                    {
                    if (m_v8Schemas.find(Utf8String(it->second->GetName().c_str())) == m_v8Schemas.end())
                        {
                        m_v8Schemas[Utf8String(it->second->GetName().c_str())] = it->second;
                        }
                    }
                }
            }

        if (BSIERROR == ProcessSchemaXml(schemaKey, schemaXml.c_str(), isDynamicSchema, v8Model))
            return BSIERROR;
        }
    m_skipECContent = false;

    return BentleyApi::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2017
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus DynamicSchemaGenerator::ProcessReferenceSchemasFromExternal(ECObjectsV8::ECSchemaCR schema, DgnV8ModelR v8Model)
    {

    ECObjectsV8::ECSchemaReferenceListCR referencedSchemas = schema.GetReferencedSchemas();
    for (ECObjectsV8::ECSchemaReferenceList::const_iterator it = referencedSchemas.begin(); it != referencedSchemas.end(); ++it)
        {
        ECObjectsV8::ECSchemaPtr refSchema = it->second;
        Bentley::Utf8String refSchemaXml;
        if (ECObjectsV8::SCHEMA_WRITE_STATUS_Success != refSchema->WriteToXmlString(refSchemaXml))
            {
            Utf8PrintfString error("Could not serialize externally referenced v8 ECSchema %s.  Instances of classes from this schema will not be converted.", refSchema->GetName().c_str());
            ReportIssueV(Converter::IssueSeverity::Warning, Converter::IssueCategory::MissingData(), Converter::Issue::Message(), nullptr, error.c_str());
            return BSIERROR;
            }
        if (BSISUCCESS != ProcessSchemaXml(refSchema->GetSchemaKey(), refSchemaXml.c_str(), IsDynamicSchema(*refSchema), v8Model))
            return BSIERROR;
        ProcessReferenceSchemasFromExternal(*refSchema, v8Model);
        }
    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2017
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus DynamicSchemaGenerator::ProcessSchemaXml(const ECObjectsV8::SchemaKey& schemaKey, Utf8CP schemaXml, bool isDynamicSchema, DgnV8ModelR v8Model)
    {
    Utf8String schemaName(schemaKey.GetName().c_str());

    ECObjectsV8::SchemaKey existingSchemaKey;
    SyncInfo::ECSchemaMappingType existingMappingType = SyncInfo::ECSchemaMappingType::Identity;
    
    if (GetSyncInfo().TryGetECSchema(existingSchemaKey, existingMappingType, schemaName.c_str(), RepositoryLinkId()))
        {
        //ECSchema with same name already found in other model. Now check whether we need to overwrite the existing one or not
        //and also check whether the existing one and the new one are compatible.

        if (existingMappingType == SyncInfo::ECSchemaMappingType::Dynamic)
            {
            if (!isDynamicSchema)
                {
                Utf8String error;
                error.Sprintf("Dynamic ECSchema %s already found in the V8 file. Copy in model %s is not dynamic and therefore ignored.",
                              schemaName.c_str(), Converter::IssueReporter::FmtModel(v8Model).c_str());
                ReportIssue(Converter::IssueSeverity::Warning, Converter::IssueCategory::Sync(), Converter::Issue::Message(), error.c_str());
                return BSISUCCESS;
                }
            }
        else
            {
            if (isDynamicSchema)
                {
                Utf8String error;
                error.Sprintf("Non-dynamic ECSchema %s already found in the V8 file. Copy in model %s is dynamic and therefore ignored.",
                              Utf8String(existingSchemaKey.GetFullSchemaName().c_str()).c_str(), Converter::IssueReporter::FmtModel(v8Model).c_str());
                ReportIssue(Converter::IssueSeverity::Warning, Converter::IssueCategory::Sync(), Converter::Issue::Message(), error.c_str());
                return BSISUCCESS;
                }

            const int majorDiff = existingSchemaKey.GetVersionMajor() - schemaKey.GetVersionMajor();
            const int minorDiff = existingSchemaKey.GetVersionMinor() - schemaKey.GetVersionMinor();
            const int existingToNewVersionDiff = majorDiff != 0 ? majorDiff : minorDiff;

            if (existingToNewVersionDiff >= 0)
                {
                if (existingToNewVersionDiff == 0 && existingSchemaKey.m_checkSum != schemaKey.m_checkSum)
                    {
                    Utf8String error;
                    error.Sprintf("ECSchema %s already found in the V8 file with a different checksum (%u). Copy in model %s with checksum %u will be merged.  This may result in inconsistencies between the DgnDb version and the versions in the Dgn.",
                                  Utf8String(existingSchemaKey.GetFullSchemaName().c_str()).c_str(), existingSchemaKey.m_checkSum,
                                  Converter::IssueReporter::FmtModel(v8Model).c_str(), schemaKey.m_checkSum);
                    ReportIssue(Converter::IssueSeverity::Warning, Converter::IssueCategory::Sync(), Converter::Issue::Message(), error.c_str());
                    }
                else
                    {
                    // If on a previous conversion we found the schema but it wasn't used, it will have an entry in the SyncInfo table but won't actually exist in the dgndb.  We still need to import it
                    if (m_converter.IsUpdating())
                        {
                        if (GetDgnDb().Schemas().GetSchema(schemaName, false) != nullptr)
                            {
                            return BSISUCCESS;
                            }
                        }
                    else if (GetSyncInfo().TryGetECSchema(existingSchemaKey, existingMappingType, schemaName.c_str(), Converter::GetRepositoryLinkIdFromAppData(*v8Model.GetDgnFileP())))
                        return BSISUCCESS;
                    }
                }
            }
        }

    ECN::ECSchemaId schemaId;
    if (BE_SQLITE_OK != GetSyncInfo().InsertECSchema(schemaId, *v8Model.GetDgnFileP(),
                                                     schemaName.c_str(),
                                                     schemaKey.GetVersionMajor(),
                                                     schemaKey.GetVersionMinor(),
                                                     isDynamicSchema,
                                                     schemaKey.m_checkSum))
        {
        BeAssert(false && "Failed to insert ECSchema sync info");
        return BSIERROR;
        }

    BeAssert(schemaId.IsValid());

    if (BE_SQLITE_OK != V8ECSchemaXmlInfo::Insert(GetDgnDb(), schemaId, schemaXml))
        {
        BeAssert(false && "Could not insert foreign schema xml");
        return BSIERROR;
        }

    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   10/2014
//---------------------------------------------------------------------------------------
bool DynamicSchemaGenerator::IsDynamicSchema(Bentley::Utf8StringCR schemaName, Bentley::Utf8StringCR schemaXml)
    {
    if (IsWellKnownDynamicSchema(schemaName))
        return true;

    std::regex exp("DynamicSchema\\s+xmlns\\s*=\\s*[\'\"]\\s*Bentley_Standard_CustomAttributes\\.[0-9]+\\.+[0-9]+");
    size_t searchEndOffset = schemaXml.find("ECClass");
    Utf8String::const_iterator endIt = (searchEndOffset == Utf8String::npos) ? schemaXml.begin() : (schemaXml.begin() + searchEndOffset);
    return std::regex_search<Utf8String::const_iterator>(schemaXml.begin(), endIt, exp);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   10/2014
//---------------------------------------------------------------------------------------
bool DynamicSchemaGenerator::IsDynamicSchema(ECObjectsV8::ECSchemaCR schema)
    {
    Bentley::Utf8String schemaName(schema.GetName().c_str());
    if (IsWellKnownDynamicSchema(schemaName))
        return true;

    //TBD: Should be replaced by schema.IsDynamicSchema once Graphite ECObjects has been merged back to Topaz
    return schema.GetCustomAttribute(L"DynamicSchema") != nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   10/2014
//---------------------------------------------------------------------------------------
bool DynamicSchemaGenerator::IsWellKnownDynamicSchema(Bentley::Utf8StringCR schemaName)
    {
    return BeStringUtilities::Strnicmp(schemaName.c_str(), "PFLModule", 9) == 0 ||
        schemaName.EqualsI("CivilSchema_iModel") ||
        schemaName.EqualsI("BuildingDataGroup") ||
        schemaName.Equals("V8TagSetDefinitions") ||
        BeStringUtilities::Strnicmp(schemaName.c_str(), "Ifc", 3) == 0 ||
        schemaName.StartsWith("EWR") ||
        schemaName.StartsWith("DgnCustomItemTypes_");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2017
//---------------+---------------+---------------+--------------c-+---------------+-------
//BentleyStatus Converter::LastResortSchemaImport()
//    {
//    // Schema Import failed.  Therefore, start over and this time just turn everything into an ElementAspect instead
//    InitializeECSchemaConversion();
//    BentleyApi::ECN::ConversionCustomAttributeHelper::Reset();
//    V8NamedGroupInfo::Reset();

//    ConsolidateV8ECSchemas();

//    BisClassConverter::SchemaConversionContext context(*this, *m_schemaReadContext, *m_syncReadContext, m_config.GetOptionValueBool("AutoDetectMixinParams", true));
//    for (bpair<Utf8String, BECN::ECSchemaP> const& kvpair : context.GetSchemas())
//        {
//        BECN::ECSchemaP schema = kvpair.second;
//        //only interested in the domain schemas, so skip standard, system and supp schemas
//        if (context.ExcludeSchemaFromBisification(*schema))
//            continue;

//        bvector<BECN::ECClassP> relationships;
//        for (BECN::ECClassP v8Class : schema->GetClasses())
//            {
//            ECClassName v8ClassName(*v8Class);

//            BisConversionRule existingRule;
//            BECN::ECClassId existingV8ClassId;
//            bool hasSecondary;
//            const bool alreadyExists = V8ECClassInfo::TryFind(existingV8ClassId, existingRule, context.GetDgnDb(), v8ClassName, hasSecondary);

//            if (v8Class->IsRelationshipClass())
//                relationships.push_back(v8Class);
//            else if (BSISUCCESS != V8ECClassInfo::Update(*this, existingV8ClassId, BisConversionRule::ToAspectOnly))
//                return BSIERROR;
//            }
//        for (BECN::ECClassP rel : relationships)
//            schema->DeleteClass(*rel);
//        }
//    if (BentleyApi::SUCCESS != ConvertToBisBasedECSchemas())
//        return BSIERROR;

//    if (BentleyApi::SUCCESS != ImportTargetECSchemas())
//        return BSIERROR;

//    return BSISUCCESS;
//    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   10/2014
//---------------------------------------------------------------------------------------
void DynamicSchemaGenerator::InitializeECSchemaConversion()
    {
    //set-up required schema locaters and search paths
    m_schemaReadContext = ECN::ECSchemaReadContext::CreateContext();
    m_syncReadContext = ECN::ECSchemaReadContext::CreateContext();

    m_schemaReadContext->AddSchemaLocater(GetDgnDb().GetSchemaLocater());
    auto host = DgnPlatformLib::QueryHost();
    if (host != nullptr)
        {
        BeFileName ecschemasDir = GetParams().GetAssetsDir();
        ecschemasDir.AppendToPath(L"ECSchemas");

        BeFileName dgnECSchemasDir(ecschemasDir);
        dgnECSchemasDir.AppendToPath(L"Dgn");
        m_schemaReadContext->AddSchemaPath(dgnECSchemasDir.c_str());

        BeFileName v3ConversionECSchemasDir(ecschemasDir);
        v3ConversionECSchemasDir.AppendToPath(L"V3Conversion");
        m_schemaReadContext->AddConversionSchemaPath(v3ConversionECSchemasDir.c_str());

        BeFileName dgndbECSchemasDir(ecschemasDir);
        dgndbECSchemasDir.AppendToPath(L"Standard");
        m_syncReadContext->AddSchemaPath(dgnECSchemasDir.c_str());

        BeFileName dgndbSyncECSchemasDir(ecschemasDir);
        dgndbSyncECSchemasDir.AppendToPath(L"DgnDbSync");
        m_syncReadContext->AddSchemaPath(dgndbSyncECSchemasDir.c_str());

    	m_syncReadContext->AddSchemaLocater(GetDgnDb().GetSchemaLocater());

        BeFileName ecdbECSchemasDir(ecschemasDir);
        ecdbECSchemasDir.AppendToPath(L"ECDb");
        m_syncReadContext->AddSchemaPath(ecdbECSchemasDir.c_str());
        }
    else
        {
        BeAssert(false && "Could not retrieve DgnPlatformLib Host.");
        }
    m_syncReadContext->SetSkipValidation(true);
    m_schemaReadContext->SetSkipValidation(true);
    m_syncReadContext->SetCalculateChecksum(true);
    m_schemaReadContext->SetCalculateChecksum(true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   07/2015
//---------------------------------------------------------------------------------------
void DynamicSchemaGenerator::FinalizeECSchemaConversion()
    {
    m_schemaReadContext = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DynamicSchemaGenerator::CheckNoECSchemaChanges(bvector<DgnV8ModelP> const& uniqueModels)
    {
    for (auto& v8Model : uniqueModels)
        {
        bmap<Utf8String, uint32_t> syncInfoChecksums;
        RepositoryLinkId v8FileId = Converter::GetRepositoryLinkIdFromAppData(*v8Model->GetDgnFileP());
        GetSyncInfo().RetrieveECSchemaChecksums(syncInfoChecksums, v8FileId);
        CheckECSchemasForModel(*v8Model, syncInfoChecksums);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            01/2016
//---------------+---------------+---------------+---------------+---------------+-------
void DynamicSchemaGenerator::CheckECSchemasForModel(DgnV8ModelR v8Model, bmap<Utf8String, uint32_t>& syncInfoChecksums)
    {
    auto& dgnv8EC = DgnV8Api::DgnECManager::GetManager();
    const DgnV8Api::ReferencedModelScopeOption modelScopeOption = DgnV8Api::REFERENCED_MODEL_SCOPE_None;

    Bentley::bvector<DgnV8Api::SchemaInfo> v8SchemaInfos;
    dgnv8EC.DiscoverSchemasForModel(v8SchemaInfos, v8Model, DgnV8Api::ECSCHEMAPERSISTENCE_Stored, modelScopeOption);
    if (v8SchemaInfos.empty())
        return;

    for (auto& v8SchemaInfo : v8SchemaInfos)
        {
        Utf8String v8SchemaName(v8SchemaInfo.GetSchemaName());
        if (ECN::ECSchema::IsStandardSchema(v8SchemaName))
            continue;

        bmap<Utf8String, uint32_t>::const_iterator syncEntry = syncInfoChecksums.find(v8SchemaName);
        // If schema was not in the original DgnDb, we need to import it
        if (syncEntry == syncInfoChecksums.end())
            {
            m_needReimportSchemas = true;
            continue;
            }

        auto found = std::find_if(s_dgnV8DeliveredSchemas.begin(), s_dgnV8DeliveredSchemas.end(), [v8SchemaName] (Utf8CP dgnv8) ->bool { return BeStringUtilities::StricmpAscii(v8SchemaName.c_str(), dgnv8) == 0; });
        if (found != s_dgnV8DeliveredSchemas.end())
            continue;

        // It is possible we scanned the schema previously, but didn't import it.  Make sure it is actually in the db
        Utf8String bimSchemaName(v8SchemaName);
        if (0 == BeStringUtilities::Strnicmp("EWR", v8SchemaName.c_str(), 3))
            bimSchemaName.AssignOrClear("EWR");

        if (!m_converter.GetDgnDb().Schemas().ContainsSchema(bimSchemaName))
            {
            m_needReimportSchemas = true;
            continue;
            }
        Bentley::Utf8String schemaXml;
        uint32_t checksum = -1;
        if (v8SchemaInfo.IsStoredSchema())
            {
            Bentley::WString schemaXmlW;
            auto stat = dgnv8EC.LocateSchemaXmlInModel(schemaXmlW, v8SchemaInfo, ECObjectsV8::SCHEMAMATCHTYPE_Exact, v8Model, modelScopeOption);
            if (stat != BentleyApi::SUCCESS)
                {
                Utf8PrintfString msg("Could not read v8 ECSchema XML for '%s'.", v8SchemaName.c_str());
                ReportSyncInfoIssue(Converter::IssueSeverity::Fatal, Converter::IssueCategory::MissingData(), Converter::Issue::Error(), msg.c_str());
                OnFatalError(Converter::IssueCategory::MissingData());
                return;
                }

            schemaXml = Bentley::Utf8String(schemaXmlW);
            const size_t xmlByteSize = schemaXmlW.length() * sizeof(WChar);
            checksum = ECObjectsV8::ECSchema::ComputeSchemaXmlStringCheckSum(schemaXmlW.c_str(), xmlByteSize);
            }
        else
            {
            // handle external schemas
            //TBD: DgnECManager doesn't seem to allow to just return the schema xml (Is this possible somehow?)
            //So we need to get the ECSchema and then serialize it to a string.
            //(we need the string anyways as this is the only way to marshal the schema from v8 to Graphite)
            auto externalSchema = dgnv8EC.LocateExternalSchema(v8SchemaInfo, ECObjectsV8::SCHEMAMATCHTYPE_Exact);
            if (externalSchema == nullptr)
                {
                Utf8PrintfString msg("Could not locate external v8 ECSchema '%s'", v8SchemaName.c_str());
                ReportSyncInfoIssue(Converter::IssueSeverity::Fatal, Converter::IssueCategory::MissingData(), Converter::Issue::Error(), msg.c_str());
                OnFatalError(Converter::IssueCategory::MissingData());
                return;
                }
            if (ECObjectsV8::SCHEMA_WRITE_STATUS_Success != externalSchema->WriteToXmlString(schemaXml))
                {
                Utf8PrintfString msg("Could not serialize external v8 ECSchema '%s'", v8SchemaName.c_str());
                ReportSyncInfoIssue(Converter::IssueSeverity::Fatal, Converter::IssueCategory::CorruptData(), Converter::Issue::Error(), "Could not serialize external v8 ECSchema.");
                OnFatalError(Converter::IssueCategory::CorruptData());
                return;
                }
            checksum = v8SchemaInfo.GetSchemaKey().m_checkSum;
            }
        if (checksum != syncEntry->second)
            {
            ECObjectsV8::SchemaKey newSchemaKey = v8SchemaInfo.GetSchemaKey();
            ECN::ECSchemaCP bimSchema = m_converter.GetDgnDb().Schemas().GetSchema(Utf8String(newSchemaKey.GetName().c_str()).c_str(), false);
            ECN::SchemaKey bimSchemaKey = bimSchema->GetSchemaKey();
            if (newSchemaKey.GetVersionMajor() == bimSchemaKey.GetVersionRead() && newSchemaKey.GetVersionMinor() <= bimSchemaKey.GetVersionMinor() && 
                !IsDynamicSchema(v8SchemaName.c_str(), schemaXml))
                {
                Utf8PrintfString msg("v8 ECSchema '%s' checksum is different from stored schema yet the version is the same as or lower than the version stored.  Minor version must be greater than stored version in order to update.", Utf8String(v8SchemaInfo.GetSchemaName()).c_str());
                ReportSyncInfoIssue(Converter::IssueSeverity::Fatal, Converter::IssueCategory::InconsistentData(), Converter::Issue::ConvertFailure(), msg.c_str());
                OnFatalError(Converter::IssueCategory::InconsistentData());
                return;

                }
            Utf8PrintfString msg("v8 ECSchema '%s' checksum is different from stored schema.  Need to merge and reimport.", Utf8String(v8SchemaInfo.GetSchemaName()).c_str());
            m_needReimportSchemas = true;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DynamicSchemaGenerator::BisifyV8Schemas(bvector<DgnV8FileP> const& uniqueFiles, bvector<DgnV8ModelP> const& uniqueModels)
    {
    m_converter.SetStepName(Converter::ProgressMessage::STEP_DISCOVER_ECSCHEMAS());

    SchemaConversionScope scope(*this);

    StopWatch timer(true);
    
    AddTasks((uint32_t)uniqueModels.size());

    for (DgnV8ModelP v8Model : uniqueModels)
        {
        RetrieveV8ECSchemas(*v8Model);
        }

    if (m_skipECContent)
        {
        scope.SetSucceeded();
        return;
        }

    if (SUCCESS != ConsolidateV8ECSchemas() || WasAborted())
        return;

    ConverterLogging::LogPerformance(timer, "Convert Schemas> Total Retrieve and consolidate V8 ECSchemas");

    AddTasks((int32_t)(uniqueModels.size() + uniqueFiles.size()));

    timer.Start();
    for (DgnV8ModelP v8Model : uniqueModels)
        {
        SetTaskName(Converter::ProgressMessage::TASK_ANALYZE_EC_CONTENT(), Converter::IssueReporter::FmtModel(*v8Model).c_str());

        BisConversionTargetModelInfo targetModelInfo(m_converter.ShouldConvertToPhysicalModel(*v8Model)? BisConversionTargetModelInfo::ModelType::ThreeD:
                                                                                                         BisConversionTargetModelInfo::ModelType::TwoD);
        AnalyzeECContent(*v8Model, targetModelInfo);
        if (WasAborted())
            return;
        }

    //analyze named groups in dictionary models
    for (DgnV8FileP v8File : uniqueFiles)
        {
        SetTaskName(Converter::ProgressMessage::TASK_ANALYZE_EC_CONTENT(), Converter::IssueReporter::FmtModel(v8File->GetDictionaryModel()).c_str());

        DgnV8ModelR dictionaryModel = v8File->GetDictionaryModel();
        BisConversionTargetModelInfo targetModelInfo(BisConversionTargetModelInfo::ModelType::Dictionary);
        AnalyzeECContent(dictionaryModel, targetModelInfo);
        if (WasAborted())
            return;
        }

    ConverterLogging::LogPerformance(timer, "Convert Schemas> Analyze V8 EC content");

    if (!m_hasECContent)
        {
        scope.SetSucceeded();
        return;
        }

    SetStepName(Converter::ProgressMessage::STEP_IMPORT_SCHEMAS());

    timer.Start();
    if (BentleyApi::SUCCESS != ConvertToBisBasedECSchemas())
        return;

    ReportProgress();
    ConverterLogging::LogPerformance(timer, "Convert Schemas> Upgrade V8 ECSchemas");

    timer.Start();

    if (BentleyApi::SUCCESS != ImportTargetECSchemas())
        {
        // if (BentleyApi::SUCCESS != LastResortSchemaImport())
            return;
        }

    ReportProgress();
    ConverterLogging::LogPerformance(timer, "Convert Schemas> Import ECSchemas");

    if (WasAborted())
        return;
    scope.SetSucceeded();
    }

//=======================================================================================
// @bsiclass 
//=======================================================================================
struct     SchemaImportCaller : public DgnV8Api::IEnumerateAvailableHandlers
    {
    Converter& m_converter;
    SchemaImportCaller(Converter& cvt) : m_converter(cvt) {}
    virtual StatusInt _ProcessHandler(DgnV8Api::Handler& handler)
        {
        ConvertToDgnDbElementExtension* extension = ConvertToDgnDbElementExtension::Cast(handler);
        if (NULL == extension)
            return SUCCESS;

        extension->_ImportSchema(m_converter.GetDgnDb());
        return SUCCESS;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static void     importHandlerExtensionsSchema(Converter& cvt)
    {
    SchemaImportCaller importer(cvt);
    DgnV8Api::ElementHandlerManager::EnumerateAvailableHandlers(importer);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DynamicSchemaGenerator::GenerateSchemas(bvector<DgnV8FileP> const& files, bvector<DgnV8ModelP> const& models)
    {
    if (GetConfig().GetOptionValueBool("SkipECContent", false))
        return;

    if (m_converter.IsUpdating())
        {
        CheckNoECSchemaChanges(models);
        if (!m_needReimportSchemas)
            return;
        }
        
    BisifyV8Schemas(files, models);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void SpatialConverterBase::CreateProvenanceTables()
    {
    // WIP_EXTERNAL_SOURCE_INFO - stop using so-called model provenance

    if (!m_dgndb->TableExists(DGN_TABLE_ProvenanceFile) && _WantModelProvenanceInBim())
        DgnV8FileProvenance::CreateTable(*m_dgndb);
    if (!m_dgndb->TableExists(DGN_TABLE_ProvenanceModel) && _WantModelProvenanceInBim())
        DgnV8ModelProvenance::CreateTable(*m_dgndb);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SpatialConverterBase::MakeSchemaChanges(bvector<DgnFileP> const& filesInOrder, bvector<DgnV8ModelP> const& modelsInOrder)
    {
    // NB: This function is called at initialization time as part of a schema-changes-only revision.
    //      *** DO NOT CONVERT MODELS OR ELEMENTS. ***

    // *** TRICKY: We need to know if this is an update or not. The framework has not yet set the 'is-updating' flag.
    //              So, we must figure out if this is an update or not right here and now by checking to see if the job subject already exists.
    auto mmsubj = FindSourceMasterModelSubject(*GetRootModelP());
    if (mmsubj.IsValid())
        {
        auto jobsubj = FindSoleJobSubjectForSourceMasterModel(*mmsubj);
        if (jobsubj.IsValid())
            _GetParamsR().SetIsUpdating(true);
        }

#ifndef NDEBUG
    if (_WantModelProvenanceInBim())
        {
        BeAssert(m_dgndb->TableExists(DGN_TABLE_ProvenanceFile));
        }
#endif

    // Bis-ify the V8 schemas
    if (m_config.GetOptionValueBool("SkipECContent", false))
        {
        m_skipECContent = true;
        }
    else
        {
        // V8TagSets - This is tricky. We convert V8 tagset defs into V8 ECClasses, and we add ECInstances to the tagged V8 elements. That way, the normal
        //              schema conversion (below) will import the classes, and the normal element conversion will import the instances.
        if (true)
            {
            StopWatch timer(true);
            _ConvertDgnV8Tags(filesInOrder, modelsInOrder);
            ConverterLogging::LogPerformance(timer, "Convert Dgn V8Tags");
            }

        if (WasAborted())
            return BSIERROR;

        // *******
        // WARNING: GenerateSchemas calls Db::AbandonChanges if import fails! Make sure you commit your work before calling GenerateSchemas!
        // *******

        DynamicSchemaGenerator gen(*this);
        gen.GenerateSchemas(filesInOrder, modelsInOrder);

        m_skipECContent = gen.GetEcConversionFailed();

        if (gen.DidEcConversionFailDueToLockingError())
            return BSIERROR;    // This is a re-try-able failure, not a fatal error that should stop the conversion
        m_hadAnyChanges |= gen.GetAnyImported();
        }

    if (WasAborted())
        return BSIERROR;

    CheckForAndSaveChanges();

    // Let handler extensions import schemas
    importHandlerExtensionsSchema(*this);

    if (WasAborted())
        return BSIERROR;

    CheckForAndSaveChanges();

    for (auto xdomain : XDomainRegistry::s_xdomains)
        {
        if (BSISUCCESS != xdomain->_ImportSchema(*m_dgndb))
            {
            OnFatalError();
            }
        }

    if (WasAborted())
        return BSIERROR;

    CheckForAndSaveChanges();

    // This shouldn't be dependent on importing schemas.  Sometimes you want class views for just the basic Bis classes.
    if (GetConfig().GetOptionValueBool("CreateECClassViews", true))
        {
        if (!_GetParamsR().IsUpdating() || anyTxnsInFile(GetDgnDb()))   // don't regenerate class views unless we know that there are new or modified schemas
            {
            SetStepName(Converter::ProgressMessage::STEP_CREATE_CLASS_VIEWS());
            GetDgnDb().Schemas().CreateClassViewsInDb(); // Failing to create the views should not cause errors for the rest of the conversion
            }
        }

    CheckForAndSaveChanges();
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RootModelConverter::MakeSchemaChanges()
    {
    StopWatch timer(true);

    // Get the list of all models that are assigned to this bridge.
    bvector<DgnV8ModelP> modelsInFMOrder;
    std::copy_if(m_spatialModelsInAttachmentOrder.begin(), m_spatialModelsInAttachmentOrder.end(), std::back_inserter(modelsInFMOrder),
      [&](DgnV8ModelP model)
        {
        return IsFileAssignedToBridge(*model->GetDgnFileP());
        });
    std::copy_if(m_nonSpatialModelsInModelIndexOrder.begin(), m_nonSpatialModelsInModelIndexOrder.end(), std::back_inserter(modelsInFMOrder),
      [&](DgnV8ModelP model)
        {
        return IsFileAssignedToBridge(*model->GetDgnFileP());
        });

    // sort the models in file, modelid order - that matches the order in which the older converter processed models.
    auto cmp = [&](DgnV8ModelP a, DgnV8ModelP b) { return IsLessInMappingOrder(a,b); };
    std::sort(modelsInFMOrder.begin(), modelsInFMOrder.end(), cmp);

    // Get the list of all files (in original discovery order) that are assigned to this bridge.
    bvector<DgnV8FileP> filesInOrder;
    std::copy_if(m_v8Files.begin(), m_v8Files.end(), std::back_inserter(filesInOrder),
      [&](DgnV8FileP file)
        {
        return IsFileAssignedToBridge(*file);
        });

    // Processed V8 schemas
    auto status = T_Super::MakeSchemaChanges(filesInOrder, modelsInFMOrder);

    ConverterLogging::LogPerformance(timer, "Convert Schemas (total)");

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus TiledFileConverter::MakeSchemaChanges()
    {
    StopWatch timer(true);

    bvector<DgnV8ModelP> modelsInOrder;
    modelsInOrder.push_back(GetRootModelP());

    bvector<DgnV8FileP> filesInOrder;
    filesInOrder.push_back(GetRootModelP()->GetDgnFileP());

    GetRepositoryLinkId(*GetRootV8File()); // DynamicSchemaGenerator et al need to assume that all V8 files are recorded in syncinfo

    auto status = T_Super::MakeSchemaChanges(filesInOrder, modelsInOrder);

    ConverterLogging::LogPerformance(timer, "Convert Schemas (total)");

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            05/2018
//---------------+---------------+---------------+---------------+---------------+-------
Utf8CP findRootRelationshipName(ECN::ECClassCP relClass)
    {
    if (relClass->GetBaseClasses().size() > 0)
        {
        for (ECN::ECClassCP baseClass : relClass->GetBaseClasses())
            {
            if (BisClassConverter::SchemaConversionContext::ExcludeSchemaFromBisification(baseClass->GetSchema()))
                continue;
            return findRootRelationshipName(baseClass);
            }
        }
    return relClass->GetName().c_str();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2016
//---------------+---------------+---------------+---------------+---------------+-------
bool Converter::DoesRelationshipExist(Utf8StringCR relName, BeSQLite::EC::ECInstanceKey const& sourceInstanceKey, BeSQLite::EC::ECInstanceKey const& targetInstanceKey)
    {

    Utf8String ecsql("SELECT COUNT(*) FROM ");
    ecsql.append(relName).append(" WHERE SourceECInstanceId=? AND TargetECInstanceId=?");
    BeSQLite::EC::CachedECSqlStatementPtr stmt = GetDgnDb().GetPreparedECSqlStatement(ecsql.c_str());
    //Utf8String qplan = GetDgnDb().ExplainQuery();

    if (!stmt.IsValid())
        {
        ReportIssue(Converter::IssueSeverity::Error, Converter::IssueCategory::CorruptData(), Converter::Issue::Message(),
                    Utf8PrintfString("%s - failed to prepare", ecsql.c_str()).c_str());
        return false;
        }

    if (BeSQLite::EC::ECSqlStatus::Success != stmt->BindId(1, sourceInstanceKey.GetInstanceId()))
        {
        Utf8PrintfString error("Failed to search for ECRelationship %s. Binding value to SourceECInstanceId (%s) failed. (%s:%s -> %s:%s)", relName.c_str(), sourceInstanceKey.GetInstanceId().ToString().c_str(),
                               sourceInstanceKey.GetClassId().ToString().c_str(), sourceInstanceKey.GetInstanceId().ToString().c_str(), targetInstanceKey.GetClassId().ToString().c_str(), targetInstanceKey.GetInstanceId().ToString().c_str());
        ReportIssue(Converter::IssueSeverity::Error, Converter::IssueCategory::Sync(), Converter::Issue::Message(), error.c_str());
        return false;
        }

    if (BeSQLite::EC::ECSqlStatus::Success != stmt->BindId(2, targetInstanceKey.GetInstanceId()))
        {
        Utf8PrintfString error("Failed to search for ECRelationship %s. Binding value to TargetECInstanceId (%s) failed.  (%s:%s -> %s:%s)", relName.c_str(), targetInstanceKey.GetInstanceId().ToString().c_str(),
                               sourceInstanceKey.GetClassId().ToString().c_str(), sourceInstanceKey.GetInstanceId().ToString().c_str(), targetInstanceKey.GetClassId().ToString().c_str(), targetInstanceKey.GetInstanceId().ToString().c_str());
        ReportIssue(Converter::IssueSeverity::Error, Converter::IssueCategory::Sync(), Converter::Issue::Message(), error.c_str());
        return false;
        }

    if (BeSQLite::BE_SQLITE_ROW != stmt->Step())
        {
        Utf8String errorMsg;
        errorMsg.Sprintf("Failed to search for ECRelationship '%s' "
                         "(Source: (%s:%s) Target (%s:%s)). "
                         "Execution of ECSQL SELECT '%s' failed. (Native SQL: %s)\n",
                         relName.c_str(),
                         sourceInstanceKey.GetClassId().ToString().c_str(), sourceInstanceKey.GetInstanceId().ToString().c_str(),
                         targetInstanceKey.GetClassId().ToString().c_str(), targetInstanceKey.GetInstanceId().ToString().c_str(),
                         stmt->GetECSql(), stmt->GetNativeSql());

        ReportIssue(Converter::IssueSeverity::Error, Converter::IssueCategory::Sync(), Converter::Issue::Message(),
                    errorMsg.c_str());
        return false;
        }

    return stmt->GetValueInt(0) > 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   10/2014
//---------------------------------------------------------------------------------------
BentleyApi::BentleyStatus Converter::ConvertECRelationships(DgnV8Api::ElementHandle const& v8Element)
    {
    auto& v8ECManager = DgnV8Api::DgnECManager::GetManager();
    DgnV8Api::RelationshipEntryVector relationships;
    v8ECManager.FindRelationshipEntriesOnElement(v8Element.GetElementRef(), relationships);

    DgnDbR dgndb = GetDgnDb();
    RepositoryLinkId fileId = GetRepositoryLinkId(*v8Element.GetDgnFileP());

    for (DgnV8Api::RelationshipEntry const& entry : relationships)
        {
        //schemas not captured in sync info are system schemas which we don't consider during conversion
        if (!GetSyncInfo().ContainsECSchema(Utf8String(entry.RelationshipSchemaName.c_str()).c_str()))
            continue;

        V8ECInstanceKey v8SourceKey(ECClassName(Utf8String(entry.SourceSchemaName.c_str()).c_str(), Utf8String(entry.SourceClassName.c_str()).c_str()),
                                    entry.SourceInstanceId.c_str());
        V8ECInstanceKey v8TargetKey(ECClassName(Utf8String(entry.TargetSchemaName.c_str()).c_str(), Utf8String(entry.TargetClassName.c_str()).c_str()),
                                    entry.TargetInstanceId.c_str());
        ECClassName v8RelName(Utf8String(entry.RelationshipSchemaName.c_str()).c_str(), Utf8String(entry.RelationshipClassName.c_str()).c_str());
        Utf8String v8RelFullClassName = v8RelName.GetClassFullName();

        BisConversionRule rule;
        bool hasSecondary;
        if (!V8ECClassInfo::TryFind(rule, dgndb, v8RelName, hasSecondary))
            {
            BeAssert(false && "V8ECClassInfo should exist for relationship classes.");
            continue;
            }

        if (BisConversionRuleHelper::IgnoreInstance(rule))
            {
            Utf8String errorMsg;
            errorMsg.Sprintf("Skipped v8 '%s' relationship ECInstance because its class was ignored during schema conversion. See ECSchema conversion log entries above.",
                             v8RelName.GetClassFullName().c_str());
            ReportIssue(Converter::IssueSeverity::Warning, Converter::IssueCategory::Sync(), Converter::Issue::Message(), errorMsg.c_str());
            continue;
            }

        bool sourceInstanceIsElement = false;
        BeSQLite::EC::ECInstanceKey sourceInstanceKey = ECInstanceInfo::Find(sourceInstanceIsElement, dgndb, fileId, v8SourceKey);
        bool targetInstanceIsElement = false;
        BeSQLite::EC::ECInstanceKey targetInstanceKey = ECInstanceInfo::Find(targetInstanceIsElement, dgndb, fileId, v8TargetKey);

        if (IsUpdating())
            {
            if (DoesRelationshipExist(v8RelFullClassName, sourceInstanceKey, targetInstanceKey))
                {
                continue;
                }
            }
        ECDiagnostics::LogV8RelationshipDiagnostics(dgndb, v8RelName, v8SourceKey, sourceInstanceKey.IsValid(), sourceInstanceIsElement, v8TargetKey, targetInstanceKey.IsValid(), targetInstanceIsElement);

        if (!sourceInstanceKey.IsValid() || !targetInstanceKey.IsValid())
            {
            Utf8CP failingEndStr = nullptr;
            if (!sourceInstanceKey.IsValid() && !targetInstanceKey.IsValid())
                failingEndStr = "source and target ECInstances";
            else
                failingEndStr = !sourceInstanceKey.IsValid() ? "source ECInstance" : "target ECInstance";

            Utf8String errorMsg;
            errorMsg.Sprintf("Could not find %s for relationship '%s' (Source: %s|%s Target %s|%s).",
                             failingEndStr, v8RelFullClassName.c_str(),
                             v8SourceKey.GetClassName().GetClassFullName().c_str(), v8SourceKey.GetInstanceId(),
                             v8TargetKey.GetClassName().GetClassFullName().c_str(), v8TargetKey.GetInstanceId());
            ReportIssue(Converter::IssueSeverity::Warning, Converter::IssueCategory::Sync(), Converter::Issue::Message(), errorMsg.c_str());
            continue;
            }

        ECN::ECClassCP relClass = GetDgnDb().Schemas().GetClass(v8RelName.GetSchemaName(), v8RelName.GetClassName());
        if (relClass == nullptr || !relClass->IsRelationshipClass())
            {
            Utf8String error;
            error.Sprintf("Failed to convert instance of ECRelationshipClass %s. The class doesn't exist in the BIM file.", v8RelFullClassName.c_str());
            ReportIssue(Converter::IssueSeverity::Error, Converter::IssueCategory::Sync(), Converter::Issue::Message(),
                        error.c_str());
            continue;
            }

        // If the relationship class inherits from one ElementRefersToElements base relationship class, then it is a link table relationship, and can use the API
        if (relClass->Is(BIS_ECSCHEMA_NAME, BIS_REL_ElementRefersToElements))
            {
            BeSQLite::EC::ECInstanceKey relKey;
            if (BE_SQLITE_OK != GetDgnDb().InsertLinkTableRelationship(relKey, *relClass->GetRelationshipClassCP(), sourceInstanceKey.GetInstanceId(), targetInstanceKey.GetInstanceId()))
                {
                Utf8String dgndbError = GetDgnDb().GetLastError();
                Utf8String errorMsg;
                errorMsg.Sprintf("Failed to convert ECRelationship '%s' from element %" PRIu64 " in file '%s' "
                                 "(Source: %s|%s (%s:%s) Target %s|%s (%s:%s)). "
                                 "Insertion into target BIM file failed.%s%s",
                                 v8RelFullClassName.c_str(),
                                 v8Element.GetElementId(), Utf8String(v8Element.GetDgnFileP()->GetFileName().c_str()).c_str(),
                                 v8SourceKey.GetClassName().GetClassFullName().c_str(), v8SourceKey.GetInstanceId(),
                                 sourceInstanceKey.GetClassId().ToString().c_str(), sourceInstanceKey.GetInstanceId().ToString().c_str(),
                                 v8TargetKey.GetClassName().GetClassFullName().c_str(), v8TargetKey.GetInstanceId(),
                                 targetInstanceKey.GetClassId().ToString().c_str(), targetInstanceKey.GetInstanceId().ToString().c_str(),
                                 dgndbError.empty() ? "" : " Reason: ",
                                 dgndbError.empty() ? "" : dgndbError.c_str());

                ReportIssue(Converter::IssueSeverity::Error, Converter::IssueCategory::Sync(), Converter::Issue::Message(),
                            errorMsg.c_str());
                }
            continue;
            }

        ECN::ECClassCP targetClass = GetDgnDb().Schemas().GetClass(targetInstanceKey.GetClassId());
        ECN::ECClassCP sourceClass = GetDgnDb().Schemas().GetClass(sourceInstanceKey.GetClassId());
        // Otherwise, the converter should have created a navigation property on the target class, so we need to set the target instance's ECValue
        ECN::ECPropertyP prop = nullptr;

        // Search for root relationshipclass name
        Utf8CP navName = findRootRelationshipName(relClass);
        // If the class is an ElementOwnsMultiAspects base, then need to use the NavigationProperty 'Element' that is defined on the base MultiAspect class.
        bool navPropOnSource = false;
        if (relClass->Is(BIS_ECSCHEMA_NAME, BIS_REL_ElementOwnsMultiAspects))
            {
            prop = targetClass->GetPropertyP("Element");
            }
        else
            {
            prop = targetClass->GetPropertyP(navName);
            if (nullptr == prop)
                {
                prop = sourceClass->GetPropertyP(navName);
                navPropOnSource = true;
                }
            }

        if (nullptr == prop)
            {
            Utf8String errorMsg;
            errorMsg.Sprintf("Unable to find NavigationECProperty '%s' on Target-Constraint ECClass '%s'.  This relationship is not derived from a BisCore link table relationship "
                             "and therefore the conversion process should have created a NavigationECProperty on the ECClass."
                             "Failed to convert ECRelationship '%s' from element %" PRIu64 " in file '%s' "
                             "(Source: %s|%s (%s:%s) Target %s|%s (%s:%s)). "
                             "Insertion into target BIM file failed.",
                             relClass->GetName().c_str(), targetClass->GetFullName(),
                             v8RelFullClassName.c_str(),
                             v8Element.GetElementId(), Utf8String(v8Element.GetDgnFileP()->GetFileName().c_str()).c_str(),
                             v8SourceKey.GetClassName().GetClassFullName().c_str(), v8SourceKey.GetInstanceId(),
                             sourceInstanceKey.GetClassId().ToString().c_str(), sourceInstanceKey.GetInstanceId().ToString().c_str(),
                             v8TargetKey.GetClassName().GetClassFullName().c_str(), v8TargetKey.GetInstanceId(),
                             targetInstanceKey.GetClassId().ToString().c_str(), targetInstanceKey.GetInstanceId().ToString().c_str());
            ReportIssue(Converter::IssueSeverity::Error, Converter::IssueCategory::Sync(), Converter::Issue::Message(),
                        errorMsg.c_str());

            continue;
            }
        ECN::NavigationECPropertyP navProp = prop->GetAsNavigationPropertyP();
        if (nullptr == navProp)
            {
            Utf8String errorMsg;
            errorMsg.Sprintf("Unable to find NavigationECProperty '%s' on Target-Constraint ECClass '%s'.  This relationship is not derived from a BisCore link table relationship "
                             "and therefore the conversion process should have created a NavigationECProperty on the ECClass."
                             "Failed to convert ECRelationship '%s' from element %" PRIu64 " in file '%s' "
                             "(Source: %s|%s (%s:%s) Target %s|%s (%s:%s)). "
                             "Insertion into target BIM file failed.",
                             relClass->GetName().c_str(), targetClass->GetFullName(),
                             v8RelFullClassName.c_str(),
                             v8Element.GetElementId(), Utf8String(v8Element.GetDgnFileP()->GetFileName().c_str()).c_str(),
                             v8SourceKey.GetClassName().GetClassFullName().c_str(), v8SourceKey.GetInstanceId(),
                             sourceInstanceKey.GetClassId().ToString().c_str(), sourceInstanceKey.GetInstanceId().ToString().c_str(),
                             v8TargetKey.GetClassName().GetClassFullName().c_str(), v8TargetKey.GetInstanceId(),
                             targetInstanceKey.GetClassId().ToString().c_str(), targetInstanceKey.GetInstanceId().ToString().c_str());
            ReportIssue(Converter::IssueSeverity::Error, Converter::IssueCategory::Sync(), Converter::Issue::Message(),
                        errorMsg.c_str());

            continue;
            }
        ECN::ECValue val;
        val.SetNavigationInfo((BeInt64Id) targetInstanceKey.GetInstanceId().GetValue(), relClass->GetRelationshipClassCP());

        if (targetClass->Is(BIS_ECSCHEMA_NAME, BIS_CLASS_ElementAspect) && !navPropOnSource)
            {
            //DgnElementPtr element = m_dgndb->Elements().GetForEdit<DgnElement>(DgnElementId(sourceInstanceKey.GetInstanceId().GetValue()));
            //if (!element.IsValid())
            //    continue;
            //DgnElement::MultiAspect* aspect = DgnElement::MultiAspect::GetAspectP(*element, *targetClass, targetInstanceKey.GetInstanceId());
            //if (nullptr == aspect)
            //    {
            //    Utf8String errorMsg;
            //    errorMsg.Sprintf("Unable to get ElementAspect from Element."
            //                     "Failed to convert ECRelationship '%s' from element %" PRIu64 " in file '%s' "
            //                     "(Source: %s|%s (%s:%s) Target %s|%s (%s:%s)). "
            //                     "Insertion into target BIM file failed.",
            //                     v8RelFullClassName.c_str(),
            //                     v8Element.GetElementId(), Utf8String(v8Element.GetDgnFileP()->GetFileName().c_str()).c_str(),
            //                     v8SourceKey.GetClassName().GetClassFullName().c_str(), v8SourceKey.GetInstanceId(),
            //                     sourceInstanceKey.GetClassId().ToString().c_str(), sourceInstanceKey.GetInstanceId().ToString().c_str(),
            //                     v8TargetKey.GetClassName().GetClassFullName().c_str(), v8TargetKey.GetInstanceId(),
            //                     targetInstanceKey.GetClassId().ToString().c_str(), targetInstanceKey.GetInstanceId().ToString().c_str());
            //    ReportIssue(Converter::IssueSeverity::Error, Converter::IssueCategory::Sync(), Converter::Issue::Message(),
            //                errorMsg.c_str());
            //    continue;
            //    }
            //if (DgnDbStatus::Success != aspect->SetPropertyValue(navProp->GetName().c_str(), val))
            //    {
            //    Utf8String errorMsg;
            //    errorMsg.Sprintf("Failed to set NavigationECProperty on Target ElementAspect ECInstance for ECRelationship '%s' from element %" PRIu64 " in file '%s' "
            //                     "(Source: %s|%s (%s:%s) Target %s|%s (%s:%s)). ",
            //                     v8RelFullClassName.c_str(),
            //                     v8Element.GetElementId(), Utf8String(v8Element.GetDgnFileP()->GetFileName().c_str()).c_str(),
            //                     v8SourceKey.GetClassName().GetClassFullName().c_str(), v8SourceKey.GetInstanceId(),
            //                     sourceInstanceKey.GetClassId().ToString().c_str(), sourceInstanceKey.GetInstanceId().ToString().c_str(),
            //                     v8TargetKey.GetClassName().GetClassFullName().c_str(), v8TargetKey.GetInstanceId(),
            //                     targetInstanceKey.GetClassId().ToString().c_str(), targetInstanceKey.GetInstanceId().ToString().c_str());

            //    ReportIssue(Converter::IssueSeverity::Error, Converter::IssueCategory::Sync(), Converter::Issue::Message(),
            //                errorMsg.c_str());
            //    continue;
            //    }
            //element->Update();
            }
        else if (sourceClass->Is(BIS_ECSCHEMA_NAME, BIS_CLASS_ElementAspect) && navPropOnSource)
            {
            DgnElementPtr element = m_dgndb->Elements().GetForEdit<DgnElement>(DgnElementId(targetInstanceKey.GetInstanceId().GetValue()));
            if (!element.IsValid())
                continue;
            DgnElement::MultiAspect* aspect = DgnElement::MultiAspect::GetAspectP(*element, *sourceClass, sourceInstanceKey.GetInstanceId());
            if (nullptr == aspect)
                {
                Utf8String errorMsg;
                errorMsg.Sprintf("Unable to get ElementAspect from Element."
                                 "Failed to convert ECRelationship '%s' from element %" PRIu64 " in file '%s' "
                                 "(Source: %s|%s (%s:%s) Target %s|%s (%s:%s)). "
                                 "Insertion into target BIM file failed.",
                                 v8RelFullClassName.c_str(),
                                 v8Element.GetElementId(), Utf8String(v8Element.GetDgnFileP()->GetFileName().c_str()).c_str(),
                                 v8SourceKey.GetClassName().GetClassFullName().c_str(), v8SourceKey.GetInstanceId(),
                                 sourceInstanceKey.GetClassId().ToString().c_str(), sourceInstanceKey.GetInstanceId().ToString().c_str(),
                                 v8TargetKey.GetClassName().GetClassFullName().c_str(), v8TargetKey.GetInstanceId(),
                                 targetInstanceKey.GetClassId().ToString().c_str(), targetInstanceKey.GetInstanceId().ToString().c_str());
                ReportIssue(Converter::IssueSeverity::Error, Converter::IssueCategory::Sync(), Converter::Issue::Message(),
                            errorMsg.c_str());
                continue;
                }
            if (DgnDbStatus::Success != aspect->SetPropertyValue(navProp->GetName().c_str(), val))
                {
                Utf8String errorMsg;
                errorMsg.Sprintf("Failed to set NavigationECProperty on Source ElementAspect ECInstance for ECRelationship '%s' from element %" PRIu64 " in file '%s' "
                                 "(Source: %s|%s (%s:%s) Target %s|%s (%s:%s)). ",
                                 v8RelFullClassName.c_str(),
                                 v8Element.GetElementId(), Utf8String(v8Element.GetDgnFileP()->GetFileName().c_str()).c_str(),
                                 v8SourceKey.GetClassName().GetClassFullName().c_str(), v8SourceKey.GetInstanceId(),
                                 sourceInstanceKey.GetClassId().ToString().c_str(), sourceInstanceKey.GetInstanceId().ToString().c_str(),
                                 v8TargetKey.GetClassName().GetClassFullName().c_str(), v8TargetKey.GetInstanceId(),
                                 targetInstanceKey.GetClassId().ToString().c_str(), targetInstanceKey.GetInstanceId().ToString().c_str());

                ReportIssue(Converter::IssueSeverity::Error, Converter::IssueCategory::Sync(), Converter::Issue::Message(),
                            errorMsg.c_str());
                continue;
                }
            element->Update();
            }

        else
            {
            DgnElementPtr element = m_dgndb->Elements().GetForEdit<DgnElement>(DgnElementId(targetInstanceKey.GetInstanceId().GetValue()));
            if (DgnDbStatus::Success != element->SetPropertyValue(navProp->GetName().c_str(), val))
                {
                Utf8String errorMsg;
                errorMsg.Sprintf("Failed to set NavigationECProperty on Target ECInstance for ECRelationship '%s' from element %" PRIu64 " in file '%s' "
                                 "(Source: %s|%s (%s:%s) Target %s|%s (%s:%s)). ",
                                 v8RelFullClassName.c_str(),
                                 v8Element.GetElementId(), Utf8String(v8Element.GetDgnFileP()->GetFileName().c_str()).c_str(),
                                 v8SourceKey.GetClassName().GetClassFullName().c_str(), v8SourceKey.GetInstanceId(),
                                 sourceInstanceKey.GetClassId().ToString().c_str(), sourceInstanceKey.GetInstanceId().ToString().c_str(),
                                 v8TargetKey.GetClassName().GetClassFullName().c_str(), v8TargetKey.GetInstanceId(),
                                 targetInstanceKey.GetClassId().ToString().c_str(), targetInstanceKey.GetInstanceId().ToString().c_str());

                ReportIssue(Converter::IssueSeverity::Error, Converter::IssueCategory::Sync(), Converter::Issue::Message(),
                            errorMsg.c_str());
                continue;
                }
            element->Update();
            }
        }

    return BentleyApi::SUCCESS;
    }


END_DGNDBSYNC_DGNV8_NAMESPACE
