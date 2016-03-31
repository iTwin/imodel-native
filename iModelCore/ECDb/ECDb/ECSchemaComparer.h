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

#define INDENT_SIZE 3

struct ECSchemaChange;
struct ECClassChange;
struct ECEnumerationChange;
//struct KindOfQuantityChange;
struct ECPropertyValueChange;
struct StringChange;
struct ECPropertyChange;
struct ECRelationshipConstraintClassChange;
struct ECEnumeratorChange;
struct ECPropertyValueChange;
struct ECObjectChange;

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
    IsCustomAttributeClass,
    IsEntityClass,
    IsPolymorphic,
    IsReadonly,
    IsRelationshipClass,
    IsStrict,
    IsStructClass,
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

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan  03/2016
//+---------------+---------------+---------------+---------------+---------------+------
static bool operator==(ECN::ECValueCR lhs, ECN::ECValueCR rhs)
    {
    return lhs.Equals(rhs);
    }

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
    private:
        std::unique_ptr<Utf8String> m_customId;
        SystemId m_systemId;
        ECChange const* m_parent;
        ChangeState m_state;
        bool m_applied;

        virtual void _WriteToString(Utf8StringR str, int currentIndex, int indentSize) const = 0;
        virtual bool _IsEmpty() const = 0;
        virtual void _Optimize() {}

        static std::map<Utf8CP, SystemId, CompareUtf8> const& GetStringToTypeMap();
        static std::map<SystemId, Utf8String> const& GetTypeToStringMap();

    protected:
        ECChange(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr);

        static Utf8StringCR Convert(SystemId id);
        static SystemId Convert(Utf8CP id);
        static void AppendBegin(Utf8StringR str, ECChange const& change, int currentIndex);
        static void AppendEnd(Utf8StringR str);

    public:
        virtual ~ECChange() {};
        SystemId GetSystemId() const;
        Utf8StringCR GetId() const;
        bool HasCustomId() const;
        ChangeState GetState() const;
        ECChange const* GetParent() const;
        bool IsEmpty() const;
        void Optimize();
        bool Exist() const;
        bool IsPending() const;
        void Done();
        void WriteToString(Utf8StringR str, int initIndex = 0, int indentSize = INDENT_SIZE) const;
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
        bvector<ECChangePtr> m_changes;
        SystemId m_elementType;

    private:
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
                if (v->GetId() == customId)
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
//struct ECKindOfQuantityChanges : ECChangeArray<KindOfQuantityChange>
//    {
//    public:
//        ECKindOfQuantityChanges(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
//            : ECChangeArray<KindOfQuantityChange>(state, SystemId::KINDOFQUANTITIES, parent, customId, SystemId::KINDOFQUANTITY)
//            {
//            BeAssert(systemId == GetSystemId());
//            }
//        virtual ~ECKindOfQuantityChanges() {}
//    };
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

    private:
        //---------------------------------------------------------------------------------------
        // @bsimethod                                                    Affan.Khan  03/2016
        //+---------------+---------------+---------------+---------------+---------------+------
        virtual  bool _IsEmpty() const override
            {
            return m_old == m_new;
            }

        //---------------------------------------------------------------------------------------
        // @bsimethod                                                    Affan.Khan  03/2016
        //+---------------+---------------+---------------+---------------+---------------+------
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
        //---------------------------------------------------------------------------------------
        // @bsimethod                                                    Affan.Khan  03/2016
        //+---------------+---------------+---------------+---------------+---------------+------
        Nullable<T> const& GetNew() const
            {
            return m_new;
            }

        //---------------------------------------------------------------------------------------
        // @bsimethod                                                    Affan.Khan  03/2016
        //+---------------+---------------+---------------+---------------+---------------+------
        Nullable<T> const& GetOld() const
            {
            return m_new;
            }

        //---------------------------------------------------------------------------------------
        // @bsimethod                                                    Affan.Khan  03/2016
        //+---------------+---------------+---------------+---------------+---------------+------
        Utf8String ToString(ValueId id) const
            {
            return _ToString(id);
            }

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
struct ECValueChange : ECPrimitiveChange<ECValue>
    {
    private:
        virtual Utf8String _ToString(ValueId id) const override;

    public:
        ECValueChange(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECPrimitiveChange<ECValue>(state, systemId, parent, customId)
            {}
        virtual ~ECValueChange() {}
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
struct ModifierChange :ECPrimitiveChange<ECN::ECClassModifier>
    {
    private:
        virtual Utf8String _ToString(ValueId id) const override;
    public:
        ModifierChange(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECPrimitiveChange<ECN::ECClassModifier>(state, systemId, parent, customId)
            {}
        virtual ~ModifierChange() {}
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
        UInt32Change& GetVersionWrite() { return Get<UInt32Change>(SystemId::VersionWrite); }
        StringChange& GetNamespacePrefix() { return Get<StringChange>(SystemId::NamespacePrefix); }
        ReferenceChanges& References() { return Get<ReferenceChanges>(SystemId::References); }
        ECClassChanges& Classes() { return Get<ECClassChanges>(SystemId::Classes); }
        ECEnumerationChanges& Enumerations() { return Get<ECEnumerationChanges>(SystemId::Enumerations); }
        //ECKindOfQuantityChanges& KindOfQuantities() { return Get<ECKindOfQuantityChanges>(SystemId::KINDOFQUANTITIES); }
        ECInstanceChanges& CustomAttributes() { return Get<ECInstanceChanges>(SystemId::CustomAttributes); }
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
        std::unique_ptr<ECValueChange> m_value;
        std::unique_ptr<ECChangeArray<ECPropertyValueChange>> m_children;

    private:
        virtual void _WriteToString(Utf8StringR str, int currentIndex, int indentSize) const override;
        virtual bool _IsEmpty() const override;
        virtual void _Optimize();

    public:
        ECPropertyValueChange(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr);
        bool HasValue() const;
        bool HasChildren() const;

        ECPrimitiveChange<ECValue>& GetPropertyValue();
        ECChangeArray<ECPropertyValueChange>& GetChildren();
        ECPropertyValueChange& GetOrCreate(ChangeState stat, std::vector<Utf8String> const& path);
        virtual ~ECPropertyValueChange() {}
    };

//struct KindOfQuantityChange :ECObjectChange
//    {
//    public:
//        KindOfQuantityChange(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
//            : ECObjectChange(state, SystemId::KINDOFQUANTITY, parent, customId)
//            {
//            BeAssert(systemId == GetSystemId());
//            }
//        virtual ~KindOfQuantityChange() {}
//        StringChange& GetName() { return Get<StringChange>(SystemId::NAME); }
//        StringChange& GetDisplayLabel() { return Get<StringChange>(SystemId::DISPLAYLABEL); }
//        StringChange& GetDescription() { return Get<StringChange>(SystemId::DESCRIPTION); }
//        StringChange& GetDefaultPresentationUnit() { return Get<StringChange>(SystemId::DEFAULTPRESENTATIONUNIT); }
//        StringChange& GetPersistenceUnit() { return Get<StringChange>(SystemId::PERSISTENCEUNIT); }
//        UInt32Change& GetPrecision() { return Get<UInt32Change>(SystemId::PRECISION); }
//        StringChanges& GetAlternativePresentationUnitList() { return Get<StringChanges>(SystemId::ALTERNATIVEPRESENTATIONUNITLIST); }
//    };

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
        ModifierChange& GetClassModifier() { return Get<ModifierChange>(SystemId::ClassModifier); }
        BooleanChange& IsCustomAttributeClass() { return Get<BooleanChange>(SystemId::IsCustomAttributeClass); }
        BooleanChange& IsEntityClass() { return Get<BooleanChange>(SystemId::IsEntityClass); }
        BooleanChange& IsStructClass() { return Get<BooleanChange>(SystemId::IsStructClass); }
        BooleanChange& IsRelationshipClass() {return Get<BooleanChange>(SystemId::IsRelationshipClass);}
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
        StringChange& GetExtendedTypeName() { return Get<StringChange>(SystemId::ExtendedTypeName); }
        BooleanChange& IsReadonly() { return Get<BooleanChange>(SystemId::IsReadonly); }
        ECInstanceChanges& CustomAttributes() { return Get<ECInstanceChanges>(SystemId::CustomAttributes); }
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct ECSchemaComparer
    {
    private:
        BentleyStatus CompareECSchemas(ECSchemaChanges& changes, ECSchemaList const& a, ECSchemaList const& b);
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
        std::vector<Utf8String> Split(Utf8StringCR path);
        //BentleyStatus AppendECInstance(ECPropertyValueChange& changes, IECInstanceCR v, ValueId appendType);        
        //BentleyStatus AppendKindOfQuantity(ECKindOfQuantityChanges& changes, KindOfQuantityCR v, ValueId appendType);
        //BentleyStatus CompareKindOfQuantity(KindOfQuantityChange& change, KindOfQuantityCR a, KindOfQuantityCR b);
        //BentleyStatus CompareKindOfQuantities(ECKindOfQuantityChanges& changes, KindOfQuantityContainerCR a, KindOfQuantityContainerCR b);

    public:
        BentleyStatus Compare(ECSchemaChanges& changes, ECSchemaList const& existingSet, ECSchemaList const& newSet);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE

