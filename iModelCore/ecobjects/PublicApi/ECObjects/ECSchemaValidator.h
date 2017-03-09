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

struct ECSchemaValidator
    {
private:
    ECSchemaValidator() {}
    ECSchemaValidator(const ECSchemaConverter & rhs) = delete;
    ECSchemaValidator & operator= (const ECSchemaValidator & rhs) = delete;

    static ECSchemaValidatorP GetSingleton();
    bool m_validated;
    bvector<IECSchemaValidatorPtr> m_validators;

    void ValidateSchema(ECSchemaR schema);
    bvector<IECSchemaValidatorP> GetValidators();

public:
    //! Validates the schema based on the added validators.
    //! @param[in] schema   The schema to traverse
    ECOBJECTS_EXPORT static bool Validate(ECSchemaR schema);

    //! Adds the supplied IECSchemaValidatorP which will be later called when ECSchemaValidator::Validate is run
    ECOBJECTS_EXPORT static ECObjectsStatus AddValidator(IECSchemaValidatorPtr& validator);
    };

struct BaseECValidator : IECSchemaValidator
    {
    ECObjectsStatus Validate(ECSchemaR schema) const override;
    };

END_BENTLEY_ECOBJECT_NAMESPACE

