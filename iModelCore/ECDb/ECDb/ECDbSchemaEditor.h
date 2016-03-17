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
struct KindOfQuantityChange;
struct ECInstanceChange;
struct StringChange;
struct ECPropertyChange;
struct ECRelationshipConstraintClassChange;
struct ECEnumeratorChange;
struct ECPropertyValueChange;
struct ECObjectChange;
//=========================================

enum class ChangeState
    {
    Deleted = 1, //This need to be none zero base
    Modified =2 ,
    New =3,
    Unchange =4,
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
    public:
        typedef std::unique_ptr<ECChange> Ptr;
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
            ENUMERATION,
            ENUMERATIONS,
            ENUMERATOERS,
            ENUMERATOR,
            ENUMERATORS,
            EXTENDEDTYPENAME,
            INSTANCE,
            INSTANCES,
            INTEGER,
            ISCUSTOMATTRIBUTECLASS,
            ISENTITYCLASS,
            ISPOLYMORPHIC,
            ISREADONLY,
            ISRELATIONSHIPCLASS,
            ISSTRICT,
            ISSTRUCTCLASS,
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
    private:
        static std::map<Utf8CP, SystemId, CompareUtf8> const& GetStringToTypeMap()
            {
            static std::map<Utf8CP, SystemId, CompareUtf8> keyToId;
            if (keyToId.empty())
                {
                for (auto const& kp : GetTypeToStringMap())
                    {
                    keyToId[kp.second.c_str()] = kp.first;
                    }
                }

            return keyToId;
            }
        static std::map<SystemId, Utf8String> const& GetTypeToStringMap()
            {
            static std::map<SystemId, Utf8String> idToKey
                {
                        {NONE, ""},
                        {ALTERNATIVEPRESENTATIONUNITLIST, "AlternativePresentationUnitList"},
                        {ARRAY, "Array"},
                        {CARDINALITY, "Cardinality"},
                        {CLASSES, "Classes"},
                        {CLASSES, "Class"},
                        {CLASSFULLNAME, "ClassFullName"},
                        {CLASSMODIFIER, "ClassModifier"},
                        {CONSTRAINTCLASS, "ConstraintClass"},
                        {CONSTRAINTCLASSES, "ConstraintClasses"},
                        {CONSTRAINT, "Constraint"},
                        {CUSTOMATTRIBTUES, "CustomAttributes"},
                        {DEFAULTPRESENTATIONUNIT, "DefaultPresentationUnit"},
                        {DESCRIPTION, "Description"},
                        {DIRECTION, "Direction"},
                        {DISPLAYLABEL, "DisplayLabel"},
                        {ENUMERATION, "Enumeration"},
                        {ENUMERATIONS, "Enumerations"},
                        {ENUMERATOERS, "Enumerators"},
                        {ENUMERATOR, "Enumerator"},
                        {ENUMERATORS, "Enumerators"},
                        {EXTENDEDTYPENAME, "ExtendTypeName"},
                        {INSTANCE, "Instance"},
                        {INSTANCES, "Instances"},
                        {INTEGER, "Integer"},
                        {ISCUSTOMATTRIBUTECLASS, "IsCustomAttributeClass"},
                        {ISENTITYCLASS, "IsEntityClass"},
                        {ISPOLYMORPHIC, "IsPolymorphic"},
                        {ISREADONLY, "IsReadOnly"},
                        {ISRELATIONSHIPCLASS, "IsRelationshipClass"},
                        {ISSTRICT, "IsStict"},
                        {ISSTRUCTCLASS, "IsStructClass"},
                        {KEYPROPERTIES, "KeyProperties"},
                        {KEYPROPERTY, "KeyProperty"},
                        {KINDOFQUANTITIES, "KindOfQuantities"},
                        {KINDOFQUANTITY, "KindOfQuantity"},
                        {MAXIMUMVALUE, "MaximumValue"},
                        {MAXOCCURS, "MaxOccurs"},
                        {MINIMUMVALUE, "MinimumValue"},
                        {MINOCCURS, "MinOccurs"},
                        {NAME, "Name"},
                        {NAMESPACEPREFIX, "NameSpacePrefix"},
                        {NAVIGATION, "Navigation"},
                        {PERSISTENCEUNIT, "PersistenceUnit"},
                        {PRECISION, "Precision"},
                        {PROPERTIES, "Properties"},
                        {PROPERTY, "Property"},
                        {PROPERTYTYPE, "PropertyType"},
                        {REFERENCE, "Reference"},
                        {REFERENCES, "References"},
                        {RELATIONSHIP, "Relationship"},
                        {RELATIONSHIPNAME, "RelationshipName"},
                        {ROLELABEL, "RoleLabel"},
                        {SCHEMA, "Schema"},
                        {SCHEMAS, "Schemas"},
                        {SOURCE, "Source"},
                        {STRENGTHDIRECTION, "StrengthDirection"},
                        {STRENGTHTYPE, "StrengthType"},
                        {STRING, "String"},
                        {TARGET, "Target"},
                        {TYPENAME, "TypeName"},
                        {VERSIONMAJOR, "VersionMajor"},
                        {VERSIONMINOR, "VersionMinor"},
                        {VERSIONWRITE, "VersionWrite"},
                };

            return idToKey;
            }
     protected:
        static Utf8StringCR Convert(SystemId id)
            {
            std::map<SystemId, Utf8String> const& idToKey = GetTypeToStringMap();
            auto itor = idToKey.find(id);
            if (itor != idToKey.end())
                return itor->second;

            BeAssert(false && "Failed to convert to string");
            return idToKey.find(SystemId::NONE)->second;
            }
        static SystemId Convert(Utf8CP id)
            {
            std::map<Utf8CP, SystemId, CompareUtf8> const& keyToId = GetStringToTypeMap();
            auto itor = keyToId.find(id);
            if (itor != keyToId.end())
                return itor->second;

            BeAssert(false && "Failed to decode type id into string");
            return SystemId::NONE;
            }
    private:
        std::unique_ptr<Utf8String> m_customId;
        SystemId m_systemId;
        ECChange const* m_parent;
        ChangeState m_state;

        virtual bool _IsEmpty() const = 0;
        virtual void _Optimize() {}
        virtual bool _IsArray() const { return false; }
        virtual bool _IsObject() const { return false; }
        virtual bool _IsPrimitive() const { return false; }

    public:
        ECChange(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            :m_systemId(systemId), m_parent(parent), m_state(state)
            {
            if (customId != nullptr)
                m_customId = std::unique_ptr<Utf8String>(new Utf8String(customId));
            }
        virtual ~ECChange() = 0;

        SystemId GetSystemId() const { return m_systemId; }
        Utf8StringCR GetId() const
            {
            if (m_customId != nullptr)
                return *m_customId;

            return Convert(GetSystemId());
            }
        ChangeState GetState() const { return m_state; }
        ECChange const* GetParent() const { return m_parent; }
        bool IsEmpty() const { return _IsEmpty(); }
        void Optimize() { return _Optimize(); }
        bool IsArray() const { return _IsArray(); }
        bool IsObject() const { return _IsObject(); }
        bool IsPrimitive() const { return _IsPrimitive(); }

    };
struct ECObjectChange : ECChange
    {
    private:
        std::map<Utf8CP, ECChange::Ptr, CompareUtf8> m_changes;

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
        virtual bool _IsObject() const override
            {
            return true;
            }
    protected:
        template<typename T>
        T& Get(ECChange::SystemId systemId)
            {
            static_assert(std::is_base_of<ECChange, T>::value, "T not derived from ECChange");
            Utf8CP customId = Convert(systemId).c_str();
            auto itor = m_changes.find(customId);
            if (itor != m_changes.end())
                return *(static_cast<T*>(itor->second.get()));
            
            ECChange::Ptr ptr;
            
            ptr = ECChange::Ptr(new T(GetState(), systemId, this, nullptr));
            ECChange* p = ptr.get();
            m_changes[ptr->GetId().c_str()] = std::move(ptr);
            return *(static_cast<T*>(p));
            }
      
    public:
        ECObjectChange(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            :ECChange(state, systemId, parent, customId)
            {}
    };



template<typename T>
struct ECChangeArray : ECChange
    {
    private:
        std::vector<ECChange::Ptr> m_changes;
        SystemId m_elementType;
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
        virtual bool _IsArray() const override
            {
            return true;
            }

    public:
        ECChangeArray(ChangeState state, SystemId systemId, ECChange const* parent , Utf8CP customId , SystemId elementId)
            :ECChange(state, systemId, parent, customId), m_elementType(elementId)
            {
            static_assert(std::is_base_of<ECChange, T>::value, "T not derived from ECChange");
            }
        virtual ~ECChangeArray() {}
        SystemId GetElementType() const {return m_elementType;}
        T& Add(ChangeState state, Utf8CP customId = nullptr)
            {
            ECChange::Ptr ptr = ECChange::Ptr(new T(state, m_elementType, this, customId));
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
        void Erase(size_t index)
            {
            m_changes.erase(m_changes.begin() + index);
            }
    };

struct ECSchemaChanges : ECChangeArray<ECSchemaChange>
    {
    public:
        ECSchemaChanges(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECChangeArray<ECSchemaChange>(state, SCHEMAS, parent, customId, SCHEMA)
            {
            BeAssert(systemId == GetSystemId());
            }
        virtual ~ECSchemaChanges(){}
    };
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
struct ECKindOfQuantityChanges : ECChangeArray<KindOfQuantityChange>
    {
    public:
        ECKindOfQuantityChanges(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECChangeArray<KindOfQuantityChange>(state, KINDOFQUANTITIES, parent, customId, KINDOFQUANTITY)
            {
            BeAssert(systemId == GetSystemId());
            }
        virtual ~ECKindOfQuantityChanges() {}
    };
struct ECInstanceChanges : ECChangeArray<ECInstanceChange>
    {
    public:
        ECInstanceChanges(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECChangeArray<ECInstanceChange>(state, INSTANCES, parent, customId, INSTANCE)
            {
            BeAssert(systemId == GetSystemId());
            }
        virtual ~ECInstanceChanges() {}
    };
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
struct ECEnumeratorChanges : ECChangeArray<ECEnumeratorChange>
    {
    public:
        ECEnumeratorChanges(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECChangeArray<ECEnumeratorChange>(state, ENUMERATOERS, parent, customId, ENUMERATOR)
            {
            BeAssert(systemId == GetSystemId());
            }
        virtual ~ECEnumeratorChanges() {}
    };

struct StringChanges : ECChangeArray<StringChange>
    {
    public:
        StringChanges(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECChangeArray<StringChange>(state, systemId, parent, customId, STRING)
            {}
        virtual ~StringChanges() {}
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
        virtual bool _IsPrimitive() const override
            {
            return true;
            }
    public:
        ECPrimitiveChange(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECChange(state, systemId, parent, customId)
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
                return ERROR;
                }

            m_old = std::move(valueOld);
            m_new = std::move(valueNew);
            return SUCCESS;
            }
    };
struct StringChange : ECPrimitiveChange<Utf8String>
    {
    public:
        StringChange(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECPrimitiveChange<Utf8String>(state, systemId, parent, customId)
            {}
        virtual ~StringChange() {}
    };
struct BooleanChange : ECPrimitiveChange<bool>
    {
    public:
        BooleanChange(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECPrimitiveChange<bool>(state, systemId, parent, customId)
            {}
        virtual ~BooleanChange() {}
    };
struct UInt32Change : ECPrimitiveChange<uint32_t>
    {
    public:
        UInt32Change(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECPrimitiveChange<uint32_t>(state, systemId, parent, customId)
            {}
        virtual ~UInt32Change() {}
    };
struct Int32Change : ECPrimitiveChange<int32_t>
    {
    public:
        Int32Change(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECPrimitiveChange<int32_t>(state, systemId, parent, customId)
            {}
        virtual ~Int32Change() {}
    };
struct DoubleChange: ECPrimitiveChange<double>
    {
    public:
        DoubleChange(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECPrimitiveChange<double>(state, systemId, parent, customId)
            {}
        virtual ~DoubleChange() {}
    };
struct DateTimeChange: ECPrimitiveChange<DateTime>
    {
    public:
        DateTimeChange(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECPrimitiveChange<DateTime>(state, systemId, parent, customId)
            {}
        virtual ~DateTimeChange() {}
    };
struct BinaryChange: ECPrimitiveChange<Binary>
    {
    public:
        BinaryChange(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECPrimitiveChange<Binary>(state, systemId, parent, customId)
            {}
        virtual ~BinaryChange() {}
    };
struct Point2DChange: ECPrimitiveChange<Point2d>
    {
    public:
        Point2DChange(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECPrimitiveChange<Point2d>(state, systemId, parent, customId)
            {}
        virtual ~Point2DChange() {}
    };
struct Point3DChange: ECPrimitiveChange<Point3d>
    {
    public:
        Point3DChange(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECPrimitiveChange<Point3d>(state, systemId, parent, customId)
            {}
        virtual ~Point3DChange() {}
    };
struct Int64Change: ECPrimitiveChange<int64_t>
    {
    public:
        Int64Change(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECPrimitiveChange<int64_t>(state, systemId, parent, customId)
            {}
        virtual ~Int64Change() {}
    };
struct PropertyTypeChange : ECPrimitiveChange<PropertyType>
    {
    public:
        PropertyTypeChange(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECPrimitiveChange<PropertyType>(state, systemId, parent, customId)
            {}
        virtual ~PropertyTypeChange() {}
    };
struct StrengthTypeChange : ECPrimitiveChange<ECN::StrengthType>
    {
    public:
        StrengthTypeChange(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECPrimitiveChange<ECN::StrengthType>(state, systemId, parent, customId)
            {}
        virtual ~StrengthTypeChange() {}
    };
struct StrengthDirectionChange : ECPrimitiveChange<ECN::ECRelatedInstanceDirection>
    {
    public:
        StrengthDirectionChange(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECPrimitiveChange<ECN::ECRelatedInstanceDirection>(state, systemId, parent, customId)
            {}
        virtual ~StrengthDirectionChange() {}
    };
struct ModifierChange :ECPrimitiveChange<ECN::ECClassModifier>
    {
    public:
        ModifierChange(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECPrimitiveChange<ECN::ECClassModifier>(state, systemId, parent, customId)
            {}
        virtual ~ModifierChange() {}
    };


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
        StringChanges& References() { return Get<StringChanges>(REFERENCES); }
        ECClassChanges& Classes() { return Get<ECClassChanges>(CLASSES); }
        ECEnumerationChanges& Enumerations() { return Get<ECEnumerationChanges>(ENUMERATIONS); }
        ECKindOfQuantityChanges& KindOfQuantities() { return Get<ECKindOfQuantityChanges>(KINDOFQUANTITIES); }
        ECInstanceChanges& CustomAttributes() { return Get<ECInstanceChanges>(CUSTOMATTRIBTUES); }
    };
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
        StringChange& GetDescription() { return Get<StringChange>(DESCRIPTION); }
        StringChange& GetString() { return Get<StringChange>(STRING); }
        Int32Change& GetInteger() { return Get<Int32Change>(INTEGER); }
    };
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
struct ECInstanceChange: ECObjectChange
    {
    public:
        ECInstanceChange(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECObjectChange(state, INSTANCE, parent, customId)
            {
            BeAssert(systemId == GetSystemId());
            }
        virtual ~ECInstanceChange(){}
        StringChange& GetClassName() { return Get<StringChange>(CLASSFULLNAME); }
    };
struct KindOfQuantityChange :ECObjectChange
    {
    protected:

    public:
        KindOfQuantityChange(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECObjectChange(state, KINDOFQUANTITY, parent, customId)
            {
            BeAssert(systemId == GetSystemId());
            }
        virtual ~KindOfQuantityChange() {}
        StringChange& GetName() { return Get<StringChange>(NAME); }
        StringChange& GetDisplayLabel() { return Get<StringChange>(DISPLAYLABEL); }
        StringChange& GetDescription() { return Get<StringChange>(DESCRIPTION); }
        StringChange& GetDefaultPresentationUnit() { return Get<StringChange>(DEFAULTPRESENTATIONUNIT); }
        StringChange& GetPersistenceUnit() { return Get<StringChange>(PERSISTENCEUNIT); }
        UInt32Change& GetPrecision() { return Get<UInt32Change>(PRECISION); }
        StringChanges& GetAlternativePresentationUnitList() { return Get<StringChanges>(ALTERNATIVEPRESENTATIONUNITLIST); }
    };
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
struct ECRelationshipConstraintChange :ECObjectChange
    {
    public:
        ECRelationshipConstraintChange(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECObjectChange(state, CONSTRAINT, parent, customId)
            {
            BeAssert(systemId == GetSystemId());
            }
        virtual ~ECRelationshipConstraintChange() {}
        StringChange& GetRoleLabel() { return Get<StringChange>(ROLELABEL); }
        StringChange& GetCardinality() { return Get<StringChange>(CARDINALITY); }
        BooleanChange& IsPolymorphic() { return Get<BooleanChange>(ISPOLYMORPHIC); }

        ECRelationshipConstraintClassChanges& ConstraintClasses() { return Get<ECRelationshipConstraintClassChanges>(CONSTRAINTCLASSES); }
        ECInstanceChanges& CustomAttributes() { return Get<ECInstanceChanges>(CUSTOMATTRIBTUES); }
    };
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

        ECPropertyChanges& Properties() { return Get<ECPropertyChanges>(PROPERTIES); }
        ECInstanceChanges& CustomAttributes() { return Get<ECInstanceChanges>(CUSTOMATTRIBTUES); }
        ECRelationshipChange& GetRelationship() { return Get<ECRelationshipChange>(RELATIONSHIP); }
    };
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
struct ECPropertyChange :ECObjectChange
    {
    protected:

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
        StringChange& GetMaximumValue() { return Get<StringChange>(MAXIMUMVALUE); }
        StringChange& GetMinimumValue() { return Get<StringChange>(MINIMUMVALUE); }
        PropertyTypeChange& GetPropertyType() { return Get<PropertyTypeChange>(PROPERTYTYPE); }
        ArrayChange& GetArray() { return Get<ArrayChange>(ARRAY); }
        NavigationChange& GetNavigation() { return Get<NavigationChange>(NAVIGATION); }
        StringChange& GetExtendedTypeName() { return Get<StringChange>(EXTENDEDTYPENAME); }
        BooleanChange& IsReadonly() { return Get<BooleanChange>(ISREADONLY); }
        ECInstanceChanges& CustomAttributes() { return Get<ECInstanceChanges>(CUSTOMATTRIBTUES); }
    };


struct ECSchemaComparer
    {
    BentleyStatus CompareECSchema(ECSchemaChange& change, ECSchemaCR a, ECSchemaCR b)
        {
        if (a.GetName() != b.GetName())
            change.GetName().SetValue(a.GetName(), b.GetName());

        if (a.GetDisplayLabel() != b.GetDisplayLabel())
            change.GetDisplayLabel().SetValue(a.GetDisplayLabel(), b.GetDisplayLabel());

        if (a.GetDescription() != b.GetDescription())
            change.GetDescription().SetValue(a.GetDescription(), b.GetDescription());

        if (a.GetNamespacePrefix() != b.GetNamespacePrefix())
            change.GetNamespacePrefix().SetValue(a.GetNamespacePrefix(), b.GetNamespacePrefix());

        if (a.GetVersionMajor() != b.GetVersionMajor())
            change.GetVersionMajor().SetValue(a.GetVersionMajor(), b.GetVersionMajor());

        if (a.GetVersionMinor() != b.GetVersionMinor())
            change.GetVersionMinor().SetValue(a.GetVersionMinor(), b.GetVersionMinor());

        if (a.GetVersionWrite() != b.GetVersionWrite())
            change.GetVersionWrite().SetValue(a.GetVersionWrite(), b.GetVersionWrite());

        if (CompareECClasses(change.Classes(), a.GetClasses(), b.GetClasses()) != SUCCESS)
            return ERROR;

        if (CompareECEnumerations(change.Enumerations(), a.GetEnumerations(), b.GetEnumerations()) != SUCCESS)
            return ERROR;

        if (CompareKindOfQuantities(change.KindOfQuantities(), a.GetKindOfQuantities(), b.GetKindOfQuantities()) != SUCCESS)
            return ERROR;

        return CompareCustomAttributes(change.CustomAttributes(), a, b);
        }

    BentleyStatus CompareECClass(ECClassChange& change, ECClassCR a, ECClassCR b)
        {
        if (a.GetName() != b.GetName())
            change.GetName().SetValue(a.GetName(), b.GetName());

        if (a.GetDisplayLabel() != b.GetDisplayLabel())
            change.GetDisplayLabel().SetValue(a.GetDisplayLabel(), b.GetDisplayLabel());

        if (a.GetDescription() != b.GetDescription())
            change.GetDescription().SetValue(a.GetDescription(), b.GetDescription());

        if (a.GetClassModifier() != b.GetClassModifier())
            change.GetClassModifier().SetValue(a.GetClassModifier(), b.GetClassModifier());

        if (a.IsCustomAttributeClass() != b.IsCustomAttributeClass())
            change.IsCustomAttributeClass().SetValue(a.IsCustomAttributeClass(), b.IsCustomAttributeClass());

        if (a.IsStructClass() != b.IsStructClass())
            change.IsStructClass().SetValue(a.IsStructClass(), b.IsStructClass());

        if (a.IsEntityClass() != b.IsEntityClass())
            change.IsEntityClass().SetValue(a.IsEntityClass(), b.IsEntityClass());

        if (a.IsRelationshipClass() != b.IsRelationshipClass())
            {
            change.IsRelationshipClass().SetValue(a.IsRelationshipClass(), b.IsRelationshipClass());
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
            if (a.IsRelationshipClass())
                if (AppendECRelationshipClass(change.GetRelationship(), *b.GetRelationshipClassCP(), ValueId::New) != SUCCESS)
                    return ERROR;                
            }

        if (CompareECProperties(change.Properties(), a.GetProperties(false), b.GetProperties(false)) != SUCCESS)
            return ERROR;

        return CompareCustomAttributes(change.CustomAttributes(), a, b);
        }
    BentleyStatus CompareECRelationshipClass(ECRelationshipChange& change, ECRelationshipClassCR a, ECRelationshipClassCR b)
        {
         if (a.GetStrength() != b.GetStrength())
             change.GetStrength().SetValue(a.GetStrength(), b.GetStrength());

         if (a.GetStrengthDirection() != b.GetStrengthDirection())
             change.GetStrengthDirection().SetValue(a.GetStrengthDirection(), b.GetStrengthDirection());

         if (CompareECRelationshipConstraint(change.GetSource(), a.GetSource(), b.GetSource()) != SUCCESS)
             return ERROR;

         return CompareECRelationshipConstraint(change.GetTarget(), a.GetTarget(), b.GetTarget());
        }

    BentleyStatus CompareECRelationshipConstraint(ECRelationshipConstraintChange& change, ECRelationshipConstraintCR a, ECRelationshipConstraintCR b)
        {
        if (a.GetRoleLabel() != b.GetRoleLabel())
            change.GetRoleLabel().SetValue(a.GetRoleLabel(), b.GetRoleLabel());

        if (a.GetIsPolymorphic() != b.GetIsPolymorphic())
            change.IsPolymorphic().SetValue(a.GetIsPolymorphic(), b.GetIsPolymorphic());

        if (a.GetCardinality().ToString() != b.GetCardinality().ToString())
            change.GetCardinality().SetValue(a.GetCardinality().ToString(), b.GetCardinality().ToString());

        return CompareECRelationshipConstraintClasses(change.ConstraintClasses(), a.GetConstraintClasses(), b.GetConstraintClasses());
        }
    template<typename S>
    S Union(const S& s1, const S& s2)
        {
        S result = s1;

        result.insert(s2.cbegin(), s2.cend());

        return result;
        }

    template<typename T, typename Y>
    ChangeState Compare(T const& value,  std::map<T, Y>& s1,  std::map<T, Y>& s2, Y * s1Item, Y * s2Item)
        {
        *s1Item = nullptr;
        *s2Item = nullptr;

        auto itorA = s1.find(value);
        auto itorB = s2.find(value);

        bool existInA = itorA != s1.end();
        bool existInB = itorB != s2.end();
        if (existInA && existInB) 
            {
            *s1Item = itorA.second;
            *s2Item = itorB.second;
            return ChangeState::Modified;
            }
        else if (existInA && !existInB)
            {
            *s1Item = itorA.second;
            return ChangeState::Deleted;
            }
        else if (!existInA && existInB)
            {
            *s2Item = itorB.second;
            return ChangeState::New;
            }
        BeAssert(false && "value must exist in atleast one of the map");
        return static_cast<ChangeState>(0);
        }
    BentleyStatus CompareECRelationshipConstraintClassKeys(ECRelationshipConstraintClassChange& change, ECRelationshipConstraintClassCR const& a, ECRelationshipConstraintClassCR const& b)
        {

        }
    BentleyStatus CompareECRelationshipConstraintClasses(ECRelationshipConstraintClassChanges& change, ECRelationshipConstraintClassList const& a, ECRelationshipConstraintClassList const& b)
        {
        //std::map<Utf8String, ECRelationshipConstraintClassCP> aMap, bMap;
        //for (ECRelationshipConstraintClassCP constraintClassCP : a)
        //    aMap[constraintClassCP->GetClass().GetFullName()] = constraintClassCP;

        //for (ECRelationshipConstraintClassCP constraintClassCP : b)
        //    bMap[constraintClassCP->GetClass().GetFullName()] = constraintClassCP;

        //ECRelationshipConstraintClassCP* aItem, *bItem;
        //for (auto& u : Union(aMap, bMap))
        //    {
        //    switch (Compare<Utf8String, ECRelationshipConstraintClassCP>(u.first, aMap, bMap, aItem, bItem))
        //        {
        //        case ChangeState::Deleted:
        //            {
        //            if (AppendECRelationshipConstraintClass(change.Add(ChangeState::Deleted), **aItem, ValueId::Deleted) == ERROR)
        //                return ERROR;

        //            break;
        //            }
        //        case ChangeState::New:
        //            {
        //            if (AppendECRelationshipConstraintClass(change.Add(ChangeState::New), **bItem, ValueId::New) == ERROR)
        //                return ERROR;

        //            break;
        //            }
        //        case ChangeState::Modified:
        //            {
        //            auto& constraintClass = change.Add(ChangeState::Modified);
        //            if (CompareECRelationshipConstraintClassKeys(constraintClass, **aItem, **bItem) == ERROR)
        //                return ERROR;
        //            }
        //            break;
        //        default:
        //            {
        //            return ERROR;
        //            }
        //        }
        //    }

        return SUCCESS;
        }

    BentleyStatus CompareECProperty(ECPropertyChange& change, ECPropertyCR a, ECPropertyCR b);

    BentleyStatus CompareECInstance(ECInstanceChange& change, IECInstanceCR a, IECInstanceCR b);

    BentleyStatus CompareECProperties (ECPropertyChanges& changes, ECPropertyIterableCR a, ECPropertyIterableCR b);

    BentleyStatus CompareECClasses(ECClassChanges& changes, ECClassContainerCR a, ECClassContainerCR b);
    BentleyStatus CompareECEnumerations(ECEnumerationChanges& changes, ECEnumerationContainerCR a, ECEnumerationContainerCR b);
    BentleyStatus CompareKindOfQuantities(ECKindOfQuantityChanges& changes, KindOfQuantityContainerCR a, KindOfQuantityContainerCR b);
    BentleyStatus CompareCustomAttributes(ECInstanceChanges& changes, IECCustomAttributeContainerCR a, IECCustomAttributeContainerCR b);


    BentleyStatus AppendECSchema(ECSchemaChanges& changes, ECSchemaCR v, ValueId appendType)
        {
        ChangeState state = appendType == ValueId::New ? ChangeState::New : ChangeState::Deleted;
        ECSchemaChange& change = changes.Add(state, v.GetName().c_str());

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
        ECClassChange& change = changes.Add(state, v.GetName().c_str());

        change.GetName().SetValue(appendType, v.GetName());
        change.GetDisplayLabel().SetValue(appendType, v.GetDisplayLabel());
        change.GetDescription().SetValue(appendType, v.GetDescription());
        change.GetClassModifier().SetValue(appendType, v.GetClassModifier());
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
            if (AppendECRelationshipConstraintClass(constraintClassChange, *constraintClass, appendType) == ERROR)
                return ERROR;
            }
        return SUCCESS;
        }
    BentleyStatus AppendECRelationshipConstraintClass(ECRelationshipConstraintClassChange& change, ECRelationshipConstraintClassCR const& v, ValueId appendType)
        {
        ChangeState state = appendType == ValueId::New ? ChangeState::New : ChangeState::Deleted;
        change.GetClassName().SetValue(appendType, v.GetClass().GetFullName());
        for (Utf8StringCR relationshipKeyProperty : v.GetKeys())
            {
            if (!relationshipKeyProperty.empty())
                change.KeyProperties().Add(state).SetValue(appendType, relationshipKeyProperty);
            }
        return SUCCESS;
        }

    BentleyStatus AppendECEnumeration(ECEnumerationChanges& changes, ECEnumerationCR v, ValueId appendType)
        {
        ChangeState state = appendType == ValueId::New ? ChangeState::New : ChangeState::Deleted;
        ECEnumerationChange& enumerationChange = changes.Add(state,v.GetName().c_str());
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
        KindOfQuantityChange& kindOfQuantityChange = changes.Add(state, v.GetName().c_str());
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
        ECPropertyChange& propertyChange = changes.Add(state, v.GetName().c_str());
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
        ECInstanceChange& instanceChange = changes.Add(state, v.GetClass().GetFullName());
        instanceChange.GetClassName().SetValue(appendType, v.GetClass().GetFullName());
        //return AppendPropertyValues(instanceChange.Instance(), *propertyValues, appendType);
        return SUCCESS;
        }
    //std::map<Path,ECValueCR>
    BentleyStatus AppendCustomAttributes(ECInstanceChanges& changes, IECCustomAttributeContainerCR v, ValueId appendType)
        {
        for (auto& instance : v.GetPrimaryCustomAttributes(false))
            {
            if (AppendECInstance(changes, *instance, appendType) != SUCCESS)
                return ERROR;
            }

        return SUCCESS;
        }
    //BentleyStatus AppendPropertyValues(ECPropertyValueChange& parentChange, ECValuesCollectionCR values, ValueId appendType)
    //    {
    //    for (ECValuesCollection::const_iterator itor = values.begin(); itor != values.end(); ++itor)
    //        {
    //        ECValueAccessorCR valueAccessor = (*itor).GetValueAccessor();
    //        const Utf8String propertyName = valueAccessor.GetPropertyName();
    //        ECPropertyValueChange& valueChange = parentChange.Get(propertyName.c_str());
    //        
    //        if ((*itor).HasChildValues())
    //            AppendPropertyValues(valueChange, *(*itor).GetChildValues(), appendType);
    //        else
    //            {
    //            ECValueCR v = (*itor).GetValue();
    //            if (v.IsNull())
    //                continue;

    //            if (v.IsPrimitive())
    //                {
    //                valueChange.SetValue(appendType, v);
    //                }
    //            }
    //        }

    //    return SUCCESS;
    //    }

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

