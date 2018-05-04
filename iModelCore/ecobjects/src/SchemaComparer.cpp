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
Binary::Binary(Binary const& rhs) { *this = rhs; }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
Binary::Binary(Binary&& rhs) { *this = std::move(rhs); }
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
            SchemaChange& schemaChange = changes.Add(ChangeType::Modified, schemaName);
            if (SUCCESS != CompareECSchema(schemaChange, *lhsIt->second, *rhsIt->second))
                return ERROR;
            }
        else if (existInLhs && !existInRhs)
            {
            SchemaChange& schemaChange = changes.Add(ChangeType::Deleted, schemaName);
            if (AppendECSchema(schemaChange, *lhsIt->second) == ERROR)
                return ERROR;
            }
        else if (!existInLhs && existInRhs)
            {
            SchemaChange& schemaChange = changes.Add(ChangeType::New, schemaName);
            if (AppendECSchema(schemaChange, *rhsIt->second) == ERROR)
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
        change.Name().Set(a.GetName(), b.GetName());

    if (a.GetIsDisplayLabelDefined() && !b.GetIsDisplayLabelDefined())
        change.DisplayLabel().Set(a.GetInvariantDisplayLabel(), nullptr);
    else if (!a.GetIsDisplayLabelDefined() && b.GetIsDisplayLabelDefined())
        change.DisplayLabel().Set(nullptr, b.GetInvariantDisplayLabel());
    else if (a.GetIsDisplayLabelDefined() && b.GetIsDisplayLabelDefined() && !a.GetInvariantDisplayLabel().EqualsIAscii(b.GetInvariantDisplayLabel()))
         change.DisplayLabel().Set(a.GetInvariantDisplayLabel(), b.GetInvariantDisplayLabel());

    if (!a.GetInvariantDescription().EqualsIAscii(b.GetInvariantDescription()))
        change.Description().Set(a.GetInvariantDescription(), b.GetInvariantDescription());

    if (a.GetAlias() != b.GetAlias())
        change.Alias().Set(a.GetAlias(), b.GetAlias());

    if (a.GetVersionRead() != b.GetVersionRead())
        change.VersionRead().Set(a.GetVersionRead(), b.GetVersionRead());

    if (a.GetVersionMinor() != b.GetVersionMinor())
        change.VersionMinor().Set(a.GetVersionMinor(), b.GetVersionMinor());

    if (a.GetVersionWrite() != b.GetVersionWrite())
        change.VersionWrite().Set(a.GetVersionWrite(), b.GetVersionWrite());

    if (a.GetECVersion() != b.GetECVersion())
        change.ECVersion().Set((uint32_t) a.GetECVersion(), (uint32_t) b.GetECVersion());

    if (a.GetOriginalECXmlVersionMajor() != b.GetOriginalECXmlVersionMajor())
        change.OriginalECXmlVersionMajor().Set(a.GetOriginalECXmlVersionMajor(), b.GetOriginalECXmlVersionMajor());

    if (a.GetOriginalECXmlVersionMinor() != b.GetOriginalECXmlVersionMinor())
        change.OriginalECXmlVersionMinor().Set(a.GetOriginalECXmlVersionMinor(), b.GetOriginalECXmlVersionMinor());

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

    if (SUCCESS != CompareFormats(change.Formats(), a.GetFormats(), b.GetFormats()))
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
        change.GetName().Set(a.GetName(), b.GetName());

    if (a.GetIsDisplayLabelDefined() && !b.GetIsDisplayLabelDefined())
        change.GetDisplayLabel().Set(a.GetInvariantDisplayLabel(), nullptr);
    else if (!a.GetIsDisplayLabelDefined() && b.GetIsDisplayLabelDefined())
        change.GetDisplayLabel().Set(nullptr, b.GetInvariantDisplayLabel());
    else if (a.GetIsDisplayLabelDefined() && b.GetIsDisplayLabelDefined())
        {
        if (!a.GetInvariantDisplayLabel().EqualsIAscii(b.GetInvariantDisplayLabel()))
            change.GetDisplayLabel().Set(a.GetInvariantDisplayLabel(), b.GetInvariantDisplayLabel());
        }

    if (!a.GetInvariantDescription().EqualsIAscii(b.GetInvariantDescription()))
        change.GetDescription().Set(a.GetInvariantDescription(), b.GetInvariantDescription());

    if (a.GetClassModifier() != b.GetClassModifier())
        change.ClassModifier().Set(a.GetClassModifier(), b.GetClassModifier());

    if (a.GetClassType() != b.GetClassType())
        change.ClassType().Set(a.GetClassType(), b.GetClassType());

    if (a.IsRelationshipClass() != b.IsRelationshipClass())
        {
        if (a.IsRelationshipClass())
            {
            if (AppendECRelationshipClass(change.GetRelationship(), *a.GetRelationshipClassCP()) != SUCCESS)
                return ERROR;
            }
        else if (b.IsRelationshipClass())
            {
            if (AppendECRelationshipClass(change.GetRelationship(), *b.GetRelationshipClassCP()) != SUCCESS)
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
            changes.Add(ChangeType::Modified).Set(Utf8String(a[i]->GetFullName()), Utf8String(b[i]->GetFullName()));
            }
        }
    for (size_t i = m; i < a.size(); i++)
        changes.Add(ChangeType::Deleted).Set(Utf8String(a[i]->GetFullName()));

    for (size_t i = m; i < b.size(); i++)
        changes.Add(ChangeType::New).Set(Utf8String(b[i]->GetFullName()));

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::CompareECRelationshipClass(RelationshipChange& change, ECRelationshipClassCR a, ECRelationshipClassCR b)
    {
    if (a.GetStrength() != b.GetStrength())
        change.Strength().Set(a.GetStrength(), b.GetStrength());

    if (a.GetStrengthDirection() != b.GetStrengthDirection())
        change.StrengthDirection().Set(a.GetStrengthDirection(), b.GetStrengthDirection());

    if (CompareECRelationshipConstraint(change.Source(), a.GetSource(), b.GetSource()) != SUCCESS)
        return ERROR;

    return CompareECRelationshipConstraint(change.Target(), a.GetTarget(), b.GetTarget());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::CompareECRelationshipConstraint(ECRelationshipConstraintChange& change, ECRelationshipConstraintCR a, ECRelationshipConstraintCR b)
    {
    if (a.GetRoleLabel() != b.GetRoleLabel())
        change.RoleLabel().Set(a.GetRoleLabel(), b.GetRoleLabel());

    if (a.GetIsPolymorphic() != b.GetIsPolymorphic())
        change.IsPolymorphic().Set(a.GetIsPolymorphic(), b.GetIsPolymorphic());

    if (a.GetMultiplicity().ToString() != b.GetMultiplicity().ToString())
        change.Multiplicity().Set(a.GetMultiplicity().ToString(), b.GetMultiplicity().ToString());

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
        auto oldValIt = aMap.find(u.first);
        auto newValIt = bMap.find(u.first);

        bool existInOld = oldValIt != aMap.end();
        bool existInNew = newValIt != bMap.end();
        if (existInOld && !existInNew)
            change.Add(ChangeType::Deleted).Set(Utf8String(oldValIt->second->GetFullName()));
        else if (!existInOld && existInNew)
            change.Add(ChangeType::New).Set(Utf8String(newValIt->second->GetFullName()));
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::CompareECProperty(PropertyChange& change, ECPropertyCR a, ECPropertyCR b)
    {
    if (a.GetTypeName() != b.GetTypeName())
        change.GetTypeName().Set(a.GetTypeName(), b.GetTypeName());

    if (a.GetName() != b.GetName())
        change.GetName().Set(a.GetName(), b.GetName());

    if (a.GetIsDisplayLabelDefined() && !b.GetIsDisplayLabelDefined())
        change.GetDisplayLabel().Set(a.GetInvariantDisplayLabel(), nullptr);
    else if (!a.GetIsDisplayLabelDefined() && b.GetIsDisplayLabelDefined())
        change.GetDisplayLabel().Set(nullptr, b.GetInvariantDisplayLabel());
    else if (a.GetIsDisplayLabelDefined() && b.GetIsDisplayLabelDefined())
        {
        if (!a.GetInvariantDisplayLabel().EqualsIAscii(b.GetInvariantDisplayLabel()))
            change.GetDisplayLabel().Set(a.GetInvariantDisplayLabel(), b.GetInvariantDisplayLabel());
        }

    if (!a.GetInvariantDescription().EqualsIAscii(b.GetInvariantDescription()))
        change.GetDescription().Set(a.GetInvariantDescription(), b.GetInvariantDescription());

    PrimitiveECPropertyCP aPrimProp = a.GetAsPrimitiveProperty();
    PrimitiveECPropertyCP bPrimProp = b.GetAsPrimitiveProperty();
    NavigationECPropertyCP aNavProp = a.GetAsNavigationProperty();
    NavigationECPropertyCP bNavProp = b.GetAsNavigationProperty();
    ArrayECPropertyCP aArrayProp = a.GetAsArrayProperty();
    ArrayECPropertyCP bArrayProp = b.GetAsArrayProperty();
    PrimitiveArrayECPropertyCP aPrimArrayProp = a.GetAsPrimitiveArrayProperty();
    PrimitiveArrayECPropertyCP bPrimArrayProp = b.GetAsPrimitiveArrayProperty();

    if (a.GetIsPrimitive() != b.GetIsPrimitive())
        change.IsPrimitive().Set(a.GetIsPrimitive(), b.GetIsPrimitive());

    if (a.GetIsStruct() != b.GetIsStruct())
        change.IsStruct().Set(a.GetIsStruct(), b.GetIsStruct());

    if (a.GetIsStructArray() != b.GetIsStructArray())
        change.IsStructArray().Set(a.GetIsStructArray(), b.GetIsStructArray());

    if (a.GetIsPrimitiveArray() != b.GetIsPrimitiveArray())
        change.IsPrimitiveArray().Set(a.GetIsPrimitiveArray(), b.GetIsPrimitiveArray());

    if (a.GetIsNavigation() != b.GetIsNavigation())
        change.IsNavigation().Set(a.GetIsNavigation(), b.GetIsNavigation());

    if (a.GetIsReadOnly() != b.GetIsReadOnly())
        change.IsReadonly().Set(a.GetIsReadOnly(), b.GetIsReadOnly());

    if (a.GetPriority() != b.GetPriority())
        change.GetPriority().Set(a.GetPriority(), b.GetPriority());

    // MinimumLength
    {
    Nullable<uint32_t> oldVal;
    if (a.IsMinimumLengthDefined())
        oldVal = a.GetMinimumLength();

    Nullable<uint32_t> newVal;
    if (b.IsMinimumLengthDefined())
        newVal = b.GetMinimumLength();

    if (oldVal != newVal)
        change.GetMinimumLength().Set(oldVal, newVal);
    }

    // MaximumLength
    {
    Nullable<uint32_t> oldVal;
    if (a.IsMaximumLengthDefined())
        oldVal = a.GetMaximumLength();

    Nullable<uint32_t> newVal;
    if (b.IsMaximumLengthDefined())
        newVal = b.GetMaximumLength();

    if (oldVal != newVal)
        change.GetMaximumLength().Set(oldVal, newVal);
    }

    // MinimumValue
    {
    Nullable<ECValue> oldVal;
    if (a.IsMinimumValueDefined())
        {
        ECValue v;
        if (ECObjectsStatus::Success != a.GetMinimumValue(v))
            return ERROR;

        oldVal = v;
        }

    Nullable<ECValue> newVal;
    if (b.IsMinimumValueDefined())
        {
        ECValue v;

        if (ECObjectsStatus::Success != a.GetMinimumValue(v))
            return ERROR;

        newVal = v;
        }

    if (oldVal != newVal)
        change.GetMinimumValue().Set(oldVal, newVal);
    }

    // MaximumValue
    {
    Nullable<ECValue> oldVal;
    if (a.IsMaximumValueDefined())
        {
        ECValue v;
        if (ECObjectsStatus::Success != a.GetMaximumValue(v))
            return ERROR;

        oldVal = v;
        }

    Nullable<ECValue> newVal;
    if (b.IsMaximumValueDefined())
        {
        ECValue v;

        if (ECObjectsStatus::Success != a.GetMaximumValue(v))
            return ERROR;

        newVal = v;
        }

    if (oldVal != newVal)
        change.GetMaximumValue().Set(oldVal, newVal);
    }

    // KOQ
    {
    KindOfQuantityCP aKoq = a.GetKindOfQuantity();
    KindOfQuantityCP bKoq = b.GetKindOfQuantity();

    Nullable<Utf8String> oldVal;
    if (aKoq != nullptr)
        oldVal = aKoq->GetFullName();

    Nullable<Utf8String> newVal;
    if (bKoq != nullptr)
        newVal = bKoq->GetFullName();

    if (oldVal != newVal)
        change.GetKindOfQuantity().Set(oldVal, newVal);
    }

    // PropertyCategory
    {
    PropertyCategoryCP oldCat = a.GetCategory();
    PropertyCategoryCP newCat = b.GetCategory();

    Nullable<Utf8String> oldVal;
    if (oldCat != nullptr)
        oldVal = oldCat->GetFullName();

    Nullable<Utf8String> newVal;
    if (newCat != nullptr)
        newVal = newCat->GetFullName();

    if (oldVal != newVal)
        change.GetCategory().Set(oldVal, newVal);
    }

    //ECEnumeration
    {
    ECEnumerationCP oldEnum = nullptr, newEnum = nullptr;
    if (aPrimProp != nullptr)
        oldEnum = aPrimProp->GetEnumeration();
    else if (aPrimArrayProp != nullptr)
        oldEnum = aPrimArrayProp->GetEnumeration();

    if (bPrimProp != nullptr)
        newEnum = bPrimProp->GetEnumeration();
    else if (bPrimArrayProp != nullptr)
        newEnum = bPrimArrayProp->GetEnumeration();

    Nullable<Utf8String> oldVal;
    if (oldEnum != nullptr)
        oldVal = oldEnum->GetFullName();

    Nullable<Utf8String> newVal;
    if (newEnum != nullptr)
        newVal = newEnum->GetFullName();

    if (oldVal != newVal)
        change.GetEnumeration().Set(oldVal, newVal);
    }


    {
    //ExtendedType
    Nullable<Utf8String> oldVal;
    if (a.HasExtendedType())
        {
        if (aPrimProp != nullptr)
            oldVal = aPrimProp->GetExtendedTypeName();
        else if (aPrimArrayProp != nullptr)
            oldVal = aPrimArrayProp->GetExtendedTypeName();
        else
            {
            BeAssert(false && "Property type which is not expected to have an extended type name. Code needs to be adjusted");
            return ERROR;
            }
        }

    Nullable<Utf8String> newVal;
    if (b.HasExtendedType())
        {
        if (bPrimProp != nullptr)
            newVal = bPrimProp->GetExtendedTypeName();
        else if (bPrimArrayProp != nullptr)
            newVal = bPrimArrayProp->GetExtendedTypeName();
        else
            {
            BeAssert(false && "Property type which is not expected to have an extended type name. Code needs to be adjusted");
            return ERROR;
            }
        }

    if (oldVal != newVal)
            change.GetExtendedTypeName().Set(oldVal, newVal);
    }

    {
    // Nav prop
    Nullable<ECRelatedInstanceDirection> oldDirection;
    Nullable<Utf8String> oldRel;
    if (aNavProp != nullptr)
        {
        oldDirection = aNavProp->GetDirection();
        if (aNavProp->GetRelationshipClass() != nullptr)
            oldRel = aNavProp->GetRelationshipClass()->GetFullName();
        }

    Nullable<ECRelatedInstanceDirection> newDirection;
    Nullable<Utf8String> newRel;
    if (bNavProp != nullptr)
        {
        newDirection = bNavProp->GetDirection();
        if (bNavProp->GetRelationshipClass() != nullptr)
            newRel = bNavProp->GetRelationshipClass()->GetFullName();
        }

    if (oldDirection != newDirection)
        change.GetNavigation().Direction().Set(oldDirection, newDirection);

    if (oldRel != newRel)
        change.GetNavigation().Relationship().Set(oldRel, newRel);
    }
    
    {
    //Array
    Nullable<uint32_t> oldMinOccurs;
    Nullable<uint32_t> oldMaxOccurs;
    if (aArrayProp != nullptr)
        {
        oldMinOccurs = aArrayProp->GetMinOccurs();
        oldMaxOccurs = aArrayProp->GetStoredMaxOccurs();
        }

    Nullable<uint32_t> newMinOccurs;
    Nullable<uint32_t> newMaxOccurs;
    if (bArrayProp != nullptr)
        {
        newMinOccurs = bArrayProp->GetMinOccurs();
        newMaxOccurs = bArrayProp->GetStoredMaxOccurs();
        }

    if (oldMinOccurs != newMinOccurs)
        change.GetArray().MinOccurs().Set(oldMinOccurs, newMinOccurs);

    if (oldMaxOccurs != newMaxOccurs)
        change.GetArray().MaxOccurs().Set(oldMaxOccurs, newMaxOccurs);
    }

    return CompareCustomAttributes(change.CustomAttributes(), a, b);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::CompareECProperties(PropertyChanges& changes, ECPropertyIterableCR a, ECPropertyIterableCR b)
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
            auto& propertyChange = changes.Add(ChangeType::Modified, u.first);
            if (CompareECProperty(propertyChange, *itorA->second, *itorB->second) == ERROR)
                return ERROR;
            }
        else if (existInA && !existInB)
            {
            auto& propertyChange = changes.Add(ChangeType::Deleted, u.first);
            if (AppendECProperty(propertyChange, *itorA->second) == ERROR)
                return ERROR;
            }
        else if (!existInA && existInB)
            {
            auto& propertyChange = changes.Add(ChangeType::New, u.first);
            if (AppendECProperty(propertyChange, *itorB->second) == ERROR)
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
            auto& classChange = changes.Add(ChangeType::Modified, u.first);
            if (CompareECClass(classChange, *itorA->second, *itorB->second) == ERROR)
                return ERROR;
            }
        else if (existInA && !existInB)
            {
            auto& classChange = changes.Add(ChangeType::Deleted, u.first);
            if (AppendECClass(classChange, *itorA->second) == ERROR)
                return ERROR;
            }
        else if (!existInA && existInB)
            {
            auto& classChange = changes.Add(ChangeType::New, u.first);
            if (AppendECClass(classChange, *itorB->second) == ERROR)
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
            auto& enumChange = changes.Add(ChangeType::Modified, u.first);
            if (CompareECEnumeration(enumChange, *itorA->second, *itorB->second) == ERROR)
                return ERROR;
            }
        else if (existInA && !existInB)
            {
            auto& enumChange = changes.Add(ChangeType::Deleted, u.first);
            if (AppendECEnumeration(enumChange, *itorA->second) == ERROR)
                return ERROR;
            }
        else if (!existInA && existInB)
            {
            auto& enumChange = changes.Add(ChangeType::New, u.first);
            if (AppendECEnumeration(enumChange, *itorB->second) == ERROR)
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
            auto& kindOfQuantityChange = changes.Add(ChangeType::Modified, u.first);
            if (CompareKindOfQuantity(kindOfQuantityChange, *itorA->second, *itorB->second) == ERROR)
                return ERROR;
            }
        else if (existInA && !existInB)
            {
            auto& kindOfQuantityChange = changes.Add(ChangeType::Deleted, u.first);
            if (AppendKindOfQuantity(kindOfQuantityChange, *itorA->second) == ERROR)
                return ERROR;
            }
        else if (!existInA && existInB)
            {
            auto& kindOfQuantityChange = changes.Add(ChangeType::New, u.first);
            if (AppendKindOfQuantity(kindOfQuantityChange, *itorB->second) == ERROR)
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
            PropertyCategoryChange& catChange = changes.Add(ChangeType::Modified, name);
            if (ComparePropertyCategory(catChange, *oldIt->second, *newIt->second) == ERROR)
                return ERROR;

            continue;
            }
        
        if (existInOld && !existInNew)
            {
            PropertyCategoryChange& catChange = changes.Add(ChangeType::Deleted, name);
            if (AppendPropertyCategory(catChange, *oldIt->second) == ERROR)
                return ERROR;

            continue;
            }

        if (!existInOld && existInNew)
            {
            PropertyCategoryChange& catChange = changes.Add(ChangeType::New, name);
            if (AppendPropertyCategory(catChange, *newIt->second) == ERROR)
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
            PhenomenonChange& change = changes.Add(ChangeType::Modified, name);
            if (SUCCESS != ComparePhenomenon(change, *oldIt->second, *newIt->second))
                return ERROR;

            continue;
            }

        if (existInOld && !existInNew)
            {
            PhenomenonChange& change = changes.Add(ChangeType::Deleted, name);
            if (SUCCESS != AppendPhenomenon(change, *oldIt->second))
                return ERROR;

            continue;
            }

        if (!existInOld && existInNew)
            {
            PhenomenonChange& change = changes.Add(ChangeType::New, name);
            if (SUCCESS != AppendPhenomenon(change, *newIt->second))
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
            UnitSystemChange& change = changes.Add(ChangeType::Modified, name);
            if (SUCCESS != CompareUnitSystem(change, *oldIt->second, *newIt->second))
                return ERROR;

            continue;
            }

        if (existInOld && !existInNew)
            {
            UnitSystemChange& change = changes.Add(ChangeType::Deleted, name);
            if (SUCCESS != AppendUnitSystem(change, *oldIt->second))
                return ERROR;

            continue;
            }

        if (!existInOld && existInNew)
            {
            UnitSystemChange& change = changes.Add(ChangeType::New, name);
            if (SUCCESS != AppendUnitSystem(change, *newIt->second))
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
            UnitChange& change = changes.Add(ChangeType::Modified, name);
            if (SUCCESS != CompareUnit(change, *oldIt->second, *newIt->second))
                return ERROR;

            continue;
            }

        if (existInOld && !existInNew)
            {
            UnitChange& change = changes.Add(ChangeType::Deleted, name);
            if (SUCCESS != AppendUnit(change, *oldIt->second))
                return ERROR;

            continue;
            }

        if (!existInOld && existInNew)
            {
            UnitChange& change = changes.Add(ChangeType::New, name);
            if (SUCCESS != AppendUnit(change, *newIt->second))
                return ERROR;
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Kyle.Abramowitz    04/2018
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::CompareFormats(FormatChanges& changes, ECN::FormatContainerCR oldValues, ECN::FormatContainerCR newValues)
    {
    std::map<Utf8CP, ECFormatCP, CompareIUtf8Ascii> oldMap, newMap, allMap;
    for (ECFormatCP ph : oldValues)
        oldMap[ph->GetName().c_str()] = ph;

    for (ECFormatCP ph : newValues)
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
            FormatChange& change = changes.Add(ChangeType::Modified, name);
            if (SUCCESS != CompareFormat(change, *oldIt->second, *newIt->second))
                return ERROR;

            continue;
            }

        if (existInOld && !existInNew)
            {
            FormatChange& change = changes.Add(ChangeType::Deleted, name);
            if (SUCCESS != AppendFormat(change, *oldIt->second))
                return ERROR;

            continue;
            }

        if (!existInOld && existInNew)
            {
            FormatChange& change = changes.Add(ChangeType::New, name);
            if (SUCCESS != AppendFormat(change, *newIt->second))
                return ERROR;
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::CompareCustomAttribute(CustomAttributeChange& change, IECInstanceCR a, IECInstanceCR b)
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

    for (Utf8CP accessString : cMap)
        {
        auto oldPropValuesIt = aMap.find(accessString);
        auto newPropValuesIt = bMap.find(accessString);

        bool existInOld = oldPropValuesIt != aMap.end();
        bool existInNew = newPropValuesIt != bMap.end();
        if (existInOld && existInNew)
            {
            if (!oldPropValuesIt->second.Equals(newPropValuesIt->second))
                change.Add(ChangeType::Modified, accessString).Set(oldPropValuesIt->second, newPropValuesIt->second);
            }
        else if (existInOld && !existInNew)
            {
            change.Add(ChangeType::Deleted, accessString).Set(oldPropValuesIt->second);
            }
        else if (!existInOld && existInNew)
            {
            change.Add(ChangeType::New, accessString).Set(newPropValuesIt->second);
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::AppendCustomAttribute(CustomAttributeChange& change, IECInstanceCR ca)
    {
    std::map<Utf8String, ECValue> propValueMap;
    if (ConvertECInstanceToValueMap(propValueMap, ca) != SUCCESS)
        return ERROR;

    for (std::pair<Utf8String, ECValue> const& kvPair : propValueMap)
        change.Add(change.GetChangeType(), kvPair.first.c_str()).Set(kvPair.second);

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::AppendCustomAttributes(CustomAttributeChanges& changes, IECCustomAttributeContainerCR caContainer)
    {
    for (IECInstancePtr ca : caContainer.GetCustomAttributes(false))
        {
        //key CA instances by the CA class name (as no more than one CA per class can be on a container
        if (SUCCESS != AppendCustomAttribute(changes.Add(changes.GetChangeType(), ca->GetClass().GetFullName()), *ca))
            return ERROR;
        }
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::CompareCustomAttributes(CustomAttributeChanges& changes, IECCustomAttributeContainerCR a, IECCustomAttributeContainerCR b)
    {
    std::map<Utf8CP, IECInstanceCP, CompareIUtf8Ascii> oldCAs, newCAs, cMap;
    for (IECInstancePtr const& instancePtr : a.GetCustomAttributes(false))
        oldCAs[instancePtr->GetClass().GetFullName()] = instancePtr.get();

    for (IECInstancePtr const& instancePtr : b.GetCustomAttributes(false))
        newCAs[instancePtr->GetClass().GetFullName()] = instancePtr.get();

    cMap.insert(oldCAs.cbegin(), oldCAs.cend());
    cMap.insert(newCAs.cbegin(), newCAs.cend());

    for (auto& u : cMap)
        {
        auto oldCAIt= oldCAs.find(u.first);
        auto newCAIt = newCAs.find(u.first);

        bool existInOld = oldCAIt != oldCAs.end();
        bool existInNew = newCAIt != newCAs.end();
        if (existInOld && existInNew)
            {
            auto& caChange = changes.Add(ChangeType::Modified, u.first);
            if (CompareCustomAttribute(caChange, *oldCAIt->second, *newCAIt->second) == ERROR)
                return ERROR;
            }
        else if (existInOld && !existInNew)
            {
            auto& caChange = changes.Add(ChangeType::Deleted, u.first);
            if (AppendCustomAttribute(caChange, *oldCAIt->second) == ERROR)
                return ERROR;
            }
        else if (!existInOld && existInNew)
            {
            auto& caChange = changes.Add(ChangeType::New, u.first);
            if (AppendCustomAttribute(caChange, *newCAIt->second) == ERROR)
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
        change.Name().Set(oldVal.GetName(), newVal.GetName());

    if (oldVal.GetIsDisplayLabelDefined() && !newVal.GetIsDisplayLabelDefined())
        change.DisplayLabel().Set(oldVal.GetInvariantDisplayLabel(), nullptr);
    else if (!oldVal.GetIsDisplayLabelDefined() && newVal.GetIsDisplayLabelDefined())
        change.DisplayLabel().Set(nullptr, newVal.GetInvariantDisplayLabel());
    else if (oldVal.GetIsDisplayLabelDefined() && newVal.GetIsDisplayLabelDefined())
        {
        if (!oldVal.GetInvariantDisplayLabel().EqualsIAscii(newVal.GetInvariantDisplayLabel()))
            change.DisplayLabel().Set(oldVal.GetInvariantDisplayLabel(), newVal.GetInvariantDisplayLabel());
        }

    if (!oldVal.GetInvariantDescription().EqualsIAscii(newVal.GetInvariantDescription()))
        change.Description().Set(oldVal.GetInvariantDescription(), newVal.GetInvariantDescription());

    if (oldVal.GetIsStrict() != newVal.GetIsStrict())
        change.IsStrict().Set(oldVal.GetIsStrict(), newVal.GetIsStrict());

    if (oldVal.GetType() == newVal.GetType())
        return CompareECEnumerators(change.Enumerators(), oldVal.GetEnumerators(), newVal.GetEnumerators());

    change.TypeName().Set(oldVal.GetTypeName(), newVal.GetTypeName());
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
            ECEnumeratorChange& enumeratorChange = changes.Add(ChangeType::Modified, enumeratorName);
            if (oldEnumerator->IsInteger())
                {
                if (oldEnumerator->GetInteger() != newEnumerator->GetInteger())
                    enumeratorChange.Integer().Set(oldEnumerator->GetInteger(), newEnumerator->GetInteger());
                }

            if (oldIt->second->IsString())
                {
                if (oldIt->second->GetString() != newEnumerator->GetString())
                    enumeratorChange.String().Set(oldEnumerator->GetString(), newEnumerator->GetString());
                }

            const bool displayLabelDefinedInOld = oldEnumerator->GetIsDisplayLabelDefined();
            const bool displayLabelDefinedInNew = newEnumerator->GetIsDisplayLabelDefined();
            if (displayLabelDefinedInOld && !displayLabelDefinedInNew)
                enumeratorChange.DisplayLabel().Set(oldEnumerator->GetInvariantDisplayLabel(), nullptr);
            else if (!displayLabelDefinedInOld && displayLabelDefinedInNew)
                enumeratorChange.DisplayLabel().Set(nullptr, newEnumerator->GetInvariantDisplayLabel());
            else if (displayLabelDefinedInOld && displayLabelDefinedInNew)
                {
                if (!oldEnumerator->GetInvariantDisplayLabel().EqualsIAscii(newEnumerator->GetInvariantDisplayLabel()))
                    enumeratorChange.DisplayLabel().Set(oldEnumerator->GetInvariantDisplayLabel(), newEnumerator->GetInvariantDisplayLabel());
                }

            if (!oldIt->second->GetInvariantDescription().EqualsIAscii(newEnumerator->GetInvariantDescription()))
                enumeratorChange.Description().Set(oldEnumerator->GetInvariantDescription(), newEnumerator->GetInvariantDescription());

            }
        else if (existsInOld && !existsInNew)
            {
            auto& change = changes.Add(ChangeType::Deleted, enumeratorName);
            change.Name().Set(oldIt->second->GetName(), nullptr);
            change.DisplayLabel().Set(oldIt->second->GetInvariantDisplayLabel(), nullptr);
            change.Description().Set(oldIt->second->GetInvariantDescription(), nullptr);
            if (oldEnumerator->IsInteger())
                change.Integer().Set(oldIt->second->GetInteger(), nullptr);
            else
                change.String().Set(oldIt->second->GetString(), nullptr);
            }
        else if (!existsInOld && existsInNew)
            {
            auto& change = changes.Add(ChangeType::New, enumeratorName);
            change.Name().Set(nullptr, newEnumerator->GetName());
            change.DisplayLabel().Set(nullptr, newEnumerator->GetInvariantDisplayLabel());
            change.Description().Set(nullptr, newEnumerator->GetInvariantDescription());
            if (newEnumerator->IsInteger())
                change.Integer().Set(nullptr, newEnumerator->GetInteger());
            else
                change.String().Set(nullptr, newEnumerator->GetString());
            }
        }

    return SUCCESS;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::CompareKindOfQuantity(KindOfQuantityChange& change, KindOfQuantityCR oldVal, KindOfQuantityCR newVal)
    {
    if (!oldVal.GetName().EqualsIAscii(newVal.GetName()))
        change.Name().Set(oldVal.GetName(), newVal.GetName());

    if (oldVal.GetIsDisplayLabelDefined() && !newVal.GetIsDisplayLabelDefined())
        change.DisplayLabel().Set(oldVal.GetInvariantDisplayLabel(), nullptr);
    else if (!oldVal.GetIsDisplayLabelDefined() && newVal.GetIsDisplayLabelDefined())
        change.DisplayLabel().Set(nullptr, newVal.GetInvariantDisplayLabel());
    else if (oldVal.GetIsDisplayLabelDefined() && newVal.GetIsDisplayLabelDefined())
        {
        if (!oldVal.GetInvariantDisplayLabel().EqualsIAscii(newVal.GetInvariantDisplayLabel()))
            change.DisplayLabel().Set(oldVal.GetInvariantDisplayLabel(), newVal.GetInvariantDisplayLabel());
        }

    if (!oldVal.GetInvariantDescription().EqualsIAscii(newVal.GetInvariantDescription()))
        change.Description().Set(oldVal.GetInvariantDescription(), newVal.GetInvariantDescription());

    if (oldVal.GetRelativeError() != newVal.GetRelativeError())
        change.RelativeError().Set(oldVal.GetRelativeError(), newVal.GetRelativeError());

    Utf8String oldPersUnitStr = oldVal.GetPersistenceUnit()->GetFullName();

    Utf8String newPersUnitStr = newVal.GetPersistenceUnit()->GetFullName();
    
    if (!oldPersUnitStr.EqualsIAscii(newPersUnitStr))
        change.PersistenceUnit().Set(oldPersUnitStr, newPersUnitStr);

    std::vector<Utf8String> oldPresUnits, newPresUnits;
    for (NamedFormat const& format : oldVal.GetPresentationFormats())
        {
        oldPresUnits.push_back(format.GetQualifiedName(oldVal.GetSchema()));
        }

    for (NamedFormat const& format : newVal.GetPresentationFormats())
        {
        newPresUnits.push_back(format.GetQualifiedName(newVal.GetSchema()));
        }

    const size_t oldPresUnitCount = oldPresUnits.size();
    const size_t newPresUnitCount = newPresUnits.size();
    const size_t maxPresUnitCount = std::max(oldPresUnitCount, newPresUnitCount);
    StringChanges& presUnitListChange = change.PresentationFormats();

    for (size_t i = 0; i < maxPresUnitCount; i++)
        {
        if (i < oldPresUnitCount && i < newPresUnitCount)
            {
            if (!oldPresUnits[i].EqualsIAscii(newPresUnits[i]))
                presUnitListChange.Add(ChangeType::Modified).Set(oldPresUnits[i], newPresUnits[i]);

            continue;
            }

        if (i >= oldPresUnitCount)
            {
            presUnitListChange.Add(ChangeType::New).Set(newPresUnits[i]);
            continue;
            }

        if (i >= newPresUnitCount)
            presUnitListChange.Add(ChangeType::Deleted).Set(oldPresUnits[i]);
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle  06/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::ComparePropertyCategory(PropertyCategoryChange& change, PropertyCategoryCR oldValue, PropertyCategoryCR newValue)
    {
    if (!oldValue.GetName().EqualsIAscii(newValue.GetName()))
        change.Name().Set(oldValue.GetName(), newValue.GetName());

    const bool displayLabelDefinedInOld = oldValue.GetIsDisplayLabelDefined();
    const bool displayLabelDefinedInNew = newValue.GetIsDisplayLabelDefined();
    if (displayLabelDefinedInOld && !displayLabelDefinedInNew)
        change.DisplayLabel().Set(oldValue.GetInvariantDisplayLabel(), nullptr);
    else if (!displayLabelDefinedInOld && displayLabelDefinedInNew)
        change.DisplayLabel().Set(nullptr, newValue.GetInvariantDisplayLabel());
    else if (displayLabelDefinedInOld && displayLabelDefinedInNew)
        {
        if (!oldValue.GetInvariantDisplayLabel().EqualsIAscii(newValue.GetInvariantDisplayLabel()))
            change.DisplayLabel().Set(oldValue.GetInvariantDisplayLabel(), newValue.GetInvariantDisplayLabel());
        }

    if (oldValue.GetDescription() != newValue.GetDescription())
        change.Description().Set(oldValue.GetDescription(), newValue.GetDescription());

    if (oldValue.GetPriority() != newValue.GetPriority())
        change.Priority().Set(oldValue.GetPriority(), newValue.GetPriority());

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle  02/2018
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::ComparePhenomenon(PhenomenonChange& change, PhenomenonCR oldVal, PhenomenonCR newVal)
    {
    if (!oldVal.GetName().EqualsIAscii(newVal.GetName()))
        change.Name().Set(oldVal.GetName(), newVal.GetName());

    if (oldVal.GetIsDisplayLabelDefined() && !newVal.GetIsDisplayLabelDefined())
        change.DisplayLabel().Set(oldVal.GetInvariantDisplayLabel(), nullptr);
    else if (!oldVal.GetIsDisplayLabelDefined() && newVal.GetIsDisplayLabelDefined())
        change.DisplayLabel().Set(nullptr, newVal.GetInvariantDisplayLabel());
    else if (oldVal.GetIsDisplayLabelDefined() && newVal.GetIsDisplayLabelDefined())
        {
        if (!oldVal.GetInvariantDisplayLabel().EqualsIAscii(newVal.GetInvariantDisplayLabel()))
            change.DisplayLabel().Set(oldVal.GetInvariantDisplayLabel(), newVal.GetInvariantDisplayLabel());
        }

    if (!oldVal.GetInvariantDescription().EqualsIAscii(newVal.GetInvariantDescription()))
        change.Description().Set(oldVal.GetInvariantDescription(), newVal.GetInvariantDescription());

    if (!oldVal.GetDefinition().EqualsIAscii(newVal.GetDefinition()))
        change.Definition().Set(oldVal.GetDefinition(), newVal.GetDefinition());

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle  02/2018
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::CompareUnitSystem(UnitSystemChange& change, UnitSystemCR oldVal, UnitSystemCR newVal)
    {
    if (!oldVal.GetName().EqualsIAscii(newVal.GetName()))
        change.Name().Set(oldVal.GetName(), newVal.GetName());

    if (oldVal.GetIsDisplayLabelDefined() && !newVal.GetIsDisplayLabelDefined())
        change.DisplayLabel().Set(oldVal.GetInvariantDisplayLabel(), nullptr);
    else if (!oldVal.GetIsDisplayLabelDefined() && newVal.GetIsDisplayLabelDefined())
        change.DisplayLabel().Set(nullptr, newVal.GetInvariantDisplayLabel());
    else if (oldVal.GetIsDisplayLabelDefined() && newVal.GetIsDisplayLabelDefined())
        {
        if (!oldVal.GetInvariantDisplayLabel().EqualsIAscii(newVal.GetInvariantDisplayLabel()))
            change.DisplayLabel().Set(oldVal.GetInvariantDisplayLabel(), newVal.GetInvariantDisplayLabel());
        }

    if (!oldVal.GetInvariantDescription().EqualsIAscii(newVal.GetInvariantDescription()))
        change.Description().Set(oldVal.GetInvariantDescription(), newVal.GetInvariantDescription());

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle  02/2018
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::CompareUnit(UnitChange& change, ECUnitCR oldVal, ECUnitCR newVal)
    {
    if (!oldVal.GetName().EqualsIAscii(newVal.GetName()))
        change.Name().Set(oldVal.GetName(), newVal.GetName());

    if (oldVal.GetIsDisplayLabelDefined() && !newVal.GetIsDisplayLabelDefined())
        change.DisplayLabel().Set(oldVal.GetInvariantDisplayLabel(), nullptr);
    else if (!oldVal.GetIsDisplayLabelDefined() && newVal.GetIsDisplayLabelDefined())
        change.DisplayLabel().Set(nullptr, newVal.GetInvariantDisplayLabel());
    else if (oldVal.GetIsDisplayLabelDefined() && newVal.GetIsDisplayLabelDefined())
        {
        if (!oldVal.GetInvariantDisplayLabel().EqualsIAscii(newVal.GetInvariantDisplayLabel()))
            change.DisplayLabel().Set(oldVal.GetInvariantDisplayLabel(), newVal.GetInvariantDisplayLabel());
        }

    if (!oldVal.GetInvariantDescription().EqualsIAscii(newVal.GetInvariantDescription()))
        change.Description().Set(oldVal.GetInvariantDescription(), newVal.GetInvariantDescription());

    if (!oldVal.GetDefinition().EqualsIAscii(newVal.GetDefinition()))
        change.Definition().Set(oldVal.GetDefinition(), newVal.GetDefinition());

    if (!oldVal.GetDefinition().EqualsIAscii(newVal.GetDefinition()))
        change.Definition().Set(oldVal.GetDefinition(), newVal.GetDefinition());

    {
    Nullable<double> oldNum;
    if (oldVal.HasNumerator())
        oldNum = oldVal.GetNumerator();

    Nullable<double> newNum;
    if (newVal.HasNumerator())
        newNum = newVal.GetNumerator();

    if (oldNum != newNum)
        change.Numerator().Set(oldNum, newNum);
    }

    {
    Nullable<double> oldDen;
    if (oldVal.HasDenominator())
        oldDen = oldVal.GetDenominator();

    Nullable<double> newDen;
    if (newVal.HasDenominator())
        newDen = newVal.GetDenominator();

    if (oldDen != newDen)
        change.Denominator().Set(oldDen, newDen);
    }

    {
    Nullable<double> oldOff;
    if (oldVal.HasOffset())
        oldOff = oldVal.GetOffset();

    Nullable<double> newOff;
    if (newVal.HasOffset())
        newOff = newVal.GetOffset();

    if (oldOff != newOff)
        change.Offset().Set(oldOff, newOff);
    }

    {
    PhenomenonCP oldPhen = oldVal.GetPhenomenon();
    PhenomenonCP newPhen = newVal.GetPhenomenon();
    if (!oldPhen->GetFullName().EqualsIAscii(newPhen->GetFullName()))
        change.Phenomenon().Set(oldPhen->GetFullName(), newPhen->GetFullName());
    }

    {
    Nullable<Utf8String> oldUs;
    if (oldVal.HasUnitSystem())
        oldUs = oldVal.GetUnitSystem()->GetFullName();

    Nullable<Utf8String> newUs;
    if (newVal.HasUnitSystem())
        newUs = newVal.GetUnitSystem()->GetFullName();

    if (oldUs != newUs)
        change.UnitSystem().Set(oldUs, newUs);
    }

    if (oldVal.IsConstant() != newVal.IsConstant())
        change.IsConstant().Set(oldVal.IsConstant(), newVal.IsConstant());

    Nullable<Utf8String> oldInvertingUnitName = oldVal.IsInvertedUnit() ? oldVal.GetInvertingUnit()->GetFullName() : nullptr;
    Nullable<Utf8String> newInvertingUnitName = newVal.IsInvertedUnit() ? newVal.GetInvertingUnit()->GetFullName() : nullptr;
    if (oldInvertingUnitName != newInvertingUnitName)
        change.InvertingUnit().Set(oldInvertingUnitName, newInvertingUnitName);

    return SUCCESS;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Kyle.Abramowitz  04/2018
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::CompareFormat(FormatChange& change, ECN::ECFormatCR oldVal, ECN::ECFormatCR newVal)
    {
    if (!oldVal.GetName().EqualsIAscii(newVal.GetName()))
        change.GetName().Set(oldVal.GetName(), newVal.GetName());

    {
    Nullable<Utf8String> oldLabel;
    if (oldVal.GetIsDisplayLabelDefined())
        oldLabel = oldVal.GetInvariantDisplayLabel();

    Nullable<Utf8String> newLabel;
    if (newVal.GetIsDisplayLabelDefined())
        newLabel = newVal.GetInvariantDisplayLabel();

    if (oldLabel != newLabel)
        change.GetDisplayLabel().Set(oldLabel, newLabel);
    }

    if (!oldVal.GetInvariantDescription().EqualsIAscii(newVal.GetInvariantDescription()))
        change.GetDescription().Set(oldVal.GetInvariantDescription(), newVal.GetInvariantDescription());

    {
    Formatting::NumericFormatSpecCP oldSpec = oldVal.HasNumeric() ? oldVal.GetNumericSpec() : nullptr;
    Formatting::NumericFormatSpecCP newSpec = newVal.HasNumeric() ? newVal.GetNumericSpec() : nullptr;
    if (SUCCESS != change.NumericSpec().SetFrom(oldSpec, newSpec))
        return ERROR;
    }

    {
    Formatting::CompositeValueSpecCP oldSpec = oldVal.HasComposite() ? oldVal.GetCompositeSpec() : nullptr;
    Formatting::CompositeValueSpecCP newSpec = newVal.HasComposite() ? newVal.GetCompositeSpec() : nullptr;
    if (SUCCESS != change.CompositeSpec().SetFrom(oldSpec, newSpec))
        return ERROR;
    }

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
            changes.Add(ChangeType::Deleted).Set(Utf8String(u));
        else if (!existInA && existInB)
            changes.Add(ChangeType::New).Set(Utf8String(u));
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
            changes.Add(ChangeType::Modified).Set(itorA->second->GetFullSchemaName(), itorB->second->GetFullSchemaName());
        else if (existInA && !existInB)
            changes.Add(ChangeType::Deleted).Set(itorA->second->GetFullSchemaName());
        else if (!existInA && existInB)
            changes.Add(ChangeType::New).Set(itorB->second->GetFullSchemaName());
        }
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::AppendECSchema(SchemaChange& change, ECSchemaCR schema)
    {
    change.Name().Set(schema.GetName());

    if (schema.GetIsDisplayLabelDefined())
        change.DisplayLabel().Set(schema.GetInvariantDisplayLabel());

    change.Description().Set(schema.GetInvariantDescription());

    change.Alias().Set(schema.GetAlias());
    change.VersionRead().Set(schema.GetVersionRead());
    change.VersionMinor().Set(schema.GetVersionMinor());
    change.VersionWrite().Set(schema.GetVersionWrite());

    change.ECVersion().Set((uint32_t) schema.GetECVersion());
    change.OriginalECXmlVersionMajor().Set(schema.GetOriginalECXmlVersionMajor());
    change.OriginalECXmlVersionMinor().Set(schema.GetOriginalECXmlVersionMinor());

    if ((change.GetChangeType() == ChangeType::Deleted && m_options.GetSchemaDeleteDetailLevel() == AppendDetailLevel::Partial) ||
        (change.GetChangeType() == ChangeType::New && m_options.GetSchemaNewDetailLevel() == AppendDetailLevel::Partial))
        return SUCCESS;

    for (ECClassCP classCP : schema.GetClasses())
        {
        ClassChange& classChange = change.Classes().Add(change.GetChangeType(), classCP->GetName().c_str());
        if (AppendECClass(classChange, *classCP) == ERROR)
            return ERROR;
        }

    for (ECEnumerationCP enumerationCP : schema.GetEnumerations())
        {
        ECEnumerationChange& enumChange = change.Enumerations().Add(change.GetChangeType(), enumerationCP->GetName().c_str());
        if (AppendECEnumeration(enumChange, *enumerationCP) == ERROR)
            return ERROR;
        }

    for (KindOfQuantityCP kindOfQuantityCP : schema.GetKindOfQuantities())
        {
        KindOfQuantityChange& koqChange = change.KindOfQuantities().Add(change.GetChangeType(), kindOfQuantityCP->GetName().c_str());
        if (AppendKindOfQuantity(koqChange, *kindOfQuantityCP) == ERROR)
            return ERROR;
        }

    for (PropertyCategoryCP cat : schema.GetPropertyCategories())
        {
        PropertyCategoryChange& catChange = change.PropertyCategories().Add(change.GetChangeType(), cat->GetName().c_str());
        if (SUCCESS != AppendPropertyCategory(catChange, *cat))
            return ERROR;
        }

    for (UnitSystemCP us : schema.GetUnitSystems())
        {
        UnitSystemChange& usChange = change.UnitSystems().Add(change.GetChangeType(), us->GetName().c_str());
        if (SUCCESS != AppendUnitSystem(usChange, *us))
            return ERROR;
        }

    for (PhenomenonCP ph : schema.GetPhenomena())
        {
        PhenomenonChange& phChange = change.Phenomena().Add(change.GetChangeType(), ph->GetName().c_str());
        if (SUCCESS != AppendPhenomenon(phChange, *ph))
            return ERROR;
        }

    for (ECUnitCP unit : schema.GetUnits())
        {
        UnitChange& unitChange = change.Units().Add(change.GetChangeType(), unit->GetName().c_str());
        if (SUCCESS != AppendUnit(unitChange, *unit))
            return ERROR;
        }

    for (ECFormatCP format : schema.GetFormats())
        {
        FormatChange& formatChange = change.Formats().Add(change.GetChangeType(), format->GetName().c_str());
        if (SUCCESS != AppendFormat(formatChange, *format))
            return ERROR;
        }

    if (AppendReferences(change.References(), schema.GetReferencedSchemas()) != SUCCESS)
        return ERROR;

    return AppendCustomAttributes(change.CustomAttributes(), schema);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::AppendECClass(ClassChange& change, ECClassCR v)
    {
    change.GetName().Set(v.GetName());
    if (v.GetIsDisplayLabelDefined())
        change.GetDisplayLabel().Set(v.GetInvariantDisplayLabel());

    change.GetDescription().Set(v.GetInvariantDescription());
    change.ClassModifier().Set(v.GetClassModifier());
    change.ClassType().Set(v.GetClassType());

    for (ECPropertyCP prop : v.GetProperties(false))
        {
        PropertyChange& propertyChange = change.Properties().Add(change.GetChangeType(), prop->GetName().c_str());

        if (AppendECProperty(propertyChange, *prop) == ERROR)
            return ERROR;
        }

    if (v.GetRelationshipClassCP() != nullptr)
        {
        if (AppendECRelationshipClass(change.GetRelationship(), *v.GetRelationshipClassCP()) == ERROR)
            return ERROR;
        }

    if (AppendBaseClasses(change.BaseClasses(), v.GetBaseClasses()) != SUCCESS)
        return ERROR;

    return AppendCustomAttributes(change.CustomAttributes(), v);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::AppendECRelationshipClass(RelationshipChange& change, ECRelationshipClassCR v)
    {
    change.StrengthDirection().Set(v.GetStrengthDirection());
    change.Strength().Set(v.GetStrength());
    if (AppendECRelationshipConstraint(change.Source(), v.GetSource()) == ERROR)
        return ERROR;

    if (AppendECRelationshipConstraint(change.Target(), v.GetTarget()) == ERROR)
        return ERROR;

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::AppendECRelationshipConstraint(ECRelationshipConstraintChange& change, ECRelationshipConstraintCR v)
    {
    change.RoleLabel().Set(v.GetRoleLabel());
    change.Multiplicity().Set(v.GetMultiplicity().ToString());
    change.IsPolymorphic().Set(v.GetIsPolymorphic());
    if (AppendECRelationshipConstraintClasses(change.ConstraintClasses(), v.GetConstraintClasses()) != SUCCESS)
        return ERROR;

    return AppendCustomAttributes(change.CustomAttributes(), v);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::AppendECRelationshipConstraintClasses(ECRelationshipConstraintClassChanges& changes, ECRelationshipConstraintClassList const& v)
    {
    for (ECClassCP constraintClass : v)
        {
        changes.Add(changes.GetChangeType()).Set(Utf8String(constraintClass->GetFullName()));
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::AppendECEnumeration(ECEnumerationChange& enumerationChange, ECEnumerationCR v)
    {
    if (v.GetIsDisplayLabelDefined())
        enumerationChange.DisplayLabel().Set(v.GetInvariantDisplayLabel());

    enumerationChange.Description().Set(v.GetInvariantDescription());
    enumerationChange.IsStrict().Set(v.GetIsStrict());
    enumerationChange.TypeName().Set(v.GetTypeName());
    for (ECEnumeratorCP enumeratorCP : v.GetEnumerators())
        {
        ECEnumeratorChange& enumeratorChange = enumerationChange.Enumerators().Add(enumerationChange.GetChangeType(), enumeratorCP->GetName().c_str());
        enumeratorChange.Name().Set(enumeratorCP->GetName());

        if (enumeratorCP->IsInteger())
            enumeratorChange.Integer().Set(enumeratorCP->GetInteger());

        if (enumeratorCP->IsString())
            enumeratorChange.String().Set(enumeratorCP->GetString());

        if (enumeratorCP->GetIsDisplayLabelDefined())
            enumeratorChange.DisplayLabel().Set(enumeratorCP->GetInvariantDisplayLabel());

        enumeratorChange.Description().Set(enumeratorCP->GetInvariantDescription());
        }
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::AppendKindOfQuantity(KindOfQuantityChange& change, KindOfQuantityCR koq)
    {
    if (koq.GetIsDisplayLabelDefined())
        change.DisplayLabel().Set(koq.GetInvariantDisplayLabel());

    change.Description().Set(koq.GetInvariantDescription());
    change.PersistenceUnit().Set(koq.GetPersistenceUnit()->GetFullName());
    change.RelativeError().Set(koq.GetRelativeError());
    for (NamedFormat const& format : koq.GetPresentationFormats())
        change.PresentationFormats().Add(change.GetChangeType()).Set(format.GetName());

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle  06/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::AppendPropertyCategory(PropertyCategoryChange& catChange, PropertyCategoryCR cat)
    {
    catChange.Name().Set(cat.GetName());
    catChange.DisplayLabel().Set(cat.GetInvariantDisplayLabel());
    catChange.Description().Set(cat.GetDescription());
    catChange.Priority().Set(cat.GetPriority());
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle  02/2018
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::AppendPhenomenon(PhenomenonChange& change, PhenomenonCR phen)
    {
    if (phen.GetIsDisplayLabelDefined())
        change.DisplayLabel().Set(phen.GetInvariantDisplayLabel());

    change.Description().Set(phen.GetInvariantDescription());
    change.Definition().Set(phen.GetDefinition());
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle  02/2018
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::AppendUnitSystem(UnitSystemChange& change, UnitSystemCR system)
    {
    if (system.GetIsDisplayLabelDefined())
        change.DisplayLabel().Set(system.GetInvariantDisplayLabel());

    change.Description().Set(system.GetInvariantDescription());
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle  02/2018
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::AppendUnit(UnitChange& change, ECUnitCR unit)
    {
    if (unit.GetIsDisplayLabelDefined())
        change.DisplayLabel().Set(unit.GetInvariantDisplayLabel());

    change.Description().Set(unit.GetInvariantDescription());
    change.Definition().Set(unit.GetDefinition());
    if (unit.HasNumerator())
        change.Numerator().Set(unit.GetNumerator());

    if (unit.HasDenominator())
        change.Denominator().Set(unit.GetDenominator());

    if (unit.HasOffset())
        change.Offset().Set(unit.GetOffset());

    PhenomenonCP phen = unit.GetPhenomenon();
    if (phen != nullptr)
        change.Phenomenon().Set(phen->GetFullName());

    if (unit.HasUnitSystem())
        change.UnitSystem().Set(unit.GetUnitSystem()->GetFullName());

    change.IsConstant().Set(unit.IsConstant());

    if (unit.IsInvertedUnit())
        {
        Nullable<Utf8String> invertingUnitName = unit.GetInvertingUnit() != nullptr ? unit.GetInvertingUnit()->GetFullName() : nullptr;
        change.InvertingUnit().Set(invertingUnitName);
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Kyle.Abramowitz     04/2018
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::AppendFormat(FormatChange& change, ECFormatCR format)
    {
    if (format.GetIsDisplayLabelDefined())
        change.GetDisplayLabel().Set(format.GetInvariantDisplayLabel());

    change.GetDescription().Set(format.GetInvariantDescription());
    if (format.HasNumeric())
        {
        Formatting::NumericFormatSpecCP num = format.GetNumericSpec();
        if (SUCCESS != change.NumericSpec().SetFrom(*num))
            return ERROR;
        }

    if (format.HasComposite())
        {
        Formatting::CompositeValueSpecCP spec = format.GetCompositeSpec();
        if (SUCCESS != change.CompositeSpec().SetFrom(*spec))
            return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::AppendECProperty(PropertyChange& propertyChange, ECPropertyCR v)
    {
    propertyChange.GetName().Set(v.GetName());
    if (v.GetIsDisplayLabelDefined())
        propertyChange.GetDisplayLabel().Set(v.GetInvariantDisplayLabel());

    propertyChange.GetDescription().Set(v.GetInvariantDescription());
    propertyChange.GetTypeName().Set(v.GetTypeName());

    propertyChange.IsReadonly().Set(v.GetIsReadOnly());
    if (v.IsPriorityLocallyDefined())
        propertyChange.GetPriority().Set(v.GetPriority());

    if (v.GetKindOfQuantity() != nullptr)
        propertyChange.GetKindOfQuantity().Set(v.GetKindOfQuantity()->GetFullName());

    if (v.GetCategory() != nullptr)
        propertyChange.GetCategory().Set(v.GetCategory()->GetFullName());

    if (NavigationECPropertyCP prop = v.GetAsNavigationProperty())
        {
        propertyChange.IsNavigation().Set(true);
        NavigationPropertyChange& navigationChange = propertyChange.GetNavigation();
        navigationChange.Direction().Set(prop->GetDirection());
        if (prop->GetRelationshipClass())
            navigationChange.Relationship().Set(Utf8String(prop->GetRelationshipClass()->GetFullName()));
        }
    else if (v.GetIsPrimitive())
        {
        propertyChange.IsPrimitive().Set(true);
        auto primitiveProp = v.GetAsPrimitiveProperty();

        propertyChange.GetExtendedTypeName().Set(primitiveProp->GetExtendedTypeName());
        if (primitiveProp->GetEnumeration())
            propertyChange.GetEnumeration().Set(primitiveProp->GetEnumeration()->GetFullName());

        if (primitiveProp->GetKindOfQuantity())
            propertyChange.GetKindOfQuantity().Set(primitiveProp->GetKindOfQuantity()->GetFullName());
        }
    else if (v.GetIsStruct())
        {
        propertyChange.IsStruct().Set(true);
        }
    else if (v.GetIsStructArray())
        {
        propertyChange.IsStructArray().Set(true);
        }
    else if (v.GetIsPrimitiveArray())
        {
        propertyChange.IsPrimitiveArray().Set(true);
        auto primitivePropArray = v.GetAsPrimitiveArrayProperty();
        propertyChange.GetExtendedTypeName().Set(primitivePropArray->GetExtendedTypeName());
        if (primitivePropArray->GetKindOfQuantity())
            propertyChange.GetKindOfQuantity().Set(primitivePropArray->GetKindOfQuantity()->GetFullName());
        }
    else
        return ERROR;


    if (v.IsMaximumValueDefined())
        {
        ECValue maxV;
        if (ECObjectsStatus::Success != v.GetMaximumValue(maxV))
            return ERROR;

        propertyChange.GetMaximumValue().Set(maxV);
        }

    if (v.IsMinimumValueDefined())
        {
        ECValue minV;
        if (ECObjectsStatus::Success != v.GetMinimumValue(minV))
            return ERROR;

        propertyChange.GetMinimumValue().Set(minV);
        }

    if (v.GetIsArray())
        {
        auto arrayProp = v.GetAsArrayProperty();
        propertyChange.GetArray().MaxOccurs().Set(arrayProp->GetStoredMaxOccurs());
        propertyChange.GetArray().MinOccurs().Set(arrayProp->GetMinOccurs());
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::AppendBaseClasses(BaseClassChanges& changes, ECBaseClassesList const& baseClasses)
    {
    const ChangeType state = changes.GetChangeType();
    for (ECClassCP baseClassCP : baseClasses)
        changes.Add(state).Set(Utf8String(baseClassCP->GetFullName()));

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::AppendReferences(ReferenceChanges& changes, ECSchemaReferenceListCR references)
    {
    const ChangeType state = changes.GetChangeType();
    for (auto& referenceCP : references)
        changes.Add(state).Set(referenceCP.first.GetFullSchemaName());

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
    if (change.GetChangeType() == ChangeType::Deleted)
        str += "-";
    else if (change.GetChangeType() == ChangeType::New)
        str += "+";
    else if (change.GetChangeType() == ChangeType::Modified)
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
            case SystemId::ArrayProperty: return "ArrayProperty";
            case SystemId::BaseClass: return "BaseClass";
            case SystemId::BaseClasses: return "BaseClasses";
            case SystemId::Classes: return "Classes";
            case SystemId::Class: return "Class";
            case SystemId::ClassModifier: return "ClassModifier";
            case SystemId::CompositeIncludeZero: return "CompositeIncludeZero";
            case SystemId::CompositeSpacer: return "CompositeSpacer";
            case SystemId::CompositeMajorUnit: return "CompositeMajorUnit";
            case SystemId::CompositeMajorLabel: return "CompositeMajorLabel";
            case SystemId::CompositeMiddleUnit: return "CompositeMiddleUnit";
            case SystemId::CompositeMiddleLabel: return "CompositeMiddleLabel";
            case SystemId::CompositeMinorUnit: return "CompositeMinorUnit";
            case SystemId::CompositeMinorLabel: return "CompositeMinorLabel";
            case SystemId::CompositeSubUnit: return "CompositeSubUnit";
            case SystemId::CompositeSubLabel: return "CompositeSubLabel";
            case SystemId::CompositeValueSpec: return "CompositeValueSpec";
            case SystemId::ConstraintClass: return "ConstraintClass";
            case SystemId::ConstraintClasses: return "ConstraintClasses";
            case SystemId::Constraint: return "Constraint";
            case SystemId::CustomAttribute: return "CustomAttribute";
            case SystemId::CustomAttributes: return "CustomAttributes";
            case SystemId::DecimalPrecision: return "DecimalPrecision";
            case SystemId::DecimalSeparator: return "DecimalSeparator";
            case SystemId::Description: return "Description";
            case SystemId::Direction: return "Direction";
            case SystemId::DisplayLabel: return "DisplayLabel";
            case SystemId::ECVersion: return "ECVersion";
            case SystemId::Enumeration: return "Enumeration";
            case SystemId::Enumerations: return "Enumerations";
            case SystemId::Enumerator: return "Enumerator";
            case SystemId::Enumerators: return "Enumerators";
            case SystemId::ExtendedTypeName: return "ExtendedTypeName";
            case SystemId::Format: return "Format";
            case SystemId::Formats: return "Formats";
            case SystemId::FormatTraits: return "FormatTraits";
            case SystemId::FractionalPrecision: return "FractionalPrecision";
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
            case SystemId::KoqPresentationFormats: return "KoqPresentationFormats";
            case SystemId::KoqRelativeError: return "KoqRelativeError";
            case SystemId::MaximumLength: return "MaximumLength";
            case SystemId::MaximumValue: return "MaximumValue";
            case SystemId::MaxOccurs: return "MaxOccurs";
            case SystemId::MinimumLength: return "MinimumLength";
            case SystemId::MinimumValue: return "MinimumValue";
            case SystemId::MinOccurs: return "MinOccurs";
            case SystemId::MinWidth: return "MinWidth";
            case SystemId::Multiplicity: return "Multiplicity";
            case SystemId::Name: return "Name";
            case SystemId::NavigationProperty: return "NavigationProperty";
            case SystemId::NumericFormatSpec: return "NumericFormatSpec";
            case SystemId::OriginalECXmlVersionMajor: return "OriginalECXmlVersionMajor";
            case SystemId::OriginalECXmlVersionMinor: return "OriginalECXmlVersionMinor";
            case SystemId::Phenomena: return "Phenomena";
            case SystemId::Phenomenon: return "Phenomenon";
            case SystemId::PhenomenonDefinition: return "PhenomenonDefinition";
            case SystemId::PresentationType: return "PresentationType";
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
            case SystemId::RelationshipName: return "RelationshipName";
            case SystemId::RoleLabel: return "RoleLabel";
            case SystemId::RoundingFactor: return "RoundingFactor";
            case SystemId::Schema: return "Schema";
            case SystemId::Schemas: return "Schemas";
            case SystemId::ScientificType: return "ScientificType";
            case SystemId::ShowSignOption: return "ShowSignOption";
            case SystemId::Source: return "Source";
            case SystemId::StationSeparator: return "StationSeparator";
            case SystemId::StationOffsetSize: return "StationOffsetSize";
            case SystemId::StrengthDirection: return "StrengthDirection";
            case SystemId::StrengthType: return "StrengthType";
            case SystemId::String: return "String";
            case SystemId::Target: return "Target";
            case SystemId::ThousandSeparator: return "ThousandSeparator";
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
            case SystemId::UomSeparator: return "UomSeparator";
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
void CompositeECChange::_WriteToString(Utf8StringR str, int currentIndex, int indentSize) const
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
bool CompositeECChange::_IsEmpty() const
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
void CompositeECChange::_Optimize()
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
//ECPropertyValueChange
//======================================================================================>
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECPropertyValueChange::Set(ECValueCR value)
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
                BeAssert(false && "Geometry not supported type");
                return ERROR;
                }
            case PRIMITIVETYPE_Binary:
                {
                Binary v;
                v.CopyFrom(value);
                return GetBinary()->Set(std::move(v));
                }
            case PRIMITIVETYPE_Boolean:
                return GetBoolean()->Set(value.GetBoolean());
            case PRIMITIVETYPE_DateTime:
                return GetDateTime()->Set(value.GetDateTime());
            case PRIMITIVETYPE_Double:
                return GetDouble()->Set(value.GetDouble());
            case PRIMITIVETYPE_Integer:
                return GetInteger()->Set(value.GetInteger());
            case PRIMITIVETYPE_Long:
                return GetLong()->Set(value.GetLong());
            case PRIMITIVETYPE_Point2d:
                return GetPoint2d()->Set(value.GetPoint2d());
            case PRIMITIVETYPE_Point3d:
                return GetPoint3d()->Set(value.GetPoint3d());
            case PRIMITIVETYPE_String:
                return GetString()->Set(Utf8String(value.GetUtf8CP()));
        }

    return ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECPropertyValueChange::Set(ECValueCR oldValue, ECValueCR newValue)
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
                return GetBinary()->Set(Converter<Binary>::Copy(oldValue), Converter<Binary>::Copy(newValue));
            case PRIMITIVETYPE_Boolean:
                return GetBoolean()->Set(Converter<bool>::Copy(oldValue), Converter<bool>::Copy(newValue));
            case PRIMITIVETYPE_DateTime:
                return GetDateTime()->Set(Converter<DateTime>::Copy(oldValue), Converter<DateTime>::Copy(newValue));
            case PRIMITIVETYPE_Double:
                return GetDouble()->Set(Converter<double>::Copy(oldValue), Converter<double>::Copy(newValue));
            case PRIMITIVETYPE_Integer:
                return GetInteger()->Set(Converter<int>::Copy(oldValue), Converter<int>::Copy(newValue));
            case PRIMITIVETYPE_Long:
                return GetLong()->Set(Converter<int64_t>::Copy(oldValue), Converter<int64_t>::Copy(newValue));
            case PRIMITIVETYPE_Point2d:
                return GetPoint2d()->Set(Converter<DPoint2d>::Copy(oldValue), Converter<DPoint2d>::Copy(newValue));
            case PRIMITIVETYPE_Point3d:
                return GetPoint3d()->Set(Converter<DPoint3d>::Copy(oldValue), Converter<DPoint3d>::Copy(newValue));
            case PRIMITIVETYPE_String:
                return GetString()->Set(Converter<Utf8String>::Copy(oldValue), Converter<Utf8String>::Copy(newValue));
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
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
bool ECPropertyValueChange::_IsEmpty() const
    {
    if (auto parent = GetParent())
        {
        if (parent->GetSystemId() == SystemId::CustomAttributes && GetChangeType() != ChangeType::Modified)
            return false;
        }

    if (m_value != nullptr)
        return m_value->IsEmpty();

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
                m_value = std::unique_ptr<ECChange>(new BinaryChange(GetChangeType(), SystemId::PropertyValue, this, GetId())); break;
            case ECN::PRIMITIVETYPE_Boolean:
                m_value = std::unique_ptr<ECChange>(new BooleanChange(GetChangeType(), SystemId::PropertyValue, this, GetId())); break;
            case ECN::PRIMITIVETYPE_DateTime:
                m_value = std::unique_ptr<ECChange>(new DateTimeChange(GetChangeType(), SystemId::PropertyValue, this, GetId())); break;
            case ECN::PRIMITIVETYPE_Double:
                m_value = std::unique_ptr<ECChange>(new DoubleChange(GetChangeType(), SystemId::PropertyValue, this, GetId())); break;
            case ECN::PRIMITIVETYPE_IGeometry:
            {
            LOG.errorv("ECSchemaComparer: Changes in ECProperties of type IGeometry are not supported.");
            return ERROR;
            }
            case ECN::PRIMITIVETYPE_Integer:
                m_value = std::unique_ptr<ECChange>(new Int32Change(GetChangeType(), SystemId::PropertyValue, this, GetId())); break;
            case ECN::PRIMITIVETYPE_Long:
                m_value = std::unique_ptr<ECChange>(new Int64Change(GetChangeType(), SystemId::PropertyValue, this, GetId())); break;
            case ECN::PRIMITIVETYPE_Point2d:
                m_value = std::unique_ptr<ECChange>(new Point2dChange(GetChangeType(), SystemId::PropertyValue, this, GetId())); break;
            case ECN::PRIMITIVETYPE_Point3d:
                m_value = std::unique_ptr<ECChange>(new Point3dChange(GetChangeType(), SystemId::PropertyValue, this, GetId())); break;
            case ECN::PRIMITIVETYPE_String:
                m_value = std::unique_ptr<ECChange>(new StringChange(GetChangeType(), SystemId::PropertyValue, this, GetId())); break;
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
ECPropertyValueChange::ECPropertyValueChange(ChangeType state, SystemId systemId, ECChange const* parent, Utf8CP accessString)
    : ECChange(state, SystemId::PropertyValue, parent, accessString), m_type(static_cast<PrimitiveType>(0))
    {
    BeAssert(!Utf8String::IsNullOrEmpty(accessString) && "access string must not be empty for ECPropertyValueChange");
    BeAssert(systemId == GetSystemId());
    }

//***********************************************************************
// NumericFormatSpecChange
//***********************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle  05/2018
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus NumericFormatSpecChange::SetFrom(Formatting::NumericFormatSpecCR spec)
    {
    if (GetChangeType() == ChangeType::Modified)
        {
        BeAssert(false && "Wrong overload for ChangeType::Modified");
        return ERROR;
        }

    PresentationType().Set(Formatting::Utils::GetPresentationTypeString(spec.GetPresentationType()));
    ScientificType().Set(Formatting::Utils::GetScientificTypeString(spec.GetScientificType()));

    if (spec.HasRoundingFactor())
        RoundingFactor().Set(spec.GetRoundingFactor());

    if (spec.HasPrecision())
        {
        DecimalPrecision().Set(Formatting::Utils::DecimalPrecisionToInt(spec.GetDecimalPrecision()));
        FractionalPrecision().Set(Formatting::Utils::FractionalPrecisionDenominator(spec.GetFractionalPrecision()));
        }

    if (spec.HasMinWidth())
        MinWidth().Set(spec.GetMinWidth());

    if (spec.HasSignOption())
        ShowSignOption().Set(Formatting::Utils::GetSignOptionString(spec.GetSignOption()));

    if (spec.HasFormatTraits())
        FormatTraits().Set(spec.GetFormatTraitsString());

    if (spec.HasDecimalSeparator())
        DecimalSeparator().Set(Utf8String(1, spec.GetDecimalSeparator()));

    if (spec.HasThousandsSeparator())
        ThousandsSeparator().Set(Utf8String(1, spec.GetThousandSeparator()));

    if (spec.HasUomSeparator())
        UomSeparator().Set(Utf8String(spec.GetUomSeparator()));

    if (spec.HasStationSeparator())
        StationSeparator().Set(Utf8String(1, spec.GetStationSeparator()));

    StationOffsetSize().Set(spec.GetStationOffsetSize());

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle  05/2018
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus NumericFormatSpecChange::SetFrom(Formatting::NumericFormatSpecCP oldSpec, Formatting::NumericFormatSpecCP newSpec)
    {
    if (GetChangeType() != ChangeType::Modified)
        {
        BeAssert(false && "Wrong overload for ChangeType::New or ChangeType::Deleted");
        return ERROR;
        }

    {
    Nullable<Utf8String> oldVal = oldSpec != nullptr ? Formatting::Utils::GetPresentationTypeString(oldSpec->GetPresentationType()) : nullptr;
    Nullable<Utf8String> newVal = newSpec != nullptr ? Formatting::Utils::GetPresentationTypeString(newSpec->GetPresentationType()) : nullptr;
    if (oldVal != newVal)
        PresentationType().Set(oldVal, newVal);
    }

    {
    Nullable<Utf8String> oldVal = oldSpec != nullptr ? Formatting::Utils::GetScientificTypeString(oldSpec->GetScientificType()) : nullptr;
    Nullable<Utf8String> newVal = newSpec != nullptr ? Formatting::Utils::GetScientificTypeString(newSpec->GetScientificType()) : nullptr;
    if (oldVal != newVal)
        ScientificType().Set(oldVal, newVal);
    }

    {
    Nullable<double> oldVal = oldSpec != nullptr && oldSpec->HasRoundingFactor() ? oldSpec->GetRoundingFactor() : Nullable<double>();
    Nullable<double> newVal = newSpec != nullptr && newSpec->HasRoundingFactor() ? newSpec->GetRoundingFactor() : Nullable<double>();
    if (oldVal != newVal)
        RoundingFactor().Set(oldVal, newVal);
    }

    {
    Nullable<int32_t> oldDecPrec;
    Nullable<int32_t> oldFractPrec;
    if (oldSpec != nullptr && oldSpec->HasPrecision())
        {
        oldDecPrec = Formatting::Utils::DecimalPrecisionToInt(oldSpec->GetDecimalPrecision());
        oldFractPrec = Formatting::Utils::FractionalPrecisionDenominator(oldSpec->GetFractionalPrecision());
        }

    Nullable<int32_t> newDecPrec;
    Nullable<int32_t> newFractPrec;
    if (newSpec != nullptr && newSpec->HasPrecision())
        {
        newDecPrec = Formatting::Utils::DecimalPrecisionToInt(newSpec->GetDecimalPrecision());
        newFractPrec = Formatting::Utils::FractionalPrecisionDenominator(newSpec->GetFractionalPrecision());
        }

    if (oldDecPrec != newDecPrec)
        DecimalPrecision().Set(oldDecPrec, newDecPrec);

    if (oldFractPrec != oldFractPrec)
        FractionalPrecision().Set(oldFractPrec, oldFractPrec);
    }

    {
    Nullable<uint32_t> oldVal;
    if (oldSpec != nullptr && oldSpec->HasMinWidth())
        oldVal = (uint32_t) oldSpec->GetMinWidth();

    Nullable<uint32_t> newVal;
    if (newSpec != nullptr && newSpec->HasMinWidth())
        newVal = (uint32_t) newSpec->GetMinWidth();

    if (oldVal != newVal)
        MinWidth().Set(oldVal, newVal);
    }

    {
    Nullable<Utf8String> oldVal;
    if (oldSpec != nullptr && oldSpec->HasSignOption())
        oldVal = Formatting::Utils::GetSignOptionString(oldSpec->GetSignOption());

    Nullable<Utf8String> newVal;
    if (newSpec != nullptr && newSpec->HasSignOption())
        newVal = Formatting::Utils::GetSignOptionString(newSpec->GetSignOption());

    if (oldVal != newVal)
        ShowSignOption().Set(oldVal, newVal);
    }

    {
    Nullable<Utf8String> oldVal;
    if (oldSpec != nullptr && oldSpec->HasFormatTraits())
        oldVal = oldSpec->GetFormatTraitsString();

    Nullable<Utf8String> newVal;
    if (newSpec != nullptr && newSpec->HasFormatTraits())
        newVal = newSpec->GetFormatTraitsString();

    if (oldVal != newVal)
        FormatTraits().Set(oldVal, newVal);
    }

    {
    Nullable<Utf8String> oldVal;
    if (oldSpec != nullptr && oldSpec->HasDecimalSeparator())
        oldVal = Utf8String(1, oldSpec->GetDecimalSeparator());

    Nullable<Utf8String> newVal;
    if (newSpec != nullptr && newSpec->HasDecimalSeparator())
        newVal = Utf8String(1, newSpec->GetDecimalSeparator());

    if (oldVal != newVal)
        DecimalSeparator().Set(oldVal, newVal);
    }

    {
    Nullable<Utf8String> oldVal;
    if (oldSpec != nullptr && oldSpec->HasThousandsSeparator())
        oldVal = Utf8String(1, oldSpec->GetThousandSeparator());

    Nullable<Utf8String> newVal;
    if (newSpec != nullptr && newSpec->HasThousandsSeparator())
        newVal = Utf8String(1, newSpec->GetThousandSeparator());

    if (oldVal != newVal)
        ThousandsSeparator().Set(oldVal, newVal);
    }

    {
    Nullable<Utf8String> oldVal;
    if (oldSpec != nullptr && oldSpec->HasUomSeparator())
        oldVal = Utf8String(oldSpec->GetUomSeparator());

    Nullable<Utf8String> newVal;
    if (newSpec != nullptr && newSpec->HasUomSeparator())
        newVal = Utf8String(newSpec->GetUomSeparator());

    if (oldVal != newVal)
        UomSeparator().Set(oldVal, newVal);
    }

    {
    Nullable<Utf8String> oldVal;
    if (oldSpec != nullptr && oldSpec->HasStationSeparator())
        oldVal = Utf8String(1, oldSpec->GetStationSeparator());

    Nullable<Utf8String> newVal;
    if (newSpec != nullptr && newSpec->HasStationSeparator())
        newVal = Utf8String(1, newSpec->GetStationSeparator());

    if (oldVal != newVal)
        StationSeparator().Set(oldVal, newVal);
    }

    {
    Nullable<uint32_t> oldVal;
    if (oldSpec != nullptr)
        oldVal = oldSpec->GetStationOffsetSize();

    Nullable<uint32_t> newVal;
    if (newSpec != nullptr)
        newVal = newSpec->GetStationOffsetSize();

    if (oldVal != newVal)
        StationOffsetSize().Set(oldVal, newVal);
    }

    return SUCCESS;
    }

//***********************************************************************
// CompositeValueSpecChange
//***********************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle  05/2018
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus CompositeValueSpecChange::SetFrom(Formatting::CompositeValueSpecCR spec)
    {
    if (GetChangeType() == ChangeType::Modified)
        {
        BeAssert(false && "Wrong overload for ChangeType::Modified");
        return ERROR;
        }

    if (spec.HasSpacer())
        Spacer().Set(spec.GetSpacer());

    IncludeZero().Set(spec.IsIncludeZero());

    if (spec.HasMajorUnit())
        {
        BeAssert(dynamic_cast<ECUnitCP> (spec.GetMajorUnit()) != nullptr);
        MajorUnit().Set(((ECUnitCP) spec.GetMajorUnit())->GetFullName());
        }

    if (spec.HasMajorLabel())
        GetMajorLabel().Set(spec.GetMajorLabel());

    if (spec.HasMiddleUnit())
        {
        BeAssert(dynamic_cast<ECUnitCP> (spec.GetMiddleUnit()) != nullptr);
        GetMiddleUnit().Set(((ECUnitCP) spec.GetMiddleUnit())->GetFullName());
        }

    if (spec.HasMiddleLabel())
        GetMiddleLabel().Set(spec.GetMiddleLabel());

    if (spec.HasMinorUnit())
        {
        BeAssert(dynamic_cast<ECUnitCP> (spec.GetMinorUnit()) != nullptr);
        GetMinorUnit().Set(((ECUnitCP) spec.GetMinorUnit())->GetFullName());
        }

    if (spec.HasMinorLabel())
        GetMinorLabel().Set(spec.GetMinorLabel());

    if (spec.HasSubUnit())
        {
        BeAssert(dynamic_cast<ECUnitCP> (spec.GetSubUnit()) != nullptr);
        GetSubUnit().Set(((ECUnitCP) spec.GetSubUnit())->GetFullName());
        }

    if (spec.HasSubLabel())
        GetSubLabel().Set(spec.GetSubLabel());

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle  05/2018
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus CompositeValueSpecChange::SetFrom(Formatting::CompositeValueSpecCP oldSpec, Formatting::CompositeValueSpecCP newSpec)
    {
    if (GetChangeType() != ChangeType::Modified)
        {
        BeAssert(false && "Wrong overload for ChangeType::New or ChangeType::Deleted");
        return ERROR;
        }

    {
    Nullable<Utf8String> oldVal;
    if (oldSpec != nullptr && oldSpec->HasSpacer())
        oldVal = oldSpec->GetSpacer();

    Nullable<Utf8String> newVal;
    if (newSpec != nullptr && newSpec->HasSpacer())
        newVal = newSpec->GetSpacer();

    if (oldVal != newVal)
        Spacer().Set(oldVal, newVal);
    }

    {
    Nullable<bool> oldVal = oldSpec != nullptr ? oldSpec->IsIncludeZero() : nullptr;
    Nullable<bool> newVal = newSpec != nullptr ? newSpec->IsIncludeZero() : nullptr;
    if (oldVal != newVal)
        IncludeZero().Set(oldVal, newVal);
    }

    {
    Nullable<Utf8String> oldVal;
    if (oldSpec != nullptr && oldSpec->HasMajorUnit())
        {
        BeAssert(dynamic_cast<ECUnitCP> (oldSpec->GetMajorUnit()) != nullptr);
        oldVal = ((ECUnitCP) oldSpec->GetMajorUnit())->GetFullName();
        }

    Nullable<Utf8String> newVal;
    if (newSpec != nullptr && newSpec->HasMajorUnit())
        {
        BeAssert(dynamic_cast<ECUnitCP> (newSpec->GetMajorUnit()) != nullptr);
        newVal = ((ECUnitCP) newSpec->GetMajorUnit())->GetFullName();
        }

    if (oldVal != newVal)
        MajorUnit().Set(oldVal, newVal);
    }

    {
    Nullable<Utf8String> oldVal;
    if (oldSpec != nullptr && oldSpec->HasMajorLabel())
        oldVal = oldSpec->GetMajorLabel();

    Nullable<Utf8String> newVal;
    if (newSpec != nullptr && newSpec->HasMajorLabel())
        newVal = newSpec->GetMajorLabel();

    if (oldVal != newVal)
        GetMajorLabel().Set(oldVal, newVal);
    }

    {
    Nullable<Utf8String> oldVal;
    if (oldSpec != nullptr && oldSpec->HasMiddleUnit())
        {
        BeAssert(dynamic_cast<ECUnitCP> (oldSpec->GetMiddleUnit()) != nullptr);
        oldVal = ((ECUnitCP) oldSpec->GetMiddleUnit())->GetFullName();
        }

    Nullable<Utf8String> newVal;
    if (newSpec != nullptr && newSpec->HasMiddleUnit())
        {
        BeAssert(dynamic_cast<ECUnitCP> (newSpec->GetMiddleUnit()) != nullptr);
        newVal = ((ECUnitCP) newSpec->GetMiddleUnit())->GetFullName();
        }

    if (oldVal != newVal)
        GetMiddleUnit().Set(oldVal, newVal);
    }

    {
    Nullable<Utf8String> oldVal;
    if (oldSpec != nullptr && oldSpec->HasMiddleLabel())
        oldVal = oldSpec->GetMiddleLabel();

    Nullable<Utf8String> newVal;
    if (newSpec != nullptr && newSpec->HasMiddleLabel())
        newVal = newSpec->GetMiddleLabel();

    if (oldVal != newVal)
        GetMiddleLabel().Set(oldVal, newVal);
    }

    {
    Nullable<Utf8String> oldVal;
    if (oldSpec != nullptr && oldSpec->HasMinorUnit())
        {
        BeAssert(dynamic_cast<ECUnitCP> (oldSpec->GetMinorUnit()) != nullptr);
        oldVal = ((ECUnitCP) oldSpec->GetMinorUnit())->GetFullName();
        }

    Nullable<Utf8String> newVal;
    if (newSpec != nullptr && newSpec->HasMinorUnit())
        {
        BeAssert(dynamic_cast<ECUnitCP> (newSpec->GetMinorUnit()) != nullptr);
        newVal = ((ECUnitCP) newSpec->GetMinorUnit())->GetFullName();
        }

    if (oldVal != newVal)
        GetMinorUnit().Set(oldVal, newVal);
    }

    {
    Nullable<Utf8String> oldVal;
    if (oldSpec != nullptr && oldSpec->HasMinorLabel())
        oldVal = oldSpec->GetMinorLabel();

    Nullable<Utf8String> newVal;
    if (newSpec != nullptr && newSpec->HasMinorLabel())
        newVal = newSpec->GetMinorLabel();

    if (oldVal != newVal)
        GetMinorLabel().Set(oldVal, newVal);
    }

    {
    Nullable<Utf8String> oldVal;
    if (oldSpec != nullptr && oldSpec->HasSubUnit())
        {
        BeAssert(dynamic_cast<ECUnitCP> (oldSpec->GetSubUnit()) != nullptr);
        oldVal = ((ECUnitCP) oldSpec->GetSubUnit())->GetFullName();
        }

    Nullable<Utf8String> newVal;
    if (newSpec != nullptr && newSpec->HasSubUnit())
        {
        BeAssert(dynamic_cast<ECUnitCP> (newSpec->GetSubUnit()) != nullptr);
        newVal = ((ECUnitCP) newSpec->GetSubUnit())->GetFullName();
        }

    if (oldVal != newVal)
        GetSubUnit().Set(oldVal, newVal);
    }

    {
    Nullable<Utf8String> oldVal;
    if (oldSpec != nullptr && oldSpec->HasSubLabel())
        oldVal = oldSpec->GetSubLabel();

    Nullable<Utf8String> newVal;
    if (newSpec != nullptr && newSpec->HasSubLabel())
        newVal = newSpec->GetSubLabel();

    if (oldVal != newVal)
        GetSubLabel().Set(oldVal, newVal);
    }

    return SUCCESS;
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
void CustomAttributeValidator::AddAcceptRule(Utf8StringCR accessString)
    {
    Utf8String schemaName = GetSchemaName(accessString);
    if (schemaName.empty())
        return;

    m_rules[schemaName].push_back(std::unique_ptr<Rule>(new Rule(Policy::Accept, accessString)));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
void CustomAttributeValidator::AddRejectRule(Utf8StringCR accessString)
    {
    Utf8String schemaName = GetSchemaName(accessString);
    if (schemaName.empty())
        return;

    m_rules[schemaName].push_back(std::unique_ptr<Rule>(new Rule(Policy::Reject, accessString)));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
std::vector<std::unique_ptr<CustomAttributeValidator::Rule>> const& CustomAttributeValidator::GetRelevantRules(CustomAttributeChange& change) const
    {
    Utf8String schemaName = GetSchemaName(change.GetId());
    if (schemaName.empty())
        return m_rules.find(m_wildcard)->second;

    auto itor = m_rules.find(schemaName);
    if (itor != m_rules.end())
        return itor->second;

    return m_rules.find(m_wildcard)->second;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
CustomAttributeValidator::Policy CustomAttributeValidator::Validate(CustomAttributeChange& change) const
    {
    std::vector<std::unique_ptr<Rule>> const& rules = GetRelevantRules(change);
    if (rules.empty())
        return GetDefaultPolicy();

    const size_t propValueCount = change.Count();
    for (size_t i = 0; i < propValueCount; i++)
        {
        ECPropertyValueChange const& propValueChange = change[i];

        std::vector<Utf8String> path = SchemaComparer::Split(propValueChange.GetAccessString(), true);

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
//static
Utf8String CustomAttributeValidator::GetSchemaName(Utf8StringCR path)
    {
    size_t i = path.find(':');
    if (i == Utf8String::npos)
        return Utf8String();

    return path.substr(0, i);
    }

END_BENTLEY_ECOBJECT_NAMESPACE
