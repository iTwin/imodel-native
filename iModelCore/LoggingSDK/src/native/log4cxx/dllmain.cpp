/*--------------------------------------------------------------------------------------+
|
|     $Source: src/native/log4cxx/dllmain.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <windows.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
BOOL APIENTRY DllMain
(
HANDLE /*hModule*/,
DWORD  ul_reason_for_call,
LPVOID /*lpReserved*/
)
    {
    switch (ul_reason_for_call)
        {
        case DLL_PROCESS_ATTACH:
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
        break;
        }
    return TRUE;
    }
