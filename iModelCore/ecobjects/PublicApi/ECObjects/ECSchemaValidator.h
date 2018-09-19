/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/ECSchemaValidator.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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

    ECOBJECTS_EXPORT void AddSchemaValidator(Validator<ECSchemaCR> validator);
    ECOBJECTS_EXPORT void AddClassValidator(Validator<ECClassCR> validator);
    ECOBJECTS_EXPORT void AddKindOfQuantityValidator(Validator<KindOfQuantityCR> validator);
};

END_BENTLEY_ECOBJECT_NAMESPACE
