/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <BeSQLite/BeSQLite.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

struct ECDb;

//=======================================================================================
//! Abstraction over a row-based data source that ECSqlField and its derived classes
//! use to read raw column values. Implemented by ECSqlSelectPreparedStatement (backed
//! by a BeSQLite::Statement) and by ECChangesetReader (backed by a Changes::Change).
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct IECSqlFieldReader
    {
    public:
        virtual ~IECSqlFieldReader() {}

        virtual ECDb const& GetECDb() const = 0;

        virtual bool IsColumnNull(int col) const = 0;
        virtual bool GetValueBoolean(int col) const = 0;
        virtual double GetValueDouble(int col) const = 0;
        virtual int GetValueInt(int col) const = 0;
        virtual int64_t GetValueInt64(int col) const = 0;
        virtual Utf8CP GetValueText(int col) const = 0;
        virtual int GetColumnBytes(int col) const = 0;
        virtual void const* GetValueBlob(int col) const = 0;
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
