/*--------------------------------------------------------------------------------+
|
|     $Source: src/SchemaComparer.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+-------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"
#include <Bentley/Base64Utilities.h>
#include <ECObjects/SchemaComparer.h>
#include <set>

USING_NAMESPACE_BENTLEY_EC

#define NULL_TEXT "<null>"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

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
    if ((m_buff = realloc(m_buff, len)))
        {
        m_len = len;
        return SUCCESS;
        }

    m_buff=nullptr;
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
//=======================================================================================
// For case-insensitive UTF-8 string comparisons in STL collections that only use ASCII
// strings
// @bsistruct
//+===============+===============+===============+===============+===============+======
struct CompareIUtf8Ascii
    {
    bool operator()(Utf8CP s1, Utf8CP s2) const { return BeStringUtilities::StricmpAscii(s1, s2) < 0; }
    bool operator()(Utf8StringCR s1, Utf8StringCR s2) const { return BeStringUtilities::StricmpAscii(s1.c_str(), s2.c_str()) < 0; }
    bool operator()(Utf8StringCP s1, Utf8StringCP s2) const { BeAssert(s1 != nullptr && s2 != nullptr); return BeStringUtilities::StricmpAscii(s1->c_str(), s2->c_str()) < 0; }
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::Compare(SchemaChanges& changes, bvector<ECN::ECSchemaCP> const& lhs, bvector<ECN::ECSchemaCP> const& rhs, Options options)
    {
    m_options = options;
    std::map<Utf8CP, ECSchemaCP, CompareIUtf8Ascii> lhsMap, rhsMap, allSchemasMap;
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
            SchemaChange& schemaChange = changes.Add(ChangeState::Modified, schemaName);
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
BentleyStatus SchemaComparer::CompareECSchema(SchemaChange& change, ECSchemaCR a, ECSchemaCR b)
    {
    if (a.GetName() != b.GetName())
        change.GetName().SetValue(a.GetName(), b.GetName());

    if (a.GetIsDisplayLabelDefined() && !b.GetIsDisplayLabelDefined())
        change.GetDisplayLabel().SetValue(ValueId::Deleted, a.GetInvariantDisplayLabel());
    else if (!a.GetIsDisplayLabelDefined() && b.GetIsDisplayLabelDefined())
        change.GetDisplayLabel().SetValue(ValueId::New, b.GetInvariantDisplayLabel());
    else if (a.GetIsDisplayLabelDefined() && b.GetIsDisplayLabelDefined())
        {
        if (!a.GetInvariantDisplayLabel().EqualsIAscii(b.GetInvariantDisplayLabel()))
            change.GetDisplayLabel().SetValue(a.GetInvariantDisplayLabel(), b.GetInvariantDisplayLabel());
        }

    if (!a.GetInvariantDescription().EqualsIAscii(b.GetInvariantDescription()))
        change.GetDescription().SetValue(a.GetInvariantDescription(), b.GetInvariantDescription());

    if (a.GetAlias() != b.GetAlias())
        change.GetAlias().SetValue(a.GetAlias(), b.GetAlias());

    if (a.GetVersionRead() != b.GetVersionRead())
        change.GetVersionRead().SetValue(a.GetVersionRead(), b.GetVersionRead());

    if (a.GetVersionMinor() != b.GetVersionMinor())
        change.GetVersionMinor().SetValue(a.GetVersionMinor(), b.GetVersionMinor());

    if (a.GetVersionWrite() != b.GetVersionWrite())
        change.GetVersionWrite().SetValue(a.GetVersionWrite(), b.GetVersionWrite());

    if (a.GetECVersion() != b.GetECVersion())
        change.GetECVersion().SetValue((uint32_t) a.GetECVersion(), (uint32_t) b.GetECVersion());

    if (a.GetOriginalECXmlVersionMajor() != b.GetOriginalECXmlVersionMajor())
        change.GetOriginalECXmlVersionMajor().SetValue(a.GetOriginalECXmlVersionMajor(), b.GetOriginalECXmlVersionMajor());

    if (a.GetOriginalECXmlVersionMinor() != b.GetOriginalECXmlVersionMinor())
        change.GetOriginalECXmlVersionMinor().SetValue(a.GetOriginalECXmlVersionMinor(), b.GetOriginalECXmlVersionMinor());

    if (CompareECClasses(change.Classes(), a.GetClasses(), b.GetClasses()) != SUCCESS)
        return ERROR;

    if (CompareECEnumerations(change.Enumerations(), a.GetEnumerations(), b.GetEnumerations()) != SUCCESS)
        return ERROR;

    if (CompareKindOfQuantities(change.KindOfQuantities(), a.GetKindOfQuantities(), b.GetKindOfQuantities()) != SUCCESS)
        return ERROR;

    if (ComparePropertyCategories(change.PropertyCategories(), a.GetPropertyCategories(), b.GetPropertyCategories()) != SUCCESS)
        return ERROR;

    if (ComparePhenomena(change.Phenomena(), a.GetPhenomena(), b.GetPhenomena()) != SUCCESS)
        return ERROR;

    if (CompareUnitSystems(change.UnitSystems(), a.GetUnitSystems(), b.GetUnitSystems()) != SUCCESS)
        return ERROR;

    if (CompareUnits(change.Units(), a.GetUnits(), b.GetUnits()) != SUCCESS)
        return ERROR;

    if (CompareReferences(change.References(), a.GetReferencedSchemas(), b.GetReferencedSchemas()) != SUCCESS)
        return ERROR;

    return CompareCustomAttributes(change.CustomAttributes(), a, b);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::CompareECClass(ClassChange& change, ECClassCR a, ECClassCR b)
    {
    if (a.GetName() != b.GetName())
        change.GetName().SetValue(a.GetName(), b.GetName());

    if (a.GetIsDisplayLabelDefined() && !b.GetIsDisplayLabelDefined())
        change.GetDisplayLabel().SetValue(ValueId::Deleted, a.GetInvariantDisplayLabel());
    else if (!a.GetIsDisplayLabelDefined() && b.GetIsDisplayLabelDefined())
        change.GetDisplayLabel().SetValue(ValueId::New, b.GetInvariantDisplayLabel());
    else if (a.GetIsDisplayLabelDefined() && b.GetIsDisplayLabelDefined())
        {
        if (!a.GetInvariantDisplayLabel().EqualsIAscii(b.GetInvariantDisplayLabel()))
            change.GetDisplayLabel().SetValue(a.GetInvariantDisplayLabel(), b.GetInvariantDisplayLabel());
        }

    if (!a.GetInvariantDescription().EqualsIAscii(b.GetInvariantDescription()))
        change.GetDescription().SetValue(a.GetInvariantDescription(), b.GetInvariantDescription());

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
BentleyStatus SchemaComparer::CompareECBaseClasses(BaseClassChanges& changes, ECBaseClassesList const& a, ECBaseClassesList const& b)
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
BentleyStatus SchemaComparer::CompareECRelationshipClass(ECRelationshipChange& change, ECRelationshipClassCR a, ECRelationshipClassCR b)
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
BentleyStatus SchemaComparer::CompareECRelationshipConstraint(ECRelationshipConstraintChange& change, ECRelationshipConstraintCR a, ECRelationshipConstraintCR b)
    {
    if (a.GetRoleLabel() != b.GetRoleLabel())
        change.GetRoleLabel().SetValue(a.GetRoleLabel(), b.GetRoleLabel());

    if (a.GetIsPolymorphic() != b.GetIsPolymorphic())
        change.IsPolymorphic().SetValue(a.GetIsPolymorphic(), b.GetIsPolymorphic());

    if (a.GetMultiplicity().ToString() != b.GetMultiplicity().ToString())
        change.GetMultiplicity().SetValue(a.GetMultiplicity().ToString(), b.GetMultiplicity().ToString());

    return CompareECRelationshipConstraintClasses(change.ConstraintClasses(), a.GetConstraintClasses(), b.GetConstraintClasses());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::CompareECRelationshipConstraintClasses(ECRelationshipConstraintClassChanges& change, ECRelationshipConstraintClassList const& a, ECRelationshipConstraintClassList const& b)
    {
    std::map<Utf8CP, ECClassCP, CompareIUtf8Ascii> aMap, bMap, cMap;
    for (ECClassCP constraintClassCP : a)
        aMap[constraintClassCP->GetFullName()] = constraintClassCP;

    for (ECClassCP constraintClassCP : b)
        bMap[constraintClassCP->GetFullName()] = constraintClassCP;

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
            change.Add(ChangeState::Modified);
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
BentleyStatus SchemaComparer::CompareECProperty(ECPropertyChange& change, ECPropertyCR a, ECPropertyCR b)
    {
    if (a.GetTypeName() != b.GetTypeName())
        change.GetTypeName().SetValue(a.GetTypeName(), b.GetTypeName());

    if (a.GetName() != b.GetName())
        change.GetName().SetValue(a.GetName(), b.GetName());

    if (a.GetIsDisplayLabelDefined() && !b.GetIsDisplayLabelDefined())
        change.GetDisplayLabel().SetValue(ValueId::Deleted, a.GetInvariantDisplayLabel());
    else if (!a.GetIsDisplayLabelDefined() && b.GetIsDisplayLabelDefined())
        change.GetDisplayLabel().SetValue(ValueId::New, b.GetInvariantDisplayLabel());
    else if (a.GetIsDisplayLabelDefined() && b.GetIsDisplayLabelDefined())
        {
        if (!a.GetInvariantDisplayLabel().EqualsIAscii(b.GetInvariantDisplayLabel()))
            change.GetDisplayLabel().SetValue(a.GetInvariantDisplayLabel(), b.GetInvariantDisplayLabel());
        }

    if (!a.GetInvariantDescription().EqualsIAscii(b.GetInvariantDescription()))
        change.GetDescription().SetValue(a.GetInvariantDescription(), b.GetInvariantDescription());

    PrimitiveECPropertyCP aPrimProp = a.GetAsPrimitiveProperty();
    PrimitiveECPropertyCP bPrimProp = b.GetAsPrimitiveProperty();
    NavigationECPropertyCP aNavProp = a.GetAsNavigationProperty();
    NavigationECPropertyCP bNavProp = b.GetAsNavigationProperty();
    ArrayECPropertyCP aArrayProp = a.GetAsArrayProperty();
    ArrayECPropertyCP bArrayProp = b.GetAsArrayProperty();
    PrimitiveArrayECPropertyCP aPrimArrayProp = a.GetAsPrimitiveArrayProperty();
    PrimitiveArrayECPropertyCP bPrimArrayProp = b.GetAsPrimitiveArrayProperty();

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

    if (a.GetPriority() != b.GetPriority())
        change.GetPriority().SetValue(a.GetPriority(), b.GetPriority());

    // MinimumLength
    {
    const bool aMinLengthDefined = a.IsMinimumLengthDefined();
    const bool bMinLengthDefined = b.IsMinimumLengthDefined();
    if (aMinLengthDefined && bMinLengthDefined)
        {
        const uint32_t aMinLength = a.GetMinimumLength();
        const uint32_t bMinLength = b.GetMinimumLength();
        if (aMinLength != bMinLength)
            change.GetMinimumLength().SetValue(aMinLength, bMinLength);
        }
    else if (aMinLengthDefined && !bMinLengthDefined)
        change.GetMinimumLength().SetValue(ValueId::Deleted, a.GetMinimumLength());
    else if (!aMinLengthDefined && bMinLengthDefined)
        change.GetMinimumLength().SetValue(ValueId::New, b.GetMinimumLength());
    }

    // MaximumLength
    {
    const bool aMaxLengthDefined = a.IsMaximumLengthDefined();
    const bool bMaxLengthDefined = b.IsMaximumLengthDefined();
    if (aMaxLengthDefined && bMaxLengthDefined)
        {
        const uint32_t aMaxLength = a.GetMaximumLength();
        const uint32_t bMaxLength = b.GetMaximumLength();
        if (aMaxLength != bMaxLength)
            change.GetMaximumLength().SetValue(aMaxLength, bMaxLength);
        }
    else if (aMaxLengthDefined && !bMaxLengthDefined)
        change.GetMaximumLength().SetValue(ValueId::Deleted, a.GetMaximumLength());
    else if (!aMaxLengthDefined && bMaxLengthDefined)
        change.GetMaximumLength().SetValue(ValueId::New, b.GetMaximumLength());
    }

    // MinimumValue
    {
    const bool aMinValueDefined = a.IsMinimumValueDefined();
    const bool bMinValueDefined = b.IsMinimumValueDefined();
    if (aMinValueDefined && bMinValueDefined)
        {
        ECValue aVal, bVal;
        if (ECObjectsStatus::Success != a.GetMinimumValue(aVal) ||
            ECObjectsStatus::Success != b.GetMinimumValue(bVal))
            return ERROR;

        if (aVal != bVal)
            change.GetMinimumValue().SetValue(aVal, bVal);
        }
    else if (aMinValueDefined && !bMinValueDefined)
        {
        ECValue aVal;
        if (ECObjectsStatus::Success != a.GetMinimumValue(aVal))
            return ERROR;

        change.GetMinimumValue().SetValue(ValueId::Deleted, aVal);
        }
    else if (!aMinValueDefined && bMinValueDefined)
        {
        ECValue bVal;
        if (ECObjectsStatus::Success != b.GetMinimumValue(bVal))
            return ERROR;

        change.GetMinimumValue().SetValue(ValueId::New, bVal);
        }
    }

    // MaximumValue
    {
    const bool aMaxValueDefined = a.IsMaximumValueDefined();
    const bool bMaxValueDefined = b.IsMaximumValueDefined();
    if (aMaxValueDefined && bMaxValueDefined)
        {
        ECValue aVal, bVal;
        if (ECObjectsStatus::Success != a.GetMaximumValue(aVal) ||
            ECObjectsStatus::Success != b.GetMaximumValue(bVal))
            return ERROR;

        if (aVal != bVal)
            change.GetMaximumValue().SetValue(aVal, bVal);
        }
    else if (aMaxValueDefined && !bMaxValueDefined)
        {
        ECValue aVal;
        if (ECObjectsStatus::Success != a.GetMaximumValue(aVal))
            return ERROR;

        change.GetMaximumValue().SetValue(ValueId::Deleted, aVal);
        }
    else if (!aMaxValueDefined && bMaxValueDefined)
        {
        ECValue bVal;
        if (ECObjectsStatus::Success != b.GetMaximumValue(bVal))
            return ERROR;

        change.GetMaximumValue().SetValue(ValueId::New, bVal);
        }
    }

    // KOQ
    KindOfQuantityCP aKoq = a.GetKindOfQuantity();
    KindOfQuantityCP bKoq = b.GetKindOfQuantity();
    if (aKoq != nullptr && bKoq != nullptr)
        {
        if (aKoq != bKoq)
            change.GetKindOfQuantity().SetValue(aKoq->GetFullName(), bKoq->GetFullName());
        }
    else if (aKoq != nullptr && bKoq == nullptr)
        change.GetKindOfQuantity().SetValue(ValueId::Deleted, aKoq->GetFullName());
    else if (aKoq == nullptr && bKoq != nullptr)
        change.GetKindOfQuantity().SetValue(ValueId::New, bKoq->GetFullName());

    // PropertyCategory
    PropertyCategoryCP aCat = a.GetCategory();
    PropertyCategoryCP bCat = b.GetCategory();
    if (aCat != nullptr && bCat != nullptr)
        {
        if (aCat != bCat)
            change.GetCategory().SetValue(aCat->GetFullName(), bCat->GetFullName());
        }
    else if (aCat != nullptr && bCat == nullptr)
        change.GetCategory().SetValue(ValueId::Deleted, aCat->GetFullName());
    else if (aCat == nullptr && bCat != nullptr)
        change.GetCategory().SetValue(ValueId::New, bCat->GetFullName());

    //ECEnumeration
    ECEnumerationCP aEnum = nullptr, bEnum = nullptr;
    if (aPrimProp != nullptr)
        aEnum = aPrimProp->GetEnumeration();
    else if (aPrimArrayProp != nullptr)
        aEnum = aPrimArrayProp->GetEnumeration();

    if (bPrimProp != nullptr)
        bEnum = bPrimProp->GetEnumeration();
    else if (bPrimArrayProp != nullptr)
        bEnum = bPrimArrayProp->GetEnumeration();

    if (aEnum != bEnum)
        {
        if (aEnum != nullptr && bEnum == nullptr)
            change.GetEnumeration().SetValue(ValueId::Deleted, aEnum->GetFullName());
        else if (aEnum == nullptr && bEnum != nullptr)
            change.GetEnumeration().SetValue(ValueId::New, bEnum->GetFullName());
        else
            change.GetEnumeration().SetValue(aEnum->GetFullName(), bEnum->GetFullName());
        }

    //ExtendedType
    Utf8StringCP aExtendedType = nullptr, bExtendedType = nullptr;
    if (a.HasExtendedType())
        {
        if (aPrimProp != nullptr)
            aExtendedType = &aPrimProp->GetExtendedTypeName();
        else if (aPrimArrayProp != nullptr)
            aExtendedType = &aPrimArrayProp->GetExtendedTypeName();
        else
            {
            BeAssert(false && "Property type which is not expected to have an extended type name. Code needs to be adjusted");
            return ERROR;
            }
        }

    if (b.HasExtendedType())
        {
        if (bPrimProp != nullptr)
            bExtendedType = &bPrimProp->GetExtendedTypeName();
        else if (bPrimArrayProp != nullptr)
            bExtendedType = &bPrimArrayProp->GetExtendedTypeName();
        else
            {
            BeAssert(false && "Property type which is not expected to have an extended type name. Code needs to be adjusted");
            return ERROR;
            }
        }

    if (aExtendedType != nullptr && bExtendedType != nullptr)
        {
        if (!aExtendedType->EqualsIAscii(*bExtendedType))
            change.GetExtendedTypeName().SetValue(*aExtendedType, *bExtendedType);
        }
    else if (aExtendedType != nullptr && bExtendedType == nullptr)
        change.GetExtendedTypeName().SetValue(ValueId::Deleted, *aExtendedType);
    else if (aExtendedType == nullptr && bExtendedType != nullptr)
        change.GetExtendedTypeName().SetValue(ValueId::New, *bExtendedType);

    // Nav prop
    if (aNavProp != nullptr && bNavProp != nullptr)
        {
        if (aNavProp->GetDirection() != bNavProp->GetDirection())
            change.GetNavigation().Direction().SetValue(aNavProp->GetDirection(), bNavProp->GetDirection());

        if (aNavProp->GetRelationshipClass() != nullptr && bNavProp->GetRelationshipClass() != nullptr)
            change.GetNavigation().GetRelationshipClassName().SetValue(Utf8String(aNavProp->GetRelationshipClass()->GetFullName()),
                                                                       Utf8String(bNavProp->GetRelationshipClass()->GetFullName()));
        else if (aNavProp->GetRelationshipClass() == nullptr && bNavProp->GetRelationshipClass() == nullptr)
            change.GetNavigation().GetRelationshipClassName().SetValue(ValueId::Deleted, aNavProp->GetRelationshipClass()->GetFullName());
        else if (aNavProp->GetRelationshipClass() == nullptr && bNavProp->GetRelationshipClass() != nullptr)
            change.GetNavigation().GetRelationshipClassName().SetValue(ValueId::New, bNavProp->GetRelationshipClass()->GetFullName());
        }
    else if (aNavProp != nullptr && bNavProp == nullptr)
        {
        change.GetNavigation().Direction().SetValue(ValueId::Deleted, aNavProp->GetDirection());
        change.GetNavigation().GetRelationshipClassName().SetValue(ValueId::Deleted, aNavProp->GetRelationshipClass()->GetFullName());
        }
    else if(aNavProp == nullptr && bNavProp != nullptr)
        {
        change.GetNavigation().Direction().SetValue(ValueId::New, bNavProp->GetDirection());
        change.GetNavigation().GetRelationshipClassName().SetValue(ValueId::New, bNavProp->GetRelationshipClass()->GetFullName());
        }
   
    
    //Array
    if (aArrayProp != nullptr && bArrayProp != nullptr)
        {
        if (aArrayProp->GetStoredMaxOccurs() != bArrayProp->GetStoredMaxOccurs())
            change.GetArray().MaxOccurs().SetValue(aArrayProp->GetStoredMaxOccurs(), bArrayProp->GetStoredMaxOccurs());

        if (aArrayProp->GetMinOccurs() != bArrayProp->GetMinOccurs())
            change.GetArray().MinOccurs().SetValue(aArrayProp->GetMinOccurs(), bArrayProp->GetMinOccurs());
        }
    else if (aArrayProp != nullptr && bArrayProp == nullptr)
        {
        change.GetArray().MaxOccurs().SetValue(ValueId::Deleted, aArrayProp->GetStoredMaxOccurs());
        change.GetArray().MinOccurs().SetValue(ValueId::Deleted, aArrayProp->GetMinOccurs());
        }
    else if (aArrayProp == nullptr && bArrayProp != nullptr)
        {
        change.GetArray().MaxOccurs().SetValue(ValueId::New, bArrayProp->GetStoredMaxOccurs());
        change.GetArray().MinOccurs().SetValue(ValueId::New, bArrayProp->GetMinOccurs());
        }

    return CompareCustomAttributes(change.CustomAttributes(), a, b);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::CompareECProperties(ECPropertyChanges& changes, ECPropertyIterableCR a, ECPropertyIterableCR b)
    {
    std::map<Utf8CP, ECPropertyCP, CompareIUtf8Ascii> aMap, bMap, cMap;
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
BentleyStatus SchemaComparer::CompareECClasses(ClassChanges& changes, ECClassContainerCR a, ECClassContainerCR b)
    {
    std::map<Utf8CP, ECClassCP, CompareIUtf8Ascii> aMap, bMap, cMap;
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
BentleyStatus SchemaComparer::CompareECEnumerations(ECEnumerationChanges& changes, ECEnumerationContainerCR a, ECEnumerationContainerCR b)
    {
    std::map<Utf8CP, ECEnumerationCP, CompareIUtf8Ascii> aMap, bMap, cMap;
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
BentleyStatus SchemaComparer::CompareKindOfQuantities(KindOfQuantityChanges& changes, KindOfQuantityContainerCR a, KindOfQuantityContainerCR b)
    {
    std::map<Utf8CP, KindOfQuantityCP, CompareIUtf8Ascii> aMap, bMap, cMap;
    for (KindOfQuantityCP enumCP : a)
        aMap[enumCP->GetName().c_str()] = enumCP;

    for (KindOfQuantityCP enumCP : b)
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
            auto& kindOfQuantityChange = changes.Add(ChangeState::Modified, u.first);
            if (CompareKindOfQuantity(kindOfQuantityChange, *itorA->second, *itorB->second) == ERROR)
                return ERROR;
            }
        else if (existInA && !existInB)
            {
            if (AppendKindOfQuantity(changes, *itorA->second, ValueId::Deleted) == ERROR)
                return ERROR;
            }
        else if (!existInA && existInB)
            {
            if (AppendKindOfQuantity(changes, *itorB->second, ValueId::New) == ERROR)
                return ERROR;
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle  06/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::ComparePropertyCategories(PropertyCategoryChanges& changes, PropertyCategoryContainerCR a, PropertyCategoryContainerCR b)
    {
    std::map<Utf8CP, PropertyCategoryCP, CompareIUtf8Ascii> oldMap, newMap, allMap;
    for (PropertyCategoryCP catCP : a)
        oldMap[catCP->GetName().c_str()] = catCP;

    for (PropertyCategoryCP catCP : b)
        newMap[catCP->GetName().c_str()] = catCP;

    allMap.insert(oldMap.cbegin(), oldMap.cend());
    allMap.insert(newMap.cbegin(), newMap.cend());

    for (auto& kvPair : allMap)
        {
        Utf8CP name = kvPair.first;
        auto oldIt = oldMap.find(name);
        auto newIt = newMap.find(name);

        const bool existInOld = oldIt != oldMap.end();
        const bool existInNew = newIt != newMap.end();
        if (existInOld && existInNew)
            {
            PropertyCategoryChange& catChange = changes.Add(ChangeState::Modified, name);
            if (ComparePropertyCategory(catChange, *oldIt->second, *newIt->second) == ERROR)
                return ERROR;

            continue;
            }
        
        if (existInOld && !existInNew)
            {
            if (AppendPropertyCategory(changes, *oldIt->second, ValueId::Deleted) == ERROR)
                return ERROR;

            continue;
            }

        if (!existInOld && existInNew)
            {
            if (AppendPropertyCategory(changes, *newIt->second, ValueId::New) == ERROR)
                return ERROR;
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  02/2018
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::ComparePhenomena(PhenomenonChanges& changes, PhenomenonContainerCR oldValues, PhenomenonContainerCR newValues)
    {
    std::map<Utf8CP, PhenomenonCP, CompareIUtf8Ascii> oldMap, newMap, allMap;
    for (PhenomenonCP ph : oldValues)
        oldMap[ph->GetName().c_str()] = ph;

    for (PhenomenonCP ph : newValues)
        newMap[ph->GetName().c_str()] = ph;

    allMap.insert(oldMap.cbegin(), oldMap.cend());
    allMap.insert(newMap.cbegin(), newMap.cend());

    for (auto& kvPair : allMap)
        {
        Utf8CP name = kvPair.first;
        auto oldIt = oldMap.find(name);
        auto newIt = newMap.find(name);

        const bool existInOld = oldIt != oldMap.end();
        const bool existInNew = newIt != newMap.end();
        if (existInOld && existInNew)
            {
            PhenomenonChange& change = changes.Add(ChangeState::Modified, name);
            if (SUCCESS != ComparePhenomenon(change, *oldIt->second, *newIt->second))
                return ERROR;

            continue;
            }

        if (existInOld && !existInNew)
            {
            if (SUCCESS != AppendPhenomenon(changes, *oldIt->second, ValueId::Deleted))
                return ERROR;

            continue;
            }

        if (!existInOld && existInNew)
            {
            if (SUCCESS != AppendPhenomenon(changes, *newIt->second, ValueId::New))
                return ERROR;
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  02/2018
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::CompareUnitSystems(UnitSystemChanges& changes, UnitSystemContainerCR oldValues, UnitSystemContainerCR newValues)
    {
    std::map<Utf8CP, UnitSystemCP, CompareIUtf8Ascii> oldMap, newMap, allMap;
    for (UnitSystemCP ph : oldValues)
        oldMap[ph->GetName().c_str()] = ph;

    for (UnitSystemCP ph : newValues)
        newMap[ph->GetName().c_str()] = ph;

    allMap.insert(oldMap.cbegin(), oldMap.cend());
    allMap.insert(newMap.cbegin(), newMap.cend());

    for (auto& kvPair : allMap)
        {
        Utf8CP name = kvPair.first;
        auto oldIt = oldMap.find(name);
        auto newIt = newMap.find(name);

        const bool existInOld = oldIt != oldMap.end();
        const bool existInNew = newIt != newMap.end();
        if (existInOld && existInNew)
            {
            UnitSystemChange& change = changes.Add(ChangeState::Modified, name);
            if (SUCCESS != CompareUnitSystem(change, *oldIt->second, *newIt->second))
                return ERROR;

            continue;
            }

        if (existInOld && !existInNew)
            {
            if (SUCCESS != AppendUnitSystem(changes, *oldIt->second, ValueId::Deleted))
                return ERROR;

            continue;
            }

        if (!existInOld && existInNew)
            {
            if (SUCCESS != AppendUnitSystem(changes, *newIt->second, ValueId::New))
                return ERROR;
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  02/2018
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::CompareUnits(UnitChanges& changes, UnitContainerCR oldValues, UnitContainerCR newValues)
    {
    std::map<Utf8CP, ECUnitCP, CompareIUtf8Ascii> oldMap, newMap, allMap;
    for (ECUnitCP ph : oldValues)
        oldMap[ph->GetName().c_str()] = ph;

    for (ECUnitCP ph : newValues)
        newMap[ph->GetName().c_str()] = ph;

    allMap.insert(oldMap.cbegin(), oldMap.cend());
    allMap.insert(newMap.cbegin(), newMap.cend());

    for (auto& kvPair : allMap)
        {
        Utf8CP name = kvPair.first;
        auto oldIt = oldMap.find(name);
        auto newIt = newMap.find(name);

        const bool existInOld = oldIt != oldMap.end();
        const bool existInNew = newIt != newMap.end();
        if (existInOld && existInNew)
            {
            UnitChange& change = changes.Add(ChangeState::Modified, name);
            if (SUCCESS != CompareUnit(change, *oldIt->second, *newIt->second))
                return ERROR;

            continue;
            }

        if (existInOld && !existInNew)
            {
            if (SUCCESS != AppendUnit(changes, *oldIt->second, ValueId::Deleted))
                return ERROR;

            continue;
            }

        if (!existInOld && existInNew)
            {
            if (SUCCESS != AppendUnit(changes, *newIt->second, ValueId::New))
                return ERROR;
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::CompareCustomAttribute(ECPropertyValueChange& changes, IECInstanceCR a, IECInstanceCR b)
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
BentleyStatus SchemaComparer::AppendCustomAttribute(ECInstanceChanges& changes, IECInstanceCR v, ValueId appendType)
    {
    ChangeState state = appendType == ValueId::New ? ChangeState::New : ChangeState::Deleted;
    std::map<Utf8String, ECValue> map;
    if (ConvertECInstanceToValueMap(map, v) != SUCCESS)
        return ERROR;

    auto& change = changes.Add(state, v.GetClass().GetFullName());
    for (auto& key : map)
        change.GetOrCreate(state, Split(key.first)).SetValue(appendType, key.second);

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::AppendCustomAttributes(ECInstanceChanges& changes, IECCustomAttributeContainerCR v, ValueId appendType)
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
BentleyStatus SchemaComparer::CompareCustomAttributes(ECInstanceChanges& changes, IECCustomAttributeContainerCR a, IECCustomAttributeContainerCR b)
    {
    std::map<Utf8CP, IECInstanceCP, CompareIUtf8Ascii> aMap, bMap, cMap;
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
BentleyStatus SchemaComparer::CompareECEnumeration(ECEnumerationChange& change, ECEnumerationCR oldVal, ECEnumerationCR newVal)
    {
    if (!oldVal.GetName().EqualsIAscii(newVal.GetName()))
        change.GetName().SetValue(oldVal.GetName(), newVal.GetName());

    if (oldVal.GetIsDisplayLabelDefined() && !newVal.GetIsDisplayLabelDefined())
        change.GetDisplayLabel().SetValue(ValueId::Deleted, oldVal.GetInvariantDisplayLabel());
    else if (!oldVal.GetIsDisplayLabelDefined() && newVal.GetIsDisplayLabelDefined())
        change.GetDisplayLabel().SetValue(ValueId::New, newVal.GetInvariantDisplayLabel());
    else if (oldVal.GetIsDisplayLabelDefined() && newVal.GetIsDisplayLabelDefined())
        {
        if (!oldVal.GetInvariantDisplayLabel().EqualsIAscii(newVal.GetInvariantDisplayLabel()))
            change.GetDisplayLabel().SetValue(oldVal.GetInvariantDisplayLabel(), newVal.GetInvariantDisplayLabel());
        }

    if (!oldVal.GetInvariantDescription().EqualsIAscii(newVal.GetInvariantDescription()))
        change.GetDescription().SetValue(oldVal.GetInvariantDescription(), newVal.GetInvariantDescription());

    if (oldVal.GetIsStrict() != newVal.GetIsStrict())
        change.IsStrict().SetValue(oldVal.GetIsStrict(), newVal.GetIsStrict());

    if (oldVal.GetType() == newVal.GetType())
        return CompareECEnumerators(change.Enumerators(), oldVal.GetEnumerators(), newVal.GetEnumerators());

    change.GetTypeName().SetValue(oldVal.GetTypeName(), newVal.GetTypeName());
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::CompareECEnumerators(ECEnumeratorChanges& changes, EnumeratorIterable const& oldValues, EnumeratorIterable const& newValues)
    {
    std::map<Utf8CP, ECEnumeratorCP, CompareIUtf8Ascii> oldEnumValues, newEnumValues, allEnumValues;
    for (ECEnumeratorCP ecenum : oldValues)
        oldEnumValues[ecenum->GetName().c_str()] = ecenum;

    for (ECEnumeratorCP ecenum : newValues)
        newEnumValues[ecenum->GetName().c_str()] = ecenum;


    allEnumValues.insert(oldEnumValues.cbegin(), oldEnumValues.cend());
    allEnumValues.insert(newEnumValues.cbegin(), newEnumValues.cend());

    for (std::pair<Utf8CP, ECEnumeratorCP> const& kvPair : allEnumValues)
        {
        Utf8CP enumeratorName = kvPair.first;
        auto oldIt = oldEnumValues.find(enumeratorName);
        auto newIt = newEnumValues.find(enumeratorName);

        bool existsInOld = oldIt != oldEnumValues.end();
        ECEnumeratorCP oldEnumerator = existsInOld ? oldIt->second : nullptr;
        bool existsInNew = newIt != newEnumValues.end();
        ECEnumeratorCP newEnumerator = existsInNew ? newIt->second : nullptr;

        if (existsInOld && existsInNew)
            {
            ECEnumeratorChange& enumeratorChange = changes.Add(ChangeState::Modified, enumeratorName);
            if (oldEnumerator->IsInteger())
                {
                if (oldEnumerator->GetInteger() != newEnumerator->GetInteger())
                    enumeratorChange.GetInteger().SetValue(oldEnumerator->GetInteger(), newEnumerator->GetInteger());
                }

            if (oldIt->second->IsString())
                {
                if (oldIt->second->GetString() != newEnumerator->GetString())
                    enumeratorChange.GetString().SetValue(oldEnumerator->GetString(), newEnumerator->GetString());
                }

            const bool displayLabelDefinedInOld = oldEnumerator->GetIsDisplayLabelDefined();
            const bool displayLabelDefinedInNew = newEnumerator->GetIsDisplayLabelDefined();
            if (displayLabelDefinedInOld && !displayLabelDefinedInNew)
                enumeratorChange.GetDisplayLabel().SetValue(ValueId::Deleted, oldEnumerator->GetInvariantDisplayLabel());
            else if (!displayLabelDefinedInOld && displayLabelDefinedInNew)
                enumeratorChange.GetDisplayLabel().SetValue(ValueId::New, newEnumerator->GetInvariantDisplayLabel());
            else if (displayLabelDefinedInOld && displayLabelDefinedInNew)
                {
                if (!oldEnumerator->GetInvariantDisplayLabel().EqualsIAscii(newEnumerator->GetInvariantDisplayLabel()))
                    enumeratorChange.GetDisplayLabel().SetValue(oldEnumerator->GetInvariantDisplayLabel(), newEnumerator->GetInvariantDisplayLabel());
                }

            if (!oldIt->second->GetInvariantDescription().EqualsIAscii(newEnumerator->GetInvariantDescription()))
                enumeratorChange.GetDescription().SetValue(oldEnumerator->GetInvariantDescription(), newEnumerator->GetInvariantDescription());

            }
        else if (existsInOld && !existsInNew)
            {
            auto& change = changes.Add(ChangeState::Deleted, enumeratorName);
            change.GetName().SetValue(ValueId::Deleted, oldIt->second->GetName());
            change.GetDisplayLabel().SetValue(ValueId::Deleted, oldIt->second->GetInvariantDisplayLabel());
            change.GetDescription().SetValue(ValueId::Deleted, oldIt->second->GetInvariantDescription());
            if (oldEnumerator->IsInteger())
                change.GetInteger().SetValue(ValueId::Deleted, oldIt->second->GetInteger());
            else
                change.GetString().SetValue(ValueId::Deleted, oldIt->second->GetString());
            }
        else if (!existsInOld && existsInNew)
            {
            auto& change = changes.Add(ChangeState::New, enumeratorName);
            change.GetName().SetValue(ValueId::New, newEnumerator->GetName());
            change.GetDisplayLabel().SetValue(ValueId::New, newEnumerator->GetInvariantDisplayLabel());
            change.GetDescription().SetValue(ValueId::New, newEnumerator->GetInvariantDescription());
            if (newEnumerator->IsInteger())
                change.GetInteger().SetValue(ValueId::New, newEnumerator->GetInteger());
            else
                change.GetString().SetValue(ValueId::New, newEnumerator->GetString());
            }
        }

    return SUCCESS;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::CompareKindOfQuantity(KindOfQuantityChange& change, KindOfQuantityCR oldValue, KindOfQuantityCR newValue)
    {
    if (oldValue.GetName() != newValue.GetName())
        change.GetName().SetValue(oldValue.GetName(), newValue.GetName());

    const bool displayLabelDefinedInOld = oldValue.GetIsDisplayLabelDefined();
    const bool displayLabelDefinedInNew = newValue.GetIsDisplayLabelDefined();
    if (displayLabelDefinedInOld && !displayLabelDefinedInNew)
        change.GetDisplayLabel().SetValue(ValueId::Deleted, oldValue.GetInvariantDisplayLabel());
    else if (!displayLabelDefinedInOld && displayLabelDefinedInNew)
        change.GetDisplayLabel().SetValue(ValueId::New, newValue.GetInvariantDisplayLabel());
    else if (displayLabelDefinedInOld && displayLabelDefinedInNew)
        {
        if (!oldValue.GetInvariantDisplayLabel().EqualsIAscii(newValue.GetInvariantDisplayLabel()))
            change.GetDisplayLabel().SetValue(oldValue.GetInvariantDisplayLabel(), newValue.GetInvariantDisplayLabel());
        }

    if (oldValue.GetDescription() != newValue.GetDescription())
        change.GetDescription().SetValue(oldValue.GetDescription(), newValue.GetDescription());

    auto qualifiedToText = [](ECUnitCR unit, Formatting::NamedFormatSpecCR format) -> Utf8String 
        {
        Utf8String ret;
        ret += unit.GetFullName();
        ret += "(";
        ret += format.GetName();
        ret += ")";
        return ret;
        };

    Utf8String oldPersUnitStr = qualifiedToText(*(ECUnitCP)oldValue.GetPersistenceUnit().GetUnit(), *oldValue.GetPersistenceUnit().GetNamedFormatSpec());

    Utf8String newPersUnitStr = qualifiedToText(*(ECUnitCP)newValue.GetPersistenceUnit().GetUnit(), *newValue.GetPersistenceUnit().GetNamedFormatSpec());
    
    if (!oldPersUnitStr.Equals(newPersUnitStr))
        change.GetPersistenceUnit().SetValue(oldPersUnitStr, newPersUnitStr);

    if (oldValue.GetRelativeError() != newValue.GetRelativeError())
        change.GetRelativeError().SetValue(oldValue.GetRelativeError(), newValue.GetRelativeError());

    bset<Utf8String> oldPresUnits, newPresUnits, allPresUnits;
    for (Formatting::FormatUnitSet const& fus : oldValue.GetPresentationUnitList())
        {
        oldPresUnits.insert(qualifiedToText(*(ECUnitCP)fus.GetUnit(), *fus.GetNamedFormatSpec()));
        }

    for (Formatting::FormatUnitSet const& fus : newValue.GetPresentationUnitList())
        {
        newPresUnits.insert(qualifiedToText(*(ECUnitCP)fus.GetUnit(), *fus.GetNamedFormatSpec()));
        }

    allPresUnits.insert(oldPresUnits.begin(), oldPresUnits.end());
    allPresUnits.insert(newPresUnits.begin(), newPresUnits.end());

    for (Utf8StringCR presUnit : allPresUnits)
        {
        auto oldIt = oldPresUnits.find(presUnit);
        auto newIt = newPresUnits.find(presUnit);

        bool existsInOld = oldIt != oldPresUnits.end();
        bool existsInNew = newIt != newPresUnits.end();
        if (existsInOld && !existsInNew)
            change.GetPresentationUnitList().Add(ChangeState::Deleted).SetValue(ValueId::Deleted, *oldIt);
        else if (!existsInOld && existsInNew)
            change.GetPresentationUnitList().Add(ChangeState::New).SetValue(ValueId::New, *newIt);
        }
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle  06/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::ComparePropertyCategory(PropertyCategoryChange& change, PropertyCategoryCR oldValue, PropertyCategoryCR newValue)
    {
    if (!oldValue.GetName().EqualsIAscii(newValue.GetName()))
        change.GetName().SetValue(oldValue.GetName(), newValue.GetName());

    const bool displayLabelDefinedInOld = oldValue.GetIsDisplayLabelDefined();
    const bool displayLabelDefinedInNew = newValue.GetIsDisplayLabelDefined();
    if (displayLabelDefinedInOld && !displayLabelDefinedInNew)
        change.GetDisplayLabel().SetValue(ValueId::Deleted, oldValue.GetInvariantDisplayLabel());
    else if (!displayLabelDefinedInOld && displayLabelDefinedInNew)
        change.GetDisplayLabel().SetValue(ValueId::New, newValue.GetInvariantDisplayLabel());
    else if (displayLabelDefinedInOld && displayLabelDefinedInNew)
        {
        if (!oldValue.GetInvariantDisplayLabel().EqualsIAscii(newValue.GetInvariantDisplayLabel()))
            change.GetDisplayLabel().SetValue(oldValue.GetInvariantDisplayLabel(), newValue.GetInvariantDisplayLabel());
        }

    if (oldValue.GetDescription() != newValue.GetDescription())
        change.GetDescription().SetValue(oldValue.GetDescription(), newValue.GetDescription());

    if (oldValue.GetPriority() != newValue.GetPriority())
        change.GetPriority().SetValue(oldValue.GetPriority(), newValue.GetPriority());

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle  02/2018
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::ComparePhenomenon(PhenomenonChange& change, PhenomenonCR oldVal, PhenomenonCR newVal)
    {
    if (!oldVal.GetName().EqualsIAscii(newVal.GetName()))
        change.GetName().SetValue(oldVal.GetName(), newVal.GetName());

    if (oldVal.GetIsDisplayLabelDefined() && !newVal.GetIsDisplayLabelDefined())
        change.GetDisplayLabel().SetValue(ValueId::Deleted, oldVal.GetInvariantDisplayLabel());
    else if (!oldVal.GetIsDisplayLabelDefined() && newVal.GetIsDisplayLabelDefined())
        change.GetDisplayLabel().SetValue(ValueId::New, newVal.GetInvariantDisplayLabel());
    else if (oldVal.GetIsDisplayLabelDefined() && newVal.GetIsDisplayLabelDefined())
        {
        if (!oldVal.GetInvariantDisplayLabel().EqualsIAscii(newVal.GetInvariantDisplayLabel()))
            change.GetDisplayLabel().SetValue(oldVal.GetInvariantDisplayLabel(), newVal.GetInvariantDisplayLabel());
        }

    if (!oldVal.GetInvariantDescription().EqualsIAscii(newVal.GetInvariantDescription()))
        change.GetDescription().SetValue(oldVal.GetInvariantDescription(), newVal.GetInvariantDescription());

    if (!oldVal.GetDefinition().EqualsIAscii(newVal.GetDefinition()))
        change.GetDefinition().SetValue(oldVal.GetDefinition(), newVal.GetDefinition());

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle  02/2018
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::CompareUnitSystem(UnitSystemChange& change, UnitSystemCR oldVal, UnitSystemCR newVal)
    {
    if (!oldVal.GetName().EqualsIAscii(newVal.GetName()))
        change.GetName().SetValue(oldVal.GetName(), newVal.GetName());

    if (oldVal.GetIsDisplayLabelDefined() && !newVal.GetIsDisplayLabelDefined())
        change.GetDisplayLabel().SetValue(ValueId::Deleted, oldVal.GetInvariantDisplayLabel());
    else if (!oldVal.GetIsDisplayLabelDefined() && newVal.GetIsDisplayLabelDefined())
        change.GetDisplayLabel().SetValue(ValueId::New, newVal.GetInvariantDisplayLabel());
    else if (oldVal.GetIsDisplayLabelDefined() && newVal.GetIsDisplayLabelDefined())
        {
        if (!oldVal.GetInvariantDisplayLabel().EqualsIAscii(newVal.GetInvariantDisplayLabel()))
            change.GetDisplayLabel().SetValue(oldVal.GetInvariantDisplayLabel(), newVal.GetInvariantDisplayLabel());
        }

    if (!oldVal.GetInvariantDescription().EqualsIAscii(newVal.GetInvariantDescription()))
        change.GetDescription().SetValue(oldVal.GetInvariantDescription(), newVal.GetInvariantDescription());

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle  02/2018
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::CompareUnit(UnitChange& change, ECUnitCR oldVal, ECUnitCR newVal)
    {
    if (!oldVal.GetName().EqualsIAscii(newVal.GetName()))
        change.GetName().SetValue(oldVal.GetName(), newVal.GetName());

    if (oldVal.GetIsDisplayLabelDefined() && !newVal.GetIsDisplayLabelDefined())
        change.GetDisplayLabel().SetValue(ValueId::Deleted, oldVal.GetInvariantDisplayLabel());
    else if (!oldVal.GetIsDisplayLabelDefined() && newVal.GetIsDisplayLabelDefined())
        change.GetDisplayLabel().SetValue(ValueId::New, newVal.GetInvariantDisplayLabel());
    else if (oldVal.GetIsDisplayLabelDefined() && newVal.GetIsDisplayLabelDefined())
        {
        if (!oldVal.GetInvariantDisplayLabel().EqualsIAscii(newVal.GetInvariantDisplayLabel()))
            change.GetDisplayLabel().SetValue(oldVal.GetInvariantDisplayLabel(), newVal.GetInvariantDisplayLabel());
        }

    if (!oldVal.GetInvariantDescription().EqualsIAscii(newVal.GetInvariantDescription()))
        change.GetDescription().SetValue(oldVal.GetInvariantDescription(), newVal.GetInvariantDescription());

    if (!oldVal.GetDefinition().EqualsIAscii(newVal.GetDefinition()))
        change.GetDefinition().SetValue(oldVal.GetDefinition(), newVal.GetDefinition());

    if (!oldVal.GetDefinition().EqualsIAscii(newVal.GetDefinition()))
        change.GetDefinition().SetValue(oldVal.GetDefinition(), newVal.GetDefinition());

    if (oldVal.HasNumerator() && newVal.HasNumerator() && oldVal.GetNumerator() != newVal.GetNumerator())
        change.GetNumerator().SetValue(oldVal.GetNumerator(), newVal.GetNumerator());
    else if(!oldVal.HasNumerator() && newVal.HasNumerator())
        change.GetNumerator().SetValue(ValueId::New, newVal.GetNumerator());
    else if (oldVal.HasNumerator() && !newVal.HasNumerator())
        change.GetNumerator().SetValue(ValueId::Deleted, oldVal.GetNumerator());

    if (oldVal.HasDenominator() && newVal.HasDenominator() && oldVal.GetDenominator() != newVal.GetDenominator())
        change.GetDenominator().SetValue(oldVal.GetDenominator(), newVal.GetDenominator());
    else if (!oldVal.HasDenominator() && newVal.HasDenominator())
        change.GetDenominator().SetValue(ValueId::New, newVal.GetDenominator());
    else if (oldVal.HasDenominator() && !newVal.HasDenominator())
        change.GetDenominator().SetValue(ValueId::Deleted, oldVal.GetDenominator());

    if (oldVal.HasOffset() && newVal.HasOffset() && oldVal.GetOffset() != newVal.GetOffset())
        change.GetOffset().SetValue(oldVal.GetOffset(), newVal.GetOffset());
    else if (!oldVal.HasOffset() && newVal.HasOffset())
        change.GetOffset().SetValue(ValueId::New, newVal.GetOffset());
    else if (oldVal.HasOffset() && !newVal.HasOffset())
        change.GetOffset().SetValue(ValueId::Deleted, oldVal.GetOffset());

    BeAssert(dynamic_cast<PhenomenonCP> (oldVal.GetPhenomenon()) != nullptr && dynamic_cast<PhenomenonCP> (newVal.GetPhenomenon()));
    PhenomenonCP oldPhen = static_cast<PhenomenonCP> (oldVal.GetPhenomenon());
    PhenomenonCP newPhen = static_cast<PhenomenonCP> (newVal.GetPhenomenon());
    if (!oldPhen->GetFullName().EqualsIAscii(newPhen->GetFullName()))
        change.GetPhenomenon().SetValue(oldPhen->GetFullName(), newPhen->GetFullName());

    BeAssert(dynamic_cast<UnitSystemCP> (oldVal.GetUnitSystem()) != nullptr && dynamic_cast<UnitSystemCP> (newVal.GetUnitSystem()));
    UnitSystemCP oldSystem = static_cast<UnitSystemCP> (oldVal.GetUnitSystem());
    UnitSystemCP newSystem = static_cast<UnitSystemCP> (newVal.GetUnitSystem());
    if (!oldSystem->GetFullName().EqualsIAscii(newSystem->GetFullName()))
        change.GetUnitSystem().SetValue(oldSystem->GetFullName(), newSystem->GetFullName());

    if (oldVal.IsConstant() != newVal.IsConstant())
        change.GetIsConstant().SetValue(oldVal.IsConstant(), newVal.IsConstant());

    Nullable<Utf8String> oldInvertingUnitName = oldVal.IsInvertedUnit() ? oldVal.GetInvertingUnit()->GetFullName() : nullptr;
    Nullable<Utf8String> newInvertingUnitName = newVal.IsInvertedUnit() ? newVal.GetInvertingUnit()->GetFullName() : nullptr;

    if (oldInvertingUnitName != nullptr && newInvertingUnitName == nullptr)
        change.GetInvertingUnit().SetValue(ValueId::Deleted, oldInvertingUnitName.Value());
    else if (oldInvertingUnitName == nullptr && newInvertingUnitName != nullptr)
        change.GetInvertingUnit().SetValue(ValueId::New, newInvertingUnitName.Value());
    else if (!oldInvertingUnitName.Value().EqualsIAscii(newInvertingUnitName.Value()))
        change.GetInvertingUnit().SetValue(std::move(oldInvertingUnitName), std::move(newInvertingUnitName));

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::CompareBaseClasses(BaseClassChanges& changes, ECBaseClassesList const& a, ECBaseClassesList const& b)
    {
    bset<Utf8CP, CompareIUtf8Ascii> aMap, bMap, cMap;
    for (ECClassCP classCP : a)
        aMap.insert(classCP->GetFullName());

    for (ECClassCP classCP : b)
        bMap.insert(classCP->GetFullName());

    cMap.insert(aMap.begin(), aMap.end());
    cMap.insert(bMap.begin(), bMap.end());

    for (Utf8CP u : cMap)
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
BentleyStatus SchemaComparer::CompareReferences(ReferenceChanges& changes, ECSchemaReferenceListCR a, ECSchemaReferenceListCR b)
    {
    bmap<Utf8CP, ECSchemaCP, CompareIUtf8Ascii> aMap, bMap, cMap;
    for (bpair<SchemaKey, ECSchemaPtr> const& ref : a)
        aMap[ref.first.GetName().c_str()] = ref.second.get();

    for (bpair<SchemaKey, ECSchemaPtr> const& ref : b)
        bMap[ref.first.GetName().c_str()] = ref.second.get();

    cMap.insert(aMap.begin(), aMap.end());
    cMap.insert(bMap.begin(), bMap.end());

    for (bpair<Utf8CP, ECSchemaCP> const& u : cMap)
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
BentleyStatus SchemaComparer::AppendECSchema(SchemaChanges& changes, ECSchemaCR v, ValueId appendType)
    {
    ChangeState state = appendType == ValueId::New ? ChangeState::New : ChangeState::Deleted;
    SchemaChange& change = changes.Add(state, v.GetName().c_str());
    change.GetName().SetValue(appendType, v.GetName());

    if (v.GetIsDisplayLabelDefined())
        change.GetDisplayLabel().SetValue(appendType, v.GetInvariantDisplayLabel());

    change.GetDescription().SetValue(appendType, v.GetInvariantDescription());

    change.GetAlias().SetValue(appendType, v.GetAlias());
    change.GetVersionRead().SetValue(appendType, v.GetVersionRead());
    change.GetVersionMinor().SetValue(appendType, v.GetVersionMinor());
    change.GetVersionWrite().SetValue(appendType, v.GetVersionWrite());

    if ((appendType == ValueId::Deleted && m_options.GetSchemaDeleteDetailLevel() == AppendDetailLevel::Partial) ||
        (appendType == ValueId::New && m_options.GetSchemaNewDetailLevel() == AppendDetailLevel::Partial))
        return SUCCESS;

    for (ECClassCP classCP : v.GetClasses())
        if (AppendECClass(change.Classes(), *classCP, appendType) == ERROR)
            return ERROR;

    for (ECEnumerationCP enumerationCP : v.GetEnumerations())
        if (AppendECEnumeration(change.Enumerations(), *enumerationCP, appendType) == ERROR)
            return ERROR;

    for (KindOfQuantityCP kindOfQuantityCP : v.GetKindOfQuantities())
        if (AppendKindOfQuantity(change.KindOfQuantities(), *kindOfQuantityCP, appendType) == ERROR)
            return ERROR;

    if (AppendReferences(change.References(), v.GetReferencedSchemas(), appendType) != SUCCESS)
        return ERROR;

    return AppendCustomAttributes(change.CustomAttributes(), v, appendType);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::AppendECClass(ClassChanges& changes, ECClassCR v, ValueId appendType)
    {
    ChangeState state = appendType == ValueId::New ? ChangeState::New : ChangeState::Deleted;
    ClassChange& change = changes.Add(state, v.GetName().c_str());

    change.GetName().SetValue(appendType, v.GetName());
    if (v.GetIsDisplayLabelDefined())
        change.GetDisplayLabel().SetValue(appendType, v.GetInvariantDisplayLabel());

    change.GetDescription().SetValue(appendType, v.GetInvariantDescription());
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
BentleyStatus SchemaComparer::AppendECRelationshipClass(ECRelationshipChange& change, ECRelationshipClassCR v, ValueId appendType)
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
BentleyStatus SchemaComparer::AppendECRelationshipConstraint(ECRelationshipConstraintChange& change, ECRelationshipConstraintCR v, ValueId appendType)
    {
    change.GetRoleLabel().SetValue(appendType, v.GetRoleLabel());
    change.GetMultiplicity().SetValue(appendType, v.GetMultiplicity().ToString());
    change.IsPolymorphic().SetValue(appendType, v.GetIsPolymorphic());
    if (AppendECRelationshipConstraintClasses(change.ConstraintClasses(), v.GetConstraintClasses(), appendType) != SUCCESS)
        return ERROR;

    return AppendCustomAttributes(change.CustomAttributes(), v, appendType);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::AppendECRelationshipConstraintClasses(ECRelationshipConstraintClassChanges& changes, ECRelationshipConstraintClassList const& v, ValueId appendType)
    {
    ChangeState state = appendType == ValueId::New ? ChangeState::New : ChangeState::Deleted;
    for (ECClassCP constraintClass : v)
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
BentleyStatus SchemaComparer::AppendECRelationshipConstraintClass(ECRelationshipConstraintClassChange& change, ECClassCR v, ValueId appendType)
    {
    change.GetClassName().SetValue(appendType, v.GetFullName());
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::AppendECEnumeration(ECEnumerationChanges& changes, ECEnumerationCR v, ValueId appendType)
    {
    ChangeState state = appendType == ValueId::New ? ChangeState::New : ChangeState::Deleted;
    ECEnumerationChange& enumerationChange = changes.Add(state, v.GetName().c_str());
    if (v.GetIsDisplayLabelDefined())
        enumerationChange.GetDisplayLabel().SetValue(appendType, v.GetInvariantDisplayLabel());

    enumerationChange.GetDescription().SetValue(appendType, v.GetInvariantDescription());
    enumerationChange.IsStrict().SetValue(appendType, v.GetIsStrict());
    enumerationChange.GetTypeName().SetValue(appendType, v.GetTypeName());
    for (ECEnumeratorCP enumeratorCP : v.GetEnumerators())
        {
        ECEnumeratorChange& enumeratorChange = enumerationChange.Enumerators().Add(state, enumeratorCP->GetName().c_str());

        enumeratorChange.GetName().SetValue(appendType, enumeratorCP->GetName());

        if (enumeratorCP->IsInteger())
            enumeratorChange.GetInteger().SetValue(appendType, enumeratorCP->GetInteger());

        if (enumeratorCP->IsString())
            enumeratorChange.GetString().SetValue(appendType, enumeratorCP->GetString());

        if (enumeratorCP->GetIsDisplayLabelDefined())
            enumeratorChange.GetDisplayLabel().SetValue(appendType, enumeratorCP->GetInvariantDisplayLabel());

        enumeratorChange.GetDescription().SetValue(appendType, enumeratorCP->GetInvariantDescription());
        }
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::AppendKindOfQuantity(KindOfQuantityChanges& changes, KindOfQuantityCR koq, ValueId appendType)
    {
    const ChangeState state = appendType == ValueId::New ? ChangeState::New : ChangeState::Deleted;
    KindOfQuantityChange& change = changes.Add(state, koq.GetName().c_str());
    if (koq.GetIsDisplayLabelDefined())
        change.GetDisplayLabel().SetValue(appendType, koq.GetInvariantDisplayLabel());

    change.GetDescription().SetValue(appendType, koq.GetInvariantDescription());
    change.GetPersistenceUnit().SetValue(appendType, koq.GetPersistenceUnit().ToText());
    change.GetRelativeError().SetValue(appendType, koq.GetRelativeError());
    for (Formatting::FormatUnitSet const& fus : koq.GetPresentationUnitList())
        {
        change.GetPresentationUnitList().Add(state).SetValue(appendType, fus.ToText());
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle  06/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::AppendPropertyCategory(PropertyCategoryChanges& changes, PropertyCategoryCR cat, ValueId appendType)
    {
    ChangeState state = appendType == ValueId::New ? ChangeState::New : ChangeState::Deleted;
    PropertyCategoryChange& catChange = changes.Add(state, cat.GetName().c_str());
    catChange.GetName().SetValue(appendType, cat.GetName());
    catChange.GetDisplayLabel().SetValue(appendType, cat.GetDisplayLabel());
    catChange.GetDescription().SetValue(appendType, cat.GetDescription());
    catChange.GetPriority().SetValue(appendType, cat.GetPriority());
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle  02/2018
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::AppendPhenomenon(PhenomenonChanges& changes, PhenomenonCR phen, ValueId appendType)
    {
    ChangeState state = appendType == ValueId::New ? ChangeState::New : ChangeState::Deleted;
    PhenomenonChange& change = changes.Add(state, phen.GetName().c_str());
    if (phen.GetIsDisplayLabelDefined())
        change.GetDisplayLabel().SetValue(appendType, phen.GetInvariantDisplayLabel());

    change.GetDescription().SetValue(appendType, phen.GetInvariantDescription());
    change.GetDefinition().SetValue(appendType, phen.GetDefinition());
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle  02/2018
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::AppendUnitSystem(UnitSystemChanges& changes, UnitSystemCR system, ValueId appendType)
    {
    ChangeState state = appendType == ValueId::New ? ChangeState::New : ChangeState::Deleted;
    UnitSystemChange& change = changes.Add(state, system.GetName().c_str());
    if (system.GetIsDisplayLabelDefined())
        change.GetDisplayLabel().SetValue(appendType, system.GetInvariantDisplayLabel());

    change.GetDescription().SetValue(appendType, system.GetInvariantDescription());
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle  02/2018
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::AppendUnit(UnitChanges& changes, ECUnitCR unit, ValueId appendType)
    {
    ChangeState state = appendType == ValueId::New ? ChangeState::New : ChangeState::Deleted;
    UnitChange& change = changes.Add(state, unit.GetName().c_str());
    if (unit.GetIsDisplayLabelDefined())
        change.GetDisplayLabel().SetValue(appendType, unit.GetInvariantDisplayLabel());

    change.GetDescription().SetValue(appendType, unit.GetInvariantDescription());
    change.GetDefinition().SetValue(appendType, unit.GetDefinition());
    change.GetNumerator().SetValue(appendType, unit.GetNumerator());
    change.GetDenominator().SetValue(appendType, unit.GetDenominator());
    change.GetOffset().SetValue(appendType, unit.GetOffset());

    BeAssert(dynamic_cast<PhenomenonCP> (unit.GetPhenomenon()) != nullptr);
    PhenomenonCP phen = static_cast<PhenomenonCP> (unit.GetPhenomenon());
    change.GetPhenomenon().SetValue(appendType, phen->GetFullName());

    BeAssert(dynamic_cast<UnitSystemCP> (unit.GetUnitSystem()) != nullptr);
    UnitSystemCP system = static_cast<UnitSystemCP> (unit.GetUnitSystem());
    change.GetUnitSystem().SetValue(appendType, system->GetFullName());

    change.GetIsConstant().SetValue(appendType, unit.IsConstant());

    if (unit.IsInvertedUnit())
        {
        Nullable<Utf8String> invertingUnitName = unit.GetInvertingUnit() != nullptr ? unit.GetInvertingUnit()->GetFullName() : nullptr;
        change.GetInvertingUnit().SetValue(appendType, std::move(invertingUnitName));
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::AppendECProperty(ECPropertyChanges& changes, ECPropertyCR v, ValueId appendType)
    {
    ChangeState state = appendType == ValueId::New ? ChangeState::New : ChangeState::Deleted;
    ECPropertyChange& propertyChange = changes.Add(state, v.GetName().c_str());
    propertyChange.GetName().SetValue(appendType, v.GetName());
    if (v.GetIsDisplayLabelDefined())
        propertyChange.GetDisplayLabel().SetValue(appendType, v.GetInvariantDisplayLabel());

    propertyChange.GetDescription().SetValue(appendType, v.GetInvariantDescription());
    propertyChange.GetTypeName().SetValue(appendType, v.GetTypeName());

    propertyChange.IsReadonly().SetValue(appendType, v.GetIsReadOnly());
    if (v.IsPriorityLocallyDefined())
        propertyChange.GetPriority().SetValue(appendType, v.GetPriority());

    if (v.GetKindOfQuantity() != nullptr)
        propertyChange.GetKindOfQuantity().SetValue(appendType, v.GetKindOfQuantity()->GetFullName());

    if (v.GetCategory() != nullptr)
        propertyChange.GetCategory().SetValue(appendType, v.GetCategory()->GetFullName());

    if (NavigationECPropertyCP prop = v.GetAsNavigationProperty())
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
        auto primitiveProp = v.GetAsPrimitiveProperty();

        propertyChange.GetExtendedTypeName().SetValue(appendType, primitiveProp->GetExtendedTypeName());
        if (primitiveProp->GetEnumeration())
            propertyChange.GetEnumeration().SetValue(appendType, primitiveProp->GetEnumeration()->GetFullName());

        if (primitiveProp->GetKindOfQuantity())
            propertyChange.GetKindOfQuantity().SetValue(appendType, primitiveProp->GetKindOfQuantity()->GetFullName());
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
        auto primitivePropArray = v.GetAsPrimitiveArrayProperty();
        propertyChange.GetExtendedTypeName().SetValue(appendType, primitivePropArray->GetExtendedTypeName());
        if (primitivePropArray->GetKindOfQuantity())
            propertyChange.GetKindOfQuantity().SetValue(appendType, primitivePropArray->GetKindOfQuantity()->GetFullName());
        }
    else
        return ERROR;


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
BentleyStatus SchemaComparer::AppendBaseClasses(BaseClassChanges& changes, ECBaseClassesList const& v, ValueId appendType)
    {
    ChangeState state = appendType == ValueId::New ? ChangeState::New : ChangeState::Deleted;
    for (ECClassCP baseClassCP : v)
        changes.Add(state).SetValue(appendType, baseClassCP->GetFullName());

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::AppendReferences(ReferenceChanges& changes, ECSchemaReferenceListCR v, ValueId appendType)
    {
    ChangeState state = appendType == ValueId::New ? ChangeState::New : ChangeState::Deleted;
    for (auto& referenceCP : v)
        changes.Add(state).SetValue(appendType, referenceCP.first.GetFullSchemaName());

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::ConvertECInstanceToValueMap(std::map<Utf8String, ECValue>& map, IECInstanceCR instance)
    {
    ECValuesCollectionPtr values = ECValuesCollection::Create(instance);
    if (values.IsNull())
        return SUCCESS;

    return ConvertECValuesCollectionToValueMap(map, *values);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::ConvertECValuesCollectionToValueMap(std::map<Utf8String, ECValue>& map, ECValuesCollectionCR values)
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
std::vector<Utf8String> SchemaComparer::Split(Utf8StringCR path , bool stripArrayIndex)
    {
    auto stripArrayIndexIfRequired = [&stripArrayIndex] (Utf8String str)
        {
        if (stripArrayIndex)
            {
            auto i = str.find("[");
            if (i == Utf8String::npos)
                {
                return str;
                }

            str = str.substr(0, i);
            }
        return str;
        };

    std::vector<Utf8String> axis;
    size_t b = 0;
    size_t i = 0;
    for (; i < path.size(); i++)
        {
        if (path[i] == '.')
            {

            axis.push_back(stripArrayIndexIfRequired(path.substr(b, i - b)));
            b = i + 1;
            }
        }

    if (b < i)
        axis.push_back(stripArrayIndexIfRequired(path.substr(b , i - b )));

    return axis;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String SchemaComparer::Join(std::vector<Utf8String> const& paths, Utf8CP delimiter)
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
Utf8CP ECChange::SystemIdToString(SystemId id)
    {
    switch (id)
        {
            case SystemId::None: return "";
            case SystemId::Alias: return "Alias";
            case SystemId::Array: return "Array";
            case SystemId::BaseClass: return "BaseClass";
            case SystemId::BaseClasses: return "BaseClasses";
            case SystemId::Classes: return "Classes";
            case SystemId::Class: return "Class";
            case SystemId::ConstantKey: return "ConstantKey";
            case SystemId::ClassFullName: return "ClassFullName";
            case SystemId::ClassModifier: return "ClassModifier";
            case SystemId::ConstraintClass: return "ConstraintClass";
            case SystemId::ConstraintClasses: return "ConstraintClasses";
            case SystemId::Constraint: return "Constraint";
            case SystemId::CustomAttributes: return "CustomAttributes";
            case SystemId::Description: return "Description";
            case SystemId::Direction: return "Direction";
            case SystemId::DisplayLabel: return "DisplayLabel";
            case SystemId::ECVersion: return "ECVersion";
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
            case SystemId::KindOfQuantities: return "KindOfQuantities";
            case SystemId::KindOfQuantity: return "KindOfQuantity";
            case SystemId::KoqPersistenceUnit: return "KoqPersistenceUnit";
            case SystemId::KoqPresentationUnitList: return "KoqPresentationUnitList";
            case SystemId::KoqRelativeError: return "KoqRelativeError";
            case SystemId::MaximumLength: return "MaximumLength";
            case SystemId::MaximumValue: return "MaximumValue";
            case SystemId::MaxOccurs: return "MaxOccurs";
            case SystemId::MinimumLength: return "MinimumLength";
            case SystemId::MinimumValue: return "MinimumValue";
            case SystemId::MinOccurs: return "MinOccurs";
            case SystemId::Multiplicity: return "Multiplicity";
            case SystemId::Name: return "Name";
            case SystemId::Navigation: return "Navigation";
            case SystemId::OriginalECXmlVersionMajor: return "OriginalECXmlVersionMajor";
            case SystemId::OriginalECXmlVersionMinor: return "OriginalECXmlVersionMinor";
            case SystemId::Phenomena: return "Phenomena";
            case SystemId::Phenomenon: return "Phenomenon";
            case SystemId::PhenomenonDefinition: return "PhenomenonDefinition";
            case SystemId::Properties: return "Properties";
            case SystemId::Property: return "Property";
            case SystemId::PropertyCategories: return "PropertyCategories";
            case SystemId::PropertyCategory: return "PropertyCategory";
            case SystemId::PropertyCategoryPriority: return "PropertyCategoryPriority";
            case SystemId::PropertyPriority: return "PropertyPriority";
            case SystemId::PropertyType: return "PropertyType";
            case SystemId::PropertyValue: return "PropertyValue";
            case SystemId::PropertyValues: return "PropertyValues";
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
            case SystemId::UnitSystem: return "UnitSystem";
            case SystemId::UnitSystems: return "UnitSystems";
            case SystemId::Unit: return "Unit";
            case SystemId::UnitDefinition: return "UnitDefinition";
            case SystemId::UnitNumerator: return "UnitNumerator";
            case SystemId::UnitDenominator: return "UnitDenominator";
            case SystemId::UnitInvertingUnit: return "UnitInvertingUnit";
            case SystemId::UnitIsConstant: return "UnitIsConstant";
            case SystemId::UnitOffset: return "UnitOffset";
            case SystemId::Units: return "Units";
            case SystemId::VersionRead: return "VersionRead";
            case SystemId::VersionMinor: return "VersionMinor";
            case SystemId::VersionWrite: return "VersionWrite";
        }

    BeAssert(false && "Unhandled SystemId");
    return "";
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
        str.assign(v.Value().ToString());

    return str;
    }

//======================================================================================>
//BinaryChange
//======================================================================================>
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  01/2017
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String BinaryChange::_ToString(ValueId id) const
    {
    auto& v = GetValue(id);
    if (v.IsNull())
        return NULL_TEXT;

    Utf8String str;
    Base64Utilities::Encode(str, (Byte const*) v.Value().GetPointer(), v.Value().Size());
    return str;
    }

//======================================================================================>
//Point2dChange
//======================================================================================>
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String Point2dChange::_ToString(ValueId id) const
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
//Point3dChange
//======================================================================================>
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String Point3dChange::_ToString(ValueId id) const
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

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle  06/2017
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String MinMaxValueChange::_ToString(ValueId id) const
    {
    Nullable<ECValue> const& v = GetValue(id);
    if (v.IsNull())
        return Utf8String(NULL_TEXT);

    return v.Value().ToString();
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
            case PRIMITIVETYPE_Point2d:
                return GetPoint2d()->SetValue(id, value.GetPoint2d());
            case PRIMITIVETYPE_Point3d:
                return GetPoint3d()->SetValue(id, value.GetPoint3d());
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
            case PRIMITIVETYPE_Point2d:
                return GetPoint2d()->SetValue(Converter<DPoint2d>::Copy(oldValue), Converter<DPoint2d>::Copy(newValue));
            case PRIMITIVETYPE_Point3d:
                return GetPoint3d()->SetValue(Converter<DPoint3d>::Copy(oldValue), Converter<DPoint3d>::Copy(newValue));
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


    if (m_derivedTables != nullptr)
        {
        //AppendBegin(str, *this, currentIndex);
        for (size_t i = 0; i < m_derivedTables->Count(); i++)
            {
            m_derivedTables->At(i).WriteToString(str, currentIndex + indentSize, indentSize);
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

    if (m_derivedTables != nullptr)
        return m_derivedTables->IsEmpty();

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

    if (m_derivedTables != nullptr)
        if (m_derivedTables->IsEmpty())
            m_derivedTables = nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECPropertyValueChange::InitValue(ECN::PrimitiveType type)
    {
    if (m_type == type)
        return SUCCESS;

    if (type == static_cast<ECN::PrimitiveType>(0))
        {
        m_value = nullptr;
        return SUCCESS;
        }

    switch (type)
        {
            case ECN::PRIMITIVETYPE_Binary:
                m_value = std::unique_ptr<ECChange>(new BinaryChange(GetState(), SystemId::PropertyValue, this, GetId())); break;
            case ECN::PRIMITIVETYPE_Boolean:
                m_value = std::unique_ptr<ECChange>(new BooleanChange(GetState(), SystemId::PropertyValue, this, GetId())); break;
            case ECN::PRIMITIVETYPE_DateTime:
                m_value = std::unique_ptr<ECChange>(new DateTimeChange(GetState(), SystemId::PropertyValue, this, GetId())); break;
            case ECN::PRIMITIVETYPE_Double:
                m_value = std::unique_ptr<ECChange>(new DoubleChange(GetState(), SystemId::PropertyValue, this, GetId())); break;
            case ECN::PRIMITIVETYPE_IGeometry:
            {
            LOG.errorv("ECSchemaComparer: Changes in ECProperties of type IGeometry are not supported.");
            return ERROR;
            }
            case ECN::PRIMITIVETYPE_Integer:
                m_value = std::unique_ptr<ECChange>(new Int32Change(GetState(), SystemId::PropertyValue, this, GetId())); break;
            case ECN::PRIMITIVETYPE_Long:
                m_value = std::unique_ptr<ECChange>(new Int64Change(GetState(), SystemId::PropertyValue, this, GetId())); break;
            case ECN::PRIMITIVETYPE_Point2d:
                m_value = std::unique_ptr<ECChange>(new Point2dChange(GetState(), SystemId::PropertyValue, this, GetId())); break;
            case ECN::PRIMITIVETYPE_Point3d:
                m_value = std::unique_ptr<ECChange>(new Point3dChange(GetState(), SystemId::PropertyValue, this, GetId())); break;
            case ECN::PRIMITIVETYPE_String:
                m_value = std::unique_ptr<ECChange>(new StringChange(GetState(), SystemId::PropertyValue, this, GetId())); break;
            default:
                BeAssert(false && "Unexpected value for PrimitiveType");
                return ERROR;
        }

    m_type = type;
    return SUCCESS;
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
void ECPropertyValueChange::GetFlatListOfChildren(std::vector<ECPropertyValueChange const*>& childrens) const
    {
    if (m_derivedTables != nullptr)
        {
        for (size_t i = 0; i < m_derivedTables->Count(); i++)
            {
            m_derivedTables->At(i).GetFlatListOfChildren(childrens);
            }
        }
    else
        childrens.push_back(this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
std::vector<ECPropertyValueChange const*> ECPropertyValueChange::GetFlatListOfChildren() const
    {
    std::vector<ECPropertyValueChange const*> v;
    GetFlatListOfChildren(v);
    return v;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
ECChangeArray<ECPropertyValueChange>& ECPropertyValueChange::GetChildren() const
    {
    if (m_derivedTables == nullptr)
        m_derivedTables = std::unique_ptr<ECChangeArray<ECPropertyValueChange>>(new ECChangeArray<ECPropertyValueChange>(GetState(), SystemId::PropertyValues, this, GetId(), SystemId::PropertyValue));

    return *m_derivedTables;
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
    Utf8String ap(accessPath);
    std::vector<Utf8String> path = SchemaComparer::Split(ap);
    if (path.empty())
        return nullptr;

    if (ap.find(":") != Utf8String::npos)
        {
        if (path.front() != GetId())
            return nullptr;

        path.erase(path.begin());
        }

    ECPropertyValueChange* c = this;
    for (Utf8StringCR str : path)
        {
        if (!c->HasChildren())
            return nullptr;

        auto m = c->GetChildren().Find(str.c_str());
        if (m == nullptr)
            return nullptr;

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
bool CustomAttributeValidator::Rule::Match(std::vector<Utf8String> const& source) const
    {
    if (source.empty())
        return false;

    if (m_pattern.size() == 1)//Foo:*
        if (m_pattern.front().EndsWith("*"))
            return true;

    auto sb = source.begin();
    auto pb = m_pattern.begin();
    auto se = source.end();
    auto pe = m_pattern.end();
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
std::vector<std::unique_ptr<CustomAttributeValidator::Rule>> const& CustomAttributeValidator::GetRelevantRules(ECPropertyValueChange& change) const
    {
    if (!change.IsDefinition())
        return m_rules.find(m_wildCard)->second;

    Utf8String prefix = GetPrefix(change.GetAccessString());
    if (prefix.empty())
        return m_rules.find(m_wildCard)->second;;

    auto itor = m_rules.find(prefix);
    if (itor != m_rules.end())
        return itor->second;

    return m_rules.find(m_wildCard)->second;
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
    std::vector<ECPropertyValueChange const*> flatListOfChanges = change.GetFlatListOfChildren();
    std::vector<std::unique_ptr<Rule>> const& rules = GetRelevantRules(change);
    if (rules.empty())
        return GetDefaultPolicy();

    for (ECPropertyValueChange const* v : flatListOfChanges)
        {
        if (v->HasChildren())
            continue;

        std::vector<Utf8String> path = SchemaComparer::Split(v->GetAccessString(), true);

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
void CustomAttributeValidator::Reset()
    {
    m_rules.clear();
    m_rules[m_wildCard].clear();
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


END_BENTLEY_ECOBJECT_NAMESPACE
