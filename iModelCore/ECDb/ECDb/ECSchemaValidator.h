/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSchemaValidator.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
    ECSchemaValidator ();
    ~ECSchemaValidator ();

    static bool ValidateClass (ECSchemaValidationResult& result, ECN::ECClassCR ecClass, bool supportLegacySchemas);

public:
    static bool ValidateSchema (ECSchemaValidationResult& result, ECN::ECSchemaCR schema, bool supportLegacySchemas);
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
        NoPropertiesOfSameTypeAsClass, //!< Struct or array properties within an ECClass must not be of same type or derived type than the ECClass.
        StructWithRegularBaseClass, //!< Struct classes can not have regular class as struct class. 
        InvalidMapStrategy //!< ECClass has ClassMap custom attribute with invalid values for the properties MapStrategy or MapStrategyOptions
        };

    //=======================================================================================
    // @bsiclass                                                Krischan.Eberle      06/2014
    //+===============+===============+===============+===============+===============+======
    struct Error
        {
    private:
        Type m_ruleType;

        virtual Utf8String _ToString () const = 0;

    protected:
        explicit Error (Type ruleType)
            : m_ruleType (ruleType) 
            {}
    
    public:
        virtual ~Error () {}

        Utf8String ToString () const;
        Type GetRuleType () const { return m_ruleType; }
        };
private:
    Type m_type;

    virtual bool _ValidateSchema (ECN::ECSchemaCR schema, ECN::ECClassCR ecClass) { return true; }
    virtual bool _ValidateClass (ECN::ECClassCR ecClass, ECN::ECPropertyCR ecProperty) { return true; }
    virtual std::unique_ptr<Error> _GetError () const = 0;

protected:
    explicit ECSchemaValidationRule (Type type)
         : m_type (type)
        {}

public:
    virtual ~ECSchemaValidationRule () {}

    bool ValidateSchema (ECN::ECSchemaCR schema, ECN::ECClassCR ecClass);
    bool ValidateClass (ECN::ECClassCR ecClass, ECN::ECPropertyCR ecProperty);

    void AddErrorToResult (ECSchemaValidationResult& result) const;
    Type GetType () const { return m_type; }
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      06/2014
//+===============+===============+===============+===============+===============+======
struct ECSchemaValidationResult : NonCopyableClass
    {
private:
    std::vector<std::unique_ptr<ECSchemaValidationRule::Error>> m_errors;

public:
    ECSchemaValidationResult () {}

    void AddError (std::unique_ptr<ECSchemaValidationRule::Error> error);

    bool HasErrors () const { return !m_errors.empty (); }

    std::vector<std::unique_ptr<ECSchemaValidationRule::Error>> const& GetErrors () const { return m_errors; }

    void ToString (std::vector<Utf8String>& errorMessages) const;
    };


//**************************** Subclasses *********************************************
//=======================================================================================
// @bsiclass                             Muhammad.Zaighum                  04/2015
//+===============+===============+===============+===============+===============+======
struct MapStrategyRule : ECSchemaValidationRule
    {
public:
    //=======================================================================================
    // @bsiclass                  Muhammad.Zaighum                          07/2015
    //+===============+===============+===============+===============+===============+======
    struct Error : ECSchemaValidationRule::Error
        {
    private:
        bvector<ECN::ECClassCP> m_classesWithInvalidMapStrategy;
        virtual Utf8String _ToString() const override;
    public:
        Error(Type ruleType) : ECSchemaValidationRule::Error(ruleType) {}
        ~Error() {}

        void AddClassWithInvalidMapStrategy(ECN::ECClassCR ecClass) { m_classesWithInvalidMapStrategy.push_back(&ecClass); }
        bool IsError() const { return !m_classesWithInvalidMapStrategy.empty(); }
        };

private:
    mutable std::unique_ptr<Error> m_error;
    virtual bool _ValidateSchema(ECN::ECSchemaCR schema, ECN::ECClassCR ecClass) override;
    virtual std::unique_ptr<ECSchemaValidationRule::Error> _GetError() const override;
public:
    MapStrategyRule();
    ~MapStrategyRule() {}
    };

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
        typedef bmap<WCharCP, bset<ECN::ECClassCP>, CompareIWChar> InvalidClasses;

    private:
        InvalidClasses m_invalidClasses;
        bool m_supportLegacySchemas;

        virtual Utf8String _ToString () const override;

    public:
        explicit Error (Type ruleType, bool supportLegacySchemas) : ECSchemaValidationRule::Error (ruleType), m_supportLegacySchemas (supportLegacySchemas) {}
        ~Error () {}

        bset<ECN::ECClassCP> const* TryGetInvalidClasses (ECN::ECClassCR ecClass) const;

        InvalidClasses const& GetInvalidClasses () const { return m_invalidClasses; }
        InvalidClasses& GetInvalidClassesR () { return m_invalidClasses; }
        };

private:
    bool m_supportLegacySchemas;
    bset<WCharCP, CompareIWChar> m_classNameSet;
    mutable std::unique_ptr<Error> m_error;

    virtual bool _ValidateSchema (ECN::ECSchemaCR schema, ECN::ECClassCR ecClass) override;
    virtual std::unique_ptr<ECSchemaValidationRule::Error> _GetError () const override;

public:
    explicit CaseInsensitiveClassNamesRule (bool supportLegacySchemas);
    ~CaseInsensitiveClassNamesRule () {}
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
        typedef bmap<WCharCP, bset<ECN::ECPropertyCP>, CompareIWChar> InvalidProperties;

    private:
        ECN::ECClassCR m_ecClass;
        InvalidProperties m_invalidProperties;
        bool m_supportLegacySchemas;

        virtual Utf8String _ToString () const override;

    public:
        Error (Type ruleType, ECN::ECClassCR ecClass, bool supportLegacySchemas)
            : ECSchemaValidationRule::Error (ruleType), m_ecClass (ecClass), m_supportLegacySchemas (supportLegacySchemas)
            {}

        ~Error () {}

        InvalidProperties const& GetInvalidProperties () const { return m_invalidProperties; }
        InvalidProperties& GetInvalidPropertiesR () { return m_invalidProperties; }
        };

    bool m_supportLegacySchemas;
    bset<WCharCP, CompareIWChar> m_propertyNameSet;
    mutable std::unique_ptr<Error> m_error;

    virtual bool _ValidateClass (ECN::ECClassCR ecClass, ECN::ECPropertyCR ecProperty) override;
    virtual std::unique_ptr<ECSchemaValidationRule::Error> _GetError () const override;

public:
    CaseInsensitivePropertyNamesRule (ECN::ECClassCR ecClass, bool supportLegacySchemas);

    ~CaseInsensitivePropertyNamesRule () {}
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

        virtual Utf8String _ToString () const override;

    public:
        Error (Type ruleType, ECN::ECClassCR ecClass)
            : ECSchemaValidationRule::Error (ruleType), m_ecClass (ecClass)
            {}

        ~Error () {}

        std::vector<ECN::ECPropertyCP> const& GetInvalidProperties () const { return m_invalidProperties; }
        std::vector<ECN::ECPropertyCP>& GetInvalidPropertiesR () { return m_invalidProperties; }
        };

    mutable std::unique_ptr<Error> m_error;

    virtual bool _ValidateClass (ECN::ECClassCR ecClass, ECN::ECPropertyCR ecProperty) override;
    virtual std::unique_ptr<ECSchemaValidationRule::Error> _GetError () const override;

public:
    explicit NoPropertiesOfSameTypeAsClassRule (ECN::ECClassCR ecClass);
    ~NoPropertiesOfSameTypeAsClassRule () {}
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan           05/2015
//+===============+===============+===============+===============+===============+======
struct StructWithRegularBaseClassRule : ECSchemaValidationRule
    {
    public:
        //=======================================================================================
        // @bsiclass                                                Affan.Khan           05/2015
        //+===============+===============+===============+===============+===============+======
        struct Error : ECSchemaValidationRule::Error
            {
            public:
                typedef bmap<ECN::ECClassCP, bvector<ECN::ECClassCP>> InvalidClasses;

            private:
                InvalidClasses m_invalidClasses;
                bool m_supportLegacySchemas;

                virtual Utf8String _ToString () const override;

            public:
                explicit Error (Type ruleType, bool supportLegacySchemas)
                    : ECSchemaValidationRule::Error (ruleType), m_supportLegacySchemas (supportLegacySchemas)
                    {}

                ~Error () {}

                bvector<ECN::ECClassCP> const* TryGetInvalidBaseClasses (ECN::ECClassCR ecClass) const;

                InvalidClasses const& GetInvalidClasses () const { return m_invalidClasses; }
                InvalidClasses& GetInvalidClassesR () { return m_invalidClasses; }
             
            };

    private:
        bool m_supportLegacySchemas;
        bset<WCharCP, CompareIWChar> m_classNameSet;
        mutable std::unique_ptr<Error> m_error;

        virtual bool _ValidateSchema (ECN::ECSchemaCR schema, ECN::ECClassCR ecClass) override;
        virtual std::unique_ptr<ECSchemaValidationRule::Error> _GetError () const override;

    public:
        explicit StructWithRegularBaseClassRule (bool supportLegacySchemas);
        ~StructWithRegularBaseClassRule () {}
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
