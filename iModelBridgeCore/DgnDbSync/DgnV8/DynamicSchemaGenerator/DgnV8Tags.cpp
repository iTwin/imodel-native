/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ConverterInternal.h"
#include "ECConversion.h"

BEGIN_DGNDBSYNC_DGNV8_NAMESPACE

namespace V8ECN = Bentley::ECN;

typedef bmap<DgnV8Api::ElementId, Utf8String> T_TagSetDefToClassNameMap;
typedef bmap<DgnV8Api::ElementId, bmap<uint16_t, Utf8String>> T_TagSetPropNameMap;

//---------------------------------------------------------------------------------------
// Mostly identical to Sam's implementation in Graphite0505.
// @bsimethod                                                   Jeff.Marker     06/2016
//---------------------------------------------------------------------------------------
static BentleyStatus mergeClasses(ECN::ECClassR existingECClass, ECN::ECClassR newECClass)
    {
    bvector<ECN::ECPropertyCP> newProps;

    // First, check for conflicting property definitions. If any, we can't merge.
    for (ECN::ECPropertyCP prop : newECClass.GetProperties())
        {
        ECN::ECPropertyCP existingProp = existingECClass.GetPropertyP(prop->GetName().c_str());
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
    for (ECN::ECPropertyCP newProp : newProps)
        {
        ECN::PrimitiveECPropertyP addedProp;
        if (ECN::ECObjectsStatus::Success != existingECClass.CreatePrimitiveProperty(addedProp, newProp->GetName(), newProp->GetAsPrimitiveProperty()->GetType()))
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
ECN::ECEntityClassP& ecclass,
ECN::ECSchemaR schema,
Utf8StringCR tagsetDefClassName,
Utf8StringCR displayLabel,
Bentley::bvector<DgnV8Api::DgnTagDefinition> const& tagDefs,
bmap<uint16_t, Utf8String>& tagPropertyNames
)
    {
    // Create an ECClass to represent this tagsetdef.
    if (ECN::ECObjectsStatus::Success != schema.CreateEntityClass(ecclass, tagsetDefClassName.c_str()))
         { BeAssert(false); return ERROR; }

    ecclass->SetDisplayLabel(displayLabel.c_str());
    
    // Ensure property names are unique (required for EC, but not tags).
    bmap<Utf8String, bvector<uint16_t>> tagsSeen;
    for (Bentley::DgnTagDefinitionCR def : tagDefs)
        tagsSeen[Utf8String(def.name)].push_back(def.id);

    bset<uint16_t> nonUniqueTags;
    for (bmap<Utf8String, bvector<uint16_t>>::value_type const& tagSeenWithThisName : tagsSeen)
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
        // Must be unique within the set.
        Utf8String uniqueTagName(def.name);
        if (uniqueTagName.empty())
            continue;

        if (nonUniqueTags.end() != nonUniqueTags.find(def.id))
            uniqueTagName.append(Bentley::Utf8PrintfString("-%d", def.id));
        if (uniqueTagName.EqualsIAscii("Element"))
            uniqueTagName.append(Utf8PrintfString("-%d", def.id));

        if (uniqueTagName.EqualsIAscii("Id"))
            uniqueTagName.append(Utf8PrintfString("-%d", def.id));

        // Must be a valid EC identifier.
        Utf8String ecPropName;
        ECN::ECNameValidation::EncodeToValidName(ecPropName, uniqueTagName);


        // Record the tag -> property association. We'll look this up when we process tag elements later.
        tagPropertyNames[def.id] = ecPropName;

        // Create the property. V8 tags only have primitive types.
        ECN::PrimitiveECPropertyP prop;
        if (ECN::ECObjectsStatus::Success != ecclass->CreatePrimitiveProperty(prop, ecPropName.c_str()))
            { BeAssert(false); continue; }
        
        if (!ecPropName.Equals(uniqueTagName))
            prop->SetDisplayLabel(uniqueTagName);

        prop->SetIsReadOnly(0 != (def.propsMask & DgnV8Api::TAG_PROP_CONST));
        // N.B. ECProperty has no inherent "hidden" flag.

        switch (def.value.GetTagType())
            {
            case DgnV8Api::MS_TAGTYPE_SINT:
            case DgnV8Api::MS_TAGTYPE_LINT:     prop->SetType (ECN::PRIMITIVETYPE_Integer);   break;
            case DgnV8Api::MS_TAGTYPE_DOUBLE:   prop->SetType (ECN::PRIMITIVETYPE_Double);    break;
            case DgnV8Api::MS_TAGTYPE_WCHAR:
            case DgnV8Api::MS_TAGTYPE_CHAR:     prop->SetType (ECN::PRIMITIVETYPE_String);    break;

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
static void createClassPropertyThumbprint(Utf8StringR thumbprint, ECN::ECClassCR ecClass)
    {
    // N.B. Class names can only contain [0-9a-zA-Z_].
    
    Utf8Char itoaBuff[_MAX_ITOSTR_BASE10_COUNT + 1];

    // Use a set as a cheap way to sort, so that order is disregarded when determining uniqueness.
    bset<Utf8String> propThumbprints;
    for (ECN::ECPropertyCP prop : ecClass.GetProperties(false))
        {
        if (0 != _itoa_s((int)prop->GetAsPrimitiveProperty()->GetType(), itoaBuff, 10))
            {
            BeAssert(false);
            continue;
            }
        itoaBuff[_countof(itoaBuff) - 1] = 0;
        
        Utf8String propThumbprint(prop->GetName().c_str());
        propThumbprint += "|";
        propThumbprint += itoaBuff;

        propThumbprints.insert(propThumbprint);
        }
    
    thumbprint.clear();
    for (Utf8String const& propThumbprint : propThumbprints)
        {
        if (!thumbprint.empty())
            thumbprint += "|";

        thumbprint += propThumbprint;
        }
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
// @bsimethod                                                   Jeff.Marker     06/2016
//---------------------------------------------------------------------------------------
ECN::ECSchemaPtr DynamicSchemaGenerator::CreateDgnV8TagSetDefinitionSchema(bvector<DgnV8FileP> const& v8Files, bvector<DgnV8ModelP> const& uniqueModels)
    {
    DgnV8Api::DgnECManager& v8ECManager = DgnV8Api::DgnECManager::GetManager();
    //...............................................................................................................................................
    //...............................................................................................................................................
    // Create schema

    ECN::ECSchemaPtr tagSetDefSchema;
    if (ECN::ECObjectsStatus::Success != ECN::ECSchema::CreateSchema(tagSetDefSchema, Converter::GetV8TagSetDefinitionSchemaName(), "v8tag", 1, 0, 0))
        {
        BeAssert(false); 
        return nullptr;
        }
    ECN::ECSchemaPtr coreSchema = ECN::CoreCustomAttributeHelper::GetSchema();
    if (coreSchema.IsValid())
        {
        ECN::ECObjectsStatus stat = tagSetDefSchema->AddReferencedSchema(*coreSchema);
        if (ECN::ECObjectsStatus::Success != stat && ECN::ECObjectsStatus::NamedItemAlreadyExists != stat)
            LOG.warning("Error adding a reference to the core custom attributes schema.");
        else
            {
            ECN::IECInstancePtr dynamicInstance = ECN::CoreCustomAttributeHelper::CreateCustomAttributeInstance("DynamicSchema");
            if (dynamicInstance.IsValid())
                tagSetDefSchema->SetCustomAttribute(*dynamicInstance);
            }
        }

    // V8 class names are case-sensitive, but not in DgnDb.
    // We cannot easily use the V8 API to query insensitively, so need to maintain our own collection.
    // I am also assuming that tag sets differing only by case should be merged.
    // N.B. We first insert into the collection with the initial case, but compare insensitively. This way, the first-one-in determines the "real" case that can be used in the V8 API.
    // N.B. Due to namespace collisions inside btree, the easiest way is to use V8's bset...
    typedef BentleyApi::bset<Utf8String, CompareIUtf8Ascii> T_ExistingClassNameSet;
    T_ExistingClassNameSet existingClassNames;
    T_TagSetDefToClassNameMap tagSetDefToClassMap;
    T_TagSetPropNameMap tagSetPropNameMap;

    ECN::ECClassCP elementAspectBaseClass = m_converter.GetDgnDb().Schemas().GetClass(BIS_ECSCHEMA_NAME, BIS_CLASS_ElementUniqueAspect);
    ECN::ECClassP nonConstBase = const_cast<ECN::ECClassP>(elementAspectBaseClass);
    tagSetDefSchema->AddReferencedSchema(nonConstBase->GetSchemaR());
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
                {
                BeDataAssert(false); continue;
                }

            if (Bentley::SUCCESS != DgnV8Api::TagSetHandler::ExtractTagDefs(eh, tagDefs))
                {
                BeDataAssert(false); continue;
                }

            // Generate the name of the new class from that of the tagset def
            Utf8String tagsetDefClassName = Utf8String(V8ECN::ECNameValidation::EncodeToValidName(rawSetName).c_str());
            Utf8String displayLabel(rawSetName);

            T_ExistingClassNameSet::const_iterator foundExistingName = existingClassNames.find(tagsetDefClassName);
            if (existingClassNames.end() != foundExistingName)
                tagsetDefClassName = *foundExistingName;
            else
                existingClassNames.insert(tagsetDefClassName);

            ECN::ECClassP existingecclass = tagSetDefSchema->GetClassP(tagsetDefClassName.c_str());
            if (nullptr != existingecclass)
                {
                // We've seen this tagset def before but in a different file. Create a unique version now. Try to merge later.
                Utf8PrintfString uniqueness(" [%s]", Utf8String(BeFileName(BeFileName::Basename, v8Model->GetDgnFileP()->GetFileName().c_str()).c_str()).c_str());
                ECN::ECNameValidation::EncodeToValidName(tagsetDefClassName, tagsetDefClassName.append(uniqueness));
                displayLabel.append(uniqueness);
                }

            // Create the class
            ECN::ECEntityClassP ecclass;
            if (SUCCESS != convertTagSetToECClass(ecclass, *tagSetDefSchema, tagsetDefClassName, displayLabel, tagDefs, tagSetPropNameMap[v8tagsetdefelementid]))
                continue;

            // Optionally merge different versions of tagset defs w/ same name
            if (nullptr != existingecclass)
                {
                if (SUCCESS == mergeClasses(*existingecclass, *ecclass))
                    {
                    tagSetDefSchema->DeleteClass(*ecclass);
                    ecclass = existingecclass->GetEntityClassP();
                    }
                }

            // Now that we know the ecclass, associate it with the tagset def ELEMENT. That's how we'll associate the class with the tags.
            tagSetDefToClassMap[v8tagsetdefelementid] = ecclass->GetName();
            ecclass->AddBaseClass(*elementAspectBaseClass);
            }
        }

    if (tagSetDefSchema->GetClassCount() == 0)
        return nullptr;

    //...............................................................................................................................................
    //...............................................................................................................................................
    // Detect large-scale tag set definition duplication and try to de-dup to prevent EC class bloat (e.g. Black and Veatch).
    // When a tag set def is de-dupped, also remember its original name through a fabricated property.

    // We cannot know for certain whether a duplicate def is legitimate. To help mitigate, have an arbitrary cutoff.
    static const size_t MIN_DUPLICATES_FOR_DEDUP = 5;

    // Determine the set of unique property sets, and which classes map to them.

    typedef bmap<Utf8String, bvector<Utf8String>> T_ThumbprintToClassNames;
    T_ThumbprintToClassNames thumbprintToClassNames;

    for (ECN::ECClassCP ecClass : tagSetDefSchema->GetClasses())
        {
        Utf8String thumbprint;
        createClassPropertyThumbprint(thumbprint, *ecClass);

        T_ThumbprintToClassNames::iterator foundIter = thumbprintToClassNames.find(thumbprint);
        if (thumbprintToClassNames.end() != foundIter)
            foundIter->second.push_back(ecClass->GetName().c_str());
        else
            thumbprintToClassNames[thumbprint] = {ecClass->GetName()};
        }

    // Then detect when to de-dup, strip dups, and remember what property to use to store the original tag set def name.

    typedef bmap<Utf8String, Utf8String> T_Utf8StringToUtf8String;
    T_Utf8StringToUtf8String dupClassNameRemap;
    T_Utf8StringToUtf8String dupClassNamePropNames;

    for (T_ThumbprintToClassNames::value_type iter : thumbprintToClassNames)
        {
        bvector<Utf8String> const& dupClassNames = iter.second;

        if (dupClassNames.size() <= MIN_DUPLICATES_FOR_DEDUP)
            continue;

        Utf8String effectiveClassName = dupClassNames[0];

        ECN::ECClassP effectiveClass = tagSetDefSchema->GetClassP(effectiveClassName.c_str());
        if (nullptr == effectiveClass)
            {
            BeAssert(false); continue;
            }

        Utf8String rootOldClassNamePropName = "OriginalTagSetDefName";
        Utf8String oldClassNamePropName = rootOldClassNamePropName;
        size_t retry = 0;
        while (nullptr != effectiveClass->GetPropertyP(oldClassNamePropName.c_str()))
            oldClassNamePropName = Utf8PrintfString("%s-%u", rootOldClassNamePropName.c_str(), ++retry);

        ECN::PrimitiveECPropertyP oldClassNameProp;
        if (ECN::ECObjectsStatus::Success != effectiveClass->CreatePrimitiveProperty(oldClassNameProp, oldClassNamePropName.c_str()))
            {
            BeAssert(false); continue;
            }

        oldClassNameProp->SetIsReadOnly(true);
        oldClassNameProp->SetType(ECN::PRIMITIVETYPE_String);

        dupClassNamePropNames[effectiveClassName] = oldClassNamePropName;

        for (size_t iDupClassName = 1; iDupClassName < dupClassNames.size(); ++iDupClassName)
            {
            Utf8StringCR dupClassName = dupClassNames[iDupClassName];

            // =="Remapping tag set definition class from '%s' to '%s' due to a large number of duplicate definitions."==
            //ReportIssueV(IssueSeverity::Info, IssueCategory::Filtering(), Issue::DeDupTagSetDef(), "", Utf8String(dupClassName).c_str(), Utf8String(effectiveClassName).c_str());

            dupClassNameRemap[dupClassName] = effectiveClassName;

            ECN::ECClassP dupClass = tagSetDefSchema->GetClassP(dupClassName.c_str());
            if (nullptr == dupClass)
                {
                BeAssert(false); continue;
                }

            for (T_TagSetDefToClassNameMap::iterator it = tagSetDefToClassMap.begin(); it != tagSetDefToClassMap.end(); it++)
                {
                if (it->second.Equals(dupClass->GetName()))
                    tagSetDefToClassMap[it->first] = effectiveClassName;
                }
            tagSetDefSchema->DeleteClass(*dupClass);
            }
        }

    for (auto const& pair : tagSetDefToClassMap)
        {
        BeSQLite::DbResult result;
        if (BeSQLite::DbResult::BE_SQLITE_OK != (result = m_converter.GetDgnDb().SavePropertyString(DgnV8Info::V8TagSet(pair.second.c_str()), nullptr, pair.first)))
            return nullptr;
        }

    for (auto const& pair : tagSetPropNameMap)
        {
        Utf8PrintfString elementId("%" PRIu64, pair.first);
        Json::Value mapping;
        for (auto const& mapEntry : pair.second)
            {
            Utf8PrintfString tagId("%" PRIu16, mapEntry.first);
            mapping[tagId] = mapEntry.second;
            }
        BeSQLite::DbResult result;
        if (BeSQLite::DbResult::BE_SQLITE_OK != (result = m_converter.GetDgnDb().SavePropertyString(DgnV8Info::V8TagDefinition(elementId.c_str()), mapping.ToString())))
            return nullptr;
        }
    return tagSetDefSchema;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2016
//---------------------------------------------------------------------------------------
void ConvertV8TagToDgnDbExtension::Register()
    {
    ConvertV8TagToDgnDbExtension* instance = new ConvertV8TagToDgnDbExtension();
    RegisterExtension(DgnV8Api::TagElementHandler::GetInstance(), *instance);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            04/2019
//---------------+---------------+---------------+---------------+---------------+-------
bool ConvertV8TagToDgnDbExtension::TryFindClassNameForTagSet(Converter& converter, DgnV8Api::ElementId tagSetId, Utf8StringR className)
    {
    CachedStatementPtr stmt = nullptr;
    Utf8String sql("SELECT Name FROM " BEDB_TABLE_Property " WHERE Id=? AND NameSpace='dgn_V8TagSet'");

    auto stat = converter.GetDgnDb().GetCachedStatement(stmt, sql.c_str());
    if (stat != BE_SQLITE_OK)
        {
        BeAssert(false && "Could not retrieve cached statement.");
        return false;
        }

    stmt->BindUInt64(1, tagSetId);
    if (BE_SQLITE_ROW != stmt->Step())
        return false;

    className = stmt->GetValueText(0);
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            04/2019
//---------------+---------------+---------------+---------------+---------------+-------
bool ConvertV8TagToDgnDbExtension::TryFindPropNamesForTagSet(Converter& converter, DgnV8Api::ElementId tagSetId, Json::Value& propNames)
    {
    CachedStatementPtr stmt = nullptr;
    Utf8String sql("SELECT StrData FROM " BEDB_TABLE_Property " WHERE Name=? AND NameSpace='dgn_V8TagDef'");

    auto stat = converter.GetDgnDb().GetCachedStatement(stmt, sql.c_str());
    if (stat != BE_SQLITE_OK)
        {
        BeAssert(false && "Could not retrieve cached statement.");
        return false;
        }

    Utf8PrintfString elementId("%" PRIu64, tagSetId);

    stmt->BindText(1, elementId, BeSQLite::Statement::MakeCopy::No);
    if (BE_SQLITE_ROW != stmt->Step())
        return false;

    if (!Json::Reader::Parse(stmt->GetValueText(0), propNames))
        return false;

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2016
//---------------------------------------------------------------------------------------
ConvertToDgnDbElementExtension::Result ConvertV8TagToDgnDbExtension::_PreConvertElement(DgnV8EhCR v8el, Converter& converter, ResolvedModelMapping const& v8mm)
    {
    if (&DgnV8Api::TagElementHandler::GetInstance() != &v8el.GetHandler())
        return Result::SkipElement;

    DgnV8Api::ElementHandle v8TargetEh;
    if (Bentley::SUCCESS != DgnV8Api::TagElementHandler::GetTargetElement(v8TargetEh, v8el))
        return Result::SkipElement;

    DgnElementId targetId;
    if (!converter.TryFindElement(targetId, v8TargetEh))
        {
        return Result::SkipElement;
        }
    DgnElementPtr targetElement = converter.GetDgnDb().Elements().GetForEdit<DgnElement>(targetId);
    if (!targetElement.IsValid())
        {
        return Result::SkipElement;
        }

    ECN::ECSchemaCP tagSetDefSchema = converter.GetDgnDb().Schemas().GetSchema(Converter::GetV8TagSetDefinitionSchemaName());
    if (nullptr == tagSetDefSchema)
        return Result::SkipElement;

    DgnV8Api::ElementId v8TagSetDefId = DgnV8Api::TagElementHandler::GetSetDefinitionID(v8el);
    Utf8String foundTagClassName;
    if (!TryFindClassNameForTagSet(converter, v8TagSetDefId, foundTagClassName))
        {
        BeAssert(false); 
        return Result::SkipElement;
        }

    ECN::ECClassCP tagClass = tagSetDefSchema->GetClassCP(foundTagClassName.c_str());
    if (nullptr == tagClass)
        {
        return Result::SkipElement;
        }

    DgnV8Api::DgnTagDefinition def;
    if (Bentley::SUCCESS != DgnV8Api::TagElementHandler::Extract(def, v8el, *v8el.GetDgnModelP()))
        {
        BeDataAssert(false); 
        return Result::SkipElement;
        }

    DgnV8Api::DgnTagValue tv;
    if (Bentley::SUCCESS != DgnV8Api::TagElementHandler::GetAttributeValue(v8el, tv))
        {
        BeDataAssert(false);
        return Result::SkipElement;
        }

    DgnElement::UniqueAspect* aspect = DgnElement::UniqueAspect::GetAspectP(*targetElement, *tagClass);
    if (nullptr == aspect)
        {
        ECN::StandaloneECInstancePtr instance = tagClass->GetDefaultStandaloneEnabler()->CreateInstance();
        DgnElement::GenericUniqueAspect::SetAspect(*targetElement, *instance);
        aspect = DgnElement::UniqueAspect::GetAspectP(*targetElement, *tagClass);
        }

    ECN::ECValue convertedValue;
    switch (tv.GetTagType())
        {
        case DgnV8Api::MS_TAGTYPE_SINT:     convertedValue = ECN::ECValue((int32_t) tv.GetShortValue());       break;
        case DgnV8Api::MS_TAGTYPE_LINT:     convertedValue = ECN::ECValue((int32_t) tv.GetLongValue());        break;
        case DgnV8Api::MS_TAGTYPE_DOUBLE:   convertedValue = ECN::ECValue(tv.GetDoubleValue());               break;
        case DgnV8Api::MS_TAGTYPE_WCHAR:
        case DgnV8Api::MS_TAGTYPE_CHAR:     convertedValue = ECN::ECValue(tv.GetStringValue(NULL).c_str());   break;

        default:
            // Unsupported type. Should have been filtered above by the implication that its property name wouldn't have been mapped.
            BeAssert(false);
            return Result::SkipElement;
        }

    Json::Value propNames;
    if (!TryFindPropNamesForTagSet(converter, v8TagSetDefId, propNames))
        { BeAssert(false); return Result::SkipElement; }
            
    Utf8PrintfString tagId("%" PRIu16, def.id);

    if (!propNames.isMember(tagId.c_str()))
        { 
        /* Can be expected if using an unsupported value type. */ 
        return Result::SkipElement;
        }

    aspect->SetPropertyValue(propNames[tagId.c_str()].asCString(), convertedValue);
    targetElement->Update();

    return Result::SkipElement;
    }

END_DGNDBSYNC_DGNV8_NAMESPACE
