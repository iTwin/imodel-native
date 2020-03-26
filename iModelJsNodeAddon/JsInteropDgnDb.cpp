/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "IModelJsNative.h"
#include <Bentley/BeDirectoryIterator.h>
#include <DgnPlatform/DgnGeoCoord.h>

using namespace IModelJsNative;

#define SET_IF_NOT_EMPTY_STR(j, str) {if (!(str).empty()) j = str;}
#define SET_IF_NOT_NULL_STR(j, str) {if (nullptr != (str)) j = str;}

BE_JSON_NAME(baseClasses)
BE_JSON_NAME(code)
BE_JSON_NAME(customAttributes)
BE_JSON_NAME(direction)
BE_JSON_NAME(displayLabel)
BE_JSON_NAME(ecclass)
BE_JSON_NAME(extendedType)
BE_JSON_NAME(federationGuid)
BE_JSON_NAME(isCustomHandled)
BE_JSON_NAME(isCustomHandledOrphan);
// unused - BE_JSON_NAME(kindOfQuantity)
BE_JSON_NAME(maxOccurs)
BE_JSON_NAME(maximumLength)
BE_JSON_NAME(maximumValue)
BE_JSON_NAME(minOccurs)
BE_JSON_NAME(minimumLength)
BE_JSON_NAME(minimumValue)
BE_JSON_NAME(modifier)
BE_JSON_NAME(primitiveType)
BE_JSON_NAME(properties)
BE_JSON_NAME(readOnly)
BE_JSON_NAME(relationshipClass)
BE_JSON_NAME(structName)
BE_JSON_NAME(geoCoords)
BE_JSON_NAME(iModelCoords)
BE_JSON_NAME(sourceDatum)
BE_JSON_NAME(targetDatum)
// unused - BE_JSON_NAME(p) // point - used in array so kept very short.
// unused - BE_JSON_NAME(s) // status - used in array so kept short


//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                  06/17
//---------------------------------------------------------------------------------------
static Utf8CP modifierToString(ECN::ECClassModifier m)
    {
    switch (m)
        {
        case ECN::ECClassModifier::Sealed: return "Sealed";
        case ECN::ECClassModifier::Abstract: return "Abstract";
        }
    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                  06/17
//---------------------------------------------------------------------------------------
static void getPrimitiveECPropertyMetaData(JsonValueR json, ECN::PrimitiveECPropertyCR prop)
    {
    json[json_primitiveType()] = prop.GetType();
    SET_IF_NOT_EMPTY_STR(json[json_extendedType()], prop.GetExtendedTypeName());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                  06/17
//---------------------------------------------------------------------------------------
static void getNavigationECPropertyMetaData(JsonValueR json, ECN::NavigationECPropertyCR navProp)
    {
    json[json_direction()] = (ECN::ECRelatedInstanceDirection::Forward == navProp.GetDirection())? "Forward": "Backward";
    auto rc = navProp.GetRelationshipClass();
    if (nullptr != rc)
        json[json_relationshipClass()] = rc->GetFullName();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                  06/17
//---------------------------------------------------------------------------------------
static void getStructECPropertyMetaData(JsonValueR json, ECN::StructECPropertyCR structProp)
    {
    json[json_structName()] = structProp.GetType().GetFullName();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                  06/17
//---------------------------------------------------------------------------------------
static void getArrayECPropertyMetaData(JsonValueR json, ECN::ArrayECPropertyCR arrayProp)
    {
    json[json_minOccurs()] = arrayProp.GetMinOccurs();
    if (!arrayProp.IsStoredMaxOccursUnbounded())
        json[json_maxOccurs()] = arrayProp.GetStoredMaxOccurs();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                  06/17
//---------------------------------------------------------------------------------------
static void getPrimitiveArrayECPropertyMetaData(JsonValueR json, ECN::PrimitiveArrayECPropertyCR arrayProp)
    {
    getArrayECPropertyMetaData(json, arrayProp);
    json[json_primitiveType()] = arrayProp.GetPrimitiveElementType();
    SET_IF_NOT_EMPTY_STR(json[json_extendedType()], arrayProp.GetExtendedTypeName());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                  06/17
//---------------------------------------------------------------------------------------
static void getStructArrayECPropertyMetaData(JsonValueR json, ECN::StructArrayECPropertyCR arrayProp)
    {
    json[json_structName()] = arrayProp.GetStructElementType().GetFullName();
    getArrayECPropertyMetaData(json, arrayProp);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                  06/17
//---------------------------------------------------------------------------------------
static void customAttributesToJson(JsonValueR jcas, ECN::IECCustomAttributeContainerCR cas)
    {
    for (auto ca : cas.GetCustomAttributes(false))
        {
        Json::Value jca(Json::objectValue);
        ECN::ECValuesCollection caProps(*ca);
        JsInterop::GetECValuesCollectionAsJson(jca[json_properties()], caProps);
        jca[json_ecclass()] = ca->GetClass().GetFullName();
        jcas.append(jca);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                  06/17
//---------------------------------------------------------------------------------------
static void doubleOrIntToJson(JsonValueR json, ECN::ECValueR v)
    {
    if (v.IsNull())
        {
        json = Json::nullValue;
        return;
        }
    
    auto primType = v.GetPrimitiveType();
    if (primType == PrimitiveType::PRIMITIVETYPE_Double)
        json = v.GetDouble();
    else if (primType == PrimitiveType::PRIMITIVETYPE_Long)
        json = Json::Value(v.GetLong());
    else
        json = v.GetInteger();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                  06/17
//---------------------------------------------------------------------------------------
DgnDbStatus JsInterop::GetECClassMetaData(JsonValueR mjson, DgnDbR dgndb, Utf8CP ecSchemaName, Utf8CP ecClassName)
    {
    auto ecclass = dgndb.Schemas().GetClass(ecSchemaName, ecClassName, SchemaLookupMode::AutoDetect);
    if (nullptr == ecclass)
        return DgnDbStatus::NotFound;    // This is not an exception. It just returns an empty result.

    if (ecclass->Is(dgndb.Schemas().GetClass(BIS_ECSCHEMA_NAME, BIS_CLASS_Element)))
        {
        dgn_ElementHandler::Element::FindHandler(dgndb, DgnClassId(ecclass->GetId().GetValue()));
        }

    mjson[json_ecclass()] = ecclass->GetFullName();
    SET_IF_NOT_EMPTY_STR(mjson[json_description()], ecclass->GetDescription());
    SET_IF_NOT_NULL_STR (mjson[json_modifier()], modifierToString(ecclass->GetClassModifier()));
    SET_IF_NOT_EMPTY_STR(mjson[json_displayLabel()], ecclass->GetDisplayLabel());

    auto& basesjson = mjson[json_baseClasses()] = Json::arrayValue;
    for (auto base: ecclass->GetBaseClasses())
        basesjson.append(base->GetFullName());

    customAttributesToJson(mjson[json_customAttributes()], *ecclass);

    auto& propsjson = mjson[json_properties()] = Json::objectValue;

    auto customHandledProperty = dgndb.Schemas().GetClass(BIS_ECSCHEMA_NAME, "CustomHandledProperty");

    for (auto ecprop : ecclass->GetProperties(false))
        {
        Utf8String pname(ecprop->GetName());
        pname[0] = static_cast<Utf8Char>(tolower(pname[0]));
        auto& propjson = propsjson[pname];

        SET_IF_NOT_EMPTY_STR(propjson[json_description()], ecprop->GetDescription());
        SET_IF_NOT_EMPTY_STR(propjson[json_displayLabel()], ecprop->GetDisplayLabel());
        ECN::ECValue v;
        if (ECN::ECObjectsStatus::Success == ecprop->GetMinimumValue(v))
            doubleOrIntToJson(propjson[json_minimumValue()], v);
        if (ECN::ECObjectsStatus::Success == ecprop->GetMaximumValue(v))
            doubleOrIntToJson(propjson[json_maximumValue()], v);
        if (ecprop->IsMaximumLengthDefined())
            propjson[json_maximumLength()] = ecprop->GetMaximumLength();
        if (ecprop->IsMinimumLengthDefined())
            propjson[json_minimumLength()] = ecprop->GetMinimumLength();
        if (ecprop->GetIsReadOnly())
            propjson[json_readOnly()] = true;
        // ###TODO: GetKindOfQuantity does not return a string. KOQ has several types of 'name'. Which do you want? SET_IF_NOT_NULL_STR(propjson[json_kindOfQuantity()], ecprop->GetKindOfQuantity());

        bool isCA;
        propjson[json_isCustomHandled()] = (isCA = ecprop->IsDefined(*customHandledProperty));
        propjson[json_isCustomHandledOrphan()] = isCA && AutoHandledPropertiesCollection::IsOrphanCustomHandledProperty(*ecprop);
        customAttributesToJson(propjson[json_customAttributes()], *ecprop);

        if (ecprop->GetAsPrimitiveProperty())
            getPrimitiveECPropertyMetaData(propjson, *ecprop->GetAsPrimitiveProperty());
        else if (ecprop->GetAsNavigationProperty())
            getNavigationECPropertyMetaData(propjson, *ecprop->GetAsNavigationProperty());
        else if (ecprop->GetAsStructProperty())
            getStructECPropertyMetaData(propjson, *ecprop->GetAsStructProperty());
        else if (ecprop->GetAsPrimitiveArrayProperty())
            getPrimitiveArrayECPropertyMetaData(propjson, *ecprop->GetAsPrimitiveArrayProperty());
        else if (ecprop->GetAsStructArrayProperty())
            getStructArrayECPropertyMetaData(propjson, *ecprop->GetAsStructArrayProperty());
        }

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Robert.Schili                  05/18
//---------------------------------------------------------------------------------------
DgnDbStatus JsInterop::GetSchemaItem(JsonValueR mjson, DgnDbR dgndb, Utf8CP schemaName, Utf8CP itemName)
    {
    const auto& schemaManager = dgndb.Schemas();
    /*auto schema = schemaManager.GetSchema(schemaName, true);
    if (nullptr == schema)
        return DgnDbStatus::NotFound;    // This is not an exception. It just returns an empty result.*/

    const auto ecClass = schemaManager.GetClass(schemaName, itemName);
    if (nullptr != ecClass)
        {
        auto nonConstWorkaround = const_cast<ECClassP>(ecClass);
        auto result = nonConstWorkaround->ToJson(mjson, true, true);
        if(result == true)
            return DgnDbStatus::Success;
        else
            return DgnDbStatus::NotFound; //TODO: is there a better status? Item was found, but failed to serialize
        }

    const auto kindOfQuantity = schemaManager.GetKindOfQuantity(schemaName, itemName);
    if (nullptr != kindOfQuantity)
        {
        auto result = kindOfQuantity->ToJson(mjson, true);
        if (result == true)
            return DgnDbStatus::Success;
        else
            return DgnDbStatus::NotFound; //TODO: is there a better status? Item was found, but failed to serialize
        }

    const auto enumeration = schemaManager.GetEnumeration(schemaName, itemName);
    if (nullptr != enumeration)
        {
        auto result = enumeration->ToJson(mjson, true);
        if (result == true)
            return DgnDbStatus::Success;
        else
            return DgnDbStatus::NotFound; //TODO: is there a better status? Item was found, but failed to serialize
        }

    const auto category = schemaManager.GetPropertyCategory(schemaName, itemName);
    if (nullptr != category)
        {
        auto result = category->ToJson(mjson, true);
        if (result == true)
            return DgnDbStatus::Success;
        else
            return DgnDbStatus::NotFound; //TODO: is there a better status? Item was found, but failed to serialize
        }

    return DgnDbStatus::NotFound;    // This is not an exception. It just returns an empty result.
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Robert.Schili                  05/18
//---------------------------------------------------------------------------------------
DgnDbStatus JsInterop::GetSchema(JsonValueR mjson, DgnDbR dgndb, Utf8CP name)
    {
    auto schema = dgndb.Schemas().GetSchema(name, true);
    if (nullptr == schema)
        return DgnDbStatus::NotFound;    // This is not an exception. It just returns an empty result.

    auto result = schema->WriteToJsonValue(mjson);
    if (result == true)
        return DgnDbStatus::Success;

    return DgnDbStatus::NotFound; //TODO: is there a better status? Item was found, but failed to serialize
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                  06/17
//---------------------------------------------------------------------------------------
DgnDbStatus JsInterop::GetElement(JsonValueR elementJson, DgnDbR dgndb, JsonValueCR inOpts)
    {
    DgnElementCPtr elem;
    DgnElementId eid(inOpts[json_id()].asUInt64());

    if (!eid.IsValid())
        {
        if (inOpts.isMember(json_federationGuid()))
            {
            BeGuid federationGuid;
            federationGuid.FromString(inOpts[json_federationGuid()].asCString());
            elem = dgndb.Elements().QueryElementByFederationGuid(federationGuid);
            }
        else
            {
            eid =dgndb.Elements().QueryElementIdByCode(DgnCode::FromJson2(inOpts[json_code()]));
            }
        }

    if (!elem.IsValid())
        elem = dgndb.Elements().GetElement(eid);

    if (!elem.IsValid())
        return DgnDbStatus::NotFound;

    elementJson = elem->ToJson(inOpts);
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus JsInterop::InsertElement(JsonValueR outJson, DgnDbR dgndb, JsonValueR inJson)
    {
    DgnElement::CreateParams params(dgndb, inJson);
    if (!params.m_classId.IsValid())
        return DgnDbStatus::WrongClass;

    ElementHandlerP elHandler = dgn_ElementHandler::Element::FindHandler(dgndb, params.m_classId);
    if (nullptr == elHandler)
        {
        BeAssert(false);
        return DgnDbStatus::WrongClass;
        }

    DgnElementPtr el = elHandler->Create(params);
    if (!el.IsValid())
        {
        BeAssert(false);
        return DgnDbStatus::BadArg;
        }

    el->FromJson(inJson);

    DgnDbStatus status;
    auto newEl = el->Insert(&status);
    if (newEl.IsValid())
        outJson[json_id()] = newEl->GetElementId().ToHexStr();

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      09/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus JsInterop::UpdateElement(DgnDbR dgndb, JsonValueR inJson)
    {
    if (!inJson.isMember(DgnElement::json_id()))
        return DgnDbStatus::BadArg;

    auto idJsonVal = inJson[DgnElement::json_id()];
    DgnElementId eid(BeInt64Id::FromString(idJsonVal.asCString()).GetValue());
    if (!eid.IsValid())
        return DgnDbStatus::InvalidId;

    DgnElementCPtr elPersist = dgndb.Elements().GetElement(eid);
    if (!elPersist.IsValid())
        return DgnDbStatus::MissingId;

    auto el = elPersist->CopyForEdit();
    el->FromJson(inJson);

    DgnDbStatus status;
    el->Update(&status);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Nate.Rex                        01/18
+---------------+---------------+---------------+---------------+---------------+------*/
void JsInterop::UpdateProjectExtents(DgnDbR dgndb, JsonValueCR newExtents)
	{
	auto& geolocation = dgndb.GeoLocation();
	AxisAlignedBox3d extents;
	extents.FromJson(newExtents);
	geolocation.SetProjectExtents(extents);
    geolocation.Save();
	}

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                    Keith.Bentley                    06/18
+---------------+---------------+---------------+---------------+---------------+------*/
void JsInterop::UpdateIModelProps(DgnDbR dgndb, JsonValueCR props)
    {
    auto& geolocation = dgndb.GeoLocation();
    if (props.isMember(json_projectExtents()))
        {
        AxisAlignedBox3d extents;
	    extents.FromJson(props[json_projectExtents()]);
	    geolocation.SetProjectExtents(extents);
        }
    if (props.isMember(json_globalOrigin()))
        geolocation.SetGlobalOrigin(JsonUtils::ToDPoint3d(props[json_globalOrigin()]));

    if (props.isMember(json_ecefLocation()))
        {
        EcefLocation ecef;
        ecef.FromJson(props[json_ecefLocation()]);
        geolocation.SetEcefLocation(ecef);
        }

    geolocation.Save();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      09/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus JsInterop::DeleteElement(DgnDbR dgndb, Utf8StringCR eidStr)
    {
    DgnElementId eid(BeInt64Id::FromString(eidStr.c_str()).GetValue());
    if (!eid.IsValid())
        return DgnDbStatus::InvalidId;

    DgnElementCPtr elPersist = dgndb.Elements().GetElement(eid);
    if (!elPersist.IsValid())
        return DgnDbStatus::MissingId;

    return elPersist->Delete();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    09/18
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus JsInterop::InsertElementAspect(DgnDbR db, JsonValueCR aspectProps)
    {
    DgnElement::RelatedElement relatedElement;
    relatedElement.FromJson(db, aspectProps[json_element()]);
    if (!relatedElement.IsValid())
        return DgnDbStatus::InvalidId;

    DgnElementCPtr element = db.Elements().GetElement(relatedElement.m_id);
    if (!element.IsValid())
        return DgnDbStatus::MissingId;

    DgnClassId aspectClassId = ECJsonUtilities::GetClassIdFromClassNameJson(aspectProps[DgnElement::json_classFullName()], db.GetClassLocater());
    if (!aspectClassId.IsValid())
        return DgnDbStatus::WrongClass;

    ECClassCP aspectClass = db.Schemas().GetClass(aspectClassId);
    if (nullptr == aspectClass)
        return DgnDbStatus::BadSchema;

    bool isMultiAspect = aspectClass->Is(BIS_ECSCHEMA_NAME, BIS_CLASS_ElementMultiAspect);

    StandaloneECInstancePtr aspect = aspectClass->GetDefaultStandaloneEnabler()->CreateInstance();
    if (!aspect.IsValid())
        return DgnDbStatus::BadRequest;

    std::function<bool(Utf8CP)> shouldConvertProperty = [aspectClass](Utf8CP propName)
        {
        if (0 == strcmp(propName, DgnElement::json_classFullName())) return false;
        return nullptr != aspectClass->GetPropertyP(propName);
        };
    if (BentleyStatus::SUCCESS != ECN::JsonECInstanceConverter::JsonToECInstance(*aspect, aspectProps, db.GetClassLocater(), shouldConvertProperty))
        return DgnDbStatus::BadRequest;

    DgnElementPtr elementEdit = element->CopyForEdit();
    if (!elementEdit.IsValid())
        return DgnDbStatus::WriteError;

    if (isMultiAspect)
        {
        if (DgnDbStatus::Success != DgnElement::GenericMultiAspect::AddAspect(*elementEdit, *aspect))
            return DgnDbStatus::WriteError;
        }
    else
        {
        if (DgnDbStatus::Success != DgnElement::GenericUniqueAspect::SetAspect(*elementEdit, *aspect))
            return DgnDbStatus::WriteError;
        }

    return elementEdit->Update().IsValid() ? DgnDbStatus::Success : DgnDbStatus::WriteError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    12/18
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus JsInterop::UpdateElementAspect(DgnDbR db, JsonValueCR aspectProps)
    {
    DgnElement::RelatedElement relatedElement;
    relatedElement.FromJson(db, aspectProps[json_element()]);
    if (!relatedElement.IsValid())
        return DgnDbStatus::InvalidId;

    DgnElementCPtr element = db.Elements().GetElement(relatedElement.m_id);
    if (!element.IsValid())
        return DgnDbStatus::MissingId;

    DgnClassId aspectClassId = ECJsonUtilities::GetClassIdFromClassNameJson(aspectProps[DgnElement::json_classFullName()], db.GetClassLocater());
    if (!aspectClassId.IsValid())
        return DgnDbStatus::WrongClass;

    ECClassCP aspectClass = db.Schemas().GetClass(aspectClassId);
    if (nullptr == aspectClass)
        return DgnDbStatus::BadSchema;

    DgnElementPtr elementEdit = element->CopyForEdit();
    if (!elementEdit.IsValid())
        return DgnDbStatus::WriteError;

    IECInstanceP aspect;
    bool isMultiAspect = aspectClass->Is(BIS_ECSCHEMA_NAME, BIS_CLASS_ElementMultiAspect);
    if (isMultiAspect)
        {
        ECInstanceId aspectId(BeInt64Id::FromString(aspectProps[DgnElement::json_id()].asCString()).GetValue());
        aspect = DgnElement::GenericMultiAspect::GetAspectP(*elementEdit, *aspectClass, aspectId);
        }
    else
        {
        aspect = DgnElement::GenericUniqueAspect::GetAspectP(*elementEdit, *aspectClass);
        }

    if (nullptr == aspect)
        return DgnDbStatus::NotFound;

    std::function<bool(Utf8CP)> shouldConvertProperty = [aspectClass](Utf8CP propName)
        {
        if ((0 == strcmp(propName, DgnElement::json_classFullName())) || (0 == strcmp(propName, json_element()))) return false;
        return nullptr != aspectClass->GetPropertyP(propName);
        };
    if (BentleyStatus::SUCCESS != ECN::JsonECInstanceConverter::JsonToECInstance(*aspect, aspectProps, db.GetClassLocater(), shouldConvertProperty))
        return DgnDbStatus::BadRequest;

    return elementEdit->Update().IsValid() ? DgnDbStatus::Success : DgnDbStatus::WriteError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    09/18
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus JsInterop::DeleteElementAspect(DgnDbR db, Utf8StringCR aspectIdStr)
    {
    ECInstanceId aspectId(BeInt64Id::FromString(aspectIdStr.c_str()).GetValue());
    if (!aspectId.IsValid())
        return DgnDbStatus::InvalidId;

    CachedECSqlStatementPtr statement = db.GetPreparedECSqlStatement(
        "SELECT ECClassId,Element.Id FROM " BIS_SCHEMA(BIS_CLASS_ElementUniqueAspect) " WHERE ECInstanceId=? UNION "
        "SELECT ECClassId,Element.Id FROM " BIS_SCHEMA(BIS_CLASS_ElementMultiAspect)  " WHERE ECInstanceId=?");
    if (!statement.IsValid())
        return DgnDbStatus::SQLiteError;

    statement->BindId(1, aspectId);
    statement->BindId(2, aspectId);
    if (BE_SQLITE_ROW != statement->Step())
        return DgnDbStatus::NotFound;

    DgnClassId aspectClassId = statement->GetValueId<DgnClassId>(0);
    DgnElementId elementId = statement->GetValueId<DgnElementId>(1);

    ECClassCP aspectClass = db.Schemas().GetClass(aspectClassId);
    if (nullptr == aspectClass)
        return DgnDbStatus::BadSchema;

    bool isMultiAspect = aspectClass->Is(BIS_ECSCHEMA_NAME, BIS_CLASS_ElementMultiAspect);

    DgnElementCPtr element = db.Elements().GetElement(elementId);
    if (!element.IsValid())
        return DgnDbStatus::MissingId;

    DgnElementPtr elementEdit = element->CopyForEdit();
    if (!elementEdit.IsValid())
        return DgnDbStatus::WriteError;

    if (isMultiAspect)
        {
        DgnElement::MultiAspect* aspect = DgnElement::MultiAspect::GetAspectP(*elementEdit, *aspectClass, aspectId);
        if (nullptr == aspect)
            return DgnDbStatus::NotFound;

        aspect->Delete();
        }
    else
        {
        DgnElement::UniqueAspect* aspect = DgnElement::UniqueAspect::GetAspectP(*elementEdit, *aspectClass);
        if (nullptr == aspect)
            return DgnDbStatus::NotFound;

        aspect->Delete();
        }

    return elementEdit->Update().IsValid() ? DgnDbStatus::Success : DgnDbStatus::WriteError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/17
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::ECRelationshipClassCP parseRelClass(DgnDbR dgndb, JsonValueCR inJson)
    {
    auto relClassId = ECJsonUtilities::GetClassIdFromClassNameJson(inJson[DgnElement::json_classFullName()], dgndb.GetClassLocater());
    if (!relClassId.IsValid())
        return nullptr;

    auto ecClass = dgndb.Schemas().GetClass(relClassId);
    if (nullptr == ecClass)
        return nullptr;

    return ecClass->GetRelationshipClassCP();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/17
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::EC::ECInstanceKey parseECRelationshipInstanceKeyKey(DgnDbR dgndb, JsonValueCR inJson)
    {
    auto relClass = parseRelClass(dgndb, inJson);
    if (nullptr == relClass)
        return BeSQLite::EC::ECInstanceKey();

    DgnElementId relId;
    relId.FromJson(inJson["id"]);

    return BeSQLite::EC::ECInstanceKey(relClass->GetId(), ECInstanceId(relId.GetValue()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      09/17
+---------------+---------------+---------------+---------------+---------------+------*/
static ECN::StandaloneECRelationshipInstancePtr getRelationshipProperties(ECN::ECRelationshipClassCP relClass, JsonValueR inJson, DgnDbR dgndb)
    {
    ECN::StandaloneECRelationshipEnablerPtr relationshipEnabler = ECN::StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler (*relClass);
    auto relationshipInstance = relationshipEnabler->CreateRelationshipInstance ();
    ECN::IECInstanceR instance = *relationshipInstance;
    IECClassLocaterR classLocater = dgndb.GetClassLocater();
    bool hasProperties = false;

    static auto const isDefinitelyNotAutoHandled = [](Utf8CP propName) -> bool
        {
        // *** NEEDS WORK:  We have defined a bunch of special properties in our element wire format
        // ***              That are not in the biscore ECSchema. So, we have no way of checking
        // ***              if they are auto-handled or not using metadata. Only some _FromJson method somewhere
        // ***              knows what these properties are. Here, we check for the few special properties
        // ***              that we know about. Similar Workaround as in (see DgnElement.cpp around Line 1300)
        switch (propName[0])
            {
            case 'i': return 0==strcmp(propName, "id");
            case 'c': return 0==strcmp(propName, "classFullName") || 0==strcmp(propName, "code");
            case 'p': return 0==strcmp(propName, "placement");
            }
        return false;
        };

    std::function<bool(Utf8CP)> shouldConvertProperty = [&relClass, &hasProperties](Utf8CP propName)
        {
        if (isDefinitelyNotAutoHandled(propName))
            return false;

        if (ECJsonSystemNames::IsTopLevelSystemMember(propName))
            return false;

        ECPropertyP ecprop = relClass->GetPropertyP(propName);            // will be null for system properties, such as sourceId, targetId, classFullName, id
        if (nullptr == ecprop)
            return false; // TODO: support other property types

        hasProperties = true;
        return true;
        };

    JsonECInstanceConverter::JsonToECInstance(instance, inJson, classLocater, shouldConvertProperty);

    return hasProperties ? relationshipInstance : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/17
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult JsInterop::InsertLinkTableRelationship(JsonValueR outJson, DgnDbR dgndb, JsonValueR inJson)
    {
    auto relClass = parseRelClass(dgndb, inJson);
    if (nullptr == relClass)
        return BE_SQLITE_ERROR;

    DgnElementId sourceId, targetId;
    sourceId.FromJson(inJson["sourceId"]);
    targetId.FromJson(inJson["targetId"]);

    ECN::StandaloneECRelationshipInstancePtr props = getRelationshipProperties(relClass, inJson, dgndb);

    BeSQLite::EC::ECInstanceKey relKey;
    auto rc = dgndb.InsertLinkTableRelationship(relKey, *relClass, sourceId, targetId, props.get()); // nullptr is okay if there are no props
    if (BE_SQLITE_OK != rc)
        return rc;

    outJson = relKey.GetInstanceId().ToHexStr();

    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      09/17
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult JsInterop::UpdateLinkTableRelationship(DgnDbR dgndb, JsonValueR inJson)
    {
    BeSQLite::EC::ECInstanceKey relKey = parseECRelationshipInstanceKeyKey(dgndb, inJson);
    auto relClass = parseRelClass(dgndb, inJson);
    if (nullptr == relClass)
        return BE_SQLITE_NOTFOUND;

    ECN::StandaloneECRelationshipInstancePtr props = getRelationshipProperties(relClass, inJson, dgndb);
    if (!props.IsValid())
        return BE_SQLITE_OK; // Relationship class had no properties - consider update successful

    return dgndb.UpdateLinkTableRelationshipProperties(relKey, *props);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      09/17
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult JsInterop::DeleteLinkTableRelationship(DgnDbR dgndb, Json::Value& inJson)
    {
    BeSQLite::EC::ECInstanceKey relKey = parseECRelationshipInstanceKeyKey(dgndb, inJson);
    return dgndb.DeleteLinkTableRelationship(relKey);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus JsInterop::InsertCodeSpec(Utf8StringR idStr, DgnDbR db, Utf8StringCR name, JsonValueCR jsonProperties)
    {
    CodeSpecPtr codeSpec = CodeSpec::Create(db, name.c_str(), jsonProperties);
    if (!codeSpec.IsValid())
        return DgnDbStatus::BadRequest;
    DgnDbStatus status = codeSpec->Insert();
    if (DgnDbStatus::Success != status)
        return status;
    idStr = codeSpec->GetCodeSpecId().ToHexStr();
    return DgnDbStatus::Success;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus JsInterop::InsertModel(JsonValueR outJson, DgnDbR dgndb, JsonValueR inJson) {
    DgnModel::CreateParams params(dgndb, inJson);
    if (!params.m_classId.IsValid())
        return DgnDbStatus::WrongClass;

    ModelHandlerP handler = dgn_ModelHandler::Model::FindHandler(dgndb, params.m_classId);
    if (nullptr == handler) {
        BeAssert(false);
        return DgnDbStatus::WrongClass;
    }

    DgnModelPtr model = handler->Create(params);
    if (!model.IsValid()) {
        BeAssert(false);
        return DgnDbStatus::BadArg;
    }

    model->FromJson(inJson);

    DgnDbStatus status = model->Insert();
    outJson[json_id()] = model->GetModelId().ToHexStr();
    return status;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      09/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus JsInterop::UpdateModel(DgnDbR dgndb, JsonValueR inJson) {
    if (!inJson.isMember(DgnModel::json_id()))
        return DgnDbStatus::BadArg;

    auto idJsonVal = inJson[DgnModel::json_id()];
    DgnModelId mid(BeInt64Id::FromString(idJsonVal.asCString()).GetValue());
    if (!mid.IsValid())
        return DgnDbStatus::InvalidId;

    DgnModelPtr model = dgndb.Models().GetModel(mid);
    if (!model.IsValid())
        return DgnDbStatus::MissingId;

    if (model->IsGeometricModel() && inJson.isMember("geometryChanged")) {
        auto stmt = dgndb.GetGeometricModelUpdateStatement();
        if (stmt.IsValid()) { // has the db been upgraded?
            stmt->BindGuid(1, BeGuid(true));
            stmt->BindId(2, mid);
            stmt->Step();
        }
    } else if (inJson.isMember("updateLastMod")) {
        auto stmt = dgndb.GetModelLastModUpdateStatement();
        if (stmt.IsValid()) {
            stmt->BindId(1, mid);
            stmt->Step();
        }
    }

    model->FromJson(inJson);
    return model->Update();
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                    Sam.Wilson                      09/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus JsInterop::DeleteModel(DgnDbR dgndb, Utf8StringCR midStr) {
    DgnModelId mid(BeInt64Id::FromString(midStr.c_str()).GetValue());
    if (!mid.IsValid())
        return DgnDbStatus::InvalidId;

    DgnModelPtr model = dgndb.Models().GetModel(mid);
    if (!model.IsValid())
        return DgnDbStatus::MissingId;

    return model->Delete();
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                    Keith.Bentley                   07/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus JsInterop::GetModel(JsonValueR modelJson, DgnDbR dgndb, JsonValueCR inOpts) {
    DgnModelId modelId(inOpts[json_id()].asUInt64());
    if (!modelId.IsValid()) {
        auto codeVal = inOpts[json_code()];
        if (!codeVal)
            return DgnDbStatus::NotFound;

        modelId = dgndb.Models().QuerySubModelId(DgnCode::FromJson2(codeVal));
    }

    //  Look up the model
    auto model = dgndb.Models().GetModel(modelId);
    if (!model.IsValid())
        return DgnDbStatus::NotFound;

    modelJson = model->ToJson(inOpts);
    // Note: there are no auto-handled properties on DgnModels. Any such data goes on the modeled element.
    return DgnDbStatus::Success;
}

//---------------------------------------------------------------------------------------
// @bsimethod                               Shaun.Sewall                  07/18
//---------------------------------------------------------------------------------------
DgnDbStatus JsInterop::QueryModelExtents(JsonValueR extentsJson, DgnDbR db, JsonValueCR options) {
    DgnModelId modelId(options[json_id()].asUInt64());
    if (!modelId.IsValid())
        return DgnDbStatus::InvalidId;

    DgnModelPtr model = db.Models().GetModel(modelId);
    if (!model.IsValid())
        return DgnDbStatus::NotFound;

    GeometricModelCP geometricModel = model->ToGeometricModel();
    if (!geometricModel)
        return DgnDbStatus::WrongModel;

    AxisAlignedBox3d extents = geometricModel->QueryElementsRange();
    if (!extents.IsValid())
        return DgnDbStatus::NoGeometry;

    extents.ToJson(extentsJson[json_modelExtents()]);
    return DgnDbStatus::Success;
}

//---------------------------------------------------------------------------------------
// @bsimethod                               Keith.Bentley                 12/17
//---------------------------------------------------------------------------------------
void JsInterop::GetIModelProps(JsonValueR val, DgnDbCR dgndb) {
    // add the root subject, if available.
    auto rootSubject = dgndb.Elements().GetRootSubject();
    if (rootSubject.IsValid()) {
        auto& subject = val[json_rootSubject()];
        subject[json_name()] = rootSubject->GetCode().GetValueUtf8CP();
        auto descr = rootSubject->GetDescription();
        if (!descr.empty())
            subject[json_description()] = descr;
    }

    auto& geolocation = dgndb.GeoLocation();

    // add project extents
    auto extents = geolocation.GetProjectExtents();
    extents.ToJson(val[json_projectExtents()]);

    // add global origin
    val[json_globalOrigin()] = JsonUtils::DPoint3dToJson(geolocation.GetGlobalOrigin());

    auto ecefLocation = geolocation.GetEcefLocation();
    if (ecefLocation.m_isValid)
        val[json_ecefLocation()] = ecefLocation.ToJson();
}

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   02/20
//=======================================================================================
struct CachedDatumConverter
{
private:
    GeoCoordinates::DatumConverterP m_converter = nullptr;
    GeoCoordinates::DatumCP         m_sourceDatum = nullptr;
    GeoCoordinates::DatumCP         m_targetDatum = nullptr;
    WString                         m_sourceName;
    WString                         m_targetName;

    void Reset()
        {
        m_sourceDatum = m_targetDatum = nullptr;
        m_converter = nullptr;
        }

    void MoveFrom(CachedDatumConverter&& src)
        {
        m_converter = src.m_converter;
        m_sourceDatum = src.m_sourceDatum;
        m_targetDatum = src.m_targetDatum;
        m_sourceName = src.m_sourceName;
        m_targetName = src.m_targetName;

        src.Reset();
        }
public:
    CachedDatumConverter() = delete;
    CachedDatumConverter& operator=(CachedDatumConverter const& src) = delete;

    CachedDatumConverter(CachedDatumConverter&& src)
        {
        MoveFrom(std::move(src));
        }

    CachedDatumConverter& operator=(CachedDatumConverter&& src)
        {
        MoveFrom(std::move(src));
        return *this;
        }

    CachedDatumConverter(WStringCR sourceName, WStringCR targetName) : m_sourceName(sourceName), m_targetName(targetName)
        {
        auto source = GeoCoordinates::Datum::CreateDatum(sourceName.c_str());
        if (nullptr == source)
            return;

        auto target = GeoCoordinates::Datum::CreateDatum(targetName.c_str());
        if (nullptr == target)
            {
            source->Destroy();
            return;
            }

        auto converter = GeoCoordinates::DatumConverter::Create(*source, *target);
        if (nullptr == converter)
            {
            source->Destroy();
            target->Destroy();
            return;
            }

        m_sourceDatum = source;
        m_targetDatum = target;
        m_converter = converter;
        }

    ~CachedDatumConverter()
        {
        if (nullptr == m_converter)
            return;

        m_sourceDatum->Destroy();
        m_targetDatum->Destroy();
        m_converter->Destroy();

        Reset();
        }

    bool Matches(WCharCP sourceName, WCharCP targetName) const
        {
        return m_sourceName.Equals(sourceName) && m_targetName.Equals(targetName);
        }

    GeoCoordinates::DatumConverterP GetConverter() const { return m_converter; }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   02/20
//=======================================================================================
struct DatumConverterCache : DgnDb::AppData
{
private:
    // This vector will typically hold 1 or 2 converters between WGS84 and the iModel's GCS.
    std::vector<CachedDatumConverter>   m_converters;

    static DatumConverterCache& Get(DgnDbR db)
        {
        static Key s_key;
        return *db.ObtainAppData(s_key, []() { return new DatumConverterCache(); });
        }

    GeoCoordinates::DatumConverterP GetConverter(WStringCR sourceDatumName, WStringCR targetDatumName)
        {
        auto iter = std::find_if(m_converters.begin(), m_converters.end(), [&](CachedDatumConverter& cvtr) { return cvtr.Matches(sourceDatumName.c_str(), targetDatumName.c_str()); });
        if (m_converters.end() != iter)
            return iter->GetConverter();

        m_converters.emplace_back(sourceDatumName, targetDatumName);

        // In some cases the converter returned is null possibly due to misconfiguration of grid shift files
        // So don't cache unless we get a non-null converter.
        auto converter = m_converters.back().GetConverter();
        if (nullptr == converter)
            m_converters.pop_back();

        return converter;
        }

    static WString GetRequiredDatumName(Utf8String requestedDatumName, DgnDbR db)
        {
        requestedDatumName.DropQuotes();
        if (requestedDatumName.empty())
            return WString();

        auto gcs = db.GeoLocation().GetDgnGCS();
        if (nullptr == gcs || !gcs->IsValid())
            return WString();

        WCharCP iModelDatumName = gcs->GetDatumName();
        WString requestedDatumNameW(requestedDatumName.c_str(), true);
        if (requestedDatumNameW.Equals(iModelDatumName))
            return WString();

        return requestedDatumNameW;
        }
public:
    static GeoCoordinates::DatumConverterP GetConverterFromIModelCoords(Utf8StringCR targetDatumName, DgnDbR db)
        {
        auto targetDatumNameW = GetRequiredDatumName(targetDatumName, db);
        if (targetDatumNameW.empty())
            return nullptr;

        return Get(db).GetConverter(db.GeoLocation().GetDgnGCS()->GetDatumName(), targetDatumNameW);
        }

    static GeoCoordinates::DatumConverterP GetConverterToIModelCoords(Utf8StringCR sourceDatumName, DgnDbR db)
        {
        auto sourceDatumNameW = GetRequiredDatumName(sourceDatumName, db);
        if (sourceDatumNameW.empty())
            return nullptr;

        return Get(db).GetConverter(sourceDatumNameW, db.GeoLocation().GetDgnGCS()->GetDatumName());
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/20
+---------------+---------------+---------------+---------------+---------------+------*/
static void populateGeoCoordResult(JsonValueR result, bvector<DPoint3d> const& points, bvector<ReprojectStatus> const& statuses)
    {
    for (size_t i = 0; i < points.size(); i++)
        {
        auto& outputPointWithStatus = result[static_cast<Json::ArrayIndex>(i)];
        auto& outputPoint = outputPointWithStatus["p"];

        outputPoint[0] = points[i].x;
        outputPoint[1] = points[i].y;
        outputPoint[2] = points[i].z;

        outputPointWithStatus["s"] = statuses[i];
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Barry.Bentley                 12/18
//---------------------------------------------------------------------------------------
BentleyStatus JsInterop::GetGeoCoordsFromIModelCoords(JsonValueR results, DgnDbR dgnDb, JsonValueCR props)
    {
    // get the vector of points.
    bvector<DPoint3d> iModelPoints;
    JsonUtils::DPoint3dVectorFromJson(iModelPoints, props[json_iModelCoords()]);

    // create return vectors.
    bvector<DPoint3d> geoPoints(iModelPoints.size());
    bvector<ReprojectStatus> statusList(geoPoints.size());

    Utf8String targetDatumName = props[json_targetDatum()].ToString();
    auto datumConverter = DatumConverterCache::GetConverterFromIModelCoords(targetDatumName, dgnDb);
    auto gcs = dgnDb.GeoLocation().GetDgnGCS();

    auto outputStatus = statusList.begin();
    for (auto input = iModelPoints.begin(), output = geoPoints.begin(); input != iModelPoints.end(); input++, output++, outputStatus++)
        {
        if (nullptr != gcs && gcs->IsValid())
            {
            GeoPoint tempPoint;
            *outputStatus = gcs->LatLongFromUors(tempPoint, *input);

            // get the output point into the desired Datum.
            if (nullptr != datumConverter)
                datumConverter->ConvertLatLong3D((GeoPointR)*output, tempPoint);
            else
                (GeoPointR) *output = tempPoint;
            }
        else
            {
            *outputStatus = ReprojectStatus::REPROJECT_BadArgument;
            }
        }

    // Put the results (a point named p and a status named s) into a Json object.
    populateGeoCoordResult(results[json_geoCoords()], geoPoints, statusList);

    return BentleyStatus::BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Barry.Bentley                 12/18
//---------------------------------------------------------------------------------------
BentleyStatus JsInterop::GetIModelCoordsFromGeoCoords (JsonValueR results, DgnDbR dgnDb, JsonValueCR props)
    {
    // get the vector of points.
    bvector<DPoint3d> geoPoints;
    JsonUtils::DPoint3dVectorFromJson (geoPoints, props[json_geoCoords()]);

    // create return vectors.
    bvector<DPoint3d> iModelPoints(geoPoints.size());
    bvector<ReprojectStatus> statusList(geoPoints.size());

    // get the GCS
    Utf8String sourceDatumName = props[json_sourceDatum()].ToString();
    auto datumConverter = DatumConverterCache::GetConverterToIModelCoords(sourceDatumName, dgnDb);
    auto gcs = dgnDb.GeoLocation().GetDgnGCS();

    auto outputStatus = statusList.begin();
    for (auto input = geoPoints.begin(), output = iModelPoints.begin(); input != geoPoints.end(); input++, output++, outputStatus++ )
        {
        if (nullptr != gcs && gcs->IsValid())
            {
            GeoPoint tempPoint;
            // get point into the correct Datum.
            if (nullptr != datumConverter)
                datumConverter->ConvertLatLong3D (tempPoint, (GeoPointCR)*input);
            else
                tempPoint = (GeoPointCR) *input;

            *outputStatus = gcs->UorsFromLatLong(*output, tempPoint);
            }
        else
            {
            *outputStatus = ReprojectStatus::REPROJECT_BadArgument;
            }
        }

    // Put the results (a point named p and a status named s) into a Json object.
    populateGeoCoordResult(results[json_iModelCoords()], iModelPoints, statusList);

    return BentleyStatus::BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/19
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementIdSet JsInterop::FindGeometryPartReferences(bvector<Utf8String> const& idArray, bool is2d, DgnDbR db)
    {
    BeSQLite::IdSet<DgnGeometryPartId> idSet;
    for (auto const& idStr : idArray)
        {
        auto id = BeInt64Id::FromString(idStr.c_str());
        if (id.IsValid())
            idSet.insert(DgnGeometryPartId(id.GetValue()));
        }

    return db.Elements().FindGeometryPartReferences(idSet, is2d);
    }
