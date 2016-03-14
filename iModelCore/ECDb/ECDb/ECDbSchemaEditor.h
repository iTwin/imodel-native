/*--------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbSchemaEditor.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+-------------------------------------------------------------------------------------*/
#pragma once
#include "ECDbInternalTypes.h"
USING_NAMESPACE_BENTLEY_EC
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//Forward declaration======================
struct ECSchemaChange;
struct ECClassChange;
struct ECEnumerationChange;
struct ECKindOfQuantityChange;
struct ECInstanceChange;
struct StringChange;
struct ECPropertyChange;
struct ECRelationshipConstraintClassChange;
struct ECEnumeratorChange;
struct ECPropertyValueChange;
//=========================================

enum class ChangeState
    {
    Deleted,
    Modified,
    New,
    };
enum class PropertyType
    {
    Primitive,
    PrimitiveArray,
    Struct,
    StructArray,
    Navigation,
    ExtendedType,
    };
enum class ChangeMode
    {
    Read, Write
    };
enum class ValueId
    {
    Deleted, New
    };

template<typename T>
struct Nullable
    {
    private:
        T m_value;
        bool m_isNull;
    public:
        Nullable()
            :m_isNull(true), m_value(T())
            {}
        Nullable(T const& value)
            :m_isNull(false), m_value(value)
            {}
        Nullable(Nullable<T> const& rhs)
            :m_isNull(rhs.m_isNull), m_value(rhs.m_value)
            {}
        Nullable(Nullable<T> const&& rhs)
            :m_isNull(std::move(rhs.m_isNull)), m_value(std::move(rhs.m_value))
            {}

        bool IsNull() const { return m_isNull; }
        T const& Value() const
            {
            //   BeAssert(!IsNull());
            return m_value;
            }
        T& ValueR() { return m_value; }

        bool operator == (Nullable<T> const& rhs) const
            {
            if (rhs.IsNull() != IsNull())
                return false;
            else if (rhs.IsNull() && IsNull())
                return true;
            else
                return rhs.Value() == rhs.Value();
            }
        bool operator == (nullptr_t rhs)const
            {
            return IsNull();
            }
        bool operator != (Nullable<T> const& rhs) const
            {
            return !operator==(rhs);
            }
        bool operator != (nullptr_t rhs) const
            {
            return !operator==(rhs);
            }
        Nullable<T>& operator = (Nullable<T> const&& rhs)
            {
            if (&rhs != this)
                {
                m_value = std::move(rhs.m_value);
                m_isNull = std::move(rhs.m_isNull);
                }

            return *this;
            }
        Nullable<T>& operator = (Nullable<T> const& rhs)
            {
            if (&rhs != this)
                {
                m_value = rhs.m_value;
                m_isNull = rhs.m_isNull;
                }
            return *this;
            }
        Nullable<T>& operator = (T const& rhs)
            {
            m_value = rhs;
            m_isNull = false;

            return *this;
            }
        Nullable<T>& operator = (T const&& rhs)
            {
            m_value = std::move(rhs);
            m_isNull = false;

            return *this;
            }
        Nullable<T>& operator = (nullptr_t rhs)
            {
            m_value = T();
            m_isNull = true;
            return *this;
            }
    };
struct Binary
    {
    private:
        void* m_buff;
        size_t m_len;
    private:
        bool _empty() const
            {
            return m_len == 0;
            }
        BentleyStatus _resize(size_t len)
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
        BentleyStatus _free()
            {
            if (m_buff)
                free(m_buff);

            m_buff = nullptr;
            m_len = 0;
            return SUCCESS;
            }
        BentleyStatus _assign(void* buff = nullptr, size_t len = 0)
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
    public:
        Binary():
            m_buff(nullptr), m_len(0)
            {}
        Binary(Binary const& rhs)
            {
            *this = rhs;
            }
        Binary(Binary&& rhs)
            {
            *this = std::move(rhs);
            }
        Binary& operator = (Binary const& rhs)
            {
            if (this != &rhs)
                {
                _assign(rhs.m_buff, rhs.m_len);
                }
            return *this;
            }
        Binary& operator = (Binary&& rhs)
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
        int Compare(Binary const& rhs) const
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
        void* GetPointerP()
            {
            return m_buff;
            }
        void const* GetPointer() const
            {
            return m_buff;
            }
        size_t Size() const
            {
            return m_len;
            }
        bool Empty() const
            {
            return m_len == 0;
            }
        bool operator == (Binary const& rhs) const
            {
            return Compare(rhs) == 0;
            }
        bool Equalls(Binary const& rhs) const
            {
            return  *this == rhs;
            }
        void CopyTo(ECValueR value) const
            {
            value.SetBinary(static_cast<Byte*>(m_buff), m_len, true);
            }
        BentleyStatus CopyFrom(ECValueCR value)
            {
            if (value.IsNull())
                return _free();

            if (!value.IsBinary())
                return ERROR;

            size_t len = 0;
            void* buff = (void*)value.GetBinary(len);
            return _assign(buff, len);
            }
        ~Binary()
            {
            _free();
            }
    };
struct ECChange
    {
    typedef std::unique_ptr<ECChange> Ptr;
    private:
        ChangeState m_state;
        virtual bool _IsEmpty() const=0;
        virtual void _Optimize(){}

    public:
        ECChange(ChangeState state)
            :m_state(state)
            {
            }
        virtual ~ECChange() = 0;
        ChangeState GetState() const { return m_state; }
        bool IsEmpty() const { return _IsEmpty(); }
        void Optimize()  { return _Optimize(); }

    };
struct ECChangeObject : ECChange
    {
    private:
        std::map<uint32_t, ECChange::Ptr> m_changes;
        virtual bool _IsEmpty() const override
            {
            for (auto& change : m_changes)
                {
                if (!change.second->IsEmpty())
                    return false;
                }

            return true;
            }
        virtual void _Optimize() override
            {
            for (auto itor = m_changes.begin(); itor != m_changes.end(); ++itor)
                {
                if (itor->second->IsEmpty())
                    itor = m_changes.erase(itor);
                }
            }
    protected:
        enum
            {
            NAME,
            DISPLAYLABEL,
            DESCRIPTION,
            VERSIONMAJOR,
            VERSIONMINOR,
            VERSIONWRITE,
            NAMESPACEPREFIX,
            REFERENCES,
            CLASSES,
            ENUMERATIONS,
            KINDOFQUANTITIES,
            CUSTOMATTRIBTUES,
            MODIFIER,
            PROPERTIES,
            ISSTRUCTCLASS,
            ISENTITYCLASS,
            ISCUSTOMATTRIBUTECLASS,
            STRENGTHTYPE,
            STRENGTHDIRECTION,
            SOURCE,
            TARGET,
            CONSTRAINTCLASSES,
            KEYPROPERTIES,
            CONSTRAINTCLASS,
            TYPENAME,
            MINOCCURS,
            MAXOCCURS,
            ISSTRICT,
            ENUMERATORS,
            INTEGER,
            STRING,
            DEFAULTPRESENTATIONUNIT,
            PERSISTENCEUNIT,
            PRECISION,
            ALTERNATIVEPRESENTATIONUNITLIST,
            ISREADONLY,
            MAXIMUMVALUE,
            MINIMUMVALUE,
            PROPERTYTYPE,
            ARRAY,
            RELATIONSHIP,
            DIRECTION,
            RELATIONSHIPNAME,
            EXTENDEDTYPENAME,
            ISRELATIONSHIPCLASS,
            ROLELABEL,
            CARDINALITY,
            ISPOLYMORPHIC
            };
    protected:
        template<typename T>
        T& Get(uint32_t attributeId)
            {
            static_assert(std::is_base_of<ECChange, T>::value, "T not derived from ECChange");

            auto itor = m_changes.find(attributeId);
            if (itor != m_changes.end())
                return *(static_cast<T*>(itor->second.get()));

            ECChange::Ptr ptr = ECChange::Ptr(new T(GetState()));
            ECChange* p = ptr.get();
            m_changes[attributeId] = std::move(ptr);
            return *(static_cast<T*>(p));
            }

    public:
        ECChangeObject(ChangeState state)
            :ECChange(state)
            {}
    };



template<typename T>
struct ECChangeArray : ECChange
    {
    private:
        std::vector<ECChange::Ptr> m_changes;
        virtual bool _IsEmpty() const override
            {
            for (ECChange::Ptr const& change : m_changes)
                {
                if (!change->IsEmpty())
                    return false;
                }

            return true;
            }
        virtual void _Optimize() override
            {
            for (auto itor = m_changes.begin(); itor != m_changes.end(); ++itor)
                {
                if ((*itor)->IsEmpty())
                    {
                    itor = m_changes.erase(itor);
                    }
                }
            }
    public:
        ECChangeArray(ChangeState state)
            :ECChange(state)
            {
            static_assert(std::is_base_of<ECChange, T>::value, "T not derived from ECChange");
            }
        virtual ~ECChangeArray() {}
       
        T& Add(ChangeState state)
            {
            ECChange::Ptr ptr = ECChange::Ptr(new T(state));
            BeAssert(dynamic_cast<T*>(ptr.get()) != nullptr);

            T* p = static_cast<T*>(ptr.get());
            m_changes.push_back(std::move(ptr));
            return *p;
            }
        T& At(size_t index)
            {
            return static_cast<T&>(*m_changes[index]);
            }
        size_t Count() const { return m_changes.size(); }
        bool Empty() const { return m_changes.empty(); }
        void Remove(size_t index)
            {
            m_changes.erase(m_changes.begin() + index);
            }
    };
struct ECSchemaChanges : ECChangeArray<ECSchemaChange> 
    {
    public:
        ECSchemaChanges(ChangeState state)
            : ECChangeArray<ECSchemaChange>(state)
            {}
        virtual ~ECSchemaChanges(){}
    };
struct ECClassChanges : ECChangeArray<ECClassChange> 
    {
    public:
        ECClassChanges(ChangeState state)
            : ECChangeArray<ECClassChange>(state)
            {}
        virtual ~ECClassChanges() {}
    }; 
struct ECEnumerationChanges : ECChangeArray<ECEnumerationChange>
    {
    public:
        ECEnumerationChanges(ChangeState state)
            : ECChangeArray<ECEnumerationChange>(state)
            {}
        virtual ~ECEnumerationChanges() {}
    }; 
struct ECKindOfQuantityChanges : ECChangeArray<ECKindOfQuantityChange>
    {
    public:
        ECKindOfQuantityChanges(ChangeState state)
            : ECChangeArray<ECKindOfQuantityChange>(state)
            {}
        virtual ~ECKindOfQuantityChanges() {}
    };
struct ECInstanceChanges : ECChangeArray<ECInstanceChange>
    {
    public:
        ECInstanceChanges(ChangeState state)
            : ECChangeArray<ECInstanceChange>(state)
            {}
        virtual ~ECInstanceChanges() {}
    };
struct StringChanges : ECChangeArray<StringChange>
    {
    public:
        StringChanges(ChangeState state)
            : ECChangeArray<StringChange>(state)
            {}
        virtual ~StringChanges() {}
    };
struct ECPropertyChanges : ECChangeArray<ECPropertyChange>
    {
    public:
        ECPropertyChanges(ChangeState state)
            : ECChangeArray<ECPropertyChange>(state)
            {}
        virtual ~ECPropertyChanges() {}
    };
struct ECRelationshipConstraintClassChanges : ECChangeArray<ECRelationshipConstraintClassChange>
    {
    public:
        ECRelationshipConstraintClassChanges(ChangeState state)
            : ECChangeArray<ECRelationshipConstraintClassChange>(state)
            {}
        virtual ~ECRelationshipConstraintClassChanges() {}
    };
struct ECEnumeratorChanges : ECChangeArray<ECEnumeratorChange>
    {
    public:
        ECEnumeratorChanges(ChangeState state)
            : ECChangeArray<ECEnumeratorChange>(state)
            {}
        virtual ~ECEnumeratorChanges() {}
    };

template<typename T>
struct ECPrimitiveChange : ECChange 
    {
    private:
        Nullable<T> m_old;
        Nullable<T> m_new;
        virtual bool _IsEmpty() const override
            {
            return m_old == nullptr && m_new == nullptr;
            }
    public:
        ECPrimitiveChange(ChangeState state)
            : ECChange(state)
            {}        
        virtual ~ECPrimitiveChange(){}
        Nullable<T> const& GetValue(ValueId id) const
            {
            if (id == ValueId::New)
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
        BentleyStatus SetValue(ValueId id, Nullable<T> const&& value)
            {
            if (id == ValueId::Deleted)
                {
                if (GetState() == ChangeState::New)
                    {
                    BeAssert(false && "Cannot set OLD value for Change which is marked as NEW");
                    return ERROR:
                    }

                m_old = std::move(value);
                }
            else
                {
                if (GetState() == ChangeState::Deleted)
                    {
                    BeAssert(false && "Cannot set NEW value for Change which is marked DELETED");
                    return ERROR:
                    }

                m_new = std::move(value);
                }
            return SUCCESS;
            }
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
        BentleyStatus SetValue(Nullable<T> const&& valueOld, Nullable<T> const&& valueNew)
            {
            if (GetState() != ChangeState::Modified)
                {
                BeAssert(false && "Cannot set both OLD and NEW for change that is not makred as MODIFIED");
                return ERROR:
                }

            m_old = std::move(valueOld);
            m_new = std::move(valueNew);
            return SUCCESS;
            }
    };
struct StringChange : ECPrimitiveChange<Utf8String>
    {
    public:
        StringChange(ChangeState state)
            : ECPrimitiveChange<Utf8String>(state)
            {}
        virtual ~StringChange() {}
    };
struct BooleanChange : ECPrimitiveChange<bool>
    {
    public:
        BooleanChange(ChangeState state)
            : ECPrimitiveChange<bool>(state)
            {}
        virtual ~BooleanChange() {}
    };
struct UInt32Change : ECPrimitiveChange<uint32_t>
    {
    public:
        UInt32Change(ChangeState state)
            : ECPrimitiveChange<uint32_t>(state)
            {}
        virtual ~UInt32Change() {}
    };
struct Int32Change : ECPrimitiveChange<int32_t>
    {
    public:
        Int32Change(ChangeState state)
            : ECPrimitiveChange<int32_t>(state)
            {}
        virtual ~Int32Change() {}
    };
struct DoubleChange: ECPrimitiveChange<double>
    {
    public:
        DoubleChange(ChangeState state)
            : ECPrimitiveChange<double>(state)
            {}
        virtual ~DoubleChange() {}
    };
struct DateTimeChange: ECPrimitiveChange<DateTime>
    {
    public:
        DateTimeChange(ChangeState state)
            : ECPrimitiveChange<DateTime>(state)
            {}
        virtual ~DateTimeChange() {}
    };
struct BinaryChange: ECPrimitiveChange<Binary>
    {
    public:
        BinaryChange(ChangeState state)
            : ECPrimitiveChange<Binary>(state)
            {}
        virtual ~BinaryChange() {}
    };
struct Point2DChange: ECPrimitiveChange<Point2d>
    {
    public:
        Point2DChange(ChangeState state)
            : ECPrimitiveChange<Point2d>(state)
            {}
        virtual ~Point2DChange() {}
    };
struct Point3DChange: ECPrimitiveChange<Point3d>
    {
    public:
        Point3DChange(ChangeState state)
            : ECPrimitiveChange<Point3d>(state)
            {}
        virtual ~Point3DChange() {}
    };
struct Int64Change: ECPrimitiveChange<int64_t>
    {
    public:
        Int64Change(ChangeState state)
            : ECPrimitiveChange<int64_t>(state)
            {}
        virtual ~Int64Change() {}
    };

struct PropertyTypeChange : ECPrimitiveChange<PropertyType>
    {
    public:
        PropertyTypeChange(ChangeState state)
            : ECPrimitiveChange<PropertyType>(state)
            {}
        virtual ~PropertyTypeChange() {}
    };
struct StrengthTypeChange : ECPrimitiveChange<ECN::StrengthType>
    {
    public:
        StrengthTypeChange(ChangeState state)
            : ECPrimitiveChange<ECN::StrengthType>(state)
            {}
        virtual ~StrengthTypeChange() {}
    };
struct StrengthDirectionChange : ECPrimitiveChange<ECN::ECRelatedInstanceDirection>
    {
    public:
        StrengthDirectionChange(ChangeState state)
            : ECPrimitiveChange<ECN::ECRelatedInstanceDirection>(state)
            {}
        virtual ~StrengthDirectionChange() {}
    };
struct ModifierChange :ECPrimitiveChange<ECN::ECClassModifier>
    {
    public:
        ModifierChange(ChangeState state)
            : ECPrimitiveChange<ECN::ECClassModifier>(state)
            {}
        virtual ~ModifierChange() {}
    };
struct ECSchemaChange : ECChangeObject
    {
    public:
        ECSchemaChange(ChangeState state): ECChangeObject(state){}
        virtual ~ECSchemaChange(){}
        StringChange& GetName() { return Get<StringChange>(NAME); }
        StringChange& GetDisplayLabel() { return Get<StringChange>(DISPLAYLABEL); }
        StringChange& GetDescription() { return Get<StringChange>(DESCRIPTION); }
        UInt32Change& GetVersionMajor() { return Get<UInt32Change>(VERSIONMAJOR); }
        UInt32Change& GetVersionMinor() { return Get<UInt32Change>(VERSIONMINOR); }
        UInt32Change& GetVersionWrite() { return Get<UInt32Change>(VERSIONWRITE); }
        StringChange& GetNamespacePrefix() { return Get<StringChange>(NAMESPACEPREFIX); }
        StringChanges& References() { return Get<StringChanges>(REFERENCES); }
        ECClassChanges& Classes() { return Get<ECClassChanges>(CLASSES); }
        ECEnumerationChanges& Enumerations() { return Get<ECEnumerationChanges>(ENUMERATIONS); }
        ECKindOfQuantityChanges& KindOfQuantities() { return Get<ECKindOfQuantityChanges>(KINDOFQUANTITIES); }
        ECInstanceChanges& CustomAttributes() { return Get<ECInstanceChanges>(CUSTOMATTRIBTUES); }
    };
struct ECEnumeratorChange :ECChangeObject
    {
    public:
        ECEnumeratorChange(ChangeState state) : ECChangeObject(state) {}
        virtual ~ECEnumeratorChange() {}
        StringChange& GetDisplayLabel() { return Get<StringChange>(DISPLAYLABEL); }
        StringChange& GetDescription() { return Get<StringChange>(DESCRIPTION); }
        StringChange& GetString() { return Get<StringChange>(STRING); }
        Int32Change& GetInteger() { return Get<Int32Change>(INTEGER); }
    };
struct ECEnumerationChange :ECChangeObject
    {
    public:
        ECEnumerationChange(ChangeState state) : ECChangeObject(state) {}
        virtual ~ECEnumerationChange() {}
        StringChange& GetName() { return Get<StringChange>(NAME); }
        StringChange& GetDisplayLabel() { return Get<StringChange>(DISPLAYLABEL); }
        StringChange& GetDescription() { return Get<StringChange>(DESCRIPTION); }
        StringChange& GetTypeName() { return Get<StringChange>(TYPENAME); }
        BooleanChange& IsStrict() { return Get<BooleanChange>(ISSTRICT); }
        ECEnumeratorChanges& Enumerators() { return Get<ECEnumeratorChanges>(ENUMERATORS); }
    };
struct ECPropertyValueChange :ECChange
    {
    private:
        std::map<Utf8String, ECChange::Ptr> m_changes;
        ECValue m_value;
        Utf8String m_id;
        bool m_array;
        virtual bool _IsEmpty() const override
            {
            for (auto& change : m_changes)
                {
                if (!change.second->IsEmpty())
                    return false;
                }

            return true;
            }
        virtual void _Optimize() override
            {
            for (auto itor = m_changes.begin(); itor != m_changes.end(); ++itor)
                {
                if (itor->second->IsEmpty())
                    itor = m_changes.erase(itor);
                }
            }
    protected:
        template<typename T>
        Utf8StringCR GetId() const {return m_id;}

    public:
        ECPropertyValueChange(ChangeState state, Utf8CP id)
            :ECChange(state),m_id(id)
            {}
  
        ECPropertyValueChange& Get(Utf8CP attributeId)
            {
            m_value = ECValue();
            auto itor = m_changes.find(attributeId);
            if (itor != m_changes.end())
                return *(static_cast<ECPropertyValueChange*>(itor->second.get()));

            ECChange::Ptr ptr = ECChange::Ptr(new ECPropertyValueChange(GetState(), attributeId));
            ECChange* p = ptr.get();
            m_changes[attributeId] = std::move(ptr);
            return *(static_cast<ECPropertyValueChange*>(p));
            }
        bool  HasChildren() const {return !m_changes.empty(); }
        bool HasValue() const {return !HasChildren(); }
        std::vector<ECPropertyValueChange*> ChildrenR()
            {
            std::vector<ECPropertyValueChange*> temp;
            for (auto& kp : m_changes)
                {
                temp.push_back(static_cast<ECPropertyValueChange*>(kp.second.get()));
                }
            return temp;
            }
        const std::vector<ECPropertyValueChange const*> Children() const
            {
            std::vector<ECPropertyValueChange const*> temp;
            for (auto& kp : m_changes)
                {
                temp.push_back(static_cast<ECPropertyValueChange*>(kp.second.get()));
                }

            return temp;
            }
        void SetValue(ValueId appendType, ECValueCR value)
            {
            m_changes.clear();
            m_value = value;
            }
    };
struct ECInstanceChange:ECChange
    {
    private:
        ECPropertyValueChange m_instance;
        StringChange m_className;
       
        virtual bool _IsEmpty() const override
            {
            return m_className.IsEmpty();
            }
        virtual void _Optimize() override
            {
            m_instance.Optimize();
            }

    public:
        ECInstanceChange(ChangeState state)
            :ECChange(state), m_className(state), m_instance(state, "$")
            {}
        virtual ~ECInstanceChange(){}
        StringChange& ClassName() 
            {
            return m_className;
            }
        ECPropertyValueChange& Instance() {return m_instance;}
    };

struct ECKindOfQuantityChange :ECChangeObject
    {
    public:
        ECKindOfQuantityChange(ChangeState state) : ECChangeObject(state) {}
        virtual ~ECKindOfQuantityChange() {}
        StringChange& GetName() { return Get<StringChange>(NAME); }
        StringChange& GetDisplayLabel() { return Get<StringChange>(DISPLAYLABEL); }
        StringChange& GetDescription() { return Get<StringChange>(DESCRIPTION); }
        StringChange& GetDefaultPresentationUnit() { return Get<StringChange>(DEFAULTPRESENTATIONUNIT); }
        StringChange& GetPersistenceUnit() { return Get<StringChange>(PERSISTENCEUNIT); }
        UInt32Change& GetPrecision() { return Get<UInt32Change>(PRECISION); }
        StringChanges& GetAlternativePresentationUnitList() { return Get<StringChanges>(ALTERNATIVEPRESENTATIONUNITLIST); }
    };
struct ECRelationshipConstraintClassChange :ECChangeObject
    {
    public:
        ECRelationshipConstraintClassChange(ChangeState state) : ECChangeObject(state) {}
        virtual ~ECRelationshipConstraintClassChange() {}
        StringChange& GetClassName() { return Get<StringChange>(CONSTRAINTCLASS); }
        StringChanges& KeyProperties() { return Get<StringChanges>(KEYPROPERTIES); }
    };
struct ECRelationshipConstraintChange :ECChangeObject
    {
    public:
        ECRelationshipConstraintChange(ChangeState state) : ECChangeObject(state) {}
        virtual ~ECRelationshipConstraintChange() {}
        StringChange& GetRoleLabel() { return Get<StringChange>(ROLELABEL); }
        StringChange& GetCardinality() { return Get<StringChange>(CARDINALITY); }
        BooleanChange& IsPolymorphic() { return Get<BooleanChange>(ISPOLYMORPHIC); }

        ECRelationshipConstraintClassChanges& ConstraintClasses() { return Get<ECRelationshipConstraintClassChanges>(CONSTRAINTCLASSES); }
        ECInstanceChanges& CustomAttributes() { return Get<ECInstanceChanges>(CUSTOMATTRIBTUES); }
    };
struct ECRelationshipChange :ECChangeObject
    {
    public:
        ECRelationshipChange(ChangeState state) : ECChangeObject(state) {}
        virtual ~ECRelationshipChange() {}

        StrengthTypeChange& GetStrength() { return Get<StrengthTypeChange>(STRENGTHTYPE); }
        StrengthDirectionChange& GetStrengthDirection() { return Get<StrengthDirectionChange>(STRENGTHDIRECTION); }
        ECRelationshipConstraintChange& GetSource() { return Get<ECRelationshipConstraintChange>(SOURCE); }
        ECRelationshipConstraintChange& GetTarget() { return Get<ECRelationshipConstraintChange>(TARGET); }
    };
struct ECClassChange :ECChangeObject
    {
    public:
        ECClassChange(ChangeState state) : ECChangeObject(state) {}
        virtual ~ECClassChange() {}
        StringChange& GetName() { return Get<StringChange>(NAME); }
        StringChange& GetDisplayLabel() { return Get<StringChange>(DISPLAYLABEL); }
        StringChange& GetDescription() { return Get<StringChange>(DESCRIPTION); }
        ModifierChange& GetModifier() { return Get<ModifierChange>(MODIFIER); }
        BooleanChange& IsCustomAttributeClass() { return Get<BooleanChange>(ISCUSTOMATTRIBUTECLASS); }
        BooleanChange& IsEntityClass() { return Get<BooleanChange>(ISENTITYCLASS); }
        BooleanChange& IsStructClass() { return Get<BooleanChange>(ISSTRUCTCLASS); }
        BooleanChange& IsRelationshipClass() {
            return Get<BooleanChange>(ISRELATIONSHIPCLASS);
            }

        ECPropertyChanges& Properties() { return Get<ECPropertyChanges>(PROPERTIES); }
        ECInstanceChanges& CustomAttributes() { return Get<ECInstanceChanges>(CUSTOMATTRIBTUES); }
        ECRelationshipChange& GetRelationship() { return Get<ECRelationshipChange>(RELATIONSHIP); }
    };
struct NavigationChange :ECChangeObject
    {
    public:
        NavigationChange(ChangeState state) : ECChangeObject(state) {}
        virtual ~NavigationChange() {}
        StrengthDirectionChange& Direction() { return Get<StrengthDirectionChange>(DIRECTION); }
        StringChange& GetRelationshipClassName() { return Get<StringChange>(RELATIONSHIPNAME); }
    };
struct ArrayChange :ECChangeObject
    {
    public:
        ArrayChange(ChangeState state) : ECChangeObject(state) {}
        virtual ~ArrayChange() {}
        UInt32Change& MinOccurs() { return Get<UInt32Change>(MINOCCURS); }
        UInt32Change& MaxOccurs() { return Get<UInt32Change>(MAXOCCURS); }
    };
struct ECPropertyChange :ECChangeObject
    {
    public:
        ECPropertyChange(ChangeState state) : ECChangeObject(state) {}
        virtual ~ECPropertyChange() {}
        StringChange& GetName() { return Get<StringChange>(NAME); }
        StringChange& GetDisplayLabel() { return Get<StringChange>(DISPLAYLABEL); }
        StringChange& GetDescription() { return Get<StringChange>(DESCRIPTION); }
        StringChange& GetTypeName() { return Get<StringChange>(TYPENAME); }
        StringChange& GetMaximumValue() { return Get<StringChange>(MAXIMUMVALUE); }
        StringChange& GetMinimumValue() { return Get<StringChange>(MINIMUMVALUE); }
        PropertyTypeChange& GetPropertyType() { return Get<PropertyTypeChange>(PROPERTYTYPE); }
        ArrayChange& GetArray() { return Get<ArrayChange>(ARRAY); }
        NavigationChange& GetNavigation() { return Get<NavigationChange>(ARRAY); }
        StringChange& GetExtendedTypeName() { return Get<StringChange>(EXTENDEDTYPENAME); }
        BooleanChange& IsReadonly() { return Get<BooleanChange>(ISREADONLY); }
        ECInstanceChanges& CustomAttributes() { return Get<ECInstanceChanges>(CUSTOMATTRIBTUES); }
    };

struct ECSchemaComparer
    {
    BentleyStatus CompareECSchema(ECSchemaChange& change, ECSchemaCR a, ECSchemaCR b);
    BentleyStatus CompareECClass(ECClassChange& change, ECClassCR a, ECClassCR b);
    BentleyStatus CompareECRelationshipClass(ECRelationshipChange& change, ECRelationshipClassCR a, ECRelationshipClassCR b);
    BentleyStatus CompareECProperty(ECPropertyChange& change, ECPropertyCR a, ECPropertyCR b);
    BentleyStatus CompareECInstance(ECInstanceChange& change, IECInstanceCR a, IECInstanceCR b);
    BentleyStatus AppendECSchema(ECSchemaChanges& changes, ECSchemaCR v, ValueId appendType)
        {
        ChangeState state = appendType == ValueId::New ? ChangeState::New : ChangeState::Deleted;
        ECSchemaChange& change = changes.Add(state);

        change.GetName().SetValue(appendType,  v.GetName());
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

        for (KindOfQuantityCP kindOfQuantityCP : v.GetKindOfQuantities())
            if (AppendKindOfQuantity(change.KindOfQuantities(), *kindOfQuantityCP, appendType) == ERROR)
                return ERROR;

        return AppendCustomAttributes(change.CustomAttributes(), v, appendType);       
        }
    BentleyStatus AppendECClass(ECClassChanges& changes, ECClassCR v, ValueId appendType)
        {
        ChangeState state = appendType == ValueId::New ? ChangeState::New : ChangeState::Deleted;
        ECClassChange& change = changes.Add(state);

        change.GetName().SetValue(appendType, v.GetName());
        change.GetDisplayLabel().SetValue(appendType, v.GetDisplayLabel());
        change.GetDescription().SetValue(appendType, v.GetDescription());
        change.GetModifier().SetValue(appendType, v.GetClassModifier());
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

        return AppendCustomAttributes(change.CustomAttributes(), v, appendType);
        }
    BentleyStatus AppendECRelationshipClass(ECRelationshipChange& change, ECRelationshipClassCR v, ValueId appendType)
        {
        change.GetStrengthDirection().SetValue(appendType, v.GetStrengthDirection());
        change.GetStrength().SetValue(appendType, v.GetStrength());
        if (AppendECRelationshipConstraint(change.GetSource(), v.GetSource(), appendType) == ERROR)
            return ERROR;

        if (AppendECRelationshipConstraint(change.GetTarget(), v.GetTarget(), appendType) == ERROR)
            return ERROR;

        return SUCCESS;
        }
    BentleyStatus AppendECRelationshipConstraint(ECRelationshipConstraintChange& change, ECRelationshipConstraintCR v, ValueId appendType)
        {
        change.GetRoleLabel().SetValue(appendType, v.GetRoleLabel());
        change.GetCardinality().SetValue(appendType, v.GetCardinality().ToString());
        change.IsPolymorphic().SetValue(appendType, v.GetIsPolymorphic());
        if (AppendECRelationshipConstraintClasses(change.ConstraintClasses(), v.GetConstraintClasses(), appendType) != SUCCESS)
            return ERROR;

        return AppendCustomAttributes(change.CustomAttributes(), v, appendType);
        }
    BentleyStatus AppendECRelationshipConstraintClasses(ECRelationshipConstraintClassChanges& changes, ECRelationshipConstraintClassList const& v, ValueId appendType)
        {
        ChangeState state = appendType == ValueId::New ? ChangeState::New : ChangeState::Deleted;
        for (ECRelationshipConstraintClassCP constraintClass : v)
            {
            ECRelationshipConstraintClassChange & constraintClassChange = changes.Add(state);
            constraintClassChange.GetClassName().SetValue(appendType, constraintClass->GetClass().GetFullName());
            for (Utf8StringCR relationshipKeyProperty : constraintClass->GetKeys())
                {
                if (!relationshipKeyProperty.empty())
                    constraintClassChange.KeyProperties().Add(state).SetValue(appendType, relationshipKeyProperty);
                }
            }

        return SUCCESS;
        }
    BentleyStatus AppendECEnumeration(ECEnumerationChanges& changes, ECEnumerationCR v, ValueId appendType)
        {
        ChangeState state = appendType == ValueId::New ? ChangeState::New : ChangeState::Deleted;
        ECEnumerationChange& enumerationChange = changes.Add(state);
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
    BentleyStatus AppendKindOfQuantity(ECKindOfQuantityChanges& changes, KindOfQuantityCR v, ValueId appendType)
        {
        ChangeState state = appendType == ValueId::New ? ChangeState::New : ChangeState::Deleted;
        ECKindOfQuantityChange& kindOfQuantityChange = changes.Add(state);
        kindOfQuantityChange.GetName().SetValue(appendType, v.GetName());
        kindOfQuantityChange.GetDisplayLabel().SetValue(appendType, v.GetDisplayLabel());
        kindOfQuantityChange.GetDescription().SetValue(appendType, v.GetDescription());
        kindOfQuantityChange.GetPersistenceUnit().SetValue(appendType, v.GetPersistenceUnit());
        kindOfQuantityChange.GetPrecision().SetValue(appendType, v.GetPrecision());
        kindOfQuantityChange.GetDefaultPresentationUnit().SetValue(appendType, v.GetDefaultPresentationUnit());
        for (Utf8StringCR unitstr : v.GetAlternativePresentationUnitList())
            {
            kindOfQuantityChange.GetAlternativePresentationUnitList().Add(state).SetValue(appendType, unitstr);
            }
              
        return SUCCESS;
        } 
    BentleyStatus AppendECProperty(ECPropertyChanges& changes, ECPropertyCR v, ValueId appendType)
        {
        ChangeState state = appendType == ValueId::New ? ChangeState::New : ChangeState::Deleted;
        ECPropertyChange& propertyChange = changes.Add(state);
        propertyChange.GetName().SetValue(appendType, v.GetName());
        propertyChange.GetDisplayLabel().SetValue(appendType, v.GetDisplayLabel());
        propertyChange.GetDescription().SetValue(appendType, v.GetDescription());
        propertyChange.GetTypeName().SetValue(appendType, v.GetTypeName());
   
        if (auto prop = v.GetAsExtendedTypeProperty())
            {
            propertyChange.GetPropertyType().SetValue(appendType, PropertyType::ExtendedType);
            propertyChange.GetExtendedTypeName().SetValue(appendType, prop->GetExtendedTypeName());
            }
        else if (auto prop = v.GetAsNavigationProperty())
            {
            propertyChange.GetPropertyType().SetValue(appendType, PropertyType::Navigation);
            NavigationChange& navigationChange = propertyChange.GetNavigation();
            navigationChange.Direction().SetValue(appendType, prop->GetDirection());
            if (prop->GetRelationshipClass())
                navigationChange.GetRelationshipClassName().SetValue(appendType, prop->GetRelationshipClass()->GetFullName());           
            }
        else if (v.GetIsPrimitive())
            {
            propertyChange.GetPropertyType().SetValue(appendType, PropertyType::Primitive);
            }
        else if (v.GetIsStruct())
            {
            propertyChange.GetPropertyType().SetValue(appendType, PropertyType::Struct);
            }
        else if (v.GetIsStructArray())
            {
            propertyChange.GetPropertyType().SetValue(appendType, PropertyType::StructArray);
            }
        else if (v.GetIsPrimitiveArray())
            {
            propertyChange.GetPropertyType().SetValue(appendType, PropertyType::PrimitiveArray);
            }
        else
            {
            return ERROR;
            }

        if (v.IsMaximumValueDefined())
            propertyChange.GetMaximumValue().SetValue(appendType, v.GetMaximumValue());

        if (v.IsMinimumValueDefined())
            propertyChange.GetMinimumValue().SetValue(appendType, v.GetMinimumValue());

        if (v.GetIsArray())
            {        
            auto arrayProp = v.GetAsArrayProperty();
            propertyChange.GetArray().MaxOccurs().SetValue(appendType, arrayProp->GetMaxOccurs());
            propertyChange.GetArray().MinOccurs().SetValue(appendType, arrayProp->GetMinOccurs());
            }

        return SUCCESS;
        }
    BentleyStatus AppendECInstance(ECInstanceChanges& changes, IECInstanceCR v, ValueId appendType)
        {
        ECValuesCollectionPtr propertyValues = ECValuesCollection::Create(v);
        if (propertyValues.IsNull())
            return SUCCESS;

        ChangeState state = appendType == ValueId::New ? ChangeState::New : ChangeState::Deleted;
        ECInstanceChange& instanceChange = changes.Add(state);
        instanceChange.ClassName().SetValue(appendType, v.GetClass().GetFullName());
        return AppendPropertyValues(instanceChange.Instance(), *propertyValues, appendType);
        }
    BentleyStatus AppendCustomAttributes(ECInstanceChanges& changes, IECCustomAttributeContainerCR v, ValueId appendType)
        {
        for (auto& instance : v.GetPrimaryCustomAttributes(false))
            {
            if (AppendECInstance(changes, *instance, appendType) != SUCCESS)
                return ERROR;
            }

        return SUCCESS;
        }
    BentleyStatus AppendPropertyValues(ECPropertyValueChange& parentChange, ECValuesCollectionCR values, ValueId appendType)
        {
        for (ECValuesCollection::const_iterator itor = values.begin(); itor != values.end(); ++itor)
            {
            ECValueAccessorCR valueAccessor = (*itor).GetValueAccessor();
            const Utf8String propertyName = valueAccessor.GetPropertyName();
            ECPropertyValueChange& valueChange = parentChange.Get(propertyName.c_str());
            
            if ((*itor).HasChildValues())
                AppendPropertyValues(valueChange, *(*itor).GetChildValues(), appendType);
            else
                {
                ECValueCR v = (*itor).GetValue();
                if (v.IsNull())
                    continue;

                if (v.IsPrimitive())
                    {
                    valueChange.SetValue(appendType, v);
                    }
                }
            }

        return SUCCESS;
        }

    };












//struct Editor
//    {
//
//    struct SchemaFinalizationInfo
//        {
//        private:
//            DateTime m_finalizationDate;
//            Utf8String m_schemaFullNameKey;
//        public:
//            DateTimeCR GetFinalizationDate() const { return m_finalizationDate; }
//            Utf8StringCR GetSchemaFullNameKey () const { return m_schemaFullNameKey; }
//        };
//    struct SchemaChangeTrackingState
//        {
//        typedef std::unique_ptr<SchemaChangeTrackingState> Ptr;
//        private:
//            bool m_isFinalizedForRelease;
//            Utf8String m_schemaFullNameAtEditingStart;
//            bool m_trackingEnabled;
//            std::vector<SchemaFinalizationInfo> m_finalizationHistory;
//        public:
//            bool IsFinalizedForRelease() const { return m_isFinalizedForRelease; }
//            Utf8StringCR GetSchemaFullNameAtEditingStart () const { return m_schemaFullNameAtEditingStart; }
//            bool GetTrackingEnabled() const { return m_trackingEnabled; }
//            std::vector<SchemaFinalizationInfo> const& GetFinalizationHistory() const { return m_finalizationHistory; }
//        };
//    struct PreviousName
//        {
//        private:
//            Utf8String m_oldName;
//            Utf8String m_schemaFullNameKey;
//            DateTime m_date;
//            bool m_isNewInThisVersion;
//        public:
//            PreviousName(Utf8CP oldName, Utf8CP schemaFullKey, bool isnewInThisVersion)
//                :m_oldName(oldName), m_schemaFullNameKey(schemaFullKey), m_date(DateTime::GetCurrentTimeUtc()), m_isNewInThisVersion(isnewInThisVersion)
//                {}
//            PreviousName(Utf8CP oldName, Utf8CP schemaFullKey, DateTimeCR date, bool isnewInThisVersion)
//                :m_oldName(oldName), m_schemaFullNameKey(schemaFullKey), m_date(date), m_isNewInThisVersion(isnewInThisVersion)
//                {}
//            Utf8StringCR GetOldName() const { return m_oldName; }
//            Utf8StringCR GetSchemaFullNameKey() const { return m_schemaFullNameKey; }
//            DateTimeCR GetDate() const { return m_date; }
//            bool IsNewInThisVersion() const { return m_isNewInThisVersion; }
//        };
//    struct PreviousNameArray
//        {
//        typedef std::unique_ptr<PreviousNameArray> Ptr;
//        private:
//            std::vector<PreviousName> m_previousNames;
//            bool m_previousNameAlreadySaved;
//        public:
//            PreviousNameArray()
//                :m_previousNameAlreadySaved(false)
//                {
//                }
//         
//            bool GetPreviousNameAlreadySaved() const { return m_previousNameAlreadySaved; }
//            std::vector<PreviousName> const& GetPreviousNames() const { return m_previousNames; }
//            void Rename(SchemaChangeTrackingState const& changeTrackingState, Utf8CP oldName, bool isnewInThisVersion)
//                {
//                m_previousNames.push_back(PreviousName(oldName, changeTrackingState.GetSchemaFullNameAtEditingStart().c_str(), isnewInThisVersion));
//                }
//            bool Contains(Utf8CP oldName);       
//            static Ptr From(IECCustomAttributeContainerCR container)
//                {
//                IECInstancePtr ca = container.GetPrimaryCustomAttributeLocal("PreviousNameArray");
//                if (ca.IsNull())
//                    return nullptr;
//
//                
//                Ptr ptr = Ptr(new PreviousNameArray());
//                ECValue v;
//                if (ca->GetValue(v, "PreviousNameAlreadySaved") != ECObjectsStatus::Success)
//                    return nullptr;
//
//                ptr->m_previousNameAlreadySaved = v.GetBoolean();
//
//                if (ca->GetValue(v, "PreviousNames") != ECObjectsStatus::Success)
//                    return nullptr;
//
//                for (uint32_t i = 0; i < v.GetArrayInfo().GetCount(); i++)
//                    {
//                    Utf8String name, schemaKey;
//                    DateTime date;
//                    bool isNewInThisVesrion;
//
//                    if (ca->GetValue(v, "PreviousNames.OldName", i) != ECObjectsStatus::Success)
//                        return nullptr;
//                    name = v.GetUtf8CP();
//
//                    if (ca->GetValue(v, "PreviousNames.SchemaFullNameKey", i) != ECObjectsStatus::Success)
//                        return nullptr;
//
//                    schemaKey = v.GetUtf8CP();
//                    if (ca->GetValue(v, "PreviousNames.Date", i) != ECObjectsStatus::Success)
//                        return nullptr;
//
//                    date = v.GetDateTime();
//                    if (ca->GetValue(v, "PreviousNames.IsNewInThisVersion", i) != ECObjectsStatus::Success)
//                        return nullptr;
//
//                    isNewInThisVesrion = v.GetBoolean();
//
//                    ptr->m_previousNames.push_back(PreviousName(name.c_str(), schemaKey.c_str(), date, isNewInThisVesrion));
//                    }
//
//                }
//        };
//
//    std::map<IECCustomAttributeContainerCP, PreviousNameArray::Ptr> m_previousNameArray;
//    std::map<ECSchemaCP, SchemaChangeTrackingState::Ptr> m_schemaChangeTrackingState;
//
//
//    SchemaChangeTrackingState* GetSchemaChangeTrackingState(ECSchemaCR schema)
//        {
//
//        }
//    PreviousNameArray* GetPreviousNameArray(IECCustomAttributeContainerCR container)
//        {
//        auto itor = m_previousNameArray.find(&container);
//        if (itor != m_previousNameArray.end())
//            return itor->second.get();
//
//        bool isSchema = dynamic_cast<ECSchemaCP>(&container) != nullptr;
//        bool isClass = dynamic_cast<ECClassCP>(&container) != nullptr;
//        bool isProperty = dynamic_cast<ECPropertyCP>(&container) != nullptr;
//        if (!isSchema && !isClass && !isProperty)
//            {
//            BeAssert(false && "Container is not supported");
//            return nullptr;
//            }
//        m_previousNameArray[&container] = PreviousNameArray::From(container);
//        return m_previousNameArray[&container].get();
//        }
//    };







END_BENTLEY_SQLITE_EC_NAMESPACE

