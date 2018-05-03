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
#include <ECObjects/ECObjects.h>

//__BENTLEY_INTERNAL_ONLY__
BEGIN_BENTLEY_ECOBJECT_NAMESPACE

#define INDENT_SIZE 3

struct SchemaChange;
struct ClassChange;
struct ECEnumerationChange;
struct StringChange;
struct ECPropertyChange;
struct ECEnumeratorChange;
struct ECPropertyValueChange;
struct ECObjectChange;
struct ClassTypeChange;
struct KindOfQuantityChange;
struct PropertyCategoryChange;
struct PhenomenonChange;
struct UnitSystemChange;
struct UnitChange;
struct FormatChange;

//=======================================================================================
// @bsienum                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
enum class ChangeType
    {
    Deleted = 1, //This need to be none zero base
    Modified = 2,
    New =3
    };

//=======================================================================================
// @bsienum                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
enum class SystemId
    {
    None,
    Alias,
    Array,
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
    Navigation,
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
    References,
    Reference,
    Relationship,
    RelationshipName,
    RoleLabel,
    RoundingFactor,
    Schema,
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
struct Binary final
    {
private:
    void* m_buff;
    size_t m_len;

    BentleyStatus Resize(size_t len);
    BentleyStatus Free();
    BentleyStatus Assign(void* buff = nullptr, size_t len = 0);

public:
    Binary();
    ~Binary();
    Binary(Binary const& rhs);
    Binary(Binary&& rhs);
    Binary& operator=(Binary const& rhs);
    Binary& operator=(Binary&& rhs);
    bool operator==(Binary const& rhs) const { return Compare(rhs) == 0; }
    int Compare(Binary const& rhs) const;
    void* GetPointerP() { return m_buff; }
    void const* GetPointer() const { return m_buff; }
    size_t Size() const { return m_len; }
    bool Empty() const { return m_len == 0; }
    void CopyTo(ECN::ECValueR value) const;
    BentleyStatus CopyFrom(ECN::ECValueCR value);
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

        virtual void _WriteToString(Utf8StringR str, int currentIndex, int indentSize) const = 0;
        virtual bool _IsEmpty() const = 0;
        virtual void _Optimize() {}

    protected:
        ECChange(ChangeType changeType, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : m_systemId(systemId), m_customId(customId), m_changeType(changeType), m_parent(parent)
            {}

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
        bool IsEmpty() const { return _IsEmpty(); }
        bool IsValid() const { return !IsEmpty(); }
        void Optimize() { _Optimize(); }
        Status GetStatus() { return m_status; }
        void SetStatus(Status status) { m_status = status; }
        void WriteToString(Utf8StringR str, int initIndex = 0, int indentSize = INDENT_SIZE) const { _WriteToString(str, initIndex, indentSize); }
        Utf8String String() const { Utf8String str;  WriteToString(str); return str; }
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
struct ECObjectChange : ECChange
    {
    private:
        bmap<Utf8CP, ECChangePtr, CompareUtf8> m_changes;

        ECOBJECTS_EXPORT void _WriteToString(Utf8StringR str, int currentIndex, int indentSize) const override;
        ECOBJECTS_EXPORT bool _IsEmpty() const override;
        ECOBJECTS_EXPORT void _Optimize() override;

    protected:
        template<typename T>
        T& Get(SystemId systemId)
            {
            static_assert(std::is_base_of<ECChange, T>::value, "T not derived from ECChange");
            Utf8CP id = SystemIdToString(systemId);
            auto itor = m_changes.find(id);
            if (itor != m_changes.end())
                return *(static_cast<T*>(itor->second.get()));

            ECChangePtr changePtr = new T(GetChangeType(), systemId, this, nullptr);
            ECChange* changeP = changePtr.get();
            m_changes[changePtr->GetId()] = changePtr;
            return *(static_cast<T*>(changeP));
            }


    public:
        ECObjectChange(ChangeType state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            :ECChange(state, systemId, parent, customId)
            {}

        virtual ~ECObjectChange() {}
        size_t ChangesCount() const 
            { 
            size_t count = 0;
            for (auto const& kvPair : m_changes)
                {
                if (kvPair.second->IsValid())
                    count++;
                }

            return count; 
            }
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
template<typename T>
struct ECChangeArray : ECChange
    {
    private:
        SystemId m_elementType;
        bvector<ECChangePtr> m_changes;

        //---------------------------------------------------------------------------------------
        // @bsimethod                                                    Affan.Khan  03/2016
        //+---------------+---------------+---------------+---------------+---------------+------
        void _WriteToString(Utf8StringR str, int currentIndex, int indentSize) const override
            {
            AppendBegin(str, *this, currentIndex);
            AppendEnd(str);
            for (ECChangePtr const& change : m_changes)
                {
                change->WriteToString(str, currentIndex + indentSize, indentSize);
                }
            }

        //---------------------------------------------------------------------------------------
        // @bsimethod                                                    Affan.Khan  03/2016
        //+---------------+---------------+---------------+---------------+---------------+------
        bool _IsEmpty() const override
            {
            for (ECChangePtr const& change : m_changes)
                {
                if (!change->IsEmpty())
                    return false;
                }

            return true;
            }

        //---------------------------------------------------------------------------------------
        // @bsimethod                                                    Affan.Khan  03/2016
        //+---------------+---------------+---------------+---------------+---------------+------
        void _Optimize() override
            {
            auto itor = m_changes.begin();
            while (itor != m_changes.end())
                {
                (*itor)->Optimize();
                if ((*itor)->IsEmpty())
                    itor = m_changes.erase(itor);
                else
                    ++itor;
                }
            }
    public:
        ECChangeArray(ChangeType state, SystemId systemId, ECChange const* parent, Utf8CP customId, SystemId elementId)
            :ECChange(state, systemId, parent, customId), m_elementType(elementId)
            {
            static_assert(std::is_base_of<ECChange, T>::value, "T not derived from ECChange");
            }

        virtual ~ECChangeArray() {}

        //---------------------------------------------------------------------------------------
        // @bsimethod                                                    Affan.Khan  03/2016
        //+---------------+---------------+---------------+---------------+---------------+------
        SystemId GetElementType() const { return m_elementType; }

        T& Add(ChangeType state, Utf8CP customId = nullptr)
            {
            ECChangePtr changePtr = new T(state, m_elementType, this, customId);
            T* changeP = static_cast<T*>(changePtr.get());
            m_changes.push_back(changePtr);
            return *changeP;
            }

        T& operator[](size_t index) { return static_cast<T&>(*m_changes[index]); }
        T* Find(Utf8CP customId)
            {
            for (auto& v : m_changes)
                if (strcmp(v->GetId(), customId) == 0)
                    return static_cast<T*>(v.get());

            return nullptr;
            }
        //---------------------------------------------------------------------------------------
        // @bsimethod                                                    Affan.Khan  03/2016
        //+---------------+---------------+---------------+---------------+---------------+------
        size_t Count() const { return m_changes.size(); }

        //---------------------------------------------------------------------------------------
        // @bsimethod                                                    Affan.Khan  03/2016
        //+---------------+---------------+---------------+---------------+---------------+------
        bool Empty() const { return m_changes.empty(); }

        //---------------------------------------------------------------------------------------
        // @bsimethod                                                    Affan.Khan  03/2016
        //+---------------+---------------+---------------+---------------+---------------+------
        void Erase(size_t index) { m_changes.erase(m_changes.begin() + index); }
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct SchemaChanges final: ECChangeArray<SchemaChange>
    {
    public:
        SchemaChanges() : ECChangeArray<SchemaChange>(ChangeType::Modified, SystemId::Schemas, nullptr, nullptr, SystemId::Schema) {}
        SchemaChanges(ChangeType type, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECChangeArray<SchemaChange>(type, SystemId::Schemas, parent, customId, SystemId::Schema)
            {
            BeAssert(systemId == GetSystemId());
            }
        ~SchemaChanges(){}
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct ClassChanges final: ECChangeArray<ClassChange>
    {
    public:
        ClassChanges(ChangeType type, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECChangeArray<ClassChange>(type, SystemId::Classes, parent, customId, SystemId::Class)
            {
            BeAssert(systemId == GetSystemId());
            }

        ~ClassChanges() {}
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct ECEnumerationChanges final: ECChangeArray<ECEnumerationChange>
    {
    public:
        ECEnumerationChanges(ChangeType type, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECChangeArray<ECEnumerationChange>(type, SystemId::Enumerations, parent, customId, SystemId::Enumeration)
            {
            BeAssert(systemId == GetSystemId());
            }
        ~ECEnumerationChanges() {}
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct KindOfQuantityChanges final: ECChangeArray<KindOfQuantityChange>
    {
    public:
        KindOfQuantityChanges(ChangeType type, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECChangeArray<KindOfQuantityChange>(type, SystemId::KindOfQuantities, parent, customId, SystemId::KindOfQuantity)
            {
            BeAssert(systemId == GetSystemId());
            }
        ~KindOfQuantityChanges() {}
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle       06/2017
//+===============+===============+===============+===============+===============+======
struct PropertyCategoryChanges final : ECChangeArray<PropertyCategoryChange>
    {
    public:
        PropertyCategoryChanges(ChangeType type, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECChangeArray<PropertyCategoryChange>(type, SystemId::PropertyCategories, parent, customId, SystemId::PropertyCategory)
            {
            BeAssert(systemId == GetSystemId());
            }
        ~PropertyCategoryChanges() {}
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle       02/2018
//+===============+===============+===============+===============+===============+======
struct PhenomenonChanges final : ECChangeArray<PhenomenonChange>
    {
    public:
        PhenomenonChanges(ChangeType type, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECChangeArray<PhenomenonChange>(type, SystemId::Phenomena, parent, customId, SystemId::Phenomenon)
            {
            BeAssert(systemId == GetSystemId());
            }
        ~PhenomenonChanges() {}
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle       02/2018
//+===============+===============+===============+===============+===============+======
struct UnitSystemChanges final : ECChangeArray<UnitSystemChange>
    {
    public:
        UnitSystemChanges(ChangeType type, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECChangeArray<UnitSystemChange>(type, SystemId::UnitSystems, parent, customId, SystemId::UnitSystem)
            {
            BeAssert(systemId == GetSystemId());
            }
        ~UnitSystemChanges() {}
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle       02/2018
//+===============+===============+===============+===============+===============+======
struct UnitChanges final : ECChangeArray<UnitChange>
    {
    public:
        UnitChanges(ChangeType type, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECChangeArray<UnitChange>(type, SystemId::Units, parent, customId, SystemId::Unit)
            {
            BeAssert(systemId == GetSystemId());
            }
        ~UnitChanges() {}
    };

//=======================================================================================
// @bsiclass                                              Kyle.Abramowtiz         04/2018
//+===============+===============+===============+===============+===============+======
struct FormatChanges final : ECChangeArray<FormatChange>
    {
    public:
        FormatChanges(ChangeType type, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECChangeArray<FormatChange>(type, SystemId::Formats, parent, customId, SystemId::Format)
            {
            BeAssert(systemId == GetSystemId());
            }
        ~FormatChanges() {}
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct CustomAttributeChange final : ECChangeArray<ECPropertyValueChange>
    {
    public:
        CustomAttributeChange(ChangeType type, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECChangeArray<ECPropertyValueChange>(type, SystemId::CustomAttribute, parent, customId, SystemId::PropertyValue)
            {
            BeAssert(systemId == GetSystemId());
            }
        ~CustomAttributeChange() {}
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct CustomAttributeChanges final : ECChangeArray<CustomAttributeChange>
    {
    public:
        CustomAttributeChanges(ChangeType type, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECChangeArray<CustomAttributeChange>(type, SystemId::CustomAttributes, parent, customId, SystemId::CustomAttribute)
            {
            BeAssert(systemId == GetSystemId());
            }
        ~CustomAttributeChanges() {}
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct ECPropertyChanges final: ECChangeArray<ECPropertyChange>
    {
    public:
        ECPropertyChanges(ChangeType type, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECChangeArray<ECPropertyChange>(type, SystemId::Properties, parent, customId, SystemId::Property)
            {
            BeAssert(systemId == GetSystemId());
            }
        ~ECPropertyChanges() {}
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct ECRelationshipConstraintClassChanges final: ECChangeArray<StringChange>
    {
    public:
        ECRelationshipConstraintClassChanges(ChangeType type, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECChangeArray<StringChange>(type, SystemId::ConstraintClasses, parent, customId, SystemId::ConstraintClass)
            {
            BeAssert(systemId == GetSystemId());
            }
        ~ECRelationshipConstraintClassChanges() {}
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct ECEnumeratorChanges final: ECChangeArray<ECEnumeratorChange>
    {
    public:
        ECEnumeratorChanges(ChangeType type, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECChangeArray<ECEnumeratorChange>(type, SystemId::Enumerators, parent, customId, SystemId::Enumerator)
            {
            BeAssert(systemId == GetSystemId());
            }
        ~ECEnumeratorChanges() {}
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct StringChanges final: ECChangeArray<StringChange>
    {
    public:
        StringChanges(ChangeType type, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECChangeArray<StringChange>(type, systemId, parent, customId, SystemId::String)
            {}
        ~StringChanges() {}
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct BaseClassChanges final : ECChangeArray<StringChange>
    {
    public:
        BaseClassChanges(ChangeType type, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECChangeArray<StringChange>(type, systemId, parent, customId, SystemId::BaseClass)
            {}
        ~BaseClassChanges() {}
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct ReferenceChanges final: ECChangeArray<StringChange>
    {
    public:
        ReferenceChanges(ChangeType type, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECChangeArray<StringChange>(type, systemId, parent, customId, SystemId::Reference)
            {}
        ~ReferenceChanges() {}
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle            05/2018
//+===============+===============+===============+===============+===============+======
struct Stringifier final
    {
    private:
        Stringifier() = delete;
        ~Stringifier() = delete;

    public:
        static Utf8String ToString(Utf8StringCR val) { return val; }
        static Utf8String ToString(bool val) { return val ? "true" : "false"; }
        static Utf8String ToString(uint32_t val) { return Utf8PrintfString("%" PRIu32, val); }
        static Utf8String ToString(int32_t val) { return Utf8PrintfString("%" PRIi32, val); }
        static Utf8String ToString(int64_t val) { return Utf8PrintfString("%" PRIi64, val); }
        static Utf8String ToString(double val) { return Utf8PrintfString("%.17g", val); }
        static Utf8String ToString(DateTime const& val) { return val.ToString(); }
        static Utf8String ToString(Binary const& val)
            {
            Utf8String str;
            Base64Utilities::Encode(str, (Byte const*) val.GetPointer(), val.Size());
            return str;
            }
        static Utf8String ToString(DPoint2d const& val) { return Utf8PrintfString("(%.17g, %.17g)", val.x, val.y); }
        static Utf8String ToString(DPoint3d const& val) { return Utf8PrintfString("(%.17g, %.17g,  %.17g)", val.x, val.y, val.z); }
        static Utf8String ToString(ECN::ECValue const& val) { return val.ToString(); }
        static Utf8String ToString(ECN::StrengthType val)
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

        static Utf8String ToString(ECN::ECRelatedInstanceDirection val)
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

        static Utf8String ToString(ECN::ECClassModifier val)
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
        static Utf8String ToString(ECN::ECClassType val)
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
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
template<typename T>
struct ECPrimitiveChange : ECChange
    {
    private:
        Nullable<T> m_old;
        Nullable<T> m_new;

        bool _IsEmpty() const override { return m_old == m_new; }

        void _WriteToString(Utf8StringR str, int currentIndex, int indentSize) const override
            {
            AppendBegin(str, *this, currentIndex);
            str.append(": ").append(ToString());
            AppendEnd(str);
            }

    public:
        ECPrimitiveChange(ChangeType state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECChange(state, systemId, parent, customId)
            {}

        virtual ~ECPrimitiveChange() {}
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
                str.append(Stringifier::ToString(GetOld().Value()));
            if (GetChangeType() == ChangeType::New)
                str.append(Stringifier::ToString(GetNew().Value()));
            if (GetChangeType() == ChangeType::Modified)
                str.append(Stringifier::ToString(GetOld().Value())).append(" -> ").append(Stringifier::ToString(GetNew().Value()));
            return str;
            }

    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct StringChange final: ECPrimitiveChange<Utf8String>
    {
    public:
        StringChange(ChangeType state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECPrimitiveChange<Utf8String>(state, systemId, parent, customId)
            {}
        ~StringChange() {}
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct BooleanChange final: ECPrimitiveChange<bool>
    {
    public:
        BooleanChange(ChangeType state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECPrimitiveChange<bool>(state, systemId, parent, customId)
            {}
        ~BooleanChange() {}

    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct UInt32Change final: ECPrimitiveChange<uint32_t>
    {
    public:
        UInt32Change(ChangeType state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECPrimitiveChange<uint32_t>(state, systemId, parent, customId)
            {}
        ~UInt32Change() {}
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct Int32Change final: ECPrimitiveChange<int32_t>
    {
    public:
        Int32Change(ChangeType state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECPrimitiveChange<int32_t>(state, systemId, parent, customId)
            {}
        ~Int32Change() {}
    };
//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct DoubleChange final: ECPrimitiveChange<double>
    {
    public:
        DoubleChange(ChangeType state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECPrimitiveChange<double>(state, systemId, parent, customId)
            {}
        ~DoubleChange() {}
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct DateTimeChange final: ECPrimitiveChange<DateTime>
    {
    public:
        DateTimeChange(ChangeType state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECPrimitiveChange<DateTime>(state, systemId, parent, customId)
            {}
        ~DateTimeChange() {}
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct BinaryChange final: ECPrimitiveChange<Binary>
    {
    public:
        BinaryChange(ChangeType state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECPrimitiveChange<Binary>(state, systemId, parent, customId)
            {}
        ~BinaryChange() {}
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct Point2dChange final: ECPrimitiveChange<DPoint2d>
    {
    public:
        Point2dChange(ChangeType state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECPrimitiveChange<DPoint2d>(state, systemId, parent, customId)
            {}
        ~Point2dChange() {}
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct Point3dChange final: ECPrimitiveChange<DPoint3d>
    {
    public:
        Point3dChange(ChangeType state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECPrimitiveChange<DPoint3d>(state, systemId, parent, customId)
            {}
        ~Point3dChange() {}
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct Int64Change final: ECPrimitiveChange<int64_t>
    {
    public:
        Int64Change(ChangeType state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECPrimitiveChange<int64_t>(state, systemId, parent, customId)
            {}
        ~Int64Change() {}
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct StrengthTypeChange final : ECPrimitiveChange<ECN::StrengthType>
    {
    public:
        StrengthTypeChange(ChangeType state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECPrimitiveChange<ECN::StrengthType>(state, systemId, parent, customId)
            {}
        ~StrengthTypeChange() {}
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct StrengthDirectionChange final: ECPrimitiveChange<ECN::ECRelatedInstanceDirection>
    {
    public:
        StrengthDirectionChange(ChangeType state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECPrimitiveChange<ECN::ECRelatedInstanceDirection>(state, systemId, parent, customId)
            {}
        ~StrengthDirectionChange() {}
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct ClassModifierChange final :ECPrimitiveChange<ECN::ECClassModifier>
    {
    public:
        ClassModifierChange(ChangeType state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECPrimitiveChange<ECN::ECClassModifier>(state, systemId, parent, customId)
            {}
        ~ClassModifierChange() {}
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct ClassTypeChange final :ECPrimitiveChange<ECN::ECClassType>
    {
    public:
        ClassTypeChange(ChangeType state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECPrimitiveChange<ECN::ECClassType>(state, systemId, parent, customId)
            {}
        ~ClassTypeChange() {}
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct SchemaChange final : ECObjectChange
    {
    public:
        SchemaChange(ChangeType state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECObjectChange(state, SystemId::Schema, parent, customId)
            {
            BeAssert(systemId == GetSystemId());
            }
        ~SchemaChange(){}

        StringChange& Name() { return Get<StringChange>(SystemId::Name); }
        StringChange& Alias() { return Get<StringChange>(SystemId::Alias); }
        StringChange& DisplayLabel() { return Get<StringChange>(SystemId::DisplayLabel); }
        StringChange& Description() { return Get<StringChange>(SystemId::Description); }
        UInt32Change& VersionRead() { return Get<UInt32Change>(SystemId::VersionRead); }
        UInt32Change& VersionMinor() { return Get<UInt32Change>(SystemId::VersionMinor); }
        UInt32Change& VersionWrite() { return Get<UInt32Change>(SystemId::VersionWrite); }
        UInt32Change& ECVersion() { return Get<UInt32Change>(SystemId::ECVersion); }
        UInt32Change& OriginalECXmlVersionMajor() { return Get<UInt32Change>(SystemId::OriginalECXmlVersionMajor); }
        UInt32Change& OriginalECXmlVersionMinor() { return Get<UInt32Change>(SystemId::OriginalECXmlVersionMinor); }

        ReferenceChanges& References() { return Get<ReferenceChanges>(SystemId::References); }
        ClassChanges& Classes() { return Get<ClassChanges>(SystemId::Classes); }
        ECEnumerationChanges& Enumerations() { return Get<ECEnumerationChanges>(SystemId::Enumerations); }
        CustomAttributeChanges& CustomAttributes() { return Get<CustomAttributeChanges>(SystemId::CustomAttributes); }
        KindOfQuantityChanges& KindOfQuantities() { return Get<KindOfQuantityChanges>(SystemId::KindOfQuantities); }
        PropertyCategoryChanges& PropertyCategories() { return Get<PropertyCategoryChanges>(SystemId::PropertyCategories); }
        PhenomenonChanges& Phenomena() { return Get<PhenomenonChanges>(SystemId::Phenomena); }
        UnitSystemChanges& UnitSystems() { return Get<UnitSystemChanges>(SystemId::UnitSystems); }
        UnitChanges& Units() { return Get<UnitChanges>(SystemId::Units); }
        FormatChanges& Formats() {return Get<FormatChanges>(SystemId::Formats);}
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct ECEnumeratorChange final : ECObjectChange
    {
    public:
        ECEnumeratorChange(ChangeType state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr) : ECObjectChange(state, SystemId::Enumerator, parent, customId) { BeAssert(systemId == GetSystemId()); }
        ~ECEnumeratorChange() {}
        StringChange& Name() {return Get<StringChange>(SystemId::Name); }
        StringChange& DisplayLabel() { return Get<StringChange>(SystemId::DisplayLabel); }
        StringChange& Description() { return Get<StringChange>(SystemId::Description); }
        StringChange& String() { return Get<StringChange>(SystemId::String); }
        Int32Change& Integer() { return Get<Int32Change>(SystemId::Integer); }
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct ECEnumerationChange final :ECObjectChange
    {
    public:
        ECEnumerationChange(ChangeType state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr) : ECObjectChange(state, SystemId::Enumeration, parent, customId) { BeAssert(systemId == GetSystemId()); }
        ~ECEnumerationChange() {}
        StringChange& Name() { return Get<StringChange>(SystemId::Name); }
        StringChange& DisplayLabel() { return Get<StringChange>(SystemId::DisplayLabel); }
        StringChange& Description() { return Get<StringChange>(SystemId::Description); }
        StringChange& TypeName() { return Get<StringChange>(SystemId::TypeName); }
        BooleanChange& IsStrict() { return Get<BooleanChange>(SystemId::IsStrict); }
        ECEnumeratorChanges& Enumerators() { return Get<ECEnumeratorChanges>(SystemId::Enumerators); }
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct ECPropertyValueChange final : ECChange
    {
    private:
        std::unique_ptr<ECChange> m_value;
        ECN::PrimitiveType m_type;

        void _WriteToString(Utf8StringR str, int currentIndex, int indentSize) const override;
        bool _IsEmpty() const override;
        void _Optimize() override;
        BentleyStatus InitValue(ECN::PrimitiveType type);

        template<typename T>
        class Converter
            {
            template<bool cond, typename U>
            using resolvedType = Nullable<typename std::enable_if< cond, U >::type>;

            public:
                template< typename U = T >
                static resolvedType< std::is_same<U, Binary>::value, U > Copy(ECN::ECValueCR v)
                    {
                    if (v.IsNull()) return Nullable<T>();
                    Nullable<T> t;
                    Binary bin;
                    bin.CopyFrom(v);
                    t = bin;
                    return t;
                    }

                template< typename U = T >
                static resolvedType<std::is_same<U, int32_t>::value, U > Copy(ECN::ECValueCR v)
                    {
                    if (v.IsNull()) return Nullable<T>();
                    return v.GetInteger();
                    }
                template< typename U = T >
                static resolvedType< std::is_same<U, int64_t>::value, U > Copy(ECN::ECValueCR v)
                    {
                    if (v.IsNull()) return Nullable<T>();
                    return v.GetLong();
                    }
                template< typename U = T >
                static resolvedType< std::is_same<U, DateTime>::value, U > Copy(ECN::ECValueCR v)
                    {
                    if (v.IsNull()) return Nullable<T>();
                    return v.GetDateTime();
                    }
                template< typename U = T >
                static resolvedType< std::is_same<U, Utf8String>::value, U > Copy(ECN::ECValueCR v)
                    {
                    if (v.IsNull()) return Nullable<T>();
                    return Nullable<T>(v.GetUtf8CP());
                    }
                template< typename U = T >
                static resolvedType< std::is_same<U, DPoint2d>::value, U > Copy(ECN::ECValueCR v)
                    {
                    if (v.IsNull()) return Nullable<T>();
                    return v.GetPoint2d();
                    }
                template< typename U = T >
                static resolvedType< std::is_same<U, DPoint3d>::value, U > Copy(ECN::ECValueCR v)
                    {
                    if (v.IsNull()) return Nullable<T>();
                    return v.GetPoint3d();
                    }
                template< typename U = T >
                static resolvedType< std::is_same<U, double>::value, U > Copy(ECN::ECValueCR v)
                    {
                    if (v.IsNull()) return Nullable<T>();
                    return v.GetDouble();
                    }
                template< typename U = T >
                static resolvedType< std::is_same<U, bool>::value, U > Copy(ECN::ECValueCR v)
                    {
                    if (v.IsNull()) return Nullable<T>();
                    return v.GetBoolean();
                    }
            };

    public:
        ECPropertyValueChange(ChangeType state, SystemId systemId = SystemId::PropertyValue, ECChange const* parent = nullptr, Utf8CP accessString = nullptr);
        ~ECPropertyValueChange() {}
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
struct KindOfQuantityChange final :ECObjectChange
    {
    public:
        KindOfQuantityChange(ChangeType state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr) : ECObjectChange(state, SystemId::KindOfQuantity, parent, customId) { BeAssert(systemId == GetSystemId()); }
        ~KindOfQuantityChange() {}
        StringChange& Name() { return Get<StringChange>(SystemId::Name); }
        StringChange& DisplayLabel() { return Get<StringChange>(SystemId::DisplayLabel); }
        StringChange& Description() { return Get<StringChange>(SystemId::Description); }
        StringChange& PersistenceUnit() { return Get<StringChange>(SystemId::KoqPersistenceUnit); }
        DoubleChange& RelativeError() { return Get<DoubleChange>(SystemId::KoqRelativeError); }
        StringChanges& PresentationFormats() { return Get<StringChanges>(SystemId::KoqPresentationFormats); }
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle       06/2017
//+===============+===============+===============+===============+===============+======
struct PropertyCategoryChange final : ECObjectChange
    {
    public:
        PropertyCategoryChange(ChangeType state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr) : ECObjectChange(state, SystemId::PropertyCategory, parent, customId) { BeAssert(systemId == GetSystemId()); }
        ~PropertyCategoryChange() {}
        StringChange& Name() { return Get<StringChange>(SystemId::Name); }
        StringChange& DisplayLabel() { return Get<StringChange>(SystemId::DisplayLabel); }
        StringChange& Description() { return Get<StringChange>(SystemId::Description); }
        UInt32Change& Priority() { return Get<UInt32Change>(SystemId::PropertyCategoryPriority); }
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle       02/2018
//+===============+===============+===============+===============+===============+======
struct PhenomenonChange final : ECObjectChange
    {
    public:
        PhenomenonChange(ChangeType state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr) : ECObjectChange(state, SystemId::Phenomenon, parent, customId) { BeAssert(systemId == GetSystemId()); }
        ~PhenomenonChange() {}
        StringChange& Name() { return Get<StringChange>(SystemId::Name); }
        StringChange& DisplayLabel() { return Get<StringChange>(SystemId::DisplayLabel); }
        StringChange& Description() { return Get<StringChange>(SystemId::Description); }
        StringChange& Definition() { return Get<StringChange>(SystemId::PhenomenonDefinition); }
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle       02/2018
//+===============+===============+===============+===============+===============+======
struct UnitSystemChange final : ECObjectChange
    {
    public:
        UnitSystemChange(ChangeType state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr) : ECObjectChange(state, SystemId::UnitSystem, parent, customId) { BeAssert(systemId == GetSystemId()); }
        ~UnitSystemChange() {}
        StringChange& Name() { return Get<StringChange>(SystemId::Name); }
        StringChange& DisplayLabel() { return Get<StringChange>(SystemId::DisplayLabel); }
        StringChange& Description() { return Get<StringChange>(SystemId::Description); }
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle       02/2018
//+===============+===============+===============+===============+===============+======
struct UnitChange final : ECObjectChange
    {
    public:
        UnitChange(ChangeType state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECObjectChange(state, SystemId::Unit, parent, customId)
            {
            BeAssert(systemId == GetSystemId());
            }
        ~UnitChange() {}
        StringChange& Name() { return Get<StringChange>(SystemId::Name); }
        StringChange& DisplayLabel() { return Get<StringChange>(SystemId::DisplayLabel); }
        StringChange& Description() { return Get<StringChange>(SystemId::Description); }
        StringChange& Definition() { return Get<StringChange>(SystemId::UnitDefinition); }
        StringChange& Phenomenon() { return Get<StringChange>(SystemId::Phenomenon); }
        StringChange& UnitSystem() { return Get<StringChange>(SystemId::UnitSystem); }
        DoubleChange& Numerator() { return Get<DoubleChange>(SystemId::UnitNumerator); }
        DoubleChange& Denominator() { return Get<DoubleChange>(SystemId::UnitDenominator); }
        DoubleChange& Offset() { return Get<DoubleChange>(SystemId::UnitOffset); }
        BooleanChange& IsConstant() { return Get<BooleanChange>(SystemId::UnitIsConstant); }
        StringChange& InvertingUnit() { return Get<StringChange>(SystemId::UnitInvertingUnit); }
    };

//=======================================================================================
// @bsiclass                                                      05/2018
//+===============+===============+===============+===============+===============+======
struct NumericFormatSpecChange final : ECObjectChange
    {
    public:
        NumericFormatSpecChange(ChangeType state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECObjectChange(state, SystemId::NumericFormatSpec, parent, customId)
            {
            BeAssert(systemId == GetSystemId());
            }
        ~NumericFormatSpecChange() {}

        DoubleChange& GetRoundingFactor() { return Get<DoubleChange>(SystemId::RoundingFactor); }
        StringChange& GetPresentationType() { return Get<StringChange>(SystemId::PresentationType); }
        Int32Change& GetDecimalPrecision() { return Get<Int32Change>(SystemId::DecimalPrecision); }
        Int32Change& GetFractionalPrecision() { return Get<Int32Change>(SystemId::FractionalPrecision); }
        UInt32Change& GetMinWidth() { return Get<UInt32Change>(SystemId::MinWidth); }
        StringChange& GetScientificType() { return Get<StringChange>(SystemId::ScientificType); }
        StringChange& GetShowSignOption() { return Get<StringChange>(SystemId::ShowSignOption); }
        StringChange& GetFormatTraits() { return Get<StringChange>(SystemId::FormatTraits); }
        StringChange& GetDecimalSeparator() { return Get<StringChange>(SystemId::DecimalSeparator); }
        StringChange& GetThousandsSeparator() { return Get<StringChange>(SystemId::ThousandSeparator); }
        StringChange& GetUomSeparator() { return Get<StringChange>(SystemId::UomSeparator); }
        StringChange& GetStationSeparator() { return Get<StringChange>(SystemId::StationSeparator); }
        UInt32Change& GetStationOffsetSize() { return Get<UInt32Change>(SystemId::StationOffsetSize); }

        BentleyStatus SetFrom(Formatting::NumericFormatSpecCP oldSpec, Formatting::NumericFormatSpecCP newSpec);
        BentleyStatus SetFrom(Formatting::NumericFormatSpecCR);
    };

//=======================================================================================
// @bsiclass                                                      05/2018
//+===============+===============+===============+===============+===============+======
struct CompositeValueSpecChange final : ECObjectChange
    {
    public:
        CompositeValueSpecChange(ChangeType state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECObjectChange(state, SystemId::CompositeValueSpec, parent, customId)
            {
            BeAssert(systemId == GetSystemId());
            }
        ~CompositeValueSpecChange() {}

        BooleanChange& GetIncludeZero() { return Get<BooleanChange>(SystemId::CompositeIncludeZero); }
        StringChange& GetSpacer() { return Get<StringChange>(SystemId::CompositeSpacer); }
        StringChange& GetMajorUnit() { return Get<StringChange>(SystemId::CompositeMajorUnit); }
        StringChange& GetMajorLabel() { return Get<StringChange>(SystemId::CompositeMajorLabel); }
        StringChange& GetMiddleUnit() { return Get<StringChange>(SystemId::CompositeMiddleUnit); }
        StringChange& GetMiddleLabel() { return Get<StringChange>(SystemId::CompositeMiddleLabel); }
        StringChange& GetMinorUnit() { return Get<StringChange>(SystemId::CompositeMinorUnit); }
        StringChange& GetMinorLabel() { return Get<StringChange>(SystemId::CompositeMinorLabel); }
        StringChange& GetSubUnit() { return Get<StringChange>(SystemId::CompositeSubUnit); }
        StringChange& GetSubLabel() { return Get<StringChange>(SystemId::CompositeSubLabel); }

        BentleyStatus SetFrom(Formatting::CompositeValueSpecCP oldSpec, Formatting::CompositeValueSpecCP newSpec);
        BentleyStatus SetFrom(Formatting::CompositeValueSpecCR);
    };

//=======================================================================================
// @bsiclass                                               Kyle.Abramowitz        04/2018
//+===============+===============+===============+===============+===============+======
struct FormatChange final : ECObjectChange
    {
    public:
        FormatChange(ChangeType state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECObjectChange(state, SystemId::Format, parent, customId)
            {
            BeAssert(systemId == GetSystemId());
            }
        ~FormatChange() {}
        StringChange& GetName() {return Get<StringChange>(SystemId::Name);}
        StringChange& GetDisplayLabel() {return Get<StringChange>(SystemId::DisplayLabel);}
        StringChange& GetDescription() {return Get<StringChange>(SystemId::Description);}
        NumericFormatSpecChange& GetNumericSpec() {return Get<NumericFormatSpecChange>(SystemId::NumericFormatSpec);}
        CompositeValueSpecChange& GetCompositeSpec() { return Get<CompositeValueSpecChange>(SystemId::CompositeValueSpec); }
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct ECRelationshipConstraintChange final :ECObjectChange
    {
    public:
        ECRelationshipConstraintChange(ChangeType state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECObjectChange(state, systemId, parent, customId)
            {
            BeAssert(systemId == SystemId::Source || systemId == SystemId::Target);
            }
        ~ECRelationshipConstraintChange() {}
        StringChange& GetRoleLabel() { return Get<StringChange>(SystemId::RoleLabel); }
        StringChange& GetMultiplicity() { return Get<StringChange>(SystemId::Multiplicity); }
        BooleanChange& IsPolymorphic() { return Get<BooleanChange>(SystemId::IsPolymorphic); }
        ECRelationshipConstraintClassChanges& ConstraintClasses() { return Get<ECRelationshipConstraintClassChanges>(SystemId::ConstraintClasses); }
        CustomAttributeChanges& CustomAttributes() { return Get<CustomAttributeChanges>(SystemId::CustomAttributes); }
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct ECRelationshipChange final :ECObjectChange
    {
    public:
        ECRelationshipChange(ChangeType state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECObjectChange(state, SystemId::Relationship, parent, customId)
            {
            BeAssert(systemId == GetSystemId());
            }
        ~ECRelationshipChange() {}
        StrengthTypeChange& GetStrength() { return Get<StrengthTypeChange>(SystemId::StrengthType); }
        StrengthDirectionChange& GetStrengthDirection() { return Get<StrengthDirectionChange>(SystemId::StrengthDirection); }
        ECRelationshipConstraintChange& GetSource() { return Get<ECRelationshipConstraintChange>(SystemId::Source); }
        ECRelationshipConstraintChange& GetTarget() { return Get<ECRelationshipConstraintChange>(SystemId::Target); }
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct ClassChange final :ECObjectChange
    {
    public:
        ClassChange(ChangeType state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECObjectChange(state, SystemId::Class, parent, customId)
            {
            BeAssert(systemId == GetSystemId());
            }
        ~ClassChange() {}
        StringChange& GetName() { return Get<StringChange>(SystemId::Name); }
        StringChange& GetDisplayLabel() { return Get<StringChange>(SystemId::DisplayLabel); }
        StringChange& GetDescription() { return Get<StringChange>(SystemId::Description); }
        ClassModifierChange& GetClassModifier() { return Get<ClassModifierChange>(SystemId::ClassModifier); }
        ClassTypeChange& ClassType() {return Get<ClassTypeChange>(SystemId::ClassType);}
        BaseClassChanges& BaseClasses() { return Get<BaseClassChanges>(SystemId::BaseClasses); }
        ECPropertyChanges& Properties() { return Get<ECPropertyChanges>(SystemId::Properties); }
        CustomAttributeChanges& CustomAttributes() { return Get<CustomAttributeChanges>(SystemId::CustomAttributes); }
        ECRelationshipChange& GetRelationship() { return Get<ECRelationshipChange>(SystemId::Relationship); }
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct NavigationChange final :ECObjectChange
    {
    public:
        NavigationChange(ChangeType state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECObjectChange(state, SystemId::Navigation, parent, customId)
            {
            BeAssert(systemId == GetSystemId());
            }
        ~NavigationChange() {}
        StrengthDirectionChange& Direction() { return Get<StrengthDirectionChange>(SystemId::Direction); }
        StringChange& GetRelationshipClassName() { return Get<StringChange>(SystemId::RelationshipName); }
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct ArrayChange final :ECObjectChange
    {
    public:
        ArrayChange(ChangeType state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECObjectChange(state, SystemId::Array, parent, customId)
            {
            BeAssert(systemId == GetSystemId());
            }
        ~ArrayChange() {}
        UInt32Change& MinOccurs() { return Get<UInt32Change>(SystemId::MinOccurs); }
        UInt32Change& MaxOccurs() { return Get<UInt32Change>(SystemId::MaxOccurs); }
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct ECPropertyChange final :ECObjectChange
    {
    public:
        ECPropertyChange(ChangeType state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECObjectChange(state, SystemId::Property, parent, customId)
            {
            BeAssert(systemId == GetSystemId());
            }
        ~ECPropertyChange() {}
        StringChange& GetName() { return Get<StringChange>(SystemId::Name); }
        StringChange& GetDisplayLabel() { return Get<StringChange>(SystemId::DisplayLabel); }
        StringChange& GetDescription() { return Get<StringChange>(SystemId::Description); }
        StringChange& GetTypeName() { return Get<StringChange>(SystemId::TypeName); }
        ECPrimitiveChange<ECN::ECValue>& GetMinimumValue() { return Get<ECPrimitiveChange<ECN::ECValue>>(SystemId::MinimumValue); }
        ECPrimitiveChange<ECN::ECValue>& GetMaximumValue() { return Get<ECPrimitiveChange<ECN::ECValue>>(SystemId::MaximumValue); }
        UInt32Change& GetMinimumLength() { return Get<UInt32Change>(SystemId::MinimumLength); }
        UInt32Change& GetMaximumLength() { return Get<UInt32Change>(SystemId::MaximumLength); }
        BooleanChange& IsStruct() { return Get<BooleanChange>(SystemId::IsStrict); }
        BooleanChange& IsStructArray() { return Get<BooleanChange>(SystemId::IsStructArray); }
        BooleanChange& IsPrimitive() { return Get<BooleanChange>(SystemId::IsPrimitive); }
        BooleanChange& IsPrimitiveArray() { return Get<BooleanChange>(SystemId::IsPrimitiveArray); }
        BooleanChange& IsNavigation() { return Get<BooleanChange>(SystemId::IsNavigation); }
        ArrayChange& GetArray() { return Get<ArrayChange>(SystemId::Array); }
        NavigationChange& GetNavigation() { return Get<NavigationChange>(SystemId::Navigation); }
        StringChange& GetExtendedTypeName() { return Get<StringChange>(SystemId::ExtendedTypeName); }
        BooleanChange& IsReadonly() { return Get<BooleanChange>(SystemId::IsReadonly); }
        Int32Change& GetPriority() { return Get<Int32Change>(SystemId::PropertyPriority); }
        CustomAttributeChanges& CustomAttributes() { return Get<CustomAttributeChanges>(SystemId::CustomAttributes); }
        StringChange& GetKindOfQuantity() { return Get<StringChange>(SystemId::KindOfQuantity); }
        StringChange& GetEnumeration() { return Get<StringChange>(SystemId::Enumeration); }
        StringChange& GetCategory() { return Get<StringChange>(SystemId::PropertyCategory); }
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
    BentleyStatus CompareECRelationshipClass(ECRelationshipChange&, ECN::ECRelationshipClassCR, ECN::ECRelationshipClassCR);
    BentleyStatus CompareECRelationshipConstraint(ECRelationshipConstraintChange&, ECN::ECRelationshipConstraintCR, ECN::ECRelationshipConstraintCR);
    BentleyStatus CompareECRelationshipConstraintClasses(ECRelationshipConstraintClassChanges&, ECN::ECRelationshipConstraintClassList const&, ECN::ECRelationshipConstraintClassList const&);
    BentleyStatus CompareECProperty(ECPropertyChange&, ECN::ECPropertyCR, ECN::ECPropertyCR);
    BentleyStatus CompareECProperties(ECPropertyChanges&, ECN::ECPropertyIterableCR, ECN::ECPropertyIterableCR);
    BentleyStatus CompareECClasses(ClassChanges&, ECN::ECClassContainerCR, ECN::ECClassContainerCR);
    BentleyStatus CompareECEnumerations(ECEnumerationChanges&, ECN::ECEnumerationContainerCR, ECN::ECEnumerationContainerCR);
    BentleyStatus CompareCustomAttributes(CustomAttributeChanges&, ECN::IECCustomAttributeContainerCR, ECN::IECCustomAttributeContainerCR);
    BentleyStatus CompareCustomAttribute(CustomAttributeChange&, ECN::IECInstanceCR, ECN::IECInstanceCR);
    BentleyStatus CompareECEnumeration(ECEnumerationChange&, ECN::ECEnumerationCR oldVal, ECN::ECEnumerationCR newVal);
    BentleyStatus CompareECEnumerators(ECEnumeratorChanges&, ECN::EnumeratorIterable const&, ECN::EnumeratorIterable const&);
    BentleyStatus CompareBaseClasses(BaseClassChanges&, ECN::ECBaseClassesList const&, ECN::ECBaseClassesList const&);
    BentleyStatus CompareReferences(ReferenceChanges&, ECN::ECSchemaReferenceListCR, ECN::ECSchemaReferenceListCR);
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
    BentleyStatus AppendECRelationshipClass(ECRelationshipChange&, ECN::ECRelationshipClassCR);
    BentleyStatus AppendECRelationshipConstraintClasses(ECRelationshipConstraintClassChanges&, ECN::ECRelationshipConstraintClassList const&);
    BentleyStatus AppendECRelationshipConstraint(ECRelationshipConstraintChange&, ECN::ECRelationshipConstraintCR);
    BentleyStatus AppendECEnumeration(ECEnumerationChange&, ECN::ECEnumerationCR);
    BentleyStatus AppendECProperty(ECPropertyChange&, ECN::ECPropertyCR);
    BentleyStatus AppendCustomAttributes(CustomAttributeChanges&, ECN::IECCustomAttributeContainerCR);
    BentleyStatus AppendCustomAttribute(CustomAttributeChange&, ECN::IECInstanceCR);
    BentleyStatus AppendBaseClasses(BaseClassChanges&, ECN::ECBaseClassesList const&);
    BentleyStatus AppendReferences(ReferenceChanges&, ECN::ECSchemaReferenceListCR);
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

    ECOBJECTS_EXPORT BentleyStatus Compare(SchemaChanges&, bvector<ECN::ECSchemaCP> const& existingSet, bvector<ECN::ECSchemaCP> const& newSet, Options options = Options());
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

