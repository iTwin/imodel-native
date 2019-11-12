/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "iModelJsInternal.h"

BEGIN_BENTLEY_IMODELJS_SERVICES_TIER_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
#ifdef COMMENT_OUT
Utf8CP Utilities::InitScript()
    {
    return u8R"(

    (function (params) {
        let uv_StatusCode = {};
        uv_StatusCode [uv_StatusCode ["SUCCESS"]            = 0]  = "SUCCESS";
        uv_StatusCode [uv_StatusCode ["UV_E2BIG"]           = 1]  = "UV_E2BIG";
        uv_StatusCode [uv_StatusCode ["UV_EACCES"]          = 2]  = "UV_EACCES";
        uv_StatusCode [uv_StatusCode ["UV_EADDRINUSE"]      = 3]  = "UV_EADDRINUSE";
        uv_StatusCode [uv_StatusCode ["UV_EADDRNOTAVAIL"]   = 4]  = "UV_EADDRNOTAVAIL";
        uv_StatusCode [uv_StatusCode ["UV_EAFNOSUPPORT"]    = 5]  = "UV_EAFNOSUPPORT";
        uv_StatusCode [uv_StatusCode ["UV_EAGAIN"]          = 6]  = "UV_EAGAIN";
        uv_StatusCode [uv_StatusCode ["UV_EAI_ADDRFAMILY"]  = 7]  = "UV_EAI_ADDRFAMILY";
        uv_StatusCode [uv_StatusCode ["UV_EAI_AGAIN"]       = 8]  = "UV_EAI_AGAIN";
        uv_StatusCode [uv_StatusCode ["UV_EAI_BADFLAGS"]    = 9]  = "UV_EAI_BADFLAGS";
        uv_StatusCode [uv_StatusCode ["UV_EAI_BADHINTS"]    = 10] = "UV_EAI_BADHINTS";
        uv_StatusCode [uv_StatusCode ["UV_EAI_CANCELED"]    = 11] = "UV_EAI_CANCELED";
        uv_StatusCode [uv_StatusCode ["UV_EAI_FAIL"]        = 12] = "UV_EAI_FAIL";
        uv_StatusCode [uv_StatusCode ["UV_EAI_FAMILY"]      = 13] = "UV_EAI_FAMILY";
        uv_StatusCode [uv_StatusCode ["UV_EAI_MEMORY"]      = 14] = "UV_EAI_MEMORY";
        uv_StatusCode [uv_StatusCode ["UV_EAI_NODATA"]      = 15] = "UV_EAI_NODATA";
        uv_StatusCode [uv_StatusCode ["UV_EAI_NONAME"]      = 16] = "UV_EAI_NONAME";
        uv_StatusCode [uv_StatusCode ["UV_EAI_OVERFLOW"]    = 17] = "UV_EAI_OVERFLOW";
        uv_StatusCode [uv_StatusCode ["UV_EAI_PROTOCOL"]    = 18] = "UV_EAI_PROTOCOL";
        uv_StatusCode [uv_StatusCode ["UV_EAI_SERVICE"]     = 19] = "UV_EAI_SERVICE";
        uv_StatusCode [uv_StatusCode ["UV_EAI_SOCKTYPE"]    = 20] = "UV_EAI_SOCKTYPE";
        uv_StatusCode [uv_StatusCode ["UV_EALREADY"]        = 21] = "UV_EALREADY";
        uv_StatusCode [uv_StatusCode ["UV_EBADF"]           = 22] = "UV_EBADF";
        uv_StatusCode [uv_StatusCode ["UV_EBUSY"]           = 23] = "UV_EBUSY";
        uv_StatusCode [uv_StatusCode ["UV_ECANCELED"]       = 24] = "UV_ECANCELED";
        uv_StatusCode [uv_StatusCode ["UV_ECHARSET"]        = 25] = "UV_ECHARSET";
        uv_StatusCode [uv_StatusCode ["UV_ECONNABORTED"]    = 26] = "UV_ECONNABORTED";
        uv_StatusCode [uv_StatusCode ["UV_ECONNREFUSED"]    = 27] = "UV_ECONNREFUSED";
        uv_StatusCode [uv_StatusCode ["UV_ECONNRESET"]      = 28] = "UV_ECONNRESET";
        uv_StatusCode [uv_StatusCode ["UV_EDESTADDRREQ"]    = 29] = "UV_EDESTADDRREQ";
        uv_StatusCode [uv_StatusCode ["UV_EEXIST"]          = 30] = "UV_EEXIST";
        uv_StatusCode [uv_StatusCode ["UV_EFAULT"]          = 31] = "UV_EFAULT";
        uv_StatusCode [uv_StatusCode ["UV_EFBIG"]           = 32] = "UV_EFBIG";
        uv_StatusCode [uv_StatusCode ["UV_EHOSTUNREACH"]    = 33] = "UV_EHOSTUNREACH";
        uv_StatusCode [uv_StatusCode ["UV_EINTR"]           = 34] = "UV_EINTR";
        uv_StatusCode [uv_StatusCode ["UV_EINVAL"]          = 35] = "UV_EINVAL";
        uv_StatusCode [uv_StatusCode ["UV_EIO"]             = 36] = "UV_EIO";
        uv_StatusCode [uv_StatusCode ["UV_EISCONN"]         = 37] = "UV_EISCONN";
        uv_StatusCode [uv_StatusCode ["UV_EISDIR"]          = 38] = "UV_EISDIR";
        uv_StatusCode [uv_StatusCode ["UV_ELOOP"]           = 39] = "UV_ELOOP";
        uv_StatusCode [uv_StatusCode ["UV_EMFILE"]          = 40] = "UV_EMFILE";
        uv_StatusCode [uv_StatusCode ["UV_EMSGSIZE"]        = 41] = "UV_EMSGSIZE";
        uv_StatusCode [uv_StatusCode ["UV_ENAMETOOLONG"]    = 42] = "UV_ENAMETOOLONG";
        uv_StatusCode [uv_StatusCode ["UV_ENETDOWN"]        = 43] = "UV_ENETDOWN";
        uv_StatusCode [uv_StatusCode ["UV_ENETUNREACH"]     = 44] = "UV_ENETUNREACH";
        uv_StatusCode [uv_StatusCode ["UV_ENFILE"]          = 45] = "UV_ENFILE";
        uv_StatusCode [uv_StatusCode ["UV_ENOBUFS"]         = 46] = "UV_ENOBUFS";
        uv_StatusCode [uv_StatusCode ["UV_ENODEV"]          = 47] = "UV_ENODEV";
        uv_StatusCode [uv_StatusCode ["UV_ENOENT"]          = 48] = "UV_ENOENT";
        uv_StatusCode [uv_StatusCode ["UV_ENOMEM"]          = 49] = "UV_ENOMEM";
        uv_StatusCode [uv_StatusCode ["UV_ENONET"]          = 50] = "UV_ENONET";
        uv_StatusCode [uv_StatusCode ["UV_ENOPROTOOPT"]     = 51] = "UV_ENOPROTOOPT";
        uv_StatusCode [uv_StatusCode ["UV_ENOSPC"]          = 52] = "UV_ENOSPC";
        uv_StatusCode [uv_StatusCode ["UV_ENOSYS"]          = 53] = "UV_ENOSYS";
        uv_StatusCode [uv_StatusCode ["UV_ENOTCONN"]        = 54] = "UV_ENOTCONN";
        uv_StatusCode [uv_StatusCode ["UV_ENOTDIR"]         = 55] = "UV_ENOTDIR";
        uv_StatusCode [uv_StatusCode ["UV_ENOTEMPTY"]       = 56] = "UV_ENOTEMPTY";
        uv_StatusCode [uv_StatusCode ["UV_ENOTSOCK"]        = 57] = "UV_ENOTSOCK";
        uv_StatusCode [uv_StatusCode ["UV_ENOTSUP"]         = 58] = "UV_ENOTSUP";
        uv_StatusCode [uv_StatusCode ["UV_EPERM"]           = 59] = "UV_EPERM";
        uv_StatusCode [uv_StatusCode ["UV_EPIPE"]           = 60] = "UV_EPIPE";
        uv_StatusCode [uv_StatusCode ["UV_EPROTO"]          = 61] = "UV_EPROTO";
        uv_StatusCode [uv_StatusCode ["UV_EPROTONOSUPPORT"] = 62] = "UV_EPROTONOSUPPORT";
        uv_StatusCode [uv_StatusCode ["UV_EPROTOTYPE"]      = 63] = "UV_EPROTOTYPE";
        uv_StatusCode [uv_StatusCode ["UV_ERANGE"]          = 64] = "UV_ERANGE";
        uv_StatusCode [uv_StatusCode ["UV_EROFS"]           = 65] = "UV_EROFS";
        uv_StatusCode [uv_StatusCode ["UV_ESHUTDOWN"]       = 66] = "UV_ESHUTDOWN";
        uv_StatusCode [uv_StatusCode ["UV_ESPIPE"]          = 67] = "UV_ESPIPE";
        uv_StatusCode [uv_StatusCode ["UV_ESRCH"]           = 68] = "UV_ESRCH";
        uv_StatusCode [uv_StatusCode ["UV_ETIMEDOUT"]       = 69] = "UV_ETIMEDOUT";
        uv_StatusCode [uv_StatusCode ["UV_ETXTBSY"]         = 70] = "UV_ETXTBSY";
        uv_StatusCode [uv_StatusCode ["UV_EXDEV"]           = 71] = "UV_EXDEV";
        uv_StatusCode [uv_StatusCode ["UV_UNKNOWN"]         = 72] = "UV_UNKNOWN";
        uv_StatusCode [uv_StatusCode ["UV_EOF"]             = 73] = "UV_EOF";
        uv_StatusCode [uv_StatusCode ["UV_ENXIO"]           = 74] = "UV_ENXIO";
        uv_StatusCode [uv_StatusCode ["UV_EMLINK"]          = 75] = "UV_EMLINK";
            
        )"
        u8R"(

        let uv_Handle = function Handle() {
            throw new Error ("Cannot create Handle instances directly.");
        };

        uv_Handle.prototype.close = params.uv_Handle_close;

        let uv_Status = function Status() {
            throw new Error ("Cannot create Status instances directly.");
        };

        uv_Status.prototype.toString = function() {
            return "Status Code " + this.code.toString();
        };

        uv_Status.prototype.success = function() {
            return this.code === uv_StatusCode.SUCCESS;
        };

        let uv_io_Stream = function Stream() {
            throw new Error ("Cannot create Stream instances directly.");
        };

        uv_io_Stream.prototype = Object.create (uv_Handle.prototype);
        uv_io_Stream.prototype.constructor = uv_Handle;

        uv_io_Stream.prototype.isReadable = params.uv_io_Stream_isReadable;
        uv_io_Stream.prototype.isWritable = params.uv_io_Stream_isWritable;
        uv_io_Stream.prototype.read       = params.uv_io_Stream_read;
        uv_io_Stream.prototype.write      = params.uv_io_Stream_write;

        let uv_io_shutdown = params.uv_io_shutdown;

        let uv_io_createDefaultStreamAllocator = function() {
            let pool = {};

            let allocateHandler = function (suggestedSize) {
                if (pool.hasOwnProperty (suggestedSize) && pool [suggestedSize].length !== 0)
                    return pool [suggestedSize].pop();

                return new ArrayBuffer (suggestedSize);
            };

            let recycleHandler = function (buffer) {
                let identifier = buffer.byteLength;

                if (!pool.hasOwnProperty (identifier))
                    pool [identifier] = [];

                pool [identifier].push (buffer);
                    
                return true;
            };

            return { allocate: allocateHandler, recycle: recycleHandler };
        };

        let uv_net_IP = {};
        uv_net_IP [uv_net_IP ["V4"] = 0] = "V4";
        uv_net_IP [uv_net_IP ["V6"] = 1] = "V6";

        let uv_tcp_BindResult = function BindResult() {
            this.server = null;
            throw new Error ("Cannot create BindResult instances directly.");
        };

        uv_tcp_BindResult.prototype = Object.create (uv_Status.prototype);
        uv_tcp_BindResult.prototype.constructor = uv_tcp_BindResult;

        let uv_tcp_ConnectResult = function ConnectResult() {
            this.connection = null;
            throw new Error ("Cannot create ConnectResult instances directly.");
        };

        uv_tcp_ConnectResult.prototype = Object.create (uv_Status.prototype);
        uv_tcp_ConnectResult.prototype.constructor = uv_tcp_ConnectResult;

        let uv_tcp_Handle = function Handle() {
            throw new Error ("Cannot create Handle instances directly.");
        };

        uv_tcp_Handle.prototype = Object.create (uv_io_Stream.prototype);
        uv_tcp_Handle.prototype.constructor = uv_tcp_Handle;

        uv_tcp_Handle.prototype.setNoDelay   = params.uv_tcp_Handle_setNoDelay;
        uv_tcp_Handle.prototype.setKeepAlive = params.uv_tcp_Handle_setKeepAlive;

        let uv_tcp_Server = function Server() {
            throw new Error ("Cannot create Server instances directly.");
        };

        uv_tcp_Server.prototype = Object.create (uv_tcp_Handle.prototype);
        uv_tcp_Server.prototype.constructor = uv_tcp_Server;

        uv_tcp_Server.prototype.setSimultaneousAccepts = params.uv_tcp_Server_setSimultaneousAccepts;
        uv_tcp_Server.prototype.listen                 = params.uv_tcp_Server_listen;
        uv_tcp_Server.prototype.accept                 = params.uv_tcp_Server_accept;

        let uv_tcp_bind     = params.uv_tcp_bind;
        let uv_tcp_connect  = params.uv_tcp_connect;

        let websocketpp_OpCode = {};
        websocketpp_OpCode [websocketpp_OpCode ["Continuation"] = 0x0]  = "Continuation";
        websocketpp_OpCode [websocketpp_OpCode ["Text"]         = 0x1]  = "Text";
        websocketpp_OpCode [websocketpp_OpCode ["Binary"]       = 0x2]  = "Binary";
        websocketpp_OpCode [websocketpp_OpCode ["Close"]        = 0x8]  = "Close";
        websocketpp_OpCode [websocketpp_OpCode ["Ping"]         = 0x9]  = "Ping";
        websocketpp_OpCode [websocketpp_OpCode ["Pong"]         = 0xA]  = "Pong";

        let websocketpp_Base = function Base() {
            throw new Error ("Cannot create Base instances directly.");
        };

        websocketpp_Base.prototype.Dispose = params.websocketpp_Base_Dispose;

        let websocketpp_ClientConnection = function ClientConnection() {
            this.handler = null;
            throw new Error ("Cannot create ClientConnection instances directly.");
        };

        websocketpp_ClientConnection.prototype = Object.create (websocketpp_Base.prototype);
        websocketpp_ClientConnection.prototype.constructor = websocketpp_ClientConnection;

        websocketpp_ClientConnection.prototype.process = params.websocketpp_ClientConnection_process;
        websocketpp_ClientConnection.prototype.send    = params.websocketpp_ClientConnection_send;

        let websocketpp_ServerEndpoint = params.websocketpp_ServerEndpoint_constructor;

        websocketpp_ServerEndpoint.prototype = Object.create (websocketpp_Base.prototype);
        websocketpp_ServerEndpoint.prototype.constructor = websocketpp_ServerEndpoint;

        websocketpp_ServerEndpoint.prototype.createConnection = params.websocketpp_ServerEndpoint_createConnection;

        let abtostr = function (buf, start, len) {
            return String.fromCharCode.apply (this, new Uint16Array (buf, start, len));
        };

        let abtostr8 = function (buf, start, len) {
            return String.fromCharCode.apply (this, new Uint8Array (buf, start, len));
        };

        let strtoab = function (str) {
            let buffer = new ArrayBuffer (str.length * 2);
            let view = new Uint16Array (buffer);
                
            for (var i = 0; i != str.length; ++i)
                view [i] = str.charCodeAt (i);
                    
            return buffer;
        };

        let strtoab8 = function (str) {
            let buffer = new ArrayBuffer (str.length);
            let view = new Uint8Array (buffer);
                
            for (var i = 0; i != str.length; ++i)
                view [i] = str.charCodeAt (i);
                    
            return buffer;
        };

        let uv_fs_open  = params.uv_fs_open;
        let uv_fs_realpath  = params.uv_fs_realpath;
        let uv_fs_stat  = params.uv_fs_stat;
        let uv_fs_read  = params.uv_fs_read;
        let uv_fs_close = params.uv_fs_close;

        let uv_fs_isValid = function (file)
            {
            return file >= 0;
            };

        )"
        u8R"(

        let exports = {
            abtostr: abtostr,
            abtostr8: abtostr8,
            strtoab: strtoab,
            strtoab8: strtoab8,

            uv: {
                StatusCode: uv_StatusCode,
                Handle:     uv_Handle,
                Status:     uv_Status,

                io: {
                    Stream:                       uv_io_Stream,
                    shutdown:                     uv_io_shutdown,
                    createDefaultStreamAllocator: uv_io_createDefaultStreamAllocator
                },

                net: {
                    IP: uv_net_IP
                },

                tcp: {
                    BindResult:    uv_tcp_BindResult,
                    ConnectResult: uv_tcp_ConnectResult,
                    Handle:        uv_tcp_Handle,
                    Server:        uv_tcp_Server,
                    bind:          uv_tcp_bind,
                    connect:       uv_tcp_connect
                }
            },

            websocketpp: {
                OpCode:           websocketpp_OpCode,
                Base:             websocketpp_Base,
                ClientConnection: websocketpp_ClientConnection,
                ServerEndpoint:   websocketpp_ServerEndpoint
            },

            fs: {
                O_RDONLY:     params.uv_fs_O_RDONLY,
                O_WRONLY:     params.uv_fs_O_WRONLY,
                O_RDWR:       params.uv_fs_O_RDWR,
                O_APPEND:     params.uv_fs_O_APPEND,
                O_CREAT:      params.uv_fs_O_CREAT,
                O_TRUNC:      params.uv_fs_O_TRUNC,
                O_EXCL:       params.uv_fs_O_EXCL,
                O_TEXT:       params.uv_fs_O_TEXT,
                O_BINARY:     params.uv_fs_O_BINARY,
                O_RAW:        params.uv_fs_O_RAW,
                O_TEMPORARY:  params.uv_fs_O_TEMPORARY,
                O_NOINHERIT:  params.uv_fs_O_NOINHERIT,
                O_SEQUENTIAL: params.uv_fs_O_SEQUENTIAL,
                O_RANDOM:     params.uv_fs_O_RANDOM,

                S_IFMT:   params.uv_fs_S_IFMT,
                S_IFDIR:  params.uv_fs_S_IFDIR,
                S_IFCHR:  params.uv_fs_S_IFCHR,
                S_IFREG:  params.uv_fs_S_IFREG,
                S_IREAD:  params.uv_fs_S_IREAD,
                S_IWRITE: params.uv_fs_S_IWRITE,
                S_IEXEC:  params.uv_fs_S_IEXEC,

                open:    uv_fs_open,
                realpath: uv_fs_realpath,
                stat:    uv_fs_stat,
                isValid: uv_fs_isValid,
                read:    uv_fs_read,
                close :  uv_fs_close
            }
        };

        return exports;
    });

    )";
    }
#endif
//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Utf8CP Utilities::SimpleInitScript()
    {
    return u8R"(

    console_log("imodeljsMobile = " + JSON.stringify(self.imodeljsMobile));

    // mocha thinks that it is running in a browser, and so it expects to have a URL
    self.location = {};

    // mocha expects the global 'setTimeout' and 'clearTimeout' functions to be defined.
    /* Affan: timer funtion define in objective c
    function setTimeout(func, delay, param1, param2, param3, param4, param5, param6, param7, param8, param9, param10) {
      var timer = new IModelJsTimer(
            (delay || 0),
            function () {
                func(param1, param2, param3, param4, param5, param6, param7, param8, param9, param10)
            });
      timer.start();
      // Native code keeps the timer object alive until it fires or stop is called.
      return timer;
    }

    function clearTimeout(timer) {
      if (timer) {
        timer.stop();  // cancels the timer and tells native code to release its reference.
      }
    }
    */
 
   (function (params) {

        let abtostr = function (buf, start, len) {
            return String.fromCharCode.apply (this, new Uint16Array (buf, start, len));
        };

        let abtostr8 = function (buf, start, len) {
            return String.fromCharCode.apply (this, new Uint8Array (buf, start, len));
        };

        let strtoab = function (str) {
            let buffer = new ArrayBuffer (str.length * 2);
            let view = new Uint16Array (buffer);
                
            for (var i = 0; i != str.length; ++i)
                view [i] = str.charCodeAt (i);
                    
            return buffer;
        };

        let strtoab8 = function (str) {
            let buffer = new ArrayBuffer (str.length);
            let view = new Uint8Array (buffer);
                
            for (var i = 0; i != str.length; ++i)
                view [i] = str.charCodeAt (i);
                    
            return buffer;
        };

        let uv_fs_open  = params.uv_fs_open;
        let uv_fs_realpath  = params.uv_fs_realpath;
        let uv_fs_stat  = params.uv_fs_stat;
        let uv_fs_read  = params.uv_fs_read;
        let uv_fs_close = params.uv_fs_close;

        let uv_fs_isValid = function (file)
            {
            return file >= 0;
            };

        let exports = {
            abtostr: abtostr,
            abtostr8: abtostr8,
            strtoab: strtoab,
            strtoab8: strtoab8,

            fs: {
                O_RDONLY:     params.uv_fs_O_RDONLY,
                O_WRONLY:     params.uv_fs_O_WRONLY,
                O_RDWR:       params.uv_fs_O_RDWR,
                O_APPEND:     params.uv_fs_O_APPEND,
                O_CREAT:      params.uv_fs_O_CREAT,
                O_TRUNC:      params.uv_fs_O_TRUNC,
                O_EXCL:       params.uv_fs_O_EXCL,
                O_TEXT:       params.uv_fs_O_TEXT,
                O_BINARY:     params.uv_fs_O_BINARY,
                O_RAW:        params.uv_fs_O_RAW,
                O_TEMPORARY:  params.uv_fs_O_TEMPORARY,
                O_NOINHERIT:  params.uv_fs_O_NOINHERIT,
                O_SEQUENTIAL: params.uv_fs_O_SEQUENTIAL,
                O_RANDOM:     params.uv_fs_O_RANDOM,

                S_IFMT:   params.uv_fs_S_IFMT,
                S_IFDIR:  params.uv_fs_S_IFDIR,
                S_IFCHR:  params.uv_fs_S_IFCHR,
                S_IFREG:  params.uv_fs_S_IFREG,
                S_IREAD:  params.uv_fs_S_IREAD,
                S_IWRITE: params.uv_fs_S_IWRITE,
                S_IEXEC:  params.uv_fs_S_IEXEC,

                open:    uv_fs_open,
                realpath: uv_fs_realpath,
                stat:    uv_fs_stat,
                isValid: uv_fs_isValid,
                read:    uv_fs_read,
                close :  uv_fs_close
            }
        };

        return exports;
    });

    )";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
Utf8CP Host::InitScript()
    {
    return u8R"(

    (function (params) {
        //todo: reorganize all this
        let originalRequire = null;
        let extensionCache = {};
        let nextIdleCallbacks = [];
        let shutDownHandlers = [];

        if (!params.hasOwnProperty("deliverExtension"))
            throw new Error("deliverExtension not found on params");

        if (!params.hasOwnProperty("evaluateScript"))
            throw new Error("evaluateScript not found on params");

        params.notifyIdle = function() {
            let l = nextIdleCallbacks.length;
            for (var i = 0; i != l; ++i) 
                nextIdleCallbacks [i].call (this);

            nextIdleCallbacks = nextIdleCallbacks.slice (l);
        };

        params.notifyShutdown = function() {
            for (var i = 0; i != shutDownHandlers.length; ++i) 
                shutDownHandlers [i].call (this);
        };

        params.replacementRequire = function (identifier) {

            if (extensionCache.hasOwnProperty (identifier))
                return extensionCache [identifier];

            let extensionContents = params.deliverExtension(identifier);
            if (typeof (extensionContents) !== "undefined") {
                extensionCache [identifier] = extensionContents;
                return extensionContents;
            }

            let moduleContents = params.require.apply (this, arguments);
            if (typeof (moduleContents) !== "undefined")
                return moduleContents;

            if (originalRequire !== null) {
                return originalRequire.apply (this, arguments);
            }
        };

        if (typeof (require) !== "undefined") {
            let Module = require ("module");
            originalRequire = Module.prototype.require;
            Module.prototype.require = params.replacementRequire;
        } else {
            var require = params.replacementRequire;
        }

        let parseArgv = function (argv) {
            let result = { executable: "", options: [], data: [], arguments: [] };
            argv = argv.trim();
            let tokens = argv.split (/\s+/);
            
            result.executable = tokens [0];

            let restAreArgs = false;
            for (var i = 1; i !== tokens.length; ++i) {
                let token = tokens [i];

                if (restAreArgs) {
                    result.arguments.push (token);
                } else {
                    if (token [0] === "-") {
                        result.options.push (token)

                        if (token === "-" || token === "--")
                            restAreArgs = true;
                    } else {
                        result.data.push (token);
                    }
                }
            }

            return result;
        };

        params.notifyReady = function() {
            let info = bentley.imodeljs.servicesTier.getHostInfo();

            let argv = parseArgv (info.argv);
            let initialScript = (argv.data.length !== 0) ? argv.data [argv.data.length - 1] : null;

            if (initialScript !== null)
                {
                if (initialScript [0] !== '/' && initialScript.indexOf (':') === -1)
                    {
                    initialScript = info.cwd + '/' + initialScript;
                    
                    let offset = initialScript.indexOf (':');
                    if (offset !== -1)
                        initialScript = initialScript.slice (offset + 1);
                    }

                require (initialScript);
                }
        };

        //todo: move these to utilities (ie, not at global scope)

        if (!this.hasOwnProperty ("bentley"))
            this.bentley = {};

        if (!this.bentley.hasOwnProperty ("imodeljs"))
            this.bentley.imodeljs = {};

        if (!this.bentley.imodeljs.hasOwnProperty ("servicesTier"))
            this.bentley.imodeljs.servicesTier = {};

        this.bentley.imodeljs.servicesTier.getHostInfo = function() {
            return params.info;
        };

        this.bentley.imodeljs.servicesTier.scheduleIdleCallback = function (callback) {
            if (typeof (callback) !== "function")
                return;

            nextIdleCallbacks.push (callback);
        };

        this.bentley.imodeljs.servicesTier.registerShutdownHandler = function (handler) {
            if (typeof (handler) !== "function")
                return;

            shutDownHandlers.push (handler);
        };

        this.bentley.imodeljs.servicesTier.evaluateScript = function (script, identifier) {
            if (arguments.length === 1)
                var identifier = "";

            return params.evaluateScript (script, identifier);
        };

        this.bentley.imodeljs.servicesTier.createStringFromUtf8Buffer = function (buffer) {
            return params.createStringFromUtf8Buffer (buffer);
        };

        this.bentley.imodeljs.servicesTier.require = require;
    });

    )";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
Utf8CP UvHost::RequireScript()
    {
    return u8R"(

    (function (params) {
        let parentModule = null;
        let moduleCache = {};

        let utilities = null;



        class Path {
            constructor (path) {
                if (typeof (path) === "string")
                    this.components = Path.normalize (path).split ('/');
                else if (path instanceof Path)
                    this.components = path.components;
                else if (path instanceof Array)
                    this.components = path;
                else
                    this.components = [];
            }

            static normalize (path) {
                var path2 = path.trim().replace (/["']/g, '').replace (/\\/g, '/').replace (/\/\//g, '/');
                return utilities.fs.realpath(path2);
            }

            toString() {
                return this.components.join ('/');
            }

            front() {
                if (this.components.length === 0)
                    return "";

                return this.components [0];
            }

            back() {
                if (this.components.length === 0)
                    return "";

                return this.components [this.components.length - 1];
            }

            empty() {
                return this.components.length === 0;
            }

            ascend() {
                return new Path (this.components.slice (0, -1));
            }

            append (data) {
                if (typeof (data) === "string")
                    this.components.push (data);
                else if (data instanceof Path)
                    Array.prototype.push.apply (this.components, data.components);
                else if (data instanceof Array)
                    Array.prototype.push.apply (this.components, data);

                return this;
            }

            prepend (data) {
                if (typeof (data) === "string")
                    this.components.unshift (data);
                else if (data instanceof Path)
                    Array.prototype.unshift.apply (this.components, data.components);
                else if (data instanceof Array)
                    Array.prototype.unshift.apply (this.components, data);

                return this;
            }
        }

        let requirePrefix = "(function (exports, require, module, __filename, __dirname) {\n";
        let requireSuffix = "\n});";
        
        class Module {
            constructor (id, parent) {
                id = Module.normalizeId (id);

                this.id = id;
                this.filename = id;
                this.exports = {};
                this.parent = parent;
                this.loaded = false;
                //todo: children
            }

            static normalizeId (id) {
                while (id.indexOf ("/./") !== -1)
                    id = id.replace ("/./", '/');

                return id;
            }

            load() {
                let require = makeRequireFunction (this);
                let contents = requirePrefix + readFileUtf8 (this.filename) + requireSuffix;
                let compiled = bentley.imodeljs.servicesTier.evaluateScript (contents, "file:///" + this.filename);
                let result = compiled.call (this.exports, this.exports, require, this, this.filename, dirname (this.filename));
                //todo json

                this.loaded = true;
            }

            loadJson() {
                this.exports = JSON.parse(readFileUtf8(this.filename));
                this.loaded = true;
            }

            require (identifier) {
                parentModule = this;

                return params.replacementRequire (identifier);
            }
        }

        let makeRequireFunction = function (mod) {
            function require (path) {

                return mod.require (path);
            }

            //todo require.resolve, main, cache

            return require;
        };

        let stat = function (identifier) {
            let info = null;

            let file = utilities.fs.open (identifier, utilities.fs.O_RDONLY, 0);
            if (utilities.fs.isValid (file)) {
                info = utilities.fs.stat (file);
                utilities.fs.close (file);
                }

            return info;
        };

        let isFile = function (identifier) {
            let info = stat (identifier);
            return (info === null) ? false : info.isFile;
        };

        let isDirectory = function (identifier) {
            let info = stat (identifier);
            return (info === null) ? false : info.isDirectory;
        };

        let dirname = function (path) {
            return new Path (path).ascend().toString();
        };

        let readFileUtf8 = function (path) {
            let file = utilities.fs.open (path, utilities.fs.O_RDONLY, 0);
            let contents = new ArrayBuffer (utilities.fs.stat (file).size);
            utilities.fs.read (file, contents);
            utilities.fs.close (file);

            return bentley.imodeljs.servicesTier.createStringFromUtf8Buffer (contents);
        };

        let getNodeModulesPaths = function (start) {
            let parts = new Path (start).components;
            let dirs = [];
            
            let i = parts.length - 1;
            while (i >= 0) {
                if (parts [i] !== "node_modules") {
                    let dir = parts.slice (0, i + 1).join ('/') + "/node_modules";
                    dirs.push (dir);
                }
                --i;
            }

            return dirs;
        };

        let result = null;

        let loadFirstExisting = function (paths) {
            for (var i = 0; i != paths.length; ++i) {
                if (isFile (paths [i])) {
                    result = new Module (paths [i], parentModule);
                    return;
                }
            }
        }
        
        let loadAsFile = function (identifier) {
            if (result !== null) return;
            loadFirstExisting ([identifier, identifier + ".js", identifier + ".json"]);
        };

        let loadAsIndex = function (identifier) {
            if (result !== null) return;
            loadFirstExisting ([identifier, identifier + "/index.js", identifier + "/index.json"]);
        };

        let loadAsDirectory = function (identifier) {
            if (result !== null) return;

            let configPath = identifier + "/package.json";
            if (isFile (configPath)) {
                let config = JSON.parse (readFileUtf8 (configPath));
                if (config.hasOwnProperty ("main"))
                    {
                    let mainPath = identifier + "/" + config.main;

                    loadAsFile (mainPath);
                    if (result !== null) return;

                    loadAsIndex (mainPath);
                    if (result !== null) return;
                    }
                }

            loadAsIndex (identifier);
        };

        let loadFromNodeModules = function (identifier, start) {
            if (result !== null) return;

            let dirs = getNodeModulesPaths (start);

            //console_log('loadFromNodeModules(identifier=' + identifier + ', start=' + JSON.stringify(start) + ')');
            //console_log('dirs to try = ' + JSON.stringify(dirs));

            for (var i = 0; i != dirs.length; ++i) {
                let qualifiedIdentifier = dirs [i] + "/" + identifier;

                loadAsFile (qualifiedIdentifier);
                if (result !== null) return;

                loadAsDirectory (qualifiedIdentifier);
                if (result !== null) return;
            }
        };

        var indent = 0;

        let load = function (identifier) {
            if (utilities === null)
                utilities = params.replacementRequire ("@bentley/imodeljs-services-tier-utilities");
            
            result = null;
            
            console_log('load(' + indent + ' ' + identifier + ')');

            let info = bentley.imodeljs.servicesTier.getHostInfo();

            if (parentModule === null)
                parentModule = new Module (info.cwd, null);

            let parentPrefix = parentModule.filename;
            if (isFile (parentPrefix))
                parentPrefix = dirname (parentPrefix);

            let normalizedIdentifier = Path.normalize (identifier);
            let resolvedIdentifier = normalizedIdentifier;
            let tryFs = false;

            if (normalizedIdentifier [0] === '/') {
                tryFs = true;

                if (info.isWindows && normalizedIdentifier.indexOf (':') === -1)
                    resolvedIdentifier = new Path (info.cwd).front() + normalizedIdentifier;
            } else if (normalizedIdentifier.indexOf ("./") === 0 || normalizedIdentifier.indexOf ("..") === 0) {
                tryFs = true;
                resolvedIdentifier = parentPrefix + '/' + normalizedIdentifier;
            }

            resolvedIdentifier = Path.normalize (resolvedIdentifier);

            if (tryFs) {
                loadAsFile (resolvedIdentifier);
                loadAsDirectory (resolvedIdentifier);
            }

            loadFromNodeModules (identifier, parentPrefix);

            if (result !== null) {
                if (moduleCache.hasOwnProperty (result.id)) {
                    console_log('cached module: ' + result.id + '\n');
                    return moduleCache [result.id].exports;
                }

                let module = result;
                moduleCache [module.id] = module;

                if (module.id.endsWith('.json')) {
                    module.loadJson();
                } else {
                    indent = indent + 1;
                    module.load(); // Note this may call require recursively (redefining 'result' each time)
                    indent = indent - 1;
                    if (!module.loaded) {
                        console_log('load failed.\n');
                        delete moduleCache [module.id];
                    }                
                }                
                console_log('loaded module: ' + indent + ' ' + module.id + '\n');
                return module.exports;
            } else {
                throw new Error ("Cannot find module '" + identifier + "'");
            }
        };

        return load;
    });

    )";
    }

END_BENTLEY_IMODELJS_SERVICES_TIER_NAMESPACE
