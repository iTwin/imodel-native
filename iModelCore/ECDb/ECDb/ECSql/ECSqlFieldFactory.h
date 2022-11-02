/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "ECSqlPrepareContext.h"
#include "PropertyNameExp.h"
#include "ECSqlField.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct ECSqlFieldFactory final
    {
    private:
        //static class
        ECSqlFieldFactory() = delete;
        ~ECSqlFieldFactory() = delete;

        static ECSqlStatus CreateNullField(std::unique_ptr<ECSqlField>&, int& sqlColumnIndex, ECSqlPrepareContext&, ECSqlColumnInfo const&);
        static ECSqlStatus CreatePrimitiveField(std::unique_ptr<ECSqlField>&, int& sqlColumnIndex, ECSqlPrepareContext&, ECSqlColumnInfo const&, ECN::PrimitiveType);
        static ECSqlStatus CreateStructField(std::unique_ptr<ECSqlField>&, int& sqlColumnIndex, ECSqlPrepareContext&, ECSqlColumnInfo const&, ECN::ECStructClassCR);
        static ECSqlStatus CreateNavigationPropertyField(std::unique_ptr<ECSqlField>&, int& sqlColumnIndex, ECSqlPrepareContext&, ECSqlColumnInfo const&);
        static ECSqlStatus CreateArrayField(std::unique_ptr<ECSqlField>&, int& sqlColumnIndex, ECSqlPrepareContext&, ECSqlColumnInfo const&);
        static ECSqlStatus CreateStructMemberFields(std::unique_ptr<ECSqlField>&, int& sqlColumnIndex, ECSqlPrepareContext&, ECN::ECStructClassCR, ECSqlColumnInfo const&);
        static ECSqlStatus CreateChildField(std::unique_ptr<ECSqlField>& childField, ECSqlPrepareContext&, int& sqlColumnIndex, ECSqlColumnInfo const& parentFieldColumnInfo, ECN::ECPropertyCR childProperty);

        static ECSqlColumnInfo CreateColumnInfoForProperty(ECSqlPrepareContext const& ctx, ECN::ECPropertyCP generatedProperty, PropertyNameExp const* propertyNameExp);
        static ECSqlColumnInfo CreateTopLevelColumnInfo(IssueDataSource const& issues, bool isSystemProperty, bool isGeneratedProperty, ECSqlPropertyPath const& propertyPath, ECSqlColumnInfo::RootClass const& rootClass, ECN::ECPropertyCP originalProperty);
        static ECN::ECTypeDescriptor DetermineDataType(DateTime::Info&, ECN::ECStructClassCP&, IssueDataSource const&, ECN::ECPropertyCR);

        static ECSqlSelectPreparedStatement& GetPreparedStatement(ECSqlPrepareContext&);

    public:
        static ECSqlStatus CreateField(ECSqlPrepareContext&, DerivedPropertyExp const* derivedProperty, int startColumnIndex);

        static ECSqlColumnInfo CreateChildColumnInfo(IssueDataSource const&, ECSqlColumnInfo const& parent, ECN::ECPropertyCR childProperty, bool isSystemProperty);
        static ECSqlColumnInfo CreateColumnInfoForArrayElement(ECSqlColumnInfo const& parent, int arrayIndex);

    };

END_BENTLEY_SQLITE_EC_NAMESPACE
