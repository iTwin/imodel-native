/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ECDbInternalTypes.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      04/2014
//+===============+===============+===============+===============+===============+======
struct SchemaValidator final
    {
    private:
        //=======================================================================================
        // @bsiclass                                                Krischan.Eberle      10/2018
        //+===============+===============+===============+===============+===============+======
        struct NoUnitsInEC31SchemaRule final
            {
            private:
                //not copyable
                NoUnitsInEC31SchemaRule(NoUnitsInEC31SchemaRule const&) = delete;
                NoUnitsInEC31SchemaRule& operator=(NoUnitsInEC31SchemaRule const&) = delete;

            public:
                NoUnitsInEC31SchemaRule() {}
                bool Validate(SchemaImportContext const&, IIssueReporter const&, ECN::ECSchemaCR) const;
            };

        //=======================================================================================
        // @bsiclass                                                Krischan.Eberle      04/2014
        //+===============+===============+===============+===============+===============+======
        struct ValidBaseClassesRule final
            {
        private:
            //not copyable
            ValidBaseClassesRule(ValidBaseClassesRule const&) = delete;
            ValidBaseClassesRule& operator=(ValidBaseClassesRule const&) = delete;

        public:
            ValidBaseClassesRule() {}
            bool Validate(SchemaImportContext const&, IIssueReporter const&, ECN::ECSchemaCR, ECN::ECClassCR) const;
            };

        //=======================================================================================
        // @bsiclass                                                Krischan.Eberle      04/2014
        //+===============+===============+===============+===============+===============+======
        struct ValidRelationshipRule final
            {
            private:
                //not copyable
                ValidRelationshipRule(ValidRelationshipRule const&) = delete;
                ValidRelationshipRule& operator=(ValidRelationshipRule const&) = delete;

                bool ValidateConstraint(IIssueReporter const&, ECN::ECRelationshipClassCR, ECN::ECRelationshipEnd, ECN::ECRelationshipConstraintCR) const;

            public:
                ValidRelationshipRule() {}
                bool Validate(IIssueReporter const&, ECN::ECSchemaCR, ECN::ECClassCR) const;
            };

        //=======================================================================================
        // @bsiclass                                                Krischan.Eberle      04/2014
        //+===============+===============+===============+===============+===============+======
        struct ValidPropertyRule final
            {
            private:
                //not copyable
                ValidPropertyRule(ValidPropertyRule const&) = delete;
                ValidPropertyRule& operator=(ValidPropertyRule const&) = delete;

                bool ValidatePropertyName(IIssueReporter const&, ECN::ECClassCR, ECN::ECPropertyCR) const;
                bool ValidatePropertyStructType(IIssueReporter const&, ECN::ECStructClassCR, ECN::ECPropertyCR) const;

            public:
                ValidPropertyRule() {}
                bool Validate(IIssueReporter const&,ECN::ECClassCR, ECN::ECPropertyCR) const;
            };

        
        SchemaValidator();
        ~SchemaValidator();

    public:
        static bool ValidateSchemas(SchemaImportContext&, IIssueReporter const&, bvector<ECN::ECSchemaCP> const&);
    };


END_BENTLEY_SQLITE_EC_NAMESPACE
