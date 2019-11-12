/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECObjects/ECObjectsAPI.h>
#include <BimFromDgnDb/BimFromJson.h>

#define LOG (*NativeLogging::LoggingManager::GetLogger (L"BimFromDgnDbUpgrader"))
#include "SchemaFlattener.h"

USING_NAMESPACE_BENTLEY_EC
BEGIN_BIM_FROM_DGNDB_NAMESPACE

#define BIS_ECSCHEMA_NAME       "BisCore"
#define BIS_CLASS_Element                   "Element"

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            03/2018
//---------------+---------------+---------------+---------------+---------------+-------
bool excludeSchema(ECN::ECSchemaCR schema)
    {
    return schema.IsStandardSchema() || schema.IsSystemSchema() || schema.IsSupplementalSchema() ||
        schema.GetName().EqualsI("DgnDbSyncV8") || schema.GetName().EqualsI(BIS_ECSCHEMA_NAME) ||
        schema.GetName().EqualsIAscii("Generic") || schema.GetName().EqualsIAscii("Functional") ||
        schema.GetName().StartsWithI("ecdb") || schema.GetName().EqualsIAscii("ECv3ConversionAttributes");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2017
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus SchemaFlattener::CopyFlatCustomAttributes(ECN::IECCustomAttributeContainerR targetContainer, ECN::IECCustomAttributeContainerCR sourceContainer)
    {
    for (ECN::IECInstancePtr instance : sourceContainer.GetCustomAttributes(true))
        {
        if (excludeSchema(instance->GetClass().GetSchema()))
            continue;

        if (instance->GetClass().GetName().Equals("CalculatedECPropertySpecification") && instance->GetClass().GetSchema().GetName().Equals("Bentley_Standard_CustomAttributes"))
            continue;

        ECN::ECSchemaPtr flatCustomAttributeSchema = m_flattenedRefs[instance->GetClass().GetSchema().GetName()];
        if (!flatCustomAttributeSchema.IsValid())
            {
            LOG.warningv("Failed to find ECSchema '%s' for custom attribute '%'.  Skipping custom attribute.", Utf8String(instance->GetClass().GetFullName()).c_str());
            continue;
            }
        ECN::IECInstancePtr copiedCA = instance->CreateCopyThroughSerialization(*flatCustomAttributeSchema);
        if (!copiedCA.IsValid())
            {
            LOG.warningv("Failed to copy custom attribute '%s'. Skipping custom attribute.", Utf8String(instance->GetClass().GetFullName()).c_str());
            continue;
            }
        if (!ECN::ECSchema::IsSchemaReferenced(*targetContainer.GetContainerSchema(), *flatCustomAttributeSchema))
            targetContainer.GetContainerSchema()->AddReferencedSchema(*flatCustomAttributeSchema);

        targetContainer.SetCustomAttribute(*copiedCA);
        }
    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2017
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus SchemaFlattener::CopyFlatConstraint(ECN::ECRelationshipConstraintR toRelationshipConstraint, ECN::ECRelationshipConstraintCR fromRelationshipConstraint)
    {
    if (fromRelationshipConstraint.IsRoleLabelDefined() && ECN::ECObjectsStatus::Success != toRelationshipConstraint.SetRoleLabel(fromRelationshipConstraint.GetInvariantRoleLabel().c_str()))
        return BSIERROR;
    if (ECN::ECObjectsStatus::Success != toRelationshipConstraint.SetMultiplicity(fromRelationshipConstraint.GetMultiplicity()))
        return BSIERROR;
    if (ECN::ECObjectsStatus::Success != toRelationshipConstraint.SetIsPolymorphic(fromRelationshipConstraint.GetIsPolymorphic()))
        return BSIERROR;

    ECN::ECSchemaP destSchema = const_cast<ECN::ECSchemaP>(&(toRelationshipConstraint.GetRelationshipClass().GetSchema()));

    for (auto constraintClass : fromRelationshipConstraint.GetConstraintClasses())
        {
        ECN::ECClassP destConstraintClass = nullptr;
        if (toRelationshipConstraint.GetRelationshipClass().GetSchema().GetSchemaKey() != constraintClass->GetSchema().GetSchemaKey())
            {
            ECN::ECSchemaPtr flatBaseSchema = m_flattenedRefs[constraintClass->GetSchema().GetName()];
            destConstraintClass = flatBaseSchema->GetClassP(constraintClass->GetName().c_str());
            }
        else
            destConstraintClass = destSchema->GetClassP(constraintClass->GetName().c_str());

        // All classes should already have been created
        if (nullptr == destConstraintClass)
            return BSIERROR;
        if (ECN::ECObjectsStatus::Success != toRelationshipConstraint.AddClass(*destConstraintClass->GetEntityClassCP()))
            return BSIERROR;
        }
    CopyFlatCustomAttributes(toRelationshipConstraint, fromRelationshipConstraint);
    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2017
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus SchemaFlattener::CreateFlatClass(ECN::ECClassP& targetClass, ECN::ECSchemaP flatSchema, ECN::ECClassCP sourceClass)
    {
    ECN::ECRelationshipClassCP sourceAsRelationshipClass = sourceClass->GetRelationshipClassCP();
    ECN::ECStructClassCP sourceAsStructClass = sourceClass->GetStructClassCP();
    ECN::ECCustomAttributeClassCP sourceAsCAClass = sourceClass->GetCustomAttributeClassCP();
    if (nullptr != sourceAsRelationshipClass)
        {
        ECN::ECRelationshipClassP newRelationshipClass;
        if (ECN::ECObjectsStatus::Success != flatSchema->CreateRelationshipClass(newRelationshipClass, sourceClass->GetName()))
            return BSIERROR;
        newRelationshipClass->SetStrength(sourceAsRelationshipClass->GetStrength());
        newRelationshipClass->SetStrengthDirection(sourceAsRelationshipClass->GetStrengthDirection());

        CopyFlatConstraint(newRelationshipClass->GetSource(), sourceAsRelationshipClass->GetSource());
        CopyFlatConstraint(newRelationshipClass->GetTarget(), sourceAsRelationshipClass->GetTarget());
        targetClass = newRelationshipClass;
        }
    else if (nullptr != sourceAsStructClass)
        {
        ECN::ECStructClassP newStructClass;
        if (ECN::ECObjectsStatus::Success != flatSchema->CreateStructClass(newStructClass, sourceClass->GetName()))
            return BSIERROR;
        targetClass = newStructClass;
        }
    else if (nullptr != sourceAsCAClass)
        {
        ECN::ECCustomAttributeClassP newCAClass;
        if (ECN::ECObjectsStatus::Success != flatSchema->CreateCustomAttributeClass(newCAClass, sourceClass->GetName()))
            return BSIERROR;
        newCAClass->SetContainerType(sourceAsCAClass->GetContainerType());
        targetClass = newCAClass;
        }
    else
        {
        ECN::ECEntityClassP newEntityClass;
        if (ECN::ECObjectsStatus::Success != flatSchema->CreateEntityClass(newEntityClass, sourceClass->GetName()))
            return BSIERROR;
        targetClass = newEntityClass;
        }

    if (sourceClass->GetIsDisplayLabelDefined())
        targetClass->SetDisplayLabel(sourceClass->GetInvariantDisplayLabel());
    targetClass->SetDescription(sourceClass->GetInvariantDescription());
    targetClass->SetClassModifier(sourceClass->GetClassModifier());

    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2017
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus SchemaFlattener::CopyFlatClass(ECN::ECClassP& targetClass, ECN::ECSchemaP flatSchema, ECN::ECClassCP sourceClass)
    {
    if (BSISUCCESS != CreateFlatClass(targetClass, flatSchema, sourceClass))
        return BSIERROR;

    for (ECN::ECPropertyCP sourceProperty : sourceClass->GetProperties(true))
        {
        if (BSISUCCESS != CopyFlattenedProperty(targetClass, sourceProperty))
            return BSIERROR;
        }

    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2017
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus SchemaFlattener::CopyFlattenedProperty(ECN::ECClassP targetClass, ECN::ECPropertyCP sourceProperty)
    {
    if (excludeSchema(sourceProperty->GetClass().GetSchema()))
        return BSISUCCESS;

    // Only copy properties either directly on the same class or on a base class that was dropped.  Don't copy properties from base classes that are still set
    if (0 != strcmp(targetClass->GetFullName(), sourceProperty->GetClass().GetFullName()))
        {
        ECN::ECClassP targetPropertyClass = nullptr;
        ECN::ECSchemaPtr flatSchema = m_flattenedRefs[sourceProperty->GetClass().GetSchema().GetName()];
        if (!flatSchema.IsValid())
            {
            if (Utf8String(sourceProperty->GetClass().GetSchema().GetName()).StartsWithIAscii("SP3D"))
                targetPropertyClass = targetClass->GetSchemaR().GetClassP(sourceProperty->GetClass().GetName().c_str());
            else
                return BSIERROR;
            }
        else
            targetPropertyClass = flatSchema->GetClassP(sourceProperty->GetClass().GetName().c_str());
        if (targetClass->Is(targetPropertyClass))
            return BSISUCCESS;
        }

    ECN::ECPropertyP destProperty = nullptr;
    if (sourceProperty->GetIsPrimitive())
        {
        ECN::PrimitiveECPropertyP destPrimitive;
        ECN::PrimitiveECPropertyCP sourcePrimitive = sourceProperty->GetAsPrimitiveProperty();
        ECN::ECEnumerationCP enumeration = sourcePrimitive->GetEnumeration();
        if (nullptr != enumeration)
            {
            if (enumeration->GetSchema().GetSchemaKey() == sourceProperty->GetClass().GetSchema().GetSchemaKey())
                {
                ECN::ECEnumerationP destEnum = targetClass->GetSchemaR().GetEnumerationP(enumeration->GetName().c_str());
                if (nullptr == destEnum)
                    {
                    auto status = targetClass->GetSchemaR().CopyEnumeration(destEnum, *enumeration);
                    if (ECN::ECObjectsStatus::Success != status && ECN::ECObjectsStatus::NamedItemAlreadyExists != status)
                        return BSIERROR;
                    }
                targetClass->CreateEnumerationProperty(destPrimitive, sourceProperty->GetName(), *destEnum);
                }
            else
                {
                ECN::ECSchemaPtr flatBaseSchema = m_flattenedRefs[enumeration->GetSchema().GetName()];
                targetClass->CreateEnumerationProperty(destPrimitive, sourceProperty->GetName(), *flatBaseSchema->GetEnumerationP(enumeration->GetName().c_str()));
                }
            }
        else
            targetClass->CreatePrimitiveProperty(destPrimitive, sourceProperty->GetName(), sourcePrimitive->GetType());

        if (sourcePrimitive->IsMinimumValueDefined())
            {
            ECN::ECValue valueToCopy;
            sourcePrimitive->GetMinimumValue(valueToCopy);
            destPrimitive->SetMinimumValue(valueToCopy);
            }

        if (sourcePrimitive->IsMaximumValueDefined())
            {
            ECN::ECValue valueToCopy;
            sourcePrimitive->GetMaximumValue(valueToCopy);
            destPrimitive->SetMaximumValue(valueToCopy);
            }

        if (sourcePrimitive->IsMinimumLengthDefined())
            destPrimitive->SetMinimumLength(sourcePrimitive->GetMinimumLength());
        if (sourcePrimitive->IsMaximumLengthDefined())
            destPrimitive->SetMaximumLength(sourcePrimitive->GetMaximumLength());

        if (sourcePrimitive->IsExtendedTypeDefinedLocally())
            destPrimitive->SetExtendedTypeName(sourcePrimitive->GetExtendedTypeName().c_str());
        destProperty = destPrimitive;
        }
    else if (sourceProperty->GetIsStructArray())
        {
        ECN::StructArrayECPropertyP destArray;
        ECN::StructArrayECPropertyCP sourceArray = sourceProperty->GetAsStructArrayProperty();
        ECN::ECStructClassCR structElementType = sourceArray->GetStructElementType();
        if (structElementType.GetSchema().GetSchemaKey() == targetClass->GetSchema().GetSchemaKey())
            {
            ECN::ECClassP destClass = targetClass->GetSchemaR().GetClassP(structElementType.GetName().c_str());
            if (nullptr == destClass)
                {
                auto status = CopyFlatClass(destClass, &(targetClass->GetSchemaR()), &structElementType);
                if (BSISUCCESS != status)
                    return BSIERROR;
                }
            targetClass->CreateStructArrayProperty(destArray, sourceProperty->GetName(), *destClass->GetStructClassCP());
            }
        else
            {
            ECN::ECSchemaPtr flatBaseSchema = m_flattenedRefs[structElementType.GetSchema().GetName()];
            targetClass->CreateStructArrayProperty(destArray, sourceProperty->GetName(), *flatBaseSchema->GetClassP(structElementType.GetName().c_str())->GetStructClassP());
            }

        destArray->SetMaxOccurs(sourceArray->GetStoredMaxOccurs());
        destArray->SetMinOccurs(sourceArray->GetMinOccurs());
        destProperty = destArray;
        }
    else if (sourceProperty->GetIsPrimitiveArray())
        {
        ECN::PrimitiveArrayECPropertyP destArray;
        ECN::PrimitiveArrayECPropertyCP sourceArray = sourceProperty->GetAsPrimitiveArrayProperty();
        ECN::ECEnumerationCP enumeration = sourceArray->GetEnumeration();
        if (nullptr != enumeration)
            {
            if (enumeration->GetSchema().GetSchemaKey() == sourceProperty->GetClass().GetSchema().GetSchemaKey())
                {
                ECN::ECEnumerationP destEnum = targetClass->GetSchemaR().GetEnumerationP(enumeration->GetName().c_str());
                if (nullptr == destEnum)
                    {
                    auto status = targetClass->GetSchemaR().CopyEnumeration(destEnum, *enumeration);
                    if (ECN::ECObjectsStatus::Success != status && ECN::ECObjectsStatus::NamedItemAlreadyExists != status)
                        return BSIERROR;
                    }
                targetClass->CreatePrimitiveArrayProperty(destArray, sourceProperty->GetName(), *destEnum);
                }
            else
                {
                ECN::ECSchemaPtr flatBaseSchema = m_flattenedRefs[enumeration->GetSchema().GetName()];
                targetClass->CreatePrimitiveArrayProperty(destArray, sourceProperty->GetName(), *flatBaseSchema->GetEnumerationP(enumeration->GetName().c_str()));
                }
            }
        else
            targetClass->CreatePrimitiveArrayProperty(destArray, sourceProperty->GetName(), sourceArray->GetPrimitiveElementType());

        if (sourceArray->IsMinimumValueDefined())
            {
            ECN::ECValue valueToCopy;
            sourceArray->GetMinimumValue(valueToCopy);
            destArray->SetMinimumValue(valueToCopy);
            }

        if (sourceArray->IsMaximumValueDefined())
            {
            ECN::ECValue valueToCopy;
            sourceArray->GetMaximumValue(valueToCopy);
            destArray->SetMaximumValue(valueToCopy);
            }

        if (sourceArray->IsMinimumLengthDefined())
            destArray->SetMinimumLength(sourceArray->GetMinimumLength());
        if (sourceArray->IsMaximumLengthDefined())
            destArray->SetMaximumLength(sourceArray->GetMaximumLength());

        if (sourceArray->IsExtendedTypeDefinedLocally())
            destArray->SetExtendedTypeName(sourceArray->GetExtendedTypeName().c_str());

        destArray->SetMaxOccurs(sourceArray->GetStoredMaxOccurs());
        destArray->SetMinOccurs(sourceArray->GetMinOccurs());
        destProperty = destArray;
        }
    else if (sourceProperty->GetIsStruct())
        {
        ECN::StructECPropertyP destStruct;
        ECN::StructECPropertyCP sourceStruct = sourceProperty->GetAsStructProperty();
        ECN::ECStructClassCR sourceType = sourceStruct->GetType();
        if (sourceType.GetSchema().GetSchemaKey() == sourceProperty->GetClass().GetSchema().GetSchemaKey())
            {
            ECN::ECClassP destClass = targetClass->GetSchemaR().GetClassP(sourceType.GetName().c_str());
            if (nullptr == destClass)
                {
                auto status = CopyFlatClass(destClass, &(targetClass->GetSchemaR()), &sourceType);
                if (BSISUCCESS != status)
                    return BSIERROR;
                }
            targetClass->CreateStructProperty(destStruct, sourceProperty->GetName(), *destClass->GetStructClassP());
            }
        else
            {
            ECN::ECSchemaPtr flatBaseSchema = m_flattenedRefs[sourceType.GetSchema().GetName()];
            if (!ECN::ECSchema::IsSchemaReferenced(targetClass->GetSchema(), *flatBaseSchema))
                targetClass->GetSchemaR().AddReferencedSchema(*flatBaseSchema);
            targetClass->CreateStructProperty(destStruct, sourceProperty->GetName(), *flatBaseSchema->GetClassP(sourceType.GetName().c_str())->GetStructClassP());
            }
        destProperty = destStruct;
        }
    else if (sourceProperty->GetIsNavigation())
        {
        ECN::NavigationECPropertyP destNav;
        ECN::NavigationECPropertyCP sourceNav = sourceProperty->GetAsNavigationProperty();

        ECN::ECRelationshipClassCP sourceRelClass = sourceNav->GetRelationshipClass();
        if (targetClass->GetSchema().GetSchemaKey() == sourceProperty->GetClass().GetSchema().GetSchemaKey())
            {
            ECN::ECClassP destClass = targetClass->GetSchemaR().GetClassP(sourceRelClass->GetName().c_str());
            if (nullptr == destClass)
                {
                auto status = CopyFlatClass(destClass, &(targetClass->GetSchemaR()), sourceRelClass);
                if (BSISUCCESS != status)
                    return BSIERROR;
                }
            targetClass->GetEntityClassP()->CreateNavigationProperty(destNav, sourceProperty->GetName(), *destClass->GetRelationshipClassCP(), sourceNav->GetDirection(), false);
            }
        else
            {
            ECN::ECSchemaPtr flatBaseSchema = m_flattenedRefs[sourceRelClass->GetSchema().GetName()];
            targetClass->GetEntityClassP()->CreateNavigationProperty(destNav, sourceProperty->GetName(), *flatBaseSchema->GetClassP(sourceRelClass->GetName().c_str())->GetRelationshipClassP(), sourceNav->GetDirection(), false);
            }
        destProperty = destNav;
        }

    destProperty->SetDescription(sourceProperty->GetInvariantDescription());
    if (sourceProperty->GetIsDisplayLabelDefined())
        destProperty->SetDisplayLabel(sourceProperty->GetInvariantDisplayLabel());
    destProperty->SetIsReadOnly(sourceProperty->IsReadOnlyFlagSet());
    destProperty->SetPriority(sourceProperty->GetPriority());

    if (sourceProperty->IsCategoryDefinedLocally())
        {
        ECN::PropertyCategoryCP sourcePropCategory = sourceProperty->GetCategory();
        if (sourcePropCategory->GetSchema().GetSchemaKey() == sourceProperty->GetClass().GetSchema().GetSchemaKey())
            {
            ECN::PropertyCategoryP destPropCategory = targetClass->GetSchemaR().GetPropertyCategoryP(sourcePropCategory->GetName().c_str());
            if (nullptr == destPropCategory)
                {
                auto status = targetClass->GetSchemaR().CopyPropertyCategory(destPropCategory, *sourcePropCategory);
                if (ECN::ECObjectsStatus::Success != status && ECN::ECObjectsStatus::NamedItemAlreadyExists != status)
                    return BSIERROR;
                }
            destProperty->SetCategory(destPropCategory);
            }
        else
            {
            ECN::ECSchemaPtr flatBaseSchema = m_flattenedRefs[sourcePropCategory->GetSchema().GetName()];
            destProperty->SetCategory(flatBaseSchema->GetPropertyCategoryP(sourcePropCategory->GetName().c_str()));
            }
        }

    if (sourceProperty->IsKindOfQuantityDefinedLocally())
        {
        ECN::KindOfQuantityCP sourceKoq = sourceProperty->GetKindOfQuantity();
        if (sourceKoq->GetSchema().GetSchemaKey() == sourceProperty->GetClass().GetSchema().GetSchemaKey())
            {
            ECN::KindOfQuantityP destKoq = targetClass->GetSchemaR().GetKindOfQuantityP(sourceKoq->GetName().c_str());
            if (nullptr == destKoq)
                {
                auto status = targetClass->GetSchemaR().CopyKindOfQuantity(destKoq, *sourceKoq);
                if (ECN::ECObjectsStatus::Success != status && ECN::ECObjectsStatus::NamedItemAlreadyExists != status)
                    return BSIERROR;
                }
            destProperty->SetKindOfQuantity(destKoq);
            }
        else
            {
            ECN::ECSchemaPtr flatBaseSchema = m_flattenedRefs[sourceKoq->GetSchema().GetName()];
            destProperty->SetKindOfQuantity(flatBaseSchema->GetKindOfQuantityP(sourceKoq->GetName().c_str()));
            }
        }

    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2017
//---------------+---------------+---------------+---------------+---------------+-------
void verifyDerivedClassesNotAbstract(ECN::ECClassP ecClass)
    {
    for (ECN::ECClassP derivedClass : ecClass->GetDerivedClasses())
        {
        if (ECN::ECClassModifier::Abstract == derivedClass->GetClassModifier())
            derivedClass->SetClassModifier(ECN::ECClassModifier::None);
        verifyDerivedClassesNotAbstract(derivedClass);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2017
//---------------+---------------+---------------+---------------+---------------+-------
void verifyBaseClassAbstract(ECN::ECClassP ecClass)
    {
    // There are cases out there where a base class is non-abstract and has instances, yet a derived class (generally in another schema) is set to abstract.  Therefore, instead of setting
    // the base classes Abstract, the derived class must be set as non-abstract
    for (ECN::ECClassP baseClass : ecClass->GetBaseClasses())
        {
        if (excludeSchema(baseClass->GetSchema()))
            continue;
        if (ECN::ECClassModifier::Abstract != baseClass->GetClassModifier() && ECN::ECClassModifier::Abstract == ecClass->GetClassModifier())
            {
            ecClass->SetClassModifier(ECN::ECClassModifier::None);
            verifyDerivedClassesNotAbstract(ecClass);
            }
        verifyBaseClassAbstract(baseClass);
        }
    }

// Create the pseudo polymorphic hierarchy by keeping track of any derived class that was lost.
//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2017
//---------------+---------------+---------------+---------------+---------------+-------
void addDroppedDerivedClass(ECN::ECClassP baseClass, ECN::ECClassP derivedClass)
    {
    ECN::IECInstancePtr droppedInstance = baseClass->GetCustomAttributeLocal("ECv3ConversionAttributes", "OldDerivedClasses");
    if (!droppedInstance.IsValid())
        droppedInstance = ECN::ConversionCustomAttributeHelper::CreateCustomAttributeInstance("OldDerivedClasses");
    if (!droppedInstance.IsValid())
        {
        LOG.warningv("Failed to create 'OldDerivedClasses' custom attribute for ECClass '%s'", baseClass->GetFullName());
        return;
        }
    ECN::ECValue v;
    droppedInstance->GetValue(v, "Classes");
    Utf8String classes("");
    if (!v.IsNull())
        classes = Utf8String(v.GetUtf8CP()).append(";");

    classes.append(derivedClass->GetFullName());

    v.SetUtf8CP(classes.c_str());
    if (ECN::ECObjectsStatus::Success != droppedInstance->SetValue("Classes", v))
        {
        LOG.warningv("Failed to create 'OldDerivedClasses' custom attribute for the ECClass '%s' with 'Classes' set to '%s'.", baseClass->GetFullName(), classes.c_str());
        return;
        }

    if (!ECN::ECSchema::IsSchemaReferenced(baseClass->GetSchemaR(), droppedInstance->GetClass().GetSchema()))
        {
        ECN::ECClassP nonConstClass = const_cast<ECN::ECClassP>(&droppedInstance->GetClass());
        if (ECN::ECObjectsStatus::Success != baseClass->GetSchemaR().AddReferencedSchema(nonConstClass->GetSchemaR()))
            {
            LOG.warningv("Failed to add %s as a referenced schema to %s.", droppedInstance->GetClass().GetSchema().GetName().c_str(), baseClass->GetSchemaR().GetName().c_str());
            LOG.warningv("Failed to add 'OldDerivedClasses' custom attribute to ECClass '%s'.", baseClass->GetFullName());
            return;
            }
        }

    if (ECN::ECObjectsStatus::Success != baseClass->SetCustomAttribute(*droppedInstance))
        {
        LOG.warningv("Failed to add 'OldDerivedClasses' custom attribute, with 'PropertyMapping' set to '%s', to ECClass '%s'.", classes.c_str(), baseClass->GetFullName());
        return;
        }

    LOG.debugv("Successfully added OldDerivedClasses custom attribute to ECClass '%s'", baseClass->GetFullName());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            03/2018
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus SchemaFlattener::FindBisBaseClass(ECN::ECClassP targetClass, ECN::ECClassCP sourceClass)
    {
    if (excludeSchema(sourceClass->GetSchema()))
        {
        ECN::ECSchemaPtr flatBaseSchema = m_flattenedRefs[sourceClass->GetSchema().GetName()];
        if (!flatBaseSchema.IsValid())
            return BSIERROR;
        targetClass->AddBaseClass(*flatBaseSchema->GetClassCP(sourceClass->GetName().c_str()), false, false, false);
        return BSISUCCESS;
        }
    const ECN::ECBaseClassesList& baseClasses = sourceClass->GetBaseClasses();
    for (ECN::ECClassP sourceBaseClass : baseClasses)
        {
        if (BSISUCCESS == FindBisBaseClass(targetClass, sourceBaseClass))
            return BSISUCCESS;
        }
    return BSIERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            06/2018
//---------------+---------------+---------------+---------------+---------------+-------
void SchemaFlattener::CheckConstraintForDerivedClasses(ECN::ECRelationshipConstraintR constraint)
    {
    bvector<ECClassCP> constraintsToAdd;
    bvector<ECClassCP> constraintsToRemove;
    if (!constraint.GetIsPolymorphic())
        return;

    // In the case of flattened schemas, we have broken polymorphism so need to fix that
    for (auto constraintClass : constraint.GetConstraintClasses())
        {
        ECN::ECClassP nonConstConstraint = const_cast<ECN::ECClassP>(constraintClass);
        // Need to find a common base class between this constraint and any derived classes that were removed
        ECN::IECInstancePtr droppedInstance = constraintClass->GetCustomAttributeLocal("ECv3ConversionAttributes", "OldDerivedClasses");
        if (droppedInstance.IsValid())
            {
            ECN::ECValue v;
            droppedInstance->GetValue(v, "Classes");
            if (!v.IsNull())
                {
                bvector<Utf8String> classNames;
                BeStringUtilities::Split(v.GetUtf8CP(), ";", classNames);
                bvector<ECN::ECClassCP> searchClasses;
                for (Utf8String className : classNames)
                    {
                    bvector<Utf8String> components;
                    BeStringUtilities::Split(className.c_str(), ":", components);
                    if (components.size() != 2)
                        continue;

                    ECN::ECSchemaPtr droppedSchema = m_flattenedRefs[components[0]];
                    if (droppedSchema.IsValid())
                        {
                        ECN::ECClassCP dropped = droppedSchema->GetClassCP(components[1].c_str());
                        if (nullptr != dropped)
                            {
                            ECN::ECClassP nonConst = const_cast<ECN::ECClassP>(dropped);
                            searchClasses.push_back(nonConst);
                            }
                        }
                    }
                ECN::ECEntityClassCP commonClass = nullptr;
                ECN::ECClass::FindCommonBaseClass(commonClass, constraintClass->GetEntityClassCP(), searchClasses);
                if (commonClass != nullptr)
                    {
                    constraintsToRemove.push_back(nonConstConstraint);
                    constraintsToAdd.push_back(commonClass);
                    }
                }
            }
        }
    for (ECClassCP toRemove : constraintsToRemove)
        constraint.RemoveClass(*toRemove->GetEntityClassCP());
    for (ECClassCP toAdd : constraintsToAdd)
        constraint.AddClass(*toAdd->GetEntityClassCP());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2017
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus SchemaFlattener::FlattenSchemas(ECN::ECSchemaP ecSchema)
    {
    bvector<ECN::ECSchemaP> schemas;
    ecSchema->FindAllSchemasInGraph(schemas, true);


    for (ECN::ECSchemaP sourceSchema : schemas)
        {
        if (excludeSchema(*sourceSchema))
            {
            m_flattenedRefs[sourceSchema->GetName()] = sourceSchema;
            continue;
            }

        if (m_flattenedRefs.find(sourceSchema->GetName()) != m_flattenedRefs.end())
            continue;

        ECN::ECSchemaPtr flatSchema;
        ECN::ECSchema::CreateSchema(flatSchema, sourceSchema->GetName(), sourceSchema->GetAlias(), sourceSchema->GetVersionRead(), sourceSchema->GetVersionWrite(), sourceSchema->GetVersionMinor(), sourceSchema->GetECVersion());
        m_flattenedRefs[flatSchema->GetName()] = flatSchema.get();
        flatSchema->SetOriginalECXmlVersion(2, 0);

        ECN::ECSchemaReferenceListCR referencedSchemas = sourceSchema->GetReferencedSchemas();
        for (ECN::ECSchemaReferenceList::const_iterator it = referencedSchemas.begin(); it != referencedSchemas.end(); ++it)
            {
            ECN::ECSchemaPtr flatRefSchema = m_flattenedRefs[it->second->GetName()];
            flatSchema->AddReferencedSchema(*flatRefSchema);
            }

        bvector<ECN::ECClassCP> relationshipClasses;
        for (ECN::ECClassCP sourceClass : sourceSchema->GetClasses())
            {
            ECN::ECClassP targetClass = nullptr;
            if (sourceClass->IsRelationshipClass())
                {
                relationshipClasses.push_back(sourceClass);
                continue;
                }
            CreateFlatClass(targetClass, flatSchema.get(), sourceClass);
            }

        // Need to make sure that all constraint classes are already created
        for (ECN::ECClassCP sourceClass : relationshipClasses)
            {
            ECN::ECClassP targetClass = nullptr;
            CreateFlatClass(targetClass, flatSchema.get(), sourceClass);
            }

        ECN::IECInstancePtr flattenedInstance = ECN::ConversionCustomAttributeHelper::CreateCustomAttributeInstance("IsFlattened");
        if (flattenedInstance.IsValid())
            {
            if (!ECN::ECSchema::IsSchemaReferenced(flattenedInstance->GetClass().GetSchema(), *flatSchema))
                {
                ECN::ECClassCR constClass = flattenedInstance->GetClass();
                ECN::ECClassP nonConst = const_cast<ECN::ECClassP>(&constClass);
                flatSchema->AddReferencedSchema(nonConst->GetSchemaR());
                }
            flatSchema->SetCustomAttribute(*flattenedInstance);
            }

        for (ECN::ECClassCP sourceClass : sourceSchema->GetClasses())
            {
            ECN::ECClassP targetClass = flatSchema->GetClassP(sourceClass->GetName().c_str());

            const ECN::ECBaseClassesList& baseClasses = sourceClass->GetBaseClasses();
            int totalBaseClasses = 0;
            int baseClassesFromSchema = 0;
            for (ECN::ECClassP sourceBaseClass : baseClasses)
                {
                totalBaseClasses++;
                if (sourceBaseClass->GetSchema().GetName().EqualsIAscii(sourceClass->GetSchema().GetName().c_str()))
                    baseClassesFromSchema++;
                }

            if (totalBaseClasses == 1)
                {
                for (ECN::ECClassP sourceBaseClass : baseClasses)
                    {
                    ECN::ECSchemaPtr flatBaseSchema = m_flattenedRefs[sourceBaseClass->GetSchema().GetName()];
                    if (!flatBaseSchema.IsValid())
                        return BSIERROR;
                    targetClass->AddBaseClass(*flatBaseSchema->GetClassCP(sourceBaseClass->GetName().c_str()), false, false, false);
                    }
                }
            else if (baseClassesFromSchema < 2)
                {
                for (ECN::ECClassP sourceBaseClass : baseClasses)
                    {
                    if (sourceBaseClass->GetSchema().GetName().EqualsIAscii(sourceClass->GetSchema().GetName().c_str()))
                        {
                        targetClass->AddBaseClass(*flatSchema->GetClassCP(sourceBaseClass->GetName().c_str()), false, false, false);
                        }
                    else
                        {
                        ECN::ECSchemaPtr flatBaseSchema = m_flattenedRefs[sourceBaseClass->GetSchema().GetName()];
                        if (!flatBaseSchema.IsValid())
                            continue;
                        ECN::ECClassP flatBase = flatBaseSchema->GetClassP(sourceBaseClass->GetName().c_str());
                        addDroppedDerivedClass(flatBase, targetClass);
                        }
                    }
                if (targetClass->GetBaseClasses().size() == 0)
                    FindBisBaseClass(targetClass, sourceClass);
                }
            else if (totalBaseClasses > 1)
                {
                for (ECN::ECClassP baseClass : baseClasses)
                    {
                    ECN::ECSchemaPtr flatBaseSchema = m_flattenedRefs[baseClass->GetSchema().GetName()];
                    if (!flatBaseSchema.IsValid())
                        continue;
                    ECN::ECClassP flatBase = flatBaseSchema->GetClassP(baseClass->GetName().c_str());
                    addDroppedDerivedClass(flatBase, targetClass);
                    }
                // Drop the immediate base class, but we still need to add the BIS base class
                FindBisBaseClass(targetClass, sourceClass);
                }
            if (targetClass->GetClassModifier() == ECN::ECClassModifier::Abstract)
                verifyBaseClassAbstract(targetClass);
            }

        // This needs to happen after all of the baseclasses have been set.
        for (ECN::ECClassCP sourceClass : sourceSchema->GetClasses())
            {
            ECN::ECClassP targetClass = flatSchema->GetClassP(sourceClass->GetName().c_str());
            for (ECN::ECPropertyCP sourceProperty : sourceClass->GetProperties(true))
                {
                if (BSISUCCESS != CopyFlattenedProperty(targetClass, sourceProperty))
                    return BSIERROR;
                }
            }

        for (auto ecClass : relationshipClasses)
            {
            ECClassP flattenedClass = flatSchema->GetClassP(ecClass->GetName().c_str());
            if (nullptr == flattenedClass)
                continue;
            ECRelationshipClassP relClass = flattenedClass->GetRelationshipClassP();
            CheckConstraintForDerivedClasses(relClass->GetSource());
            CheckConstraintForDerivedClasses(relClass->GetTarget());
            }

        // This needs to happen after we have copied all of the properties for all of the classes as the custom attributes could be defined locally
        CopyFlatCustomAttributes(*flatSchema, *sourceSchema);

        for (ECN::ECClassCP sourceClass : sourceSchema->GetClasses())
            {
            ECN::ECClassP targetClass = flatSchema->GetClassP(sourceClass->GetName().c_str());
            CopyFlatCustomAttributes(*targetClass, *sourceClass);
            for (ECN::ECPropertyCP sourceProperty : sourceClass->GetProperties(true))
                {
                ECN::ECPropertyP targetProperty = targetClass->GetPropertyP(sourceProperty->GetName().c_str());
                if (nullptr == targetProperty) // If we didn't copy over the property because it came from an excluded schema
                    continue;
                CopyFlatCustomAttributes(*targetProperty, *sourceProperty);
                }
            }
        }

    for (ECN::ECSchemaP sourceSchema : schemas)
        {
        Utf8String sourceSchemaName(sourceSchema->GetName().c_str());
        m_schemaReadContext->GetCache().DropSchema(sourceSchema->GetSchemaKey());
        m_schemaReadContext->AddSchema(*m_flattenedRefs[sourceSchemaName]);
        }

    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2017
//---------------+---------------+---------------+---------------+---------------+-------
void findBase(ECClassCP &inputClass)
    {
    ECClassCP test = inputClass;
    while (true)
        {
        if (!test->HasBaseClasses())
            break;
        ECClassCP t2 = *test->GetBaseClasses().begin();
        if (t2->GetName().Equals(inputClass->GetName()))
            test = t2;
        else
            break;
        }
    inputClass = test;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            05/2018
//---------------+---------------+---------------+---------------+---------------+-------
void SchemaFlattener::ProcessConstraints(ECRelationshipConstraintR constraint, bool isSource, ECRelationshipClassR relClass, ECEntityClassP defaultConstraintClass)
    {
    if (relClass.HasBaseClasses())
        {
        ECRelationshipClassCP baseClass = nullptr;
        for (ECClassCP ecClass : relClass.GetBaseClasses())
            {
            if (0 == strcmp(BIS_ECSCHEMA_NAME, ecClass->GetSchema().GetName().c_str()))
                continue;
            baseClass = ecClass->GetRelationshipClassCP();
            break;
            }
        if (nullptr != baseClass)
            {
            ECRelationshipConstraintR baseConstraint = isSource ? baseClass->GetSource() : baseClass->GetTarget();
            for (auto constraintClass : constraint.GetConstraintClasses())
                {

                bvector<ECClassCP> constraintsToRemove;
                for (auto baseConstraintClass : baseConstraint.GetConstraintClasses())
                    {
                    if (!constraintClass->Is(baseConstraintClass))
                        constraintsToRemove.push_back(baseConstraintClass);
                    }

                if (constraintsToRemove.size() > 0)
                    {
                    ECObjectsStatus status = baseConstraint.AddClass(*defaultConstraintClass);
                    if (ECObjectsStatus::Success != status)
                        return;

                    for (ECClassCP ecClass : constraintsToRemove)
                        {
                        if (ecClass->IsEntityClass())
                            baseConstraint.RemoveClass(*ecClass->GetEntityClassCP());
                        else if (ecClass->IsRelationshipClass())
                            baseConstraint.RemoveClass(*ecClass->GetRelationshipClassCP());
                        }
                    }
                }
            }
        }

    // It is possible that a constraint can have multiple classes, one of which got turned into an aspect.  This makes for an incompatible constraint.  Need to remove the conflicting type.
    bool haveFirstType = false;
    bool firstIsElement = true;
    bvector<ECClassCP> constraintsToRemove;
    for (auto constraintClass : constraint.GetConstraintClasses())
        {
        if (!haveFirstType)
            {
            haveFirstType = true;
            firstIsElement = constraintClass->Is(BIS_ECSCHEMA_NAME, BIS_CLASS_Element);
            continue;
            }
        if (firstIsElement != constraintClass->Is(BIS_ECSCHEMA_NAME, BIS_CLASS_Element))
            constraintsToRemove.push_back(constraintClass);
        }

    for (ECClassCP ecClass : constraintsToRemove)
        constraint.RemoveClass(*ecClass->GetEntityClassCP());

    CheckConstraintForDerivedClasses(constraint);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2017
//---------------+---------------+---------------+---------------+---------------+-------
void SchemaFlattener::ProcessSP3DSchema(ECN::ECSchemaP schema, ECN::ECClassCP baseInterface, ECN::ECClassCP baseObject, bset<ECClassP>& rootClasses, ECEntityClassP defaultConstraintClass)
    {
    bool wasFlattened = false;
    m_baseInterface = baseInterface;
    m_baseObject = baseObject;
    findBase(m_baseObject);
    findBase(m_baseInterface);

    bvector<ECRelationshipClassP> relationshipClasses;
    for (ECN::ECClassP ecClass : schema->GetClasses())
        {
        ECN::ECEntityClassP entityClass = ecClass->GetEntityClassP();
        ECN::ECRelationshipClassP relClass = ecClass->GetRelationshipClassP();

        if (nullptr != relClass)
            relationshipClasses.push_back(relClass);

        // Classes derived from BaseObject can have multiple BaseInterface-derived base classes, but only one BaseObject base class
        // Classes derived from BaseInterface can only have one base class
        if (nullptr == entityClass)
            continue;
        if (!ecClass->Is(m_baseInterface) && !ecClass->Is(m_baseObject))
            continue;
        else if (ecClass->HasBaseClasses())
            {
            bool isInterface = ecClass->Is(m_baseInterface) && !ecClass->Is(m_baseObject);
            int baseClassCounter = 0;
            bvector<ECN::ECClassP> toRemove;
            for (auto& baseClass : ecClass->GetBaseClasses())
                {
                if (baseClass->GetSchema().GetName().Equals(BIS_ECSCHEMA_NAME) || baseClass->GetSchema().GetName().Equals("Generic"))
                    {
                    if (ecClass == m_baseInterface)
                        ecClass->RemoveBaseClasses();
                    rootClasses.insert(ecClass);
                    continue;
                    }
                ECN::ECEntityClassCP asEntity = baseClass->GetEntityClassCP();
                if (nullptr == asEntity)
                    continue;
                else if (!isInterface && !asEntity->Is(m_baseObject))
                    continue;
                baseClassCounter++;
                if (baseClassCounter > 1)
                    toRemove.push_back(baseClass);
                }
            for (auto& baseClass : toRemove)
                {
                ecClass->RemoveBaseClass(*baseClass);
                addDroppedDerivedClass(baseClass, ecClass);
                for (ECN::ECPropertyCP sourceProperty : baseClass->GetProperties(true))
                    {
                    if (excludeSchema(sourceProperty->GetClass().GetSchema()))
                        continue;

                    if (nullptr != ecClass->GetPropertyP(sourceProperty->GetName().c_str(), true))
                        continue;
                    if (BSISUCCESS != CopyFlattenedProperty(ecClass, sourceProperty))
                        return;
                    }
                wasFlattened = true;
                }
            }

        if (ecClass->GetClassModifier() == ECN::ECClassModifier::Abstract)
            verifyBaseClassAbstract(ecClass);
        }

    for (ECRelationshipClassP relationshipClass : relationshipClasses)
        {
        ProcessConstraints(relationshipClass->GetSource(), true, *relationshipClass, defaultConstraintClass);
        ProcessConstraints(relationshipClass->GetTarget(), false, *relationshipClass, defaultConstraintClass);
        }
    if (wasFlattened)
        {
        ECN::IECInstancePtr flattenedInstance = ECN::ConversionCustomAttributeHelper::CreateCustomAttributeInstance("IsFlattened");
        if (flattenedInstance.IsValid())
            {
            if (!ECN::ECSchema::IsSchemaReferenced(flattenedInstance->GetClass().GetSchema(), *schema))
                {
                ECN::ECClassCR constClass = flattenedInstance->GetClass();
                ECN::ECClassP nonConst = const_cast<ECN::ECClassP>(&constClass);
                schema->AddReferencedSchema(nonConst->GetSchemaR());
                }
            schema->SetCustomAttribute(*flattenedInstance);
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            03/2018
//---------------+---------------+---------------+---------------+---------------+-------
void SchemaFlattener::CheckForMixinConversion(ECN::ECClassR inputClass)
    {
    ECSchemaR targetSchema = inputClass.GetSchemaR();
    if (nullptr == m_baseInterface)
        {
        m_baseInterface = targetSchema.GetClassP("BaseInterface");
        findBase(m_baseInterface);
        }
    if (nullptr == m_baseObject)
        {
        m_baseObject = targetSchema.GetClassP("BaseObject");
        findBase(m_baseObject);
        }

    if (ShouldConvertECClassToMixin(targetSchema, inputClass))
        {
        ECClassP appliesTo;
        if (BSISUCCESS == FindAppliesToClass(appliesTo, targetSchema, inputClass))
            ConvertECClassToMixin(targetSchema, inputClass, *appliesTo);
        }

    for (ECClassP childClass : inputClass.GetDerivedClasses())
        CheckForMixinConversion(*childClass);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            03/2018
//---------------+---------------+---------------+---------------+---------------+-------
bool SchemaFlattener::ShouldConvertECClassToMixin(ECN::ECSchemaR targetSchema, ECN::ECClassR inputClass)
    {
    if (inputClass.GetClassModifier() != ECClassModifier::Abstract)
        return false;

    // check base class
    for (auto baseClass : inputClass.GetBaseClasses())
        {
        if (baseClass->GetSchemaR().GetName().EqualsI(BIS_ECSCHEMA_NAME))
            continue;
        if (baseClass->IsEntityClass())
            {
            auto baseEntityClass = baseClass->GetEntityClassP();
            if (!baseEntityClass->IsMixin() && !ShouldConvertECClassToMixin(baseEntityClass->GetSchemaR(), *baseEntityClass))
                return false;
            }
        else
            return false;
        }

    if (!inputClass.Is(m_baseInterface) || inputClass.Is(m_baseObject))
        return false;

    return true;

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            03/2018
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus SchemaFlattener::ConvertECClassToMixin(ECN::ECSchemaR targetSchema, ECN::ECClassR inputClass, ECN::ECClassCR appliesTo)
    {
    if (ECN::ECClassModifier::Abstract != inputClass.GetClassModifier())
        return BSIERROR;

    ECN::IECInstancePtr mixinInstance = ECN::CoreCustomAttributeHelper::CreateCustomAttributeInstance("IsMixin");
    if (!mixinInstance.IsValid())
        return BSIERROR;

    auto& coreCA = mixinInstance->GetClass().GetSchema();
    if (!ECN::ECSchema::IsSchemaReferenced(targetSchema, coreCA))
        targetSchema.AddReferencedSchema(const_cast<ECSchemaR>(coreCA));
    ECValue appliesToClass(ECN::ECClass::GetQualifiedClassName(targetSchema, appliesTo).c_str());

    ECN::ECObjectsStatus status;

    status = mixinInstance->SetValue("AppliesToEntityClass", appliesToClass);
    if (ECN::ECObjectsStatus::Success != status)
        return BSIERROR;

    status = inputClass.SetCustomAttribute(*mixinInstance);
    if (ECN::ECObjectsStatus::Success != status)
        return BSIERROR;

    for (auto baseClass : inputClass.GetBaseClasses())
        {
        if (baseClass->GetSchemaR().GetName().EqualsI(BIS_ECSCHEMA_NAME))
            inputClass.RemoveBaseClass(*baseClass);
        }

    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            03/2018
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus SchemaFlattener::FindAppliesToClass(ECN::ECClassP& appliesTo, ECN::ECSchemaR targetSchema, ECN::ECClassR mixinClass)
    { 
    auto applies = m_mixinAppliesToMap.find(mixinClass.GetEntityClassCP());
    if (applies != m_mixinAppliesToMap.end())
        {
        appliesTo = applies->second;
        return BSISUCCESS;
        }

    bvector<ECClassCP> propogationFilter;
    propogationFilter.push_back(m_baseObject);

    ECDerivedClassesList derivedClasses = mixinClass.GetDerivedClasses();
    ECDerivedClassesList searchClasses;
    for (auto derived : derivedClasses)
        {
        ECClassP derivedAppliesTo;
        // if concrete class
        if (derived->Is(m_baseObject))
            searchClasses.push_back(derived);
        else if (BSISUCCESS == FindAppliesToClass(derivedAppliesTo, derived->GetSchemaR(), *derived))
            searchClasses.push_back(derivedAppliesTo);
        else
            return BSIERROR;
        }

    if (searchClasses.empty())
        appliesTo = targetSchema.GetClassP(m_baseObject->GetName().c_str());
    else
        FindCommonBaseClass(appliesTo, searchClasses.front(), searchClasses, propogationFilter);

    if (appliesTo == nullptr)
        return BSIERROR;

    return AddMixinAppliesToMapping(mixinClass.GetEntityClassCP(), appliesTo);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            03/2018
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus SchemaFlattener::AddMixinAppliesToMapping(ECClassCP mixinClass, ECClassP appliesToClass)
    {
    BeAssert(m_mixinAppliesToMap.find(mixinClass) == m_mixinAppliesToMap.end());
    m_mixinAppliesToMap.Insert(mixinClass, appliesToClass);
    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            03/2018
//---------------+---------------+---------------+---------------+---------------+-------
void SchemaFlattener::FindCommonBaseClass(ECN::ECClassP& commonClass, ECN::ECClassP currentClass, ECN::ECBaseClassesList const& classes, const bvector<ECN::ECClassCP> propogationFilter)
    {
    ECClassP tempCommonClass = currentClass;
    for (const auto &secondConstraint : classes)
        {
        ECClassCP secondClass = secondConstraint;
        ECEntityClassCP asEntity = secondClass->GetEntityClassCP();
        if (nullptr != asEntity && asEntity->IsMixin() && asEntity->GetAppliesToClass()->Is(tempCommonClass->GetEntityClassCP()))
            continue;
        if (secondClass->Is(tempCommonClass))
            continue;

        for (const auto baseClass : tempCommonClass->GetBaseClasses())
            {
            bool shouldPropogate = false;
            for (const auto filterClass : propogationFilter)
                {
                if (baseClass->Is(filterClass))
                    {
                    shouldPropogate = true;
                    break;
                    }
                }
            if (!shouldPropogate)
                continue;

            FindCommonBaseClass(commonClass, baseClass->GetEntityClassP(), classes, propogationFilter);
            if (commonClass != nullptr)
                return;
            }

        tempCommonClass = nullptr;
        break;
        }

    if (nullptr != tempCommonClass)
        commonClass = tempCommonClass;
    }

END_BIM_FROM_DGNDB_NAMESPACE
