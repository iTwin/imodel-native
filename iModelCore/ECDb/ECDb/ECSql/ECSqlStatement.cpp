/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlStatement.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <ECDb/ECSqlStatement.h>
#include "ECSqlStatementImpl.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//******************* ECSqlStatement ************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle   06/2013
//---------------------------------------------------------------------------------------
ECSqlStatement::ECSqlStatement ()
    : m_pimpl (new ECSqlStatement::Impl ())
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      06/2013
//---------------------------------------------------------------------------------------
ECSqlStatement::~ECSqlStatement ()
    {
    if (m_pimpl != nullptr)
        {        
        delete m_pimpl;
        m_pimpl = nullptr;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle   06/2013
//---------------------------------------------------------------------------------------
ECSqlStatement::ECSqlStatement (ECSqlStatement&& rhs)
    : m_pimpl (std::move (rhs.m_pimpl))
    {
    //nulling out the pimpl on the RHS to avoid that rhs' destructor tries to delete it
    rhs.m_pimpl = nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle   06/2013
//---------------------------------------------------------------------------------------
ECSqlStatement& ECSqlStatement::operator= (ECSqlStatement&& rhs)
    {
    if (this != &rhs)
        {
        m_pimpl = std::move (rhs.m_pimpl);

        //nulling out the pimpl on the RHS to avoid that rhs' destructor tries to delete it
        rhs.m_pimpl = nullptr;
        }

    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle   06/2013
//---------------------------------------------------------------------------------------
void ECSqlStatement::Finalize ()
    {
    m_pimpl->Finalize ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      06/2013
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlStatement::Prepare (ECDbCR ecdb, Utf8CP ecsql)
    {
    return m_pimpl->Prepare (ecdb, ecsql);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      06/2013
//---------------------------------------------------------------------------------------
bool ECSqlStatement::IsPrepared () const 
    {
    return m_pimpl->IsPrepared ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle     07/2013
//---------------------------------------------------------------------------------------
IECSqlBinder& ECSqlStatement::GetBinder (int parameterIndex)
    {
    return m_pimpl->GetBinder (parameterIndex);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle     07/2013
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlStatement::BindNull (int parameterIndex)
    {
    return GetBinder (parameterIndex).BindNull ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle     07/2013
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlStatement::BindBoolean (int parameterIndex, bool value)
    {
    return GetBinder (parameterIndex).BindBoolean (value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle     07/2013
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlStatement::BindBinary (int parameterIndex, const void* value, int binarySize, IECSqlBinder::MakeCopy makeCopy)
    {
    return GetBinder (parameterIndex).BindBinary (value, binarySize, makeCopy);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle     07/2013
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlStatement::BindDateTime (int parameterIndex, DateTimeCR value)
    {
    return GetBinder (parameterIndex).BindDateTime (value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle     07/2013
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlStatement::BindDouble (int parameterIndex, double value)
    {
    return GetBinder (parameterIndex).BindDouble (value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle     11/2014
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlStatement::BindGeometry (int parameterIndex, IGeometryCR value)
    {
    return GetBinder (parameterIndex).BindGeometry (value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle     07/2013
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlStatement::BindInt (int parameterIndex, int value)
    {
    return GetBinder (parameterIndex).BindInt (value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle     07/2013
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlStatement::BindInt64 (int parameterIndex, int64_t value)
    {
    return GetBinder (parameterIndex).BindInt64 (value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle     07/2013
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlStatement::BindPoint2D (int parameterIndex, DPoint2dCR value)
    {
    return GetBinder (parameterIndex).BindPoint2D (value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle     07/2013
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlStatement::BindPoint3D (int parameterIndex, DPoint3dCR value)
    {
    return GetBinder (parameterIndex).BindPoint3D (value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle     07/2013
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlStatement::BindText (int parameterIndex, Utf8CP value, IECSqlBinder::MakeCopy makeCopy, int byteCount)
    {
    return GetBinder (parameterIndex).BindText (value, makeCopy, byteCount);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle     07/2013
//---------------------------------------------------------------------------------------
IECSqlStructBinder& ECSqlStatement::BindStruct (int parameterIndex) 
    {
    return GetBinder (parameterIndex).BindStruct ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle     07/2013
//---------------------------------------------------------------------------------------
IECSqlArrayBinder& ECSqlStatement::BindArray (int parameterIndex, uint32_t initialCapacity)
    {
    return GetBinder (parameterIndex).BindArray (initialCapacity);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle     07/2013
//---------------------------------------------------------------------------------------
int ECSqlStatement::GetParameterIndex (Utf8CP parameterName) const 
    {
    return m_pimpl->GetParameterIndex (parameterName);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      06/2013
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlStatement::ClearBindings ()
    {
    return m_pimpl->ClearBindings ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      06/2013
//---------------------------------------------------------------------------------------
ECSqlStepStatus ECSqlStatement::Step ()
    {
    return m_pimpl->Step ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      06/2013
//---------------------------------------------------------------------------------------
ECSqlStepStatus ECSqlStatement::Step (ECInstanceKey& ecInstanceKey) const
    {
    return m_pimpl->Step (ecInstanceKey);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      06/2013
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlStatement::Reset ()
    {
    return m_pimpl->Reset ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      06/2013
//---------------------------------------------------------------------------------------
int ECSqlStatement::GetColumnCount () const
    {
    return m_pimpl->GetColumnCount ();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlColumnInfo const& ECSqlStatement::GetColumnInfo (int columnIndex) const
    {
    //Reports errors (not prepared yet, index out of bounds) and uses no-op field in case of error
    return GetValue (columnIndex).GetColumnInfo ();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 04/2013
//+---------------+---------------+---------------+---------------+---------------+------
bool ECSqlStatement::IsValueNull (int columnIndex) const
    {
    //Reports errors (not prepared yet, index out of bounds) and uses no-op field in case of error
    return GetValue (columnIndex).IsNull ();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 04/2013
//+---------------+---------------+---------------+---------------+---------------+------
void const* ECSqlStatement::GetValueBinary (int columnIndex, int* binarySize) const
    {
    //Reports errors (not prepared yet, index out of bounds) and uses no-op field in case of error
    return GetValue (columnIndex).GetBinary (binarySize);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 04/2013
//+---------------+---------------+---------------+---------------+---------------+------
bool ECSqlStatement::GetValueBoolean (int columnIndex) const
    {
    //Reports errors (not prepared yet, index out of bounds) and uses no-op field in case of error
    return GetValue (columnIndex).GetBoolean ();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 04/2013
//+---------------+---------------+---------------+---------------+---------------+------
DateTime ECSqlStatement::GetValueDateTime (int columnIndex) const
    {
    //Reports errors (not prepared yet, index out of bounds) and uses no-op field in case of error
    return GetValue (columnIndex).GetDateTime ();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 04/2013
//+---------------+---------------+---------------+---------------+---------------+------
double ECSqlStatement::GetValueDouble (int columnIndex) const
    {
    //Reports errors (not prepared yet, index out of bounds) and uses no-op field in case of error
    return GetValue (columnIndex).GetDouble ();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 11/2014
//+---------------+---------------+---------------+---------------+---------------+------
IGeometryPtr ECSqlStatement::GetValueGeometry (int columnIndex) const
    {
    //Reports errors (not prepared yet, index out of bounds) and uses no-op field in case of error
    return GetValue (columnIndex).GetGeometry ();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 04/2013
//+---------------+---------------+---------------+---------------+---------------+------
int ECSqlStatement::GetValueInt (int columnIndex) const
    {
    //Reports errors (not prepared yet, index out of bounds) and uses no-op field in case of error
    return GetValue (columnIndex).GetInt ();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 04/2013
//+---------------+---------------+---------------+---------------+---------------+------
int64_t ECSqlStatement::GetValueInt64 (int columnIndex) const
    {
    //Reports errors (not prepared yet, index out of bounds) and uses no-op field in case of error
    return GetValue (columnIndex).GetInt64 ();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 04/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8CP ECSqlStatement::GetValueText (int columnIndex) const
    {
    //Reports errors (not prepared yet, index out of bounds) and uses no-op field in case of error
    return GetValue (columnIndex).GetText ();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 04/2013
//+---------------+---------------+---------------+---------------+---------------+------
DPoint2d ECSqlStatement::GetValuePoint2D (int columnIndex) const
    {
    //Reports errors (not prepared yet, index out of bounds) and uses no-op field in case of error
    return GetValue (columnIndex).GetPoint2D ();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 04/2013
//+---------------+---------------+---------------+---------------+---------------+------
DPoint3d ECSqlStatement::GetValuePoint3D (int columnIndex) const
    {
    //Reports errors (not prepared yet, index out of bounds) and uses no-op field in case of error
    return GetValue (columnIndex).GetPoint3D ();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 04/2013
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlArrayValue const& ECSqlStatement::GetValueArray (int columnIndex) const
    {
    //Reports errors (not prepared yet, index out of bounds) and uses no-op field in case of error
    return GetValue (columnIndex).GetArray ();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 04/2013
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlStructValue const& ECSqlStatement::GetValueStruct (int columnIndex) const
    {
    //Reports errors (not prepared yet, index out of bounds) and uses no-op field in case of error
    return GetValue (columnIndex).GetStruct ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle     03/2014
//---------------------------------------------------------------------------------------
IECSqlValue const& ECSqlStatement::GetValue (int columnIndex) const
    {
    //Reports errors (not prepared yet, index out of bounds) and uses no-op field in case of error
    return m_pimpl->GetValue (columnIndex);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      10/2013
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlStatement::GetLastStatus () const
    {
    return m_pimpl->GetLastStatus ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      07/2013
//---------------------------------------------------------------------------------------
Utf8String ECSqlStatement::GetLastStatusMessage () const
    {
    return m_pimpl->GetLastStatusMessage ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      06/2014
//---------------------------------------------------------------------------------------
Utf8CP ECSqlStatement::GetECSql () const
    {
    return m_pimpl->GetECSql ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
Utf8CP ECSqlStatement::GetNativeSql() const
    {
    return m_pimpl->GetNativeSql ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      02/2014
//---------------------------------------------------------------------------------------
ECDbCP ECSqlStatement::GetECDb () const
    {
    return m_pimpl->GetECDb ();
    }


END_BENTLEY_SQLITE_EC_NAMESPACE