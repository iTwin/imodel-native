//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/win/h/HFCService.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HFCService
//-----------------------------------------------------------------------------
// HFCService.h : header file
//-----------------------------------------------------------------------------
#pragma once

#ifndef _WIN32
#error Tough Luck building a Windows service on this platform
#endif

#include <ImagePP/all/h/HFCExclusiveKey.h>

typedef list<WString, allocator<WString> >
HFCServiceDependencies;

//An MFC framework encapsulation of an NT service
//You are meant to derive your own class from this and
//override its functions to implement your own
//service specific functionality.
class HFCService
    {
public:
    //--------------------------------------
    // Constructor/Destructor
    //--------------------------------------

    HFCService(const WString& pi_rServiceName,
               const WString& pi_rDisplayName,
               DWORD         pi_ControlsAccepted);
    virtual             ~HFCService();


    //--------------------------------------
    // Reports the status of this service back to the SCM
    //--------------------------------------

    bool               ReportStatusToSCM();
    bool               ReportStatusToSCM(DWORD pi_CurrentState,
                                          DWORD pi_Win32ExitCode,
                                          DWORD pi_ServiceSpecificExitCode,
                                          DWORD pi_CheckPoint,
                                          DWORD pi_WaitHint);


    //--------------------------------------
    // Service Control Manager Setup
    //--------------------------------------

    // Installs & uninstall the service
    virtual bool       Install();
    virtual bool       Uninstall();

    // Modifies the service
    virtual bool       ModifyUserAccount (bool         pi_SystemAccount,
                                           const WString& pi_rUserName,
                                           const WString& pi_rPassword);
    virtual bool       ModifyDependencies(const HFCServiceDependencies& pi_rDependencies);


    // Indicates if the service is installed in the SCM
    bool               IsInstalled() const;

    // Get the configuration for the service in the SCM.
    // Includes dependencies, user name and password
    bool               GetConfiguration(HFCServiceDependencies& po_rDependencies,
                                         bool&                  po_rSystemAccount,
                                         WString&                po_rUserName,
                                         WString&                po_rPassword) const;

    // Returns the current state of the service
    bool               GetCurrentState(DWORD& po_rState) const;



    //--------------------------------------
    //
    //--------------------------------------

    //Installs the callback funtion by calling RegisterServiceCtrlHandler
    bool               RegisterCtrlHandler();

    //Member function which does the job of responding to SCM requests
    virtual void WINAPI ServiceCtrlHandler(DWORD pi_Control);

    //The ServiceMain function for this service
    virtual void WINAPI ServiceMain(DWORD pi_Argc, LPWSTR* pi_ppArgv);

    //Called in reponse to a shutdown request
    virtual void        OnStop();

    //Called in reponse to a pause request
    virtual void        OnPause();

    //Called in reponse to a continue request
    virtual void        OnContinue();

    //Called in reponse to a Interrogate request
    virtual void        OnInterrogate();

    //Called in reponse to a Shutdown request
    virtual void        OnShutdown();

    //Called in reponse to a user defined request
    virtual void        OnUserDefinedRequest(DWORD pi_Control);

    // Send a user defined request to the running service
    virtual bool       ControlService(DWORD pi_Control);

    //Kicks off the Service. You would normally call this
    //somewhere in your main/wmain or InitInstance
    //a standard process rather than as a service. If you are
    //using the CNTServiceCommandLineInfo class, then internally
    //it will call this function for you.
    virtual bool       RunAsService();

    // Runs the service as a normal function as opposed to a service
    virtual void        RunAsConsole();

    //Displays help for this service
    virtual void        ShowHelp();

    // Indicates the run mode of the server
    bool               IsRunningAsService() const;

protected:
    //--------------------------------------
    //Methods
    //--------------------------------------

    //These two static functions are used internally to
    //go from the SDK functions to the C++ member functions
    static void WINAPI  _ServiceCtrlHandler  (DWORD pi_Control);
    static void WINAPI  _ServiceMain         (DWORD pi_Argc, LPWSTR* pi_ppArgv);

    //Used internally by the persistance functions
    HKEY                GetSectionKey(const WString& pi_rSection);
    HKEY                GetServiceRegistryKey();


    //--------------------------------------
    // Attributes
    //--------------------------------------

    // Indicates if the service is runned as a service or
    // a console
    bool               m_RunsAsService;

    SERVICE_STATUS_HANDLE
    m_hStatus;
    DWORD               m_ControlsAccepted;
    DWORD               m_CurrentState;
    WString             m_ServiceName;
    WString             m_DisplayName;
    static HFCService*  s_pService;
    HFCExclusiveKey     m_Key;
    };