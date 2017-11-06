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

#define TEMPTABLE_ATTACH(name) "temp." name

BEGIN_DGNDBSYNC_DGNV8_NAMESPACE

using namespace BeSQLite::EC;

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
BisConversionRule BisConversionRuleHelper::ConvertToBisConversionRule(V8ElementType v8ElementType, const bool is3d, const bool namedGroupOwnsMembersFlag, bool isSecondaryInstancesClass)
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
    return !is3d ? BisConversionRule::ToDrawingGraphic : BisConversionRule::ToPhysicalElement;
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
BentleyStatus V8ECClassInfo::Insert(DynamicSchemaGenerator& converter, DgnV8EhCR v8Eh, ECClassName const& v8ClassName, bool namedGroupOwnsMembers, bool isSecondaryInstancesClass, bool is3d)
    {
    const V8ElementType v8ElementType = V8ElementTypeHelper::GetType(v8Eh);

    BisConversionRule existingRule;
    BECN::ECClassId existingClassId;
    bool hasSecondary;
    const bool classInfoFound = TryFind(existingClassId, existingRule, converter.GetDgnDb(), v8ClassName, hasSecondary);

    BisConversionRule rule = BisConversionRule::ToDefaultBisBaseClass;
    ConvertToDgnDbElementExtension* upx = ConvertToDgnDbElementExtension::Cast(v8Eh.GetHandler());
    if (nullptr != upx)
        rule = upx->_DetermineBisConversionRule(v8Eh, converter.GetDgnDb(), is3d);
    for (auto xdomain : XDomainRegistry::s_xdomains)
        xdomain->_DetermineBisConversionRule(rule, v8Eh, converter.GetDgnDb(), is3d);
    if (BisConversionRule::ToDefaultBisBaseClass == rule)
        rule = BisConversionRuleHelper::ConvertToBisConversionRule(v8ElementType, is3d, namedGroupOwnsMembers, isSecondaryInstancesClass);

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

    return SupplementV8ECSchemas();
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
    // _AddSupplementalSchemas(supplementalSchemas, *m_schemaReadContext);
    for (BECN::ECSchemaP primarySchema : primarySchemas)
        {
        if (primarySchema->IsSupplemented())
            {
            BeAssert(false && "V8 primary schemas are not expected to be supplemented already when deserialized from XML.");
            continue;
            }

        BECN::SupplementedSchemaBuilder builder;
        if (BECN::SupplementedSchemaStatus::Success != builder.UpdateSchema(*primarySchema, supplementalSchemas, false))
            {
            Utf8String error;
            error.Sprintf("Failed to supplement ECSchema '%s'. See log file for details.", Utf8String(primarySchema->GetName ()).c_str());
            ReportIssue(Converter::IssueSeverity::Warning, Converter::IssueCategory::Sync(), Converter::Issue::Message(), error.c_str());
            continue;
            }
        if (!ECN::ECSchemaConverter::Convert(*primarySchema, false))
            {
            Utf8PrintfString error("Failed to run the schema converter on v8 ECSchema '%s'", primarySchema->GetFullSchemaName().c_str());
            ReportError(Converter::IssueCategory::Sync(), Converter::Issue::Message(), error.c_str());
            return BentleyApi::BSIERROR;
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
// @bsimethod                                    Sam.Wilson                      07/14
//+---------------+---------------+---------------+---------------+---------------+------
void DynamicSchemaGenerator::AnalyzeECContent(DgnV8ModelR v8Model, bool is3d)
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

            Analyze(v8eh, is3d);
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

            Analyze(v8eh, is3d);
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     03/2015
//---------------------------------------------------------------------------------------
BentleyApi::BentleyStatus DynamicSchemaGenerator::Analyze(DgnV8Api::ElementHandle const& v8Element, bool is3d)
    {
    DoAnalyze(v8Element, is3d);
    //recurse into component elements (if the element has any)
    for (DgnV8Api::ChildElemIter childIt(v8Element); childIt.IsValid(); childIt = childIt.ToNext())
        {
        Analyze(childIt, is3d);
        }

    return BentleyApi::SUCCESS;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     03/2015
//---------------------------------------------------------------------------------------
BentleyApi::BentleyStatus DynamicSchemaGenerator::DoAnalyze(DgnV8Api::ElementHandle const& v8Element, bool is3d)
    {
    auto& v8ECManager = DgnV8Api::DgnECManager::GetManager();
    DgnV8Api::ElementECClassInfo classes;
    v8ECManager.FindECClassesOnElement(v8Element.GetElementRef(), classes);
    for (auto& ecClassInfo : classes)
        {
        auto& ecClass = ecClassInfo.first;
        bool isPrimary = ecClassInfo.second;
        
        // We fabricate the DgnV8 Tag Set Definition schema at runtime during conversion; never allow instances of that schema to be considered primary.
        if (isPrimary && ecClass.m_schemaName.Equals(GetV8TagSetDefinitionSchemaName()))
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
        if (BentleyApi::SUCCESS != V8ECClassInfo::Insert(*this, v8Element, v8ClassName, namedGroupOwnsMembers, !isPrimary, is3d))
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

    if ((RepositoryStatus::Success != GetDgnDb().BriefcaseManager().LockSchemas().Result()) || (SchemaStatus::Success != GetDgnDb().ImportSchemas(constSchemas)))
        {
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
            auto externalSchema = dgnv8EC.LocateExternalSchema(v8SchemaInfo, ECObjectsV8::SCHEMAMATCHTYPE_Exact);
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
            }

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
                    continue;
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
                    continue;
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
                        continue;
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

        if (BE_SQLITE_OK != V8ECSchemaXmlInfo::Insert(GetDgnDb(), schemaId, schemaXml.c_str()))
            {
            BeAssert(false && "Could not insert foreign schema xml");
            return BSIERROR;
            }

        m_skipECContent = false;
        }

    return BentleyApi::SUCCESS;
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
void DynamicSchemaGenerator::CheckNoECSchemaChanges(bset<DgnV8ModelP> const& uniqueModels)
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
void DynamicSchemaGenerator::BisifyV8Schemas(bset<DgnV8ModelP> const& uniqueModels)
    {
    m_converter.SetStepName(Converter::ProgressMessage::STEP_DISCOVER_ECSCHEMAS());

    SchemaConversionScope scope(*this);

    StopWatch timer(true);
    
    AddTasks((uint32_t)uniqueModels.size());

    bset<DgnV8FileP> uniqueFiles;

    for (DgnV8ModelP v8Model : uniqueModels)
        {
        RetrieveV8ECSchemas(*v8Model);
        uniqueFiles.insert(v8Model->GetDgnFileP());
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

        bool is3d = m_converter.ShouldConvertToPhysicalModel(*v8Model);
        AnalyzeECContent(*v8Model, is3d);
        if (WasAborted())
            return;
        }

    //analyze named groups in dictionary models
    for (DgnV8FileP v8File : uniqueFiles)
        {
        SetTaskName(Converter::ProgressMessage::TASK_ANALYZE_EC_CONTENT(), Converter::IssueReporter::FmtModel(v8File->GetDictionaryModel()).c_str());

        DgnV8ModelR dictionaryModel = v8File->GetDictionaryModel();
        AnalyzeECContent(dictionaryModel, nullptr);
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
        return;

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
void DynamicSchemaGenerator::GenerateSchemas(bset<DgnV8ModelP> const& uniqueModels)
    {
    if (GetConfig().GetOptionValueBool("SkipECContent", false))
        return;

    if (m_converter.IsUpdating())
        {
        CheckNoECSchemaChanges(uniqueModels);
        if (!m_needReimportSchemas)
            return;
        }
        
    BisifyV8Schemas(uniqueModels);
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
void SpatialConverterBase::MakeSchemaChanges(bset<DgnV8ModelP> const& uniqueModels)
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
        // *******
        // WARNING: GenerateSchemas calls Db::AbandonChanges if import fails! Make sure you commit your work before calling GenerateSchemas!
        // *******

        DynamicSchemaGenerator gen(*this);
        gen.GenerateSchemas(uniqueModels);

        m_skipECContent = gen.GetEcConversionFailed();
        }

    if (WasAborted())
        return;

    GetDgnDb().SaveChanges();

    // V8TagSets -> generate schemas
    // *** TBD

    // Let handler extensions import schemas
    importHandlerExtensionsSchema(*this);

    GetDgnDb().SaveChanges();

    for (auto xdomain : XDomainRegistry::s_xdomains)
        {
        if (BSISUCCESS != xdomain->_ImportSchema(*m_dgndb))
            {
            OnFatalError();
            }
        }

    if (WasAborted())
        return;

    GetDgnDb().SaveChanges();

    // This shouldn't be dependent on importing schemas.  Sometimes you want class views for just the basic Bis classes.
    if (GetConfig().GetOptionValueBool("CreateECClassViews", true))
        {
        SetStepName(Converter::ProgressMessage::STEP_CREATE_CLASS_VIEWS());
        GetDgnDb().Schemas().CreateClassViewsInDb(); // Failing to create the views should not cause errors for the rest of the conversion
        }

    GetDgnDb().SaveChanges();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void RootModelConverter::MakeSchemaChanges()
    {
    StopWatch timer(true);

    bset<DgnV8ModelP> uniqueModels(m_spatialModels);
    uniqueModels.insert(m_nonSpatialModelsInModelIndexOrder.begin(), m_nonSpatialModelsInModelIndexOrder.end());

    T_Super::MakeSchemaChanges(uniqueModels);

    ConverterLogging::LogPerformance(timer, "Convert Schemas (total)");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void TiledFileConverter::MakeSchemaChanges()
    {
    StopWatch timer(true);

    bset<DgnV8ModelP> uniqueModels;
    uniqueModels.insert(GetRootModelP());

    GetV8FileSyncInfoId(*GetRootV8File()); // DynamicSchemaGenerator et al need to assume that all V8 files are recorded in syncinfo

    T_Super::MakeSchemaChanges(uniqueModels);

    ConverterLogging::LogPerformance(timer, "Convert Schemas (total)");
    }

END_DGNDBSYNC_DGNV8_NAMESPACE
