/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECDb/ECInstanceId.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <ECDb/ECDbTypes.h>
#include <Bentley/BeId.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! ECInstanceId is the unique id of an ECInstance in an @ref ECDbFile "ECDb file". 
//! @see @ref ECInstanceIdInECDb
//! @ingroup ECDbGroup
//+===============+===============+===============+===============+===============+======
struct ECInstanceId : BeInt64Id
    {
public:
    BEINT64_ID_DECLARE_MEMBERS(ECInstanceId, BeInt64Id)

public:
    explicit ECInstanceId(BeBriefcaseBasedId id) : ECInstanceId(id.GetValue()) {}

    //! Converts the ECInstanceId string to an ECInstanceId.
    //! @remarks In order to parse correctly, the ECInstanceId string must contain an unsigned number in decimal format.
    //! @param[out] ecInstanceId resulting ECInstanceId
    //! @param[in] ecInstanceIdString ECInstanceId string to convert
    //! @return SUCCESS if the string could be converted to a valid ECInstanceId. ERROR otherwise.
    ECDB_EXPORT static BentleyStatus FromString(ECInstanceId& ecInstanceId, Utf8CP ecInstanceIdString);
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
    ECInstanceKey() {}

    //! Construct an ECInstanceKey
    ECInstanceKey(ECN::ECClassId ecClassId, ECInstanceId ecInstanceId) : m_ecClassId(ecClassId), m_ecInstanceId(ecInstanceId) {}

    //! Compare this ECInstanceKey with another key for equality
    bool operator==(ECInstanceKey const& other) const { return m_ecClassId == other.m_ecClassId && m_ecInstanceId == other.m_ecInstanceId; }
    
    //! Compare this ECInstanceKey with another key for inequality
    bool operator!=(ECInstanceKey const& other) const { return !(*this == other); }

    //! Compare this ECInstanceKey with another key for ordering
    bool operator<(ECInstanceKey const& other) const
        {
        if (m_ecClassId < other.m_ecClassId)
            return true;

        if (m_ecClassId > other.m_ecClassId)
            return false;

        return m_ecInstanceId < other.m_ecInstanceId;
        }

    //! Get the ECClassId of this key
    ECN::ECClassId GetECClassId() const { return m_ecClassId; }
    //! Get the ECInstanceId of this key
    ECInstanceId GetECInstanceId() const { return m_ecInstanceId; }
    
    //! Test if this key is valid
    bool IsValid() const { return (m_ecClassId.IsValid() && m_ecInstanceId.IsValid()); }
    };

typedef ECInstanceKey const& ECInstanceKeyCR;
typedef ECInstanceKey const* ECInstanceKeyCP;
typedef ECInstanceKey& ECInstanceKeyR;

//=======================================================================================
//! A VirtualSet of @ref ECInstanceId "ECInstanceIds" that can be used to bind a list
//! of ECInstanceIds to the parameter in the SQL function @b InVirtualSet in an ECSqlStatement.
//! @see ECSqlStatement::BindVirtualSet, ECDbCodeSampleECSqlStatementVirtualSets
//! @ingroup ECDbGroup
//+===============+===============+===============+===============+===============+======
struct ECInstanceIdSet : bset<ECInstanceId>, BeSQLite::VirtualSet
{
private:
    virtual bool _IsInSet(int nVals, DbValue const* vals) const
        {
        BeAssert(nVals == 1);
        return this->end() != this->find(vals[0].GetValueId<ECInstanceId>());
        }
};

END_BENTLEY_SQLITE_EC_NAMESPACE
