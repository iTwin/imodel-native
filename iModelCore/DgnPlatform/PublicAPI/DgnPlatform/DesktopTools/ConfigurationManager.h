/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DesktopTools/ConfigurationManager.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

// __BENTLEY_INTERNAL_ONLY__

#include <DgnPlatform/DgnPlatform.h>
#include <DgnPlatform/DgnHost.h>

BEGIN_BENTLEY_DGN_NAMESPACE

//! Provide a delegate mechanism for iterating through all configuration variables.
struct IConfigVariableIteratorDelegate
    {
    //! Called for each configuration variable.
    virtual void EachConfigVariable (WCharCP name, WCharCP value, ConfigurationVariableLevel level, bool locked) = 0;
    };

//! Provide configuration-related values for ConfigurationManager. 
struct IConfigurationAdmin
    {
    friend struct ConfigurationManager;

public:
    //! Test for the existence of a configuration variable, and optionally return its (recursivley) expanded value.
    //! @param[out]  cfgValue   The expanded value of cfgVarName. May be NULL to merely test for the existence of the configuration variable.
    //!                         If non-NULL, and if cfgVarName is not defined, the value is set to the empty string.
    //! @param[in]   cfgVarName The name of the configuration variable to test. If cfgVarName is NULL, return ERROR
    //! @param[in]   level      The level of the configuration variable.
    //! @return SUCCESS if cfgVarName is defined and cfgValue is valid.
    //! @note The default implementation first checks for configuration variables defined by _DefineConfigVariable and, if present,
    //!       returns that value. If no configuration variable exists, it checks the Environment variables (via the Windows
    //!       function GetEnvironmentVariable) and returns that value, if it exists.
    //! @note This method supplies the implementation for ConfigurationManager::GetVariable.
    virtual BentleyStatus _GetConfigVariable (WStringR cfgValue, WCharCP cfgVarName, ConfigurationVariableLevel level) = 0;

    //! Test for the existence of a configuration variable.
    //! @param[in]   cfgVarName The name of the configuration variable to test. If cfgVarName is NULL, returns false.
    //! @param[in]   level      The level of the configuration variable.
    //! @return true if cfgVarName is defined.
    //! @note This method supplies the implementation for ConfigurationManager::IsVariableDefined.
    virtual bool _IsConfigVariableDefined (WCharCP cfgVarName, ConfigurationVariableLevel level) = 0;

    //! Test for the existence of a configuration variable.
    //! @param[in]   cfgVarName The name of the configuration variable to test. If cfgVarName is NULL, returns false.
    //! @param[in]   level      The level of the configuration variable.
    //! @return true if \c cfgVarName is defined to "1" or "true" (case insensitive).
    //! @note This method supplies the implementation for ConfigurationManager::IsVariableDefinedAndTrue.
    virtual bool _IsConfigVariableDefinedAndTrue (WCharCP cfgVarName, ConfigurationVariableLevel level) = 0;

    //! Define, or redefine, a configuration variable. This method supplies the implementation for ConfigurationManager::DefineVariable.
    virtual BentleyStatus _DefineConfigVariable (WCharCP cfgVarName, WCharCP cfgValue, ConfigurationVariableLevel level) = 0;

    //! Undefine (remove) a configuration variable. This method supplies the implementation for ConfigurationManager::UndefineVariable.
    virtual BentleyStatus _UndefineConfigVariable (WCharCP cfgVarName) = 0;

    virtual BentleyStatus _IterateThroughVariables (IConfigVariableIteratorDelegate *delegate) = 0;
    };

/**
@addtogroup ConfigManagement
Configuration variables allow applications and users to supply values specific to their needs at runtime. Configuration variables
are pairs of name/value strings. Configuration variables can be defined recursivley, such that one configuration variable's value
is defined in terms of another configuration variable. When the value of a configuation variable is retrieved, these references
are resolved.
Configuration variable names are not case sensitive.
*/

/*=================================================================================**//**
The ConfigurationManager supplies methods that allow users and applications to customize the behavior of programs at runtime.

 @ingroup ConfigManagement
* @bsiclass
+===============+===============+===============+===============+===============+======*/
// ***
// *** NB: The ConfigurationManager functions must call the s_GetAdminFunc function pointer every time a config
// ***     variable is needed. The admin object that it returns must *not* be cached.
// ***
struct  ConfigurationManager
{
    typedef IConfigurationAdmin& (*T_GetAdminFunc) ();
    static  T_GetAdminFunc    s_getAdminFunc;

public:
//! Register a function that supplies the admin to use.
DESKTOP_TOOLS_EXPORT static void SetGetAdminFunc (T_GetAdminFunc);

//! Get the currently registered ConfigurationAdmin. This is either the default admin (MacroConfigurationAdmin) or the admin returned by the function passed to SetGetAdminFunc.
DESKTOP_TOOLS_EXPORT static IConfigurationAdmin& GetAdmin();

//! Get a singleton instance of an implementation of ConfigurationAdmin that accesses the native OS system environment. This is a stripped down admin that could be used in preference to the default MacroConfigurationAdmin.
DESKTOP_TOOLS_EXPORT static IConfigurationAdmin& GetEnvAdmin();

public:
    //! Check for the existence of a configuration variable.
    //! @param[in] cfgVarName The name of the configuration variable to check.
    //! @return true if \c cfgVarName is defined.
    DESKTOP_TOOLS_EXPORT  static bool IsVariableDefined (WCharCP cfgVarName);

    //! Check for a configuration variable defined to a "true" value.
    //! @param[in] cfgVarName The name of the configuration variable to check.
    //! @return true if \c cfgVarName is defined to "1" or "true" (case insensitive).
    DESKTOP_TOOLS_EXPORT  static bool IsVariableDefinedAndTrue (WCharCP cfgVarName);

    //! Check for a configuration variable defined to a "false" value.
    //! @param[in] cfgVarName The name of the configuration variable to check.
    //! @return true if \c cfgVarName is defined to "0" or "false" (case insensitive).
    //! @remarks If the \c cfgVarName is not defined, returns false.
    DESKTOP_TOOLS_EXPORT  static bool IsVariableDefinedAndFalse (WCharCP cfgVarName);

    //! Check for the existence of a configuration variable if not already cached.
    //! @remarks If the value of \c checked is 0, then this function calls IsVariableDefined(cfgVarName)
    //! and sets \c checked to 1 or -1. If \c checked is not 0, then this function returns true if \c checked == 1 or false otherwise.
    //! @remarks The typical use of this function is to declare a host variable to hold the cached value and pass that in as the \c checked parameter.
    //! @param[in,out] checked      cached value of previous check
    //! @param[in] cfgVarName       The name of the configuration variable to check.
    //! @return true if \c cfgVarName is defined.
    DESKTOP_TOOLS_EXPORT  static bool CheckVariableIsDefined (int& checked, WCharCP cfgVarName);

    //! Get the value for a configuration variable in a WString.
    //! @param[out] cfgValue The WString to fill with the fully expanded value of the configuration variable.
    //! @param[in] cfgVarName The name of the configuration variable to retrieve.
    //! @param[in]  level level of configuration variable. Typically, use #ConfigurationVariableLevel::User.
    //! @return SUCCESS if the configuration variable was defined and its value is in \c cfgValue. On failure, \c cfgValue will be empty.
    //! @remarks if the variable is defined at a higher level, this function returns SUCCESS but cfgValue will be empty.
    DESKTOP_TOOLS_EXPORT  static BentleyStatus GetVariable (WStringR cfgValue, WCharCP cfgVarName, ConfigurationVariableLevel level=ConfigurationVariableLevel::User);

    //! Define a configuration variable. If the variable already exists, its value is redefined.
    //! @param[in]  cfgVarName Name of the configuration variable. Must not be NULL.
    //! @param[in]  cfgValue The new value. Must not be NULL. There is limit of 2048 characters for a value.
    //! @param[in]  level level of configuration variable. Typically, use #ConfigurationVariableLevel::User.
    //! @return SUCCESS if the variable is successfully defined.
    DESKTOP_TOOLS_EXPORT  static BentleyStatus DefineVariable (WCharCP cfgVarName, WCharCP cfgValue, ConfigurationVariableLevel level=ConfigurationVariableLevel::User);

    //! Undefine (remove) the configuration variable.
    //! @param cfgVarName The name of the variable to undefine.
    //! @note This method cannot be used to remove configuration values that come from operating system environment variables.
    //! @return  SUCCESS if the variable is successfully deleted. This method will fail if the variable is not defined.
    DESKTOP_TOOLS_EXPORT  static BentleyStatus UndefineVariable (WCharCP cfgVarName);

    //! Get the value of a configuration variable and cache it. The first time you call it in a given host, this function reads the configuration
    //! variable from the host's Configuration manager and caches the value as a host object. The cached value is returned from all subsequent calls
    //! on the same host.
    //! @param[in,out] key          Key that identifies the cached variable value
    //! @param[in]     cfgVarName   The name of the configuration variable
    //! @return The value of the configuration variable (as of the first call to this function on the current host) or NULL if it is not defined.
    //! @remarks \c key should be statically defined by the caller.
    DESKTOP_TOOLS_EXPORT static WCharCP GetVariableOnce (DgnHost::Key& key, WCharCP cfgVarName);

    //! Check if the configuration variable is defined and cache the the answer. The first time you call it in a given host, this function calls on the
    //! host's Configuration manager to detect if the variable is defined. It then caches the result as a host object. The cached value is returned from all subsequent calls
    //! on the same host.
    //! @param[in,out] key          Key that identifies the cached variable value.
    //! @param[in]     cfgVarName   The name of the configuration variable
    //! @return true if the configuration variable is defined as of the first call to this function on the current host.
    //! @remarks \c key should be statically defined by the caller.
    DESKTOP_TOOLS_EXPORT static bool    CheckVariableIsDefinedOnce (DgnHost::Key& key, WCharCP cfgVarName);

    //! Iterate through all config variables and call the delegate for each.
    //! @param delegate The delegate to call for each config variable.
    //! @return  SUCCESS if the variables could be iterated through.
    DESKTOP_TOOLS_EXPORT  static BentleyStatus IterateThroughVariables (IConfigVariableIteratorDelegate *delegate);

    //__PUBLISH_SECTION_END__

    DESKTOP_TOOLS_EXPORT  static bool     StringContainsMacros (WCharCP string);
    DESKTOP_TOOLS_EXPORT  static void     StringExpandMacros (WStringR expanded);

#define DEFINE_CFGVAR_CHECKER(_V_)\
    DgnHost::Key    _V_ ## cfgvar_checker;

#define CHECK_CFGVAR_IS_DEFINED(_V_)\
    ConfigurationManager::CheckVariableIsDefinedOnce (_V_ ## cfgvar_checker, _CRT_WIDE (#_V_))
};

END_BENTLEY_DGN_NAMESPACE
