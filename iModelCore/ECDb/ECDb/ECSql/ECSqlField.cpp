/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlField.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECSqlStatementBase.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      10/2013
//---------------------------------------------------------------------------------------
//static
ECSqlField::Collection* ECSqlField::s_emptyChildCollection = nullptr;

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      09/2013
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlField::OnAfterStep()
    {
    ECSqlStatus stat = _OnAfterStep();
    if (!stat.IsSuccess())
        return stat;

    for (std::unique_ptr<ECSqlField> const& child : GetChildren())
        {
        stat = child->OnAfterStep();
        if (!stat.IsSuccess())
            return stat;
        }

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
ECSqlColumnInfoCR ECSqlField::_GetColumnInfo() const
    {
    return m_ecsqlColumnInfo;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      10/2013
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlField::OnAfterReset()
    {
    ECSqlStatus stat = _OnAfterReset();
    if (!stat.IsSuccess())
        return stat;

    for (std::unique_ptr<ECSqlField> const& child : GetChildren())
        {
        stat = child->OnAfterReset();
        if (!stat.IsSuccess())
            return stat;
        }

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      09/2013
//---------------------------------------------------------------------------------------
ECSqlStatementBase& ECSqlField::GetECSqlStatementR() const
    {
    return m_ecsqlStatement;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      09/2013
//---------------------------------------------------------------------------------------
Statement& ECSqlField::GetSqliteStatement() const
    {
    BeAssert(GetECSqlStatementR().IsPrepared());
    return GetECSqlStatementR().GetPreparedStatementP()->GetSqliteStatementR();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//+---------------+---------------+---------------+---------------+---------------+------
//static
Utf8CP ECSqlField::GetPrimitiveGetMethodName(ECN::PrimitiveType getMethodType)
    {
    switch (getMethodType)
        {
            case PRIMITIVETYPE_Binary:
                return "GetBlob";
            case PRIMITIVETYPE_Boolean:
                return "GetBoolean";
            case PRIMITIVETYPE_DateTime:
                return "GetDateTime";
            case PRIMITIVETYPE_Double:
                return "GetDouble";
            case PRIMITIVETYPE_IGeometry:
                return "GetGeometry";
            case PRIMITIVETYPE_Integer:
                return "GetInt";
            case PRIMITIVETYPE_Long:
                return "GetInt64";
            case PRIMITIVETYPE_Point2d:
                return "GetPoint2d";
            case PRIMITIVETYPE_Point3d:
                return "GetPoint3d";
            case PRIMITIVETYPE_String:
                return "GetText";

            default:
                BeAssert(false && "ECSqlField::GetPrimitiveGetMethodName needs to be adjusted to new ECN::PrimitiveType value");
                return "";
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlField::Collection const& ECSqlField::GetEmptyChildCollection()
    {
    if (s_emptyChildCollection == nullptr)
        s_emptyChildCollection = new Collection();

    return *s_emptyChildCollection;
    }



END_BENTLEY_SQLITE_EC_NAMESPACE
