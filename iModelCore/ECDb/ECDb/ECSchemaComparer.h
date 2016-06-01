/*--------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSchemaComparer.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+-------------------------------------------------------------------------------------*/
#pragma once
#include "ECDbInternalTypes.h"
#include <Bentley/RefCounted.h>
#include "Nullable.h"
USING_NAMESPACE_BENTLEY_EC
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//#define KIND_OF_QUANTITY_SUPPORT
#define INDENT_SIZE 3

struct ECSchemaChange;
struct ECClassChange;
struct ECEnumerationChange;
//struct KindOfQuantityChange;
struct StringChange;
struct ECPropertyChange;
struct ECRelationshipConstraintClassChange;
struct ECEnumeratorChange;
struct ECPropertyValueChange;
struct ECObjectChange;
struct ClassTypeChange;
#ifdef KIND_OF_QUANTITY_SUPPORT
struct KindOfQuantityChange;
#endif
//=======================================================================================
// @bsienum                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
enum class ChangeState
    {
    Deleted = 1, //This need to be none zero base
    Modified =2 ,
    New =3,
    };

//=======================================================================================
// @bsienum                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
enum class ValueId
    {
    Deleted, New
    };

//=======================================================================================
// @bsienum                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
enum class SystemId
    {
    None,
    AlternativePresentationUnitList,
    Array,
    BaseClass,
    BaseClasses,
    Cardinality,
    Class,
    Classes,
    ClassFullName,
    ClassModifier,
    ClassType,
    ConstantKey,
    Constraint,
    ConstraintClass,
    ConstraintClasses,
    CustomAttributes,
    DefaultPresentationUnit,
    Description,
    Direction,
    DisplayLabel,
    Enumeration,
    Enumerations,
    Enumerator,
    Enumerators,
    ExtendedTypeName,
    Instance,
    Instances,
    Integer,
    IsPolymorphic,
    IsReadonly,
    IsStrict,
    IsStruct,
    IsStructArray,
    IsPrimitive,
    IsPrimitiveArray,
    IsNavigation,
    KeyProperties,
    KeyProperty,
    KindOfQuantities,
    KindOfQuantity,
    MaximumValue,
    MaxOccurs,
    MinimumValue,
    MinOccurs,
    Name,
    NamespacePrefix,
    Navigation,
    PersistenceUnit,
    Precision,
    Properties,
    Property,
    PropertyType,
    PropertyValue,
    PropertyValues,
    References,
    Reference,
    Relationship,
    RelationshipName,
    RoleLabel,
    Schema,
    Schemas,
    Source,
    StrengthDirection,
    StrengthType,
    String,
    Target,
    TypeName,
    VersionMajor,
    VersionMinor,
    VersionWrite
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct Binary
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
        ChangeState m_state;
        Status m_status;
        ECChange const* m_parent;

        virtual void _WriteToString(Utf8StringR str, int currentIndex, int indentSize) const = 0;
        virtual bool _IsEmpty() const = 0;
        virtual void _Optimize() {}

        static SystemId StringToSystemId(Utf8CP);

    protected:
        ECChange(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr);

        static void AppendBegin(Utf8StringR str, ECChange const& change, int currentIndex);
        static void AppendEnd(Utf8StringR str) { str.append("\r\n"); }

        static Utf8CP SystemIdToString(SystemId);

    public:
        virtual ~ECChange() {};
        SystemId GetSystemId() const { return m_systemId; }
        bool HasCustomId() const { return !m_customId.empty(); }
        Utf8CP GetId() const;
        ChangeState GetState() const { return m_state; }
        ECChange const* GetParent() const { return m_parent; }
        bool IsEmpty() const { return _IsEmpty(); }
        bool IsValid() const { return !IsEmpty(); }
        void Optimize() { _Optimize(); }
        Status GetStatus() { return m_status; }
        void SetStatus(Status status) { m_status = status; }
        void WriteToString(Utf8StringR str, int initIndex = 0, int indentSize = INDENT_SIZE) const { _WriteToString(str, initIndex, indentSize); }
        Utf8String GetString() const 
            {
            Utf8String str;
            WriteToString(str);
            return str;
            }
    };

typedef RefCountedPtr<ECChange> ECChangePtr;

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct ECObjectChange : ECChange
    {
    private:
        bmap<Utf8CP, ECChangePtr, CompareUtf8> m_changes;

        virtual void _WriteToString(Utf8StringR str, int currentIndex, int indentSize) const override;
        virtual bool _IsEmpty() const override;
        virtual void _Optimize() override;
        
    protected:
        template<typename T>
        T& Get(SystemId systemId);
      
    public:
        ECObjectChange(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            :ECChange(state, systemId, parent, customId)
            {}
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
        virtual void _WriteToString(Utf8StringR str, int currentIndex, int indentSize) const override
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
        virtual bool _IsEmpty() const override
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
        virtual void _Optimize() override
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
        ECChangeArray(ChangeState state, SystemId systemId, ECChange const* parent, Utf8CP customId, SystemId elementId)
            :ECChange(state, systemId, parent, customId), m_elementType(elementId)
            {
            static_assert(std::is_base_of<ECChange, T>::value, "T not derived from ECChange");
            }        
        virtual ~ECChangeArray() {}

        //---------------------------------------------------------------------------------------
        // @bsimethod                                                    Affan.Khan  03/2016
        //+---------------+---------------+---------------+---------------+---------------+------
        SystemId GetElementType() const { return m_elementType; }

        //---------------------------------------------------------------------------------------
        // @bsimethod                                                    Affan.Khan  03/2016
        //+---------------+---------------+---------------+---------------+---------------+------
        T& Add(ChangeState state, Utf8CP customId = nullptr)
            {
            ECChangePtr changePtr = new T(state, m_elementType, this, customId);
            T* changeP = static_cast<T*>(changePtr.get());
            m_changes.push_back(changePtr);
            return *changeP;
            }

        //---------------------------------------------------------------------------------------
        // @bsimethod                                                    Affan.Khan  03/2016
        //+---------------+---------------+---------------+---------------+---------------+------
        T& At(size_t index)
            {
            return static_cast<T&>(*m_changes[index]);
            }

        //---------------------------------------------------------------------------------------
        // @bsimethod                                                    Affan.Khan  03/2016
        //+---------------+---------------+---------------+---------------+---------------+------
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
        void Erase(size_t index)
            {
            m_changes.erase(m_changes.begin() + index);
            }
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct ECSchemaChanges : ECChangeArray<ECSchemaChange>
    {
    public:
        ECSchemaChanges()
            : ECChangeArray<ECSchemaChange>(ChangeState::Modified, SystemId::Schemas, nullptr, nullptr, SystemId::Schema)
            {}
        ECSchemaChanges(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECChangeArray<ECSchemaChange>(state, SystemId::Schemas, parent, customId, SystemId::Schema)
            {
            BeAssert(systemId == GetSystemId());
            }
        virtual ~ECSchemaChanges(){}
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct ECClassChanges : ECChangeArray<ECClassChange>
    {
    public:
        ECClassChanges(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECChangeArray<ECClassChange>(state, SystemId::Classes, parent, customId, SystemId::Class)
            {
            BeAssert(systemId == GetSystemId());
            }
        virtual ~ECClassChanges() {}
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct ECEnumerationChanges : ECChangeArray<ECEnumerationChange>
    {
    public:
        ECEnumerationChanges(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECChangeArray<ECEnumerationChange>(state, SystemId::Enumerations, parent, customId, SystemId::Enumeration)
            {
            BeAssert(systemId == GetSystemId());
            }
        virtual ~ECEnumerationChanges() {}
    }; 

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
#ifdef KIND_OF_QUANTITY_SUPPORT
struct ECKindOfQuantityChanges : ECChangeArray<KindOfQuantityChange>
    {
    public:
        ECKindOfQuantityChanges(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECChangeArray<KindOfQuantityChange>(state, SystemId::KindOfQuantities, parent, customId, SystemId::KindOfQuantity)
            {
            BeAssert(systemId == GetSystemId());
            }
        virtual ~ECKindOfQuantityChanges() {}
    };
#endif

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct ECInstanceChanges : ECChangeArray<ECPropertyValueChange>
    {
    public:
        ECInstanceChanges(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECChangeArray<ECPropertyValueChange>(state, SystemId::CustomAttributes, parent, customId, SystemId::PropertyValue)
            {
            BeAssert(systemId == GetSystemId());
            }
        virtual ~ECInstanceChanges() {}
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct ECPropertyChanges : ECChangeArray<ECPropertyChange>
    {
    public:
        ECPropertyChanges(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECChangeArray<ECPropertyChange>(state, SystemId::Properties, parent, customId, SystemId::Property)
            {
            BeAssert(systemId == GetSystemId());
            }
        virtual ~ECPropertyChanges() {}
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct ECRelationshipConstraintClassChanges : ECChangeArray<ECRelationshipConstraintClassChange>
    {
    public:
        ECRelationshipConstraintClassChanges(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECChangeArray<ECRelationshipConstraintClassChange>(state, SystemId::ConstraintClasses, parent, customId, SystemId::ConstraintClass)
            {
            BeAssert(systemId == GetSystemId());
            }
        virtual ~ECRelationshipConstraintClassChanges() {}
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct ECEnumeratorChanges : ECChangeArray<ECEnumeratorChange>
    {
    public:
        ECEnumeratorChanges(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECChangeArray<ECEnumeratorChange>(state, SystemId::Enumerators, parent, customId, SystemId::Enumerator)
            {
            BeAssert(systemId == GetSystemId());
            }
        virtual ~ECEnumeratorChanges() {}
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct StringChanges : ECChangeArray<StringChange>
    {
    public:
        StringChanges(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECChangeArray<StringChange>(state, systemId, parent, customId, SystemId::String)
            {}
        virtual ~StringChanges() {}
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct BaseClassChanges : ECChangeArray<StringChange>
    {
    public:
        BaseClassChanges(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECChangeArray<StringChange>(state, systemId, parent, customId, SystemId::BaseClass)
            {}
        virtual ~BaseClassChanges() {}
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct ReferenceChanges : ECChangeArray<StringChange>
    {
    public:
        ReferenceChanges(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECChangeArray<StringChange>(state, systemId, parent, customId, SystemId::Reference)
            {}
        virtual ~ReferenceChanges() {}
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

        virtual  bool _IsEmpty() const override { return m_old == m_new; }
        virtual Utf8String _ToString(ValueId id) const { return "_TOSTRING_NOT_IMPLEMENTED_"; }

        //---------------------------------------------------------------------------------------
        // @bsimethod                                                    Affan.Khan  03/2016
        //+---------------+---------------+---------------+---------------+---------------+------
        virtual void _WriteToString(Utf8StringR str, int currentIndex, int indentSize) const override
            {
            AppendBegin(str, *this, currentIndex);
            str.append(": ");
            if (GetState() == ChangeState::Deleted)
                str.append(ToString(ValueId::Deleted));
            if (GetState() == ChangeState::New)
                str.append(ToString(ValueId::New));
            if (GetState() == ChangeState::Modified)
                str.append(ToString(ValueId::Deleted)).append(" -> ").append(ToString(ValueId::New));
            AppendEnd(str);
            }
    public:
        ECPrimitiveChange(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECChange(state, systemId, parent, customId)
            {}

        virtual ~ECPrimitiveChange() {}
        Nullable<T> const& GetNew() const { return m_new; }
        Nullable<T> const& GetOld() const { return m_old; }
        Utf8String ToString(ValueId id) const { return _ToString(id); }

        //---------------------------------------------------------------------------------------
        // @bsimethod                                                    Affan.Khan  03/2016
        //+---------------+---------------+---------------+---------------+---------------+------
        Nullable<T> const& GetValue(ValueId id) const
            {
            if (id == ValueId::Deleted)
                {
                if (GetState() == ChangeState::New)
                    {
                    BeAssert("For change that marked as NEW user tried to access OLD value");
                    }

                return m_old;
                }

            if (GetState() == ChangeState::Deleted)
                {
                BeAssert("For change that marked as DELETED user tried to access NEW value which would wrong check code");
                }
            return m_new;
            }

        //---------------------------------------------------------------------------------------
        // @bsimethod                                                    Affan.Khan  03/2016
        //+---------------+---------------+---------------+---------------+---------------+------
        BentleyStatus SetValue(ValueId id, Nullable<T> const&& value)
            {
            if (id == ValueId::Deleted)
                {
                if (GetState() == ChangeState::New)
                    {
                    BeAssert(false && "Cannot set OLD value for Change which is marked as NEW");
                    return ERROR;
                    }

                m_old = std::move(value);
                }
            else
                {
                if (GetState() == ChangeState::Deleted)
                    {
                    BeAssert(false && "Cannot set NEW value for Change which is marked DELETED");
                    return ERROR;
                    }

                m_new = std::move(value);
                }
            return SUCCESS;
            }

        //---------------------------------------------------------------------------------------
        // @bsimethod                                                    Affan.Khan  03/2016
        //+---------------+---------------+---------------+---------------+---------------+------
        BentleyStatus SetValue(ValueId id, T const& value)
            {
            if (id == ValueId::Deleted)
                {
                if (GetState() == ChangeState::New)
                    {
                    BeAssert(false && "Cannot set OLD value for Change which is marked as NEW");
                    return ERROR;
                    }

                m_old = value;
                }
            else
                {
                if (GetState() == ChangeState::Deleted)
                    {
                    BeAssert(false && "Cannot set NEW value for Change which is marked DELETED");
                    return ERROR;
                    }

                m_new = value;
                }

            return SUCCESS;
            }

        //---------------------------------------------------------------------------------------
        // @bsimethod                                                    Affan.Khan  03/2016
        //+---------------+---------------+---------------+---------------+---------------+------
        BentleyStatus SetValue(Nullable<T> const&& valueOld, Nullable<T> const&& valueNew)
            {
            if (GetState() != ChangeState::Modified)
                {
                BeAssert(false && "Cannot set both OLD and NEW for change that is not makred as MODIFIED");
                return ERROR;
                }

            m_old = std::move(valueOld);
            m_new = std::move(valueNew);
            return SUCCESS;
            }
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct StringChange : ECPrimitiveChange<Utf8String>
    {
   
    private:
        virtual Utf8String _ToString(ValueId id) const override;

    public:
        StringChange(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECPrimitiveChange<Utf8String>(state, systemId, parent, customId)
            {}
        virtual ~StringChange() {}
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct BooleanChange : ECPrimitiveChange<bool>
    {
    private:
        virtual Utf8String _ToString(ValueId id) const override;

    public:
        BooleanChange(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECPrimitiveChange<bool>(state, systemId, parent, customId)
            {}
        virtual ~BooleanChange() {}
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct UInt32Change : ECPrimitiveChange<uint32_t>
    {
    private:
        virtual Utf8String _ToString(ValueId id) const override;

    public:
        UInt32Change(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECPrimitiveChange<uint32_t>(state, systemId, parent, customId)
            {}
        virtual ~UInt32Change() {}
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct Int32Change : ECPrimitiveChange<int32_t>
    {
    virtual Utf8String _ToString(ValueId id) const override;

    public:
        Int32Change(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECPrimitiveChange<int32_t>(state, systemId, parent, customId)
            {}
        virtual ~Int32Change() {}
    };
//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct DoubleChange: ECPrimitiveChange<double>
    {
    private:
        virtual Utf8String _ToString(ValueId id) const override;

    public:
        DoubleChange(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECPrimitiveChange<double>(state, systemId, parent, customId)
            {}
        virtual ~DoubleChange() {}
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct DateTimeChange: ECPrimitiveChange<DateTime>
    {
    private:
        virtual Utf8String _ToString(ValueId id) const override;

    public:
        DateTimeChange(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECPrimitiveChange<DateTime>(state, systemId, parent, customId)
            {}
        virtual ~DateTimeChange() {}
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct BinaryChange: ECPrimitiveChange<Binary>
    {
    public:
        BinaryChange(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECPrimitiveChange<Binary>(state, systemId, parent, customId)
            {}
        virtual ~BinaryChange() {}
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct Point2DChange: ECPrimitiveChange<DPoint2d>
    {
    private:
        virtual Utf8String _ToString(ValueId id) const override;
    public:
        Point2DChange(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECPrimitiveChange<DPoint2d>(state, systemId, parent, customId)
            {}
        virtual ~Point2DChange() {}
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct Point3DChange: ECPrimitiveChange<DPoint3d>
    {
    private:
        virtual Utf8String _ToString(ValueId id) const override;
    public:
        Point3DChange(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECPrimitiveChange<DPoint3d>(state, systemId, parent, customId)
            {}
        virtual ~Point3DChange() {}
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct Int64Change: ECPrimitiveChange<int64_t>
    {
    private:
        virtual Utf8String _ToString(ValueId id) const override;

    public:
        Int64Change(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECPrimitiveChange<int64_t>(state, systemId, parent, customId)
            {}
        virtual ~Int64Change() {}
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct StrengthTypeChange : ECPrimitiveChange<ECN::StrengthType>
    {
    private:
        virtual Utf8String _ToString(ValueId id) const override;
    public:
        StrengthTypeChange(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECPrimitiveChange<ECN::StrengthType>(state, systemId, parent, customId)
            {}
        virtual ~StrengthTypeChange() {}
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct StrengthDirectionChange : ECPrimitiveChange<ECN::ECRelatedInstanceDirection>
    {
    private:
        virtual Utf8String _ToString(ValueId id) const override;
    public:
        StrengthDirectionChange(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECPrimitiveChange<ECN::ECRelatedInstanceDirection>(state, systemId, parent, customId)
            {}
        virtual ~StrengthDirectionChange() {}
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct ClassModifierChange :ECPrimitiveChange<ECN::ECClassModifier>
    {
    private:
        virtual Utf8String _ToString(ValueId id) const override;
    public:
        ClassModifierChange(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECPrimitiveChange<ECN::ECClassModifier>(state, systemId, parent, customId)
            {}
        virtual ~ClassModifierChange() {}
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct ClassTypeChange :ECPrimitiveChange<ECClassType>
    {
    private:
        virtual Utf8String _ToString(ValueId id) const override;
    public:
        ClassTypeChange(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECPrimitiveChange<ECClassType>(state, systemId, parent, customId)
            {}
        virtual ~ClassTypeChange() {}
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct ECSchemaChange : ECObjectChange
    {
    public:
        ECSchemaChange(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECObjectChange(state, SystemId::Schema, parent, customId)
            {
            BeAssert(systemId == GetSystemId());
            }
        virtual ~ECSchemaChange(){}

        StringChange& GetName() { return Get<StringChange>(SystemId::Name); }
        StringChange& GetDisplayLabel() { return Get<StringChange>(SystemId::DisplayLabel); }
        StringChange& GetDescription() { return Get<StringChange>(SystemId::Description); }
        UInt32Change& GetVersionMajor() { return Get<UInt32Change>(SystemId::VersionMajor); }
        UInt32Change& GetVersionMinor() { return Get<UInt32Change>(SystemId::VersionMinor); }
        StringChange& GetNamespacePrefix() { return Get<StringChange>(SystemId::NamespacePrefix); }
        ReferenceChanges& References() { return Get<ReferenceChanges>(SystemId::References); }
        ECClassChanges& Classes() { return Get<ECClassChanges>(SystemId::Classes); }
        ECEnumerationChanges& Enumerations() { return Get<ECEnumerationChanges>(SystemId::Enumerations); }
        ECInstanceChanges& CustomAttributes() { return Get<ECInstanceChanges>(SystemId::CustomAttributes); }
#ifdef KIND_OF_QUANTITY_SUPPORT
        ECKindOfQuantityChanges& KindOfQuantities() { return Get<ECKindOfQuantityChanges>(SystemId::KindOfQuantities); }
#endif
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct ECEnumeratorChange :ECObjectChange
    {
    public:
        ECEnumeratorChange(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECObjectChange(state, SystemId::Enumerator, parent, customId)
            {
            BeAssert(systemId == GetSystemId());
            }
        virtual ~ECEnumeratorChange() {}
        StringChange& GetDisplayLabel() { return Get<StringChange>(SystemId::DisplayLabel); }
        StringChange& GetString() { return Get<StringChange>(SystemId::String); }
        Int32Change& GetInteger() { return Get<Int32Change>(SystemId::Integer); }
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct ECEnumerationChange :ECObjectChange
    {
    public:
        ECEnumerationChange(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECObjectChange(state, SystemId::Enumeration, parent, customId)
            {
            BeAssert(systemId == GetSystemId());
            }
        virtual ~ECEnumerationChange() {}
        StringChange& GetName() { return Get<StringChange>(SystemId::Name); }
        StringChange& GetDisplayLabel() { return Get<StringChange>(SystemId::DisplayLabel); }
        StringChange& GetDescription() { return Get<StringChange>(SystemId::Description); }
        StringChange& GetTypeName() { return Get<StringChange>(SystemId::TypeName); }
        BooleanChange& IsStrict() { return Get<BooleanChange>(SystemId::IsStrict); }
        ECEnumeratorChanges& Enumerators() { return Get<ECEnumeratorChanges>(SystemId::Enumerators); }
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct ECPropertyValueChange : ECChange
    {
    private:
        std::unique_ptr<ECChange> m_value;
        std::unique_ptr<ECChangeArray<ECPropertyValueChange>> m_children;
        ECN::PrimitiveType m_type;
        Utf8String m_accessString;
    private:
        virtual void _WriteToString(Utf8StringR str, int currentIndex, int indentSize) const override;
        virtual bool _IsEmpty() const override;
        virtual void _Optimize();
        BentleyStatus InitValue(PrimitiveType type)
            {
            if (m_type == type)
                return SUCCESS;

            if (type == static_cast<PrimitiveType>(0))
                {
                m_value = nullptr;
                return SUCCESS;
                }

            switch (type)
                {
                    case PRIMITIVETYPE_Binary:
                        m_value = std::unique_ptr<ECChange>(new BinaryChange(GetState(), SystemId::PropertyValue, this, GetId())); break;
                    case PRIMITIVETYPE_Boolean:
                        m_value = std::unique_ptr<ECChange>(new BooleanChange(GetState(), SystemId::PropertyValue, this, GetId())); break;
                    case PRIMITIVETYPE_DateTime:
                        m_value = std::unique_ptr<ECChange>(new DateTimeChange(GetState(), SystemId::PropertyValue, this, GetId())); break;
                    case PRIMITIVETYPE_Double:
                        m_value = std::unique_ptr<ECChange>(new DoubleChange(GetState(), SystemId::PropertyValue, this, GetId())); break;
                    case PRIMITIVETYPE_IGeometry:
                        {
                        BeAssert(false && "Geometry type is not supported");
                        return ERROR;
                        }
                    case PRIMITIVETYPE_Integer:
                        m_value = std::unique_ptr<ECChange>(new Int32Change(GetState(), SystemId::PropertyValue, this, GetId())); break;
                    case PRIMITIVETYPE_Long:
                        m_value = std::unique_ptr<ECChange>(new Int64Change(GetState(), SystemId::PropertyValue, this, GetId())); break;
                    case PRIMITIVETYPE_Point2D:
                        m_value = std::unique_ptr<ECChange>(new Point2DChange(GetState(), SystemId::PropertyValue, this, GetId())); break;
                    case PRIMITIVETYPE_Point3D:
                        m_value = std::unique_ptr<ECChange>(new Point3DChange(GetState(), SystemId::PropertyValue, this, GetId())); break;
                    case PRIMITIVETYPE_String:
                        m_value = std::unique_ptr<ECChange>(new StringChange(GetState(), SystemId::PropertyValue, this, GetId())); break;
                    default:
                        BeAssert(false && "Unexpected value for PrimitiveType");
                        return ERROR;
                }

            m_type = type;
            return SUCCESS;
            }

        template< typename T >
        class Converter
            {
            template< bool cond, typename U >
            using resolvedType = Nullable<typename std::enable_if< cond, U >::type>;

            public:
                template< typename U = T >
                static resolvedType< std::is_same<U, Binary>::value, U > Copy(ECValueCR v)
                    {
                    if (v.IsNull()) return Nullable<T>();
                    Nullable<T> t;
                    Binary bin;
                    bin.CopyFrom(v);
                    t = bin;
                    return t;
                    }

                template< typename U = T >
                static resolvedType<std::is_same<U, int32_t>::value, U > Copy(ECValueCR v)
                    {
                    if (v.IsNull()) return Nullable<T>();
                    return v.GetInteger();
                    }
                template< typename U = T >
                static resolvedType< std::is_same<U, int64_t>::value, U > Copy(ECValueCR v)
                    {
                    if (v.IsNull()) return Nullable<T>();
                    return v.GetLong();
                    }
                template< typename U = T >
                static resolvedType< std::is_same<U, DateTime>::value, U > Copy(ECValueCR v)
                    {
                    if (v.IsNull()) return Nullable<T>();
                    return v.GetDateTime();
                    }
                template< typename U = T >
                static resolvedType< std::is_same<U, Utf8String>::value, U > Copy(ECValueCR v)
                    {
                    if (v.IsNull()) return Nullable<T>();
                    return Nullable<T>(v.GetUtf8CP());
                    }
                template< typename U = T >
                static resolvedType< std::is_same<U, DPoint2d>::value, U > Copy(ECValueCR v)
                    {
                    if (v.IsNull()) return Nullable<T>();
                    return v.GetPoint2D();
                    }
                template< typename U = T >
                static resolvedType< std::is_same<U, DPoint3d>::value, U > Copy(ECValueCR v)
                    {
                    if (v.IsNull()) return Nullable<T>();
                    return v.GetPoint3D();
                    }
                template< typename U = T >
                static resolvedType< std::is_same<U, double>::value, U > Copy(ECValueCR v)
                    {
                    if (v.IsNull()) return Nullable<T>();
                    return v.GetDouble();
                    }
                template< typename U = T >
                static resolvedType< std::is_same<U, bool>::value, U > Copy(ECValueCR v)
                    {
                    if (v.IsNull()) return Nullable<T>();
                    return v.GetBoolean();
                    }
            };
    protected:
        void  GetFlatListOfChildren(std::vector<ECPropertyValueChange*>& childrens);

    public:
        ECPropertyValueChange(ChangeState state, SystemId systemId = SystemId::PropertyValue, ECChange const* parent = nullptr, Utf8CP customId = nullptr);
        bool HasValue() const;
        bool HasChildren() const;
        Utf8StringCR GetAccessString() const { return m_accessString; }
        PrimitiveType GetValueType() const { return m_type; }
        StringChange* GetString() const { BeAssert(m_type == PRIMITIVETYPE_String); if (m_type != PRIMITIVETYPE_String) return nullptr; return static_cast<StringChange*>(m_value.get()); }
        BooleanChange* GetBoolean() const { BeAssert(m_type == PRIMITIVETYPE_Boolean); if (m_type != PRIMITIVETYPE_Boolean) return nullptr; return static_cast<BooleanChange*>(m_value.get()); }
        DateTimeChange* GetDateTime() const { BeAssert(m_type == PRIMITIVETYPE_DateTime); if (m_type != PRIMITIVETYPE_DateTime) return nullptr; return static_cast<DateTimeChange*>(m_value.get()); }
        DoubleChange* GetDouble() const { BeAssert(m_type == PRIMITIVETYPE_Double); if (m_type != PRIMITIVETYPE_Double) return nullptr; return static_cast<DoubleChange*>(m_value.get()); }
        Int32Change* GetInteger() const { BeAssert(m_type == PRIMITIVETYPE_Integer); if (m_type != PRIMITIVETYPE_Integer) return nullptr; return static_cast<Int32Change*>(m_value.get()); }
        Int64Change* GetLong() const { BeAssert(m_type == PRIMITIVETYPE_Long); if (m_type != PRIMITIVETYPE_Long) return nullptr; return static_cast<Int64Change*>(m_value.get()); }
        Point2DChange* GetPoint2D() const { BeAssert(m_type == PRIMITIVETYPE_Point2D); if (m_type != PRIMITIVETYPE_Point2D) return nullptr; return static_cast<Point2DChange*>(m_value.get()); }
        Point3DChange* GetPoint3D() const { BeAssert(m_type == PRIMITIVETYPE_Point3D); if (m_type != PRIMITIVETYPE_Point3D) return nullptr; return static_cast<Point3DChange*>(m_value.get()); }
        BinaryChange* GetBinary() const { BeAssert(m_type == PRIMITIVETYPE_Binary); if (m_type != PRIMITIVETYPE_Binary) return nullptr; return static_cast<BinaryChange*>(m_value.get()); }
        BentleyStatus SetValue(ValueId id, ECValueCR value);
        BentleyStatus SetValue(ECValueCR oldValue, ECValueCR newValue);
        ECChangeArray<ECPropertyValueChange>& GetChildren();
        ECPropertyValueChange& GetOrCreate(ChangeState stat, std::vector<Utf8String> const& path);
        ECPropertyValueChange* GetValue(Utf8CP accessPath);
        bool IsDefinition() const
            {
            return dynamic_cast<ECPropertyValueChange const*>(GetParent()) == nullptr;
            }
        std::vector<ECPropertyValueChange*> GetFlatListOfChildren()
            {
            std::vector<ECPropertyValueChange*> v;
            this->GetFlatListOfChildren(v);
            return v;
            }

        virtual ~ECPropertyValueChange() {}
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
#ifdef KIND_OF_QUANTITY_SUPPORT
struct KindOfQuantityChange :ECObjectChange
    {
    public:
        KindOfQuantityChange(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECObjectChange(state, SystemId::KindOfQuantity, parent, customId)
            {
            BeAssert(systemId == GetSystemId());
            }
        virtual ~KindOfQuantityChange() {}
        StringChange& GetName() { return Get<StringChange>(SystemId::Name); }
        StringChange& GetDisplayLabel() { return Get<StringChange>(SystemId::DisplayLabel); }
        StringChange& GetDescription() { return Get<StringChange>(SystemId::Description); }
        StringChange& GetDefaultPresentationUnit() { return Get<StringChange>(SystemId::DefaultPresentationUnit); }
        StringChange& GetPersistenceUnit() { return Get<StringChange>(SystemId::PersistenceUnit); }
        UInt32Change& GetPrecision() { return Get<UInt32Change>(SystemId::Precision); }
        StringChanges& GetAlternativePresentationUnitList() { return Get<StringChanges>(SystemId::AlternativePresentationUnitList); }
    };
#endif

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct ECRelationshipConstraintClassChange :ECObjectChange
    {
    public:
        ECRelationshipConstraintClassChange(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECObjectChange(state, SystemId::ConstraintClass, parent, customId)
            {
            BeAssert(systemId == GetSystemId());
            }
        virtual ~ECRelationshipConstraintClassChange() {}
        StringChange& GetClassName() { return Get<StringChange>(SystemId::ClassFullName); }
        StringChanges& KeyProperties() { return Get<StringChanges>(SystemId::KeyProperties); }
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct ECRelationshipConstraintChange :ECObjectChange
    {
    public:
        ECRelationshipConstraintChange(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECObjectChange(state, systemId, parent, customId)
            {
            BeAssert(systemId == SystemId::Source || systemId == SystemId::Target);
            }
        virtual ~ECRelationshipConstraintChange() {}
        StringChange& GetRoleLabel() { return Get<StringChange>(SystemId::RoleLabel); }
        StringChange& GetCardinality() { return Get<StringChange>(SystemId::Cardinality); }
        BooleanChange& IsPolymorphic() { return Get<BooleanChange>(SystemId::IsPolymorphic); }
        ECRelationshipConstraintClassChanges& ConstraintClasses() { return Get<ECRelationshipConstraintClassChanges>(SystemId::ConstraintClasses); }
        ECInstanceChanges& CustomAttributes() { return Get<ECInstanceChanges>(SystemId::CustomAttributes); }
    };


//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct ECRelationshipChange :ECObjectChange
    {
    public:
        ECRelationshipChange(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr) 
            : ECObjectChange(state, SystemId::Relationship, parent, customId)
            {
            BeAssert(systemId == GetSystemId());
            }
        virtual ~ECRelationshipChange() {}
        StrengthTypeChange& GetStrength() { return Get<StrengthTypeChange>(SystemId::StrengthType); }
        StrengthDirectionChange& GetStrengthDirection() { return Get<StrengthDirectionChange>(SystemId::StrengthDirection); }
        ECRelationshipConstraintChange& GetSource() { return Get<ECRelationshipConstraintChange>(SystemId::Source); }
        ECRelationshipConstraintChange& GetTarget() { return Get<ECRelationshipConstraintChange>(SystemId::Target); }
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct ECClassChange :ECObjectChange
    {
    public:
        ECClassChange(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECObjectChange(state, SystemId::Class, parent, customId)
            {
            BeAssert(systemId == GetSystemId());
            }
        virtual ~ECClassChange() {}
        StringChange& GetName() { return Get<StringChange>(SystemId::Name); }
        StringChange& GetDisplayLabel() { return Get<StringChange>(SystemId::DisplayLabel); }
        StringChange& GetDescription() { return Get<StringChange>(SystemId::Description); }
        ClassModifierChange& GetClassModifier() { return Get<ClassModifierChange>(SystemId::ClassModifier); }
        ClassTypeChange& ClassType() {return Get<ClassTypeChange>(SystemId::ClassType);}
        BaseClassChanges& BaseClasses() { return Get<BaseClassChanges>(SystemId::BaseClasses); }
        ECPropertyChanges& Properties() { return Get<ECPropertyChanges>(SystemId::Properties); }
        ECInstanceChanges& CustomAttributes() { return Get<ECInstanceChanges>(SystemId::CustomAttributes); }
        ECRelationshipChange& GetRelationship() { return Get<ECRelationshipChange>(SystemId::Relationship); }
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct NavigationChange :ECObjectChange
    {
    public:
        NavigationChange(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr) 
            : ECObjectChange(state, SystemId::Navigation, parent, customId)
            {
            BeAssert(systemId == GetSystemId());
            }
        virtual ~NavigationChange() {}
        StrengthDirectionChange& Direction() { return Get<StrengthDirectionChange>(SystemId::Direction); }
        StringChange& GetRelationshipClassName() { return Get<StringChange>(SystemId::RelationshipName); }
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct ArrayChange :ECObjectChange
    {
    public:
        ArrayChange(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECObjectChange(state, SystemId::Array, parent, customId)
            {
            BeAssert(systemId == GetSystemId());
            }
        virtual ~ArrayChange() {}
        UInt32Change& MinOccurs() { return Get<UInt32Change>(SystemId::MinOccurs); }
        UInt32Change& MaxOccurs() { return Get<UInt32Change>(SystemId::MaxOccurs); }
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct ECPropertyChange :ECObjectChange
    {
    public:
        ECPropertyChange(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr) 
            : ECObjectChange(state, SystemId::Property, parent, customId)
            {
            BeAssert(systemId == GetSystemId());
            }
        virtual ~ECPropertyChange() {}
        StringChange& GetName() { return Get<StringChange>(SystemId::Name); }
        StringChange& GetDisplayLabel() { return Get<StringChange>(SystemId::DisplayLabel); }
        StringChange& GetDescription() { return Get<StringChange>(SystemId::Description); }
        StringChange& GetTypeName() { return Get<StringChange>(SystemId::TypeName); }
        //StringChange& GetMaximumValue() { return Get<StringChange>(SystemId::MAXIMUMVALUE); }
        //StringChange& GetMinimumValue() { return Get<StringChange>(SystemId::MINIMUMVALUE); }
        BooleanChange& IsStruct() { return Get<BooleanChange>(SystemId::IsStrict); }
        BooleanChange& IsStructArray() { return Get<BooleanChange>(SystemId::IsStructArray); }
        BooleanChange& IsPrimitive() { return Get<BooleanChange>(SystemId::IsPrimitive); }
        BooleanChange& IsPrimitiveArray() { return Get<BooleanChange>(SystemId::IsPrimitiveArray); }
        BooleanChange& IsNavigation() { return Get<BooleanChange>(SystemId::IsNavigation); }
        ArrayChange& GetArray() { return Get<ArrayChange>(SystemId::Array); }
        NavigationChange& GetNavigation() { return Get<NavigationChange>(SystemId::Navigation); }
        BooleanChange& IsReadonly() { return Get<BooleanChange>(SystemId::IsReadonly); }
        ECInstanceChanges& CustomAttributes() { return Get<ECInstanceChanges>(SystemId::CustomAttributes); }
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct ECSchemaComparer
    {
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
    private:
        BentleyStatus CompareECSchema(ECSchemaChange& change, ECSchemaCR a, ECSchemaCR b);
        BentleyStatus CompareECClass(ECClassChange& change, ECClassCR a, ECClassCR b);
        BentleyStatus CompareECBaseClasses(BaseClassChanges& changes, ECBaseClassesList const& a, ECBaseClassesList const& b);
        BentleyStatus CompareECRelationshipClass(ECRelationshipChange& change, ECRelationshipClassCR a, ECRelationshipClassCR b);
        BentleyStatus CompareECRelationshipConstraint(ECRelationshipConstraintChange& change, ECRelationshipConstraintCR a, ECRelationshipConstraintCR b);
        BentleyStatus CompareECRelationshipConstraintClassKeys(ECRelationshipConstraintClassChange& change, ECRelationshipConstraintClassCR a, ECRelationshipConstraintClassCR b);
        BentleyStatus CompareECRelationshipConstraintClasses(ECRelationshipConstraintClassChanges& change, ECRelationshipConstraintClassList const& a, ECRelationshipConstraintClassList const& b);
        BentleyStatus CompareECProperty(ECPropertyChange& change, ECPropertyCR a, ECPropertyCR b);
        BentleyStatus CompareECProperties(ECPropertyChanges& changes, ECPropertyIterableCR a, ECPropertyIterableCR b);
        BentleyStatus CompareECClasses(ECClassChanges& changes, ECClassContainerCR a, ECClassContainerCR b);
        BentleyStatus CompareECEnumerations(ECEnumerationChanges& changes, ECEnumerationContainerCR a, ECEnumerationContainerCR b);
        BentleyStatus CompareCustomAttributes(ECInstanceChanges& changes, IECCustomAttributeContainerCR a, IECCustomAttributeContainerCR b);
        BentleyStatus CompareCustomAttribute(ECPropertyValueChange& changes, IECInstanceCR a, IECInstanceCR b);        
        BentleyStatus CompareECEnumeration(ECEnumerationChange& change, ECEnumerationCR a, ECEnumerationCR b);
        BentleyStatus CompareIntegerECEnumerators(ECEnumeratorChanges& changes, EnumeratorIterable const& a, EnumeratorIterable const& b);
        BentleyStatus CompareStringECEnumerators(ECEnumeratorChanges& changes, EnumeratorIterable const& a, EnumeratorIterable const& b);
        BentleyStatus CompareBaseClasses(BaseClassChanges& changes, ECBaseClassesList const& a, ECBaseClassesList const& b);
        BentleyStatus CompareReferences(ReferenceChanges& changes, ECSchemaReferenceListCR a, ECSchemaReferenceListCR b);
        BentleyStatus AppendECSchema(ECSchemaChanges& changes, ECSchemaCR v, ValueId appendType);
        BentleyStatus AppendECClass(ECClassChanges& changes, ECClassCR v, ValueId appendType);
        BentleyStatus AppendECRelationshipClass(ECRelationshipChange& change, ECRelationshipClassCR v, ValueId appendType);
        BentleyStatus AppendECRelationshipConstraint(ECRelationshipConstraintChange& change, ECRelationshipConstraintCR v, ValueId appendType);
        BentleyStatus AppendECRelationshipConstraintClasses(ECRelationshipConstraintClassChanges& changes, ECRelationshipConstraintClassList const& v, ValueId appendType);
        BentleyStatus AppendECRelationshipConstraintClass(ECRelationshipConstraintClassChange& change, ECRelationshipConstraintClassCR v, ValueId appendType);
        BentleyStatus AppendECEnumeration(ECEnumerationChanges& changes, ECEnumerationCR v, ValueId appendType);
        BentleyStatus AppendECProperty(ECPropertyChanges& changes, ECPropertyCR v, ValueId appendType);
        BentleyStatus AppendCustomAttributes(ECInstanceChanges& changes, IECCustomAttributeContainerCR v, ValueId appendType);
        BentleyStatus AppendCustomAttribute(ECInstanceChanges& changes, IECInstanceCR v, ValueId appendType);        
        BentleyStatus AppendBaseClasses(BaseClassChanges& changes, ECBaseClassesList const& v, ValueId appendType);
        BentleyStatus AppendReferences(ReferenceChanges& changes, ECSchemaReferenceListCR v, ValueId appendType);
        BentleyStatus ConvertECInstanceToValueMap(std::map<Utf8String, ECValue>& map, IECInstanceCR instance);
        BentleyStatus ConvertECValuesCollectionToValueMap(std::map<Utf8String, ECValue>& map, ECValuesCollectionCR values);
#ifdef KIND_OF_QUANTITY_SUPPORT
        BentleyStatus AppendKindOfQuantity(ECKindOfQuantityChanges& changes, KindOfQuantityCR v, ValueId appendType);
        BentleyStatus CompareKindOfQuantity(KindOfQuantityChange& change, KindOfQuantityCR a, KindOfQuantityCR b);
        BentleyStatus CompareKindOfQuantities(ECKindOfQuantityChanges& changes, KindOfQuantityContainerCR a, KindOfQuantityContainerCR b);
#endif
    public:
        ECSchemaComparer(){}
        ~ECSchemaComparer(){}
        BentleyStatus Compare(ECSchemaChanges& changes, bvector<ECN::ECSchemaCP> const& existingSet, bvector<ECN::ECSchemaCP> const& newSet, Options options = Options());
        static std::vector<Utf8String> Split(Utf8StringCR path, bool stripArrayIndex = false);
        static Utf8String Join(std::vector<Utf8String> const& paths, Utf8CP delimiter);
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct CustomAttributeValidator : NonCopyableClass
    {
    enum class Policy
        {
        Accept,
        Reject
        };

    private:
        struct Rule : NonCopyableClass
            {
            private:
                Policy m_policy;
                std::vector<Utf8String> m_pattren;
            public:
                Rule(Policy policy, Utf8CP pattren);
                ~Rule() {}
                bool Match(std::vector<Utf8String> const& source) const;
                Policy GetPolicy() const { return m_policy; }
            };

    private:
        Policy m_defaultPolicy;
        typedef std::vector <std::unique_ptr<Rule>> RuleList;
        std::map<Utf8String, RuleList> m_rules;
        Utf8String m_wilfCard;
    private:
        RuleList const& GetRelaventRules(ECPropertyValueChange& change) const;
        static Utf8String GetPrefix(Utf8StringCR path);

    public:
        Policy GetDefaultPolicy() const { return m_defaultPolicy; }
        Policy Validate(ECPropertyValueChange& change) const;
        CustomAttributeValidator();
        void Reset();
        void SetDefaultPolicy(Policy defaultPolicy) { m_defaultPolicy = defaultPolicy; }
        void Accept(Utf8CP accessString);
        bool HasAnyRuleForSchema(Utf8CP schemaName) const;
        void Reject(Utf8CP accessString);
    };
END_BENTLEY_SQLITE_EC_NAMESPACE

