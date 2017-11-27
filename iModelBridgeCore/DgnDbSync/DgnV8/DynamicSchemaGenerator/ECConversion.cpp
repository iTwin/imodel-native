/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnV8/DynamicSchemaGenerator/ECConversion.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ConverterInternal.h"
#include "ECConversion.h"
#include "ECDiff.h"
#include <regex>
#include <ECObjects/StandardCustomAttributeHelper.h>

#define TEMPTABLE_ATTACH(name) "temp." name

BEGIN_DGNDBSYNC_DGNV8_NAMESPACE

using namespace BeSQLite::EC;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
static bool anyTxnsInFile(DgnDbR db)
    {
    Statement stmt;
    stmt.Prepare(db, "SELECT Id FROM " DGN_TABLE_Txns " LIMIT 1");
    return (BE_SQLITE_ROW == stmt.Step());
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
    return BECN::SchemaKey(m_sql->GetValueText(0),
        (uint32_t) m_sql->GetValueInt(1),
                           (uint32_t) m_sql->GetValueInt(2));
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

        m_converter.SetTaskName(Converter::ProgressMessage::TASK_MERGING_V8_ECSCHEMA(), kvPairs.first.c_str());
        schemaIter++;
        for (; schemaIter != kvPairs.second.end(); schemaIter++)
            {
//            ReportProgress();
            BECN::ECSchemaPtr rightSchema;
            if (BECN::SchemaReadStatus::Success != BECN::ECSchema::ReadFromXmlString(rightSchema, schemaIter->second.c_str(), schemaContext))
                return nullptr;
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
                    Utf8String xml;
                    merged->WriteToXmlString(xml);
                    leftSchema->ReComputeCheckSum();
                    LOG.infov("Merged two versions of ECSchema '%s' successfully. Updated checksum: 0x%llx", leftSchema->GetFullSchemaName().c_str(), leftSchema->GetSchemaKey().m_checkSum);
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
    m_schemaXmlMap[schemaName].push_back({key, xml});
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     02/2015
//---------------------------------------------------------------------------------------
BentleyStatus ECSchemaXmlDeserializer::DeserializeSchemas(BECN::ECSchemaReadContextR schemaContext, BECN::SchemaMatchType matchType)
    {
    m_schemaCache.Clear();

    m_converter.AddTasks(m_schemaXmlMap.size());
    //Prefer ECDb and standard schemas over ones embedded in DGN file.
    schemaContext.SetFinalSchemaLocater(*this);

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

static bool s_doFileSaveTimeCheck = true;

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
        Utf8StringCR schemaName = key.GetName();
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
         "ams", "bmf", "pid", "schematics", "OpenPlant_PID", "OpenPlant3D_PID", "speedikon", "autoplant_PIW", "ECXA_autoplant_PIW", "Bentley_Plant", "globals", "Electrical_RCM", "pid_ansi"};
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
         }

     if (needsFlattening)
         {
         for (BECN::ECSchemaP schema : schemas)
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

     for (BECN::ECSchemaP schema : schemas)
         {
         if (schema->IsSupplementalSchema())
             continue;
         if (!ECN::ECSchemaConverter::Convert(*schema, false))
             {
             Utf8PrintfString error("Failed to run the schema converter on v8 ECSchema '%s'", schema->GetFullSchemaName().c_str());
             ReportError(Converter::IssueCategory::Sync(), Converter::Issue::Message(), error.c_str());
             return BentleyApi::BSIERROR;
             }
         }

     return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2017
//---------------+---------------+---------------+---------------+---------------+-------
void DynamicSchemaGenerator::SwizzleOpenPlantSupplementals(bvector<BECN::ECSchemaPtr>& tmpSupplementals, BECN::ECSchemaP primarySchema, bvector<BECN::ECSchemaP> supplementalSchemas)
    {
    bool foundSupplemental = false;
    bvector<BECN::ECSchemaP> units;
    Utf8PrintfString suppName("%s_Supplemental_Units", primarySchema->GetName().c_str());
    for (BECN::ECSchemaP supp : supplementalSchemas)
        {
        if (supp->GetName().StartsWithIAscii(suppName.c_str()))
            foundSupplemental = true;
        else if (supp->GetName().StartsWithIAscii("OpenPlant_Supplemental_Units"))
            units.push_back(supp);
        }
    if (!foundSupplemental)
        {
        for (BECN::ECSchemaP unitSchema : units)
            {
            BECN::ECSchemaPtr op3d;
            if (BECN::ECObjectsStatus::Success != unitSchema->CopySchema(op3d))
                {
                Utf8String error;
                error.Sprintf("Failed to create an %s copy of the units schema '%s'; Unit information will be unavailable. See log file for details.", primarySchema->GetName().c_str(), (unitSchema->GetName()).c_str());
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
                error.Sprintf("Failed to get supplemental metadata from supplemental units schema '%s'; Unit information will be unavailable. See log file for details.", Utf8String(unitSchema->GetName()).c_str());
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
            error.Sprintf("Failed to find ECSchema '%s' for custom attribute '%'.  Skipping custom attribute.", Utf8String(instance->GetClass().GetFullName()));
            ReportIssue(Converter::IssueSeverity::Warning, Converter::IssueCategory::Sync(), Converter::Issue::Message(), error.c_str());
            continue;
            }
        ECN::IECInstancePtr copiedCA = instance->CreateCopyThroughSerialization(*flatCustomAttributeSchema);
        if (!copiedCA.IsValid())
            {
            Utf8String error;
            error.Sprintf("Failed to copy custom attribute '%s'. Skipping custom attribute.", Utf8String(instance->GetClass().GetFullName()));
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
        if (targetClass->Is(targetPropertyClass))
            return BSISUCCESS;
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

// Create the pseudo polymorphic hierarchy by keeping track of any derived class that was lost.
//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2017
//---------------+---------------+---------------+---------------+---------------+-------
void addDroppedDerivedClass(ECN::ECClassP baseClass, ECN::ECClassP derivedClass)
    {
    ECN::IECInstancePtr droppedInstance = baseClass->GetCustomAttributeLocal("ECv3ConversionAttributes", "OldDerivedClasses");
    if (!droppedInstance.IsValid())
        droppedInstance = ECN::ConversionCustomAttributeHelper::CreateCustomAttributeInstance("OldDerivedClasses");
    if (!droppedInstance.IsValid())
        {
        LOG.warningv("Failed to create 'OldDerivedClasses' custom attribute for ECClass '%s'", baseClass->GetFullName());
        return;
        }
    ECN::ECValue v;
    droppedInstance->GetValue(v, "Classes");
    Utf8String classes("");
    if (!v.IsNull())
        classes = Utf8String(v.GetUtf8CP()).append(";");

    classes.append(derivedClass->GetFullName());

    v.SetUtf8CP(classes.c_str());
    if (ECN::ECObjectsStatus::Success != droppedInstance->SetValue("Classes", v))
        {
        LOG.warningv("Failed to create 'OldDerivedClasses' custom attribute for the ECClass '%s' with 'Classes' set to '%s'.", baseClass->GetFullName(), classes.c_str());
        return;
        }

    if (!ECN::ECSchema::IsSchemaReferenced(baseClass->GetSchemaR(), droppedInstance->GetClass().GetSchema()))
        {
        ECN::ECClassP nonConstClass = const_cast<ECN::ECClassP>(&droppedInstance->GetClass());
        if (ECN::ECObjectsStatus::Success != baseClass->GetSchemaR().AddReferencedSchema(nonConstClass->GetSchemaR()))
            {
            LOG.warningv("Failed to add %s as a referenced schema to %s.", droppedInstance->GetClass().GetSchema().GetName().c_str(), baseClass->GetSchemaR().GetName().c_str());
            LOG.warningv("Failed to add 'OldDerivedClasses' custom attribute to ECClass '%s'.", baseClass->GetFullName());
            return;
            }
        }

    if (ECN::ECObjectsStatus::Success != baseClass->SetCustomAttribute(*droppedInstance))
        {
        LOG.warningv("Failed to add 'OldDerivedClasses' custom attribute, with 'PropertyMapping' set to '%s', to ECClass '%s'.", classes.c_str(), baseClass->GetFullName());
        return;
        }

    LOG.debugv("Successfully added OldDerivedClasses custom attribute to ECClass '%s'", baseClass->GetFullName());
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

        CopyFlatCustomAttributes(*flatSchema, *sourceSchema);

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
                        addDroppedDerivedClass(flatBase, targetClass);
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
                    addDroppedDerivedClass(flatBase, targetClass);
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
        m_schemaReadContext->GetCache().DropSchema(sourceSchema->GetSchemaKey());
        m_schemaReadContext->AddSchema(*m_flattenedRefs[sourceSchema->GetName()]);
        }

    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2017
//---------------+---------------+---------------+---------------+---------------+-------
void DynamicSchemaGenerator::ProcessSP3DSchema(ECN::ECSchemaP schema, ECN::ECClassCP baseInterface, ECN::ECClassCP baseObject)
    {
    bool wasFlattened = false;
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
                addDroppedDerivedClass(baseClass, ecClass);
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
        
        // We fabricate the DgnV8 Tag Set Definition schema at runtime during conversion; never allow instances of that schema to be considered primary.
        if (isPrimary && ecClass.m_schemaName.Equals(Converter::GetV8TagSetDefinitionSchemaName()))
            isPrimary = false;
        
        ECClassName v8ClassName(Utf8String(ecClass.m_schemaName.c_str()).c_str(), Utf8String(ecClass.m_className.c_str()).c_str());
        ECN::SchemaKey conversionKey(Utf8String(v8ClassName.GetSchemaName()).append("_DgnDbSync").c_str(), 1, 0);
        ECN::ECSchemaPtr conversionSchema = m_syncReadContext->LocateSchema(conversionKey, ECN::SchemaMatchType::Latest);
        bool namedGroupOwnsMembers = false;
        if (conversionSchema.IsValid())
            {
            ECN::ECClassCP ecClass = conversionSchema->GetClassCP(v8ClassName.GetClassName());
            if (nullptr != ecClass)
                namedGroupOwnsMembers = ecClass->GetCustomAttribute("NamedGroupOwnsMembers") != nullptr;
            }
        if (BentleyApi::SUCCESS != V8ECClassInfo::Insert(*this, v8Element, v8ClassName, namedGroupOwnsMembers, !isPrimary, targetModelInfo))
            return BSIERROR;
        if (!isPrimary && (BentleyApi::SUCCESS != V8ElementSecondaryECClassInfo::Insert(GetDgnDb(), v8Element, v8ClassName)))
            return BSIERROR;
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

    for (BECN::ECRelationshipClassP relationshipClass : relationshipClasses)
        {
        if (BisClassConverter::ConvertECRelationshipClass(context, *relationshipClass, m_syncReadContext.get()) != BentleyApi::SUCCESS)
            return BentleyApi::BSIERROR;
        }

    for (bpair<Utf8String, BECN::ECSchemaP> const& kvpair : context.GetSchemas())
        {
        BECN::ECSchemaP schema = kvpair.second;
        if (context.ExcludeSchemaFromBisification(*schema))
            continue;

        if (!schema->Validate(true) || !schema->IsECVersion(ECN::ECVersion::V3_1))
            {
            Utf8String errorMsg;
            errorMsg.Sprintf("Failed to validate ECSchema %s as an EC3.1 ECSchema.", schema->GetFullSchemaName().c_str());
            ReportIssue(Converter::IssueSeverity::Error, Converter::IssueCategory::Sync(), Converter::Issue::Message(), errorMsg.c_str());
            return BentleyApi::BSIERROR;
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
    // Once we've constructed the handler info, we need only retain those property names which are used in SELECT statements.
    auto removeAt = std::remove_if(constSchemas.begin(), constSchemas.end(), [&] (BECN::ECSchemaCP const& arg) { return arg->IsStandardSchema() || arg->IsSystemSchema(); });
    constSchemas.erase(removeAt, constSchemas.end());

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

    return BentleyApi::SUCCESS;
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
            const size_t xmlByteSize = schemaXml.length() * sizeof(Utf8Char);
            schemaKey.m_checkSum = BECN::ECSchema::ComputeSchemaXmlStringCheckSum(schemaXml.c_str(), xmlByteSize);

            isDynamicSchema = IsDynamicSchema(schemaName, schemaXml);
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
    Bentley::Utf8String schemaName(schemaKey.GetName());
    BECN::SchemaKey existingSchemaKey;
    SyncInfo::ECSchemaMappingType existingMappingType = SyncInfo::ECSchemaMappingType::Identity;
    if (GetSyncInfo().TryGetECSchema(existingSchemaKey, existingMappingType, schemaName.c_str()))
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
                              Utf8String(existingSchemaKey.GetFullSchemaName()).c_str(), Converter::IssueReporter::FmtModel(v8Model).c_str());
                ReportIssue(Converter::IssueSeverity::Warning, Converter::IssueCategory::Sync(), Converter::Issue::Message(), error.c_str());
                return BSISUCCESS;
                }

            const int majorDiff = existingSchemaKey.GetVersionRead() - schemaKey.GetVersionMajor();
            const int minorDiff = existingSchemaKey.GetVersionMinor() - schemaKey.GetVersionMinor();
            const int existingToNewVersionDiff = majorDiff != 0 ? majorDiff : minorDiff;

            if (existingToNewVersionDiff >= 0)
                {
                if (existingToNewVersionDiff == 0 && existingSchemaKey.m_checkSum != schemaKey.m_checkSum)
                    {
                    Utf8String error;
                    error.Sprintf("ECSchema %s already found in the V8 file with a different checksum (%u). Copy in model %s with checksum %u will be merged.  This may result in inconsistencies between the DgnDb version and the versions in the Dgn.",
                                  existingSchemaKey.GetFullSchemaName().c_str(), existingSchemaKey.m_checkSum,
                                  Converter::IssueReporter::FmtModel(v8Model).c_str(), schemaKey.m_checkSum);
                    ReportIssue(Converter::IssueSeverity::Warning, Converter::IssueCategory::Sync(), Converter::Issue::Message(), error.c_str());
                    }
                else
                    return BSISUCCESS;
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
    return BeStringUtilities::Strnicmp(schemaName.c_str(),"PFLModule", 9) == 0 ||
        schemaName.EqualsI("CivilSchema_iModel") ||
        schemaName.EqualsI("BuildingDataGroup");
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
    bmap<Utf8String, uint32_t> syncInfoChecksums;
    GetSyncInfo().RetrieveECSchemaChecksums(syncInfoChecksums);

    for (auto& v8Model : uniqueModels)
        CheckECSchemasForModel(*v8Model, syncInfoChecksums);
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
        if (ECN::ECSchema::IsStandardSchema(Utf8String(v8SchemaInfo.GetSchemaName())))
            continue;

        bmap<Utf8String, uint32_t>::const_iterator syncEntry = syncInfoChecksums.find(Utf8String(v8SchemaInfo.GetSchemaName()));
        // If schema was not in the original DgnDb, we need to import it
        if (syncEntry == syncInfoChecksums.end())
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
                Utf8PrintfString msg("Could not read v8 ECSchema XML for '%s'.", Utf8String(v8SchemaInfo.GetSchemaName()).c_str());
                ReportSyncInfoIssue(Converter::IssueSeverity::Fatal, Converter::IssueCategory::MissingData(), Converter::Issue::Error(), msg.c_str());
                OnFatalError(Converter::IssueCategory::MissingData());
                return;
                }

            schemaXml = Bentley::Utf8String(schemaXmlW);
            const size_t xmlByteSize = schemaXml.length() * sizeof(Utf8Char);
            checksum = BECN::ECSchema::ComputeSchemaXmlStringCheckSum(schemaXml.c_str(), xmlByteSize);
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
                Utf8PrintfString msg("Could not locate external v8 ECSchema '%s'", Utf8String(v8SchemaInfo.GetSchemaName()).c_str());
                ReportSyncInfoIssue(Converter::IssueSeverity::Fatal, Converter::IssueCategory::MissingData(), Converter::Issue::Error(), msg.c_str());
                OnFatalError(Converter::IssueCategory::MissingData());
                return;
                }
            if (ECObjectsV8::SCHEMA_WRITE_STATUS_Success != externalSchema->WriteToXmlString(schemaXml))
                {
                Utf8PrintfString msg("Could not serialize external v8 ECSchema '%s'", Utf8String(v8SchemaInfo.GetSchemaName()).c_str());
                ReportSyncInfoIssue(Converter::IssueSeverity::Fatal, Converter::IssueCategory::CorruptData(), Converter::Issue::Error(), "Could not serialize external v8 ECSchema.");
                OnFatalError(Converter::IssueCategory::CorruptData());
                return;
                }
            checksum = v8SchemaInfo.GetSchemaKey().m_checkSum;
            }
        if (checksum != syncEntry->second)
            {
            Utf8PrintfString msg("v8 ECSchema '%s' checksum is different from stored schema", Utf8String(v8SchemaInfo.GetSchemaName()).c_str());
            ReportSyncInfoIssue(Converter::IssueSeverity::Fatal, Converter::IssueCategory::InconsistentData(), Converter::Issue::ConvertFailure(), msg.c_str());
            OnFatalError(Converter::IssueCategory::InconsistentData());
            return;
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
    if (_WantProvenanceInBim() && !m_dgndb->TableExists(DGN_TABLE_ProvenanceFile))
        {
        DgnV8FileProvenance::CreateTable(*m_dgndb);
        DgnV8ModelProvenance::CreateTable(*m_dgndb);
        DgnV8ElementProvenance::CreateTable(*m_dgndb);
        }
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
    _GetParamsR().SetIsUpdating(FindImportJobForModel(*GetRootModelP()).IsValid());

#ifndef NDEBUG
    if (_WantProvenanceInBim())
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
            _ConvertDgnV8Tags(filesInOrder);
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
        }

    if (WasAborted())
        return BSIERROR;

    GetDgnDb().SaveChanges();

    // Let handler extensions import schemas
    importHandlerExtensionsSchema(*this);

    if (WasAborted())
        return BSIERROR;

    GetDgnDb().SaveChanges();

    for (auto xdomain : XDomainRegistry::s_xdomains)
        {
        if (BSISUCCESS != xdomain->_ImportSchema(*m_dgndb))
            {
            OnFatalError();
            }
        }

    if (WasAborted())
        return BSIERROR;

    GetDgnDb().SaveChanges();

    // This shouldn't be dependent on importing schemas.  Sometimes you want class views for just the basic Bis classes.
    if (GetConfig().GetOptionValueBool("CreateECClassViews", true))
        {
        if (!_GetParamsR().IsUpdating() || anyTxnsInFile(GetDgnDb()))   // don't regenerate class views unless we know that there are new or modified schemas
            {
            SetStepName(Converter::ProgressMessage::STEP_CREATE_CLASS_VIEWS());
            GetDgnDb().Schemas().CreateClassViewsInDb(); // Failing to create the views should not cause errors for the rest of the conversion
            }
        }

    GetDgnDb().SaveChanges();
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RootModelConverter::MakeSchemaChanges()
    {
    StopWatch timer(true);

    bvector<DgnV8ModelP> modelsInFMOrder;   // models in file, modelid order - that matches the order in which the older converter processed models.
    modelsInFMOrder.assign(m_spatialModelsInAttachmentOrder.begin(), m_spatialModelsInAttachmentOrder.end());
    modelsInFMOrder.insert(modelsInFMOrder.end(), m_nonSpatialModelsInModelIndexOrder.begin(), m_nonSpatialModelsInModelIndexOrder.end());

    auto cmp = [&](DgnV8ModelP a, DgnV8ModelP b) { return IsLessInMappingOrder(a,b); };
    std::sort(modelsInFMOrder.begin(), modelsInFMOrder.end(), cmp);

    auto status = T_Super::MakeSchemaChanges(m_v8Files, modelsInFMOrder);

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

    GetV8FileSyncInfoId(*GetRootV8File()); // DynamicSchemaGenerator et al need to assume that all V8 files are recorded in syncinfo

    auto status = T_Super::MakeSchemaChanges(filesInOrder, modelsInOrder);

    ConverterLogging::LogPerformance(timer, "Convert Schemas (total)");

    return status;
    }

END_DGNDBSYNC_DGNV8_NAMESPACE
