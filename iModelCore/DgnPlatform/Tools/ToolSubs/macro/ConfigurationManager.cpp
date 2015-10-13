/*--------------------------------------------------------------------------------------+
|
|     $Source: Tools/ToolSubs/macro/ConfigurationManager.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnPlatform.h>
#include <DgnPlatform/DesktopTools/ConfigurationManager.h>
#include <DgnPlatform/DesktopTools/envvutil.h>
#include "macro.h"

#ifdef BENTLEYCONFIG_OS_APPLE_IOS
// Avoids ambiguous resolution of wcs functions.
#define wcsstr ::wcsstr
#endif

USING_NAMESPACE_BENTLEY_DGNPLATFORM

BentleyStatus IConfigurationAdmin::GetConfigVariable(WStringR cfgValue, WCharCP cfgVarName, ConfigurationVariableLevel level) { return _GetConfigVariable(cfgValue, cfgVarName, level); }

/*=================================================================================**//**
* An implementation of IConfigurationAdmin that accesses system environment variables and CSIDL.
* @bsiclass                                     Sam.Wilson                      05/2011
+===============+===============+===============+===============+===============+======*/
struct          EnvAdmin : IConfigurationAdmin
{
virtual void          _OnHostTermination (bool isProcessShutdown) override {;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
virtual BentleyStatus _GetConfigVariable (WStringR envStr, WCharCP envVar, ConfigurationVariableLevel level) override
    {
    BentleyStatus status = util_getSysEnv (&envStr, envVar);

    if (BSISUCCESS==status)
        ConfigurationManager::StringExpandMacros (envStr);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   12/11
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool          _IsConfigVariableDefined (WCharCP cfgVarName, ConfigurationVariableLevel level) override
    {
    return (SUCCESS == util_getSysEnv (nullptr, cfgVarName));
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

    if (0 == BeStringUtilities::Wcsicmp (L"true", value.c_str()))
        return true;

    return (0 == BeStringUtilities::Wcsicmp (L"on", value.c_str()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
virtual BentleyStatus _DefineConfigVariable (WCharCP cfgVarName, WCharCP cfgValue, ConfigurationVariableLevel level) override
    {
    BeAssert (level == ConfigurationVariableLevel::User);

    if (nullptr == cfgValue)
        cfgValue = L"?";       // *** NEEDS WORK: Need some way to mimic the ability of our macro system to define a variable without assigning it a value.

    return util_putenv (cfgVarName, cfgValue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
virtual BentleyStatus _UndefineConfigVariable (WCharCP cfgVarName) override
    {
    return util_putenv (cfgVarName, nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
virtual BeFileNameCR  _GetLocalTempDirectoryBaseName () override
    {
    static  bool        s_checked;
    static  BeFileName  s_tempPath; // MT: This default implementation not used by DgnPlatform

    if (!s_checked)
        {
        BeFileName::BeGetTempPath (s_tempPath);
        s_checked = true;
        }

    return s_tempPath;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DanEast         11/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual BentleyStatus _IterateThroughVariables (IConfigVariableIteratorDelegate *delegate) override
    {
    return BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/14
+---------------+---------------+---------------+---------------+---------------+------*/
virtual BentleyStatus _MonitorVariable (WCharCP cfgVarName, IVariableMonitorR monitor) override
    {
    WString cfgValue;
    _GetConfigVariable (cfgValue, cfgVarName, ConfigurationVariableLevel::User);

    monitor._VariableChanged (cfgVarName);

    // can't monitor.
    return BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _RemoveMonitor (WCharCP cfgVarName, IVariableMonitorR monitor) override
    {
    // not monitoring.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _OnUnloadMdlDescr (MdlDescP mdlDesc) override
    {
    // do nothing, we didn't save anything.
    }

}; // EnvAdmin

ConfigurationManager::T_GetAdminFunc ConfigurationManager::s_getAdminFunc;  // MT: ___DGNPLATFORMTOOLS_SERIALIZED___

static EnvAdmin s_envCfg;  // MT: We'll only use this when we have no host, in which case multi-threading is not supported.

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

    return s_envCfg;     // MT: We'll only use this when we have no host, in which case multi-threading is not supported.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void            ConfigurationManager::SetGetAdminFunc (T_GetAdminFunc f)
    {
    if (nullptr == s_getAdminFunc)
        s_getAdminFunc = f;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
void            ConfigurationManager::OnUnloadMdlDescr (MdlDescP mdlDesc)
    {
    IConfigurationAdmin& ca = GetAdmin();
    return ca._OnUnloadMdlDescr (mdlDesc);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   ConfigurationManager::GetVariable (WStringR cfgValue, WCharCP cfgVarName, ConfigurationVariableLevel level)
    {
    if ( (nullptr == cfgVarName) || (0 == *cfgVarName) )
        return BSIERROR;

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

    if (0 == BeStringUtilities::Wcsicmp (L"false", value.c_str()))
        return true;

    return (0 == BeStringUtilities::Wcsicmp (L"off", value.c_str()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileNameCR  ConfigurationManager::GetLocalTempDirectoryBaseName()
    {
    return GetAdmin()._GetLocalTempDirectoryBaseName ();
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
* @bsimethod                                                    DanEast         11/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool    ConfigurationManager::StringContainsMacros (WCharCP string)
    {
    return (nullptr != wcsstr (string, L"$["));
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
        size_t  macroEnd = expanded.find (']', macroStart);
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

#if defined (NEEDSWORK_DESKTOP_PLATFORM)
/*---------------------------------------------------------------------------------**//**
* Generates a temporary filename
* @bsimethod                                                    BernMcCarty     06/05
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ConfigurationManager::GetNameForTempFile (BeFileNameR tempFileName, WCharCP partialPathName, WCharCP prefixString)
    {
    WChar   fullPath[MAX_PATH];
    WCharCP pathToUse;

    tempFileName.Clear ();
    if (nullptr != partialPathName)
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
* @bsimethod                                    Keith.Bentley                   12/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   ConfigurationManager::GetLocalTempDirectory (BeFileNameR tempDir, WCharCP subDir)
    {
    tempDir = GetLocalTempDirectoryBaseName();

    if (nullptr != subDir)
        {
        tempDir.AppendToPath (subDir);
        tempDir.AppendSeparator ();
        }

    BeFileNameStatus status = BeFileName::CreateNewDirectory (tempDir);
    if (status != BeFileNameStatus::Success && status != BeFileNameStatus::AlreadyExists)
        return ERROR;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DanEast         11/07
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   ConfigurationManager::IterateThroughVariables (IConfigVariableIteratorDelegate *delegate)
    {
    return GetAdmin()._IterateThroughVariables (delegate);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   ConfigurationManager::MonitorVariable (WCharCP cfgVarName, IVariableMonitorR monitor)
    {
    if ( (nullptr == cfgVarName) || (0 == *cfgVarName) )
        return BSIERROR;

    return GetAdmin()._MonitorVariable (cfgVarName, monitor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   ConfigurationManager::RemoveMonitor (WCharCP cfgVarName, IVariableMonitorR monitor)
    {
    if ( (nullptr == cfgVarName) || (0 == *cfgVarName) )
        return BSIERROR;

    GetAdmin()._RemoveMonitor (cfgVarName, monitor);

    return BSISUCCESS;
    }

/*=================================================================================**//**
* @bsiclass                                     Barry.Bentley                   06/2014
+===============+===============+===============+===============+===============+======*/
class MonitoredBoolean : public IVariableMonitor
{
private:
    bool&           m_monitored;
    bool            m_defaultValue;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool    GetMonitoredValue (WCharCP variableName)
    {
    WString value;
    ConfigurationManager::GetVariable (value, variableName);

    // the default is false.
    if (value.empty())
        return m_defaultValue;

    if (0 == value.CompareTo (L"1"))
        return true;

    if (0 == value.CompareToI (L"true"))
        return true;

    if (0 == value.CompareToI (L"on"))
        return true;

    if (0 == value.CompareTo (L"0"))
        return false;

    if (0 == value.CompareToI (L"false"))
        return false;

    if (0 == value.CompareToI (L"off"))
        return false;

    return m_defaultValue;
    }

public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/14
+---------------+---------------+---------------+---------------+---------------+------*/
MonitoredBoolean (bool& monitored, WCharCP configVariable, bool defaultValue ) : m_monitored (monitored)
    {
    // Note: _VariableChanged is called right away, setting value.
    m_defaultValue = defaultValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
void    _VariableChanged (WCharCP variableName) override
    {
    m_monitored = GetMonitoredValue (variableName);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
void    _VariableRootChanged (WCharCP variableName, WCharCP rootVariableName) override
    {
    m_monitored = GetMonitoredValue (variableName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
void    _VariableUndefined (WCharCP variableName) override
    {
    m_monitored = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
void    _VariableRootUndefined (WCharCP variableName, WCharCP rootVariableName) override
    {
    m_monitored = GetMonitoredValue (variableName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
void    _MonitorStopped (WCharCP variableName) override
    {
    delete this;
    }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool        ConfigurationManager::MonitorBoolean (IVariableMonitorP& monitor, bool& value, WCharCP cfgVarName, bool defaultValue)
    {
    if (nullptr == monitor)
        GetAdmin()._MonitorVariable (cfgVarName, *(monitor = new MonitoredBoolean (value, cfgVarName, defaultValue)));        // MonitoredBoolean sets value in its constructor.

    return value;
    }



/*=================================================================================**//**
* @bsiclass                                     Barry.Bentley                   06/2014
+===============+===============+===============+===============+===============+======*/
class MonitoredInteger : public IVariableMonitor
{
private:
    int&            m_monitored;
    int             m_defaultValue;
    int             m_minimumValue;
    int             m_maximumValue;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
int     GetMonitoredValue (WCharCP variableName)
    {
    WString value;
    ConfigurationManager::GetVariable (value, variableName);

    // if there is no translation, return the default value.
    if (value.empty())
        return m_defaultValue;

    int intValue;
    if (1 == swscanf (value.c_str(), L"%d", &intValue))
        {
        if (intValue < m_minimumValue)
            intValue = m_minimumValue;
        else if (intValue > m_maximumValue)
            intValue = m_maximumValue;

        return intValue; 
        }

    // if swcanf doesn't work, return original value
    return m_defaultValue;
    }

public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/14
+---------------+---------------+---------------+---------------+---------------+------*/
MonitoredInteger (WCharCP configVariable, int& monitored, int defaultValue, int minimumValue, int maximumValue) : m_monitored (monitored)
    {
    m_monitored     = defaultValue;
    m_defaultValue  = defaultValue;
    m_minimumValue  = minimumValue;
    m_maximumValue  = maximumValue;
    BeAssert (defaultValue >= minimumValue);
    BeAssert (defaultValue <= maximumValue);
    BeAssert (minimumValue < maximumValue);

    // Note: _VariableChanged is called right away, setting value if the configuration variable is set.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
void    _VariableChanged (WCharCP variableName)
    {
    m_monitored = GetMonitoredValue (variableName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
void    _VariableRootChanged (WCharCP variableName, WCharCP rootVariableName) override
    {
    m_monitored = GetMonitoredValue (variableName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
void    _VariableUndefined (WCharCP variableName) override
    {
    m_monitored = m_defaultValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
void    _VariableRootUndefined (WCharCP variableName, WCharCP rootVariableName) override
    {
    m_monitored = GetMonitoredValue (variableName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
void    _MonitorStopped (WCharCP variableName) override
    {
    delete this;
    }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/14
+---------------+---------------+---------------+---------------+---------------+------*/
int     ConfigurationManager::MonitorInteger (IVariableMonitorP& monitor, int& value, WCharCP cfgVarName, int defaultValue, int minimumValue, int maximumValue)
    {
    if (nullptr == monitor)
        GetAdmin()._MonitorVariable (cfgVarName, *(monitor = new MonitoredInteger (cfgVarName, value, defaultValue, minimumValue, maximumValue)));        // MonitoredBoolean sets value in its constructor.

    return value;
    }



/*=================================================================================**//**
* @bsiclass                                     Barry.Bentley                   06/2014
+===============+===============+===============+===============+===============+======*/
class MonitoredString : public IVariableMonitor
{
private:
    WStringR        m_monitored;

public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/14
+---------------+---------------+---------------+---------------+---------------+------*/
MonitoredString (WCharCP configVariable, WStringR monitored) : m_monitored (monitored)
    {
    // Note: _VariableChanged is called right away, setting value if the configuration variable is set.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
void    _VariableChanged (WCharCP variableName) override
    {
    ConfigurationManager::GetVariable (m_monitored, variableName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
void    _VariableRootChanged (WCharCP variableName, WCharCP rootVariableName) override
    {
    ConfigurationManager::GetVariable (m_monitored, variableName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
void    _VariableUndefined (WCharCP variableName) override
    {
    m_monitored.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
void    _VariableRootUndefined (WCharCP variableName, WCharCP rootVariableName) override
    {
    ConfigurationManager::GetVariable (m_monitored, variableName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
void    _MonitorStopped (WCharCP variableName) override
    {
    delete this;
    }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/14
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP     ConfigurationManager::MonitorString (IVariableMonitorP& monitor, WStringR value, WCharCP cfgVarName)
    {
    if (nullptr == monitor)
        GetAdmin()._MonitorVariable (cfgVarName, *(monitor = new MonitoredString (cfgVarName, value)));        // MonitoredBoolean sets value in its constructor.

    return value.c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
SimpleConfigurationVariableMonitor::SimpleConfigurationVariableMonitor () {}
void SimpleConfigurationVariableMonitor::_VariableChanged (WCharCP variableName) { _UpdateState (variableName); }
void SimpleConfigurationVariableMonitor::_VariableRootChanged (WCharCP variableName, WCharCP rootVariableName) { _UpdateState (variableName); }
void SimpleConfigurationVariableMonitor::_VariableUndefined (WCharCP variableName) { _UpdateState (variableName); }
void SimpleConfigurationVariableMonitor::_VariableRootUndefined (WCharCP variableName, WCharCP rootVariableName) { _UpdateState (variableName); }
void SimpleConfigurationVariableMonitor::_MonitorStopped (WCharCP variableName) { delete this; }


/*---------------------------------------------------------------------------------**//**
* Methods for DgnPlatformConfigurationVariables
+---------------+---------------+---------------+---------------+---------------+------*/

// this is the host key that retrieves the DgnPlatformConfigVars per-thread.
static DgnHost::Key s_dgnPlatformConfigVarsKey;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
DgnPlatformConfigVars&  DgnPlatformConfigVars::Instance()
    {
    DgnPlatformConfigVars* dgnPlatformConfigVars = (DgnPlatformConfigVars*) T_HOST.GetHostObject (s_dgnPlatformConfigVarsKey);
    if (nullptr == dgnPlatformConfigVars)
        T_HOST.SetHostObject (s_dgnPlatformConfigVarsKey, dgnPlatformConfigVars = new DgnPlatformConfigVars);

    return *dgnPlatformConfigVars;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool    DgnPlatformConfigVars::AccumulateColorAdjustment()
    {
    return ConfigurationManager::MonitorBoolean (m_accumulateColorAdjustment.m_monitor, m_accumulateColorAdjustment.m_value, L"MS_NEST_COLORADJUSTMENT", false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  12/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool    DgnPlatformConfigVars::NeedsBlankSpaceFit ()
    {
    return ConfigurationManager::MonitorBoolean (m_needsBlankSpaceFit.m_monitor, m_needsBlankSpaceFit.m_value, L"MS_FIT_BLANKTEXT_INCLUDE", false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool    DgnPlatformConfigVars::RefMergeBreakAssociations()
    {
    return ConfigurationManager::MonitorBoolean (m_refMergeBreakAssociations.m_monitor, m_refMergeBreakAssociations.m_value, L"MS_REF_MERGE_BREAK_ASSOCIATIONS", false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool    DgnPlatformConfigVars::WireframeSheetLegacyOverride ()
    {
    return ConfigurationManager::MonitorBoolean (m_wireframeSheetLegacyOverride.m_monitor, m_wireframeSheetLegacyOverride.m_value, L"MS_RENDER_SHEET_REFS", false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool    DgnPlatformConfigVars::LoadCveAttachmentsDisabled ()
    {
    return ConfigurationManager::MonitorBoolean (m_loadCveAttachmentsDisabled.m_monitor, m_loadCveAttachmentsDisabled.m_value, L"MS_REF_NO_CVE_LOAD", false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool*   DgnPlatformConfigVars::GetLoadCveValueP ()
    {
    return &m_loadCveAttachmentsDisabled.m_value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BarryBentley    09/05
+---------------+---------------+---------------+---------------+---------------+------*/
bool    DgnPlatformConfigVars::AllowDwgMasterColors ()
    {
    return ConfigurationManager::MonitorBoolean (m_allowDwgMasterColors.m_monitor, m_allowDwgMasterColors.m_value, L"MS_DWGREF_ALLOWMASTERCOLORS", false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool    DgnPlatformConfigVars::UseClipFrozenLevel ()
    {
    return ConfigurationManager::MonitorBoolean (m_useClipFrozenLevel.m_monitor, m_useClipFrozenLevel.m_value, L"MS_REF_USECLIP_FROZENLEVEL", false);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                   Abeesh.Basheer                  12/2009
//---------------------------------------------------------------------------------------
bool    DgnPlatformConfigVars::AllowCapitalOWithStrokeAsDiameter ()
    {
    return ConfigurationManager::MonitorBoolean (m_treatOWithStokeAsDiameter.m_monitor, m_treatOWithStokeAsDiameter.m_value, L"MS_TREAT_OWITHSTROKE_AS_DIAMETER", false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool    DgnPlatformConfigVars::NoStrokeNonPlanarPolygon ()
    {
    return ConfigurationManager::MonitorBoolean (m_noStrokeNonPlanarPolygon.m_monitor, m_noStrokeNonPlanarPolygon.m_value, L"MS_NO_STROKE_NONPLANAR_POLYGONS", false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool    DgnPlatformConfigVars::StrokeNonPlanarPolygonTopView () 
    {
    return ConfigurationManager::MonitorBoolean (m_strokeNonPlanarPolygonTopView.m_monitor, m_strokeNonPlanarPolygonTopView.m_value, L"MS_STROKE_NONPLANAR_POLYGONS_TOP_VIEW", false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool    DgnPlatformConfigVars::MaterialMasterMat ()
    {
    return ConfigurationManager::MonitorBoolean (m_materialMasterMat.m_monitor, m_materialMasterMat.m_value, L"MS_MATERIAL_MASTERMAT", false);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool    DgnPlatformConfigVars::RefCycleCheck()
    {
    // any value except 0 is regarded as true. Default is true.
    return ConfigurationManager::MonitorBoolean (m_refCycleCheck.m_monitor, m_refCycleCheck.m_value, L"MS_REF_CYCLECHECK", true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool    DgnPlatformConfigVars::NewLevelDisplay ()
    {
    return ConfigurationManager::MonitorBoolean (m_newLevelDisplay.m_monitor, m_newLevelDisplay.m_value, L"MS_REF_NEWLEVELDISPLAY", false);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool    DgnPlatformConfigVars::DrawDimensionMasking ()
    {
    return ConfigurationManager::MonitorBoolean (m_drawDimensionMasking.m_monitor, m_drawDimensionMasking.m_value, L"MS_DRAWDIMENSIONMASKING", false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool    DgnPlatformConfigVars::DebugAssociativityData ()
    {
    return ConfigurationManager::MonitorBoolean (m_debugAssociativityData.m_monitor, m_debugAssociativityData.m_value, L"MS_DEBUG_ASSOCIATION_IDS", false);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP     DgnPlatformConfigVars::DetailingRequiredSettings ()
    {
    return ConfigurationManager::MonitorString (m_detailingRequiredSettings.m_monitor, m_detailingRequiredSettings.m_value, L"MS_DETAILINGSYMBOLSTYLE_REQUIREDSETTINGS");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP     DgnPlatformConfigVars::HistoryRevisionNumberFormat()
    {
    return ConfigurationManager::MonitorString (m_historyRevisionFormat.m_monitor, m_historyRevisionFormat.m_value, L"MS_DESIGN_HISTORY_REVISION_NUMBER_FORMAT");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool        DgnPlatformConfigVars::DisableQuickScan()
    {
    return ConfigurationManager::MonitorBoolean (m_disableQuickScan.m_monitor, m_disableQuickScan.m_value, L"MS_DISABLE_OCCLUSION_SORT", false);
    }
