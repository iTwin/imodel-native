/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
export declare namespace uv {
    export enum StatusCode {
        SUCCESS            = 0,
        UV_E2BIG           = 1,  //argument list too long
        UV_EACCES          = 2,  //permission denied
        UV_EADDRINUSE      = 3,  //address already in use
        UV_EADDRNOTAVAIL   = 4,  //address not available
        UV_EAFNOSUPPORT    = 5,  //address family not supported
        UV_EAGAIN          = 6,  //resource temporarily unavailable
        UV_EAI_ADDRFAMILY  = 7,  //address family not supported
        UV_EAI_AGAIN       = 8,  //temporary failure
        UV_EAI_BADFLAGS    = 9,  //bad ai_flags value
        UV_EAI_BADHINTS    = 10, //invalid value for hints
        UV_EAI_CANCELED    = 11, //request canceled
        UV_EAI_FAIL        = 12, //permanent failure
        UV_EAI_FAMILY      = 13, //ai_family not supported
        UV_EAI_MEMORY      = 14, //out of memory
        UV_EAI_NODATA      = 15, //no address
        UV_EAI_NONAME      = 16, //unknown node or service
        UV_EAI_OVERFLOW    = 17, //argument buffer overflow
        UV_EAI_PROTOCOL    = 18, //resolved protocol is unknown
        UV_EAI_SERVICE     = 19, //service not available for socket type
        UV_EAI_SOCKTYPE    = 20, //socket type not supported
        UV_EALREADY        = 21, //connection already in progress
        UV_EBADF           = 22, //bad file descriptor
        UV_EBUSY           = 23, //resource busy or locked
        UV_ECANCELED       = 24, //operation canceled
        UV_ECHARSET        = 25, //invalid Unicode character
        UV_ECONNABORTED    = 26, //software caused connection abort
        UV_ECONNREFUSED    = 27, //connection refused
        UV_ECONNRESET      = 28, //connection reset by peer
        UV_EDESTADDRREQ    = 29, //destination address required
        UV_EEXIST          = 30, //file already exists
        UV_EFAULT          = 31, //bad address in system call argument
        UV_EFBIG           = 32, //file too large
        UV_EHOSTUNREACH    = 33, //host is unreachable
        UV_EINTR           = 34, //interrupted system call
        UV_EINVAL          = 35, //invalid argument
        UV_EIO             = 36, //i/o error
        UV_EISCONN         = 37, //socket is already connected
        UV_EISDIR          = 38, //illegal operation on a directory
        UV_ELOOP           = 39, //too many symbolic links encountered
        UV_EMFILE          = 40, //too many open files
        UV_EMSGSIZE        = 41, //message too long
        UV_ENAMETOOLONG    = 42, //name too long
        UV_ENETDOWN        = 43, //network is down
        UV_ENETUNREACH     = 44, //network is unreachable
        UV_ENFILE          = 45, //file table overflow
        UV_ENOBUFS         = 46, //no buffer space available
        UV_ENODEV          = 47, //no such device
        UV_ENOENT          = 48, //no such file or directory
        UV_ENOMEM          = 49, //not enough memory
        UV_ENONET          = 50, //machine is not on the network
        UV_ENOPROTOOPT     = 51, //protocol not available
        UV_ENOSPC          = 52, //no space left on device
        UV_ENOSYS          = 53, //function not implemented
        UV_ENOTCONN        = 54, //socket is not connected
        UV_ENOTDIR         = 55, //not a directory
        UV_ENOTEMPTY       = 56, //directory not empty
        UV_ENOTSOCK        = 57, //socket operation on non-socket
        UV_ENOTSUP         = 58, //operation not supported on socket
        UV_EPERM           = 59, //operation not permitted
        UV_EPIPE           = 60, //broken pipe
        UV_EPROTO          = 61, //protocol error
        UV_EPROTONOSUPPORT = 62, //protocol not supported
        UV_EPROTOTYPE      = 63, //protocol wrong type for socket
        UV_ERANGE          = 64, //result too large
        UV_EROFS           = 65, //read-only file system
        UV_ESHUTDOWN       = 66, //cannot send after transport endpoint shutdown
        UV_ESPIPE          = 67, //invalid seek
        UV_ESRCH           = 68, //no such process
        UV_ETIMEDOUT       = 69, //connection timed out
        UV_ETXTBSY         = 70, //text file is busy
        UV_EXDEV           = 71, //cross-device link not permitted
        UV_UNKNOWN         = 72, //unknown error
        UV_EOF             = 73, //end of file
        UV_ENXIO           = 74, //no such device or address
        UV_EMLINK          = 75, //too many links
    }

    export class Handle {
        public close() : void;
    }

    export class Status {
        code : StatusCode;
        success() : boolean;
    }

    export namespace io {
        export interface StreamAllocator {
            allocate (suggestedSize : number) : ArrayBuffer;
            recycle (buffer : ArrayBuffer) : boolean;
        }

        export interface StreamReadDataCallback {
            (status : Status, data : ArrayBuffer, nread : number) : boolean;
        }

        export class Stream extends Handle {
            public isReadable() : boolean;
            public isWritable() : boolean;

            public read (allocator : StreamAllocator, readCallback : StreamReadDataCallback) : Status;
            public write (data : ArrayBuffer) : Promise<Status>;
        }

        export function shutdown (handle : Stream) : Promise<Status>;

        export function createDefaultStreamAllocator() : StreamAllocator;
    }

    export namespace net {
        export enum IP { V4 = 0, V6 = 1 }
    }

    export namespace tcp {
        export class BindResult extends Status {
            server : Server;
        }

        export class ConnectResult extends Status {
            connection : Handle;
        }

        export interface ServerListenCallback {
            (connection : Handle, status : Status) : void;
        }

        export class Handle extends io.Stream {
            public setNoDelay (enable : boolean) : Status;
            public setKeepAlive (enable : boolean, delay : number) : Status;
        }

        export class Server extends Handle {
            public setSimultaneousAccepts (enable : boolean) : Status;
            public listen (backlog : number, callback : ServerListenCallback) : Status;
            public accept (connection : Handle) : Status;
        }

        export function bind (address : string, port : number, protocol : net.IP) : BindResult;
        export function connect (address : string, port : number, protocol : net.IP) : Promise<ConnectResult>;
    }
}

export declare namespace websocketpp {
    export class ReadOnlyArrayBuffer extends ArrayBuffer {}

    export enum OpCode {
        Continuation = 0x0,
        Text         = 0x1,
        Binary       = 0x2,
        Close        = 0x8,
        Ping         = 0x9,
        Pong         = 0xA,
    }

    export interface Handler {
        open ?: () => void;
        fail ?: () => void;
        message: (data : ArrayBuffer, code : OpCode) => void;
        transport: (data : ReadOnlyArrayBuffer) => void;
    }

    export class Base {
        Dispose() : void;
    }

    export class ClientConnection extends Base {
        handler : Handler;
        process (input : ArrayBuffer, offset : number, length : number) : boolean;
        send (message : ArrayBuffer, code : OpCode) : boolean;
    }

    export class ServerEndpoint extends Base {
        constructor();
        createConnection() : ClientConnection;
    }
}

export declare namespace fs {
    export type FileHandle = number;

    export var O_RDONLY : number;
    export var O_WRONLY : number;
    export var O_RDWR : number;
    export var O_APPEND : number;
    export var O_CREAT : number;
    export var O_TRUNC : number;
    export var O_EXCL : number;
    export var O_TEXT : number;
    export var O_BINARY : number;
    export var O_RAW : number;
    export var O_TEMPORARY : number;
    export var O_NOINHERIT : number;
    export var O_SEQUENTIAL : number;
    export var O_RANDOM : number;

    export var S_IFMT : number;
    export var S_IFDIR : number;
    export var S_IFCHR : number;
    export var S_IFREG : number;
    export var S_IREAD : number;
    export var S_IWRITE : number;
    export var S_IEXEC : number;

    export interface FileInfo {
        size : number;
        isFile : boolean;
        isDirectory : boolean;
    }
    
    export function open (path : string, flags : number, mode : number) : FileHandle;
    export function stat (file : FileHandle) : FileInfo;
    export function isValid (file : FileHandle) : boolean;
    export function read (file : FileHandle, destination : ArrayBuffer) : boolean;
    //todo: write
    export function close (file : FileHandle) : boolean;
}
