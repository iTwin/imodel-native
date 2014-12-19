//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/win/h/HFCProcess.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

namespace HFCProcess
{

bool               TerminateProcess(uint32_t pi_ProcessID);

bool               SafeTerminateProcess(HANDLE ProcessHandle, uint32_t ExitTimeout, uint32_t ExitCode = 1);

bool               SetPrivilege(HANDLE hToken, LPCWSTR Privilege, BOOL bEnablePrivilege);

};