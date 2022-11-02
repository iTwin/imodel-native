/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "ECDbInternalTypes.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct SchemaValidator final
    {
    private:
        //=======================================================================================
        // @bsiclass
        //+===============+===============+===============+===============+===============+======
        struct NoUnitsInEC31SchemaRule final
            {
            private:
                //not copyable
                NoUnitsInEC31SchemaRule(NoUnitsInEC31SchemaRule const&) = delete;
                NoUnitsInEC31SchemaRule& operator=(NoUnitsInEC31SchemaRule const&) = delete;

            public:
                NoUnitsInEC31SchemaRule() {}
                bool Validate(SchemaImportContext const&, IssueDataSource const&, ECN::ECSchemaCR) const;
            };

        //=======================================================================================
        // @bsiclass
        //+===============+===============+===============+===============+===============+======
        struct ValidBaseClassesRule final
            {
        private:
            //not copyable
            ValidBaseClassesRule(ValidBaseClassesRule const&) = delete;
            ValidBaseClassesRule& operator=(ValidBaseClassesRule const&) = delete;

        public:
            ValidBaseClassesRule() {}
            bool Validate(SchemaImportContext const&, IssueDataSource const&, ECN::ECSchemaCR, ECN::ECClassCR) const;
            };

        //=======================================================================================
        // @bsiclass
        //+===============+===============+===============+===============+===============+======
        struct ValidRelationshipRule final
            {
            private:
                //not copyable
                ValidRelationshipRule(ValidRelationshipRule const&) = delete;
                ValidRelationshipRule& operator=(ValidRelationshipRule const&) = delete;

                bool ValidateConstraint(IssueDataSource const&, ECN::ECRelationshipClassCR, ECN::ECRelationshipEnd, ECN::ECRelationshipConstraintCR) const;

            public:
                ValidRelationshipRule() {}
                bool Validate(IssueDataSource const&, ECN::ECSchemaCR, ECN::ECClassCR) const;
            };

        //=======================================================================================
        // @bsiclass
        //+===============+===============+===============+===============+===============+======
        struct ValidPropertyRule final
            {
            private:
                //not copyable
                ValidPropertyRule(ValidPropertyRule const&) = delete;
                ValidPropertyRule& operator=(ValidPropertyRule const&) = delete;

                bool ValidatePropertyName(IssueDataSource const&, ECN::ECClassCR, ECN::ECPropertyCR) const;
                bool ValidatePropertyStructType(IssueDataSource const&, ECN::ECStructClassCR, ECN::ECPropertyCR) const;

            public:
                ValidPropertyRule() {}
                bool Validate(IssueDataSource const&,ECN::ECClassCR, ECN::ECPropertyCR) const;
            };

        
        SchemaValidator();
        ~SchemaValidator();

    public:
        static bool ValidateSchemas(SchemaImportContext&, IssueDataSource const&, bvector<ECN::ECSchemaCP> const&);
    };


END_BENTLEY_SQLITE_EC_NAMESPACE
