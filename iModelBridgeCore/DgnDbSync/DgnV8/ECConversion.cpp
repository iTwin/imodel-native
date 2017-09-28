/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnV8/ECConversion.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ConverterInternal.h"

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
BisConversionRule BisConversionRuleHelper::ConvertToBisConversionRule(V8ElementType v8ElementType, DgnModel *targetModel, const bool namedGroupOwnsMembersFlag, bool isSecondaryInstancesClass)
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
    return targetModel.IsSpatialModel() || nullptr != dynamic_cast<ComponentModel*>(&targetModel) ? BisConversionRule::ToPhysicalElement : BisConversionRule::ToDrawingGraphic;
#endif
    return nullptr != targetModel && targetModel->Is2dModel() ? BisConversionRule::ToDrawingGraphic : BisConversionRule::ToPhysicalElement;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     07/2015
//---------------------------------------------------------------------------------------
//static
BentleyStatus BisConversionRuleHelper::ConvertToBisConversionRule(BisConversionRule& rule, Converter& converter, BECN::ECClassCR v8ECClass)
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
BentleyStatus V8ECClassInfo::Insert(Converter& converter, DgnV8EhCR v8Eh, ECClassName const& v8ClassName, bool namedGroupOwnsMembers, bool isSecondaryInstancesClass, DgnModel *targetModel)
    {
    const V8ElementType v8ElementType = V8ElementTypeHelper::GetType(v8Eh);

    BisConversionRule existingRule;
    BECN::ECClassId existingClassId;
    bool hasSecondary;
    const bool classInfoFound = TryFind(existingClassId, existingRule, converter.GetDgnDb(), v8ClassName, hasSecondary);

    BisConversionRule rule = BisConversionRule::ToDefaultBisBaseClass;
    ConvertToDgnDbElementExtension* upx = ConvertToDgnDbElementExtension::Cast(v8Eh.GetHandler());
    if (nullptr != upx)
        rule = upx->_DetermineBisConversionRule(v8Eh, converter.GetDgnDb(), targetModel->Is3dModel());
    for (auto xdomain : XDomainRegistry::s_xdomains)
        xdomain->_DetermineBisConversionRule(rule, v8Eh, converter.GetDgnDb(), targetModel->Is3dModel());
    if (BisConversionRule::ToDefaultBisBaseClass == rule)
        rule = BisConversionRuleHelper::ConvertToBisConversionRule(v8ElementType, targetModel, namedGroupOwnsMembers, isSecondaryInstancesClass);

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
BentleyStatus V8ECClassInfo::InsertOrUpdate(Converter& converter, ECClassName const& v8ClassName, BisConversionRule rule)
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
BentleyStatus V8ECClassInfo::Insert(Converter& converter, ECClassName const& v8ClassName, BisConversionRule rule)
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
BentleyStatus V8ECClassInfo::Update(Converter& converter, BECN::ECClassId v8ClassId, BisConversionRule rule, bool hasSecondary)
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
// ECInstanceInfo
//****************************************************************************************
#define ECINSTANCE_TABLE SYNCINFO_TABLE("ECInstance")

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     02/2015
//---------------------------------------------------------------------------------------
//static
ECInstanceKey ECInstanceInfo::Find(bool& isElement, DgnDbR db, SyncInfo::V8FileSyncInfoId fileId, V8ECInstanceKey const& v8Key)
    {
    isElement = false;

    CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement(stmt, "SELECT ECClassId,ECInstanceId,IsElement FROM " SYNCINFO_ATTACH(ECINSTANCE_TABLE) " WHERE V8SchemaName = ? AND V8ClassName = ? AND V8InstanceId = ? AND V8FileSyncInfoId = ?");
    if (stat != BE_SQLITE_OK)
        {
        BeAssert(false && "Could not retrieve cached statement.");
        return ECInstanceKey();
        }

    stmt->BindText(1, v8Key.GetClassName().GetSchemaName(), Statement::MakeCopy::No);
    stmt->BindText(2, v8Key.GetClassName().GetClassName(), Statement::MakeCopy::No);
    stmt->BindText(3, v8Key.GetInstanceId(), Statement::MakeCopy::No);
    stmt->BindInt64(4, fileId.GetValue());

    if (stmt->Step() != BE_SQLITE_ROW)
        return ECInstanceKey();

    ECInstanceKey foundKey(stmt->GetValueId<BECN::ECClassId>(0), stmt->GetValueId<ECInstanceId>(1));
    isElement = stmt->GetValueInt(2) == 1;

    if (stmt->Step() == BE_SQLITE_ROW)
        {
        BeAssert(false && "Only one result row expected when looking up instance key for v8 instance key.");
        return ECInstanceKey();
        }

    return foundKey;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     02/2015
//---------------------------------------------------------------------------------------
//static
BentleyStatus ECInstanceInfo::Insert(DgnDbR db, SyncInfo::V8FileSyncInfoId fileId, V8ECInstanceKey const& v8Key, BeSQLite::EC::ECInstanceKey const& key, bool isElement)
    {
    if (!v8Key.IsValid() || !key.IsValid())
        {
        BeAssert(false);
        return BSIERROR;
        }

    CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement(stmt, "INSERT INTO " SYNCINFO_ATTACH(ECINSTANCE_TABLE) " (V8SchemaName,V8ClassName,V8InstanceId, ECClassId,ECInstanceId,IsElement,V8FileSyncInfoId) VALUES (?,?,?,?,?,?,?)");
    if (stat != BE_SQLITE_OK)
        {
        BeAssert(false && "Could not retrieve cached statement for ECInstanceInfo::Insert.");
        return BSIERROR;
        }

    stmt->BindText(1, v8Key.GetClassName().GetSchemaName(), Statement::MakeCopy::No);
    stmt->BindText(2, v8Key.GetClassName().GetClassName(), Statement::MakeCopy::No);
    stmt->BindText(3, v8Key.GetInstanceId(), Statement::MakeCopy::No);
    stmt->BindId(4, key.GetClassId());
    stmt->BindId(5, key.GetInstanceId());
    stmt->BindInt(6, isElement ? 1 : 0);
    stmt->BindInt64(7, fileId.GetValue());
    stat = stmt->Step();

    return stat == BE_SQLITE_DONE ? BSISUCCESS : BSIERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     02/2015
//---------------------------------------------------------------------------------------
//static
BentleyStatus ECInstanceInfo::CreateTable(DgnDbR db)
    {
    if (db.TableExists(SYNCINFO_ATTACH(ECINSTANCE_TABLE)))
        return BSISUCCESS;

    return db.ExecuteSql("CREATE TABLE " SYNCINFO_ATTACH(ECINSTANCE_TABLE) " (V8SchemaName TEXT NOT NULL, V8ClassName TEXT NOT NULL, V8InstanceId TEXT NOT NULL, ECClassId INTEGER NOT NULL, ECInstanceId INTEGER NOT NULL, IsElement BOOL NOT NULL, V8FileSyncInfoId INTEGER NOT NULL,"
                         "PRIMARY KEY (V8SchemaName, V8ClassName, V8InstanceId, V8FileSyncInfoId))") == BE_SQLITE_OK ? BSISUCCESS : BSIERROR;
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
// V8NamedGroupInfo
//****************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     03/2015
//---------------------------------------------------------------------------------------
//static
bmap<SyncInfo::V8FileSyncInfoId, bset<DgnV8Api::ElementId>> V8NamedGroupInfo::s_namedGroupsWithOwnershipHint;

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     03/2015
//---------------------------------------------------------------------------------------
//static
void V8NamedGroupInfo::AddNamedGroupWithOwnershipHint(DgnV8EhCR v8eh)
    {
    s_namedGroupsWithOwnershipHint[Converter::GetV8FileSyncInfoIdFromAppData(*v8eh.GetDgnFileP())].insert(v8eh.GetElementId());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2017
//---------------+---------------+---------------+---------------+---------------+-------
//static
void V8NamedGroupInfo::Reset()
    {
    s_namedGroupsWithOwnershipHint.clear();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     03/2015
//---------------------------------------------------------------------------------------
//static
bool V8NamedGroupInfo::TryGetNamedGroupsWithOwnershipHint(bset<DgnV8Api::ElementId> const*& namedGroupsWithOwnershipHintPerFile, SyncInfo::V8FileSyncInfoId v8FileId)
    {
    auto it = s_namedGroupsWithOwnershipHint.find(v8FileId);
    const bool found = it != s_namedGroupsWithOwnershipHint.end();

    if (found)
        namedGroupsWithOwnershipHintPerFile = &it->second;
    else
        namedGroupsWithOwnershipHintPerFile = nullptr;

    return found;
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
BentleyStatus ECSchemaXmlDeserializer::DeserializeSchemas(BECN::ECSchemaReadContextR schemaContext, BECN::SchemaMatchType matchType, Converter& converter)
    {
    m_schemaCache.Clear();

    converter.AddTasks(m_schemaXmlMap.size());
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
                converter.ReportIssue(Converter::IssueSeverity::Warning, Converter::IssueCategory::Sync(), Converter::Issue::Message(), warning.c_str());
                continue;
                }
            Utf8String error;
            error.Sprintf("Failed to deserialize v8 ECSchema '%s'.", schemaKey.GetFullSchemaName().c_str());
            converter.ReportError(Converter::IssueCategory::Sync(), Converter::Issue::Message(), error.c_str());
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


END_DGNDBSYNC_DGNV8_NAMESPACE
