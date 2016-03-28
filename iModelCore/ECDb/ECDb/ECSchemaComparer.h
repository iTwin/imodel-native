/*--------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSchemaComparer.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+-------------------------------------------------------------------------------------*/
#pragma once
#include "ECDbInternalTypes.h"
USING_NAMESPACE_BENTLEY_EC
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
#define INDENT_SIZE 3
//Forward declaration======================
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
enum SystemId : uint32_t
    {
    NONE,
    ALTERNATIVEPRESENTATIONUNITLIST,
    ARRAY,
    CARDINALITY,
    CLASSES,
    CLASS,
    CLASSMODIFIER,
    CLASSFULLNAME,
    CONSTANTKEY,
    CONSTRAINTCLASS,
    CONSTRAINTCLASSES,
    CONSTRAINT,
    CUSTOMATTRIBTUES,
    DEFAULTPRESENTATIONUNIT,
    DESCRIPTION,
    DIRECTION,
    DISPLAYLABEL,
    BASECLASSES,
    BASECLASS,
    ENUMERATION,
    ENUMERATIONS,
    ENUMERATOR,
    ENUMERATORS,
    EXTENDEDTYPENAME,
    INSTANCE,
    INSTANCES,
    PROPERTYVALUE,
    PROPERTYVALUES,
    INTEGER,
    ISCUSTOMATTRIBUTECLASS,
    ISENTITYCLASS,
    ISPOLYMORPHIC,
    ISREADONLY,
    ISRELATIONSHIPCLASS,
    ISSTRICT,
    ISSTRUCTCLASS,
    ISSTRUCT,
    ISSTRUCTARRAY,
    ISPRIMITIVE,
    ISPRIMITIVEARRAY,
    ISNAVIGATION,
    KEYPROPERTIES,
    KEYPROPERTY,
    KINDOFQUANTITIES,
    KINDOFQUANTITY,
    MAXIMUMVALUE,
    MAXOCCURS,
    MINIMUMVALUE,
    MINOCCURS,
    NAME,
    NAMESPACEPREFIX,
    NAVIGATION,
    PERSISTENCEUNIT,
    PRECISION,
    PROPERTIES,
    PROPERTY,
    PROPERTYTYPE,
    REFERENCES,
    REFERENCE,
    RELATIONSHIP,
    RELATIONSHIPNAME,
    ROLELABEL,
    SCHEMA,
    SCHEMAS,
    SOURCE,
    STRENGTHDIRECTION,
    STRENGTHTYPE,
    STRING,
    TARGET,
    TYPENAME,
    VERSIONMAJOR,
    VERSIONMINOR,
    VERSIONWRITE,
    };
//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
template<typename T>
struct Nullable
    {
    private:
        T m_value;
        bool m_isNull;

    public:
        Nullable();
        Nullable(T const& value);
        Nullable(Nullable<T> const& rhs);
        Nullable(Nullable<T> const&& rhs);
        bool IsNull() const;
        T const& Value() const;
        T& ValueR();
        bool operator == (Nullable<T> const& rhs) const;
        bool operator == (std::nullptr_t rhs)const;
        bool operator != (Nullable<T> const& rhs) const;
        bool operator != (std::nullptr_t rhs) const;
        Nullable<T>& operator = (Nullable<T> const&& rhs);
        Nullable<T>& operator = (Nullable<T> const& rhs);
        Nullable<T>& operator = (T const& rhs);
        Nullable<T>& operator = (T const&& rhs);
        Nullable<T>& operator = (std::nullptr_t rhs);
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct Binary
    {
    private:
        void* m_buff;
        size_t m_len;

    private:
        bool _empty() const;
        BentleyStatus _resize(size_t len);
        BentleyStatus _free();
        BentleyStatus _assign(void* buff = nullptr, size_t len = 0);

    public:
        Binary();
        Binary(Binary const& rhs);
        Binary(Binary&& rhs);
        Binary& operator = (Binary const& rhs);
        Binary& operator = (Binary&& rhs);
        int Compare(Binary const& rhs) const;
        void* GetPointerP();
        void const* GetPointer() const;
        size_t Size() const;
        bool Empty() const;
        bool operator == (Binary const& rhs) const;
        bool Equalls(Binary const& rhs) const;
        void CopyTo(ECValueR value) const;
        BentleyStatus CopyFrom(ECValueCR value);
        ~Binary();
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct ECChange
    {
    public:
        typedef std::unique_ptr<ECChange> Ptr;

    private:
        static std::map<Utf8CP, SystemId, CompareUtf8> const& GetStringToTypeMap();
        static std::map<SystemId, Utf8String> const& GetTypeToStringMap();

    protected:
        static Utf8StringCR Convert(SystemId id);
        static SystemId Convert(Utf8CP id);
        static  void AppendBegin(Utf8StringR str, ECChange const& change, int currentIndex);
        static  void AppendEnd(Utf8StringR str);

    private:
        std::unique_ptr<Utf8String> m_customId;
        SystemId m_systemId;
        ECChange const* m_parent;
        ChangeState m_state;
        bool m_applied;

    private:
        virtual void _WriteToString(Utf8StringR str, int currentIndex, int indentSize) const = 0;
        virtual bool _IsEmpty() const = 0;
        virtual void _Optimize() {}

    public:
        ECChange(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr);
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

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct ECObjectChange : ECChange
    {
    private:
        std::map<Utf8CP, ECChange::Ptr, CompareUtf8> m_changes;

    private:
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
        std::vector<ECChange::Ptr> m_changes;
        SystemId m_elementType;

    private:
        //---------------------------------------------------------------------------------------
        // @bsimethod                                                    Affan.Khan  03/2016
        //+---------------+---------------+---------------+---------------+---------------+------
        virtual void _WriteToString(Utf8StringR str, int currentIndex, int indentSize) const override
            {
            AppendBegin(str, *this, currentIndex);
            AppendEnd(str);
            for (ECChange::Ptr const& change : m_changes)
                {
                change->WriteToString(str, currentIndex + indentSize, indentSize);
                }
            }

        //---------------------------------------------------------------------------------------
        // @bsimethod                                                    Affan.Khan  03/2016
        //+---------------+---------------+---------------+---------------+---------------+------
        virtual bool _IsEmpty() const override
            {
            for (ECChange::Ptr const& change : m_changes)
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
                    {
                    itor = m_changes.erase(itor);
                    }
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
            ECChange::Ptr ptr = ECChange::Ptr(new T(state, m_elementType, this, customId));
            BeAssert(dynamic_cast<T*>(ptr.get()) != nullptr);
            T* p = static_cast<T*>(ptr.get());
            m_changes.push_back(std::move(ptr));
            return *p;
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
            : ECChangeArray<ECSchemaChange>(ChangeState::Modified, SCHEMAS, nullptr, nullptr, SCHEMA)
            {}
        ECSchemaChanges(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECChangeArray<ECSchemaChange>(state, SCHEMAS, parent, customId, SCHEMA)
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
            : ECChangeArray<ECClassChange>(state, CLASSES, parent, customId, CLASS)
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
            : ECChangeArray<ECEnumerationChange>(state, ENUMERATIONS, parent, customId, ENUMERATION)
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
//            : ECChangeArray<KindOfQuantityChange>(state, KINDOFQUANTITIES, parent, customId, KINDOFQUANTITY)
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
            : ECChangeArray<ECPropertyValueChange>(state, CUSTOMATTRIBTUES, parent, customId, PROPERTYVALUE)
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
            : ECChangeArray<ECPropertyChange>(state, PROPERTIES, parent, customId, PROPERTY)
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
            : ECChangeArray<ECRelationshipConstraintClassChange>(state, CONSTRAINTCLASSES, parent, customId, CONSTRAINTCLASS)
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
            : ECChangeArray<ECEnumeratorChange>(state, ENUMERATORS, parent, customId, ENUMERATOR)
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
            : ECChangeArray<StringChange>(state, systemId, parent, customId, STRING)
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
            : ECChangeArray<StringChange>(state, systemId, parent, customId, BASECLASS)
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
            : ECChangeArray<StringChange>(state, systemId, parent, customId, REFERENCE)
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
            : ECObjectChange(state, SCHEMA, parent, customId)
            {
            BeAssert(systemId == GetSystemId());
            }
        virtual ~ECSchemaChange(){}
        StringChange& GetName() { return Get<StringChange>(NAME); }
        StringChange& GetDisplayLabel() { return Get<StringChange>(DISPLAYLABEL); }
        StringChange& GetDescription() { return Get<StringChange>(DESCRIPTION); }
        UInt32Change& GetVersionMajor() { return Get<UInt32Change>(VERSIONMAJOR); }
        UInt32Change& GetVersionMinor() { return Get<UInt32Change>(VERSIONMINOR); }
        UInt32Change& GetVersionWrite() { return Get<UInt32Change>(VERSIONWRITE); }
        StringChange& GetNamespacePrefix() { return Get<StringChange>(NAMESPACEPREFIX); }
        ReferenceChanges& References() { return Get<ReferenceChanges>(REFERENCES); }
        ECClassChanges& Classes() { return Get<ECClassChanges>(CLASSES); }
        ECEnumerationChanges& Enumerations() { return Get<ECEnumerationChanges>(ENUMERATIONS); }
        //ECKindOfQuantityChanges& KindOfQuantities() { return Get<ECKindOfQuantityChanges>(KINDOFQUANTITIES); }
        ECInstanceChanges& CustomAttributes() { return Get<ECInstanceChanges>(CUSTOMATTRIBTUES); }
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct ECEnumeratorChange :ECObjectChange
    {
    public:
        ECEnumeratorChange(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECObjectChange(state, ENUMERATOR, parent, customId)
            {
            BeAssert(systemId == GetSystemId());
            }
        virtual ~ECEnumeratorChange() {}
        StringChange& GetDisplayLabel() { return Get<StringChange>(DISPLAYLABEL); }
        StringChange& GetString() { return Get<StringChange>(STRING); }
        Int32Change& GetInteger() { return Get<Int32Change>(INTEGER); }
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct ECEnumerationChange :ECObjectChange
    {
    public:
        ECEnumerationChange(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECObjectChange(state, ENUMERATION, parent, customId)
            {
            BeAssert(systemId == GetSystemId());
            }
        virtual ~ECEnumerationChange() {}
        StringChange& GetName() { return Get<StringChange>(NAME); }
        StringChange& GetDisplayLabel() { return Get<StringChange>(DISPLAYLABEL); }
        StringChange& GetDescription() { return Get<StringChange>(DESCRIPTION); }
        StringChange& GetTypeName() { return Get<StringChange>(TYPENAME); }
        BooleanChange& IsStrict() { return Get<BooleanChange>(ISSTRICT); }
        ECEnumeratorChanges& Enumerators() { return Get<ECEnumeratorChanges>(ENUMERATORS); }
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
//            : ECObjectChange(state, KINDOFQUANTITY, parent, customId)
//            {
//            BeAssert(systemId == GetSystemId());
//            }
//        virtual ~KindOfQuantityChange() {}
//        StringChange& GetName() { return Get<StringChange>(NAME); }
//        StringChange& GetDisplayLabel() { return Get<StringChange>(DISPLAYLABEL); }
//        StringChange& GetDescription() { return Get<StringChange>(DESCRIPTION); }
//        StringChange& GetDefaultPresentationUnit() { return Get<StringChange>(DEFAULTPRESENTATIONUNIT); }
//        StringChange& GetPersistenceUnit() { return Get<StringChange>(PERSISTENCEUNIT); }
//        UInt32Change& GetPrecision() { return Get<UInt32Change>(PRECISION); }
//        StringChanges& GetAlternativePresentationUnitList() { return Get<StringChanges>(ALTERNATIVEPRESENTATIONUNITLIST); }
//    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct ECRelationshipConstraintClassChange :ECObjectChange
    {
    public:
        ECRelationshipConstraintClassChange(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECObjectChange(state, CONSTRAINTCLASS, parent, customId)
            {
            BeAssert(systemId == GetSystemId());
            }
        virtual ~ECRelationshipConstraintClassChange() {}
        StringChange& GetClassName() { return Get<StringChange>(CLASSFULLNAME); }
        StringChanges& KeyProperties() { return Get<StringChanges>(KEYPROPERTIES); }
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
            BeAssert(systemId == SOURCE || systemId == TARGET);
            }
        virtual ~ECRelationshipConstraintChange() {}
        StringChange& GetRoleLabel() { return Get<StringChange>(ROLELABEL); }
        StringChange& GetCardinality() { return Get<StringChange>(CARDINALITY); }
        BooleanChange& IsPolymorphic() { return Get<BooleanChange>(ISPOLYMORPHIC); }
        ECRelationshipConstraintClassChanges& ConstraintClasses() { return Get<ECRelationshipConstraintClassChanges>(CONSTRAINTCLASSES); }
        ECInstanceChanges& CustomAttributes() { return Get<ECInstanceChanges>(CUSTOMATTRIBTUES); }
    };


//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct ECRelationshipChange :ECObjectChange
    {
    public:
        ECRelationshipChange(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr) 
            : ECObjectChange(state, RELATIONSHIP, parent, customId) 
            {
            BeAssert(systemId == GetSystemId());
            }
        virtual ~ECRelationshipChange() {}
        StrengthTypeChange& GetStrength() { return Get<StrengthTypeChange>(STRENGTHTYPE); }
        StrengthDirectionChange& GetStrengthDirection() { return Get<StrengthDirectionChange>(STRENGTHDIRECTION); }
        ECRelationshipConstraintChange& GetSource() { return Get<ECRelationshipConstraintChange>(SOURCE); }
        ECRelationshipConstraintChange& GetTarget() { return Get<ECRelationshipConstraintChange>(TARGET); }
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct ECClassChange :ECObjectChange
    {
    public:
        ECClassChange(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECObjectChange(state, CLASS, parent, customId)
            {
            BeAssert(systemId == GetSystemId());
            }
        virtual ~ECClassChange() {}
        StringChange& GetName() { return Get<StringChange>(NAME); }
        StringChange& GetDisplayLabel() { return Get<StringChange>(DISPLAYLABEL); }
        StringChange& GetDescription() { return Get<StringChange>(DESCRIPTION); }
        ModifierChange& GetClassModifier() { return Get<ModifierChange>(CLASSMODIFIER); }
        BooleanChange& IsCustomAttributeClass() { return Get<BooleanChange>(ISCUSTOMATTRIBUTECLASS); }
        BooleanChange& IsEntityClass() { return Get<BooleanChange>(ISENTITYCLASS); }
        BooleanChange& IsStructClass() { return Get<BooleanChange>(ISSTRUCTCLASS); }
        BooleanChange& IsRelationshipClass() {return Get<BooleanChange>(ISRELATIONSHIPCLASS);}
        BaseClassChanges& BaseClasses() { return Get<BaseClassChanges>(BASECLASSES); }
        ECPropertyChanges& Properties() { return Get<ECPropertyChanges>(PROPERTIES); }
        ECInstanceChanges& CustomAttributes() { return Get<ECInstanceChanges>(CUSTOMATTRIBTUES); }
        ECRelationshipChange& GetRelationship() { return Get<ECRelationshipChange>(RELATIONSHIP); }
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct NavigationChange :ECObjectChange
    {
    public:
        NavigationChange(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr) 
            : ECObjectChange(state, NAVIGATION, parent, customId)
            {
            BeAssert(systemId == GetSystemId());
            }
        virtual ~NavigationChange() {}
        StrengthDirectionChange& Direction() { return Get<StrengthDirectionChange>(DIRECTION); }
        StringChange& GetRelationshipClassName() { return Get<StringChange>(RELATIONSHIPNAME); }
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct ArrayChange :ECObjectChange
    {
    public:
        ArrayChange(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECObjectChange(state, ARRAY, parent, customId)
            {
            BeAssert(systemId == GetSystemId());
            }
        virtual ~ArrayChange() {}
        UInt32Change& MinOccurs() { return Get<UInt32Change>(MINOCCURS); }
        UInt32Change& MaxOccurs() { return Get<UInt32Change>(MAXOCCURS); }
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct ECPropertyChange :ECObjectChange
    {
    public:
        ECPropertyChange(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr) 
            : ECObjectChange(state, PROPERTY, parent, customId)
            {
            BeAssert(systemId == GetSystemId());
            }
        virtual ~ECPropertyChange() {}
        StringChange& GetName() { return Get<StringChange>(NAME); }
        StringChange& GetDisplayLabel() { return Get<StringChange>(DISPLAYLABEL); }
        StringChange& GetDescription() { return Get<StringChange>(DESCRIPTION); }
        StringChange& GetTypeName() { return Get<StringChange>(TYPENAME); }
        //StringChange& GetMaximumValue() { return Get<StringChange>(MAXIMUMVALUE); }
        //StringChange& GetMinimumValue() { return Get<StringChange>(MINIMUMVALUE); }
        BooleanChange& IsStruct() { return Get<BooleanChange>(ISSTRICT); }
        BooleanChange& IsStructArray() { return Get<BooleanChange>(ISSTRUCTARRAY); }
        BooleanChange& IsPrimitive() { return Get<BooleanChange>(ISPRIMITIVE); }
        BooleanChange& IsPrimitiveArray() { return Get<BooleanChange>(ISPRIMITIVEARRAY); }
        BooleanChange& IsNavigation() { return Get<BooleanChange>(ISNAVIGATION); }
        ArrayChange& GetArray() { return Get<ArrayChange>(ARRAY); }
        NavigationChange& GetNavigation() { return Get<NavigationChange>(NAVIGATION); }
        StringChange& GetExtendedTypeName() { return Get<StringChange>(EXTENDEDTYPENAME); }
        BooleanChange& IsReadonly() { return Get<BooleanChange>(ISREADONLY); }
        ECInstanceChanges& CustomAttributes() { return Get<ECInstanceChanges>(CUSTOMATTRIBTUES); }
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

