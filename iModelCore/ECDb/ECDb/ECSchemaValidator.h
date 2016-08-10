/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSchemaValidator.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
        static bool ValidateSchemas(ECSchemaValidationResult&, bvector<ECN::ECSchemaP> const&);
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
            CaseInsensitiveClassNames, //!< Names of ECClasses within one ECSchema must be case insensitive, i.e. must not only differ by case
            CaseInsensitivePropertyNames, //!< Names of ECProperties within one ECClass must be case insensitive, i.e. must not only differ by case
            SchemaAlias,
            NoPropertiesOfSameTypeAsClass, //!< Struct or array properties within an ECClass must not be of same type or derived type than the ECClass.
            ValidRelationshipConstraints
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
                explicit Error(Type ruleType)
                    : m_ruleType(ruleType)
                    {}

            public:
                virtual ~Error() {}

                Utf8String ToString() const;
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

    public:
        virtual ~ECSchemaValidationRule() {}

        bool ValidateSchemas(bvector<ECN::ECSchemaP> const& schemas, ECN::ECSchemaCR schema);
        bool ValidateSchema(ECN::ECSchemaCR schema, ECN::ECClassCR ecClass);
        bool ValidateClass(ECN::ECClassCR ecClass, ECN::ECPropertyCR ecProperty);

        void AddErrorToResult(ECSchemaValidationResult& result) const;
        Type GetType() const { return m_type; }
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      06/2014
//+===============+===============+===============+===============+===============+======
struct ECSchemaValidationResult : NonCopyableClass
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
// @bsiclass                                                Krischan.Eberle      06/2014
//+===============+===============+===============+===============+===============+======
struct CaseInsensitiveClassNamesRule : ECSchemaValidationRule
    {
    public:
        //=======================================================================================
        // @bsiclass                                                Krischan.Eberle      06/2014
        //+===============+===============+===============+===============+===============+======
        struct Error : ECSchemaValidationRule::Error
            {
            public:
                typedef bmap<Utf8CP, bset<ECN::ECClassCP>, CompareIUtf8Ascii> InvalidClasses;

            private:
                InvalidClasses m_invalidClasses;

                virtual Utf8String _ToString() const override;

            public:
                explicit Error(Type ruleType) : ECSchemaValidationRule::Error(ruleType) {}
                ~Error() {}

                InvalidClasses const& GetInvalidClasses() const { return m_invalidClasses; }
                InvalidClasses& GetInvalidClassesR() { return m_invalidClasses; }
            };

    private:
        bset<Utf8CP, CompareIUtf8Ascii> m_classNameSet;
        mutable std::unique_ptr<Error> m_error;

        virtual bool _ValidateSchema(ECN::ECSchemaCR schema, ECN::ECClassCR) override;
        virtual std::unique_ptr<ECSchemaValidationRule::Error> _GetError() const override;

    public:
        CaseInsensitiveClassNamesRule();
        ~CaseInsensitiveClassNamesRule() {}
    };


//=======================================================================================
// @bsiclass                                                Krischan.Eberle      06/2014
//+===============+===============+===============+===============+===============+======
struct CaseInsensitivePropertyNamesRule : ECSchemaValidationRule
    {
    private:
        //=======================================================================================
        // @bsiclass                                                Krischan.Eberle      06/2014
        //+===============+===============+===============+===============+===============+======
        struct Error : ECSchemaValidationRule::Error
            {
            public:
                typedef bmap<Utf8CP, bset<ECN::ECPropertyCP>, CompareIUtf8Ascii> InvalidProperties;

            private:
                ECN::ECClassCR m_ecClass;
                InvalidProperties m_invalidProperties;

                virtual Utf8String _ToString() const override;

            public:
                Error(Type ruleType, ECN::ECClassCR ecClass)
                    : ECSchemaValidationRule::Error(ruleType), m_ecClass(ecClass)
                    {}

                ~Error() {}

                InvalidProperties const& GetInvalidProperties() const { return m_invalidProperties; }
                InvalidProperties& GetInvalidPropertiesR() { return m_invalidProperties; }
            };

        bset<Utf8CP, CompareIUtf8Ascii> m_propertyNameSet;
        mutable std::unique_ptr<Error> m_error;

        virtual bool _ValidateClass(ECN::ECClassCR ecClass, ECN::ECPropertyCR ecProperty) override;
        virtual std::unique_ptr<ECSchemaValidationRule::Error> _GetError() const override;

    public:
        explicit CaseInsensitivePropertyNamesRule(ECN::ECClassCR ecClass);
        ~CaseInsensitivePropertyNamesRule() {}
    };
//=======================================================================================
// @bsiclass                                                Krischan.Eberle      06/2014
//+===============+===============+===============+===============+===============+======
struct NoPropertiesOfSameTypeAsClassRule : ECSchemaValidationRule
    {
    private:
        //=======================================================================================
        // @bsiclass                                                Krischan.Eberle      06/2014
        //+===============+===============+===============+===============+===============+======
        struct Error : ECSchemaValidationRule::Error
            {
            private:
                ECN::ECClassCR m_ecClass;
                std::vector<ECN::ECPropertyCP> m_invalidProperties;

                virtual Utf8String _ToString() const override;

            public:
                Error(Type ruleType, ECN::ECClassCR ecClass)
                    : ECSchemaValidationRule::Error(ruleType), m_ecClass(ecClass)
                    {}

                ~Error() {}

                std::vector<ECN::ECPropertyCP> const& GetInvalidProperties() const { return m_invalidProperties; }
                std::vector<ECN::ECPropertyCP>& GetInvalidPropertiesR() { return m_invalidProperties; }
            };

        mutable std::unique_ptr<Error> m_error;

        virtual bool _ValidateClass(ECN::ECClassCR ecClass, ECN::ECPropertyCR ecProperty) override;
        virtual std::unique_ptr<ECSchemaValidationRule::Error> _GetError() const override;

    public:
        explicit NoPropertiesOfSameTypeAsClassRule(ECN::ECClassCR ecClass);
        ~NoPropertiesOfSameTypeAsClassRule() {}
    };


//=======================================================================================
// @bsiclass                                                Krischan.Eberle      07/2015
//+===============+===============+===============+===============+===============+======
struct ValidRelationshipConstraintsRule : ECSchemaValidationRule
    {
    private:
        //=======================================================================================
        // @bsiclass                                                Krischan.Eberle      07/2015
        //+===============+===============+===============+===============+===============+======
        struct Error : ECSchemaValidationRule::Error
            {
            enum class Kind
                {
                None = 0,
                HasAnyClassConstraint = 1,
                HasRelationshipClassAsConstraint = 2,
                HasIncompleteConstraintDefinition = 4,
                HasMultipleConstraintClasses = 8,
                HasKeyProperties = 16
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

                std::vector<Inconsistency> m_inconsistencies;

                virtual Utf8String _ToString() const override;


            public:
                explicit Error(Type ruleType) : ECSchemaValidationRule::Error(ruleType) {}
                ~Error() {}

                void AddInconsistency(ECN::ECRelationshipClassCR relClass, Kind kind, ECN::ECRelationshipClassCP relClassAsConstraint = nullptr) { m_inconsistencies.push_back(Inconsistency(relClass, kind, relClassAsConstraint)); }
                bool HasInconsistencies() const { return !m_inconsistencies.empty(); }
            };

        mutable std::unique_ptr<Error> m_error;

        virtual bool _ValidateSchema(ECN::ECSchemaCR schema, ECN::ECClassCR ecClass) override;
        virtual std::unique_ptr<ECSchemaValidationRule::Error> _GetError() const override;

        bool ValidateRelationshipClass(ECN::ECRelationshipClassCR) const;
        bool ValidateConstraint(ECN::ECRelationshipClassCR, ECN::ECRelationshipConstraintCR) const;

    public:
        ValidRelationshipConstraintsRule();
        ~ValidRelationshipConstraintsRule() {}
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      07/2015
//+===============+===============+===============+===============+===============+======
struct SchemaAliasRule : ECSchemaValidationRule
    {
    private:
        //=======================================================================================
        // @bsiclass                                                Krischan.Eberle      07/2015
        //+===============+===============+===============+===============+===============+======
        struct Error : ECSchemaValidationRule::Error
            {
            private:
                std::vector<ECN::ECSchemaCP> m_invalidAliases;

                virtual Utf8String _ToString() const override;

            public:
                explicit Error(Type ruleType) : ECSchemaValidationRule::Error(ruleType) {}
                ~Error() {}

                void AddInvalidAlias(ECN::ECSchemaCR schema) { m_invalidAliases.push_back(&schema); }
                bool HasInconsistencies() const { return !m_invalidAliases.empty(); }
            };

        mutable std::unique_ptr<Error> m_error;

        virtual bool _ValidateSchemas(bvector<ECN::ECSchemaP> const&, ECN::ECSchemaCR) override;
        virtual std::unique_ptr<ECSchemaValidationRule::Error> _GetError() const override;

    public:
        SchemaAliasRule();
        ~SchemaAliasRule() {}
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
