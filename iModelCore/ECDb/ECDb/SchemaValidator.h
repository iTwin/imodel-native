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
        SchemaValidator();
        ~SchemaValidator();

    public:
        static bool ValidateSchemas(IssueReporter const&, bvector<ECN::ECSchemaCP> const&, bool doNotFailOnLegacyIssues);
    };


//=======================================================================================
// @bsiclass                                                Krischan.Eberle      04/2014
//+===============+===============+===============+===============+===============+======
struct IClassValidationRule
    {
    private:
        virtual bool _ValidateClass(IssueReporter const&, ECN::ECSchemaCR, ECN::ECClassCR) const = 0;

    protected:
        IClassValidationRule()  {}

    public:
        virtual ~IClassValidationRule() {}

        bool ValidateClass(IssueReporter const& issues, ECN::ECSchemaCR schema, ECN::ECClassCR ecClass) const { return _ValidateClass(issues, schema, ecClass); }
    };

//**************************** Subclasses *********************************************
//=======================================================================================
// @bsiclass                                                Krischan.Eberle      02/2017
//+===============+===============+===============+===============+===============+======
struct ValidBaseClassesRule final : IClassValidationRule
    {
    private:
        bool m_doNotFailForLegacyIssues = false;

        bool _ValidateClass(IssueReporter const&, ECN::ECSchemaCR, ECN::ECClassCR) const override;

    public:
        explicit ValidBaseClassesRule(bool doNotFailForLegacyIssues) : IClassValidationRule(), m_doNotFailForLegacyIssues(doNotFailForLegacyIssues) {}
        ~ValidBaseClassesRule() {}
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      07/2015
//+===============+===============+===============+===============+===============+======
struct ValidRelationshipRule final : IClassValidationRule
    {
    private:
        bool _ValidateClass(IssueReporter const&, ECN::ECSchemaCR, ECN::ECClassCR) const override;
        bool ValidateConstraint(IssueReporter const&, ECN::ECRelationshipClassCR, ECN::ECRelationshipConstraintCR) const;

    public:
        ValidRelationshipRule() : IClassValidationRule() {}
        ~ValidRelationshipRule() {}
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      06/2014
//+===============+===============+===============+===============+===============+======
struct ValidPropertiesRule final : IClassValidationRule
    {
    private:
        struct NavigationPropertyValidationContext final : NonCopyableClass
            {
            IssueReporter const& m_issues;
            ECN::ECClassCR m_ecClass;
            bmap<ECN::ECRelationshipClassCP, bmap<ECN::ECRelatedInstanceDirection, bset<ECN::NavigationECPropertyCP>>> m_navPropsByRelAndDirection;
            bool m_hasDuplicates = false;

            NavigationPropertyValidationContext(IssueReporter const& issues, ECN::ECClassCR ecClass) : m_issues(issues), m_ecClass(ecClass) {}
            bool HasNavigationProperties() const { return !m_navPropsByRelAndDirection.empty(); }
            void LogIssues() const;
            static ECN::ECRelationshipClassCR GetRootRelationship(ECN::ECRelationshipClassCR);
            };

        bool _ValidateClass(IssueReporter const&, ECN::ECSchemaCR, ECN::ECClassCR) const override;

        bool ValidatePropertyName(IssueReporter const&, ECN::ECClassCR, ECN::ECPropertyCR) const;
        bool ValidatePropertyStructType(IssueReporter const&, ECN::ECClassCR, ECN::ECPropertyCR) const;
        bool ValidateNavigationProperty(NavigationPropertyValidationContext&, ECN::ECPropertyCR) const;
        bool ValidateInheritedNavigationProperties(NavigationPropertyValidationContext&) const;

    public:
        ValidPropertiesRule() : IClassValidationRule() {}
        ~ValidPropertiesRule() {}
    };


END_BENTLEY_SQLITE_EC_NAMESPACE
