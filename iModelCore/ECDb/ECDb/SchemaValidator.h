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

struct ISchemaValidationRule;
struct SchemaValidationResult;

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      04/2014
//+===============+===============+===============+===============+===============+======
struct SchemaValidator final
    {
    private:
        SchemaValidator();
        ~SchemaValidator();

        static bool ValidateSchema(SchemaValidationResult&, std::vector<std::unique_ptr<ISchemaValidationRule>> const&, ECN::ECSchemaCR);
        static void Log(IssueReporter const&, SchemaValidationResult const&);

    public:
        static bool ValidateSchemas(IssueReporter const&, bvector<ECN::ECSchemaCP> const&, bool doNotFailOnLegacyIssues);
    };


//=======================================================================================
// @bsiclass                                                Krischan.Eberle      04/2014
//+===============+===============+===============+===============+===============+======
struct ISchemaValidationRule
    {
    public:
        //=======================================================================================
        // @bsiclass                                                Krischan.Eberle      06/2014
        //+===============+===============+===============+===============+===============+======
        enum Type
            {
            ValidBaseClasses,
            NoPropertiesOfSameTypeAsClass, //!< Struct or array properties within an ECClass must not be of same type or derived type than the ECClass.
            ValidRelationshipClass,
            ValidPropertyName,
            ValidNavigationProperty
            };

        //=======================================================================================
        // @bsiclass                                                Krischan.Eberle      06/2014
        //+===============+===============+===============+===============+===============+======
        struct IError
            {
            private:
                Type m_ruleType;

                virtual void _Log(IssueReporter const&) const = 0;

            protected:
                explicit IError(Type ruleType) : m_ruleType(ruleType) {}

            public:
                virtual ~IError() {}

                void Log(IssueReporter const& issues) const { return _Log(issues); }
                Type GetRuleType() const { return m_ruleType; }
            };
    private:
        Type m_type;

        virtual bool _ValidateSchema(SchemaValidationResult&, ECN::ECSchemaCR schema, ECN::ECClassCR ecClass) const { return true; }

    protected:
        explicit ISchemaValidationRule(Type type) : m_type(type) {}

        Type GetType() const { return m_type; }
    public:
        virtual ~ISchemaValidationRule() {}

        bool ValidateSchema(SchemaValidationResult& result, ECN::ECSchemaCR schema, ECN::ECClassCR ecClass) const { return _ValidateSchema(result, schema, ecClass); }
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      06/2014
//+===============+===============+===============+===============+===============+======
struct SchemaValidationResult final : NonCopyableClass
    {
    private:
        std::map<const ISchemaValidationRule::Type, std::unique_ptr<ISchemaValidationRule::IError>> m_errors;

    public:
        SchemaValidationResult() {}

        ISchemaValidationRule::IError& AddError(std::unique_ptr<ISchemaValidationRule::IError> error);

        ISchemaValidationRule::IError* operator[](ISchemaValidationRule::Type);

        std::map<const ISchemaValidationRule::Type, std::unique_ptr<ISchemaValidationRule::IError>> const& GetErrors() const { return m_errors; }
    };


//**************************** Subclasses *********************************************
//=======================================================================================
// @bsiclass                                                Krischan.Eberle      02/2017
//+===============+===============+===============+===============+===============+======
struct ValidBaseClassesRule final : ISchemaValidationRule
    {
    private:
        //=======================================================================================
        // @bsiclass                                                Krischan.Eberle      02/2017
        //+===============+===============+===============+===============+===============+======
        struct Error final : IError
            {
            enum class Kind
                {
                None = 0,
                MultiInheritance = 1,
                AbstractClassHasNonAbstractBaseClass = 2
                };

            private:
                std::map<ECN::ECSchemaCP, std::vector<std::pair<ECN::ECClassCP, Kind>>> m_violatingClasses;
                bool m_doNotFailForLegacyIssues = false;

                void _Log(IssueReporter const&) const override;

            public:
                explicit Error(bool doNotFailForLegacyIssues) : IError(Type::ValidBaseClasses), m_doNotFailForLegacyIssues(doNotFailForLegacyIssues) {}
                ~Error() {}

                void AddViolatingClass(ECN::ECClassCR ecClass, Kind kind) { m_violatingClasses[&ecClass.GetSchema()].push_back(std::make_pair(&ecClass, kind)); }
            };

        bool m_doNotFailForLegacyIssues = false;

        bool _ValidateSchema(SchemaValidationResult&, ECN::ECSchemaCR, ECN::ECClassCR) const override;

    public:
        explicit ValidBaseClassesRule(bool doNotFailForLegacyIssues) : ISchemaValidationRule(Type::ValidBaseClasses), m_doNotFailForLegacyIssues(doNotFailForLegacyIssues) {}
        ~ValidBaseClassesRule() {}
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      07/2015
//+===============+===============+===============+===============+===============+======
struct ValidRelationshipRule final : ISchemaValidationRule
    {
    private:
        //=======================================================================================
        // @bsiclass                                                Krischan.Eberle      07/2015
        //+===============+===============+===============+===============+===============+======
        struct Error final : IError
            {
            enum class Kind
                {
                None = 0,
                HasAnyClassConstraint = 1,
                HasRelationshipClassAsConstraint = 2,
                HasIncompleteConstraintDefinition = 4,
                HasAdditionalProperties = 8
                };

            private:
                struct Inconsistency final
                    {
                    ECN::ECRelationshipClassCP m_relationshipClass;
                    Kind m_kind;
                    ECN::ECRelationshipClassCP m_relationshipClassAsConstraintClass;

                    Inconsistency(ECN::ECRelationshipClassCR relClass, Kind kind, ECN::ECRelationshipClassCP relationshipClassAsConstraintClass)
                        : m_relationshipClass(&relClass), m_kind(kind), m_relationshipClassAsConstraintClass(relationshipClassAsConstraintClass)
                        {
                        BeAssert(kind != Kind::None);
                        }
                    };

                std::map<ECN::ECSchemaCP, std::vector<Inconsistency>> m_inconsistencies;

                void _Log(IssueReporter const&) const override;


            public:
                Error() : IError(Type::ValidRelationshipClass) {}
                ~Error() {}

                void AddInconsistency(ECN::ECRelationshipClassCR relClass, Kind kind, ECN::ECRelationshipClassCP relClassAsConstraint = nullptr) { m_inconsistencies[&relClass.GetSchema()].push_back(Inconsistency(relClass, kind, relClassAsConstraint)); }
            };

        bool _ValidateSchema(SchemaValidationResult&, ECN::ECSchemaCR schema, ECN::ECClassCR ecClass) const override;

        bool ValidateConstraint(Error&, ECN::ECRelationshipClassCR, ECN::ECRelationshipConstraintCR) const;

    public:
        ValidRelationshipRule() : ISchemaValidationRule(Type::ValidRelationshipClass) {}
        ~ValidRelationshipRule() {}
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      06/2014
//+===============+===============+===============+===============+===============+======
struct NoPropertiesOfSameTypeAsClassRule final : ISchemaValidationRule
    {
    private:
        //=======================================================================================
        // @bsiclass                                                Krischan.Eberle      06/2014
        //+===============+===============+===============+===============+===============+======
        struct Error final : IError
            {
            private:
                std::map<ECN::ECClassCP, std::vector<ECN::ECPropertyCP>> m_invalidProperties;

                void _Log(IssueReporter const&) const override;

            public:
                Error() : IError(Type::NoPropertiesOfSameTypeAsClass) {}
                ~Error() {}

                void AddInvalidProperty(ECN::ECPropertyCR prop) { m_invalidProperties[&prop.GetClass()].push_back(&prop); }
            };

        bool _ValidateSchema(SchemaValidationResult&, ECN::ECSchemaCR schema, ECN::ECClassCR ecClass) const override;

    public:
        NoPropertiesOfSameTypeAsClassRule() : ISchemaValidationRule(Type::NoPropertiesOfSameTypeAsClass) {}
        ~NoPropertiesOfSameTypeAsClassRule() {}
    };


//=======================================================================================
// @bsiclass                                                Krischan.Eberle      03/2017
//+===============+===============+===============+===============+===============+======
struct ValidPropertyNameRule final : ISchemaValidationRule
    {
    private:
        //=======================================================================================
        // @bsiclass                                                Krischan.Eberle      03/2017
        //+===============+===============+===============+===============+===============+======
        struct Error final : IError
            {
            enum class Kind
                {
                SystemPropertyNamingCollision
                };

            private:
                struct Inconsistency final
                    {
                    ECN::ECPropertyCP m_prop = nullptr;
                    Kind m_kind;

                    Inconsistency(ECN::ECPropertyCR prop, Kind kind) : m_prop(&prop), m_kind(kind) {}
                    };

                std::map<ECN::ECClassCP, std::vector<Inconsistency>> m_inconsistencies;

                void _Log(IssueReporter const&) const override;

            public:
                Error() : IError(Type::ValidPropertyName) {}
                ~Error() {}

                void AddInconsistency(ECN::ECPropertyCR prop, Kind kind) { m_inconsistencies[&prop.GetClass()].push_back(Inconsistency(prop, kind)); }
            };

        bool _ValidateSchema(SchemaValidationResult&, ECN::ECSchemaCR schema, ECN::ECClassCR ecClass) const override;

    public:
        ValidPropertyNameRule() : ISchemaValidationRule(Type::ValidPropertyName) {}
        ~ValidPropertyNameRule() {}
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      10/2016
//+===============+===============+===============+===============+===============+======
struct ValidNavigationPropertyRule final : ISchemaValidationRule
    {
    private:
        //=======================================================================================
        // @bsiclass                                                Krischan.Eberle      10/2016
        //+===============+===============+===============+===============+===============+======
        struct Error final : IError
            {
            private:
                bmap<ECN::ECClassCP, bvector<ECN::NavigationECPropertyCP>> m_multiplicityGreaterOneInconsistencies;
                bmap<ECN::ECRelationshipClassCP, bmap<ECN::ECRelatedInstanceDirection, bset<ECN::NavigationECPropertyCP>>> const* m_multipleNavPropsWithSameRelHierarchyInconsistency = nullptr;

                void _Log(IssueReporter const&) const override;

            public:
                Error() : IError(Type::ValidNavigationProperty) {}
                ~Error() {}

                void AddMultiplicityInconsistency(ECN::ECClassCR ecClass, ECN::NavigationECPropertyCR navProp) { m_multiplicityGreaterOneInconsistencies[&ecClass].push_back(&navProp); }
                void AddMultipleNavPropsWithSameRelHierarchyInconsistency(bmap<ECN::ECRelationshipClassCP, bmap<ECN::ECRelatedInstanceDirection, bset<ECN::NavigationECPropertyCP>>> const& incons) 
                    { 
                    if (m_multipleNavPropsWithSameRelHierarchyInconsistency == nullptr)
                        m_multipleNavPropsWithSameRelHierarchyInconsistency = &incons; 
                    }
            };


        mutable bmap<ECN::ECRelationshipClassCP, bmap<ECN::ECRelatedInstanceDirection, bset<ECN::NavigationECPropertyCP>>> m_navPropsPerRelClass;

        bool _ValidateSchema(SchemaValidationResult&, ECN::ECSchemaCR schema, ECN::ECClassCR ecClass) const override;

        Error& GetError(SchemaValidationResult&) const;

        static ECN::ECRelationshipClassCR GetRootRelationship(ECN::ECRelationshipClassCR);

    public:
        ValidNavigationPropertyRule() : ISchemaValidationRule(Type::ValidNavigationProperty) {}
        ~ValidNavigationPropertyRule() {}
    };
END_BENTLEY_SQLITE_EC_NAMESPACE
