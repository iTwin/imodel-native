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
//struct KindOfQuantityChange;
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
        Nullable();
        Nullable(T const& value);
        Nullable(Nullable<T> const& rhs);
        Nullable(Nullable<T> const&& rhs);

        bool IsNull() const;
        T const& Value() const;
        T& ValueR();

        bool operator == (Nullable<T> const& rhs) const;
        bool operator == (nullptr_t rhs)const;
        bool operator != (Nullable<T> const& rhs) const;
        bool operator != (nullptr_t rhs) const;
        Nullable<T>& operator = (Nullable<T> const&& rhs);
        Nullable<T>& operator = (Nullable<T> const& rhs);
        Nullable<T>& operator = (T const& rhs);
        Nullable<T>& operator = (T const&& rhs);
        Nullable<T>& operator = (nullptr_t rhs);
    };

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
struct ECChange
    {
    public:
        typedef std::unique_ptr<ECChange> Ptr;
   
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
                        {ISSTRUCT, "IsStruct"},
                        {ISSTRUCTARRAY, "IsStructArray"},
                        {ISPRIMITIVE, "IsPrimitive"},
                        {ISPRIMITIVEARRAY, "IsPrimitiveArray"},
                        {ISNAVIGATION, "IsNavigation"},
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
        bool m_applied;
        virtual bool _IsEmpty() const = 0;
        virtual void _Optimize() {}
        virtual bool _IsArray() const { return false; }
        virtual bool _IsObject() const { return false; }
        virtual bool _IsPrimitive() const { return false; }

    public:
        ECChange(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            :m_systemId(systemId), m_parent(parent), m_state(state), m_applied(false)
            {
            if (customId != nullptr)
                m_customId = std::unique_ptr<Utf8String>(new Utf8String(customId));
            }
        virtual ~ECChange() {};

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
        //bool IsArray() const { return _IsArray(); }
        //bool IsObject() const { return _IsObject(); }
        //bool IsPrimitive() const { return _IsPrimitive(); }
        bool Exist() const { return !IsEmpty(); }
        bool IsPending() const { return m_applied; }
        void Done() { BeAssert(m_applied == false); m_applied = true; }
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
        T& Get(SystemId systemId)
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
        T* Find(Utf8CP customId)
            {
            for (auto& v : m_changes)
                if (v->GetId() == customId)
                    return static_cast<T*>(v.get());

            return nullptr;
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
struct BaseClassChanges : ECChangeArray<StringChange>
    {
    public:
        BaseClassChanges(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECChangeArray<StringChange>(state, systemId, parent, customId, BASECLASS)
            {}
        virtual ~BaseClassChanges() {}
    };
struct ReferenceChanges : ECChangeArray<StringChange>
    {
    public:
        ReferenceChanges(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECChangeArray<StringChange>(state, systemId, parent, customId, REFERENCE)
            {}
        virtual ~ReferenceChanges() {}
    };
template<typename T>
struct ECPrimitiveChange : ECChange 
    {
    private:
        Nullable<T> m_old;
        Nullable<T> m_new;
        virtual bool _IsEmpty() const override
            {
            return m_old == m_new;
            }
        virtual bool _IsPrimitive() const override
            {
            return true;
            }
    public:
        ECPrimitiveChange(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECChange(state, systemId, parent, customId)
            {}        
        virtual ~ECPrimitiveChange() {}
        Nullable<T> const& GetNew() const
            {
            return m_new;
            }
        Nullable<T> const& GetOld() const
            {
            return m_new;
            }

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
        ReferenceChanges& References() { return Get<ReferenceChanges>(REFERENCES); }
        ECClassChanges& Classes() { return Get<ECClassChanges>(CLASSES); }
        ECEnumerationChanges& Enumerations() { return Get<ECEnumerationChanges>(ENUMERATIONS); }
        //ECKindOfQuantityChanges& KindOfQuantities() { return Get<ECKindOfQuantityChanges>(KINDOFQUANTITIES); }
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
        BaseClassChanges& BaseClasses() { return Get<BaseClassChanges>(BASECLASSES); }
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


struct ECSchemaComparer
    {
    private:
        BentleyStatus CompareECSchemas(ECSchemaChanges& changes, ECSchemaList const& a, ECSchemaList const& b);
        BentleyStatus CompareECSchema(ECSchemaChange& change, ECSchemaCR a, ECSchemaCR b);
        BentleyStatus CompareECClass(ECClassChange& change, ECClassCR a, ECClassCR b);
        BentleyStatus CompareECBaseClasses(BaseClassChanges& changes, ECBaseClassesList const& a, ECBaseClassesList const& b);
        BentleyStatus CompareECRelationshipClass(ECRelationshipChange& change, ECRelationshipClassCR a, ECRelationshipClassCR b);
        BentleyStatus CompareECRelationshipConstraint(ECRelationshipConstraintChange& change, ECRelationshipConstraintCR a, ECRelationshipConstraintCR b);
        BentleyStatus CompareECRelationshipConstraintClassKeys(ECRelationshipConstraintClassChange& change, ECRelationshipConstraintClassCR const& a, ECRelationshipConstraintClassCR const& b);
        BentleyStatus CompareECRelationshipConstraintClasses(ECRelationshipConstraintClassChanges& change, ECRelationshipConstraintClassList const& a, ECRelationshipConstraintClassList const& b);
        BentleyStatus CompareECProperty(ECPropertyChange& change, ECPropertyCR a, ECPropertyCR b);
        BentleyStatus CompareECProperties(ECPropertyChanges& changes, ECPropertyIterableCR a, ECPropertyIterableCR b);
        BentleyStatus CompareECClasses(ECClassChanges& changes, ECClassContainerCR a, ECClassContainerCR b);
        BentleyStatus CompareECEnumerations(ECEnumerationChanges& changes, ECEnumerationContainerCR a, ECEnumerationContainerCR b);
        //BentleyStatus CompareKindOfQuantities(ECKindOfQuantityChanges& changes, KindOfQuantityContainerCR a, KindOfQuantityContainerCR b);
        BentleyStatus CompareCustomAttributes(ECInstanceChanges& changes, IECCustomAttributeContainerCR a, IECCustomAttributeContainerCR b);
        BentleyStatus CompareECEnumeration(ECEnumerationChange& change, ECEnumerationCR a, ECEnumerationCR b);
        BentleyStatus CompareIntegerECEnumerators(ECEnumeratorChanges& changes, EnumeratorIterable const& a, EnumeratorIterable const& b);
        BentleyStatus CompareStringECEnumerators(ECEnumeratorChanges& changes, EnumeratorIterable const& a, EnumeratorIterable const& b);
        //BentleyStatus CompareKindOfQuantity(KindOfQuantityChange& change, KindOfQuantityCR a, KindOfQuantityCR b);
        BentleyStatus CompareECInstance(ECInstanceChange& change, IECInstanceCR a, IECInstanceCR b);
        BentleyStatus CompareBaseClasses(BaseClassChanges& changes, ECBaseClassesList const& a, ECBaseClassesList const& b);
        BentleyStatus CompareReferences(ReferenceChanges& changes, ECSchemaReferenceListCR a, ECSchemaReferenceListCR b);
        BentleyStatus AppendECSchema(ECSchemaChanges& changes, ECSchemaCR v, ValueId appendType);
        BentleyStatus AppendECClass(ECClassChanges& changes, ECClassCR v, ValueId appendType);
        BentleyStatus AppendECRelationshipClass(ECRelationshipChange& change, ECRelationshipClassCR v, ValueId appendType);
        BentleyStatus AppendECRelationshipConstraint(ECRelationshipConstraintChange& change, ECRelationshipConstraintCR v, ValueId appendType);
        BentleyStatus AppendECRelationshipConstraintClasses(ECRelationshipConstraintClassChanges& changes, ECRelationshipConstraintClassList const& v, ValueId appendType);
        BentleyStatus AppendECRelationshipConstraintClass(ECRelationshipConstraintClassChange& change, ECRelationshipConstraintClassCR const& v, ValueId appendType);
        BentleyStatus AppendECEnumeration(ECEnumerationChanges& changes, ECEnumerationCR v, ValueId appendType);
        //BentleyStatus AppendKindOfQuantity(ECKindOfQuantityChanges& changes, KindOfQuantityCR v, ValueId appendType);
        BentleyStatus AppendECProperty(ECPropertyChanges& changes, ECPropertyCR v, ValueId appendType);
        BentleyStatus AppendECInstance(ECInstanceChange& changes, IECInstanceCR v, ValueId appendType);
        BentleyStatus AppendCustomAttributes(ECInstanceChanges& changes, IECCustomAttributeContainerCR v, ValueId appendType);
        BentleyStatus AppendBaseClasses(BaseClassChanges& changes, ECBaseClassesList const& v, ValueId appendType);
        BentleyStatus AppendReferences(ReferenceChanges& changes, ECSchemaReferenceListCR v, ValueId appendType);
        BentleyStatus ConvertECInstanceToValueMap(std::map<Utf8String, ECValue>& map, IECInstanceCR instance);
        BentleyStatus ConvertECValuesCollectionToValueMap(std::map<Utf8String, ECValue>& map, ECValuesCollectionCR values);
        std::vector<Utf8String> Split(Utf8StringCR path);
    public:
        BentleyStatus Compare(ECSchemaChanges& changes, ECSchemaList const& existingSet, ECSchemaList const& newSet);
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

