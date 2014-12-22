/*--------------------------------------------------------------------------------------+
|
|     $Source: src/native/log4cxx/dllmain.cpp $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
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

/*---------------------------------------------------------------------------------**//**
* @description  This function is required to be defined with the correct 24 character
*               password functions so that it can be loaded by applications such as
*               PowerDraft that require protected DLM's.
* @bsimethod                                                    VijaiKalyan     04/06
+---------------+---------------+---------------+---------------+---------------+------*/
extern "C" void __declspec(dllexport) dlmPassword
(
char*           passwordP,      /* <==> Password back                     */
char*           /*appNameP*/        /* <==> Application name                  */
)
    {                  //UStation   Draft       PowerMap    PowerMapFld View        Navigator
    strcpy (passwordP, "78TB204409CO70XRB48PD97O9BZ4P6RNZ3QU87Q9S3WYO0LR96R8T2VZN7UQB9WO51UKQBZP");
    }

