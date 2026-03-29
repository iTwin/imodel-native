/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "InstanceReaderImpl.h"
#include "ECSql/ECSqlField.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

using TableView = InstanceReader::Impl::TableView;

struct ChangesetFieldFactory final {
   private:
    using Stage = Changes::Change::Stage;
    static const ClassMap* GetRootClassMap(DbTable const& tbl, ECDbCR conn);
    static DateTime::Info GetDateTimeInfo(PropertyMap const& propertyMap);
    static ECSqlPropertyPath GetPropertyPath(PropertyMap const&);
    static std::unique_ptr<ECSqlField> CreatePrimitiveField(ECDbCR ecdb, PropertyMap const&, TableView const&, Changes::Change const&, Stage const&);
    static std::unique_ptr<ECSqlField> CreateSystemField(ECDbCR ecdb, PropertyMap const&, TableView const&, Changes::Change const&, Stage const&);
    static std::unique_ptr<ECSqlField> CreateStructField(ECDbCR ecdb, PropertyMap const&, TableView const&, Changes::Change const&, Stage const&);
    static std::unique_ptr<ECSqlField> CreateNavigationField(ECDbCR ecdb, PropertyMap const&, TableView const&, Changes::Change const&, Stage const&);
    static std::unique_ptr<ECSqlField> CreateArrayField(ECDbCR ecdb, PropertyMap const&, TableView const&, Changes::Change const&, Stage const&);
    static std::unique_ptr<ECSqlField> CreateField(ECDbCR ecdb, PropertyMap const&, TableView const&, Changes::Change const&, Stage const&);
    static std::unique_ptr<ECSqlField> CreateClassIdField(ECDbCR ecdb, PropertyMap const&, ECN::ECClassId, TableView const&, Changes::Change const&, Stage const&);

   public:
    using FieldPtr = std::unique_ptr<ECSqlField>;
    static std::vector<FieldPtr> Create(ECDbCR conn, DbTable const& tbl, Changes::Change const& change, Changes::Change::Stage const& stage);
};

END_BENTLEY_SQLITE_EC_NAMESPACE
