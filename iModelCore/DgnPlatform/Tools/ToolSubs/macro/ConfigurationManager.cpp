/*--------------------------------------------------------------------------------------+
|
|     $Source: Tools/ToolSubs/macro/ConfigurationManager.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/Bentley.h>
#include <DgnPlatform/ExportMacros.h>
#include <DgnPlatform/DgnPlatform.h>
#include <DgnPlatform/DesktopTools/ConfigurationManager.h>
#include <DgnPlatform/DesktopTools/MacroConfigurationAdmin.h>
#include <DgnPlatform/DesktopTools/envvutil.h>
#include "macro.h"
#include <RmgrTools/Tools/mdlResourceMT.h>
#include <RmgrTools/Tools/UglyStrings.h>
#include <DgnPlatform/DgnPlatformLib.h>

USING_NAMESPACE_BENTLEY_DGN

/*=================================================================================**//**
* An implementation of IConfigurationAdmin that accesses system environment variables and CSIDL.
* @bsiclass                                     Sam.Wilson                      05/2011
+===============+===============+===============+===============+===============+======*/
struct          EnvAdmin : IConfigurationAdmin
{
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
virtual BentleyStatus _GetConfigVariable (WStringR envStr, WCharCP envVar, ConfigurationVariableLevel level) override
    {
    if (NULL == envVar)
        return BSIERROR;

    util_getSysEnv (&envStr, envVar);

    ConfigurationManager::StringExpandMacros (envStr);
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   12/11
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool          _IsConfigVariableDefined (WCharCP cfgVarName, ConfigurationVariableLevel level) override
    {
    return (SUCCESS == util_getSysEnv (NULL, cfgVarName));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   12/11
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool          _IsConfigVariableDefinedAndTrue (WCharCP cfgVarName, ConfigurationVariableLevel level) override
    {
    WString     value;
    if (SUCCESS != util_getSysEnv (&value, cfgVarName))
        return false;
    if (0 == wcscmp (L"1", value.c_str()))
        return true;

    return (0 == BeStringUtilities::Wcsicmp (L"true", value.c_str()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
virtual BentleyStatus _DefineConfigVariable (WCharCP cfgVarName, WCharCP cfgValue, ConfigurationVariableLevel level) override
    {
    BeAssert (level == ConfigurationVariableLevel::User);

    if (NULL == cfgValue)
        cfgValue = L"?";       // *** NEEDS WORK: Need some way to mimic the ability of our macro system to define a variable without assigning it a value.

    return util_putenv (cfgVarName, cfgValue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
virtual BentleyStatus _UndefineConfigVariable (WCharCP cfgVarName) override
    {
    return util_putenv (cfgVarName, NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DanEast         11/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual BentleyStatus _IterateThroughVariables (IConfigVariableIteratorDelegate *delegate) override
    {
    return BSIERROR;
    }

}; // EnvAdmin

ConfigurationManager::T_GetAdminFunc ConfigurationManager::s_getAdminFunc;

static EnvAdmin s_envCfg;  // MT: We'll only use this when we have no host, in which case multi-threading is not supported.
static MacroConfigurationAdmin s_defaultCfg;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IConfigurationAdmin& ConfigurationManager::GetEnvAdmin()
    {
    return s_envCfg;     
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IConfigurationAdmin& ConfigurationManager::GetAdmin()
    {
    if (s_getAdminFunc)
        return s_getAdminFunc();
    
    return s_defaultCfg;     // MT: We'll only use this when we have no host, in which case multi-threading is not supported.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void            ConfigurationManager::SetGetAdminFunc (T_GetAdminFunc f)
    {
    s_getAdminFunc = f;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   ConfigurationManager::GetVariable (WStringR cfgValue, WCharCP cfgVarName, ConfigurationVariableLevel level)
    {
    IConfigurationAdmin& ca = GetAdmin();
    return ca._GetConfigVariable (cfgValue, cfgVarName, level);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool          ConfigurationManager::IsVariableDefined (WCharCP cfgVarName) 
    {
    IConfigurationAdmin& ca = GetAdmin();
    return ca._IsConfigVariableDefined (cfgVarName, ConfigurationVariableLevel::User);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool          ConfigurationManager::IsVariableDefinedAndTrue (WCharCP cfgVarName)
    {
    IConfigurationAdmin& ca = GetAdmin();
    return ca._IsConfigVariableDefinedAndTrue (cfgVarName, ConfigurationVariableLevel::User);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool          ConfigurationManager::IsVariableDefinedAndFalse (WCharCP cfgVarName)
    {
    // note: this is less often used, and thus is not implemented to be optimized like IsVariableDefinedAndTrue.
    WString     value;

    // if not defined, return false.
    if (BSISUCCESS != GetVariable (value, cfgVarName))
        return false;

    if (value.empty())
        return false;
    
    if (0 == wcscmp (L"0", value.c_str()))
        return true;
    
    return (0 == BeStringUtilities::Wcsicmp (L"false", value.c_str()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ConfigurationManager::DefineVariable (WCharCP cfgVarName, WCharCP cfgValue, ConfigurationVariableLevel level)
    {
    return GetAdmin()._DefineConfigVariable(cfgVarName, cfgValue, level);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ConfigurationManager::UndefineVariable (WCharCP cfgVarName) 
    {
    return GetAdmin()._UndefineConfigVariable(cfgVarName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ConfigurationManager::CheckVariableIsDefined (int& result, WCharCP v)
    {
    // MT: It's up to the caller to call this in a way that is thread-safe, e.g., by defining result to be a thread-local variable.
    if (result == -1)
        result = IsVariableDefined (v)? 1: -1;
    return result == 1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DanEast         11/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool    ConfigurationManager::StringContainsMacros (WCharCP string)
    {
    return (NULL != ::wcsstr (string, L"$["));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DanEast         11/07
+---------------+---------------+---------------+---------------+---------------+------*/
void    ConfigurationManager::StringExpandMacros (WStringR expanded)
    {
    // Within a string or message, replace macros in the form of $[macroname] with the macro's value.
    // Macros in the form of $(macroname) are left alone in the string.
    // This does an in-place replacement in 'expanded'.
    size_t      macroStart;
    size_t      startPos = 0;
    while (WString::npos != (macroStart = expanded.find(L"$[", startPos)))
        {
        size_t  macroEnd = expanded.find (']');
        if (WString::npos != macroEnd)
            {
            WString     macroValue;
            size_t      macroNameLength = (macroEnd - macroStart) - 2;
            WString     macroName = expanded.substr (macroStart+2, macroNameLength);
            ConfigurationManager::GetVariable (macroValue, macroName.c_str());
            expanded.replace (macroStart, macroEnd - macroStart + 1, macroValue);
            startPos = macroStart + macroValue.length();
            }
        else
            {
            startPos = macroStart + 2;
            }
        }
    }

#if 0 // WIP_BEFILENAME_PORTABILITY
/*---------------------------------------------------------------------------------**//**
* Generates a temporary filename
* @bsimethod                                                    BernMcCarty     06/05
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ConfigurationManager::GetNameForTempFile (BeFileNameR tempFileName, WCharCP partialPathName, WCharCP prefixString)
    {
    WChar   fullPath[MAX_PATH];
    WCharCP pathToUse;

    tempFileName.Clear ();
    if (NULL != partialPathName)
        {
        pathToUse = &fullPath[0];
        wcscpy(fullPath, GetLocalTempDirectoryBaseName().GetName ());
        wcscat(fullPath, partialPathName);
        if (WCSDIR_SEPARATOR_CHAR != fullPath[wcslen(fullPath) - 1])
            wcscat(fullPath, WCSDIR_SEPARATOR);
        }
    else
        {
        pathToUse = GetLocalTempDirectoryBaseName().GetName ();
        }

    /* ensure that the directory exists */
    BeFileNameStatus status = BeFileName::CreateNewDirectory (pathToUse);

    if (status != BeFileNameStatus::Success && status != BeFileNameStatus::AlreadyExists)
        return ERROR;

    if (BeFileName::BeGetTempFileName (tempFileName, BeFileName(pathToUse), prefixString) != BeFileNameStatus::Success)
        return ERROR;

    return SUCCESS;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DanEast         11/07
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ConfigurationManager::IterateThroughVariables (IConfigVariableIteratorDelegate *delegate)
    {
    return GetAdmin()._IterateThroughVariables (delegate);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP ConfigurationManager::GetVariableOnce (DgnHost::Key& key, WCharCP cfgVarName)
    {
    DgnHost::HostValue<WString>* value = (DgnHost::HostValue<WString>*) T_HOST.GetHostObject (key);
    if (NULL == value)
        {
        WString cfgVarValue;
        if (SUCCESS != ConfigurationManager::GetVariable (cfgVarValue, cfgVarName))
            return NULL;
        value = new DgnHost::HostValue<WString> (cfgVarValue);
        T_HOST.SetHostObject (key, value);
        }
    return value->GetValue().c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool ConfigurationManager::CheckVariableIsDefinedOnce (DgnHost::Key& key, WCharCP cfgVarName)
    {
    DgnHost::HostValue<bool>* value = (DgnHost::HostValue<bool>*) T_HOST.GetHostObject (key);
    if (NULL == value)
        {
        value = new DgnHost::HostValue<bool> (ConfigurationManager::IsVariableDefined (cfgVarName));
        T_HOST.SetHostObject (key, value);
        }
    return value->GetValue();
    }
