/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/ECSchemaValidator.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/
#include <ECObjects/ECSchema.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE
//=====================================================================================
// Interface for schema level validation. When implemented and added to ECSchemaValidator,
// it will be called to validate a schema.
//@bsiclass
//+===============+===============+===============+===============+===============+======
struct IECSchemaValidator : RefCountedBase, NonCopyableClass
    {
    //! Converts the custom attribute
    //! @param[in] schema   The schema to run validation on
    virtual ECObjectsStatus Validate(ECSchemaR schema) const = 0;

    //! destructor
    virtual ~IECSchemaValidator() {};
    };

typedef RefCountedPtr<IECSchemaValidator> IECSchemaValidatorPtr;

struct IECClassValidator : RefCountedBase, NonCopyableClass
{
    virtual ECObjectsStatus Validate(ECClassCR ecClass) const = 0;
    virtual bool CanValidate(ECClassCR ecClass) const = 0;

    //! destructor
    virtual ~IECClassValidator() {};
};

struct IKindOfQuantityValidator : RefCountedBase, NonCopyableClass
    {
    virtual ECObjectsStatus Validate(KindOfQuantityCP koq) const = 0;

    //! destructor
    virtual ~IKindOfQuantityValidator() {};
    };

typedef RefCountedPtr<IECClassValidator> IECClassValidatorPtr;
typedef RefCountedPtr<IKindOfQuantityValidator> IKindOfQuantityValidatorPtr;

struct ECSchemaValidator
    {
private:
    ECSchemaValidator() {}
    ECSchemaValidator(const ECSchemaConverter & rhs) = delete;
    ECSchemaValidator & operator= (const ECSchemaValidator & rhs) = delete;

    static ECSchemaValidatorP GetSingleton();
    bool m_validated;
    bvector<IECSchemaValidatorPtr> m_validators;
    bvector<IECClassValidatorPtr> m_classValidators;
    bvector<IKindOfQuantityValidatorPtr> m_koqValidators;

    void ValidateSchema(ECSchemaR schema);
    bvector<IECSchemaValidatorPtr>const & GetValidators() { return m_validators; }
    bvector<IECClassValidatorPtr>const & GetClassValidators() { return m_classValidators; }
    bvector<IKindOfQuantityValidatorPtr>const & GetKindOfQuantityValidators() { return m_koqValidators; }

public:
    //! Validates the schema based on the added validators.
    //! @param[in] schema   The schema to traverse
    ECOBJECTS_EXPORT static bool Validate(ECSchemaR schema);

    //! Adds the supplied IECSchemaValidatorP which will be later called when ECSchemaValidator::Validate is run
    ECOBJECTS_EXPORT static ECObjectsStatus AddValidator(IECSchemaValidatorPtr& validator);
    ECOBJECTS_EXPORT static ECObjectsStatus AddClassValidator(IECClassValidatorPtr& validator);
    ECOBJECTS_EXPORT static ECObjectsStatus AddKindOfQuantityValidator(IKindOfQuantityValidatorPtr& validator);
    };

struct BaseECValidator : IECSchemaValidator
    {
    ECObjectsStatus Validate(ECSchemaR schema) const override;
    };

struct BaseECClassValidator : IECClassValidator
    {
    ECObjectsStatus Validate(ECClassCR ecClass) const override {return ECObjectsStatus::Success;}
    };

struct MixinValidator : IECClassValidator
    {
    // A mixin may have 0 or 1 base class
    // A mxin may not override an inherited property
    ECObjectsStatus Validate(ECClassCR ecClass) const override;
    bool CanValidate(ECClassCR ecClass) const override {return ecClass.IsEntityClass() && ecClass.GetEntityClassCP()->IsMixin();}
    };

struct EntityValidator : IECClassValidator
    {
    // An entity class may not inherit a property from more than one base class
    // An entity class may not override a property inherited from a mixin class
    // ... etc
    ECObjectsStatus Validate(ECClassCR ecClass) const override;
    bool CanValidate(ECClassCR ecClass) const override {return ecClass.IsEntityClass();}
    };

struct RelationshipValidator : IECClassValidator
    {
    // Relationship strength and local definition validation
    ECObjectsStatus Validate(ECClassCR ecClass) const override;
    bool CanValidate(ECClassCR ecClass) const override {return ecClass.IsRelationshipClass();}
    ECObjectsStatus CheckLocalDefinitions(ECRelationshipConstraintCR constraint, Utf8String constraintType) const;
    };

struct KindOfQuantityValidator : IKindOfQuantityValidator
    {
    // KindOfQuantities should only use SI units for persistence units
    ECObjectsStatus Validate(KindOfQuantityCP koq) const;
    };
END_BENTLEY_ECOBJECT_NAMESPACE

