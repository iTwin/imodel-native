/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../ECDbPch.h"
#include "SchemaCommandHelpers.h"
#include <regex>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
namespace {
using namespace SchemaCommands;
enum class Operation { Create,
                       Alter,
                       Drop };

Result Number(SchemaCommandArgParser& parser, Utf8CP key, double& out) {
    auto value = parser.Value(key);
    if (value.isNumeric())
        out = value.GetDouble();
    else if (value.isString()) {
        Utf8String text(value.asCString());
        text.Trim();
        char* end = nullptr;
        out = text.empty() ? 0.0 : strtod(text.c_str(), &end);
        if (text.empty() || end != text.c_str() + text.size())
            return Invalid(key, "Expected a number.");
    } else
        return Invalid(key, "Expected a number.");
    return std::isfinite(out) ? Result() : Invalid(key, "Expected a finite number.");
}

Result AlteredTexts(SchemaCommandArgParser& parser, Result& parsed, SchemaMetaRow& row) {
    for (auto key : {"label", "description"}) {
        if (!parser.Has(key))
            continue;
        Utf8String value;
        if (!parser.String(key, value))
            return parsed;
        row.push_back({strcmp(key, "label") == 0 ? "DisplayLabel" : "Description",
                       value.empty() ? SchemaMetaValue(nullptr) : SchemaMetaValue(value)});
    }
    return {};
}

template <typename T>
void CreatedTexts(T const& item, SchemaMetaRow& row) {
    if (item.GetIsDisplayLabelDefined())
        row.push_back({"DisplayLabel", item.GetInvariantDisplayLabel()});
    if (!item.GetInvariantDescription().empty())
        row.push_back({"Description", item.GetInvariantDescription()});
}

// ECJSON writes items as Schema.Item, the format string parser expects Schema:Item. Unit labels are kept verbatim.
Utf8String NormalizePresentationFormat(Utf8StringCR value) {
    Utf8String normalized;
    bool inUnit = false, inLabel = false;
    for (auto c : value) {
        if (c == '[')
            inUnit = true, inLabel = false;
        else if (c == ']')
            inUnit = inLabel = false;
        else if (c == '|' && inUnit)
            inLabel = true;
        else if (c == '.' && !inLabel)
            c = ':';
        normalized.push_back(c);
    }
    return normalized;
}

Result PresentationFormats(SchemaCommandArgParser& parser, Result& parsed, bvector<Utf8String>& formats) {
    auto value = parser.Value("presentationUnits");
    if (value.isArray()) {
        for (uint32_t i = 0; i < value.size(); ++i) {
            if (!value[i].isString())
                return Invalid(Utf8PrintfString("presentationUnits[%u]", i).c_str(), "Expected a format string.");
            formats.push_back(value[i].asCString());
        }
    } else if (value.isString()) {
        // Same splitting as the ECXML PresentationUnits attribute (KindOfQuantity::ReadXml).
        static std::regex const rgx(R"(([^[;]+(\[[^|\]]+[|]?([^\]]+)?\]){0,4}))");
        std::string text(value.asCString());
        for (auto it = std::sregex_iterator(text.begin(), text.end(), rgx); it != std::sregex_iterator(); ++it) {
            Utf8String format(it->str().c_str());
            format.Trim();
            if (!format.empty())
                formats.push_back(format);
        }
    } else if (!value.isNull())
        return Invalid("presentationUnits", "Expected an array of format strings.");
    return {};
}

struct Resolver {
    SchemaEditContext& m_ctx;
    ECSchemaR m_working;
    bvector<ECSchemaId> m_ids;
    Result m_error;
    Resolver(SchemaEditContext& ctx, ECSchemaR working) : m_ctx(ctx), m_working(working) {}

    ECSchemaCP Schema(Utf8StringCR alias, Utf8CP arg) {
        if (alias.empty() || alias.EqualsI(m_working.GetAlias()) || alias.EqualsI(m_working.GetName()))
            return &m_working;
        auto persisted = FindSchema(m_ctx, alias);
        if (persisted == nullptr) {
            m_error = Missing(arg, Utf8PrintfString("Schema '%s' not found.", alias.c_str()));
            return nullptr;
        }
        auto result = Reference(m_ctx, m_working, *persisted);
        if (!result.IsOk()) {
            m_error = result;
            return nullptr;
        }
        if (!persisted->GetName().EqualsI(m_working.GetName()) && std::find(m_ids.begin(), m_ids.end(), persisted->GetId()) == m_ids.end())
            m_ids.push_back(persisted->GetId());
        return WorkingSchema(m_working, persisted->GetName());
    }
    ECUnitCP Unit(Utf8StringCR alias, Utf8StringCR name, Utf8CP arg) {
        auto schema = Schema(alias, arg);
        auto unit = schema == nullptr ? nullptr : schema->GetUnitCP(name.c_str());
        if (unit == nullptr && m_error.IsOk())
            m_error = Missing(arg, Utf8PrintfString("Unit '%s:%s' not found.", alias.c_str(), name.c_str()));
        return unit;
    }
    ECFormatCP Format(Utf8StringCR alias, Utf8StringCR name, Utf8CP arg) {
        auto schema = Schema(alias, arg);
        auto format = schema == nullptr ? nullptr : schema->GetFormatCP(name.c_str());
        if (format == nullptr && m_error.IsOk())
            m_error = Missing(arg, Utf8PrintfString("Format '%s:%s' not found.", alias.c_str(), name.c_str()));
        return format;
    }
};

Result ApplyPresentationFormats(Resolver& resolver, KindOfQuantityR koq, bvector<Utf8String> const& formats) {
    koq.RemoveAllPresentationFormats();
    for (size_t i = 0; i < formats.size(); ++i) {
        Utf8PrintfString arg("presentationUnits[%zu]", i);
        auto status = koq.AddPresentationFormatByString(NormalizePresentationFormat(formats[i]), [&](Utf8StringCR alias, Utf8StringCR name) { return resolver.Format(alias, name, arg.c_str()); }, [&](Utf8StringCR alias, Utf8StringCR name) { return resolver.Unit(alias, name, arg.c_str()); });
        if (!resolver.m_error.IsOk())
            return resolver.m_error;
        if (ECObjectsStatus::Success != status)
            return Rejected(arg.c_str(), Utf8PrintfString("Invalid presentation format '%s'.", formats[i].c_str()));
    }
    for (auto const& format : koq.GetPresentationFormats())
        if (format.IsProblem())
            return Rejected("presentationUnits", Utf8PrintfString("Invalid presentation format: %s", format.GetProblemDescription().c_str()));
    return {};
}

Result SerializePresentationFormats(SchemaEditContext& ctx, KindOfQuantityCR koq, SchemaMetaRow& row, bool alter) {
    if (koq.GetPresentationFormats().empty()) {
        if (alter)
            row.push_back({"PresentationUnits", nullptr});
        return {};
    }
    Utf8String json;
    if (SUCCESS != SchemaPersistenceHelper::SerializeKoqPresentationFormats(json, ctx.m_db, koq, FeatureManager::IsEC32Available(ctx.m_db)))
        return Rejected("presentationUnits", "Cannot serialize the presentation formats.");
    row.push_back({"PresentationUnits", json});
    return {};
}

Utf8String PersistenceUnit(SchemaEditContext& ctx, KindOfQuantityCR koq) {
    return FeatureManager::IsEC32Available(ctx.m_db) ? koq.GetPersistenceUnit()->GetQualifiedName(koq.GetSchema()) : koq.GetDescriptorCache().first;
}

Result InUse(SchemaEditContext& ctx, Utf8CP column, BeInt64Id id, Utf8CP arg, Utf8CP what) {
    Statement stmt;
    Utf8PrintfString sql("SELECT s.Name,c.Name,p.Name FROM main.ec_Property p JOIN main.ec_Class c ON c.Id=p.ClassId "
                         "JOIN main.ec_Schema s ON s.Id=c.SchemaId WHERE p.%s=? LIMIT 1",
                         column);
    if (BE_SQLITE_OK != stmt.Prepare(ctx.m_db, sql.c_str()) || BE_SQLITE_OK != stmt.BindId(1, id))
        return DatabaseError("Preparing the property usage query failed.");
    auto step = stmt.Step();
    if (step == BE_SQLITE_DONE)
        return {};
    if (step != BE_SQLITE_ROW)
        return DatabaseError("Executing the property usage query failed.");
    return Rejected(arg, Utf8PrintfString("Cannot delete %s which is still referenced by a property, %s:%s.%s.", what,
                                          stmt.GetValueText(0), stmt.GetValueText(1), stmt.GetValueText(2)));
}

Result PrepareKindOfQuantity(SchemaEditContext& ctx, BeJsConst args, PreparedSchemaCommand& out, Operation operation) {
    Result result;
    SchemaCommandArgParser parser(args, result);
    if (operation == Operation::Create ? !parser.CheckKeys({"schema", "name", "persistenceUnit", "relativeError", "presentationUnits", "label", "description"}) : operation == Operation::Alter ? !parser.CheckKeys({"koq", "persistenceUnit", "relativeError", "presentationUnits", "label", "description"})
                                                                                                                                                                                                : !parser.CheckKeys({"koq"}))
        return result;
    Utf8String owner, name;
    ECSchemaCP schema;
    KindOfQuantityCP old = nullptr;
    if (operation == Operation::Create) {
        if (!parser.String("schema", owner, true) || !parser.String("name", name, true))
            return result;
        schema = FindSchema(ctx, owner);
        if (schema == nullptr)
            return Missing("schema", "Schema not found.");
        if (!ECNameValidation::IsValidName(name.c_str()))
            return Invalid("name", "Invalid ECName.");
    } else {
        if (!parser.String("koq", owner, true))
            return result;
        Utf8String schemaName, item;
        old = QualifiedName(owner, schemaName, item) ? ctx.m_db.Schemas().GetKindOfQuantity(schemaName, item, SchemaLookupMode::AutoDetect) : nullptr;
        if (old == nullptr)
            return Missing("koq", "KindOfQuantity not found; use Schema:KindOfQuantity.");
        schema = &old->GetSchema();
        name = old->GetName();
    }
    result = Editable(ctx, *schema);
    if (!result.IsOk())
        return result;
    auto schemaId = schema->GetId();
    BeInt64Id itemId = old == nullptr ? BeInt64Id() : BeInt64Id(old->GetId().GetValue());
    if (operation == Operation::Drop) {
        // SchemaWriter::DeleteKindOfQuantity requires a major version change.
        result = ctx.RequireMajorUpgrade(*schema);
        if (!result.IsOk())
            return result;
        result = InUse(ctx, "KindOfQuantityId", itemId, "koq", "KindOfQuantity");
        if (!result.IsOk())
            return result;
        out.m_apply = [schemaId, itemId](SchemaEditContext& apply) {
            auto written = apply.m_writer.Delete("ec_KindOfQuantity", itemId);
            if (written.IsOk())
                Affected(apply, schemaId);
            return written;
        };
        return {};
    }

    ECSchemaPtr working;
    result = Copy(ctx, working, *schema);
    if (!result.IsOk())
        return result;
    Resolver resolver(ctx, *working);
    SchemaMetaRow row;
    KindOfQuantityP koq = nullptr;
    if (operation == Operation::Create) {
        if (ECObjectsStatus::Success != working->CreateKindOfQuantity(koq, name.c_str()) || koq == nullptr)
            return Rejected("name", "Invalid or duplicate KindOfQuantity name.");
        Utf8String text;
        if (parser.Has("label") && (!parser.String("label", text) || ECObjectsStatus::Success != koq->SetDisplayLabel(text)))
            return result.IsOk() ? Invalid("label", "Invalid label.") : result;
        if (parser.Has("description") && (!parser.String("description", text) || ECObjectsStatus::Success != koq->SetDescription(text)))
            return result.IsOk() ? Invalid("description", "Invalid description.") : result;
        if (!parser.Has("relativeError"))
            return Invalid("relativeError", "Missing required argument.");
        double relativeError;
        result = Number(parser, "relativeError", relativeError);
        if (!result.IsOk())
            return result;
        koq->SetRelativeError(relativeError);
        if (!parser.String("persistenceUnit", text, true))
            return result;
        Utf8String unitSchema, unitName;
        if (!QualifiedName(text, unitSchema, unitName))
            return Invalid("persistenceUnit", "Expected a qualified unit name (Schema:Unit).");
        auto unit = resolver.Unit(unitSchema, unitName, "persistenceUnit");
        if (unit == nullptr)
            return resolver.m_error;
        if (ECObjectsStatus::Success != koq->SetPersistenceUnit(*unit))
            return Rejected("persistenceUnit", "ECObjects rejected the persistence unit.");
        if (parser.Has("presentationUnits")) {
            bvector<Utf8String> formats;
            result = PresentationFormats(parser, result, formats);
            if (!result.IsOk())
                return result;
            result = ApplyPresentationFormats(resolver, *koq, formats);
            if (!result.IsOk())
                return result;
        }
        result = ValidateWorking(ctx, *working);
        if (!result.IsOk())
            return result;
        row = {{"SchemaId", int64_t(schemaId.GetValue())}, {"Name", name}};
        CreatedTexts(*koq, row);
        row.push_back({"RelativeError", koq->GetRelativeError()});
        row.push_back({"PersistenceUnit", PersistenceUnit(ctx, *koq)});
        result = SerializePresentationFormats(ctx, *koq, row, false);
        if (!result.IsOk())
            return result;
    } else {
        koq = working->GetKindOfQuantityP(name.c_str());
        if (koq == nullptr)
            return Rejected("koq", "KindOfQuantity missing from working schema.");
        if (parser.Has("persistenceUnit")) {
            Utf8String text, unitSchema, unitName;
            if (!parser.String("persistenceUnit", text))
                return result;
            auto unit = QualifiedName(text, unitSchema, unitName) ? ctx.m_db.Schemas().GetUnit(unitSchema, unitName, SchemaLookupMode::AutoDetect) : nullptr;
            if (unit == nullptr)
                return Missing("persistenceUnit", "Unit not found.");
            if (!unit->GetFullName().EqualsIAscii(old->GetPersistenceUnit()->GetFullName()))
                return Unsupported("persistenceUnit", "Changing properties of KindOfQuantity is not supported except for RelativeError, PresentationFormats, DisplayLabel and Description.");
        }
        result = AlteredTexts(parser, result, row);
        if (!result.IsOk())
            return result;
        if (parser.Has("relativeError")) {
            if (parser.Value("relativeError").isNull())
                return Rejected("relativeError", "Removing the RelativeError of a KindOfQuantity is not valid. A KindOfQuantity must always have a RelativeError.");
            double relativeError;
            result = Number(parser, "relativeError", relativeError);
            if (!result.IsOk())
                return result;
            if (relativeError != old->GetRelativeError()) {
                koq->SetRelativeError(relativeError);
                row.push_back({"RelativeError", relativeError});
            }
        }
        if (parser.Has("presentationUnits")) {
            bvector<Utf8String> formats;
            result = PresentationFormats(parser, result, formats);
            if (!result.IsOk())
                return result;
            bvector<Utf8String> oldFormats;
            for (auto const& format : old->GetPresentationFormats())
                oldFormats.push_back(format.GetQualifiedFormatString(old->GetSchema()));
            result = ApplyPresentationFormats(resolver, *koq, formats);
            if (!result.IsOk())
                return result;
            bvector<Utf8String> newFormats;
            for (auto const& format : koq->GetPresentationFormats())
                newFormats.push_back(format.GetQualifiedFormatString(koq->GetSchema()));
            if (newFormats != oldFormats) {
                result = SerializePresentationFormats(ctx, *koq, row, true);
                if (!result.IsOk())
                    return result;
            }
        }
        if (row.empty())
            return Invalid("koq", "At least one KindOfQuantity attribute must change.");
        result = ValidateWorking(ctx, *working);
        if (!result.IsOk())
            return result;
    }
    auto references = resolver.m_ids;
    bool create = operation == Operation::Create;
    out.m_apply = [create, schemaId, itemId, row, references](SchemaEditContext& apply) mutable {
        for (auto id : references) {
            auto added = apply.AddReference(schemaId, id);
            if (!added.IsOk())
                return added;
        }
        Result written;
        if (create) {
            row.push_back({"Id", int64_t(apply.m_db.GetImpl().GetIdFactory().KindOfQuantity().NextId().GetValue())});
            written = apply.m_writer.Insert("ec_KindOfQuantity", row);
        } else
            written = apply.m_writer.Update("ec_KindOfQuantity", itemId, row);
        if (written.IsOk())
            Affected(apply, schemaId);
        return written;
    };
    return {};
}

Result PreparePropertyCategory(SchemaEditContext& ctx, BeJsConst args, PreparedSchemaCommand& out, Operation operation) {
    Result result;
    SchemaCommandArgParser parser(args, result);
    if (operation == Operation::Create ? !parser.CheckKeys({"schema", "name", "priority", "label", "description"}) : operation == Operation::Alter ? !parser.CheckKeys({"category", "priority", "label", "description"})
                                                                                                                                                   : !parser.CheckKeys({"category"}))
        return result;
    Utf8String owner, name;
    ECSchemaCP schema;
    PropertyCategoryCP old = nullptr;
    ECSchemaPtr working;
    // A reused graph already passed full validation after the command that published it.
    bool validatedGraph = false;
    if (operation == Operation::Create) {
        if (!parser.String("schema", owner, true) || !parser.String("name", name, true))
            return result;
        // Read only the schema header first; the full schema is needed only for a fresh copy.
        schema = ctx.m_db.Schemas().GetSchema(owner, false, SchemaLookupMode::AutoDetect);
        if (schema == nullptr)
            return Missing("schema", "Schema not found.");
        if (ctx.m_propertyCreationSchema.IsValid() &&
            ctx.m_propertyCreationSchema->GetSchemaKey().Matches(schema->GetSchemaKey(), SchemaMatchType::Exact)) {
            working = ctx.m_propertyCreationSchema;
            validatedGraph = true;
        } else if ((schema = FindSchema(ctx, owner)) == nullptr)
            return Missing("schema", "Schema not found.");
        if (!ECNameValidation::IsValidName(name.c_str()))
            return Invalid("name", "Invalid ECName.");
    } else {
        if (!parser.String("category", owner, true))
            return result;
        Utf8String schemaName, item;
        old = QualifiedName(owner, schemaName, item) ? ctx.m_db.Schemas().GetPropertyCategory(schemaName, item, SchemaLookupMode::AutoDetect) : nullptr;
        if (old == nullptr)
            return Missing("category", "PropertyCategory not found; use Schema:PropertyCategory.");
        schema = &old->GetSchema();
        name = old->GetName();
    }
    result = Editable(ctx, *schema);
    if (!result.IsOk())
        return result;
    auto schemaId = schema->GetId();
    BeInt64Id itemId = old == nullptr ? BeInt64Id() : BeInt64Id(old->GetId().GetValue());
    if (operation == Operation::Drop) {
        // SchemaWriter::DeletePropertyCategory has no version requirement but refuses categories still in use.
        result = InUse(ctx, "CategoryId", itemId, "category", "PropertyCategory");
        if (!result.IsOk())
            return result;
        out.m_apply = [schemaId, itemId](SchemaEditContext& apply) {
            auto written = apply.m_writer.Delete("ec_PropertyCategory", itemId);
            if (written.IsOk())
                Affected(apply, schemaId);
            return written;
        };
        return {};
    }

    uint32_t priority = 0;
    if (!parser.UInt32("priority", priority))
        return result;
    SchemaMetaRow row;
    if (operation == Operation::Create) {
        if (!validatedGraph) {
            result = Copy(ctx, working, *schema);
            if (!result.IsOk())
                return result;
        }
        PropertyCategoryP category = nullptr;
        if (ECObjectsStatus::Success != working->CreatePropertyCategory(category, name.c_str(), false) || category == nullptr)
            return Rejected("name", "Invalid or duplicate PropertyCategory name.");
        Utf8String text;
        if (parser.Has("label") && (!parser.String("label", text) || ECObjectsStatus::Success != category->SetDisplayLabel(text)))
            return result.IsOk() ? Invalid("label", "Invalid label.") : result;
        if (parser.Has("description") && (!parser.String("description", text) || ECObjectsStatus::Success != category->SetDescription(text)))
            return result.IsOk() ? Invalid("description", "Invalid description.") : result;
        category->SetPriority(priority);
        // No ECObjects or ECDb schema rule inspects property categories beyond what
        // CreatePropertyCategory checked, so a new one cannot invalidate a validated graph.
        result = validatedGraph ? ValidateWorkingGates(ctx, *working) : ValidateWorking(ctx, *working);
        if (!result.IsOk())
            return result;
        out.m_propertyCreationSchema = working;
        row = {{"SchemaId", int64_t(schemaId.GetValue())}, {"Name", name}};
        CreatedTexts(*category, row);
        row.push_back({"Priority", int64_t(category->GetPriority())});
    } else {
        result = AlteredTexts(parser, result, row);
        if (!result.IsOk())
            return result;
        if (parser.Has("priority") && priority != old->GetPriority())
            row.push_back({"Priority", int64_t(priority)});
        if (row.empty())
            return Invalid("category", "At least one PropertyCategory attribute must change.");
    }
    bool create = operation == Operation::Create;
    out.m_apply = [create, schemaId, itemId, row](SchemaEditContext& apply) mutable {
        Result written;
        if (create) {
            row.push_back({"Id", int64_t(apply.m_db.GetImpl().GetIdFactory().PropertyCategory().NextId().GetValue())});
            written = apply.m_writer.Insert("ec_PropertyCategory", row);
        } else
            written = apply.m_writer.Update("ec_PropertyCategory", itemId, row);
        if (written.IsOk())
            Affected(apply, schemaId);
        return written;
    };
    return {};
}
}

void RegisterSchemaQuantityCommands(SchemaCommandRegistry& registry) {
    for (auto operation : {Operation::Create, Operation::Alter, Operation::Drop}) {
        Utf8CP verb = operation == Operation::Create ? "Create" : operation == Operation::Alter ? "Alter"
                                                                                                : "Drop";
        Register(registry, Utf8PrintfString("%sKindOfQuantity", verb).c_str(),
                 [operation](SchemaEditContext& ctx, BeJsConst args, PreparedSchemaCommand& out) {
                     out.m_mapsClasses = false;
                     return PrepareKindOfQuantity(ctx, args, out, operation);
                 });
        Register(registry, Utf8PrintfString("%sPropertyCategory", verb).c_str(),
                 [operation](SchemaEditContext& ctx, BeJsConst args, PreparedSchemaCommand& out) {
                     out.m_mapsClasses = false;
                     return PreparePropertyCategory(ctx, args, out, operation);
                 });
    }
}
END_BENTLEY_SQLITE_EC_NAMESPACE
