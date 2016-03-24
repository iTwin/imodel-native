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
        bool operator == (nullptr_t rhs)const;
        bool operator != (Nullable<T> const& rhs) const;
        bool operator != (nullptr_t rhs) const;
        Nullable<T>& operator = (Nullable<T> const&& rhs);
        Nullable<T>& operator = (Nullable<T> const& rhs);
        Nullable<T>& operator = (T const& rhs);
        Nullable<T>& operator = (T const&& rhs);
        Nullable<T>& operator = (nullptr_t rhs);
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
                        {BASECLASS, "BaseClass"},
                        {BASECLASSES, "BaseClasses"},
                        {CARDINALITY, "Cardinality"},
                        {CLASSES, "Classes"},
                        {CLASS, "Class"},
                        {CONSTANTKEY, "ConstantKey"},
                        {CLASSFULLNAME, "ClassFullName"},
                        {CLASSMODIFIER, "ClassModifier"},
                        {CONSTRAINTCLASS, "ConstraintClass"},
                        {CONSTRAINTCLASSES, "ConstraintClasses"},
                        {CONSTRAINT, "Constraint"},
                        {CUSTOMATTRIBTUES, "CustomAttributes"},
                        {DEFAULTPRESENTATIONUNIT, "DefaultPresentationUnit"},
                        {DESCRIPTION, "Description"},
                        {DIRECTION, "Direction"},
                        {PROPERTYVALUE,"PropertyValue"},
                        {PROPERTYVALUES,"PropertyValues"},
                        {DISPLAYLABEL, "DisplayLabel"},
                        {ENUMERATION, "Enumeration"},
                        {ENUMERATIONS, "Enumerations"},
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
        bool m_applied;
        virtual void _WriteToString(Utf8StringR str, int currentIndex, int indentSize) const = 0;
        virtual bool _IsEmpty() const = 0;
        virtual void _Optimize() {}
    protected:
        static  void AppendBegin(Utf8StringR str, ECChange const& change, int currentIndex)
            {
          
            if (change.GetState() == ChangeState::Deleted)
                str += "-";
            else if (change.GetState() == ChangeState::New)
                str += "+";
            else if (change.GetState() == ChangeState::Modified)
                str += "!";

            for (int i = 0; i < currentIndex; i++)
                str.append(" ");

            str.append(Convert(change.GetSystemId()));
            if (change.HasCustomId())
                str.append("(").append(change.GetId().c_str()).append(")");
            }
        static  void AppendEnd(Utf8StringR str)
            { 
            str.append("\r\n");
            }
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
        bool HasCustomId() const { return m_customId != nullptr; }
        ChangeState GetState() const { return m_state; }
        ECChange const* GetParent() const { return m_parent; }
        bool IsEmpty() const { return _IsEmpty(); }
        void Optimize() { return _Optimize(); }
        //bool IsArray() const { return _IsArray(); }
        //bool IsObject() const { return _IsObject(); }
        //bool IsPrimitive() const { return _IsPrimitive(); }
        bool Exist() const { return !IsEmpty(); }
        bool IsPending() const { return !m_applied; }
        void Done() { BeAssert(m_applied == false); m_applied = true; }
        void WriteToString(Utf8StringR str, int initIndex = 0, int indentSize =3) const
            {

            _WriteToString(str, initIndex, indentSize);
            }

    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct ECObjectChange : ECChange
    {
    private:
        std::map<Utf8CP, ECChange::Ptr, CompareUtf8> m_changes;
        virtual void _WriteToString(Utf8StringR str, int currentIndex, int indentSize) const override
            {
            AppendBegin(str, *this, currentIndex);
            AppendEnd(str);
            for (auto& change : m_changes)
                {
                change.second->WriteToString(str, currentIndex + indentSize, indentSize);
                }
            }

        virtual bool _IsEmpty() const override
            {
            for (auto& change : m_changes)
                {
                if (change.second->Exist())
                    return false;
                }

            return true;
            }
        virtual void _Optimize() override
            {
            auto itor = m_changes.begin();
            while (itor != m_changes.end())
                {
                itor->second->Optimize();
                if (itor->second->IsEmpty())
                    {
                    itor = m_changes.erase(itor);
                    }
                else
                    ++itor;
                }
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

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
template<typename T>
struct ECChangeArray : ECChange
    {
    private:
        std::vector<ECChange::Ptr> m_changes;
        SystemId m_elementType;
        virtual void _WriteToString(Utf8StringR str, int currentIndex, int indentSize) const override
            {
            AppendBegin(str, *this, currentIndex);
            AppendEnd(str);
            for (auto& change : m_changes)
                {
                change->WriteToString(str, currentIndex + indentSize, indentSize);
                }
            }

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
     
        virtual bool _IsEmpty() const override
            {
            return m_old == m_new;           
            }
        
        virtual Utf8String _ToString(ValueId id) const  { return "NOT_IMP"; }
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
        Nullable<T> const& GetNew() const
            {
            return m_new;
            }
        Nullable<T> const& GetOld() const
            {
            return m_new;
            }
        Utf8String ToString(ValueId id) const
            {
            return _ToString(id);
            }
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

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct StringChange : ECPrimitiveChange<Utf8String>
    {
   
    private:
        virtual Utf8String _ToString(ValueId id) const override 
            { 
            Utf8String str;
            auto& v = GetValue(id);
            if (v.IsNull())
                str = "<null>";
            else
                str = v.Value();
            return str;
            }

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
        virtual Utf8String _ToString(ValueId id) const override
            {
            Utf8String str;
            auto& v = GetValue(id);
            if (v.IsNull())
                str = "<null>";
            else
                str = v.Value() ? "true": "false";
            return str;
            }

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
        virtual Utf8String _ToString(ValueId id) const override
            {
            Utf8String str;
            auto& v = GetValue(id);
            if (v.IsNull())
                str = "<null>";
            else
                str.Sprintf("%u", v.Value());

            return str;
            }
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
    virtual Utf8String _ToString(ValueId id) const override
        {
        Utf8String str;
        auto& v = GetValue(id);
        if (v.IsNull())
            str = "<null>";
        else
            str.Sprintf("%d", v.Value());

        return str;
        }

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
        virtual Utf8String _ToString(ValueId id) const override
            {
            Utf8String str;
            auto& v = GetValue(id);
            if (v.IsNull())
                str = "<null>";
            else
                str.Sprintf("%f", v.Value());

            return str;
            }

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
        virtual Utf8String _ToString(ValueId id) const override
            {
            Utf8String str;
            auto& v = GetValue(id);
            if (v.IsNull())
                str = "<null>";
            else
                str = Utf8String(v.Value().ToString().c_str());

            return str;
            }

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
        virtual Utf8String _ToString(ValueId id) const override
            {
            auto& v = GetValue(id);
            if (v.IsNull())
                return "<null>";

            if (v.Value().IsNull())
                return "<null>";

            return v.Value().ToString();
            }

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
struct Point2DChange: ECPrimitiveChange<Point2d>
    {
    public:
        Point2DChange(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECPrimitiveChange<Point2d>(state, systemId, parent, customId)
            {}
        virtual ~Point2DChange() {}
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct Point3DChange: ECPrimitiveChange<Point3d>
    {
    public:
        Point3DChange(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECPrimitiveChange<Point3d>(state, systemId, parent, customId)
            {}
        virtual ~Point3DChange() {}
    };

//=======================================================================================
// @bsiclass                                                Affan.Khan            03/2016
//+===============+===============+===============+===============+===============+======
struct Int64Change: ECPrimitiveChange<int64_t>
    {
    private:
        virtual Utf8String _ToString(ValueId id) const override
            {
            Utf8String str;
            auto& v = GetValue(id);
            if (v.IsNull())
                str = "<null>";
            else
                str.Sprintf("%ll",v.Value());

            return str;
            }

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
        virtual void _WriteToString(Utf8StringR str, int currentIndex, int indentSize) const override
            {
            if (m_value != nullptr)
                {
                m_value->WriteToString(str, currentIndex, indentSize);
                return;
                }

            AppendBegin(str, *this, currentIndex);
            AppendEnd(str);


            if (m_children != nullptr)
                {
                //AppendBegin(str, *this, currentIndex);
                for (size_t i = 0; i < m_children->Count(); i++)
                    {
                    m_children->At(i).WriteToString(str, currentIndex + indentSize, indentSize);
                    }
                //AppendEnd(str);
                }
            }

        virtual bool _IsEmpty() const override
            {
            if (auto parent = GetParent())
                if (parent->GetSystemId() == CUSTOMATTRIBTUES && GetState() != ChangeState::Modified)
                    return false;

            if (m_value != nullptr)
                return m_value->IsEmpty();

            if (m_children != nullptr)
                return m_children->IsEmpty();

            return true;
            }

        virtual void _Optimize() 
            {
            if (m_value != nullptr)
                if (m_value->IsEmpty())
                    m_value = nullptr;

            if (m_children != nullptr)
                if (m_children->IsEmpty())
                    m_children = nullptr;
            }
    public:
        ECPropertyValueChange(ChangeState state, SystemId systemId, ECChange const* parent = nullptr, Utf8CP customId = nullptr)
            : ECChange(state, PROPERTYVALUE, parent, customId)
            {
            BeAssert(systemId == GetSystemId());
       
            }
        bool HasValue() const { return m_value != nullptr; }
        bool HasChildren() const { return m_children != nullptr; }

        ECPrimitiveChange<ECValue>& GetPropertyValue() 
            { 
            if (m_value == nullptr)
                {
                m_value = std::unique_ptr<ECValueChange>(new ECValueChange(GetState(), PROPERTYVALUE, this, GetId().c_str()));
                }

            return *m_value;
            }
        ECChangeArray<ECPropertyValueChange>& GetChildren() 
            { 
            if (m_children == nullptr)
                {
                m_children = std::unique_ptr<ECChangeArray<ECPropertyValueChange>>(new ECChangeArray<ECPropertyValueChange>(GetState(), PROPERTYVALUES, this, GetId().c_str(), PROPERTYVALUE));
                }

            return *m_children;
            }
        ECPropertyValueChange& GetOrCreate(ChangeState stat, std::vector<Utf8String> const& path)
            {
            ECPropertyValueChange* c = this;
            for (auto& str : path)
                {
                auto m = c->GetChildren().Find(str.c_str());
                if (m == nullptr)
                    {
                    c = &c->GetChildren().Add(stat, str.c_str());
                    }
                else
                    c = m;
                }

            return *c;
            }
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
        BentleyStatus CompareECRelationshipConstraintClassKeys(ECRelationshipConstraintClassChange& change, ECRelationshipConstraintClassCR const& a, ECRelationshipConstraintClassCR const& b);
        BentleyStatus CompareECRelationshipConstraintClasses(ECRelationshipConstraintClassChanges& change, ECRelationshipConstraintClassList const& a, ECRelationshipConstraintClassList const& b);
        BentleyStatus CompareECProperty(ECPropertyChange& change, ECPropertyCR a, ECPropertyCR b);
        BentleyStatus CompareECProperties(ECPropertyChanges& changes, ECPropertyIterableCR a, ECPropertyIterableCR b);
        BentleyStatus CompareECClasses(ECClassChanges& changes, ECClassContainerCR a, ECClassContainerCR b);
        BentleyStatus CompareECEnumerations(ECEnumerationChanges& changes, ECEnumerationContainerCR a, ECEnumerationContainerCR b);
        //BentleyStatus CompareKindOfQuantities(ECKindOfQuantityChanges& changes, KindOfQuantityContainerCR a, KindOfQuantityContainerCR b);
        BentleyStatus CompareCustomAttributes(ECInstanceChanges& changes, IECCustomAttributeContainerCR a, IECCustomAttributeContainerCR b);
        BentleyStatus CompareCustomAttribute(ECPropertyValueChange& changes, IECInstanceCR a, IECInstanceCR b);
        
        BentleyStatus CompareECEnumeration(ECEnumerationChange& change, ECEnumerationCR a, ECEnumerationCR b);
        BentleyStatus CompareIntegerECEnumerators(ECEnumeratorChanges& changes, EnumeratorIterable const& a, EnumeratorIterable const& b);
        BentleyStatus CompareStringECEnumerators(ECEnumeratorChanges& changes, EnumeratorIterable const& a, EnumeratorIterable const& b);
        //BentleyStatus CompareKindOfQuantity(KindOfQuantityChange& change, KindOfQuantityCR a, KindOfQuantityCR b);
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
        //BentleyStatus AppendECInstance(ECPropertyValueChange& changes, IECInstanceCR v, ValueId appendType);        
        BentleyStatus AppendCustomAttributes(ECInstanceChanges& changes, IECCustomAttributeContainerCR v, ValueId appendType);
        BentleyStatus AppendCustomAttribute(ECInstanceChanges& changes, IECInstanceCR v, ValueId appendType);        
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

