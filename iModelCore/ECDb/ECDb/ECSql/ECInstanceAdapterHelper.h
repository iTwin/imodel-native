/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECInstanceAdapterHelper.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include <ECDb/IECSqlBinder.h>
#include <ECDb/ECInstanceId.h>
#include <Bentley/NonCopyableClass.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//======================================================================================
//! Contains metadata for how properties of given ECClass map to parameters in the ECSQL
//! statement used by ECInstanceInserter, ECInstanceUpdater, ECInstanceDeleter
// @bsiclass                                                 Krischan.Eberle     06/2014
//+===============+===============+===============+===============+===============+======
struct ECValueBindingInfo : NonCopyableClass
    {
public:
    enum class Type
        {
        Primitive,
        Struct,
        Array,
        ECSqlSystemProperty
        };

    enum class SystemPropertyKind
        {
        ECInstanceId,
        SourceECInstanceId,
        SourceECClassId,
        TargetECInstanceId,
        TargetECClassId
        };

protected:
    static const int UNSET_INDEX = 0;

private:
    Type m_type;
    int m_ecsqlParameterIndex;

protected:
    ECValueBindingInfo (Type type, int ecsqlParameterIndex)
        : m_type (type), m_ecsqlParameterIndex (ecsqlParameterIndex)
        {}

public:
    virtual ~ECValueBindingInfo () {}

    //! Only binding infos for top-level properties have a corresponding ECSQL parameter index.
    //! Binding infos for nested properties don't. Use this to tell between the two.
    bool HasECSqlParameterIndex () const;
    //! Gets the ECSQL parameter index to which this binding info maps to.
    //! Only call this for top-level properties. Don't call it for nested properties.
    int GetECSqlParameterIndex () const;

    Type GetType () const;
    };

//======================================================================================
//! Factory to create ECValueBindingInfos
// @bsiclass                                                 Krischan.Eberle     06/2014
//+===============+===============+===============+===============+===============+======
struct ECValueBindingInfoFactory
    {
private:
    ECValueBindingInfoFactory ();
    ~ECValueBindingInfoFactory ();

public:
    static std::unique_ptr<ECValueBindingInfo> CreateBindingInfo (ECN::ECEnablerCR enabler, ECN::ECPropertyCR ecProperty, Utf8CP propertyAccessString, int ecsqlParameterIndex);
    static std::unique_ptr<ECValueBindingInfo> CreateSystemBindingInfo (ECValueBindingInfo::SystemPropertyKind kind, int ecsqlParameterIndex);
    };

//======================================================================================
//! BindingInfo for ECSQL System properties
// @bsiclass                                                 Krischan.Eberle     07/2014
//+===============+===============+===============+===============+===============+======
struct ECSqlSystemPropertyBindingInfo : ECValueBindingInfo
    {
public:

private:
    SystemPropertyKind m_kind;

    ECSqlSystemPropertyBindingInfo (SystemPropertyKind kind, int ecsqlParameterIndex);

public:
    static std::unique_ptr<ECSqlSystemPropertyBindingInfo> Create (SystemPropertyKind kind, int ecsqlParameterIndex);
    ~ECSqlSystemPropertyBindingInfo () {}

    SystemPropertyKind GetKind () const;
    };

//======================================================================================
//! ECValueBindingInfo for primitive ECProperties
// @bsiclass                                                 Krischan.Eberle     06/2014
//+===============+===============+===============+===============+===============+======
struct PrimitiveECValueBindingInfo : ECValueBindingInfo
    {
private:
    uint32_t m_propertyIndex;

    PrimitiveECValueBindingInfo (uint32_t propertyIndex, int ecsqlParameterIndex);

public:
    static std::unique_ptr<PrimitiveECValueBindingInfo> Create (uint32_t propertyIndex, int ecsqlParameterIndex);
    ~PrimitiveECValueBindingInfo () {}

    uint32_t GetPropertyIndex () const;
    };

//======================================================================================
//! ECValueBindingInfo for struct ECProperties
// @bsiclass                                                 Krischan.Eberle     06/2014
//+===============+===============+===============+===============+===============+======
struct StructECValueBindingInfo : ECValueBindingInfo
    {
private:
    std::map<ECN::ECPropertyId, std::unique_ptr<ECValueBindingInfo>> m_memberBindingInfos;

    StructECValueBindingInfo (ECN::ECEnablerCR parentEnabler, ECN::ECClassCR structType, Utf8CP parentPropertyAccessString, int ecsqlParameterIndex);

public:
    static std::unique_ptr<StructECValueBindingInfo> Create (ECN::ECEnablerCR parentEnabler, ECN::ECClassCR structType, Utf8CP parentPropertyAccessString, int ecsqlParameterIndex);
    static std::unique_ptr<StructECValueBindingInfo> CreateForNestedStruct (ECN::ECClassCR structType);

    ~StructECValueBindingInfo () {}

    std::map<ECN::ECPropertyId, std::unique_ptr<ECValueBindingInfo>> const& GetMemberBindingInfos () const;
    };

//======================================================================================
//! ECValueBindingInfo for array ECProperties
// @bsiclass                                                 Krischan.Eberle     06/2014
//+===============+===============+===============+===============+===============+======
struct ArrayECValueBindingInfo : ECValueBindingInfo
    {
private:
    uint32_t m_arrayPropIndex;
    std::unique_ptr<StructECValueBindingInfo> m_structArrayElementBindingInfo;

    ArrayECValueBindingInfo (ECN::ECPropertyCR prop, uint32_t arrayPropIndex, int ecsqlParameterIndex);

public:
    static std::unique_ptr<ArrayECValueBindingInfo> Create (ECN::ECPropertyCR prop, uint32_t arrayPropIndex, int ecsqlParameterIndex);

    ~ArrayECValueBindingInfo () {}
    uint32_t GetArrayPropertyIndex () const;
    
    //! Indicates whether this is a binding for a struct array or not.
    bool IsStructArray () const;
    //! Returns the binding info for the struct element type, in case this is binding info for a struct array
    StructECValueBindingInfo const& GetStructArrayElementBindingInfo () const;
    };

//======================================================================================
//! Collection of ECValueBindingInfos
// @bsiclass                                                 Krischan.Eberle     06/2014
//+===============+===============+===============+===============+===============+======
struct ECValueBindingInfoCollection
    {
public:
    //======================================================================================
    // @bsiclass                                                 Krischan.Eberle     06/2014
    //+===============+===============+===============+===============+===============+======
    struct const_iterator : std::iterator<std::forward_iterator_tag, ECValueBindingInfo const*>
        {
    private:
        std::vector<std::unique_ptr<ECValueBindingInfo>>::const_iterator m_iterator;

    public:
        explicit const_iterator (std::vector<std::unique_ptr<ECValueBindingInfo>>::const_iterator const& iterator)
            : m_iterator (iterator)
            {}
        ~const_iterator () {}
        //copyable
        const_iterator (const_iterator const& rhs)
            : m_iterator (rhs.m_iterator)
            {}
        const_iterator& operator= (const_iterator const& rhs)
            {
            if (this != &rhs)
                m_iterator = rhs.m_iterator;

            return *this;
            }
        //moveable
        const_iterator (const_iterator&& rhs)
            : m_iterator (std::move (rhs.m_iterator))
            {}

        const_iterator& operator= (const_iterator&& rhs)
            {
            if (this != &rhs)
                m_iterator = std::move (rhs.m_iterator);

            return *this;
            }

        ECValueBindingInfo const* operator* () const
            {
            return m_iterator->get ();
            }

        const_iterator& operator++ ()
            {
            m_iterator++;
            return *this;
            }
        bool operator== (const_iterator const& rhs) const
            {
            return m_iterator == rhs.m_iterator;
            }

        bool operator!= (const_iterator const& rhs) const
            {
            return !(*this == rhs);
            }
        };

private:
    std::vector<std::unique_ptr<ECValueBindingInfo>> m_bindingInfos;

public:
    //Intentionally use the compiler-generated versions of copy constructor, assignment operator, and destructor

    ECValueBindingInfoCollection ()
        {}

    //!Generates and adds a new binding info for the given parameter.
    //!@return SUCCESS in case of success, ERROR otherwise
    BentleyStatus AddBindingInfo (ECN::ECClassCR ecClass, ECN::ECPropertyCR ecProperty, int ecsqlParameterIndex);
    BentleyStatus AddBindingInfo(ECN::ECEnablerCR ecEnabler, ECN::ECPropertyCR ecProperty, Utf8CP accessString, int ecsqlParameterIndex);
    //!Generates and adds a new binding info for the specified ECSQL System property for the given parameter.
    //!@return binding info in case of success, nullptr otherwise
    ECSqlSystemPropertyBindingInfo* AddBindingInfo (ECValueBindingInfo::SystemPropertyKind kind, int ecsqlParameterIndex);

    const_iterator begin () const;
    const_iterator end () const;
    };

//======================================================================================
//! Helper class for ECInstance ECSQL adapters
// @bsiclass                                                 Krischan.Eberle     06/2014
//+===============+===============+===============+===============+===============+======
struct ECInstanceAdapterHelper 
    {
public:
    struct ECInstanceInfo : NonCopyableClass
        {
    private:
        ECN::IECInstanceCR m_instance;
        ECInstanceId m_instanceId;

    public:
        explicit ECInstanceInfo (ECN::IECInstanceCR instance)
            : m_instance (instance)
            {}

        ECInstanceInfo (ECN::IECInstanceCR instance, ECInstanceId const& instanceId)
            : m_instance (instance), m_instanceId (instanceId)
            {}

        ECN::IECInstanceCR GetInstance () const {return m_instance;}
        bool HasInstanceId () const {return m_instanceId.IsValid ();}
        ECInstanceId const& GetInstanceId () const { return m_instanceId; }
        };
private:
    ECInstanceAdapterHelper ();
    ~ECInstanceAdapterHelper ();

    static BentleyStatus BindPrimitiveValue (IECSqlBinder& binder, ECInstanceInfo const& instance, PrimitiveECValueBindingInfo const& valueBindingInfo);
    static BentleyStatus BindPrimitiveValue (IECSqlBinder& binder, ECN::ECValueCR value);

    static BentleyStatus BindStructValue (IECSqlBinder& binder, ECInstanceInfo const& instance, StructECValueBindingInfo const& valueBindingInfo);
    static BentleyStatus BindArrayValue (IECSqlBinder& binder, ECInstanceInfo const& instance, ArrayECValueBindingInfo const& valueBindingInfo);

    static BentleyStatus BindECSqlSystemPropertyValue (IECSqlBinder& binder, ECInstanceInfo const& instance, ECSqlSystemPropertyBindingInfo const& valueBindingInfo);

    static IECSqlBinder::MakeCopy DetermineMakeCopy (ECN::ECValueCR ecValue);

public:
    //! Binds the respective ECValue (specified through @p valueBindingInfo) of the specified ECInstance
    //! to the specified ECSQL statement's binder.
    //! @param[in,out] binder ECSQL statement binder representing the ECSQL parameter to which the value is bound to the ECSQL statement
    //! @param[in] instance ECInstance from which the ECValue to be bound is extracted
    //! @param[in] valueBindingInfo Information needed to extract the ECValue from the right property value from the ECInstance
    static BentleyStatus BindValue (IECSqlBinder& binder, ECInstanceInfo const& instance, ECValueBindingInfo const& valueBindingInfo);
    
    static bool IsOrContainsCalculatedProperty (ECN::ECPropertyCR prop);

    static bool TryGetCurrentTimeStampProperty (ECN::ECPropertyCP& currentTimeStampProp, ECN::ECClassCR ecClass);

    static bool HasReadonlyPropertiesAreUpdatableOption(ECDbCR, ECN::ECClassCR, Utf8CP ecsqlOptions);

    //! Sets the ECInstanceId on the given ECInstance.
    //! @param[in,out] instance ECInstance on which the id is set
    //! @param[out] ecInstanceId Id to set
    //! @return SUCCESS or ERROR
    static BentleyStatus SetECInstanceId (ECN::IECInstanceR instance, ECInstanceId const& ecInstanceId);

    //! Creates an empty ECInstance for the given class. Supports relationship classes, too.
    static ECN::IECInstancePtr CreateECInstance(ECN::ECClassCR);

    //! Logs an error message based on the given information
    //! @param[in] operationName Name of the ECSQL operation for which the method is called. Should be one 
    //! out of 'insert', 'update', 'delete'.
    //! @param[in] instance ECInstance for which the operation failed
    //! @param[in] errorMessage Detailed error message
    static void LogFailure (Utf8CP operationName, ECN::IECInstanceCR instance, Utf8CP errorMessage);

    static bool Equals(ECN::ECClassCR lhs, ECN::ECClassCR rhs);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
