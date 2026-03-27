/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BeSQLite::DbValue ECChangesetReader::GetValue(int col, Stage stage) const
    {
    BeAssert(m_currentChange != nullptr && "SetCurrentChange must be called before reading values");
    return m_currentChange->GetValue(col, stage);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool ECChangesetReader::IsColumnNull(int col, Stage stage) const
    {
    return GetValue(col, stage).IsNull();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool ECChangesetReader::GetValueBoolean(int col, Stage stage) const
    {
    return GetValue(col, stage).GetValueInt() != 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
double ECChangesetReader::GetValueDouble(int col, Stage stage) const
    {
    return GetValue(col, stage).GetValueDouble();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
int ECChangesetReader::GetValueInt(int col, Stage stage) const
    {
    return GetValue(col, stage).GetValueInt();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
int64_t ECChangesetReader::GetValueInt64(int col, Stage stage) const
    {
    return GetValue(col, stage).GetValueInt64();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Utf8CP ECChangesetReader::GetValueText(int col, Stage stage) const
    {
    return GetValue(col, stage).GetValueText();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
int ECChangesetReader::GetColumnBytes(int col, Stage stage) const
    {
    return GetValue(col, stage).GetValueBytes();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void const* ECChangesetReader::GetValueBlob(int col, Stage stage) const
    {
    return GetValue(col, stage).GetValueBlob();
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
