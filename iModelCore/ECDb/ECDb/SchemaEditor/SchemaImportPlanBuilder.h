/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/SchemaImportAdapter.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//! Translates incoming ECSchemas into an ordered SchemaEditor command stream. Read-only with respect to the
//! database contents and the caller's schemas: every input graph is deep-copied before normalization.
struct SchemaImportPlanBuilder final {
    static SchemaCommandResult Build(ECDbR, bvector<ECN::ECSchemaCP> const&, bvector<SchemaImportPlanStep>& steps,
                                     bvector<Utf8String>& warnings);
    //! Identity of the schema/mapping state a plan was built against.
    static Utf8String ComputeBaseline(ECDbR);
};

END_BENTLEY_SQLITE_EC_NAMESPACE
