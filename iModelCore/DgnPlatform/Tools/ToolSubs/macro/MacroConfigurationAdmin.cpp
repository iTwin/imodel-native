/*--------------------------------------------------------------------------------------+
|
|     $Source: Tools/ToolSubs/macro/MacroConfigurationAdmin.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/Bentley.h>
#include <DgnPlatform/ExportMacros.h>
#include <DgnPlatform/DesktopTools/MacroConfigurationAdmin.h>
#include <DgnPlatform/DesktopTools/envvutil.h>
#include <DgnPlatform/DesktopTools/MacroFileProcessor.h>
#include "macro.h"

#define CFGVAR_UNLOCK_PASSWORD      L"WorkspaceManagerAppUseOnly"
#define CFGVAR_DEFINED_NULL         ((WStringP)-1) /* defined but no translation */
#define INVALID_MACRO_LEVEL         (ConfigurationVariableLevel)-99

USING_NAMESPACE_BENTLEY_DGN

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    09/94
+---------------+---------------+---------------+---------------+---------------+------*/
static WCharCP     getLevelDebugString (ConfigurationVariableLevel level)
    {
    static WCharCP s_levelNames[] =
        {
        L"system",
        L"appl",
        L"site",
        L"project",
        L"user"
        };

    if (level == ConfigurationVariableLevel::Predefined)
        return (L"predefined");

    if (level == ConfigurationVariableLevel::SysEnv)
        return (L"system env");

    return (s_levelNames[level]);
    }

BEGIN_BENTLEY_DGN_NAMESPACE

struct MacroEntry
{
WStringP    m_sysEnv;
WStringP    m_preDefined;    /* hidden level for built-in macros */
WStringP    m_system;
WStringP    m_appl;
WStringP    m_site;
WStringP    m_dgndb;
WStringP    m_user;
bool        m_locked;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/12
+---------------+---------------+---------------+---------------+---------------+------*/
MacroEntry ()
    {
    m_sysEnv        = NULL;
    m_preDefined    = NULL;
    m_system        = NULL;
    m_appl          = NULL;
    m_site          = NULL;
    m_dgndb       = NULL;
    m_user          = NULL;
    m_locked        = NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/12
+---------------+---------------+---------------+---------------+---------------+------*/
~MacroEntry ()
    {
    if (CFGVAR_DEFINED_NULL != m_sysEnv)
        delete m_sysEnv;
    if (CFGVAR_DEFINED_NULL != m_preDefined)
        delete m_preDefined;
    if (CFGVAR_DEFINED_NULL != m_system)
        delete m_system;
    if (CFGVAR_DEFINED_NULL != m_appl)
        delete m_appl;
    if (CFGVAR_DEFINED_NULL != m_site)
        delete m_site;
    if (CFGVAR_DEFINED_NULL != m_dgndb)
        delete m_dgndb;
    if (CFGVAR_DEFINED_NULL != m_user)
        delete m_user;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/12
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP     GetTranslationAtSysEnvLevel (WCharCP macroName)
    {
    /* Always re-get current operating system value unless locked */
    if (m_sysEnv != NULL)
        {
        if (m_locked)
            return m_sysEnv->c_str();

        delete m_sysEnv;
        m_sysEnv = NULL;
        }

    m_sysEnv = new WString();
    if (BSISUCCESS == util_getSysEnv (m_sysEnv, macroName))
        return (m_sysEnv->c_str());
    else
        {
        delete m_sysEnv;
        m_sysEnv = NULL;
        }

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/12
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP     GetTranslationFromLevel (ConfigurationVariableLevel fromLevel, WCharCP macroName)
    {
    /* -----------------------------------------------------------
        Jump in at the value of "fromLevel" to begin searching
            from that level.  If not found at the starting level,
            keep dropping thru cases until found.
       ----------------------------------------------------------- */

    WStringP    translation = NULL;
    switch (fromLevel)
        {
        case ConfigurationVariableLevel::User:
            if (m_user != NULL)
                {
                translation = m_user;
                break;
                }

        case ConfigurationVariableLevel::Project:
            if (m_dgndb != NULL)
                {
                translation = m_dgndb;
                break;
                }

        case ConfigurationVariableLevel::Site:
            if (m_site != NULL)
                {
                translation = m_site;
                break;
                }

        case ConfigurationVariableLevel::Appl:
            if (m_appl != NULL)
                {
                translation = m_appl;
                break;
                }

        case ConfigurationVariableLevel::System:
            if (m_system != NULL)
                {
                translation = m_system;
                break;
                }

        case ConfigurationVariableLevel::Predefined:
            if (m_preDefined != NULL)
                {
                translation = m_preDefined;
                break;
                }

        case ConfigurationVariableLevel::SysEnv:
            {
            WCharCP     transChars;
            if (NULL != (transChars = GetTranslationAtSysEnvLevel (macroName)))
                return transChars;
            break;
            }
        }

    if (NULL == translation)
        return NULL;

    return (CFGVAR_DEFINED_NULL != translation) ? translation->c_str() : NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   GetTranslationAtSpecificLevel (WStringP envStr, ConfigurationVariableLevel level, WCharCP macroName)
    {
    if (NULL != envStr)
        envStr->clear();

    // get pointer to translation at desired level.
    WStringP    translation = NULL;
    switch (level)
        {
        case ConfigurationVariableLevel::User:
            translation = m_user;
            break;

        case ConfigurationVariableLevel::Project:
            translation = m_dgndb;
            break;

        case ConfigurationVariableLevel::Site:
            translation = m_site;
            break;

        case ConfigurationVariableLevel::Appl:
            translation = m_appl;
            break;

        case ConfigurationVariableLevel::System:
            {
            WCharCP transChar;
            if (NULL != (transChar = GetTranslationAtSysEnvLevel (macroName)))
                {
                if (NULL != envStr)
                    envStr->assign (transChar);
                return BSISUCCESS;
                }
            break;
            }

        case ConfigurationVariableLevel::Predefined:
            translation = m_preDefined;
            break;

        case ConfigurationVariableLevel::SysEnv:
            translation = m_sysEnv;
            break;

        }

    // return translation if found
    if (NULL != translation)
        {
        if ( (NULL != envStr) && (CFGVAR_DEFINED_NULL != translation) )
            envStr->assign (translation->c_str());

        return BSISUCCESS;
        }

    return BSIERROR;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    04/93
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   RemoveAtLevel (ConfigurationVariableLevel level, bool removeLocked)
    {
    if (m_locked && !removeLocked)
        return BSIERROR;

    WStringP* translation = NULL;
    switch (level)
        {
        case ConfigurationVariableLevel::System:
            translation = &m_system;
            break;

        case ConfigurationVariableLevel::Appl:
            translation = &m_appl;
            break;

        case ConfigurationVariableLevel::Site:
            translation = &m_site;
            break;

        case ConfigurationVariableLevel::Project:
            translation = &m_dgndb;
            break;

        case ConfigurationVariableLevel::User:
            translation = &m_user;
            break;

        case ConfigurationVariableLevel::SysEnv:
            translation = &m_sysEnv;
            break;

        default:
            break;
        }

    if (NULL != *translation)
        {
        if (CFGVAR_DEFINED_NULL != *translation)
            delete *translation;

        *translation = NULL;
        m_locked = false;
        }

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/12
+---------------+---------------+---------------+---------------+---------------+------*/
ConfigurationVariableLevel  GetLevelOfDefinition ()
    {
    if (m_user != NULL)
        return ConfigurationVariableLevel::User;

    if (m_dgndb != NULL)
        return ConfigurationVariableLevel::Project;

    if (m_site != NULL)
        return ConfigurationVariableLevel::Site;

    if (m_appl != NULL)
        return ConfigurationVariableLevel::Appl;

    if (m_system != NULL)
        return ConfigurationVariableLevel::System;

    if (m_preDefined != NULL)
        return ConfigurationVariableLevel::Predefined;

    if (m_sysEnv != NULL)
        return ConfigurationVariableLevel::SysEnv;

    return INVALID_MACRO_LEVEL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    04/93
+---------------+---------------+---------------+---------------+---------------+------*/
void    SetTranslation (WCharCP translation, ConfigurationVariableLevel level)
    {
    /* If variable is locked, no other values may be set */
    if (m_locked)
        return;

    /* Even if translation NULL set to a non-NULL value */
    WStringP setTrans;
    if (NULL == translation)
        setTrans = CFGVAR_DEFINED_NULL;
    else
        setTrans = new WString (translation);

    switch (level)
        {
        case ConfigurationVariableLevel::System:
            delete m_system;
            m_system = setTrans;
            break;

        case ConfigurationVariableLevel::Appl:
            delete m_appl;
            m_appl = setTrans;
            break;

        case ConfigurationVariableLevel::Site:
            delete m_site;
            m_site = setTrans;
            break;

        case ConfigurationVariableLevel::Project:
            delete m_dgndb;
            m_dgndb = setTrans;
            break;

        case ConfigurationVariableLevel::User:
            delete m_user;
            m_user = setTrans;
            break;

        case ConfigurationVariableLevel::Predefined:
            delete m_preDefined;
            m_preDefined = setTrans;

            /* automatically lock all pre-defined macros */
            m_locked = true;
            break;

        case ConfigurationVariableLevel::SysEnv:
            delete m_sysEnv;
            m_sysEnv = setTrans;
            break;

        default:
            return;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool    HasTranslationAtLevel (ConfigurationVariableLevel level)
    {
    /* -----------------------------------------------------------
        Jump in at the value of "fromLevel" to begin searching
        from that level.  If not found at the starting level,
        keep dropping thru cases until found.
    ----------------------------------------------------------- */
    switch (level)
        {
        case ConfigurationVariableLevel::User:
            if (m_user != NULL)
                return true;

        case ConfigurationVariableLevel::Project:
            if (m_dgndb != NULL)
                return true;

        case ConfigurationVariableLevel::Site:
            if (m_site != NULL)
                return true;

        case ConfigurationVariableLevel::Appl:
            if (m_appl != NULL)
                return true;

        case ConfigurationVariableLevel::System:
            if (m_system != NULL)
                return true;

        case ConfigurationVariableLevel::Predefined:
            if (m_preDefined != NULL)
                return true;

        case ConfigurationVariableLevel::SysEnv:
            if (m_sysEnv != NULL)
                return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool    IsLocked()
    {
    return m_locked;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/12
+---------------+---------------+---------------+---------------+---------------+------*/
void    SetLocked (bool locked)
    {
    m_locked = locked;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    04/93
+---------------+---------------+---------------+---------------+---------------+------*/
bool     HasAnyTranslation ()
    {
    return ( (NULL != m_user) || (NULL != m_dgndb) || (NULL != m_site) || (NULL != m_appl) || (NULL != m_system) || (NULL != m_preDefined) ||  (NULL != m_sysEnv) );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    04/93
+---------------+---------------+---------------+---------------+---------------+------*/
void            PrintCfgVarSummaryLine (BeTextFileR debugFile, WCharCP macroName, MacroConfigurationAdmin& macroCfgAdmin)
    {
    debugFile.PrintfTo (true, L"%-17ls: %-12ls = ", macroName, getLevelDebugString (GetLevelOfDefinition()));

    WCharCP translation;
    if (NULL != (translation= GetTranslationFromLevel (ConfigurationVariableLevel::User, macroName)))
        {
        WString expansion;
        macroCfgAdmin.ExpandMacro (expansion, translation, MacroExpandOptions (ConfigurationVariableLevel::User));
        debugFile.PrintfTo (true, L"%ls %ls\n", expansion.empty() ? L"(null)" : expansion.c_str(), (m_locked) ? L"<Locked>" : L" ");
        }
    else
        {
        debugFile.PrintfTo (true, L"(null) %ls\n", (m_locked) ? L"<Locked>" : L" ");
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    04/93
+---------------+---------------+---------------+---------------+---------------+------*/
void            PrintLevelCfgVarSummaryLine (BeTextFileR debugFile, WCharCP macroName, MacroConfigurationAdmin& macroCfgAdmin)
    {
    debugFile.PrintfTo (true, L"%-17ls: ", macroName);
    if (!HasAnyTranslation())
        {
        debugFile.PrintfTo (true, L"(null) %ls\n", (m_locked) ? L"<Locked>" : L" ");
        return;
        }

    int counter = 0;
    for (ConfigurationVariableLevel iLevel=ConfigurationVariableLevel::Predefined; iLevel<=ConfigurationVariableLevel::User; iLevel = (ConfigurationVariableLevel)(iLevel+1))
        {
        if (HasTranslationAtLevel (iLevel))
            {
            WString     translation;
            GetTranslationAtSpecificLevel (&translation, iLevel, macroName);

            WString     expansion;
            macroCfgAdmin.ExpandMacro (expansion, translation.c_str(), MacroExpandOptions(iLevel));

            if (counter > 0)
                debugFile.PrintfTo (true, L"%-17ls: ", L" ");

            debugFile.PrintfTo (true, L"%-12ls = ", getLevelDebugString (iLevel));
            debugFile.PrintfTo (true, L"%ls", expansion.empty () ? L"(null)" : expansion.c_str() );

            if (0 == counter)
                debugFile.PrintfTo (true, L" %ls\n", (m_locked) ? L"<Locked>" : L" ");
            else
                debugFile.PrintfTo (true, L"\n");
            }
        counter++;
        }
    }

};

END_BENTLEY_DGN_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    04/93
+---------------+---------------+---------------+---------------+---------------+------*/
bool        MacroConfigurationAdmin::IsValidCfgVarLevelForNewDefinition (ConfigurationVariableLevel level)
    {
    return ((level <= ConfigurationVariableLevel::User) && (level >=ConfigurationVariableLevel::SysEnv));
    }

typedef     MacroEntry*     MacroEntryP;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    05/95
+---------------+---------------+---------------+---------------+---------------+------*/
bool        MacroConfigurationAdmin::IsMacroNameValid (WCharCP macroName)
    {
    /* check for valid macro name */
    if ( (NULL== macroName) || (0 == *macroName) )
        return false;

    // must have at least a two character macro name so it won't be confused with a drive letter.
    if ((0 == *(macroName+1)) || (wcslen (macroName) >= MAXNAMELENGTH) )
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MacroEntryP     MacroConfigurationAdmin::GetMacroDefinition (WStringR upcaseName, WCharCP macroName)
    {
    upcaseName.assign (macroName);
    upcaseName.ToUpper();

    T_MacroMap::const_iterator  definition;
    if (m_macroMap.end() == (definition = m_macroMap.find (upcaseName.c_str())))
        return NULL;
    return definition->second;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP     MacroConfigurationAdmin::GetMacroTranslation (WCharCP macroName, WStringR tmpStorage, ConfigurationVariableLevel level)
    {
    if (!IsMacroNameValid (macroName))
        return NULL;

    WString     upcaseName;
    MacroEntryP macroEntry;
    if (NULL != (macroEntry = GetMacroDefinition (upcaseName, macroName)))
        {
        WCharCP translation = macroEntry->GetTranslationFromLevel (level, macroName);
        return translation;
        }

    if (util_getSysEnv (&tmpStorage, macroName) == BSISUCCESS)
        return  tmpStorage.c_str();

    return  NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   10/92
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   MacroConfigurationAdmin::GetMacro (WStringP envStr, WCharCP macroName)
    {
    if (NULL != envStr)
        envStr->empty();

    WString     tmpStorage;
    WCharCP     translation;
    if (NULL == (translation = GetMacroTranslation (macroName, tmpStorage, ConfigurationVariableLevel::User)))
        return BSIERROR;

    if (NULL != envStr)
        envStr->assign (translation);

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    04/93
+---------------+---------------+---------------+---------------+---------------+------*/
bool MacroConfigurationAdmin::IsMacroDefinedFromLevel (WCharCP macroName, ConfigurationVariableLevel fromLevel)
    {
    // check for valid macro name */
    if ( (NULL == macroName) || (0 == *macroName) )
        return false;

    // if defined in macro map, return true.
    WString     upcaseName;
    MacroEntryP macroEntry = GetMacroDefinition (upcaseName, macroName);
    if ( (NULL != macroEntry) && macroEntry->HasTranslationAtLevel (fromLevel) )
        return true;

    // otherwise check system environment.
    return BSISUCCESS == util_getSysEnv (NULL, macroName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    04/93
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus MacroConfigurationAdmin::GetMacroFromLevel (WStringP envStr, WCharCP macroName, ConfigurationVariableLevel level)
    {
    if (!IsValidCfgVarLevelForNewDefinition (level))
        return BSIERROR;

    if (NULL != envStr)
        envStr->clear();

    WString     tmpStorage;
    WCharCP     translation;
    if (NULL != (translation = GetMacroTranslation (macroName, tmpStorage, level)))
        {
        if (NULL != envStr)
            envStr->assign (translation);

        return BSISUCCESS;
        }

    return BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    04/93
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   MacroConfigurationAdmin::GetMacroAtLevel (WStringP envStr, WCharCP macroName, ConfigurationVariableLevel level)
    {
    if (NULL != envStr)
        envStr->clear();

    // check for valid macro name
    if ( (NULL == macroName) || (0 == *macroName) )
        return BSIERROR;

    if ( !IsValidCfgVarLevelForNewDefinition (level) && (ConfigurationVariableLevel::Predefined != level) )
        return BSIERROR;

    // if not defined in macro map, error.
    WString     upcaseName;
    MacroEntryP macroEntry;
    if (NULL == (macroEntry = GetMacroDefinition (upcaseName, macroName)))
        return BSIERROR;

    return macroEntry->GetTranslationAtSpecificLevel (envStr, level, macroName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    03/93
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   MacroConfigurationAdmin::GetMacroLevel (ConfigurationVariableLevel& level, WCharCP macroName)
    {
    // check for valid macro name
    if ( (NULL == macroName) || (0 == *macroName) )
        return BSIERROR;

    // see if there is an entry for it.
    WString     upcaseName;
    MacroEntryP macroEntry;
    if (NULL != (macroEntry = GetMacroDefinition (upcaseName, macroName)))
        {
        level = macroEntry->GetLevelOfDefinition();
        return BSISUCCESS;
        }

    // check the system environment for the variable
    WString tmpStr;
    if (BSISUCCESS == util_getSysEnv (&tmpStr, macroName))
        {
        level = ConfigurationVariableLevel::SysEnv;
        return  BSISUCCESS;
        }

    // getting here means variable not found, return error
    return  BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   10/92
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus MacroConfigurationAdmin::DefineMacro (MacroEntryP& newEntry, WCharCP macroName, WCharCP translation, ConfigurationVariableLevel level)
    {
    if ( (NULL == macroName) || (0 == *macroName) )
        return BSIERROR;

    WString     upcaseName;
    if (NULL == (newEntry = GetMacroDefinition (upcaseName, macroName)))
        {
        newEntry = new MacroEntry ();
        m_macroMap[upcaseName.c_str()] = newEntry;
        }
    newEntry->SetTranslation (translation, level);

    return  BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   MacroConfigurationAdmin::DefineMacroWithDebugging (WCharCP macroName, WCharCP translation, ConfigurationVariableLevel level, BeTextFileP debugFile, int debugLevel)
    {
    MacroEntryP     macroEntry;
    BentleyStatus   status;
    if (BSISUCCESS != (status = DefineMacro (macroEntry, macroName, translation, level)))
        return status;

    if (NULL != debugFile)
        {
        WCharCP  outTrans = macroEntry->GetTranslationFromLevel (level, macroName);
        debugFile->PrintfTo (true, L"(%ls): %ls=%ls", getLevelDebugString (level), macroName, (outTrans != NULL) ? outTrans : L"(null)");

        if ((debugLevel > 1) && (NULL != outTrans))
            {
            WString expansion;
            ExpandMacro (expansion, outTrans, ExpandOptions(level));
            debugFile->PrintfTo (true, L" [%ls]", expansion.empty() ? L"(null)" : expansion.c_str() );
            }
        debugFile->PrintfTo (true, L"\n");
        }

    return BSISUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2010
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   MacroConfigurationAdmin::DefineBuiltinMacro (WCharCP macroName, WCharCP translation)
    {
    MacroEntryP macroEntry;
    return DefineMacro (macroEntry, macroName, translation, ConfigurationVariableLevel::Predefined);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    04/93
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   MacroConfigurationAdmin::RemoveMacroAtLevel (WCharCP macroName, ConfigurationVariableLevel level)
    {
    if ( (NULL == macroName) || (0 == *macroName) )
        return BSIERROR;

    if (!IsValidCfgVarLevelForNewDefinition (level))
        return BSIERROR;

    WString     upcaseName;
    MacroEntryP macroEntry;
    if (NULL == (macroEntry = GetMacroDefinition (upcaseName, macroName)))
        return BSIERROR;

    macroEntry->RemoveAtLevel (level, false);
    if (!macroEntry->HasAnyTranslation ())
        return ConfigurationManager::UndefineVariable (macroName);

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    04/93
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   MacroConfigurationAdmin::RemoveAllMacrosAtLevel (ConfigurationVariableLevel level)
    {
    if (!IsValidCfgVarLevelForNewDefinition (level))
        return BSIERROR;
 
    for (T_MacroMap::iterator thisIter = m_macroMap.begin(); thisIter != m_macroMap.end(); )
        {
        MacroEntryP             macroEntry = thisIter->second;

        macroEntry->RemoveAtLevel (level, true);

        // if we're going to remove, get the next element before we do that.
        if (!macroEntry->HasAnyTranslation())
            {
            T_MacroMap::iterator    nextIter = thisIter;
            nextIter++;
            m_macroMap.erase (thisIter);
            thisIter = nextIter;
            }
        else
            ++thisIter;
        }

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
void            MacroConfigurationAdmin::EnsureSysEnvDefinition (WCharCP macroName, ConfigurationVariableLevel level, BeTextFileP debugFile, int debugLevel)
    {
    // this is called from MacroFileProcessor::NewMacro to ensure that there is a definition in the macroMap
    //  when the macro is defined only in the system environment.
    WString     upcaseName;
    if (NULL != GetMacroDefinition (upcaseName, macroName))
        return;     // already defined at some level.

    MacroExpandOptions options (level);
    options.SetImmediate (false);
    WString expansion;
    ExpandMacro (expansion, macroName, options);
    DefineMacroWithDebugging (macroName, expansion.c_str(), ConfigurationVariableLevel::SysEnv, debugFile, debugLevel);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    05/93
+---------------+---------------+---------------+---------------+---------------+------*/
void    MacroConfigurationAdmin::RemoveAllMacros ()
    {
    m_macroMap.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    05/93
+---------------+---------------+---------------+---------------+---------------+------*/
bool      MacroConfigurationAdmin::IsMacroLocked (WCharCP macroName)
    {
    WString     upcaseName;
    MacroEntryP macroEntry;
    if (NULL == (macroEntry = GetMacroDefinition (upcaseName, macroName)))
        return false;

    return macroEntry->IsLocked();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    05/93
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   MacroConfigurationAdmin::LockMacro (WCharCP macroName)
    {
    // If already a macro, just set the lock flag
    WString     upcaseName;
    MacroEntryP macroEntry;
    if (NULL != (macroEntry = GetMacroDefinition (upcaseName, macroName)))
        {
        macroEntry->SetLocked (true);
        return BSISUCCESS;
        }

    WString     tmpStr;
    if (BSISUCCESS == util_getSysEnv (&tmpStr, macroName))
        {
        // Found in environment - add as a macro */
        macroEntry = new MacroEntry ();
        macroEntry->SetTranslation (tmpStr.c_str(), ConfigurationVariableLevel::SysEnv);
        macroEntry->SetLocked (true);
        m_macroMap[macroName] = macroEntry;
        return BSISUCCESS;
        }

    // Not found, can't lock.
    return BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    04/95
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   MacroConfigurationAdmin::UnlockMacro (WCharCP macroName, WCharCP passWord)
    {
    // check password before continuing.
    if (0 != wcscmp (passWord, CFGVAR_UNLOCK_PASSWORD))
        return BSIERROR;

    // find it and unset the lock flag.
    WString     upcaseName;
    MacroEntryP macroEntry;
    if (NULL == (macroEntry = GetMacroDefinition (upcaseName, macroName)))
        return BSIERROR;

    macroEntry->SetLocked (false);
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/12
+---------------+---------------+---------------+---------------+---------------+------*/
void        MacroConfigurationAdmin::PrintAllMacros (BeTextFileR outFile, int debugLevel)
    {
    // First print the configuration variables with a leading "_"
    for (T_MacroMap::iterator thisIter = m_macroMap.begin(); thisIter != m_macroMap.end(); thisIter++)
        {
        WStringCR               macroName  = thisIter->first;
        MacroEntryP             macroEntry = thisIter->second;

        if ('-' != macroName[0])
            continue;

        if (debugLevel > 4)
            macroEntry->PrintLevelCfgVarSummaryLine (outFile, macroName.c_str(), *this);
        else
            macroEntry->PrintCfgVarSummaryLine (outFile, macroName.c_str(), *this);
        }

    outFile.PrintfTo (true, L"\n");

    // Now print the rest.
    for (T_MacroMap::iterator thisIter = m_macroMap.begin(); thisIter != m_macroMap.end(); thisIter++)
        {
        WStringCR               macroName  = thisIter->first;
        MacroEntryP             macroEntry = thisIter->second;

        if ('-' == macroName[0])
            continue;

        if (debugLevel > 4)
            macroEntry->PrintLevelCfgVarSummaryLine (outFile, macroName.c_str(), *this);
        else
            macroEntry->PrintCfgVarSummaryLine (outFile, macroName.c_str(), *this);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    kab             07/90
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus MacroConfigurationAdmin::_UndefineConfigVariable (WCharCP macroName)
    {
    if ( (NULL == macroName) || (0 == *macroName) )
        return BSIERROR;

    WString upcaseName (macroName);
    upcaseName.ToUpper ();

    T_MacroMap::iterator    iterator;
    if (m_macroMap.end() != (iterator = m_macroMap.find (upcaseName.c_str())))
        {
        delete iterator->second;
        m_macroMap.erase (iterator);
        return BSISUCCESS;
        }

    return BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   10/92
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus MacroConfigurationAdmin::_GetConfigVariable (WStringR cfgValue, WCharCP cfgVarName, ConfigurationVariableLevel level)
    {
    cfgValue.clear();

    WCharCP     translation;
    WString     tmpStr;
    if (NULL == (translation = GetMacroTranslation (cfgVarName, tmpStr, level)))
        return BSIERROR;

    if (0 != *translation)
        return (BSISUCCESS != ExpandMacro (cfgValue, translation, ExpandOptions(level))) ? BSIERROR : BSISUCCESS;

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   10/92
+---------------+---------------+---------------+---------------+---------------+------*/
bool          MacroConfigurationAdmin::_IsConfigVariableDefined (WCharCP cfgVarName, ConfigurationVariableLevel level)
    {
    WString     tmpStr;
    return (NULL != GetMacroTranslation (cfgVarName, tmpStr, level));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   12/11
+---------------+---------------+---------------+---------------+---------------+------*/
bool          MacroConfigurationAdmin::_IsConfigVariableDefinedAndTrue (WCharCP cfgVarName, ConfigurationVariableLevel level)
    {
    WCharCP     translation;
    WString     tmpStr;
    if (NULL == (translation = GetMacroTranslation (cfgVarName, tmpStr, level)))
        return false;

    // try to avoid the expansion part of it.
    if (wcscmp (L"1", translation))
        return true;

    if (BeStringUtilities::Wcsicmp (L"true", translation))
        return true;

    WString expansion;
    if ( ContainsExpression (translation) && (BSISUCCESS == ExpandMacro (expansion, translation, ExpandOptions(level))) )
        {
        if (wcscmp (L"1", expansion.c_str()))
            return true;

        if (BeStringUtilities::Wcsicmp (L"on", expansion.c_str()))
            return true;

        return (0 == BeStringUtilities::Wcsicmp (L"true", expansion.c_str()));
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   10/92
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus MacroConfigurationAdmin::_DefineConfigVariable (WCharCP cfgVarName, WCharCP cfgValue, ConfigurationVariableLevel level)
    {
    if (NULL == cfgVarName || (0 == *cfgVarName))
        return BSIERROR;

    if (!IsValidCfgVarLevelForNewDefinition (level))
        return  BSIERROR;

    MacroEntryP newEntry;
    return DefineMacro (newEntry, cfgVarName, cfgValue, level);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
MacroConfigurationAdmin::ExpandOptions::ExpandOptions (ConfigurationVariableLevel lvl)
    :
    m_level(lvl),
    m_immediate (true),
    m_formatExpansion(true),
    m_expandOnlyIfFullyDefined (false)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dan.East        03/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus MacroConfigurationAdmin::_IterateThroughVariables (IConfigVariableIteratorDelegate *delegate)
    {
    if (NULL == delegate)
        return BSIERROR;
        
    BentleyStatus   status = BSISUCCESS;
        
    for (T_MacroMap::iterator thisIter = m_macroMap.begin(); thisIter != m_macroMap.end(); thisIter++)
        {
        WStringCR               macroName  = thisIter->first;
        MacroEntryP             macroEntry = thisIter->second;
        
        ConfigurationVariableLevel  level = macroEntry->GetLevelOfDefinition();
        if (INVALID_MACRO_LEVEL == level)
            {
            status = BSIERROR;      // Need more than this
            continue;
            }
            
        WCharCP     value = macroEntry->GetTranslationFromLevel (level, macroName.c_str());
        if (NULL == value)
            {
            status = BSIERROR;      // Need more than this
            continue;
            }
            
        bool    locked = macroEntry->IsLocked();

        delegate->EachConfigVariable (macroName.c_str(), value, level, locked);
        }

    return status;
    }

