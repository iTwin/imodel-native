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
        //! Makes sure an ECClass has only one navigation property (local or inherited) pointing to the same root relationship with same direction
        // @bsiclass                                                Krischan.Eberle      05/2017
        //+===============+===============+===============+===============+===============+======
        struct ClassHasNoDuplicateNavigationPropertiesRule final : NonCopyableClass
            {
            public:
                struct Context final : NonCopyableClass
                    {
                    ECN::ECClassCR m_ecClass;
                    bmap<ECN::ECRelationshipClassCP, bmap<ECN::ECRelatedInstanceDirection, bset<ECN::NavigationECPropertyCP>>> m_navPropsByRelAndDirection;
                    bool m_hasDuplicates = false;

                    explicit Context(ECN::ECClassCR ecClass) : m_ecClass(ecClass) {}

                    bool HasNavigationProperties() const { return !m_navPropsByRelAndDirection.empty(); }
                    };

            private:
                void LogIssues(Context const&, IssueReporter const&) const;
                static ECN::ECRelationshipClassCR GetRootRelationship(ECN::ECRelationshipClassCR);

            public:
                ClassHasNoDuplicateNavigationPropertiesRule() {}
                bool Validate(Context&, IssueReporter const&, ECN::ECPropertyCR) const;
                bool PostProcessValidation(Context&, IssueReporter const&) const;
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
