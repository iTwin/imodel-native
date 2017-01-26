/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/NavigationPropertyECSqlField.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle      11/2016
//---------------------------------------------------------------------------------------
void NavigationPropertyECSqlField::SetMembers(std::unique_ptr<ECSqlField> idField, std::unique_ptr<ECSqlField> relClassIdField)
    {
    BeAssert(idField != nullptr && !idField->RequiresOnAfterReset() && !idField->RequiresOnAfterStep());
    BeAssert(relClassIdField != nullptr && !relClassIdField->RequiresOnAfterReset() && !relClassIdField->RequiresOnAfterStep());
    m_idField = std::move(idField);
    m_relClassIdField = std::move(relClassIdField);
    BeAssert(m_idField != nullptr && m_relClassIdField != nullptr);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle      11/2016
//---------------------------------------------------------------------------------------
IECSqlValue const& NavigationPropertyECSqlField::_GetValue(int columnIndex) const
    {
    switch (columnIndex)
        {
            case 0:
                return *m_idField;

            case 1:
                return *m_relClassIdField;

            default:
            {
            LOG.errorv("Column index '%d' passed to IECSqlStructValue::GetValue is out of bounds.", columnIndex);
            return NoopECSqlValue::GetSingleton().GetValue(columnIndex);
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle      11/2016
//---------------------------------------------------------------------------------------
IECSqlPrimitiveValue const& NavigationPropertyECSqlField::_GetPrimitive() const
    {
    LOG.error("GetPrimitive cannot be called for a NavigationECProperty column. Call GetValueNavigation instead.");
    return NoopECSqlValue::GetSingleton().GetPrimitive();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle      11/2016
//---------------------------------------------------------------------------------------
IECSqlArrayValue const& NavigationPropertyECSqlField::_GetArray() const
    {
    LOG.error("GetArray cannot be called for a NavigationECProperty column. Call GetValueNavigation instead.");
    return NoopECSqlValue::GetSingleton().GetArray();
    }

END_BENTLEY_SQLITE_EC_NAMESPACE