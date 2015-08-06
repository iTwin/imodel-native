/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DesktopTools/MacroConfigurationAdmin.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include <DgnPlatform/DgnPlatform.h>
#include "ConfigurationManager.h"

BEGIN_BENTLEY_DGN_NAMESPACE

/*=================================================================================**//**
* An implementation of IConfigurationAdmin that maintains
* a temporary macro database and falls back on the system environment variables and CSIDL.
* @bsiclass                                     Sam.Wilson                      05/2011
+===============+===============+===============+===============+===============+======*/
struct MacroConfigurationAdmin : IConfigurationAdmin
{
    //! Options to control how macros are expanded
    struct  ExpandOptions
        {
        bool                        m_immediate;
        bool                        m_expandOnlyIfFullyDefined;
        bool                        m_formatExpansion;
        ConfigurationVariableLevel  m_level;

        public:
        DESKTOP_TOOLS_EXPORT  explicit    ExpandOptions (ConfigurationVariableLevel lv = ConfigurationVariableLevel::User);
                        void        SetImmediate                   (bool value) {m_immediate        = value;}
                        void        SetFormat                      (bool value) {m_formatExpansion  = value;}
                        void        SetExpandOnlyIfFullyDefined    (bool value) {m_expandOnlyIfFullyDefined = value;}
                        void        SetLevel (ConfigurationVariableLevel level) {m_level = level;}
        ConfigurationVariableLevel  GetLevel() const  {return m_level;}
        };

    typedef bmap<WString, struct MacroEntry*>  T_MacroMap;

private:
    T_MacroMap      m_macroMap;
    DESKTOP_TOOLS_EXPORT  virtual BentleyStatus   _GetConfigVariable (WStringR envStr, WCharCP envVar, ConfigurationVariableLevel level) override;
    DESKTOP_TOOLS_EXPORT  virtual BentleyStatus   _UndefineConfigVariable (WCharCP cfgVarName) override;
    DESKTOP_TOOLS_EXPORT  virtual BentleyStatus   _DefineConfigVariable (WCharCP cfgVarName, WCharCP cfgValue, ConfigurationVariableLevel level) override;
    DESKTOP_TOOLS_EXPORT  virtual bool            _IsConfigVariableDefined (WCharCP cfgVarName, ConfigurationVariableLevel level) override;
    DESKTOP_TOOLS_EXPORT  virtual bool            _IsConfigVariableDefinedAndTrue (WCharCP cfgVarName, ConfigurationVariableLevel level) override;
    DESKTOP_TOOLS_EXPORT  virtual BentleyStatus   _IterateThroughVariables (IConfigVariableIteratorDelegate *delegate) override;

                    MacroEntry*             GetMacroDefinition (WStringR upcasedMacroName, WCharCP macroName);
                    bool                    ContainsExpression (WCharCP expression);

public:
    //  *** NEEDS WORK These methods are used by the legacy msMacro_ API
                    static bool             IsValidCfgVarLevelForNewDefinition (ConfigurationVariableLevel level);
    DESKTOP_TOOLS_EXPORT  static bool             IsMacroNameValid (WCharCP macroName);

    DESKTOP_TOOLS_EXPORT  WCharCP                 GetMacroTranslation (WCharCP macroName, WStringR tmpString, ConfigurationVariableLevel level);
    DESKTOP_TOOLS_EXPORT  BentleyStatus           GetMacro (WStringP envStr, WCharCP macroName);
    DESKTOP_TOOLS_EXPORT  bool                    IsMacroDefinedFromLevel (WCharCP macroName, ConfigurationVariableLevel fromLevel);
    DESKTOP_TOOLS_EXPORT  BentleyStatus           GetMacroAtLevel (WStringP envStr, WCharCP macroName, ConfigurationVariableLevel level);
    DESKTOP_TOOLS_EXPORT  BentleyStatus           GetMacroFromLevel (WStringP envStr, WCharCP macroName, ConfigurationVariableLevel level);
    DESKTOP_TOOLS_EXPORT  BentleyStatus           GetMacroLevel (ConfigurationVariableLevel& levelP, WCharCP macroName);
    DESKTOP_TOOLS_EXPORT  BentleyStatus           DefineMacro (MacroEntry*& newEntry, WCharCP macroName, WCharCP translation, ConfigurationVariableLevel level);
                    BentleyStatus           DefineMacroWithDebugging (WCharCP macroName, WCharCP translation, ConfigurationVariableLevel level, BeTextFileP debugFile, int debugLevel);
    DESKTOP_TOOLS_EXPORT  BentleyStatus           DefineBuiltinMacro (WCharCP macroName, WCharCP translation);

    DESKTOP_TOOLS_EXPORT  BentleyStatus           RemoveMacroAtLevel (WCharCP macroName, ConfigurationVariableLevel level);
    DESKTOP_TOOLS_EXPORT  BentleyStatus           RemoveAllMacrosAtLevel (ConfigurationVariableLevel level);
    DESKTOP_TOOLS_EXPORT  void                    RemoveAllMacros();

                    void                    EnsureSysEnvDefinition (WCharCP macroName, ConfigurationVariableLevel level, BeTextFileP debugFile, int debugLevel);
    DESKTOP_TOOLS_EXPORT  void                    PrintAllMacros (BeTextFileR outFile, int debugLevel);

    //  These methods are used by MacroFileProcessor
    DESKTOP_TOOLS_EXPORT  BentleyStatus           LockMacro (WCharCP macroName);
    DESKTOP_TOOLS_EXPORT  bool                    IsMacroLocked (WCharCP macroName);
    DESKTOP_TOOLS_EXPORT  BentleyStatus           UnlockMacro (WCharCP macroName, WCharCP passWordP);

    DESKTOP_TOOLS_EXPORT  StatusInt               ExpandMacro (WStringR expansion, WCharCP macroExpression, ExpandOptions const& opts = ExpandOptions());

}; // MacroConfigurationAdmin

typedef MacroConfigurationAdmin::ExpandOptions MacroExpandOptions;

END_BENTLEY_DGN_NAMESPACE
