/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "ConverterInternal.h"

BEGIN_DGNDBSYNC_DGNV8_NAMESPACE
using namespace BeSQLite::EC;
using namespace BECN;

//****************************************************************************************
// ElementConverter
//****************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            01/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ElementConverter::ConvertToElementItem(ElementConversionResults& results, ECObjectsV8::IECInstance const* v8Instance, BisConversionRule const* primaryInstanceConversionRule) const
    {
    if (v8Instance == nullptr)
        return BSISUCCESS;

    BeAssert(primaryInstanceConversionRule != nullptr);

    BECN::ECClassCP dgnDbClass = GetDgnDbClass(*v8Instance, nullptr);
    if (dgnDbClass == nullptr)
        {
        Utf8String error;
        error.Sprintf("Inserting properties for %s failed. Could not find target ECClass in the DgnDb file.", ToInstanceLabel(*v8Instance).c_str());
        m_converter.ReportIssue(Converter::IssueSeverity::Error, Converter::IssueCategory::Sync(), Converter::Issue::Error(), error.c_str());
        return BSIERROR;
        }

    ECN::IECInstancePtr targetInstance = Transform(*v8Instance, *dgnDbClass);
    if (targetInstance == nullptr)
        {
        Utf8String error;
        error.Sprintf("Inserting properties for %s failed. Transforming v8 ECInstance to ECInstance failed.", ToInstanceLabel(*v8Instance).c_str());
        m_converter.ReportIssue(Converter::IssueSeverity::Error, Converter::IssueCategory::Sync(), Converter::Issue::Error(), error.c_str());
        return BSIERROR;
        }
    results.m_v8PrimaryInstance = V8ECInstanceKey(ECClassName(v8Instance->GetClass()), v8Instance->GetInstanceId().c_str());
    return (DgnDbStatus::Success == results.m_element->SetPropertyValues(*targetInstance))? BSISUCCESS: BSIERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            02/2017
//---------------+---------------+---------------+---------------+---------------+-------
BECN::ECInstanceReadContextPtr ElementConverter::LocateInstanceReadContext(BECN::ECSchemaCR schema) const
    {
    auto it = m_instanceReadContextCache.find(schema.GetName().c_str());
    if (it != m_instanceReadContextCache.end())
        return it->second;

    BECN::ECInstanceReadContextPtr context = BECN::ECInstanceReadContext::CreateContext(schema);
    m_instanceReadContextCache[schema.GetName().c_str()] = context;
    return context;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     02/2015
//---------------------------------------------------------------------------------------
BECN::IECInstancePtr ElementConverter::Transform(ECObjectsV8::IECInstance const& v8Instance, BECN::ECClassCR dgnDbClass, bool transformAsAspect) const
    {
    Bentley::Utf8String ecInstanceXml;
    const ECObjectsV8::InstanceWriteStatus writeStat = const_cast<ECObjectsV8::IECInstance&> (v8Instance).WriteToXmlString(ecInstanceXml, true,
                                                                                                                           false); //don't bring over v8 instance id, to make clear to DgnElement that it should not attempt to update the instance and instead insert right away
    if (writeStat != ECObjectsV8::INSTANCE_WRITE_STATUS_Success)
        return nullptr;

    BECN::ECInstanceReadContextPtr context = LocateInstanceReadContext(dgnDbClass.GetSchema());
    context->SetUnitResolver(&m_unitResolver);
    context->SetSchemaRemapper(&m_schemaRemapper);
    m_schemaRemapper.SetRemapAsAspect(transformAsAspect);

    BECN::IECInstancePtr dgnDbECInstance = nullptr;
    const BECN::InstanceReadStatus readStat = BECN::IECInstance::ReadFromXmlString(dgnDbECInstance, (Utf8CP) ecInstanceXml.c_str(), *context);
    if (readStat != BECN::InstanceReadStatus::Success)
        return nullptr;

    return dgnDbECInstance;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     02/2015
//---------------------------------------------------------------------------------------
BECN::ECClassCP ElementConverter::GetDgnDbClass(ECObjectsV8::IECInstance const& v8Instance, Utf8CP aspectClassSuffix) const
    {
    ECObjectsV8::ECClassCR elementClass = v8Instance.GetClass();
    Utf8String schemaName(elementClass.GetSchema().GetName().c_str());
    if (schemaName.StartsWith("EWR"))
        schemaName.AssignOrClear("EWR");

    Utf8String className(elementClass.GetName().c_str());
    if (aspectClassSuffix != nullptr)
        className.append(aspectClassSuffix);

    return m_converter.GetDgnDb().Schemas().GetClass(schemaName.c_str(), className.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     02/2015
//---------------------------------------------------------------------------------------
//static
Utf8String ElementConverter::ToInstanceLabel(ECObjectsV8::IECInstance const& v8Instance)
    {
    Utf8String label;
    label.Sprintf("v8 '%s' instance '%s'",
                  Bentley::Utf8String(v8Instance.GetClass().GetFullName()).c_str(),
                  Bentley::Utf8String(v8Instance.GetInstanceId()).c_str());

    return label;
    }

//****************************************************************************************
// SchemaRemapper
//****************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     02/2015
//---------------------------------------------------------------------------------------
bool SchemaRemapper::_ResolveClassName(Utf8StringR serializedClassName, BECN::ECSchemaCR ecSchema) const
    {
    BisConversionRule conversionRule;
    bool hasSecondary;
    Utf8PrintfString name("%s.%s", ecSchema.GetName().c_str(), serializedClassName.c_str());
    if (!V8ECClassInfo::TryFind(conversionRule, hasSecondary, m_converter.GetDgnDb(), name))
        {
        BeAssert(false);
        return false;
        }

    if (hasSecondary && m_remapAsAspect)
        conversionRule = BisConversionRule::ToAspectOnly;
    Utf8CP suffix = BisConversionRuleHelper::GetAspectClassSuffix(conversionRule);
    if (!Utf8String::IsNullOrEmpty(suffix))
        serializedClassName.append(suffix);
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    01/2017
//---------------+---------------+---------------+---------------+---------------+-------
bool SchemaRemapper::_ResolvePropertyName(Utf8StringR serializedPropertyName, ECN::ECClassCR ecClass) const
    {
    if (!m_convSchema.IsValid())
        {
        ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
        SchemaKey key("ECv3ConversionAttributes", 1, 0);
        m_convSchema = ECSchema::LocateSchema(key, *context);
        if (!m_convSchema.IsValid())
            {
            BeAssert(false);
            return false;
            }
        }

    if (ECSchema::IsSchemaReferenced(ecClass.GetSchema(), *m_convSchema))
        {
        T_propertyNameMappings properties;
        T_ClassPropertiesMap::iterator mappedClassIter = m_renamedClassProperties.find(ecClass.GetFullName());
        if (mappedClassIter == m_renamedClassProperties.end())
            {
            IECInstancePtr renameInstance = ecClass.GetCustomAttributeLocal("ECv3ConversionAttributes", "RenamedPropertiesMapping");
            if (renameInstance.IsValid())
                {
                ECValue v;
                renameInstance->GetValue(v, "PropertyMapping");

                bvector<Utf8String> components;
                BeStringUtilities::Split(v.GetUtf8CP(), ";", components);
                for (Utf8String mapping : components)
                    {
                    bvector<Utf8String> components2;
                    BeStringUtilities::Split(mapping.c_str(), "|", components2);
                    bpair<Utf8String, Utf8String> pair(components2[0], components2[1]);
                    properties.insert(pair);
                    }
                }
            bpair<Utf8String, T_propertyNameMappings> pair2(Utf8String(ecClass.GetFullName()), properties);
            m_renamedClassProperties.insert(pair2);
            }
        else
            properties = mappedClassIter->second;

        for (T_propertyNameMappings::iterator mappedPropertiesIterator = properties.begin(); mappedPropertiesIterator != properties.end(); ++mappedPropertiesIterator)
            {
            if (mappedPropertiesIterator->first.EqualsIAscii(serializedPropertyName.c_str()))
                {
                serializedPropertyName = mappedPropertiesIterator->second;
                return true;
                }
            }
        }

    for (ECClassP baseClass : ecClass.GetBaseClasses())
        {
        if (_ResolvePropertyName(serializedPropertyName, *baseClass))
            return true;
        }

    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    06/2017
//---------------+---------------+---------------+---------------+---------------+-------
Utf8String ElementConverter::UnitResolver::_ResolveUnitName(ECPropertyCR ecProperty) const
    {
    if (!m_convSchema.IsValid())
        {
        ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
        SchemaKey key("ECv3ConversionAttributes", 1, 0);
        m_convSchema = ECSchema::LocateSchema(key, *context);
        if (!m_convSchema.IsValid())
            {
            BeAssert(false);
            return "";
            }
        }

    if (ECSchema::IsSchemaReferenced(ecProperty.GetClass().GetSchema(), *m_convSchema))
        {
        IECInstancePtr instance = ecProperty.GetCustomAttribute("ECv3ConversionAttributes", "OldPersistenceUnit");
        if (instance.IsValid())
            {
            ECValue unitName;
            instance->GetValue(unitName, "Name");

            if (!unitName.IsNull() && unitName.IsUtf8())
                return unitName.GetUtf8CP();
            }
        }
    
    return "";
    }

//****************************************************************************************
// ElementAspectConverter
//****************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     03/2015
//---------------------------------------------------------------------------------------
BentleyStatus ElementAspectConverter::ConvertToAspects(SyncInfo::V8ElementExternalSourceAspect* aspect, ElementConversionResults& results,
                                                       std::vector<std::pair<ECObjectsV8::IECInstancePtr, BisConversionRule>> const& secondaryInstances, bool isNewElement) const
    {
    for (std::pair<ECObjectsV8::IECInstancePtr, BisConversionRule> const& v8SecondaryInstance : secondaryInstances)
        {
        if (BSISUCCESS != ConvertToAspect(aspect, results, *v8SecondaryInstance.first, BisConversionRuleHelper::GetAspectClassSuffix(v8SecondaryInstance.second), isNewElement))
            return BSIERROR;
        }

    return BSISUCCESS;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     03/2015
//---------------------------------------------------------------------------------------
BentleyStatus ElementAspectConverter::ConvertToAspect(SyncInfo::V8ElementExternalSourceAspect* aspect, ElementConversionResults& results, ECObjectsV8::IECInstance const& v8Instance, Utf8CP aspectClassSuffix, bool isNewElement) const
    {
    BECN::ECClassCP aspectClass = GetDgnDbClass(v8Instance, aspectClassSuffix);
    if (aspectClass == nullptr)
        {
        Utf8String error;
        error.Sprintf("Inserting aspect for %s failed. Could not find target aspect ECClass in the DgnDb file.", ToInstanceLabel(v8Instance).c_str());
        m_converter.ReportIssue(Converter::IssueSeverity::Warning, Converter::IssueCategory::Sync(), Converter::Issue::Error(), error.c_str());
        return BSIERROR;
        }

    ECN::IECInstancePtr targetInstance = Transform(v8Instance, *aspectClass, true);
    if (targetInstance == nullptr)
        {
        Utf8String error;
        error.Sprintf("Inserting aspect for %s failed. Transforming v8 ECInstance to aspect ECInstance failed.", ToInstanceLabel(v8Instance).c_str());
        m_converter.ReportIssue(Converter::IssueSeverity::Warning, Converter::IssueCategory::Sync(), Converter::Issue::Error(), error.c_str());
        return BSIERROR;
        }

    // Need to see if there is an existing aspect or if we are updating
    bool found = false;
    if (!isNewElement && nullptr != aspect)
        {
        auto propData = aspect->GetProperties();
        if (propData.HasMember("SecondaryInstances"))
            {
            Utf8String v8Id(v8Instance.GetInstanceId().c_str());
            auto& secondary = propData["SecondaryInstances"];
            if (secondary.HasMember(rapidjson::StringRef(v8Id.c_str())))
                {
                BeSQLite::EC::ECInstanceId id(secondary[v8Id.c_str()].GetUint64());
                DgnElement::GenericMultiAspect::SetAspect(*results.m_element, *targetInstance, id);
                found = true;
                }
            }
        }

    if (!found)
        DgnElement::GenericMultiAspect::AddAspect(*results.m_element, *targetInstance);

    results.m_v8SecondaryInstanceMappings.push_back(bpair<V8ECInstanceKey, BECN::IECInstancePtr>(
        V8ECInstanceKey(ECClassName(v8Instance.GetClass()), v8Instance.GetInstanceId().c_str()),
        targetInstance));

    return BSISUCCESS;
    }

//****************************************************************************************
// V8NamedGroupInfo
//****************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     03/2015
//---------------------------------------------------------------------------------------
//static
bmap<RepositoryLinkId, bset<DgnV8Api::ElementId>> V8NamedGroupInfo::s_namedGroupsWithOwnershipHint;

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     03/2015
//---------------------------------------------------------------------------------------
//static
void V8NamedGroupInfo::AddNamedGroupWithOwnershipHint(DgnV8EhCR v8eh, Converter& converter)
    {
    s_namedGroupsWithOwnershipHint[converter.GetRepositoryLinkId(*v8eh.GetDgnFileP())].insert(v8eh.GetElementId());
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
bool V8NamedGroupInfo::TryGetNamedGroupsWithOwnershipHint(bset<DgnV8Api::ElementId> const*& namedGroupsWithOwnershipHintPerFile, RepositoryLinkId v8FileId)
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
// ECInstanceInfo
//****************************************************************************************
#define ECINSTANCE_TABLE "ECInstance"
#define TEMPTABLE_ATTACH(name) "temp." name

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     02/2015
//---------------------------------------------------------------------------------------
//static
ECInstanceKey ECInstanceInfo::Find(bool& isElement, DgnDbR db, RepositoryLinkId fileId, V8ECInstanceKey const& v8Key)
    {
    isElement = false;

    CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement(stmt, "SELECT ECClassId,ECInstanceId,IsElement FROM " TEMPTABLE_ATTACH(ECINSTANCE_TABLE) " WHERE V8SchemaName = ? AND V8ClassName = ? AND V8InstanceId = ? AND V8FileSyncInfoId = ?");
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
BentleyStatus ECInstanceInfo::Insert(DgnDbR db, RepositoryLinkId fileId, V8ECInstanceKey const& v8Key, BeSQLite::EC::ECInstanceKey const& key, bool isElement)
    {
    if (!v8Key.IsValid() || !key.IsValid())
        {
        BeAssert(false);
        return BSIERROR;
        }

    CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement(stmt, "INSERT INTO " TEMPTABLE_ATTACH(ECINSTANCE_TABLE) " (V8SchemaName,V8ClassName,V8InstanceId, ECClassId,ECInstanceId,IsElement,V8FileSyncInfoId) VALUES (?,?,?,?,?,?,?)");
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
    return db.ExecuteSql("CREATE TABLE " TEMPTABLE_ATTACH(ECINSTANCE_TABLE) " (V8SchemaName TEXT NOT NULL, V8ClassName TEXT NOT NULL, V8InstanceId TEXT NOT NULL, ECClassId INTEGER NOT NULL, ECInstanceId INTEGER NOT NULL, IsElement BOOL NOT NULL, V8FileSyncInfoId INTEGER NOT NULL,"
                         "PRIMARY KEY (V8SchemaName, V8ClassName, V8InstanceId, V8FileSyncInfoId))") == BE_SQLITE_OK ? BSISUCCESS : BSIERROR;
    }
END_DGNDBSYNC_DGNV8_NAMESPACE
