/*--------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbSchemaEditor.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+-------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
USING_NAMESPACE_BENTLEY_EC
using namespace std;

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
template<typename T>
Nullable<T>::Nullable()
    :m_isNull(true), m_value(T())
    {}
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
template<typename T>
Nullable<T>::Nullable(T const& value)
    :m_isNull(false), m_value(value)
    {}
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
template<typename T>
Nullable<T>::Nullable(Nullable<T> const& rhs)
    :m_isNull(rhs.m_isNull), m_value(rhs.m_value)
    {}
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
template<typename T>
Nullable<T>::Nullable(Nullable<T> const&& rhs)
    :m_isNull(std::move(rhs.m_isNull)), m_value(std::move(rhs.m_value))
    {}
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
template<typename T>
bool Nullable<T>::IsNull() const { return m_isNull; }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
template<typename T>
T const& Nullable<T>::Value() const
    {
    //   BeAssert(!IsNull());
    return m_value;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
template<typename T>
T& Nullable<T>::ValueR() { return m_value; }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
template<typename T>
bool Nullable<T>::operator == (Nullable<T> const& rhs) const
    {
    if (rhs.IsNull() != IsNull())
        return false;
    else if (rhs.IsNull() && IsNull())
        return true;
    else
        return rhs.Value() == Value();
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
template<typename T>
bool Nullable<T>::operator == (nullptr_t rhs)const
    {
    return IsNull();
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
template<typename T>
bool Nullable<T>::operator != (Nullable<T> const& rhs) const
    {
    return !operator==(rhs);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
template<typename T>
bool Nullable<T>::operator != (nullptr_t rhs) const
    {
    return !operator==(rhs);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
template<typename T>
Nullable<T>& Nullable<T>::operator = (Nullable<T> const&& rhs)
    {
    if (&rhs != this)
        {
        m_value = std::move(rhs.m_value);
        m_isNull = std::move(rhs.m_isNull);
        }

    return *this;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
template<typename T>
Nullable<T>& Nullable<T>::operator = (Nullable<T> const& rhs)
    {
    if (&rhs != this)
        {
        m_value = rhs.m_value;
        m_isNull = rhs.m_isNull;
        }
    return *this;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
template<typename T>
Nullable<T>& Nullable<T>::operator = (T const& rhs)
    {
    m_value = rhs;
    m_isNull = false;

    return *this;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
template<typename T>
Nullable<T>& Nullable<T>::operator = (T const&& rhs)
    {
    m_value = std::move(rhs);
    m_isNull = false;

    return *this;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
template<typename T>
Nullable<T>& Nullable<T>::operator = (nullptr_t rhs)
    {
    m_value = T();
    m_isNull = true;
    return *this;
    }

/////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
bool Binary::_empty() const
    {
    return m_len == 0;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus Binary::_resize(size_t len)
    {
    if (len == 0)
        return _free();

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
BentleyStatus Binary::_free()
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
BentleyStatus Binary::_assign(void* buff , size_t len)
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
        {
        return _free();
        }
    else if (_resize(len) != SUCCESS)
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
Binary::Binary() :
    m_buff(nullptr), m_len(0)
    {}
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
        {
        _assign(rhs.m_buff, rhs.m_len);
        }
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
    else if (Size() == 0 && rhs.Size() != 0)
        return 1;
    else if (Size() != 0 && rhs.Size() == 0)
        return -1;
    else
        {
        if (Size() > rhs.Size())
            return -1;
        else if (Size() < rhs.Size())
            return 1;
        else
            {
            return memcmp(m_buff, rhs.m_buff, m_len);
            }
        }
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
void* Binary::GetPointerP()
    {
    return m_buff;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
void const* Binary::GetPointer() const
    {
    return m_buff;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
size_t Binary::Size() const
    {
    return m_len;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
bool Binary::Empty() const
    {
    return m_len == 0;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
bool Binary::operator == (Binary const& rhs) const
    {
    return Compare(rhs) == 0;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
bool Binary::Equalls(Binary const& rhs) const
    {
    return  *this == rhs;
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
        return _free();

    if (!value.IsBinary())
        return ERROR;

    size_t len = 0;
    void* buff = (void*) value.GetBinary(len);
    return _assign(buff, len);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
Binary::~Binary()
    {
    _free();
    }

///////////////////////////////////////////////////////////////////////////////////////


//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSchemaComparer::CompareECSchemas(ECSchemaChanges& changes, ECSchemaList const& a, ECSchemaList const& b)
    {
    std::map<Utf8CP, ECSchemaCP, CompareUtf8> aMap, bMap, cMap;
    for (ECSchemaCP schemaCP : a)
        aMap[schemaCP->GetName().c_str()] = schemaCP;

    for (ECSchemaCP schemaCP : b)
        bMap[schemaCP->GetName().c_str()] = schemaCP;

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
            if (CompareECSchema(classChange, *itorA->second, *itorB->second) == ERROR)
                return ERROR;
            }
        else if (existInA && !existInB)
            {
            if (AppendECSchema(changes, *itorA->second, ValueId::Deleted) == ERROR)
                return ERROR;
            }
        else if (!existInA && existInB)
            {
            if (AppendECSchema(changes, *itorB->second, ValueId::New) == ERROR)
                return ERROR;
            }
        }

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

    if (a.GetVersionWrite() != b.GetVersionWrite())
        change.GetVersionWrite().SetValue(a.GetVersionWrite(), b.GetVersionWrite());

    if (CompareECClasses(change.Classes(), a.GetClasses(), b.GetClasses()) != SUCCESS)
        return ERROR;

    if (CompareECEnumerations(change.Enumerations(), a.GetEnumerations(), b.GetEnumerations()) != SUCCESS)
        return ERROR;

    //if (CompareKindOfQuantities(change.KindOfQuantities(), a.GetKindOfQuantities(), b.GetKindOfQuantities()) != SUCCESS)
    //    return ERROR;

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

    if (a.IsCustomAttributeClass() != b.IsCustomAttributeClass())
        change.IsCustomAttributeClass().SetValue(a.IsCustomAttributeClass(), b.IsCustomAttributeClass());

    if (a.IsStructClass() != b.IsStructClass())
        change.IsStructClass().SetValue(a.IsStructClass(), b.IsStructClass());

    if (a.IsEntityClass() != b.IsEntityClass())
        change.IsEntityClass().SetValue(a.IsEntityClass(), b.IsEntityClass());

    if (a.IsRelationshipClass() != b.IsRelationshipClass())
        {
        change.IsRelationshipClass().SetValue(a.IsRelationshipClass(), b.IsRelationshipClass());
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
        if (a.IsRelationshipClass())
            if (AppendECRelationshipClass(change.GetRelationship(), *b.GetRelationshipClassCP(), ValueId::New) != SUCCESS)
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
        if (strcmp(a[i]->GetFullName(), b[i]->GetFullName()) == 0)
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
BentleyStatus ECSchemaComparer::CompareECRelationshipConstraintClassKeys(ECRelationshipConstraintClassChange& change, ECRelationshipConstraintClassCR const& a, ECRelationshipConstraintClassCR const& b)
    {
    std::set<Utf8CP, CompareUtf8> aMap, bMap, cMap;
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
    std::map<Utf8CP, ECRelationshipConstraintClassCP, CompareUtf8> aMap, bMap, cMap;
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
        change.GetTypeName().SetValue(a.GetDescription(), b.GetDescription());

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

    if (a.GetIsReadOnly() != b.GetIsReadOnly())
        change.IsReadonly().SetValue(a.GetIsReadOnly(), b.GetIsReadOnly());


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

        if (aArray->GetMaxOccurs() != bArray->GetMaxOccurs())
            change.GetArray().MaxOccurs().SetValue(aArray->GetMaxOccurs(), bArray->GetMaxOccurs());

        if (aArray->GetMinOccurs() != bArray->GetMinOccurs())
            change.GetArray().MinOccurs().SetValue(aArray->GetMinOccurs(), bArray->GetMinOccurs());
        }
    else if (aArray && !bArray)
        {
        change.GetArray().MaxOccurs().SetValue(ValueId::Deleted, aArray->GetMaxOccurs());
        change.GetArray().MinOccurs().SetValue(ValueId::Deleted, bArray->GetMinOccurs());
        }
    else if (!aArray && bArray)
        {
        change.GetArray().MaxOccurs().SetValue(ValueId::New, aArray->GetMaxOccurs());
        change.GetArray().MinOccurs().SetValue(ValueId::New, bArray->GetMinOccurs());
        }

    auto aExtendType = a.GetAsExtendedTypeProperty();
    auto bExtendType = b.GetAsExtendedTypeProperty();
    if (aExtendType && bExtendType)
        {
        if (aExtendType->GetExtendedTypeName() != bExtendType->GetExtendedTypeName())
            change.GetExtendedTypeName().SetValue(aExtendType->GetExtendedTypeName(), bExtendType->GetExtendedTypeName());
        }
    else if (aExtendType && !bExtendType)
        change.GetExtendedTypeName().SetValue(ValueId::Deleted, aExtendType->GetExtendedTypeName());
    else if (!aExtendType && bExtendType)
        change.GetExtendedTypeName().SetValue(ValueId::New, bExtendType->GetExtendedTypeName());

    return CompareCustomAttributes(change.CustomAttributes(), a, b);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSchemaComparer::CompareECProperties(ECPropertyChanges& changes, ECPropertyIterableCR a, ECPropertyIterableCR b)
    {
    std::map<Utf8CP, ECPropertyCP, CompareUtf8> aMap, bMap, cMap;
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
            auto& propertyChange = changes.Add(ChangeState::Modified);
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
    std::map<Utf8CP, ECClassCP, CompareUtf8> aMap, bMap, cMap;
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
    std::map<Utf8CP, ECEnumerationCP, CompareUtf8> aMap, bMap, cMap;
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
//    std::map<Utf8CP, KindOfQuantityCP, CompareUtf8> aMap, bMap, cMap;
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
BentleyStatus ECSchemaComparer::CompareCustomAttributes(ECInstanceChanges& changes, IECCustomAttributeContainerCR a, IECCustomAttributeContainerCR b)
    {
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
    std::map<Utf8CP, ECEnumeratorCP, CompareUtf8> aMap, bMap, cMap;
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
BentleyStatus ECSchemaComparer::CompareECInstance(ECInstanceChange& change, IECInstanceCR a, IECInstanceCR b)
    {
    return SUCCESS;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSchemaComparer::CompareBaseClasses(BaseClassChanges& changes, ECBaseClassesList const& a, ECBaseClassesList const& b)
    {
    std::set<Utf8CP, CompareUtf8> aMap, bMap, cMap;
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
    std::set<Utf8String> aMap, bMap, cMap;
    for (auto& ref : a)
        aMap.insert(ref.first.GetFullSchemaName());

    for (auto& ref : b)
        bMap.insert(ref.first.GetFullSchemaName());

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
    change.GetVersionWrite().SetValue(appendType, v.GetVersionWrite());

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
    change.IsCustomAttributeClass().SetValue(appendType, v.IsCustomAttributeClass());
    change.IsEntityClass().SetValue(appendType, v.IsEntityClass());
    change.IsStructClass().SetValue(appendType, v.IsStructClass());

    for (ECPropertyCP propertyCP : v.GetProperties(false))
        if (AppendECProperty(change.Properties(), *propertyCP, appendType) == ERROR)
            return ERROR;

    change.IsRelationshipClass().SetValue(appendType, v.GetRelationshipClassCP() != nullptr);
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
BentleyStatus ECSchemaComparer::AppendECRelationshipConstraintClass(ECRelationshipConstraintClassChange& change, ECRelationshipConstraintClassCR const& v, ValueId appendType)
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

    if (auto prop = v.GetAsExtendedTypeProperty())
        {
        propertyChange.GetExtendedTypeName().SetValue(appendType, prop->GetExtendedTypeName());
        }
    else if (auto prop = v.GetAsNavigationProperty())
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
        propertyChange.GetArray().MaxOccurs().SetValue(appendType, arrayProp->GetMaxOccurs());
        propertyChange.GetArray().MinOccurs().SetValue(appendType, arrayProp->GetMinOccurs());
        }

    return SUCCESS;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSchemaComparer::AppendECInstance(ECInstanceChange& changes, IECInstanceCR v, ValueId appendType)
    {
    std::map<Utf8String, ECValue> map;
    if (ConvertECInstanceToValueMap(map, v) != SUCCESS)
        return ERROR;

    if (map.empty())
        return SUCCESS;

    //for (auto& k : map)
    //    {
    //    k.first
    //    }
    return SUCCESS;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSchemaComparer::AppendCustomAttributes(ECInstanceChanges& changes, IECCustomAttributeContainerCR v, ValueId appendType)
    {
    ChangeState state = appendType == ValueId::New ? ChangeState::New : ChangeState::Deleted;
    for (auto& instance : v.GetPrimaryCustomAttributes(false))
        {
        ECInstanceChange& instanceChange = changes.Add(state, instance->GetClass().GetFullName());
        instanceChange.GetClassName().SetValue(appendType, instance->GetClass().GetFullName());
        if (AppendECInstance(instanceChange, *instance, appendType) != SUCCESS)
            return ERROR;
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
            b = i;
            }
        }

    if (b < --i)
        axis.push_back(path.substr(b, i - b));

    return axis;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSchemaComparer::Compare(ECSchemaChanges& changes, ECSchemaList const& existingSet, ECSchemaList const& newSet)
    {
    if (CompareECSchemas(changes, existingSet, newSet) != SUCCESS)
        return ERROR;

    Utf8String beforeOptimize, afterOptimize;
    changes.WriteToString(beforeOptimize);
    changes.Optimize();
    changes.WriteToString(afterOptimize);
    printf("%s", beforeOptimize.c_str());
    printf("%s", afterOptimize.c_str());

    return SUCCESS;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
