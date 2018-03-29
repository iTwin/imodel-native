/*--------------------------------------------------------------------------------------+
|
|     $Source: JsInteropDgnDb.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "IModelJsNative.h"
#include <Bentley/BeDirectoryIterator.h>
#include <DgnPlatform/DgnGeoCoord.h>

using namespace IModelJsNative;

#define SET_IF_NOT_EMPTY_STR(j, str) {if (!(str).empty()) j = str;}
#define SET_IF_NOT_NULL_STR(j, str) {if (nullptr != (str)) j = str;}

BE_JSON_NAME(baseClasses)
BE_JSON_NAME(code)
BE_JSON_NAME(customAttributes)
//BE_JSON_NAME(description)
BE_JSON_NAME(direction)
BE_JSON_NAME(displayLabel)
BE_JSON_NAME(ecclass)
BE_JSON_NAME(extendedType)
BE_JSON_NAME(federationGuid)
BE_JSON_NAME(id)
BE_JSON_NAME(isCustomHandled)
BE_JSON_NAME(isCustomHandledOrphan);
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
        JsInterop::GetECValuesCollectionAsJson(jca[json_properties()], caProps);
        jca[json_ecclass()] = ca->GetClass().GetFullName();
        jcas.append(jca);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                  06/17
//---------------------------------------------------------------------------------------
DgnDbStatus JsInterop::GetECClassMetaData(JsonValueR mjson, DgnDbR dgndb, Utf8CP ecSchemaName, Utf8CP ecClassName)
    {
    auto ecclass = dgndb.Schemas().GetClass(ecSchemaName, ecClassName);
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
DgnDbStatus JsInterop::UpdateProjectExtents(DgnDbR dgndb, JsonValueCR newExtents)
	{
	auto& geolocation = dgndb.GeoLocation();
	AxisAlignedBox3d extents;
	extents.FromJson(newExtents);
	geolocation.SetProjectExtents(extents);
	return DgnDbStatus::Success;	// Is there a failure case here?
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
* @bsimethod                                    Keith.Bentley                   08/17
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult JsInterop::InsertLinkTableRelationship(JsonValueR outJson, DgnDbR dgndb, JsonValueR inJson)
    {
    auto relClass = parseRelClass(dgndb, inJson);
    if (nullptr == relClass)
        return BE_SQLITE_ERROR;

    DgnElementId sourceId, targetid;
    sourceId.FromJson(inJson["sourceId"]);
    targetid.FromJson(inJson["targetId"]);

    ECN::IECRelationshipInstanceCP props = nullptr;

    BeSQLite::EC::ECInstanceKey relKey;
    auto rc = dgndb.InsertLinkTableRelationship(relKey, *relClass, sourceId, targetid, props);
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
    // ECN::IECRelationshipInstanceP
    auto props = relClass->GetDefaultStandaloneEnabler()->CreateInstance();
    for (auto const& jsPropName : inJson.getMemberNames())
        {
        ECPropertyP ecprop = relClass->GetPropertyP(jsPropName.c_str());
        if (nullptr == ecprop || !ecprop->GetIsPrimitive())
            continue; // TODO: support other property types

        ECN::ECValue value;
        ECUtils::ConvertJsonToECValue(value, inJson[jsPropName], ecprop->GetAsPrimitiveProperty()->GetType());

        props->SetValue(jsPropName.c_str(), value);
        }
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
static CodeScopeSpec getCodeScopeSpec(CodeScopeSpec::Type cstype, CodeScopeSpec::ScopeRequirement cssreq)
    {
    switch (cstype)
        {
        case CodeScopeSpec::Type::Repository:       return CodeScopeSpec::CreateRepositoryScope(cssreq);
        case CodeScopeSpec::Type::Model:            return CodeScopeSpec::CreateModelScope(cssreq);
        case CodeScopeSpec::Type::ParentElement:    return CodeScopeSpec::CreateParentElementScope(cssreq);
        case CodeScopeSpec::Type::RelatedElement:   return CodeScopeSpec::CreateRelatedElementScope(nullptr, cssreq);
        }

    BeAssert(false && "it's up to imodeljs-backend to keep these enums straight!");
    return CodeScopeSpec::CreateRepositoryScope();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus JsInterop::InsertCodeSpec(Utf8StringR idStr, DgnDbR db, Utf8StringCR name, CodeScopeSpec::Type cstype, CodeScopeSpec::ScopeRequirement cssreq)
    {
    CodeSpecPtr codeSpec = CodeSpec::Create(db, name.c_str(), getCodeScopeSpec(cstype, cssreq));
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
DgnDbStatus JsInterop::InsertModel(JsonValueR outJson, DgnDbR dgndb, JsonValueR inJson)
    {
    DgnModel::CreateParams params(dgndb, inJson);
    if (!params.m_classId.IsValid())
        return DgnDbStatus::WrongClass;

    ModelHandlerP handler = dgn_ModelHandler::Model::FindHandler(dgndb, params.m_classId);
    if (nullptr == handler)
        {
        BeAssert(false);
        return DgnDbStatus::WrongClass;
        }

    DgnModelPtr model = handler->Create(params);
    if (!model.IsValid())
        {
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
DgnDbStatus JsInterop::UpdateModel(DgnDbR dgndb, JsonValueR inJson)
    {
    if (!inJson.isMember(DgnModel::json_id()))
        return DgnDbStatus::BadArg;

    auto idJsonVal = inJson[DgnModel::json_id()];
    DgnModelId mid(BeInt64Id::FromString(idJsonVal.asCString()).GetValue());
    if (!mid.IsValid())
        return DgnDbStatus::InvalidId;

    DgnModelPtr model = dgndb.Models().GetModel(mid);
    if (!model.IsValid())
        return DgnDbStatus::MissingId;

    model->FromJson(inJson);
    return model->Update();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      09/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus JsInterop::DeleteModel(DgnDbR dgndb, Utf8StringCR midStr)
    {
    DgnModelId mid(BeInt64Id::FromString(midStr.c_str()).GetValue());
    if (!mid.IsValid())
        return DgnDbStatus::InvalidId;

    DgnModelPtr model = dgndb.Models().GetModel(mid);
    if (!model.IsValid())
        return DgnDbStatus::MissingId;

    return model->Delete();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus JsInterop::GetModel(JsonValueR modelJson, DgnDbR dgndb, JsonValueCR inOpts)
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
    // Note: there are no auto-handled properties on DgnModels. Any such data goes on the modeled element.
    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Ramanujam.Raman                 09/17
//---------------------------------------------------------------------------------------
void JsInterop::CloseDgnDb(DgnDbR dgndb)
    {
    if (dgndb.Txns().HasChanges())
        dgndb.SaveChanges();
    dgndb.CloseDb();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Ramanujam.Raman                 09/17
//---------------------------------------------------------------------------------------
DbResult JsInterop::GetCachedBriefcaseInfos(JsonValueR jsonBriefcaseInfos, BeFileNameCR cachePath)
    {
    /*
     * Folder structure for cached imodels:
     *  /assets/imodels/                => cachePath (can be specified)
     *    iModelId1/                    => iModelPath
     *      csets/                      => csetPath
     *        csetId1.cs
     *        csetid2.cs
     *        ...
     *      readOnly/
     *        0/IModelName.bim
     *        1/IModelName.bim
     *        ...
     *      readWrite/
     *        briefcaseId1/IModelName.bim
     *        briefcaseId2/IModelName.bim
     *        ...
     *    iModelId2/
     *      ...
     *
     * Format of returned JSON:
     *  {
     *    "iModelId1": [
     *      {
     *        "pathname": "path to imodel",
     *        "parentChangeSetId": "Id of parent change set",
     *        "briefcaseId": "Id of brief case",
     *        "readOnly": true or false
     *      },
     *      {
     *        ...
     *      },
     *    ],
     *    "iModelId2": [
     *      ...
     *    ]
     * }
     */

    BeFileName iModelPath;
    bool isDir;
    jsonBriefcaseInfos = Json::objectValue;
    for (BeDirectoryIterator iModelPaths(cachePath); iModelPaths.GetCurrentEntry(iModelPath, isDir, true /*fullpath*/) == SUCCESS; iModelPaths.ToNext())
        {
        if (!isDir)
            continue;
        Json::Value jsonIModelBriefcases = Json::arrayValue;

        BeFileName subFolderPath;
        for (BeDirectoryIterator subFolderPaths(iModelPath); subFolderPaths.GetCurrentEntry(subFolderPath, isDir, true /*fullpath*/) == SUCCESS; subFolderPaths.ToNext())
            {
            if (!isDir)
                continue;

            BeFileName basePath = subFolderPath.GetBaseName();
            bool isReadonly;
            if (basePath.Equals(L"readOnly"))
                isReadonly = true;
            else if (basePath.Equals(L"readWrite"))
                isReadonly = false;
            else
                continue;

            BeFileName briefcasePath;
            for (BeDirectoryIterator briefcasePaths(subFolderPath); briefcasePaths.GetCurrentEntry(briefcasePath, isDir, true /*fullpath*/) == SUCCESS; briefcasePaths.ToNext())
                {
                BeFileName briefcasePathname;
                BeDirectoryIterator briefcasePathnames(briefcasePath);
                if (briefcasePathnames.GetCurrentEntry(briefcasePathname, isDir, true /*fullpath*/) != SUCCESS)
                    continue;

                DbResult result;
                DgnDb::OpenParams openParams(Db::OpenMode::Readonly);
                DgnDbPtr db = DgnDb::OpenDgnDb(&result, briefcasePathname, openParams);
                if (!EXPECTED_CONDITION(result == BE_SQLITE_OK))
                    {
                    BeFileName::EmptyAndRemoveDirectory(briefcasePath);
                    continue;
                    }
                RevisionManagerR revisions = db->Revisions();

                Json::Value jsonBriefcase = Json::objectValue;
                jsonBriefcase["pathname"] = briefcasePathname.GetNameUtf8();
                jsonBriefcase["parentChangeSetId"] = revisions.GetParentRevisionId();
                jsonBriefcase["briefcaseId"] = db->GetBriefcaseId().GetValue();
                jsonBriefcase["readOnly"] = isReadonly;
                if (revisions.HasReversedRevisions())
                    jsonBriefcase["reversedChangeSetId"] = revisions.GetReversedRevisionId();

                jsonIModelBriefcases.append(jsonBriefcase);
                }
            }
            jsonBriefcaseInfos[iModelPath.GetBaseName().GetNameUtf8()] = jsonIModelBriefcases;
        }

    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Keith.Bentley                 12/17
//---------------------------------------------------------------------------------------
void JsInterop::GetIModelProps(JsonValueR val, DgnDbCR dgndb)
    {
    // add the root subject, if available.
    auto rootSubject = dgndb.Elements().GetRootSubject();
    if (rootSubject.IsValid())
        {
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
    JsonUtils::DPoint3dToJson(val[json_globalOrigin()], geolocation.GetGlobalOrigin());

    // if the project is geolocated, add the world to ecef transform
    auto* dgnGCS = geolocation.GetDgnGCS();
    if (nullptr == dgnGCS)
        return;

    DPoint3d origin = extents.GetCenter();
    DPoint3d ecfOrigin, ecfNorth;
    DPoint3d north = origin;
    north.y += 100.0;

    GeoPoint originLatLong, northLatLong;
    dgnGCS->LatLongFromUors(originLatLong, origin);
    dgnGCS->XYZFromLatLong(ecfOrigin, originLatLong);
    dgnGCS->LatLongFromUors(northLatLong, north);
    dgnGCS->XYZFromLatLong(ecfNorth, northLatLong);

    DVec3d zVector, yVector;
    zVector.Normalize((DVec3dCR) ecfOrigin);
    yVector.NormalizedDifference(ecfNorth, ecfOrigin);

    RotMatrix rMatrix = RotMatrix::FromIdentity();
    rMatrix.SetColumn(yVector, 1);
    rMatrix.SetColumn(zVector, 2);
    rMatrix.SquareAndNormalizeColumns(rMatrix, 1, 2);
    JsonUtils::TransformToJson(val[json_ecefTrans()], Transform::From(rMatrix, ecfOrigin));
    }
