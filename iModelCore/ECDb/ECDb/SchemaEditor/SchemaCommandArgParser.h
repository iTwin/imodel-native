/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/SchemaEditor.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//! On failure, out is unchanged. Scalars remain strings; dotted keys become nested objects.
//! Container JSON in customAttributes/indexes/enumerators/instance is parsed strictly.
//! Legacy index/enumerator notation without a JSON container prefix remains a string.
SchemaCommandResult NormalizeSchemaCommandArgs(SchemaCommandArgs const&, BeJsValue out);
//! Validates the object and its entire JSON subtree before command-specific parsing.
//! Argument paths must be nested objects, not literal dotted member names.
SchemaCommandResult ValidateSchemaCommandJson(BeJsConst);

class SchemaCommandArgParser final {
private:
    BeJsConst m_args;
    SchemaCommandResult& m_result;
    bool m_valid;

    bool Error(Utf8CP path, Utf8CP message);
    bool Missing(Utf8CP path, bool required);

public:
    SchemaCommandArgParser(BeJsConst, SchemaCommandResult&);
    //! Dotted allowed keys validate nested objects; an allowed object key owns its subtree.
    bool CheckKeys(std::initializer_list<Utf8CP> allowed);
    bool Has(Utf8CP) const;
    BeJsConst Value(Utf8CP) const;
    bool String(Utf8CP, Utf8String&, bool required = false);
    //! Accepts native booleans and exactly "true"/"false", without case or whitespace coercion.
    bool Boolean(Utf8CP, bool&, bool required = false);
    bool UInt32(Utf8CP, uint32_t&, bool required = false);
    bool StringList(Utf8CP, bvector<Utf8String>&, bool required = false);
};

END_BENTLEY_SQLITE_EC_NAMESPACE
