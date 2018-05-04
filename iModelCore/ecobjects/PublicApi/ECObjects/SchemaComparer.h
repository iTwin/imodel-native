/*--------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/SchemaComparer.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+-------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/RefCounted.h>
#include <Bentley/Nullable.h>
#include <Bentley/DateTime.h>
#include <Bentley/ByteStream.h>
#include <ECObjects/ECSchema.h>
#include <ECObjects/ECObjects.h>

//__BENTLEY_INTERNAL_ONLY__
BEGIN_BENTLEY_ECOBJECT_NAMESPACE

//=======================================================================================
// @bsienum                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
enum class ChangeType
    {
    Deleted = 1, //This need to be none zero base
    Modified = 2,
    New = 3
    };

//=======================================================================================
// @bsienum                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
enum class SystemId
    {
    None,
    Alias,
    ArrayProperty,
    BaseClass,
    BaseClasses,
    Class,
    Classes,
    ClassModifier,
    ClassType,
    CompositeIncludeZero,
    CompositeSpacer,
    CompositeMajorUnit,
    CompositeMajorLabel,
    CompositeMiddleUnit,
    CompositeMiddleLabel,
    CompositeMinorUnit,
    CompositeMinorLabel,
    CompositeSubUnit,
    CompositeSubLabel,
    CompositeValueSpec,
    Constraint,
    ConstraintClass,
    ConstraintClasses,
    CustomAttribute,
    CustomAttributes,
    DecimalPrecision,
    DecimalSeparator,
    Description,
    Direction,
    DisplayLabel,
    ECVersion,
    Enumeration,
    Enumerations,
    Enumerator,
    Enumerators,
    ExtendedTypeName,
    Format,
    Formats,
    FormatTraits,
    FractionalPrecision,
    Integer,
    IsPolymorphic,
    IsReadonly,
    IsStrict,
    IsStruct,
    IsStructArray,
    IsPrimitive,
    IsPrimitiveArray,
    IsNavigation,
    KindOfQuantities,
    KindOfQuantity,
    KoqRelativeError,
    KoqPersistenceUnit,
    KoqPresentationFormat,
    KoqPresentationFormats,
    MaximumLength,
    MaximumValue,
    MaxOccurs,
    MinimumLength,
    MinimumValue,
    MinOccurs,
    MinWidth,
    Multiplicity,
    Name,
    NavigationProperty,
    NumericFormatSpec,
    OriginalECXmlVersionMajor,
    OriginalECXmlVersionMinor,
    Phenomena,
    Phenomenon,
    PhenomenonDefinition,
    PresentationType,
    Properties,
    Property,
    PropertyCategories,
    PropertyCategory,
    PropertyCategoryPriority,
    PropertyPriority,
    PropertyType,
    PropertyValue,
    PropertyValues,
    Relationship,
    RoleLabel,
    RoundingFactor,
    Schema,
    SchemaReference,
    SchemaReferences,
    Schemas,
    ScientificType,
    ShowSignOption,
    Source,
    StationSeparator,
    StationOffsetSize,
    StrengthDirection,
    StrengthType,
    String,
    Target,
    ThousandSeparator,
    TypeName,
    Unit,
    UnitDefinition,
    UnitNumerator,
    UnitDenominator,
    UnitInvertingUnit,
    UnitIsConstant,
    UnitOffset,
    Units,
    UnitSystem,
    UnitSystems,
    UomSeparator,
    VersionRead,
    VersionMinor,
    VersionWrite
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct ECChange : RefCountedBase
    {
    enum class Status
        {
        Pending,
        Done,
        };

    private:
        SystemId m_systemId;
        Utf8String m_customId;
        ChangeType m_changeType;
        ECChange const* m_parent = nullptr;
        Status m_status = Status::Pending;

        virtual bool _IsChanged() const = 0;
        virtual void _WriteToString(Utf8StringR str, int currentIndex, int indentSize) const = 0;
        virtual void _Optimize() {}

    protected:
        ECChange(ChangeType changeType, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr) : m_systemId(systemId), m_customId(customId), m_changeType(changeType), m_parent(parent) {}

        ECOBJECTS_EXPORT static void AppendBegin(Utf8StringR str, ECChange const& change, int currentIndex);
        static void AppendEnd(Utf8StringR str) { str.append("\r\n"); }
        
        ECOBJECTS_EXPORT static Utf8CP SystemIdToString(SystemId);

    public:
        virtual ~ECChange() {};
        SystemId GetSystemId() const { return m_systemId; }
        bool HasCustomId() const { return !m_customId.empty(); }
        ECOBJECTS_EXPORT Utf8CP GetId() const;
        ChangeType GetChangeType() const { return m_changeType; }
        ECChange const* GetParent() const { return m_parent; }
        bool IsChanged() const { return _IsChanged(); }
        void Optimize() { _Optimize(); }
        Status GetStatus() { return m_status; }
        void SetStatus(Status status) { m_status = status; }
        void WriteToString(Utf8StringR str, int initIndex = 0, int indentSize = 4) const { _WriteToString(str, initIndex, indentSize); }
        Utf8String ToString() const { Utf8String str;  WriteToString(str); return str; }
    };

typedef RefCountedPtr<ECChange> ECChangePtr;
//=======================================================================================
// For case-sensitive UTF-8 string comparisons in STL collections.
// @bsistruct
//+===============+===============+===============+===============+===============+======
struct CompareUtf8
    {
    bool operator()(Utf8CP s1, Utf8CP s2) const { return strcmp(s1, s2) < 0; }
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct CompositeECChange : public ECChange
    {
    private:
        bmap<Utf8CP, ECChangePtr, CompareUtf8> m_changes;

        ECOBJECTS_EXPORT bool _IsChanged() const override;
        ECOBJECTS_EXPORT void _WriteToString(Utf8StringR str, int currentIndex, int indentSize) const override;
        ECOBJECTS_EXPORT void _Optimize() override;

    protected:
        CompositeECChange(ChangeType state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr) : ECChange(state, systemId, parent, customId) {}

        template<typename T>
        T& Get(SystemId memberSystemId)
            {
            static_assert(std::is_base_of<ECChange, T>::value, "T not derived from ECChange");
            Utf8CP memberId = SystemIdToString(memberSystemId);
            auto itor = m_changes.find(memberId);
            if (itor != m_changes.end())
                return *(static_cast<T*>(itor->second.get()));

            ECChangePtr changePtr = new T(GetChangeType(), memberSystemId, this, nullptr);
            ECChange* changeP = changePtr.get();
            m_changes[changePtr->GetId()] = changePtr;
            return *(static_cast<T*>(changeP));
            }


    public:
        virtual ~CompositeECChange() {}

        size_t MemberChangesCount() const 
            { 
            size_t count = 0;
            for (auto const& kvPair : m_changes)
                {
                if (kvPair.second->IsChanged())
                    count++;
                }

            return count; 
            }
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
template<typename TArrayElement>
struct ECChangeArray final : public ECChange
    {
    private:
        bvector<ECChangePtr> m_changes;

        bool _IsChanged() const override
            {
            for (ECChangePtr const& change : m_changes)
                {
                if (change->IsChanged())
                    return true;
                }

            return false;
            }

        void _WriteToString(Utf8StringR str, int currentIndex, int indentSize) const override
            {
            AppendBegin(str, *this, currentIndex);
            AppendEnd(str);
            for (ECChangePtr const& change : m_changes)
                {
                change->WriteToString(str, currentIndex + indentSize, indentSize);
                }
            }

        void _Optimize() override
            {
            auto it = m_changes.begin();
            while (it != m_changes.end())
                {
                (*it)->Optimize();
                if (!(*it)->IsChanged())
                    it = m_changes.erase(it);
                else
                    ++it;
                }
            }

    public:
        ECChangeArray(ChangeType state, SystemId systemId, ECChange const* parent, Utf8CP customId = nullptr) : ECChange(state, systemId, parent, customId)
            {
            static_assert(std::is_base_of<ECChange, TArrayElement>::value, "TArrayElement not derived from ECChange");
            }

        virtual ~ECChangeArray() {}

        size_t Count() const { return m_changes.size(); }
        bool IsEmpty() const { return m_changes.empty(); }
        TArrayElement& operator[](size_t index) { return static_cast<TArrayElement&>(*m_changes[index]); }
        TArrayElement& Add(ChangeType state, SystemId elementSystemId, Utf8CP customId = nullptr)
            {
            ECChangePtr changePtr = new TArrayElement(state, elementSystemId, this, customId);
            TArrayElement* changeP = static_cast<TArrayElement*>(changePtr.get());
            m_changes.push_back(changePtr);
            return *changeP;
            }
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
template<typename T>
struct PrimitiveChange final : public ECChange
    {
    private:
        Nullable<T> m_old;
        Nullable<T> m_new;

        bool _IsChanged() const override { return (m_old != nullptr || m_new != nullptr) && m_old != m_new; }

        void _WriteToString(Utf8StringR str, int currentIndex, int indentSize) const override
            {
            AppendBegin(str, *this, currentIndex);
            str.append(": ").append(ToString());
            AppendEnd(str);
            }

        static Utf8String Stringify(Utf8StringCR val) { return val; }
        static Utf8String Stringify(bool val) { return val ? "true" : "false"; }
        static Utf8String Stringify(uint32_t val) { return Utf8PrintfString("%" PRIu32, val); }
        static Utf8String Stringify(int32_t val) { return Utf8PrintfString("%" PRIi32, val); }
        static Utf8String Stringify(int64_t val) { return Utf8PrintfString("%" PRIi64, val); }
        static Utf8String Stringify(double val) { return Utf8PrintfString("%.17g", val); }
        static Utf8String Stringify(DateTime const& val) { return val.ToString(); }
        static Utf8String Stringify(bvector<Byte> const& val)
            {
            Utf8String str;
            Base64Utilities::Encode(str, val.data(), val.size());
            return str;
            }
        static Utf8String Stringify(DPoint2d const& val) { return Utf8PrintfString("(%.17g, %.17g)", val.x, val.y); }
        static Utf8String Stringify(DPoint3d const& val) { return Utf8PrintfString("(%.17g, %.17g,  %.17g)", val.x, val.y, val.z); }
        static Utf8String Stringify(ECN::ECValue const& val) { return val.ToString(); }
        static Utf8String Stringify(ECN::StrengthType val)
            {
            switch (val)
                {
                    case ECN::StrengthType::Embedding:
                        return "Embedding";
                    case ECN::StrengthType::Holding:
                        return "Holding";
                    case ECN::StrengthType::Referencing:
                        return "Referencing";
                    default:
                        BeAssert(false && "Unhandled enum value");
                        return Utf8String();
                }
            }

        static Utf8String Stringify(ECN::ECRelatedInstanceDirection val)
            {
            switch (val)
                {
                    case ECN::ECRelatedInstanceDirection::Forward:
                        return "Forward";
                    case ECN::ECRelatedInstanceDirection::Backward:
                        return "Backward";
                    default:
                        BeAssert(false && "Unhandled enum value");
                        return Utf8String();
                }
            }

        static Utf8String Stringify(ECN::ECClassModifier val)
            {
            switch (val)
                {
                    case ECN::ECClassModifier::Abstract:
                        return "Abstract";
                    case ECN::ECClassModifier::None:
                        return "None";
                    case ECN::ECClassModifier::Sealed:
                        return "Sealed";
                    default:
                        BeAssert(false && "Unhandled enum value");
                        return Utf8String();
                }
            }
        static Utf8String Stringify(ECN::ECClassType val)
            {
            switch (val)
                {
                    case ECN::ECClassType::CustomAttribute:
                        return "CustomAttribute";
                    case ECN::ECClassType::Entity:
                        return "Entity";
                    case ECN::ECClassType::Relationship:
                        return "Relationship";
                    case ECN::ECClassType::Struct:
                        return "Struct";
                    default:
                        BeAssert(false && "Unhandled enum value");
                        return Utf8String();
                }
            }

    public:
        PrimitiveChange(ChangeType state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr) : ECChange(state, systemId, parent, customId) {}
        ~PrimitiveChange() {}
        Nullable<T> const& GetNew() const { return m_new; }
        Nullable<T> const& GetOld() const { return m_old; }

        BentleyStatus Set(Nullable<T> const& value)
            {
            if (GetChangeType() == ChangeType::Modified)
                {
                BeAssert(false && "Cannot use this method for Changes marked as Modified");
                return ERROR;
                }

            if (GetChangeType() == ChangeType::Deleted)
                m_old = value;
            else
                {
                BeAssert(GetChangeType() == ChangeType::New);
                m_new = value;
                }

            return SUCCESS;
            }

        BentleyStatus Set(Nullable<T> const& oldValue, Nullable<T> const& newValue)
            {
            if (GetChangeType() != ChangeType::Modified)
                {
                BeAssert(false && "Cannot set old and new value for Change which is not marked as Modified");
                return ERROR;
                }

            if (oldValue != nullptr)
                m_old = oldValue;

            if (newValue != nullptr)
                m_new = newValue;

            return SUCCESS;
            }

        Utf8String ToString() const
            {
            Utf8String str;
            if (GetChangeType() == ChangeType::Deleted)
                str.append(Stringify(GetOld().Value()));
            if (GetChangeType() == ChangeType::New)
                str.append(Stringify(GetNew().Value()));
            if (GetChangeType() == ChangeType::Modified)
                str.append(Stringify(GetOld().Value())).append(" -> ").append(Stringify(GetNew().Value()));
            return str;
            }

    };

typedef PrimitiveChange<bool> BooleanChange;
typedef PrimitiveChange<bvector<Byte>> BinaryChange;
typedef PrimitiveChange<DateTime> DateTimeChange;
typedef PrimitiveChange<double> DoubleChange;
typedef PrimitiveChange<int32_t> Int32Change;
typedef PrimitiveChange<uint32_t> UInt32Change;
typedef PrimitiveChange<int64_t> Int64Change;
typedef PrimitiveChange<DPoint2d> Point2dChange;
typedef PrimitiveChange<DPoint3d> Point3dChange;
typedef PrimitiveChange<Utf8String> StringChange;

//=======================================================================================
// @bsiclass                                             Krischan.Eberle          05/2018
//+===============+===============+===============+===============+===============+======
struct SchemaElementChange : public CompositeECChange
    {
protected:
    SchemaElementChange(ChangeType type, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr) : CompositeECChange(type, systemId, parent, customId) {}

public:
    virtual ~SchemaElementChange() {}

    StringChange& Name() { return Get<StringChange>(SystemId::Name); }
    StringChange& DisplayLabel() { return Get<StringChange>(SystemId::DisplayLabel); }
    StringChange& Description() { return Get<StringChange>(SystemId::Description); }
    };

typedef ECChangeArray<StringChange> SchemaReferenceChanges;
typedef ECChangeArray<StringChange> BaseClassChanges;
typedef ECChangeArray<StringChange> RelationshipConstraintClassChanges;

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct PropertyValueChange final : public ECChange
    {
    private:
        std::unique_ptr<ECChange> m_value;
        ECN::PrimitiveType m_type;

        bool _IsChanged() const override { return m_value != nullptr && m_value->IsChanged(); }
        void _WriteToString(Utf8StringR str, int currentIndex, int indentSize) const override;
        void _Optimize() override
            {
            if (m_value != nullptr && !m_value->IsChanged())
                m_value = nullptr;
            }

        BentleyStatus Inititalize(ECN::PrimitiveType);

    public:
        PropertyValueChange(ChangeType state, SystemId systemId = SystemId::PropertyValue, ECChange const* parent = nullptr, Utf8CP accessString = nullptr)
            : ECChange(state, SystemId::PropertyValue, parent, accessString)
            {
            BeAssert(!Utf8String::IsNullOrEmpty(accessString) && "access string must not be empty for PropertyValueChange");
            BeAssert(systemId == GetSystemId());
            }

        ~PropertyValueChange() {}
        Utf8CP GetAccessString() const { BeAssert(HasCustomId()); return GetId(); }
        bool HasValue() const { return m_value != nullptr; }
        ECN::PrimitiveType GetValueType() const { return m_type; }
        StringChange* GetString() const { BeAssert(m_type == ECN::PRIMITIVETYPE_String); if (m_type != ECN::PRIMITIVETYPE_String) return nullptr; return static_cast<StringChange*>(m_value.get()); }
        BooleanChange* GetBoolean() const { BeAssert(m_type == ECN::PRIMITIVETYPE_Boolean); if (m_type != ECN::PRIMITIVETYPE_Boolean) return nullptr; return static_cast<BooleanChange*>(m_value.get()); }
        DateTimeChange* GetDateTime() const { BeAssert(m_type == ECN::PRIMITIVETYPE_DateTime); if (m_type != ECN::PRIMITIVETYPE_DateTime) return nullptr; return static_cast<DateTimeChange*>(m_value.get()); }
        DoubleChange* GetDouble() const { BeAssert(m_type == ECN::PRIMITIVETYPE_Double); if (m_type != ECN::PRIMITIVETYPE_Double) return nullptr; return static_cast<DoubleChange*>(m_value.get()); }
        Int32Change* GetInteger() const { BeAssert(m_type == ECN::PRIMITIVETYPE_Integer); if (m_type != ECN::PRIMITIVETYPE_Integer) return nullptr; return static_cast<Int32Change*>(m_value.get()); }
        Int64Change* GetLong() const { BeAssert(m_type == ECN::PRIMITIVETYPE_Long); if (m_type != ECN::PRIMITIVETYPE_Long) return nullptr; return static_cast<Int64Change*>(m_value.get()); }
        Point2dChange* GetPoint2d() const { BeAssert(m_type == ECN::PRIMITIVETYPE_Point2d); if (m_type != ECN::PRIMITIVETYPE_Point2d) return nullptr; return static_cast<Point2dChange*>(m_value.get()); }
        Point3dChange* GetPoint3d() const { BeAssert(m_type == ECN::PRIMITIVETYPE_Point3d); if (m_type != ECN::PRIMITIVETYPE_Point3d) return nullptr; return static_cast<Point3dChange*>(m_value.get()); }
        BinaryChange* GetBinary() const { BeAssert(m_type == ECN::PRIMITIVETYPE_Binary); if (m_type != ECN::PRIMITIVETYPE_Binary) return nullptr; return static_cast<BinaryChange*>(m_value.get()); }

        BentleyStatus Set(ECN::ECValueCR oldValue, ECN::ECValueCR newValue);
        BentleyStatus Set(ECN::ECValueCR);
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct CustomAttributeChange final : public ECChange
    {
    private:
        std::unique_ptr<ECChangeArray<PropertyValueChange>> m_propValueChanges;

        bool _IsChanged() const override { return m_propValueChanges->IsChanged(); }
        void _Optimize() override { m_propValueChanges->Optimize(); }
        void _WriteToString(Utf8StringR str, int currentIndex, int indentSize) const override
            {
            AppendBegin(str, *this, currentIndex);
            AppendEnd(str);
            m_propValueChanges->WriteToString(str, currentIndex + indentSize, indentSize);
            }

    public:
        CustomAttributeChange(ChangeType type, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr) : ECChange(type, SystemId::CustomAttribute, parent, customId) 
            { 
            BeAssert(systemId == GetSystemId()); 
            m_propValueChanges = std::make_unique<ECChangeArray<PropertyValueChange>>(type, GetSystemId(), this, GetId());
            }
        ~CustomAttributeChange() {}

        ECChangeArray<PropertyValueChange>& PropValues() { return *m_propValueChanges; }
    };

typedef ECChangeArray<CustomAttributeChange> CustomAttributeChanges;

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct RelationshipConstraintChange final : public CompositeECChange
    {
    public:
        RelationshipConstraintChange(ChangeType state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr) : CompositeECChange(state, systemId, parent, customId) { BeAssert(systemId == SystemId::Source || systemId == SystemId::Target); }
        ~RelationshipConstraintChange() {}
        StringChange& RoleLabel() { return Get<StringChange>(SystemId::RoleLabel); }
        StringChange& Multiplicity() { return Get<StringChange>(SystemId::Multiplicity); }
        BooleanChange& IsPolymorphic() { return Get<BooleanChange>(SystemId::IsPolymorphic); }
        RelationshipConstraintClassChanges& ConstraintClasses() { return Get<RelationshipConstraintClassChanges>(SystemId::ConstraintClasses); }
        CustomAttributeChanges& CustomAttributes() { return Get<CustomAttributeChanges>(SystemId::CustomAttributes); }
    };




//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct NavigationPropertyChange final : public CompositeECChange
    {
    public:
        NavigationPropertyChange(ChangeType state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr) : CompositeECChange(state, SystemId::NavigationProperty, parent, customId) { BeAssert(systemId == GetSystemId()); }
        ~NavigationPropertyChange() {}
        PrimitiveChange<ECN::ECRelatedInstanceDirection>& Direction() { return Get<PrimitiveChange<ECN::ECRelatedInstanceDirection>>(SystemId::Direction); }
        StringChange& Relationship() { return Get<StringChange>(SystemId::Relationship); }
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct ArrayPropertyChange final : public CompositeECChange
    {
    public:
        ArrayPropertyChange(ChangeType state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr) : CompositeECChange(state, SystemId::ArrayProperty, parent, customId) { BeAssert(systemId == GetSystemId()); }
        ~ArrayPropertyChange() {}
        UInt32Change& MinOccurs() { return Get<UInt32Change>(SystemId::MinOccurs); }
        UInt32Change& MaxOccurs() { return Get<UInt32Change>(SystemId::MaxOccurs); }
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct PropertyChange final : public SchemaElementChange
    {
    public:
        PropertyChange(ChangeType state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr) : SchemaElementChange(state, SystemId::Property, parent, customId) { BeAssert(systemId == GetSystemId()); }
        ~PropertyChange() {}
        StringChange& TypeName() { return Get<StringChange>(SystemId::TypeName); }
        PrimitiveChange<ECN::ECValue>& MinimumValue() { return Get<PrimitiveChange<ECN::ECValue>>(SystemId::MinimumValue); }
        PrimitiveChange<ECN::ECValue>& MaximumValue() { return Get<PrimitiveChange<ECN::ECValue>>(SystemId::MaximumValue); }
        UInt32Change& MinimumLength() { return Get<UInt32Change>(SystemId::MinimumLength); }
        UInt32Change& MaximumLength() { return Get<UInt32Change>(SystemId::MaximumLength); }
        BooleanChange& IsStruct() { return Get<BooleanChange>(SystemId::IsStrict); }
        BooleanChange& IsStructArray() { return Get<BooleanChange>(SystemId::IsStructArray); }
        BooleanChange& IsPrimitive() { return Get<BooleanChange>(SystemId::IsPrimitive); }
        BooleanChange& IsPrimitiveArray() { return Get<BooleanChange>(SystemId::IsPrimitiveArray); }
        BooleanChange& IsNavigation() { return Get<BooleanChange>(SystemId::IsNavigation); }
        ArrayPropertyChange& Array() { return Get<ArrayPropertyChange>(SystemId::ArrayProperty); }
        NavigationPropertyChange& Navigation() { return Get<NavigationPropertyChange>(SystemId::NavigationProperty); }
        StringChange& ExtendedTypeName() { return Get<StringChange>(SystemId::ExtendedTypeName); }
        BooleanChange& IsReadonly() { return Get<BooleanChange>(SystemId::IsReadonly); }
        Int32Change& Priority() { return Get<Int32Change>(SystemId::PropertyPriority); }
        StringChange& KindOfQuantity() { return Get<StringChange>(SystemId::KindOfQuantity); }
        StringChange& Enumeration() { return Get<StringChange>(SystemId::Enumeration); }
        StringChange& Category() { return Get<StringChange>(SystemId::PropertyCategory); }
        CustomAttributeChanges& CustomAttributes() { return Get<CustomAttributeChanges>(SystemId::CustomAttributes); }
    };

typedef ECChangeArray<PropertyChange> PropertyChanges;

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct ClassChange final : public SchemaElementChange
    {
    public:
        ClassChange(ChangeType state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr) : SchemaElementChange(state, systemId, parent, customId) {}
        ~ClassChange() {}

        PrimitiveChange<ECN::ECClassModifier>& ClassModifier() { return Get<PrimitiveChange<ECN::ECClassModifier>>(SystemId::ClassModifier); }
        PrimitiveChange<ECN::ECClassType>& ClassType() { return Get<PrimitiveChange<ECN::ECClassType>>(SystemId::ClassType); }
        BaseClassChanges& BaseClasses() { return Get<BaseClassChanges>(SystemId::BaseClasses); }
        PropertyChanges& Properties() { return Get<PropertyChanges>(SystemId::Properties); }
        CustomAttributeChanges& CustomAttributes() { return Get<CustomAttributeChanges>(SystemId::CustomAttributes); }

        PrimitiveChange<ECN::StrengthType>& Strength() { return Get<PrimitiveChange<ECN::StrengthType>>(SystemId::StrengthType); }
        PrimitiveChange<ECN::ECRelatedInstanceDirection>& StrengthDirection() { return Get<PrimitiveChange<ECN::ECRelatedInstanceDirection>>(SystemId::StrengthDirection); }
        RelationshipConstraintChange& Source() { return Get<RelationshipConstraintChange>(SystemId::Source); }
        RelationshipConstraintChange& Target() { return Get<RelationshipConstraintChange>(SystemId::Target); }
    };

typedef ECChangeArray<ClassChange> ClassChanges;

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct EnumeratorChange final : public SchemaElementChange
    {
    public:
        EnumeratorChange(ChangeType state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr) : SchemaElementChange(state, SystemId::Enumerator, parent, customId) { BeAssert(systemId == GetSystemId()); }
        ~EnumeratorChange() {}
        StringChange& String() { return Get<StringChange>(SystemId::String); }
        Int32Change& Integer() { return Get<Int32Change>(SystemId::Integer); }
    };

typedef ECChangeArray<EnumeratorChange> EnumeratorChanges;

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct EnumerationChange final : public SchemaElementChange
    {
    public:
        EnumerationChange(ChangeType state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr) : SchemaElementChange(state, SystemId::Enumeration, parent, customId) { BeAssert(systemId == GetSystemId()); }
        ~EnumerationChange() {}
        StringChange& TypeName() { return Get<StringChange>(SystemId::TypeName); }
        BooleanChange& IsStrict() { return Get<BooleanChange>(SystemId::IsStrict); }
        EnumeratorChanges& Enumerators() { return Get<EnumeratorChanges>(SystemId::Enumerators); }
    };

typedef ECChangeArray<EnumerationChange> EnumerationChanges;

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct KindOfQuantityChange final : public SchemaElementChange
    {
    public:
        KindOfQuantityChange(ChangeType state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr) : SchemaElementChange(state, SystemId::KindOfQuantity, parent, customId) { BeAssert(systemId == GetSystemId()); }
        ~KindOfQuantityChange() {}
        StringChange& PersistenceUnit() { return Get<StringChange>(SystemId::KoqPersistenceUnit); }
        DoubleChange& RelativeError() { return Get<DoubleChange>(SystemId::KoqRelativeError); }
        ECChangeArray<StringChange>& PresentationFormats() { return Get<ECChangeArray<StringChange>>(SystemId::KoqPresentationFormats); }
    };

typedef ECChangeArray<KindOfQuantityChange> KindOfQuantityChanges;

//=======================================================================================
// @bsiclass                                                Krischan.Eberle       06/2017
//+===============+===============+===============+===============+===============+======
struct PropertyCategoryChange final : public SchemaElementChange
    {
    public:
        PropertyCategoryChange(ChangeType state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr) : SchemaElementChange(state, SystemId::PropertyCategory, parent, customId) { BeAssert(systemId == GetSystemId()); }
        ~PropertyCategoryChange() {}
        UInt32Change& Priority() { return Get<UInt32Change>(SystemId::PropertyCategoryPriority); }
    };


typedef ECChangeArray<PropertyCategoryChange> PropertyCategoryChanges;


//=======================================================================================
// @bsiclass                                                Krischan.Eberle       02/2018
//+===============+===============+===============+===============+===============+======
struct PhenomenonChange final : public SchemaElementChange
    {
    public:
        PhenomenonChange(ChangeType state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr) : SchemaElementChange(state, SystemId::Phenomenon, parent, customId) { BeAssert(systemId == GetSystemId()); }
        ~PhenomenonChange() {}
        StringChange& Definition() { return Get<StringChange>(SystemId::PhenomenonDefinition); }
    };

typedef ECChangeArray<PhenomenonChange> PhenomenonChanges;

//=======================================================================================
// @bsiclass                                                Krischan.Eberle       02/2018
//+===============+===============+===============+===============+===============+======
struct UnitSystemChange final : public SchemaElementChange
    {
    public:
        UnitSystemChange(ChangeType state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr) : SchemaElementChange(state, SystemId::UnitSystem, parent, customId) { BeAssert(systemId == GetSystemId()); }
        ~UnitSystemChange() {}
    };

typedef ECChangeArray<UnitSystemChange> UnitSystemChanges;

//=======================================================================================
// @bsiclass                                                Krischan.Eberle       02/2018
//+===============+===============+===============+===============+===============+======
struct UnitChange final : public SchemaElementChange
    {
    public:
        UnitChange(ChangeType state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr) : SchemaElementChange(state, SystemId::Unit, parent, customId) { BeAssert(systemId == GetSystemId()); }
        ~UnitChange() {}
        StringChange& Definition() { return Get<StringChange>(SystemId::UnitDefinition); }
        StringChange& Phenomenon() { return Get<StringChange>(SystemId::Phenomenon); }
        StringChange& UnitSystem() { return Get<StringChange>(SystemId::UnitSystem); }
        DoubleChange& Numerator() { return Get<DoubleChange>(SystemId::UnitNumerator); }
        DoubleChange& Denominator() { return Get<DoubleChange>(SystemId::UnitDenominator); }
        DoubleChange& Offset() { return Get<DoubleChange>(SystemId::UnitOffset); }
        BooleanChange& IsConstant() { return Get<BooleanChange>(SystemId::UnitIsConstant); }
        StringChange& InvertingUnit() { return Get<StringChange>(SystemId::UnitInvertingUnit); }
    };

typedef ECChangeArray<UnitChange> UnitChanges;

//=======================================================================================
// @bsiclass                                                      05/2018
//+===============+===============+===============+===============+===============+======
struct NumericFormatSpecChange final : public CompositeECChange
    {
    public:
        NumericFormatSpecChange(ChangeType state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr) : CompositeECChange(state, SystemId::NumericFormatSpec, parent, customId) { BeAssert(systemId == GetSystemId()); }
        ~NumericFormatSpecChange() {}

        DoubleChange& RoundingFactor() { return Get<DoubleChange>(SystemId::RoundingFactor); }
        StringChange& PresentationType() { return Get<StringChange>(SystemId::PresentationType); }
        Int32Change& DecimalPrecision() { return Get<Int32Change>(SystemId::DecimalPrecision); }
        Int32Change& FractionalPrecision() { return Get<Int32Change>(SystemId::FractionalPrecision); }
        UInt32Change& MinWidth() { return Get<UInt32Change>(SystemId::MinWidth); }
        StringChange& ScientificType() { return Get<StringChange>(SystemId::ScientificType); }
        StringChange& ShowSignOption() { return Get<StringChange>(SystemId::ShowSignOption); }
        StringChange& FormatTraits() { return Get<StringChange>(SystemId::FormatTraits); }
        StringChange& DecimalSeparator() { return Get<StringChange>(SystemId::DecimalSeparator); }
        StringChange& ThousandsSeparator() { return Get<StringChange>(SystemId::ThousandSeparator); }
        StringChange& UomSeparator() { return Get<StringChange>(SystemId::UomSeparator); }
        StringChange& StationSeparator() { return Get<StringChange>(SystemId::StationSeparator); }
        UInt32Change& StationOffsetSize() { return Get<UInt32Change>(SystemId::StationOffsetSize); }

        BentleyStatus SetFrom(Formatting::NumericFormatSpecCP oldSpec, Formatting::NumericFormatSpecCP newSpec);
        BentleyStatus SetFrom(Formatting::NumericFormatSpecCR);
    };

//=======================================================================================
// @bsiclass                                                      05/2018
//+===============+===============+===============+===============+===============+======
struct CompositeValueSpecChange final : public CompositeECChange
    {
    public:
        CompositeValueSpecChange(ChangeType state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr) : CompositeECChange(state, SystemId::CompositeValueSpec, parent, customId) { BeAssert(systemId == GetSystemId()); }
        ~CompositeValueSpecChange() {}

        BooleanChange& IncludeZero() { return Get<BooleanChange>(SystemId::CompositeIncludeZero); }
        StringChange& Spacer() { return Get<StringChange>(SystemId::CompositeSpacer); }
        StringChange& MajorUnit() { return Get<StringChange>(SystemId::CompositeMajorUnit); }
        StringChange& MajorLabel() { return Get<StringChange>(SystemId::CompositeMajorLabel); }
        StringChange& MiddleUnit() { return Get<StringChange>(SystemId::CompositeMiddleUnit); }
        StringChange& MiddleLabel() { return Get<StringChange>(SystemId::CompositeMiddleLabel); }
        StringChange& MinorUnit() { return Get<StringChange>(SystemId::CompositeMinorUnit); }
        StringChange& MinorLabel() { return Get<StringChange>(SystemId::CompositeMinorLabel); }
        StringChange& SubUnit() { return Get<StringChange>(SystemId::CompositeSubUnit); }
        StringChange& SubLabel() { return Get<StringChange>(SystemId::CompositeSubLabel); }

        BentleyStatus SetFrom(Formatting::CompositeValueSpecCP oldSpec, Formatting::CompositeValueSpecCP newSpec);
        BentleyStatus SetFrom(Formatting::CompositeValueSpecCR);
    };

//=======================================================================================
// @bsiclass                                               Kyle.Abramowitz        04/2018
//+===============+===============+===============+===============+===============+======
struct FormatChange final : public SchemaElementChange
    {
    public:
        FormatChange(ChangeType state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr) : SchemaElementChange(state, SystemId::Format, parent, customId) { BeAssert(systemId == GetSystemId()); }
        ~FormatChange() {}
        NumericFormatSpecChange& NumericSpec() { return Get<NumericFormatSpecChange>(SystemId::NumericFormatSpec); }
        CompositeValueSpecChange& CompositeSpec() { return Get<CompositeValueSpecChange>(SystemId::CompositeValueSpec); }
    };

typedef ECChangeArray<FormatChange> FormatChanges;


//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct SchemaChange final : public SchemaElementChange
    {
    public:
        SchemaChange(ChangeType state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr) : SchemaElementChange(state, SystemId::Schema, parent, customId) { BeAssert(systemId == GetSystemId()); }
        ~SchemaChange() {}

        StringChange& Alias() { return Get<StringChange>(SystemId::Alias); }
        UInt32Change& VersionRead() { return Get<UInt32Change>(SystemId::VersionRead); }
        UInt32Change& VersionMinor() { return Get<UInt32Change>(SystemId::VersionMinor); }
        UInt32Change& VersionWrite() { return Get<UInt32Change>(SystemId::VersionWrite); }
        UInt32Change& ECVersion() { return Get<UInt32Change>(SystemId::ECVersion); }
        UInt32Change& OriginalECXmlVersionMajor() { return Get<UInt32Change>(SystemId::OriginalECXmlVersionMajor); }
        UInt32Change& OriginalECXmlVersionMinor() { return Get<UInt32Change>(SystemId::OriginalECXmlVersionMinor); }

        CustomAttributeChanges& CustomAttributes() { return Get<CustomAttributeChanges>(SystemId::CustomAttributes); }
        SchemaReferenceChanges& References() { return Get<SchemaReferenceChanges>(SystemId::SchemaReferences); }
        ClassChanges& Classes() { return Get<ClassChanges>(SystemId::Classes); }
        EnumerationChanges& Enumerations() { return Get<EnumerationChanges>(SystemId::Enumerations); }
        KindOfQuantityChanges& KindOfQuantities() { return Get<KindOfQuantityChanges>(SystemId::KindOfQuantities); }
        PropertyCategoryChanges& PropertyCategories() { return Get<PropertyCategoryChanges>(SystemId::PropertyCategories); }
        PhenomenonChanges& Phenomena() { return Get<PhenomenonChanges>(SystemId::Phenomena); }
        UnitSystemChanges& UnitSystems() { return Get<UnitSystemChanges>(SystemId::UnitSystems); }
        UnitChanges& Units() { return Get<UnitChanges>(SystemId::Units); }
        FormatChanges& Formats() { return Get<FormatChanges>(SystemId::Formats); }
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle       05/2018
//+===============+===============+===============+===============+===============+======
struct SchemaDiff final
    {
private:
    ECChangeArray<SchemaChange> m_changes;

public:
    SchemaDiff() : m_changes(ChangeType::Modified, SystemId::Schemas, nullptr) {}

    ECChangeArray<SchemaChange>& Changes() { return m_changes; }

    SchemaChange* GetSchemaChange(Utf8StringCR schemaName)
        {
        for (size_t i = 0; i < Changes().Count(); i++)
            {
            SchemaChange& change = m_changes[i];
            if (schemaName.Equals(change.GetId()))
                return &change;
            }

        return nullptr;
        }

    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct SchemaComparer
    {
public:
    enum class AppendDetailLevel
        {
        Full,
        Partial
        };

    struct Options
        {
        private:
            AppendDetailLevel m_schemaDeleteDetailLevel;
            AppendDetailLevel m_schemaNewDetailLevel;
        public:
            Options(AppendDetailLevel schemDeleteDetailLevel = AppendDetailLevel::Full, AppendDetailLevel schemaNewDetailLevel = AppendDetailLevel::Full)
                :m_schemaDeleteDetailLevel(schemDeleteDetailLevel), m_schemaNewDetailLevel(schemaNewDetailLevel)
                {}

            AppendDetailLevel GetSchemaDeleteDetailLevel() const { return m_schemaDeleteDetailLevel; }
            AppendDetailLevel GetSchemaNewDetailLevel() const { return m_schemaNewDetailLevel; }
        };

private :
    Options m_options;

    BentleyStatus CompareECSchema(SchemaChange&, ECN::ECSchemaCR, ECN::ECSchemaCR);
    BentleyStatus CompareECClass(ClassChange&, ECN::ECClassCR, ECN::ECClassCR);
    BentleyStatus CompareECBaseClasses(BaseClassChanges&, ECN::ECBaseClassesList const&, ECN::ECBaseClassesList const&);
    BentleyStatus CompareECRelationshipClass(ClassChange&, ECN::ECRelationshipClassCR, ECN::ECRelationshipClassCR);
    BentleyStatus CompareECRelationshipConstraint(RelationshipConstraintChange&, ECN::ECRelationshipConstraintCR, ECN::ECRelationshipConstraintCR);
    BentleyStatus CompareECRelationshipConstraintClasses(RelationshipConstraintClassChanges&, ECN::ECRelationshipConstraintClassList const&, ECN::ECRelationshipConstraintClassList const&);
    BentleyStatus CompareECProperty(PropertyChange&, ECN::ECPropertyCR, ECN::ECPropertyCR);
    BentleyStatus CompareECProperties(PropertyChanges&, ECN::ECPropertyIterableCR, ECN::ECPropertyIterableCR);
    BentleyStatus CompareECClasses(ClassChanges&, ECN::ECClassContainerCR, ECN::ECClassContainerCR);
    BentleyStatus CompareECEnumerations(EnumerationChanges&, ECN::ECEnumerationContainerCR, ECN::ECEnumerationContainerCR);
    BentleyStatus CompareCustomAttributes(CustomAttributeChanges&, ECN::IECCustomAttributeContainerCR, ECN::IECCustomAttributeContainerCR);
    BentleyStatus CompareCustomAttribute(CustomAttributeChange&, ECN::IECInstanceCR, ECN::IECInstanceCR);
    BentleyStatus CompareECEnumeration(EnumerationChange&, ECN::ECEnumerationCR oldVal, ECN::ECEnumerationCR newVal);
    BentleyStatus CompareECEnumerators(EnumeratorChanges&, ECN::EnumeratorIterable const&, ECN::EnumeratorIterable const&);
    BentleyStatus CompareBaseClasses(BaseClassChanges&, ECN::ECBaseClassesList const&, ECN::ECBaseClassesList const&);
    BentleyStatus CompareReferences(SchemaReferenceChanges&, ECN::ECSchemaReferenceListCR, ECN::ECSchemaReferenceListCR);
    BentleyStatus CompareKindOfQuantity(KindOfQuantityChange&, ECN::KindOfQuantityCR, ECN::KindOfQuantityCR);
    BentleyStatus CompareKindOfQuantities(KindOfQuantityChanges&, ECN::KindOfQuantityContainerCR, ECN::KindOfQuantityContainerCR);
    BentleyStatus ComparePropertyCategory(PropertyCategoryChange&, ECN::PropertyCategoryCR, ECN::PropertyCategoryCR);
    BentleyStatus ComparePropertyCategories(PropertyCategoryChanges&, ECN::PropertyCategoryContainerCR, ECN::PropertyCategoryContainerCR);
    BentleyStatus ComparePhenomena(PhenomenonChanges&, ECN::PhenomenonContainerCR, ECN::PhenomenonContainerCR);
    BentleyStatus ComparePhenomenon(PhenomenonChange&, ECN::PhenomenonCR, ECN::PhenomenonCR);
    BentleyStatus CompareUnitSystems(UnitSystemChanges&, ECN::UnitSystemContainerCR, ECN::UnitSystemContainerCR);
    BentleyStatus CompareUnitSystem(UnitSystemChange&, ECN::UnitSystemCR, ECN::UnitSystemCR);
    BentleyStatus CompareUnits(UnitChanges&, ECN::UnitContainerCR, ECN::UnitContainerCR);
    BentleyStatus CompareUnit(UnitChange&, ECN::ECUnitCR, ECN::ECUnitCR);
    BentleyStatus CompareFormats(FormatChanges&, ECN::FormatContainerCR, ECN::FormatContainerCR);
    BentleyStatus CompareFormat(FormatChange&, ECN::ECFormatCR, ECN::ECFormatCR);

    BentleyStatus AppendECSchema(SchemaChange&, ECN::ECSchemaCR);
    BentleyStatus AppendECClass(ClassChange&, ECN::ECClassCR);
    BentleyStatus AppendECRelationshipClass(ClassChange&, ECN::ECRelationshipClassCR);
    BentleyStatus AppendECRelationshipConstraintClasses(RelationshipConstraintClassChanges&, ECN::ECRelationshipConstraintClassList const&);
    BentleyStatus AppendECRelationshipConstraint(RelationshipConstraintChange&, ECN::ECRelationshipConstraintCR);
    BentleyStatus AppendECEnumeration(EnumerationChange&, ECN::ECEnumerationCR);
    BentleyStatus AppendECProperty(PropertyChange&, ECN::ECPropertyCR);
    BentleyStatus AppendCustomAttributes(CustomAttributeChanges&, ECN::IECCustomAttributeContainerCR);
    BentleyStatus AppendCustomAttribute(CustomAttributeChange&, ECN::IECInstanceCR);
    BentleyStatus AppendBaseClasses(BaseClassChanges&, ECN::ECBaseClassesList const&);
    BentleyStatus AppendReferences(SchemaReferenceChanges&, ECN::ECSchemaReferenceListCR);
    BentleyStatus AppendKindOfQuantity(KindOfQuantityChange&, ECN::KindOfQuantityCR);
    BentleyStatus AppendPropertyCategory(PropertyCategoryChange&, ECN::PropertyCategoryCR);
    BentleyStatus AppendPhenomenon(PhenomenonChange&, ECN::PhenomenonCR);
    BentleyStatus AppendUnitSystem(UnitSystemChange&, ECN::UnitSystemCR);
    BentleyStatus AppendUnit(UnitChange&, ECN::ECUnitCR);
    BentleyStatus AppendFormat(FormatChange&, ECN::ECFormatCR);

    BentleyStatus ConvertECInstanceToValueMap(std::map<Utf8String, ECN::ECValue>&, ECN::IECInstanceCR);
    BentleyStatus ConvertECValuesCollectionToValueMap(std::map<Utf8String, ECN::ECValue>&, ECN::ECValuesCollectionCR);

public:
    SchemaComparer(){}

    ECOBJECTS_EXPORT BentleyStatus Compare(SchemaDiff&, bvector<ECN::ECSchemaCP> const& existingSet, bvector<ECN::ECSchemaCP> const& newSet, Options options = Options());
    static std::vector<Utf8String> Split(Utf8StringCR path, bool stripArrayIndex = false);
    static Utf8String Join(std::vector<Utf8String> const& paths, Utf8CP delimiter);
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct CustomAttributeValidator final : NonCopyableClass
    {
    enum class Policy
        {
        Accept,
        Reject
        };

    private:
        struct Rule final : NonCopyableClass
            {
            private:
                Policy m_policy;
                std::vector<Utf8String> m_pattern;
            public:
                Rule(Policy policy, Utf8StringCR pattern) :m_policy(policy), m_pattern(SchemaComparer::Split(pattern)) {}
                ~Rule() {}
                bool Match(std::vector<Utf8String> const& source) const;
                Policy GetPolicy() const { return m_policy; }
            };

        Policy m_defaultPolicy = Policy::Accept;
        std::map<Utf8String, std::vector<std::unique_ptr<Rule>>> m_rules;
        Utf8String m_wildcard = Utf8String("*");

        std::vector<std::unique_ptr<Rule>> const& GetRelevantRules(CustomAttributeChange&) const;

        static Utf8String GetSchemaName(Utf8StringCR path);

    public:
        CustomAttributeValidator() 
            { 
            // add entry for wildcard rules
            m_rules[m_wildcard].clear();
            }

        ~CustomAttributeValidator() {}

        ECOBJECTS_EXPORT void AddAcceptRule(Utf8StringCR accessString);
        ECOBJECTS_EXPORT void AddRejectRule(Utf8StringCR accessString);
        Policy GetDefaultPolicy() const { return m_defaultPolicy; }
        void SetDefaultPolicy(Policy defaultPolicy) { m_defaultPolicy = defaultPolicy; }
        bool HasRuleForSchema(Utf8StringCR schemaName) const { return m_rules.find(schemaName) != m_rules.end(); }

        ECOBJECTS_EXPORT Policy Validate(CustomAttributeChange&) const;
    };
END_BENTLEY_ECOBJECT_NAMESPACE

