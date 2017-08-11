/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/SchemaValidator.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ECDbInternalTypes.h"
#include <Bentley/NonCopyableClass.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      04/2014
//+===============+===============+===============+===============+===============+======
struct SchemaValidator final
    {
    private:
        //=======================================================================================
        // @bsiclass                                                Krischan.Eberle      04/2014
        //+===============+===============+===============+===============+===============+======
        struct ValidBaseClassesRule final : NonCopyableClass
            {
        public:
            ValidBaseClassesRule() {}
            bool Validate(SchemaImportContext const&, IssueReporter const&, ECN::ECSchemaCR, ECN::ECClassCR) const;
            };

        //=======================================================================================
        // @bsiclass                                                Krischan.Eberle      04/2014
        //+===============+===============+===============+===============+===============+======
        struct ValidRelationshipRule final : NonCopyableClass
            {
            private:
                bool ValidateConstraint(IssueReporter const&, ECN::ECRelationshipClassCR, ECN::ECRelationshipEnd, ECN::ECRelationshipConstraintCR) const;

            public:
                ValidRelationshipRule() {}
                bool Validate(IssueReporter const&, ECN::ECSchemaCR, ECN::ECClassCR) const;
            };
        //=======================================================================================
        // @bsiclass                                                Affan.Khan      08/2017
        //+===============+===============+===============+===============+===============+======
        struct CyclicDependencyRule final : NonCopyableClass
            {
            private:
                static bool HasRecusiveStructTypeProperty(ECN::ECClassCR, bvector<ECN::ECPropertyCP>&);
                static bool HasRecusiveStructTypeProperty(ECN::ECClassCR, Utf8StringR);
                static ECN::ECStructClassCP GetStructType(ECN::ECPropertyCP);
            public:
                CyclicDependencyRule() {}
                bool Validate(IssueReporter const&, ECN::ECClassCR) const;
            };

        //=======================================================================================
        // @bsiclass                                                Krischan.Eberle      04/2014
        //+===============+===============+===============+===============+===============+======
        struct ValidPropertyRule final : NonCopyableClass
            {
            private:
                bool ValidatePropertyName(IssueReporter const&, ECN::ECClassCR, ECN::ECPropertyCR) const;
                bool ValidatePropertyStructType(IssueReporter const&, ECN::ECClassCR, ECN::ECPropertyCR) const;
            public:
                ValidPropertyRule() {}
                bool Validate(IssueReporter const&,ECN::ECClassCR, ECN::ECPropertyCR) const;
            };

        
        SchemaValidator();
        ~SchemaValidator();

    public:
        static bool ValidateSchemas(SchemaImportContext&, IssueReporter const&, bvector<ECN::ECSchemaCP> const&);
    };


END_BENTLEY_SQLITE_EC_NAMESPACE
