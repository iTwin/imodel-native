/*--------------------------------------------------------------------------------------+
|
|     $Source: ServicesTierUtilities.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ServicesTierUtilities.h"
#include <Bentley/Desktop/FileSystem.h>

BEGIN_BENTLEY_IMODELJS_SERVICES_TIER_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Napi::Function Utilities::uv_fs_open (Napi::Env& env)
    {
    return Napi::Function::New(env, [](Napi::CallbackInfo const& info) -> Napi::Value
        {
        JS_CALLBACK_REQUIRE_AT_LEAST_N_ARGS (3);

        auto path  = JS_CALLBACK_GET_STRING (0);
        auto flags = JS_CALLBACK_GET_NUMBER (1);
        auto mode  = JS_CALLBACK_GET_NUMBER (2);

        uv_fs_t req;
        ::uv_fs_open (Host::GetInstance().GetEventLoop(), &req, path.Utf8Value().c_str(), static_cast<int>(flags.Int32Value()), static_cast<int>(mode.Int32Value()), nullptr);
        uv_fs_req_cleanup (&req);

        return JS_CALLBACK_NUMBER (static_cast<double>(req.result));
        });
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Napi::Function Utilities::uv_fs_realpath (Napi::Env& env)
    {
    return Napi::Function::New(env, [](Napi::CallbackInfo const& info) -> Napi::Value
        {
        JS_CALLBACK_REQUIRE_AT_LEAST_N_ARGS (1);

        auto path  = JS_CALLBACK_GET_STRING (0);

#ifdef USE_UV_REALPATH
        uv_fs_t req;
        ::uv_fs_realpath (Host::GetInstance().GetEventLoop(), &req, path.Utf8Value().c_str(), nullptr);
        auto newPath = Napi::String::New(info.Env(), req.path);
        uv_fs_req_cleanup (&req);
#else
        BeFileName wpath(path.Utf8Value().c_str(), true);
        BeFileName wpathresolved;
        if (wpath.IsSymbolicLink())
            {
            BeFileName::GetTargetOfSymbolicLink(wpathresolved, wpath.c_str(), true);
            wpath = wpathresolved;
            }
        WString fixedWpath;
        BeFileName::FixPathName(fixedWpath, wpath.c_str(), true);
        fixedWpath.ReplaceAll(L"\\", L"/");
        Utf8String fixedPath(fixedWpath);
        auto newPath = Napi::String::New(info.Env(), fixedPath.c_str());
#endif

        return newPath;
        });
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Napi::Function Utilities::uv_fs_stat (Napi::Env& env)
    {
    return Napi::Function::New(env, [](Napi::CallbackInfo const& info) -> Napi::Value
        {
        JS_CALLBACK_REQUIRE_AT_LEAST_N_ARGS (1);

        auto file = JS_CALLBACK_GET_NUMBER (0);

        uv_fs_t req;
        auto result = ::uv_fs_fstat (Host::GetInstance().GetEventLoop(), &req, static_cast<uv_file>(file.Int32Value()), nullptr);
        
        if (result < 0)
            {
            uv_fs_req_cleanup (&req);

            return JS_CALLBACK_NULL;
            }
        else
            {
            auto object = JS_CALLBACK_OBJECT;
            object.Set ("size", JS_CALLBACK_NUMBER (static_cast<double>(req.statbuf.st_size)));
            object.Set ("isFile", JS_CALLBACK_BOOLEAN ((req.statbuf.st_mode & S_IFREG) == S_IFREG));
            object.Set ("isDirectory", JS_CALLBACK_BOOLEAN ((req.statbuf.st_mode & S_IFDIR) == S_IFDIR));

            uv_fs_req_cleanup (&req);

            return object;
            }
        });
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Napi::Function Utilities::uv_fs_read (Napi::Env& env)
    {
    return Napi::Function::New(env, [](Napi::CallbackInfo const& info) -> Napi::Value
        {
        JS_CALLBACK_REQUIRE_AT_LEAST_N_ARGS (2);

        auto file = JS_CALLBACK_GET_NUMBER (0);
        auto destination = JS_CALLBACK_GET_ARRAY_BUFFER (1);

        uv_fs_t req;
        auto buf = uv_buf_init (static_cast<char*>(destination.Data()), static_cast<unsigned int>(destination.ByteLength()));
        auto result = ::uv_fs_read (Host::GetInstance().GetEventLoop(), &req, static_cast<uv_file>(file.Uint32Value()), &buf, 1, 0, nullptr);
        uv_fs_req_cleanup (&req);

        return JS_CALLBACK_BOOLEAN (result >= 0);
        });
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Napi::Function Utilities::uv_fs_close (Napi::Env& env)
    {
    return Napi::Function::New(env, [](Napi::CallbackInfo const& info) -> Napi::Value
        {
        JS_CALLBACK_REQUIRE_AT_LEAST_N_ARGS (1);

        auto file = JS_CALLBACK_GET_NUMBER (0);

        uv_fs_t req;
        auto result = ::uv_fs_close (Host::GetInstance().GetEventLoop(), &req, static_cast<uv_file>(file.Uint32Value()), nullptr);
        uv_fs_req_cleanup (&req);

        return JS_CALLBACK_BOOLEAN (result >= 0);
        });
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Napi::Object Utilities::CreateInitParams(Napi::Env& env)
    {
    auto params = Napi::Object::New(env);
#ifdef COMMENT_OUT
    params.Set ("uv_Handle_close",                             uv_Handle_close (env));
    params.Set ("uv_io_shutdown",                              uv_io_shutdown (env));
    params.Set ("uv_io_Stream_isReadable",                     uv_io_Stream_isReadable (env));
    params.Set ("uv_io_Stream_isWritable",                     uv_io_Stream_isWritable (env));
    params.Set ("uv_io_Stream_read",                           uv_io_Stream_read (env));
    params.Set ("uv_io_Stream_write",                          uv_io_Stream_write (env));
    params.Set ("uv_tcp_bind",                                 uv_tcp_bind (env));
    params.Set ("uv_tcp_connect",                              uv_tcp_connect (env));
    params.Set ("uv_tcp_Handle_setNoDelay",                    uv_tcp_Handle_setNoDelay (env));
    params.Set ("uv_tcp_Handle_setKeepAlive",                  uv_tcp_Handle_setKeepAlive (env));
    params.Set ("uv_tcp_Server_setSimultaneousAccepts",        uv_tcp_Server_setSimultaneousAccepts (env));
    params.Set ("uv_tcp_Server_listen",                        uv_tcp_Server_listen (env));
    params.Set ("uv_tcp_Server_accept",                        uv_tcp_Server_accept (env));
    params.Set ("websocketpp_Base_Dispose",                    websocketpp_Base_Dispose (env));
    params.Set ("websocketpp_ClientConnection_process",        websocketpp_ClientConnection_process (env));
    params.Set ("websocketpp_ClientConnection_send",           websocketpp_ClientConnection_send (env));
    params.Set ("websocketpp_ServerEndpoint_constructor",      websocketpp_ServerEndpoint_constructor (env));
    params.Set ("websocketpp_ServerEndpoint_createConnection", websocketpp_ServerEndpoint_createConnection (env));
#endif
    params.Set ("uv_fs_open",                                  uv_fs_open (env));
    params.Set ("uv_fs_realpath",                              uv_fs_realpath (env));
    params.Set ("uv_fs_stat",                                  uv_fs_stat (env));
    params.Set ("uv_fs_read",                                  uv_fs_read (env));
    params.Set ("uv_fs_close",                                 uv_fs_close (env));

    static_assert (O_RDONLY     <= Js::MAX_SAFE_JS_INTEGER, "Js");
    static_assert (O_WRONLY     <= Js::MAX_SAFE_JS_INTEGER, "Js");
    static_assert (O_RDWR       <= Js::MAX_SAFE_JS_INTEGER, "Js");
    static_assert (O_APPEND     <= Js::MAX_SAFE_JS_INTEGER, "Js");
    static_assert (O_CREAT      <= Js::MAX_SAFE_JS_INTEGER, "Js");
    static_assert (O_TRUNC      <= Js::MAX_SAFE_JS_INTEGER, "Js");
    static_assert (O_EXCL       <= Js::MAX_SAFE_JS_INTEGER, "Js");
#ifdef _WIN32
    static_assert (O_TEXT       <= Js::MAX_SAFE_JS_INTEGER, "Js");
    static_assert (O_BINARY     <= Js::MAX_SAFE_JS_INTEGER, "Js");
    static_assert (O_RAW        <= Js::MAX_SAFE_JS_INTEGER, "Js");
    static_assert (O_TEMPORARY  <= Js::MAX_SAFE_JS_INTEGER, "Js");
    static_assert (O_NOINHERIT  <= Js::MAX_SAFE_JS_INTEGER, "Js");
    static_assert (O_SEQUENTIAL <= Js::MAX_SAFE_JS_INTEGER, "Js");
    static_assert (O_RANDOM     <= Js::MAX_SAFE_JS_INTEGER, "Js");
#endif
    static_assert (S_IFMT   <= Js::MAX_SAFE_JS_INTEGER, "Js");
    static_assert (S_IFDIR  <= Js::MAX_SAFE_JS_INTEGER, "Js");
    static_assert (S_IFCHR  <= Js::MAX_SAFE_JS_INTEGER, "Js");
    static_assert (S_IFREG  <= Js::MAX_SAFE_JS_INTEGER, "Js");
    static_assert (S_IREAD  <= Js::MAX_SAFE_JS_INTEGER, "Js");
    static_assert (S_IWRITE <= Js::MAX_SAFE_JS_INTEGER, "Js");
    static_assert (S_IEXEC  <= Js::MAX_SAFE_JS_INTEGER, "Js");

    params.Set ("uv_fs_O_RDONLY",     Napi::Number::New(env, O_RDONLY));
    params.Set ("uv_fs_O_WRONLY",     Napi::Number::New(env, O_WRONLY));
    params.Set ("uv_fs_O_RDWR",       Napi::Number::New(env, O_RDWR));
    params.Set ("uv_fs_O_APPEND",     Napi::Number::New(env, O_APPEND));
    params.Set ("uv_fs_O_CREAT",      Napi::Number::New(env, O_CREAT));
    params.Set ("uv_fs_O_TRUNC",      Napi::Number::New(env, O_TRUNC));
    params.Set ("uv_fs_O_EXCL",       Napi::Number::New(env, O_EXCL));
#ifdef _WIN32
    params.Set ("uv_fs_O_TEXT",       Napi::Number::New(env, O_TEXT));
    params.Set ("uv_fs_O_BINARY",     Napi::Number::New(env, O_BINARY));
    params.Set ("uv_fs_O_RAW",        Napi::Number::New(env, O_RAW));
    params.Set ("uv_fs_O_TEMPORARY",  Napi::Number::New(env, O_TEMPORARY));
    params.Set ("uv_fs_O_NOINHERIT",  Napi::Number::New(env, O_NOINHERIT));
    params.Set ("uv_fs_O_SEQUENTIAL", Napi::Number::New(env, O_SEQUENTIAL));
    params.Set ("uv_fs_O_RANDOM",     Napi::Number::New(env, O_RANDOM));
#endif
    params.Set ("uv_fs_S_IFMT",   Napi::Number::New(env, S_IFMT));
    params.Set ("uv_fs_S_IFDIR",  Napi::Number::New(env, S_IFDIR));
    params.Set ("uv_fs_S_IFCHR",  Napi::Number::New(env, S_IFCHR));
    params.Set ("uv_fs_S_IFREG",  Napi::Number::New(env, S_IFREG));
    params.Set ("uv_fs_S_IREAD",  Napi::Number::New(env, S_IREAD));
    params.Set ("uv_fs_S_IWRITE", Napi::Number::New(env, S_IWRITE));
    params.Set ("uv_fs_S_IEXEC",  Napi::Number::New(env, S_IEXEC));

    return params;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void Utilities::FindPrototypes (Napi::Object exports)
    {
#ifdef COMMENT_OUT
    auto& env = Env();
    
    auto exports_uv = exports.Get("uv").As<Object>();
    m_prototypes.uv_Handle.Assign(env, exports_uv.Get("Handle").As<Object>().Get("prototype"));
    m_prototypes.uv_Status.Assign(env, exports_uv.Get("Status").As<Object>().Get("prototype"));
    
    auto exports_uv_io = exports_uv.Get ("io");
    m_prototypes.uv_io_Stream.Assign            (env, exports_uv_io.Get ("Stream").Get ("prototype"));
    
    auto exports_uv_tcp = exports_uv.Get ("tcp");
    m_prototypes.uv_tcp_BindResult.Assign    (env, exports_uv_tcp.Get ("BindResult").Get ("prototype"));
    m_prototypes.uv_tcp_ConnectResult.Assign (env, exports_uv_tcp.Get ("ConnectResult").Get ("prototype"));
    m_prototypes.uv_tcp_Handle.Assign        (env, exports_uv_tcp.Get ("Handle").Get ("prototype"));
    m_prototypes.uv_tcp_Server.Assign        (env, exports_uv_tcp.Get ("Server").Get ("prototype"));

    auto exports_websocketpp = exports.Get ("websocketpp");
    m_prototypes.websocketpp_Base.Assign (env, exports_websocketpp.Get ("Base").Get ("prototype"));
    m_prototypes.websocketpp_ClientConnection.Assign (env, exports_websocketpp.Get ("ClientConnection").Get ("prototype"));
    m_prototypes.websocketpp_ServerEndpoint.Assign   (env, exports_websocketpp.Get ("ServerEndpoint").Get ("prototype"));
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Napi::Function Utilities::EvaluateSimpleInitScript (Js::RuntimeR runtime)
    {
    auto evaluation = runtime.EvaluateScript (SimpleInitScript());
    BeAssert (evaluation.status == Js::EvaluateStatus::Success);

    return evaluation.value.As<Napi::Function>();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Napi::Value Utilities::ExportJsModule(Js::RuntimeR runtime)
    {
    auto initScript = EvaluateSimpleInitScript(runtime);//EvaluateInitScript(runtime);
    auto initParams = CreateInitParams(runtime.Env());

    auto exports = initScript({initParams}).As<Napi::Object>();
    FindPrototypes (exports);

    return exports;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Uv::AddressDescriptor::AddressDescriptor (Utf8CP address, uint16_t port, IP protocol, int& status)
    : m_protocol (protocol)
    {
    if (m_protocol == IP::V6)
        status = uv_ip6_addr (address, port, &m_addr6);
    else
        status = uv_ip4_addr (address, port, &m_addr4);

    BeAssert (status >= 0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
const sockaddr* Uv::AddressDescriptor::GetPointer() const
    {
    if (m_protocol == IP::V6)
        return reinterpret_cast<const sockaddr*>(&m_addr6);
    else
        return reinterpret_cast<const sockaddr*>(&m_addr4);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Uv::Handle::Handle (uv_handle_t* handle)
    : m_handle (handle)
    {
    m_handle->data = this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Uv::Handle::Handle (Handle&& other)
    : m_handle (other.m_handle)
    {
    other.m_handle = nullptr;
    m_handle->data = this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Uv::Handle::~Handle()
    {
    Close();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void Uv::Handle::CloseHandler (uv_handle_t* handle)
    {
    free (handle);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void Uv::Handle::Close()
    {
    if (m_handle == nullptr)
        return;

    auto handle = m_handle;
    m_handle = nullptr;
    ::uv_close (handle, &CloseHandler);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
uv_stream_t* Uv::IoStream::GetPointer() const
    {
    auto handle = Handle::GetPointer();
    BeAssert (handle->type == UV_STREAM || handle->type == UV_TCP);

    return reinterpret_cast<uv_stream_t*>(handle);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
uv_handle_t* Uv::IoStream::GetHandlePointer() const
    {
    return static_cast<HandleCP>(this)->GetPointer();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Uv::Status Uv::IoStream::Read (Read_Callback_T const& callback)
    {
    m_callback = callback;
    return ::uv_read_start (GetPointer(), &AllocHandler, &ReadHandler);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
bool Uv::IoStream::IsReadable() const
    {
    return ::uv_is_readable (GetPointer()) == 1;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
bool Uv::IoStream::IsWritable() const
    {
    return ::uv_is_writable (GetPointer()) == 1;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Uv::ShutdownResult Uv::IoStream::Shutdown()
    {
    int status;

    auto request = ShutdownRequest::Create (*this, status);
    if (status >= 0)
        return ShutdownResult (status, request.get());

    BeAssert (status < 0);
    return ShutdownResult (status, nullptr);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void Uv::IoStream::AllocHandler (uv_handle_t* handle, size_t suggestedSize, uv_buf_t* buf)
    {
    buf->base = reinterpret_cast<char*>(malloc (suggestedSize));
    buf->len = suggestedSize;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void Uv::IoStream::ReadHandler (uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf)
    {
    BeAssert (stream->data != nullptr);

    auto& instance = *reinterpret_cast<IoStreamP>(stream->data);

    if (Host::GetInstance().IsStopped())
        instance.Close();
    else if (!instance.m_callback (Status (nread), *buf, nread))
        ::uv_read_stop (stream);

    free (buf->base);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Uv::WriteResult Uv::IoStream::Write (unsigned char* data, size_t length, Write_Callback_T const& callback)
    {
    int status;
    auto request = WriteRequest::Create (*this, data, length, callback, status);
    if (status >= 0)
        return WriteResult (status, request.get());

    BeAssert (status < 0);
    return WriteResult (status, nullptr);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Uv::WriteResult Uv::IoStream::Write (const char* data, size_t length, Write_Callback_T const& callback)
    {
    return Write ((unsigned char*) data, length, callback);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Uv::TcpHandle::TcpHandle (int& status)
    : IoStream (reinterpret_cast<uv_stream_t*>(malloc (sizeof (uv_tcp_t))))
    {
    status = ::uv_tcp_init (Host::GetInstance().GetEventLoop(), GetPointerUnchecked<uv_tcp_t>());
    BeAssert (status >= 0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
uv_tcp_t* Uv::TcpHandle::GetPointer() const
    {
    auto handle = Handle::GetPointer();
    BeAssert (handle->type == UV_TCP);

    return reinterpret_cast<uv_tcp_t*>(handle);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
uv_stream_t* Uv::TcpHandle::GetStreamPointer() const
    {
    return static_cast<IoStreamCP>(this)->GetPointer();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Uv::Status Uv::TcpHandle::SetKeepAlive (bool enable, uint32_t delay)
    {
    return ::uv_tcp_keepalive (GetPointer(), enable, delay);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Uv::Status Uv::TcpHandle::SetSimultaneousAccepts (bool enable)
    {
    return ::uv_tcp_simultaneous_accepts (GetPointer(), enable);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Uv::Status Uv::TcpHandle::SetNoDelay (bool enable)
    {
    return ::uv_tcp_nodelay (GetPointer(), enable);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Uv::Request::Request (uv_req_t* request)
    : m_request (request)
    {
    m_request->data = this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Uv::Request::Request (Request&& other)
    : m_request (other.m_request)
    {
    other.m_request = nullptr;
    m_request->data = this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Uv::Request::~Request()
    {
    if (m_request != nullptr)
        free (m_request);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Uv::WriteRequest::WriteRequest (uv_stream_t* handle, unsigned char* data, size_t length, Write_Callback_T const& callback, int& status)
    : Request    (reinterpret_cast<uv_req_t*>(malloc (sizeof (uv_write_t)))),
      m_callback (callback),m_payload((char*)data,length)
    {
    auto buf = ::uv_buf_init ((char*)m_payload.c_str(), m_payload.length() );
    status = ::uv_write (GetPointerUnchecked<uv_write_t>(), handle, &buf, 1, &Handler);
    if (status >= 0)
        AddRef();
    else
        BeAssert (false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void Uv::WriteRequest::Handler (uv_write_t* req, int status)
    {
    BeAssert (req->data != nullptr);
    auto& instance = *reinterpret_cast<WriteRequestP>(req->data);
#if defined(PRINT_STAT)
    if (status < 0)
        {
        printf("Error: %s\n ", uv_strerror(status));
        }
#endif
    if (!Host::GetInstance().IsStopped())
        instance.m_callback (Status (status));
        
    instance.Release();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Uv::WriteRequestPtr Uv::WriteRequest::Create (IoStreamCR handle, unsigned char* data, size_t length, Write_Callback_T const& callback, int& status)
    {
    return new WriteRequest (handle.GetPointer(), data, length, callback, status);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
uv_write_t* Uv::WriteRequest::GetPointer() const
    {
    auto request = Request::GetPointer();
    BeAssert (request->type = UV_WRITE);

    return reinterpret_cast<uv_write_t*>(request);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Uv::ShutdownRequest::ShutdownRequest (IoStreamCR handle, int& status)
    : Request (reinterpret_cast<uv_req_t*>(malloc (sizeof (uv_shutdown_t))))
    {
    status = ::uv_shutdown (GetPointerUnchecked<uv_shutdown_t>(), handle.GetPointer(), &Handler);
    if (status >= 0)
        AddRef();
    else
        BeAssert (false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Uv::ShutdownRequestPtr Uv::ShutdownRequest::Create (IoStreamCR handle, int& status)
    {
    return new ShutdownRequest (handle, status);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void Uv::ShutdownRequest::Handler (uv_shutdown_t* req, int status)
    {
    BeAssert (req->data != nullptr);

    auto& instance = *reinterpret_cast<ShutdownRequestP>(req->data);

    if (!Host::GetInstance().IsStopped())
        instance.m_callback (Status (status));

    instance.Release();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
uv_shutdown_t* Uv::ShutdownRequest::GetPointer() const
    {
    auto request = Request::GetPointer();
    BeAssert (request->type = UV_SHUTDOWN);

    return reinterpret_cast<uv_shutdown_t*>(request);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Uv::ConnectRequest::ConnectRequest (AddressDescriptorCR address, TcpHandle&& handle, int& status, ConnectCallback_T const& callback)
    : Request    (reinterpret_cast<uv_req_t*>(malloc (sizeof (uv_connect_t)))),
      m_handle   (new TcpHandle (std::move (handle))),
      m_callback (callback)
    {
    status = ::uv_tcp_connect (GetPointerUnchecked<uv_connect_t>(), m_handle->GetPointer(), address.GetPointer(), &Handler);
    if (status >= 0)
        AddRef();
    else
        BeAssert (false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Uv::ConnectRequestPtr Uv::ConnectRequest::Create (AddressDescriptorCR address, TcpHandle&& handle, int& status, ConnectCallback_T const& callback)
    {
    return new ConnectRequest (address, std::move (handle), status, callback);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
uv_connect_t* Uv::ConnectRequest::GetPointer() const
    {
    auto request = Request::GetPointer();
    BeAssert (request->type = UV_CONNECT);

    return reinterpret_cast<uv_connect_t*>(request);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void Uv::ConnectRequest::Handler (uv_connect_t* req, int status)
    {
    BeAssert (req->data != nullptr);

    auto& instance = *reinterpret_cast<ConnectRequestP>(req->data);

    if (!Host::GetInstance().IsStopped())
        instance.m_callback (Status (status), instance.m_handle);

    instance.Release();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Uv::BindResult Uv::Bind (Utf8CP address, uint16_t port, IP protocol)
    {
    int status;

    AddressDescriptor descriptor (address, port, protocol, status);
    if (status >= 0)
        {
        TcpHandle handle (status);
        if (status >= 0)
            {
            status = ::uv_tcp_bind (handle.GetPointer(), descriptor.GetPointer(), 0);
            if (status >= 0)
                {
                struct sockaddr_storage name;
                int namelen = sizeof (name);
                status = uv_tcp_getsockname (handle.GetPointer(), (sockaddr*) &name, &namelen);
                if (status >= 0)
                    {
                    struct sockaddr_in addr = *(struct sockaddr_in*)(&name);
                    return BindResult (status, new Server (std::move (handle)), ntohs (addr.sin_port));
                    }
                }
            }
        }

    BeAssert (status < 0);
    return BindResult (status, nullptr, 0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Uv::ConnectResult Uv::Connect (Utf8CP address, uint16_t port, IP protocol, ConnectCallback_T const& callback)
    {
    int status;

    AddressDescriptor descriptor (address, port, protocol, status);
    if (status >= 0)
        {
        TcpHandle handle (status);
        if (status >= 0)
            {
            auto request = ConnectRequest::Create (descriptor, std::move (handle), status, callback);
            if (status >= 0)
                return ConnectResult (status, request.get());
            }
        }

    BeAssert (status < 0);
    return ConnectResult (status, nullptr);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Uv::Status Uv::Server::Listen (uint32_t backlog, ListenCallback_T const& callback)
    {
    m_callback = callback;
    return ::uv_listen (GetStreamPointer(), backlog, &Server::ListenHandler);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Uv::Status Uv::Server::Accept (TcpHandleR connection)
    {
    return ::uv_accept (GetStreamPointer(), connection.GetStreamPointer());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void Uv::Server::ListenHandler (uv_stream_t* server, int status)
    {
    auto& instance = *reinterpret_cast<ServerP>(server->data);

    if (Host::GetInstance().IsStopped())
        {
        instance.Close();
        return;
        }

    if (status >= 0)
        {
        TcpHandlePtr handle = new TcpHandle (status);
        if (status >= 0)
            {
            instance.m_callback (Status (status), handle);
            return;
            }
        }

    BeAssert (status < 0);
    instance.m_callback (Status (status), nullptr);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
WebSockets::ServerEndpoint::ServerEndpoint()
    {
    m_server.clear_access_channels (websocketpp::log::alevel::all);
    m_server.clear_error_channels (websocketpp::log::elevel::all);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
WebSockets::ServerEndpointPtr WebSockets::ServerEndpoint::Create()
    {
    return new ServerEndpoint;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
WebSockets::ClientConnectionPtr WebSockets::ServerEndpoint::CreateConnection (EventCallback_T const& callback, TransportHandler_T const& handler)
    {
    return new ClientConnection (m_server.get_connection(), callback, handler);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
WebSockets::ClientConnection::ClientConnection (websocketpp_connection_ptr_t const& connection, EventCallback_T const& callback, TransportHandler_T const& handler)
    : m_callback   (callback),
      m_handler    (handler),
      m_connection (connection),
      m_relay      (*this),
      m_output     (&m_relay)
    {
    Forwarder forwarder (*this);
    m_connection->set_open_handler (forwarder);
    m_connection->set_fail_handler (forwarder);
    m_connection->set_message_handler (forwarder);

    m_connection->register_ostream (&m_output);
    m_connection->start();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
std::streamsize WebSockets::ClientConnection::Relay::xsputn (const char_type* s, std::streamsize count)
    {
    return m_connection.m_handler (s, count);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void WebSockets::ClientConnection::Forwarder::operator() (websocketpp::connection_hdl handle)
    {
    auto& connection = m_connection.GetConnection();
    if (connection.get_state() == websocketpp::session::state::open)
        m_connection.m_callback (Event::Open, nullptr);
    else
        m_connection.m_callback (Event::Error, nullptr);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void WebSockets::ClientConnection::Forwarder::operator() (websocketpp::connection_hdl handle, websocketpp_server_t::message_ptr message)
    {
    m_connection.m_callback (Event::Message, message);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
bool WebSockets::ClientConnection::Process (Utf8CP input, size_t offset, size_t length)
    {
    return GetConnection().read_all (input + offset, length) == length;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
bool WebSockets::ClientConnection::Send (Utf8CP message, size_t length, websocketpp::frame::opcode::value code)
    {
    return GetConnection().send (message, length, code).value() == 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
MobileGatewayR MobileGateway::GetInstance()
    {
    if (s_instance.IsNull())
        s_instance = new MobileGateway;

    return *s_instance;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void MobileGateway::Terminate()
    {
    BeAssert (s_instance.IsValid());
    s_instance = nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
MobileGateway::MobileGateway()
    {
    auto bindResult = Uv::Bind ("127.0.0.1", 0, Uv::IP::V4);
    BeAssert (!bindResult.IsError());

    m_server = bindResult.GetServer();
    m_port = bindResult.GetPort();

    m_server->SetNoDelay (true);
    m_server->SetKeepAlive (true, 0);

    m_server->Listen (1024, [this](Uv::StatusCR status, Uv::TcpHandlePtr connection)
        {
        BeAssert (!status.IsError());

        auto acceptResult = m_server->Accept (*connection);
        BeAssert (!acceptResult.IsError());
        m_client = connection;


#if defined(BENTLEYCONFIG_OS_APPLE_IOS)
        m_connection = m_endpoint.CreateConnection ([](WebSockets::Event event, WebSockets::websocketpp_server_t::message_ptr message)
#else
        m_connection = m_endpoint.CreateConnection ([this](WebSockets::Event event, WebSockets::websocketpp_server_t::message_ptr message)
#endif
            {
            if (event == WebSockets::Event::Message)
                {
                BeAssert (message->get_opcode() == websocketpp::frame::opcode::value::binary);
                auto& env = Host::GetInstance().GetJsRuntime().Env();
                Napi::HandleScope scope (env);
                std::string const& data = message->get_payload();
                auto payload = Napi::ArrayBuffer::New (env, (void*)data.c_str(), data.size());
                
#if defined(BENTLEYCONFIG_OS_APPLE_IOS)
                env.Global().Get("__imodeljs_mobilegateway_handler__").As<Napi::Function>().Call({ payload }); //WIP: napi add ref not implemented yet for jsc
#else
                m_exports.Get("handler").As<Napi::Function>().Call({ payload });
#endif
                }
            }, [this](const std::streambuf::char_type* s, std::streamsize c)
            {
            auto writeResult = m_client->Write (s, c, [&] (Uv::StatusCR status)
                {
                if (status.IsError())
                    {
                    BeAssert (!status.IsError());
                    }
                });
                
            BeAssert (!writeResult.IsError());
            return c;
            });

        BeAssert (m_connection.IsValid());
        auto readResult = m_client->Read ([this](Uv::StatusCR status, uv_buf_t const& buffer, size_t nread)
            {
            if (status.GetCode() == UV_EOF)
                return false;

            BeAssert (!status.IsError());
            auto processed = m_connection->Process (buffer.base, 0, nread);
            BeAssert (processed);
            return true;
            });

        BeAssert (!readResult.IsError());
        });
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Napi::Value MobileGateway::ExportJsModule (Js::RuntimeR runtime)
    {
    auto& env = runtime.Env();

    auto exports = Napi::Object::New (env);
    m_exports.Reset (exports, 1);

    exports.Set ("handler", env.Null());
    exports.Set ("port", Napi::Number::New (env, GetPort()));
    exports.Set ("send", Napi::Function::New (env, [this](Napi::CallbackInfo const& info) -> Napi::Value
        {
        BeAssert (m_client.IsValid());
        JS_CALLBACK_REQUIRE_N_ARGS (1);
        auto binArray = JS_CALLBACK_GET_ARRAY (0);
        size_t totalBytes = 0;
        for(int i=0; i < binArray.Length(); ++i) {
            Napi::Value val = binArray[i];
            totalBytes += val.As<Napi::Uint8Array>().ByteLength();
        }
        
        size_t pos = 0;
        void* data = malloc (totalBytes);
        for(int i=0; i < binArray.Length(); ++i) {
            Napi::Value val = binArray[i];
            Napi::Uint8Array intArray = val.As<Napi::Uint8Array>();
            Napi::ArrayBuffer buffer = intArray.ArrayBuffer();
            const size_t byteLength = intArray.ByteLength();
            const size_t byteOffset = intArray.ByteOffset();
            void* target = (void*)((unsigned char*)data + pos);
            void* source = (void*)((unsigned char*)buffer.Data() + byteOffset);
            memcpy(target, source, byteLength);
            pos += byteLength;
        }
        BeAssert(pos == totalBytes);
        if(!m_connection->Send ((const char*)data, totalBytes, websocketpp::frame::opcode::value::binary))
            {
            BeAssert(false);
            }
        free(data);
        return info.Env().Undefined();
        }));

    return exports;
    }

MobileGatewayPtr MobileGateway::s_instance;

#ifdef COMMENT_OUT
//---------------------------------------------------------------------------------------
// @bsimethod                                Sam.Wilson                    01/2018
//---------------------------------------------------------------------------------------
void NodeWorkAlike::Globals::Install(Js::RuntimeR runActime)
    {
	auto& env = runtime.Env();

    Napi::HandleScope scope (env);

    auto process = Napi::Object::New(env);
    process["env"] = Napi::Object::New(env);       // this is a dummy. It's supposed to be the system environment, but that doesn't make sense on mobile devices.
                                                // We might try populating it with config variables ...
    process["on"] = Napi::Function::New(env, [] (Napi::CallbackInfo const& info) {
        JS_CALLBACK_REQUIRE_AT_LEAST_N_ARGS(2);

        // *** TBD: support for event handlers?

    });

    process["exit"] = Napi::Function::New(env, [] (Napi::CallbackInfo const& info) {
        // *** TBD: support this?

    });

    env.Global()["process"] = process;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Sam.Wilson                    01/2018
//---------------------------------------------------------------------------------------
Napi::Value NodeWorkAlike::Extension_path::ExportJsModule(Js::RuntimeR runtime)
    {
	auto& env = runtime.Env();

    Napi::HandleScope scope (env);

    auto exports = Napi::Object::New(env);

    auto path = Napi::Object::New(env);
#ifdef _WIN32
    bool isWin32 = true;
    Utf8CP delim = ";";
#else
    bool isWin32 = false;
    Utf8CP delim = ":";
#endif
    path["posix"] = Napi::Boolean::New(env, !isWin32);
    path["win32"] = Napi::Boolean::New(env, isWin32);
    path["sep"] = Napi::String::New(env, DIR_SEPARATOR);
    path["delimiter"] = Napi::String::New(env, delim);
    path["basename"] = Napi::Function::New(env, [] (Napi::CallbackInfo const& info) -> Napi::Value {
            JS_CALLBACK_REQUIRE_AT_LEAST_N_ARGS(1);
            auto fnameS = JS_CALLBACK_GET_STRING(0);
            // TBD: optional 'ext' argument ...
            BeFileName fname(fnameS.Utf8Value().c_str(), true);
            return Napi::String::New(info.Env(), Utf8String(fname.GetBaseName()).c_str());
        });
    path["dirname"] = Napi::Function::New(env, [] (Napi::CallbackInfo const& info) -> Napi::Value {
            JS_CALLBACK_REQUIRE_AT_LEAST_N_ARGS(1);
            auto fnameS = JS_CALLBACK_GET_STRING(0);
            BeFileName fname(fnameS.Utf8Value().c_str(), true);
            return Napi::String::New(info.Env(), Utf8String(fname.GetDirectoryName()).c_str());
        });
    path["extname"] = Napi::Function::New(env, [] (Napi::CallbackInfo const& info) -> Napi::Value {
            JS_CALLBACK_REQUIRE_AT_LEAST_N_ARGS(1);
            auto fnameS = JS_CALLBACK_GET_STRING(0);
            BeFileName fname(fnameS.Utf8Value().c_str(), true);
            return Napi::String::New(info.Env(), Utf8String(fname.GetExtension()).c_str());
        });
    path["normalize"] = Napi::Function::New(env, [] (Napi::CallbackInfo const& info) -> Napi::Value {
            JS_CALLBACK_REQUIRE_AT_LEAST_N_ARGS(1);
            auto fnameS = JS_CALLBACK_GET_STRING(0);
            if (fnameS.Utf8Value().empty())
                {
                return Napi::String::New(info.Env(), "."); // as per node docs
                }
            BeFileName fname(fnameS.Utf8Value().c_str(), true);
            WString fixed;
            BeFileName::FixPathName(fixed, fname, true);
            return Napi::String::New(info.Env(), Utf8String(fixed).c_str());
        });
    path["join"] = Napi::Function::New(env, [] (Napi::CallbackInfo const& info) -> Napi::Value {
            JS_CALLBACK_REQUIRE_AT_LEAST_N_ARGS(1);
            // *** TBD: what will the arguments look like? An array?
            auto fnames = JS_CALLBACK_GET_ARRAY(0);
            if (fnames.Length() == 0)
                {
                Napi::Error::New(info.Env(), "not enough arguments").ThrowAsJavaScriptException();
                return info.Env().Undefined();
                }
            auto const& fnamesC = fnames;
            BeFileName path(fnamesC[(uint32_t)0].As<Napi::String>().Utf8Value().c_str(), true);
            for (uint32_t i = 1; i < fnamesC.Length(); ++i)
                {
                path.AppendToPath(BeFileName(fnamesC[i].As<Napi::String>().Utf8Value().c_str(), true));
                }
            return Napi::String::New(info.Env(), Utf8String(path).c_str());
        });
    path["isAbsolute"] = Napi::Function::New(env, [] (Napi::CallbackInfo const& info) -> Napi::Value {
            JS_CALLBACK_REQUIRE_AT_LEAST_N_ARGS(1);
            auto fnameS = JS_CALLBACK_GET_STRING(0);
            BeFileName fname(fnameS.Utf8Value().c_str(), true);
            return Napi::Boolean::New(info.Env(), fname.IsAbsolutePath());
        });

        /* TBD
path.format(pathObject)
path.parse(path)
path.relative(from, to)
path.resolve([...paths])
path.toNamespacedPath(path)
*/

    exports["path"] = path;

    return exports;
    }
#endif

END_BENTLEY_IMODELJS_SERVICES_TIER_NAMESPACE
