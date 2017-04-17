/*--------------------------------------------------------------------------------------+
|
|     $Source: src/ECSchemaValidator.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"
BEGIN_BENTLEY_ECOBJECT_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                    Caleb.Shafer                  02/2017
//+---------------+---------------+---------------+---------------+---------------+------
// static
ECSchemaValidatorP ECSchemaValidator::GetSingleton()
    {
    static ECSchemaValidatorP ECSchemaValidatorSingleton = nullptr;

    if (nullptr == ECSchemaValidatorSingleton)
        {
        ECSchemaValidatorSingleton = new ECSchemaValidator();

        IECSchemaValidatorPtr baseECValidater = new BaseECValidator();
        ECSchemaValidatorSingleton->AddValidator(baseECValidater);
        }    

    return ECSchemaValidatorSingleton;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Caleb.Shafer                  02/2017
//+---------------+---------------+---------------+---------------+---------------+------
// static
ECObjectsStatus ECSchemaValidator::AddValidator(IECSchemaValidatorPtr& validator)
    {
    ECSchemaValidatorP schemaValidator = GetSingleton();
    if (0 == schemaValidator->m_validators.size())
        schemaValidator->m_validators.push_back(validator);
    else
        {
        LOG.errorv("Multiple validators not supported yet.");
        return ECObjectsStatus::Error;
        }

    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Dan.Perlman                  04/2017
//+---------------+---------------+---------------+---------------+---------------+------
// static
ECObjectsStatus ECSchemaValidator::AddClassValidator(IECClassValidatorPtr& validator)
{
    ECSchemaValidatorP schemaValidator = GetSingleton();
    if (0 == schemaValidator->m_classValidators.size())
        schemaValidator->m_classValidators.push_back(validator);
    else
    {
        LOG.errorv("Multiple validators not supported yet.");
        return ECObjectsStatus::Error;
    }

    return ECObjectsStatus::Success;
}
//---------------------------------------------------------------------------------------
// @bsimethod                                    Caleb.Shafer                  02/2017
//+---------------+---------------+---------------+---------------+---------------+------
// static
bool ECSchemaValidator::Validate(ECSchemaR schema)
    {
    ECSchemaValidatorP schemaValidator = GetSingleton();
    schemaValidator->m_validated = true;

    schemaValidator->ValidateSchema(schema);

    return schemaValidator->m_validated;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Caleb.Shafer                  02/2017
//+---------------+---------------+---------------+---------------+---------------+------
bvector<IECSchemaValidatorP> ECSchemaValidator::GetValidators()
    {
    bvector<IECSchemaValidatorP> validatorVector;

    for (auto validator : m_validators)
        validatorVector.push_back(validator.get());

    return validatorVector;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Dan.Perlman                  04/2017
//+---------------+---------------+---------------+---------------+---------------+------
bvector<IECClassValidatorP> ECSchemaValidator::GetClassValidators()
{
    bvector<IECClassValidatorP> validatorVector;

    for (IECClassValidatorPtr validator : m_classValidators)
        validatorVector.push_back(validator.get());

    return validatorVector;
}
//---------------------------------------------------------------------------------------
// @bsimethod                                    Caleb.Shafer                  02/2017
//+---------------+---------------+---------------+---------------+---------------+------
void ECSchemaValidator::ValidateSchema(ECSchemaR schema)
    {
    for (IECSchemaValidatorP validator : GetValidators())
        {
        ECObjectsStatus status = validator->Validate(schema);
        if (ECObjectsStatus::Success != status)
            {
            LOG.errorv("Failed to validate %s", schema.GetFullSchemaName().c_str());
            m_validated = false;
            }
        else
            LOG.debugv("Succeeded validation of %s", schema.GetFullSchemaName().c_str());
        }
    for (ECClassCP ecClass : schema.GetClasses())
        {
        for (IECClassValidatorP classValidator : GetClassValidators())
            {
            ECObjectsStatus status = classValidator->Validate(*ecClass);
            if (ECObjectsStatus::Success != status)
                {
                LOG.errorv("Failed to validate %s", schema.GetFullSchemaName().c_str());
                m_validated = false;
                }
            else
                LOG.debugv("Succeeded validation of %s", schema.GetFullSchemaName().c_str());
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Caleb.Shafer                  02/2017
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus BaseECValidator::Validate(ECSchemaR schema) const
    {
    if (!schema.Validate() || !schema.IsECVersion(ECVersion::Latest))
        {
        LOG.errorv("Failed to validate %s as ECVersion, %s", schema.GetFullSchemaName().c_str(), ECSchema::GetECVersionString(ECVersion::Latest));
        return ECObjectsStatus::Error;
        }

    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Dan.Perlman                  04/2017
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus MixInValidator::Validate(ECClassCR mixin) const
    {
    if (!mixin.IsEntityClass() || !mixin.GetEntityClassCP()->IsMixin())
        return ECObjectsStatus::Error;

    if (mixin.GetBaseClasses().size() > 1)
        {
        LOG.errorv("Mixin %s has more than 1 base class", mixin.GetName().c_str());
        return ECObjectsStatus::Error;
        } 
       
    // if ... [other rules]

    // else, success
    return ECObjectsStatus::Success;
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                    Daniel.Perlman                  04/2017
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus BaseECClassValidator::Validate(ECClassCR schema) const
    {
    return ECObjectsStatus::Success;
    }

END_BENTLEY_ECOBJECT_NAMESPACE
