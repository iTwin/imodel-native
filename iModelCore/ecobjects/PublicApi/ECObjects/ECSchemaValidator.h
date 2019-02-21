/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/ECSchemaValidator.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/
#include <ECObjects/ECSchema.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

template <typename T>
using Validator = std::function<ECObjectsStatus(T)>;

struct ECSchemaValidator
{
private:
    ECSchemaValidator(ECSchemaConverter const& rhs) = delete;
    ECSchemaValidator& operator= (ECSchemaValidator const& rhs) = delete;

    bool m_validated;
    bvector<Validator<ECSchemaCR>> m_schemaValidators;
    bvector<Validator<ECClassCR>> m_classValidators;
    bvector<Validator<KindOfQuantityCR>> m_koqValidators;

    void RunSchemaValidators(ECSchemaCR schema);
    void RunClassValidators(ECClassCR ecClass);
    void RunKindOfQuantityValidators(KindOfQuantityCR koq);

    // Current "built-in" bis validators.
    static ECObjectsStatus BaseECValidator(ECSchemaCR);
    static ECObjectsStatus AllClassValidator(ECClassCR);
    static ECObjectsStatus EntityValidator(ECClassCR);
    static ECObjectsStatus MixinValidator(ECClassCR);
    static ECObjectsStatus StructValidator(ECClassCR);
    static ECObjectsStatus CustomAttributeClassValidator(ECClassCR);
    static ECObjectsStatus RelationshipValidator(ECClassCR);
    static ECObjectsStatus KindOfQuantityValidator(KindOfQuantityCR);

public:
    ECOBJECTS_EXPORT ECSchemaValidator();

    //! Run the provided schema against all added validators.
    ECOBJECTS_EXPORT bool Validate(ECSchemaCR schema);

    void AddSchemaValidator(Validator<ECSchemaCR> validator) {m_schemaValidators.push_back(validator);}
    void AddClassValidator(Validator<ECClassCR> validator) {m_classValidators.push_back(validator);}
    void AddKindOfQuantityValidator(Validator<KindOfQuantityCR> validator) {m_koqValidators.push_back(validator);}
};

END_BENTLEY_ECOBJECT_NAMESPACE
