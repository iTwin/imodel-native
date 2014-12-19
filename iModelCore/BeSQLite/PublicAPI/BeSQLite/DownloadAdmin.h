/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/BeSQLite/DownloadAdmin.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include "BeSQLite.h"

#define BEGIN_DOWNLOAD_NAMESPACE BEGIN_BENTLEY_SQLITE_NAMESPACE namespace Download {
#define END_DOWNLOAD_NAMESPACE   } END_BENTLEY_SQLITE_NAMESPACE
#define USING_NAMESPACE_DOWNLOAD using namespace BentleyApi::BeSQLite::Download;

BEGIN_DOWNLOAD_NAMESPACE

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   11/13
//=======================================================================================
enum DownloadStatus
{
    DOWNLOAD_Success                    = SUCCESS,
    DOWNLOAD_VcaNotFound                = 0x100,
    DOWNLOAD_CantOpenVcaForRead         = 0x101,
    DOWNLOAD_CantCreateVca              = 0x102,
    DOWNLOAD_VcaReadError               = 0x103,
    DOWNLOAD_VcaWriteError              = 0x104,
    DOWNLOAD_CantCreateLocalFile        = 0x105,
    DOWNLOAD_LocalFileNotFound          = 0x106,
    DOWNLOAD_CantReopenLocalFile        = 0x107,
    DOWNLOAD_LocalFileWriteError        = 0x108,
    DOWNLOAD_ChunkSizeTooSmall          = 0x109,
    DOWNLOAD_ConnectionDoesNotMatch     = 0x10a,
    DOWNLOAD_Timeout                    = 0x10b,
    DOWNLOAD_CorruptData                = 0x10c,

    SERVER_CantFindServer               = 0x200,
    SERVER_InvalidUri                   = 0x201,
    SERVER_InvalidCredentials           = 0x202,
    SERVER_CantFindFile                 = 0x203,
    SERVER_FileReadError                = 0x204,
    SERVER_FileOutOfDate                = 0x205,
    SERVER_InvalidRequest               = 0x206,
};

//=======================================================================================
//! To allow concurrency between the server thread (that responds to messages from the server, and potentially decompresses)
//! and the receiver thread (that writes to the local file), we buffer downloaded chunks in memory in a queue.
//! That queue holds instances of this class with the actual (decompressed) data for a chunk.
// @bsiclass                                                    Keith.Bentley   10/13
//=======================================================================================
struct ChunkBuffer
{
private:
    byte*   m_data;
    UInt32  m_nBytes;
    UInt32  m_chunkNo;

public:
    void* operator new(size_t size) {return bentleyAllocator_allocateRefCounted (size);}
    void operator delete(void* rawMemory, size_t size) {bentleyAllocator_deleteRefCounted (rawMemory, size);}

    ChunkBuffer (UInt32 chunkNo, UInt32 nBytes)
        {
        m_nBytes=nBytes;
        m_data=(byte*) bentleyAllocator_malloc(nBytes);
        m_chunkNo=chunkNo;
        }
    ~ChunkBuffer() {bentleyAllocator_free ((void*)m_data, m_nBytes);}
    byte* GetData() const {return m_data;}
    UInt32 GetChunkNo() const {return m_chunkNo;}
    UInt32 GetNBytes() const {return m_nBytes;}
};

/*---------------------------------------------------------------------------------**//**
* Server-supplied information about a file to be downloaded. Every time the IDownloadAdmin
* attempts to reconnect with the server to continue downloading a file, this structure must match
* the previous version, or the download will not proceed.
* @bsiclass                                     Keith.Bentley                   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
struct FileStats
{
private:
    UInt64  m_fileSize;
    UInt64  m_signature;
    UInt32  m_chunkSize;

public:
    UInt64 GetFileSize() const {return m_fileSize;}
    UInt32 GetChunkSize() const {return m_chunkSize;}
    UInt32 GetNChunks() const {BeAssert(m_chunkSize!=0 && m_fileSize!=0); return (UInt32) ((m_fileSize-1)/m_chunkSize) + 1;}
    UInt32 GetSizeForChunkNo (UInt32 chunkNo) 
        {
        BeAssert(chunkNo<GetNChunks()); 
        UInt32 retval = (chunkNo == (GetNChunks()-1)) ? (m_fileSize%m_chunkSize) : m_chunkSize;
        return retval ? retval : m_chunkSize;  //  retval is 0 when the file is an exact multiple of the chunk size.
        }
    bool Matches (FileStats const& other) const {return 0==memcmp(this, &other, sizeof(*this));}
    FileStats(UInt64 fileSize, UInt32 chunkSize, UInt64 sig=0) {m_fileSize = fileSize; m_signature = sig; m_chunkSize=chunkSize;}
};

//=======================================================================================
//! Supplied by BeSQLite in the IDownloadAdmin::_CreateConnection method to receive chunks of a downloaded file
//! as they arrive from the server.
// @bsiclass                                                    Keith.Bentley   10/13
//=======================================================================================
struct IReceiver
{
    //! Connections should call this method when a chunk of a downloaded file is available.
    //! @param chunk The newly arrived file chunk. Must be allocated via "new".
    //! @note the chunk should be created by the Connection via "new" and becomes owned by BeSQLite
    //! after this call. It will be freed by BeSQLite after the chunk has been saved in the local file.
    virtual void _OnChunkReceived(ChunkBuffer& chunk) = 0;
};

//=======================================================================================
//! Configuration options that control the behavior of a download Connection.
// @bsiclass                                                    Keith.Bentley   10/13
//=======================================================================================
struct DownloadConfig
{
    UInt32 m_saveInterval;
    UInt32 m_backgroundDelay;
    UInt32 m_timeout;

    //! Construct a DownloadConfig with the default values
    DownloadConfig() {m_saveInterval=5000; m_backgroundDelay=1000; m_timeout=20000;}

    //! Construct a DownloadConfig.
    //! @param saveInterval
    //! The amount of time, in milliseconds, between saving the downloaded data to disk. BeSQLite periodically ensures that the downloaded
    //! data is flushed to disk and records the download progress to a sister "download progress file" with the extension "-dl". This value
    //! controls how often that happens. If the application crashes during download, only information before the last save
    //! will be kept when the session resumes. Shorter values mean that if the application crashes during the download, less
    //! data will need to be re-downloaded the session, but also slows down the download process.
    //! @note When the download completes, the "-dl" file is deleted.

    //! @param backgroundDelay
    //! The amount of time, in milliseconds, to wait before requesting a background chunk. When the download manager detects that there
    //! are no pending chunk requests, it starts downloading speculative chunks in the
    //! background. This value controls how long to wait before starting a background request. Shorter values mean
    //! that the download manager will be more aggressive about requesting background chunks. However, if a background chunk is being
    //! downloaded when a direct request for a page from BeSQLite arrives, it is forced to wait (block) longer for the foreground chunk
    //! to arrive.

    //! @param timeout
    //! The amount of time, in milliseconds, to wait before deciding that a chunk request has timed out. When a Connection times out,
    //! the application is forced to terminate, since it cannot read from its local database file.
    DownloadConfig(UInt32 saveInterval, UInt32 backgroundDelay, UInt32 timeout) :
        m_saveInterval(saveInterval), m_backgroundDelay(backgroundDelay), m_timeout(timeout) {}
};

//=======================================================================================
//! An application-supplied object used by the download manager to request file chunks from a server. Applications should create a subclass
//! that implements the virtual methods as appropriate.
// @bsiclass                                                    Keith.Bentley   10/13
//=======================================================================================
struct Connection
{
protected:
    FileStats m_stats;
    Download::IReceiver*  m_receiver;

public:
    //! Called by the download manager to request that the Connection download a single chunk from the server.
    //! Implementers should call  IReceiver::_OnChunkReceived within this method to return the data for the chunk when
    //! it is available.
    //! @param chunkNo The required chunk to download.
    virtual void _RequestChunk(UInt32 chunkNo) = 0;

    //! Called when the IReceiver thread exits. After this call it is illegal to access the m_receiver member of this
    //! structure.
    virtual void _OnReceiverExit() {m_receiver = NULL;}

    //! Called when the Connection should close and delete itself. In addition to closing the connection, this method
    //! should, as its last line, call "delete this".
    virtual void _Destroy() = 0;

    //! Return the DownloadConfig object for this Connection.
    virtual DownloadConfig _GetOptions() const {return DownloadConfig(5000, 1000, 20000);}

    //! Create a new instance of a Connection. Subclasses will hold additional information about the state of the
    //! Connection.
    Connection(Download::IReceiver& receiver) : m_stats(0,0,0), m_receiver(&receiver){}

    //! Get the FileStats about this Connection. The IDownloadAdmin will call this method on Connection objects returned
    //! from its _CreateConnection method.
    FileStats GetFileStats () {return m_stats;}

    //! dtor for Connection
    virtual  ~Connection() {}
};

END_DOWNLOAD_NAMESPACE

BEGIN_BENTLEY_SQLITE_NAMESPACE

//=======================================================================================
//! Applications that wish to support incremental downloading of BeSQLite files implement this interface and
//! supply it to BeSQLite via the BeSQLiteLib::SetDownloadAdmin method. Once a IDownloadAdmin is installed,
//! it remains active for the entire session and cannot be uninstalled or replaced.
// @bsiclass                                                    Keith.Bentley   10/13
//=======================================================================================
struct IDownloadAdmin
{
    // Internal use only
    BE_SQLITE_EXPORT Utf8CP GetVfs();

    //! Initialize the process of incremental downloading of a BeSQLite file from a server. This method creates the
    //! local file that can then be used as a "regular" BeSQLite file by applications, but will be downloaded
    //! incrementally as needed.
    //! @param localFileName The full path of the local file that will hold the downloaded data. Until the download completes, this file
    //! may only be accessed when this IDownloadAdmin is present, and then only while an active Connection can be established to the server.
    //! After the download finishes, this file becomes a "normal" BeSQLite database.
    //! This method will attempt to connect to the server (via _CreateConnection) and will always prefetch the first @c prefetchBytes bytes
    //! in the file before it returns.
    //! @param serverUri The URI to be used to contact a (remote) server to download this file. Normally this URI will include the remote
    //! file specification, but its format is entirely determined by the implementation of the _CreateConnection call. This string will be
    //! stored persistently locally so the connection can be reestablished in future sessions.
    //! @param prefetchBytes The number of bytes at the beginning of the file to be downloaded now (minimum is 4K)
    //! @return SUCCESS if the connection was successful and the local file was created with at least prefetchBytes valid bytes in it.
    //! Error indication otherwise.
    BE_SQLITE_EXPORT Download::DownloadStatus CreateLocalDownloadFile (Utf8String localFileName, Utf8String serverUri, Utf8String serverFileSpec, UInt32 prefetchBytes);

    //! Establish a new Connection to the server.
    //! @param serverUri The URI originally supplied to CreateLocalDownloadFile that specifies the remote file.
    //! @param receiver The IReceiver that the newly created Connection should use to return chunks after they are downloaded.
    //! @param status An indication of what went wrong in the case where return value is NULL.
    //! @return A Connection object to use to request chunks as necessary. If the connection cannot be made, return NULL.
    virtual Download::Connection* _CreateConnection(Utf8String hostUri, Utf8String filespec, Download::IReceiver& receiver, Download::DownloadStatus& status) = 0;

    //! Called after each chunk has been downloaded. This method can be used give feedback that the download is proceeding and its percentage complete.
    //! @param localFile The name of the localFile that just received a new chunk.
    //! @param conn The Connection object that supplied the chunk.
    //! @param nChunksValid The number of chunks that are now valid in the localFile. The Connection object knows the total number of chunks
    //! in the remote file, so percentage complete can be determined.
    virtual void _OnDownloadProgress(Utf8CP localFile, Download::Connection& conn, UInt32 nChunksValid) {}

    //! Called after the entire file has been successfully downloaded.
    //! @param conn The Connection that supplied the download.
    virtual void _OnDownloadComplete(Download::Connection& conn) {}

    //! Called when there has been no response from the server within the timeout period specified in the DownloadConfig for a Connection.
    //! Generally this method is used to provide an alert to the user that the Connection failed, asking if s/he wants to retry.
    //! @param conn The Connection that timed out.

    //! @return true if the download manager should continue to wait for the server, false to give up. If this method returns false,
    //! the application must generally be terminated, since the information in the needed chunk can not be supplied to BeSQLite, and there is no way of
    //! predicting the adverse effect that may have on the application.
    virtual bool _OnDownloadTimeout(Download::Connection& conn) {return false; /*no retry*/}
};

END_BENTLEY_SQLITE_NAMESPACE
