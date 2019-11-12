/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct ECChange : RefCountedBase
    {
public:
    enum class OpCode
        {
        Deleted = 1, //This need to be none zero base
        Modified = 2,
        New = 3
        };

    enum class Type
        {
        Alias,
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
        IsInteger,
        IsPolymorphic,
        IsReadonly,
        IsStrict,
        IsString,
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
        RoundFactor,
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

    enum class Status
        {
        Pending,
        Done,
        };

    private:
        Type m_type;
        Utf8String m_changeName;
        OpCode m_opCode;
        ECChange const* m_parent = nullptr;
        Status m_status = Status::Pending;

        virtual bool _IsChanged() const = 0;
        virtual void _WriteToString(Utf8StringR str, int currentIndex, int indentSize) const = 0;

    protected:
        ECChange(OpCode opCode, Type type, ECChange const* parent = nullptr, Utf8CP name = nullptr) : m_type(type), m_changeName(name), m_opCode(opCode), m_parent(parent) {}

        ECOBJECTS_EXPORT static void AppendBegin(Utf8StringR str, ECChange const& change, int currentIndex);
        static void AppendEnd(Utf8StringR str) { str.append("\r\n"); }
        
        ECOBJECTS_EXPORT static Utf8CP TypeToString(Type);

    public:
        virtual ~ECChange() {};
        Type GetType() const { return m_type; }
        bool HasChangeName() const { return !m_changeName.empty(); }
        ECOBJECTS_EXPORT Utf8CP GetChangeName() const;
        OpCode GetOpCode() const { return m_opCode; }
        ECChange const* GetParent() const { return m_parent; }
        //! IsChanged is always true if ECChange::GetOpCode is OpCode::New or OpCode::Deleted.
        //! If ECChange::GetOpCode is OpCode::Modified, IsChanged returns true if old and new value differ from each other
        bool IsChanged() const { return m_opCode != OpCode::Modified || _IsChanged(); }
        void WriteToString(Utf8StringR str, int initIndex = 0, int indentSize = 4) const { _WriteToString(str, initIndex, indentSize); }
        Utf8String ToString() const { Utf8String str;  WriteToString(str); return str; }

        Status GetStatus() { return m_status; }
        void SetStatus(Status status) { m_status = status; }
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

    protected:
        CompositeECChange(OpCode opCode, Type type, ECChange const* parent = nullptr, Utf8CP name = nullptr) : ECChange(opCode, type, parent, name) {}

        template<typename T>
        T& Get(Type memberType)
            {
            static_assert(std::is_base_of<ECChange, T>::value, "T not derived from ECChange");
            Utf8CP memberTypeStr = TypeToString(memberType);
            auto itor = m_changes.find(memberTypeStr);
            if (itor != m_changes.end())
                return *(static_cast<T*>(itor->second.get()));

            ECChangePtr changePtr = new T(GetOpCode(), memberType, this, nullptr);
            ECChange* changeP = changePtr.get();
            m_changes[changePtr->GetChangeName()] = changePtr;
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
            if (m_changes.empty() || !IsChanged())
                return;

            AppendBegin(str, *this, currentIndex);
            AppendEnd(str);
            for (ECChangePtr const& change : m_changes)
                {
                change->WriteToString(str, currentIndex + indentSize, indentSize);
                }
            }

    public:
        ECChangeArray(OpCode opCode, Type type, ECChange const* parent, Utf8CP name = nullptr) : ECChange(opCode, type, parent, name)
            {
            static_assert(std::is_base_of<ECChange, TArrayElement>::value, "TArrayElement not derived from ECChange");
            }

        virtual ~ECChangeArray() {}

        size_t Count() const { return m_changes.size(); }
        bool IsEmpty() const { return m_changes.empty(); }
        TArrayElement& operator[](size_t index) { return static_cast<TArrayElement&>(*m_changes[index]); }

        RefCountedPtr<TArrayElement> const* begin() const { return reinterpret_cast<RefCountedPtr<TArrayElement> const*>(m_changes.begin()); }
        RefCountedPtr<TArrayElement>* begin() { return reinterpret_cast<RefCountedPtr<TArrayElement>*>(m_changes.begin()); }
        RefCountedPtr<TArrayElement> const* end() const { return reinterpret_cast<RefCountedPtr<TArrayElement> const*>(m_changes.end()); }
        RefCountedPtr<TArrayElement>* end() { return reinterpret_cast<RefCountedPtr<TArrayElement>*>(m_changes.end()); }

        RefCountedPtr<TArrayElement> CreateElement(OpCode opCode, Type elementType, Utf8CP name = nullptr) { return new TArrayElement(opCode, elementType, this, name); }

        //! Adds a change to the array.
        //! Changes for which IsChanged is false are skipped
        //! The typical workflow is:
        //! - Call CreateElement
        //! - Populate the array element change
        //! - Call Add
        void Add(RefCountedPtr<TArrayElement>& change)
            {
            if (!change->IsChanged())
                return;

            m_changes.push_back(change);
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

        bool _IsChanged() const override { return m_old != m_new; }

        void _WriteToString(Utf8StringR str, int currentIndex, int indentSize) const override
            {
            if (!IsChanged())
                return;

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
        PrimitiveChange(OpCode opCode, Type type, ECChange const* parent = nullptr, Utf8CP name = nullptr) : ECChange(opCode, type, parent, name) {}
        ~PrimitiveChange() {}
        //! Gets the value after the change
        Nullable<T> const& GetNew() const { return m_new; }
        //! Gets the value before the change
        Nullable<T> const& GetOld() const { return m_old; }

        //! Sets old and new value only if the differ from each other
        BentleyStatus Set(Nullable<T> const& oldValue, Nullable<T> const& newValue)
            {
            if (GetOpCode() == OpCode::Deleted)
                {
                if (newValue != nullptr)
                    {
                    BeAssert(false && "For Change marked as Deleted, new value must be nullptr");
                    return ERROR;
                    }
                }

            if (GetOpCode() == OpCode::New)
                {
                if (oldValue != nullptr)
                    {
                    BeAssert(false && "For Change marked as New, old value must be nullptr");
                    return ERROR;
                    }
                }

            m_old = nullptr;
            m_new = nullptr;

            if (oldValue == newValue)
                return SUCCESS;

            m_old = oldValue;
            m_new = newValue;
            return SUCCESS;
            }

        Utf8String ToString() const
            {
            switch (GetOpCode())
                {
                    case OpCode::Deleted:
                    {
                    if (GetOld().IsNull())
                        return "<unset>";

                    return Stringify(GetOld().Value());
                    }

                    case OpCode::New:
                    {
                    if (GetNew().IsNull())
                        return "<unset>";

                    return Stringify(GetNew().Value());
                    }

                    case OpCode::Modified:
                    {
                    Utf8String str;
                    if (GetOld().IsNull())
                        str = "<unset>";
                    else
                        str = Stringify(GetOld().Value());

                    str.append(" -> ");

                    if (GetNew().IsNull())
                        str.append("<unset>");
                    else
                        str.append(Stringify(GetNew().Value()));

                    return str;
                    }

                    default:
                        BeAssert(false && "Unhandled OpCode enum value");
                        return "<programmer error: Unhanlded OpCode enum value";
                }
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
    SchemaElementChange(OpCode opCode, Type type, ECChange const* parent = nullptr, Utf8CP name = nullptr) : CompositeECChange(opCode, type, parent, name) {}

public:
    virtual ~SchemaElementChange() {}

    StringChange& Name() { return Get<StringChange>(Type::Name); }
    StringChange& DisplayLabel() { return Get<StringChange>(Type::DisplayLabel); }
    StringChange& Description() { return Get<StringChange>(Type::Description); }
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
        ECN::PrimitiveType m_primType;

        bool _IsChanged() const override { return m_value != nullptr && m_value->IsChanged(); }
        void _WriteToString(Utf8StringR str, int currentIndex, int indentSize) const override;

        BentleyStatus Inititalize(ECN::PrimitiveType);

    public:
        PropertyValueChange(OpCode opCode, Type type, ECChange const* parent = nullptr, Utf8CP accessString = nullptr)
            : ECChange(opCode, Type::PropertyValue, parent, accessString)
            {
            BeAssert(!Utf8String::IsNullOrEmpty(accessString) && "access string must not be empty for PropertyValueChange");
            BeAssert(type == GetType());
            }

        ~PropertyValueChange() {}
        Utf8CP GetAccessString() const { BeAssert(HasChangeName()); return GetChangeName(); }
        bool HasValue() const { return m_value != nullptr; }
        ECN::PrimitiveType GetValueType() const { return m_primType; }
        StringChange* GetString() const { BeAssert(m_primType == ECN::PRIMITIVETYPE_String); if (m_primType != ECN::PRIMITIVETYPE_String) return nullptr; return static_cast<StringChange*>(m_value.get()); }
        BooleanChange* GetBoolean() const { BeAssert(m_primType == ECN::PRIMITIVETYPE_Boolean); if (m_primType != ECN::PRIMITIVETYPE_Boolean) return nullptr; return static_cast<BooleanChange*>(m_value.get()); }
        DateTimeChange* GetDateTime() const { BeAssert(m_primType == ECN::PRIMITIVETYPE_DateTime); if (m_primType != ECN::PRIMITIVETYPE_DateTime) return nullptr; return static_cast<DateTimeChange*>(m_value.get()); }
        DoubleChange* GetDouble() const { BeAssert(m_primType == ECN::PRIMITIVETYPE_Double); if (m_primType != ECN::PRIMITIVETYPE_Double) return nullptr; return static_cast<DoubleChange*>(m_value.get()); }
        Int32Change* GetInteger() const { BeAssert(m_primType == ECN::PRIMITIVETYPE_Integer); if (m_primType != ECN::PRIMITIVETYPE_Integer) return nullptr; return static_cast<Int32Change*>(m_value.get()); }
        Int64Change* GetLong() const { BeAssert(m_primType == ECN::PRIMITIVETYPE_Long); if (m_primType != ECN::PRIMITIVETYPE_Long) return nullptr; return static_cast<Int64Change*>(m_value.get()); }
        Point2dChange* GetPoint2d() const { BeAssert(m_primType == ECN::PRIMITIVETYPE_Point2d); if (m_primType != ECN::PRIMITIVETYPE_Point2d) return nullptr; return static_cast<Point2dChange*>(m_value.get()); }
        Point3dChange* GetPoint3d() const { BeAssert(m_primType == ECN::PRIMITIVETYPE_Point3d); if (m_primType != ECN::PRIMITIVETYPE_Point3d) return nullptr; return static_cast<Point3dChange*>(m_value.get()); }
        BinaryChange* GetBinary() const { BeAssert(m_primType == ECN::PRIMITIVETYPE_Binary); if (m_primType != ECN::PRIMITIVETYPE_Binary) return nullptr; return static_cast<BinaryChange*>(m_value.get()); }

        BentleyStatus Set(ECN::ECValueCR oldValue, ECN::ECValueCR newValue);
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct CustomAttributeChange final : public ECChange
    {
    private:
        std::unique_ptr<ECChangeArray<PropertyValueChange>> m_propValueChanges;

        bool _IsChanged() const override { return m_propValueChanges->IsChanged(); }
        void _WriteToString(Utf8StringR str, int currentIndex, int indentSize) const override
            {
            if (m_propValueChanges->Count() == 0 || !IsChanged())
                return;

            AppendBegin(str, *this, currentIndex);
            AppendEnd(str);
            m_propValueChanges->WriteToString(str, currentIndex + indentSize, indentSize);
            }

    public:
        CustomAttributeChange(OpCode opCode, Type type, ECChange const* parent = nullptr, Utf8CP name = nullptr) : ECChange(opCode, Type::CustomAttribute, parent, name)
            { 
            BeAssert(type == GetType());
            m_propValueChanges = std::make_unique<ECChangeArray<PropertyValueChange>>(opCode, GetType(), this, GetChangeName());
            }
        ~CustomAttributeChange() {}

        ECChangeArray<PropertyValueChange>& PropValues() { return *m_propValueChanges; }
        ECChangeArray<PropertyValueChange> const& PropValues() const { return *m_propValueChanges; }
    };

typedef ECChangeArray<CustomAttributeChange> CustomAttributeChanges;

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct RelationshipConstraintChange final : public CompositeECChange
    {
    public:
        RelationshipConstraintChange(OpCode opCode, Type type, ECChange const* parent = nullptr, Utf8CP name = nullptr) : CompositeECChange(opCode, type, parent, name) { BeAssert(type == Type::Source || type == Type::Target); }
        ~RelationshipConstraintChange() {}
        StringChange& RoleLabel() { return Get<StringChange>(Type::RoleLabel); }
        StringChange& Multiplicity() { return Get<StringChange>(Type::Multiplicity); }
        BooleanChange& IsPolymorphic() { return Get<BooleanChange>(Type::IsPolymorphic); }
        RelationshipConstraintClassChanges& ConstraintClasses() { return Get<RelationshipConstraintClassChanges>(Type::ConstraintClasses); }
        CustomAttributeChanges& CustomAttributes() { return Get<CustomAttributeChanges>(Type::CustomAttributes); }
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct PropertyChange final : public SchemaElementChange
    {
    public:
        PropertyChange(OpCode opCode, Type type, ECChange const* parent = nullptr, Utf8CP customId = nullptr) : SchemaElementChange(opCode, Type::Property, parent, customId) { BeAssert(type == GetType()); }
        ~PropertyChange() {}
        StringChange& TypeName() { return Get<StringChange>(Type::TypeName); }
        PrimitiveChange<ECN::ECValue>& MinimumValue() { return Get<PrimitiveChange<ECN::ECValue>>(Type::MinimumValue); }
        PrimitiveChange<ECN::ECValue>& MaximumValue() { return Get<PrimitiveChange<ECN::ECValue>>(Type::MaximumValue); }
        UInt32Change& MinimumLength() { return Get<UInt32Change>(Type::MinimumLength); }
        UInt32Change& MaximumLength() { return Get<UInt32Change>(Type::MaximumLength); }
        BooleanChange& IsStruct() { return Get<BooleanChange>(Type::IsStrict); }
        BooleanChange& IsStructArray() { return Get<BooleanChange>(Type::IsStructArray); }
        BooleanChange& IsPrimitive() { return Get<BooleanChange>(Type::IsPrimitive); }
        BooleanChange& IsPrimitiveArray() { return Get<BooleanChange>(Type::IsPrimitiveArray); }
        BooleanChange& IsNavigation() { return Get<BooleanChange>(Type::IsNavigation); }
        StringChange& ExtendedTypeName() { return Get<StringChange>(Type::ExtendedTypeName); }
        BooleanChange& IsReadonly() { return Get<BooleanChange>(Type::IsReadonly); }
        Int32Change& Priority() { return Get<Int32Change>(Type::PropertyPriority); }
        StringChange& KindOfQuantity() { return Get<StringChange>(Type::KindOfQuantity); }
        StringChange& Enumeration() { return Get<StringChange>(Type::Enumeration); }
        StringChange& Category() { return Get<StringChange>(Type::PropertyCategory); }

        PrimitiveChange<ECN::ECRelatedInstanceDirection>& NavigationDirection() { return Get<PrimitiveChange<ECN::ECRelatedInstanceDirection>>(Type::Direction); }
        StringChange& NavigationRelationship() { return Get<StringChange>(Type::Relationship); }
        UInt32Change& ArrayMinOccurs() { return Get<UInt32Change>(Type::MinOccurs); }
        UInt32Change& ArrayMaxOccurs() { return Get<UInt32Change>(Type::MaxOccurs); }

        CustomAttributeChanges& CustomAttributes() { return Get<CustomAttributeChanges>(Type::CustomAttributes); }
    };

typedef ECChangeArray<PropertyChange> PropertyChanges;

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct ClassChange final : public SchemaElementChange
    {
    public:
        ClassChange(OpCode opCode, Type type, ECChange const* parent = nullptr, Utf8CP name = nullptr) : SchemaElementChange(opCode, type, parent, name) {}
        ~ClassChange() {}

        PrimitiveChange<ECN::ECClassModifier>& ClassModifier() { return Get<PrimitiveChange<ECN::ECClassModifier>>(Type::ClassModifier); }
        PrimitiveChange<ECN::ECClassType>& ClassType() { return Get<PrimitiveChange<ECN::ECClassType>>(Type::ClassType); }
        BaseClassChanges& BaseClasses() { return Get<BaseClassChanges>(Type::BaseClasses); }
        PropertyChanges& Properties() { return Get<PropertyChanges>(Type::Properties); }
        CustomAttributeChanges& CustomAttributes() { return Get<CustomAttributeChanges>(Type::CustomAttributes); }

        PrimitiveChange<ECN::StrengthType>& Strength() { return Get<PrimitiveChange<ECN::StrengthType>>(Type::StrengthType); }
        PrimitiveChange<ECN::ECRelatedInstanceDirection>& StrengthDirection() { return Get<PrimitiveChange<ECN::ECRelatedInstanceDirection>>(Type::StrengthDirection); }
        RelationshipConstraintChange& Source() { return Get<RelationshipConstraintChange>(Type::Source); }
        RelationshipConstraintChange& Target() { return Get<RelationshipConstraintChange>(Type::Target); }
    };

typedef ECChangeArray<ClassChange> ClassChanges;

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct EnumeratorChange final : public SchemaElementChange
    {
    public:
        EnumeratorChange(OpCode opCode, Type type, ECChange const* parent = nullptr, Utf8CP name = nullptr) : SchemaElementChange(opCode, Type::Enumerator, parent, name) { BeAssert(type == GetType()); }
        ~EnumeratorChange() {}
        BooleanChange IsInteger() { return Get<BooleanChange>(Type::IsInteger); }
        Int32Change& Integer() { return Get<Int32Change>(Type::Integer); }
        BooleanChange IsString() { return Get<BooleanChange>(Type::IsString); }
        StringChange& String() { return Get<StringChange>(Type::String); }
    };

typedef ECChangeArray<EnumeratorChange> EnumeratorChanges;

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct EnumerationChange final : public SchemaElementChange
    {
    public:
        EnumerationChange(OpCode opCode, Type type, ECChange const* parent = nullptr, Utf8CP name = nullptr) : SchemaElementChange(opCode, Type::Enumeration, parent, name) { BeAssert(type == GetType()); }
        ~EnumerationChange() {}
        StringChange& TypeName() { return Get<StringChange>(Type::TypeName); }
        BooleanChange& IsStrict() { return Get<BooleanChange>(Type::IsStrict); }
        EnumeratorChanges& Enumerators() { return Get<EnumeratorChanges>(Type::Enumerators); }
    };

typedef ECChangeArray<EnumerationChange> EnumerationChanges;

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct KindOfQuantityChange final : public SchemaElementChange
    {
    public:
        KindOfQuantityChange(OpCode opCode, Type type, ECChange const* parent = nullptr, Utf8CP name = nullptr) : SchemaElementChange(opCode, Type::KindOfQuantity, parent, name) { BeAssert(type == GetType()); }
        ~KindOfQuantityChange() {}
        StringChange& PersistenceUnit() { return Get<StringChange>(Type::KoqPersistenceUnit); }
        DoubleChange& RelativeError() { return Get<DoubleChange>(Type::KoqRelativeError); }
        ECChangeArray<StringChange>& PresentationFormats() { return Get<ECChangeArray<StringChange>>(Type::KoqPresentationFormats); }
    };

typedef ECChangeArray<KindOfQuantityChange> KindOfQuantityChanges;

//=======================================================================================
// @bsiclass                                                Krischan.Eberle       06/2017
//+===============+===============+===============+===============+===============+======
struct PropertyCategoryChange final : public SchemaElementChange
    {
    public:
        PropertyCategoryChange(OpCode opCode, Type type, ECChange const* parent = nullptr, Utf8CP name = nullptr) : SchemaElementChange(opCode, Type::PropertyCategory, parent, name) { BeAssert(type == GetType()); }
        ~PropertyCategoryChange() {}
        UInt32Change& Priority() { return Get<UInt32Change>(Type::PropertyCategoryPriority); }
    };


typedef ECChangeArray<PropertyCategoryChange> PropertyCategoryChanges;


//=======================================================================================
// @bsiclass                                                Krischan.Eberle       02/2018
//+===============+===============+===============+===============+===============+======
struct PhenomenonChange final : public SchemaElementChange
    {
    public:
        PhenomenonChange(OpCode opCode, Type type, ECChange const* parent = nullptr, Utf8CP name = nullptr) : SchemaElementChange(opCode, Type::Phenomenon, parent, name) { BeAssert(type == GetType()); }
        ~PhenomenonChange() {}
        StringChange& Definition() { return Get<StringChange>(Type::PhenomenonDefinition); }
    };

typedef ECChangeArray<PhenomenonChange> PhenomenonChanges;

//=======================================================================================
// @bsiclass                                                Krischan.Eberle       02/2018
//+===============+===============+===============+===============+===============+======
struct UnitSystemChange final : public SchemaElementChange
    {
    public:
        UnitSystemChange(OpCode opCode, Type type, ECChange const* parent = nullptr, Utf8CP name = nullptr) : SchemaElementChange(opCode, Type::UnitSystem, parent, name) { BeAssert(type == GetType()); }
        ~UnitSystemChange() {}
    };

typedef ECChangeArray<UnitSystemChange> UnitSystemChanges;

//=======================================================================================
// @bsiclass                                                Krischan.Eberle       02/2018
//+===============+===============+===============+===============+===============+======
struct UnitChange final : public SchemaElementChange
    {
    public:
        UnitChange(OpCode opCode, Type type, ECChange const* parent = nullptr, Utf8CP name = nullptr) : SchemaElementChange(opCode, Type::Unit, parent, name) { BeAssert(type == GetType()); }
        ~UnitChange() {}
        StringChange& Definition() { return Get<StringChange>(Type::UnitDefinition); }
        StringChange& Phenomenon() { return Get<StringChange>(Type::Phenomenon); }
        StringChange& UnitSystem() { return Get<StringChange>(Type::UnitSystem); }
        DoubleChange& Numerator() { return Get<DoubleChange>(Type::UnitNumerator); }
        DoubleChange& Denominator() { return Get<DoubleChange>(Type::UnitDenominator); }
        DoubleChange& Offset() { return Get<DoubleChange>(Type::UnitOffset); }
        BooleanChange& IsConstant() { return Get<BooleanChange>(Type::UnitIsConstant); }
        StringChange& InvertingUnit() { return Get<StringChange>(Type::UnitInvertingUnit); }
    };

typedef ECChangeArray<UnitChange> UnitChanges;

//=======================================================================================
// @bsiclass                                                      05/2018
//+===============+===============+===============+===============+===============+======
struct NumericFormatSpecChange final : public CompositeECChange
    {
    public:
        NumericFormatSpecChange(OpCode opCode, Type type, ECChange const* parent = nullptr, Utf8CP name = nullptr) : CompositeECChange(opCode, Type::NumericFormatSpec, parent, name) { BeAssert(type == GetType()); }
        ~NumericFormatSpecChange() {}

        DoubleChange& RoundFactor() { return Get<DoubleChange>(Type::RoundFactor); }
        StringChange& PresentationType() { return Get<StringChange>(Type::PresentationType); }
        Int32Change& DecimalPrecision() { return Get<Int32Change>(Type::DecimalPrecision); }
        Int32Change& FractionalPrecision() { return Get<Int32Change>(Type::FractionalPrecision); }
        UInt32Change& MinWidth() { return Get<UInt32Change>(Type::MinWidth); }
        StringChange& ScientificType() { return Get<StringChange>(Type::ScientificType); }
        StringChange& ShowSignOption() { return Get<StringChange>(Type::ShowSignOption); }
        StringChange& FormatTraits() { return Get<StringChange>(Type::FormatTraits); }
        StringChange& DecimalSeparator() { return Get<StringChange>(Type::DecimalSeparator); }
        StringChange& ThousandsSeparator() { return Get<StringChange>(Type::ThousandSeparator); }
        StringChange& UomSeparator() { return Get<StringChange>(Type::UomSeparator); }
        StringChange& StationSeparator() { return Get<StringChange>(Type::StationSeparator); }
        UInt32Change& StationOffsetSize() { return Get<UInt32Change>(Type::StationOffsetSize); }

        BentleyStatus SetFrom(Formatting::NumericFormatSpecCP oldSpec, Formatting::NumericFormatSpecCP newSpec);
    };

//=======================================================================================
// @bsiclass                                                      05/2018
//+===============+===============+===============+===============+===============+======
struct CompositeValueSpecChange final : public CompositeECChange
    {
    public:
        CompositeValueSpecChange(OpCode opCode, Type type, ECChange const* parent = nullptr, Utf8CP name = nullptr) : CompositeECChange(opCode, Type::CompositeValueSpec, parent, name) { BeAssert(type == GetType()); }
        ~CompositeValueSpecChange() {}

        BooleanChange& IncludeZero() { return Get<BooleanChange>(Type::CompositeIncludeZero); }
        StringChange& Spacer() { return Get<StringChange>(Type::CompositeSpacer); }
        StringChange& MajorUnit() { return Get<StringChange>(Type::CompositeMajorUnit); }
        StringChange& MajorLabel() { return Get<StringChange>(Type::CompositeMajorLabel); }
        StringChange& MiddleUnit() { return Get<StringChange>(Type::CompositeMiddleUnit); }
        StringChange& MiddleLabel() { return Get<StringChange>(Type::CompositeMiddleLabel); }
        StringChange& MinorUnit() { return Get<StringChange>(Type::CompositeMinorUnit); }
        StringChange& MinorLabel() { return Get<StringChange>(Type::CompositeMinorLabel); }
        StringChange& SubUnit() { return Get<StringChange>(Type::CompositeSubUnit); }
        StringChange& SubLabel() { return Get<StringChange>(Type::CompositeSubLabel); }

        BentleyStatus SetFrom(Formatting::CompositeValueSpecCP oldSpec, Formatting::CompositeValueSpecCP newSpec);
    };

//=======================================================================================
// @bsiclass                                               Kyle.Abramowitz        04/2018
//+===============+===============+===============+===============+===============+======
struct FormatChange final : public SchemaElementChange
    {
    public:
        FormatChange(OpCode opCode, Type type, ECChange const* parent = nullptr, Utf8CP name = nullptr) : SchemaElementChange(opCode, Type::Format, parent, name) { BeAssert(type == GetType()); }
        ~FormatChange() {}
        NumericFormatSpecChange& NumericSpec() { return Get<NumericFormatSpecChange>(Type::NumericFormatSpec); }
        CompositeValueSpecChange& CompositeSpec() { return Get<CompositeValueSpecChange>(Type::CompositeValueSpec); }
    };

typedef ECChangeArray<FormatChange> FormatChanges;


//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct SchemaChange final : public SchemaElementChange
    {
    public:
        SchemaChange(OpCode opCode, Type type, ECChange const* parent = nullptr, Utf8CP name = nullptr) : SchemaElementChange(opCode, Type::Schema, parent, name) { BeAssert(type == GetType()); }
        ~SchemaChange() {}

        StringChange& Alias() { return Get<StringChange>(Type::Alias); }
        UInt32Change& VersionRead() { return Get<UInt32Change>(Type::VersionRead); }
        UInt32Change& VersionMinor() { return Get<UInt32Change>(Type::VersionMinor); }
        UInt32Change& VersionWrite() { return Get<UInt32Change>(Type::VersionWrite); }
        UInt32Change& ECVersion() { return Get<UInt32Change>(Type::ECVersion); }
        UInt32Change& OriginalECXmlVersionMajor() { return Get<UInt32Change>(Type::OriginalECXmlVersionMajor); }
        UInt32Change& OriginalECXmlVersionMinor() { return Get<UInt32Change>(Type::OriginalECXmlVersionMinor); }

        CustomAttributeChanges& CustomAttributes() { return Get<CustomAttributeChanges>(Type::CustomAttributes); }
        SchemaReferenceChanges& References() { return Get<SchemaReferenceChanges>(Type::SchemaReferences); }
        ClassChanges& Classes() { return Get<ClassChanges>(Type::Classes); }
        EnumerationChanges& Enumerations() { return Get<EnumerationChanges>(Type::Enumerations); }
        KindOfQuantityChanges& KindOfQuantities() { return Get<KindOfQuantityChanges>(Type::KindOfQuantities); }
        PropertyCategoryChanges& PropertyCategories() { return Get<PropertyCategoryChanges>(Type::PropertyCategories); }
        PhenomenonChanges& Phenomena() { return Get<PhenomenonChanges>(Type::Phenomena); }
        UnitSystemChanges& UnitSystems() { return Get<UnitSystemChanges>(Type::UnitSystems); }
        UnitChanges& Units() { return Get<UnitChanges>(Type::Units); }
        FormatChanges& Formats() { return Get<FormatChanges>(Type::Formats); }
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle       05/2018
//+===============+===============+===============+===============+===============+======
struct SchemaDiff final
    {
private:
    ECChangeArray<SchemaChange> m_changes;

public:
    SchemaDiff() : m_changes(ECChange::OpCode::Modified, ECChange::Type::Schemas, nullptr) {}

    ECChangeArray<SchemaChange>& Changes() { return m_changes; }

    SchemaChange* GetSchemaChange(Utf8StringCR schemaName)
        {
        for (size_t i = 0; i < Changes().Count(); i++)
            {
            SchemaChange& change = m_changes[i];
            if (schemaName.Equals(change.GetChangeName()))
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
    enum class DetailLevel
        {
        Full,
        NoSchemaElements
        };

    struct Options final
        {
        private:
            DetailLevel m_levelForNewSchema = DetailLevel::Full;
            DetailLevel m_levelForDeletedSchema = DetailLevel::Full;
        public:
            Options() {}
            Options(DetailLevel schemaNewDetailLevel, DetailLevel levelForDeletedSchema) :m_levelForNewSchema(schemaNewDetailLevel), m_levelForDeletedSchema(levelForDeletedSchema) {}
            DetailLevel GetDetailLevelForNewSchema() const { return m_levelForNewSchema; }
            DetailLevel GetDetailLevelForDeletedSchema() const { return m_levelForDeletedSchema; }
        };

private :
    Options m_options;

    BentleyStatus CompareSchema(SchemaChange&, ECN::ECSchemaCP, ECN::ECSchemaCP);
    BentleyStatus CompareReferences(SchemaReferenceChanges&, ECN::ECSchemaReferenceList const*, ECN::ECSchemaReferenceList const*);
    BentleyStatus CompareClasses(ClassChanges&, ECN::ECClassContainerCP, ECN::ECClassContainerCP);
    BentleyStatus CompareClass(ClassChange&, ECN::ECClassCP, ECN::ECClassCP);
    BentleyStatus CompareBaseClasses(BaseClassChanges&, ECN::ECBaseClassesList const*, ECN::ECBaseClassesList const*);
    BentleyStatus CompareRelationshipConstraint(RelationshipConstraintChange&, ECN::ECRelationshipConstraintCP, ECN::ECRelationshipConstraintCP);
    BentleyStatus CompareRelationshipConstraintClasses(RelationshipConstraintClassChanges&, ECN::ECRelationshipConstraintClassList const*, ECN::ECRelationshipConstraintClassList const*);
    BentleyStatus CompareProperties(PropertyChanges&, bvector<ECN::ECPropertyCP> const&, bvector<ECN::ECPropertyCP> const&);
    BentleyStatus CompareProperty(PropertyChange&, ECN::ECPropertyCP, ECN::ECPropertyCP);
    BentleyStatus CompareEnumerations(EnumerationChanges&, ECN::ECEnumerationContainerCP, ECN::ECEnumerationContainerCP);
    BentleyStatus CompareEnumeration(EnumerationChange&, ECN::ECEnumerationCP oldVal, ECN::ECEnumerationCP newVal);
    BentleyStatus CompareEnumerators(EnumeratorChanges&, bvector<ECN::ECEnumeratorCP> const&, bvector<ECN::ECEnumeratorCP> const&);
    BentleyStatus CompareEnumerator(EnumeratorChange&, ECN::ECEnumeratorCP oldVal, ECN::ECEnumeratorCP newVal);
    BentleyStatus CompareKindOfQuantities(KindOfQuantityChanges&, ECN::KindOfQuantityContainerCP, ECN::KindOfQuantityContainerCP);
    BentleyStatus CompareKindOfQuantity(KindOfQuantityChange&, ECN::KindOfQuantityCP, ECN::KindOfQuantityCP);
    BentleyStatus ComparePropertyCategories(PropertyCategoryChanges&, ECN::PropertyCategoryContainerCP, ECN::PropertyCategoryContainerCP);
    BentleyStatus ComparePropertyCategory(PropertyCategoryChange&, ECN::PropertyCategoryCP, ECN::PropertyCategoryCP);
    BentleyStatus ComparePhenomena(PhenomenonChanges&, ECN::PhenomenonContainerCP, ECN::PhenomenonContainerCP);
    BentleyStatus ComparePhenomenon(PhenomenonChange&, ECN::PhenomenonCP, ECN::PhenomenonCP);
    BentleyStatus CompareUnitSystems(UnitSystemChanges&, ECN::UnitSystemContainerCP, ECN::UnitSystemContainerCP);
    BentleyStatus CompareUnitSystem(UnitSystemChange&, ECN::UnitSystemCP, ECN::UnitSystemCP);
    BentleyStatus CompareUnits(UnitChanges&, ECN::UnitContainerCP, ECN::UnitContainerCP);
    BentleyStatus CompareUnit(UnitChange&, ECN::ECUnitCP, ECN::ECUnitCP);
    BentleyStatus CompareFormats(FormatChanges&, ECN::FormatContainerCP, ECN::FormatContainerCP);
    BentleyStatus CompareFormat(FormatChange&, ECN::ECFormatCP, ECN::ECFormatCP);
    BentleyStatus CompareCustomAttributes(CustomAttributeChanges&, bvector<ECN::IECInstancePtr> const&, bvector<ECN::IECInstancePtr> const&);
    BentleyStatus CompareCustomAttribute(CustomAttributeChange&, ECN::IECInstanceCP, ECN::IECInstanceCP);

    BentleyStatus ConvertECInstanceToValueMap(bmap<Utf8String, ECN::ECValue>&, ECN::IECInstanceCR);
    BentleyStatus ConvertECValuesCollectionToValueMap(bmap<Utf8String, ECN::ECValue>&, ECN::ECValuesCollectionCR);

public:
    SchemaComparer(){}

    ECOBJECTS_EXPORT BentleyStatus Compare(SchemaDiff&, bvector<ECN::ECSchemaCP> const& existingSet, bvector<ECN::ECSchemaCP> const& newSet, Options options = Options());
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            07/2019
//+===============+===============+===============+===============+===============+======
struct CustomAttributeValidator final : NonCopyableClass
    {
    static Utf8String WILDCARD;
    enum class Policy
        {
        Accept,
        Reject
        };

    enum class ChangeType
        {
        New = 1,
        Modified = 2,
        Delete = 4,
        All = New | Modified | Delete,
        };

    struct PropertyRule final : RefCountedBase
        {
        private:
            Utf8String m_accessString;
            ChangeType m_changeType;
            bvector<Utf8String> m_path;
            Policy m_policy;
            ECOBJECTS_EXPORT PropertyRule(Policy policy, Utf8CP accessString, ChangeType changeType);

        public:
            ~PropertyRule(){}
            Utf8StringCR GetAccessString() const { return m_accessString; }
            ChangeType GetChangeType() const { return m_changeType; }
            bool AppliesToChangeType(ChangeType type) const { return ((int) type & (int) m_changeType) == (int) type; }
            bool Matches(bvector<Utf8String> const& accessStringTokens) const;
            Policy GetPolicy() const { return m_policy; }
            bvector<Utf8String> GetPath() const { return m_path; }
            ECOBJECTS_EXPORT static RefCountedPtr<PropertyRule> Create(Policy policy,  Utf8CP accessString, ChangeType changeType);
        };

    struct ClassRule final : RefCountedBase
        {
        private:
            Utf8String m_schemaFullName;
            Utf8String m_customAttributeClassName;
            ChangeType m_changeType;
            Policy m_policy;
            std::vector<RefCountedPtr<PropertyRule>> m_rules;
            ClassRule(Policy policy, Utf8CP schemaFullName, Utf8CP customAttributeClassName, ChangeType changeType)
                : m_schemaFullName(schemaFullName), m_customAttributeClassName (customAttributeClassName), m_changeType(changeType), m_policy(policy)
                {}
        public:
            Utf8StringCR GetSchemaName() const { return m_schemaFullName; }
            Utf8StringCR GetClassName() const { return m_customAttributeClassName; }
            ChangeType GetChangeType() const { return m_changeType; }
            Policy GetPolicy() const { return m_policy; }
            bool AppliesToChangeType(ChangeType type) const { return ((int) type & (int) m_changeType) == (int) type; }
            bool AppliesToPropertyChange(ChangeType propertyChangeType, bvector<Utf8String> const& accessStringTokens) const;
            std::vector<RefCountedPtr<PropertyRule>> GetRules() const { return m_rules; }
            ECOBJECTS_EXPORT ClassRule& Append(Policy policy, Utf8CP accessString, ChangeType changeType);
            ECOBJECTS_EXPORT static RefCountedPtr<ClassRule> Create(Policy policy, Utf8CP schemaFullName, Utf8CP customAttributeClassName, ChangeType changeType);
        };
    private:
        std::vector<RefCountedPtr<ClassRule>> m_rules;

        bvector<Utf8String> AccessStringToTokens(Utf8CP accessString) const;

    public:
        CustomAttributeValidator() {}
        ~CustomAttributeValidator() {}
        ECOBJECTS_EXPORT ClassRule& Append(Policy policy, Utf8CP schemaFullName, Utf8CP customAttributeClassName, ChangeType changeType = ChangeType::All);
        ECOBJECTS_EXPORT Policy Validate(CustomAttributeChange const& change) const;
    };

END_BENTLEY_ECOBJECT_NAMESPACE

