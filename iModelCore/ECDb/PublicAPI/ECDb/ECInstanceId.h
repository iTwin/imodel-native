/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECDb/ECInstanceId.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <ECDb/ECDbTypes.h>
#include <limits>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! ECInstanceId is the unique id of an ECInstance in an @ref ECDbFile "ECDb file". 
//! @see @ref ECInstanceIdInECDb, ECInstanceIdHelper
//! @ingroup ECDbGroup
//+===============+===============+===============+===============+===============+======
struct ECInstanceId : BeRepositoryBasedId
    {
public:
    //! Constructs an empty, i.e. invalid ECInstanceId.
    ECInstanceId() : BeRepositoryBasedId() {}

    //! Constructs an ECInstanceId from a 64 bit value.
    //! @param[in] id Numeric value of the ECInstanceId, that must be composed of the RepositoryId and the local id.
    //! (see overload ECInstanceId::ECInstanceId(BeRepositoryId,UInt32) )
    explicit ECInstanceId(int64_t id) : BeRepositoryBasedId(id) {}
    
    //! Constructs an ECInstanceId from a RepositoryId value and an id.
    //! @param[in] repositoryId RepositoryId
    //! @param[in] id Id locally unique for the given @p repositoryId
    ECInstanceId(BeRepositoryId repositoryId, uint32_t id) : BeRepositoryBasedId(repositoryId, id) {}
    };

//=======================================================================================
//! An ECInstanceKey of an ECInstance is made up of the ECInstance's ECInstanceId and the ECN::ECClassId 
//! of the ECInstance's ECClass. 
//! @ingroup ECDbGroup
//+===============+===============+===============+===============+===============+======
struct ECInstanceKey
    {
private:
    ECN::ECClassId m_ecClassId;
    ECInstanceId m_ecInstanceId;

public:
    //! Construct an empty/invalid ECInstanceKey
    ECInstanceKey() : m_ecClassId(-1LL) {}

    //! Construct an ECInstanceKey
    ECInstanceKey(ECN::ECClassId ecClassId, ECInstanceId const& ecInstanceId) : m_ecClassId(ecClassId), m_ecInstanceId(ecInstanceId) {}

    //! Compare this ECInstanceKey with another key for equality
    bool operator == (ECInstanceKey const& other) const 
        { 
        return m_ecClassId == other.m_ecClassId && m_ecInstanceId == other.m_ecInstanceId; 
        }
    
    //! Compare this ECInstanceKey with another key for inequality
    bool operator != (ECInstanceKey const& other) const
        {
        return m_ecClassId != other.m_ecClassId || m_ecInstanceId != other.m_ecInstanceId;
        }

    //! Compare this ECInstanceKey with another key for ordering
    bool operator < (ECInstanceKey const& other) const
        {
        if (m_ecClassId < other.m_ecClassId)
            {
            return true;
            }
        if (m_ecClassId > other.m_ecClassId)
            {
            return false;
            }
        return m_ecInstanceId < other.m_ecInstanceId;
        }

    //! Get the ECClassId of this key
    ECN::ECClassId GetECClassId() const { return m_ecClassId; }
    //! Get the ECInstanceId of this key
    ECInstanceId GetECInstanceId() const { return m_ecInstanceId; }
    
    //! Test if this key is valid
    bool IsValid() const 
        { 
        return (m_ecClassId > 0LL && m_ecInstanceId.IsValid()); 
        }
    };

typedef ECInstanceKey const& ECInstanceKeyCR;
typedef ECInstanceKey const* ECInstanceKeyCP;
typedef ECInstanceKey& ECInstanceKeyR;

//=======================================================================================
//! Provides functionality related to an ECInstanceId
//! @see ECInstanceId
//! @ingroup ECDbGroup
//+===============+===============+===============+===============+===============+======
struct ECInstanceIdHelper
    {
private:
    ECInstanceIdHelper();
    ~ECInstanceIdHelper();

public:
    //! Required number of characters to represent an ECInstanceId as string.
    //! @see ECInstanceIdHelper::ToString
    static const size_t ECINSTANCEID_STRINGBUFFER_LENGTH = std::numeric_limits<uint64_t>::digits + 1; //+1 for the trailing 0 character

    //! Converts the specified ECInstanceId to its string representation.
    //! 
    //! Typical example:
    //!
    //!     WChar idStrBuffer[ECInstanceIdHelper::ECINSTANCEID_STRINGBUFFER_LENGTH];
    //!     bool success = ECInstanceIdHelper::ToString (idStrBuffer, ECInstanceIdHelper::ECINSTANCEID_STRINGBUFFER_LENGTH, ecInstanceId);
    //!
    //! @remarks The string representation can be used as an ECN::IECInstance's InstanceId
    //! (see ECN::IECInstance::SetInstanceId).
    //! @param[in,out] stringBuffer The output buffer for the ECInstanceId string. Must be large enough
    //! to hold the maximal number of decimal digits of UInt64 plus the trailing 0 character.
    //! You can use ECInstanceIdHelper::ECINSTANCEID_STRINGBUFFER_LENGTH to allocate the @p stringBuffer.
    //! @param[in] stringBufferLength Number of characters allocated in @p stringBuffer
    //! @param[in] ecInstanceId ECInstanceId to convert
    //! @return true in case of success, false if @p ecInstanceId is not valid or if @p stringBuffer is too small.
    ECDB_EXPORT static bool ToString(WCharP stringBuffer, size_t stringBufferLength, ECInstanceId const& ecInstanceId);

    //! Converts the ECInstanceId string to an ECInstanceId.
    //! @remarks In order to parse correctly, the ECInstanceId string must contain an unsigned number in decimal format.
    //! @param[out] ecInstanceId resulting ECInstanceId
    //! @param[in] ecInstanceIdString ECInstanceId string to convert
    //! @return true in case of success, false otherwise
    ECDB_EXPORT static bool FromString(ECInstanceId& ecInstanceId, WCharCP ecInstanceIdString);
    };

//=======================================================================================
//! @ingroup ECDbGroup
//@bsiclass                                                 Ramanujam.Raman      02/2014
//+===============+===============+===============+===============+===============+======
struct ECInstanceIdSet : bset<ECInstanceId>, BeSQLite::VirtualSet
    {
    virtual bool _IsInSet(int nVals, DbValue const* vals) const override;
    };

END_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
// Standard way of defining XxxKey and XxxKeyCR types (enhances API type safety)
//=======================================================================================
#define ECINSTANCEKEY_SUBCLASS(classname, subclassname) \
    struct classname : subclassname \
    {\
        classname() : subclassname() {} \
        classname(ECN::ECClassId classId, BeSQLite::EC::ECInstanceId instanceId) : subclassname(classId, instanceId) {} \
        explicit classname (BeSQLite::EC::ECInstanceKeyCR key) : subclassname (key) {} \
    };\
    typedef classname const& classname##CR;
