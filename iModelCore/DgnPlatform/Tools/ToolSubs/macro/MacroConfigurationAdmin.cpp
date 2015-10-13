/*--------------------------------------------------------------------------------------+
|
|     $Source: Tools/ToolSubs/macro/MacroConfigurationAdmin.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DesktopTools/MacroConfigurationAdmin.h>
#include <DgnPlatform/DesktopTools/envvutil.h>
#include <DgnPlatform/DesktopTools/fileutil.h>
#include <DgnPlatform/DesktopTools/MacroFileProcessor.h>

#if defined (NEEDSWORK_DESKTOP_PLATFORM_READV8ICONFIG)
// the Cryptographer stuff is in the Resource Manager, but so far DgnPlatform does not depend on that.
#include <RmgrTools/Tools/CryptUtils.h>
#endif

#include "macro.h"
#if defined (BENTLEY_WIN32)
#include <shlwapi.h>
#include <shlobj.h>
#undef EXTERN_C
#include <KnownFolders.h>
#endif


BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

#define CFGVAR_UNLOCK_PASSWORD      L"WorkspaceManagerAppUseOnly"
#define CFGVAR_DEFINED_NULL         ((WStringP)-1) /* defined but no translation */
#define INVALID_MACRO_LEVEL         (ConfigurationVariableLevel)-99

static const WCharCP EMPTY_STRING=L"";

#if defined (NEEDSWORK_DESKTOP_PLATFORM)
/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct SaveAndSwitchMdlDesc
{
private:
    MdlDescP m_oldDesc;

public:
    SaveAndSwitchMdlDesc (MdlDescP newDesc)  // switch to new mdl descr
        {
        DgnPlatformLib::SessionAdmin&   admin = DgnPlatformLib::GetHost().GetSessionAdmin();
        if (NULL == newDesc)
            m_oldDesc = admin._SetCurrentMdlDescr (admin._GetSystemMdlDescr());
        else
            m_oldDesc = admin._SetCurrentMdlDescr (newDesc);
        }

    ~SaveAndSwitchMdlDesc ()
        {
        DgnPlatformLib::GetHost().GetSessionAdmin()._SetCurrentMdlDescr (m_oldDesc);
        }
};
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static void     breakIntoLines (IMacroDebugOutput& debugOutput, WStringR expansion, size_t lineLength, size_t indent)
    {
    size_t          length = expansion.length();
    WString         thisLine;
    for (size_t start = 0; start < length; )
        {
        size_t nextSemicolon = expansion.find (';', start);
        size_t lastChar = (WString::npos == nextSemicolon) ? length : nextSemicolon + 1;

        // line too long to fit?
        if ( ((nextSemicolon - start) + thisLine.length()) > lineLength)
            {
            if (!thisLine.empty())
                {
                // won't fit in thisLine.
                debugOutput.ShowDebugMessage (0, L"%ls\n", thisLine.c_str());
                thisLine.clear();
                size_t  insertPos = 0;
                thisLine.insert (insertPos, indent, ' ');
                }
            }

        // put the current string thisLine.
        thisLine.append (expansion, start, (lastChar - start));

        // Are we done with the string
        if (nextSemicolon == WString::npos)
            {
            if (!thisLine.empty())
                debugOutput.ShowDebugMessage (0, L"%ls\n",thisLine.c_str());
            break;
            }

        // look for next semicolon.
        start = lastChar;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    09/94
+---------------+---------------+---------------+---------------+---------------+------*/
static WCharCP     getLevelDebugString (ConfigurationVariableLevel level)
    {
    static WCharCP s_levelNames[] =
        {
        L"System",
        L"Application",
        L"Organization",
        L"WorkSpace",
        L"WorkSet",
        L"Role",
        L"User"
        };

    if (level == INVALID_MACRO_LEVEL)
        return (L"unknown");

    if (level == ConfigurationVariableLevel::Predefined)
        return (L"predefined");

    if (level == ConfigurationVariableLevel::SysEnv)
        return (L"system env");

    return (s_levelNames[static_cast<int>(level)]);
    }


typedef     struct MacroEntry const*   MacroEntryCP;
typedef     struct MacroEntry const&   MacroEntryCR;

struct VariableMonitor
    {
    IVariableMonitorP   m_monitor;
    MdlDescP            m_mdlDesc;
    VariableMonitor (IVariableMonitorP monitor, MdlDescP mdlDesc)
        {
        m_monitor = monitor;
        m_mdlDesc = mdlDesc;
        }
    bool operator==(VariableMonitor const& other) const
        {
        // we don't care about the MdlDesc.
        return m_monitor == other.m_monitor;
        }
    };

static bool    s_doDebug = false;
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
static void     ConfigVariableDebug (WCharCP format, ...) 
    {
    if (!s_doDebug)
        return;

    va_list args; 
    va_start (args, format); 
    WString tmp;
    tmp.VSprintf (format, args); 
    va_end (args);

    wprintf (tmp.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/15
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     ReplaceFromMap (WStringR translation, T_ReplacementMap& replacementMap)
    {
    // to make the comparison work, replace backslashes with forward slashes. Those are better to put into the output anyway.
    translation.ReplaceAll (L"\\", L"/");

    bool changed = false;
    for (T_ReplacementMap::iterator it = replacementMap.begin(); it != replacementMap.end(); ++it)
        {
        bpair <WString, WString> thisRemapEntry = *it;
        // we have to use ReplaceI because sometimes there are lower case definitions like $(_ustn_projectdata)
        while (translation.ReplaceI (thisRemapEntry.first.c_str(), thisRemapEntry.second.c_str()))
            {
            changed = true;
            }
        }

    return changed;
    }


typedef bvector<VariableMonitor>    T_VariableMonitors;
typedef bvector<MacroEntryCP>       T_DependentMacros;

/*=================================================================================**//**
* MacroEntry class
* @bsiclass                                     Barry.Bentley                   01/2012
+===============+===============+===============+===============+===============+======*/
struct MacroEntry
{
friend MacroConfigurationAdmin;

WString                     m_macroName;
mutable WStringP            m_sysEnv;
mutable WStringP            m_preDefined;    /* hidden level for built-in macros */
mutable WStringP            m_system;
mutable WStringP            m_application;
mutable WStringP            m_organization;
mutable WStringP            m_workspace;
mutable WStringP            m_workset;
mutable WStringP            m_role;
mutable WStringP            m_user;
mutable T_VariableMonitors* m_monitors;
mutable T_DependentMacros*  m_dependents;
mutable bool                m_locked;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/12
+---------------+---------------+---------------+---------------+---------------+------*/
MacroEntry (WCharCP macroName) : m_macroName (macroName)
    {
    // all macro names are upper case.
    m_macroName.Trim();
    m_macroName.ToUpper();
    m_sysEnv        = nullptr;
    m_preDefined    = nullptr;
    m_system        = nullptr;
    m_application   = nullptr;
    m_organization  = nullptr;
    m_workspace     = nullptr;
    m_workset       = nullptr;
    m_role          = nullptr;
    m_user          = nullptr;
    m_dependents    = nullptr;
    m_monitors      = nullptr;
    m_locked        = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/12
+---------------+---------------+---------------+---------------+---------------+------*/
MacroEntry ()
    {
    m_sysEnv        = nullptr;
    m_preDefined    = nullptr;
    m_system        = nullptr;
    m_application   = nullptr;
    m_organization  = nullptr;
    m_workspace     = nullptr;
    m_workset       = nullptr;
    m_role          = nullptr;
    m_user          = nullptr;
    m_dependents    = nullptr;
    m_monitors      = nullptr;
    m_locked        = false;
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
    if (CFGVAR_DEFINED_NULL != m_application)
        delete m_application;
    if (CFGVAR_DEFINED_NULL != m_organization)
        delete m_organization;
    if (CFGVAR_DEFINED_NULL != m_workspace)
        delete m_workspace;
    if (CFGVAR_DEFINED_NULL != m_workset)
        delete m_workset;
    if (CFGVAR_DEFINED_NULL != m_role)
        delete m_role;
    if (CFGVAR_DEFINED_NULL != m_user)
        delete m_user;

    if (nullptr != m_dependents)
        delete m_dependents;
    if (nullptr != m_monitors)
        delete m_monitors;
    }

#if defined (NEEDED_ONLY_IF_STRUCTS_IN_BSET)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/14
+---------------+---------------+---------------+---------------+---------------+------*/
MacroEntry (MacroEntryCR source)        // copy constructor
    {                                        
    *this = source;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/14
+---------------+---------------+---------------+---------------+---------------+------*/
MacroEntry& operator= (MacroEntryCR source)
    {
    if (this == &source)
        return *this;

    m_macroName.assign (source.m_macroName);

    if (CFGVAR_DEFINED_NULL == source.m_sysEnv)
        m_sysEnv = CFGVAR_DEFINED_NULL;
    else if (nullptr != source.m_sysEnv)
        m_sysEnv = new WString (*source.m_sysEnv);
    else
        m_sysEnv = nullptr;

    if (CFGVAR_DEFINED_NULL == source.m_preDefined)
        m_preDefined = CFGVAR_DEFINED_NULL;
    else if (nullptr != source.m_preDefined)
        m_preDefined = new WString (*source.m_preDefined);
    else
        m_preDefined = nullptr;

    if (CFGVAR_DEFINED_NULL == source.m_system)
        m_system = CFGVAR_DEFINED_NULL;
    else if (nullptr != source.m_system)
        m_system = new WString (*source.m_system);
    else
        m_system = nullptr;

    if (CFGVAR_DEFINED_NULL == source.m_application)
        m_application = CFGVAR_DEFINED_NULL;
    else if (nullptr != source.m_application)
        m_application = new WString (*source.m_application);
    else
        m_application = nullptr;

    if (CFGVAR_DEFINED_NULL == source.m_organization)
        m_organization = CFGVAR_DEFINED_NULL;
    else if (nullptr != source.m_organization)
        m_organization = new WString (*source.m_organization);
    else
        m_organization = nullptr;

    if (CFGVAR_DEFINED_NULL == source.m_workspace)
        m_workspace = CFGVAR_DEFINED_NULL;
    else if (nullptr != source.m_workspace)
        m_workspace = new WString (*source.m_workspace);
    else
        m_workspace = nullptr;

    if (CFGVAR_DEFINED_NULL == source.m_workset)
        m_workset = CFGVAR_DEFINED_NULL;
    else if (nullptr != source.m_workset)
        m_workset = new WString (*source.m_workset);
    else
        m_workset = nullptr;

    if (CFGVAR_DEFINED_NULL == source.m_role)
        m_role = CFGVAR_DEFINED_NULL;
    else if (nullptr != source.m_role)
        m_role = new WString (*source.m_role);
    else
        m_role = nullptr;

    if (CFGVAR_DEFINED_NULL == source.m_user)
        m_user = CFGVAR_DEFINED_NULL;
    else if (nullptr != source.m_user)
        m_user = new WString (*source.m_user);
    else
        m_user = nullptr;

    if (nullptr != source.m_dependents)
        m_dependents = new T_DependentMacros (*source.m_dependents);
    else
        m_dependents = nullptr;

    if (nullptr != source.m_monitors)
        m_monitors = new T_VariableMonitors (*source.m_monitors);
    else
        m_monitors = nullptr;

    m_locked = source.m_locked;

    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/14
+---------------+---------------+---------------+---------------+---------------+------*/
MacroEntry (MacroEntry const&& source)        // move constructor
    {
    *this = source;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/14
+---------------+---------------+---------------+---------------+---------------+------*/
MacroEntry& operator= (MacroEntry const&& source)
    {
    if (this == &source)
        return *this;

    m_macroName.assign (source.m_macroName);

    // copy the members from the source so we don't have to create new WStrings.
    m_sysEnv        = source.m_sysEnv;
    m_preDefined    = source.m_preDefined;
    m_system        = source.m_system;
    m_application   = source.m_application;
    m_organization  = source.m_organization;
    m_workspace     = source.m_workspace;
    m_workset       = source.m_workset;
    m_role          = source.m_role;
    m_user          = source.m_user;
    m_dependents    = source.m_dependents;
    m_monitors      = source.m_monitors;
    m_locked        = source.m_locked;

    // null out the source to prevent freeing.
    source.m_sysEnv         = nullptr;
    source.m_preDefined     = nullptr;
    source.m_system         = nullptr;
    source.m_application    = nullptr;
    source.m_organization   = nullptr;
    source.m_workspace      = nullptr;
    source.m_workset        = nullptr;
    source.m_role           = nullptr;
    source.m_user           = nullptr;
    source.m_dependents     = nullptr;
    source.m_monitors       = nullptr;

    return *this;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/12
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP     GetTranslationAtSysEnvLevel () const
    {
    /* Always re-get current operating system value unless locked */
    if (m_sysEnv != nullptr)
        {
        if (m_locked)
            return m_sysEnv->c_str();

        delete m_sysEnv;
        m_sysEnv = nullptr;
        }

    m_sysEnv = new WString();
    if (BSISUCCESS == util_getSysEnv (m_sysEnv, m_macroName.c_str()))
        return (m_sysEnv->c_str());
    else
        {
        delete m_sysEnv;
        m_sysEnv = nullptr;
        }

    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/12
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP     GetTranslationFromLevel (ConfigurationVariableLevel fromLevel) const
    {
    /* -----------------------------------------------------------
        Jump in at the value of "fromLevel" to begin searching
            from that level.  If not found at the starting level,
            keep dropping thru cases until found.
       ----------------------------------------------------------- */

    WStringP    translation = nullptr;
    switch (fromLevel)
        {
        case ConfigurationVariableLevel::User:
            if (m_user != nullptr)
                {
                translation = m_user;
                break;
                }

        case ConfigurationVariableLevel::Role:
            if (m_role != nullptr)
                {
                translation = m_role;
                break;
                }

        case ConfigurationVariableLevel::WorkSet:
            if (m_workset != nullptr)
                {
                translation = m_workset;
                break;
                }

        case ConfigurationVariableLevel::WorkSpace:
            if (m_workspace != nullptr)
                {
                translation = m_workspace;
                break;
                }

        case ConfigurationVariableLevel::Organization:
            if (m_organization != nullptr)
                {
                translation = m_organization;
                break;
                }

        case ConfigurationVariableLevel::Application:
            if (m_application != nullptr)
                {
                translation = m_application;
                break;
                }

        case ConfigurationVariableLevel::System:
            if (m_system != nullptr)
                {
                translation = m_system;
                break;
                }

        case ConfigurationVariableLevel::Predefined:
            if (m_preDefined != nullptr)
                {
                translation = m_preDefined;
                break;
                }

        case ConfigurationVariableLevel::SysEnv:
            {
            WCharCP     transChars;
            if (nullptr != (transChars = GetTranslationAtSysEnvLevel ()))
                return transChars;
            break;
            }
        }

    if (nullptr == translation)
        return nullptr;

    return (CFGVAR_DEFINED_NULL != translation) ? translation->c_str() : EMPTY_STRING;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   GetTranslationAtSpecificLevel (WStringP envStr, ConfigurationVariableLevel level) const
    {
    if (nullptr != envStr)
        envStr->clear();

    // get pointer to translation at desired level.
    WStringP    translation = nullptr;
    switch (level)
        {
        case ConfigurationVariableLevel::User:
            translation = m_user;
            break;

        case ConfigurationVariableLevel::Role:
            translation = m_role;
            break;

        case ConfigurationVariableLevel::WorkSet:
            translation = m_workset;
            break;

        case ConfigurationVariableLevel::WorkSpace:
            translation = m_workspace;
            break;

        case ConfigurationVariableLevel::Organization:
            translation = m_organization;
            break;

        case ConfigurationVariableLevel::Application:
            translation = m_application;
            break;

        case ConfigurationVariableLevel::System:
            {
            WCharCP transChar;
            if (nullptr != (transChar = GetTranslationAtSysEnvLevel ()))
                {
                if (nullptr != envStr)
                    envStr->assign (transChar);
                return BSISUCCESS;
                }
            translation = m_system;
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
    if (nullptr != translation)
        {
        if ( (nullptr != envStr) && (CFGVAR_DEFINED_NULL != translation) )
            envStr->assign (translation->c_str());

        return BSISUCCESS;
        }

    return BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool            SameDefinition (WCharCP newTranslation, ConfigurationVariableLevel newLevel) const
    {
    // if you change the definition at a level that is blocked by a higher priority definition, it's still the same.
    WCharCP     oldTranslation;
    if (nullptr == (oldTranslation = GetTranslationFromLevel (ConfigurationVariableLevel::User)))
        return false;

    return (0 == wcscmp (oldTranslation, newTranslation));
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
WStringP        GetTranslationAtNextLowerLevel (ConfigurationVariableLevel thisLevel) const
    {
    if (thisLevel > ConfigurationVariableLevel::User)
        {
        if (nullptr != m_user)
            return m_user;
        }

    if (thisLevel > ConfigurationVariableLevel::Role)
        {
        if (nullptr != m_role)
            return m_role;
        }

    if (thisLevel > ConfigurationVariableLevel::WorkSet)
        {
        if (nullptr != m_workset)
            return m_workset;
        }

    if (thisLevel > ConfigurationVariableLevel::WorkSpace)
        {
        if (nullptr != m_workspace)
            return m_workspace;
        }

    if (thisLevel > ConfigurationVariableLevel::Organization)
        {
        if (nullptr != m_organization)
            return m_organization;
        }

    if (thisLevel > ConfigurationVariableLevel::Application)
        {
        if (nullptr != m_application)
            return m_application;
        }

    if (thisLevel > ConfigurationVariableLevel::System)
        {
        if (nullptr != m_system)
            return m_system;
        }

    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool            HasTranslationAtHigherLevel (ConfigurationVariableLevel thisLevel) const
    {
    if (thisLevel < ConfigurationVariableLevel::User)
        {
        if (nullptr != m_user)
            return true;
        }

    if (thisLevel < ConfigurationVariableLevel::Role)
        {
        if (nullptr != m_role)
            return true;
        }

    if (thisLevel < ConfigurationVariableLevel::WorkSet)
        {
        if (nullptr != m_workset)
            return true;
        }

    if (thisLevel < ConfigurationVariableLevel::WorkSpace)
        {
        if (nullptr != m_workspace)
            return true;
        }

    if (thisLevel < ConfigurationVariableLevel::Organization)
        {
        if (nullptr != m_organization)
            return true;
        }

    if (thisLevel < ConfigurationVariableLevel::Application)
        {
        if (nullptr != m_application)
            return true;
        }

    if (thisLevel < ConfigurationVariableLevel::System)
        {
        if (nullptr != m_system)
            return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    04/93
+---------------+---------------+---------------+---------------+---------------+------*/
bool            RemoveAtLevel (bool& stillHasTranslation, bool& nowHasDifferentTranslation, ConfigurationVariableLevel level, bool removeLocked, MacroConfigurationAdmin& mca) const
    {
    // these outputs are valid only if function return true!
    stillHasTranslation = false;
    nowHasDifferentTranslation = false;

    if (m_locked && !removeLocked)
        return false;

    WStringP* translation = nullptr;
    switch (level)
        {
        case ConfigurationVariableLevel::System:
            translation = &m_system;
            break;

        case ConfigurationVariableLevel::Application:
            translation = &m_application;
            break;

        case ConfigurationVariableLevel::Organization:
            translation = &m_organization;
            break;

        case ConfigurationVariableLevel::WorkSpace:
            translation = &m_workspace;
            break;

        case ConfigurationVariableLevel::WorkSet:
            translation = &m_workset;
            break;

        case ConfigurationVariableLevel::Role:
            translation = &m_role;
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

    // if it has no translation at this level, return false.
    if (nullptr == *translation)
        return false;

    // if the macro has a translation at a higher level, it's value (when retrieved at User level, as it almost always is) is unchanged. So do not remove it as a dependent.
    WStringP    remainingTranslation;
    if (HasTranslationAtHigherLevel (level))
        {
        stillHasTranslation = true;
        nowHasDifferentTranslation = false;
        }
    // Will there still be a translation (for example, if we are removing it at the Project level, is there a translation at the System level?).
    else if (nullptr != (remainingTranslation = GetTranslationAtNextLowerLevel (level)))
        {
        stillHasTranslation = true;

        if (CFGVAR_DEFINED_NULL == *translation)
            {
            nowHasDifferentTranslation = (CFGVAR_DEFINED_NULL != remainingTranslation);
            }
        else if (0 != remainingTranslation->CompareTo ((*translation)->c_str()))
            {
            nowHasDifferentTranslation = true;
            RemoveFromRootsAsDependent (mca);
            }
        }
    else
        {
        // has translation only at this level. 
        if (CFGVAR_DEFINED_NULL != *translation)
            RemoveFromRootsAsDependent (mca);
        }

    if (CFGVAR_DEFINED_NULL != *translation)
        delete *translation;

    *translation = nullptr;
    m_locked = false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/12
+---------------+---------------+---------------+---------------+---------------+------*/
ConfigurationVariableLevel  GetLevelOfDefinition () const
    {
    if (m_user != nullptr)
        return ConfigurationVariableLevel::User;

    if (m_role != nullptr)
        return ConfigurationVariableLevel::Role;

    if (m_workset != nullptr)
        return ConfigurationVariableLevel::WorkSet;

    if (m_workspace != nullptr)
        return ConfigurationVariableLevel::WorkSpace;

    if (m_organization != nullptr)
        return ConfigurationVariableLevel::Organization;

    if (m_application != nullptr)
        return ConfigurationVariableLevel::Application;

    if (m_system != nullptr)
        return ConfigurationVariableLevel::System;

    if (m_preDefined != nullptr)
        return ConfigurationVariableLevel::Predefined;

    if (m_sysEnv != nullptr)
        return ConfigurationVariableLevel::SysEnv;

    return INVALID_MACRO_LEVEL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    04/93
+---------------+---------------+---------------+---------------+---------------+------*/
void    SetTranslation (WCharCP translation, ConfigurationVariableLevel level) const
    {
    /* If variable is locked, no other values may be set */
    if (m_locked)
        return;

    /* Even if translation nullptr set to a non-nullptr value */
    WStringP setTrans;
    if (nullptr == translation)
        setTrans = CFGVAR_DEFINED_NULL;
    else
        setTrans = new WString (translation);

    switch (level)
        {
        case ConfigurationVariableLevel::System:
            delete m_system;
            m_system = setTrans;
            break;

        case ConfigurationVariableLevel::Application:
            delete m_application;
            m_application = setTrans;
            break;

        case ConfigurationVariableLevel::Organization:
            delete m_organization;
            m_organization = setTrans;
            break;

        case ConfigurationVariableLevel::WorkSpace:
            delete m_workspace;
            m_workspace = setTrans;
            break;

        case ConfigurationVariableLevel::WorkSet:
            delete m_workset;
            m_workset = setTrans;
            break;

        case ConfigurationVariableLevel::Role:
            delete m_role;
            m_role = setTrans;
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
bool    HasTranslationAtLevel (ConfigurationVariableLevel level) const
    {
    /* -----------------------------------------------------------
        Jump in at the value of "fromLevel" to begin searching
        from that level.  If not found at the starting level,
        keep dropping thru cases until found.
    ----------------------------------------------------------- */
    switch (level)
        {
        case ConfigurationVariableLevel::User:
            if (m_user != nullptr)
                return true;

        case ConfigurationVariableLevel::Role:
            if (m_role != nullptr)
                return true;

        case ConfigurationVariableLevel::WorkSet:
            if (m_workset != nullptr)
                return true;

        case ConfigurationVariableLevel::WorkSpace:
            if (m_workspace!= nullptr)
                return true;

        case ConfigurationVariableLevel::Organization:
            if (m_organization != nullptr)
                return true;

        case ConfigurationVariableLevel::Application:
            if (m_application != nullptr)
                return true;

        case ConfigurationVariableLevel::System:
            if (m_system != nullptr)
                return true;

        case ConfigurationVariableLevel::Predefined:
            if (m_preDefined != nullptr)
                return true;

        case ConfigurationVariableLevel::SysEnv:
            if (m_sysEnv != nullptr)
                return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool    IsLocked() const
    {
    return m_locked;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/12
+---------------+---------------+---------------+---------------+---------------+------*/
void    SetLocked (bool locked) const
    {
    m_locked = locked;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    04/93
+---------------+---------------+---------------+---------------+---------------+------*/
bool     HasAnyTranslation () const
    {
    return ( (nullptr != m_user) || (nullptr != m_role) || (nullptr != m_workset) || (nullptr != m_workspace) || (nullptr != m_organization) || 
             (nullptr != m_application) || (nullptr != m_system) || (nullptr != m_preDefined) ||  (nullptr != m_sysEnv) );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    04/93
+---------------+---------------+---------------+---------------+---------------+------*/
void            PrintCfgVarSummaryLine (IMacroDebugOutput& debugOutput, WCharCP macroName, MacroConfigurationAdmin& macroCfgAdmin) const
    {
    debugOutput.ShowDebugMessage (0, L"%-20ls: %-14ls = ", macroName, getLevelDebugString (GetLevelOfDefinition()));

    WCharCP translation;
    if (nullptr != (translation= GetTranslationFromLevel (ConfigurationVariableLevel::User)))
        {
        WString expansion;
        macroCfgAdmin.ExpandMacro (expansion, translation, MacroExpandOptions (ConfigurationVariableLevel::User));
        if (expansion.empty())
            expansion.assign (L"(null)");

        // if it's too long, break into multiple lines to make it easier to read.
        if (expansion.length() > 100)
            {
            breakIntoLines (debugOutput, expansion, 150, 22);
            if (m_locked)
                debugOutput.ShowDebugMessage (0, L"%ls\n", L"<Locked>");
            }
        else
            debugOutput.ShowDebugMessage (0, L"%ls %ls\n", expansion.c_str(), (m_locked) ? L"<Locked>" : L" ");
        }
    else
        {
        debugOutput.ShowDebugMessage (0, L"(null) %ls\n", (m_locked) ? L"<Locked>" : L" ");
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    04/93
+---------------+---------------+---------------+---------------+---------------+------*/
void            PrintLevelCfgVarSummaryLine (IMacroDebugOutput& debugOutput, WCharCP macroName, MacroConfigurationAdmin& macroCfgAdmin) const
    {
    debugOutput.ShowDebugMessage (0, L"%-17ls: ", macroName);
    if (!HasAnyTranslation())
        {
        debugOutput.ShowDebugMessage (0, L"(null) %ls\n", (m_locked) ? L"<Locked>" : L" ");
        return;
        }

    int counter = 0;
    for (ConfigurationVariableLevel iLevel=ConfigurationVariableLevel::Predefined; static_cast<int>(iLevel)<=static_cast<int>(ConfigurationVariableLevel::User); iLevel = (ConfigurationVariableLevel)(static_cast<int>(iLevel)+1))
        {
        if (!HasTranslationAtLevel (iLevel))
            continue;

        WString     translation;
        GetTranslationAtSpecificLevel (&translation, iLevel);

        WString     expansion;
        macroCfgAdmin.ExpandMacro (expansion, translation.c_str(), MacroExpandOptions(iLevel));

        if (counter > 0)
            debugOutput.ShowDebugMessage (0, L"%-17ls: ", L" ");

        debugOutput.ShowDebugMessage (0, L"%-12ls = ", getLevelDebugString (iLevel));
        debugOutput.ShowDebugMessage (0, L"%ls", expansion.empty () ? L"(null)" : expansion.c_str() );

        if (0 == counter)
            debugOutput.ShowDebugMessage (0, L" %ls\n", (m_locked) ? L"<Locked>" : L" ");
        else
            debugOutput.ShowDebugMessage (0, L"\n");

        counter++;
        }

    T_WStringVector dependencies;
    macroCfgAdmin.GetDependentMacros (dependencies, macroName);
    if (dependencies.size() > 0)
        {
        debugOutput.ShowDebugMessage (0, L"    Affects the definition of:\n");
        for (WStringR dependent : dependencies)
            {
            debugOutput.ShowDebugMessage (0, L"   %ls\n", dependent.c_str());
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/14
+---------------+---------------+---------------+---------------+---------------+------*/
void            AddMonitor (IVariableMonitorP monitor) const
    {
    if (nullptr == m_monitors)
        m_monitors = new T_VariableMonitors();

#if defined (NEEDSWORK_DESKTOP_PLATFORM)
    VariableMonitor variableMonitor (monitor, T_HOST.GetSessionAdmin()._GetCurrentMdlDescr());
#else
    VariableMonitor variableMonitor (monitor, nullptr);
#endif
    if (m_monitors->end() == std::find (m_monitors->begin(), m_monitors->end(), variableMonitor))
        m_monitors->push_back (variableMonitor);

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/14
+---------------+---------------+---------------+---------------+---------------+------*/
void            RemoveMonitor (IVariableMonitorR monitor) const
    {
    if (nullptr == m_monitors)
        return;

    VariableMonitor variableMonitor (&monitor, nullptr);
    T_VariableMonitors::iterator thisMonitor;
    if (m_monitors->end() != (thisMonitor = std::find (m_monitors->begin(), m_monitors->end(), variableMonitor)))
        {
#if defined (NEEDSWORK_DESKTOP_PLATFORM)
        SaveAndSwitchMdlDesc _v_v_v ((*thisMonitor).m_mdlDesc);
#endif

        (*thisMonitor).m_monitor->_MonitorStopped (m_macroName.c_str());
        m_monitors->erase (thisMonitor);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
void            AddToRootsAsDependent (MacroConfigurationAdmin& mca) const
    {
    ProcessDependencies (true, mca);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
void            RemoveFromRootsAsDependent (MacroConfigurationAdmin& mca) const
    {
    ProcessDependencies (false, mca);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
void            DiscardAllDependents () const
    {
    delete m_dependents;
    m_dependents = nullptr;
    }

enum class MacroSearchState
    {
    LookingForStart,
    InParentheses,
    LookingForFinalCloseParenthesis
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
void            ProcessDependencies (bool add, MacroConfigurationAdmin& mca) const
    {
    // if we have dependencies suspended, don't keep them up to date.
    if (mca.m_dependenciesSuspended)
        return;

    WCharCP         translation = GetTranslationFromLevel (ConfigurationVariableLevel::User);
    if (nullptr == translation)
        return;

    // find any occurrences of a macro between () or {}.
    MacroSearchState state = MacroSearchState::LookingForStart;
    int              parenNestDepth = 0;
    WCharCP          startMacro = nullptr;
    for (WCharCP thisChar = translation; 0 != *thisChar; thisChar++)
        {
        switch (state)
            {
            case MacroSearchState::LookingForStart:
                {
                // look for an open parenthesis.
                if ('(' == *thisChar)
                    {
                    state = MacroSearchState::InParentheses;
                    startMacro = thisChar+1;
                    parenNestDepth = 1;
                    }
                break;
                }

            case MacroSearchState::InParentheses:
                {
                if ('(' == *thisChar)
                    {
                    parenNestDepth++;
                    startMacro = thisChar+1;
                    }
                else if (')' == *thisChar)
                    {
                    // we have found what might be a macro.
                    if ( (nullptr != startMacro) && (startMacro < thisChar) )
                        {
                        // we found what is believed to be a macro. dependentMacro depends on thisMacro.
                        WString         possibleMacro (startMacro, (thisChar - startMacro));
                        MacroEntryCP    rootMacro;
                        if (NULL == (rootMacro = mca.GetMacroDefinition (possibleMacro.c_str())))
                            {
                            // if it's a macro starting with _USTN_, create that macro as undefined.
                            if (0 == wcsncmp (possibleMacro.c_str(), L"_USTN_", 6))
                                mca.CreateMacro (rootMacro, possibleMacro.c_str());
                            }

                        if (NULL != rootMacro)
                            {
                            if (add)
                                rootMacro->AddDependent (*this);
                            else
                                rootMacro->RemoveDependent (*this);
                            }
                        else
                            {
                            if (add)
                                ConfigVariableDebug (L"cfgVar %ls depends on cfgVar %ls, but %ls is not defined\n", m_macroName.c_str(), possibleMacro.c_str(), possibleMacro.c_str());
                            }
                        }

                    if (--parenNestDepth <= 0)
                        state = MacroSearchState::LookingForStart;
                    else
                        state = MacroSearchState::LookingForFinalCloseParenthesis;
                    }
                break;
                }

            case MacroSearchState::LookingForFinalCloseParenthesis:
                {
                if ('(' == *thisChar)
                    {
                    state = MacroSearchState::InParentheses;
                    parenNestDepth++;
                    startMacro = thisChar+1;
                    }
                else if (')' == *thisChar)
                    {
                    if (--parenNestDepth <= 0)
                        state = MacroSearchState::LookingForStart;
                        
                    }
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/14
+---------------+---------------+---------------+---------------+---------------+------*/
void            AddDependent (MacroEntryCR dependent) const
    {
    if (nullptr == m_dependents)
        m_dependents = new T_DependentMacros;

    // make sure we don't duplicate dependents.
    if (m_dependents->end() == std::find (m_dependents->begin(), m_dependents->end(), &dependent))
        m_dependents->push_back (&dependent);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/14
+---------------+---------------+---------------+---------------+---------------+------*/
void            RemoveDependent (MacroEntryCR dependent) const
    {
    if (nullptr == m_dependents)
        return;

    T_DependentMacros::iterator found;
    if (m_dependents->end() != (found = std::find (m_dependents->begin(), m_dependents->end(), &dependent)))
        m_dependents->erase (found);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/14
+---------------+---------------+---------------+---------------+---------------+------*/
void            ClearDependencies () const
    {
    if (nullptr != m_dependents)
        m_dependents->clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
void            RemoveAllTranslations () const
    {
    if (CFGVAR_DEFINED_NULL != m_sysEnv)
        DELETE_AND_CLEAR (m_sysEnv);
    if (CFGVAR_DEFINED_NULL != m_preDefined)
        DELETE_AND_CLEAR (m_preDefined);
    if (CFGVAR_DEFINED_NULL != m_system)
        DELETE_AND_CLEAR (m_system);
    if (CFGVAR_DEFINED_NULL != m_application)
        DELETE_AND_CLEAR (m_application);
    if (CFGVAR_DEFINED_NULL != m_organization)
        DELETE_AND_CLEAR (m_organization);
    if (CFGVAR_DEFINED_NULL != m_workspace)
        DELETE_AND_CLEAR (m_workspace);
    if (CFGVAR_DEFINED_NULL != m_workset)
        DELETE_AND_CLEAR (m_workset);
    if (CFGVAR_DEFINED_NULL != m_role)
        DELETE_AND_CLEAR (m_role);
    if (CFGVAR_DEFINED_NULL != m_user)
        DELETE_AND_CLEAR (m_user);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/15
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   GetAppropriateAssignmentStatement (WStringR assignmentStatement, ConfigurationVariableLevel level, T_ReplacementMap& replacementMap) const
    {
    // this method is called from the configuration conversion process. The objective is to get the right assignment
    // statement at "level" given the other assignments. The right assignment statement could be an =, <, >, or +,
    // depending on the assignment at level and at the lower levels.
    if (!HasTranslationAtLevel (level))
        return BSIERROR;

    WString     currentAssignment;
    GetTranslationAtSpecificLevel (&currentAssignment, level);
    if ( currentAssignment.empty() )
        return BSIERROR;

    WStringP        nextLowerAssignment;
    // if there is no priority assignment, or if the new assignment is the same, then it's a simple assignment.
    if ( (nullptr == (nextLowerAssignment = GetTranslationAtNextLowerLevel (level)) ) || 
         (currentAssignment.size() <= nextLowerAssignment->size()) )
        {
        currentAssignment.ReplaceAll (L"\\", L"/");
        assignmentStatement.Sprintf (L"= %ls", currentAssignment.c_str());
        return BSISUCCESS;
        }

    ReplaceFromMap (*nextLowerAssignment, replacementMap);

    // is it an append? 
    if (currentAssignment.StartsWithI (nextLowerAssignment->c_str()))
        {
        // could be a '+' or a '>' type. They are distinguished by a semicolon separating or not.
        if (';' == currentAssignment[nextLowerAssignment->size()])
            {
            // erase the part that's the same and the semicolon.
            currentAssignment.erase (0, nextLowerAssignment->size()+1);
            currentAssignment.ReplaceAll (L"\\", L"/");
            assignmentStatement.Sprintf (L"> %ls", currentAssignment.c_str());
            return BSISUCCESS;
            }
#if defined (TRY_NONPATH_APPEND)
        // I decided not do to this when I found a case where MS_DRAWINGMODELSEEDName was defined as Design at System level and then in the .cpf it was defined as DrawingSeed. The new assignment of "+ Seed" didn't seem useful.
        else
            {
            // if the current assignment is a directory (ends with a / or \), it's just a coincidence that they start the same. Fall through to "no relationship" case.
            if (!currentAssignment.EndsWith (L"/") && !currentAssignment.EndsWith (L"\\"))
                {
                // erase the part that's the same.
                currentAssignment.erase (0, nextLowerAssignment->size());
                currentAssignment.ReplaceAll (L"\\", L"/");
                assignmentStatement.Sprintf (L"+ %ls", currentAssignment.c_str());
                return BSISUCCESS;
                }
            }
#endif
        }
    // is it a prepend? The ending parts must match and there must be a semicolon separator before the matching part.
    else if ( (currentAssignment.EndsWithI (nextLowerAssignment->c_str())) && (';' == currentAssignment[currentAssignment.size() - nextLowerAssignment->size() - 1]) )
        {
        currentAssignment.erase (currentAssignment.size() - nextLowerAssignment->size() - 1, WString::npos);
        currentAssignment.ReplaceAll (L"\\", L"/");
        assignmentStatement.Sprintf (L"< %ls", currentAssignment.c_str());
        return BSISUCCESS;
        }

    // no relationship between the value and lower level values, just assign.
    currentAssignment.ReplaceAll (L"\\", L"/");
    assignmentStatement.Sprintf (L"= %ls", currentAssignment.c_str());
    return BSISUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool            InformOfChangedValue (bool isRoot, WCharCP rootMacroName) const
    {
    bool    anyMonitorFound = false;

    // this macro is getting undefined. Inform monitors and dependents.
    if (nullptr != m_monitors)
        {
        for (VariableMonitor variableMonitor : *m_monitors)
            {
            if (s_doDebug)
                ConfigVariableDebug (L"Value of %ls changing\n", rootMacroName);

#if defined (NEEDSWORK_DESKTOP_PLATFORM)
            SaveAndSwitchMdlDesc _v_v_v (variableMonitor.m_mdlDesc);
#endif
            if (isRoot)
                variableMonitor.m_monitor->_VariableChanged (rootMacroName);
            else
                variableMonitor.m_monitor->_VariableRootChanged (m_macroName.c_str(), rootMacroName);
            anyMonitorFound = true;
            }
        }

    if (nullptr == m_dependents)
        return anyMonitorFound;

    for (MacroEntryCP dependent : *m_dependents)
        {
        bool dependentMonitorFound = dependent->InformOfChangedValue (false, rootMacroName);
        if (dependentMonitorFound)
            anyMonitorFound = true;
        }

    return anyMonitorFound;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool            InformOfUndefine (bool isRoot, WCharCP rootMacroName) const
    {
    bool    anyMonitorFound = false;

    // this macro is getting undefined. Inform monitors and dependents.
    if (nullptr != m_monitors)
        {
        for (VariableMonitor variableMonitor : *m_monitors)
            {
#if defined (NEEDSWORK_DESKTOP_PLATFORM)
            SaveAndSwitchMdlDesc _v_v_v (variableMonitor.m_mdlDesc);
#endif
            if (isRoot)
                variableMonitor.m_monitor->_VariableUndefined (rootMacroName);
            else
                variableMonitor.m_monitor->_VariableRootUndefined (m_macroName.c_str(), rootMacroName);

            anyMonitorFound = true;
            }
        }

    if (nullptr == m_dependents)
        return anyMonitorFound;

    for (MacroEntryCP dependent : *m_dependents)
        {
        bool dependentMonitorFound = dependent->InformOfUndefine (false, rootMacroName);
        if (dependentMonitorFound)
            anyMonitorFound = true;
        }

    return anyMonitorFound;
    }

};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool CompareMacroEntries::operator () (MacroEntryCP first, MacroEntryCP second) const
    {
    return (first->m_macroName.CompareTo (second->m_macroName) < 0);
    }


END_BENTLEY_DGNPLATFORM_NAMESPACE


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
MacroConfigurationAdmin::MacroConfigurationAdmin ()
    {
    // m_dependenciesFound is set to true when we determine the dependencies (when the first dependency set is requested)
    m_dependenciesSuspended = false;
    m_searchEntry           = NULL;
    SetDefaultExpansionOperators();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
MacroConfigurationAdmin::~MacroConfigurationAdmin ()
    {
    // m_dependenciesFound is set to true when we determine the dependencies (when the first dependency set is requested)
    for (T_MacroSet::iterator it = m_macroSet.begin(); it != m_macroSet.end(); )
        {
        MacroEntry* macroEntry = *it;
        delete (macroEntry);
        it = m_macroSet.erase(it);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    04/93
+---------------+---------------+---------------+---------------+---------------+------*/
bool        MacroConfigurationAdmin::IsValidCfgVarLevelForNewDefinition (ConfigurationVariableLevel level)
    {
    return ((level <= ConfigurationVariableLevel::User) && (level >=ConfigurationVariableLevel::SysEnv));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    05/95
+---------------+---------------+---------------+---------------+---------------+------*/
bool        MacroConfigurationAdmin::IsMacroNameValid (WCharCP macroName)
    {
    /* check for valid macro name */
    if ( (nullptr== macroName) || (0 == *macroName) )
        return false;

    // must have at least a two character macro name so it won't be confused with a drive letter.
    if ((0 == *(macroName+1)) || (wcslen (macroName) >= MAXNAMELENGTH) )
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MacroEntryCP    MacroConfigurationAdmin::GetMacroDefinition (WCharCP macroName)
    {
    // avoid having to construct a new MacroEntry every time - all we are going to use is the name.
    if (NULL == m_searchEntry)
        m_searchEntry = new MacroEntry (macroName);
    else
        {
        m_searchEntry->m_macroName.assign (macroName);
        m_searchEntry->m_macroName.ToUpper();
        }


    T_MacroSet::const_iterator  definition;
    if (m_macroSet.end() == (definition = m_macroSet.find (m_searchEntry)))
        return nullptr;

    return *definition;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/14
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP     MacroConfigurationAdmin::GetMacroEntryAndTranslation (WCharCP macroName, MacroEntryCP& macroEntry, WStringR tmpStorage, ConfigurationVariableLevel level, bool addIfNotDefined)
    {
    macroEntry = nullptr;

    if (!IsMacroNameValid (macroName))
        return nullptr;

    if (nullptr != (macroEntry = GetMacroDefinition (macroName)))
        {
        WCharCP translation = macroEntry->GetTranslationFromLevel (level);
        return translation;
        }

    // This is used from _MonitorVariable. If a configuration variable that is not yet defined is monitored, we need to keep track of it in case it is set later.
    if (addIfNotDefined)
        {
        // this should only be done if we searched at "User" level.
        BeAssert (ConfigurationVariableLevel::User == level);
        // However, we should set it at the most overrideable level, which is System.
        CreateMacro (macroEntry, macroName);
        }

    if (util_getSysEnv (&tmpStorage, macroName) == BSISUCCESS)
        return  tmpStorage.c_str();

    return  nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP     MacroConfigurationAdmin::GetMacroTranslation (WCharCP macroName, WStringR tmpStorage, ConfigurationVariableLevel level)
    {
    MacroEntryCP macroEntry;
    return GetMacroEntryAndTranslation (macroName, macroEntry, tmpStorage, level, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   10/92
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   MacroConfigurationAdmin::GetMacro (WStringP envStr, WCharCP macroName)
    {
    if (nullptr != envStr)
        envStr->empty();

    WString         tmpStorage;
    WCharCP         translation;
    MacroEntryCP    macroEntry;
    if (nullptr == (translation = GetMacroEntryAndTranslation (macroName, macroEntry, tmpStorage, ConfigurationVariableLevel::User, false)))
        return BSIERROR;

    if (nullptr != envStr)
        envStr->assign (translation);

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    04/93
+---------------+---------------+---------------+---------------+---------------+------*/
bool MacroConfigurationAdmin::IsMacroDefinedFromLevel (WCharCP macroName, ConfigurationVariableLevel fromLevel)
    {
    // check for valid macro name */
    if ( (nullptr == macroName) || (0 == *macroName) )
        return false;

    // if defined in macro map, return true.
    MacroEntryCP macroEntry = GetMacroDefinition (macroName);
    if ( (nullptr != macroEntry) && macroEntry->HasTranslationAtLevel (fromLevel) )
        return true;

    // otherwise check system environment.
    return BSISUCCESS == util_getSysEnv (nullptr, macroName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    04/93
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus MacroConfigurationAdmin::GetMacroFromLevel (WStringP envStr, WCharCP macroName, ConfigurationVariableLevel level)
    {
    if (!IsValidCfgVarLevelForNewDefinition (level))
        return BSIERROR;

    if (nullptr != envStr)
        envStr->clear();

    WString         tmpStorage;
    WCharCP         translation;
    MacroEntryCP    macroEntry;
    if (nullptr != (translation = GetMacroEntryAndTranslation (macroName, macroEntry, tmpStorage, level, false)))
        {
        if (nullptr != envStr)
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
    if (nullptr != envStr)
        envStr->clear();

    // check for valid macro name
    if ( (nullptr == macroName) || (0 == *macroName) )
        return BSIERROR;

    if ( !IsValidCfgVarLevelForNewDefinition (level) && (ConfigurationVariableLevel::Predefined != level) )
        return BSIERROR;

    // if not defined in macro map, error.
    MacroEntryCP macroEntry;
    if (nullptr == (macroEntry = GetMacroDefinition (macroName)))
        return BSIERROR;

    return macroEntry->GetTranslationAtSpecificLevel (envStr, level);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    03/93
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   MacroConfigurationAdmin::GetMacroLevel (ConfigurationVariableLevel& level, WCharCP macroName)
    {
    // check for valid macro name
    if ( (nullptr == macroName) || (0 == *macroName) )
        return BSIERROR;

    // see if there is an entry for it.
    MacroEntryCP macroEntry;
    if (nullptr != (macroEntry = GetMacroDefinition (macroName)))
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
BentleyStatus MacroConfigurationAdmin::DefineMacro (MacroEntryCP& newEntry, WCharCP macroName, WCharCP translation, ConfigurationVariableLevel level)
    {
    newEntry = nullptr;

    if ( (nullptr == macroName) || (0 == *macroName) )
        return BSIERROR;

    // Note: m_macroSet is a set of pointers rather than MacroEntry structs because they have pointers to each other through the m_dependents member. Those pointers are
    //       not stable in a set of structs when the structs are rearranged within the set.
    // put a new MacroEntry pointer into the set.
    MacroEntry  *tmp = new MacroEntry (macroName);
    bpair <T_MacroSet::const_iterator, bool> inserted = m_macroSet.insert(tmp);
    newEntry = *inserted.first;

    // if it was not inserted (inserted.second false), we must remove it as a dependent before we set the translation.
    if (!inserted.second)
        {
        // if it is the same definition as before, no need to do all the dependent determination and call monitors
        if (newEntry->SameDefinition (translation, level))
            {
            // even if it already has the same definition at a lower level, we need to set it at this level, (but we can skip all the dependents and informing of change).
            newEntry->SetTranslation (translation, level);
            return BSISUCCESS;
            }

        newEntry->RemoveFromRootsAsDependent (*this);

        // since tmp was not inserted, we need to delete it.
        delete tmp;
        }

    newEntry->SetTranslation (translation, level);

    // inform monitors of new value.
    newEntry->InformOfChangedValue (true, macroName);

    // Now add it as a dependent to whatever it currently depends on.
    newEntry->AddToRootsAsDependent (*this);

    return  BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus MacroConfigurationAdmin::CreateMacro (MacroEntryCP& newEntry, WCharCP macroName)
    {
    newEntry = nullptr;

    if ( (nullptr == macroName) || (0 == *macroName) )
        return BSIERROR;

    // Note: m_macroSet is a set of pointers rather than MacroEntry structs because they have pointers to each other through the m_dependents member. Those pointers are
    //       not stable in a set of structs when the structs are rearranged within the set.
    // put a new MacroEntry pointer into the set.
    MacroEntry  *tmp = new MacroEntry (macroName);
    bpair <T_MacroSet::const_iterator, bool> inserted = m_macroSet.insert(tmp);
    newEntry = *inserted.first;

    return  BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   MacroConfigurationAdmin::DefineMacroWithDebugging (WCharCP macroName, WCharCP translation, ConfigurationVariableLevel level, int indent, IMacroDebugOutput* debugOutput, int debugLevel)
    {
    MacroEntryCP    macroEntry;
    BentleyStatus   status;
    if (BSISUCCESS != (status = DefineMacro (macroEntry, macroName, translation, level)))
        return status;

    if (nullptr != debugOutput)
        {
        WCharCP  outTrans = macroEntry->GetTranslationFromLevel (level);
        debugOutput->ShowDebugMessage (indent, L"(%ls): %ls=%ls", getLevelDebugString (level), macroName, (outTrans != nullptr) ? outTrans : L"(null)");

        if ((debugLevel > 1) && (nullptr != outTrans))
            {
            WString expansion;
            ExpandMacro (expansion, outTrans, ExpandOptions(level));
            debugOutput->ShowDebugMessage (0, L" [%ls]", expansion.empty() ? L"(null)" : expansion.c_str() );
            }
        debugOutput->ShowDebugMessage (0, L"\n");
        }

    return BSISUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2010
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   MacroConfigurationAdmin::DefineBuiltinMacro (WCharCP macroName, WCharCP translation)
    {
    MacroEntryCP macroEntry;
    return DefineMacro (macroEntry, macroName, translation, ConfigurationVariableLevel::Predefined);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    04/93
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   MacroConfigurationAdmin::RemoveMacroAtLevel (WCharCP macroName, ConfigurationVariableLevel level, bool removeLocked)
    {
    if ( (nullptr == macroName) || (0 == *macroName) )
        return BSIERROR;

    if (!IsValidCfgVarLevelForNewDefinition (level))
        return BSIERROR;

    MacroEntryCP macroEntry;
    if (nullptr == (macroEntry = GetMacroDefinition (macroName)))
        return BSIERROR;

    // its dependents might change.
    bool    stillHasTranslation = false;
    bool    nowHasDifferentTranslation = false;
    if (macroEntry->RemoveAtLevel (stillHasTranslation, nowHasDifferentTranslation, level, removeLocked, *this))
        {
        // it had a translation at level.
        if (!stillHasTranslation)
            {
            // if it no longer has a translation, just undefine it.
            return _UndefineConfigVariable (macroName);
            }
        else if (nowHasDifferentTranslation)
            {
            // it still has a translation, and its value is different.
            macroEntry->InformOfChangedValue (true, macroName);
            macroEntry->AddToRootsAsDependent (*this);
            }
        }

    return BSISUCCESS;
    }

#if defined (DebugRemove)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
void            MacroConfigurationAdmin::VerifyNotDependentOnAnyMacro (MacroEntryCP testMacro)
    {
    for (MacroEntryCP thisMacro : m_macroSet)
        {
        if (nullptr == thisMacro->m_dependents)
            continue;
        for (MacroEntryCP dependent : *thisMacro->m_dependents)
            {
            BeAssert (dependent != testMacro);
            }
        }
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    04/93
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   MacroConfigurationAdmin::RemoveAllMacrosAtLevel (ConfigurationVariableLevel level)
    {
    if (!IsValidCfgVarLevelForNewDefinition (level))
        return BSIERROR;

    for (T_MacroSet::iterator thisIter = m_macroSet.begin(); thisIter != m_macroSet.end(); )
        {
        MacroEntryCP    macroEntry = *thisIter;
        bool            stillHasTranslation = false;
        bool            nowHasDifferentTranslation = false;
        if (macroEntry->RemoveAtLevel (stillHasTranslation, nowHasDifferentTranslation, level, true, *this))
            {
            // if we're going to remove, get the next element before we do that.
            if (!stillHasTranslation)
                {
                // if there are no monitors and no dependents with monitors, remove it from the set. Otherwise must keep it.
                if (!macroEntry->InformOfUndefine (true, macroEntry->m_macroName.c_str()))
                    {
#if defined (DebugRemove)
                    VerifyNotDependentOnAnyMacro (macroEntry);
#endif
                    thisIter = m_macroSet.erase (thisIter);
                    delete macroEntry;
                    continue;
                    }
                }
            else if (nowHasDifferentTranslation)
                {
                // still defined, but possible that the value has changed.
                macroEntry->InformOfChangedValue (true, macroEntry->m_macroName.c_str());
                macroEntry->AddToRootsAsDependent (*this);
                }
            }
        ++thisIter;
        }

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
void            MacroConfigurationAdmin::EnsureSysEnvDefinition (WCharCP macroName, ConfigurationVariableLevel level, IMacroDebugOutput* debugOutput, int debugLevel)
    {
    // this is called from MacroFileProcessor::NewMacro to ensure that there is a definition in the macroMap
    //  when the macro is defined only in the system environment.
    if (nullptr != GetMacroDefinition (macroName))
        return;     // already defined at some level.

    MacroExpandOptions options (level);
    options.SetImmediate (false);
    WString expansion;
    ExpandMacro (expansion, macroName, options);
    DefineMacroWithDebugging (macroName, expansion.c_str(), ConfigurationVariableLevel::SysEnv, 0, debugOutput, debugLevel);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    05/93
+---------------+---------------+---------------+---------------+---------------+------*/
void    MacroConfigurationAdmin::RemoveAllMacros ()
    {
    m_macroSet.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    05/93
+---------------+---------------+---------------+---------------+---------------+------*/
bool      MacroConfigurationAdmin::IsMacroLocked (WCharCP macroName)
    {
    MacroEntryCP macroEntry;
    if (nullptr == (macroEntry = GetMacroDefinition (macroName)))
        return false;

    return macroEntry->IsLocked();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    05/93
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   MacroConfigurationAdmin::LockMacro (WCharCP macroName)
    {
    // If already a macro, just set the lock flag
    MacroEntryCP macroEntry;
    if (nullptr != (macroEntry = GetMacroDefinition (macroName)))
        {
        macroEntry->SetLocked (true);
        return BSISUCCESS;
        }

    WString     tmpStr;
    if (BSISUCCESS == util_getSysEnv (&tmpStr, macroName))
        {
        // Found in environment - add as a macro */
        MacroEntry *macroEntry = new MacroEntry (macroName);
        macroEntry->SetTranslation (tmpStr.c_str(), ConfigurationVariableLevel::SysEnv);
        macroEntry->SetLocked (true);
        m_macroSet.insert (macroEntry);
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
    MacroEntryCP macroEntry;
    if (nullptr == (macroEntry = GetMacroDefinition (macroName)))
        return BSIERROR;

    macroEntry->SetLocked (false);
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/14
+---------------+---------------+---------------+---------------+---------------+------*/
void            MacroConfigurationAdmin::ResumeDependencies()
    {
    if (!m_dependenciesSuspended)
        return;

    m_dependenciesSuspended = false;

    // AddToRootsAsDependent() function can add new macros to the configuration, thus we can't
    // just iterate over m_macroSet (because the iterator may become invalid if the tree
    // rebalances) - this is what we need this vector copy for.
    std::vector<MacroEntryCP> macrosToProcess (m_macroSet.begin(), m_macroSet.end());

    for (MacroEntryCP dependentMacro : macrosToProcess)
        {
        dependentMacro->AddToRootsAsDependent (*this);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
void            MacroConfigurationAdmin::SuspendDependencies()
    {
    if (m_dependenciesSuspended)
        return;

    m_dependenciesSuspended = true;
    for (MacroEntryCP dependentMacro : m_macroSet)
        {
        dependentMacro->DiscardAllDependents ();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   MacroConfigurationAdmin::GetDependentMacros (T_WStringVectorR dependents, WCharCP macroName)
    {
    if (!m_dependenciesSuspended)
        ResumeDependencies();

    dependents.clear();

    MacroEntryCP    thisMacro;
    if (nullptr == (thisMacro = GetMacroDefinition (macroName)))
        return BSIERROR;

    if (nullptr == thisMacro->m_dependents)
        return BSISUCCESS;

    for (MacroEntryCP dependentMacro : *thisMacro->m_dependents)
        {
        dependents.push_back (dependentMacro->m_macroName);
        }

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/15
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   MacroConfigurationAdmin::GetVariable (WStringR cfgValue, WCharCP cfgVarName, ConfigurationVariableLevel level)
    {
    return _GetConfigVariable (cfgValue, cfgVarName, level);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/12
+---------------+---------------+---------------+---------------+---------------+------*/
void        MacroConfigurationAdmin::PrintAllMacros (IMacroDebugOutput& debugOutput, int debugLevel)
    {
    // First print the configuration variables with a leading "_"
    for (MacroEntryCP macroEntry : m_macroSet)
        {
        WCharCP                 macroName  = macroEntry->m_macroName.c_str();

        if ('_' != macroName[0])
            continue;

        if (debugLevel > 4)
            macroEntry->PrintLevelCfgVarSummaryLine (debugOutput, macroName, *this);
        else
            macroEntry->PrintCfgVarSummaryLine (debugOutput, macroName, *this);
        }

    debugOutput.ShowDebugMessage (0, L"\n");

    // Now print the rest.
    for (MacroEntryCP macroEntry : m_macroSet)
        {
        WCharCP                 macroName  = macroEntry->m_macroName.c_str();

        if ('_' == macroName[0])
            continue;

        if (debugLevel > 4)
            macroEntry->PrintLevelCfgVarSummaryLine (debugOutput, macroName, *this);
        else
            macroEntry->PrintCfgVarSummaryLine (debugOutput, macroName, *this);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/14
+---------------+---------------+---------------+---------------+---------------+------*/
void        MacroConfigurationAdmin::_OnHostTermination (bool isProcessShutdown)
    {
    delete this;
    }


/*---------------------------------------------------------------------------------**//**
* Generates a temporary filename
* @bsimethod                                                    BernMcCarty     06/05
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileNameCR MacroConfigurationAdmin::_GetLocalTempDirectoryBaseName ()
    {
    static  bool        s_checked;
    static  BeFileName  s_tempPath;

    if (!s_checked)
        {
        WString     expansion;
        _GetConfigVariable (expansion, L"_PRODUCT_LocalUserTempPath", ConfigurationVariableLevel::User);
        if (!expansion.empty())
            s_tempPath.SetName (expansion.c_str());
        else
            BeFileName::BeGetTempPath (s_tempPath);

        s_checked = true;
        }

    return s_tempPath;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    kab             07/90
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus MacroConfigurationAdmin::_UndefineConfigVariable (WCharCP macroName)
    {
    if ( (nullptr == macroName) || (0 == *macroName) )
        return BSIERROR;

    // make a static MacroEntry so we don't have to keep constructing and destroying - all we use in find is the macroName.
    static MacroEntry*  s_tmp;
    if (NULL == s_tmp)
        s_tmp = new MacroEntry (macroName);
    else
        {
        s_tmp->m_macroName.assign (macroName);
        s_tmp->m_macroName.ToUpper();
        }

    T_MacroSet::iterator    iterator;
    if (m_macroSet.end() != (iterator = m_macroSet.find (s_tmp)))
        {
        MacroEntryCP    macroEntry = *iterator;

        // we are no longer dependent on anything, since we're undefined.
        macroEntry->RemoveFromRootsAsDependent (*this);

        // inform all the listeners for each of its dependents that it is being undefined.
        // If there are no monitors for this macroEntry or any of its dependents, we can remove it. Otherwise we have to keep it as undefined so we don't lose the monitors.
        if (!macroEntry->InformOfUndefine (true, macroEntry->m_macroName.c_str()))
            {
            m_macroSet.erase (iterator);
            delete macroEntry;
            }
        else
            macroEntry->RemoveAllTranslations();

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

    WCharCP         translation;
    WString         tmpStr;
    MacroEntryCP    macroEntry;
    if (nullptr == (translation = GetMacroEntryAndTranslation (cfgVarName, macroEntry, tmpStr, level, false)))
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
    WString         tmpStr;
    MacroEntryCP    macroEntry;
    return (nullptr != GetMacroEntryAndTranslation (cfgVarName, macroEntry, tmpStr, level, false));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   12/11
+---------------+---------------+---------------+---------------+---------------+------*/
bool          MacroConfigurationAdmin::_IsConfigVariableDefinedAndTrue (WCharCP cfgVarName, ConfigurationVariableLevel level)
    {
    WCharCP         translation;
    WString         tmpStr;
    MacroEntryCP    macroEntry;
    if (nullptr == (translation = GetMacroEntryAndTranslation (cfgVarName, macroEntry, tmpStr, level, false)))
        return false;

    // try to avoid the expansion part of it.
    if (0 == wcscmp (L"1", translation))
        return true;

    if (0 == BeStringUtilities::Wcsicmp (L"true", translation))
        return true;

    if (0 == BeStringUtilities::Wcsicmp (L"on", translation))
        return true;

    WString expansion;
    if ( ContainsExpression (translation) && (BSISUCCESS == ExpandMacro (expansion, translation, ExpandOptions(level))) )
        {
        if (0 == wcscmp (L"1", expansion.c_str()))
            return true;

        if (0 == BeStringUtilities::Wcsicmp (L"on", expansion.c_str()))
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
    if (nullptr == cfgVarName || (0 == *cfgVarName))
        return BSIERROR;

    if (!IsValidCfgVarLevelForNewDefinition (level))
        return  BSIERROR;

    MacroEntryCP newEntry;
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
BentleyStatus   MacroConfigurationAdmin::_IterateThroughVariables (IConfigVariableIteratorDelegate *delegate)
    {
    if (nullptr == delegate)
        return BSIERROR;

    BentleyStatus   status = BSISUCCESS;

    for (MacroEntryCP macroEntry : m_macroSet)
        {
        ConfigurationVariableLevel  level = macroEntry->GetLevelOfDefinition();
        if (INVALID_MACRO_LEVEL == level)
            {
            status = BSIERROR;      // Need more than this
            continue;
            }

        WCharCP     value = macroEntry->GetTranslationFromLevel (level);
        if (nullptr == value)
            {
            status = BSIERROR;      // Need more than this
            continue;
            }

        bool    locked = macroEntry->IsLocked();

        delegate->EachConfigVariable (macroEntry->m_macroName.c_str(), value, level, locked);
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   MacroConfigurationAdmin::_MonitorVariable (WCharCP macroName, IVariableMonitorR monitor)
    {
    WString         tmpString;
    MacroEntryCP    macroEntry;
    GetMacroEntryAndTranslation (macroName, macroEntry, tmpString, ConfigurationVariableLevel::User, true);

    if (nullptr != macroEntry)
        macroEntry->AddMonitor (&monitor);
    else
        return BSIERROR;    // This happens only if macroName is illegal.

    monitor._VariableChanged (macroName);

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/14
+---------------+---------------+---------------+---------------+---------------+------*/
void            MacroConfigurationAdmin::_RemoveMonitor (WCharCP macroName, IVariableMonitorR monitor)
    {
    MacroEntryCP    macroEntry;
    if (nullptr != (macroEntry = GetMacroDefinition (macroName)))
        macroEntry->RemoveMonitor (monitor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
void            MacroConfigurationAdmin::_OnUnloadMdlDescr (MdlDescP mdlDesc)
    {
    // must search through all the configuration variables and all their monitors to remove those that are getting unloaded.
    for (MacroEntryCP macroEntry : m_macroSet)
        {
        if (nullptr == macroEntry->m_monitors)
            continue;
        for (T_VariableMonitors::iterator iterator = macroEntry->m_monitors->begin(); iterator != macroEntry->m_monitors->end(); )
            {
            VariableMonitor& monitor = *iterator;
            if (monitor.m_mdlDesc != mdlDesc)
                iterator++;
            else
                {
#if defined (NEEDSWORK_DESKTOP_PLATFORM)
                SaveAndSwitchMdlDesc _v_v_v ((*iterator).m_mdlDesc);
#endif
                (*iterator).m_monitor->_MonitorStopped (macroEntry->m_macroName.c_str());
                iterator = macroEntry->m_monitors->erase (iterator);
                }

            }
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/15
+---------------+---------------+---------------+---------------+---------------+------*/
void            MacroConfigurationAdmin::RenameMacros (T_ReplacementMap& replacementMap)
    {
    MacroEntry*         searchEntry = new MacroEntry (L"temp");

    for (T_ReplacementMap::iterator it = replacementMap.begin(); it != replacementMap.end(); ++it)
        {
        bpair <WString, WString> thisRemapEntry = *it;

        // get a copy, we have to extract the macroName from an entry like (USTN_USERDESCR)
        if (!thisRemapEntry.first.StartsWith (L"(") || !thisRemapEntry.first.EndsWith (L")"))
            continue;

        // strip beginning ( and ending ).
        WString thisMacroName = thisRemapEntry.first.substr (1, thisRemapEntry.first.size()-2);

        // avoid having to construct a new MacroEntry every time - all we are going to use is the name.
        searchEntry->m_macroName.assign (thisMacroName.c_str());

        T_MacroSet::iterator  foundEntry;
        if (m_macroSet.end() == (foundEntry = m_macroSet.find (searchEntry)))
            continue;

        // the replacement should have the same form - enclosed in parentheses.
        if (!thisRemapEntry.second.StartsWith (L"(") || !thisRemapEntry.second.EndsWith (L")"))
            {
            BeAssert (false);
            continue;
            }
        WString thisReplacement = thisRemapEntry.second.substr (1, thisRemapEntry.second.size()-2);
        MacroEntry* foundMacroEntry = *foundEntry;

        // we found the entry. We have to remove it and put its replacement in place.
        m_macroSet.erase (foundEntry);

        foundMacroEntry->m_macroName.assign (thisReplacement.c_str());
        bpair <T_MacroSet::const_iterator, bool> inserted = m_macroSet.insert (foundMacroEntry);
        }

    delete searchEntry;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/15
+---------------+---------------+---------------+---------------+---------------+------*/
MacroEntry*     MacroConfigurationAdmin::GetMacroEntry (WCharCP cfgVarName)
    {
    return const_cast<MacroEntry*> (GetMacroDefinition (cfgVarName));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/15
+---------------+---------------+---------------+---------------+---------------+------*/
void            MacroConfigurationAdmin::GetEntryVariableName (WStringR cfgVarName, MacroEntry const* macroEntry)
    {
    cfgVarName.assign (macroEntry->m_macroName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/15
+---------------+---------------+---------------+---------------+---------------+------*/
void            MacroConfigurationAdmin::GetEntryTranslationAtLevel (WStringR translation, MacroEntry const* macroEntry, ConfigurationVariableLevel level)
    {
    macroEntry->GetTranslationAtSpecificLevel (&translation, level);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/15
+---------------+---------------+---------------+---------------+---------------+------*/
void            MacroConfigurationAdmin::RemoveEntryReferencesTo (T_WStringVectorCR cfgVarList, MacroEntry* macroEntry, ConfigurationVariableLevel level)
    {
    WString         translation;
    BentleyStatus   status = macroEntry->GetTranslationAtSpecificLevel (&translation, level);
    if ( (BSISUCCESS != status) || translation.empty())
        return;

    // consider each part that is separated by semicolons separately (for path-type configuration variables).
    T_WStringVector subParts;
    BeStringUtilities::Split (translation.c_str(), L";", nullptr, subParts);

    bool        anyChange = false;
    for (T_WStringVector::iterator thisPart = subParts.begin(); thisPart != subParts.end(); )
        {
        WStringR    thisString = *thisPart;
        // we have to look through the translation string and see if there are any occurrences of any of the variables in cfgVarList.
        bool        wasErased  = false;
        for (WStringCR thisCfgVar : cfgVarList)
            {
            // if we find any use of thisCfgVar, eliminate this subpart.
            if (thisString .ContainsI (thisCfgVar))
                {
                // delete this subpart and break out of the loop.
                thisPart  = subParts.erase (thisPart);
                wasErased = true;
                anyChange = true;
                break;
                }
            }

        // if we didn't erase the subpart, increment the iterator.
        if (!wasErased)
            ++thisPart;
        }

    if (anyChange)
        {
        WString newTranslation;
        bool    first = true;
        for (WString thisPart : subParts)
            {
            if (!first)
                newTranslation.append (1, L';');
            newTranslation.append (thisPart);
            }

        macroEntry->SetTranslation (newTranslation.c_str(), level);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/15
+---------------+---------------+---------------+---------------+---------------+------*/
void            MacroConfigurationAdmin::ReplaceEntryReferences (T_ReplacementMap& replacementMap, MacroEntry* macroEntry, ConfigurationVariableLevel level)
    {
    WString         translation;
    BentleyStatus   status = macroEntry->GetTranslationAtSpecificLevel (&translation, level);
    if ( (BSISUCCESS != status) || translation.empty())
        return;

    if (ReplaceFromMap (translation, replacementMap))
        {
        bool savedLock = macroEntry->m_locked;
        macroEntry->m_locked = false;
        macroEntry->SetTranslation (translation.c_str(), level);
        macroEntry->m_locked = savedLock;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/15
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   MacroConfigurationAdmin::GetAppropriateAssignmentStatement (WStringR assignmentStatement, MacroEntryCP macroEntry, ConfigurationVariableLevel level, T_ReplacementMap& replacementMap)
    {
    // if we don't currently have an assignment at this level, can't do it.
    return macroEntry->GetAppropriateAssignmentStatement (assignmentStatement, level, replacementMap);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/15
+---------------+---------------+---------------+---------------+---------------+------*/
MacroConfigurationAdmin::const_iterator::const_iterator (T_MacroSet::const_iterator  inputIterator)
    {
    m_setIterator = inputIterator;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/15
+---------------+---------------+---------------+---------------+---------------+------*/
MacroConfigurationAdmin::const_iterator MacroConfigurationAdmin::begin () const
    {
    return const_iterator (m_macroSet.begin());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/15
+---------------+---------------+---------------+---------------+---------------+------*/
MacroConfigurationAdmin::const_iterator MacroConfigurationAdmin::end() const
    {
    return const_iterator (m_macroSet.end());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/15
+---------------+---------------+---------------+---------------+---------------+------*/
MacroConfigurationAdmin::const_iterator& MacroConfigurationAdmin::const_iterator::operator ++ ()
    {
    ++m_setIterator;
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/15
+---------------+---------------+---------------+---------------+---------------+------*/
MacroEntry* MacroConfigurationAdmin::const_iterator::operator * () const
    {
    return *m_setIterator;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool    MacroConfigurationAdmin::const_iterator::operator != (const_iterator const &rhs) const
    {
    return m_setIterator != rhs.m_setIterator;
    }


#if defined (BENTLEY_WIN32)

// This section is for reading .cfg files from old version. Old versions are only on Windows, so no need for it outside that environment.

// These classes are used by applications that use DgnPlatform only, but want to
//  process configuration files like PowerPlatform applications do. For example, the
//  converter from DgnV8 to DgnDb uses these classes. See further comments below.
#if defined (DEBUG_DGNPLATFORM_MACRO)
struct      PrintDebugOutput : IMacroDebugOutput
{
PrintDebugOutput () {}

virtual void        ShowDebugMessage (int indent, WCharCP format, ...) override
    {
    WString     message;
    va_list     ap;

    va_start (ap, format);
    message.VSprintf (format, ap);
    va_end (ap);

    int numSpaces = indent*2;
    for (int iSpace = 0; iSpace < numSpaces; iSpace++)
        wprintf (L" ");

    wprintf (L"%ls", message.c_str());
    }
};
#endif


/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                     Barry.Bentley   05/2012
* The purpose of this class is to read the configuration files from the CONNECT versions
* without using any of the PowerPlatform dlls.
+---------------+---------------+---------------+---------------+---------------+------*/
class           CONNECTMacroFileProcessor : public MacroFileProcessor
{
WString             m_workSpaceName;
WString             m_workSetName;
WString             m_configurationRootDir; // the location of the install/Configuration directory. 
WString             m_msConfigFileName;     // the full name of the msconfig file
WString             m_connectInstallDir;    // the directory containing the MicroStation exe.
WString             m_fallbackConfigDir;    // the location where we have stored the CONNECT system .cfg files in the delivery of the client of this macro file reader, in case there isn't a MicroStation installation on the machine.
T_WStringVector&    m_assignmentArgs;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _OnProcessError (WCharCP msg) override
    {
    // for now, we're just fwprintfing this message, but perhaps it should be put in the message center or something.
    fwprintf (stderr, L"%ls", msg);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/15
+---------------+---------------+---------------+---------------+---------------+------*/
virtual ConfigurationVariableLevel _ReinterpretConfigurationVariableLevel (ConfigurationVariableLevel inputLevel) override
    {
    switch (inputLevel)
        {
        // these are the same in CONNECT as they were in V8i and previous versions:
        case ConfigurationVariableLevel::Predefined:        // = -2,
        case ConfigurationVariableLevel::SysEnv:            // = -1,
        case ConfigurationVariableLevel::System:            // = 0,
        case ConfigurationVariableLevel::Application:       // = 1,
        case ConfigurationVariableLevel::Organization:      // = 2, There was no Organization level in V8i. 2 mean Site
            return inputLevel;

        case ConfigurationVariableLevel::WorkSpace:         // = 3, in V8i, 3 meant Project which is now WorkSet.
            return ConfigurationVariableLevel::WorkSet;

        case ConfigurationVariableLevel::WorkSet:           // = 4, in V8i, 4 meant User, so keep that.
            return ConfigurationVariableLevel::User;

        // we should never see these coming from a V8i configuration file.
        case ConfigurationVariableLevel::Role:              // = 5, There was no Role in V8i, should never encounter anything > 4.
        case ConfigurationVariableLevel::User:              // = 6, Should never encounter
        default:
            BeAssert (false);
            return inputLevel;
        }
    }

public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
CONNECTMacroFileProcessor (MacroConfigurationAdmin& mca, WCharCP workSpaceName, WCharCP workSetName, WCharCP configurationRootDir, WCharCP installDir, WCharCP msConfigFileName, WCharCP fallbackConfigDir, T_WStringVector& assignmentArgs) : MacroFileProcessor (mca, nullptr, nullptr), m_assignmentArgs (assignmentArgs)
    {
    if (nullptr != workSpaceName)
        m_workSpaceName.assign (workSpaceName);
    if (nullptr != workSetName)
        m_workSetName.assign (workSetName);

    if (nullptr != configurationRootDir)
        m_configurationRootDir.assign (configurationRootDir);
    if (!m_configurationRootDir.empty())
        BeFileName::AppendSeparator (m_configurationRootDir);

    if (nullptr != msConfigFileName)
        m_msConfigFileName.assign (msConfigFileName);
    
    if (nullptr != installDir)
        m_connectInstallDir.assign (installDir);
    if (!m_connectInstallDir.empty())
        BeFileName::AppendSeparator (m_connectInstallDir);


    m_fallbackConfigDir.assign (fallbackConfigDir);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       LoadCONNECTMacros ()
    {
    // the first thing we have to do is find the MS_LOCAL or MS_CONFIG
    BeFileName      foundName;

    DefineBuiltinMacros();
    DefineBuiltinRuntimeMacros ();
    SetUserCfg ();

    // get the root directory of the (possible) MicroStation install. If it exists, then we use its config/mslocal.cfg.
    // (the only use of _ROOTDIR in the config files is to set MSDIR).
    BeFileName      configDir;
    if (!m_connectInstallDir.empty())
        {
        if (BeFileNameStatus::Success == BeFileName::CheckAccess (m_connectInstallDir.c_str(), BeFileNameAccess::Read))
            {
            m_macroCfgAdmin.DefineBuiltinMacro(L"_ROOTDIR", m_connectInstallDir.c_str());

            // look for the msconfig.cfg file in the "config" directory off of the directory in which MicroStation resides
            configDir.assign (m_connectInstallDir);
            configDir.AppendToPath (L"config");
            configDir.AppendSeparator();
            }
        }

    if (configDir.empty())
        {
        // The user hasn't specified a V8i install directory or a V8i configuration directory. We need to use the system configuration files
        // that we ship with the application. The caller supplies the directory, less the "config" portion of it.
        m_macroCfgAdmin.DefineBuiltinMacro(L"_ROOTDIR", m_fallbackConfigDir.c_str());
        configDir.assign (m_fallbackConfigDir.c_str());
        configDir.AppendToPath (L"config");
        configDir.AppendSeparator();
        }

    if (SUCCESS != util_findFile (nullptr, &foundName, L"mslocal.cfg", configDir.c_str(), L"mslocal.cfg", UF_NO_CUR_DIR))
        {
        if (SUCCESS != util_findFile (nullptr, &foundName, L"msconfig.cfg", configDir.c_str(), L"msconfig.cfg", UF_NO_CUR_DIR))
            {
            wprintf(L"\nWarning: Could not open configuration file [%ls]\n\n", foundName.c_str());
            return ERROR;
            }
        }

    // we need to set and lock the username, projectname, etc.
    const MacroEntry* newEntry;
    if (!m_workSpaceName.empty())
        {
        m_macroCfgAdmin.DefineMacro (newEntry, L"_USTN_WORKSPACENAME", m_workSpaceName.c_str(), ConfigurationVariableLevel::System);
        m_macroCfgAdmin.LockMacro(L"_USTN_WORKSPACENAME");
        }
    if (!m_workSetName.empty())
        {
        m_macroCfgAdmin.DefineMacro (newEntry, L"_USTN_WORKSETNAME", m_workSetName.c_str(), ConfigurationVariableLevel::System);
        m_macroCfgAdmin.LockMacro(L"_USTN_WORKSETNAME");
        }
    if (!m_configurationRootDir.empty())
        {
        BeFileName::AppendSeparator (m_configurationRootDir);
        m_macroCfgAdmin.DefineMacro (newEntry, L"_USTN_CONFIGURATION", m_configurationRootDir.c_str(), ConfigurationVariableLevel::System);
        m_macroCfgAdmin.LockMacro(L"_USTN_CONFIGURATION");
        }
    for (WStringR thisAssignment : m_assignmentArgs)
        {
        size_t equalsPos;
        if (WString::npos != (equalsPos = thisAssignment.find ('=')))
            {
            if ( (equalsPos > 0) && (equalsPos < thisAssignment.length()-1) )
                {
                WString definition = thisAssignment.substr (equalsPos+1);
                WString macroName  = thisAssignment.substr (0, equalsPos);
                m_macroCfgAdmin.DefineMacro (newEntry, macroName.c_str(), definition.c_str(), ConfigurationVariableLevel::System);
                }
            }
        else
            {
            // no value was specified, define macro to have a null value
            m_macroCfgAdmin.DefineMacro (newEntry, thisAssignment.c_str(), nullptr, ConfigurationVariableLevel::System);
            }
        }

#if defined (DEBUG_DGNPLATFORM_MACRO)
    PrintDebugOutput    debugOutput;
    SetDebugOutput (&debugOutput, 4);
#endif

    return ProcessTopLevelFile (foundName, ConfigurationVariableLevel::System, nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void    DefineBuiltinMacros ()
    {
    m_macroCfgAdmin.DefineBuiltinMacro(L"_VERSION_10_0", NULL);
    m_macroCfgAdmin.DefineBuiltinMacro(L"_winNT", NULL);
    m_macroCfgAdmin.DefineBuiltinMacro(L"_intelNT", NULL);
    m_macroCfgAdmin.DefineBuiltinMacro(L"_PLATFORMNAME", L"intelnt");
    m_macroCfgAdmin.DefineBuiltinMacro (L"_ENGINENAME", L"MicroStation");
    m_macroCfgAdmin.DefineBuiltinMacro(L"MICROSTATION", L"1");

    // Define macro for current working directory when ustn was started.
    WString workDir;
    BeFileName::GetCwd (workDir);
    BeFileName::AppendSeparator (workDir);
    m_macroCfgAdmin.DefineBuiltinMacro(L"_WORKDIR", workDir.c_str());
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
static bool CompareDirectories (WStringR lhs, WStringR rhs)
    {
    T_WStringVector       leftTokens;
    BeStringUtilities::Split (lhs.c_str(), L"._", leftTokens);

    T_WStringVector       rightTokens;
    BeStringUtilities::Split (rhs.c_str(), L"._", nullptr, rightTokens);

    // for now, we're not looking at any like 10.0.0_1.
    if (leftTokens.size() != 3)
        return true;
    if (rightTokens.size() != 3)
        return false;

    for (int iString=0; iString < 3; iString++)
        {
        int     leftInt = BeStringUtilities::Wtoi (leftTokens[iString].c_str());
        int     rightInt = BeStringUtilities::Wtoi (rightTokens[iString].c_str());

        // only looking for 10 and greater.
        if ( (iString == 0) && (leftInt < 10) )
            return true;
        if ( (iString == 0) && (rightInt < 10) )
            return false;

        if (leftInt != rightInt)
            return (leftInt > rightInt);
        }
    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
void    GetCONNECTSubDirectory (WStringR subDirectory)
    {
    subDirectory.assign (L"Bentley\\MicroStation\\");
    // This won't matter for non-Windows platforms, since it applies to previous version .cfg files.

    // In CONNECT, the directories are named from version release/major/minor (10.0.0, etc.)
    // There's a way to install multiple builds of a particular xx.xx.xx version that ends up
    // with names like 10.0.0_1, but we pay no attention to that here. We look for the most recent
    // build number in the (%LOCALAPPDATA%)Bentley\MicroStation directory. If we don't find anything,
    // (which is possible on a machine where MicroStation has never been installed) we just use 10.0.0. 

    WCharP      localAppData;
    SHGetKnownFolderPath (FOLDERID_LocalAppData, 0, nullptr, &localAppData);
    WString     ourLocalAppData (localAppData);
    BeFileName::AppendToPath (ourLocalAppData, L"Bentley\\MicroStation\\*");
    CoTaskMemFree (localAppData);

    BeFileListIterator  fileIterator (ourLocalAppData.c_str(), false);
    BeFileName          fullFileName;
    T_WStringVector     possibleDirs;
    while (BSISUCCESS == fileIterator.GetNextFileName (fullFileName))
        {
        // every file starts with the directory.
        BeFileName fileName;
        fileName.assign (fullFileName.substr(ourLocalAppData.length()-1, WString::npos));

        // we only want directories that start with a number greater than or equal to 10.
        if (!BeFileName::IsDirectory(fullFileName))
            continue;

        size_t firstDot;
        if (WString::npos == (firstDot = fileName.find (L'.')))
            continue;

        // must start with all numbers.
        size_t firstNonNumber = fileName.find_first_not_of (L"0123456789");
        if (firstNonNumber < (firstDot-1))
            continue;

        WString firstNumString = fileName.substr (0, firstDot);
        int     firstNum;
        swscanf (firstNumString.c_str(), L"%d", &firstNum);
        if (firstNum < 10)
            continue;

        // don't want the 99 bunch unless there's nothing else.
        if ( (firstNum > 20) && !possibleDirs.empty() )
            continue;

        possibleDirs.push_back (fileName);
        }

    // couldn't find subdirectory, go with default.
    if (possibleDirs.size() < 1)
        {
        subDirectory.append (L"10.0.0");
        return;
        }
        
    std::sort (possibleDirs.begin(), possibleDirs.end(), CompareDirectories);
    subDirectory.append (possibleDirs[0].c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void    DefineBuiltinRuntimeMacros ()
    {
    // This won't matter for non-Windows platforms, since it applies to previous version .cfg files.

    WString bentleyProductInstall;
    GetCONNECTSubDirectory (bentleyProductInstall);
    BeFileName::AppendSeparator (bentleyProductInstall);

    // _USTN_LocalUserAppDataPath
    WCharP      localAppData;
    SHGetKnownFolderPath (FOLDERID_LocalAppData, 0, nullptr, &localAppData);
    WString     ourLocalAppData (localAppData);
    BeFileName::AppendSeparator (ourLocalAppData);
    ourLocalAppData.append (bentleyProductInstall);
    m_macroCfgAdmin.DefineBuiltinMacro (L"_USTN_LocalUserAppDataPath", ourLocalAppData.c_str());
    CoTaskMemFree (localAppData);

    // _USTN_LocalUserTempPath
    BeFileName  tempPath;
    BeFileName::BeGetTempPath (tempPath);
    tempPath.AppendSeparator ();
    tempPath.append (bentleyProductInstall);
    tempPath.AppendSeparator ();
    m_macroCfgAdmin.DefineBuiltinMacro (L"_USTN_LocalUserTempPath", tempPath.c_str());

    // _USTN_UserAppDataPath
    WCharP      roamingAppData;
    SHGetKnownFolderPath (FOLDERID_RoamingAppData, 0, nullptr, &roamingAppData);
    WString     ourRoamingAppData (roamingAppData);
    BeFileName::AppendSeparator (ourRoamingAppData);
    ourRoamingAppData.append (L"Bentley\\MicroStation\\");  // doesn't include version.
    m_macroCfgAdmin.DefineBuiltinMacro (L"_USTN_UserAppDataPath", ourRoamingAppData.c_str());
    CoTaskMemFree (roamingAppData);

    // _USTN_CommonAppDataPath
    WCharP      commonAppData;
    SHGetKnownFolderPath (FOLDERID_ProgramData, 0, nullptr, &commonAppData);
    WString     ourCommonAppData (commonAppData);
    BeFileName::AppendSeparator (ourCommonAppData);
    ourCommonAppData.append (bentleyProductInstall);
    m_macroCfgAdmin.DefineBuiltinMacro (L"_USTN_CommonAppDataPath", ourCommonAppData.c_str());
    CoTaskMemFree (commonAppData);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void    SetUserCfg ()
    {
    // _USTN_USERCFG must be defined and the file must exist for msconfig.cfg to work.
    WString localAppData;
    m_macroCfgAdmin.GetVariable (localAppData, L"_USTN_LocalUserAppDataPath");
    BeAssert (!localAppData.empty());

    WString prefsFolder (localAppData);
    prefsFolder.append(L"prefs\\");

    if (_waccess (prefsFolder.c_str(), 0))
        {
        BeFileNameStatus stat = BeFileName::CreateNewDirectory (prefsFolder.c_str());
        BeAssert (BeFileNameStatus::Success == stat);
        }

    WString ucfPath (prefsFolder);
    ucfPath.append (L"Personal.ucf");

    // make sure the file exists.
    if (_waccess (ucfPath.c_str(), 0))
        {
        BeFileStatus  status;
        BeTextFilePtr textFile = BeTextFile::Open(status, ucfPath.c_str(), TextFileOpenType::Write, TextFileOptions::None);
        textFile->PutLine(L"_USTN_USERDESCR = User", true);
        }

    m_macroCfgAdmin.DefineBuiltinMacro (L"_USTN_USERCFG", ucfPath.c_str());
    }

};



#if defined (NEEDSWORK_DESKTOP_PLATFORM_READV8ICONFIG)
// The only thing missing is the Cryptographer stuff, which is in the Resource Manager, but we don't yet have a dependency on that.
// Wait and see whether reading V8i Configuration Files is needed before adding that dependency.

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                     Barry.Bentley   05/2012
* The purpose of this class is to read the configuration files from V8i and earlier versions.
* They can then be converted to CONNECT edition WorkSpaces.
+---------------+---------------+---------------+---------------+---------------+------*/
class           V8iMacroFileProcessor : public MacroFileProcessor
{
WString             m_userName;
WString             m_projectName;
WString             m_workspaceRootDir;     // the location of the install/WorkSpace directory. 
WString             m_msConfigFileName;     // the full name of the msconfig file
WString             m_v8iInstallDir;        // the location of the directory containing the V8i MicroStation.exe.
WString             m_fallbackConfigDir;    // the location where we have stored the v8i system .cfg files in the delivery of the client of this macro file reader, in case there isn't a MicroStation installation on the machine.
T_WStringVector&    m_assignmentArgs;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _OnProcessError (WCharCP msg) override
    {
    // for now, we're just fwprintfing this message, but perhaps it should be put in the message center or something.
    fwprintf (stderr, L"%ls", msg);
    }

public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
V8iMacroFileProcessor (MacroConfigurationAdmin& mca, WCharCP userName, WCharCP projectName, WCharCP configRootDir, WCharCP v8iInstallDir, WCharCP msConfigFileName, WCharCP fallbackConfigDir, T_WStringVector& assignmentArgs) : MacroFileProcessor (mca, nullptr, nullptr), m_assignmentArgs (assignmentArgs)
    {
    if (nullptr != userName)
        m_userName.assign (userName);
    if (nullptr != projectName)
        m_projectName.assign (projectName);

    if (nullptr != configRootDir)
        m_workspaceRootDir.assign (configRootDir);
    if (!m_workspaceRootDir.empty())
        BeFileName::AppendSeparator (m_workspaceRootDir);

    if (nullptr != msConfigFileName)
        m_msConfigFileName.assign (msConfigFileName);
    
    if (nullptr != v8iInstallDir)
        m_v8iInstallDir.assign (v8iInstallDir);
    if (!m_v8iInstallDir.empty())
        BeFileName::AppendSeparator (m_v8iInstallDir);

    m_fallbackConfigDir.assign (fallbackConfigDir);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       LoadV8iMacros ()
    {
    // the first thing we have to do is find the MS_LOCAL or MS_CONFIG
    BeFileName      foundName;

    DefineBuiltinMacros();
    DefineBuiltinRuntimeMacros (m_v8iInstallDir.c_str());

    // get the root directory of the (possible) MicroStation install. If it exists, then we use its config/mslocal.cfg.
    // (the only use of _ROOTDIR in the config files is to set MSDIR).
    BeFileName      configDir;
    if (!m_v8iInstallDir.empty())
        {
        if (BeFileNameStatus::Success == BeFileName::CheckAccess (m_v8iInstallDir.c_str(), BeFileNameAccess::Read))
            {
            m_macroCfgAdmin.DefineBuiltinMacro(L"_ROOTDIR", m_v8iInstallDir.c_str());

            // look for the msconfig.cfg file in the "config" directory off of the directory in which MicroStation resides
            configDir.assign (m_v8iInstallDir);
            configDir.AppendToPath (L"config");
            configDir.AppendSeparator();
            }
        }

    if (configDir.empty())
        {
        // The user hasn't specified a V8i install directory or a V8i configuration directory. We need to use the system configuration files
        // that we ship with the application. The caller supplies the directory, less the "config" portion of it.
        m_macroCfgAdmin.DefineBuiltinMacro(L"_ROOTDIR", m_fallbackConfigDir.c_str());
        configDir.assign (m_fallbackConfigDir.c_str());
        configDir.AppendToPath (L"config");
        configDir.AppendSeparator();
        }

    if (SUCCESS != util_findFile (nullptr, &foundName, L"mslocal.cfg", configDir.c_str(), L"mslocal.cfg", UF_NO_CUR_DIR))
        {
        if (SUCCESS != util_findFile (nullptr, &foundName, L"msconfig.cfg", configDir.c_str(), L"msconfig.cfg", UF_NO_CUR_DIR))
            {
            wprintf(L"\nWarning: Could not open configuration file [%ls]\n\n", foundName);
            return ERROR;
            }
        }

    // we need to set and lock the username, projectname, etc.
    const MacroEntry* newEntry;
    if (!m_userName.empty())
        {
        m_macroCfgAdmin.DefineMacro (newEntry, L"_USTN_USERNAME", m_userName.c_str(), ConfigurationVariableLevel::System);
        m_macroCfgAdmin.LockMacro(L"_USTN_USERNAME");
        }
    if (!m_projectName.empty())
        {
        m_macroCfgAdmin.DefineMacro (newEntry, L"_USTN_PROJECTNAME", m_projectName.c_str(), ConfigurationVariableLevel::System);
        m_macroCfgAdmin.LockMacro(L"_USTN_PROJECTNAME");
        }
    if (!m_workspaceRootDir.empty())
        {
        BeFileName::AppendSeparator (m_workspaceRootDir);
        m_macroCfgAdmin.DefineMacro (newEntry, L"_USTN_WORKSPACEROOT", m_workspaceRootDir.c_str(), ConfigurationVariableLevel::System);
        m_macroCfgAdmin.LockMacro(L"_USTN_WORKSPACEROOT");
        }
    for (WStringR thisAssignment : m_assignmentArgs)
        {
        size_t equalsPos;
        if (WString::npos != (equalsPos = thisAssignment.find ('=')))
            {
            if ( (equalsPos > 0) && (equalsPos < thisAssignment.length()-1) )
                {
                WString definition = thisAssignment.substr (equalsPos+1);
                WString macroName  = thisAssignment.substr (0, equalsPos);
                m_macroCfgAdmin.DefineMacro (newEntry, macroName.c_str(), definition.c_str(), ConfigurationVariableLevel::System);
                }
            }
        else
            {
            // no value was specified, define macro to have a null value
            m_macroCfgAdmin.DefineMacro (newEntry, thisAssignment.c_str(), nullptr, ConfigurationVariableLevel::System);
            }
        }

#if defined (DEBUG_DGNPLATFORM_MACRO)
    PrintDebugOutput    debugOutput;
    SetDebugOutput (&debugOutput, 4);
#endif

    return ProcessTopLevelFile (foundName, ConfigurationVariableLevel::System, nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void    DefineBuiltinMacros ()
    {
    m_macroCfgAdmin.DefineBuiltinMacro (L"_VERSION_8_11", nullptr);
    m_macroCfgAdmin.DefineBuiltinMacro (L"_ENGINENAME", L"MicroStation");
    m_macroCfgAdmin.DefineBuiltinMacro (L"_winNT", nullptr);
    m_macroCfgAdmin.DefineBuiltinMacro (L"_intelNT", nullptr);
    m_macroCfgAdmin.DefineBuiltinMacro (L"_PLATFORMNAME", L"intelnt");

    // Define macro for current working directory when ustn was started.
    WString workDir;
    BeFileName::GetCwd (workDir);
    BeFileName::AppendSeparator (workDir);
    m_macroCfgAdmin.DefineBuiltinMacro(L"_WORKDIR", workDir.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Chuck.Kirschman 04/05
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt   base64Encode (const byte* inputBuffer, uint32_t inputLen, WCharP outputBuffer, uint32_t outputLen)
    {
    static WChar b64table[65] = L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    int mod = inputLen%3;
    uint32_t reqOutput = (inputLen/3)*4 + (3-mod)%3 + 1;
    if (outputLen < reqOutput)
        return ERROR;

    WChar *pOutput = outputBuffer;
    uint32_t iChar = 0;
    while (iChar < inputLen-mod)
        {
        *pOutput++ = b64table[inputBuffer[iChar++] >> 2];
        *pOutput++ = b64table[((inputBuffer[iChar-1] << 4) | (inputBuffer[iChar] >> 4)) & 0x3f];
        *pOutput++ = b64table[((inputBuffer[iChar] << 2) | (inputBuffer[iChar+1] >> 6)) & 0x3f];
        *pOutput++ = b64table[inputBuffer[iChar+1] &0x3f];
        iChar += 2;
        }

    if (0 != mod)
        {
        *pOutput++ = b64table[inputBuffer[iChar++] >> 2];
        *pOutput++ = b64table[((inputBuffer[iChar-1] << 4) | (inputBuffer[iChar] >> 4)) & 0x3f];

        if (1 == mod)
            {
            *pOutput++ = L'=';
            *pOutput++ = L'=';
            }
        else
            {
            *pOutput++ = b64table[(inputBuffer[iChar]<<2) &0x3f];
            *pOutput++ = L'=';
            }
        }
    *pOutput++ = L'\0';
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
void    GetStupidInstallHashString (WStringR installPathHash, WCharCP installDirectory)
    {
    WChar                   longPathName[1024];

    // get the install directory. If it's not provided, make one up. If it's got the shortName character, get the long name.
    if ( (nullptr == installDirectory) || (0 == *installDirectory))
        installDirectory = L"c:\\Program Files (x86)\\Bentley\\MicroStation V8i (SELECTseries)\\MicroStation\\";
    else if (nullptr != wcschr (installDirectory, '~'))
        {
        GetLongPathNameW (installDirectory, longPathName, _countof(longPathName));
        installDirectory = longPathName;
        }
    BeFileName  ustationDllName(installDirectory);
    ustationDllName.AppendSeparator();
    ustationDllName.append (L"ustation.dll");
    ustationDllName.ToUpper();

    Cryptographer       cryptographer;
    cryptographer.initializeForHashOnly();

    CryptographerHash   hashCalculator;
    hashCalculator.init (cryptographer, DIGITAL_SIGNATURE_ALG_RSA_MD5);
    hashCalculator.hash ((const byte*)ustationDllName.c_str(), 2*ustationDllName.size());
    DsigRawHash         hashedValue;
    hashCalculator.save (hashedValue);
    WChar               encodedHash[48];

    // put it in base64 to make a string out of it, and replace characters that don't work in file names.
    base64Encode (&hashedValue.b[0], 16, encodedHash, _countof (encodedHash));
    installPathHash. assign (encodedHash);
    installPathHash.ReplaceAll (L"+", L"-");
    installPathHash.ReplaceAll (L"/", L"_");
    installPathHash.ReplaceAll (L"=", L"");
    }


// need this for FOLDERID_xxx, etc.
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DefineBuiltinRuntimeMacros (WCharCP installDirectory)
    {
    WString     installPathHash;
    GetStupidInstallHashString (installPathHash, installDirectory);

    WString     bentleyProductInstall (L"Bentley\\MicroStation\\8.11\\");
    WString     bentleyProductInstallWithHash (bentleyProductInstall);
    BeFileName::AppendSeparator (bentleyProductInstallWithHash);
    bentleyProductInstallWithHash.append (installPathHash);
    BeFileName::AppendSeparator (bentleyProductInstallWithHash);

    // _USTN_LocalUserAppDataPath
    WCharP      localAppData;
    SHGetKnownFolderPath (FOLDERID_LocalAppData, 0, nullptr, &localAppData);
    WString     ourLocalAppData (localAppData);
    BeFileName::AppendSeparator (ourLocalAppData);
    ourLocalAppData.append (bentleyProductInstallWithHash);
    m_macroCfgAdmin.DefineBuiltinMacro (L"_USTN_LocalUserAppDataPath", ourLocalAppData.c_str());
    CoTaskMemFree (localAppData);

    
    // _USTN_LocalUserTempPath
    BeFileName  tempPath;
    BeFileName::BeGetTempPath (tempPath);
    tempPath.AppendSeparator ();
    tempPath.append (bentleyProductInstallWithHash);
    tempPath.AppendSeparator ();
    m_macroCfgAdmin.DefineBuiltinMacro (L"_USTN_LocalUserTempPath", tempPath.c_str());

    // _USTN_UserAppDataPath
    WCharP      roamingAppData;
    SHGetKnownFolderPath (FOLDERID_RoamingAppData, 0, nullptr, &roamingAppData);
    WString     ourRoamingAppData (roamingAppData);
    BeFileName::AppendSeparator (ourRoamingAppData);
    ourRoamingAppData.append (bentleyProductInstall);
    m_macroCfgAdmin.DefineBuiltinMacro (L"_USTN_UserAppDataPath", ourRoamingAppData.c_str());
    CoTaskMemFree (roamingAppData);

    // _USTN_CommonAppDataPath
    WCharP      commonAppData;
    SHGetKnownFolderPath (FOLDERID_ProgramData, 0, nullptr, &commonAppData);
    WString     ourCommonAppData (commonAppData);
    BeFileName::AppendSeparator (ourCommonAppData);
    ourCommonAppData.append (bentleyProductInstallWithHash);
    m_macroCfgAdmin.DefineBuiltinMacro (L"_USTN_CommonAppDataPath", ourCommonAppData.c_str());
    CoTaskMemFree (commonAppData);
    }

};
#endif // NEEDSWORK_DESKTOP_PLATFORM_READV8ICONFIG


// The methods below are used by applications that use DgnPlatform only, but want to
//  process configuration files like PowerPlatform applications do. For example, the
//  converter from DgnV8 to DgnDb uses these methods.
// 
// We can read either V8i configuration files or CONNECT configuration files. 
//  If we know where the V8i or CONNECT installation directory is, we can get 
//  everything right. But if you are running on a server where there is no
//  V8i or CONNECT installation, the temporary files that are based on the name
//  and version of the MicroStation product, such as _USTN_LocalUserAppDataPath,
//  _USTN_LocalUserTempPath, _USTN_UserAppDataPath, and _USTN_CommonAppDataPath
//  cannot be correctly set. That usually doesn't matter for the type of applications
//  that use these methods because those cfg variables keep userprefs and temporary
//  files rather than data files. We are usually concerned with finding symbology
//  resources, materials, reference file directories, and other display-oriented
//  configuration variables, and those aren't in the user- or appdata- directories.
//
// When we read V8i configuration files, we convert the V8i configuration levels (system, 
// site, project, user) to the corresponding CONNECT configuration levels, but we do not
// convert any of the names of the configuration variables. We count on V8i and CONNECT
// both using MS_PATTERN or MS_MATERIAL or MS_LINESTYLERSC, etc., and those are defined
// for both V8i and CONNECT.

// These are instance methods. The caller constructs a MacroConfigurationAdmin instance so
//  it can be passed to the constructor of a MacroFileProcessor. The caller subclasses 
//  MacroFileProcessor so it can get control over error handleing.


#if defined (NEEDSWORK_DESKTOP_PLATFORM_READV8ICONFIG)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   MacroConfigurationAdmin::ReadV8iConfigurationFiles (WCharCP userName, WCharCP projectName, WCharCP workspaceRootDir, WCharCP msInstallDir, WCharCP msConfigFileName, WCharCP fallbackConfigDir, T_WStringVector& assignmentArgs)
    {
    V8iMacroFileProcessor    v8iMFP (*this, userName, projectName, workspaceRootDir, msInstallDir, msConfigFileName, fallbackConfigDir, assignmentArgs);
    v8iMFP.LoadV8iMacros();
    return SUCCESS;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   MacroConfigurationAdmin::ReadCONNECTConfigurationFiles (WCharCP workSpaceName, WCharCP workSetName, WCharCP configurationRootDir, WCharCP msInstallDir, WCharCP msConfigFileName, WCharCP fallbackConfigDir, T_WStringVector& assignmentArgs)
    {
    CONNECTMacroFileProcessor    connectMFP (*this, workSpaceName, workSetName, configurationRootDir, msInstallDir, msConfigFileName, fallbackConfigDir, assignmentArgs);
    connectMFP.LoadCONNECTMacros();
    return SUCCESS;
    }

#endif // BENTLEY_WIN32
