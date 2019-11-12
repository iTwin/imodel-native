/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <WebServices/iModelHub/Common.h>
#include <WebServices/iModelHub/Client/iModelConnection.h>

BEGIN_BENTLEY_IMODELHUB_NAMESPACE

//=======================================================================================
//@bsiclass                                      Algirdas.Mikoliunas             01/2017
//=======================================================================================
struct FileLock : NonCopyableClass
{
private:
    BeFileName m_lockFilePath;
    bool m_locked = false;
public:
    FileLock(BeFileName filePath) {m_lockFilePath = filePath; m_lockFilePath.AppendExtension(L"lock");}
    ~FileLock();
    bool Lock();
};

END_BENTLEY_IMODELHUB_NAMESPACE
