/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECDb/ECInstanceId.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <ECDb/ECDbTypes.h>
#include <Bentley/BeId.h>
#include <BeSQLite/BeSQLite.h>
#include <ECObjects/ECObjectsAPI.h>

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
    explicit ECInstanceId(BeInt64Id id) : ECInstanceId(id.GetValueUnchecked()) {}
    };

//=======================================================================================
//! An ECInstanceKey of an ECInstance is made up of the ECInstance's ECInstanceId and the ECN::ECClassId 
//! of the ECInstance's ECClass. 
//! @ingroup ECDbGroup
//+===============+===============+===============+===============+===============+======
struct ECInstanceKey final
    {
private:
    ECN::ECClassId m_ecClassId;
    ECInstanceId m_ecInstanceId;
   
public:
    //! Construct an empty/invalid ECInstanceKey
    ECInstanceKey() {}
    ~ECInstanceKey() {}

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
//! A @ref BentleyApi::BeSQLite::VirtualSet "VirtualSet" of @ref ECInstanceId "ECInstanceIds" 
//! that can be used to bind a list of ECInstanceIds to the parameter in the 
//! SQL function @b InVirtualSet in an ECSqlStatement.
//! @see ECSqlStatement::BindVirtualSet, ECDbCodeSampleECSqlStatementVirtualSets
//! @ingroup ECDbGroup
//+===============+===============+===============+===============+===============+======
typedef BeSQLite::IdSet<ECInstanceId> ECInstanceIdSet;

END_BENTLEY_SQLITE_EC_NAMESPACE
