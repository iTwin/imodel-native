/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/iModelJs/iModelJsApp.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <iModelJs/iModelJs.h>
#include <websocketpp/server.hpp>

/** @namespace BentleyApi::iModelJs::App The iModel.js application framework. */
#define BEGIN_BENTLEY_IMODELJS_APP_NAMESPACE  BEGIN_BENTLEY_IMODELJS_NAMESPACE namespace App {
#define END_BENTLEY_IMODELJS_APP_NAMESPACE    } END_BENTLEY_IMODELJS_NAMESPACE

#define IMODELJS_APP_TYPEDEFS(_name_) \
    BEGIN_BENTLEY_IMODELJS_APP_NAMESPACE DEFINE_POINTER_SUFFIX_TYPEDEFS(_name_) END_BENTLEY_IMODELJS_APP_NAMESPACE

#define IMODELJS_APP_REF_COUNTED_PTR(_sname_) \
    BEGIN_BENTLEY_IMODELJS_APP_NAMESPACE struct _sname_; DEFINE_REF_COUNTED_PTR(_sname_) END_BENTLEY_IMODELJS_APP_NAMESPACE

IMODELJS_APP_TYPEDEFS (Activity)
IMODELJS_APP_TYPEDEFS (Application)
IMODELJS_APP_TYPEDEFS (Delegate)
IMODELJS_APP_TYPEDEFS (Environment)
IMODELJS_APP_TYPEDEFS (NodeEnvironment)
IMODELJS_APP_TYPEDEFS (WebEnvironment)
IMODELJS_APP_TYPEDEFS (WebWorkerEnvironment)
IMODELJS_APP_TYPEDEFS (WebBrowserEnvironment)
IMODELJS_APP_TYPEDEFS (ElectronEnvironment)
IMODELJS_APP_TYPEDEFS (WebViewEnvironment)
IMODELJS_APP_TYPEDEFS (AndroidWebViewEnvironment)
IMODELJS_APP_TYPEDEFS (iOSWebViewEnvironment)
IMODELJS_APP_TYPEDEFS (UwpWebViewEnvironment)
IMODELJS_APP_TYPEDEFS (EmbeddedEnvironment)
IMODELJS_APP_TYPEDEFS (V8Environment)
IMODELJS_APP_TYPEDEFS (ChakraEnvironment)
IMODELJS_APP_TYPEDEFS (JscEnvironment)
IMODELJS_APP_TYPEDEFS (History)
IMODELJS_APP_TYPEDEFS (Host)
IMODELJS_APP_TYPEDEFS (InteractiveHost)
IMODELJS_APP_TYPEDEFS (WebBrowserHost)
IMODELJS_APP_TYPEDEFS (MobileAppHost)
IMODELJS_APP_TYPEDEFS (iOSAppHost)
IMODELJS_APP_TYPEDEFS (AndroidAppHost)
IMODELJS_APP_TYPEDEFS (UwpAppHost)
IMODELJS_APP_TYPEDEFS (DesktopAppHost)
IMODELJS_APP_TYPEDEFS (WindowsDesktopAppHost)
IMODELJS_APP_TYPEDEFS (MacDesktopAppHost)
IMODELJS_APP_TYPEDEFS (ServerHost)
IMODELJS_APP_TYPEDEFS (WindowsServerHost)
IMODELJS_APP_TYPEDEFS (LinuxServerHost)
IMODELJS_APP_TYPEDEFS (Service)
IMODELJS_APP_TYPEDEFS (Session)
IMODELJS_APP_TYPEDEFS (Transport)
IMODELJS_APP_TYPEDEFS (WebSocketsTransport)

BEGIN_BENTLEY_IMODELJS_APP_NAMESPACE

//=======================================================================================
//! A collection of application functionality in an environment.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
struct Activity
    {
    Activity() = delete;
    };

//=======================================================================================
//! An iModel.js application.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
struct Application
    {
    Application() = delete;
    };

//=======================================================================================
//! Acts on behalf of a service.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
struct Delegate
    {
    };

//=======================================================================================
//! An iModel.js runtime environment.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
struct Environment
    {
private:
    Environment (EnvironmentCR other) = delete;
    EnvironmentR operator= (EnvironmentCR other) = delete;
    Environment (Environment&& other) = delete;

protected:
    IMODELJS_EXPORT Environment() { ; }
    };

//=======================================================================================
//! An environment where Node.js is available.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
template <typename T>
struct INodeEnvironment : public T
    {
protected:
    INodeEnvironment() { ; }
    };

//=======================================================================================
//! The environment delivered by Node.js on a server host.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
struct NodeEnvironment : public INodeEnvironment<Environment>
    {
    };

//=======================================================================================
//! An environment where web technologies are available.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
template <typename T>
struct IWebEnvironment : public T
    {
protected:
    IWebEnvironment() { ; }
    };

//=======================================================================================
//! An environment where web rendering technologies like HTML, CSS, and WebGL are available.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
struct WebEnvironment : public IWebEnvironment<Environment>
    {
protected:
    IMODELJS_EXPORT WebEnvironment() { ; }
    };

//=======================================================================================
//! An environment where Web Workers technology is available.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
struct WebWorkersEnvironment : public IWebEnvironment<Environment>
    {
    WebWorkersEnvironment() = delete;
    };

//=======================================================================================
//! The environment delivered by a web browser host.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
struct WebBrowserEnvironment : public WebEnvironment
    {
    WebBrowserEnvironment() = delete;
    };

//=======================================================================================
//! The environment delivered by the Electron framework on a desktop app host.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
struct ElectronEnvironment : public INodeEnvironment<WebEnvironment>
    {
    };

//=======================================================================================
//! The environment delivered by a web content control on a mobile app host.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
struct WebViewEnvironment : public WebEnvironment
    {
protected:
    IMODELJS_EXPORT WebViewEnvironment() { ; }
    };

//=======================================================================================
//! An Android android.webkit.WebView.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
struct AndroidWebViewEnvironment : public WebViewEnvironment
    {
    };

//=======================================================================================
//! An iOS WKWebView.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
struct iOSWebViewEnvironment : public WebViewEnvironment
    {
    };

//=======================================================================================
//! A Universal Windows Platform Windows.UI.Xaml.Controls.Web​View.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
struct UwpWebViewEnvironment : public WebViewEnvironment
    {
    };

//=======================================================================================
//! An environment delivered by an embedded instance of a JavaScript runtime.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
struct EmbeddedEnvironment : public Environment
    {
protected:
    IMODELJS_EXPORT EmbeddedEnvironment() { ; }
    };

//=======================================================================================
//! A Google V8 engine environment.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
struct V8Environment : public EmbeddedEnvironment
    {
    };

//=======================================================================================
//! A Microsoft Chakra engine environment.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
struct ChakraEnvironment : public EmbeddedEnvironment
    {
    };

//=======================================================================================
//! A WebKit JavaScriptCore engine environment.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
struct JscEnvironment : public EmbeddedEnvironment
    {
    };

//=======================================================================================
//! A chronological sequence of activity instances in a session.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
struct History
    {
    History() = delete;
    };

//=======================================================================================
//! An iModel.js hosting context.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
struct Host
    {
private:
    static HostP s_instance;

    Host (HostCR other) = delete;
    HostR operator= (HostCR other) = delete;
    Host (Host&& other) = delete;

protected:
    IMODELJS_EXPORT Host();

public:
    IMODELJS_EXPORT static HostR GetInstance();

    IMODELJS_EXPORT ~Host();
    };

//=======================================================================================
//! A web browser session.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
struct InteractiveHost : public Host
    {
protected:
    IMODELJS_EXPORT InteractiveHost() { ; }
    };

//=======================================================================================
//! A host where user interaction is possible.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
struct WebBrowserHost : public InteractiveHost
    {
    WebBrowserHost() = delete;
    };

//=======================================================================================
//! A mobile application.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
struct MobileAppHost : public InteractiveHost
    {
protected:
    IMODELJS_EXPORT MobileAppHost() { ; }
    };

//=======================================================================================
//! An iOS application.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
struct iOSAppHost : public MobileAppHost
    {
    };

//=======================================================================================
//! An Android application.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
struct AndroidAppHost : public MobileAppHost
    {
    };

//=======================================================================================
//! A Universal Windows Platform application.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
struct UwpAppHost : public MobileAppHost
    {
    };

//=======================================================================================
//! A desktop application.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
struct DesktopAppHost : public InteractiveHost
    {
protected:
    IMODELJS_EXPORT DesktopAppHost() { ; }
    };

//=======================================================================================
//! A Windows application.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
struct WindowsDesktopAppHost : public InteractiveHost
    {
    IMODELJS_EXPORT WindowsDesktopAppHost();
    };

//=======================================================================================
//! A Mac application.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
struct MacDesktopAppHost : public InteractiveHost
    {
    };

//=======================================================================================
//! An HTTP server.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
struct ServerHost : public Host
    {
protected:
    IMODELJS_EXPORT ServerHost() { ; }
    };

//=======================================================================================
//! A Windows server.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
struct WindowsServerHost : public ServerHost
    {
    };

//=======================================================================================
//! A Linux server.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
struct LinuxServerHost : public ServerHost
    {
    };

//=======================================================================================
//! An iModel.js service.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
struct Service
    {
    Service() = delete;
    };

//=======================================================================================
//! A period of application usage on a host.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
struct Session
    {
    Session() = delete;
    };

//=======================================================================================
//! A mechanism for exchanging data between environments.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
struct Transport
    {
protected:
    IMODELJS_EXPORT Transport() { ; }
    };

//=======================================================================================
//! A transport that uses WebSockets technology.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
struct WebSocketsTransport : public Transport
    {
public:
    IMODELJS_EXPORT WebSocketsTransport();
    };

END_BENTLEY_IMODELJS_APP_NAMESPACE

//__PUBLISH_SECTION_END__
