/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECInstanceAdapterHelper.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include <ECDb/IECSqlBinder.h>
#include <ECDb/ECInstanceId.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//======================================================================================
//! Contains metadata for how properties of given ECClass map to parameters in the ECSQL
//! statement used by ECInstanceInserter, ECInstanceUpdater, ECInstanceDeleter
// @bsiclass                                                 Krischan.Eberle     06/2014
//+===============+===============+===============+===============+===============+======
struct ECValueBindingInfo
    {
    public:
        enum class Type
            {
            Primitive,
            Struct,
            Array,
            Navigation,
            ECSqlSystemProperty
            };

        enum class SystemPropertyKind
            {
            ECInstanceId,
            SourceECInstanceId,
            TargetECInstanceId
            };

    protected:
        static const int UNSET_PARAMETERINDEX = 0;

    private:
        Type m_type;
        int m_ecsqlParameterIndex = UNSET_PARAMETERINDEX;
        bool m_hasPropertyIndex = false;
        uint32_t m_propertyIndex = 0;

        //not copyable
        ECValueBindingInfo(ECValueBindingInfo const&) = delete;
        ECValueBindingInfo& operator=(ECValueBindingInfo const&) = delete;

    protected:
        ECValueBindingInfo(Type type, int ecsqlParameterIndex) : m_type(type), m_ecsqlParameterIndex(ecsqlParameterIndex) {}

        ECValueBindingInfo(Type type, int ecsqlParameterIndex, uint32_t propertyIndex)
            : m_type(type), m_ecsqlParameterIndex(ecsqlParameterIndex), m_hasPropertyIndex(true), m_propertyIndex(propertyIndex)
            {}

    public:
        virtual ~ECValueBindingInfo() {}
        Type GetType() const { return m_type; }

        //! Only binding infos for top-level properties have a corresponding ECSQL parameter index.
        //! Binding infos for nested properties don't. Use this to tell between the two.
        bool HasECSqlParameterIndex() const { return m_ecsqlParameterIndex > UNSET_PARAMETERINDEX; }
        //! Gets the ECSQL parameter index to which this binding info maps to.
        //! Only call this for top-level properties. Don't call it for nested properties.
        int GetECSqlParameterIndex() const { BeAssert(HasECSqlParameterIndex());  return m_ecsqlParameterIndex; }

        bool HasPropertyIndex() const { return m_hasPropertyIndex; }
        uint32_t GetPropertyIndex() const { BeAssert(HasPropertyIndex());  return m_propertyIndex; }
    };

//======================================================================================
//! Factory to create ECValueBindingInfos
// @bsiclass                                                 Krischan.Eberle     06/2014
//+===============+===============+===============+===============+===============+======
struct ECValueBindingInfoFactory final
    {
    private:
        ECValueBindingInfoFactory();
        ~ECValueBindingInfoFactory();

    public:
        static std::unique_ptr<ECValueBindingInfo> CreateBindingInfo(ECN::ECEnablerCR enabler, ECN::ECPropertyCR ecProperty, Utf8StringCR propertyAccessString, int ecsqlParameterIndex);
        static std::unique_ptr<ECValueBindingInfo> CreateSystemBindingInfo(ECValueBindingInfo::SystemPropertyKind kind, int ecsqlParameterIndex);
    };

//======================================================================================
//! BindingInfo for ECSQL System properties
// @bsiclass                                                 Krischan.Eberle     07/2014
//+===============+===============+===============+===============+===============+======
struct ECSqlSystemPropertyBindingInfo final : ECValueBindingInfo
    {
    public:

    private:
        SystemPropertyKind m_kind;

        ECSqlSystemPropertyBindingInfo(SystemPropertyKind kind, int ecsqlParameterIndex) : ECValueBindingInfo(Type::ECSqlSystemProperty, ecsqlParameterIndex), m_kind(kind) {}

    public:
        static std::unique_ptr<ECSqlSystemPropertyBindingInfo> Create(SystemPropertyKind kind, int ecsqlParameterIndex);
        ~ECSqlSystemPropertyBindingInfo() {}

        SystemPropertyKind GetKind() const { return m_kind; }
    };

//======================================================================================
//! ECValueBindingInfo for primitive ECProperties
// @bsiclass                                                 Krischan.Eberle     06/2014
//+===============+===============+===============+===============+===============+======
struct PrimitiveECValueBindingInfo final : ECValueBindingInfo
    {
    private:
        PrimitiveECValueBindingInfo(int ecsqlParameterIndex, uint32_t propertyIndex) :  ECValueBindingInfo(Type::Primitive, ecsqlParameterIndex, propertyIndex) {}

    public:
        static std::unique_ptr<PrimitiveECValueBindingInfo> Create(int ecsqlParameterIndex, uint32_t propertyIndex);
        ~PrimitiveECValueBindingInfo() {}
    };

//======================================================================================
//! ECValueBindingInfo for struct ECProperties
// @bsiclass                                                 Krischan.Eberle     06/2014
//+===============+===============+===============+===============+===============+======
struct StructECValueBindingInfo final : ECValueBindingInfo
    {
    private:
        std::map<ECN::ECPropertyId, std::unique_ptr<ECValueBindingInfo>> m_memberBindingInfos;

        StructECValueBindingInfo(ECN::ECEnablerCR parentEnabler, ECN::ECStructClassCR, Utf8StringCP parentPropertyAccessString, int ecsqlParameterIndex);

    public:
        static std::unique_ptr<StructECValueBindingInfo> Create(ECN::ECEnablerCR parentEnabler, ECN::ECStructClassCR, Utf8StringCP parentPropertyAccessString, int ecsqlParameterIndex);
        static std::unique_ptr<StructECValueBindingInfo> CreateForNestedStruct(ECN::ECStructClassCR);

        ~StructECValueBindingInfo() {}

        std::map<ECN::ECPropertyId, std::unique_ptr<ECValueBindingInfo>> const& GetMemberBindingInfos() const { return m_memberBindingInfos; }
    };

//======================================================================================
//! ECValueBindingInfo for array ECProperties
// @bsiclass                                                 Krischan.Eberle     06/2014
//+===============+===============+===============+===============+===============+======
struct ArrayECValueBindingInfo final : ECValueBindingInfo
    {
    private:
        std::unique_ptr<StructECValueBindingInfo> m_structArrayElementBindingInfo = nullptr;

        ArrayECValueBindingInfo(ECN::ECPropertyCR prop, int ecsqlParameterIndex, uint32_t arrayPropIndex);

    public:
        static std::unique_ptr<ArrayECValueBindingInfo> Create(ECN::ECPropertyCR prop, int ecsqlParameterIndex, uint32_t arrayPropIndex);

        ~ArrayECValueBindingInfo() {}

        //! Indicates whether this is a binding for a struct array or not.
        bool IsStructArray() const { return m_structArrayElementBindingInfo != nullptr; }
        //! Returns the binding info for the struct element type, in case this is binding info for a struct array
        StructECValueBindingInfo const& GetStructArrayElementBindingInfo() const { BeAssert(IsStructArray()); return *m_structArrayElementBindingInfo; }
    };

//======================================================================================
//! ECValueBindingInfo for NavigationECProperties
// @bsiclass                                                 Krischan.Eberle     11/2016
//+===============+===============+===============+===============+===============+======
struct NavigationECValueBindingInfo final : ECValueBindingInfo
    {
    private:
        NavigationECValueBindingInfo(int ecsqlParameterIndex, uint32_t propertyIndex) : ECValueBindingInfo(Type::Navigation, ecsqlParameterIndex, propertyIndex) {}

    public:
        static std::unique_ptr<NavigationECValueBindingInfo> Create(int ecsqlParameterIndex, uint32_t propertyIndex)
            {
            return std::unique_ptr<NavigationECValueBindingInfo>(new NavigationECValueBindingInfo(ecsqlParameterIndex, propertyIndex));
            }

        ~NavigationECValueBindingInfo() {}
    };

//======================================================================================
//! Collection of ECValueBindingInfos
// @bsiclass                                                 Krischan.Eberle     06/2014
//+===============+===============+===============+===============+===============+======
struct ECValueBindingInfoCollection final
    {
    public:
        //======================================================================================
        // @bsiclass                                                 Krischan.Eberle     06/2014
        //+===============+===============+===============+===============+===============+======
        struct const_iterator final : std::iterator<std::forward_iterator_tag, ECValueBindingInfo const*>
            {
            private:
                std::vector<std::unique_ptr<ECValueBindingInfo>>::const_iterator m_iterator;

            public:
                explicit const_iterator(std::vector<std::unique_ptr<ECValueBindingInfo>>::const_iterator const& iterator)
                    : m_iterator(iterator)
                    {}
                ~const_iterator() {}
                //copyable
                const_iterator(const_iterator const& rhs) : m_iterator(rhs.m_iterator) {}
                const_iterator& operator= (const_iterator const& rhs)
                    {
                    if (this != &rhs)
                        m_iterator = rhs.m_iterator;

                    return *this;
                    }
                //moveable
                const_iterator(const_iterator&& rhs) : m_iterator(std::move(rhs.m_iterator)) {}

                const_iterator& operator=(const_iterator&& rhs)
                    {
                    if (this != &rhs)
                        m_iterator = std::move(rhs.m_iterator);

                    return *this;
                    }

                bool operator== (const_iterator const& rhs) const { return m_iterator == rhs.m_iterator; }
                bool operator!= (const_iterator const& rhs) const { return !(*this == rhs); }

                ECValueBindingInfo const* operator* () const { return m_iterator->get();  }
                const_iterator& operator++ () { m_iterator++; return *this; }
            };

    private:
        std::vector<std::unique_ptr<ECValueBindingInfo>> m_bindingInfos;

    public:
        //Intentionally use the compiler-generated versions of copy constructor, assignment operator, and destructor
        ECValueBindingInfoCollection() {}

        //!Generates and adds a new binding info for the given parameter.
        //!@return SUCCESS in case of success, ERROR otherwise
        BentleyStatus AddBindingInfo(ECN::ECClassCR ecClass, ECN::ECPropertyCR ecProperty, int ecsqlParameterIndex);
        BentleyStatus AddBindingInfo(ECN::ECEnablerCR ecEnabler, ECN::ECPropertyCR ecProperty, Utf8StringCR accessString, int ecsqlParameterIndex);
        //!Generates and adds a new binding info for the specified ECSQL System property for the given parameter.
        //!@return binding info in case of success, nullptr otherwise
        ECSqlSystemPropertyBindingInfo* AddBindingInfo(ECValueBindingInfo::SystemPropertyKind kind, int ecsqlParameterIndex);

        const_iterator begin() const;
        const_iterator end() const;
    };

//======================================================================================
//! Helper class for ECInstance ECSQL adapters
// @bsiclass                                                 Krischan.Eberle     06/2014
//+===============+===============+===============+===============+===============+======
struct ECInstanceAdapterHelper final
    {
    public:
        struct ECInstanceInfo final
            {
            private:
                ECN::IECInstanceCP m_instance = nullptr;
                bool m_allowPointersIntoInstanceMemory = true;
                ECInstanceId m_instanceId;
                ECInstanceId m_sourceId;
                ECInstanceId m_targetId;

                //not copyable
                ECInstanceInfo(ECInstanceInfo const&) = delete;
                ECInstanceInfo& operator=(ECInstanceInfo const&) = delete;

            public:
                explicit ECInstanceInfo(ECN::IECInstanceCR instance, bool allowPointersIntoInstanceMemory)
                    : m_instance(&instance), m_allowPointersIntoInstanceMemory(allowPointersIntoInstanceMemory)
                    {}

                ECInstanceInfo(ECN::IECInstanceCR instance, ECInstanceId instanceId, bool allowPointersIntoInstanceMemory)
                    : m_instance(&instance), m_allowPointersIntoInstanceMemory(allowPointersIntoInstanceMemory), m_instanceId(instanceId)
                    {}

                ECInstanceInfo(ECInstanceId instanceId, ECInstanceId sourceId, ECInstanceId targetId)
                    : ECInstanceInfo(instanceId, sourceId, targetId, nullptr)
                    {}

                ECInstanceInfo(ECInstanceId instanceId, ECInstanceId sourceId, ECInstanceId targetId, ECN::IECRelationshipInstanceCP relationshipProperties)
                    : m_instance(relationshipProperties), m_instanceId(instanceId), m_sourceId(sourceId), m_targetId(targetId)
                    {}

                bool HasInstance() const { return m_instance != nullptr; }
                ECN::IECInstanceCR GetInstance() const { BeAssert(HasInstance()); return *m_instance; }
                bool AllowPointersIntoInstanceMemory() const { BeAssert(HasInstance()); return m_allowPointersIntoInstanceMemory; }
                bool HasInstanceId() const { return m_instanceId.IsValid(); }
                ECInstanceId GetInstanceId() const { return m_instanceId; }
                ECInstanceId GetSourceId() const { return m_sourceId; }
                ECInstanceId GetTargetId() const { return m_targetId; }
            };
    private:
        ECInstanceAdapterHelper();
        ~ECInstanceAdapterHelper();

        static BentleyStatus BindPrimitiveValue(IECSqlBinder&, ECInstanceInfo const&, PrimitiveECValueBindingInfo const&);
        static BentleyStatus BindPrimitiveValue(IECSqlBinder&, ECN::ECValueCR);

        static BentleyStatus BindStructValue(IECSqlBinder&, ECInstanceInfo const&, StructECValueBindingInfo const&);
        static BentleyStatus BindArrayValue(IECSqlBinder&, ECInstanceInfo const&, ArrayECValueBindingInfo const&);
        static BentleyStatus BindNavigationValue(IECSqlBinder&, ECInstanceInfo const&, NavigationECValueBindingInfo const&);

        static BentleyStatus BindECSqlSystemPropertyValue(IECSqlBinder&, ECInstanceInfo const&, ECSqlSystemPropertyBindingInfo const&);

        static IECSqlBinder::MakeCopy DetermineMakeCopy(ECN::ECValueCR);

        static bool IsPropertyValueNull(ECN::IECInstanceCR, ECValueBindingInfo const&);

    public:
        //! Binds the respective ECValue (specified through @p valueBindingInfo) of the specified ECInstance
        //! to the specified ECSQL statement's binder.
        //! @param[in,out] binder ECSQL statement binder representing the ECSQL parameter to which the value is bound to the ECSQL statement
        //! @param[in] instance ECInstance from which the ECValue to be bound is extracted
        //! @param[in] valueBindingInfo Information needed to extract the ECValue from the right property value from the ECInstance
        static BentleyStatus BindValue(IECSqlBinder& binder, ECInstanceInfo const& instance, ECValueBindingInfo const& valueBindingInfo);

        static bool IsOrContainsCalculatedProperty(ECN::ECPropertyCR prop);

        static bool HasReadonlyPropertiesAreUpdatableOption(ECDbCR, ECN::ECClassCR, Utf8CP ecsqlOptions);

        //! Sets the ECInstanceId on the given ECInstance.
        //! @param[in,out] instance ECInstance on which the id is set
        //! @param[out] ecInstanceId Id to set
        //! @return SUCCESS or ERROR
        static BentleyStatus SetECInstanceId(ECN::IECInstanceR instance, ECInstanceId ecInstanceId);

        //! Creates an empty ECInstance for the given class. Supports relationship classes, too.
        static ECN::IECInstancePtr CreateECInstance(ECN::ECClassCR);

        //! Logs an error message based on the given information
        //! @param[in] operationName Name of the ECSQL operation for which the method is called. Should be one 
        //! out of 'insert', 'update', 'delete'.
        //! @param[in] instance ECInstance for which the operation failed
        //! @param[in] errorMessage Detailed error message
        static void LogFailure(Utf8CP operationName, ECN::IECInstanceCR instance, Utf8CP errorMessage);

        static bool Equals(ECN::ECClassCR lhs, ECN::ECClassCR rhs);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
