/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnV8/RootModelConverter.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ConverterInternal.h"
#include <regex>

// We enter this namespace in order to avoid having to qualify all of the types, such as bmap, that are common
// to bim and v8. The problem is that the V8 Bentley namespace is shifted in.
BEGIN_DGNDBSYNC_DGNV8_NAMESPACE

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
Converter::SchemaConversionScope::SchemaConversionScope(Converter& converter)
: m_converter(converter), m_succeeded(false)
    {
    m_converter.InitializeECSchemaConversion();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   07/2015
//---------------------------------------------------------------------------------------
Converter::SchemaConversionScope::~SchemaConversionScope()
    {
    m_converter.FinalizeECSchemaConversion();
    if (!m_succeeded)
        {
        m_converter.SetSkipECContent(true);
        m_converter.ReportError(Converter::IssueCategory::Sync(), Converter::Issue::Error(), "Failed to transform the v8 ECSchemas to a BIS based ECSchema. Therefore EC content is not converted. See logs for details. Please try to adjust the v8 ECSchemas.");
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   11/2014
//---------------------------------------------------------------------------------------
BentleyApi::BentleyStatus Converter::ConsolidateV8ECSchemas()
    {
    if (m_skipECContent)
        return BentleyApi::SUCCESS;

    ECSchemaXmlDeserializer schemaXmlDeserializer(*this);
    bset<Utf8String> targetSchemaNames;
//#define EXPORT_V8SCHEMA_XML 1
#ifdef EXPORT_V8SCHEMA_XML
    BeFileName bimFileName = m_dgndb->GetFileName();
    BeFileName outFolder = bimFileName.GetDirectoryName().AppendToPath(bimFileName.GetFileNameWithoutExtension().AppendUtf8("_V8").c_str());
    if (!outFolder.DoesPathExist())
        BeFileName::CreateNewDirectory(outFolder.GetName());

#endif

    for (auto const& entry : V8ECSchemaXmlInfo::Iterable(*m_dgndb))
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

     if (BentleyApi::SUCCESS != schemaXmlDeserializer.DeserializeSchemas(*m_schemaReadContext, ECN::SchemaMatchType::Latest, *this))
         {
         ReportError(IssueCategory::Unknown(), Issue::ConvertFailure(), "Failed to merge dynamic V8 ECSchemas.");
         BeAssert(false && "Failed to merge dynamic V8 ECSchemas.");
         return BSIERROR;
         }

    return SupplementV8ECSchemas();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   03/2015
//---------------------------------------------------------------------------------------
BentleyApi::BentleyStatus Converter::SupplementV8ECSchemas()
    {
    DgnPlatformLib::Host* host = DgnPlatformLib::QueryHost();
    if (host == nullptr)
        {
        BeAssert(false && "Could not retrieve Graphite DgnPlatformLib Host.");
        return BSIERROR;
        }

    BeFileName supplementalECSchemasDir = _GetParams().GetAssetsDir();
    supplementalECSchemasDir.AppendToPath(L"ECSchemas");
    supplementalECSchemasDir.AppendToPath(L"Supplemental");

    if (!supplementalECSchemasDir.DoesPathExist())
        {
        Utf8String error;
        error.Sprintf("Could not find deployed system supplemental ECSchemas.Directory '%s' does not exist.", supplementalECSchemasDir.GetNameUtf8().c_str());
        ReportIssue(IssueSeverity::Fatal, Converter::IssueCategory::Sync(), Converter::Issue::Message(), error.c_str());
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
                ReportIssue(IssueSeverity::Warning, Converter::IssueCategory::Sync(), Converter::Issue::Message(), error.c_str());
                continue;
                }

            supplementalSchemas.push_back(supplementalSchema.get ());
            }
        }
    _AddSupplementalSchemas(supplementalSchemas, *m_schemaReadContext);
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
            ReportIssue(IssueSeverity::Warning, Converter::IssueCategory::Sync(), Converter::Issue::Message(), error.c_str());
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
void Converter::AnalyzeECContent(DgnV8ModelR v8Model, DgnModel* targetModel)
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

            Analyze(v8eh, targetModel);
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

            Analyze(v8eh, targetModel);
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     03/2015
//---------------------------------------------------------------------------------------
BentleyApi::BentleyStatus Converter::Analyze(DgnV8Api::ElementHandle const& v8Element, DgnModel* targetModel)
    {
    DoAnalyze(v8Element, targetModel);
    //recurse into component elements (if the element has any)
    for (DgnV8Api::ChildElemIter childIt(v8Element); childIt.IsValid(); childIt = childIt.ToNext())
        {
        Analyze(childIt, targetModel);
        }

    return BentleyApi::SUCCESS;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     03/2015
//---------------------------------------------------------------------------------------
BentleyApi::BentleyStatus Converter::DoAnalyze(DgnV8Api::ElementHandle const& v8Element, DgnModel* targetModel)
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
        if (BentleyApi::SUCCESS != V8ECClassInfo::Insert(*this, v8Element, v8ClassName, namedGroupOwnsMembers, !isPrimary, targetModel))
            return BSIERROR;
        if (!isPrimary && (BentleyApi::SUCCESS != V8ElementSecondaryECClassInfo::Insert(*m_dgndb, v8Element, v8ClassName)))
            return BSIERROR;
        }

    return BentleyApi::SUCCESS;
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
    SyncInfo::V8FileSyncInfoId fileId = GetV8FileSyncInfoIdFromAppData(*v8Element.GetDgnFileP());

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

        // If the relationship class inherits from one of the two biscore base relationship classes, then it is a link table relationship, and can use the API
        if (relClass->Is(BIS_ECSCHEMA_NAME, BIS_REL_ElementRefersToElements) || relClass->Is(BIS_ECSCHEMA_NAME, BIS_REL_ElementOwnsMultiAspects))
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
        // Otherwise, the converter should have created a navigation property on the target class, so we need to set the target instance's ECValue
        ECN::ECPropertyP prop = targetClass->GetPropertyP(relClass->GetName().c_str());
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

        DgnElementPtr element = m_dgndb->Elements().GetForEdit<DgnElement>(DgnElementId(targetInstanceKey.GetInstanceId().GetValue()));
        if (targetClass->Is(BIS_ECSCHEMA_NAME, BIS_CLASS_ElementAspect))
            {
            DgnElement::MultiAspect* aspect = DgnElement::MultiAspect::GetAspectP(*element, *targetClass, targetInstanceKey.GetInstanceId());
            if (nullptr == aspect)
                {
                Utf8String errorMsg;
                errorMsg.Sprintf("Unable to get ElementAspect."
                                 "and therefore the conversion process should have created a NavigationECProperty on the ECClass."
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
                errorMsg.Sprintf("Failed to set NavigationECProperty on Target ElementAspect ECInstance for ECRelationship '%s' from element %" PRIu64 " in file '%s' "
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
BentleyApi::BentleyStatus Converter::ConvertToBisBasedECSchemas()
    {
    BisClassConverter::SchemaConversionContext context(*this, *m_schemaReadContext, *m_syncReadContext, m_config.GetOptionValueBool("AutoDetectMixinParams", true));

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
bool DirectlyReferences(ECN::ECSchemaCP thisSchema, ECN::ECSchemaCP possiblyReferencedSchema)
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
bool DependsOn(ECN::ECSchemaCP thisSchema, ECN::ECSchemaCP possibleDependency)
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
void InsertSchemaInDependencyOrderedList(bvector<ECN::ECSchemaP>& schemas, ECN::ECSchemaP insertSchema)
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
void BuildDependencyOrderedSchemaList(bvector<ECN::ECSchemaP>& schemas, ECN::ECSchemaP insertSchema)
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
BentleyApi::BentleyStatus Converter::ImportTargetECSchemas()
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

    if (SchemaStatus::Success != GetDgnDb().ImportV8LegacySchemas(constSchemas))
        {
        return BentleyApi::ERROR;
        }

    GetChangeDetector()._OnSchemasConverted(*this, constSchemas);

    if (m_config.GetOptionValueBool("CreateECClassViews", true))
        {
        SetStepName(Converter::ProgressMessage::STEP_CREATE_CLASS_VIEWS());
        GetDgnDb().Schemas().CreateClassViewsInDb(); // Failing to create the views should not cause errors for the rest of the conversion
        }

    return BentleyApi::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   10/2014
//---------------------------------------------------------------------------------------
void Converter::InitializeECSchemaConversion()
    {
    //set-up required schema locaters and search paths
    m_schemaReadContext = ECN::ECSchemaReadContext::CreateContext();
    m_syncReadContext = ECN::ECSchemaReadContext::CreateContext();

    m_schemaReadContext->AddSchemaLocater(GetDgnDb().GetSchemaLocater());
    auto host = DgnPlatformLib::QueryHost();
    if (host != nullptr)
        {
        BeFileName ecschemasDir = _GetParams().GetAssetsDir();
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
void Converter::FinalizeECSchemaConversion()
    {
    m_schemaReadContext = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/15
+---------------+---------------+---------------+---------------+---------------+------*/
void RootModelConverter::_ConvertSpatialViews()
    {
    if (IsUpdating())
        return;         // pity, but we don't have the logic to *update* previously converted views.

    if (!m_viewGroup.IsValid())
        m_viewGroup = m_rootFile->GetViewGroupsR().FindByModelId(GetRootModelP()->GetModelId(), true, -1);
    if (!m_viewGroup.IsValid())
        {
        DgnV8Api::ViewGroupStatus vgStatus;
        if (DgnV8Api::VG_Success != (vgStatus = DgnV8Api::ViewGroup::Create(m_viewGroup, *GetRootModelP(), true, NULL, true)))
            return;
        }

    SpatialViewFactory vf(*this);

    DgnViewId firstView;
    ConvertViewGroup(firstView, *m_viewGroup, m_rootTrans, vf);

    if (firstView.IsValid() && !m_defaultViewId.IsValid())
        m_defaultViewId = firstView;

    NamedViewCollectionCR namedViews = GetRootV8File()->GetNamedViews();
    for (DgnV8Api::NamedViewPtr namedView : namedViews)
        ConvertNamedView(*namedView, m_rootTrans, vf);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnV8Api::DgnFileStatus RootModelConverter::_InitRootModel()
    {
    // don't bother to convert a DWG master file - let DwgImporter do the job.
    BeFileNameCR rootFileName = GetRootFileName ();
    if (Converter::IsDwgOrDxfFile(rootFileName))
        {
        ReportError (IssueCategory::Unsupported(), Converter::Issue::DwgFileIgnored(), Utf8String(rootFileName.c_str()).c_str());
        return  DgnV8Api::DGNFILE_ERROR_UnknownFormat;
        }

    SetStepName(ProgressMessage::STEP_LOADING_V8());

    m_rootTrans.InitIdentity();
                
    //  Open the root V8File
    DgnV8Api::DgnFileStatus openStatus;    
    m_rootFile = OpenDgnV8File(openStatus, rootFileName);
    if (!m_rootFile.IsValid())
        return openStatus;

    //  Identify the root model
    auto rootModelId = _GetRootModelId();

    //  Load the root model
    m_rootModelRef = m_rootFile->LoadRootModelById((Bentley::StatusInt*)&openStatus, rootModelId);
    if (NULL == m_rootModelRef)
        return openStatus;

    if (!ShouldConvertToPhysicalModel(*GetRootModelP()))
        {
        auto defaultModel = m_rootFile->LoadRootModelById((Bentley::StatusInt*)&openStatus, m_rootFile->GetDefaultModelId());
        if (ShouldConvertToPhysicalModel(*defaultModel))
            {
            m_rootModelRef = defaultModel;
            rootModelId = m_rootModelRef->GetModelId();
            ReportIssue(Converter::IssueSeverity::Info, Converter::IssueCategory::Compatibility(), Converter::Issue::DefaultedRootModel(), Converter::IssueReporter::FmtModel(*m_rootModelRef->GetDgnModelP()).c_str());
            }
        }

    if (DgnV8Api::DGNFILE_STATUS_Success != (openStatus = _ComputeCoordinateSystemTransform()))
        return openStatus;

    m_isRootModelSpatial = ShouldConvertToPhysicalModel(*GetRootModelP());

    if (!m_isRootModelSpatial)
        ReportIssue(Converter::IssueSeverity::Warning, Converter::IssueCategory::Unsupported(), Converter::Issue::RootModelMustBePhysical(), Converter::IssueReporter::FmtModel(*GetRootModelP()).c_str());

    SetLineStyleConverterRootModel(m_rootModelRef->GetDgnModelP());

    return WasAborted() ? DgnV8Api::DGNFILE_STATUS_UnknownError: DgnV8Api::DGNFILE_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/17
+---------------+---------------+---------------+---------------+---------------+------*/
SpatialConverterBase::ImportJobLoadStatus SpatialConverterBase::FindJob()
    {
    BeAssert(m_rootFile.IsValid() && "Must define root file before loading the job");
    BeAssert((nullptr != m_rootModelRef) && "Must define root model before loading the job");

    m_importJob = FindImportJobForModel(*GetRootModelP());

    if (!m_importJob.IsValid())
        return ImportJobLoadStatus::FailedNotFound;
    
    // *** TRICKY: If this is called by the framework as a check *after* it calls _IntializeJob, then don't change the change detector!
    if (!_HaveChangeDetector() || IsUpdating())
        _SetChangeDetector(true);

    _GetV8FileIntoSyncInfo(*m_rootFile, _GetIdPolicy(*m_rootFile)); // (on update, this just looks up the existing mapping and caches it in memory)

    m_rootModelMapping = GetModelForDgnV8Model(*m_rootModelRef->GetDgnModelP(), m_rootTrans); // (on update, this just looks up the existing mapping)

    GetOrCreateJobPartitions(); // (on update, this just looks up and caches existing modelids)

    auto hsubj = GetJobHierarchySubject(); // There's only one hierarchy subject for a job. Look it up.
    if (!hsubj.IsValid())
        {
        BeAssert(false);
        return ImportJobLoadStatus::FailedNotFound;
        }
    SetSpatialParentSubject(*hsubj);

    CheckModelUnitsUnchanged(*GetRootModelP(), m_rootTrans);
    return ImportJobLoadStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void SpatialConverterBase::ComputeDefaultImportJobName()
    {
    if (nullptr == GetRootModelP())
        {
        BeAssert(false && "Call InitRootModel first");
        return;
        }

    Utf8String jobName(iModelBridge::str_BridgeType_DgnV8());
    jobName.append(":");
    jobName.append(GetFileBaseName(*GetRootV8File()));
    jobName.append(", ");
    jobName.append(Utf8String(GetRootModelP()->GetModelName()));

    DgnCode code = Subject::CreateCode(*GetDgnDb().Elements().GetRootSubject(), jobName.c_str());
    int i=0;
    while (GetDgnDb().Elements().QueryElementIdByCode(code).IsValid())
        {
        Utf8String uniqueJobName(jobName);
        uniqueJobName.append(Utf8PrintfString("%d", ++i).c_str());
        code = Subject::CreateCode(*GetDgnDb().Elements().GetRootSubject(), uniqueJobName.c_str());
        }

    _GetParamsR().SetBridgeJobName(jobName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/17
+---------------+---------------+---------------+---------------+---------------+------*/
SpatialConverterBase::ImportJobCreateStatus SpatialConverterBase::InitializeJob(Utf8CP comments, SyncInfo::ImportJob::Type jtype)
    {
    if (IsUpdating())
        {
        BeAssert(false);
        return ImportJobCreateStatus::FailedExistingRoot;
        }

    Utf8String jobName = _GetParams().GetBridgeJobName();
    if (jobName.empty())
        {
        ComputeDefaultImportJobName();
        jobName = _GetParams().GetBridgeJobName();
        }
    else
        {
        if (!jobName.StartsWithI(iModelBridge::str_BridgeType_DgnV8()))
            {
            jobName = iModelBridge::str_BridgeType_DgnV8();
            jobName.append(":");
            jobName.append(_GetParams().GetBridgeJobName());
            }
        }

    BeAssert(m_rootFile.IsValid() && "Must define root file before creating the job");
    BeAssert((nullptr != m_rootModelRef) && "Must define root model before creating the job");
    BeAssert(!jobName.empty());

    // 0. Make sure that the domains/schemas and other fixed tables that I use are imported
    CreateJobStructure_ImportSchemas();

    if (FindImportJobForModel(*GetRootModelP()).IsValid())
        return ImportJobCreateStatus::FailedExistingRoot;

    SyncInfo::V8ModelMapping mapping;
    if (BSISUCCESS == GetSyncInfo().FindModel(&mapping, *GetRootModelP(), &m_rootTrans, GetCurrentIdPolicy()))
        return ImportJobCreateStatus::FailedExistingNonRootModel;

    // 1. Map in the root file.
    auto fileId = GetV8FileSyncInfoId(*m_rootFile); // NB! file might already be in syncinfo! The logic that tries to detect an existing Job puts it there!
    SyncInfo::FileById syncInfoFiles(GetDgnDb(), fileId);
    SyncInfo::FileIterator::Entry syncInfoFile = syncInfoFiles.begin();
    if (syncInfoFile == syncInfoFiles.end())
        {
        BeAssert(false);
        return ImportJobCreateStatus::FailedExistingNonRootModel;
        }

    _SetChangeDetector(false);

    // 2. Create a subject, as a child of the rootsubject
    SubjectPtr ed = Subject::Create(*GetDgnDb().Elements().GetRootSubject(), jobName);

    Json::Value v8JobProps(Json::objectValue);
    v8JobProps["ConverterType"] = (int)jtype;
    v8JobProps["NamePrefix"] = _GetNamePrefix();
    v8JobProps["V8File"] = syncInfoFile.GetUniqueName();
    v8JobProps["V8RootModel"] = Utf8String(m_rootModelRef->GetDgnModelP()->GetModelName());

    Json::Value jobProps(Json::objectValue);
    if (!Utf8String::IsNullOrEmpty(comments))
        jobProps["Comments"] = comments;
    jobProps[iModelBridge::str_BridgeType_DgnV8()] = v8JobProps;

    ed->SetSubjectJsonProperties(Subject::json_Job(), jobProps);

    SubjectCPtr jobSubject = ed->InsertT<Subject>();
    if (!jobSubject.IsValid())
        return ImportJobCreateStatus::FailedInsertFailure;

    // 3. Set up m_importJob with the subject. That leaves out the syncinfo part, but we don't need that yet. 
    //      We do need m_importJob to be defined and it must have its subject at this point, as GetOrCreateJobPartitions 
    //      and GetModelForDgnV8Model refer to the subject in it.

    m_importJob = ResolvedImportJob(*jobSubject);

    // 4. Create the job-specific stuff in the DgnDb (relative to the job subject).
    GetOrCreateJobPartitions();

    //  ... and create and push the root model's "hierarchy" subject. The root model's physical partition will be a child of that.
    Utf8String mastermodelName = _ComputeModelName(*m_rootModelRef->GetDgnModelP());
    SubjectCPtr hsubj = GetOrCreateModelSubject(GetJobSubject(), mastermodelName, ModelSubjectType::Hierarchy);
    if (!hsubj.IsValid())
        {
        BeAssert(false);
        return ImportJobCreateStatus::FailedInsertFailure;
        }
    SetSpatialParentSubject(*hsubj);

    // 5. Map the root model into the DgnDb. Note that this will generally create a partition, which is relative to the job subject,
    //    So, the job subject and its framework must be created first.
    m_rootModelMapping = GetModelForDgnV8Model(*m_rootModelRef->GetDgnModelP(), m_rootTrans);

    // 6. Now that we have the root model's syncinfo id, we can define the syncinfo part of the importjob.
    SyncInfo::ImportJob importJob;
    importJob.SetV8ModelSyncInfoId(m_rootModelMapping.GetV8ModelSyncInfoId());
    importJob.SetPrefix(_GetNamePrefix());
    importJob.SetType(jtype);
    importJob.SetSubjectId(jobSubject->GetElementId());
    if (BSISUCCESS != GetSyncInfo().InsertImportJob(importJob))
        return ImportJobCreateStatus::FailedExistingRoot;

    m_importJob.GetImportJob() = importJob;     // Update the syncinfo part of the import job

    return ImportJobCreateStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/17
+---------------+---------------+---------------+---------------+---------------+------*/
SubjectCPtr SpatialConverterBase::GetOrCreateModelSubject(SubjectCR parent, Utf8StringCR modelName, ModelSubjectType stype)
    {
    Json::Value modelProps(Json::nullValue);
    modelProps["Type"] = (ModelSubjectType::Hierarchy==stype)? "Hierarchy": "References";

    for (auto childid : parent.QueryChildren())
        {
        auto subj = GetDgnDb().Elements().Get<Subject>(childid);
        if (subj.IsValid() && modelName.Equals(subj->GetCode().GetValueUtf8CP()) && (modelProps == subj->GetSubjectJsonProperties().GetMember(Subject::json_Model())))
            return subj;
        }

    BeAssert((!IsUpdating() || (ModelSubjectType::Hierarchy != stype)) && "You create a hierarchy subject once when you create the job");

    SubjectPtr ed = Subject::Create(parent, modelName.c_str());

    ed->SetSubjectJsonProperties(Subject::json_Model(), modelProps);

    return ed->InsertT<Subject>();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void RootModelConverter::_ConvertModels()
    {
    SetStepName(IsUpdating() ? Converter::ProgressMessage::STEP_UPDATING() : 
                               Converter::ProgressMessage::STEP_CREATING(), Utf8String(GetDgnDb().GetFileName()).c_str());

    if (nullptr != m_rootModelRef && m_isRootModelSpatial)
        {
        bool haveFoundSpatialRoot = false;
        ImportSpatialModels(haveFoundSpatialRoot, *m_rootModelRef, m_rootTrans);
        if (WasAborted())
            return;
        }

    _ImportDrawingAndSheetModels(m_rootModelMapping); // we also need to convert all drawing models now, so that we can analyze them for EC content.

    m_syncInfo.SetValid(true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool RootModelConverter::IsFileAssignedToBridge(DgnV8FileCR v8File) const 
    {
    BeFileName fn(v8File.GetFileName().c_str());
    return m_params.IsFileAssignedToBridge(fn);
    }

/*---------------------------------------------------------------------------------**//**
* The purpose of this function is to *discover* spatial models, not to convert their contents.
* The outcome of this function is to a) create an (empty) BIM spatial model for each discovered V8 spatial model,
* and b) to discover and record the spatial transform that should be applied when (later) converting
* the elements in each model.
* This function also has the side effect of creating or updating the job subject hierarchy.
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void RootModelConverter::ImportSpatialModels(bool& haveFoundSpatialRoot, DgnV8ModelRefR thisModelRef, BentleyApi::TransformCR trans)
    {
    // NB: Don't filter out files or models here. We need to know about the existence of all of them 
    // (e.g., so that we can infer deleted models). We will do any applicable filtering in ConvertElementsInModel.

    DgnV8ModelR thisV8Model = *thisModelRef.GetDgnModelP();
    DgnV8FileR  thisV8File  = *thisV8Model.GetDgnFileP();

    bool isThisMyFile = IsFileAssignedToBridge(thisV8File);
    if (isThisMyFile)
        haveFoundSpatialRoot = true;

    // look at the models in this file. If there are 2d models with DgnModelType::Normal, decide whether they should be considered to be spatial models or drawing models.
    ClassifyNormal2dModels (thisV8File);

    SyncInfo::V8FileSyncInfoId v8FileId = GetV8FileSyncInfoId(thisV8File);

    // NB: We must not try to skip entire files if we need to follow reference attachments. Instead, we can skip 
    //      the elements in a model if the model (i.e., the file) is unchanged.

    // Map this v8 model into the project
    ResolvedModelMapping v8mm = GetModelForDgnV8Model(thisModelRef, trans);

    if (true)
        {
        auto fillMsg = ProgressMessage::GetString(ProgressMessage::TASK_FILLING_V8_MODELS());
        DgnV8Api::IDgnProgressMeter::TaskMark loading(Bentley::WString(fillMsg.c_str(),true).c_str());
        DgnV8Api::DgnAttachmentLoadOptions loadOptions;
        loadOptions.SetTopLevelModel(!thisModelRef.IsDgnAttachment() || &thisModelRef == GetRootModelRefP());
        loadOptions.SetSectionsToFill(DgnV8Api::DgnModelSections::Model);
        // If we know that we won't be processing graphics in this model but only attachments
        // then we can save a lot of time by filling only the control section. We do that in two
        // cases: 1) this bridge should not convert this file (but may have to convert references attached to it),
        // or 2) this is an update and this file has not changed and so no elements in it will be converted.
        if (!isThisMyFile || GetChangeDetector()._AreContentsOfModelUnChanged(*this, v8mm)) 
            loadOptions.SetSectionsToFill(DgnV8Api::DgnModelSections::ControlElements);

        if (!m_config.GetXPathBool("/ImportConfig/Raster/@importAttachments", false))
            loadOptions.m_loadRasterRefs = true;

        thisModelRef.ReadAndLoadDgnAttachments(loadOptions);
        }

    if (nullptr == thisModelRef.GetDgnAttachmentsP())
        return; 

    // All attachments to a spatial model are spatial, unless they are specifically DgnModelType::Drawing or DgnModelType::Sheet, which BIM doesn't support.
    ForceAttachmentsToSpatial (*thisModelRef.GetDgnAttachmentsP());

    SubjectCR parentRefsSubject = _GetSpatialParentSubject();

    bool hasPushedReferencesSubject = false;
    for (DgnV8Api::DgnAttachment* attachment : *thisModelRef.GetDgnAttachmentsP())
        {                  
        if (nullptr == attachment->GetDgnModelP() || !_WantAttachment(*attachment))
            continue; // missing reference 

        if (haveFoundSpatialRoot && !IsFileAssignedToBridge(*attachment->GetDgnModelP()->GetDgnFileP()))
            continue; // this leads to models in another bridge's territory. Stay out.

        // Keep the mapping between models and the attachment that references the model
        DgnV8Api::Fd_opts fdOpts = attachment->GetFDOptsCR();
        Bentley::DgnModelP dgnModelP = attachment->GetDgnModelP();
        m_modelAttachmentMapping[dgnModelP] = attachment;

        if (!hasPushedReferencesSubject)
            {
            SubjectCPtr myRefsSubject = GetOrCreateModelSubject(parentRefsSubject, v8mm.GetDgnModel().GetName(), ModelSubjectType::References);
            if (!myRefsSubject.IsValid())
                {
                BeAssert(false);
                ReportError(IssueCategory::Unsupported(), Issue::Error(), Utf8PrintfString("Failed to create references subject. Parent[%s]. Attachment[%s].",
                                                                                           IssueReporter::FmtModel(thisV8Model).c_str(),
                                                                                           IssueReporter::FmtAttachment(*attachment).c_str()).c_str());
                continue;
                }
            SetSpatialParentSubject(*myRefsSubject); // >>>>>>>>>> Push spatial parent subject
            hasPushedReferencesSubject = true;
            }

        if (!ShouldConvertToPhysicalModel(*attachment->GetDgnModelP()))
            {
            // 3D model referencing a 2D model
            // *** WIP_TEMPLATES convert the 2D model to a template and insert an instance of it at the ref attachment point
            ReportError(IssueCategory::Unsupported(), Issue::Error(), Utf8PrintfString("2D attachments to 3D are not supported. 3D:[%s]. 2D:[%s]",
                                                                                       IssueReporter::FmtModel(thisV8Model).c_str(),
                                                                                       IssueReporter::FmtAttachment(*attachment).c_str()).c_str());
            continue;
            }

        Transform refTrans = ComputeAttachmentTransform(trans, *attachment);
        ImportSpatialModels(haveFoundSpatialRoot, *attachment, refTrans);
        }

    if (hasPushedReferencesSubject)
        SetSpatialParentSubject(parentRefsSubject); // <<<<<<<<<<<< Pop spatial parent subject
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/15
+---------------+---------------+---------------+---------------+---------------+------*/
void RootModelConverter::_ConvertSpatialLevels()
    {
    for (DgnV8FileP v8File : m_v8Files)
        _ConvertSpatialLevelTable(*v8File);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    06/2015
//---------------------------------------------------------------------------------------
void RootModelConverter::_ConvertLineStyles()
    {
    BeAssert(GetRootModelP() != nullptr);
    if (GetRootModelP() == nullptr)
        return;     //  the line style converter uses m_rootModel

    for (DgnV8FileP v8File : m_v8Files)
        ConvertAllLineStyles(*v8File);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/14
+---------------+---------------+---------------+---------------+---------------+------*/
DgnV8Api::ModelId RootModelConverter::_GetRootModelId()
    {
    if (IsUpdating() && !m_params.GetRootModelChoice().IsSet())
        {
        auto importJob = FindSoleImportJobForFile(*m_rootFile);
        if (importJob.IsValid())
            return GetSyncInfo().GetV8ModelIdFromV8ModelSyncInfoId(importJob.GetV8ModelSyncInfoId());
        }

    if (!IsV8Format(*m_rootFile))
        return DgnV8Api::DEFAULTMODEL;

    if (RootModelChoice::Method::ById == m_params.GetRootModelChoice().m_method)
        return m_params.GetRootModelChoice().m_id;

    if (RootModelChoice::Method::UseDefaultModel == m_params.GetRootModelChoice().m_method)
        return m_rootFile->GetDefaultModelId();

    if (RootModelChoice::Method::ByName == m_params.GetRootModelChoice().m_method)
        return m_rootFile->FindModelIdByName(WString(m_params.GetRootModelChoice().m_name.c_str(),BentleyCharEncoding::Utf8).c_str());

    BeAssert(RootModelChoice::Method::FromActiveViewGroup == m_params.GetRootModelChoice().m_method);

    ViewGroupCollectionR viewGroups = m_rootFile->GetViewGroupsR();
    m_viewGroup = viewGroups.FindByElementId(m_rootFile->GetActiveViewGroupId());

    // the viewgroup stored in the tcb no longer exists. Use the most recently modified one instead.
    if (!m_viewGroup.IsValid())
        m_viewGroup = viewGroups.FindLastModifiedMatchingModel(INVALID_ELEMENTID, INVALID_MODELID, false, -1);

    DgnV8Api::ModelId defaultModelId = m_rootFile->GetDefaultModelId();

    if (!m_viewGroup.IsValid()) // there must not be any viewgroup in the file. 
        DgnV8Api::ViewGroup::Create(m_viewGroup, defaultModelId, *m_rootFile, false, nullptr, true);

    if (!m_viewGroup.IsValid())
        {
        BeAssert(false);
        return defaultModelId;
        }

    // try to find a spatial view in the viewgroup
    for (uint32_t iView=0; iView < DgnV8Api::MAX_VIEWS; ++iView)
        {
        ViewInfoR viewInfo = m_viewGroup->GetViewInfoR(iView);
        Bentley::ViewPortInfoCR viewPortInfo = m_viewGroup->GetViewPortInfo(iView);

        if (!viewInfo.GetViewFlags().on_off || !viewPortInfo.m_wasDefined)
            continue;

        auto modelId = viewInfo.GetRootModelId();
        Bentley::StatusInt openStatus;    
        auto model = m_rootFile->LoadRootModelById(&openStatus, modelId);
        if (nullptr == model)
            continue;

        if (!ShouldConvertToPhysicalModel(*model))
            continue;

        return modelId;
        }

    return defaultModelId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/15
+---------------+---------------+---------------+---------------+---------------+------*/
RootModelConverter::RootModelConverter(RootModelSpatialParams& params) 
    : T_Super(params), m_params(params)
    {
    // We do the Config map lookup here and save the result to this variable.
    m_considerNormal2dModelsSpatial = (GetConfig().GetOptionValueBool("Consider2dModelsSpatial", false) || m_params.GetConsiderNormal2dModelsSpatial());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/15
+---------------+---------------+---------------+---------------+---------------+------*/
void RootModelConverter::_ConvertSchemas()
    {
    if (IsUpdating())
        {
        CheckNoECSchemaChanges();
        return;
        }

    SetStepName(Converter::ProgressMessage::STEP_DISCOVER_ECSCHEMAS());

    bset<DgnV8ModelP> uniqueModels;
    SchemaConversionScope scope(*this);

    StopWatch timer(true);
    
    for (auto& modelMapping : m_v8ModelMappings)
        uniqueModels.insert(&modelMapping.GetV8Model());

    AddTasks((uint32_t)uniqueModels.size());

    for (DgnV8ModelP v8Model : uniqueModels)
        RetrieveV8ECSchemas(*v8Model);

    if (m_skipECContent)
        {
        scope.SetSucceeded();
        return;
        }

    if (SUCCESS != ConsolidateV8ECSchemas() || WasAborted())
        return;

    ConverterLogging::LogPerformance(timer, "Convert Schemas> Total Retrieve and consolidate V8 ECSchemas");

    AddTasks((int32_t)(m_v8ModelMappings.size() + m_v8Files.size()));

    timer.Start();
    for (auto& modelMapping : m_v8ModelMappings)
        {
        SetTaskName(ProgressMessage::TASK_ANALYZE_EC_CONTENT(), Converter::IssueReporter::FmtModel(modelMapping.GetV8Model()).c_str());

        AnalyzeECContent(modelMapping.GetV8Model(), &modelMapping.GetDgnModel());
        if (WasAborted())
            return;
        }

    //analyze named groups in dictionary models
    for (DgnV8FileP v8File : m_v8Files)
        {
        SetTaskName(ProgressMessage::TASK_ANALYZE_EC_CONTENT(), Converter::IssueReporter::FmtModel(v8File->GetDictionaryModel()).c_str());

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
    //The schema import is wrapped into transaction management so that in case of a failed
    //schema import the DB can be set into a clean state again. This also means
    //* any previous changes to the file are committed
    //* in case of a successful schema import, changes are committed again
    if (BentleyApi::SUCCESS != ImportTargetECSchemas())
        return;

    ReportProgress();
    ConverterLogging::LogPerformance(timer, "Convert Schemas> Import ECSchemas");

    if (WasAborted())
        return;
    scope.SetSucceeded();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/15
+---------------+---------------+---------------+---------------+---------------+------*/
void RootModelConverter::_ConvertSpatialElements()
    {
    SetStepName(Converter::ProgressMessage::STEP_CONVERTING_ELEMENTS());
    DoConvertSpatialElements();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   10/2014
//---------------------------------------------------------------------------------------
BentleyApi::BentleyStatus Converter::RetrieveV8ECSchemas(DgnV8ModelR v8Model)
    {
    SetTaskName(ProgressMessage::TASK_READING_V8_ECSCHEMA(), Utf8String(v8Model.GetDgnFileP()->GetFileName().c_str()).c_str());
    if (BentleyApi::SUCCESS != RetrieveV8ECSchemas(v8Model, DgnV8Api::ECSCHEMAPERSISTENCE_Stored))
        return BentleyApi::ERROR;
    return RetrieveV8ECSchemas(v8Model, DgnV8Api::ECSCHEMAPERSISTENCE_External);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   10/2014
//---------------------------------------------------------------------------------------
BentleyApi::BentleyStatus Converter::RetrieveV8ECSchemas(DgnV8ModelR v8Model, DgnV8Api::ECSchemaPersistence persistence)
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
                ReportError(IssueCategory::Unknown(), Issue::ConvertFailure(), "Could not read v8 ECSchema XML.");
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
                ReportIssueV(IssueSeverity::Warning, IssueCategory::MissingData(), Issue::Message(), nullptr, error.c_str());
                continue;
                }

            isDynamicSchema = IsDynamicSchema(*externalSchema);

            if (ECObjectsV8::SCHEMA_WRITE_STATUS_Success != externalSchema->WriteToXmlString(schemaXml))
                {
                Utf8PrintfString error("Could not serialize external v8 ECSchema %s.  Instances of classes from this schema will not be converted.", schemaName.c_str());
                ReportIssueV(IssueSeverity::Warning, IssueCategory::MissingData(), Issue::Message(), nullptr, error.c_str());
                continue;
                }
            }

        BECN::SchemaKey existingSchemaKey;
        SyncInfo::ECSchemaMappingType existingMappingType = SyncInfo::ECSchemaMappingType::Identity;
        if (m_syncInfo.TryGetECSchema(existingSchemaKey, existingMappingType, schemaName.c_str()))
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
                    ReportIssue(IssueSeverity::Warning, Converter::IssueCategory::Sync(), Converter::Issue::Message(), error.c_str());
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
                    ReportIssue(IssueSeverity::Warning, Converter::IssueCategory::Sync(), Converter::Issue::Message(), error.c_str());
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
                        ReportIssue(IssueSeverity::Warning, IssueCategory::Sync(), Issue::Message(), error.c_str());
                        }
                    else
                        continue;
                    }
                }
            }

        ECN::ECSchemaId schemaId;
        if (BE_SQLITE_OK != m_syncInfo.InsertECSchema(schemaId, *v8Model.GetDgnFileP(),
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

        if (BE_SQLITE_OK != V8ECSchemaXmlInfo::Insert(*m_dgndb, schemaId, schemaXml.c_str()))
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
bool Converter::IsDynamicSchema(Bentley::Utf8StringCR schemaName, Bentley::Utf8StringCR schemaXml)
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
bool Converter::IsDynamicSchema(ECObjectsV8::ECSchemaCR schema)
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
bool Converter::IsWellKnownDynamicSchema(Bentley::Utf8StringCR schemaName)
    {
    return BeStringUtilities::Strnicmp(schemaName.c_str(),"PFLModule", 9) == 0 ||
        schemaName.EqualsI("CivilSchema_iModel") ||
        schemaName.EqualsI("BuildingDataGroup");
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Keith.Bentley                   02 / 15
//---------------------------------------------------------------------------------------
void RootModelConverter::ConvertElementsInModel(ResolvedModelMapping const& v8mm)
    {
    DgnModelR targetModel = v8mm.GetDgnModel();

    if (!targetModel.Is3d())
        {
        BeAssert(false);
        return;
        }

    if (GetChangeDetector()._AreContentsOfModelUnChanged(*this, v8mm))
        return;

    DgnV8Api::DgnModel& v8Model = v8mm.GetV8Model();

    if (!v8Model.Is3D() && GetRootModelP() != &v8Model) // if this is a drawing, then it won't already be filled.
        v8Model.FillCache();

    ReportProgress();

    m_currIdPolicy = GetIdPolicyFromAppData(*v8Model.GetDgnFileP());

    ConvertElementList(v8Model.GetControlElementsP(), v8mm);
    ConvertElementList(v8Model.GetGraphicElementsP(), v8mm);

    GetDgnDb().Memory().PurgeUntil(1024*1024);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/15
+---------------+---------------+---------------+---------------+---------------+------*/
void RootModelConverter::DoConvertSpatialElements()
    {
    bmultiset<ResolvedModelMapping> spatialModels;
    for (auto v8mm : m_v8ModelMappings)
        {
        if (v8mm.GetDgnModel().Is3dModel())
            spatialModels.insert(v8mm);
        }

    if (spatialModels.empty())
        return;

    AddTasks((int32_t)(spatialModels.size()));
    for (auto& modelMapping : spatialModels)
        {
        if (WasAborted())
            return;

        SetTaskName(Converter::ProgressMessage::TASK_CONVERTING_MODEL(), modelMapping.GetDgnModel().GetName().c_str());

        StopWatch timer(true);
        uint32_t start = GetElementsConverted();

        ConvertElementsInModel(modelMapping);

        uint32_t convertedElementCount = (uint32_t) GetElementsConverted() - start;
        ConverterLogging::LogPerformance(timer, "Convert Spatial Elements> Model '%s' (%" PRIu32 " element(s))", 
                                         modelMapping.GetDgnModel().GetName().c_str(),
                                         convertedElementCount);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/15
+---------------+---------------+---------------+---------------+---------------+------*/
void RootModelConverter::ConvertNamedGroupsAndECRelationships()
    {
    StopWatch timer(true);
    uint32_t start = GetElementsConverted();

    AddTasks((int32_t)(m_v8Files.size()));
    //convert dictionary model named groups
    for (DgnV8FileP v8File : m_v8Files)
        {
        SetTaskName(Converter::ProgressMessage::TASK_CONVERTING_MODEL(), Converter::IssueReporter::FmtFileBaseName(*v8File).c_str());

        if (WasAborted())
            return;

        ResolvedModelMapping v8DictionaryModelMapping;

        for (DgnV8Api::PersistentElementRef* controlElem : *v8File->GetDictionaryModel().GetControlElementsP())
            {
            if (V8ElementType::NamedGroup == V8ElementTypeHelper::GetType(*controlElem))
                {
                if (!v8DictionaryModelMapping.IsValid())
                    v8DictionaryModelMapping = MapDgnV8ModelToDgnDbModel(v8File->GetDictionaryModel(), Transform::FromIdentity(), GetDgnDb().GetDictionaryModel().GetModelId());
                DgnV8Api::EditElementHandle v8eeh(controlElem);
                ElementConversionResults res;
                _ConvertControlElement(res, v8eeh, v8DictionaryModelMapping);
                }
            }
        }

    uint32_t convertedElementCount = (uint32_t) GetElementsConverted() - start;
    ConverterLogging::LogPerformance(timer, "Convert NamedGroups in dictionary (%" PRIu32 " element(s))", convertedElementCount);

    AddTasks(1);
    SetTaskName(Converter::ProgressMessage::TASK_CONVERTING_RELATIONSHIPS());
    ConvertNamedGroupsRelationships();
    ConverterLogging::LogPerformance(timer, "Convert Elements> NamedGroups");

    timer.Start();
    ConvertECRelationships();
    ConverterLogging::LogPerformance(timer, "Convert Elements> ECRelationships (total)");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            03/2017
//---------------+---------------+---------------+---------------+---------------+-------
void RootModelConverter::UpdateCalculatedProperties()
    {

    CachedStatementPtr stmt = nullptr;
    auto stat = m_dgndb->GetCachedStatement(stmt, "SELECT p.ClassId, p.Name FROM ec_Property p, ec_CustomAttribute ca, ec_Class c WHERE ca.ClassId = c.Id AND c.Name='CalculatedECPropertySpecification' AND ca.ContainerId=p.Id AND ca.Instance LIKE '%GetRelatedInstance%'");
    //auto stat = m_dgndb->GetCachedStatement(stmt, "SELECT p.ClassId, p.Name FROM ec_Property p, ec_CustomAttribute ca, ec_Class c WHERE ca.ClassId = c.Id AND c.Name='CalculatedECPropertySpecification' AND ca.ContainerId=p.Id");
    if (stat != BE_SQLITE_OK)
        {
        BeAssert(false && "Could not retrieve cached statement.");
        return;
        }

    bmap<BECN::ECClassId, bvector<Utf8String>> classMap;
    while (BE_SQLITE_ROW == stmt->Step())
        {
        BECN::ECClassId classId = stmt->GetValueId<BECN::ECClassId>(0);
        bmap<BECN::ECClassId, bvector<Utf8String>>::iterator iter = classMap.find(classId);
        classMap[classId].push_back(stmt->GetValueText(1));
        }

    for (bmap<BECN::ECClassId, bvector<Utf8String>>::iterator iter = classMap.begin(); iter != classMap.end(); iter++)
        {
        BECN::ECClassCP ecClass = m_dgndb->Schemas().GetClass(iter->first);

        Utf8String propertyNames;
        bvector<Utf8String> properties = iter->second;
        for (int i = 0; i < properties.size(); i++)
            {
            if (i != 0)
                propertyNames.append(", ");
            propertyNames.append(properties[i]);
            }
        Utf8PrintfString sql("SELECT ECClassId, ECInstanceId, * FROM %s"/*, propertyNames.c_str()*/, ecClass->GetECSqlName().c_str());

        BeSQLite::EC::CachedECSqlStatementPtr instanceStmt = m_dgndb->GetPreparedECSqlStatement(sql.c_str());
        BeSQLite::EC::ECInstanceECSqlSelectAdapter dataAdapter(*instanceStmt);
        while (BE_SQLITE_ROW == instanceStmt->Step())
            {
            BECN::IECInstancePtr selectedInstance = dataAdapter.GetInstance();
            uint64_t id;
            BeStringUtilities::ParseUInt64(id, selectedInstance->GetInstanceId().c_str());
            DgnElementPtr element = m_dgndb->Elements().GetForEdit<DgnElement>(DgnElementId(id));
            BECN::ECValue nullValue;
            nullValue.SetToNull();
            for (int i = 0; i < properties.size(); i++)
                {
                BECN::ECValue v;
                selectedInstance->SetValue(properties[i].c_str(), nullValue);
                selectedInstance->GetValue(v, properties[i].c_str());
                element->SetPropertyValue(properties[i].c_str(), v);
                }
            element->Update();
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   03/2015
//---------------------------------------------------------------------------------------
BentleyApi::BentleyStatus RootModelConverter::ConvertNamedGroupsRelationships()
    {
    for (auto const& modelMapping : m_v8ModelMappings)
        {
        if (BentleyApi::SUCCESS != ConvertNamedGroupsRelationshipsInModel(modelMapping.GetV8Model()))
            return BSIERROR;
        }

    return BentleyApi::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   04/2015
//---------------------------------------------------------------------------------------
BentleyApi::BentleyStatus Converter::ConvertNamedGroupsRelationshipsInModel(DgnV8ModelR v8Model)
    {
    struct NamedGroupConverter : DgnV8Api::INamedGroupMemberVisitor
        {
        private:
            Converter& m_converter;
            DgnElementId const& m_parentId;
            bool m_namedGroupOwnsMembers;
            bset<DgnElementId> m_visitedMembers;

            virtual DgnV8Api::MemberTraverseStatus VisitMember(DgnV8Api::NamedGroupMember const* member, DgnV8Api::NamedGroup const* ng, UInt32 index) override
                {
                DgnV8Api::ElementHandle memberEh(member->GetElementRef());
                DgnElementId childElementId;
                if (!m_converter.GetSyncInfo().TryFindElement(childElementId, memberEh))
                    {
                    Utf8String error;
                    error.Sprintf("No BIS grouping relationship created for v8 NamedGroup Member element (%s) because the member element was not converted.",
                                  Converter::IssueReporter::FmtElement(memberEh).c_str());
                    m_converter.ReportIssue(IssueSeverity::Warning, Converter::IssueCategory::Sync(), Converter::Issue::Message(), error.c_str());
                    return DgnV8Api::MemberTraverseStatus::Continue;
                    }
                m_visitedMembers.insert(childElementId);

                DgnElements& elementTable = m_converter.GetDgnDb().Elements();

                DgnElementCPtr child = elementTable.GetElement(childElementId);
                if (!child.IsValid())
                    return DgnV8Api::MemberTraverseStatus::Continue;

                if (m_namedGroupOwnsMembers)
                    {
                    DgnElementPtr childEdit = child->CopyForEdit();
                    DgnClassId parentRelClassId = m_converter.GetDgnDb().Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_REL_ElementOwnsChildElements);
                    childEdit->SetParentId(m_parentId, parentRelClassId);
                    elementTable.Update(*childEdit);
                    return DgnV8Api::MemberTraverseStatus::Continue;
                    }

                GroupInformationElementCPtr group = elementTable.Get<GroupInformationElement>(m_parentId);
                if (group.IsValid())
                    ElementGroupsMembers::Insert(*group, *child, 0);

                return DgnV8Api::MemberTraverseStatus::Continue;
                }

        public:
            NamedGroupConverter(Converter& converter, DgnElementId const& parentId, bool namedGroupOwnsMembers)
                : m_converter(converter), m_parentId(parentId), m_namedGroupOwnsMembers(namedGroupOwnsMembers)
                {}

            bool WasVisited(DgnElementId member)
                {
                return m_visitedMembers.find(member) != m_visitedMembers.end();
                }
        };

    if (WasAborted())
        return BentleyApi::SUCCESS;

    //TODO: do we need to load the model and its attachments or can we consider them loaded already?

    SyncInfo::V8FileSyncInfoId v8FileId = GetV8FileSyncInfoIdFromAppData(*v8Model.GetDgnFileP());
    bset<DgnV8Api::ElementId> const* namedGroupsWithOwnershipHintPerFile = nullptr;
    V8NamedGroupInfo::TryGetNamedGroupsWithOwnershipHint(namedGroupsWithOwnershipHintPerFile, v8FileId);

    for (auto v8El : *v8Model.GetControlElementsP())
        {
        BeAssert(v8El != nullptr);
        DgnV8Api::ElementHandle v8eh(v8El);
        if (V8ElementTypeHelper::GetType(v8eh) != V8ElementType::NamedGroup)
            continue;

        DgnV8Api::DgnModel* ngRootModel = &v8Model;
        if (ngRootModel->IsDictionaryModel()) // *** TBD: Check that file was orginally DWG
            {
            DgnV8Api::DgnModel* defaultModel = v8Model.GetDgnFileP()->FindLoadedModelById (v8Model.GetDgnFileP()->GetDefaultModelId());
            if (nullptr != defaultModel)
                ngRootModel = defaultModel;
            }

        DgnV8Api::NamedGroupPtr ng = nullptr;
        if (DgnV8Api::NamedGroupStatus::NG_Success != DgnV8Api::NamedGroup::Create(ng, v8eh, ngRootModel))
            {
            BeAssert(false && "Could not instantiate NamedGroup object");
            return BSIERROR;
            }

        DgnElementId ngElementId;
        if (!GetSyncInfo().TryFindElement(ngElementId, v8eh))
            {
            Utf8String error;
            error.Sprintf("No BIS grouping created for v8 NamedGroup element (%s) because the NamedGroup was not converted.",
                          Converter::IssueReporter::FmtElement(v8eh).c_str());
            ReportIssue(IssueSeverity::Warning, Converter::IssueCategory::Sync(), Converter::Issue::Message(), error.c_str());
            continue;
            }

        const bool namedGroupOwnsMembersFlag = namedGroupsWithOwnershipHintPerFile != nullptr && namedGroupsWithOwnershipHintPerFile->find(v8eh.GetElementId()) != namedGroupsWithOwnershipHintPerFile->end();
        NamedGroupConverter ngConverter(*this, ngElementId, namedGroupOwnsMembersFlag);
        //if traversal fails, still continue with next element, so return value doesn't matter here.
        ng->TraverseMembers(&ngConverter, DgnV8Api::MemberTraverseType::Enumerate, false, false);

        if (!IsUpdating())
            continue;

        DgnElements& elementTable = GetDgnDb().Elements();
        if (namedGroupOwnsMembersFlag)
            {
            DgnElementCPtr header = elementTable.GetElement(ngElementId);
            ;
            for (DgnElementId childId : header->QueryChildren())
                {
                if (ngConverter.WasVisited(childId))
                    continue;
                auto child = elementTable.GetElement(childId);
                if (child.IsValid())
                    {
                    auto editChild = child->CopyForEdit();
                    DgnClassId parentRelClassId = GetDgnDb().Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_REL_ElementOwnsChildElements);
                    editChild->SetParentId(DgnElementId(), parentRelClassId);
                    editChild->Update();
                    }
                }
            }
        else
            {
            GenericGroupCPtr group = elementTable.Get<GenericGroup>(ngElementId);
            if (group.IsValid())
                {
                for (DgnElementId memberId : ElementGroupsMembers::QueryMembers(*group))
                    {
                    if (ngConverter.WasVisited(memberId))
                        continue;
                    DgnElementCPtr child = elementTable.GetElement(memberId);
                    if (child.IsValid())
                        group->RemoveMember(*child);
                    }
                }
            }
        }

    GetDgnDb().Memory().PurgeUntil(1024*1024);
    return BentleyApi::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle   10/2014
//---------------------------------------------------------------------------------------
BentleyApi::BentleyStatus RootModelConverter::ConvertECRelationships()
    {
    if (m_skipECContent)
        return BentleyApi::SUCCESS;

    //As all v8 relationships end up in the same table (bis_ElementRefersToElements)
    //it gets a lot of indexes. These hurt performance a lot, so we drop the indexes before the bulk insert
    //and re-add them later.
    bmap<Utf8String, Utf8String> indexDdlList;
    if (!IsUpdating())
        {
        StopWatch timer(true);
        Statement stmt;
        if (BE_SQLITE_OK != stmt.Prepare(*m_dgndb, "SELECT name, sql FROM sqlite_master WHERE tbl_name='bis_ElementRefersToElements' AND type='index'"))
            return BentleyApi::ERROR;

        while (stmt.Step() == BE_SQLITE_ROW)
            {
            indexDdlList[Utf8String(stmt.GetValueText(0))] = Utf8String(stmt.GetValueText(1));
            }

        stmt.Finalize();

        for (auto const& index : indexDdlList)
            {
            Utf8String sql("DROP INDEX ");
            sql.append(index.first);
            if (BE_SQLITE_OK != GetDgnDb().ExecuteSql(sql.c_str()))
                return BentleyApi::ERROR;
            }

        ConverterLogging::LogPerformance(timer, "Convert Elements> ECRelationships: Dropped indices for bulk insertion into BisCore:ElementRefersToElements class hierarchy.");
        }

    for (auto& modelMapping : m_v8ModelMappings)
        {
        ConvertECRelationshipsInModel(modelMapping.GetV8Model());
        if (WasAborted())
            return BentleyApi::ERROR;
        }

    //analyze named groups in dictionary models
    for (DgnV8FileP v8File : m_v8Files)
        {
        DgnV8ModelR dictionaryModel = v8File->GetDictionaryModel();
        ConvertECRelationshipsInModel(dictionaryModel);
        if (WasAborted())
            return BentleyApi::ERROR;
        }

    //recreate indexes that were previously dropped
    if (!IsUpdating())
        {
        StopWatch timer(true);
        for (auto const& index : indexDdlList)
            {
            if (BE_SQLITE_OK != GetDgnDb().TryExecuteSql(index.second.c_str()))
                {
                Utf8String error;
                error.Sprintf("Failed to recreate index '%s' for BisCore:ElementRefersToElements class hierarchy: %s", index.second.c_str(), GetDgnDb().GetLastError().c_str());
                ReportIssue(IssueSeverity::Info, IssueCategory::Sync(), Issue::Message(), error.c_str());
                CachedStatementPtr stmt = GetDgnDb().GetCachedStatement("DELETE FROM ec_Index WHERE Name=?");
                if (stmt == nullptr)
                    {
                    BeAssert(false && "Could not retrieve cached statement.");
                    return BentleyApi::ERROR;
                    }
                if (BE_SQLITE_OK != stmt->BindText(1, index.first, Statement::MakeCopy::No))
                    {
                    BeAssert(false && "Could not bind to statement.");
                    return BentleyApi::ERROR;
                    }

                if (BE_SQLITE_DONE != stmt->Step())
                    {
                    error.Sprintf("Failed to delete index '%s' from table ec_Index: %s", index.first.c_str(), GetDgnDb().GetLastError().c_str());
                    ReportError(IssueCategory::Sync(), Issue::Message(), error.c_str());
                    return BentleyApi::ERROR;
                    }
                }
            }

        ConverterLogging::LogPerformance(timer, "Convert Elements> ECRelationships: Recreated indices for BisCore:ElementRefersToElements class hierarchy.");
        }

    return BentleyApi::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            02/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyApi::BentleyStatus Converter::ConvertECRelationshipsInModel(DgnV8ModelR v8Model)
    {
    if (m_skipECContent)
        return BentleyApi::SUCCESS;

    DgnV8Api::PersistentElementRefList* graphicElements = v8Model.GetGraphicElementsP();
    if (nullptr != graphicElements)
        {
        for (DgnV8Api::PersistentElementRef* v8Element : *graphicElements)
            {
            DgnV8Api::ElementHandle v8eh(v8Element);
            //TODO if (_FilterElement(v8eh))
            //TODO     continue;

            ConvertECRelationships(v8eh);
            }
        }

    DgnV8Api::PersistentElementRefList* controlElems = v8Model.GetControlElementsP();
    if (nullptr != controlElems)
        {
        for (DgnV8Api::PersistentElementRef* v8Element : *controlElems)
            {
            DgnV8Api::ElementHandle v8eh(v8Element);
            //TODO if (_FilterElement(v8eh))
            //TODO     continue;

            ConvertECRelationships(v8eh);
            }
        }
    return BentleyApi::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RootModelConverter::_BeginConversion()
    {
    if (!GetImportJob().IsValid() || (GetImportJob().GetConverterType() != SyncInfo::ImportJob::Type::RootModels))
        {
        OnFatalError();
        return;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RootModelConverter::_FinishConversion()
    {
    ConvertNamedGroupsAndECRelationships();   // Now that we know all elements, work on the relationships between elements.
    if (IsUpdating())
        UpdateCalculatedProperties();
    _RemoveUnusedMaterials();

    m_linkConverter->PurgeOrphanedLinks();
    
    EmbedSpecifiedFiles();

    for (auto f: m_finishers)
        {
        f->_OnFinishConversion(*this);
        }

    if (!IsUpdating())
        {
        EmbedFilesInSource(GetRootFileName());
        }
    else
        {
        // Detect deletions in the V8 files that we processed. (Don't assume we saw all V8 files.)
        for (DgnV8FileP v8File : m_v8Files)
            {
            GetChangeDetector()._DetectDeletedElementsInFile(*this, *v8File);
            GetChangeDetector()._DetectDeletedModelsInFile(*this, *v8File);    // NB: call this *after* DetectDeletedElements!
            }
        GetChangeDetector()._DetectDeletedElementsEnd(*this);
        GetChangeDetector()._DetectDeletedModelsEnd(*this);

        // Update syncinfo for all V8 files *** WIP_UPDATER - really only need to update syncinfo for changed files, but we don't keep track of that.
        for (DgnFileP v8File : m_v8Files)
            {
            SyncInfo::V8FileProvenance prov(*v8File, GetSyncInfo(), GetCurrentIdPolicy());
            prov.Update();
            }
        }

    CopyExpirationDate(*m_rootFile);
    }

/*=================================================================================**//**
* @bsiclass                                     Brien.Bastings                  03/17
+===============+===============+===============+===============+===============+======*/
struct ConvertModelACSTraverser : DgnV8Api::IACSTraversalHandler
{
ResolvedModelMapping    m_v8mm;
double                  m_toMeters;

ConvertModelACSTraverser(ResolvedModelMapping const& v8mm, double toMeters) : m_v8mm(v8mm), m_toMeters(toMeters) {}
UInt32 _GetACSTraversalOptions () override {return 0;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  03/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool _HandleACSTraversal(DgnV8Api::IAuxCoordSys& v8acs) override
    {
    if (DgnV8Api::ACSType::Extended == v8acs.GetType())
        return false; // Entering lat/long will not be handled by requiring a special ACS in Bim002...
        
    Bentley::WString acsNameV8 = v8acs.GetName();

    if (Bentley::WString::IsNullOrEmpty(acsNameV8.c_str()))
        return false;

    // See if this ACS already exists...if same name from different models, just keep first one...
    Utf8String    acsName(acsNameV8.c_str());
    DgnCode       acsCode = AuxCoordSystem::CreateCode(m_v8mm.GetDgnModel(), nullptr, acsName);
    DgnElementId  acsId = m_v8mm.GetDgnModel().GetDgnDb().Elements().QueryElementIdByCode(acsCode);

    if (acsId.IsValid()) // Do we need to do something here for update???
        return false;

    AuxCoordSystemPtr   acsElm = AuxCoordSystem::CreateNew(m_v8mm.GetDgnModel(), nullptr, acsName);
    Bentley::DPoint3d   acsOrigin;
    Bentley::RotMatrix  acsRMatrix;

    v8acs.GetRotation(acsRMatrix);
    v8acs.GetOrigin(acsOrigin);
    acsOrigin.Scale(acsOrigin, m_toMeters); // Account for uor->meter scale...

    acsElm->SetType((ACSType) v8acs.GetType());
    acsElm->SetOrigin((DPoint3dCR) acsOrigin);
    acsElm->SetRotation((RotMatrixCR) acsRMatrix);

    Bentley::WString acsDescrV8 = v8acs.GetDescription();

    if (!Bentley::WString::IsNullOrEmpty(acsDescrV8.c_str()))
        {
        Utf8String acsDescr;

        acsDescr.Assign(acsDescrV8.c_str());
        acsElm->SetDescription(acsDescr.c_str());
        }

    DgnDbStatus acsStatus;
    acsElm->Insert(&acsStatus);

    return false;
    }

}; // ConvertModelACSTraverser

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/15
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus  RootModelConverter::Process()
    {
    AddSteps(9);

    StopWatch totalTimer(true);
   
    StopWatch timer(true);

    _OnConversionStart();

    _BeginConversion();
    if (WasAborted())
        return ERROR;

    ConverterLogging::LogPerformance(timer, "Begin Conversion");

    timer.Start();
    
    _ConvertModels();       // This is where we discover just about all of the V8 files and models that we'll need to mine for data in the subsequent steps
    if (WasAborted())
        return ERROR;
    
    ConverterLogging::LogPerformance(timer, "Convert Models");

    timer.Start();
    _ConvertDgnV8Tags();        // *** TRICKY! You must convert tags BEFORE calling _ConvertSchemas ***
    if (WasAborted())
        return ERROR;

    ConverterLogging::LogPerformance(timer, "Convert Dgn V8Tags");

    timer.Start();
    if (!m_config.GetOptionValueBool("SkipECContent", false))
        _ConvertSchemas();  // Convert the ECSchemas found in spatial and drawing models
    if (WasAborted())
        return ERROR;
    
    ConverterLogging::LogPerformance(timer, "Convert Schemas (total)");
    
    SetStepName(Converter::ProgressMessage::STEP_CONVERTING_STYLES());

    timer.Start();
    _ConvertLineStyles();
    if (WasAborted())
        return ERROR;
    
    ConverterLogging::LogPerformance(timer, "Convert Line Styles");

    AddTasks((int32_t)(m_v8ModelMappings.size()));
    for (auto& modelMapping : m_v8ModelMappings)
        {
        if (WasAborted())
            return ERROR;

        SetTaskName(Converter::ProgressMessage::TASK_CONVERTING_MATERIALS(), modelMapping.GetDgnModel().GetName().c_str());
        ConvertModelMaterials (modelMapping.GetV8Model());

        ConvertModelACSTraverser acsTraverser(modelMapping, ComputeUnitsScaleFactor(modelMapping.GetV8Model()));
        DgnV8Api::IACSManager::GetManager().Traverse(acsTraverser, &modelMapping.GetV8Model());
        }

    if (m_isRootModelSpatial)
        {
        timer.Start();
        _ConvertSpatialLevels();
        if (WasAborted())
            return ERROR;

        ConverterLogging::LogPerformance(timer, "Convert Spatial Levels");

        timer.Start();
        _ConvertSpatialViews();
        if (WasAborted())
            return ERROR;

        ConverterLogging::LogPerformance(timer, "Convert Spatial Views");

        timer.Start();
        _ConvertSpatialElements();
        if (WasAborted())
            return ERROR;

        ConverterLogging::LogPerformance(timer, "Convert Spatial Elements (total)");
        }

    timer.Start();
    _ConvertDrawings();
    if (WasAborted())
        return ERROR;

    ConverterLogging::LogPerformance(timer, "Convert Drawings (total)");

    timer.Start();
    _ConvertSheets();
    if (WasAborted())
        return ERROR;

    ConverterLogging::LogPerformance(timer, "Convert Sheets (total)");

    timer.Start();
    _FinishConversion();
    if (WasAborted())
        return ERROR;

    _OnConversionComplete();
    ConverterLogging::LogPerformance(timer, "Finish conversion");

    ConverterLogging::LogPerformance(totalTimer, "Total conversion time");
    return WasAborted() ? ERROR : SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/15
+---------------+---------------+---------------+---------------+---------------+------*/
void SpatialConverterBase::_OnUpdateLevel(DgnV8Api::LevelHandle const& v8Level, DgnCategoryId cat, DgnV8FileR v8File)
    {
    // We can only check the root file's levels for changes. If a reference file defines a level with the same name as
    // some other file but with a different definition, the converter will have made an attachment-specific copy of the
    // level. We need the DgnAttachment in order to find that copy and check ... or even to know if a copy was made.
    if (v8Level.GetLevelId() == DGNV8_LEVEL_DEFAULT_LEVEL_ID || GetRootModelP()->GetDgnFileP() != &v8File)
        {
        // we will check that all attachment-specific subcategories are unchanged when we go through the view level mask.
        return;
        }

    CheckNoLevelChange(v8Level,cat,v8File);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void RootModelConverter::_SetChangeDetector(bool isUpdating)
    {
    BeAssert(isUpdating == IsUpdating());

    if (!isUpdating)
        m_changeDetector.reset(new CreatorChangeDetector);
    else
        {
        m_changeDetector.reset(new ChangeDetector); 
        m_skipECContent = false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void RootModelConverter::RootModelSpatialParams::Legacy_Converter_Init(BeFileNameCR bcName)
    {
    m_briefcaseName = bcName;
    SetReportFileName();
    m_isCreatingNewDb = true;
    m_isUpdating = false;
    }

END_DGNDBSYNC_DGNV8_NAMESPACE
