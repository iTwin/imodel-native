/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <GeomSerialization/GeomLibsSerialization.h>
#include <GeomSerialization/GeomLibsJsonSerialization.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus ECSqlRowAdaptor::RenderRow(BeJsValue rowJson, IECSqlRow const& stmt, bool asArray) const {
    if (asArray) {
        rowJson.SetEmptyArray();
        const int count = stmt.GetColumnCount();
        int consecutiveNulls = 0;
        for (int columnIndex = 0; columnIndex < count; columnIndex++) {
            IECSqlValue const& ecsqlValue = stmt.GetValue(columnIndex);
            if (m_options.SkipReadOnlyProperties() && ecsqlValue.GetColumnInfo().GetProperty() && ecsqlValue.GetColumnInfo().GetProperty()->GetIsReadOnly()) {
                continue;
            }

            if (ecsqlValue.IsNull()) {
                ++consecutiveNulls;
                continue;
            }

            while (consecutiveNulls > 0) {
                rowJson.appendValue().SetNull();
                --consecutiveNulls;
            }
            if (SUCCESS != RenderRootProperty(rowJson.appendValue(), ecsqlValue))
                return ERROR;
        }
    } else {
        rowJson.SetEmptyObject();
        const int count = stmt.GetColumnCount();
        for (int columnIndex = 0; columnIndex < count; columnIndex++) {
            IECSqlValue const& ecsqlValue = stmt.GetValue(columnIndex);
            if (m_options.SkipReadOnlyProperties() && ecsqlValue.GetColumnInfo().GetProperty() && ecsqlValue.GetColumnInfo().GetProperty()->GetIsReadOnly()) {
                continue;
            }
            if (ecsqlValue.IsNull()) {
                continue;
            }

            const auto memberProp = ecsqlValue.GetColumnInfo().GetProperty();
            if (m_options.UseJsNames()) {
                const auto prim = memberProp->GetAsPrimitiveProperty();
                Utf8String memberName = memberProp->GetName();
                if (prim && !prim->GetExtendedTypeName().empty()) {
                    const auto extendTypeId = ExtendedTypeHelper::GetExtendedType(prim->GetExtendedTypeName());
                    if (extendTypeId == ExtendedTypeHelper::ExtendedType::Id && memberName.EqualsIAscii(ECDBSYS_PROP_ECInstanceId))
                        memberName = ECN::ECJsonSystemNames::Id();
                    else if(extendTypeId == ExtendedTypeHelper::ExtendedType::ClassId && memberName.EqualsIAscii(ECDBSYS_PROP_ECClassId))
                        memberName = m_options.UseClassFullNameInsteadofClassName() ?  ECN::ECJsonSystemNames::ClassFullName() : ECN::ECJsonSystemNames::ClassName();
                    else if(extendTypeId == ExtendedTypeHelper::ExtendedType::SourceId && memberName.EqualsIAscii(ECDBSYS_PROP_SourceECInstanceId))
                        memberName = ECN::ECJsonSystemNames::SourceId();
                    else if(extendTypeId == ExtendedTypeHelper::ExtendedType::SourceClassId && memberName.EqualsIAscii(ECDBSYS_PROP_SourceECClassId))
                        memberName = ECN::ECJsonSystemNames::SourceClassName();
                    else if(extendTypeId == ExtendedTypeHelper::ExtendedType::TargetId && memberName.EqualsIAscii(ECDBSYS_PROP_TargetECInstanceId))
                        memberName = ECN::ECJsonSystemNames::TargetId();
                    else if(extendTypeId == ExtendedTypeHelper::ExtendedType::TargetClassId && memberName.EqualsIAscii(ECDBSYS_PROP_TargetECClassId))
                        memberName = ECN::ECJsonSystemNames::TargetClassName();
                    else
                        ECN::ECJsonUtilities::LowerFirstChar(memberName);
                } else {
                    ECN::ECJsonUtilities::LowerFirstChar(memberName);
                }

                if (m_skipPropertyHandler && m_skipPropertyHandler(*memberProp))
                    continue;

                if (SUCCESS != RenderRootProperty(rowJson[memberName], ecsqlValue))
                    return ERROR;
            } else {
                if (SUCCESS != RenderRootProperty(rowJson[memberProp->GetName()], ecsqlValue))
                    return ERROR;
            }
        }
    }
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus ECSqlRowAdaptor::RenderRootProperty(BeJsValue out, IECSqlValue const& in) const {
    if (m_customHandler != nullptr) {
        if (m_customHandler(out, in) == PropertyHandlerResult::Handled)
            return SUCCESS;
    }
    return RenderProperty(out, in);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus ECSqlRowAdaptor::RenderProperty(BeJsValue out, IECSqlValue const& in) const {
    auto prop = in.GetColumnInfo().GetProperty();
    if (prop == nullptr) {
        BeAssert(false && "property is null");
        return ERROR;
    }
    if (prop->GetIsPrimitive())
        return RenderPrimitiveProperty(out, in, nullptr);
    if (prop->GetIsStruct())
        return RenderStructProperty(out, in);
    if (prop->GetIsNavigation())
        return RenderNavigationProperty(out, in);
    if (prop->GetIsPrimitiveArray())
        return RenderPrimitiveArrayProperty(out, in);
    if (prop->GetIsStructArray())
        return RenderStructArrayProperty(out, in);
    BeAssert(false && "property type unsupported");
    return ERROR;
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus ECSqlRowAdaptor::RenderPrimitiveProperty(BeJsValue out, IECSqlValue const& in, ECN::PrimitiveType const* type) const {
    ECN::PrimitiveECPropertyCP prop = nullptr;
    ECN::PrimitiveType propType = Enum::FromInt<ECN::PrimitiveType>(0);
    if (type != nullptr) {
        propType = *type;
    } else {
        auto rootProp = in.GetColumnInfo().GetProperty();
        if (rootProp != nullptr) {
            prop = rootProp->GetAsPrimitiveProperty();
            propType = prop->GetType();
        } else {
            BeAssert("developer error");
            return ERROR;
        }
    }
    if (propType == ECN::PRIMITIVETYPE_Long) {
        return RenderLong(out, in, prop);
    }
    if (propType == ECN::PRIMITIVETYPE_String) {
        out = in.GetText();
        return SUCCESS;
    }
    if (propType == ECN::PRIMITIVETYPE_Double) {
        out = in.GetDouble();
        return SUCCESS;
    }
    if (propType == ECN::PRIMITIVETYPE_Integer) {
        out = in.GetInt64();
        return SUCCESS;
    }
    if (propType == ECN::PRIMITIVETYPE_Boolean) {
        out = in.GetBoolean();
        return SUCCESS;
    }
    if (propType == ECN::PRIMITIVETYPE_Binary) {
        return RenderBinaryProperty(out, in);
    }
    if (propType == ECN::PRIMITIVETYPE_DateTime) {
        out = in.GetDateTime().ToString();
        return SUCCESS;
    }
    if (propType == ECN::PRIMITIVETYPE_Point2d) {
        return RenderPoint2d(out, in);
    }
    if (propType == ECN::PRIMITIVETYPE_Point3d) {
         return RenderPoint3d(out, in);
    }
    if (propType == ECN::PRIMITIVETYPE_IGeometry){
        return RenderGeometryProperty(out, in);
    }
    BeAssert(false && "property type unsupported");
    return ERROR;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static bool IsClassIdProperty(Utf8StringCR propertyName) {
    static const auto classIdProperties = std::set<Utf8String,CompareIUtf8Ascii> {
        ECDBSYS_PROP_ECClassId,
        ECDBSYS_PROP_SourceECClassId,
        ECDBSYS_PROP_TargetECClassId,
        ECDBSYS_PROP_NavPropRelECClassId
    };
    return classIdProperties.find(propertyName) != classIdProperties.end();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus ECSqlRowAdaptor::RenderLong(BeJsValue out, IECSqlValue const& in, ECN::PrimitiveECPropertyCP prop) const {
    if (prop != nullptr) {
        const auto id = in.GetId<ECN::ECClassId>();
        const auto extendTypeId = ExtendedTypeHelper::GetExtendedType(prop->GetExtendedTypeName());
        const auto isClassId = Enum::Intersects<ExtendedTypeHelper::ExtendedType>(extendTypeId, ExtendedTypeHelper::ExtendedType::ClassIds);
        const auto isId = Enum::Intersects<ExtendedTypeHelper::ExtendedType>(extendTypeId, ExtendedTypeHelper::ExtendedType::Ids);
        if (isClassId) {
            if (!id.IsValid()) {
                return SUCCESS;
            }
            if(m_options.DoNotConvertClassIdsToClassNamesWhenAliased() && !IsClassIdProperty(prop->GetName())) {
                out = id.ToHexStr();
                return SUCCESS;
            }
            if (m_options.ConvertClassIdsToClassNames() || m_options.UseJsNames()) {
                auto classCP = m_ecdb.Schemas().GetClass(id, in.GetColumnInfo().GetRootClass().GetTableSpace().c_str());
                if (classCP != nullptr) {
                    ECN::ECJsonUtilities::ClassNameToJson(out, *classCP, m_options.UseClassFullNameInsteadofClassName());
                }
            }
            out = id.ToHexStr();
            return SUCCESS;
        } else if (isId) {
            if (!id.IsValid()) {
                return SUCCESS;
            }
            out = id.ToHexStr();
            return SUCCESS;
        }
    }
    out = std::trunc(in.GetDouble());
    return SUCCESS;
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus ECSqlRowAdaptor::RenderPoint3d(BeJsValue out, IECSqlValue const& in) const {
    const auto pt = in.GetPoint3d();
    out.SetEmptyObject();
    if (m_options.UseJsNames()) {
        out[ECN::ECJsonSystemNames::Point::X()] = pt.x;
        out[ECN::ECJsonSystemNames::Point::Y()] = pt.y;
        out[ECN::ECJsonSystemNames::Point::Z()] = pt.z;
    } else {
        out[ECDBSYS_PROP_PointX] = pt.x;
        out[ECDBSYS_PROP_PointY] = pt.y;
        out[ECDBSYS_PROP_PointZ] = pt.z;
    }
    return SUCCESS;
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus ECSqlRowAdaptor::RenderPoint2d(BeJsValue out, IECSqlValue const& in) const {
    const auto pt = in.GetPoint2d();
    out.SetEmptyObject();
    if (m_options.UseJsNames()) {
        out[ECN::ECJsonSystemNames::Point::X()] = pt.x;
        out[ECN::ECJsonSystemNames::Point::Y()] = pt.y;
    } else {
        out[ECDBSYS_PROP_PointX] = pt.x;
        out[ECDBSYS_PROP_PointY] = pt.y;
    }
    return SUCCESS;
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus ECSqlRowAdaptor::RenderGeometryProperty(BeJsValue out, IECSqlValue const& in) const {
    IGeometryPtr geom = in.GetGeometry();
    if (geom == nullptr)
        return ERROR;

    Utf8String jsonStr;
    if (!IModelJson::TryGeometryToIModelJsonString(jsonStr, *geom) || jsonStr.empty())
        return ERROR;

    rapidjson::Document jsonDoc;
    if (jsonDoc.Parse<0>(jsonStr.c_str()).HasParseError())
        return ERROR;

    out.From(jsonDoc);
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus ECSqlRowAdaptor::RenderBinaryProperty(BeJsValue out, IECSqlValue const& in) const {
    bool isGuid = false;
    if (in.GetColumnInfo().GetProperty() != nullptr) {
        const auto prop = in.GetColumnInfo().GetProperty()->GetAsPrimitiveProperty();
        isGuid = !prop->GetExtendedTypeName().empty() && prop->GetExtendedTypeName().EqualsIAscii("BeGuid");
    }

    int size = 0;
    Byte const* data = (Byte const*)in.GetBlob(&size);
    if (isGuid && size == sizeof(BeGuid)) {
        BeGuid guid;
        std::memcpy(&guid, data, sizeof(guid));
        out = guid.ToString().c_str();
        return SUCCESS;
    }

    // Abbreviate blobs as a json of their size; i.e., "{bytes:123}"
    if (m_options.AbbreviateBlobs()) {
        Utf8String outString;
        outString.Sprintf("{\"bytes\":%" PRId32 "}", size);
        out = outString.c_str();
        return SUCCESS;
    }

    out.SetBinary(data, (size_t)size);
    return SUCCESS;
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus ECSqlRowAdaptor::RenderNavigationProperty(BeJsValue out, IECSqlValue const& in) const {
    out.SetEmptyObject();
    const auto jsId = m_options.UseJsNames() ? ECN::ECJsonSystemNames::Navigation::Id() : ECDBSYS_PROP_NavPropId;
    const auto jsClassId = m_options.UseJsNames() ? ECN::ECJsonSystemNames::Navigation::RelClassName() : ECDBSYS_PROP_NavPropRelECClassId;
    auto const& navIdVal = in[ECDBSYS_PROP_NavPropId];
    if (navIdVal.IsNull())
        return SUCCESS;

    out[jsId] = navIdVal.GetId<ECInstanceId>().ToHexStr();
    auto const& relClassIdVal = in[ECDBSYS_PROP_NavPropRelECClassId];
    if (!relClassIdVal.IsNull()) {
        const auto classId = relClassIdVal.GetId<ECN::ECClassId>();
        if (m_options.ConvertClassIdsToClassNames() || m_options.UseJsNames()) {
            auto classCP = m_ecdb.Schemas().GetClass(classId, in.GetColumnInfo().GetRootClass().GetTableSpace().c_str());
            if (classCP != nullptr) {
                ECN::ECJsonUtilities::ClassNameToJson(out[jsClassId], *classCP);
                return SUCCESS;
            }
        }
        out[jsClassId] = classId.ToHexStr();
    }
    return SUCCESS;
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus ECSqlRowAdaptor::RenderStructProperty(BeJsValue out, IECSqlValue const& in) const {
    out.SetEmptyObject();
    for (IECSqlValue const& structMemberValue : in.GetStructIterable()) {
        if (structMemberValue.IsNull())
            continue;

        auto memberProp = structMemberValue.GetColumnInfo().GetProperty();
        if (m_options.UseJsNames()) {
            Utf8String memberName = memberProp->GetName();
            ECN::ECJsonUtilities::LowerFirstChar(memberName);
            if (SUCCESS != RenderProperty(out[memberName], structMemberValue))
                return ERROR;
        } else {
            if (SUCCESS != RenderProperty(out[memberProp->GetName()], structMemberValue))
                return ERROR;
        }
    }
    return SUCCESS;
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus ECSqlRowAdaptor::RenderPrimitiveArrayProperty(BeJsValue out, IECSqlValue const& in) const {
    out.SetEmptyArray();
    auto elementType  = in.GetColumnInfo().GetProperty()->GetAsPrimitiveArrayProperty()->GetPrimitiveElementType();
    for (IECSqlValue const& arrayElementValue : in.GetArrayIterable()) {
        if (arrayElementValue.IsNull())
            continue;

        if (SUCCESS != RenderPrimitiveProperty(out.appendValue(), arrayElementValue, &elementType))
            return ERROR;
    }
    return SUCCESS;
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus ECSqlRowAdaptor::RenderStructArrayProperty(BeJsValue out, IECSqlValue const& in) const {
    out.SetEmptyArray();
    for (IECSqlValue const& arrayElementValue : in.GetArrayIterable()) {
        if (arrayElementValue.IsNull()){
            out.appendValue().SetEmptyObject();
            continue;
        }

        if (SUCCESS != RenderStructProperty(out.appendValue(), arrayElementValue))
            return ERROR;
    }
    return SUCCESS;
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ECSqlRowAdaptor::GetMetaData(ECSqlRowProperty::List& list, ECSqlStatement const& stmt) const {
    const int count = stmt.GetColumnCount();
    using ExtendedType = ExtendedTypeHelper::ExtendedType;
    std::map<std::string, int> uniqueJsMembers;
    std::map<std::string, int> uniqueECSqlMembers;
    for (int i = 0; i < count; i++) {
        Utf8String jsName;
        auto const& val = stmt.GetValue(i);
        auto const& col = val.GetColumnInfo();
        auto const prop = col.GetProperty();
        Utf8String extendType = prop->GetIsPrimitive() ? prop->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str() : "";
        auto const extendedTypeId = ExtendedTypeHelper::GetExtendedType(extendType);
        auto const isSystem = Enum::Intersects<ExtendedType>(extendedTypeId, ExtendedType::All);
        std::string className = col.IsGeneratedProperty() || isSystem ? "" : prop->GetClass().GetFullName();
        std::string typeName = col.GetDataType().IsNavigation() ? "navigation" : prop->GetTypeFullName();
        std::string name = col.IsGeneratedProperty() ? prop->GetDisplayLabel() : prop->GetName();
        std::string accessString = col.IsGeneratedProperty() ? prop->GetDisplayLabel() : col.GetPropertyPath().ToString();

        // Blobs are abbreviated as a Json string with info about the blob. See RenderBinaryProperty().
        if (m_options.AbbreviateBlobs() && typeName == "binary") {
            typeName = "string";
            extendType = "Json";
        }

        if (col.IsGeneratedProperty()) {
            jsName.assign(prop->GetDisplayLabel());
            if (jsName.empty()) {
                jsName.assign(prop->GetName());
            }
            ECN::ECJsonUtilities::LowerFirstChar(jsName);
        } else if (isSystem) {
            if (col.GetPropertyPath().Size() > 1)  {
                T_Utf8StringVector accessStringV;
                for (auto const* it : col.GetPropertyPath())
                    accessStringV.push_back(it->GetProperty()->GetName().c_str());
                Utf8String tmp = accessStringV.front() + ".";
                for (int j = 1; j < accessStringV.size() - 1; ++j)
                    tmp += accessStringV[j] + ".";

                auto &leafEntry = accessStringV.back();
                if (leafEntry == ECDBSYS_PROP_NavPropId)
                    tmp += ECN::ECJsonSystemNames::Navigation::Id();
                else if (leafEntry == ECDBSYS_PROP_NavPropRelECClassId)
                    tmp += ECN::ECJsonSystemNames::Navigation::RelClassName();
                else if (leafEntry == ECDBSYS_PROP_PointX)
                    tmp += ECN::ECJsonSystemNames::Point::X();
                else if (leafEntry == ECDBSYS_PROP_PointY)
                    tmp += ECN::ECJsonSystemNames::Point::Y();
                else if (leafEntry == ECDBSYS_PROP_PointZ)
                    tmp += ECN::ECJsonSystemNames::Point::Z();
                else
                    tmp += leafEntry;

                jsName = tmp;
                ECN::ECJsonUtilities::LowerFirstChar(jsName);
            } else {
                jsName.assign(prop->GetName());
                if (extendedTypeId == ExtendedTypeHelper::ExtendedType::Id && jsName.EqualsIAscii(ECDBSYS_PROP_ECInstanceId))
                    jsName = ECN::ECJsonSystemNames::Id();
                else if(extendedTypeId == ExtendedTypeHelper::ExtendedType::ClassId && jsName.EqualsIAscii(ECDBSYS_PROP_ECClassId))
                    jsName = m_options.UseClassFullNameInsteadofClassName() ? ECN::ECJsonSystemNames::ClassFullName(): ECN::ECJsonSystemNames::ClassName();
                else if(extendedTypeId == ExtendedTypeHelper::ExtendedType::SourceId && jsName.EqualsIAscii(ECDBSYS_PROP_SourceECInstanceId))
                    jsName = ECN::ECJsonSystemNames::SourceId();
                else if(extendedTypeId == ExtendedTypeHelper::ExtendedType::SourceClassId && jsName.EqualsIAscii(ECDBSYS_PROP_SourceECClassId))
                    jsName = ECN::ECJsonSystemNames::SourceClassName();
                else if(extendedTypeId == ExtendedTypeHelper::ExtendedType::TargetId && jsName.EqualsIAscii(ECDBSYS_PROP_TargetECInstanceId))
                    jsName = ECN::ECJsonSystemNames::TargetId();
                else if(extendedTypeId == ExtendedTypeHelper::ExtendedType::TargetClassId && jsName.EqualsIAscii(ECDBSYS_PROP_TargetECClassId))
                    jsName = ECN::ECJsonSystemNames::TargetClassName();
                else if(extendedTypeId == ExtendedTypeHelper::ExtendedType::NavId && jsName.EqualsIAscii(ECDBSYS_PROP_NavPropId))
                    jsName = ECN::ECJsonSystemNames::Navigation::Id();
                else if(extendedTypeId == ExtendedTypeHelper::ExtendedType::NavRelClassId && jsName.EqualsIAscii(ECDBSYS_PROP_NavPropRelECClassId))
                    jsName = ECN::ECJsonSystemNames::Navigation::RelClassName();
                else
                    ECN::ECJsonUtilities::LowerFirstChar(jsName);
            }
        } else {
            jsName = col.GetPropertyPath().ToString();
            ECN::ECJsonUtilities::LowerFirstChar(jsName);
        }
        if (uniqueJsMembers.find(jsName) == uniqueJsMembers.end()) {
            uniqueJsMembers[jsName] = 0;
        } else {
            uniqueJsMembers[jsName]++;
            Utf8String suffix;
            suffix.Sprintf("_%d", uniqueJsMembers[jsName]);
            jsName.append(suffix);
        }
        if (uniqueECSqlMembers.find(name) == uniqueECSqlMembers.end()) {
            uniqueECSqlMembers[name] = 0;
        } else {
            uniqueECSqlMembers[name]++;
            Utf8String suffix;
            suffix.Sprintf("_%d", uniqueECSqlMembers[name]);
            name.append(suffix);
        }
        list.append(className, accessString, jsName, name, typeName, col.IsGeneratedProperty(), extendType, i);
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void JsReadOptions::FromJson(BeJsValue opts) {
    if (opts.isBoolMember(JAbbreviateBlobs))
        m_abbreviateBlobs = opts[JAbbreviateBlobs].asBool();

    if (opts.isBoolMember(JClassIdsToClassNames))
        m_classIdToClassNames = opts[JClassIdsToClassNames].asBool();

    if (opts.isBoolMember(JUseJsName))
        m_useJsName = opts[JUseJsName].asBool();

    if (opts.isBoolMember(JDoNotConvertClassIdsToClassNamesWhenAliased))
        m_doNotConvertClassIdsToClassNamesWhenAliased = opts[JDoNotConvertClassIdsToClassNamesWhenAliased].asBool();

    if (opts.isBoolMember(JSkipReadOnlyProperties))
        m_skipReadOnlyProperties = opts[JSkipReadOnlyProperties].asBool();

    if (opts.isBoolMember(JUseClassFullNameInsteadofClassName))
        m_useClassFullNameInsteadofClassName = opts[JUseClassFullNameInsteadofClassName].asBool();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void JsReadOptions::ToJson(BeJsValue opts) const {
    opts.SetEmptyObject();
    opts[JAbbreviateBlobs] = m_abbreviateBlobs;
    opts[JClassIdsToClassNames] = m_classIdToClassNames;
    opts[JUseJsName] = m_useJsName;
    opts[JDoNotConvertClassIdsToClassNamesWhenAliased] = m_doNotConvertClassIdsToClassNamesWhenAliased;
    opts[JSkipReadOnlyProperties] = m_skipReadOnlyProperties;
    opts[JUseClassFullNameInsteadofClassName] = m_useClassFullNameInsteadofClassName;
}

END_BENTLEY_SQLITE_EC_NAMESPACE