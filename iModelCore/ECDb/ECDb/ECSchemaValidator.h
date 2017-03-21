/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSchemaValidator.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ECDbInternalTypes.h"
#include <Bentley/NonCopyableClass.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

struct ECSchemaValidationResult;
struct ECSchemaValidationRules;

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      04/2014
//+===============+===============+===============+===============+===============+======
struct ECSchemaValidator final
    {
    private:

        ECSchemaValidator();
        ~ECSchemaValidator();

        static bool ValidateSchema(ECSchemaValidationResult&, ECSchemaValidationRules const&, ECN::ECSchemaCR);
        static bool ValidateClass(ECSchemaValidationResult&, ECSchemaValidationRules const&, ECN::ECClassCR);

        static void Log(IssueReporter const&, ECSchemaValidationResult const&);

    public:
        static bool ValidateSchemas(IssueReporter const&, bvector<ECN::ECSchemaCP> const&, bool doNotFailOnLegacyIssues);
    };


//=======================================================================================
// @bsiclass                                                Krischan.Eberle      04/2014
//+===============+===============+===============+===============+===============+======
struct IECSchemaValidationRule
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

        virtual bool _ValidateSchema(ECSchemaValidationResult&, ECN::ECSchemaCR schema, ECN::ECClassCR ecClass) const { return true; }
        virtual bool _ValidateClass(ECSchemaValidationResult&, ECN::ECClassCR ecClass, ECN::ECPropertyCR ecProperty) const { return true; }

    protected:
        explicit IECSchemaValidationRule(Type type) : m_type(type) {}

        Type GetType() const { return m_type; }
    public:
        virtual ~IECSchemaValidationRule() {}

        bool ValidateSchema(ECSchemaValidationResult& result, ECN::ECSchemaCR schema, ECN::ECClassCR ecClass) const { return _ValidateSchema(result, schema, ecClass); }
        bool ValidateClass(ECSchemaValidationResult& result, ECN::ECClassCR ecClass, ECN::ECPropertyCR ecProperty) const { return _ValidateClass(result, ecClass, ecProperty); }
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      03/2017
//+===============+===============+===============+===============+===============+======
struct ECSchemaValidationRules final
    {
    std::vector<std::unique_ptr<IECSchemaValidationRule>> m_classRules;
    std::vector<std::unique_ptr<IECSchemaValidationRule>> m_propertyRules;
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      06/2014
//+===============+===============+===============+===============+===============+======
struct ECSchemaValidationResult final : NonCopyableClass
    {
    private:
        std::map<const IECSchemaValidationRule::Type, std::unique_ptr<IECSchemaValidationRule::IError>> m_errors;

    public:
        ECSchemaValidationResult() {}

        IECSchemaValidationRule::IError& AddError(std::unique_ptr<IECSchemaValidationRule::IError> error);

        IECSchemaValidationRule::IError* operator[](IECSchemaValidationRule::Type);

        std::map<const IECSchemaValidationRule::Type, std::unique_ptr<IECSchemaValidationRule::IError>> const& GetErrors() const { return m_errors; }
    };


//**************************** Subclasses *********************************************
//=======================================================================================
// @bsiclass                                                Krischan.Eberle      02/2017
//+===============+===============+===============+===============+===============+======
struct ValidBaseClassesRule final : IECSchemaValidationRule
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

        bool _ValidateSchema(ECSchemaValidationResult&, ECN::ECSchemaCR, ECN::ECClassCR) const override;

    public:
        explicit ValidBaseClassesRule(bool doNotFailForLegacyIssues) : IECSchemaValidationRule(Type::ValidBaseClasses), m_doNotFailForLegacyIssues(doNotFailForLegacyIssues) {}
        ~ValidBaseClassesRule() {}
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      07/2015
//+===============+===============+===============+===============+===============+======
struct ValidRelationshipRule final : IECSchemaValidationRule
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
                struct Inconsistency
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

        bool _ValidateSchema(ECSchemaValidationResult&, ECN::ECSchemaCR schema, ECN::ECClassCR ecClass) const override;

        bool ValidateConstraint(Error&, ECN::ECRelationshipClassCR, ECN::ECRelationshipConstraintCR) const;

    public:
        ValidRelationshipRule() : IECSchemaValidationRule(Type::ValidRelationshipClass) {}
        ~ValidRelationshipRule() {}
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      06/2014
//+===============+===============+===============+===============+===============+======
struct NoPropertiesOfSameTypeAsClassRule final : IECSchemaValidationRule
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

        bool _ValidateClass(ECSchemaValidationResult&, ECN::ECClassCR ecClass, ECN::ECPropertyCR ecProperty) const override;

    public:
        NoPropertiesOfSameTypeAsClassRule() : IECSchemaValidationRule(Type::NoPropertiesOfSameTypeAsClass) {}
        ~NoPropertiesOfSameTypeAsClassRule() {}
    };






//=======================================================================================
// @bsiclass                                                Krischan.Eberle      03/2017
//+===============+===============+===============+===============+===============+======
struct ValidPropertyNameRule final : IECSchemaValidationRule
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
                struct Inconsistency
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

        bool _ValidateClass(ECSchemaValidationResult&, ECN::ECClassCR ecClass, ECN::ECPropertyCR ecProperty) const override;

    public:
        ValidPropertyNameRule() : IECSchemaValidationRule(Type::ValidPropertyName) {}
        ~ValidPropertyNameRule() {}
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      10/2016
//+===============+===============+===============+===============+===============+======
struct ValidNavigationPropertyRule final : IECSchemaValidationRule
    {
    private:
        //=======================================================================================
        // @bsiclass                                                Krischan.Eberle      10/2016
        //+===============+===============+===============+===============+===============+======
        struct Error final : IError
            {
            enum class Kind
                {
                MultiplicityGreaterThanOne
                };

            private:
                struct Inconsistency
                    {
                    ECN::NavigationECPropertyCP m_navProp;
                    Kind m_kind;

                    Inconsistency(ECN::NavigationECPropertyCR navProp, Kind kind)
                        : m_navProp(&navProp), m_kind(kind)
                        {}
                    };

                std::map<ECN::ECClassCP, std::vector<Inconsistency>> m_inconsistencies;

                void _Log(IssueReporter const&) const override;


            public:
                Error() : IError(Type::ValidNavigationProperty) {}
                ~Error() {}

                void AddInconsistency(ECN::NavigationECPropertyCR navProp, Kind kind) { m_inconsistencies[&navProp.GetClass()].push_back(Inconsistency(navProp, kind)); }
            };

        bool _ValidateClass(ECSchemaValidationResult&, ECN::ECClassCR ecClass, ECN::ECPropertyCR ecProperty) const override;

    public:
        ValidNavigationPropertyRule() : IECSchemaValidationRule(Type::ValidNavigationProperty) {}
        ~ValidNavigationPropertyRule() {}
    };
END_BENTLEY_SQLITE_EC_NAMESPACE
