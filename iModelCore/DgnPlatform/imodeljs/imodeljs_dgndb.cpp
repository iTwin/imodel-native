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

BE_JSON_NAME(id)
BE_JSON_NAME(code)

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                  06/17
//---------------------------------------------------------------------------------------
static void ecclassToJson(JsonValueR json, ECN::ECClassCR ecclass)
    {
    json["name"]   = ecclass.GetName();
    json["schema"] = ecclass.GetSchema().GetName();
    }

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
static void getPrimitiveECPropertyMetaData(JsonValueR parentJson, ECN::PrimitiveECPropertyCR prop)
    {
    auto& json = parentJson["primitiveECProperty"];
    json["type"] = prop.GetTypeName();
    SET_IF_NOT_EMPTY_STR(json["extendedType"], prop.GetExtendedTypeName());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                  06/17
//---------------------------------------------------------------------------------------
static void getNavigationECPropertyMetaData(JsonValueR parentJson, ECN::NavigationECPropertyCR navProp)
    {
    auto& json = parentJson["navigationECProperty"];
    json["type"] = navProp.GetTypeName();
    json["direction"] = (ECN::ECRelatedInstanceDirection::Forward == navProp.GetDirection())? "Forward": "Backward";
    auto rc = navProp.GetRelationshipClass();
    if (nullptr != rc)
        ecclassToJson(json["relationshipClass"], *rc);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                  06/17
//---------------------------------------------------------------------------------------
static void getStructECPropertyMetaData(JsonValueR parentJson, ECN::StructECPropertyCR structProp)
    {
    auto& json = parentJson["structECProperty"];
    json["type"] = structProp.GetTypeName();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                  06/17
//---------------------------------------------------------------------------------------
static void getArrayECPropertyMetaData(JsonValueR json, ECN::ArrayECPropertyCR arrayProp)
    {
    json["minOccurs"] = arrayProp.GetMinOccurs();
    if (!arrayProp.IsStoredMaxOccursUnbounded())
        json["maxOccurs"] = arrayProp.GetStoredMaxOccurs();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                  06/17
//---------------------------------------------------------------------------------------
static void getPrimitiveArrayECPropertyMetaData(JsonValueR parentJson, ECN::PrimitiveArrayECPropertyCR arrayProp)
    {
    auto& json = parentJson["primitiveArrayECProperty"];
    getArrayECPropertyMetaData(json, arrayProp);
    json["type"] = arrayProp.GetTypeName();
    SET_IF_NOT_EMPTY_STR(json["extendedType"], arrayProp.GetExtendedTypeName());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                  06/17
//---------------------------------------------------------------------------------------
static void getStructArrayECPropertyMetaData(JsonValueR parentJson, ECN::StructArrayECPropertyCR arrayProp)
    {
    auto& json = parentJson["structArrayECProperty"];
    getArrayECPropertyMetaData(json, arrayProp);
    json["type"] = arrayProp.GetTypeName();
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
        IModelJs::GetECValuesCollectionAsJson(jca["properties"], caProps);
        ecclassToJson(jca["ecclass"], ca->GetClass());
        jcas.append(jca);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                  06/17
//---------------------------------------------------------------------------------------
BentleyStatus IModelJs::GetECClassMetaData(DgnDbStatus& status, Utf8StringR errmsg, JsonValueR mjson, DgnDbR dgndb, Utf8CP ecSchemaName, Utf8CP ecClassName)
    {
    auto ecclass = dgndb.Schemas().GetClass(ecSchemaName, ecClassName);
    if (nullptr == ecclass)
        return BSISUCCESS;      // This is not an exception. It just returns an empty result.

    ecclassToJson(mjson, *ecclass);
    SET_IF_NOT_EMPTY_STR(mjson["description"], ecclass->GetDescription());
    SET_IF_NOT_NULL_STR (mjson["modifier"], modifierToString(ecclass->GetClassModifier()));
    SET_IF_NOT_EMPTY_STR(mjson["displayLabel"], ecclass->GetDisplayLabel());

    auto& basesjson = mjson["baseClasses"] = Json::arrayValue;
    for (auto base: ecclass->GetBaseClasses())
        {
        Json::Value basejson(Json::objectValue);
        ecclassToJson(basejson, *base);
        basesjson.append(basejson);
        }

    customAttributesToJson(mjson["customAttributes"], *ecclass);

    auto& propsjson = mjson["properties"] = Json::objectValue;

    for (auto ecprop : ecclass->GetProperties(false))
        {
        Utf8String pname(ecprop->GetName());
        pname[0] = tolower(pname[0]);
        auto& propjson = propsjson[pname];
        SET_IF_NOT_EMPTY_STR(propjson["description"], ecprop->GetDescription());
        SET_IF_NOT_EMPTY_STR(propjson["displayLabel"], ecprop->GetDisplayLabel());
        ECN::ECValue v;
        if (ECN::ECObjectsStatus::Success == ecprop->GetMinimumValue(v))
            ECUtils::ConvertECValueToJson(propjson["minimumValue"], v);
        if (ECN::ECObjectsStatus::Success == ecprop->GetMaximumValue(v))
            ECUtils::ConvertECValueToJson(propjson["maximumValue"], v);
        if (ecprop->IsMaximumLengthDefined())
            propjson["maximumLength"] = ecprop->GetMaximumLength();
        if (ecprop->IsMinimumLengthDefined())
            propjson["minimumLength"] = ecprop->GetMinimumLength();
        if (ecprop->GetIsReadOnly())
            propjson["readOnly"] = true;
        SET_IF_NOT_NULL_STR(propjson["kindOfQuantity"], ecprop->GetKindOfQuantity());
        
        customAttributesToJson(propjson["customAttributes"], *ecprop);

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

    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                  06/17
//---------------------------------------------------------------------------------------
BentleyStatus IModelJs::GetElement(DgnDbStatus& status, Utf8StringR errmsg, JsonValueR elementJson, DgnDbR dgndb, Json::Value const& inOpts)
    {
    DgnElementId eid(inOpts[json_id()].asUInt64());
    if (!eid.IsValid())
        {
        auto codeVal = inOpts[json_code()];
        if (!codeVal)
            {
            errmsg = "DgnDbStatus::NotFound";
            status = DgnDbStatus::NotFound;
            return BSIERROR;
            }
        eid = dgndb.Elements().QueryElementIdByCode(DgnCode::FromJson2(codeVal));
        }

    //  Look up the element
    auto elem = dgndb.Elements().GetElement(eid);

    if (!elem.IsValid())
        return BSISUCCESS;      // This is not an exception. It just returns an empty result.

    elementJson = elem->ToJson(inOpts);

    auto eclass = elem->GetElementClass();
        
    // Auto-handled properties
    auto autoHandledProps = dgndb.Elements().GetAutoHandledPropertiesSelectECSql(*eclass);
    if (autoHandledProps.empty())
        return BSISUCCESS;

    auto stmt = dgndb.GetPreparedECSqlStatement(autoHandledProps.c_str());
    if (!stmt.IsValid())
        {
        errmsg = IModelJs::GetLastEcdbIssue();
        status = DgnDbStatus::WrongClass;
        return BSIERROR;
        }

    stmt->BindId(1, eid);

    if (BE_SQLITE_ROW == stmt->Step())
        GetRowAsJson(elementJson, *stmt);

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus IModelJs::GetModel(DgnDbStatus& status, Utf8StringR errmsg, JsonValueR modelJson, DgnDbR dgndb, Json::Value const& inOpts)
    {
    DgnModelId modelId(inOpts[json_id()].asUInt64());
    if (!modelId.IsValid())
        {
        auto codeVal = inOpts[json_code()];
        if (!codeVal)
            {
            errmsg = "DgnDbStatus::NotFound";
            status = DgnDbStatus::NotFound;
            return BSIERROR;
            }
        modelId = dgndb.Models().QuerySubModelId(DgnCode::FromJson2(codeVal));
        }

    //  Look up the model
    auto model = dgndb.Models().GetModel(modelId);

    if (!model.IsValid())
        return BSISUCCESS;      // This is not an exception. It just returns an empty result.

    modelJson = model->ToJson(inOpts);

    auto stmt = dgndb.Models().GetSelectStmt(*model);
    if (!stmt.IsValid())
        {
        errmsg = IModelJs::GetLastEcdbIssue();
        status = DgnDbStatus::WrongClass;
        return BSIERROR;
        }

    if (BE_SQLITE_ROW == stmt->Step())
        GetRowAsJson(modelJson, *stmt);

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus IModelJs::GetElementPropertiesForDisplay(DgnDbStatus& status, Utf8StringR errmsg, JsonValueR result, DgnDbR dgndb, Utf8CP eidstr)
    {
    DgnElementId elemId(DgnElementId::FromString(eidstr).GetValueUnchecked());
    if (!elemId.IsValid())
        {
        status = DgnDbStatus::InvalidId;
        errmsg = "DgnDbStatus::InvalidId";
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
BentleyStatus IModelJs::OpenDgnDb(BeSQLite::DbResult& result, Utf8StringR errmsg, DgnDbPtr& db, BeFileNameCR dbname, DgnDb::OpenMode mode)
    {
    db = IModelJs::GetDbByName(result, errmsg, dbname, mode);
    return db.IsValid()? BSISUCCESS: BSIERROR;
    }

