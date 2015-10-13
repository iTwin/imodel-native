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

typedef struct mdlDesc* MdlDescP;

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

/**
@addtogroup ConfigManagement
Configuration variables allow applications and users to supply values specific to their needs at runtime. Configuration variables
are pairs of name/value strings. Configuration variables can be defined recursivley, such that one configuration variable's value
is defined in terms of another configuration variable. When the value of a configuation variable is retrieved, these references
are resolved.
Configuration variable names are not case sensitive.
@beginGroup
*/

/*=================================================================================**//**
The IVariableMonitor interface is implemented by classes that need to be informed when a Configuration Variable is changed.
+===============+===============+===============+===============+===============+======*/
struct IVariableMonitor
{
public:
    virtual ~IVariableMonitor() {;}
    
    //! Called to inform that a Configuration variable was redefined.
    //! @param[in] variableName     The Configuration Variable name.
    virtual void _VariableChanged (WCharCP variableName) = 0;

    //! Called to inform that a Configuration variable that this Configuration variable depends on was redefined.
    //! @param[in] variableName     The Configuration Variable name.
    //! @param[in] rootVariableName The Configuration Variable name of the changed root.
    //! @note   The variable that was changed may be an "ancestor" configuration variable, i.e., not a direct root of this configuration variable, but a root of one of its roots, etc.
    virtual void _VariableRootChanged (WCharCP variableName, WCharCP rootVariableName) = 0;

    //! Called to inform that a Configuration variable was undefined.
    //! @param[in] variableName     The Configuration Variable name.
    virtual void _VariableUndefined (WCharCP variableName) = 0;

    //! Called to inform that a Configuration variable this this Configuration variable depends on was undefined.
    //! @param[in] variableName     The Configuration Variable name.
    //! @param[in] rootVariableName The Configuration Variable name of the undefined root.
    //! @note   The variable that was undefined may be an "ancestor" configuration variable, i.e., not a direct root of this configuration variable, but a root of one of its roots, etc.
    virtual void _VariableRootUndefined (WCharCP variableName, WCharCP rootVariableName) = 0;

    //! Called to inform of the IVariableMonitor that the configuration monitor has been stopped for the configuraion variable.
    //! @param[in] variableName     The Configuration Variable name.
    //! @note   Typical behavior is to delete this instance of IVariableMonitor. In the case where one IVariableMonitor instance is used to monitor multiple configuration variables, use reference counting.
    //!         Increment the reference count for each call to MonitorVariable, and decrement the refeerence count for each call to IVariableMonitor::_MonitorStopped, deleting the instance when reference count hits 0.
    virtual void _MonitorStopped (WCharCP variableName) = 0;
};

/*=================================================================================**//**
The SimpleConfiguraionVariableMonitor class implements IVariableMonitor and provides only one callback for any change to a ConfigurationVariable.
+===============+===============+===============+===============+===============+======*/
struct SimpleConfigurationVariableMonitor : public IVariableMonitor
    {
    DGNPLATFORM_EXPORT SimpleConfigurationVariableMonitor ();

public:
    DGNPLATFORM_EXPORT virtual void _VariableChanged (WCharCP variableName) override;
    DGNPLATFORM_EXPORT virtual void _VariableRootChanged (WCharCP variableName, WCharCP rootVariableName) override;
    DGNPLATFORM_EXPORT virtual void _VariableUndefined (WCharCP variableName) override;
    DGNPLATFORM_EXPORT virtual void _VariableRootUndefined (WCharCP variableName, WCharCP rootVariableName) override;
    DGNPLATFORM_EXPORT virtual void _MonitorStopped (WCharCP variableName) override;

    //! Called when any change to the monitored Variable is detected. Application should retrieve the new configuration variable value and refresh their internal state.
    virtual void _UpdateState (WCharCP variableName) = 0;
    };

//! Provide a delegate mechanism for iterating through all configuration variables.
struct IConfigVariableIteratorDelegate
    {
    //! Called for each configuration variable.
    virtual void EachConfigVariable (WCharCP name, WCharCP value, ConfigurationVariableLevel level, bool locked) = 0;
    };


//__PUBLISH_SECTION_END__
//__PUBLISH_SCOPE_1_START__

//! Provide configuration-related values for ConfigurationManager. DgnPlatform libraries and applications use ConfigurationManager to
//! query host or user-configurable options. ConfigurationManager, in turn, calls ConfigurationAdmin methods to allow hosts to control how
//! those options are specified at runtime. For example, a host might implement this interface to return values stored in registry
//! keys. Generally, the default implementation is sufficient.
struct IConfigurationAdmin : DgnHost::IHostObject
    {
    virtual ~IConfigurationAdmin() {;}
    friend struct ConfigurationManager;

protected:
    //! Get the name of a local directory that can be used for creating temporary files. The directory must have
    //! write access. Subsystems generally create subdirectories in this directory for storing temporary data files.
    //! @return A string that holds the name of the base directory for temporary files. This is expected to always return the same object.
    //! The default implementation uses the Windows function GetTempPath().
    //! @note This method supplies the implementation for ConfigurationManager::GetLocalTempDirectoryBaseName.
    virtual BeFileNameCR _GetLocalTempDirectoryBaseName() = 0;

    //! Test for the existence of a configuration variable, and optionally return its (recursively) expanded value.
    //! @param[out]  cfgValue   The expanded value of cfgVarName. May be NULL to merely test for the existence of the configuration variable.
    //!                         If non-NULL, and if cfgVarName is not defined, the value is set to the empty string.
    //! @param[in]   cfgVarName The name of the configuration variable to test. If cfgVarName is NULL, return ERROR
    //! @param[in]   level      The level of the configuration variable.
    //! @return SUCCESS if cfgVarName is defined and cfgValue is valid.
    //! @note The default implementation first checks for configuration variables defined by _DefineConfigVariable and, if present,
    //!       returns that value. If no configuration variable exists, it checks the Environment variables (via the Windows
    //!       function #GetEnvironmentVariable) and returns that value, if it exists.
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

    // Monitor a configuration variable
    virtual BentleyStatus _MonitorVariable (WCharCP cfgVarName, IVariableMonitorR monitor) = 0;

    // Remove a configuration variable Monitor
    virtual void          _RemoveMonitor (WCharCP cfgVarName, IVariableMonitorR monitor) = 0;

    // Respond to unloading of an Mdl.
    virtual void          _OnUnloadMdlDescr (MdlDescP mdlDesc) = 0;

public: 
    // most of the virtual methods of IConfigurationAdmin are accessible only through the ConfigurationManager class.

    DGNPLATFORM_EXPORT BentleyStatus GetConfigVariable(WStringR cfgValue, WCharCP cfgVarName, ConfigurationVariableLevel level);
    };
//__PUBLISH_SECTION_START__

/*=================================================================================**//**
The ConfigurationManager supplies methods that allow users and applications to customize the behavior of programs at runtime.

@remarks A configuration is specific to a host, and there may be more than one host active in an application. For example,
         each thread in the application can have its own host.
         The methods of ConfigurationManager provide access to the configuration database of <em>the current host</em>.
         Therefore, calling ConfigurationManager::GetVariable ("A") in one thread can yield a different value or return
         status than calling the same function with the same argument in another thread.

There are two ways to share configuration variables among multiple hosts:
\li Put the configuration variables into the native OS environment. Use the util_putenv function to to this.
\li Have the threads share the same host. \em Note: Most DgnPlatform functions are \em not thread-safe. It is up to the application to ensure thread-safety when sharing hosts among threads.

* @bsiclass
+===============+===============+===============+===============+===============+======*/
// ***
// *** NB: The ConfigurationManager functions must call the s_GetAdminFunc function pointer every time a config
// ***     variable is needed. The admin object that it returns must *not* be cached.
// ***
struct          ConfigurationManager
{
//__PUBLISH_SECTION_END__
//__PUBLISH_SCOPE_1_START__
typedef IConfigurationAdmin& (*T_GetAdminFunc) ();

                    static  T_GetAdminFunc    s_getAdminFunc;
public:
//! Ask the current host for its ConfigurationAdmin object
//! @remarks This might be confusing. The application provides a <em>single</em> function pointer to identify the admin of the </em>current</em> Host, which might be specific to the current thread.
DGNPLATFORM_EXPORT     static  void SetGetAdminFunc (T_GetAdminFunc);

//! Get the currently registered ConfigurationAdmin. When DgnPlatform is initialized, this will be the ConfigurationAdmin tied to the current Host.
DGNPLATFORM_EXPORT     static  IConfigurationAdmin& GetAdmin();

//! Get a singleton instance of an implementation of ConfigurationAdmin that accesses the native OS system environment.
DGNPLATFORM_EXPORT     static  IConfigurationAdmin& GetEnvAdmin();

//__PUBLISH_SECTION_END__

DGNPLATFORM_EXPORT     static   void                OnUnloadMdlDescr (MdlDescP mdlDescr);

//__PUBLISH_SECTION_START__
public:
    //! Return the root of the local directory that can be used to store temporary files. This directory must have write access.
    //! @return NULL if no temporary directory available.
    DGNPLATFORM_EXPORT static BeFileNameCR GetLocalTempDirectoryBaseName ();

    //! Return a local directory that can be used to store temporary files. This directory can optionally be a subdirectory of #GetLocalTempDirectoryBaseName.
    //! @param[out] tempDir The name of temporary directory. This must be MAX_PATH chars in size.
    //! @param[in]  subDirName Optional subdirectory relative to default temp directory. If non-NULL, this subdirectory will be created.
    //! @return NULL if no temporary directory available.
    DGNPLATFORM_EXPORT static BentleyStatus GetLocalTempDirectory (BeFileNameR tempDir, WCharCP subDirName);

#if defined (NEEDSWORK_DESKTOP_PLATFORM)
    //! Generate a unique name for a temporary file.
    //! @param[out] tempFileName  The name for a temporary file. Should be MAX_PATH chars in size.
    //! @param[in] partialPathName  intermediate dirs you want between %tmp% and the filename generated.
    //! @param[in] prefixString    up to 3 chars of this will be used in the generated filename
    //! @return SUCCESS, if the directory for the temporary file exists or was created.
    //! @note This method does not create a temporary file, it merely returns a name that can be used to create a temporary file.
    DGNPLATFORM_EXPORT static BentleyStatus GetNameForTempFile (BeFileNameR tempFileName, WCharCP partialPathName, WCharCP prefixString);
#endif

    //! Check for the existence of a configuration variable.
    //! @param[in] cfgVarName The name of the configuration variable to check.
    //! @return true if \c cfgVarName is defined.
    DGNPLATFORM_EXPORT  static bool IsVariableDefined (WCharCP cfgVarName);

    //! Check for a configuration variable defined to a "true" value.
    //! @param[in] cfgVarName The name of the configuration variable to check.
    //! @return true if \c cfgVarName is defined to "1", "true", or "on" (case insensitive).
    DGNPLATFORM_EXPORT  static bool IsVariableDefinedAndTrue (WCharCP cfgVarName);

    //! Check for a configuration variable defined to a "false" value.
    //! @param[in] cfgVarName The name of the configuration variable to check.
    //! @return true if \c cfgVarName is defined to "0", "false", or "off" (case insensitive).
    //! @remarks If the \c cfgVarName is not defined, returns false.
    DGNPLATFORM_EXPORT  static bool IsVariableDefinedAndFalse (WCharCP cfgVarName);

    //! Get the value for a configuration variable in a WString.
    //! @param[out] cfgValue The WString to fill with the fully expanded value of the configuration variable.
    //! @param[in] cfgVarName The name of the configuration variable to retrieve.
    //! @param[in]  level level of configuration variable. Typically, use #ConfigurationVariableLevel::User.
    //! @return SUCCESS if the configuration variable was defined and its value is in \c cfgValue. On failure, \c cfgValue will be empty.
    //! @remarks if the variable is defined at a higher level, this function returns SUCCESS but cfgValue will be empty.
    DGNPLATFORM_EXPORT  static BentleyStatus GetVariable (WStringR cfgValue, WCharCP cfgVarName, ConfigurationVariableLevel level=ConfigurationVariableLevel::User);

    //! Define a configuration variable. If the variable already exists, its value is redefined.
    //! @param[in]  cfgVarName Name of the configuration variable. Must not be NULL.
    //! @param[in]  cfgValue The new value. Must not be NULL.
    //! @param[in]  level level of configuration variable. Typically, use #ConfigurationVariableLevel::User.
    //! @return SUCCESS if the variable is successfully defined.
    DGNPLATFORM_EXPORT  static BentleyStatus DefineVariable (WCharCP cfgVarName, WCharCP cfgValue, ConfigurationVariableLevel level=ConfigurationVariableLevel::User);

    //! Undefine (remove) the configuration variable.
    //! @param cfgVarName The name of the variable to undefine.
    //! @note This method cannot be used to remove configuration values that come from operating system environment variables.
    //! @return  SUCCESS if the variable is successfully deleted. This method will fail if the variable is not defined.
    DGNPLATFORM_EXPORT  static BentleyStatus UndefineVariable (WCharCP cfgVarName);

    //! Iterate through all config variables and call the delegate for each.
    //! @param[in] delegate The delegate to call for each config variable.
    //! @return  SUCCESS if the variables could be iterated through.
    DGNPLATFORM_EXPORT  static BentleyStatus IterateThroughVariables (IConfigVariableIteratorDelegate *delegate);

    //! Monitor a ConfigurationVariable for changes.
    //! @param[in] cfgVarName   The configuration variable to monitor.
    //! @param[in] monitor      An instance of a class that implements IVariableMonitor.
    //! @return The method will fail if the configuration variable is not defined, or if the implementation of ConfigurationManager cannot monitor variables. The Macro system ConfigurationManager is able to monitor variables.
    //! @note   When this method is called, the IVariableMonitor::_VariableChanged method is called with the current configuration variable value, and with reason set to VariableChangeReason::Initiated.
    DGNPLATFORM_EXPORT  static BentleyStatus    MonitorVariable (WCharCP cfgVarName, IVariableMonitorR monitor);

    //! Remove a ConfigurationVariable Monitor.
    //! @param[in] cfgVarName   The configuration variable to monitor.
    //! @param[in] monitor      The instance of the class that implements IVariableMonitor that is to be removed as a monitor.
    //! @return The method will fail if the configuration variable is not defined, or if the implementation of ConfigurationManager cannot monitor variables. The Macro system ConfigurationManager is able to monitor variables.
    //! @note   When this method is called, the IVariableMonitor::_VariableChanged method is called with the current configuration variable value, and with reason set to VariableChangeReason::Initiated.
    DGNPLATFORM_EXPORT  static BentleyStatus RemoveMonitor (WCharCP cfgVarName, IVariableMonitorR monitor);

    //! Get and update a ConfigurationVariable that represents a boolean value.
    //! @param[out] monitor     A reference to a static IVariableMonitorP variable that is used both to hold the IVariableMonitorP in case it is needed by the caller, and to detect whether the monitor has been initialized on
    //!                         subsequent calls.
    //! @param[in] value        A reference to the boolean that will be set and updated by this method. After the initial call, the variable is set to the value stored in the configuration variable.
    //!                         If the configuration variable is subsequently changed, either by changing projects or directly by the user, the boolean variable referenced is updated.
    //! @param[in] cfgVarName   The configuration variable to monitor.
    //! @param[in] defaultValue The default that value is set to if the configuration variable is not defined.
    //! @return The method returns the value reference by the "value" parameter.
    //! @note   The value of a configuration variable is considered true if the it translates to "1", "true", or "on" (case insensitive).
    //! @note   You must ensure that both the value and monitor parameters references static variables, or make sure that you call ConfigurationManager::RemoveMonitor with the IVariableMonitor
    //!         returned when the variable ceases to exist (for example in the destructor of a class if they are class members).
    //! @note   After the first calls to MonitorBoolean, subsequent calls are very fast. The program can either
    DGNPLATFORM_EXPORT  static bool MonitorBoolean (IVariableMonitorP& monitor, bool& value, WCharCP cfgVarName, bool defaultValue);

    //! Get and update a ConfigurationVariable that represents an integer value.
    //! @param[out] monitor     A reference to a static IVariableMonitorP variable that is used both to hold the IVariableMonitorP in case it is needed by the caller, and to detect whether the monitor has been initialized on
    //!                         subsequent calls.
    //! @param[in] value        A reference to the integer that will be set and updated by this method. After the initial call, the variable is set to the value stored in the configuration variable.
    //!                         If the configuration variable is subsequently changed, either by changing projects or directly by the user, the boolean variable referenced is updated.
    //! @param[in] cfgVarName   The configuration variable to monitor.
    //! @param[in] defaultValue The default that value is set to if the configuration variable is not defined.
    //! @param[in] minimumValue The minimum that value can be set to.
    //! @param[in] maximumValue The maximum that value can be set to.
    //! @return The method returns the value reference by the "value" parameter.
    //! @note   You must ensure that both the value and monitor parameters references static variables, or make sure that you call ConfigurationManager::StopMonitor with the IVariableMonitor
    //!         returned when the variable ceases to exist (for example in the destructor of a class if they are class members).
    DGNPLATFORM_EXPORT  static int  MonitorInteger (IVariableMonitorP& monitor, int& value, WCharCP cfgVarName, int defaultValue, int minimumValue, int maximumValue);

    //! Get and update a string ConfigurationVariable.
    //! @param[out] monitor     A reference to a static IVariableMonitorP variable that is used both to hold the IVariableMonitorP in case it is needed by the caller, and to detect whether the monitor has been initialized on
    //!                         subsequent calls.
    //! @param[in] string        A reference to the string that will be set and updated by this method. After the initial call, the variable is set to the value stored in the configuration variable.
    //!                         If the configuration variable is subsequently changed, either by changing projects or directly by the user, the boolean variable referenced is updated.
    //! @param[in] cfgVarName   The configuration variable to monitor.
    //! @return The method returns the value reference by the "value" parameter.
    //! @note   You must ensure that both the value and monitor parameters references static variables, or make sure that you call ConfigurationManager::StopMonitor with the IVariableMonitor
    //!         returned when the variable ceases to exist (for example in the destructor of a class if they are class members).
    DGNPLATFORM_EXPORT  static WCharCP MonitorString (IVariableMonitorP& monitor, WStringR string, WCharCP cfgVarName);


    //__PUBLISH_SECTION_END__

    DGNPLATFORM_EXPORT  static bool     StringContainsMacros (WCharCP string);
    DGNPLATFORM_EXPORT  static void     StringExpandMacros (WStringR expanded);

    //__PUBLISH_SECTION_START__

};

//__PUBLISH_SECTION_END__

/*=================================================================================**//**
* @bsiclass                                                     Barry.Bentley   06/2014
* This class stores all the monitored configuration variables used in DgnPlatform.
* It is needed because each thread has a different configuration variable.
+===============+===============+===============+===============+===============+======*/
struct  MonitorBooleanPair
    {
    IVariableMonitorP   m_monitor;
    bool                m_value;
    MonitorBooleanPair ()
        {
        m_monitor = NULL;
        m_value   = false;
        }
    };

struct  MonitorStringPair
    {
    IVariableMonitorP   m_monitor;
    WString             m_value;
    MonitorStringPair ()
        {
        m_monitor = NULL;
        }
    };

struct DgnPlatformConfigVars : DgnHost::HostObjectBase
    {
private:
    // these are all struct members so they will automatically initialize.
    MonitorBooleanPair  m_accumulateColorAdjustment;
    MonitorBooleanPair  m_noPatternGeometryMaps;
    MonitorBooleanPair  m_needsBlankSpaceFit;
    MonitorBooleanPair  m_refMergeBreakAssociations;
    MonitorBooleanPair  m_wireframeSheetLegacyOverride;
    MonitorBooleanPair  m_loadCveAttachmentsDisabled;
    MonitorBooleanPair  m_allowDwgMasterColors;
    MonitorBooleanPair  m_useClipFrozenLevel;
    MonitorBooleanPair  m_treatOWithStokeAsDiameter;
    MonitorBooleanPair  m_noStrokeNonPlanarPolygon;
    MonitorBooleanPair  m_strokeNonPlanarPolygonTopView;
    MonitorBooleanPair  m_materialMasterMat;
    MonitorBooleanPair  m_newLevelDisplay;
    MonitorBooleanPair  m_drawDimensionMasking;
    MonitorBooleanPair  m_debugAssociativityData;
    MonitorBooleanPair  m_disableQuickScan;
    MonitorBooleanPair  m_refCycleCheck;
    MonitorStringPair   m_detailingRequiredSettings;
    MonitorStringPair   m_historyRevisionFormat;


public:
    // get the instance for this thread.
    DGNPLATFORM_EXPORT  static  DgnPlatformConfigVars&     Instance();

                        bool    AccumulateColorAdjustment ();
                        bool    NeedsBlankSpaceFit ();
                        bool    RefMergeBreakAssociations ();
                        bool    WireframeSheetLegacyOverride ();
                        bool    LoadCveAttachmentsDisabled ();
                        bool    AllowDwgMasterColors ();
                        bool    UseClipFrozenLevel ();
                        bool    AllowCapitalOWithStrokeAsDiameter ();
                        bool    NoStrokeNonPlanarPolygon ();
                        bool    StrokeNonPlanarPolygonTopView ();
                        bool    MaterialMasterMat ();
                        bool    RefCycleCheck ();
                        bool    NewLevelDisplay ();
                        bool    DrawDimensionMasking ();
                        bool    DebugAssociativityData ();
    DGNPLATFORM_EXPORT  bool    DisableQuickScan ();

                        WCharCP DetailingRequiredSettings ();
                        WCharCP HistoryRevisionNumberFormat ();

                        bool*   GetLoadCveValueP ();
    };

//__PUBLISH_SECTION_START__
/** @endGroup */
END_BENTLEY_DGNPLATFORM_NAMESPACE

