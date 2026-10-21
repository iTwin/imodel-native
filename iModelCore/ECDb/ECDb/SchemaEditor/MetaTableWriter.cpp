/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../ECDbPch.h"
#include "SchemaCommand.h"
#include <type_traits>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
namespace {
bool IsIdentifier(Utf8CP value) {
    if (value == nullptr || *value == '\0')
        return false;
    for (auto p = value; *p; ++p)
        if (!((*p >= 'A' && *p <= 'Z') || (*p >= 'a' && *p <= 'z') ||
              (*p >= '0' && *p <= '9') || *p == '_'))
            return false;
    return true;
}

bool IsMetadataTable(Utf8CP table) {
    static constexpr Utf8CP tables[] = {
        "ec_Schema", "ec_SchemaReference", "ec_Class", "ec_ClassHasBaseClasses",
        "ec_Property", "ec_PropertyMap", "ec_PropertyPath", "ec_Enumeration",
        "ec_CustomAttribute", "ec_RelationshipConstraint", "ec_RelationshipConstraintClass",
        "ec_UnitSystem", "ec_Phenomenon", "ec_Unit", "ec_Format", "ec_FormatCompositeUnit",
        "ec_KindOfQuantity", "ec_PropertyCategory"};
    return std::any_of(std::begin(tables), std::end(tables),
                       [table](Utf8CP candidate) { return strcmp(candidate, table) == 0; });
}

bool ChangesHierarchy(Utf8CP table, bool rowAddedOrRemoved) {
    return strcmp(table, "ec_ClassHasBaseClasses") == 0 ||
           (rowAddedOrRemoved && (strcmp(table, "ec_Class") == 0 || strcmp(table, "ec_Schema") == 0));
}

SchemaCommandResult Write(ECDbR db, Utf8StringCR sql, SchemaMetaRow const& values, BeInt64Id const* id) {
    auto failure = [&]() {
        return SchemaCommandResult::Failure(SchemaCommandResult::Status::ERROR,
                                            "SchemaEditor.Persistence", "", db.GetLastError());
    };
    Statement statement;
    if (BE_SQLITE_OK != statement.Prepare(db, sql.c_str()))
        return failure();
    int index = 1;
    for (auto const& value : values) {
        auto status = std::visit([&](auto const& item) -> DbResult {
            using T = std::decay_t<decltype(item)>;
            if constexpr (std::is_same_v<T, std::nullptr_t>)
                return statement.BindNull(index);
            else if constexpr (std::is_same_v<T, int64_t>)
                return statement.BindInt64(index, item);
            else if constexpr (std::is_same_v<T, double>)
                return statement.BindDouble(index, item);
            else
                return statement.BindText(index, item.c_str(), Statement::MakeCopy::No);
        },
                                 value.second);
        if (status != BE_SQLITE_OK)
            return failure();
        ++index;
    }
    if (id != nullptr && BE_SQLITE_OK != statement.BindId(index, *id))
        return failure();
    if (BE_SQLITE_DONE != statement.Step())
        return failure();
    if (db.GetModifiedRowCount() != 1)
        return SchemaCommandResult::Failure(SchemaCommandResult::Status::ERROR,
                                            "SchemaEditor.Persistence", "", "Expected exactly one metadata row to change.");
    return {};
}

bool CheckColumns(SchemaMetaRow const& values) {
    std::set<Utf8String, CompareIUtf8Ascii> names;
    for (auto const& value : values)
        if (!IsIdentifier(value.first.c_str()) || !names.insert(value.first).second)
            return false;
    return true;
}
}

SchemaCommandResult SchemaMetaTableWriter::Insert(Utf8CP table, SchemaMetaRow const& values) {
    if (table == nullptr || !IsMetadataTable(table) || values.empty() || !CheckColumns(values))
        return SchemaCommandResult::Failure(SchemaCommandResult::Status::ERROR,
                                            "SchemaEditor.Persistence", "", "Invalid internal metadata insert descriptor.");
    Utf8String sql = Utf8PrintfString("INSERT INTO main.[%s] (", table);
    Utf8String placeholders;
    for (auto const& value : values) {
        if (!placeholders.empty()) {
            sql.append(",");
            placeholders.append(",");
        }
        sql.append("[").append(value.first).append("]");
        placeholders.append("?");
    }
    sql.append(") VALUES (").append(placeholders).append(")");
    auto result = Write(m_db, sql, values, nullptr);
    if (result.IsOk() && ChangesHierarchy(table, true))
        ++m_hierarchyWrites;
    return result;
}

SchemaCommandResult SchemaMetaTableWriter::Update(Utf8CP table, BeInt64Id id, SchemaMetaRow const& values) {
    if (table == nullptr || !IsMetadataTable(table) || !id.IsValid() || values.empty() || !CheckColumns(values))
        return SchemaCommandResult::Failure(SchemaCommandResult::Status::ERROR,
                                            "SchemaEditor.Persistence", "", "Invalid internal metadata update descriptor.");
    Utf8String sql = Utf8PrintfString("UPDATE main.[%s] SET ", table);
    bool first = true;
    for (auto const& value : values) {
        if (!first)
            sql.append(",");
        first = false;
        sql.append("[").append(value.first).append("]=?");
    }
    sql.append(" WHERE Id=?");
    auto result = Write(m_db, sql, values, &id);
    if (result.IsOk() && ChangesHierarchy(table, false))
        ++m_hierarchyWrites;
    return result;
}

SchemaCommandResult SchemaMetaTableWriter::Delete(Utf8CP table, BeInt64Id id) {
    if (table == nullptr || !IsMetadataTable(table) || !id.IsValid())
        return SchemaCommandResult::Failure(SchemaCommandResult::Status::ERROR,
                                            "SchemaEditor.Persistence", "", "Invalid internal metadata delete descriptor.");
    auto result = Write(m_db, Utf8PrintfString("DELETE FROM main.[%s] WHERE Id=?", table), {}, &id);
    if (result.IsOk() && ChangesHierarchy(table, true))
        ++m_hierarchyWrites;
    return result;
}
END_BENTLEY_SQLITE_EC_NAMESPACE
