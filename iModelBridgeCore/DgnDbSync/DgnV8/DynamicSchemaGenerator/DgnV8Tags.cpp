/*--------------------------------------------------------------------------------------+
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
+--------------------------------------------------------------------------------------*/
#include "ConverterInternal.h"

BEGIN_DGNDBSYNC_DGNV8_NAMESPACE

namespace V8ECN = Bentley::ECN;

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2016
//---------------------------------------------------------------------------------------
void ConvertV8TagToDgnDbExtension::Register()
    {
    ConvertV8TagToDgnDbExtension* instance = new ConvertV8TagToDgnDbExtension();
    RegisterExtension(DgnV8Api::TagElementHandler::GetInstance(), *instance);
    }

//---------------------------------------------------------------------------------------
// Mostly identical to Sam's implementation in Graphite0505.
// @bsimethod                                                   Jeff.Marker     06/2016
//---------------------------------------------------------------------------------------
static BentleyStatus mergeClasses(V8ECN::ECClassR existingECClass, V8ECN::ECClassR newECClass)
    {
    bvector<V8ECN::ECPropertyCP> newProps;

    // First, check for conflicting property definitions. If any, we can't merge.
    for (V8ECN::ECPropertyCP prop : newECClass.GetProperties())
        {
        V8ECN::ECPropertyCP existingProp = existingECClass.GetPropertyP(prop->GetName().c_str());
        if (nullptr == existingProp)
            {
            newProps.push_back(prop);
            continue;
            }
        
        // Check property type.
        if (existingProp->GetTypeName() != prop->GetTypeName())
            return ERROR; 

        // Ignoring conflicts over read-only attributes and display labels.
        }

    // Add the properties added by the new class into the existing class.
    for (V8ECN::ECPropertyCP newProp : newProps)
        {
        V8ECN::PrimitiveECPropertyP addedProp;
        if (V8ECN::ECOBJECTS_STATUS_Success != existingECClass.CreatePrimitiveProperty(addedProp, newProp->GetName(), newProp->GetAsPrimitiveProperty()->GetType()))
            { BeAssert(false); continue; }
        
        addedProp->SetDisplayLabel(newProp->GetDisplayLabel());
        }

    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// Mostly identical to Sam's implementation in Graphite0505.
// @bsimethod                                                   Jeff.Marker     06/2016
//---------------------------------------------------------------------------------------
static BentleyApi::BentleyStatus convertTagSetToECClass
(
V8ECN::ECClassP& ecclass,
V8ECN::ECSchemaR schema,
Bentley::WStringCR tagsetDefClassName,
Bentley::WStringCR displayLabel,
Bentley::bvector<DgnV8Api::DgnTagDefinition> const& tagDefs,
bmap<uint16_t, Bentley::WString>& tagPropertyNames
)
    {
    // Create an ECClass to represent this tagsetdef.
    if (V8ECN::ECOBJECTS_STATUS_Success != schema.CreateClass(ecclass, tagsetDefClassName.c_str()))
         { BeAssert(false); return ERROR; }

    ecclass->SetDisplayLabel(displayLabel.c_str());
    
    // Ensure property names are unique (required for EC, but not tags).
    bmap<WString, bvector<uint16_t>> tagsSeen;
    for (Bentley::DgnTagDefinitionCR def : tagDefs)
        tagsSeen[def.name].push_back(def.id);

    bset<uint16_t> nonUniqueTags;
    for (bmap<WString, bvector<uint16_t>>::value_type const& tagSeenWithThisName : tagsSeen)
        {
        if (1 == tagSeenWithThisName.second.size())
            continue;
            
        for (uint16_t id : tagSeenWithThisName.second)
            nonUniqueTags.insert(id);
        }

    // Convert the tags in the set to properties of the class.
    for (Bentley::DgnTagDefinitionCR def : tagDefs)
        {
        // Can we support its value type?
        switch (def.value.GetTagType())
            {
            case DgnV8Api::MS_TAGTYPE_SINT:
            case DgnV8Api::MS_TAGTYPE_LINT:
            case DgnV8Api::MS_TAGTYPE_DOUBLE:
            case DgnV8Api::MS_TAGTYPE_WCHAR:
            case DgnV8Api::MS_TAGTYPE_CHAR:
                break;

            default:
                // Unsupported type.
                continue;
            }

        // Come up with a name for the property, based on the name of the tag.
        // Must be unqiue within the set.
        Bentley::WString uniqueTagName(def.name);
        if (nonUniqueTags.end() != nonUniqueTags.find(def.id))
            uniqueTagName.append(Bentley::WPrintfString(L"-%d", def.id));

        // Must be a valid EC identifier.
        Bentley::WString ecPropName = V8ECN::ECNameValidation::EncodeToValidName(uniqueTagName.c_str());

        // Record the tag -> property association. We'll look this up when we process tag elements later.
        tagPropertyNames[def.id] = ecPropName;

        // Create the property. V8 tags only have primitive types.
        V8ECN::PrimitiveECPropertyP prop;
        if (V8ECN::ECOBJECTS_STATUS_Success != ecclass->CreatePrimitiveProperty(prop, ecPropName.c_str()))
            { BeAssert(false); continue; }
        
        prop->SetIsReadOnly(0 != (def.propsMask & DgnV8Api::TAG_PROP_CONST));
        // N.B. ECProperty has no inherent "hidden" flag.

        switch (def.value.GetTagType())
            {
            case DgnV8Api::MS_TAGTYPE_SINT:
            case DgnV8Api::MS_TAGTYPE_LINT:     prop->SetType (V8ECN::PRIMITIVETYPE_Integer);   break;
            case DgnV8Api::MS_TAGTYPE_DOUBLE:   prop->SetType (V8ECN::PRIMITIVETYPE_Double);    break;
            case DgnV8Api::MS_TAGTYPE_WCHAR:
            case DgnV8Api::MS_TAGTYPE_CHAR:     prop->SetType (V8ECN::PRIMITIVETYPE_String);    break;

            default:
                BeAssert(false); // should have been filtered above
                break;
            }
        }

    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2016
//---------------------------------------------------------------------------------------
struct CompareInsensitiveWString
{
    bool operator()(Bentley::WString const& lhs, Bentley::WString const& rhs) const { return (lhs.CompareToI(rhs) < 0); }
};

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2016
//---------------------------------------------------------------------------------------
static void createClassPropertyThumbprint(WString& thumbprint, V8ECN::ECClassCR ecClass)
    {
    // N.B. Class names can only contain [0-9a-zA-Z_].
    
    WChar itowBuff[10];

    // Use a set as a cheap way to sort, so that order is disregarded when determining uniqueness.
    bset<WString> propThumbprints;
    for (V8ECN::ECPropertyCP prop : ecClass.GetProperties())
        {
        _itow_s((int)prop->GetAsPrimitiveProperty()->GetType(), itowBuff, 10);
        
        WString propThumbprint(prop->GetName().c_str());
        propThumbprint += L"|";
        propThumbprint += itowBuff;

        propThumbprints.insert(propThumbprint);
        }
    
    thumbprint.clear();
    for (WString const& propThumbprint : propThumbprints)
        {
        if (!thumbprint.empty())
            thumbprint += L"|";

        thumbprint += propThumbprint;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2016
//---------------------------------------------------------------------------------------
void Converter::_ConvertDgnV8Tags(bvector<DgnV8FileP> const& v8Files, bvector<DgnV8ModelP> const& uniqueModels)
    {
    //...............................................................................................................................................
    //...............................................................................................................................................
    
    typedef bmap<DgnV8Api::ElementId, Bentley::WString> T_TagSetDefToClassNameMap;
    typedef bmap<DgnV8Api::ElementId, bmap<uint16_t, Bentley::WString>> T_TagSetPropNameMap;

    T_TagSetDefToClassNameMap tagSetDefToClassMap;
    T_TagSetPropNameMap tagSetPropNameMap;

    DgnV8Api::DgnECManager& v8ECManager = DgnV8Api::DgnECManager::GetManager();

    //...............................................................................................................................................
    //...............................................................................................................................................
    // Create schema

    V8ECN::ECSchemaPtr tagSetDefSchema;
    if (V8ECN::ECOBJECTS_STATUS_Success != V8ECN::ECSchema::CreateSchema(tagSetDefSchema, GetV8TagSetDefinitionSchemaName(), 1, 0))
        { BeAssert(false); return; }

    tagSetDefSchema->SetNamespacePrefix(L"v8tag");

    // V8 class names are case-sensitive, but not in DgnDb.
    // We cannot easily use the V8 API to query insensitively, so need to maintain our own collection.
    // I am also assuming that tag sets differing only by case should be merged.
    // N.B. We first insert into the collection with the initial case, but compare insensitively. This way, the first-one-in determines the "real" case that can be used in the V8 API.
    // N.B. Due to namespace collisions inside btree, the easiest way is to use V8's bset...
    typedef Bentley::bset<Bentley::WString, CompareInsensitiveWString> T_ExistingClassNameSet;
    T_ExistingClassNameSet existingClassNames;

    for (DgnV8ModelP v8Model : uniqueModels)
        {
        DgnV8Api::TagSetCollection tagSetDefs(*v8Model->GetDgnFileP());
        for (Bentley::ElementRefP tagSetDefElRef : tagSetDefs)
            {
            DgnV8Api::ElementHandle eh(tagSetDefElRef);
            DgnV8Api::ElementId v8tagsetdefelementid = eh.GetElementId();

            // Get the tagset definition
            WChar rawSetName[TAG_SET_NAME_MAX + 1];
            Bentley::bvector<DgnV8Api::DgnTagDefinition> tagDefs;

            if (Bentley::SUCCESS != DgnV8Api::TagSetHandler::GetSetName(rawSetName, _countof(rawSetName), eh))
                { BeDataAssert(false); continue; }

            if (Bentley::SUCCESS != DgnV8Api::TagSetHandler::ExtractTagDefs(eh, tagDefs))
                { BeDataAssert(false); continue; }

            // Generate the name of the new class from that of the tagset def
            Bentley::WString tagsetDefClassName = V8ECN::ECNameValidation::EncodeToValidName(rawSetName);
            Bentley::WString displayLabel(rawSetName);

            T_ExistingClassNameSet::const_iterator foundExistingName = existingClassNames.find(tagsetDefClassName);
            if (existingClassNames.end() != foundExistingName)
                tagsetDefClassName = *foundExistingName;
            else
                existingClassNames.insert(tagsetDefClassName);

            V8ECN::ECClassP existingecclass = tagSetDefSchema->GetClassP(tagsetDefClassName.c_str());
            if (nullptr != existingecclass)
                {
                // We've seen this tagset def before but in a different file. Create a unique version now. Try to merge later.
                Bentley::WPrintfString uniqueness(L" [%s]", BeFileName(BeFileName::Basename, v8Model->GetDgnFileP()->GetFileName().c_str()).c_str());
                tagsetDefClassName = V8ECN::ECNameValidation::EncodeToValidName(tagsetDefClassName.append(uniqueness));
                displayLabel.append(uniqueness);
                }

            // Create the class
            V8ECN::ECClassP ecclass;
            if (SUCCESS != convertTagSetToECClass(ecclass, *tagSetDefSchema, tagsetDefClassName, displayLabel, tagDefs, tagSetPropNameMap[v8tagsetdefelementid]))
                continue;

            // Optionally merge differnt versions of tagset defs w/ same name
            if (nullptr != existingecclass)
                {
                if (SUCCESS == mergeClasses(*existingecclass, *ecclass))
                    {
                    tagSetDefSchema->DeleteClass(*ecclass);
                    ecclass = existingecclass;
                    }
                }

            // Now that we know the ecclass, associate it with the tagset def ELEMENT. That's how we'll associate the class with the tags.
            tagSetDefToClassMap[v8tagsetdefelementid] = ecclass->GetName();
            }
        }

    //...............................................................................................................................................
    //...............................................................................................................................................
    // Detect large-scale tag set definition duplication and try to de-dup to prevent EC class bloat (e.g. Black and Veatch).
    // When a tag set def is de-dupped, also remember its original name through a fabricated property.

    // We cannot know for certain whether a duplicate def is legitimate. To help mitigate, have an arbitrary cutoff.
    static const size_t MIN_DUPLICATES_FOR_DEDUP = 5;

    // Determine the set of unique property sets, and which classes map to them.

    typedef bmap<WString, bvector<WString>> T_ThumbprintToClassNames;
    T_ThumbprintToClassNames thumbprintToClassNames;
    
    for (V8ECN::ECClassCP ecClass : tagSetDefSchema->GetClasses())
        {
        WString thumbprint;
        createClassPropertyThumbprint(thumbprint, *ecClass);
        
        T_ThumbprintToClassNames::iterator foundIter = thumbprintToClassNames.find(thumbprint);
        if (thumbprintToClassNames.end() != foundIter)
            foundIter->second.push_back(ecClass->GetName().c_str());
        else
            thumbprintToClassNames[thumbprint] = { WString(ecClass->GetName().c_str()) };
        }

    // Then detect when to de-dup, strip dups, and remember what property to use to store the original tag set def name.
    
    typedef bmap<WString, WString> T_WStringToWString;
    T_WStringToWString dupClassNameRemap;
    T_WStringToWString dupClassNamePropNames;

    for (T_ThumbprintToClassNames::value_type iter : thumbprintToClassNames)
        {
        bvector<WString> const& dupClassNames = iter.second;

        if (dupClassNames.size() <= MIN_DUPLICATES_FOR_DEDUP)
            continue;
        
        WString effectiveClassName = dupClassNames[0];
        
        V8ECN::ECClassP effectiveClass = tagSetDefSchema->GetClassP(effectiveClassName.c_str());
        if (nullptr == effectiveClass)
            { BeAssert(false); continue; }
        
        WString rootOldClassNamePropName = L"OriginalTagSetDefName";
        WString oldClassNamePropName = rootOldClassNamePropName;
        size_t retry = 0;
        while (nullptr != effectiveClass->GetPropertyP(oldClassNamePropName.c_str()))
            oldClassNamePropName = WPrintfString(L"%ls-%u", rootOldClassNamePropName.c_str(), ++retry);

        V8ECN::PrimitiveECPropertyP oldClassNameProp;
        if (V8ECN::ECOBJECTS_STATUS_Success != effectiveClass->CreatePrimitiveProperty(oldClassNameProp, oldClassNamePropName.c_str()))
            { BeAssert(false); continue; }

        oldClassNameProp->SetIsReadOnly(true);
        oldClassNameProp->SetType(V8ECN::PRIMITIVETYPE_String);
        
        dupClassNamePropNames[effectiveClassName] = oldClassNamePropName;

        for (size_t iDupClassName = 1; iDupClassName < dupClassNames.size(); ++iDupClassName)
            {
            WStringCR dupClassName = dupClassNames[iDupClassName];
            
            // =="Remapping tag set definition class from '%s' to '%s' due to a large number of duplicate definitions."==
            //ReportIssueV(IssueSeverity::Info, IssueCategory::Filtering(), Issue::DeDupTagSetDef(), "", Utf8String(dupClassName).c_str(), Utf8String(effectiveClassName).c_str());

            dupClassNameRemap[dupClassName] = effectiveClassName;
            
            V8ECN::ECClassP dupClass = tagSetDefSchema->GetClassP(dupClassName.c_str());
            if (nullptr == dupClass)
                { BeAssert(false); continue; }

            tagSetDefSchema->DeleteClass(*dupClass);
            }
        }

    //...............................................................................................................................................
    //...............................................................................................................................................
    // Create temp table(s) for tracking tags (cheaper than traversing dependency back pointers).

    #define TEMP_TAG_MAP_TABLE_NAME "V8TagElementMap"
    #define TEMP_TAG_MAP_TABLE "temp." TEMP_TAG_MAP_TABLE_NAME

    if (DbResult::BE_SQLITE_OK != m_dgndb->CreateTable(TEMP_TAG_MAP_TABLE,
        "Id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "V8FileSyncInfoId BIGINT NOT NULL,"
        "V8TagModelId INTEGER NOT NULL,"
        "V8TagElementId INTEGER NOT NULL,"
        "V8TargetFileId BIGINT NOT NULL,"
        "V8TargetElementId INTEGER NOT NULL"
        ))
        { BeAssert(false); return; }

    if (DbResult::BE_SQLITE_OK != m_dgndb->ExecuteSql("CREATE INDEX temp.idxTagMapTable ON " TEMP_TAG_MAP_TABLE_NAME "(V8TargetFileId,V8TargetElementId)"))
        { BeAssert(false); return; }

    CachedStatementPtr insertTagRecord;
    m_dgndb->GetCachedStatement(insertTagRecord,
        "INSERT INTO " TEMP_TAG_MAP_TABLE
        " (V8FileSyncInfoId,V8TagModelId,V8TagElementId,V8TargetFileId,V8TargetElementId)"
        " values (?,?,?,?,?)"
    );

    //...............................................................................................................................................
    //...............................................................................................................................................
    // Record which tag elements are associated with which target elements.

    for (DgnV8ModelP v8Model : uniqueModels)
        {
        for (auto v8ElRef : v8Model->GetElementsCollection())
            {
            DgnV8Api::ElementHandle v8TagEh(v8ElRef);
            if (&DgnV8Api::TagElementHandler::GetInstance() != &v8TagEh.GetHandler())
                continue;
            
            DgnV8Api::ElementHandle v8TargetEh;
            if (Bentley::SUCCESS != DgnV8Api::TagElementHandler::GetTargetElement(v8TargetEh, v8TagEh))
                continue;
            
            insertTagRecord->Reset();
            insertTagRecord->BindId(1, GetRepositoryLinkId(*v8TagEh.GetDgnFileP()));
            insertTagRecord->BindInt(2, v8TagEh.GetDgnModelP()->GetModelId());
            insertTagRecord->BindInt64(3, v8TagEh.GetElementId());
            insertTagRecord->BindId(4, GetRepositoryLinkId(*v8TargetEh.GetDgnFileP()));
            insertTagRecord->BindInt64(5, v8TargetEh.GetElementId());
            insertTagRecord->Step();
            }
        }

    //...............................................................................................................................................
    //...............................................................................................................................................
    // Associate the tag values with EC instances on elements.

    CachedStatementPtr selectTargets;
    m_dgndb->GetCachedStatement(selectTargets,
        "SELECT DISTINCT V8TargetFileId,V8TargetElementId"
        " FROM " TEMP_TAG_MAP_TABLE
        );

    bset<uint64_t> v8FilesWithImportedTagSetDefSchema;
    
    // Iterate over each target element that has tags associated with it.
    while (DbResult::BE_SQLITE_ROW == selectTargets->Step())
        {
        auto targetFileId = selectTargets->GetValueId<RepositoryLinkId>(0);
        DgnV8Api::ElementId targetElementId = selectTargets->GetValueInt64(1);
        
        bvector<DgnV8FileP>::const_iterator foundV8TargetFile = std::find_if(v8Files.begin(), v8Files.end(), [&](DgnV8FileP v8File) { return targetFileId == GetRepositoryLinkId(*v8File); });
        if (v8Files.end() == foundV8TargetFile)
            { BeAssert(false); continue; }

        // Since we see each EC property value independently as we traverse the individual tag elements, need to hold on to instances and write them at the end.
        typedef bmap<V8ECN::ECClassCP, DgnV8Api::DgnElementECInstancePtr> T_PerClassInstanceMap;
        T_PerClassInstanceMap perClassInstanceMap;
        
        CachedStatementPtr selectTags;
        m_dgndb->GetCachedStatement(selectTags,
            "SELECT V8FileSyncInfoId,V8TagModelId,V8TagElementId"
            " FROM " TEMP_TAG_MAP_TABLE
            " WHERE V8TargetFileId=? AND V8TargetElementId=?"
            );

        selectTags->BindId(1, targetFileId);
        selectTags->BindInt64(2, targetElementId);

        // Then iterate over every tag element associated to the current target element.
        while (DbResult::BE_SQLITE_ROW == selectTags->Step())
            {
            auto tagFileId = selectTags->GetValueId<RepositoryLinkId>(0);
            bvector<DgnV8FileP>::const_iterator foundV8File = std::find_if(v8Files.begin(), v8Files.end(), [&](DgnV8FileP v8File) { return tagFileId == GetRepositoryLinkId(*v8File); });
            
            if (v8Files.end() == foundV8File)
                { BeAssert(false); continue; }

            DgnV8Api::ModelId tagModelId = selectTags->GetValueInt(1);
            DgnV8Api::ElementId tagElementId = selectTags->GetValueInt64(2);

            Bentley::DgnModelPtr v8model = (*foundV8File)->LoadModelById(tagModelId);
            if (!v8model.IsValid())
                {
                BeAssert(false);
                continue;
                }
            v8model->FillSections(DgnV8Api::DgnModelSections::Model);    // make sure it's filled

            ElementRefP v8TagElRef = v8model->FindElementByID(tagElementId);
            if (nullptr == v8TagElRef)
                { BeAssert(false); continue; }

            DgnV8Api::ElementHandle v8TagEH(v8TagElRef);

            DgnV8Api::ElementId v8TagSetDefId = DgnV8Api::TagElementHandler::GetSetDefinitionID(v8TagEH);
            
            T_TagSetDefToClassNameMap::const_iterator foundTagClassName = tagSetDefToClassMap.find(v8TagSetDefId);
            if (tagSetDefToClassMap.end() == foundTagClassName)
                { BeAssert(false); continue; }

            V8ECN::ECClassCP tagClass = tagSetDefSchema->GetClassCP(foundTagClassName->second.c_str());
            BentleyApi::WString dupClassNamePropName;
            if (nullptr == tagClass)
                {
                // Were we de-dupped?
                T_WStringToWString::const_iterator foundDupClassRemap = dupClassNameRemap.find(WString(foundTagClassName->second.c_str()));
                if (dupClassNameRemap.end() == foundDupClassRemap)
                    { BeAssert(false); continue; }
                
                tagClass = tagSetDefSchema->GetClassCP(foundDupClassRemap->second.c_str());
                if (nullptr == tagClass)
                    { BeAssert(false); continue; }

                T_WStringToWString::const_iterator foundDupClassNamePropName = dupClassNamePropNames.find(foundDupClassRemap->second.c_str());
                if (dupClassNamePropNames.end() == foundDupClassNamePropName)
                    { BeAssert(false); continue; }
                
                dupClassNamePropName = foundDupClassNamePropName->second.c_str();
                }

            DgnV8Api::DgnTagDefinition def;
            if (Bentley::SUCCESS != DgnV8Api::TagElementHandler::Extract(def, v8TagEH, *v8TagEH.GetDgnModelP()))
                { BeDataAssert(false); continue; }

            DgnV8Api::DgnTagValue tv;
            if (Bentley::SUCCESS != DgnV8Api::TagElementHandler::GetAttributeValue(v8TagEH, tv))
                { BeDataAssert(false); continue; }


            T_TagSetPropNameMap::const_iterator foundPropNameMap = tagSetPropNameMap.find(v8TagSetDefId);
            if (tagSetPropNameMap.end() == foundPropNameMap)
                { BeAssert(false); continue; }
            
            bmap<uint16_t, Bentley::WString>::const_iterator foundPropName = foundPropNameMap->second.find(def.id);
            if (foundPropNameMap->second.end() == foundPropName)
                { /* Can be expected if using an unsupported value type. */ continue; }

            // Only import the generated tag schema as-needed.
            if (v8FilesWithImportedTagSetDefSchema.end() == v8FilesWithImportedTagSetDefSchema.find(targetFileId.GetValue()))
                {
                v8FilesWithImportedTagSetDefSchema.insert(targetFileId.GetValue());
                (*foundV8TargetFile)->GetDictionaryModel().SetReadOnly(false);

                if (DgnV8Api::SCHEMAIMPORT_Success != v8ECManager.ImportSchema(*tagSetDefSchema, **foundV8TargetFile))
                    {
                    BeAssert(false); continue;
                    }
                }

            T_PerClassInstanceMap::const_iterator foundInstance = perClassInstanceMap.find(tagClass);
            if (perClassInstanceMap.end() == foundInstance)
                {
                DgnV8Api::ElementHandle targetV8Eh((*foundV8TargetFile)->FindByElementId(targetElementId, false));
                if (!targetV8Eh.IsValid())
                    { BeAssert(false); continue; }
                
                targetV8Eh.GetDgnModelP()->SetReadOnly(false);

                DgnECInstanceEnablerP enabler = v8ECManager.ObtainInstanceEnabler(*tagClass, **foundV8TargetFile);
                if (SUCCESS != enabler->CreateInstanceOnElement(&perClassInstanceMap[tagClass], enabler->GetSharedWipInstance(), targetV8Eh))
                    { BeAssert(false); continue; }

                foundInstance = perClassInstanceMap.find(tagClass);

                // Were we a de-dup? Save the original tag set def name.
                if (!dupClassNamePropName.empty())
                    foundInstance->second->SetValue(dupClassNamePropName.c_str(), V8ECN::ECValue(foundTagClassName->second.c_str(), true));
                }
            else
                {
                // We've encountered bad DgnV8 data with duplicate values in a single tag set... let first-in win. Also prevents issues below trying to set readonly properties (more than once).
                bool isNull = false;
                if ((V8ECN::ECOBJECTS_STATUS_Success == foundInstance->second->IsPropertyNull(isNull, foundPropName->second.c_str())) && !isNull)
                    continue;
                }

            V8ECN::ECValue convertedValue;
            switch (tv.GetTagType())
                {
                case DgnV8Api::MS_TAGTYPE_SINT:     convertedValue = V8ECN::ECValue((int32_t)tv.GetShortValue());       break;
                case DgnV8Api::MS_TAGTYPE_LINT:     convertedValue = V8ECN::ECValue((int32_t)tv.GetLongValue());        break;
                case DgnV8Api::MS_TAGTYPE_DOUBLE:   convertedValue = V8ECN::ECValue(tv.GetDoubleValue());               break;
                case DgnV8Api::MS_TAGTYPE_WCHAR:
                case DgnV8Api::MS_TAGTYPE_CHAR:     convertedValue = V8ECN::ECValue(tv.GetStringValue(NULL).c_str());   break;

                default:
                    // Unsupported type. Should have been filtered above by the implication that its property name wouldn't have been mapped.
                    BeAssert(false);
                    continue;
                }

            if (V8ECN::ECOBJECTS_STATUS_Success != foundInstance->second->SetValue(foundPropName->second.c_str(), convertedValue))
                { BeAssert(false); continue; }
            }
        
        for (T_PerClassInstanceMap::value_type const& iter : perClassInstanceMap)
            {
            if (Bentley::SUCCESS != iter.second->WriteChanges())
                { BeAssert(false); }
            }
        }
    }

END_DGNDBSYNC_DGNV8_NAMESPACE
