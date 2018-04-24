/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/iModelJsServicesTierTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../Environment/PublicAPI/TestEnvironment.h"

USING_IMODELJS_NAMESPACE
USING_IMODELJS_UNIT_TESTS_NAMESPACE

static std::atomic<size_t> s_registeredExtensions;
static std::atomic<size_t> s_installedExtensions;
static std::atomic<size_t> s_startedTests;
static std::atomic<size_t> s_finishedTests;
static std::atomic<bool> s_wait;

namespace {

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
static void OnTestStarted()
    {
    ++s_startedTests;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
static void OnTestFinished()
    {
    ++s_finishedTests;
    }

//=======================================================================================
// @bsiclass                                                    Steve.Wilson   7/17
//=======================================================================================
struct TrivialExtension : public ServicesTier::Extension
    {
private:
    Utf8CP m_name;
    double m_value;

    Utf8CP SupplyName() const override { return m_name; }
    Napi::Value ExportJsModule (Js::RuntimeR runtime) override { return Napi::Number::New(runtime.Env(), m_value); }
    TrivialExtension (Utf8CP name, double value) : m_name (name), m_value (value) { ; }

public:
    //---------------------------------------------------------------------------------------
    // @bsimethod                                Steve.Wilson                    7/2017
    //---------------------------------------------------------------------------------------
    static void InstallInstance (Utf8CP name, double value)
        {
        ++s_registeredExtensions;
        ServicesTier::Extension::Install ([=]() { ++s_installedExtensions; return new TrivialExtension (name, value); });
        }
    };

//=======================================================================================
// @bsiclass                                                    Steve.Wilson   7/17
//=======================================================================================
struct Addon
    {
private:
    //---------------------------------------------------------------------------------------
    // @bsimethod                                Steve.Wilson                    7/2017
    //---------------------------------------------------------------------------------------
    static void Utilities (Js::RuntimeR runtime)
        {
        OnTestStarted();

        auto evaluateResult = runtime.EvaluateScript (u8R"(
            (function() {
                let info = bentley.imodeljs.servicesTier.getHostInfo();
                BeAssert (typeof (info.argv) !== "undefined");

                bentley.imodeljs.servicesTier.scheduleIdleCallback (function() {
                    __iModelJsServicesTierTests_OnTestFinished();
                });
            })();
        )", "iModelJsServicesTierTests:///Addon.Utilities.js");

        BeAssert (evaluateResult.status == Js::EvaluateStatus::Success);
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                Steve.Wilson                    7/2017
    //---------------------------------------------------------------------------------------
    static void Test1 (Js::RuntimeR runtime)
        {
        OnTestStarted();

        auto evaluateResult = runtime.EvaluateScript (u8R"(
            (function() {
                let require = bentley.imodeljs.servicesTier.require;
                // let addon = require ("@bentley/imodeljs-mobile");
                let addon = self.imodeljsMobile.imodeljsNative;
                BeAssert (addon !== undefined);
           })();
        )", "iModelJsServicesTierTests:///Addon.Test1.js");

        BeAssert (evaluateResult.status == Js::EvaluateStatus::Success);

        OnTestFinished();
        }

public:
    //---------------------------------------------------------------------------------------
    // @bsimethod                                Steve.Wilson                    7/2017
    //---------------------------------------------------------------------------------------
    static void PerformTests (Js::RuntimeR runtime)
        {
        Utilities (runtime);
        Test1 (runtime);
        }
    };


//=======================================================================================
// @bsiclass                                                    Steve.Wilson   7/17
//=======================================================================================
struct Core
    {
private:
    //---------------------------------------------------------------------------------------
    // @bsimethod                                Steve.Wilson                    7/2017
    //---------------------------------------------------------------------------------------
    static void Utilities (Js::RuntimeR runtime)
        {
        OnTestStarted();

        auto evaluateResult = runtime.EvaluateScript (u8R"(
            (function() {
                let info = bentley.imodeljs.servicesTier.getHostInfo();
                BeAssert (typeof (info.argv) !== "undefined");

                bentley.imodeljs.servicesTier.scheduleIdleCallback (function() {
                    __iModelJsServicesTierTests_OnTestFinished();
                });
            })();
        )", "iModelJsServicesTierTests:///Core.Utilities.js");

        BeAssert (evaluateResult.status == Js::EvaluateStatus::Success);
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                Steve.Wilson                    7/2017
    //---------------------------------------------------------------------------------------
    static void Test1 (Js::RuntimeR runtime)
        {
        OnTestStarted();

        auto evaluateResult = runtime.EvaluateScript (u8R"(
            (function() {
                let require = bentley.imodeljs.servicesTier.require;

                let before = require ("__iModelJsServicesTierTests_BeforeHostExists");
                let after = require ("__iModelJsServicesTierTests_AfterHostExists");

                BeAssert (before === 1.0);
                BeAssert (after === 2.0);
            })();
        )", "iModelJsServicesTierTests:///Core.Test1.js");

        BeAssert (evaluateResult.status == Js::EvaluateStatus::Success);

        OnTestFinished();
        }

public:
    //---------------------------------------------------------------------------------------
    // @bsimethod                                Steve.Wilson                    7/2017
    //---------------------------------------------------------------------------------------
    static void PerformTests (Js::RuntimeR runtime)
        {
        Utilities (runtime);
        Test1 (runtime);
        }
    };

//=======================================================================================
// @bsiclass                                                    Steve.Wilson   7/17
//=======================================================================================
struct ServicesTierUtilities
    {
private:
    //---------------------------------------------------------------------------------------
    // @bsimethod                                Steve.Wilson                    7/2017
    //---------------------------------------------------------------------------------------
    static void BasicTcp (Js::RuntimeR runtime)
        {
        OnTestStarted();

        auto evaluateResult = runtime.EvaluateScript (u8R"(
            (async function() {
                let require = bentley.imodeljs.servicesTier.require;

                let utilities = require ("@bentley/imodeljs-services-tier-utilities");
                let address = "127.0.0.1";
                let port = 8378;

                try {
                    let bindResult = utilities.uv.tcp.bind (address, port, utilities.uv.net.IP.V4);
                    BeAssert (bindResult.success());
                    BeAssert (bindResult.server !== null);
                    bentley.imodeljs.servicesTier.registerShutdownHandler (function() { bindResult.server.close(); });

                    let listenResult = bindResult.server.listen (1024, async function (connection, status) {
                        BeAssert (status.success());
                        bentley.imodeljs.servicesTier.registerShutdownHandler (function() { connection.close(); });

                        let acceptResult = this.accept (connection);
                        BeAssert (acceptResult.success());
                
                        let writeResult = await connection.write (new Uint8Array ([1, 2, 3]).buffer);
                        BeAssert (writeResult.success());
                    });

                    BeAssert (listenResult.success());

                    let connectResult = await utilities.uv.tcp.connect (address, port, utilities.uv.net.IP.V4);
                    BeAssert (connectResult.success());
                    BeAssert (connectResult.connection !== null);
                    bentley.imodeljs.servicesTier.registerShutdownHandler (function() { connectResult.connection.close(); });

                    connectResult.connection.read (utilities.uv.io.createDefaultStreamAllocator(), function (status, data, nread) {
                        BeAssert (status.success());
                        BeAssert (nread === 3);

                        let view = new Uint8Array (data);
                        BeAssert (view [0] === 1 &&
                                  view [1] === 2 &&
                                  view [2] === 3);

                        __iModelJsServicesTierTests_OnTestFinished();

                        return true;
                        });
                } catch (e) {
                    BeAssert (false, e);
                    __iModelJsServicesTierTests_OnTestFinished();
                }
            })();
        )", "iModelJsServicesTierTests:///ServicesTierUtilities.BasicTcp.js");

        BeAssert (evaluateResult.status == Js::EvaluateStatus::Success);
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                Steve.Wilson                    7/2017
    //---------------------------------------------------------------------------------------
    static void BasicWebSockets (Js::RuntimeR runtime)
        {
        OnTestStarted();

        auto evaluateResult = runtime.EvaluateScript (u8R"(
            (async function() {
                let require = bentley.imodeljs.servicesTier.require;

                let utilities = require ("@bentley/imodeljs-services-tier-utilities");
                let address = "127.0.0.1";
                let port = 9327;

                try {
                    let websocketServer = new utilities.websocketpp.ServerEndpoint();
                    
                    let bindResult = utilities.uv.tcp.bind (address, port, utilities.uv.net.IP.V4);
                    BeAssert (bindResult.success());
                    bentley.imodeljs.servicesTier.registerShutdownHandler (function() { bindResult.server.close(); });

                    let listenResult = bindResult.server.listen (1024, async function (connection, status) {
                        BeAssert (status.success());
                        bentley.imodeljs.servicesTier.registerShutdownHandler (function() { connection.close(); });

                        let acceptResult = this.accept (connection);
                        BeAssert (acceptResult.success());

                        let websocketClient = websocketServer.createConnection();
                        BeAssert (websocketClient !== null);

                        websocketClient.handler = {
                            message: function (data, code) {
                                BeAssert (code == utilities.websocketpp.OpCode.Text);

                                let expectedMessage = utilities.abtostr8 (data, 0, data.byteLength);
                                BeAssert (expectedMessage === "abc");
                                __iModelJsServicesTierTests_OnTestFinished();
                            },

                            transport: async function (data) {
                                let writeResult = await connection.write (data);
                                BeAssert (writeResult.success());
                            }
                        };

                        connection.read (utilities.uv.io.createDefaultStreamAllocator(), function (status, data, nread) {
                            BeAssert (status.success());
                            let processed = websocketClient.process (data, 0, nread);
                            BeAssert (processed);

                            return true;
                        });
                    });

                    BeAssert (listenResult.success());

                    let connectResult = await utilities.uv.tcp.connect (address, port, utilities.uv.net.IP.V4);
                    BeAssert (connectResult.success());
                    bentley.imodeljs.servicesTier.registerShutdownHandler (function() { connectResult.connection.close(); });

                    connectResult.connection.read (utilities.uv.io.createDefaultStreamAllocator(), async function (status, data, nread) {
                        BeAssert (status.success());

                        let upgradeResponse = utilities.abtostr8 (data, 0, nread);
                        let expected = "HTTP/1.1 101 Switching Protocols\r\nConnection: upgrade\r\nSec-WebSocket-Accept: s3pPLMBiTxaQ9kYGzzhZRbK+xOo=\r\nServer: WebSocket++/0.7.0\r\nUpgrade: websocket\r\n\r\n";
                        BeAssert (upgradeResponse === expected);

                        let message = [0b10000001, 0b10000011, 0, 0, 0, 0, 0x61, 0x62, 0x63];
                        let writeMessage = await connectResult.connection.write (new Uint8Array (message).buffer);
                        BeAssert (writeMessage.success());

                        return true;
                    });

                    let upgrade = "GET / HTTP/1.1\r\nHost: " + address + ":" + port + "\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\nSec-WebSocket-Version: 13\r\n\r\n";
                    let writeUpgrade = await connectResult.connection.write (utilities.strtoab8 (upgrade));
                    BeAssert (writeUpgrade.success());
                    
                } catch (e) {
                    BeAssert (false, e);
                    __iModelJsServicesTierTests_OnTestFinished();
                }
            })();
        )", "iModelJsServicesTierTests:///ServicesTierUtilities.BasicWebSockets.js");

        BeAssert (evaluateResult.status == Js::EvaluateStatus::Success);
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                Steve.Wilson                    7/2017
    //---------------------------------------------------------------------------------------
    static void BasicFileSystem (Js::RuntimeR runtime)
        {
        OnTestStarted();

        auto evaluateResult = runtime.EvaluateScript (u8R"(
            (function() {
                let require = bentley.imodeljs.servicesTier.require;

                let utilities = require ("@bentley/imodeljs-services-tier-utilities");

                let path = "c:\\test.txt";

                let file = utilities.fs.open (path, utilities.fs.O_RDONLY, 0);
                if (utilities.fs.isValid (file))
                    {
                    let info = utilities.fs.stat (file);
                    BeAssert (info !== null);
                    let contents = new ArrayBuffer (info.size);
                    let read = utilities.fs.read (file, contents);
                    BeAssert (read);
                    BeAssert (contents.byteLength === info.size);
                    let contentsStr = utilities.abtostr8 (contents, 0, contents.byteLength);
                    BeAssert (contentsStr == "test");
                    let closed = utilities.fs.close (file);
                    BeAssert (closed);
                    }
            })();
        )", "iModelJsServicesTierTests:///ServicesTierUtilities.BasicFileSystem.js");
        
        BeAssert (evaluateResult.status == Js::EvaluateStatus::Success);

        OnTestFinished();
        }

    DEFINE_POINTER_SUFFIX_TYPEDEFS (TestMobileClient)

    //---------------------------------------------------------------------------------------
    // @bsimethod                                Steve.Wilson                    7/2017
    //---------------------------------------------------------------------------------------
    static void MobileGatewayPOC (Js::RuntimeR runtime)
        {
        using namespace BentleyApi::iModelJs::ServicesTier;
        
        OnTestStarted();

        auto evaluateResult = runtime.EvaluateScript (u8R"(
            (function() {
                let require = bentley.imodeljs.servicesTier.require;

                let mobilegateway = require ("@bentley/imodeljs-mobilegateway");
                BeAssert (mobilegateway.handler === null);
                BeAssert (typeof (mobilegateway.port) === "number");
                BeAssert (typeof (mobilegateway.send) === "function");

                mobilegateway.handler = (payload) => {
                    BeAssert (payload === "abc");
                    mobilegateway.send ("xyz");
                };
            })();
        )", "iModelJsServicesTierTests:///ServicesTierUtilities.MobileGatewayPOC.js");
        
        BeAssert (evaluateResult.status == Js::EvaluateStatus::Success);

        auto connectResult = Uv::Connect ("127.0.0.1", MobileGateway::GetInstance().GetPort(), Uv::IP::V4, [](Uv::StatusCR status, Uv::TcpHandlePtr connection)
            {
            BeAssert (!status.IsError());
            connection->AddRef();

            auto& cref = *connection;
            auto readResult = connection->Read ([&cref](Uv::StatusCR status, uv_buf_t const& buffer, size_t nread)
                {
                BeAssert (!status.IsError());

                if (nread == 5)
                    {
                    unsigned char expected [5];
                    expected [0] = 0x81;
                    expected [1] = 0x3;
                    expected [2] = 0x78;
                    expected [3] = 0x79;
                    expected [4] = 0x7a;
                    
                    BeAssert (strncmp ((const char*) expected, buffer.base, nread) == 0);

                    cref.Release();
                    OnTestFinished();
                    return false;
                    }

                auto expected = "HTTP/1.1 101 Switching Protocols\r\nConnection: upgrade\r\nSec-WebSocket-Accept: s3pPLMBiTxaQ9kYGzzhZRbK+xOo=\r\nServer: WebSocket++/0.7.0\r\nUpgrade: websocket\r\n\r\n";
                BeAssert (strncmp (expected, buffer.base, nread) == 0);

                unsigned char message [9];
                message [0] = 0x81;
                message [1] = 0x83;
                message [2] = 0;
                message [3] = 0;
                message [4] = 0;
                message [5] = 0;
                message [6] = 0x61;
                message [7] = 0x62;
                message [8] = 0x63;

                auto writeMessageResult = cref.Write (message, sizeof (message), [](Uv::StatusCR status) { BeAssert (!status.IsError()); });

                BeAssert (!writeMessageResult.IsError());

                return true;
                });

            BeAssert (!readResult.IsError());

            auto upgrade = "GET / HTTP/1.1\r\nHost: 127.0.0.1:9327\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\nSec-WebSocket-Version: 13\r\n\r\n";
            auto writeUpgradeResult = connection->Write (upgrade, strlen (upgrade), [](Uv::StatusCR status) { BeAssert (!status.IsError()); });
            BeAssert (!writeUpgradeResult.IsError());
            });

        BeAssert (!connectResult.IsError());
        }

public:
    //---------------------------------------------------------------------------------------
    // @bsimethod                                Steve.Wilson                    7/2017
    //---------------------------------------------------------------------------------------
    static void PerformTests (Js::RuntimeR runtime)
        {
#ifdef COMMENT_OUT
        BasicTcp (runtime);
        BasicWebSockets (runtime);
#endif
        BasicFileSystem (runtime);
        MobileGatewayPOC (runtime);
        }
    };


//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
static void BeforeHostExists()
    {
    BeAssert (!ServicesTier::Host::Exists());

    s_registeredExtensions = 0;
    s_installedExtensions = 0;

    TrivialExtension::InstallInstance ("__iModelJsServicesTierTests_BeforeHostExists", 1.0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
static void AfterHostExists()
    {
    BeAssert (ServicesTier::Host::Exists());

    TrivialExtension::InstallInstance ("__iModelJsServicesTierTests_AfterHostExists", 2.0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
static void BeforeAllTests()
    {
    iModelJsTestFixture::ResetFailedJsAssertCount();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
static void AfterAllTests()
    {
    BeAssert (s_registeredExtensions == s_installedExtensions);
    BeAssert (iModelJsTestFixture::GetFailedJsAssertCount() == 0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
static void Setup (Js::RuntimeR runtime)
    {
    iModelJsTestFixture::InstallTestingUtilities (runtime);

    runtime.Env().Global().Set ("__iModelJsServicesTierTests_OnTestFinished", Napi::Function::New(runtime.Env(), [](Napi::CallbackInfo const& info) -> Napi::Value
        {
        OnTestFinished();

        return info.Env().Undefined();
        }));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
static void PerformTests()
    {
    s_startedTests = 0;
    s_finishedTests = 0;
    s_wait = true;

    ServicesTier::Host::GetInstance().PostToEventLoop([]()
        {
        auto& runtime = ServicesTier::Host::GetInstance().GetJsRuntime();
        Napi::HandleScope scope(runtime.Env());

        Setup(runtime);
        Addon::PerformTests(runtime);
        Core::PerformTests(runtime);
        ServicesTierUtilities::PerformTests(runtime);
        s_wait = false;
        });

    while (s_wait || s_startedTests != s_finishedTests) { ; }
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
TEST_F (iModelJsTestFixture, ServicesTier)
    {
    if (IsStandalone())
        {
        ServicesTier::UvHost host;
        RunSystemMessageLoop();
        }
    else
        {
        BeforeAllTests();

        {
            BeforeHostExists();
            ServicesTier::UvHost host;
            while (!host.IsReady()) { ; }
            AfterHostExists();
            PerformTests();
        }

        AfterAllTests();
        }
    }
