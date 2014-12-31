/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/JsonReader.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "JsonReaderImpl.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  06/2014
//+---------------+---------------+---------------+---------------+---------------+------
JsonReader::JsonReader (ECDbR ecdb, ECN::ECClassId ecClassId)
: m_pimpl (new Impl (ecdb, ecClassId))
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  06/2014
//+---------------+---------------+---------------+---------------+---------------+------
JsonReader::~JsonReader ()
    {
    if (m_pimpl != nullptr)
        {
        delete m_pimpl;
        m_pimpl = nullptr;
        }
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Ramanujam.Raman                 9 / 2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus JsonReader::Read
(
JsonValueR jsonInstances,
JsonValueR jsonDisplayInfo,
ECInstanceId const& ecInstanceId,
JsonECSqlSelectAdapter::FormatOptions formatOptions /* = JsonECSqlSelectAdapter::FormatOptions (ECValueFormat::FormattedStrings) */
) const
    {
    return m_pimpl->Read (jsonInstances, jsonDisplayInfo, ecInstanceId, formatOptions);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Ramanujam.Raman                 9 / 2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus JsonReader::ReadInstance
(
JsonValueR jsonValue,
ECInstanceId const& ecInstanceId,
JsonECSqlSelectAdapter::FormatOptions formatOptions /* = JsonECSqlSelectAdapter::FormatOptions (ECValueFormat::FormattedStrings) */
) const
    {
    return m_pimpl->ReadInstance (jsonValue, ecInstanceId, formatOptions);
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
