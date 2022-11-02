/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"
#include <Bentley/Base64Utilities.h>
#include <ECObjects/SchemaComparer.h>
#include <set>

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

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

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct SchemaProxy final
    {
    private:
        ECN::ECSchemaCP m_schema = nullptr;

    public:
        explicit SchemaProxy(ECN::ECSchemaCP schema) : m_schema(schema) {}

        Nullable<Utf8String> Name() const { return m_schema != nullptr ? m_schema->GetName() : Nullable<Utf8String>(); }
        Nullable<Utf8String> DisplayLabel() const { return m_schema != nullptr && m_schema->GetIsDisplayLabelDefined() ? m_schema->GetInvariantDisplayLabel() : Nullable<Utf8String>(); }
        Nullable<Utf8String> Description() const { return m_schema != nullptr && !Utf8String::IsNullOrEmpty(m_schema->GetInvariantDescription().c_str()) ? m_schema->GetInvariantDescription() : Nullable<Utf8String>(); }
        Nullable<Utf8String> Alias() const { return m_schema != nullptr ? m_schema->GetAlias() : Nullable<Utf8String>(); }
        Nullable<uint32_t> VersionRead() const { return m_schema != nullptr ? m_schema->GetVersionRead() : Nullable<uint32_t>(); }
        Nullable<uint32_t> VersionMinor() const { return m_schema != nullptr ? m_schema->GetVersionMinor() : Nullable<uint32_t>(); }
        Nullable<uint32_t> VersionWrite() const { return m_schema != nullptr ? m_schema->GetVersionWrite() : Nullable<uint32_t>(); }
        Nullable<uint32_t> ECVersion() const { return m_schema != nullptr ? (uint32_t) m_schema->GetECVersion() : Nullable<uint32_t>(); }
        Nullable<uint32_t> OriginalECXmlVersionMajor() const { return m_schema != nullptr ? m_schema->GetOriginalECXmlVersionMajor() : Nullable<uint32_t>(); }
        Nullable<uint32_t> OriginalECXmlVersionMinor() const { return m_schema != nullptr ? m_schema->GetOriginalECXmlVersionMinor() : Nullable<uint32_t>(); }

        ECN::ECSchemaReferenceList const* References() const { return m_schema != nullptr ? &m_schema->GetReferencedSchemas() : nullptr; }
        ECN::ECClassContainerCP Classes() const { return m_schema != nullptr ? &m_schema->GetClasses() : nullptr; }
        ECN::ECEnumerationContainerCP Enumerations() const { return m_schema != nullptr ? &m_schema->GetEnumerations() : nullptr; }
        ECN::PropertyCategoryContainerCP PropertyCategories() const { return m_schema != nullptr ? &m_schema->GetPropertyCategories() : nullptr; }
        ECN::KindOfQuantityContainerCP KindOfQuantities() const { return m_schema != nullptr ? &m_schema->GetKindOfQuantities() : nullptr; }
        ECN::PhenomenonContainerCP Phenomena() const { return m_schema != nullptr ? &m_schema->GetPhenomena() : nullptr; }
        ECN::UnitSystemContainerCP UnitSystems() const { return m_schema != nullptr ? &m_schema->GetUnitSystems() : nullptr; }
        ECN::UnitContainerCP Units() const { return m_schema != nullptr ? &m_schema->GetUnits() : nullptr; }
        ECN::FormatContainerCP Formats() const { return m_schema != nullptr ? &m_schema->GetFormats() : nullptr; }
        bvector<ECN::IECInstancePtr> CustomAttributes() const
            {
            bvector<ECN::IECInstancePtr> cas;
            if (m_schema == nullptr)
                return cas;

            for (ECN::IECInstancePtr const& ca : m_schema->GetCustomAttributes(false))
                {
                cas.push_back(ca);
                }

            return cas;
            }
    };

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ClassProxy final
    {
    private:
        ECN::ECClassCP m_class = nullptr;

        bool IsRelationship() const { return m_class != nullptr && m_class->IsRelationshipClass(); }
    public:
        explicit ClassProxy(ECN::ECClassCP ecClass) : m_class(ecClass) {}

        Nullable<Utf8String> Name() const { return m_class != nullptr ? m_class->GetName() : Nullable<Utf8String>(); }
        Nullable<Utf8String> DisplayLabel() const { return m_class != nullptr && m_class->GetIsDisplayLabelDefined() ? m_class->GetInvariantDisplayLabel() : Nullable<Utf8String>(); }
        Nullable<Utf8String> Description() const { return m_class != nullptr && !Utf8String::IsNullOrEmpty(m_class->GetInvariantDescription().c_str()) ? m_class->GetInvariantDescription() : Nullable<Utf8String>(); }
        Nullable<ECN::ECClassType> ClassType() const { return m_class != nullptr ? m_class->GetClassType() : Nullable<ECN::ECClassType>(); }
        Nullable<ECN::ECClassModifier> ClassModifier() const { return m_class != nullptr ? m_class->GetClassModifier() : Nullable<ECN::ECClassModifier>(); }
        Nullable<ECN::StrengthType> RelationStrength() const { return IsRelationship() ? m_class->GetRelationshipClassCP()->GetStrength() : Nullable<ECN::StrengthType>(); }
        Nullable<ECN::ECRelatedInstanceDirection> RelationStrengthDirection() const { return IsRelationship() ? m_class->GetRelationshipClassCP()->GetStrengthDirection() : Nullable<ECN::ECRelatedInstanceDirection>(); }
        ECN::ECRelationshipConstraint const* RelationSource() const { return IsRelationship() ? &m_class->GetRelationshipClassCP()->GetSource() : nullptr; }
        ECN::ECRelationshipConstraint const* RelationTarget() const { return IsRelationship() ? &m_class->GetRelationshipClassCP()->GetTarget() : nullptr; }

        ECN::ECBaseClassesList const* BaseClasses() const { return m_class != nullptr ? &m_class->GetBaseClasses() : nullptr; }
        bvector<ECN::ECPropertyCP> Properties() const
            {
            bvector<ECN::ECPropertyCP> props;
            if (m_class == nullptr)
                return props;

            for (ECN::ECPropertyCP prop : m_class->GetProperties(false))
                {
                props.push_back(prop);
                }

            return props;
            }

        bvector<ECN::IECInstancePtr> CustomAttributes() const
            {
            bvector<ECN::IECInstancePtr> cas;
            if (m_class == nullptr)
                return cas;

            for (ECN::IECInstancePtr const& ca : m_class->GetCustomAttributes(false))
                {
                cas.push_back(ca);
                }

            return cas;
            }
    };

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct RelationshipConstraintProxy final
    {
    private:
        ECN::ECRelationshipConstraintCP m_constraint = nullptr;
    public:
        explicit RelationshipConstraintProxy(ECN::ECRelationshipConstraintCP constraint) : m_constraint(constraint) {}

        Nullable<Utf8String> RoleLabel() const { return m_constraint != nullptr ? m_constraint->GetRoleLabel() : Nullable<Utf8String>(); }
        Nullable<bool> IsPolymorphic() const { return m_constraint != nullptr ? m_constraint->GetIsPolymorphic() : Nullable<bool>(); }
        Nullable<Utf8String> Multiplicity() const { return m_constraint != nullptr ? m_constraint->GetMultiplicity().ToString() : Nullable<Utf8String>(); }
        Nullable<Utf8String> AbstractConstraint() const { return (m_constraint != nullptr && m_constraint->GetAbstractConstraint() != nullptr) ? Nullable<Utf8String>(m_constraint->GetAbstractConstraint()->GetFullName()) : Nullable<Utf8String>(); }

        ECN::ECRelationshipConstraintClassList const* ConstraintClasses() const { return m_constraint != nullptr ? &m_constraint->GetConstraintClasses() : nullptr; }
        bvector<ECN::IECInstancePtr> CustomAttributes() const
            {
            bvector<ECN::IECInstancePtr> cas;
            if (m_constraint == nullptr)
                return cas;

            for (ECN::IECInstancePtr const& ca : m_constraint->GetCustomAttributes(false))
                {
                cas.push_back(ca);
                }

            return cas;
            }
    };

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct PropertyProxy final
    {
    private:
        ECN::ECPropertyCP m_prop = nullptr;

    public:
        explicit PropertyProxy(ECN::ECPropertyCP prop) : m_prop(prop) {}

        Nullable<Utf8String> Name() const { return m_prop != nullptr ? m_prop->GetName() : Nullable<Utf8String>(); }
        Nullable<Utf8String> DisplayLabel() const { return m_prop != nullptr && m_prop->GetIsDisplayLabelDefined() ? m_prop->GetInvariantDisplayLabel() : Nullable<Utf8String>(); }
        Nullable<Utf8String> Description() const { return m_prop != nullptr && !Utf8String::IsNullOrEmpty(m_prop->GetInvariantDescription().c_str()) ? m_prop->GetInvariantDescription() : Nullable<Utf8String>(); }

        Nullable<Utf8String> TypeName() const { return m_prop != nullptr ? m_prop->GetTypeName() : Nullable<Utf8String>(); }

        Nullable<bool> IsPrimitive() const { return m_prop != nullptr ? m_prop->GetIsPrimitive() : Nullable<bool>(); }
        Nullable<bool> IsStruct() const { return m_prop != nullptr ? m_prop->GetIsStruct() : Nullable<bool>(); }
        Nullable<bool> IsPrimitiveArray() const { return m_prop != nullptr ? m_prop->GetIsPrimitiveArray() : Nullable<bool>(); }
        Nullable<bool> IsStructArray() const { return m_prop != nullptr ? m_prop->GetIsStructArray() : Nullable<bool>(); }
        Nullable<bool> IsNavigation() const { return m_prop != nullptr ? m_prop->GetIsNavigation() : Nullable<bool>(); }

        Nullable<bool> IsReadOnly() const { return m_prop != nullptr ? m_prop->GetIsReadOnly() : Nullable<bool>(); }
        Nullable<int32_t> Priority() const { return m_prop != nullptr ? m_prop->GetPriority() : Nullable<int32_t>(); }

        Nullable<uint32_t> MinimumLength() const { return m_prop != nullptr && m_prop->IsMinimumLengthDefined() ? m_prop->GetMinimumLength() : Nullable<uint32_t>(); }
        Nullable<uint32_t> MaximumLength() const { return m_prop != nullptr && m_prop->IsMaximumLengthDefined() ? m_prop->GetMaximumLength() : Nullable<uint32_t>(); }

        Nullable<ECN::ECValue> MinimumValue() const
            {
            if (m_prop == nullptr || !m_prop->IsMinimumValueDefined())
                return Nullable<ECValue>();

            ECN::ECValue v;
            if (ECObjectsStatus::Success != m_prop->GetMinimumValue(v))
                {
                LOG.errorv("Could not retrieve minimum value from property '%s.%s'", m_prop->GetClass().GetFullName(), m_prop->GetName().c_str());
                return Nullable<ECValue>();
                }

            return v;
            }

        Nullable<ECN::ECValue> MaximumValue() const
            {
            if (m_prop == nullptr || !m_prop->IsMaximumValueDefined())
                return Nullable<ECValue>();

            ECN::ECValue v;
            if (ECObjectsStatus::Success != m_prop->GetMaximumValue(v))
                {
                LOG.errorv("Could not retrieve maximum value from property '%s.%s'", m_prop->GetClass().GetFullName(), m_prop->GetName().c_str());
                return Nullable<ECValue>();
                }

            return v;
            }


        Nullable<Utf8String> KindOfQuantity() const { return m_prop != nullptr && m_prop->GetKindOfQuantity() != nullptr ? m_prop->GetKindOfQuantity()->GetFullName() : Nullable<Utf8String>(); }
        Nullable<Utf8String> Category() const { return m_prop != nullptr && m_prop->GetCategory() != nullptr ? m_prop->GetCategory()->GetFullName() : Nullable<Utf8String>(); }
        Nullable<Utf8String> Enumeration() const
            {
            if (m_prop == nullptr)
                return Nullable<Utf8String>();

            ECEnumerationCP ecEnum = nullptr;
            if (m_prop->GetIsPrimitive())
                ecEnum = m_prop->GetAsPrimitiveProperty()->GetEnumeration();
            else if (m_prop->GetIsPrimitiveArray())
                ecEnum = m_prop->GetAsPrimitiveArrayProperty()->GetEnumeration();

            return ecEnum != nullptr ? ecEnum->GetFullName() : Nullable<Utf8String>();
            }

        Nullable<Utf8String> ExtendedTypeName() const
            {
            if (m_prop == nullptr || !m_prop->HasExtendedType())
                return Nullable<Utf8String>();

            if (m_prop->GetIsPrimitive())
                return m_prop->GetAsPrimitiveProperty()->GetExtendedTypeName();

            if (m_prop->GetIsPrimitiveArray())
                return m_prop->GetAsPrimitiveArrayProperty()->GetExtendedTypeName();

            BeAssert(false && "Only primitive and primitive array properties are expected to have an extended type name");
            return Nullable<Utf8String>();
            }

        Nullable<ECRelatedInstanceDirection> NavigationDirection() const
            {
            Nullable<ECRelatedInstanceDirection> direction;
            if (m_prop == nullptr || !m_prop->GetIsNavigation())
                return direction;

            return m_prop->GetAsNavigationProperty()->GetDirection();
            }

        Nullable<Utf8String> NavigationRelationship() const
            {
            Nullable<Utf8String> rel;
            if (m_prop == nullptr || !m_prop->GetIsNavigation())
                return rel;

            ECRelationshipClassCP relClass = m_prop->GetAsNavigationProperty()->GetRelationshipClass();
            return relClass != nullptr ? Utf8String(relClass->GetFullName()) : nullptr;
            }

        Nullable<uint32_t> ArrayMinOccurs() const { return m_prop != nullptr && m_prop->GetIsArray() ? m_prop->GetAsArrayProperty()->GetMinOccurs() : Nullable<uint32_t>(); }
        Nullable<uint32_t> ArrayMaxOccurs() const { return m_prop != nullptr && m_prop->GetIsArray() ? m_prop->GetAsArrayProperty()->GetStoredMaxOccurs() : Nullable<uint32_t>(); }

        bvector<ECN::IECInstancePtr> CustomAttributes() const
            {
            bvector<ECN::IECInstancePtr> cas;
            if (m_prop == nullptr)
                return cas;

            for (ECN::IECInstancePtr const& ca : m_prop->GetCustomAttributes(false))
                {
                cas.push_back(ca);
                }

            return cas;
            }
    };

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct EnumProxy final
    {
    private:
        ECN::ECEnumerationCP m_enum = nullptr;

    public:
        explicit EnumProxy(ECN::ECEnumerationCP ecEnum) : m_enum(ecEnum) {}

        Nullable<Utf8String> Name() const { return m_enum != nullptr ? m_enum->GetName() : Nullable<Utf8String>(); }
        Nullable<Utf8String> DisplayLabel() const { return m_enum != nullptr && m_enum->GetIsDisplayLabelDefined() ? m_enum->GetInvariantDisplayLabel() : Nullable<Utf8String>(); }
        Nullable<Utf8String> Description() const { return m_enum != nullptr && !Utf8String::IsNullOrEmpty(m_enum->GetInvariantDescription().c_str()) ? m_enum->GetInvariantDescription() : Nullable<Utf8String>(); }

        Nullable<bool> IsStrict() const { return m_enum != nullptr ? m_enum->GetIsStrict() : Nullable<bool>(); }
        Nullable<Utf8String> TypeName() const { return m_enum != nullptr ? m_enum->GetTypeName() : Nullable<Utf8String>(); }
        bvector<ECN::ECEnumeratorCP> Enumerators() const
            {
            bvector<ECN::ECEnumeratorCP> ens;
            if (m_enum == nullptr)
                return ens;

            for (ECN::ECEnumeratorCP en : m_enum->GetEnumerators())
                {
                ens.push_back(en);
                }

            return ens;
            }
    };

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct EnumeratorProxy final
    {
    private:
        ECN::ECEnumeratorCP m_en = nullptr;

    public:
        explicit EnumeratorProxy(ECN::ECEnumeratorCP en) : m_en(en) {}

        Nullable<Utf8String> Name() const { return m_en != nullptr ? m_en->GetName() : Nullable<Utf8String>(); }
        Nullable<Utf8String> DisplayLabel() const { return m_en != nullptr && m_en->GetIsDisplayLabelDefined() ? m_en->GetInvariantDisplayLabel() : Nullable<Utf8String>(); }
        Nullable<Utf8String> Description() const { return m_en != nullptr && !Utf8String::IsNullOrEmpty(m_en->GetInvariantDescription().c_str()) ? m_en->GetInvariantDescription() : Nullable<Utf8String>(); }

        Nullable<bool> IsInteger() const { return m_en != nullptr ? m_en->IsInteger() : Nullable<bool>(); }
        Nullable<bool> IsString() const { return m_en != nullptr ? m_en->IsString() : Nullable<bool>(); }

        Nullable<Utf8String> String() const { return m_en != nullptr && m_en->IsString() ? m_en->GetString() : Nullable<Utf8String>(); }
        Nullable<int32_t> Integer() const { return m_en != nullptr && m_en->IsInteger() ? m_en->GetInteger() : Nullable<int32_t>(); }
    };


//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct KoqProxy final
    {
    private:
        ECN::KindOfQuantityCP m_koq = nullptr;

    public:
        explicit KoqProxy(ECN::KindOfQuantityCP koq) : m_koq(koq) {}

        Nullable<Utf8String> Name() const { return m_koq != nullptr ? m_koq->GetName() : Nullable<Utf8String>(); }
        Nullable<Utf8String> DisplayLabel() const { return m_koq != nullptr && m_koq->GetIsDisplayLabelDefined() ? m_koq->GetInvariantDisplayLabel() : Nullable<Utf8String>(); }
        Nullable<Utf8String> Description() const { return m_koq != nullptr && !Utf8String::IsNullOrEmpty(m_koq->GetInvariantDescription().c_str()) ? m_koq->GetInvariantDescription() : Nullable<Utf8String>(); }

        Nullable<double> RelativeError() const { return m_koq != nullptr ? m_koq->GetRelativeError() : Nullable<double>(); }
        Nullable<Utf8String> PersistenceUnit() const { return m_koq != nullptr && m_koq->GetPersistenceUnit() != nullptr ? m_koq->GetPersistenceUnit()->GetFullName() : Nullable<Utf8String>(); }
        bvector<Utf8String> PresentationFormats() const
            {
            bvector<Utf8String> formats;
            if (m_koq == nullptr || m_koq->GetPresentationFormats().empty())
                return formats;

            for (NamedFormat const& format : m_koq->GetPresentationFormats())
                formats.push_back(format.GetQualifiedFormatString(m_koq->GetSchema()));

            return formats;
            }
    };

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct PropertyCategoryProxy final
    {
    private:
        ECN::PropertyCategoryCP m_cat = nullptr;

    public:
        explicit PropertyCategoryProxy(ECN::PropertyCategoryCP cat) : m_cat(cat) {}

        Nullable<Utf8String> Name() const { return m_cat != nullptr ? m_cat->GetName() : Nullable<Utf8String>(); }
        Nullable<Utf8String> DisplayLabel() const { return m_cat != nullptr && m_cat->GetIsDisplayLabelDefined() ? m_cat->GetInvariantDisplayLabel() : Nullable<Utf8String>(); }
        Nullable<Utf8String> Description() const { return m_cat != nullptr && !Utf8String::IsNullOrEmpty(m_cat->GetInvariantDescription().c_str()) ? m_cat->GetInvariantDescription() : Nullable<Utf8String>(); }
        Nullable<uint32_t> Priority() const { return m_cat != nullptr ? m_cat->GetPriority() : Nullable<uint32_t>(); }
    };

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct UnitSystemProxy final
    {
    private:
        ECN::UnitSystemCP m_us = nullptr;

    public:
        explicit UnitSystemProxy(ECN::UnitSystemCP us) : m_us(us) {}

        Nullable<Utf8String> Name() const { return m_us != nullptr ? m_us->GetName() : Nullable<Utf8String>(); }
        Nullable<Utf8String> DisplayLabel() const { return m_us != nullptr && m_us->GetIsDisplayLabelDefined() ? m_us->GetInvariantDisplayLabel() : Nullable<Utf8String>(); }
        Nullable<Utf8String> Description() const { return m_us != nullptr && m_us->GetIsDescriptionDefined() ? m_us->GetInvariantDescription() : Nullable<Utf8String>(); }
    };

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct PhenomenonProxy final
    {
    private:
        ECN::PhenomenonCP m_ph = nullptr;

    public:
        explicit PhenomenonProxy(ECN::PhenomenonCP ph) : m_ph(ph) {}

        Nullable<Utf8String> Name() const { return m_ph != nullptr ? m_ph->GetName() : Nullable<Utf8String>(); }
        Nullable<Utf8String> DisplayLabel() const { return m_ph != nullptr && m_ph->GetIsDisplayLabelDefined() ? m_ph->GetInvariantDisplayLabel() : Nullable<Utf8String>(); }
        Nullable<Utf8String> Description() const { return m_ph != nullptr && m_ph->GetIsDescriptionDefined() ? m_ph->GetInvariantDescription() : Nullable<Utf8String>(); }
        Nullable<Utf8String> Definition() const { return m_ph != nullptr ? m_ph->GetDefinition() : Nullable<Utf8String>(); }
    };

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct UnitProxy final
    {
    private:
        ECN::ECUnitCP m_u = nullptr;

    public:
        explicit UnitProxy(ECN::ECUnitCP u) : m_u(u) {}

        Nullable<Utf8String> Name() const { return m_u != nullptr ? m_u->GetName() : Nullable<Utf8String>(); }
        Nullable<Utf8String> DisplayLabel() const { return m_u != nullptr && m_u->GetIsDisplayLabelDefined() ? m_u->GetInvariantDisplayLabel() : Nullable<Utf8String>(); }
        Nullable<Utf8String> Description() const { return m_u != nullptr && m_u->GetIsDescriptionDefined() ? m_u->GetInvariantDescription() : Nullable<Utf8String>(); }

        Nullable<bool> IsConstant() const { return m_u != nullptr ? m_u->IsConstant() : Nullable<bool>(); }
        Nullable<Utf8String> InvertingUnit() const { return m_u != nullptr && m_u->IsInvertedUnit() ? m_u->GetInvertingUnit()->GetFullName() : Nullable<Utf8String>(); }

        Nullable<Utf8String> Definition() const { return m_u != nullptr && m_u->HasDefinition() ? m_u->GetDefinition() : Nullable<Utf8String>(); }
        Nullable<Utf8String> Phenomenon() const { return m_u != nullptr && m_u->GetPhenomenon() != nullptr ? m_u->GetPhenomenon()->GetFullName() : Nullable<Utf8String>(); }
        Nullable<Utf8String> UnitSystem() const { return m_u != nullptr && m_u->HasUnitSystem() ? m_u->GetUnitSystem()->GetFullName() : Nullable<Utf8String>(); }
        Nullable<double> Numerator() const { return m_u != nullptr && m_u->HasNumerator() ? m_u->GetNumerator() : Nullable<double>(); }
        Nullable<double> Denominator() const { return m_u != nullptr && m_u->HasDenominator() ? m_u->GetDenominator() : Nullable<double>(); }
        Nullable<double> Offset() const { return m_u != nullptr && m_u->HasOffset() ? m_u->GetOffset() : Nullable<double>(); }
    };

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct FormatProxy final
    {
    private:
        ECN::ECFormatCP m_f = nullptr;

    public:
        explicit FormatProxy(ECN::ECFormatCP f) : m_f(f) {}

        Nullable<Utf8String> Name() const { return m_f != nullptr ? m_f->GetName() : Nullable<Utf8String>(); }
        Nullable<Utf8String> DisplayLabel() const { return m_f != nullptr && m_f->GetIsDisplayLabelDefined() ? m_f->GetInvariantDisplayLabel() : Nullable<Utf8String>(); }
        Nullable<Utf8String> Description() const { return m_f != nullptr && m_f->GetIsDescriptionDefined() ? m_f->GetInvariantDescription() : Nullable<Utf8String>(); }

        Formatting::NumericFormatSpecCP NumericSpec() const { return m_f != nullptr ? m_f->GetNumericSpec() : nullptr; }
        Formatting::CompositeValueSpecCP CompositeSpec() const { return m_f != nullptr ? m_f->GetCompositeSpec() : nullptr; }
    };

//======================================================================================>
//ECSchemaComparer
//======================================================================================>

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::Compare(SchemaDiff& diff, bvector<ECN::ECSchemaCP> const& oldSchemas, bvector<ECN::ECSchemaCP> const& newSchemas, Options options)
    {
    m_options = options;
    bmap<Utf8CP, ECSchemaCP, CompareIUtf8Ascii> oldSchemaMap, newSchemaMap;
    bset<Utf8CP, CompareIUtf8Ascii> allSchemas;

    for (ECSchemaCP schema : oldSchemas)
        {
        Utf8CP schemaName = schema->GetName().c_str();
        oldSchemaMap[schemaName] = schema;
        allSchemas.insert(schemaName);
        }

    for (ECSchemaCP schema : newSchemas)
        {
        Utf8CP schemaName = schema->GetName().c_str();
        newSchemaMap[schemaName] = schema;
        allSchemas.insert(schemaName);
        }


    for (Utf8CP schemaName : allSchemas)
        {
        auto oldIt = oldSchemaMap.find(schemaName);
        auto newIt = newSchemaMap.find(schemaName);

        const bool existInOld = oldIt != oldSchemaMap.end();
        const bool existInNew = newIt != newSchemaMap.end();

        ECChange::ECChange::OpCode opCode;
        ECSchemaCP oldSchema = nullptr;
        ECSchemaCP newSchema = nullptr;
        if (existInOld && existInNew)
            {
            opCode = ECChange::ECChange::OpCode::Modified;
            oldSchema = oldIt->second;
            newSchema = newIt->second;
            }
        else if (existInOld && !existInNew)
            {
            opCode = ECChange::ECChange::OpCode::Deleted;
            oldSchema = oldIt->second;
            }
        else if (!existInOld && existInNew)
            {
            opCode = ECChange::ECChange::OpCode::New;
            newSchema = newIt->second;
            }
        else
            {
            LOG.errorv("Failed to compare %s because it was not found in either set of schemas", schemaName);
            BeAssert(false);
            return ERROR;
            }

        RefCountedPtr<SchemaChange> schemaChange = diff.Changes().CreateElement(opCode, ECChange::Type::Schema, schemaName);
        if (SUCCESS != CompareSchema(*schemaChange, oldSchema, newSchema))
            {
            LOG.errorv("The schema comparison for %s failed.", schemaName);
            return ERROR;
            }

        diff.Changes().Add(schemaChange);
        }

    return SUCCESS;
    }


//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::CompareSchema(SchemaChange& change, ECSchemaCP oldVal, ECSchemaCP newVal)
    {
    SchemaProxy oldSchema(oldVal);
    SchemaProxy newSchema(newVal);
    change.Name().Set(oldSchema.Name(), newSchema.Name());
    change.DisplayLabel().Set(oldSchema.DisplayLabel(), newSchema.DisplayLabel());
    change.Description().Set(oldSchema.Description(), newSchema.Description());
    change.Alias().Set(oldSchema.Alias(), newSchema.Alias());
    change.VersionRead().Set(oldSchema.VersionRead(), newSchema.VersionRead());
    change.VersionMinor().Set(oldSchema.VersionMinor(), newSchema.VersionMinor());
    change.VersionWrite().Set(oldSchema.VersionWrite(), newSchema.VersionWrite());
    change.ECVersion().Set(oldSchema.ECVersion(), newSchema.ECVersion());
    change.OriginalECXmlVersionMajor().Set(oldSchema.OriginalECXmlVersionMajor(), newSchema.OriginalECXmlVersionMajor());
    change.OriginalECXmlVersionMinor().Set(oldSchema.OriginalECXmlVersionMinor(), newSchema.OriginalECXmlVersionMinor());

    if (CompareClasses(change.Classes(), oldSchema.Classes(), newSchema.Classes()) != SUCCESS)
        {
        LOG.errorv("Class comparison for schema %s failed", change.GetChangeName());
        return ERROR;
        }

    if (CompareEnumerations(change.Enumerations(), oldSchema.Enumerations(), newSchema.Enumerations()) != SUCCESS)
        {
        LOG.errorv("Enumeration comparison for schema %s failed", change.GetChangeName());
        return ERROR;
        }

    if (CompareKindOfQuantities(change.KindOfQuantities(), oldSchema.KindOfQuantities(), newSchema.KindOfQuantities()) != SUCCESS)
        {
        LOG.errorv("KindOfQuantity comparison for schema %s failed", change.GetChangeName());
        return ERROR;
        }

    if (ComparePropertyCategories(change.PropertyCategories(), oldSchema.PropertyCategories(), newSchema.PropertyCategories()) != SUCCESS)
        {
        LOG.errorv("PropertyCategory comparison for schema %s failed", change.GetChangeName());
        return ERROR;
        }

    if (ComparePhenomena(change.Phenomena(), oldSchema.Phenomena(), newSchema.Phenomena()) != SUCCESS)
        {
        LOG.errorv("Phenomenon comparison for schema %s failed", change.GetChangeName());
        return ERROR;
        }

    if (CompareUnitSystems(change.UnitSystems(), oldSchema.UnitSystems(), newSchema.UnitSystems()) != SUCCESS)
        {
        LOG.errorv("UnitSystem comparison for schema %s failed", change.GetChangeName());
        return ERROR;
        }

    if (CompareUnits(change.Units(), oldSchema.Units(), newSchema.Units()) != SUCCESS)
        {
        LOG.errorv("Unit comparison for schema %s failed", change.GetChangeName());
        return ERROR;
        }

    if (SUCCESS != CompareFormats(change.Formats(), oldSchema.Formats(), newSchema.Formats()))
        {
        LOG.errorv("Format comparison for schema %s failed", change.GetChangeName());
        return ERROR;
        }

    if (CompareReferences(change.References(), oldSchema.References(), newSchema.References()) != SUCCESS)
        {
        LOG.errorv("Schema Reference comparison for schema %s failed", change.GetChangeName());
        return ERROR;
        }

    if (CompareCustomAttributes(change.CustomAttributes(), oldSchema.CustomAttributes(), newSchema.CustomAttributes()) != SUCCESS)
        {
        LOG.errorv("CustomAttribute comparison for schema %s failed", change.GetChangeName());
        return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::CompareReferences(SchemaReferenceChanges& changes, ECSchemaReferenceList const* oldVal, ECSchemaReferenceList const* newVal)
    {
    bmap<Utf8String, Utf8String, CompareIUtf8Ascii> oldReferences, newReferences;
    bset<Utf8String, CompareIUtf8Ascii> allReferences;
    if (oldVal != nullptr)
        {
        for (auto const& kvPair : *oldVal)
            {
            const Utf8String name = kvPair.second->GetName();
            const Utf8String fullname = kvPair.second->GetFullSchemaName();
            if (!oldReferences.insert(make_bpair(name, fullname)).second)
                {
                LOG.errorv("Schema Reference comparison failed because multiple schema references for %s were found in the old schema.", name.c_str());
                BeAssert(false && "Multiple version of same referenced schema is not supported");
                return ERROR;
                }

            allReferences.insert(name);
            }
        }

    if (newVal != nullptr)
        {
        for (auto const& kvPair : *newVal)
            {
            const Utf8String name = kvPair.second->GetName();
            const Utf8String fullname = kvPair.second->GetFullSchemaName();
            if (!newReferences.insert(make_bpair(name, fullname)).second)
                {
                LOG.errorv("Schema Reference comparison failed because multiple schema references for %s were found in the new schema.", name.c_str());
                BeAssert(false && "Multiple version of same referenced schema is not supported");
                return ERROR;
                }

            allReferences.insert(name);
            }
        }

    for (Utf8StringCR schemaName : allReferences)
        {
        const auto oldIt = oldReferences.find(schemaName);
        const auto newIt = newReferences.find(schemaName);

        const bool existsInOld = oldIt != oldReferences.end();
        const bool existsInNew = newIt != newReferences.end();

        ECChange::ECChange::ECChange::OpCode opCode;
        Nullable<Utf8String> oldRef;
        Nullable<Utf8String> newRef;

        if (existsInOld && existsInNew)
            {
            Utf8StringCR oldSchemaFullName = oldIt->second;
            Utf8StringCR newSchemaFullName = newIt->second;
            if (oldSchemaFullName.EqualsI(newSchemaFullName))
                continue;

            opCode = ECChange::ECChange::ECChange::OpCode::Modified;
            oldRef = oldSchemaFullName;
            newRef = newSchemaFullName;
            }
        else if (existsInOld && !existsInNew)
            {
            opCode = ECChange::ECChange::ECChange::OpCode::Deleted;
            oldRef = oldIt->second;
            }
        else if (!existsInOld && existsInNew)
            {
            opCode = ECChange::ECChange::ECChange::OpCode::New;
            newRef = newIt->second;
            }
        else
            {
            LOG.errorv("Schema Reference comparison failed because no schema reference was found for %s though the schema was expected in one of the schemas.", schemaName.c_str());
            BeAssert(false);
            return ERROR;
            }

        RefCountedPtr<StringChange> change = changes.CreateElement(opCode, ECChange::Type::SchemaReference, schemaName.c_str());
        if (SUCCESS != change->Set(oldRef, newRef))
            {
            LOG.errorv("Schema Reference comparison failed because the change object could not be created for %s.", schemaName.c_str());
            return ERROR;
            }

        changes.Add(change);
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::CompareClasses(ClassChanges& changes, ECClassContainerCP oldVal, ECClassContainerCP newVal)
    {
    bmap<Utf8CP, ECClassCP, CompareIUtf8Ascii> oldClasses, newClasses;
    bset<Utf8CP, CompareIUtf8Ascii> allClasses;
    if (oldVal != nullptr)
        {
        for (ECClassCP cl : *oldVal)
            {
            Utf8CP name = cl->GetName().c_str();
            oldClasses[name] = cl;
            allClasses.insert(name);
            }
        }

    if (newVal != nullptr)
        {
        for (ECClassCP cl : *newVal)
            {
            Utf8CP name = cl->GetName().c_str();
            newClasses[name] = cl;
            allClasses.insert(name);
            }
        }

    for (Utf8CP className : allClasses)
        {
        auto oldIt = oldClasses.find(className);
        auto newIt = newClasses.find(className);

        const bool existsInOld = oldIt != oldClasses.end();
        const bool existsInNew = newIt != newClasses.end();

        ECChange::ECChange::OpCode opCode;
        ECClassCP oldClass = nullptr;
        ECClassCP newClass = nullptr;

        if (existsInOld && existsInNew)
            {
            opCode = ECChange::OpCode::Modified;
            oldClass = oldIt->second;
            newClass = newIt->second;
            }
        else if (existsInOld && !existsInNew)
            {
            opCode = ECChange::OpCode::Deleted;
            oldClass = oldIt->second;
            }
        else if (!existsInOld && existsInNew)
            {
            opCode = ECChange::OpCode::New;
            newClass = newIt->second;
            }
        else
            {
            LOG.errorv("Class comparison failed because no class was found for %s though the class was expected in one of the schemas.", className);
            BeAssert(false);
            return ERROR;
            }

        RefCountedPtr<ClassChange> classChange = changes.CreateElement(opCode, ECChange::Type::Class, className);
        if (SUCCESS != CompareClass(*classChange, oldClass, newClass))
            {
            LOG.errorv("Class comparison for class %s", className);
            return ERROR;
            }

        changes.Add(classChange);
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::CompareClass(ClassChange& change, ECClassCP oldVal, ECClassCP newVal)
    {
    ClassProxy oldClass(oldVal);
    ClassProxy newClass(newVal);

    change.Name().Set(oldClass.Name(), newClass.Name());
    change.DisplayLabel().Set(oldClass.DisplayLabel(), newClass.DisplayLabel());
    change.Description().Set(oldClass.Description(), newClass.Description());
    change.ClassType().Set(oldClass.ClassType(), newClass.ClassType());
    change.ClassModifier().Set(oldClass.ClassModifier(), newClass.ClassModifier());

    if ((oldVal != nullptr && oldVal->IsRelationshipClass()) || (newVal != nullptr && newVal->IsRelationshipClass()))
        {
        change.Strength().Set(oldClass.RelationStrength(), newClass.RelationStrength());
        change.StrengthDirection().Set(oldClass.RelationStrengthDirection(), newClass.RelationStrengthDirection());

        if (CompareRelationshipConstraint(change.Source(), oldClass.RelationSource(), newClass.RelationSource()) != SUCCESS)
            {
            LOG.errorv("Source Relationship Constraint comparison failed for class %s", change.GetChangeName());
            return ERROR;
            }

        if (CompareRelationshipConstraint(change.Target(), oldClass.RelationTarget(), newClass.RelationTarget()) != SUCCESS)
            {
            LOG.errorv("Target Relationship Constraint comparison failed for class %s", change.GetChangeName());
            return ERROR;
            }
        }

    if (CompareBaseClasses(change.BaseClasses(), oldClass.BaseClasses(), newClass.BaseClasses()) != SUCCESS)
        {
        LOG.errorv("Base Class comparison failed for class %s", change.GetChangeName());
        return ERROR;
        }

    if (CompareProperties(change.Properties(), oldClass.Properties(), newClass.Properties()) != SUCCESS)
        {
        LOG.errorv("Property comparison failed for class %s", change.GetChangeName());
        return ERROR;
        }

    if (CompareCustomAttributes(change.CustomAttributes(), oldClass.CustomAttributes(), newClass.CustomAttributes()))
        {
        LOG.errorv("CustomAttribute comparison failed for class %s", change.GetChangeName());
        return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::CompareBaseClasses(BaseClassChanges& changes, ECBaseClassesList const* oldList, ECBaseClassesList const* newList)
    {
    ECBaseClassesList emptyClassList;
    ECBaseClassesList const& oldBaseClasses = (oldList == nullptr) ? emptyClassList : *oldList;
    ECBaseClassesList const& newBaseClasses = (newList == nullptr) ? emptyClassList : *newList;
    ECClassP oldBaseClass = nullptr;
    ECClassP newBaseClass = nullptr;
    size_t oldSize = oldBaseClasses.size();
    size_t newSize = newBaseClasses.size();
    auto oldFirstMixin = oldBaseClasses.begin();
    auto newFirstMixin = newBaseClasses.begin();

    // First: Compare Base classes (only one base class is allowed, and it has to be the first element in the collection)
    // We only consider this a "change", if the two base classes are moving within the same hierarchy (one *is* of the other),
    // otherwise we consider this an remove + add
    if(oldSize > 0)
        {
        auto firstElement = oldBaseClasses.at(0);
        if(!firstElement->IsMixin())
            {
            std::advance(oldFirstMixin, 1);
            oldBaseClass = firstElement;
            }
        }

    if(newSize > 0)
        {
        auto firstElement = newBaseClasses.at(0);
        if(!firstElement->IsMixin())
            {
            std::advance(newFirstMixin, 1);
            newBaseClass = firstElement;
            }
        }

    Nullable<Utf8String> nullValue;
    if (oldBaseClass == nullptr && newBaseClass != nullptr)
        {
        RefCountedPtr<StringChange> baseClassChange = changes.CreateElement(ECChange::OpCode::New, ECChange::Type::BaseClass);
        baseClassChange->Set(nullValue, Nullable<Utf8String>(Utf8String(newBaseClass->GetFullName())));
        changes.Add(baseClassChange);
        }
    else if (newBaseClass == nullptr && oldBaseClass != nullptr)
        {
        RefCountedPtr<StringChange> baseClassChange = changes.CreateElement(ECChange::OpCode::Deleted, ECChange::Type::BaseClass);
        baseClassChange->Set(Nullable<Utf8String>(Utf8String(oldBaseClass->GetFullName())), nullValue);
        changes.Add(baseClassChange);
        }
    else if (newBaseClass != nullptr && oldBaseClass != nullptr)
        {
        auto oldFullName = oldBaseClass->GetFullName();
        auto newFullName = newBaseClass->GetFullName();
        if (BeStringUtilities::StricmpAscii(oldFullName, newFullName) != 0)
            {
            // base class has changed, now see if we are moving down within the same hierarchy
            if(newBaseClass->Is(oldBaseClass->GetSchema().GetName().c_str(), oldBaseClass->GetName().c_str()))
                { // in this case we consider it a modification
                RefCountedPtr<StringChange> baseClassChange = changes.CreateElement(ECChange::OpCode::Modified, ECChange::Type::BaseClass);
                baseClassChange->Set(Nullable<Utf8String>(Utf8String(oldBaseClass->GetFullName())), Nullable<Utf8String>(Utf8String(newBaseClass->GetFullName())));
                changes.Add(baseClassChange);
                }
            else
                { // in this case we consider it add and remove
                RefCountedPtr<StringChange> baseClassChange = changes.CreateElement(ECChange::OpCode::Deleted, ECChange::Type::BaseClass);
                baseClassChange->Set(Nullable<Utf8String>(Utf8String(oldBaseClass->GetFullName())), nullValue);
                changes.Add(baseClassChange);
                baseClassChange = changes.CreateElement(ECChange::OpCode::New, ECChange::Type::BaseClass);
                baseClassChange->Set(nullValue, Nullable<Utf8String>(Utf8String(newBaseClass->GetFullName())));
                changes.Add(baseClassChange);
                }
            }
        }
    
    // now compare mixins by full name
    bset<Utf8String, CompareIUtf8Ascii> oldMixinFullNames, newMixinFullNames;
    std::for_each(oldFirstMixin, oldBaseClasses.end(), [&oldMixinFullNames](ECClassP element) { oldMixinFullNames.insert(Utf8String(element->GetFullName())); });
    std::for_each(newFirstMixin, newBaseClasses.end(), [&newMixinFullNames](ECClassP element) { newMixinFullNames.insert(Utf8String(element->GetFullName())); });

    bvector<Utf8String> removedMixins;
    std::set_difference(oldMixinFullNames.begin(), oldMixinFullNames.end(), newMixinFullNames.begin(), newMixinFullNames.end(), std::inserter(removedMixins, removedMixins.begin()));
    for(auto removedMixin : removedMixins)
        {
        RefCountedPtr<StringChange> change = changes.CreateElement(ECChange::OpCode::Deleted, ECChange::Type::BaseClass);
        change->Set(Nullable<Utf8String>(removedMixin), nullValue);
        changes.Add(change);
        }

    bvector<Utf8String> addedMixins;
    std::set_difference(newMixinFullNames.begin(), newMixinFullNames.end(), oldMixinFullNames.begin(), oldMixinFullNames.end(), std::inserter(addedMixins, addedMixins.begin()));
    for(auto addedMixin : addedMixins)
        {
        RefCountedPtr<StringChange> change = changes.CreateElement(ECChange::OpCode::New, ECChange::Type::BaseClass);
        change->Set(nullValue, Nullable<Utf8String>(addedMixin));
        changes.Add(change);
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::CompareRelationshipConstraint(RelationshipConstraintChange& change, ECRelationshipConstraintCP oldVal, ECRelationshipConstraintCP newVal)
    {
    RelationshipConstraintProxy oldConstraint(oldVal);
    RelationshipConstraintProxy newConstraint(newVal);

    change.RoleLabel().Set(oldConstraint.RoleLabel(), newConstraint.RoleLabel());
    change.IsPolymorphic().Set(oldConstraint.IsPolymorphic(), newConstraint.IsPolymorphic());
    change.Multiplicity().Set(oldConstraint.Multiplicity(), newConstraint.Multiplicity());
    change.AbstractConstraint().Set(oldConstraint.AbstractConstraint(), newConstraint.AbstractConstraint());

    if (SUCCESS != CompareRelationshipConstraintClasses(change.ConstraintClasses(), oldConstraint.ConstraintClasses(), newConstraint.ConstraintClasses()))
        {
        LOG.errorv("Relationship Constraint Class comparison failed for class %s", change.GetChangeName());
        return ERROR;
        }

    return CompareCustomAttributes(change.CustomAttributes(), oldConstraint.CustomAttributes(), newConstraint.CustomAttributes());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::CompareRelationshipConstraintClasses(RelationshipConstraintClassChanges& change, ECRelationshipConstraintClassList const* oldValue, ECRelationshipConstraintClassList const* newValue)
    {
    bset<Utf8CP, CompareIUtf8Ascii> oldConstraintClasses, newConstraintClasses, allConstraintClasses;
    if (oldValue != nullptr)
        {
        for (ECClassCP constraintClass : *oldValue)
            {
            Utf8CP className = constraintClass->GetFullName();
            oldConstraintClasses.insert(className);
            allConstraintClasses.insert(className);
            }
        }

    if (newValue != nullptr)
        {
        for (ECClassCP constraintClass : *newValue)
            {
            Utf8CP className = constraintClass->GetFullName();
            newConstraintClasses.insert(className);
            allConstraintClasses.insert(className);
            }
        }

    for (Utf8CP constraintClassName : allConstraintClasses)
        {
        auto oldValIt = oldConstraintClasses.find(constraintClassName);
        auto newValIt = newConstraintClasses.find(constraintClassName);

        bool existInOld = oldValIt != oldConstraintClasses.end();
        bool existInNew = newValIt != newConstraintClasses.end();
        Nullable<Utf8String> oldName, newName;
        ECChange::OpCode opCode = ECChange::OpCode::Modified;
        if (existInOld && !existInNew)
            {
            opCode = ECChange::OpCode::Deleted;
            oldName = Utf8String(constraintClassName);
            }
        else if (!existInOld && existInNew)
            {
            opCode = ECChange::OpCode::New;
            newName = Utf8String(constraintClassName);
            }
        else if (existInOld && existInNew)
            continue;
        else
            {
            LOG.errorv("Relationship constraint class comparison because no old or new constraint with name %s was found.", constraintClassName);
            BeAssert(false);
            return ERROR;
            }

        BeAssert(opCode != ECChange::OpCode::Modified);
        RefCountedPtr<StringChange> constraintClassChange = change.CreateElement(opCode, ECChange::Type::ConstraintClass);
        constraintClassChange->Set(oldName, newName);
        change.Add(constraintClassChange);
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::CompareProperties(PropertyChanges& changes, bvector<ECN::ECPropertyCP> const& oldVal, bvector<ECN::ECPropertyCP> const& newVal)
    {
    bmap<Utf8CP, ECPropertyCP, CompareIUtf8Ascii> oldProps, newProps;
    bset<Utf8CP, CompareIUtf8Ascii> allProps;
    for (ECPropertyCP prop : oldVal)
        {
        Utf8CP propName = prop->GetName().c_str();
        oldProps[propName] = prop;
        allProps.insert(propName);
        }
    for (ECPropertyCP prop : newVal)
        {
        Utf8CP propName = prop->GetName().c_str();
        newProps[propName] = prop;
        allProps.insert(propName);
        }

    for (Utf8CP propName : allProps)
        {
        auto oldIt = oldProps.find(propName);
        auto newIt = newProps.find(propName);

        const bool existsInOld = oldIt != oldProps.end();
        const bool existsInNew = newIt != newProps.end();

        ECChange::OpCode opCode;
        ECPropertyCP oldProp = nullptr;
        ECPropertyCP newProp = nullptr;

        if (existsInOld && existsInNew)
            {
            opCode = ECChange::OpCode::Modified;
            oldProp = oldIt->second;
            newProp = newIt->second;
            }
        else if (existsInOld && !existsInNew)
            {
            opCode = ECChange::OpCode::Deleted;
            oldProp = oldIt->second;
            }
        else if (!existsInOld && existsInNew)
            {
            opCode = ECChange::OpCode::New;
            newProp = newIt->second;
            }
        else
            {
            LOG.errorv("Property comparison because no old or new property with name %s was found.", propName);
            BeAssert(false);
            return ERROR;
            }

        RefCountedPtr<PropertyChange> propertyChange = changes.CreateElement(opCode, ECChange::Type::Property, propName);
        if (SUCCESS != CompareProperty(*propertyChange, oldProp, newProp))
            {
            LOG.errorv("Property comparison failed for property %s.", propName);
            return ERROR;
            }

        changes.Add(propertyChange);
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::CompareProperty(PropertyChange& change, ECPropertyCP oldVal, ECPropertyCP newVal)
    {
    PropertyProxy oldProp(oldVal), newProp(newVal);
    change.Name().Set(oldProp.Name(), newProp.Name());
    change.TypeName().Set(oldProp.TypeName(), newProp.TypeName());
    change.DisplayLabel().Set(oldProp.DisplayLabel(), newProp.DisplayLabel());
    change.Description().Set(oldProp.Description(), newProp.Description());

    change.IsPrimitive().Set(oldProp.IsPrimitive(), newProp.IsPrimitive());
    change.IsStruct().Set(oldProp.IsStruct(), newProp.IsStruct());
    change.IsStructArray().Set(oldProp.IsStructArray(), newProp.IsStructArray());
    change.IsPrimitiveArray().Set(oldProp.IsPrimitiveArray(), newProp.IsPrimitiveArray());
    change.IsNavigation().Set(oldProp.IsNavigation(), newProp.IsNavigation());

    change.IsReadonly().Set(oldProp.IsReadOnly(), newProp.IsReadOnly());
    change.Priority().Set(oldProp.Priority(), newProp.Priority());

    change.MinimumLength().Set(oldProp.MinimumLength(), newProp.MinimumLength());
    change.MaximumLength().Set(oldProp.MaximumLength(), newProp.MaximumLength());
    change.MinimumValue().Set(oldProp.MinimumValue(), newProp.MinimumValue());
    change.MaximumValue().Set(oldProp.MaximumValue(), newProp.MaximumValue());

    change.KindOfQuantity().Set(oldProp.KindOfQuantity(), newProp.KindOfQuantity());
    change.Category().Set(oldProp.Category(), newProp.Category());
    change.Enumeration().Set(oldProp.Enumeration(), newProp.Enumeration());
    change.ExtendedTypeName().Set(oldProp.ExtendedTypeName(), newProp.ExtendedTypeName());

    if ((oldVal != nullptr && oldVal->GetIsNavigation()) || (newVal != nullptr && newVal->GetIsNavigation()))
        {
        change.NavigationDirection().Set(oldProp.NavigationDirection(), newProp.NavigationDirection());
        change.NavigationRelationship().Set(oldProp.NavigationRelationship(), newProp.NavigationRelationship());
        }

    if ((oldVal != nullptr && oldVal->GetIsArray()) || (newVal != nullptr && newVal->GetIsArray()))
        {
        change.ArrayMinOccurs().Set(oldProp.ArrayMinOccurs(), newProp.ArrayMinOccurs());
        change.ArrayMaxOccurs().Set(oldProp.ArrayMaxOccurs(), newProp.ArrayMaxOccurs());
        }

    return CompareCustomAttributes(change.CustomAttributes(), oldProp.CustomAttributes(), newProp.CustomAttributes());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::CompareEnumerations(EnumerationChanges& changes, ECEnumerationContainerCP oldVal, ECEnumerationContainerCP newVal)
    {
    bmap<Utf8CP, ECEnumerationCP, CompareIUtf8Ascii> oldEnums, newEnums;
    bset<Utf8CP, CompareIUtf8Ascii> allEnums;
    if (oldVal != nullptr)
        {
        for (ECEnumerationCP en : *oldVal)
            {
            Utf8CP name = en->GetName().c_str();
            oldEnums[name] = en;
            allEnums.insert(name);
            }
        }

    if (newVal != nullptr)
        {
        for (ECEnumerationCP en : *newVal)
            {
            Utf8CP name = en->GetName().c_str();
            newEnums[name] = en;
            allEnums.insert(name);
            }
        }

    for (Utf8CP enumName : allEnums)
        {
        auto oldIt = oldEnums.find(enumName);
        auto newIt = newEnums.find(enumName);

        const bool existsInOld = oldIt != oldEnums.end();
        const bool existsInNew = newIt != newEnums.end();

        ECChange::OpCode opCode;
        ECEnumerationCP oldEnum = nullptr;
        ECEnumerationCP newEnum = nullptr;

        if (existsInOld && existsInNew)
            {
            opCode = ECChange::OpCode::Modified;
            oldEnum = oldIt->second;
            newEnum = newIt->second;
            }
        else if (existsInOld && !existsInNew)
            {
            opCode = ECChange::OpCode::Deleted;
            oldEnum = oldIt->second;
            }
        else if (!existsInOld && existsInNew)
            {
            opCode = ECChange::OpCode::New;
            newEnum = newIt->second;
            }
        else
            {
            LOG.errorv("Enumeration comparison because no old or new enum with name %s was found.", enumName);
            BeAssert(false);
            return ERROR;
            }

        RefCountedPtr<EnumerationChange> enumChange = changes.CreateElement(opCode, ECChange::Type::Enumeration, enumName);
        if (SUCCESS != CompareEnumeration(*enumChange, oldEnum, newEnum))
            {
            LOG.errorv("Enumeration comparison failed for enumeration %s.", enumName);
            return ERROR;
            }

        changes.Add(enumChange);
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::CompareEnumeration(EnumerationChange& change, ECEnumerationCP oldVal, ECEnumerationCP newVal)
    {
    EnumProxy oldEnum(oldVal), newEnum(newVal);
    change.Name().Set(oldEnum.Name(), newEnum.Name());
    change.DisplayLabel().Set(oldEnum.DisplayLabel(), newEnum.DisplayLabel());
    change.Description().Set(oldEnum.Description(), newEnum.Description());
    change.TypeName().Set(oldEnum.TypeName(), newEnum.TypeName());
    change.IsStrict().Set(oldEnum.IsStrict(), newEnum.IsStrict());
    return CompareEnumerators(change.Enumerators(), oldEnum.Enumerators(), newEnum.Enumerators());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::CompareEnumerators(EnumeratorChanges& changes, bvector<ECN::ECEnumeratorCP> const& oldVal, bvector<ECN::ECEnumeratorCP> const& newVal)
    {
    bmap<Utf8CP, ECEnumeratorCP, CompareIUtf8Ascii> oldEnumerators, newEnumerators;
    bset<Utf8CP, CompareIUtf8Ascii> allEnumerators;
    for (ECEnumeratorCP en : oldVal)
        {
        Utf8CP name = en->GetName().c_str();
        oldEnumerators[name] = en;
        allEnumerators.insert(name);
        }
    for (ECEnumeratorCP en : newVal)
        {
        Utf8CP name = en->GetName().c_str();
        newEnumerators[name] = en;
        allEnumerators.insert(name);
        }

    for (Utf8CP name : allEnumerators)
        {
        auto oldIt = oldEnumerators.find(name);
        auto newIt = newEnumerators.find(name);

        const bool existsInOld = oldIt != oldEnumerators.end();
        const bool existsInNew = newIt != newEnumerators.end();

        ECChange::OpCode opCode;
        ECEnumeratorCP oldEn = nullptr;
        ECEnumeratorCP newEn = nullptr;

        if (existsInOld && existsInNew)
            {
            opCode = ECChange::OpCode::Modified;
            oldEn = oldIt->second;
            newEn = newIt->second;
            }
        else if (existsInOld && !existsInNew)
            {
            opCode = ECChange::OpCode::Deleted;
            oldEn = oldIt->second;
            }
        else if (!existsInOld && existsInNew)
            {
            opCode = ECChange::OpCode::New;
            newEn = newIt->second;
            }
        else
            {
            LOG.errorv("Enumerator comparison because no old or new enumerator with name %s was found.", name);
            BeAssert(false);
            return ERROR;
            }

        RefCountedPtr<EnumeratorChange> change = changes.CreateElement(opCode, ECChange::Type::Enumerator, name);
        if (SUCCESS != CompareEnumerator(*change, oldEn, newEn))
            {
            LOG.errorv("Enumerator comparison failed for enumerator %s.", name);
            return ERROR;
            }

        changes.Add(change);
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::CompareEnumerator(EnumeratorChange& change, ECEnumeratorCP oldVal, ECEnumeratorCP newVal)
    {
    EnumeratorProxy oldEn(oldVal), newEn(newVal);
    change.Name().Set(oldEn.Name(), newEn.Name());
    change.DisplayLabel().Set(oldEn.DisplayLabel(), newEn.DisplayLabel());
    change.Description().Set(oldEn.Description(), newEn.Description());
    change.IsInteger().Set(oldEn.IsInteger(), newEn.IsInteger());
    change.Integer().Set(oldEn.Integer(), newEn.Integer());
    change.IsString().Set(oldEn.IsString(), newEn.IsString());
    change.String().Set(oldEn.String(), newEn.String());
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::ComparePropertyCategories(PropertyCategoryChanges& changes, PropertyCategoryContainerCP oldVal, PropertyCategoryContainerCP newVal)
    {
    bmap<Utf8CP, PropertyCategoryCP, CompareIUtf8Ascii> oldVals, newVals;
    bset<Utf8CP, CompareIUtf8Ascii> allVals;
    if (oldVal != nullptr)
        {
        for (PropertyCategoryCP cat : *oldVal)
            {
            Utf8CP name = cat->GetName().c_str();
            oldVals[name] = cat;
            allVals.insert(name);
            }
        }

    if (newVal != nullptr)
        {
        for (PropertyCategoryCP cat : *newVal)
            {
            Utf8CP name = cat->GetName().c_str();
            newVals[name] = cat;
            allVals.insert(name);
            }
        }

    for (Utf8CP catName : allVals)
        {
        auto oldIt = oldVals.find(catName);
        auto newIt = newVals.find(catName);

        const bool existsInOld = oldIt != oldVals.end();
        const bool existsInNew = newIt != newVals.end();

        ECChange::OpCode opCode;
        PropertyCategoryCP oldCat = nullptr;
        PropertyCategoryCP newCat = nullptr;

        if (existsInOld && existsInNew)
            {
            opCode = ECChange::OpCode::Modified;
            oldCat = oldIt->second;
            newCat = newIt->second;
            }
        else if (existsInOld && !existsInNew)
            {
            opCode = ECChange::OpCode::Deleted;
            oldCat = oldIt->second;
            }
        else if (!existsInOld && existsInNew)
            {
            opCode = ECChange::OpCode::New;
            newCat = newIt->second;
            }
        else
            {
            LOG.errorv("Property Category comparison because no old or new category with name %s was found.", catName);
            BeAssert(false);
            return ERROR;
            }

        RefCountedPtr<PropertyCategoryChange> change = changes.CreateElement(opCode, ECChange::Type::PropertyCategory, catName);
        if (SUCCESS != ComparePropertyCategory(*change, oldCat, newCat))
            {
            LOG.errorv("Property Category comparison failed for category %s.", catName);
            return ERROR;
            }

        changes.Add(change);
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::ComparePropertyCategory(PropertyCategoryChange& change, PropertyCategoryCP oldVal, PropertyCategoryCP newVal)
    {
    PropertyCategoryProxy oldCat(oldVal), newCat(newVal);

    change.Name().Set(oldCat.Name(), newCat.Name());
    change.DisplayLabel().Set(oldCat.DisplayLabel(), newCat.DisplayLabel());
    change.Description().Set(oldCat.Description(), newCat.Description());
    change.Priority().Set(oldCat.Priority(), newCat.Priority());
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::CompareKindOfQuantities(KindOfQuantityChanges& changes, KindOfQuantityContainerCP oldVal, KindOfQuantityContainerCP newVal)
    {
    bmap<Utf8CP, KindOfQuantityCP, CompareIUtf8Ascii> oldVals, newVals;
    bset<Utf8CP, CompareIUtf8Ascii> allVals;
    if (oldVal != nullptr)
        {
        for (KindOfQuantityCP koq : *oldVal)
            {
            Utf8CP name = koq->GetName().c_str();
            oldVals[name] = koq;
            allVals.insert(name);
            }
        }

    if (newVal != nullptr)
        {
        for (KindOfQuantityCP koq : *newVal)
            {
            Utf8CP name = koq->GetName().c_str();
            newVals[name] = koq;
            allVals.insert(name);
            }
        }

    for (Utf8CP koqName : allVals)
        {
        auto oldIt = oldVals.find(koqName);
        auto newIt = newVals.find(koqName);

        const bool existsInOld = oldIt != oldVals.end();
        const bool existsInNew = newIt != newVals.end();

        ECChange::OpCode opCode;
        KindOfQuantityCP oldKoq = nullptr;
        KindOfQuantityCP newKoq = nullptr;

        if (existsInOld && existsInNew)
            {
            opCode = ECChange::OpCode::Modified;
            oldKoq = oldIt->second;
            newKoq = newIt->second;
            }
        else if (existsInOld && !existsInNew)
            {
            opCode = ECChange::OpCode::Deleted;
            oldKoq = oldIt->second;
            }
        else if (!existsInOld && existsInNew)
            {
            opCode = ECChange::OpCode::New;
            newKoq = newIt->second;
            }
        else
            {
            LOG.errorv("KindOfQuantity comparison because no old or new koq with name %s was found.", koqName);
            BeAssert(false);
            return ERROR;
            }

        RefCountedPtr<KindOfQuantityChange> change = changes.CreateElement(opCode, ECChange::Type::KindOfQuantity, koqName);
        if (SUCCESS != CompareKindOfQuantity(*change, oldKoq, newKoq))
            {
            LOG.errorv("KindOfQuantity comparison failed for koq %s.", koqName);
            return ERROR;
            }

        changes.Add(change);
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::CompareKindOfQuantity(KindOfQuantityChange& change, KindOfQuantityCP oldVal, KindOfQuantityCP newVal)
    {
    KoqProxy oldKoq(oldVal), newKoq(newVal);

    change.Name().Set(oldKoq.Name(), newKoq.Name());
    change.DisplayLabel().Set(oldKoq.DisplayLabel(), newKoq.DisplayLabel());
    change.Description().Set(oldKoq.Description(), newKoq.Description());
    change.RelativeError().Set(oldKoq.RelativeError(), newKoq.RelativeError());
    change.PersistenceUnit().Set(oldKoq.PersistenceUnit(), newKoq.PersistenceUnit());

    bvector<Utf8String> oldFormats = oldKoq.PresentationFormats();
    bvector<Utf8String> newFormats = newKoq.PresentationFormats();
    const size_t oldPresFormatCount = oldFormats.size();
    const size_t newPresFormatCount = newFormats.size();
    const size_t maxPresFormatCount = std::max(oldPresFormatCount, newPresFormatCount);
    ECChangeArray<StringChange>& presFormatChanges = change.PresentationFormats();

    for (size_t i = 0; i < maxPresFormatCount; i++)
        {
        ECChange::OpCode opCode;
        Nullable<Utf8String> oldFormat, newFormat;
        if (i < oldPresFormatCount && i < newPresFormatCount)
            {
            opCode = ECChange::OpCode::Modified;
            oldFormat = oldFormats[i];
            newFormat = newFormats[i];
            }
        else if (i >= oldPresFormatCount)
            {
            opCode = ECChange::OpCode::New;
            newFormat = newFormats[i];
            }
        else if (i >= newPresFormatCount)
            {
            opCode = ECChange::OpCode::Deleted;
            oldFormat = oldFormats[i];
            }
        else
            {
            LOG.errorv("Unable to compare presentation formats for %s.", change.GetChangeName());
            BeAssert(false);
            return ERROR;
            }

        RefCountedPtr<StringChange> presFormatChange = presFormatChanges.CreateElement(opCode, ECChange::Type::KoqPresentationFormat);
        presFormatChange->Set(oldFormat, newFormat);
        presFormatChanges.Add(presFormatChange);
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::ComparePhenomena(PhenomenonChanges& changes, PhenomenonContainerCP oldVal, PhenomenonContainerCP newVal)
    {
    bmap<Utf8CP, PhenomenonCP, CompareIUtf8Ascii> oldVals, newVals;
    bset<Utf8CP, CompareIUtf8Ascii> allVals;
    if (oldVal != nullptr)
        {
        for (PhenomenonCP ph : *oldVal)
            {
            Utf8CP name = ph->GetName().c_str();
            oldVals[name] = ph;
            allVals.insert(name);
            }
        }

    if (newVal != nullptr)
        {
        for (PhenomenonCP ph : *newVal)
            {
            Utf8CP name = ph->GetName().c_str();
            newVals[name] = ph;
            allVals.insert(name);
            }
        }

    for (Utf8CP phName : allVals)
        {
        auto oldIt = oldVals.find(phName);
        auto newIt = newVals.find(phName);

        const bool existsInOld = oldIt != oldVals.end();
        const bool existsInNew = newIt != newVals.end();

        ECChange::OpCode opCode;
        PhenomenonCP oldPh = nullptr;
        PhenomenonCP newPh = nullptr;

        if (existsInOld && existsInNew)
            {
            opCode = ECChange::OpCode::Modified;
            oldPh = oldIt->second;
            newPh = newIt->second;
            }
        else if (existsInOld && !existsInNew)
            {
            opCode = ECChange::OpCode::Deleted;
            oldPh = oldIt->second;
            }
        else if (!existsInOld && existsInNew)
            {
            opCode = ECChange::OpCode::New;
            newPh = newIt->second;
            }
        else
            {
            LOG.errorv("Phenomenon comparison because no old or new phenomenon with name %s was found.", phName);
            BeAssert(false);
            return ERROR;
            }

        RefCountedPtr<PhenomenonChange> change = changes.CreateElement(opCode, ECChange::Type::Phenomenon, phName);
        if (SUCCESS != ComparePhenomenon(*change, oldPh, newPh))
            {
            LOG.errorv("Phenomenon comparison failed for koq %s.", phName);
            return ERROR;
            }

        changes.Add(change);
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::ComparePhenomenon(PhenomenonChange& change, PhenomenonCP oldVal, PhenomenonCP newVal)
    {
    PhenomenonProxy oldPh(oldVal), newPh(newVal);
    change.Name().Set(oldPh.Name(), newPh.Name());
    change.DisplayLabel().Set(oldPh.DisplayLabel(), newPh.DisplayLabel());
    change.Description().Set(oldPh.Description(), newPh.Description());
    change.Definition().Set(oldPh.Definition(), newPh.Definition());
    return SUCCESS;
    }
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::CompareUnitSystems(UnitSystemChanges& changes, UnitSystemContainerCP oldVal, UnitSystemContainerCP newVal)
    {
    bmap<Utf8CP, UnitSystemCP, CompareIUtf8Ascii> oldVals, newVals;
    bset<Utf8CP, CompareIUtf8Ascii> allVals;
    if (oldVal != nullptr)
        {
        for (UnitSystemCP us : *oldVal)
            {
            Utf8CP name = us->GetName().c_str();
            oldVals[name] = us;
            allVals.insert(name);
            }
        }

    if (newVal != nullptr)
        {
        for (UnitSystemCP us : *newVal)
            {
            Utf8CP name = us->GetName().c_str();
            newVals[name] = us;
            allVals.insert(name);
            }
        }

    for (Utf8CP usName : allVals)
        {
        auto oldIt = oldVals.find(usName);
        auto newIt = newVals.find(usName);

        const bool existsInOld = oldIt != oldVals.end();
        const bool existsInNew = newIt != newVals.end();

        ECChange::OpCode opCode;
        UnitSystemCP oldUs = nullptr;
        UnitSystemCP newUs = nullptr;

        if (existsInOld && existsInNew)
            {
            opCode = ECChange::OpCode::Modified;
            oldUs = oldIt->second;
            newUs = newIt->second;
            }
        else if (existsInOld && !existsInNew)
            {
            opCode = ECChange::OpCode::Deleted;
            oldUs = oldIt->second;
            }
        else if (!existsInOld && existsInNew)
            {
            opCode = ECChange::OpCode::New;
            newUs = newIt->second;
            }
        else
            {
            LOG.errorv("UnitSystem comparison because no old or new unit system with name %s was found.", usName);
            BeAssert(false);
            return ERROR;
            }

        RefCountedPtr<UnitSystemChange> change = changes.CreateElement(opCode, ECChange::Type::UnitSystem, usName);
        if (SUCCESS != CompareUnitSystem(*change, oldUs, newUs))
            {
            LOG.errorv("UnitSystem comparison failed for unit system %s.", usName);
            return ERROR;
            }


        changes.Add(change);
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::CompareUnitSystem(UnitSystemChange& change, UnitSystemCP oldVal, UnitSystemCP newVal)
    {
    UnitSystemProxy oldUs(oldVal), newUs(newVal);
    change.Name().Set(oldUs.Name(), newUs.Name());
    change.DisplayLabel().Set(oldUs.DisplayLabel(), newUs.DisplayLabel());
    change.Description().Set(oldUs.Description(), newUs.Description());
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::CompareUnits(UnitChanges& changes, UnitContainerCP oldVal, UnitContainerCP newVal)
    {
    bmap<Utf8CP, ECUnitCP, CompareIUtf8Ascii> oldVals, newVals;
    bset<Utf8CP, CompareIUtf8Ascii> allVals;
    if (oldVal != nullptr)
        {
        for (ECUnitCP u : *oldVal)
            {
            Utf8CP name = u->GetName().c_str();
            oldVals[name] = u;
            allVals.insert(name);
            }
        }

    if (newVal != nullptr)
        {
        for (ECUnitCP u : *newVal)
            {
            Utf8CP name = u->GetName().c_str();
            newVals[name] = u;
            allVals.insert(name);
            }
        }

    for (Utf8CP uName : allVals)
        {
        auto oldIt = oldVals.find(uName);
        auto newIt = newVals.find(uName);

        const bool existsInOld = oldIt != oldVals.end();
        const bool existsInNew = newIt != newVals.end();

        ECChange::OpCode opCode;
        ECUnitCP oldU = nullptr;
        ECUnitCP newU = nullptr;

        if (existsInOld && existsInNew)
            {
            opCode = ECChange::OpCode::Modified;
            oldU = oldIt->second;
            newU = newIt->second;
            }
        else if (existsInOld && !existsInNew)
            {
            opCode = ECChange::OpCode::Deleted;
            oldU = oldIt->second;
            }
        else if (!existsInOld && existsInNew)
            {
            opCode = ECChange::OpCode::New;
            newU = newIt->second;
            }
        else
            {
            LOG.errorv("Unit comparison because no old or new unit with name %s was found.", uName);
            BeAssert(false);
            return ERROR;
            }

        RefCountedPtr<UnitChange> change = changes.CreateElement(opCode, ECChange::Type::Unit, uName);
        if (SUCCESS != CompareUnit(*change, oldU, newU))
            {
            LOG.errorv("Unit comparison failed for unit %s.", uName);
            return ERROR;
            }

        changes.Add(change);
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::CompareUnit(UnitChange& change, ECUnitCP oldVal, ECUnitCP newVal)
    {
    UnitProxy oldUnit(oldVal), newUnit(newVal);

    change.Name().Set(oldUnit.Name(), newUnit.Name());
    change.DisplayLabel().Set(oldUnit.DisplayLabel(), newUnit.DisplayLabel());
    change.Description().Set(oldUnit.Description(), newUnit.Description());

    change.Phenomenon().Set(oldUnit.Phenomenon(), newUnit.Phenomenon());
    change.UnitSystem().Set(oldUnit.UnitSystem(), newUnit.UnitSystem());

    change.IsConstant().Set(oldUnit.IsConstant(), newUnit.IsConstant());
    change.InvertingUnit().Set(oldUnit.InvertingUnit(), newUnit.InvertingUnit());

    change.Definition().Set(oldUnit.Definition(), newUnit.Definition());
    change.Numerator().Set(oldUnit.Numerator(), newUnit.Numerator());
    change.Denominator().Set(oldUnit.Denominator(), newUnit.Denominator());
    change.Offset().Set(oldUnit.Offset(), newUnit.Offset());

    return SUCCESS;
    }
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::CompareFormats(FormatChanges& changes, ECN::FormatContainerCP oldVal, ECN::FormatContainerCP newVal)
    {
    bmap<Utf8CP, ECFormatCP, CompareIUtf8Ascii> oldVals, newVals;
    bset<Utf8CP, CompareIUtf8Ascii> allVals;
    if (oldVal != nullptr)
        {
        for (ECFormatCP f : *oldVal)
            {
            Utf8CP name = f->GetName().c_str();
            oldVals[name] = f;
            allVals.insert(name);
            }
        }

    if (newVal != nullptr)
        {
        for (ECFormatCP f : *newVal)
            {
            Utf8CP name = f->GetName().c_str();
            newVals[name] = f;
            allVals.insert(name);
            }
        }

    for (Utf8CP fName : allVals)
        {
        auto oldIt = oldVals.find(fName);
        auto newIt = newVals.find(fName);

        const bool existsInOld = oldIt != oldVals.end();
        const bool existsInNew = newIt != newVals.end();

        ECChange::OpCode opCode;
        ECFormatCP oldF = nullptr;
        ECFormatCP newF = nullptr;

        if (existsInOld && existsInNew)
            {
            opCode = ECChange::OpCode::Modified;
            oldF = oldIt->second;
            newF = newIt->second;
            }
        else if (existsInOld && !existsInNew)
            {
            opCode = ECChange::OpCode::Deleted;
            oldF = oldIt->second;
            }
        else if (!existsInOld && existsInNew)
            {
            opCode = ECChange::OpCode::New;
            newF = newIt->second;
            }
        else
            {
            LOG.errorv("Format comparison because no old or new format with name %s was found.", fName);
            BeAssert(false);
            return ERROR;
            }

        RefCountedPtr<FormatChange> change = changes.CreateElement(opCode, ECChange::Type::Format, fName);
        if (SUCCESS != CompareFormat(*change, oldF, newF))
            {
            LOG.errorv("Format comparison failed for format %s.", fName);
            return ERROR;
            }

        changes.Add(change);
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::CompareFormat(FormatChange& change, ECN::ECFormatCP oldVal, ECN::ECFormatCP newVal)
    {
    FormatProxy oldFormat(oldVal), newFormat(newVal);

    change.Name().Set(oldFormat.Name(), newFormat.Name());
    change.DisplayLabel().Set(oldFormat.DisplayLabel(), newFormat.DisplayLabel());
    change.Description().Set(oldFormat.Description(), newFormat.Description());

    if (SUCCESS != change.NumericSpec().SetFrom(oldFormat.NumericSpec(), newFormat.NumericSpec()))
        {
        LOG.errorv("Could not compare Numeric Specification format %s.", change.GetChangeName());
        return ERROR;
        }

    return change.CompositeSpec().SetFrom(oldFormat.CompositeSpec(), newFormat.CompositeSpec());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::CompareCustomAttributes(CustomAttributeChanges& changes, bvector<ECN::IECInstancePtr> const& oldVal, bvector<ECN::IECInstancePtr> const& newVal)
    {
    bmap<Utf8CP, IECInstancePtr, CompareIUtf8Ascii> oldVals, newVals;
    bset<Utf8CP, CompareIUtf8Ascii> allVals;
    for (IECInstancePtr const& ca : oldVal)
        {
        Utf8CP name = ca->GetClass().GetFullName();
        oldVals[name] = ca;
        allVals.insert(name);
        }

    for (IECInstancePtr const& ca : newVal)
        {
        Utf8CP name = ca->GetClass().GetFullName();
        newVals[name] = ca;
        allVals.insert(name);
        }

    for (Utf8CP caName : allVals)
        {
        auto oldIt = oldVals.find(caName);
        auto newIt = newVals.find(caName);

        const bool existsInOld = oldIt != oldVals.end();
        const bool existsInNew = newIt != newVals.end();

        ECChange::OpCode opCode;
        IECInstancePtr oldCA = nullptr;
        IECInstancePtr newCA = nullptr;

        if (existsInOld && existsInNew)
            {
            opCode = ECChange::OpCode::Modified;
            oldCA = oldIt->second;
            newCA = newIt->second;
            }
        else if (existsInOld && !existsInNew)
            {
            opCode = ECChange::OpCode::Deleted;
            oldCA = oldIt->second;
            }
        else if (!existsInOld && existsInNew)
            {
            opCode = ECChange::OpCode::New;
            newCA = newIt->second;
            }
        else
            {
            LOG.errorv("CustomAttribute comparison because no old or new custom attribute with name %s was found.", caName);
            BeAssert(false);
            return ERROR;
            }

        RefCountedPtr<CustomAttributeChange> change = changes.CreateElement(opCode, ECChange::Type::CustomAttribute, caName);
        if (SUCCESS != CompareCustomAttribute(*change, oldCA.get(), newCA.get()))
            {
            LOG.errorv("CustomAttribute comparison failed for custom attribute %s.", caName);
            return ERROR;
            }

        changes.Add(change);
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::CompareCustomAttribute(CustomAttributeChange& change, IECInstanceCP oldVal, IECInstanceCP newVal)
    {
    bmap<Utf8String, ECValue> oldPropValues, newPropValues;
    bset<Utf8CP, CompareUtf8> allAccessStrings;
    if (oldVal != nullptr)
        {
        if (ConvertECInstanceToValueMap(oldPropValues, *oldVal) != SUCCESS)
            {
            LOG.errorv("Unable to convert old custom attribute to value map %s.", change.GetChangeName());
            return ERROR;
            }
        }

    if (newVal != nullptr)
        {
        if (ConvertECInstanceToValueMap(newPropValues, *newVal) != SUCCESS)
            {
            LOG.errorv("Unable to convert new custom attribute to value map %s.", change.GetChangeName());
            return ERROR;
            }
        }
    for (auto const& kvPair : oldPropValues)
        {
        allAccessStrings.insert(kvPair.first.c_str());
        }

    for (auto const& kvPair : newPropValues)
        {
        allAccessStrings.insert(kvPair.first.c_str());
        }

    for (Utf8CP accessString : allAccessStrings)
        {
        auto oldIt = oldPropValues.find(accessString);
        auto newIt = newPropValues.find(accessString);

        bool existInOld = oldIt != oldPropValues.end();
        bool existInNew = newIt != newPropValues.end();
        ECChange::OpCode opCode;
        ECValue oldValue;
        ECValue newValue;

        if (existInOld && existInNew)
            {
            oldValue = oldIt->second;
            newValue = newIt->second;

            if (newValue.Equals(oldValue))
                continue;

            opCode = ECChange::OpCode::Modified;
            }
        else if (existInOld && !existInNew)
            {
            opCode = ECChange::OpCode::Deleted;
            oldValue = oldIt->second;
            }
        else if (!existInOld && existInNew)
            {
            opCode = ECChange::OpCode::New;
            newValue = newIt->second;
            }
        else
            {
            LOG.errorv("CustomAttribute property value comparison because no old or new value was found with access string %s.", accessString);
            BeAssert(false);
            return ERROR;
            }

        RefCountedPtr<PropertyValueChange> pvChange = change.PropValues().CreateElement(opCode, ECChange::Type::PropertyValue, accessString);
        if (SUCCESS != pvChange->Set(oldValue, newValue))
            {
            LOG.errorv("Failed to create a change for Custom Attribute property with access string %s.", accessString);
            return ERROR;
            }

        change.PropValues().Add(pvChange);
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::ConvertECInstanceToValueMap(bmap<Utf8String, ECValue>& map, IECInstanceCR instance)
    {
    ECValuesCollectionPtr values = ECValuesCollection::Create(instance);
    if (values.IsNull())
        return SUCCESS;

    return ConvertECValuesCollectionToValueMap(map, *values);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaComparer::ConvertECValuesCollectionToValueMap(bmap<Utf8String, ECValue>& map, ECValuesCollectionCR values)
    {
    for (ECValuesCollection::const_iterator itor = values.begin(); itor != values.end(); ++itor)
        {
        ECValueAccessorCR valueAccessor = (*itor).GetValueAccessor();

        if ((*itor).HasChildValues())
            {
            if (ConvertECValuesCollectionToValueMap(map, *(*itor).GetChildValues()) != SUCCESS)
                return ERROR;   // Add logging message if this method can ever return something besides SUCCESS.
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

//======================================================================================
//ECChange
//======================================================================================
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Utf8CP ECChange::GetChangeName() const
    {
    if (!m_changeName.empty())
        return m_changeName.c_str();

    return TypeToString(m_type);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
void ECChange::AppendBegin(Utf8StringR str, ECChange const& change, int currentIndex)
    {
    if (change.GetOpCode() == OpCode::Deleted)
        str += "-";
    else if (change.GetOpCode() == OpCode::New)
        str += "+";
    else if (change.GetOpCode() == OpCode::Modified)
        str += "!";

    for (int i = 0; i < currentIndex; i++)
        str.append(" ");

    str.append(TypeToString(change.GetType()));

    if (change.HasChangeName())
        str.append("(").append(change.GetChangeName()).append(")");
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
Utf8CP ECChange::TypeToString(Type type)
    {
    switch (type)
        {
            case Type::AbstractConstraint: return "AbstractConstraint";
            case Type::Alias: return "Alias";
            case Type::BaseClass: return "BaseClass";
            case Type::BaseClasses: return "BaseClasses";
            case Type::Classes: return "Classes";
            case Type::Class: return "Class";
            case Type::ClassModifier: return "ClassModifier";
            case Type::ClassType: return "ClassType";
            case Type::CompositeIncludeZero: return "CompositeIncludeZero";
            case Type::CompositeSpacer: return "CompositeSpacer";
            case Type::CompositeMajorUnit: return "CompositeMajorUnit";
            case Type::CompositeMajorLabel: return "CompositeMajorLabel";
            case Type::CompositeMiddleUnit: return "CompositeMiddleUnit";
            case Type::CompositeMiddleLabel: return "CompositeMiddleLabel";
            case Type::CompositeMinorUnit: return "CompositeMinorUnit";
            case Type::CompositeMinorLabel: return "CompositeMinorLabel";
            case Type::CompositeSubUnit: return "CompositeSubUnit";
            case Type::CompositeSubLabel: return "CompositeSubLabel";
            case Type::CompositeValueSpec: return "CompositeValueSpec";
            case Type::ConstraintClass: return "ConstraintClass";
            case Type::ConstraintClasses: return "ConstraintClasses";
            case Type::Constraint: return "Constraint";
            case Type::CustomAttribute: return "CustomAttribute";
            case Type::CustomAttributes: return "CustomAttributes";
            case Type::DecimalPrecision: return "DecimalPrecision";
            case Type::DecimalSeparator: return "DecimalSeparator";
            case Type::Description: return "Description";
            case Type::Direction: return "Direction";
            case Type::DisplayLabel: return "DisplayLabel";
            case Type::ECVersion: return "ECVersion";
            case Type::Enumeration: return "Enumeration";
            case Type::Enumerations: return "Enumerations";
            case Type::Enumerator: return "Enumerator";
            case Type::Enumerators: return "Enumerators";
            case Type::ExtendedTypeName: return "ExtendedTypeName";
            case Type::Format: return "Format";
            case Type::Formats: return "Formats";
            case Type::FormatTraits: return "FormatTraits";
            case Type::FractionalPrecision: return "FractionalPrecision";
            case Type::Integer: return "Integer";
            case Type::IsInteger: return "IsInteger";
            case Type::IsPolymorphic: return "IsPolymorphic";
            case Type::IsReadonly: return "IsReadonly";
            case Type::IsStrict: return "IsStrict";
            case Type::IsString: return "IsString";
            case Type::IsStruct: return "IsStruct";
            case Type::IsStructArray: return "IsStructArray";
            case Type::IsPrimitive: return "IsPrimitive";
            case Type::IsPrimitiveArray: return "IsPrimitiveArray";
            case Type::IsNavigation: return "IsNavigation";
            case Type::KindOfQuantities: return "KindOfQuantities";
            case Type::KindOfQuantity: return "KindOfQuantity";
            case Type::KoqPersistenceUnit: return "KoqPersistenceUnit";
            case Type::KoqPresentationFormat: return "KoqPresentationFormat";
            case Type::KoqPresentationFormats: return "KoqPresentationFormats";
            case Type::KoqRelativeError: return "KoqRelativeError";
            case Type::MaximumLength: return "MaximumLength";
            case Type::MaximumValue: return "MaximumValue";
            case Type::MaxOccurs: return "MaxOccurs";
            case Type::MinimumLength: return "MinimumLength";
            case Type::MinimumValue: return "MinimumValue";
            case Type::MinOccurs: return "MinOccurs";
            case Type::MinWidth: return "MinWidth";
            case Type::Multiplicity: return "Multiplicity";
            case Type::Name: return "Name";
            case Type::NumericFormatSpec: return "NumericFormatSpec";
            case Type::OriginalECXmlVersionMajor: return "OriginalECXmlVersionMajor";
            case Type::OriginalECXmlVersionMinor: return "OriginalECXmlVersionMinor";
            case Type::Phenomena: return "Phenomena";
            case Type::Phenomenon: return "Phenomenon";
            case Type::PhenomenonDefinition: return "PhenomenonDefinition";
            case Type::PresentationType: return "PresentationType";
            case Type::Properties: return "Properties";
            case Type::Property: return "Property";
            case Type::PropertyCategories: return "PropertyCategories";
            case Type::PropertyCategory: return "PropertyCategory";
            case Type::PropertyCategoryPriority: return "PropertyCategoryPriority";
            case Type::PropertyPriority: return "PropertyPriority";
            case Type::PropertyType: return "PropertyType";
            case Type::PropertyValue: return "PropertyValue";
            case Type::PropertyValues: return "PropertyValues";
            case Type::Relationship: return "Relationship";
            case Type::RoleLabel: return "RoleLabel";
            case Type::RoundFactor: return "RoundFactor";
            case Type::Schema: return "Schema";
            case Type::SchemaReference: return "SchemaReference";
            case Type::SchemaReferences: return "SchemaReferences";
            case Type::Schemas: return "Schemas";
            case Type::ScientificType: return "ScientificType";
            case Type::ShowSignOption: return "ShowSignOption";
            case Type::Source: return "Source";
            case Type::StationSeparator: return "StationSeparator";
            case Type::StationOffsetSize: return "StationOffsetSize";
            case Type::StrengthDirection: return "StrengthDirection";
            case Type::StrengthType: return "StrengthType";
            case Type::String: return "String";
            case Type::Target: return "Target";
            case Type::ThousandSeparator: return "ThousandSeparator";
            case Type::TypeName: return "TypeName";
            case Type::UnitSystem: return "UnitSystem";
            case Type::UnitSystems: return "UnitSystems";
            case Type::Unit: return "Unit";
            case Type::UnitDefinition: return "UnitDefinition";
            case Type::UnitNumerator: return "UnitNumerator";
            case Type::UnitDenominator: return "UnitDenominator";
            case Type::UnitInvertingUnit: return "UnitInvertingUnit";
            case Type::UnitIsConstant: return "UnitIsConstant";
            case Type::UnitOffset: return "UnitOffset";
            case Type::Units: return "Units";
            case Type::UomSeparator: return "UomSeparator";
            case Type::VersionRead: return "VersionRead";
            case Type::VersionMinor: return "VersionMinor";
            case Type::VersionWrite: return "VersionWrite";
        }

    LOG.errorv("Encountered unhandled ECChange Type, %i", type);
    BeAssert(false && "Unhandled Type");
    return "";
    }

//======================================================================================>
//ECObjectChange
//======================================================================================>
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void CompositeECChange::_WriteToString(Utf8StringR str, int currentIndex, int indentSize) const
    {
    if (m_changes.empty() || !IsChanged())
        return;

    AppendBegin(str, *this, currentIndex);
    AppendEnd(str);
    for (auto& change : m_changes)
        {
        change.second->WriteToString(str, currentIndex + indentSize, indentSize);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool CompositeECChange::_IsChanged() const
    {
    for (auto& change : m_changes)
        {
        if (change.second->IsChanged())
            return true;
        }

    return false;
    }

//======================================================================================>
//ECPropertyValueChange
//======================================================================================>
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PropertyValueChange::Set(ECValueCR oldValue, ECValueCR newValue)
    {
    if (oldValue.IsNull() && newValue.IsNull())
        return SUCCESS;

    const bool bothAreNotNull = !oldValue.IsNull() && !newValue.IsNull();
    if ((!oldValue.IsNull() && !oldValue.IsPrimitive()) || (!newValue.IsNull() && !newValue.IsPrimitive()))
        {
        LOG.error("Only primitive values are supported as a PropertyValueChange");
        BeAssert(false && "Only primitive values are supported for PropertyValueChange");
        return ERROR;
        }

    if (bothAreNotNull && oldValue.GetPrimitiveType() != newValue.GetPrimitiveType())
        {
        LOG.error("PropertyValueChange cannot be set because the primitive types of the old and new values differ.");
        return ERROR;
        }

    PrimitiveType primType = oldValue.IsPrimitive() ? oldValue.GetPrimitiveType() : newValue.GetPrimitiveType();

    if (Inititalize(primType) != SUCCESS)
        {
        LOG.error("Failed to set PropertyValueChange because initialization failed.");
        return ERROR;
        }

    switch (m_primType)
        {
            case PRIMITIVETYPE_IGeometry:
                {
                LOG.error ("PropertyValueChange does not support the Geometry type");
                BeAssert(false && "Geometry not supported type");
                return ERROR;
                }
            case PRIMITIVETYPE_Binary:
            {
            Nullable<bvector<Byte>> oldBlob;
            if (!oldValue.IsNull())
                {
                size_t binarySize = 0;
                Byte const* binary = oldValue.GetBinary(binarySize);
                oldBlob = bvector<Byte>();
                oldBlob.ValueR().resize(binarySize);
                memcpy(oldBlob.ValueR().data(), binary, binarySize);
                }

            Nullable<bvector<Byte>> newBlob;
            if (!newValue.IsNull())
                {
                size_t binarySize = 0;
                Byte const* binary = newValue.GetBinary(binarySize);
                newBlob = bvector<Byte>();
                newBlob.ValueR().resize(binarySize);
                memcpy(newBlob.ValueR().data(), binary, binarySize);
                }

            return GetBinary()->Set(oldBlob, newBlob);
            }
            case PRIMITIVETYPE_Boolean:
            {
            Nullable<bool> oldVal = oldValue.IsNull() ? Nullable<bool>() : oldValue.GetBoolean();
            Nullable<bool> newVal = newValue.IsNull() ? Nullable<bool>() : newValue.GetBoolean();
            return GetBoolean()->Set(oldVal, newVal);
            }
            case PRIMITIVETYPE_DateTime:
            {
            Nullable<DateTime> oldVal = oldValue.IsNull() ? Nullable<DateTime>() : oldValue.GetDateTime();
            Nullable<DateTime> newVal = newValue.IsNull() ? Nullable<DateTime>() : newValue.GetDateTime();
            return GetDateTime()->Set(oldVal, newVal);
            }
            case PRIMITIVETYPE_Double:
            {
            Nullable<double> oldVal = oldValue.IsNull() ? Nullable<double>() : oldValue.GetDouble();
            Nullable<double> newVal = newValue.IsNull() ? Nullable<double>() : newValue.GetDouble();
            return GetDouble()->Set(oldVal, newVal);
            }
            case PRIMITIVETYPE_Integer:
            {
            Nullable<int> oldVal = oldValue.IsNull() ? Nullable<int>() : oldValue.GetInteger();
            Nullable<int> newVal = newValue.IsNull() ? Nullable<int>() : newValue.GetInteger();
            return GetInteger()->Set(oldVal, newVal);
            }
            case PRIMITIVETYPE_Long:
            {
            Nullable<int64_t> oldVal = oldValue.IsNull() ? Nullable<int64_t>() : oldValue.GetLong();
            Nullable<int64_t> newVal = newValue.IsNull() ? Nullable<int64_t>() : newValue.GetLong();
            return GetLong()->Set(oldVal, newVal);
            }
            case PRIMITIVETYPE_Point2d:
            {
            Nullable<DPoint2d> oldVal = oldValue.IsNull() ? Nullable<DPoint2d>() : oldValue.GetPoint2d();
            Nullable<DPoint2d> newVal = newValue.IsNull() ? Nullable<DPoint2d>() : newValue.GetPoint2d();
            return GetPoint2d()->Set(oldVal, newVal);
            }
            case PRIMITIVETYPE_Point3d:
            {
            Nullable<DPoint3d> oldVal = oldValue.IsNull() ? Nullable<DPoint3d>() : oldValue.GetPoint3d();
            Nullable<DPoint3d> newVal = newValue.IsNull() ? Nullable<DPoint3d>() : newValue.GetPoint3d();
            return GetPoint3d()->Set(oldVal, newVal);
            }
            case PRIMITIVETYPE_String:
            {
            Nullable<Utf8String> oldVal = oldValue.IsNull() ? Nullable<Utf8String>() : Utf8String(oldValue.GetUtf8CP());
            Nullable<Utf8String> newVal = newValue.IsNull() ? Nullable<Utf8String>() : Utf8String(newValue.GetUtf8CP());
            return GetString()->Set(oldVal, newVal);
            }

            default:
                LOG.errorv("Unhandled PrimitiveType: %i", m_primType);
                BeAssert(false && "Unhandled PrimitiveType");
                return ERROR;
        }
    }
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void PropertyValueChange::_WriteToString(Utf8StringR str, int currentIndex, int indentSize) const
    {
    if (!IsChanged())
        return;

    m_value->WriteToString(str, currentIndex, indentSize);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PropertyValueChange::Inititalize(PrimitiveType type)
    {
    if (m_value != nullptr)
        {
        LOG.error("PropertyValueChange cannot be initialized because it has already been used");
        BeAssert(false && "PropertyValueChange has already been used");
        return ERROR;
        }

    m_primType = type;
    switch (type)
        {
            case ECN::PRIMITIVETYPE_Binary:
                m_value = std::make_unique<BinaryChange>(GetOpCode(), Type::PropertyValue, this, GetChangeName());
                return SUCCESS;
            case ECN::PRIMITIVETYPE_Boolean:
                m_value = std::unique_ptr<ECChange>(new BooleanChange(GetOpCode(), Type::PropertyValue, this, GetChangeName()));
                return SUCCESS;
            case ECN::PRIMITIVETYPE_DateTime:
                m_value = std::unique_ptr<ECChange>(new DateTimeChange(GetOpCode(), Type::PropertyValue, this, GetChangeName()));
                return SUCCESS;
            case ECN::PRIMITIVETYPE_Double:
                m_value = std::unique_ptr<ECChange>(new DoubleChange(GetOpCode(), Type::PropertyValue, this, GetChangeName()));
                return SUCCESS;
            case ECN::PRIMITIVETYPE_IGeometry:
            {
            LOG.errorv("ECSchemaComparer: Changes in ECProperties of type IGeometry are not supported.");
            return ERROR;
            }
            case ECN::PRIMITIVETYPE_Integer:
                m_value = std::unique_ptr<ECChange>(new Int32Change(GetOpCode(), Type::PropertyValue, this, GetChangeName()));
                return SUCCESS;
            case ECN::PRIMITIVETYPE_Long:
                m_value = std::unique_ptr<ECChange>(new Int64Change(GetOpCode(), Type::PropertyValue, this, GetChangeName()));
                return SUCCESS;
            case ECN::PRIMITIVETYPE_Point2d:
                m_value = std::unique_ptr<ECChange>(new Point2dChange(GetOpCode(), Type::PropertyValue, this, GetChangeName()));
                return SUCCESS;
            case ECN::PRIMITIVETYPE_Point3d:
                m_value = std::unique_ptr<ECChange>(new Point3dChange(GetOpCode(), Type::PropertyValue, this, GetChangeName()));
                return SUCCESS;
            case ECN::PRIMITIVETYPE_String:
                m_value = std::unique_ptr<ECChange>(new StringChange(GetOpCode(), Type::PropertyValue, this, GetChangeName()));
                return SUCCESS;
            default:
                LOG.errorv("Unexpected value for PrimitiveType %i", type);
                BeAssert(false && "Unexpected value for PrimitiveType");
                return ERROR;
        }
    }


//***********************************************************************
// NumericFormatSpecChange
//***********************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus NumericFormatSpecChange::SetFrom(Formatting::NumericFormatSpecCP oldSpec, Formatting::NumericFormatSpecCP newSpec)
    {
    if (GetOpCode() == OpCode::New && oldSpec != nullptr)
        {
        LOG.errorv("Cannot Set format change for %s to New because old spec must not be nullptr for that opcode.", GetChangeName());
        BeAssert(false && "Old spec must not be nullptr if OpCode is New");
        return ERROR;
        }

    if (GetOpCode() == OpCode::Deleted && newSpec != nullptr)
        {
        LOG.errorv("Cannot set format change for %s to Deleted because new spec must not be nullptr for that opcode.", GetChangeName());
        BeAssert(false && "New spec must not be nullptr if OpCode is Deleted");
        return ERROR;
        }

    {
    Nullable<Utf8String> oldVal = oldSpec != nullptr ? Formatting::Utils::GetPresentationTypeString(oldSpec->GetPresentationType()) : Nullable<Utf8String>();
    Nullable<Utf8String> newVal = newSpec != nullptr ? Formatting::Utils::GetPresentationTypeString(newSpec->GetPresentationType()) : Nullable<Utf8String>();
    PresentationType().Set(oldVal, newVal);
    }

    {
    Nullable<Utf8String> oldVal = oldSpec != nullptr ? Formatting::Utils::GetScientificTypeString(oldSpec->GetScientificType()) : Nullable<Utf8String>();
    Nullable<Utf8String> newVal = newSpec != nullptr ? Formatting::Utils::GetScientificTypeString(newSpec->GetScientificType()) : Nullable<Utf8String>();
    ScientificType().Set(oldVal, newVal);
    }

    {
    Nullable<double> oldVal = oldSpec != nullptr && oldSpec->HasRoundingFactor() ? oldSpec->GetRoundingFactor() : Nullable<double>();
    Nullable<double> newVal = newSpec != nullptr && newSpec->HasRoundingFactor() ? newSpec->GetRoundingFactor() : Nullable<double>();
    RoundFactor().Set(oldVal, newVal);
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

    DecimalPrecision().Set(oldDecPrec, newDecPrec);
    FractionalPrecision().Set(oldFractPrec, newFractPrec);
    }

    {
    Nullable<uint32_t> oldVal;
    if (oldSpec != nullptr && oldSpec->HasMinWidth())
        oldVal = (uint32_t) oldSpec->GetMinWidth();

    Nullable<uint32_t> newVal;
    if (newSpec != nullptr && newSpec->HasMinWidth())
        newVal = (uint32_t) newSpec->GetMinWidth();

    MinWidth().Set(oldVal, newVal);
    }

    {
    Nullable<Utf8String> oldVal;
    if (oldSpec != nullptr && oldSpec->HasSignOption())
        oldVal = Formatting::Utils::GetSignOptionString(oldSpec->GetSignOption());

    Nullable<Utf8String> newVal;
    if (newSpec != nullptr && newSpec->HasSignOption())
        newVal = Formatting::Utils::GetSignOptionString(newSpec->GetSignOption());

    ShowSignOption().Set(oldVal, newVal);
    }

    {
    Nullable<Utf8String> oldVal;
    if (oldSpec != nullptr && oldSpec->HasFormatTraits())
        oldVal = oldSpec->GetFormatTraitsString();

    Nullable<Utf8String> newVal;
    if (newSpec != nullptr && newSpec->HasFormatTraits())
        newVal = newSpec->GetFormatTraitsString();

    FormatTraits().Set(oldVal, newVal);
    }

    {
    Nullable<Utf8String> oldVal;
    if (oldSpec != nullptr && oldSpec->HasDecimalSeparator())
        oldVal = Utf8String(1, oldSpec->GetDecimalSeparator());

    Nullable<Utf8String> newVal;
    if (newSpec != nullptr && newSpec->HasDecimalSeparator())
        newVal = Utf8String(1, newSpec->GetDecimalSeparator());

    DecimalSeparator().Set(oldVal, newVal);
    }

    {
    Nullable<Utf8String> oldVal;
    if (oldSpec != nullptr && oldSpec->HasThousandsSeparator())
        oldVal = Utf8String(1, oldSpec->GetThousandSeparator());

    Nullable<Utf8String> newVal;
    if (newSpec != nullptr && newSpec->HasThousandsSeparator())
        newVal = Utf8String(1, newSpec->GetThousandSeparator());

    ThousandsSeparator().Set(oldVal, newVal);
    }

    {
    Nullable<Utf8String> oldVal;
    if (oldSpec != nullptr && oldSpec->HasUomSeparator())
        oldVal = Utf8String(oldSpec->GetUomSeparator());

    Nullable<Utf8String> newVal;
    if (newSpec != nullptr && newSpec->HasUomSeparator())
        newVal = Utf8String(newSpec->GetUomSeparator());

    UomSeparator().Set(oldVal, newVal);
    }

    {
    Nullable<Utf8String> oldVal;
    if (oldSpec != nullptr && oldSpec->HasStationSeparator())
        oldVal = Utf8String(1, oldSpec->GetStationSeparator());

    Nullable<Utf8String> newVal;
    if (newSpec != nullptr && newSpec->HasStationSeparator())
        newVal = Utf8String(1, newSpec->GetStationSeparator());

    StationSeparator().Set(oldVal, newVal);
    }

    {
    Nullable<uint32_t> oldVal;
    if (oldSpec != nullptr)
        oldVal = oldSpec->GetStationOffsetSize();

    Nullable<uint32_t> newVal;
    if (newSpec != nullptr)
        newVal = newSpec->GetStationOffsetSize();

    StationOffsetSize().Set(oldVal, newVal);
    }

    return SUCCESS;
    }

//***********************************************************************
// CompositeValueSpecChange
//***********************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus CompositeValueSpecChange::SetFrom(Formatting::CompositeValueSpecCP oldSpec, Formatting::CompositeValueSpecCP newSpec)
    {
    if (GetOpCode() == OpCode::New && oldSpec != nullptr)
        {
        LOG.errorv("Cannot Set composite value change for %s to New because old spec must not be nullptr for that opcode.", GetChangeName());
        BeAssert(false && "Old spec must not be nullptr if OpCode is New");
        return ERROR;
        }

    if (GetOpCode() == OpCode::Deleted && newSpec != nullptr)
        {
        LOG.errorv("Cannot set composite value change for %s to Deleted because new spec must not be nullptr for that opcode.", GetChangeName());
        BeAssert(false && "New spec must not be nullptr if OpCode is Deleted");
        return ERROR;
        }

    {
    Nullable<Utf8String> oldVal;
    if (oldSpec != nullptr && oldSpec->HasSpacer())
        oldVal = oldSpec->GetSpacer();

    Nullable<Utf8String> newVal;
    if (newSpec != nullptr && newSpec->HasSpacer())
        newVal = newSpec->GetSpacer();

    Spacer().Set(oldVal, newVal);
    }

    {
    Nullable<bool> oldVal = oldSpec != nullptr ? oldSpec->IsIncludeZero() : Nullable<bool>();
    Nullable<bool> newVal = newSpec != nullptr ? newSpec->IsIncludeZero() : Nullable<bool>();
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

    MajorUnit().Set(oldVal, newVal);
    }

    {
    Nullable<Utf8String> oldVal;
    if (oldSpec != nullptr && oldSpec->HasMajorLabel())
        oldVal = oldSpec->GetMajorLabel();

    Nullable<Utf8String> newVal;
    if (newSpec != nullptr && newSpec->HasMajorLabel())
        newVal = newSpec->GetMajorLabel();

    MajorLabel().Set(oldVal, newVal);
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

    MiddleUnit().Set(oldVal, newVal);
    }

    {
    Nullable<Utf8String> oldVal;
    if (oldSpec != nullptr && oldSpec->HasMiddleLabel())
        oldVal = oldSpec->GetMiddleLabel();

    Nullable<Utf8String> newVal;
    if (newSpec != nullptr && newSpec->HasMiddleLabel())
        newVal = newSpec->GetMiddleLabel();

    MiddleLabel().Set(oldVal, newVal);
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

     MinorUnit().Set(oldVal, newVal);
    }

    {
    Nullable<Utf8String> oldVal;
    if (oldSpec != nullptr && oldSpec->HasMinorLabel())
        oldVal = oldSpec->GetMinorLabel();

    Nullable<Utf8String> newVal;
    if (newSpec != nullptr && newSpec->HasMinorLabel())
        newVal = newSpec->GetMinorLabel();

    MinorLabel().Set(oldVal, newVal);
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

    SubUnit().Set(oldVal, newVal);
    }

    {
    Nullable<Utf8String> oldVal;
    if (oldSpec != nullptr && oldSpec->HasSubLabel())
        oldVal = oldSpec->GetSubLabel();

    Nullable<Utf8String> newVal;
    if (newSpec != nullptr && newSpec->HasSubLabel())
        newVal = newSpec->GetSubLabel();

    SubLabel().Set(oldVal, newVal);
    }

    return SUCCESS;
    }


//======================================================================================>
//CustomAttributeValidator
//======================================================================================>
Utf8String CustomAttributeValidator::WILDCARD = "*";

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
CustomAttributeValidator::Policy CustomAttributeValidator::Validate(CustomAttributeChange const& change) const
    {
    bvector<Utf8String> tmp;
    BeStringUtilities::Split(change.GetChangeName(), ":", tmp);
    Utf8StringCR schemaName = tmp.front();
    Utf8StringCR className = tmp.back();

    ChangeType classChangeType = change.GetOpCode() == ECChange::OpCode::New ? ChangeType::New :
        (change.GetOpCode() == ECChange::OpCode::Modified ? ChangeType::Modified : ChangeType::Delete);

    bvector<ClassRule const*> applicableRules;
    for (auto classRule : m_rules)
        {
        if (!classRule->AppliesToChangeType(classChangeType))
            continue;

        if (classRule->GetSchemaName() == WILDCARD)
            {
            applicableRules.push_back(classRule.get());
            continue;
            }

        if (!classRule->GetSchemaName().EqualsIAscii(schemaName))
            continue;

        if (classRule->GetClassName() == WILDCARD || classRule->GetClassName().EqualsIAscii(className))
            applicableRules.push_back(classRule.get());
        }

    if (change.PropValues().IsEmpty())
        {
        for (auto const* classRule : applicableRules)
            {
            if (classRule->GetRules().empty())
                return classRule->GetPolicy();
            }
        }

    for (auto propValueChangePtr : change.PropValues())
        {
        auto const& accessStringTokens = AccessStringToTokens(propValueChangePtr->GetAccessString());
        ChangeType propertyChangeType = propValueChangePtr->GetOpCode() == ECChange::OpCode::New ? ChangeType::New :
            (change.GetOpCode() == ECChange::OpCode::Modified ? ChangeType::Modified : ChangeType::Delete);

        for (auto const* classRule : applicableRules)
            {
            if (classRule->AppliesToPropertyChange(propertyChangeType, accessStringTokens))
                {
                if (Policy::Reject == classRule->GetPolicy())
                    return classRule->GetPolicy();

                break;
                }
            }
        }

    return Policy::Accept;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bvector<Utf8String> CustomAttributeValidator::AccessStringToTokens(Utf8CP accessString) const
    {
    bvector<Utf8String> accessStringTokens;
    BeStringUtilities::Split(accessString, ".", accessStringTokens);

    // Trim element's array position info ('[]')
    for (auto& accessStringToken : accessStringTokens)
        {
        if (accessStringToken.EndsWith("]"))
            accessStringToken.erase(accessStringToken.find_last_of('['));
        }

    return accessStringTokens;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
CustomAttributeValidator::ClassRule& CustomAttributeValidator::Append(Policy policy, Utf8CP schemaFullName, Utf8CP customAttributeClassName, ChangeType changeType)
    {
    Utf8String schemaName(schemaFullName);
    Utf8String className(customAttributeClassName);
    BeAssert(!schemaName.empty());
    BeAssert(!className.empty());
    if (schemaName == WILDCARD)
        {
        BeAssert(className == WILDCARD);
        }
    auto newRule = ClassRule::Create(policy, schemaFullName, customAttributeClassName, changeType);
    m_rules.push_back(newRule);
    return *newRule;
    }

//======================================================================================>
//CustomAttributeValidator::ClassRule
//======================================================================================>

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool CustomAttributeValidator::ClassRule::AppliesToPropertyChange(ChangeType propertyChangeType, bvector<Utf8String> const& accessStringTokens) const
    {
    if (GetRules().size() == 0)
        return true;

    for (auto const& propertyRule : GetRules())
        {
        if (propertyRule->AppliesToChangeType(propertyChangeType) && propertyRule->Matches(accessStringTokens))
            return true;
        }

    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
CustomAttributeValidator::ClassRule& CustomAttributeValidator::ClassRule::Append(CustomAttributeValidator::Policy policy, Utf8CP accessString, CustomAttributeValidator::ChangeType changeType)
    {
    m_rules.push_back(PropertyRule::Create(policy, accessString, changeType));
    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
RefCountedPtr<CustomAttributeValidator::ClassRule> CustomAttributeValidator::ClassRule::Create(CustomAttributeValidator::Policy policy, Utf8CP schemaFullName, Utf8CP customAttributeClassName, CustomAttributeValidator::ChangeType changeType)
    {
    return new ClassRule(policy, schemaFullName, customAttributeClassName, changeType);
    }

//======================================================================================>
//CustomAttributeValidator::PropertyRule
//======================================================================================>

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool CustomAttributeValidator::PropertyRule::Matches(bvector<Utf8String> const& accessStringTokens) const
    {
    auto const& ruleTokens = GetPath();
    for (size_t i = 0; i < ruleTokens.size(); i++)
        {
        if (accessStringTokens.size() == i)
            return false;

        if (ruleTokens[i] == CustomAttributeValidator::WILDCARD)
            return true;

        if (!accessStringTokens[i].EqualsIAscii(ruleTokens[i]))
            return false;
        }

    return accessStringTokens.size() == ruleTokens.size();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
RefCountedPtr<CustomAttributeValidator::PropertyRule> CustomAttributeValidator::PropertyRule::Create(CustomAttributeValidator::Policy policy, Utf8CP accessString, CustomAttributeValidator::ChangeType changeType)
    {
    return new PropertyRule(policy, accessString, changeType);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
CustomAttributeValidator::PropertyRule::PropertyRule(CustomAttributeValidator::Policy policy, Utf8CP accessString, CustomAttributeValidator::ChangeType changeType)
    : m_accessString(accessString), m_changeType(changeType), m_policy(policy)
    {
    BeStringUtilities::Split(accessString, ".", m_path);
    }

END_BENTLEY_ECOBJECT_NAMESPACE
