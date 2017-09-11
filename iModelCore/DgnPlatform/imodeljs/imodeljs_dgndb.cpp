/*--------------------------------------------------------------------------------------+
|
|     $Source: imodeljs/imodeljs_dgndb.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include "imodeljs.h"
#include <DgnPlatform/DgnECPersistence.h>

#define SET_IF_NOT_EMPTY_STR(j, str) {if (!(str).empty()) j = str;}
#define SET_IF_NOT_NULL_STR(j, str) {if (nullptr != (str)) j = str;}

BE_JSON_NAME(baseClasses)
BE_JSON_NAME(code)
BE_JSON_NAME(customAttributes)
BE_JSON_NAME(description)
BE_JSON_NAME(direction)
BE_JSON_NAME(displayLabel)
BE_JSON_NAME(ecclass)
BE_JSON_NAME(extendedType)
BE_JSON_NAME(federationGuid)
BE_JSON_NAME(id)
BE_JSON_NAME(isCustomHandled)
BE_JSON_NAME(kindOfQuantity)
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
        IModelJs::GetECValuesCollectionAsJson(jca[json_properties()], caProps);
        jca[json_ecclass()] = ca->GetClass().GetFullName();
        jcas.append(jca);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                  06/17
//---------------------------------------------------------------------------------------
DgnDbStatus IModelJs::GetECClassMetaData(JsonValueR mjson, DgnDbR dgndb, Utf8CP ecSchemaName, Utf8CP ecClassName)
    {
    auto ecclass = dgndb.Schemas().GetClass(ecSchemaName, ecClassName);
    if (nullptr == ecclass)
        return DgnDbStatus::Success;    // This is not an exception. It just returns an empty result.

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
        pname[0] = tolower(pname[0]);
        auto& propjson = propsjson[pname];

        SET_IF_NOT_EMPTY_STR(propjson[json_description()], ecprop->GetDescription());
        SET_IF_NOT_EMPTY_STR(propjson[json_displayLabel()], ecprop->GetDisplayLabel());
        ECN::ECValue v;
        if (ECN::ECObjectsStatus::Success == ecprop->GetMinimumValue(v))
            ECUtils::ConvertECValueToJson(propjson[json_minimumValue()], v);
        if (ECN::ECObjectsStatus::Success == ecprop->GetMaximumValue(v))
            ECUtils::ConvertECValueToJson(propjson[json_maximumValue()], v);
        if (ecprop->IsMaximumLengthDefined())
            propjson[json_maximumLength()] = ecprop->GetMaximumLength();
        if (ecprop->IsMinimumLengthDefined())
            propjson[json_minimumLength()] = ecprop->GetMinimumLength();
        if (ecprop->GetIsReadOnly())
            propjson[json_readOnly()] = true;
        SET_IF_NOT_NULL_STR(propjson[json_kindOfQuantity()], ecprop->GetKindOfQuantity());

        propjson[json_isCustomHandled()] = ecprop->IsDefined(*customHandledProperty);
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
// @bsimethod                                   Sam.Wilson                  06/17
//---------------------------------------------------------------------------------------
DgnDbStatus IModelJs::GetElement(JsonValueR elementJson, DgnDbR dgndb, JsonValueCR inOpts)
    {
    DgnElementCPtr elem;
    DgnElementId eid(inOpts[json_id()].asUInt64());

    if (!eid.IsValid())
        {
        if (inOpts.isMember(json_federationGuid()))
            {
            BeGuid federationGuid;
            federationGuid.FromString(inOpts[json_federationGuid()].asString().c_str());
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
DgnDbStatus IModelJs::InsertElement(JsonValueR outJson, DgnDbR dgndb, JsonValueR inJson)
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

#if defined (TOFROM_JSON)
    auto ecclass = dgndb.Schemas().GetClass(ECN::ECClassId(m_classId.GetValue()));
    for (auto prop : AutoHandledPropertiesCollection(eclass, db, ECSqlClassParams::StatementType::InsertUpdate, false))
        {
        autoHandledProperties.push_back(prop);
        }
#endif

    DgnDbStatus status;
    auto newEl = el->Insert(&status);
    if (newEl.IsValid())
        outJson[json_id()] = newEl->GetElementId().ToHexStr();

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus IModelJs::GetModel(JsonValueR modelJson, DgnDbR dgndb, Json::Value const& inOpts)
    {
    DgnModelId modelId(inOpts[json_id()].asUInt64());
    if (!modelId.IsValid())
        {
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

    auto stmt = dgndb.Models().GetSelectStmt(*model);
    if (!stmt.IsValid())
        return DgnDbStatus::WrongClass;

    if (BE_SQLITE_ROW == stmt->Step())
        GetRowAsJson(modelJson, *stmt);

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus IModelJs::GetElementPropertiesForDisplay(DgnDbStatus& status, JsonValueR result, DgnDbR dgndb, Utf8CP eidstr)
    {
    DgnElementId elemId(DgnElementId::FromString(eidstr).GetValueUnchecked());
    if (!elemId.IsValid())
        {
        status = DgnDbStatus::InvalidId;
        return BSIERROR;
        }

    Json::Value jsonInstances, jsonDisplayInfo;
    if (SUCCESS != DgnECPersistence::GetElementInfo(jsonInstances, jsonDisplayInfo, elemId, dgndb))
        return BSISUCCESS;  // Don't throw an exception. Just return an empty result.

    // => [
    //      {
    //      'category':categoryLabel,
    //      'properties': [
    //          {
    //          'label':propertyLabel,
    //          'value':propertyValue
    //          },
    //          ...
    //      ],
    //      ...
    //      }
    //    ]

    result = Json::arrayValue;
    int nCategories = jsonDisplayInfo["Categories"].size();
    for (int i = 0; i < nCategories; i++)
        {
        Json::Value categoryInfo = jsonDisplayInfo["Categories"][i];
        if (categoryInfo.isNull())
            continue;

        Json::Value propertyList(Json::arrayValue);
        Json::Value properties;
        int nProperties = categoryInfo["Properties"].size();
        for (int j = 0; j < nProperties; j++)
            {
            Json::Value propertyDef = categoryInfo["Properties"][j];
            if (propertyDef["InstanceIndex"].isNull())
                continue;

            int instanceIdx = propertyDef["InstanceIndex"].asInt();
            if (jsonInstances[instanceIdx].isNull())
                continue;

            Json::Value prop(Json::objectValue);
            Utf8String name = propertyDef["Name"].asCString();
            prop["label"] = propertyDef["DisplayLabel"];
            prop["value"] = jsonInstances[instanceIdx][name];
            propertyList.append(prop);
            }

        if (propertyList.size() > 0)
            {
            Json::Value category(Json::objectValue);
            category["category"] = categoryInfo["DisplayLabel"];
            category["properties"] = propertyList;
            result.append(category);
            }
        }

#ifdef DEBUG_PROPERTIES
    printf("Element props. Raw props:");
    printf(Json::FastWriter::ToString(jsonInstances).c_str());
    printf("Raw display info:");
    printf(Json::FastWriter::ToString(jsonDisplayInfo).c_str());
    printf("Returned properties:");
    printf(Json::FastWriter::ToString(result).c_str());
#endif

    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                  06/17
//---------------------------------------------------------------------------------------
DbResult IModelJs::OpenDgnDb(DgnDbPtr& db, BeFileNameCR dbname, DgnDb::OpenMode mode)
    {
    DbResult result;
    db = IModelJs::GetDbByName(result, dbname, mode);
    return result;
    }

