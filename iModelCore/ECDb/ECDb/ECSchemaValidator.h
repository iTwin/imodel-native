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

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      04/2014
//+===============+===============+===============+===============+===============+======
struct ECSchemaValidator
    {
    private:
        ECSchemaValidator();
        ~ECSchemaValidator();

        static bool ValidateSchema(ECSchemaValidationResult&, ECN::ECSchemaCR);
        static bool ValidateClass(ECSchemaValidationResult&, ECN::ECClassCR);

    public:
        static bool ValidateSchemas(ECSchemaValidationResult&, bvector<ECN::ECSchemaCP> const&);
    };


//=======================================================================================
// @bsiclass                                                Krischan.Eberle      04/2014
//+===============+===============+===============+===============+===============+======
struct ECSchemaValidationRule
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
            ValidNavigationProperty
            };

        //=======================================================================================
        // @bsiclass                                                Krischan.Eberle      06/2014
        //+===============+===============+===============+===============+===============+======
        struct Error
            {
            private:
                Type m_ruleType;

                virtual Utf8String _ToString() const = 0;

            protected:
                explicit Error(Type ruleType) : m_ruleType(ruleType) {}

            public:
                virtual ~Error() {}

                Utf8String ToString() const { return _ToString(); }
                Type GetRuleType() const { return m_ruleType; }
            };
    private:
        Type m_type;

        virtual bool _ValidateSchemas(bvector<ECN::ECSchemaP> const& schemas, ECN::ECSchemaCR schema) { return true; }
        virtual bool _ValidateSchema(ECN::ECSchemaCR schema, ECN::ECClassCR ecClass) { return true; }
        virtual bool _ValidateClass(ECN::ECClassCR ecClass, ECN::ECPropertyCR ecProperty) { return true; }
        virtual std::unique_ptr<Error> _GetError() const = 0;

    protected:
        explicit ECSchemaValidationRule(Type type) : m_type(type) {}

        Type GetType() const { return m_type; }
    public:
        virtual ~ECSchemaValidationRule() {}

        bool ValidateSchemas(bvector<ECN::ECSchemaP> const& schemas, ECN::ECSchemaCR schema) { return _ValidateSchemas(schemas, schema); }
        bool ValidateSchema(ECN::ECSchemaCR schema, ECN::ECClassCR ecClass) { return _ValidateSchema(schema, ecClass); }
        bool ValidateClass(ECN::ECClassCR ecClass, ECN::ECPropertyCR ecProperty) { return _ValidateClass(ecClass, ecProperty); }

        void AddErrorToResult(ECSchemaValidationResult& result) const;
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      06/2014
//+===============+===============+===============+===============+===============+======
struct ECSchemaValidationResult final : NonCopyableClass
    {
    private:
        std::vector<std::unique_ptr<ECSchemaValidationRule::Error>> m_errors;

    public:
        ECSchemaValidationResult() {}

        void AddError(std::unique_ptr<ECSchemaValidationRule::Error> error);

        bool HasErrors() const { return !m_errors.empty(); }

        std::vector<std::unique_ptr<ECSchemaValidationRule::Error>> const& GetErrors() const { return m_errors; }

        void ToString(std::vector<Utf8String>& errorMessages) const;
    };


//**************************** Subclasses *********************************************
//=======================================================================================
// @bsiclass                                                Krischan.Eberle      02/2017
//+===============+===============+===============+===============+===============+======
struct ValidBaseClassesRule final : ECSchemaValidationRule
    {
    private:
        //=======================================================================================
        // @bsiclass                                                Krischan.Eberle      02/2017
        //+===============+===============+===============+===============+===============+======
        struct Error final : ECSchemaValidationRule::Error
            {
            enum class Kind
                {
                None = 0,
                MultiInheritance = 1,
                AbstractClassHasNonAbstractBaseClass = 2
                };

            private:
                ECN::ECSchemaCR m_ecSchema;
                std::vector<std::pair<ECN::ECClassCP, Kind>> m_violatingClasses;
                Utf8String _ToString() const override;

            public:
                Error(Type ruleType, ECN::ECSchemaCR schema) : ECSchemaValidationRule::Error(ruleType), m_ecSchema(schema) {}
                ~Error() {}

                void AddViolatingClass(ECN::ECClassCR ecClass, Kind kind) { m_violatingClasses.push_back(std::make_pair(&ecClass, kind)); }
                bool HasErrors() const { return !m_violatingClasses.empty(); }
            };

        mutable std::unique_ptr<Error> m_error;

        bool _ValidateSchema(ECN::ECSchemaCR schema, ECN::ECClassCR ecClass) override;
        std::unique_ptr<ECSchemaValidationRule::Error> _GetError() const override;

    public:
        explicit ValidBaseClassesRule(ECN::ECSchemaCR);
        ~ValidBaseClassesRule() {}
    };



//=======================================================================================
// @bsiclass                                                Krischan.Eberle      06/2014
//+===============+===============+===============+===============+===============+======
struct NoPropertiesOfSameTypeAsClassRule final : ECSchemaValidationRule
    {
    private:
        //=======================================================================================
        // @bsiclass                                                Krischan.Eberle      06/2014
        //+===============+===============+===============+===============+===============+======
        struct Error final : ECSchemaValidationRule::Error
            {
            private:
                ECN::ECClassCR m_ecClass;
                std::vector<ECN::ECPropertyCP> m_invalidProperties;

                Utf8String _ToString() const override;

            public:
                Error(Type ruleType, ECN::ECClassCR ecClass) : ECSchemaValidationRule::Error(ruleType), m_ecClass(ecClass) {}
                ~Error() {}

                void AddInvalidProperty(ECN::ECPropertyCR prop) { m_invalidProperties.push_back(&prop); }
                bool HasErrors() const { return !m_invalidProperties.empty(); }
            };

        mutable std::unique_ptr<Error> m_error;

        bool _ValidateClass(ECN::ECClassCR ecClass, ECN::ECPropertyCR ecProperty) override;
        std::unique_ptr<ECSchemaValidationRule::Error> _GetError() const override;

    public:
        explicit NoPropertiesOfSameTypeAsClassRule(ECN::ECClassCR ecClass);
        ~NoPropertiesOfSameTypeAsClassRule() {}
    };


//=======================================================================================
// @bsiclass                                                Krischan.Eberle      07/2015
//+===============+===============+===============+===============+===============+======
struct ValidRelationshipRule final : ECSchemaValidationRule
    {
    private:
        //=======================================================================================
        // @bsiclass                                                Krischan.Eberle      07/2015
        //+===============+===============+===============+===============+===============+======
        struct Error final : ECSchemaValidationRule::Error
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

            ECN:: ECSchemaCR m_ecSchema;
            std::vector<Inconsistency> m_inconsistencies;

            Utf8String _ToString() const override;


            public:
                Error(Type ruleType, ECN::ECSchemaCR ecSchema) : ECSchemaValidationRule::Error(ruleType), m_ecSchema(ecSchema) {}
                ~Error() {}

                void AddInconsistency(ECN::ECRelationshipClassCR relClass, Kind kind, ECN::ECRelationshipClassCP relClassAsConstraint = nullptr) { m_inconsistencies.push_back(Inconsistency(relClass, kind, relClassAsConstraint)); }
                bool HasInconsistencies() const { return !m_inconsistencies.empty(); }
            };

        mutable std::unique_ptr<Error> m_error;

        bool _ValidateSchema(ECN::ECSchemaCR schema, ECN::ECClassCR ecClass) override;
        std::unique_ptr<ECSchemaValidationRule::Error> _GetError() const override;

        bool ValidateConstraint(ECN::ECRelationshipClassCR, ECN::ECRelationshipConstraintCR) const;

    public:
        explicit ValidRelationshipRule(ECN::ECSchemaCR);
        ~ValidRelationshipRule() {}
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      10/2016
//+===============+===============+===============+===============+===============+======
struct ValidNavigationPropertyRule final : ECSchemaValidationRule
    {
    private:
        //=======================================================================================
        // @bsiclass                                                Krischan.Eberle      10/2016
        //+===============+===============+===============+===============+===============+======
        struct Error final : ECSchemaValidationRule::Error
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

                ECN::ECClassCP m_ecClass;
                std::vector<Inconsistency> m_inconsistencies;

                Utf8String _ToString() const override;


            public:
                explicit Error(Type ruleType, ECN::ECClassCR ecClass) : ECSchemaValidationRule::Error(ruleType), m_ecClass(&ecClass) {}
                ~Error() {}

                void AddInconsistency(ECN::NavigationECPropertyCR navProp, Kind kind) { m_inconsistencies.push_back(Inconsistency(navProp, kind)); }
                bool HasInconsistencies() const { return !m_inconsistencies.empty(); }
            };

        mutable std::unique_ptr<Error> m_error;

        bool _ValidateClass(ECN::ECClassCR ecClass, ECN::ECPropertyCR ecProperty) override;
        std::unique_ptr<ECSchemaValidationRule::Error> _GetError() const override;

    public:
        explicit ValidNavigationPropertyRule(ECN::ECClassCR);
        ~ValidNavigationPropertyRule() {}
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
