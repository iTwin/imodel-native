/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/IECSqlValue.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//=======================================================================================
//! IECSqlRow represent a single row that can be read. It can be pass around to allow
//! only reading a instance.
//+======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE IECSqlRow {
    public:
        //! Gets the number of ECSQL columns in the result set returned after calling Step on a SELECT statement.
        //! @return Number of ECSQL columns in the result set
        virtual int GetColumnCount() const = 0;

        //! Gets the value of the specified column.
        //! @remarks This is the generic way of getting the value of a specified column in the result set.
        //! All other GetValueXXX methods are convenience methods around GetValue.
        //! @return Value for the column
        //! @note Possible errors:
        //! - @p columnIndex is out of bounds
        virtual IECSqlValue const& GetValue(int columnIndex) const = 0;
};

END_BENTLEY_SQLITE_EC_NAMESPACE
