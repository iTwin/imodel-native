/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DesktopTools/MacroConfigurationAdmin.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include <Bentley/bset.h>
#include <DgnPlatform/DgnPlatform.h>
#include "ConfigurationManager.h"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE


struct          IMacroDebugOutput
    {
    virtual void                    ShowDebugMessage (int indent, WCharCP format, ...) = 0;
    };


struct          MacroEntry;
struct          ExpandOperator;
struct          CompareWChar {bool operator()  (WCharCP s1, WCharCP s2) const  {return BeStringUtilities::Wcsicmp (s1, s2) < 0;}};
struct          CompareMacroEntries { bool operator () (MacroEntry const* first, MacroEntry const* second) const; };

typedef         bmap <WString, WString> T_ReplacementMap;

/*=================================================================================**//**
* An implementation of IConfigurationAdmin that maintains
* a macro database and falls back on the system environment variables and CSIDL for system folders.
* @bsiclass                                     Sam.Wilson                      05/2011
+===============+===============+===============+===============+===============+======*/
struct          MacroConfigurationAdmin : IConfigurationAdmin
    {
    friend struct   MacroExpander;
    friend struct   MacroEntry;
    friend struct   MacroFileProcessor;
    friend struct   const_iterator;

    //! Options to control how macros are expanded
    struct  ExpandOptions
        {
        bool                        m_immediate;
        bool                        m_expandOnlyIfFullyDefined;
        bool                        m_formatExpansion;
        ConfigurationVariableLevel  m_level;

        public:
        DGNPLATFORM_EXPORT   explicit ExpandOptions (ConfigurationVariableLevel lv = ConfigurationVariableLevel::User);
                        void        SetImmediate                   (bool value) {m_immediate        = value;}
                        void        SetFormat                      (bool value) {m_formatExpansion  = value;}
                        void        SetExpandOnlyIfFullyDefined    (bool value) {m_expandOnlyIfFullyDefined = value;}
                        void        SetLevel (ConfigurationVariableLevel level) {m_level = level;}
        ConfigurationVariableLevel  GetLevel() const  {return m_level;}
        };

    typedef bset<struct MacroEntry*, CompareMacroEntries>       T_MacroSet;
    typedef bmap<WCharCP, struct ExpandOperator*, CompareWChar> T_OperatorMap;

private:
    T_MacroSet          m_macroSet;
    T_OperatorMap       m_operatorMap;
    bool                m_dependenciesSuspended;
    MacroEntry*         m_searchEntry;

    DGNPLATFORM_EXPORT    virtual void              _OnHostTermination (bool isProcessShutdown) override;
    DGNPLATFORM_EXPORT    virtual BeFileNameCR      _GetLocalTempDirectoryBaseName () override;
    DGNPLATFORM_EXPORT    virtual BentleyStatus     _GetConfigVariable (WStringR envStr, WCharCP envVar, ConfigurationVariableLevel level) override;
    DGNPLATFORM_EXPORT    virtual BentleyStatus     _UndefineConfigVariable (WCharCP cfgVarName) override;
    DGNPLATFORM_EXPORT    virtual BentleyStatus     _DefineConfigVariable (WCharCP cfgVarName, WCharCP cfgValue, ConfigurationVariableLevel level) override;
    DGNPLATFORM_EXPORT    virtual bool              _IsConfigVariableDefined (WCharCP cfgVarName, ConfigurationVariableLevel level) override;
    DGNPLATFORM_EXPORT    virtual bool              _IsConfigVariableDefinedAndTrue (WCharCP cfgVarName, ConfigurationVariableLevel level) override;
    DGNPLATFORM_EXPORT    virtual BentleyStatus     _IterateThroughVariables (IConfigVariableIteratorDelegate *delegate) override;
    DGNPLATFORM_EXPORT    virtual BentleyStatus     _MonitorVariable (WCharCP cfgVarName, IVariableMonitorR monitor) override;
    DGNPLATFORM_EXPORT    virtual void              _RemoveMonitor (WCharCP cfgVarName, IVariableMonitorR monitor) override;
    DGNPLATFORM_EXPORT    virtual void              _OnUnloadMdlDescr (MdlDescP mdlDesc) override;

                        MacroEntry const*           GetMacroDefinition (WCharCP macroName);
                        bool                        ContainsExpression (WCharCP expression);
                        ExpandOperator*             FindExpansionOperator (WCharCP operatorString);
                        void                        SetDefaultExpansionOperators ();
                        WCharCP                     GetMacroEntryAndTranslation (WCharCP macroName, MacroEntry const*& macroEntry, WStringR tmpString, ConfigurationVariableLevel level, bool addIfNotDefined);
                        BentleyStatus               CreateMacro (MacroEntry const* & newEntry, WCharCP macroName);

public:
    DGNPLATFORM_EXPORT    MacroConfigurationAdmin();
    DGNPLATFORM_EXPORT   ~MacroConfigurationAdmin();

    //  *** NEEDS WORK These methods are used by the legacy msMacro_ API
                        static bool                 IsValidCfgVarLevelForNewDefinition (ConfigurationVariableLevel level);
    DGNPLATFORM_EXPORT    static bool               IsMacroNameValid (WCharCP macroName);

    DGNPLATFORM_EXPORT    WCharCP                   GetMacroTranslation (WCharCP macroName, WStringR tmpString, ConfigurationVariableLevel level);
    DGNPLATFORM_EXPORT    BentleyStatus             GetMacro (WStringP envStr, WCharCP macroName);
    DGNPLATFORM_EXPORT    bool                      IsMacroDefinedFromLevel (WCharCP macroName, ConfigurationVariableLevel fromLevel);
    DGNPLATFORM_EXPORT    BentleyStatus             GetMacroAtLevel (WStringP envStr, WCharCP macroName, ConfigurationVariableLevel level);
    DGNPLATFORM_EXPORT    BentleyStatus             GetMacroFromLevel (WStringP envStr, WCharCP macroName, ConfigurationVariableLevel level);
    DGNPLATFORM_EXPORT    BentleyStatus             GetMacroLevel (ConfigurationVariableLevel& levelP, WCharCP macroName);
    DGNPLATFORM_EXPORT    BentleyStatus             DefineMacro (MacroEntry const* & newEntry, WCharCP macroName, WCharCP translation, ConfigurationVariableLevel level);
                          BentleyStatus             DefineMacroWithDebugging (WCharCP macroName, WCharCP translation, ConfigurationVariableLevel level, int indent, IMacroDebugOutput* debugOutput, int debugLevel);
    DGNPLATFORM_EXPORT    BentleyStatus             DefineBuiltinMacro (WCharCP macroName, WCharCP translation);

    DGNPLATFORM_EXPORT    BentleyStatus             RemoveMacroAtLevel (WCharCP macroName, ConfigurationVariableLevel level, bool removeLocked);
    DGNPLATFORM_EXPORT    BentleyStatus             RemoveAllMacrosAtLevel (ConfigurationVariableLevel level);
    DGNPLATFORM_EXPORT    void                      RemoveAllMacros();

                          void                      EnsureSysEnvDefinition (WCharCP macroName, ConfigurationVariableLevel level, IMacroDebugOutput* debugOutput, int debugLevel);
    DGNPLATFORM_EXPORT    void                      PrintAllMacros (IMacroDebugOutput& debugOutput, int debugLevel);

    //  These methods are used by MacroFileProcessor
    DGNPLATFORM_EXPORT    ExpandOperator*           GetExpansionOperator (WCharCP operatorString);
    DGNPLATFORM_EXPORT    ExpandOperator*           SetExpansionOperator (WCharCP operatorString, ExpandOperator* newOperator);

    DGNPLATFORM_EXPORT    BentleyStatus             LockMacro (WCharCP macroName);
    DGNPLATFORM_EXPORT    bool                      IsMacroLocked (WCharCP macroName);
    DGNPLATFORM_EXPORT    BentleyStatus             UnlockMacro (WCharCP macroName, WCharCP passWordP);

    DGNPLATFORM_EXPORT    StatusInt                 ExpandMacro (WStringR expansion, WCharCP macroExpression, ExpandOptions const& opts = ExpandOptions());

    DGNPLATFORM_EXPORT    void                      SuspendDependencies();
    DGNPLATFORM_EXPORT    void                      ResumeDependencies();
    DGNPLATFORM_EXPORT    BentleyStatus             GetDependentMacros (T_WStringVectorR dependents, WCharCP macroName);

    DGNPLATFORM_EXPORT    BentleyStatus             GetVariable (WStringR cfgValue, WCharCP cfgVarName, ConfigurationVariableLevel level=ConfigurationVariableLevel::User);

    //===================================================================================
    // These methods are used during the conversion process from V8i -> CONNECT.
    // A number of them give access to MacroEntry (which is private to MacroConfigurationAdmin).
    // It is also possible to iterate the MacroEntry's in a MacroConfigurationAdmin.
    //===================================================================================
    DGNPLATFORM_EXPORT    void                      RenameMacros (T_ReplacementMap& replacementMap);
    DGNPLATFORM_EXPORT    MacroEntry*               GetMacroEntry (WCharCP cfgVarName);

    DGNPLATFORM_EXPORT    static void               GetEntryVariableName (WStringR cfgVarName, MacroEntry const* macroEntry);
    DGNPLATFORM_EXPORT    static void               GetEntryTranslationAtLevel (WStringR translation, MacroEntry const* macroEntry, ConfigurationVariableLevel level=ConfigurationVariableLevel::User);
    DGNPLATFORM_EXPORT    static void               RemoveEntryReferencesTo (T_WStringVectorCR cfgVarList, MacroEntry* macroEntry, ConfigurationVariableLevel level);
    DGNPLATFORM_EXPORT    static void               ReplaceEntryReferences (T_ReplacementMap& replacementMap, MacroEntry* macroEntry, ConfigurationVariableLevel level);
    DGNPLATFORM_EXPORT    static BentleyStatus      GetAppropriateAssignmentStatement (WStringR assignmentStatement, MacroEntry const* macroEntry, ConfigurationVariableLevel level, T_ReplacementMap& replacementMap);

    DGNPLATFORM_EXPORT    StatusInt                 ReadV8iConfigurationFiles (WCharCP userName, WCharCP projectName, WCharCP workspaceRootDir, WCharCP msInstallDir, WCharCP msConfigFileName, WCharCP fallbackConfigDir, T_WStringVector& assignmentArgs);
    DGNPLATFORM_EXPORT    StatusInt                 ReadCONNECTConfigurationFiles (WCharCP workSpaceName, WCharCP workSetName, WCharCP configurationRootDir, WCharCP msInstallDir, WCharCP msConfigFileName, WCharCP fallbackConfigDir, T_WStringVector& assignmentArgs);

    //===================================================================================
    // Iterator over the MacroEntries
    //===================================================================================
    struct const_iterator : std::iterator<std::forward_iterator_tag, MacroEntry*>
    {
    ///@cond DONTINCLUDEINDOC
    private:
        friend struct MacroConfigurationAdmin;

        T_MacroSet::const_iterator                  m_setIterator;
        const_iterator (T_MacroSet::const_iterator  inputIterator);

        //! @endcond
    public:
        //! Advances the iterator to the next in collection.
        DGNPLATFORM_EXPORT   const_iterator&  operator ++();
        DGNPLATFORM_EXPORT   MacroEntry* operator *() const;
        DGNPLATFORM_EXPORT   bool operator != (const_iterator const &) const;
        DGNPLATFORM_EXPORT   bool operator == (const_iterator const & rhs) const {return !(*this != rhs);}

    };

    typedef const_iterator iterator;    //!< only const iteration is possible

    //! Returns the beginning of an iteration over the MacroEntries in the Administrator
    DGNPLATFORM_EXPORT   const_iterator begin () const;

    //! Returns the end of the  an iteration over the MacroEntries in the Administrator
    DGNPLATFORM_EXPORT   const_iterator end () const;


    }; // MacroConfigurationAdmin


typedef MacroConfigurationAdmin::ExpandOptions MacroExpandOptions;

enum        class   ExpandStatus
    {
    Success          = 0,
    NullExpression,
    Circular,
    DollarNotFollowedByParentheses,
    NoCloseParentheses,
    SyntaxErrorAfterDollarSign,
    SyntaxErrorExpansion,
    SyntaxErrorUnmatchedQuote,
    SyntaxErrorUnmatchedParen,
    SyntaxErrorUnmatchedBrace,
    SyntaxErrorInOperand,
    SyntaxErrorBadOperator,
    };


/*=================================================================================**//**
* @bsiclass                                     Sam.Wilson                      02/2012
+===============+===============+===============+===============+===============+======*/
struct      MacroExpander
    {
    friend      MacroConfigurationAdmin;

    private:

    MacroConfigurationAdmin&    m_macroCfgAdmin;
    int                         m_recursionDepth; // this is the recursion level of macro expansions (i.e., macros that reference other macros).
    bool                        m_expandOnlyIfFullyDefined;
    bool                        m_expandAllImmediately;
    bool                        m_formatExpansion;
    ConfigurationVariableLevel  m_cfgLevel;

    MacroExpander (MacroConfigurationAdmin& macroCfgAdmin, MacroConfigurationAdmin::ExpandOptions const& options);

    public:

    static bool                 CharIsLeftParen (WChar thisChar, bool immediate);
    static bool                 ContainsExpression (WCharCP textExpression, bool immediate);
    size_t                      SkipWhiteSpace (WString inputString, size_t position);

    ExpandStatus                SplitArguments (size_t& errorPosition, T_WStringVectorR arguments, WStringR operandString);
    ExpandStatus                SplitExpression (size_t& errorPosition, WStringR expansion, ExpandOperator*& thisOperator, bool& immediateExpansion, WStringR operandString, WStringR macroString);
    ExpandStatus                ExpandMacroExpression (WStringR expansion, WCharCP macroExpression, bool considerToBeMacro, bool immediateExpansion);
    StatusInt                   FormatExpansion (WStringR expandedString);

    MacroConfigurationAdmin&    GetCfgAdmin();
    ConfigurationVariableLevel  GetLevel();

    StatusInt                   Expand (WStringR expansion, WCharCP macroExpression);

    }; // MacroExpander

struct      ExpandOperator
    {
    virtual ExpandStatus Execute (size_t& errorPosition, MacroExpander& macroExpander, WStringR result, WStringR operandString, bool immediateExpansion) = 0;
    };

END_BENTLEY_DGNPLATFORM_NAMESPACE
