/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "SchemaCommandArgParser.h"
#include <charconv>
#include <cmath>
#include <optional>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

namespace {
constexpr size_t MAX_ARGUMENT_DEPTH = 128;

bool Invalid(SchemaCommandResult& result, Utf8StringCR path, Utf8StringCR message) {
    if (result.IsOk())
        result = SchemaCommandResult::Failure(SchemaCommandResult::Status::ERROR_INVALID_ARGS,
                                              "SchemaEditor.InvalidArgs", path.c_str(), message);
    else
        result.AddIssue({"SchemaEditor.InvalidArgs", path, message});
    return false;
}

Utf8String MemberPath(Utf8StringCR parent, Utf8CP name) {
    return parent.empty() ? Utf8String(name) : Utf8String(parent).append(".").append(name);
}

BeJsConst NullValue() {
    static BeJsDocument const null;
    return null;
}

std::optional<BeJsConst> FindValue(BeJsConst object, Utf8StringCR name) {
    std::optional<BeJsConst> found;
    if (object.isObject())
        object.ForEachProperty([&](Utf8CP key, BeJsConst value) {
            if (!name.EqualsI(key))
                return false;
            found.emplace(value);
            return true;
        });
    return found;
}

std::optional<BeJsConst> FindPath(BeJsConst object, Utf8CP path) {
    if (path == nullptr || *path == '\0')
        return std::nullopt;
    Utf8CP dot = strchr(path, '.');
    Utf8String key = dot == nullptr ? Utf8String(path) : Utf8String(path, static_cast<size_t>(dot - path));
    auto value = FindValue(object, key);
    if (!value || dot == nullptr)
        return value;
    return FindPath(*value, dot + 1);
}

bool CheckParentObjects(BeJsConst object, Utf8CP path, Utf8StringCR parent, SchemaCommandResult& result) {
    Utf8CP dot = strchr(path, '.');
    if (dot == nullptr)
        return true;
    Utf8String key(path, static_cast<size_t>(dot - path));
    auto value = FindValue(object, key);
    if (!value)
        return true;
    auto memberPath = MemberPath(parent, key.c_str());
    if (!value->isObject())
        return Invalid(result, memberPath, "Expected an object containing nested arguments.");
    return CheckParentObjects(*value, dot + 1, memberPath, result);
}

bool HasNulCharacter(BeJsConst value) {
    // ToUtf8CP cannot represent embedded NULs. Check the JSON string before any
    // typed reader uses it, skipping escaped backslashes rather than matching literals.
    auto text = value.Stringify();
    for (size_t i = 0; i < text.size(); ++i)
        if (text[i] == '\\') {
            if (text.compare(i, 6, "\\u0000") == 0)
                return true;
            ++i;
        }
    return false;
}

bool IsStructuredArgument(Utf8StringCR name) {
    return name.EqualsI("customAttributes") || name.EqualsI("indexes") || name.EqualsI("enumerators") || name.EqualsI("instance");
}

bool ValidateNode(BeJsConst value, Utf8StringCR path, SchemaCommandResult& result, size_t depth,
                  bool argumentPaths = false) {
    if (depth > MAX_ARGUMENT_DEPTH)
        return Invalid(result, path, "Argument nesting exceeds the maximum depth of 128.");
    if (value.isJsOnlyType())
        return Invalid(result, path, "Expected a JSON value.");

    bool valid = true;
    if (value.isObject()) {
        bvector<Utf8String> names;
        value.ForEachProperty([&](Utf8CP key, BeJsConst member) {
            auto memberPath = MemberPath(path, key);
            if (*key == '\0') {
                Invalid(result, memberPath, "Argument names must not be empty.");
                valid = false;
            }
            if (argumentPaths && strchr(key, '.') != nullptr) {
                Invalid(result, memberPath, "Use a nested JSON object instead of a dotted JSON member name.");
                valid = false;
            }
            if (std::any_of(names.begin(), names.end(), [&](Utf8StringCR name) { return name.EqualsI(key); })) {
                Invalid(result, memberPath, "Duplicate argument name (case-insensitive).");
                valid = false;
            }
            names.push_back(key);
            if (!ValidateNode(member, memberPath, result, depth + 1, argumentPaths && !IsStructuredArgument(Utf8String(key))))
                valid = false;
            return false;
        });
    } else if (value.isArray())
        value.ForEachArrayMember([&](BeJsConst::ArrayIndex index, BeJsConst member) {
            if (!ValidateNode(member, Utf8PrintfString("%s[%u]", path.c_str(), index), result, depth + 1, argumentPaths))
                valid = false;
            return false;
        });
    else if (value.isNumeric()) {
        if (!std::isfinite(value.GetDouble()))
            valid = Invalid(result, path, "Expected a finite JSON number.");
    } else if (value.isString() && HasNulCharacter(value))
        valid = Invalid(result, path, "Argument values must not contain NUL characters.");
    else if (!(value.isNull() || value.isString() || value.isBool()))
        valid = Invalid(result, path, "Expected a JSON value.");
    return valid;
}

bool SplitPath(Utf8StringCR path, bvector<Utf8String>& parts, SchemaCommandResult& result) {
    size_t start = 0;
    for (;;) {
        size_t end = path.find('.', start);
        Utf8String part(path.substr(start, end == Utf8String::npos ? end : end - start));
        if (part.empty())
            return Invalid(result, path, "Argument paths must contain non-empty names separated by dots.");
        if (part.find('\0') != Utf8String::npos)
            return Invalid(result, path, "Argument names must not contain NUL characters.");
        parts.push_back(std::move(part));
        if (parts.size() > MAX_ARGUMENT_DEPTH)
            return Invalid(result, path, "Argument nesting exceeds the maximum depth of 128.");
        if (end == Utf8String::npos)
            return true;
        start = end + 1;
    }
}

bool IsPathPrefix(bvector<Utf8String> const& prefix, bvector<Utf8String> const& path) {
    if (prefix.size() > path.size())
        return false;
    for (size_t i = 0; i < prefix.size(); ++i)
        if (!prefix[i].EqualsI(path[i]))
            return false;
    return true;
}

void InsertValue(BeJsValue object, bvector<Utf8String> const& path, size_t index, BeJsConst value) {
    Utf8String key = path[index];
    object.ForEachProperty([&](Utf8CP name, BeJsConst) {
        if (!key.EqualsI(name))
            return false;
        key = name;
        return true;
    });
    auto member = object[key];
    if (index + 1 == path.size())
        member.From(value);
    else {
        member.toObject();
        InsertValue(member, path, index + 1, value);
    }
}

bool CheckObjectKeys(BeJsConst object, Utf8StringCR parent,
                     std::initializer_list<Utf8CP> allowed, SchemaCommandResult& result) {
    bool valid = true;
    object.ForEachProperty([&](Utf8CP name, BeJsConst value) {
        auto path = MemberPath(parent, name);
        bool exact = false;
        bool prefix = false;
        for (Utf8CP key : allowed) {
            if (path.EqualsI(key))
                exact = true;
            Utf8String candidate(key);
            if (candidate.size() > path.size() && candidate[path.size()] == '.' && Utf8String(candidate.substr(0, path.size())).EqualsI(path))
                prefix = true;
        }
        if (strchr(name, '.') != nullptr) {
            Invalid(result, path, "Use a nested JSON object instead of a dotted JSON member name.");
            valid = false;
        } else if (exact)
            return false;
        else if (!prefix) {
            Invalid(result, path, "Unknown argument.");
            valid = false;
        } else if (!value.isObject()) {
            Invalid(result, path, "Expected an object containing nested arguments.");
            valid = false;
        } else if (!CheckObjectKeys(value, path, allowed, result))
            valid = false;
        return false;
    });
    return valid;
}
}

SchemaCommandResult ValidateSchemaCommandJson(BeJsConst args) {
    SchemaCommandResult result;
    if (!args.isObject())
        Invalid(result, "args", "Expected an argument object.");
    else
        ValidateNode(args, "", result, 0, true);
    return result;
}

SchemaCommandResult NormalizeSchemaCommandArgs(SchemaCommandArgs const& args, BeJsValue out) {
    SchemaCommandResult result;
    BeJsDocument normalized;
    normalized.SetEmptyObject();
    bvector<bvector<Utf8String>> paths;
    for (auto const& entry : args.GetEntries()) {
        bvector<Utf8String> path;
        if (!SplitPath(entry.first, path, result))
            continue;
        bool collision = false;
        for (auto const& previous : paths)
            if (IsPathPrefix(previous, path) || IsPathPrefix(path, previous)) {
                Invalid(result, entry.first, "Duplicate or conflicting argument path (case-insensitive).");
                collision = true;
                break;
            }
        paths.push_back(path);
        if (collision)
            continue;

        BeJsDocument value;
        if (entry.second.find('\0') != Utf8String::npos) {
            Invalid(result, entry.first, "Argument values must not contain NUL characters.");
            continue;
        }
        Utf8String text(entry.second);
        text.Trim();
        // Legacy enumerator/index syntax remains a string. An attempted JSON container
        // must parse successfully; it must never fall back to a string on parse failure.
        if (IsStructuredArgument(path.back()) && !text.empty() && (text.front() == '[' || text.front() == '{')) {
            value.Parse(text);
            if (value.hasParseError()) {
                Invalid(result, entry.first, "Malformed JSON object or array.");
                continue;
            }
            if (!ValidateNode(value, entry.first, result, path.size()))
                continue;
        } else
            value.SetString(entry.second);
        InsertValue(normalized, path, 0, value);
    }
    if (result.IsOk())
        out.From(normalized);
    return result;
}

SchemaCommandArgParser::SchemaCommandArgParser(BeJsConst args, SchemaCommandResult& result)
    : m_args(args), m_result(result), m_valid(true) {
    auto validation = ValidateSchemaCommandJson(args);
    m_valid = validation.IsOk();
    if (m_valid)
        return;
    if (m_result.IsOk())
        m_result = std::move(validation);
    else
        for (auto const& issue : validation.GetIssues())
            m_result.AddIssue(issue);
}

bool SchemaCommandArgParser::Error(Utf8CP path, Utf8CP message) {
    return Invalid(m_result, path, message);
}

bool SchemaCommandArgParser::Missing(Utf8CP path, bool required) {
    if (Utf8String::IsNullOrEmpty(path))
        return Error("", "Argument paths must not be empty.");
    if (!CheckParentObjects(m_args, path, "", m_result))
        return false;
    return required ? Error(path, "Missing required argument.") : true;
}

bool SchemaCommandArgParser::CheckKeys(std::initializer_list<Utf8CP> allowed) {
    return m_valid && CheckObjectKeys(m_args, "", allowed, m_result);
}

bool SchemaCommandArgParser::Has(Utf8CP path) const {
    return m_valid && FindPath(m_args, path).has_value();
}

BeJsConst SchemaCommandArgParser::Value(Utf8CP path) const {
    auto found = m_valid ? FindPath(m_args, path) : std::nullopt;
    return found ? *found : NullValue();
}

bool SchemaCommandArgParser::String(Utf8CP path, Utf8String& out, bool required) {
    if (!m_valid)
        return false;
    if (!Has(path))
        return Missing(path, required);
    auto value = Value(path);
    if (!value.isString())
        return Error(path, "Expected a string.");
    out = value.ToUtf8CP();
    return true;
}

bool SchemaCommandArgParser::Boolean(Utf8CP path, bool& out, bool required) {
    if (!m_valid)
        return false;
    if (!Has(path))
        return Missing(path, required);
    auto value = Value(path);
    if (value.isBool()) {
        out = value.GetBoolean();
        return true;
    }
    if (value.isString()) {
        Utf8String text(value.ToUtf8CP());
        if (text.Equals("true") || text.Equals("false")) {
            out = text.Equals("true");
            return true;
        }
    }
    return Error(path, "Expected a boolean or the string 'true' or 'false'.");
}

bool SchemaCommandArgParser::UInt32(Utf8CP path, uint32_t& out, bool required) {
    if (!m_valid)
        return false;
    if (!Has(path))
        return Missing(path, required);
    auto value = Value(path);
    if (value.isNumeric()) {
        double number = value.GetDouble();
        if (std::isfinite(number) && number >= 0 && number <= UINT32_MAX && std::floor(number) == number) {
            out = static_cast<uint32_t>(number);
            return true;
        }
    } else if (value.isString()) {
        Utf8CP text = value.ToUtf8CP();
        Utf8CP end = text + strlen(text);
        uint32_t parsed;
        auto conversion = std::from_chars(text, end, parsed, 10);
        if (conversion.ec == std::errc() && conversion.ptr == end) {
            out = parsed;
            return true;
        }
    }
    return Error(path, "Expected an unsigned 32-bit integer or a decimal integer string (0..4294967295).");
}

bool SchemaCommandArgParser::StringList(Utf8CP path, bvector<Utf8String>& out, bool required) {
    if (!m_valid)
        return false;
    if (!Has(path))
        return Missing(path, required);
    auto value = Value(path);
    bvector<Utf8String> parsed;
    bool valid = true;
    if (value.isArray())
        value.ForEachArrayMember([&](BeJsConst::ArrayIndex index, BeJsConst member) {
            if (!member.isString()) {
                Error(Utf8PrintfString("%s[%u]", path, index).c_str(), "Expected a string.");
                valid = false;
            } else
                parsed.push_back(member.ToUtf8CP());
            return false;
        });
    else if (value.isString()) {
        Utf8String text(value.ToUtf8CP());
        text.Trim();
        if (!text.empty()) {
            size_t start = 0;
            for (;;) {
                size_t end = text.find(',', start);
                Utf8String item(text.substr(start, end == Utf8String::npos ? end : end - start));
                item.Trim();
                if (item.empty())
                    valid = Error(path, "A comma-separated list must not contain empty entries.");
                else
                    parsed.push_back(std::move(item));
                if (end == Utf8String::npos)
                    break;
                start = end + 1;
            }
        }
    } else
        valid = Error(path, "Expected an array of strings or a comma-separated string.");
    if (valid)
        out = std::move(parsed);
    return valid;
}

END_BENTLEY_SQLITE_EC_NAMESPACE
