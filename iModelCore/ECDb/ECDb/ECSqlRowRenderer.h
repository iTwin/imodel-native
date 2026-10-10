/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/ECSqlStatement.h>
#include <ECObjects/ECJsonUtilities.h>
#include <BeRapidJson/BeRapidJson.h>
#include "ECDbInternalTypes.h"
#include "ECDbSystemSchemaHelper.h"
#include <algorithm>
#include <charconv>
#include <cmath>
#include <set>
#include <utility>
#include <vector>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

struct ECSqlLongRenderer final {
    enum class Mode { Number, Id, ClassName };
    struct LastClassName {
        ECN::ECClassId m_id;
        Utf8String m_name;
    };

    template <typename Sink>
    static bool RenderHexId(Sink& sink, BeInt64Id id) {
        if (!id.IsValid())
            return sink.String("0");
        char buffer[BeInt64Id::ID_STRINGBUFFER_LENGTH];
        buffer[0] = '0';
        buffer[1] = 'x';
        auto converted = std::to_chars(buffer + 2, buffer + sizeof(buffer) - 1, id.GetValueUnchecked(), 16);
        if (converted.ec != std::errc())
            return false;
        *converted.ptr = '\0';
        return sink.String(buffer);
    }

    static Mode GetMode(ECN::PrimitiveECPropertyCP prop, JsReadOptions const& options) {
        if (prop == nullptr)
            return Mode::Number;
        auto extendedType = ExtendedTypeHelper::GetExtendedType(prop->GetExtendedTypeName());
        if (Enum::Intersects<ExtendedTypeHelper::ExtendedType>(extendedType, ExtendedTypeHelper::ExtendedType::ClassIds)) {
            static const std::set<Utf8String, CompareIUtf8Ascii> classIdProperties {
                ECDBSYS_PROP_ECClassId, ECDBSYS_PROP_SourceECClassId,
                ECDBSYS_PROP_TargetECClassId, ECDBSYS_PROP_NavPropRelECClassId
            };
            if (options.DoNotConvertClassIdsToClassNamesWhenAliased() && classIdProperties.find(prop->GetName()) == classIdProperties.end())
                return Mode::Id;
            return options.ConvertClassIdsToClassNames() || options.UseJsNames() ? Mode::ClassName : Mode::Id;
        }
        return Enum::Intersects<ExtendedTypeHelper::ExtendedType>(extendedType, ExtendedTypeHelper::ExtendedType::Ids) ? Mode::Id : Mode::Number;
    }

    template <typename Sink>
    static bool Render(Sink& sink, IECSqlValue const& value, Mode mode, ECDbCR ecdb,
                       Utf8CP tableSpace, bool useColon, LastClassName* cached = nullptr) {
        if (mode == Mode::Number)
            return sink.Double(std::trunc(value.GetDouble()));
        auto id = value.GetId<ECN::ECClassId>();
        if (!id.IsValid())
            return sink.Null();
        return RenderId(sink, id, mode == Mode::ClassName, ecdb, tableSpace, useColon, cached);
    }

    template <typename Sink>
    static bool RenderId(Sink& sink, ECN::ECClassId id, bool convert, ECDbCR ecdb,
                         Utf8CP tableSpace, bool useColon, LastClassName* cached = nullptr) {
        if (convert) {
            if (cached != nullptr && cached->m_id.IsValid() && cached->m_id == id)
                return sink.String(cached->m_name.c_str());
            auto classCP = ecdb.Schemas().GetClass(id, tableSpace);
            if (classCP != nullptr) {
                auto name = ECN::ECJsonUtilities::FormatClassName(*classCP, useColon);
                if (cached != nullptr) {
                    cached->m_id = id;
                    cached->m_name = std::move(name);
                    return sink.String(cached->m_name.c_str());
                }
                return sink.String(name.c_str());
            }
        }
        return RenderHexId(sink, id);
    }
};

struct PreparedECSqlRowRenderer final {
private:
    enum class Kind { Primitive, Navigation, Struct, PrimitiveArray, StructArray };
    struct Column {
        Kind m_kind = Kind::Primitive;
        ECN::PrimitiveType m_type = ECN::PRIMITIVETYPE_String;
        ECSqlLongRenderer::Mode m_longMode = ECSqlLongRenderer::Mode::Number;
        Utf8String m_tableSpace;
        Utf8String m_memberName;
        Utf8String m_jsonName;
        std::vector<Column> m_members;
        bool m_skip = false;
        ECSqlLongRenderer::LastClassName m_className;
    };
    std::vector<Column> m_columns;
    JsReadOptions m_options;
    bool m_initialized = false;
    bool m_eligible = false;

    static bool IsSupportedPrimitive(ECN::PrimitiveType type) {
        switch (type) {
            case ECN::PRIMITIVETYPE_String:
            case ECN::PRIMITIVETYPE_Double:
            case ECN::PRIMITIVETYPE_Integer:
            case ECN::PRIMITIVETYPE_Boolean:
            case ECN::PRIMITIVETYPE_Long:
            case ECN::PRIMITIVETYPE_Point2d:
            case ECN::PRIMITIVETYPE_Point3d:
                return true;
            default:
                return false;
        }
    }

    bool PrepareStruct(Column& column, ECN::ECStructClassCR type, Utf8StringCR tableSpace,
                       bool fromArray, std::vector<ECN::ECStructClassCP>& ancestors) {
        if (std::find(ancestors.begin(), ancestors.end(), &type) != ancestors.end())
            return false;
        ancestors.push_back(&type);
        for (auto prop : type.GetProperties(true)) {
            Column member;
            if (!PrepareColumn(member, *prop, tableSpace, fromArray, ancestors)) {
                ancestors.pop_back();
                return false;
            }
            member.m_memberName = prop->GetName();
            member.m_jsonName = member.m_memberName;
            if (m_options.UseJsNames())
                ECN::ECJsonUtilities::LowerFirstChar(member.m_jsonName);
            column.m_members.push_back(std::move(member));
        }
        ancestors.pop_back();
        // Native struct fields iterate a case-insensitive map; JSON-backed array structs use schema order.
        if (!fromArray) {
            std::sort(column.m_members.begin(), column.m_members.end(), [](Column const& lhs, Column const& rhs) {
                return CompareIUtf8Ascii()(lhs.m_memberName, rhs.m_memberName);
            });
        }
        return true;
    }

    bool PrepareColumn(Column& column, ECN::ECPropertyCR prop, Utf8StringCR tableSpace,
                       bool fromArray, std::vector<ECN::ECStructClassCP>& ancestors) {
        if (auto primitive = prop.GetAsPrimitiveProperty()) {
            column.m_type = primitive->GetType();
            if (!IsSupportedPrimitive(column.m_type))
                return false;
            if (column.m_type == ECN::PRIMITIVETYPE_Long) {
                column.m_longMode = ECSqlLongRenderer::GetMode(primitive, m_options);
                if (column.m_longMode == ECSqlLongRenderer::Mode::ClassName)
                    column.m_tableSpace = tableSpace;
            }
            return true;
        }
        if (prop.GetIsNavigation()) {
            column.m_kind = Kind::Navigation;
            column.m_tableSpace = tableSpace;
            return true;
        }
        if (auto structProp = prop.GetAsStructProperty()) {
            column.m_kind = Kind::Struct;
            return PrepareStruct(column, structProp->GetType(), tableSpace, fromArray, ancestors);
        }
        if (auto arrayProp = prop.GetAsPrimitiveArrayProperty()) {
            column.m_kind = Kind::PrimitiveArray;
            column.m_type = arrayProp->GetPrimitiveElementType();
            // The generic array renderer's type override leaves longs numeric, even for extended types.
            return IsSupportedPrimitive(column.m_type);
        }
        if (auto arrayProp = prop.GetAsStructArrayProperty()) {
            column.m_kind = Kind::StructArray;
            return PrepareStruct(column, arrayProp->GetStructElementType(), tableSpace, true, ancestors);
        }
        return false;
    }

    static void ClearClassNames(Column& column) {
        column.m_className = ECSqlLongRenderer::LastClassName();
        for (auto& member : column.m_members)
            ClearClassNames(member);
    }

    template <typename Writer>
    bool WritePrimitive(Writer& writer, IECSqlValue const& value, Column& column, ECDbCR ecdb) {
        switch (column.m_type) {
            case ECN::PRIMITIVETYPE_String: {
                auto text = value.GetText();
                return text != nullptr && writer.String(text);
            }
            case ECN::PRIMITIVETYPE_Double:
                return writer.Double(value.GetDouble());
            case ECN::PRIMITIVETYPE_Integer:
                return writer.Int64(value.GetInt64());
            case ECN::PRIMITIVETYPE_Boolean:
                return writer.Bool(value.GetBoolean());
            case ECN::PRIMITIVETYPE_Long:
                return ECSqlLongRenderer::Render(writer, value, column.m_longMode, ecdb,
                    column.m_tableSpace.c_str(), m_options.UseClassFullNameInsteadofClassName(), &column.m_className);
            case ECN::PRIMITIVETYPE_Point2d: {
                auto point = value.GetPoint2d();
                return writer.StartObject() &&
                    writer.Key(m_options.UseJsNames() ? ECN::ECJsonSystemNames::Point::X() : ECDBSYS_PROP_PointX) &&
                    writer.Double(point.x) &&
                    writer.Key(m_options.UseJsNames() ? ECN::ECJsonSystemNames::Point::Y() : ECDBSYS_PROP_PointY) &&
                    writer.Double(point.y) && writer.EndObject();
            }
            case ECN::PRIMITIVETYPE_Point3d: {
                auto point = value.GetPoint3d();
                return writer.StartObject() &&
                    writer.Key(m_options.UseJsNames() ? ECN::ECJsonSystemNames::Point::X() : ECDBSYS_PROP_PointX) &&
                    writer.Double(point.x) &&
                    writer.Key(m_options.UseJsNames() ? ECN::ECJsonSystemNames::Point::Y() : ECDBSYS_PROP_PointY) &&
                    writer.Double(point.y) &&
                    writer.Key(m_options.UseJsNames() ? ECN::ECJsonSystemNames::Point::Z() : ECDBSYS_PROP_PointZ) &&
                    writer.Double(point.z) && writer.EndObject();
            }
            default:
                return false;
        }
    }

    template <typename Writer>
    bool WriteNavigation(Writer& writer, IECSqlValue const& value, Column& column, ECDbCR ecdb) {
        if (!writer.StartObject())
            return false;
        auto const& id = value[ECDBSYS_PROP_NavPropId];
        if (!id.IsNull()) {
            if (!writer.Key(m_options.UseJsNames() ? ECN::ECJsonSystemNames::Navigation::Id() : ECDBSYS_PROP_NavPropId) ||
                !ECSqlLongRenderer::RenderHexId(writer, id.GetId<ECInstanceId>()))
                return false;
            auto const& classId = value[ECDBSYS_PROP_NavPropRelECClassId];
            if (!classId.IsNull()) {
                if (!writer.Key(m_options.UseJsNames() ? ECN::ECJsonSystemNames::Navigation::RelClassName() : ECDBSYS_PROP_NavPropRelECClassId) ||
                    !ECSqlLongRenderer::RenderId(writer, classId.GetId<ECN::ECClassId>(),
                        m_options.ConvertClassIdsToClassNames() || m_options.UseJsNames(), ecdb,
                        column.m_tableSpace.c_str(), m_options.UseClassFullNameInsteadofClassName(), &column.m_className))
                    return false;
            }
        }
        return writer.EndObject();
    }

    template <typename Writer>
    bool WriteStruct(Writer& writer, IECSqlValue const& value, Column& column, ECDbCR ecdb) {
        if (!writer.StartObject())
            return false;
        for (auto& member : column.m_members) {
            auto const& memberValue = value[member.m_memberName.c_str()];
            if (memberValue.IsNull())
                continue;
            if (!writer.Key(member.m_jsonName.c_str()) || !WriteValue(writer, memberValue, member, ecdb))
                return false;
        }
        return writer.EndObject();
    }

    template <typename Writer>
    bool WriteValue(Writer& writer, IECSqlValue const& value, Column& column, ECDbCR ecdb) {
        switch (column.m_kind) {
            case Kind::Primitive:
                return WritePrimitive(writer, value, column, ecdb);
            case Kind::Navigation:
                return WriteNavigation(writer, value, column, ecdb);
            case Kind::Struct:
                return WriteStruct(writer, value, column, ecdb);
            case Kind::PrimitiveArray:
            case Kind::StructArray:
                if (!writer.StartArray())
                    return false;
                for (auto const& element : value.GetArrayIterable()) {
                    if (element.IsNull()) {
                        if (column.m_kind == Kind::StructArray && (!writer.StartObject() || !writer.EndObject()))
                            return false;
                        continue;
                    }
                    if (column.m_kind == Kind::PrimitiveArray ? !WritePrimitive(writer, element, column, ecdb)
                                                           : !WriteStruct(writer, element, column, ecdb))
                        return false;
                }
                return writer.EndArray();
        }
        return false;
    }

public:
    void Reset() {
        m_columns.clear();
        m_initialized = false;
        m_eligible = false;
    }

    void BeginPage(ECSqlStatementCR stmt, ECSqlRowAdaptor const& adaptor) {
        if (!m_initialized || m_options != adaptor.m_options) {
            Reset();
            m_options = adaptor.m_options;
            m_initialized = true;
            m_eligible = true;
            std::vector<ECN::ECStructClassCP> ancestors;
            for (int i = 0; i < stmt.GetColumnCount(); ++i) {
                auto const& info = stmt.GetColumnInfo(i);
                auto prop = info.GetProperty();
                Column column;
                if (info.IsDynamic() || prop == nullptr ||
                    !PrepareColumn(column, *prop, info.GetRootClass().GetTableSpace(), false, ancestors)) {
                    m_eligible = false;
                    break;
                }
                column.m_skip = m_options.SkipReadOnlyProperties() && prop->GetIsReadOnly();
                m_columns.push_back(std::move(column));
            }
            if (!m_eligible)
                m_columns.clear();
        }
        // Schema/class lookup state is never carried across worker refreshes between pages.
        for (auto& column : m_columns)
            ClearClassNames(column);
    }

    bool CanRender(ECSqlRowAdaptor const& adaptor) const {
        return m_eligible && m_options == adaptor.m_options && !adaptor.m_customHandler && !adaptor.m_skipPropertyHandler;
    }

    template <typename Writer>
    bool WriteRow(Writer& writer, ECSqlStatementCR stmt, ECSqlRowAdaptor const& adaptor) {
        if (!writer.StartArray())
            return false;
        uint32_t nulls = 0;
        for (size_t i = 0; i < m_columns.size(); ++i) {
            auto& column = m_columns[i];
            if (column.m_skip)
                continue;
            auto const& value = stmt.GetValue(static_cast<int>(i));
            if (value.IsNull()) {
                ++nulls;
                continue;
            }
            while (nulls > 0) {
                if (!writer.Null())
                    return false;
                --nulls;
            }
            if (!WriteValue(writer, value, column, adaptor.m_ecdb))
                return false;
        }
        return writer.EndArray();
    }
};

END_BENTLEY_SQLITE_EC_NAMESPACE
