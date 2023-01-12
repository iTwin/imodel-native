/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "IModelJsNative.h"
#include <Bentley/BeDirectoryIterator.h>
#include <DgnPlatform/DgnGeoCoord.h>
#include <DgnPlatform/ElementDependency.h>
#include <ECObjects/ECJsonUtilities.h>

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
BE_JSON_NAME(maxOccurs)
BE_JSON_NAME(maximumLength)
BE_JSON_NAME(maximumValue)
BE_JSON_NAME(minOccurs)
BE_JSON_NAME(minimumLength)
BE_JSON_NAME(minimumValue)
BE_JSON_NAME(modifier)
BE_JSON_NAME(onlyBaseProperties)
BE_JSON_NAME(primitiveType)
BE_JSON_NAME(properties)
BE_JSON_NAME(readOnly)
BE_JSON_NAME(relationshipClass)
BE_JSON_NAME(structName)
BE_JSON_NAME(geoCoords)
BE_JSON_NAME(iModelCoords)
BE_JSON_NAME(source)
BE_JSON_NAME(target)
BE_JSON_NAME(geographicCoordinateSystem)

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
[[noreturn]] void JsInterop::throwDgnDbStatus(DgnDbStatus stat) {
    Utf8String msg;
    switch (stat) {
        case DgnDbStatus::BadArg:         msg = "invalid arguments"; break;
        case DgnDbStatus::BadRequest:     msg = "bad request"; break;
        case DgnDbStatus::BadSchema:      msg = "bad schema"; break;
        case DgnDbStatus::DuplicateCode:  msg = "duplicate code"; break;
        case DgnDbStatus::DuplicateName:  msg = "duplicate name"; break;
        case DgnDbStatus::InvalidId:      msg = "Invalid id"; break;
        case DgnDbStatus::InvalidParent:  msg = "invalid parent"; break;
        case DgnDbStatus::MissingId:      msg = "missing id"; break;
        case DgnDbStatus::NotFound:       msg = "not found"; break;
        case DgnDbStatus::ReadOnly:       msg = "readonly"; break;
        case DgnDbStatus::ReadOnlyDomain: msg = "readonly domain"; break;
        case DgnDbStatus::SQLiteError:    msg = "sql error"; break;
        case DgnDbStatus::WriteError:     msg = "write error"; break;
        case DgnDbStatus::WrongClass:     msg = "wrong class"; break;
        case DgnDbStatus::WrongDgnDb:     msg = "wrong iModel"; break;
        default:
            msg = Utf8PrintfString("error=%x", (int) stat); break;
    }
    BeNapi::ThrowJsException(Env(), msg.c_str(), (int) stat);
}

//---------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
//---------------------------------------------------------------------------------------
static void getPrimitiveECPropertyMetaData(BeJsValue json, ECN::PrimitiveECPropertyCR prop)
    {
    json[json_primitiveType()] = prop.GetType();
    SET_IF_NOT_EMPTY_STR(json[json_extendedType()], prop.GetExtendedTypeName());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static void getNavigationECPropertyMetaData(BeJsValue json, ECN::NavigationECPropertyCR navProp)
    {
    json[json_direction()] = (ECN::ECRelatedInstanceDirection::Forward == navProp.GetDirection())? "Forward": "Backward";
    auto rc = navProp.GetRelationshipClass();
    if (nullptr != rc)
        json[json_relationshipClass()] = rc->GetFullName();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static void getStructECPropertyMetaData(BeJsValue json, ECN::StructECPropertyCR structProp)
    {
    json[json_structName()] = structProp.GetType().GetFullName();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static void getArrayECPropertyMetaData(BeJsValue json, ECN::ArrayECPropertyCR arrayProp)
    {
    json[json_minOccurs()] = arrayProp.GetMinOccurs();
    if (!arrayProp.IsStoredMaxOccursUnbounded())
        json[json_maxOccurs()] = arrayProp.GetStoredMaxOccurs();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static void getPrimitiveArrayECPropertyMetaData(BeJsValue json, ECN::PrimitiveArrayECPropertyCR arrayProp)
    {
    getArrayECPropertyMetaData(json, arrayProp);
    json[json_primitiveType()] = arrayProp.GetPrimitiveElementType();
    SET_IF_NOT_EMPTY_STR(json[json_extendedType()], arrayProp.GetExtendedTypeName());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static void getStructArrayECPropertyMetaData(BeJsValue json, ECN::StructArrayECPropertyCR arrayProp)
    {
    json[json_structName()] = arrayProp.GetStructElementType().GetFullName();
    getArrayECPropertyMetaData(json, arrayProp);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static void customAttributesToJson(BeJsValue jcas, ECN::IECCustomAttributeContainerCR cas)
    {
    for (auto ca : cas.GetCustomAttributes(false))
        {
        auto jca = jcas.appendValue();
        ECN::ECValuesCollection caProps(*ca);
        JsInterop::GetECValuesCollectionAsJson(jca[json_properties()], caProps);
        jca[json_ecclass()] = ca->GetClass().GetFullName();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static void doubleOrIntToJson(BeJsValue json, ECN::ECValueR v)
    {
    if (v.IsNull())
        {
        json.SetNull();
        return;
        }

    auto primType = v.GetPrimitiveType();
    if (primType == PrimitiveType::PRIMITIVETYPE_Double)
        json = v.GetDouble();
    else if (primType == PrimitiveType::PRIMITIVETYPE_Long)
        json = v.GetLong();
    else
        json = v.GetInteger();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DgnDbStatus JsInterop::GetECClassMetaData(BeJsValue mjson, DgnDbR dgndb, Utf8CP ecSchemaName, Utf8CP ecClassName)
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

    auto basesjson = mjson[json_baseClasses()];
    basesjson.SetEmptyArray();
    for (auto base: ecclass->GetBaseClasses())
        basesjson.appendValue() = base->GetFullName();

    customAttributesToJson(mjson[json_customAttributes()], *ecclass);

    auto propsjson = mjson[json_properties()];

    auto customHandledProperty = dgndb.Schemas().GetClass(BIS_ECSCHEMA_NAME, "CustomHandledProperty");

    for (auto ecprop : ecclass->GetProperties(false))
        {
        Utf8String pname(ecprop->GetName());
        pname[0] = static_cast<Utf8Char>(tolower(pname[0]));
        auto propjson = propsjson[pname];

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
// @bsimethod
//---------------------------------------------------------------------------------------
DgnDbStatus JsInterop::GetSchemaItem(BeJsValue mjson, DgnDbR dgndb, Utf8CP schemaName, Utf8CP itemName)
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
// @bsimethod
//---------------------------------------------------------------------------------------
DgnDbStatus JsInterop::GetElement(BeJsValue elementJson, DgnDbR dgndb, Napi::Object obj) {
    BeJsConst inOpts(obj);
    DgnElementCPtr elem;
    DgnElementId eid = inOpts[json_id()].GetId64<DgnElementId>();

    if (!eid.IsValid()) {
        auto fedJson = inOpts[json_federationGuid()];
        if (fedJson.isString()) {
            BeGuid federationGuid;
            federationGuid.FromString(fedJson.asCString());
            elem = dgndb.Elements().QueryElementByFederationGuid(federationGuid);
        } else {
            eid = dgndb.Elements().QueryElementIdByCode(DgnCode::FromJson(inOpts[json_code()], dgndb, false));
        }
    }

    if (!elem.IsValid())
        elem = dgndb.Elements().GetElement(eid);

    if (!elem.IsValid())
        return DgnDbStatus::NotFound;

    // if they only want base properties, don't bother calling virtual function
    if (inOpts[json_onlyBaseProperties()].asBool())
        elem->ToBaseJson(elementJson);
    else
        elem->ToJson(elementJson, inOpts);
    return DgnDbStatus::Success;
}

struct SetNapiObjOnElement {
    DgnElementR m_element;
    SetNapiObjOnElement(DgnElementR element, NapiObjectCP obj) : m_element(element) {BeAssert(nullptr==element.m_napiObj); element.m_napiObj = obj;}
    ~SetNapiObjOnElement() {m_element.m_napiObj = nullptr;}
};

/*---------------------------------------------------------------------------------**/ /**
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void callJsPreHandler(DgnDbR db, DgnClassId classId, Utf8CP methodName, Napi::Object obj)  {
    auto arg = Napi::Object::New(obj.Env());
    arg.Set("props", obj);
    db.CallJsHandlerMethod(classId, methodName, arg);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Napi::String JsInterop::InsertElement(DgnDbR dgndb, Napi::Object obj, Napi::Value optionsObj) {
    BeJsConst inJson(obj);
    BeJsConst inOptionsJson(optionsObj);

    auto classId = ECJsonUtilities::GetClassIdFromClassNameJson(inJson[DgnElement::json_classFullName()], dgndb.GetClassLocater());
    callJsPreHandler(dgndb, classId, "onInsert", obj);

    try {
        DgnElement::CreateParams params(dgndb, inJson);
        if (!params.m_classId.IsValid())
            throwWrongClass();

        ElementHandlerP elHandler = dgn_ElementHandler::Element::FindHandler(dgndb, params.m_classId);
        if (nullptr == elHandler) {
            BeAssert(false);
            throwWrongClass();
        }

        DgnElementPtr el = elHandler->Create(params);
        if (!el.IsValid()) {
            BeAssert(false);
            throwBadArg();
        }

        el->FromJson(inJson);

        // if no federationGuid was supplied, create one for the element before we add it.
        if (!inJson.isStringMember(json_federationGuid()))
            el->SetFederationGuid(BeGuid(true));

        // if the option "forceUseId" is set, attempt to insert the element preserving that id - used by transformer.
        if (inOptionsJson.isObject() && inOptionsJson.Get(json_forceUseId()).asBool()) {
            if (!inJson.isStringMember(json_id())) {
                BeNapi::ThrowJsException(Env(), "invalid argument, the id is required if forcing its usage", (int)DgnDbStatus::BadArg);
            }
            auto eid = inJson[json_id()].GetId64<DgnElementId>();
            el->CopyIdentityFrom(eid, el->GetFederationGuid());
        }

        SetNapiObjOnElement _v(*el, &obj);
        DgnDbStatus status;
        auto newEl = el->Insert(&status);
        if (!newEl.IsValid())
            BeNapi::ThrowJsException(Env(), "error inserting", (int)status);
        return Napi::String::New(Env(), newEl->GetElementId().ToHexStr());
    } catch (std::logic_error const& err) {
        BeNapi::ThrowJsException(Env(), err.what(), (int)DgnDbStatus::BadArg);
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void JsInterop::UpdateElement(DgnDbR dgndb, Napi::Object obj) {
    BeJsConst inJson(obj);
    DgnElementId eid = inJson[DgnElement::json_id()].GetId64<DgnElementId>();
    if (!eid.IsValid())
        throwInvalidId();

    DgnElementCPtr elPersist = dgndb.Elements().GetElement(eid);
    if (!elPersist.IsValid())
        throwMissingId();

    try {
        auto el = elPersist->CopyForEdit();
        callJsPreHandler(dgndb, el->GetElementClassId(), "onUpdate", obj);
        el->FromJson(inJson);

        SetNapiObjOnElement _v(*el, &obj);
        DgnDbStatus status = el->Update();
        if (DgnDbStatus::Success != status)
            BeNapi::ThrowJsException(Env(), "error updating", (int)status);
    } catch (std::logic_error const& err) {
        BeNapi::ThrowJsException(Env(), err.what(), (int)DgnDbStatus::BadArg);
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnElementId getElementIdFromNapiValue(Napi::Value const& napiVal)
    {
    auto napiString = napiVal.As<Napi::String>();
    std::string utf8String = napiString.Utf8Value();
    return DgnElementId(BeInt64Id::FromString(utf8String.c_str()).GetValueUnchecked());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus JsInterop::SimplifyElementGeometry(DgnDbR db, Napi::Object simplifyArgs)
    {
    if (!simplifyArgs.Get("convertBReps").IsBoolean())
        return DgnDbStatus::Success;

    auto doConvertBReps = simplifyArgs.Get("convertBReps").As<Napi::Boolean>();
    if (!doConvertBReps)
        return DgnDbStatus::Success;

    DgnElementId elementId = getElementIdFromNapiValue(simplifyArgs.Get("id"));
    if (!elementId.IsValid())
        return DgnDbStatus::InvalidId;

    DgnElementCPtr savedElement = db.Elements().GetElement(elementId);
    if (!savedElement.IsValid())
        return DgnDbStatus::MissingId;

    bool isGeometrySource = nullptr != savedElement->ToGeometrySource();
    bool isGeometryPart = nullptr != savedElement->ToGeometryPart();

    if (!isGeometrySource && !isGeometryPart)
        return DgnDbStatus::NoGeometry;

    GeometryStream simplifiedGeom;
    GeometryStreamCP originalGeom = nullptr;
    if (isGeometrySource)
        originalGeom = &savedElement->ToGeometrySource()->GetGeometryStream();
    else
        originalGeom = &savedElement->ToGeometryPart()->GetGeometryStream();

    bool changed = false;
    try {
        DgnDbStatus status = GeometryStreamIO::ConvertBRepsToPolyfacesOrCurves(db, simplifiedGeom, *originalGeom, changed);
        if (DgnDbStatus::Success != status)
            return status;
        }
    // the parasolid kernel may throw runtime errors which we don't want to crash the process to report back to javascript
    catch (const std::runtime_error& err)
        {
        GetNativeLogger().errorv("While simplifying element %s's geometry an error occurred: '%s'", simplifyArgs.Get("id").As<Napi::String>().Utf8Value().c_str(), err.what());
        return DgnDbStatus::BadElement;
        }

    if (!changed)
       return DgnDbStatus::Success;

    auto updatedElement = savedElement->CopyForEdit();

    // An element's GeometryStream is restricted to GeometryBuilder, but that doesn't give us low enough level
    // access. Most callers should not modify directly since they may change range, placement, etc. but we know
    // that can't happen in this case. Brien blesses this const_cast.
    GeometryStreamP updatedElementGeom = nullptr;
    if (isGeometrySource)
        updatedElementGeom = const_cast<GeometryStreamP>(&updatedElement->ToGeometrySource()->GetGeometryStream());
    else
        updatedElementGeom = const_cast<GeometryStreamP>(&updatedElement->ToGeometryPart()->GetGeometryStream());

    *updatedElementGeom = std::move(simplifiedGeom);
    return updatedElement->Update();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void JsInterop::UpdateProjectExtents(DgnDbR dgndb, BeJsConst newExtents) {
    auto& geolocation = dgndb.GeoLocation();
    AxisAlignedBox3d extents;
    extents.FromJson(newExtents);
    geolocation.SetProjectExtents(extents);
    geolocation.Save();
}

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void JsInterop::UpdateIModelProps(DgnDbR dgndb, BeJsConst props) {
    auto& geolocation = dgndb.GeoLocation();
    auto extJson = props[json_projectExtents()];
    if (!extJson.isNull()) {
        AxisAlignedBox3d extents;
        extents.FromJson(props[json_projectExtents()]);
        geolocation.SetProjectExtents(extents);
    }
    auto orgJson = props[json_globalOrigin()];
    if (!orgJson.isNull())
        geolocation.SetGlobalOrigin(BeJsGeomUtils::ToDPoint3d(orgJson));

    auto ecefJson = props[json_ecefLocation()];
    if (!ecefJson.isNull()) {
        EcefLocation ecef;
        ecef.FromJson(ecefJson);
        geolocation.SetEcefLocation(ecef);
    }

    auto gcsJson = props[json_geographicCoordinateSystem()];
    if (!gcsJson.isNull()) {
        GeoCoordinates::BaseGCSPtr newGCS = GeoCoordinates::BaseGCS::CreateGCS();

        if (newGCS.IsValid()) {
            Utf8String errorFacingMessage;
            if (SUCCESS == newGCS->FromJson(gcsJson, errorFacingMessage))
                geolocation.SetGCS(newGCS.get());
        }
    }

    geolocation.Save();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void JsInterop::DeleteElement(DgnDbR dgndb, Utf8StringCR eidStr) {
    DgnElementId eid(BeInt64Id::FromString(eidStr.c_str()).GetValue());
    if (!eid.IsValid())
        throwInvalidId();

    DgnElementCPtr elPersist = dgndb.Elements().GetElement(eid);
    if (!elPersist.IsValid())
        throwMissingId();

    auto stat =  elPersist->Delete();
    if (stat != DgnDbStatus::Success)
        BeNapi::ThrowJsException(Env(), "error deleting element", (int)stat);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Napi::String JsInterop::InsertElementAspect(DgnDbR db, Napi::Object obj) {
    BeJsConst aspectProps(obj);

    DgnElement::RelatedElement relatedElement;
    relatedElement.FromJson(db, aspectProps[json_element()]);
    if (!relatedElement.IsValid())
        throwInvalidId();

    DgnElementCPtr element = db.Elements().GetElement(relatedElement.m_id);
    if (!element.IsValid())
        throwMissingId();

    DgnClassId aspectClassId = ECJsonUtilities::GetClassIdFromClassNameJson(aspectProps[DgnElement::json_classFullName()], db.GetClassLocater());
    if (!aspectClassId.IsValid())
        throwWrongClass();

    ECClassCP aspectClass = db.Schemas().GetClass(aspectClassId);
    if (nullptr == aspectClass)
        throwBadSchema();

    bool isMultiAspect = aspectClass->Is(BIS_ECSCHEMA_NAME, BIS_CLASS_ElementMultiAspect);

    StandaloneECInstancePtr aspectEcProps = aspectClass->GetDefaultStandaloneEnabler()->CreateInstance();
    if (!aspectEcProps.IsValid())
        throwBadRequest();

    std::function<bool(Utf8CP)> shouldConvertProperty = [aspectClass](Utf8CP propName)
        {
        if (0 == strcmp(propName, DgnElement::json_classFullName())) return false;
        return nullptr != aspectClass->GetPropertyP(propName);
        };
    if (BentleyStatus::SUCCESS != ECN::JsonECInstanceConverter::JsonToECInstance(*aspectEcProps, aspectProps, db.GetClassLocater(), shouldConvertProperty))
        throwBadRequest();

    DgnElementPtr elementEdit = element->CopyForEdit();
    if (!elementEdit.IsValid())
        throwWriteError();

    auto arg = Napi::Object::New(obj.Env());
    arg.Set("props", obj);
    db.CallJsHandlerMethod(aspectClassId, "onInsert", arg);

    DgnDbStatus stat;
    RefCountedCPtr<DgnElement::Aspect> createdAspectPtr
        = isMultiAspect
        ? static_cast<RefCountedCPtr<DgnElement::Aspect>>(DgnElement::GenericMultiAspect::AddAspect(*elementEdit, *aspectEcProps, &stat))
        : static_cast<RefCountedCPtr<DgnElement::Aspect>>(DgnElement::GenericUniqueAspect::SetAspect(*elementEdit, *aspectEcProps, nullptr, &stat));

    if (DgnDbStatus::Success != stat)
        throwDgnDbStatus(stat);

    BeAssert(createdAspectPtr != nullptr);

    stat = elementEdit->Update();
    if (DgnDbStatus::Success != stat)
        throwDgnDbStatus(stat);

    db.CallJsHandlerMethod(aspectClassId, "onInserted", arg);

    return Napi::String::New(obj.Env(), createdAspectPtr->GetAspectInstanceId().ToHexStr());
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void JsInterop::UpdateElementAspect(DgnDbR db, Napi::Object obj) {
    BeJsConst aspectProps(obj);

    DgnElement::RelatedElement relatedElement;
    relatedElement.FromJson(db, aspectProps[json_element()]);
    if (!relatedElement.IsValid())
        throwInvalidId();

    DgnElementCPtr element = db.Elements().GetElement(relatedElement.m_id);
    if (!element.IsValid())
        throwMissingId();

    DgnClassId aspectClassId = ECJsonUtilities::GetClassIdFromClassNameJson(aspectProps[DgnElement::json_classFullName()], db.GetClassLocater());
    if (!aspectClassId.IsValid())
        throwWrongClass();

    ECClassCP aspectClass = db.Schemas().GetClass(aspectClassId);
    if (nullptr == aspectClass)
        throwBadSchema();

    DgnElementPtr elementEdit = element->CopyForEdit();
    if (!elementEdit.IsValid())
        throwWriteError();

    auto arg = Napi::Object::New(obj.Env());
    arg.Set("props", obj);
    db.CallJsHandlerMethod(aspectClassId, "onUpdate", arg);

    IECInstanceP aspect;
    bool isMultiAspect = aspectClass->Is(BIS_ECSCHEMA_NAME, BIS_CLASS_ElementMultiAspect);
    if (isMultiAspect)
        {
        ECInstanceId aspectId = aspectProps[DgnElement::json_id()].GetId64<ECInstanceId>();
        aspect = DgnElement::GenericMultiAspect::GetAspectP(*elementEdit, *aspectClass, aspectId);
        }
    else
        {
        aspect = DgnElement::GenericUniqueAspect::GetAspectP(*elementEdit, *aspectClass);
        }

    if (nullptr == aspect)
        throwNotFound();

    std::function<bool(Utf8CP)> shouldConvertProperty = [aspectClass](Utf8CP propName)
        {
        if ((0 == strcmp(propName, DgnElement::json_classFullName())) || (0 == strcmp(propName, json_element()))) return false;
        return nullptr != aspectClass->GetPropertyP(propName);
        };
    if (BentleyStatus::SUCCESS != ECN::JsonECInstanceConverter::JsonToECInstance(*aspect, aspectProps, db.GetClassLocater(), shouldConvertProperty))
        throwBadRequest();

    auto stat = elementEdit->Update();
    if (DgnDbStatus::Success != stat)
        throwDgnDbStatus(stat);

    db.CallJsHandlerMethod(aspectClassId, "onUpdated", arg);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void JsInterop::DeleteElementAspect(DgnDbR db, Utf8StringCR aspectIdStr)   {
    ECInstanceId aspectId(BeInt64Id::FromString(aspectIdStr.c_str()).GetValue());
    if (!aspectId.IsValid())
        throwInvalidId();

    CachedECSqlStatementPtr statement = db.GetPreparedECSqlStatement(
        "SELECT ECClassId,Element.Id FROM " BIS_SCHEMA(BIS_CLASS_ElementUniqueAspect) " WHERE ECInstanceId=? UNION "
        "SELECT ECClassId,Element.Id FROM " BIS_SCHEMA(BIS_CLASS_ElementMultiAspect)  " WHERE ECInstanceId=?");
    if (!statement.IsValid())
        throwSqlError();

    statement->BindId(1, aspectId);
    statement->BindId(2, aspectId);
    if (BE_SQLITE_ROW != statement->Step())
        throwNotFound();

    DgnClassId aspectClassId = statement->GetValueId<DgnClassId>(0);
    DgnElementId elementId = statement->GetValueId<DgnElementId>(1);

    ECClassCP aspectClass = db.Schemas().GetClass(aspectClassId);
    if (nullptr == aspectClass)
        throwBadSchema();

    bool isMultiAspect = aspectClass->Is(BIS_ECSCHEMA_NAME, BIS_CLASS_ElementMultiAspect);

    DgnElementCPtr element = db.Elements().GetElement(elementId);
    if (!element.IsValid())
        throwMissingId();

    DgnElementPtr elementEdit = element->CopyForEdit();
    if (!elementEdit.IsValid())
        throwWriteError();

    BeJsNapiObject arg(db.GetJsIModelDb()->Env());
    arg["aspectId"] = aspectId;
    db.CallJsHandlerMethod(aspectClassId, "onDelete", arg);

    if (isMultiAspect)
        {
        DgnElement::MultiAspect* aspect = DgnElement::MultiAspect::GetAspectP(*elementEdit, *aspectClass, aspectId);
        if (nullptr == aspect)
            throwNotFound();

        aspect->Delete();
        }
    else
        {
        DgnElement::UniqueAspect* aspect = DgnElement::UniqueAspect::GetAspectP(*elementEdit, *aspectClass);
        if (nullptr == aspect)
            throwNotFound();

        aspect->Delete();
        }

    auto stat = elementEdit->Update();
    if (DgnDbStatus::Success != stat)
        throwDgnDbStatus(stat);

    db.CallJsHandlerMethod(aspectClassId, "onDeleted", arg);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::ECRelationshipClassCP parseRelClass(DgnDbR dgndb, BeJsConst inJson)
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::EC::ECInstanceKey parseECRelationshipInstanceKeyKey(DgnDbR dgndb, BeJsConst inJson)
    {
    auto relClass = parseRelClass(dgndb, inJson);
    if (nullptr == relClass)
        return BeSQLite::EC::ECInstanceKey();

    DgnElementId relId;
    relId.FromJson(inJson["id"]);

    return BeSQLite::EC::ECInstanceKey(relClass->GetId(), ECInstanceId(relId.GetValue()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static ECN::StandaloneECRelationshipInstancePtr getRelationshipProperties(ECN::ECRelationshipClassCP relClass, BeJsConst inJson, DgnDbR dgndb)
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Napi::String JsInterop::InsertLinkTableRelationship(DgnDbR dgndb, Napi::Object obj) {
    BeJsConst inJson(obj);

    auto relClass = parseRelClass(dgndb, inJson);
    if (nullptr == relClass)
        throwSqlError();

    DgnElementId sourceId, targetId;
    sourceId.FromJson(inJson["sourceId"]);
    targetId.FromJson(inJson["targetId"]);

    ECN::StandaloneECRelationshipInstancePtr props = getRelationshipProperties(relClass, inJson, dgndb);

    BeSQLite::EC::ECInstanceKey relKey;
    auto rc = dgndb.InsertLinkTableRelationship(relKey, *relClass, sourceId, targetId, props.get()); // nullptr is okay if there are no props
    if (BE_SQLITE_OK != rc)
        throwSqlError();

    return Napi::String::New(Env(), relKey.GetInstanceId().ToHexStr());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void JsInterop::UpdateLinkTableRelationship(DgnDbR dgndb, Napi::Object obj)
    {
    BeJsConst inJson(obj);

    BeSQLite::EC::ECInstanceKey relKey = parseECRelationshipInstanceKeyKey(dgndb, inJson);
    auto relClass = parseRelClass(dgndb, inJson);
    if (nullptr == relClass)
        throwNotFound();

    ECN::StandaloneECRelationshipInstancePtr props = getRelationshipProperties(relClass, inJson, dgndb);
    if (!props.IsValid())
        return;

    auto stat = dgndb.UpdateLinkTableRelationshipProperties(relKey, *props);
    if (stat != BE_SQLITE_OK)
        ThrowJsException("error updating relationship");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void JsInterop::DeleteLinkTableRelationship(DgnDbR dgndb, Napi::Object inJson)
    {
    BeSQLite::EC::ECInstanceKey relKey = parseECRelationshipInstanceKeyKey(dgndb, inJson);
    auto stat = dgndb.DeleteLinkTableRelationship(relKey);
    if (stat != BE_SQLITE_DONE)
        ThrowJsException("error deleting relationship");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Napi::String JsInterop::InsertCodeSpec(DgnDbR db, Utf8StringCR name, BeJsConst jsonProperties)
    {
    CodeSpecPtr codeSpec = CodeSpec::Create(db, name.c_str(), jsonProperties);
    if (!codeSpec.IsValid())
        throwBadRequest();
    DgnDbStatus status = codeSpec->Insert();
    if (DgnDbStatus::Success != status)
        throwDgnDbStatus(status);
    return Napi::String::New(Env(), codeSpec->GetCodeSpecId().ToHexStr());
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void JsInterop::UpdateCodeSpec(DgnDbR db, CodeSpecId codeSpecId, BeJsConst jsonProperties)
{
    CodeSpecPtr codeSpec = db.CodeSpecs().GetForEdit(codeSpecId);
    if (!codeSpec.IsValid())
        throwBadRequest();

    codeSpec->FromPropertiesJson(jsonProperties);

    DgnDbStatus status = db.CodeSpecs().Update(*codeSpec);
    if (DgnDbStatus::Success != status)
        throwDgnDbStatus(status);
}

struct SetNapiObjOnModel {
    DgnModelR  m_model;
    SetNapiObjOnModel(DgnModelR model, NapiObjectCP obj) : m_model(model) {BeAssert(nullptr==model.m_napiObj); model.m_napiObj = obj;}
    ~SetNapiObjOnModel() {m_model.m_napiObj = nullptr;}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Napi::String JsInterop::InsertModel(DgnDbR dgndb, Napi::Object napiObj) {
    BeJsConst inJson(napiObj);
    DgnModel::CreateParams params(dgndb, inJson);
    if (!params.m_classId.IsValid())
        throwWrongClass();

    ModelHandlerP handler = dgn_ModelHandler::Model::FindHandler(dgndb, params.m_classId);
    if (nullptr == handler) {
        BeAssert(false);
        throwWrongClass();
    }

    DgnModelPtr model = handler->Create(params);
    if (!model.IsValid()) {
        BeAssert(false);
        throwBadArg();
    }

    model->FromJson(inJson);

    SetNapiObjOnModel _v(*model, &napiObj);
    DgnDbStatus status = model->Insert();
    if (DgnDbStatus::Success != status)
        BeNapi::ThrowJsException(Env(), "error inserting model", (int)status);

    return Napi::String::New(Env(), model->GetModelId().ToHexStr());
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus JsInterop::UpdateModelGeometryGuid(DgnDbR db, DgnModelId modelId)
    {
    if (!modelId.IsValid())
        return DgnDbStatus::InvalidId;

    auto model = db.Models().GetModel(modelId);
    if (model.IsNull())
        return DgnDbStatus::MissingId;

    if (!model->IsGeometricModel())
        return DgnDbStatus::WrongModel;

    auto stmt = db.GetGeometricModelUpdateStatement();
    if (stmt.IsNull())
        return DgnDbStatus::VersionTooOld;

    TxnManager::SetandRestoreIndirectChanges _v(db.Txns());
    stmt->BindGuid(1, BeGuid(true));
    stmt->BindId(2, modelId);
    auto success = BE_SQLITE_DONE == stmt->Step();

    return success ? DgnDbStatus::Success : DgnDbStatus::WriteError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void JsInterop::UpdateModel(DgnDbR dgndb, Napi::Object napiObj) {
    BeJsConst inJson(napiObj);
    DgnModelId mid = inJson[DgnModel::json_id()].GetId64<DgnModelId>();
    if (!mid.IsValid())
        throwInvalidId();

    DgnModelPtr model = dgndb.Models().GetModel(mid);
    if (!model.IsValid())
        throwMissingId();

    if (model->IsGeometricModel() && inJson.isMember("geometryChanged")) {
        auto stmt = dgndb.GetGeometricModelUpdateStatement();
        if (stmt.IsValid()) { // has the db been upgraded?
            TxnManager::SetandRestoreIndirectChanges _v(dgndb.Txns());
            stmt->BindGuid(1, BeGuid(true));
            stmt->BindId(2, mid);
            stmt->Step();
        }
    } else if (inJson.isMember("updateLastMod")) {
        auto stmt = dgndb.GetModelLastModUpdateStatement();
        if (stmt.IsValid()) {
            TxnManager::SetandRestoreIndirectChanges _v(dgndb.Txns());
            stmt->BindId(1, mid);
            stmt->Step();
        }
    }

    SetNapiObjOnModel _v(*model, &napiObj);
    model->FromJson(inJson);
    auto stat = model->Update();
    if (stat != DgnDbStatus::Success)
        BeNapi::ThrowJsException(Env(), "error updating model", (int)stat);
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void JsInterop::DeleteModel(DgnDbR dgndb, Utf8StringCR midStr) {
    DgnModelId mid(BeInt64Id::FromString(midStr.c_str()).GetValue());
    if (!mid.IsValid())
        throwInvalidId();

    DgnModelPtr model = dgndb.Models().GetModel(mid);
    if (!model.IsValid())
        throwMissingId();

    auto stat = model->Delete();
    if (stat != DgnDbStatus::Success)
        BeNapi::ThrowJsException(Env(), "error deleting model", (int) stat);
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus JsInterop::GetModel(Napi::Object modelObj, DgnDbR dgndb, BeJsConst inOpts) {
    DgnModelId modelId = inOpts[json_id()].GetId64<DgnModelId>();
    if (!modelId.IsValid()) {
        auto codeVal = inOpts[json_code()];
        if (codeVal.isNull())
            return DgnDbStatus::NotFound;

        modelId = dgndb.Models().QuerySubModelId(DgnCode::FromJson(codeVal, dgndb, false));
    }

    //  Look up the model
    auto model = dgndb.Models().GetModel(modelId);
    if (!model.IsValid())
        return DgnDbStatus::NotFound;

    model->ToJson(modelObj, inOpts);
    // Note: there are no auto-handled properties on DgnModels. Any such data goes on the modeled element.
    return DgnDbStatus::Success;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void JsInterop::QueryModelExtents(BeJsValue extentsJson, DgnDbR db, BeJsConst options) {
    DgnModelId modelId = options[json_id()].GetId64<DgnModelId>();
    if (!modelId.IsValid())
        throwInvalidId();

    DgnModelPtr model = db.Models().GetModel(modelId);
    if (!model.IsValid())
        throwNotFound();

    GeometricModelCP geometricModel = model->ToGeometricModel();
    if (!geometricModel)
        throwDgnDbStatus(DgnDbStatus::WrongModel);

    AxisAlignedBox3d extents = geometricModel->QueryElementsRange();
    if (!extents.IsValid())
        throwDgnDbStatus(DgnDbStatus::NoGeometry);

    extents.ToJson(extentsJson[json_modelExtents()]);
}

/*---------------------------------------------------------------------------------**//**
* TEMPORARY FUNCTION #TODO
* This method converts the stored vertical datum code to the effective vertical datum code.
* This fix is temporary till we can fix Power Platform to support correctly
* code zero that currently can be interpreted differently depending on the geodetic datum.
* Once Power Platform is fixed we can start to phase off value zero for fixed values
* not subject to interpretation.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
GeoCoordinates::VertDatumCode GetEffectiveVerticalDatumCode(DgnGCSCR gcs)
    {
    GeoCoordinates::VertDatumCode effectiveVerticalDatumCode = gcs.GetVerticalDatumCode();

    // Due to a design flaw in Power Platform value 0 (gdcFromDatum) is interpreted
    // differently depending on the geodetic datum.
    // Note that this flaw prevents dataset in the USA to use Ellipsoid vertical datum.
    if (effectiveVerticalDatumCode == GeoCoordinates::vdcFromDatum)
        {
        if (gcs.IsNAD83())
            effectiveVerticalDatumCode = GeoCoordinates::vdcNAVD88;
        else if (gcs.IsNAD27())
            effectiveVerticalDatumCode = GeoCoordinates::vdcNGVD29;
        else
            effectiveVerticalDatumCode = GeoCoordinates::vdcEllipsoid; // vdcFromDatum is reinterpreted as Ellipsoid if not NAD
        }

    return effectiveVerticalDatumCode;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void JsInterop::GetIModelProps(BeJsValue val, DgnDbCR dgndb) {
    // add the root subject, if available.
    auto rootSubject = dgndb.Elements().GetRootSubject();
    if (rootSubject.IsValid()) {
        auto subject = val[json_rootSubject()];
        subject[json_name()] = rootSubject->GetCode().GetValueUtf8CP();
        auto descr = rootSubject->GetDescription();
        if (!descr.empty())
            subject[json_description()] = descr;
    }

    auto& geolocation = dgndb.GeoLocation();

    // add project extents
     geolocation.GetProjectExtents().ToJson(val[json_projectExtents()]);

    // add global origin
    BeJsGeomUtils::DPoint3dToJson(val[json_globalOrigin()], geolocation.GetGlobalOrigin());

    auto gcs = geolocation.GetDgnGCS();

    // If a GCS exists use it to calculate an ECEF transform at the project center rather than using the stored transform.
    if (gcs != nullptr && gcs->IsValid())
        {
        // Here is the definition in old style JSON format.
        Json::Value CRSOld;
        gcs->ToJson(CRSOld, true);

        // Convert old style Json value to new style
        BeJsDocument theDoc(CRSOld.toStyledString());
        val[json_geographicCoordinateSystem()].From(theDoc);

        // Invalidate current EcefLocation in case it exists (this will force to recompute it using the new gcs)
        EcefLocation invalidLocation;
        geolocation.SetEcefLocation(invalidLocation);
        }


    auto ecefLocation = geolocation.GetEcefLocation();
    if (ecefLocation.m_isValid)
        ecefLocation.ToJson(val[json_ecefLocation()]);
}

//=======================================================================================
// @bsistruct
//=======================================================================================
struct BaseGCSCache : DgnDb::AppData {
private:
    std::map<Utf8String, GeoCoordinates::BaseGCSPtr> m_gcs;

    static BaseGCSCache& Get(DgnDbCR db) {
        static Key s_key;
        return *db.ObtainAppData(s_key, []() { return new BaseGCSCache(); });
    }

    GeoCoordinates::BaseGCSP GetGCSP(Utf8String gcsKey) {
        // Find GCS in cache
        auto iter = m_gcs.find(gcsKey);
        if (m_gcs.end() != iter)
            return iter->second.get();

        // Not in cache ... we create then add
        auto newGcs = GeoCoordinates::BaseGCS::CreateGCS();

        Utf8String errorMessage;
        if (BentleyStatus::BSISUCCESS != newGcs->FromJson(BeJsDocument(gcsKey), errorMessage)) {
            // Assuming a JSON was not provided then it can be the name of a geodetic datum.
            if (BentleyStatus::BSISUCCESS != newGcs->InitLatLong(nullptr, gcsKey.c_str(), nullptr, "DEGREE", 0.0, 0.0))
                return nullptr;

            newGcs->SetVerticalDatumCode(GeoCoordinates::vdcEllipsoid);
            newGcs->SetReprojectElevation(true);
        }

        m_gcs.insert(std::pair<Utf8String, GeoCoordinates::BaseGCSPtr>(gcsKey, newGcs));

        return newGcs.get();
    }

public:
    static GeoCoordinates::BaseGCSP GetGCSP(Utf8String gcsString, DgnDbCR db) {
        return BaseGCSCache::Get(db).GetGCSP(gcsString);
    }
};


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void populateGeoCoordResult(BeJsValue result, bvector<DPoint3d> const& points, bvector<ReprojectStatus> const& statuses)
    {
    for (size_t i = 0; i < points.size(); i++)
        {
        auto outputPointWithStatus = result[static_cast<Json::ArrayIndex>(i)];
        auto outputPoint = outputPointWithStatus["p"];

        outputPoint[0] = points[i].x;
        outputPoint[1] = points[i].y;
        outputPoint[2] = points[i].z;

        outputPointWithStatus["s"] = statuses[i];
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus JsInterop::GetGeoCoordsFromIModelCoords(BeJsValue results, DgnDbR dgnDb, BeJsConst props)
    {
    // get the vector of points.
    bvector<DPoint3d> iModelPoints;
    BeJsGeomUtils::DPoint3dVectorFromJson(iModelPoints, props[json_iModelCoords()]);

    // create return vectors.
    bvector<DPoint3d> geoPoints(iModelPoints.size());
    bvector<ReprojectStatus> statusList(geoPoints.size());

    // get the GCS
    Utf8String target = props[json_target()].asString();
    GeoCoordinates::BaseGCSP targetGCS = nullptr;

    auto gcs = dgnDb.GeoLocation().GetDgnGCS();

    bool targetValid = true;
    if (target.length() != 0)
        {
        targetGCS = BaseGCSCache::GetGCSP(target, dgnDb);

        if (nullptr == targetGCS)
            targetValid = false;
        }

    // If targetGCS is nullptr and target valid at this point we only want cartesian to latitude/longitude conversion switched below

    auto outputStatus = statusList.begin();
    for (auto input = iModelPoints.begin(), output = geoPoints.begin(); input != iModelPoints.end(); input++, output++, outputStatus++)
        {
        if (nullptr != gcs && gcs->IsValid())
            {
            if (nullptr != targetGCS)
                {
                DPoint3d targetCartesian;
                gcs->CartesianFromUors(targetCartesian, *input);
                *outputStatus = gcs->CartesianFromCartesian(*output, targetCartesian, *targetGCS);
                }
            else
                {
                if (targetValid)
                    *outputStatus = gcs->LatLongFromUors((GeoPointR) *output, *input);
                else
                    *outputStatus = ReprojectStatus::REPROJECT_BadArgument; // Invalid target
                }
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
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus JsInterop::GetIModelCoordsFromGeoCoords(BeJsValue results, DgnDbR dgnDb, BeJsConst props)    {
    // get the vector of points.
    bvector<DPoint3d> geoPoints;
    BeJsGeomUtils::DPoint3dVectorFromJson(geoPoints, props[json_geoCoords()]);

    // create return vectors.
    bvector<DPoint3d> iModelPoints(geoPoints.size());
    bvector<ReprojectStatus> statusList(geoPoints.size());

    // Note that the sourceGCS will be null if the source was empty (indicating transform to self lat/long)
    auto gcs = dgnDb.GeoLocation().GetDgnGCS();

    // get the GCS
    Utf8String source = props[json_source()].asString();
    GeoCoordinates::BaseGCSP sourceGCS = nullptr;

    bool sourceValid = true;
    if (source.length() != 0)
        {
        sourceGCS = BaseGCSCache::GetGCSP(source, dgnDb);
        if (nullptr == sourceGCS)
            sourceValid = false;
        }

    // If sourceGCS is nullptr and source valid at this point we only want cartesian to latitude/longitude conversion switched below

    auto outputStatus = statusList.begin();
    for (auto input = geoPoints.begin(), output = iModelPoints.begin(); input != geoPoints.end(); ++input, ++output, ++outputStatus)
        {
        if (nullptr != gcs && gcs->IsValid())
            {
            DPoint3d targetCartesian;
            // get point into the correct Datum.
            if (nullptr != sourceGCS)
                {
                *outputStatus = sourceGCS->CartesianFromCartesian(targetCartesian, *input, *gcs);
                gcs->UorsFromCartesian(*output, targetCartesian);
                }
            else
                {
                if (sourceValid)
                    *outputStatus = gcs->UorsFromLatLong(*output, (GeoPointCR) *input);
                else
                    *outputStatus = ReprojectStatus::REPROJECT_BadArgument; // Invalid source
                }
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
* @bsimethod
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus JsInterop::QueryDefinitionElementUsage(BeJsValue value, DgnDbR db, bvector<Utf8String> const& idStringArray)
    {
    BeSQLite::IdSet<DgnElementId> idSet;
    for (Utf8String idString : idStringArray)
        {
        DgnElementId elementId;
        if (SUCCESS == BeInt64Id::FromString(elementId, idString.c_str()))
            idSet.insert(elementId);
        }

    DefinitionElementUsageInfoPtr usageInfo = DefinitionElementUsageInfo::Create(db, idSet);
    if (!usageInfo.IsValid())
        return DgnDbStatus::BadRequest;

    usageInfo->ToJson(value);
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void JsInterop::WriteFullElementDependencyGraphToFile(DgnDbR db, Utf8StringCR dotFileName)
    {
    ElementDependency::Graph graph(db.Txns());
    graph.WriteDot(BeFileName(dotFileName.c_str(), true), bvector<bvector<uint64_t>>());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void JsInterop::WriteAffectedElementDependencyGraphToFile(DgnDbR db, Utf8StringCR dotFileName, bvector<Utf8String> const& id64Array)
    {
    bvector<DgnElementId> changedIds;
    for (auto const& id64str: id64Array)
        {
        DgnElementId eid;
        DgnElementId::FromString(eid, id64str.c_str());
        changedIds.push_back(eid);
        }

    ElementDependency::Graph graph(db.Txns());
    graph.WriteAffectedGraphToFile(BeFileName(dotFileName.c_str(), true), changedIds, {});
    }
