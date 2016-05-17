/*--------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSchemaComparer.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+-------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
USING_NAMESPACE_BENTLEY_EC
using namespace std;
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
#define NULL_TEXT "<null>"


//======================================================================================>
//Binary
//======================================================================================>

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus Binary::Resize(size_t len)
    {
    if (len == 0)
        return Free();

    if (len == m_len)
        return SUCCESS;
    if (m_buff = realloc(m_buff, len))
        {
        m_len = len;
        return SUCCESS;
        }

    BeAssert(false && "_resize() failed");
    return ERROR;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus Binary::Free()
    {
    if (m_buff)
        free(m_buff);

    m_buff = nullptr;
    m_len = 0;
    return SUCCESS;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus Binary::Assign(void* buff , size_t len)
    {
    if (buff != nullptr && len == 0)
        {
        BeAssert(false && "_assign() buff != nullptr && len == 0");
        return ERROR;
        }
    else if (buff == nullptr && len != 0)
        {
        BeAssert(false && "_assign() buff == nullptr && len != 0");
        return ERROR;
        }
    else if (buff == nullptr && len == 0)
        return Free();
    else if (Resize(len) != SUCCESS)
        {
        BeAssert(false && "_assign() _resize(len) != SUCCESS");
        return ERROR;
        }

    memcpy(m_buff, buff, len);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
Binary::Binary() : m_buff(nullptr), m_len(0) {}

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
Binary::~Binary() { Free(); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
Binary::Binary(Binary const& rhs)
    {
    *this = rhs;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
Binary::Binary(Binary&& rhs)
    {
    *this = std::move(rhs);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
Binary& Binary::operator = (Binary const& rhs)
    {
    if (this != &rhs)
        Assign(rhs.m_buff, rhs.m_len);

    return *this;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
Binary& Binary::operator = (Binary&& rhs)
    {
    if (this != &rhs)
        {
        m_buff = std::move(rhs.m_buff);
        m_len = std::move(rhs.m_len);
        rhs.m_buff = nullptr;
        rhs.m_len = 0;
        }

    return *this;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
int Binary::Compare(Binary const& rhs) const
    {
    if (Size() == 0 && rhs.Size() == 0)
        return 0;

    if (Size() == 0 && rhs.Size() != 0)
        return 1;

    if (Size() != 0 && rhs.Size() == 0)
        return -1;

    if (Size() > rhs.Size())
        return -1;

    if (Size() < rhs.Size())
        return 1;

    return memcmp(m_buff, rhs.m_buff, m_len);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
void Binary::CopyTo(ECValueR value) const
    {
    value.SetBinary(static_cast<Byte*>(m_buff), m_len, true);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus Binary::CopyFrom(ECValueCR value)
    {
    if (value.IsNull())
        return Free();

    if (!value.IsBinary())
        return ERROR;

    size_t len = 0;
    void* buff = (void*) value.GetBinary(len);
    return Assign(buff, len);
    }

//======================================================================================>
//ECSchemaComparer
//======================================================================================>
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSchemaComparer::Compare(ECSchemaChanges& changes, bvector<ECN::ECSchemaCP> const& lhs, bvector<ECN::ECSchemaCP> const& rhs, Options options)
    {
    m_options = options;
    std::map<Utf8CP, ECSchemaCP, CompareIUtf8> lhsMap, rhsMap, allSchemasMap;
    for (ECSchemaCP schema : lhs)
        lhsMap[schema->GetName().c_str()] = schema;

    for (ECSchemaCP schema : rhs)
        rhsMap[schema->GetName().c_str()] = schema;

    allSchemasMap.insert(lhsMap.cbegin(), lhsMap.cend());
    allSchemasMap.insert(rhsMap.cbegin(), rhsMap.cend());

    for (std::pair<Utf8CP, ECSchemaCP> const& kvPair : allSchemasMap)
        {
        Utf8CP schemaName = kvPair.first;
        auto lhsIt = lhsMap.find(schemaName);
        auto rhsIt = rhsMap.find(schemaName);

        const bool existInLhs = lhsIt != lhsMap.end();
        const bool existInRhs = rhsIt != rhsMap.end();
        if (existInLhs && existInRhs)
            {
            ECSchemaChange& schemaChange = changes.Add(ChangeState::Modified, schemaName);
            if (SUCCESS != CompareECSchema(schemaChange, *lhsIt->second, *rhsIt->second))
                return ERROR;
            }
        else if (existInLhs && !existInRhs)
            {
            if (AppendECSchema(changes, *lhsIt->second, ValueId::Deleted) == ERROR)
                return ERROR;
            }
        else if (!existInLhs && existInRhs)
            {
            if (AppendECSchema(changes, *rhsIt->second, ValueId::New) == ERROR)
                return ERROR;
            }
        }

    changes.Optimize();
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSchemaComparer::CompareECSchema(ECSchemaChange& change, ECSchemaCR a, ECSchemaCR b)
    {
    if (a.GetName() != b.GetName())
        change.GetName().SetValue(a.GetName(), b.GetName());

    if (a.GetDisplayLabel() != b.GetDisplayLabel())
        change.GetDisplayLabel().SetValue(a.GetDisplayLabel(), b.GetDisplayLabel());

    if (a.GetDescription() != b.GetDescription())
        change.GetDescription().SetValue(a.GetDescription(), b.GetDescription());

    if (a.GetNamespacePrefix() != b.GetNamespacePrefix())
        change.GetNamespacePrefix().SetValue(a.GetNamespacePrefix(), b.GetNamespacePrefix());

    if (a.GetVersionMajor() != b.GetVersionMajor())
        change.GetVersionMajor().SetValue(a.GetVersionMajor(), b.GetVersionMajor());

    if (a.GetVersionMinor() != b.GetVersionMinor())
        change.GetVersionMinor().SetValue(a.GetVersionMinor(), b.GetVersionMinor());

    if (CompareECClasses(change.Classes(), a.GetClasses(), b.GetClasses()) != SUCCESS)
        return ERROR;

    if (CompareECEnumerations(change.Enumerations(), a.GetEnumerations(), b.GetEnumerations()) != SUCCESS)
        return ERROR;

    //if (CompareKindOfQuantities(change.KindOfQuantities(), a.GetKindOfQuantities(), b.GetKindOfQuantities()) != SUCCESS)
    //    return ERROR;
    
    if (CompareReferences(change.References(), a.GetReferencedSchemas(), b.GetReferencedSchemas()) != SUCCESS)
        return ERROR;

    return CompareCustomAttributes(change.CustomAttributes(), a, b);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSchemaComparer::CompareECClass(ECClassChange& change, ECClassCR a, ECClassCR b)
    {
    if (a.GetName() != b.GetName())
        change.GetName().SetValue(a.GetName(), b.GetName());

    if (a.GetDisplayLabel() != b.GetDisplayLabel())
        change.GetDisplayLabel().SetValue(a.GetDisplayLabel(), b.GetDisplayLabel());

    if (a.GetDescription() != b.GetDescription())
        change.GetDescription().SetValue(a.GetDescription(), b.GetDescription());

    if (a.GetClassModifier() != b.GetClassModifier())
        change.GetClassModifier().SetValue(a.GetClassModifier(), b.GetClassModifier());

    if (a.GetClassType() != b.GetClassType())
        change.ClassType().SetValue(a.GetClassType(), b.GetClassType());

    if (a.IsRelationshipClass() != b.IsRelationshipClass())
        {
        if (a.IsRelationshipClass())
            {
            if (AppendECRelationshipClass(change.GetRelationship(), *a.GetRelationshipClassCP(), ValueId::Deleted) != SUCCESS)
                return ERROR;
            }
        else if (b.IsRelationshipClass())
            {
            if (AppendECRelationshipClass(change.GetRelationship(), *b.GetRelationshipClassCP(), ValueId::New) != SUCCESS)
                return ERROR;
            }
        }
    else
        {
        if (a.IsRelationshipClass() && b.IsRelationshipClass())
            if (CompareECRelationshipClass(change.GetRelationship(), *a.GetRelationshipClassCP(), *b.GetRelationshipClassCP()) != SUCCESS)
                return ERROR;
        }

    if (CompareECBaseClasses(change.BaseClasses(), a.GetBaseClasses(), b.GetBaseClasses()) != SUCCESS)
        return ERROR;

    if (CompareECProperties(change.Properties(), a.GetProperties(false), b.GetProperties(false)) != SUCCESS)
        return ERROR;

    return CompareCustomAttributes(change.CustomAttributes(), a, b);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSchemaComparer::CompareECBaseClasses(BaseClassChanges& changes, ECBaseClassesList const& a, ECBaseClassesList const& b)
    {
    auto m = std::min(a.size(), b.size());
    for (size_t i = 0; i < m; i++)
        {
        if (strcmp(a[i]->GetFullName(), b[i]->GetFullName()) != 0)
            {
            changes.Add(ChangeState::Modified).SetValue(Utf8String(a[i]->GetFullName()), Utf8String(b[i]->GetFullName()));
            }
        }
    for (size_t i = m; i < a.size(); i++)
        changes.Add(ChangeState::Deleted).SetValue(ValueId::Deleted, Utf8String(a[i]->GetFullName()));

    for (size_t i = m; i < b.size(); i++)
        changes.Add(ChangeState::New).SetValue(ValueId::New, Utf8String(b[i]->GetFullName()));

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSchemaComparer::CompareECRelationshipClass(ECRelationshipChange& change, ECRelationshipClassCR a, ECRelationshipClassCR b)
    {
    if (a.GetStrength() != b.GetStrength())
        change.GetStrength().SetValue(a.GetStrength(), b.GetStrength());

    if (a.GetStrengthDirection() != b.GetStrengthDirection())
        change.GetStrengthDirection().SetValue(a.GetStrengthDirection(), b.GetStrengthDirection());

    if (CompareECRelationshipConstraint(change.GetSource(), a.GetSource(), b.GetSource()) != SUCCESS)
        return ERROR;

    return CompareECRelationshipConstraint(change.GetTarget(), a.GetTarget(), b.GetTarget());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSchemaComparer::CompareECRelationshipConstraint(ECRelationshipConstraintChange& change, ECRelationshipConstraintCR a, ECRelationshipConstraintCR b)
    {
    if (a.GetRoleLabel() != b.GetRoleLabel())
        change.GetRoleLabel().SetValue(a.GetRoleLabel(), b.GetRoleLabel());

    if (a.GetIsPolymorphic() != b.GetIsPolymorphic())
        change.IsPolymorphic().SetValue(a.GetIsPolymorphic(), b.GetIsPolymorphic());

    if (a.GetCardinality().ToString() != b.GetCardinality().ToString())
        change.GetCardinality().SetValue(a.GetCardinality().ToString(), b.GetCardinality().ToString());

    return CompareECRelationshipConstraintClasses(change.ConstraintClasses(), a.GetConstraintClasses(), b.GetConstraintClasses());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSchemaComparer::CompareECRelationshipConstraintClassKeys(ECRelationshipConstraintClassChange& change, ECRelationshipConstraintClassCR a, ECRelationshipConstraintClassCR b)
    {
    std::set<Utf8CP, CompareIUtf8> aMap, bMap, cMap;
    for (Utf8StringCR keyProperty : a.GetKeys())
        aMap.insert(keyProperty.c_str());

    for (Utf8StringCR keyProperty : b.GetKeys())
        bMap.insert(keyProperty.c_str());

    cMap.insert(aMap.cbegin(), aMap.cend());
    cMap.insert(bMap.cbegin(), bMap.cend());

    for (Utf8CP u : cMap)
        {
        auto itorA = aMap.find(u);
        auto itorB = bMap.find(u);

        bool existInA = itorA != aMap.end();
        bool existInB = itorB != bMap.end();
        if (existInA && existInB)
            {
            //Same
            }
        else if (existInA && !existInB)
            {
            if (change.KeyProperties().Add(ChangeState::Deleted).SetValue(ValueId::Deleted, u) == ERROR)
                return ERROR;
            }
        else if (!existInA && existInB)
            {
            if (change.KeyProperties().Add(ChangeState::New).SetValue(ValueId::New, u) == ERROR)
                return ERROR;
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSchemaComparer::CompareECRelationshipConstraintClasses(ECRelationshipConstraintClassChanges& change, ECRelationshipConstraintClassList const& a, ECRelationshipConstraintClassList const& b)
    {
    std::map<Utf8CP, ECRelationshipConstraintClassCP, CompareIUtf8> aMap, bMap, cMap;
    for (ECRelationshipConstraintClassCP constraintClassCP : a)
        aMap[constraintClassCP->GetClass().GetFullName()] = constraintClassCP;

    for (ECRelationshipConstraintClassCP constraintClassCP : b)
        bMap[constraintClassCP->GetClass().GetFullName()] = constraintClassCP;

    cMap.insert(aMap.cbegin(), aMap.cend());
    cMap.insert(bMap.cbegin(), bMap.cend());

    for (auto& u : cMap)
        {
        auto itorA = aMap.find(u.first);
        auto itorB = bMap.find(u.first);

        bool existInA = itorA != aMap.end();
        bool existInB = itorB != bMap.end();
        if (existInA && existInB)
            {
            auto& constraintClass = change.Add(ChangeState::Modified);
            if (CompareECRelationshipConstraintClassKeys(constraintClass, *itorA->second, *itorB->second) == ERROR)
                return ERROR;
            }
        else if (existInA && !existInB)
            {
            if (AppendECRelationshipConstraintClass(change.Add(ChangeState::Deleted), *itorA->second, ValueId::Deleted) == ERROR)
                return ERROR;
            }
        else if (!existInA && existInB)
            {
            if (AppendECRelationshipConstraintClass(change.Add(ChangeState::New), *itorB->second, ValueId::New) == ERROR)
                return ERROR;
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSchemaComparer::CompareECProperty(ECPropertyChange& change, ECPropertyCR a, ECPropertyCR b)
    {
    if (a.GetTypeName() != b.GetTypeName())
        change.GetTypeName().SetValue(a.GetTypeName(), b.GetTypeName());

    if (a.GetName() != b.GetName())
        change.GetName().SetValue(a.GetName(), b.GetName());

    if (a.GetDisplayLabel() != b.GetDisplayLabel())
        change.GetDisplayLabel().SetValue(a.GetDisplayLabel(), b.GetDisplayLabel());

    if (a.GetDescription() != b.GetDescription())
        change.GetDescription().SetValue(a.GetDescription(), b.GetDescription());

    if (a.GetIsPrimitive() != b.GetIsPrimitive())
        change.IsPrimitive().SetValue(a.GetIsPrimitive(), b.GetIsPrimitive());

    if (a.GetIsStruct() != b.GetIsStruct())
        change.IsStruct().SetValue(a.GetIsStruct(), b.GetIsStruct());

    if (a.GetIsStructArray() != b.GetIsStructArray())
        change.IsStructArray().SetValue(a.GetIsStructArray(), b.GetIsStructArray());

    if (a.GetIsPrimitiveArray() != b.GetIsPrimitiveArray())
        change.IsPrimitiveArray().SetValue(a.GetIsPrimitiveArray(), b.GetIsPrimitiveArray());

    if (a.GetIsNavigation() != b.GetIsNavigation())
        change.IsNavigation().SetValue(a.GetIsNavigation(), b.GetIsNavigation());

    if (a.GetIsReadOnly() != b.GetIsReadOnly())
        change.IsReadonly().SetValue(a.GetIsReadOnly(), b.GetIsReadOnly());

    //if (a.GetMaximumValue() != b.GetMaximumValue())
    //    change.GetMaximumValue().SetValue(a.GetMaximumValue(), b.GetMaximumValue());

    //if (a.GetMinimumValue() != b.GetMinimumValue())
    //    change.GetMinimumValue().SetValue(a.GetMinimumValue(), b.GetMinimumValue());

    auto aNavigation = a.GetAsNavigationProperty();
    auto bNavigation = b.GetAsNavigationProperty();
    if (aNavigation && bNavigation)
        {
        if (aNavigation->GetDirection() != bNavigation->GetDirection())
            change.GetNavigation().Direction().SetValue(aNavigation->GetDirection(), bNavigation->GetDirection());

        if (aNavigation->GetRelationshipClass() && bNavigation->GetRelationshipClass())
            change.GetNavigation().GetRelationshipClassName().SetValue(
                Utf8String(aNavigation->GetRelationshipClass()->GetFullName()),
                Utf8String(bNavigation->GetRelationshipClass()->GetFullName()));
        else if (aNavigation->GetRelationshipClass() && !bNavigation->GetRelationshipClass())
            change.GetNavigation().GetRelationshipClassName().SetValue(ValueId::Deleted, aNavigation->GetRelationshipClass()->GetFullName());
        else if (!aNavigation->GetRelationshipClass() && bNavigation->GetRelationshipClass())
            change.GetNavigation().GetRelationshipClassName().SetValue(ValueId::New, bNavigation->GetRelationshipClass()->GetFullName());
        }

    auto aArray = a.GetAsArrayProperty();
    auto bArray = b.GetAsArrayProperty();
    if (aArray && bArray)
        {

        if (aArray->GetStoredMaxOccurs() != bArray->GetStoredMaxOccurs())
            change.GetArray().MaxOccurs().SetValue(aArray->GetStoredMaxOccurs(), bArray->GetStoredMaxOccurs());

        if (aArray->GetMinOccurs() != bArray->GetMinOccurs())
            change.GetArray().MinOccurs().SetValue(aArray->GetMinOccurs(), bArray->GetMinOccurs());
        }
    else if (aArray && !bArray)
        {
        change.GetArray().MaxOccurs().SetValue(ValueId::Deleted, aArray->GetStoredMaxOccurs());
        change.GetArray().MinOccurs().SetValue(ValueId::Deleted, aArray->GetMinOccurs());
        }
    else if (!aArray && bArray)
        {
        change.GetArray().MaxOccurs().SetValue(ValueId::New, bArray->GetStoredMaxOccurs());
        change.GetArray().MinOccurs().SetValue(ValueId::New, bArray->GetMinOccurs());
        }

    return CompareCustomAttributes(change.CustomAttributes(), a, b);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSchemaComparer::CompareECProperties(ECPropertyChanges& changes, ECPropertyIterableCR a, ECPropertyIterableCR b)
    {
    std::map<Utf8CP, ECPropertyCP, CompareIUtf8> aMap, bMap, cMap;
    for (ECPropertyCP propertyCP : a)
        aMap[propertyCP->GetName().c_str()] = propertyCP;

    for (ECPropertyCP propertyCP : b)
        bMap[propertyCP->GetName().c_str()] = propertyCP;

    cMap.insert(aMap.cbegin(), aMap.cend());
    cMap.insert(bMap.cbegin(), bMap.cend());

    for (auto& u : cMap)
        {
        auto itorA = aMap.find(u.first);
        auto itorB = bMap.find(u.first);

        bool existInA = itorA != aMap.end();
        bool existInB = itorB != bMap.end();
        if (existInA && existInB)
            {
            auto& propertyChange = changes.Add(ChangeState::Modified, u.first);
            if (CompareECProperty(propertyChange, *itorA->second, *itorB->second) == ERROR)
                return ERROR;
            }
        else if (existInA && !existInB)
            {
            if (AppendECProperty(changes, *itorA->second, ValueId::Deleted) == ERROR)
                return ERROR;
            }
        else if (!existInA && existInB)
            {
            if (AppendECProperty(changes, *itorB->second, ValueId::New) == ERROR)
                return ERROR;
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSchemaComparer::CompareECClasses(ECClassChanges& changes, ECClassContainerCR a, ECClassContainerCR b)
    {
    std::map<Utf8CP, ECClassCP, CompareIUtf8> aMap, bMap, cMap;
    for (ECClassCP classCP : a)
        aMap[classCP->GetName().c_str()] = classCP;

    for (ECClassCP classCP : b)
        bMap[classCP->GetName().c_str()] = classCP;

    cMap.insert(aMap.cbegin(), aMap.cend());
    cMap.insert(bMap.cbegin(), bMap.cend());

    for (auto& u : cMap)
        {
        auto itorA = aMap.find(u.first);
        auto itorB = bMap.find(u.first);

        bool existInA = itorA != aMap.end();
        bool existInB = itorB != bMap.end();
        if (existInA && existInB)
            {
            auto& classChange = changes.Add(ChangeState::Modified, u.first);
            if (CompareECClass(classChange, *itorA->second, *itorB->second) == ERROR)
                return ERROR;
            }
        else if (existInA && !existInB)
            {
            if (AppendECClass(changes, *itorA->second, ValueId::Deleted) == ERROR)
                return ERROR;
            }
        else if (!existInA && existInB)
            {
            if (AppendECClass(changes, *itorB->second, ValueId::New) == ERROR)
                return ERROR;
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSchemaComparer::CompareECEnumerations(ECEnumerationChanges& changes, ECEnumerationContainerCR a, ECEnumerationContainerCR b)
    {
    std::map<Utf8CP, ECEnumerationCP, CompareIUtf8> aMap, bMap, cMap;
    for (ECEnumerationCP enumCP : a)
        aMap[enumCP->GetName().c_str()] = enumCP;

    for (ECEnumerationCP enumCP : b)
        bMap[enumCP->GetName().c_str()] = enumCP;

    cMap.insert(aMap.cbegin(), aMap.cend());
    cMap.insert(bMap.cbegin(), bMap.cend());

    for (auto& u : cMap)
        {
        auto itorA = aMap.find(u.first);
        auto itorB = bMap.find(u.first);

        bool existInA = itorA != aMap.end();
        bool existInB = itorB != bMap.end();
        if (existInA && existInB)
            {
            auto& enumChange = changes.Add(ChangeState::Modified, u.first);
            if (CompareECEnumeration(enumChange, *itorA->second, *itorB->second) == ERROR)
                return ERROR;
            }
        else if (existInA && !existInB)
            {
            if (AppendECEnumeration(changes, *itorA->second, ValueId::Deleted) == ERROR)
                return ERROR;
            }
        else if (!existInA && existInB)
            {
            if (AppendECEnumeration(changes, *itorB->second, ValueId::New) == ERROR)
                return ERROR;
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
//BentleyStatus ECSchemaComparer::CompareKindOfQuantities(ECKindOfQuantityChanges& changes, KindOfQuantityContainerCR a, KindOfQuantityContainerCR b)
//    {
//    std::map<Utf8CP, KindOfQuantityCP, CompareIUtf8> aMap, bMap, cMap;
//    for (KindOfQuantityCP enumCP : a)
//        aMap[enumCP->GetName().c_str()] = enumCP;
//
//    for (KindOfQuantityCP enumCP : b)
//        bMap[enumCP->GetName().c_str()] = enumCP;
//
//    cMap.insert(aMap.cbegin(), aMap.cend());
//    cMap.insert(bMap.cbegin(), bMap.cend());
//
//    for (auto& u : cMap)
//        {
//        auto itorA = aMap.find(u.first);
//        auto itorB = bMap.find(u.first);
//
//        bool existInA = itorA != aMap.end();
//        bool existInB = itorB != bMap.end();
//        if (existInA && existInB)
//            {
//            auto& kindOfQuantityChange = changes.Add(ChangeState::Modified, u.first);
//            if (CompareKindOfQuantity(kindOfQuantityChange, *itorA->second, *itorB->second) == ERROR)
//                return ERROR;
//            }
//        else if (existInA && !existInB)
//            {
//            if (AppendKindOfQuantity(changes, *itorA->second, ValueId::Deleted) == ERROR)
//                return ERROR;
//            }
//        else if (!existInA && existInB)
//            {
//            if (AppendKindOfQuantity(changes, *itorB->second, ValueId::New) == ERROR)
//                return ERROR;
//            }
//        }
//
//    return SUCCESS;
//    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSchemaComparer::CompareCustomAttribute(ECPropertyValueChange& changes, IECInstanceCR a, IECInstanceCR b)
    {
    std::map<Utf8String, ECValue> aMap, bMap;  
    std::set<Utf8CP, CompareUtf8> cMap;
    if (ConvertECInstanceToValueMap(aMap, a) != SUCCESS)
        return ERROR;

    if (ConvertECInstanceToValueMap(bMap, b) != SUCCESS)
        return ERROR;

    for (auto& i : aMap)
        cMap.insert(i.first.c_str());

    for (auto& i : bMap)
        cMap.insert(i.first.c_str());

    for (auto& u : cMap)
        {
        auto itorA = aMap.find(u);
        auto itorB = bMap.find(u);

        bool existInA = itorA != aMap.end();
        bool existInB = itorB != bMap.end();
        if (existInA && existInB)
            {
            if (!itorA->second.Equals(itorB->second))
                changes.GetOrCreate(ChangeState::Modified, Split(u)).SetValue(itorA->second, itorB->second);
            }
        else if (existInA && !existInB)
            {
            changes.GetOrCreate(ChangeState::Deleted, Split(u)).SetValue(ValueId::Deleted, itorA->second);
            }
        else if (!existInA && existInB)
            {
            changes.GetOrCreate(ChangeState::New, Split(u)).SetValue(ValueId::New, itorB->second);
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSchemaComparer::AppendCustomAttribute(ECInstanceChanges& changes, IECInstanceCR v, ValueId appendType)
    {
    ChangeState state = appendType == ValueId::New ? ChangeState::New : ChangeState::Deleted;
    std::map<Utf8String, ECValue> map;
    if (ConvertECInstanceToValueMap(map, v) != SUCCESS)
        return ERROR;


    if (map.empty())
        return SUCCESS;

    auto& change = changes.Add(state, v.GetClass().GetFullName());
    for (auto& key : map)
        change.GetOrCreate(state, Split(key.first)).SetValue(appendType, key.second);

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSchemaComparer::AppendCustomAttributes(ECInstanceChanges& changes, IECCustomAttributeContainerCR v, ValueId appendType)
    {
    for (IECInstancePtr const& ptr : v.GetCustomAttributes(false))
        {
        if (AppendCustomAttribute(changes, *ptr, appendType) != SUCCESS)
            return ERROR;
        }
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSchemaComparer::CompareCustomAttributes(ECInstanceChanges& changes, IECCustomAttributeContainerCR a, IECCustomAttributeContainerCR b)
    {
    std::map<Utf8CP, IECInstanceCP, CompareIUtf8> aMap, bMap, cMap;
    for (IECInstancePtr const& instancePtr : a.GetCustomAttributes(false))
        aMap[instancePtr->GetClass().GetFullName()] = instancePtr.get();

    for (IECInstancePtr const& instancePtr : b.GetCustomAttributes(false))
        bMap[instancePtr->GetClass().GetFullName()] = instancePtr.get();

    cMap.insert(aMap.cbegin(), aMap.cend());
    cMap.insert(bMap.cbegin(), bMap.cend());

    for (auto& u : cMap)
        {
        auto itorA = aMap.find(u.first);
        auto itorB = bMap.find(u.first);

        bool existInA = itorA != aMap.end();
        bool existInB = itorB != bMap.end();
        if (existInA && existInB)
            {
            auto& caChange = changes.Add(ChangeState::Modified, u.first);
            if (CompareCustomAttribute(caChange, *itorA->second, *itorB->second) == ERROR)
                return ERROR;
            }
        else if (existInA && !existInB)
            {
            if (AppendCustomAttribute(changes, *itorA->second, ValueId::Deleted) == ERROR)
                return ERROR;
            }
        else if (!existInA && existInB)
            {
            if (AppendCustomAttribute(changes, *itorB->second, ValueId::New) == ERROR)
                return ERROR;
            }
        }
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSchemaComparer::CompareECEnumeration(ECEnumerationChange& change, ECEnumerationCR a, ECEnumerationCR b)
    {
    if (a.GetName() != b.GetName())
        change.GetName().SetValue(a.GetName(), b.GetName());

    if (a.GetDisplayLabel() != b.GetDisplayLabel())
        change.GetDisplayLabel().SetValue(a.GetDisplayLabel(), b.GetDisplayLabel());

    if (a.GetDescription() != b.GetDescription())
        change.GetDescription().SetValue(a.GetDescription(), b.GetDescription());

    if (a.GetIsStrict() != b.GetIsStrict())
        change.IsStrict().SetValue(a.GetIsStrict(), b.GetIsStrict());

    if (a.GetType() != b.GetType())
        {
        change.GetTypeName().SetValue(a.GetTypeName(), b.GetTypeName());
        }
    else if (a.GetType() == b.GetType())
        {
        if (a.GetType() == PrimitiveType::PRIMITIVETYPE_Integer)
            return CompareIntegerECEnumerators(change.Enumerators(), a.GetEnumerators(), b.GetEnumerators());

        return CompareStringECEnumerators(change.Enumerators(), a.GetEnumerators(), b.GetEnumerators());
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSchemaComparer::CompareIntegerECEnumerators(ECEnumeratorChanges& changes, EnumeratorIterable const& a, EnumeratorIterable const& b)
    {
    std::map<int, ECEnumeratorCP> aMap, bMap, cMap;
    for (ECEnumeratorCP enumCP : a)
        aMap[enumCP->GetInteger()] = enumCP;

    for (ECEnumeratorCP enumCP : b)
        bMap[enumCP->GetInteger()] = enumCP;

    cMap.insert(aMap.cbegin(), aMap.cend());
    cMap.insert(bMap.cbegin(), bMap.cend());

    for (auto& u : cMap)
        {
        auto itorA = aMap.find(u.first);
        auto itorB = bMap.find(u.first);

        bool existInA = itorA != aMap.end();
        bool existInB = itorB != bMap.end();
        if (existInA && existInB)
            {
            if (itorA->second->GetDisplayLabel() != itorB->second->GetDisplayLabel())
                changes.Add(ChangeState::Modified).GetDisplayLabel().SetValue(itorA->second->GetDisplayLabel(), itorB->second->GetDisplayLabel());
            }
        else if (existInA && !existInB)
            {
            auto& change = changes.Add(ChangeState::Deleted);
            change.GetDisplayLabel().SetValue(ValueId::Deleted, itorA->second->GetDisplayLabel());
            change.GetInteger().SetValue(ValueId::Deleted, itorA->second->GetInteger());
            }
        else if (!existInA && existInB)
            {
            auto& change = changes.Add(ChangeState::New);
            change.GetDisplayLabel().SetValue(ValueId::New, itorB->second->GetDisplayLabel());
            change.GetInteger().SetValue(ValueId::New, itorB->second->GetInteger());
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSchemaComparer::CompareStringECEnumerators(ECEnumeratorChanges& changes, EnumeratorIterable const& a, EnumeratorIterable const& b)
    {
    std::map<Utf8CP, ECEnumeratorCP, CompareIUtf8> aMap, bMap, cMap;
    for (ECEnumeratorCP enumCP : a)
        aMap[enumCP->GetString().c_str()] = enumCP;

    for (ECEnumeratorCP enumCP : b)
        bMap[enumCP->GetString().c_str()] = enumCP;

    cMap.insert(aMap.cbegin(), aMap.cend());
    cMap.insert(bMap.cbegin(), bMap.cend());

    for (auto& u : cMap)
        {
        auto itorA = aMap.find(u.first);
        auto itorB = bMap.find(u.first);

        bool existInA = itorA != aMap.end();
        bool existInB = itorB != bMap.end();
        if (existInA && existInB)
            {
            if (itorA->second->GetDisplayLabel() != itorB->second->GetDisplayLabel())
                changes.Add(ChangeState::Modified).GetDisplayLabel().SetValue(itorA->second->GetDisplayLabel(), itorB->second->GetDisplayLabel());
            }
        else if (existInA && !existInB)
            {
            auto& change = changes.Add(ChangeState::Deleted);
            change.GetDisplayLabel().SetValue(ValueId::Deleted, itorA->second->GetDisplayLabel());
            change.GetString().SetValue(ValueId::Deleted, itorA->second->GetString());
            }
        else if (!existInA && existInB)
            {
            auto& change = changes.Add(ChangeState::New);
            change.GetDisplayLabel().SetValue(ValueId::New, itorB->second->GetDisplayLabel());
            change.GetString().SetValue(ValueId::New, itorB->second->GetString());
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
//BentleyStatus ECSchemaComparer::CompareKindOfQuantity(KindOfQuantityChange& change, KindOfQuantityCR a, KindOfQuantityCR b)
//    {
//    if (a.GetName() != b.GetName())
//        change.GetName().SetValue(a.GetName(), b.GetName());
//
//    if (a.GetDisplayLabel() != b.GetDisplayLabel())
//        change.GetDisplayLabel().SetValue(a.GetDisplayLabel(), b.GetDisplayLabel());
//
//    if (a.GetDescription() != b.GetDescription())
//        change.GetDescription().SetValue(a.GetDescription(), b.GetDescription());
//
//    if (a.GetPersistenceUnit() != b.GetPersistenceUnit())
//        change.GetPersistenceUnit().SetValue(a.GetPersistenceUnit(), b.GetPersistenceUnit());
//
//    if (a.GetPrecision() != b.GetPrecision())
//        change.GetPrecision().SetValue(a.GetPrecision(), b.GetPrecision());
//
//    if (a.GetDefaultPresentationUnit() != b.GetDefaultPresentationUnit())
//        change.GetDefaultPresentationUnit().SetValue(a.GetDefaultPresentationUnit(), b.GetDefaultPresentationUnit());
//
//
//    std::set<Utf8CP, CompareUtf8> aMap, bMap, cMap;
//    for (Utf8StringCR unit : a.GetAlternativePresentationUnitList())
//        aMap.insert(unit.c_str());
//
//    for (Utf8StringCR unit : b.GetAlternativePresentationUnitList())
//        bMap.insert(unit.c_str());
//
//    cMap.insert(aMap.cbegin(), aMap.cend());
//    cMap.insert(bMap.cbegin(), bMap.cend());
//
//    for (auto u : cMap)
//        {
//        auto itorA = aMap.find(u);
//        auto itorB = bMap.find(u);
//
//        bool existInA = itorA != aMap.end();
//        bool existInB = itorB != bMap.end();
//        if (existInA && existInB)
//            {
//            }
//        else if (existInA && !existInB)
//            change.GetAlternativePresentationUnitList().Add(ChangeState::Deleted).SetValue(ValueId::Deleted, *itorA);
//        else if (!existInA && existInB)
//            change.GetAlternativePresentationUnitList().Add(ChangeState::New).SetValue(ValueId::New, *itorB);
//        }
//    return SUCCESS;
//    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSchemaComparer::CompareBaseClasses(BaseClassChanges& changes, ECBaseClassesList const& a, ECBaseClassesList const& b)
    {
    std::set<Utf8CP, CompareIUtf8> aMap, bMap, cMap;
    for (ECClassCP classCP : a)
        aMap.insert(classCP->GetFullName());

    for (ECClassCP classCP : b)
        bMap.insert(classCP->GetFullName());

    cMap.insert(aMap.cbegin(), aMap.cend());
    cMap.insert(bMap.cbegin(), bMap.cend());

    for (auto& u : cMap)
        {
        auto itorA = aMap.find(u);
        auto itorB = bMap.find(u);

        bool existInA = itorA != aMap.end();
        bool existInB = itorB != bMap.end();
        if (existInA && existInB)
            {
            }
        else if (existInA && !existInB)
            changes.Add(ChangeState::Deleted).SetValue(ValueId::Deleted, u);
        else if (!existInA && existInB)
            changes.Add(ChangeState::New).SetValue(ValueId::New, u);
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSchemaComparer::CompareReferences(ReferenceChanges& changes, ECSchemaReferenceListCR a, ECSchemaReferenceListCR b)
    {
    std::map<Utf8CP, ECSchemaCP, CompareIUtf8> aMap, bMap, cMap;
    for (auto& ref : a)
        aMap[ref.first.GetName().c_str()] = ref.second.get();

    for (auto& ref : b)
        bMap[ref.first.GetName().c_str()] = ref.second.get();

    cMap.insert(aMap.cbegin(), aMap.cend());
    cMap.insert(bMap.cbegin(), bMap.cend());

    for (auto& u : cMap)
        {
        auto itorA = aMap.find(u.first);
        auto itorB = bMap.find(u.first);

        bool existInA = itorA != aMap.end();
        bool existInB = itorB != bMap.end();
        if (existInA && existInB)
            changes.Add(ChangeState::Modified).SetValue(itorA->second->GetFullSchemaName(), itorB->second->GetFullSchemaName());
        else if (existInA && !existInB)
            changes.Add(ChangeState::Deleted).SetValue(ValueId::Deleted, itorA->second->GetFullSchemaName());
        else if (!existInA && existInB)
            changes.Add(ChangeState::New).SetValue(ValueId::New, itorB->second->GetFullSchemaName());
        }
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSchemaComparer::AppendECSchema(ECSchemaChanges& changes, ECSchemaCR v, ValueId appendType)
    {
    ChangeState state = appendType == ValueId::New ? ChangeState::New : ChangeState::Deleted;
    ECSchemaChange& change = changes.Add(state, v.GetName().c_str());

    change.GetName().SetValue(appendType, v.GetName());
    change.GetDisplayLabel().SetValue(appendType, v.GetDisplayLabel());
    change.GetDescription().SetValue(appendType, v.GetDescription());
    change.GetNamespacePrefix().SetValue(appendType, v.GetNamespacePrefix());
    change.GetVersionMajor().SetValue(appendType, v.GetVersionMajor());
    change.GetVersionMinor().SetValue(appendType, v.GetVersionMinor());

    if ((appendType == ValueId::Deleted && m_options.GetSchemaDeleteDetailLevel() == AppendDetailLevel::Partial) ||
        (appendType == ValueId::New && m_options.GetSchemaNewDetailLevel() == AppendDetailLevel::Partial))
        return SUCCESS;

    for (ECClassCP classCP : v.GetClasses())
        if (AppendECClass(change.Classes(), *classCP, appendType) == ERROR)
            return ERROR;

    for (ECEnumerationCP enumerationCP : v.GetEnumerations())
        if (AppendECEnumeration(change.Enumerations(), *enumerationCP, appendType) == ERROR)
            return ERROR;

    //for (KindOfQuantityCP kindOfQuantityCP : v.GetKindOfQuantities())
    //    if (AppendKindOfQuantity(change.KindOfQuantities(), *kindOfQuantityCP, appendType) == ERROR)
    //        return ERROR;

    if (AppendReferences(change.References(), v.GetReferencedSchemas(), appendType) != SUCCESS)
        return ERROR;

    return AppendCustomAttributes(change.CustomAttributes(), v, appendType);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSchemaComparer::AppendECClass(ECClassChanges& changes, ECClassCR v, ValueId appendType)
    {
    ChangeState state = appendType == ValueId::New ? ChangeState::New : ChangeState::Deleted;
    ECClassChange& change = changes.Add(state, v.GetName().c_str());

    change.GetName().SetValue(appendType, v.GetName());
    change.GetDisplayLabel().SetValue(appendType, v.GetDisplayLabel());
    change.GetDescription().SetValue(appendType, v.GetDescription());
    change.GetClassModifier().SetValue(appendType, v.GetClassModifier());
    change.ClassType().SetValue(appendType, v.GetClassType());

    for (ECPropertyCP propertyCP : v.GetProperties(false))
        if (AppendECProperty(change.Properties(), *propertyCP, appendType) == ERROR)
            return ERROR;

    if (v.GetRelationshipClassCP() != nullptr)
        {
        if (AppendECRelationshipClass(change.GetRelationship(), *v.GetRelationshipClassCP(), appendType) == ERROR)
            return ERROR;
        }

    if (AppendBaseClasses(change.BaseClasses(), v.GetBaseClasses(), appendType) != SUCCESS)
        return ERROR;

    return AppendCustomAttributes(change.CustomAttributes(), v, appendType);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSchemaComparer::AppendECRelationshipClass(ECRelationshipChange& change, ECRelationshipClassCR v, ValueId appendType)
    {
    change.GetStrengthDirection().SetValue(appendType, v.GetStrengthDirection());
    change.GetStrength().SetValue(appendType, v.GetStrength());
    if (AppendECRelationshipConstraint(change.GetSource(), v.GetSource(), appendType) == ERROR)
        return ERROR;

    if (AppendECRelationshipConstraint(change.GetTarget(), v.GetTarget(), appendType) == ERROR)
        return ERROR;

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSchemaComparer::AppendECRelationshipConstraint(ECRelationshipConstraintChange& change, ECRelationshipConstraintCR v, ValueId appendType)
    {
    change.GetRoleLabel().SetValue(appendType, v.GetRoleLabel());
    change.GetCardinality().SetValue(appendType, v.GetCardinality().ToString());
    change.IsPolymorphic().SetValue(appendType, v.GetIsPolymorphic());
    if (AppendECRelationshipConstraintClasses(change.ConstraintClasses(), v.GetConstraintClasses(), appendType) != SUCCESS)
        return ERROR;

    return AppendCustomAttributes(change.CustomAttributes(), v, appendType);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSchemaComparer::AppendECRelationshipConstraintClasses(ECRelationshipConstraintClassChanges& changes, ECRelationshipConstraintClassList const& v, ValueId appendType)
    {
    ChangeState state = appendType == ValueId::New ? ChangeState::New : ChangeState::Deleted;
    for (ECRelationshipConstraintClassCP constraintClass : v)
        {
        ECRelationshipConstraintClassChange & constraintClassChange = changes.Add(state);
        if (AppendECRelationshipConstraintClass(constraintClassChange, *constraintClass, appendType) == ERROR)
            return ERROR;
        }
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSchemaComparer::AppendECRelationshipConstraintClass(ECRelationshipConstraintClassChange& change, ECRelationshipConstraintClassCR v, ValueId appendType)
    {
    ChangeState state = appendType == ValueId::New ? ChangeState::New : ChangeState::Deleted;
    change.GetClassName().SetValue(appendType, v.GetClass().GetFullName());
    for (Utf8StringCR relationshipKeyProperty : v.GetKeys())
        {
        if (!relationshipKeyProperty.empty())
            change.KeyProperties().Add(state).SetValue(appendType, relationshipKeyProperty);
        }
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSchemaComparer::AppendECEnumeration(ECEnumerationChanges& changes, ECEnumerationCR v, ValueId appendType)
    {
    ChangeState state = appendType == ValueId::New ? ChangeState::New : ChangeState::Deleted;
    ECEnumerationChange& enumerationChange = changes.Add(state, v.GetName().c_str());
    enumerationChange.GetName().SetValue(appendType, v.GetName());
    enumerationChange.GetDisplayLabel().SetValue(appendType, v.GetDisplayLabel());
    enumerationChange.GetDescription().SetValue(appendType, v.GetDescription());
    enumerationChange.IsStrict().SetValue(appendType, v.GetIsStrict());
    enumerationChange.GetTypeName().SetValue(appendType, v.GetTypeName());
    for (ECEnumeratorCP enumeratorCP : v.GetEnumerators())
        {
        ECEnumeratorChange& enumeratorChange = enumerationChange.Enumerators().Add(state);
        enumeratorChange.GetDisplayLabel().SetValue(appendType, enumeratorCP->GetDisplayLabel());
        if (enumeratorCP->IsInteger())
            enumeratorChange.GetInteger().SetValue(appendType, enumeratorCP->GetInteger());

        if (enumeratorCP->IsString())
            enumeratorChange.GetString().SetValue(appendType, enumeratorCP->GetString());
        }
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
//BentleyStatus ECSchemaComparer::AppendKindOfQuantity(ECKindOfQuantityChanges& changes, KindOfQuantityCR v, ValueId appendType)
//    {
//    ChangeState state = appendType == ValueId::New ? ChangeState::New : ChangeState::Deleted;
//    KindOfQuantityChange& kindOfQuantityChange = changes.Add(state, v.GetName().c_str());
//    kindOfQuantityChange.GetName().SetValue(appendType, v.GetName());
//    kindOfQuantityChange.GetDisplayLabel().SetValue(appendType, v.GetDisplayLabel());
//    kindOfQuantityChange.GetDescription().SetValue(appendType, v.GetDescription());
//    kindOfQuantityChange.GetPersistenceUnit().SetValue(appendType, v.GetPersistenceUnit());
//    kindOfQuantityChange.GetPrecision().SetValue(appendType, v.GetPrecision());
//    kindOfQuantityChange.GetDefaultPresentationUnit().SetValue(appendType, v.GetDefaultPresentationUnit());
//    for (Utf8StringCR unitstr : v.GetAlternativePresentationUnitList())
//        {
//        kindOfQuantityChange.GetAlternativePresentationUnitList().Add(state).SetValue(appendType, unitstr);
//        }
//
//    return SUCCESS;
//    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSchemaComparer::AppendECProperty(ECPropertyChanges& changes, ECPropertyCR v, ValueId appendType)
    {
    ChangeState state = appendType == ValueId::New ? ChangeState::New : ChangeState::Deleted;
    ECPropertyChange& propertyChange = changes.Add(state, v.GetName().c_str());
    propertyChange.GetName().SetValue(appendType, v.GetName());
    propertyChange.GetDisplayLabel().SetValue(appendType, v.GetDisplayLabel());
    propertyChange.GetDescription().SetValue(appendType, v.GetDescription());
    propertyChange.GetTypeName().SetValue(appendType, v.GetTypeName());

    if (auto prop = v.GetAsNavigationProperty())
        {
        propertyChange.IsNavigation().SetValue(appendType, true);
        NavigationChange& navigationChange = propertyChange.GetNavigation();
        navigationChange.Direction().SetValue(appendType, prop->GetDirection());
        if (prop->GetRelationshipClass())
            navigationChange.GetRelationshipClassName().SetValue(appendType, prop->GetRelationshipClass()->GetFullName());
        }
    else if (v.GetIsPrimitive())
        {
        propertyChange.IsPrimitive().SetValue(appendType, true);
        }
    else if (v.GetIsStruct())
        {
        propertyChange.IsStruct().SetValue(appendType, true);
        }
    else if (v.GetIsStructArray())
        {
        propertyChange.IsStructArray().SetValue(appendType, true);
        }
    else if (v.GetIsPrimitiveArray())
        {
        propertyChange.IsPrimitiveArray().SetValue(appendType, true);
        }
    else
        {
        return ERROR;
        }

    //if (v.IsMaximumValueDefined())
    //    propertyChange.GetMaximumValue().SetValue(appendType, v.GetMaximumValue());

    //if (v.IsMinimumValueDefined())
    //    propertyChange.GetMinimumValue().SetValue(appendType, v.GetMinimumValue());

    if (v.GetIsArray())
        {
        auto arrayProp = v.GetAsArrayProperty();
        propertyChange.GetArray().MaxOccurs().SetValue(appendType, arrayProp->GetStoredMaxOccurs());
        propertyChange.GetArray().MinOccurs().SetValue(appendType, arrayProp->GetMinOccurs());
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSchemaComparer::AppendBaseClasses(BaseClassChanges& changes, ECBaseClassesList const& v, ValueId appendType)
    {
    ChangeState state = appendType == ValueId::New ? ChangeState::New : ChangeState::Deleted;
    for (ECClassCP baseClassCP : v)
        changes.Add(state).SetValue(appendType, baseClassCP->GetFullName());

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSchemaComparer::AppendReferences(ReferenceChanges& changes, ECSchemaReferenceListCR v, ValueId appendType)
    {
    ChangeState state = appendType == ValueId::New ? ChangeState::New : ChangeState::Deleted;
    for (auto& referenceCP : v)
        changes.Add(state).SetValue(appendType, referenceCP.first.GetFullSchemaName());

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSchemaComparer::ConvertECInstanceToValueMap(std::map<Utf8String, ECValue>& map, IECInstanceCR instance)
    {
    ECValuesCollectionPtr values = ECValuesCollection::Create(instance);
    if (values.IsNull())
        return SUCCESS;

    return ConvertECValuesCollectionToValueMap(map, *values);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSchemaComparer::ConvertECValuesCollectionToValueMap(std::map<Utf8String, ECValue>& map, ECValuesCollectionCR values)
    {
    for (ECValuesCollection::const_iterator itor = values.begin(); itor != values.end(); ++itor)
        {
        ECValueAccessorCR valueAccessor = (*itor).GetValueAccessor();

        if ((*itor).HasChildValues())
            {
            if (ConvertECValuesCollectionToValueMap(map, *(*itor).GetChildValues()) != SUCCESS)
                return ERROR;
            }
        else
            {
            ECValueCR v = (*itor).GetValue();
            if (!v.IsPrimitive())
                continue;
        
            if (v.IsNull())
                continue;

            map[valueAccessor.GetManagedAccessString()] = v;
            }
        }
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
std::vector<Utf8String> ECSchemaComparer::Split(Utf8StringCR path)
    {
    std::vector<Utf8String> axis;
    size_t b = 0;
    size_t i = 0;
    for (; i < path.size(); i++)
        {
        if (path[i] == '.')
            {
            axis.push_back(path.substr(b, i - b));
            b = i + 1;
            }
        }

    if (b < i)
        axis.push_back(path.substr(b , i - b ));

    return axis;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String ECSchemaComparer::Join(std::vector<Utf8String> const& paths, Utf8CP delimiter)
    {
    Utf8String str;
    for (auto itor = paths.begin(); itor != paths.end(); ++itor)
        {
        if (itor != paths.begin())
            str.append(delimiter);
        
        str.append(*itor);
        }

    return str;
    }
//======================================================================================
//ECChange
//======================================================================================
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
ECChange::ECChange(ChangeState state, SystemId systemId, ECChange const* parent, Utf8CP customId)
    :m_systemId(systemId), m_customId(customId), m_state(state), m_status(Status::Pending), m_parent(parent)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
Utf8CP ECChange::GetId() const
    {
    if (!m_customId.empty())
        return m_customId.c_str();

    return SystemIdToString(m_systemId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
//static  
void ECChange::AppendBegin(Utf8StringR str, ECChange const& change, int currentIndex)
    {
    if (change.GetState() == ChangeState::Deleted)
        str += "-";
    else if (change.GetState() == ChangeState::New)
        str += "+";
    else if (change.GetState() == ChangeState::Modified)
        str += "!";

    for (int i = 0; i < currentIndex; i++)
        str.append(" ");

    str.append(SystemIdToString(change.GetSystemId()));

    if (change.HasCustomId())
        str.append("(").append(change.GetId()).append(")");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
//static 
SystemId ECChange::StringToSystemId(Utf8CP idString)
    {
    if (Utf8String::IsNullOrEmpty(idString))
        return SystemId::None;

    if (strcmp(idString, "AlternativePresentationUnitList") == 0)  return SystemId::AlternativePresentationUnitList;
    if (strcmp(idString, "Array") == 0) return SystemId::Array;
    if (strcmp(idString, "BaseClass") == 0) return SystemId::BaseClass;
    if (strcmp(idString, "BaseClasses") == 0) return SystemId::BaseClasses;
    if (strcmp(idString, "Cardinality") == 0) return SystemId::Cardinality;
    if (strcmp(idString, "Classes") == 0) return SystemId::Classes;
    if (strcmp(idString, "Class") == 0) return SystemId::Class;
    if (strcmp(idString, "ConstantKey") == 0) return SystemId::ConstantKey;
    if (strcmp(idString, "ClassFullName") == 0) return SystemId::ClassFullName;
    if (strcmp(idString, "ClassModifier") == 0) return SystemId::ClassModifier;
    if (strcmp(idString, "ConstraintClass") == 0) return SystemId::ConstraintClass;
    if (strcmp(idString, "ConstraintClasses") == 0) return SystemId::ConstraintClasses;
    if (strcmp(idString, "Constraint") == 0) return SystemId::Constraint;
    if (strcmp(idString, "CustomAttributes") == 0) return SystemId::CustomAttributes;
    if (strcmp(idString, "DefaultPresentationUnit") == 0) return SystemId::DefaultPresentationUnit;
    if (strcmp(idString, "Description") == 0) return SystemId::Description;
    if (strcmp(idString, "Direction") == 0) return SystemId::Direction;
    if (strcmp(idString, "PropertyValue") == 0) return SystemId::PropertyValue;
    if (strcmp(idString, "PropertyValues") == 0) return SystemId::PropertyValues;
    if (strcmp(idString, "DisplayLabel") == 0) return SystemId::DisplayLabel;
    if (strcmp(idString, "Enumeration") == 0) return SystemId::Enumeration;
    if (strcmp(idString, "Enumerations") == 0) return SystemId::Enumerations;
    if (strcmp(idString, "Enumerator") == 0) return SystemId::Enumerator;
    if (strcmp(idString, "Enumerators") == 0) return SystemId::Enumerators;
    if (strcmp(idString, "ExtendedTypeName") == 0) return SystemId::ExtendedTypeName;
    if (strcmp(idString, "Instance") == 0) return SystemId::Instance;
    if (strcmp(idString, "Instances") == 0) return SystemId::Instances;
    if (strcmp(idString, "Integer") == 0) return SystemId::Integer;
    if (strcmp(idString, "ClassType") == 0) return SystemId::ClassType;
    if (strcmp(idString, "IsPolymorphic") == 0) return SystemId::IsPolymorphic;
    if (strcmp(idString, "IsReadonly") == 0) return SystemId::IsReadonly;
    if (strcmp(idString, "IsStrict") == 0) return SystemId::IsStrict;
    if (strcmp(idString, "IsStruct") == 0) return SystemId::IsStruct;
    if (strcmp(idString, "IsStructArray") == 0) return SystemId::IsStructArray;
    if (strcmp(idString, "IsPrimitive") == 0) return SystemId::IsPrimitive;
    if (strcmp(idString, "IsPrimitiveArray") == 0) return SystemId::IsPrimitiveArray;
    if (strcmp(idString, "IsNavigation") == 0) return SystemId::IsNavigation;
    if (strcmp(idString, "KeyProperties") == 0) return SystemId::KeyProperties;
    if (strcmp(idString, "KeyProperty") == 0) return SystemId::KeyProperty;
    if (strcmp(idString, "KindOfQuantities") == 0) return SystemId::KindOfQuantities;
    if (strcmp(idString, "KindOfQuantity") == 0) return SystemId::KindOfQuantity;
    if (strcmp(idString, "MaximumValue") == 0) return SystemId::MaximumValue;
    if (strcmp(idString, "MaxOccurs") == 0) return SystemId::MaxOccurs;
    if (strcmp(idString, "MinimumValue") == 0) return SystemId::MinimumValue;
    if (strcmp(idString, "MinOccurs") == 0) return SystemId::MinOccurs;
    if (strcmp(idString, "Name") == 0) return SystemId::Name;
    if (strcmp(idString, "NamespacePrefix") == 0) return SystemId::NamespacePrefix;
    if (strcmp(idString, "Navigation") == 0) return SystemId::Navigation;
    if (strcmp(idString, "PersistenceUnit") == 0) return SystemId::PersistenceUnit;
    if (strcmp(idString, "Precision") == 0) return SystemId::Precision;
    if (strcmp(idString, "Properties") == 0) return SystemId::Properties;
    if (strcmp(idString, "Property") == 0) return SystemId::Property;
    if (strcmp(idString, "PropertyType") == 0) return SystemId::PropertyType;
    if (strcmp(idString, "Reference") == 0) return SystemId::Reference;
    if (strcmp(idString, "References") == 0) return SystemId::References;
    if (strcmp(idString, "Relationship") == 0) return SystemId::Relationship;
    if (strcmp(idString, "RelationshipName") == 0) return SystemId::RelationshipName;
    if (strcmp(idString, "RoleLabel") == 0) return SystemId::RoleLabel;
    if (strcmp(idString, "Schema") == 0) return SystemId::Schema;
    if (strcmp(idString, "Schemas") == 0) return SystemId::Schemas;
    if (strcmp(idString, "Source") == 0) return SystemId::Source;
    if (strcmp(idString, "StrengthDirection") == 0) return SystemId::StrengthDirection;
    if (strcmp(idString, "StrengthType") == 0) return SystemId::StrengthType;
    if (strcmp(idString, "String") == 0) return SystemId::String;
    if (strcmp(idString, "Target") == 0) return SystemId::Target;
    if (strcmp(idString, "TypeName") == 0) return SystemId::TypeName;
    if (strcmp(idString, "VersionMajor") == 0) return SystemId::VersionMajor;
    if (strcmp(idString, "VersionMinor") == 0) return SystemId::VersionMinor;
    if (strcmp(idString, "VersionWrite") == 0) return SystemId::VersionWrite;

    BeAssert(false && "Unknown SystemId");
    return SystemId::None;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
//static 
Utf8CP ECChange::SystemIdToString(SystemId id)
    {
    switch (id)
        {
            case SystemId::None: return "";
            case SystemId::AlternativePresentationUnitList: return "AlternativePresentationUnitList";
            case SystemId::Array: return "Array";
            case SystemId::BaseClass: return "BaseClass";
            case SystemId::BaseClasses: return "BaseClasses";
            case SystemId::Cardinality: return "Cardinality";
            case SystemId::Classes: return "Classes";
            case SystemId::Class: return "Class";
            case SystemId::ConstantKey: return "ConstantKey";
            case SystemId::ClassFullName: return "ClassFullName";
            case SystemId::ClassModifier: return "ClassModifier";
            case SystemId::ConstraintClass: return "ConstraintClass";
            case SystemId::ConstraintClasses: return "ConstraintClasses";
            case SystemId::Constraint: return "Constraint";
            case SystemId::CustomAttributes: return "CustomAttributes";
            case SystemId::DefaultPresentationUnit: return "DefaultPresentationUnit";
            case SystemId::Description: return "Description";
            case SystemId::Direction: return "Direction";
            case SystemId::PropertyValue: return "PropertyValue";
            case SystemId::PropertyValues: return "PropertyValues";
            case SystemId::DisplayLabel: return "DisplayLabel";
            case SystemId::Enumeration: return "Enumeration";
            case SystemId::Enumerations: return "Enumerations";
            case SystemId::Enumerator: return "Enumerator";
            case SystemId::Enumerators: return "Enumerators";
            case SystemId::ExtendedTypeName: return "ExtendedTypeName";
            case SystemId::Instance: return "Instance";
            case SystemId::Instances: return "Instances";
            case SystemId::Integer: return "Integer";
            case SystemId::ClassType: return "ClassType";
            case SystemId::IsPolymorphic: return "IsPolymorphic";
            case SystemId::IsReadonly: return "IsReadonly";
            case SystemId::IsStrict: return "IsStrict";
            case SystemId::IsStruct: return "IsStruct";
            case SystemId::IsStructArray: return "IsStructArray";
            case SystemId::IsPrimitive: return "IsPrimitive";
            case SystemId::IsPrimitiveArray: return "IsPrimitiveArray";
            case SystemId::IsNavigation: return "IsNavigation";
            case SystemId::KeyProperties: return "KeyProperties";
            case SystemId::KeyProperty: return "KeyProperty";
            case SystemId::KindOfQuantities: return "KindOfQuantities";
            case SystemId::KindOfQuantity: return "KindOfQuantity";
            case SystemId::MaximumValue: return "MaximumValue";
            case SystemId::MaxOccurs: return "MaxOccurs";
            case SystemId::MinimumValue: return "MinimumValue";
            case SystemId::MinOccurs: return "MinOccurs";
            case SystemId::Name: return "Name";
            case SystemId::NamespacePrefix: return "NamespacePrefix";
            case SystemId::Navigation: return "Navigation";
            case SystemId::PersistenceUnit: return "PersistenceUnit";
            case SystemId::Precision: return "Precision";
            case SystemId::Properties: return "Properties";
            case SystemId::Property: return "Property";
            case SystemId::PropertyType: return "PropertyType";
            case SystemId::Reference: return "Reference";
            case SystemId::References: return "References";
            case SystemId::Relationship: return "Relationship";
            case SystemId::RelationshipName: return "RelationshipName";
            case SystemId::RoleLabel: return "RoleLabel";
            case SystemId::Schema: return "Schema";
            case SystemId::Schemas: return "Schemas";
            case SystemId::Source: return "Source";
            case SystemId::StrengthDirection: return "StrengthDirection";
            case SystemId::StrengthType: return "StrengthType";
            case SystemId::String: return "String";
            case SystemId::Target: return "Target";
            case SystemId::TypeName: return "TypeName";
            case SystemId::VersionMajor: return "VersionMajor";
            case SystemId::VersionMinor: return "VersionMinor";
            case SystemId::VersionWrite: return "VersionWrite";

            default:
                BeAssert(false && "Unhandled SystemId");
                return "";
        }
    }

//======================================================================================>
//ECObjectChange
//======================================================================================>
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
void ECObjectChange::_WriteToString(Utf8StringR str, int currentIndex, int indentSize) const 
    {
    AppendBegin(str, *this, currentIndex);
    AppendEnd(str);
    for (auto& change : m_changes)
        {
        change.second->WriteToString(str, currentIndex + indentSize, indentSize);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
bool ECObjectChange::_IsEmpty() const
    {
    for (auto& change : m_changes)
        {
        if (!change.second->IsEmpty())
            return false;
        }

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
void ECObjectChange::_Optimize() 
    {
    auto itor = m_changes.begin();
    while (itor != m_changes.end())
        {
        itor->second->Optimize();
        if (itor->second->IsEmpty())
            itor = m_changes.erase(itor);
        else
            ++itor;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
template<typename T>
T& ECObjectChange::Get(SystemId systemId)
    {
    static_assert(std::is_base_of<ECChange, T>::value, "T not derived from ECChange");
    Utf8CP id = SystemIdToString(systemId);
    auto itor = m_changes.find(id);
    if (itor != m_changes.end())
        return *(static_cast<T*>(itor->second.get()));

    ECChangePtr changePtr = new T(GetState(), systemId, this, nullptr);
    ECChange* changeP = changePtr.get();
    m_changes[changePtr->GetId()] = changePtr;
    return *(static_cast<T*>(changeP));
    }
//======================================================================================>
//StringChange
//======================================================================================>
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String StringChange::_ToString(ValueId id) const
    {
    Utf8String str;
    auto& v = GetValue(id);
    if (v.IsNull())
        str = NULL_TEXT;
    else
        str = v.Value();
    return str;
    }
//======================================================================================>
//BooleanChange
//======================================================================================>
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String BooleanChange::_ToString(ValueId id) const
    {
    Utf8String str;
    auto& v = GetValue(id);
    if (v.IsNull())
        str = NULL_TEXT;
    else
        str = v.Value() ? "True" : "False";
    return str;
    }

//======================================================================================>
//UInt32Change
//======================================================================================>
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String UInt32Change::_ToString(ValueId id) const
    {
    Utf8String str;
    auto& v = GetValue(id);
    if (v.IsNull())
        str = NULL_TEXT;
    else
        str.Sprintf("%u", v.Value());

    return str;
    }

//======================================================================================>
//Int32Change
//======================================================================================>
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String Int32Change::_ToString(ValueId id) const
    {
    Utf8String str;
    Nullable<int32_t> const& v = GetValue(id);
    if (v.IsNull())
        str = NULL_TEXT;
    else
        str.Sprintf("%" PRId32, v.Value());

    return str;
    }

//======================================================================================>
//DoubleChange
//======================================================================================>
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String DoubleChange::_ToString(ValueId id) const
    {
    Utf8String str;
    auto& v = GetValue(id);
    if (v.IsNull())
        str = NULL_TEXT;
    else
        str.Sprintf("%.17g", v.Value());

    return str;
    }

//======================================================================================>
//DateTimeChange
//======================================================================================>
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String DateTimeChange::_ToString(ValueId id) const
    {
    Utf8String str;
    auto& v = GetValue(id);
    if (v.IsNull())
        str = NULL_TEXT;
    else
        str.assign(v.Value().ToUtf8String());

    return str;
    }

//======================================================================================>
//Point2DChange
//======================================================================================>
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String Point2DChange::_ToString(ValueId id) const
    {
    Utf8String str;
    auto& v = GetValue(id);
    if (v.IsNull())
        str = NULL_TEXT;
    else
        str.Sprintf("(%.17g, %.17g)", v.Value().x, v.Value().y);
    return str;
    }

//======================================================================================>
//Point3DChange
//======================================================================================>
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String Point3DChange::_ToString(ValueId id) const
    {
    Utf8String str;
    auto& v = GetValue(id);
    if (v.IsNull())
        str = NULL_TEXT;
    else
        str.Sprintf("(%.17g, %.17g, %.17g)", v.Value().x, v.Value().y, v.Value().z);

    return str;
    }

//======================================================================================>
//Int64Change
//======================================================================================>
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String Int64Change::_ToString(ValueId id) const
    {
    Utf8String str;
    Nullable<int64_t> const& v = GetValue(id);
    if (v.IsNull())
        str = NULL_TEXT;
    else
        str.Sprintf("%" PRId64, v.Value());

    return str;
    }

//======================================================================================>
//StrengthTypeChange
//======================================================================================>
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String StrengthTypeChange::_ToString(ValueId id) const 
    {
    Utf8String str;
    auto& v = GetValue(id);
    if (v.IsNull())
        str = NULL_TEXT;
    else
        {
        if (v.Value() == ECN::StrengthType::Embedding)
            str = "Embedding";
        else if (v.Value() == ECN::StrengthType::Holding)
            str = "Holding";
        else if (v.Value() == ECN::StrengthType::Referencing)
            str = "Referencing";
        }
    return str;
    }

//======================================================================================>
//StrengthDirectionChange
//======================================================================================>
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String StrengthDirectionChange::_ToString(ValueId id) const 
    {
    Utf8String str;
    auto& v = GetValue(id);
    if (v.IsNull())
        str = NULL_TEXT;
    else
        {
        if (v.Value() == ECN::ECRelatedInstanceDirection::Backward)
            str = "Backward";
        else if (v.Value() == ECN::ECRelatedInstanceDirection::Forward)
            str = "Forward";
        }
    return str;
    }

//======================================================================================>
//ClassModifierChange
//======================================================================================>
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String ClassModifierChange::_ToString(ValueId id) const 
    {
    Utf8String str;
    auto& v = GetValue(id);
    if (v.IsNull())
        str = NULL_TEXT;
    else
        {
        if (v.Value() == ECN::ECClassModifier::Abstract)
            str = "Abstract";
        else if (v.Value() == ECN::ECClassModifier::None)
            str = "None";
        else if (v.Value() == ECN::ECClassModifier::Sealed)
            str = "Sealed";

        }
    return str;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String ClassTypeChange::_ToString(ValueId id) const
    {
    Utf8String str;
    auto& v = GetValue(id);
    if (v.IsNull())
        str = NULL_TEXT;
    else
        {
        if (v.Value() == ECClassType::CustomAttribute)
            str = "CustomAttribute";
        else if (v.Value() == ECClassType::Entity)
            str = "Entity";
        else if (v.Value() == ECClassType::Relationship)
            str = "Relationship";
        else if (v.Value() == ECClassType::Struct)
            str = "Struct";
        }
    return str;
    }
//======================================================================================>
//ECPropertyValueChange
//======================================================================================>
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECPropertyValueChange::SetValue(ValueId id, ECValueCR value)
    {
    if (!value.IsPrimitive())
        return ERROR;

    if (InitValue(value.GetPrimitiveType()) != SUCCESS)
        return ERROR;

    if (value.IsNull())
        return SUCCESS;

    switch (m_type)
        {
            case PRIMITIVETYPE_IGeometry:
                {
                BeAssert(false && "Gemoetry not supported type");
                return ERROR;
                }
            case PRIMITIVETYPE_Binary:
                {
                Binary v;
                v.CopyFrom(value);
                return GetBinary()->SetValue(id, std::move(v));
                }
            case PRIMITIVETYPE_Boolean:
                return GetBoolean()->SetValue(id, value.GetBoolean());
            case PRIMITIVETYPE_DateTime:
                return GetDateTime()->SetValue(id, value.GetDateTime());
            case PRIMITIVETYPE_Double:
                return GetDouble()->SetValue(id, value.GetDouble());
            case PRIMITIVETYPE_Integer:
                return GetInteger()->SetValue(id, value.GetInteger());
            case PRIMITIVETYPE_Long:
                return GetLong()->SetValue(id, value.GetLong());
            case PRIMITIVETYPE_Point2D:
                return GetPoint2D()->SetValue(id, value.GetPoint2D());
            case PRIMITIVETYPE_Point3D:
                return GetPoint3D()->SetValue(id, value.GetPoint3D());
            case PRIMITIVETYPE_String:
                return GetString()->SetValue(id, value.GetUtf8CP());
        }

    return ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECPropertyValueChange::SetValue(ECValueCR oldValue, ECValueCR newValue)
    {
    const PrimitiveType NullType = static_cast<PrimitiveType>(0);
    PrimitiveType oldType = NullType;
    PrimitiveType newType = NullType;
    if (oldValue.IsPrimitive())
        oldType = oldValue.GetPrimitiveType();

    if (newValue.IsPrimitive())
        oldType = oldValue.GetPrimitiveType();

    if (oldType == NullType && newType == NullType)
        return ERROR;

    if (oldType == NullType)
        oldType = newType;

    if (newType == NullType)
        newType = oldType;

    if (InitValue(oldType) != SUCCESS)
        return ERROR;

    switch (m_type)
        {
            case PRIMITIVETYPE_IGeometry:
                {
                BeAssert(false && "Gemoetry not supported type");
                return ERROR;
                }
            case PRIMITIVETYPE_Binary:
                return GetBinary()->SetValue(Converter<Binary>::Copy(oldValue), Converter<Binary>::Copy(newValue));
            case PRIMITIVETYPE_Boolean:
                return GetBoolean()->SetValue(Converter<bool>::Copy(oldValue), Converter<bool>::Copy(newValue));
            case PRIMITIVETYPE_DateTime:
                return GetDateTime()->SetValue(Converter<DateTime>::Copy(oldValue), Converter<DateTime>::Copy(newValue));
            case PRIMITIVETYPE_Double:
                return GetDouble()->SetValue(Converter<double>::Copy(oldValue), Converter<double>::Copy(newValue));
            case PRIMITIVETYPE_Integer:
                return GetInteger()->SetValue(Converter<int>::Copy(oldValue), Converter<int>::Copy(newValue));
            case PRIMITIVETYPE_Long:
                return GetLong()->SetValue(Converter<int64_t>::Copy(oldValue), Converter<int64_t>::Copy(newValue));
            case PRIMITIVETYPE_Point2D:
                return GetPoint2D()->SetValue(Converter<DPoint2d>::Copy(oldValue), Converter<DPoint2d>::Copy(newValue));
            case PRIMITIVETYPE_Point3D:
                return GetPoint3D()->SetValue(Converter<DPoint3d>::Copy(oldValue), Converter<DPoint3d>::Copy(newValue));
            case PRIMITIVETYPE_String:
                return GetString()->SetValue(Converter<Utf8String>::Copy(oldValue), Converter<Utf8String>::Copy(newValue));
        }

    return ERROR;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
void ECPropertyValueChange::_WriteToString(Utf8StringR str, int currentIndex, int indentSize) const
    {
    if (m_value != nullptr)
        {
        m_value->WriteToString(str, currentIndex, indentSize);
        return;
        }

    AppendBegin(str, *this, currentIndex);
    AppendEnd(str);


    if (m_children != nullptr)
        {
        //AppendBegin(str, *this, currentIndex);
        for (size_t i = 0; i < m_children->Count(); i++)
            {
            m_children->At(i).WriteToString(str, currentIndex + indentSize, indentSize);
            }
        //AppendEnd(str);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
bool ECPropertyValueChange::_IsEmpty() const
    {
    if (auto parent = GetParent())
        if (parent->GetSystemId() == SystemId::CustomAttributes && GetState() != ChangeState::Modified)
            return false;

    if (m_value != nullptr)
        return m_value->IsEmpty();

    if (m_children != nullptr)
        return m_children->IsEmpty();

    return true;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
void ECPropertyValueChange::_Optimize()
    {
    if (m_value != nullptr)
        if (m_value->IsEmpty())
            m_value = nullptr;

    if (m_children != nullptr)
        if (m_children->IsEmpty())
            m_children = nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
ECPropertyValueChange::ECPropertyValueChange(ChangeState state, SystemId systemId, ECChange const* parent, Utf8CP customId)
    : ECChange(state, SystemId::PropertyValue, parent, customId),m_type(static_cast<PrimitiveType>(0))
    {
    BeAssert(systemId == GetSystemId());
    if (ECChangeArray<ECPropertyValueChange> const* array = dynamic_cast <ECChangeArray<ECPropertyValueChange> const*>(parent))
        {
        if (ECPropertyValueChange const* p = dynamic_cast <ECPropertyValueChange const*>(array->GetParent()))
            {
            m_accessString.append(p->m_accessString).append(".");
            }
        }
    m_accessString.append(GetId());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
void  ECPropertyValueChange::GetFlatListOfChildren(std::vector<ECPropertyValueChange*>& childrens)
    {
    if (HasChildren())
        {
        ECChangeArray<ECPropertyValueChange>& children = GetChildren();
        for (size_t i = 0; i < children.Count(); i++)
            {
            children.At(i).GetFlatListOfChildren(childrens);
            }
        }
    else
        childrens.push_back(this);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
bool ECPropertyValueChange::HasValue() const { return m_value != nullptr; }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
bool ECPropertyValueChange::HasChildren() const { return m_children != nullptr; }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
ECChangeArray<ECPropertyValueChange>& ECPropertyValueChange::GetChildren()
    {
    if (m_children == nullptr)
        m_children = std::unique_ptr<ECChangeArray<ECPropertyValueChange>>(new ECChangeArray<ECPropertyValueChange>(GetState(), SystemId::PropertyValues, this, GetId(), SystemId::PropertyValue));

    return *m_children;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
ECPropertyValueChange& ECPropertyValueChange::GetOrCreate(ChangeState stat, std::vector<Utf8String> const& path)
    {
    ECPropertyValueChange* c = this;
    for (auto& str : path)
        {
        auto m = c->GetChildren().Find(str.c_str());
        if (m == nullptr)
            {
            c = &c->GetChildren().Add(stat, str.c_str());
            }
        else
            c = m;
        }

    return *c;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
ECPropertyValueChange* ECPropertyValueChange::GetValue(Utf8CP accessPath) 
    {
    Utf8String ap = accessPath;
    std::vector<Utf8String> path = ECSchemaComparer::Split(ap);
    if (path.empty())
        return nullptr;

    if (ap.find(":") != Utf8String::npos)
        {
        if (path.front() != GetId())
            return nullptr;

        path.erase(path.begin());
        }

    ECPropertyValueChange* c = this;
    for (auto& str : path)
        {
        if (!c->HasChildren())
            return nullptr;

        auto m = c->GetChildren().Find(str.c_str());
        if (m == nullptr)
            {
            return nullptr;
            }
        else
            c = m;
        }

    return c;
    }

//======================================================================================>
//CustomAttributeValidator::Rule
//======================================================================================>
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
CustomAttributeValidator::Rule::Rule(Policy policy, Utf8CP pattren)
    :m_policy(policy), m_pattren(ECSchemaComparer::Split(pattren))
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
bool CustomAttributeValidator::Rule::Match(std::vector<Utf8String> const& source) const
    {
    if (source.empty())
        return false;

    if (m_pattren.size() == 1)//Foo:*
        if (m_pattren.front().EndsWith("*"))
            return true;

    auto sb = source.begin();
    auto pb = m_pattren.begin();
    auto se = source.end();
    auto pe = m_pattren.end();
    while (sb != se && pb != pe)
        {
        if (*sb == *pb)
            {
            ++sb;
            ++pb;

            if (sb == se && pb == pe) //both stream ended its a match
                return true;

            if (sb != se && pb == pe)
                return false;

            if (sb == se && pb != pe)
                {
                while (pb != pe && *pb == "*") ++pb;
                if (pb == pe)
                    return true;

                return false;
                }
            }
        else if (*pb == "*")
            {
            while (pb != pe && *pb == "*") ++pb; //skip *.*.*
            if (pb == pe) //last token seen was a * which mean success
                return true;
            else
                {
                while (*sb != *pb) //last token is not a * which mean we need to find a token in source that matchs it
                    {
                    ++sb;
                    if (sb == se)
                        return false;
                    }
                //we found one
                ++sb; //next token 
                ++pb; //next token
                if (sb == se && pb == pe) //both stream ended its a match
                    return true;

                if (sb != se && pb == pe)
                    return false;

                if (sb == se && pb != pe)
                    {
                    while (pb != pe && *pb == "*") ++pb;
                    if (pb == pe)
                        return true;

                    return false;
                    }
                }
            }
        else
            return false;
        }

    return true;
    }

//======================================================================================>
//CustomAttributeValidator
//======================================================================================>
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
CustomAttributeValidator::RuleList const& CustomAttributeValidator::GetRelaventRules(ECPropertyValueChange& change) const
    {
    if (!change.IsDefinition())
        return m_rules.find(m_wilfCard)->second;

    Utf8String prefix = GetPrefix(change.GetAccessString());
    if (prefix.empty())
        return m_rules.find(m_wilfCard)->second;;

    auto itor = m_rules.find(prefix);
    if (itor != m_rules.end())
        return itor->second;

    return m_rules.find(m_wilfCard)->second;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String CustomAttributeValidator::GetPrefix(Utf8StringCR path)
    {
    size_t i = path.find(':');
    if (i == Utf8String::npos)
        return Utf8String();

    return path.substr(0, i);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
CustomAttributeValidator::Policy CustomAttributeValidator::Validate(ECPropertyValueChange& change) const
    {
    BeAssert(change.IsDefinition());
    std::vector<ECPropertyValueChange*> flatListOfChanges = change.GetFlatListOfChildren();
    RuleList const& rules = GetRelaventRules(change);
    if (rules.empty())
        return GetDefaultPolicy();

    for (ECPropertyValueChange* v : flatListOfChanges)
        {
        if (v->HasChildren())
            continue;

        std::vector<Utf8String> path = ECSchemaComparer::Split(v->GetAccessString());
        for (std::unique_ptr<Rule> const& rule : rules)
            {
            if (rule->Match(path))
                {
                if (rule->GetPolicy() == GetDefaultPolicy())
                    break; //test next item
                else
                    {
                    return GetDefaultPolicy() == Policy::Accept ? Policy::Reject : Policy::Accept;
                    }
                }
            }
        }

    return GetDefaultPolicy();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
CustomAttributeValidator::CustomAttributeValidator()
            :m_defaultPolicy(Policy::Accept), m_wilfCard("*")
            {
            Reset();
            }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
void CustomAttributeValidator::Reset()
    {
    m_rules.clear();
    m_rules[m_wilfCard] = RuleList();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
void CustomAttributeValidator::Accept(Utf8CP accessString)
    {
    Utf8String path = GetPrefix(Utf8String(accessString));
    if (path.empty())
        return;

    m_rules[path].push_back(std::unique_ptr<Rule>(new Rule(Policy::Accept, accessString)));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
bool CustomAttributeValidator::HasAnyRuleForSchema(Utf8CP schemaName) const
    {
    return m_rules.find(schemaName) != m_rules.end();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
void CustomAttributeValidator::Reject(Utf8CP accessString)
    {
    Utf8String path = GetPrefix(Utf8String(accessString));
    if (path.empty())
        return;

    m_rules[path].push_back(std::unique_ptr<Rule>(new Rule(Policy::Reject, accessString)));
    }


END_BENTLEY_SQLITE_EC_NAMESPACE
