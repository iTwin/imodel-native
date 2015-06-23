/*--------------------------------------------------------------------------------------+
|
|     $Source: SQLite/DownloadVfs.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <BeSQLite/DownloadAdmin.h>
#include <BeSQLite/SQLiteAPI.h>
#include <Bentley/ScopedArray.h>
#include <Bentley/BeThread.h>
#include <Bentley/BeThreadLocalStorage.h>
#include <Bentley/BeTimeUtilities.h>
#include <queue>
#include <deque>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_DOWNLOAD

#define INVALID_CHUNK       0xffffffff
#define MINIMUM_CHUNK_SIZE  (4*1024)

struct DlTreadId : BeThreadLocalStorage
{
    enum DlVfsThreadType
        {
        DLVFS_THREAD_TYPE_Requester = 2,
        DLVFS_THREAD_TYPE_Receiver  = 3,
        };

    void SetIsReceiverThread() {SetValueAsInteger(DLVFS_THREAD_TYPE_Receiver);}
    void SetIsRequesterThread() {SetValueAsInteger(DLVFS_THREAD_TYPE_Requester);}
    bool InMainThread()     {intptr_t val = GetValueAsInteger(); return val==0;}
    bool InRequesterThread()   {return GetValueAsInteger() == DLVFS_THREAD_TYPE_Requester;}
    bool InReceiverThread() {return GetValueAsInteger() == DLVFS_THREAD_TYPE_Receiver;}
};

//  #define DO_DBG_PRINTF
#if defined (DO_DBG_PRINTF)
    static int s_dbgLevel = 1;
    #define DBG_LOG(LEVEL,...) {if (LEVEL<=s_dbgLevel) printf (__VA_ARGS__);}
#else
    #define DBG_LOG(LEVEL,...)
#endif

static DlTreadId g_dlVfsThreadId; // strictly for debugging

//=======================================================================================
// The valid chunk array (VCA) has one bit for each "chunk" in a download file. If the bit is on, the chunk
// has been downloaded and written to the local file. The size of chunks is determined by the server (note, the last chunk may not be full sized).
// There is not necessarily any correlation between chunks and pages. Nothing in the download logic depends on knowing
// The page size for a database. Hence, this same logic can be used to download any type of file, though we use it here for SQLite files.
// A VCA also holds the name of the server from which the chunks are downloaded, the chunk size, and the total size of the full file.
// VCAs are periodically written into sister files next to the DownloadVfs database file in case the
// session ends before the file download is complete. In that way subsequent sessions know which chunks are valid and don't
// need to be re-downloaded. If the session crashes before the VCA is saved there is no harm - that simply means
// that some valid data may not be recognized as valid and will be re-downloaded in the future.
// @bsiclass                                                    Keith.Bentley   10/13
//=======================================================================================
struct ValidChunkArray
    {
private:
    FileStats  m_stats;
    uint32_t   m_nValid;
    uint32_t   m_nChunks;
    Utf8String m_serverUrl;
    Utf8String m_serverFileSpec;
    Utf8String m_localFileName;
    Utf8String m_vcaFileName;
    Byte*      m_validChunks;
    mutable BeDbMutex  m_mutex;

    Byte MaskForChunk(uint32_t chunkNo) const {return 1<<(chunkNo%8);}
    Byte ByteForChunk(uint32_t chunkNo) const {return m_validChunks[chunkNo/8];}
    Byte* ByteForChunkP(uint32_t chunkNo) {return m_validChunks + (chunkNo/8);}
    uint32_t GetNumBytes() const {return ((m_nChunks-1)/8) + 1;}
    Byte* GetData() {return m_validChunks;}

public:
    ValidChunkArray();
    ~ValidChunkArray() {free(m_validChunks); m_validChunks=NULL; m_nValid=0;}

    void SetupFileName(Utf8String baseName){m_localFileName=baseName; m_vcaFileName=baseName+"-dl";}
    void SetServerUrl(Utf8String serverUrl, Utf8String filespec){m_serverUrl = serverUrl; m_serverFileSpec = filespec; }
    bool CheckChunk(uint32_t chunkNo) const {return 0 != (ByteForChunk(chunkNo) & MaskForChunk(chunkNo));}
    bool IsChunkValid(uint32_t chunkNo) const;
    bool SetChunkValid(uint32_t chunkNo);
    uint32_t GetNValid() const {return m_nValid;}
    uint32_t GetChunkSize() const {return m_stats.GetChunkSize();}
    uint32_t GetNChunks() const {return m_nChunks;}
    uint32_t GetNextNeededChunk(uint32_t start) const;
    uint32_t CountChunksValid() const;
    Utf8StringCR GetServerUrl() const {return m_serverUrl;}
    Utf8StringCR GetServerFileSpec() const {return m_serverFileSpec;}
    Utf8StringCR GetVcaFileName() const {return m_vcaFileName;}
    Utf8StringCR GetLocalFileName() const {return m_localFileName;}
    FileStats const& GetFileStats() const {return m_stats;}

    bool Save(DownloadStatus& dlStat);
    bool Load(DownloadStatus& dlStat);
    void Initialize(FileStats const&, Utf8String baseName, Utf8String serverName, Utf8String serverFileSpec);
    };


//=======================================================================================
// I'm not sure if this queue should even exist - we really only ask for one chunk at a time.
// @bsiclass                                                    Keith.Bentley   10/13
//=======================================================================================
struct ChunkRequestQueue
{
    friend struct DownloadFile;
protected:
    bool                m_aborted;
    std::deque<uint32_t>  m_chunksToFetch;
    BeConditionVariable m_chunkNeeded;
    Connection&         m_connection;

    bool req_WaitForChunkRequest();
    void QueueChunkRequest(uint32_t chunkNo);      //  May be called in main thread due to direct request or receiver thread due to preemptive request.
    bool IsAborted() const {return m_aborted;}
    uint32_t req_GetNextRequestedChunk();
    void Abort();

public:
    ChunkRequestQueue(Connection& conn);
    Connection& GetConnection() const {return m_connection;}
    void req_RequestedChunkLoop();
};

//=======================================================================================
// The "chunk receive queue" object holds the chunks as they become available from the server (in the Requester thread)
// and writes them to the local sqlite db file in the Receiver thread. After a chunk is successfully written to the local db,
// it wakes the main thread so it will proceed if it is blocked waiting for a chunk to become valid.
// @bsiclass                                                    Keith.Bentley   10/13
//=======================================================================================
struct ChunkReceiveQueue : IReceiver
{
    friend struct DownloadFile;
private:
    bool                     m_aborted;
    std::queue<ChunkBuffer*> m_dlChunks;
    BeConditionVariable      m_chunkAvaliable;
    struct DownloadFile*     m_dlFile;

    virtual void _OnChunkReceived(ChunkBuffer&) override;

    bool recv_ProcessNextAvailableChunk();
    bool recv_WaitForChunkAvailable();
    bool IsAborted() const {return m_aborted;}
    void Abort();

public:
    ChunkReceiveQueue(DownloadFile& dlFile);
    virtual ~ChunkReceiveQueue() {;}
    void recv_ReceiveChunkLoop();
};

//=======================================================================================
// A DownloadFile works in conjunction with a "real" sqlite vfs as it loads pages into memory. Before
// reading the data from a local sqlite db, the vfs will call "wt_ReadyBytes". The DownloadFile checks
// whether the bytes requested are valid, and if not it requests them from the server and waits (blocks) for them to become valid.
// @bsiclass                                                    Keith.Bentley   09/13
//=======================================================================================
struct DownloadFile
{
private:
    friend struct ChunkReceiveQueue;

    bool                m_done;
    ValidChunkArray*    m_chunkArray;
    BeFile              m_dbFile;
    uint32_t            m_waitingChunk;
    uint32_t            m_lastRequestedChunkNo;
    DownloadConfig      m_config;
    uint32_t            m_receivedChunks;       // number of chunks received since last VCA save
    uint32_t            m_totalReceivedChunks;  // number of chunks received this session
    uint64_t            m_lastSaveTime;
    uint64_t            m_lastChunkRequest;
    BeConditionVariable m_chunkWait;
    ChunkRequestQueue*  m_requestQueue;
    ChunkReceiveQueue*  m_receiveQueue;
    IDownloadAdmin&     m_admin;

    Connection* StartConnection (IDownloadAdmin& admin, Utf8String serverUrl, Utf8String filespec, DownloadStatus& dlStat);
    DownloadStatus ReconnectToServer(IDownloadAdmin& admin);
    void RequestChunk(uint32_t chunkNo, bool directRequest);      //  called in SQLite thread for direct requests, RecvThread for preemptive.
    void wt_WaitForChunk();                                     //  called in SQLite thread for direct requests
    void recv_OnChunkAvailable (ChunkBuffer const&);
    void AbortQueues();
    bool FlushDb();
    void CheckPreemptiveRequest();
    DownloadStatus SaveVCA();
    void CheckSaveVCA();
    void OnDownloadComplete();
    bool wt_ReadyChunk (uint32_t chunkNo);

public:
    DownloadFile(IDownloadAdmin& admin);
    ~DownloadFile();

    DownloadStatus Restart(Utf8String dbFileName, IDownloadAdmin& admin);
    uint32_t GetChunkSize() const {return m_chunkArray->GetChunkSize();}
    uint64_t GetFileSize() const {return m_chunkArray->GetChunkSize() * m_chunkArray->GetNChunks();}
    bool IsChunkValid(uint32_t chunkNo) const {return m_done || m_chunkArray->IsChunkValid(chunkNo);}
    Utf8String GetServerUrl() const {return m_chunkArray->GetServerUrl();}
    bool wt_ReadyBytes(int64_t offset, int32_t numBytes);

    uint32_t GetSaveInterval() const {return m_config.m_saveInterval;}
    uint32_t GetPreemptiveDelay() const {return m_config.m_backgroundDelay;}
    uint32_t GetDownloadTimeout() const {return m_config.m_timeout;}
    DownloadStatus CreateDownloadFile (Utf8String localFileName, Utf8String serverUrl, Utf8String serverFileSpec, IDownloadAdmin& admin, uint32_t prefetchBytes);
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
ValidChunkArray::ValidChunkArray() : m_stats(0,0,0)
    {
    m_nChunks = 0;
    m_validChunks = 0;
    m_nValid = 0;
    }

/*---------------------------------------------------------------------------------**//**
* Initialize a VCA for a newly created dlVfs local database file.
* @bsimethod                                    Keith.Bentley                   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
void ValidChunkArray::Initialize(FileStats const& stats, Utf8String baseName, Utf8String serverUrl, Utf8String serverFileSpec)
    {
    m_stats = stats;
    m_nChunks = m_stats.GetNChunks();
    m_validChunks = (Byte*) calloc(GetNumBytes(), 1);
    m_nValid = 0;
    SetupFileName(baseName);
    SetServerUrl(serverUrl, serverFileSpec);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
static bool writeFile(BeFile& file, void const* data, uint32_t size)
    {
    uint32_t written;
    return BeFileStatus::Success==file.Write (&written, data, size) && (written==size);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
static bool readFile(BeFile& file, void* data, uint32_t size)
    {
    uint32_t bytesRead;
    return BeFileStatus::Success==file.Read(data, &bytesRead, size) && (bytesRead==size);
    }

/*---------------------------------------------------------------------------------**//**
* Save the current state of the VCA to a file in case the current session ends before the entire
* database becomes valid. VCA files are stored next to the DownloadFile database file with the suffix "-dl" appended.
* This method opens, writes, and closes the VCA file.
* @bsimethod                                    Keith.Bentley                   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool ValidChunkArray::Save(DownloadStatus& dlStat)
    {
    BeFile vpaFile;
    BeFileStatus stat = vpaFile.Create(m_vcaFileName, true);
    if (BeFileStatus::Success != stat)
        {
        dlStat = DOWNLOAD_CantCreateVca;
        BeAssert(false);
        return  false;
        }

    dlStat = DOWNLOAD_VcaWriteError;

    stat = vpaFile.SetPointer(0, BeFileSeekOrigin::Begin);
    if (BeFileStatus::Success != stat)
        return  false;

    uint32_t serverUrlSize = (uint32_t) m_serverUrl.size()+1;
    uint32_t serverFileSpecSize = (uint32_t) m_serverFileSpec.size()+1;
    bool successfulSave =
           writeFile(vpaFile, &m_stats, sizeof(m_stats)) &&
           writeFile(vpaFile, &serverUrlSize, sizeof(serverUrlSize)) &&
           writeFile(vpaFile, m_serverUrl.c_str(), serverUrlSize) &&
           writeFile(vpaFile, &serverFileSpecSize, sizeof(serverFileSpecSize)) &&
           writeFile(vpaFile, m_serverFileSpec.c_str(), serverFileSpecSize) &&
           writeFile(vpaFile, m_validChunks, GetNumBytes());

    BeAssert(successfulSave);
    return successfulSave;
    }

/*---------------------------------------------------------------------------------**//**
* Count the number of bits on in the ValidChunkArray.
* @bsimethod                                    Keith.Bentley                   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t ValidChunkArray::CountChunksValid() const
    {
    uint32_t nValid=0;
    for (uint32_t i=0; i<m_nChunks; ++i)
        {
        if (IsChunkValid(i))
            ++nValid;
        }
    return  nValid;
    }

/*---------------------------------------------------------------------------------**//**
* Attempt to read the data from a VCA file about the content of its sister DownloadFile database file.
* This methods opens, reads, and closes the VCA file.
* @bsimethod                                    Keith.Bentley                   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool ValidChunkArray::Load(DownloadStatus& dlStat)
    {
    BeFile vpaFile;

    BeFileStatus stat = vpaFile.Open (m_vcaFileName, BeFileAccess::Read);
    if (BeFileStatus::Success != stat)
        {
        switch (stat)
            {
            case BeFileStatus::FileNotFoundError:
                dlStat = DOWNLOAD_VcaNotFound;
                break;

            default:
                dlStat = DOWNLOAD_CantOpenVcaForRead;
                break;
            }
        return  false;
        }

    dlStat = DOWNLOAD_VcaReadError;

    uint32_t serverUrlSize;
    if (!readFile(vpaFile, &m_stats,        sizeof(m_stats)) ||
        !readFile(vpaFile, &serverUrlSize,  sizeof(serverUrlSize)) ||
        m_stats.GetNChunks() == 0)
        return  false;

    ScopedArray<char> serverUrl(serverUrlSize);
    if (!readFile(vpaFile, serverUrl.GetData(), serverUrlSize))
        return false;

    uint32_t serverFileSpecSize;
    if (!readFile(vpaFile, &serverFileSpecSize,  sizeof(serverFileSpecSize)))
        return  false;

    ScopedArray<char> serverFileSpec(serverFileSpecSize);
    if (!readFile(vpaFile, serverFileSpec.GetData(), serverFileSpecSize))
        return false;

    SetServerUrl (serverUrl.GetData(), serverFileSpec.GetData());

    m_nChunks = m_stats.GetNChunks();
    m_validChunks = (Byte*) malloc(GetNumBytes());
    if (!readFile(vpaFile, m_validChunks, GetNumBytes()))
        return false;

    m_nValid = CountChunksValid();
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* Determine whether a chunk is valid in the VCA. The VCA mutex is acquired by this method before the check.
* @bsimethod                                    Keith.Bentley                   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool ValidChunkArray::IsChunkValid(uint32_t chunkNo) const
    {
    BeDbMutexHolder _v(m_mutex);
    return  CheckChunk(chunkNo);
    }

/*---------------------------------------------------------------------------------**//**
* Marks a chunk as valid in the VCA. The VCA mutex is acquired by this method before the bit is set.
* @bsimethod                                    Keith.Bentley                   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool ValidChunkArray::SetChunkValid(uint32_t chunkNo)
    {
    BeDbMutexHolder _v(m_mutex);

    BeAssert (!CheckChunk(chunkNo));
    *ByteForChunkP(chunkNo) |= MaskForChunk(chunkNo);

    return m_nChunks == ++m_nValid;
    }

/*---------------------------------------------------------------------------------**//**
* Starting at a given chunk, find the next higher chunk that is not currently valid, wrapping around to the beginning if necessary.
* @bsimethod                                    Keith.Bentley                   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t ValidChunkArray::GetNextNeededChunk(uint32_t start) const
    {
    BeDbMutexHolder _v(m_mutex);
    if (m_nValid == m_nChunks)
        return  INVALID_CHUNK;

    for (uint32_t chunk=start+1; chunk!=start; ++chunk)
        {
        if (chunk >= m_nChunks)
            chunk=0;

        if (!CheckChunk(chunk))
            return chunk;
        }

    BeAssert (false);
    return  INVALID_CHUNK;
    }

/*---------------------------------------------------------------------------------**//**
* called in the Requester thread when a chunk is received. Adds it to the Downloaded
* Chunks Queue (DCQ) and wakes the receiver thread to write them to the local file. Access to the DCQ
* must be done with the receiver mutex held.
* @bsimethod                                    Keith.Bentley                   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
void ChunkReceiveQueue::_OnChunkReceived(ChunkBuffer& chunk)
    {
    BeAssert (g_dlVfsThreadId.InRequesterThread());
    DBG_LOG (3, "Received chunk %d\n", (int)chunk.GetChunkNo());

    BeMutexHolder _v (m_chunkAvaliable.GetMutex());
    m_dlChunks.push (&chunk);
    m_chunkAvaliable.notify_all();
    }

/*---------------------------------------------------------------------------------**//**
* Process one entry in the Downloaded Chunk Queue
* @bsimethod                                    Keith.Bentley                   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool ChunkReceiveQueue::recv_ProcessNextAvailableChunk()
    {
    BeAssert (g_dlVfsThreadId.InReceiverThread());

    BeMutexHolder _v (m_chunkAvaliable.GetMutex());
    ChunkBuffer* chunk=NULL;
    if (m_aborted || m_dlChunks.empty())
        return false;

    chunk = m_dlChunks.front();
    m_dlChunks.pop();

    // This will write to the local file
    m_dlFile->recv_OnChunkAvailable (*chunk);

    delete chunk;   // we're done with this chunk
    return true;    // return true so we'll keep checking for more entries.
    }

/*---------------------------------------------------------------------------------**//**
* Abort the receiver thread. We do that by setting the pointer to the DownloadFile to null. This must be done
* with the critical se
* @bsimethod                                    Keith.Bentley                   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
void ChunkReceiveQueue::Abort()
    {
    BeMutexHolder _v (m_chunkAvaliable.GetMutex());
    m_aborted = true;
    m_dlFile = NULL;
    m_chunkAvaliable.notify_all();
    }

/*---------------------------------------------------------------------------------**//**
* Wait for either a new page available or the delay time so we can start a preemptive request.
* Returns true if the download was aborted, false to keep going.
* @bsimethod                                    Keith.Bentley                   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool ChunkReceiveQueue::recv_WaitForChunkAvailable()
    {
    BeMutexHolder _v (m_chunkAvaliable.GetMutex());

    if (IsAborted())
        return false;

    if (!m_dlChunks.empty())
        return true;

    // no more chunks available. Wait until more come down from server. The timeout is so we can preemptively request chunks
    m_chunkAvaliable.ProtectedWaitOnCondition (_v, NULL, m_dlFile->GetPreemptiveDelay() / 2);

    DBG_LOG (3, "receive queue triggered, abort=%d\n", IsAborted());
    return !IsAborted();
    }

/*---------------------------------------------------------------------------------**//**
* main loop for the receiver thread.
* @bsimethod                                    Keith.Bentley                   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
void ChunkReceiveQueue::recv_ReceiveChunkLoop()
    {
    while (recv_WaitForChunkAvailable())
        {
        // process the chunks one at a time so that we can receive new chunks without waiting for all current chunks to finish
        while (recv_ProcessNextAvailableChunk())
            {}

        BeMutexHolder _v (m_chunkAvaliable.GetMutex());
        if (m_dlFile)
            {
            m_dlFile->CheckPreemptiveRequest();
            m_dlFile->CheckSaveVCA();
            }
        }

    DBG_LOG (1, "Receiver exit\n");
    }

/*---------------------------------------------------------------------------------**//**
* called in the main thread to tell the Requester thread to abort.
* @bsimethod                                    Keith.Bentley                   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
void ChunkRequestQueue::Abort()
    {
    BeMutexHolder _v (m_chunkNeeded.GetMutex());
    m_aborted=true;
    m_chunkNeeded.notify_all();
    }

/*---------------------------------------------------------------------------------**//**
* Get the next requested chunk from the requested chunk queue.
* @bsimethod                                    Keith.Bentley                   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t ChunkRequestQueue::req_GetNextRequestedChunk()
    {
    BeMutexHolder _v (m_chunkNeeded.GetMutex());
    if (m_chunksToFetch.empty())
        return  INVALID_CHUNK;

    uint32_t val = m_chunksToFetch.front();
    m_chunksToFetch.pop_front();
    return  val;
    }

/*---------------------------------------------------------------------------------**//**
* Wait indefinitely until someone requests a chunk.
* @bsimethod                                    Keith.Bentley                   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool ChunkRequestQueue::req_WaitForChunkRequest()
    {
    BeMutexHolder _v (m_chunkNeeded.GetMutex());
    if (IsAborted())
        return false;

    if (!m_chunksToFetch.empty())
        return true;

    DBG_LOG (4, "Wait for request\n");
    m_chunkNeeded.ProtectedWaitOnCondition (_v, NULL, BeConditionVariable::Infinite);
    DBG_LOG (3, "Received request\n");

    return !IsAborted();
    }

/*---------------------------------------------------------------------------------**//**
* main loop for server demo thread. In a real application, this thread would be on another computer in another process.
* @bsimethod                                    Keith.Bentley                   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
void ChunkRequestQueue::req_RequestedChunkLoop()
    {
    while (req_WaitForChunkRequest())
        {
        uint32_t nextChunk = req_GetNextRequestedChunk();
        DBG_LOG (3,"Next chunk %d\n", (unsigned int) nextChunk);

        if (INVALID_CHUNK != nextChunk)
            {
            DBG_LOG (3, "Requesting chunk %d\n", (int)nextChunk);
            m_connection._RequestChunk (nextChunk);
            }
        }

    // we only get here when the download is either finished or aborted. When we return this object will be deleted.
    DBG_LOG (1,"Connection exit\n");
    m_connection._Destroy();
    }

/*---------------------------------------------------------------------------------**//**
* main loop for requester queue
* @bsimethod                                    Keith.Bentley                   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
THREAD_MAIN_DECL requesterThreadMain (void* arg)
    {
    BeThreadUtilities::SetCurrentThreadName("RequesterThread"); // for debugging only
    g_dlVfsThreadId.SetIsRequesterThread();

    ChunkRequestQueue* requestQueue = (ChunkRequestQueue*) arg;
    requestQueue->req_RequestedChunkLoop();
    delete requestQueue;

    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* ctor for request queue. Starts its corresponding thread.
* @bsimethod                                    Keith.Bentley                   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
ChunkRequestQueue::ChunkRequestQueue(Connection& conn) : m_connection(conn)
    {
    m_aborted = false;
    BeThreadUtilities::StartNewThread (50*1024, requesterThreadMain, this);
    }

/*---------------------------------------------------------------------------------**//**
* This method is called to ask the server to download a specific chunk and return - does not wait for response.
* @bsimethod                                    Keith.Bentley                   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
void ChunkRequestQueue::QueueChunkRequest(uint32_t chunkNo)
    {
    BeAssert (g_dlVfsThreadId.InMainThread() || g_dlVfsThreadId.InReceiverThread());
    if (INVALID_CHUNK== chunkNo)
        return;

    DBG_LOG (2,"requesting page %d\n", (unsigned int) chunkNo);
    BeMutexHolder _v (m_chunkNeeded.GetMutex());
    m_chunksToFetch.push_front(chunkNo);
    m_chunkNeeded.notify_all();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
DownloadFile::DownloadFile(IDownloadAdmin& admin) : m_admin(admin)
    {
    m_chunkArray = new ValidChunkArray();
    m_done = false;
    m_waitingChunk = m_lastRequestedChunkNo = INVALID_CHUNK;
    m_lastChunkRequest = m_lastSaveTime = BeTimeUtilities::QueryMillisecondsCounter();
    m_receivedChunks   = m_totalReceivedChunks = 0;
    m_receiveQueue = nullptr;
    m_requestQueue = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* when the DownloadFile is destroyed, we can abort the Requester and Receiver threads. To do that, we
* turn on their "abort" flags. They will each wake up, delete themselves, and the exit their threads.
* @bsimethod                                    Keith.Bentley                   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
DownloadFile::~DownloadFile()
    {
    SaveVCA();
    AbortQueues();
    delete m_chunkArray;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
THREAD_MAIN_DECL receiverThreadMain (void* arg)
    {
    BeThreadUtilities::SetCurrentThreadName("ReceiverThread"); // for debugging only
    g_dlVfsThreadId.SetIsReceiverThread();

    ChunkReceiveQueue* receiver = (ChunkReceiveQueue*) arg;
    receiver->recv_ReceiveChunkLoop();
    delete receiver;
    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* Ctor for receiver queue object. Creates the corresponding thread that manages the queue.
* @bsimethod                                    Keith.Bentley                   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
ChunkReceiveQueue::ChunkReceiveQueue(DownloadFile& dlFile)
    {
    m_aborted = false;
    m_dlFile = &dlFile;
    BeThreadUtilities::StartNewThread (50*1024, receiverThreadMain, this);
    }

/*---------------------------------------------------------------------------------**//**
* abort both request and receive queues. The queues objects are deleted when their threads exit.
* @bsimethod                                    Keith.Bentley                   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DownloadFile::AbortQueues()
    {
    if (NULL == m_requestQueue) // already aborted.
        return;

    m_requestQueue->GetConnection()._OnReceiverExit(); // notify connection to to send any more chunks
    BeAssert(m_receiveQueue);

    m_requestQueue->Abort(); // set abort flags to tell the other threads to quit and exit. This also wakes them up.
    m_requestQueue=NULL;
    m_receiveQueue->Abort();
    m_receiveQueue=NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
Connection* DownloadFile::StartConnection (IDownloadAdmin& admin, Utf8String serverUrl, Utf8String filespec, DownloadStatus& dlStat)
    {
    m_receiveQueue = new ChunkReceiveQueue(*this);

    Connection* connection = admin._CreateConnection(serverUrl, filespec, *m_receiveQueue, dlStat);
    if (NULL == connection)
        return NULL;

    m_config = connection->_GetOptions();
    return  connection;
    }

/*---------------------------------------------------------------------------------**//**
* Reconnect a previously successful connection to the server (note, it could have been successful in a distant past session).
* Also creates the requester and receiver threads.
* @bsimethod                                    Keith.Bentley                   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
DownloadStatus DownloadFile::ReconnectToServer(IDownloadAdmin& admin)
    {
    DownloadStatus stat;
    Connection* connection = StartConnection(admin, m_chunkArray->GetServerUrl(), m_chunkArray->GetServerFileSpec(), stat);
    if (NULL == connection)
        return stat;

    if (!m_chunkArray->GetFileStats().Matches (connection->GetFileStats()))
        {
        BeAssert (false && "remote file does not match previous connection");
        return  DOWNLOAD_ConnectionDoesNotMatch;
        }

    m_requestQueue = new ChunkRequestQueue(*connection);
    return DOWNLOAD_Success;
    }

/*---------------------------------------------------------------------------------**//**
* If the download did not complete in a previous session, the local db file will be partially valid and the
* VCA (valid chunk array) file will have the name of the server, the size of the db, and the current set of valid chunks.
* This method finds and reads the VCA file and then attempts to connect to the saved server.
* @bsimethod                                    Keith.Bentley                   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
DownloadStatus DownloadFile::Restart(Utf8String dbFileName, IDownloadAdmin& admin)
    {
    m_chunkArray->SetupFileName(dbFileName);

    DownloadStatus dlStat;
    if (!m_chunkArray->Load(dlStat))
        {
        BeAssert (false && "cannot reopen VCA file");
        return dlStat;
        }

    BeFileStatus stat = m_dbFile.Open (dbFileName, BeFileAccess::Write);
    if (BeFileStatus::Success != stat)
        {
        BeAssert (false && "can't open local database for write");
        return (stat==BeFileStatus::FileNotFoundError) ? DOWNLOAD_LocalFileNotFound : DOWNLOAD_CantReopenLocalFile;
        }

    return ReconnectToServer(admin);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
DownloadStatus DownloadFile::CreateDownloadFile (Utf8String localFileName, Utf8String serverUrl, Utf8String serverFileSpec, IDownloadAdmin& admin, uint32_t prefetchBytes)
    {
    BeFileStatus stat = m_dbFile.Create (localFileName, true);
    if (BeFileStatus::Success != stat)
        return  DOWNLOAD_CantCreateLocalFile;

    DownloadStatus dlStat;
    Connection* connection = StartConnection(admin, serverUrl, serverFileSpec, dlStat);
    if (NULL == connection)
        return dlStat;

    if (connection->GetFileStats().GetChunkSize() < MINIMUM_CHUNK_SIZE)
        {
        BeAssert(false);
        return DOWNLOAD_ChunkSizeTooSmall;
        }

    m_requestQueue = new ChunkRequestQueue(*connection);
    m_chunkArray->Initialize(connection->GetFileStats(), localFileName, serverUrl, serverFileSpec);

    if (prefetchBytes < MINIMUM_CHUNK_SIZE)
        prefetchBytes = MINIMUM_CHUNK_SIZE;

    if (!wt_ReadyBytes(0, prefetchBytes))
        return DOWNLOAD_Timeout;

    if (m_done)// we got the whole file, no need to turn it into a download file.
        return DOWNLOAD_Success;

    stat = m_dbFile.SetPointer(0, BeFileSeekOrigin::Begin); // move back to beginning
    if (!writeFile (m_dbFile, DOWNLOAD_FORMAT_SIGNATURE, sizeof(DOWNLOAD_FORMAT_SIGNATURE)))
        return DOWNLOAD_LocalFileWriteError;

    return DOWNLOAD_Success;
    }

/*---------------------------------------------------------------------------------**//**
* make sure all changes to the database file are flushed to disk
* @bsimethod                                    Keith.Bentley                   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool DownloadFile::FlushDb()
    {
    return (BeFileStatus::Success == m_dbFile.Flush());
    }

/*---------------------------------------------------------------------------------**//**
* If we are not currently waiting for a direct download, then see if enough time has passed to start
* asking for "preemptive" chunks.
* @bsimethod                                    Keith.Bentley                   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DownloadFile::CheckPreemptiveRequest()
    {
    static bool s_turnOffPreemptive = false;
    if (s_turnOffPreemptive)
        return;

    BeAssert (g_dlVfsThreadId.InReceiverThread());

    if (INVALID_CHUNK != m_waitingChunk) // we're waiting for a chunk to become available, don't preemptive request
        return;

    // wait for the preemptive delay since last direct download request, in case the main thread needs another chunk soon.
    if (BeTimeUtilities::QueryMillisecondsCounter()-m_lastChunkRequest < GetPreemptiveDelay())
        return;

    uint32_t preemptiveChunk = m_chunkArray->GetNextNeededChunk(m_lastRequestedChunkNo);
    if (INVALID_CHUNK == preemptiveChunk) // we're done
        return;

    DBG_LOG (2,"Preemptively asking for %d\n", (unsigned int)preemptiveChunk);
    RequestChunk(preemptiveChunk, false);
    }

/*---------------------------------------------------------------------------------**//**
* Save the "valid chunks array", if necessary. Before writing the VCA file, we need to flush the local database file
* to ensure the chunks marked valid in the VCA were successfully saved to disk.
* @bsimethod                                    Keith.Bentley                   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
DownloadStatus DownloadFile::SaveVCA()
    {
    if (m_done || NULL==m_chunkArray || m_receivedChunks==0)
        return DOWNLOAD_Success;

    DBG_LOG (1,"saving vca, %d valid out of %d\n", (int)m_chunkArray->GetNValid(), (int)m_chunkArray->GetNChunks());

    if (!FlushDb())
        {
        BeAssert (false && "can't save database file");
        return DOWNLOAD_LocalFileWriteError;
        }

    DownloadStatus dlStat;
    if (!m_chunkArray->Save(dlStat))
        {
        BeAssert (false && "can't save VCA file");
        return dlStat;
        }

    DBG_LOG (3,"Saved vca\n");

    m_lastSaveTime = BeTimeUtilities::QueryMillisecondsCounter();
    m_receivedChunks = 0;
    return DOWNLOAD_Success;
    }

/*---------------------------------------------------------------------------------**//**
* See if we need to save the VCA into its file. We only do this every few seconds, and only if we've received
* new chunks since the last time we checked.
* @bsimethod                                    Keith.Bentley                   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DownloadFile::CheckSaveVCA()
    {
    if (m_receivedChunks==0)
        return;

    if (m_requestQueue)
        m_admin._OnDownloadProgress (m_chunkArray->GetLocalFileName().c_str(), m_requestQueue->GetConnection(), m_chunkArray->GetNValid());

    if ((BeTimeUtilities::QueryMillisecondsCounter()-m_lastSaveTime) > GetSaveInterval())
        SaveVCA();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DownloadFile::OnDownloadComplete()
    {
    m_admin._OnDownloadComplete(m_requestQueue->GetConnection());

    // rewrite header with sqlite3 signature
    m_dbFile.SetPointer(0, BeFileSeekOrigin::Begin);
    writeFile (m_dbFile, SQLITE_FORMAT_SIGNATURE, sizeof(SQLITE_FORMAT_SIGNATURE));

    m_dbFile.Close();

    BeFileName vcaFileName (m_chunkArray->GetVcaFileName().c_str(), BentleyCharEncoding::Utf8);
    vcaFileName.BeDeleteFile();

    m_receivedChunks = 0;
    AbortQueues();
    }

/*---------------------------------------------------------------------------------**//**
* called in the Receiver thread when a chunk has been downloaded from the server and is now available
* to be saved in the permanent db.
* @bsimethod                                    Keith.Bentley                   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DownloadFile::recv_OnChunkAvailable (ChunkBuffer const& chunk)
    {
    BeAssert (g_dlVfsThreadId.InReceiverThread());
    BeAssert (!m_done);

    BeMutexHolder _v (m_chunkWait.GetMutex());

    uint32_t chunkNo = chunk.GetChunkNo();
    if (m_chunkArray->CheckChunk(chunkNo))
        {
        DBG_LOG (2,"re-received chunk %d\n", (int)chunkNo);
        return;
        }

    Byte const* data = chunk.GetData();
    uint32_t chunkSize = GetChunkSize();

    // seek to the start of the chunk we've received
    uint64_t startPos = chunkNo*chunkSize;
    BeFileStatus stat = m_dbFile.SetPointer(startPos, BeFileSeekOrigin::Begin);
    if (BeFileStatus::Success != stat)
        {
        BeAssert(false);
        return;
        }

    if (chunkSize != chunk.GetNBytes())
        {
        if ((chunkNo != (m_chunkArray->GetNChunks()-1)) || (chunkSize < chunk.GetNBytes()))
            {
            BeAssert (false); // invalid chunk size
            return;
            }
        }

    // now write the data to the local db file.
    uint32_t written, requested = chunk.GetNBytes();
    stat = m_dbFile.Write (&written, data, requested);
    if (BeFileStatus::Success != stat || written != requested)
        {
        BeAssert(false);
        return;
        }

    ++m_receivedChunks;         // so we can tell we need to save the VCA
    ++m_totalReceivedChunks;    // for debugging

    DBG_LOG (3,"chunk %d avail\n", (unsigned int) chunkNo);

    // mark the chunks as now valid, and wake up waiting threads.
    m_done = m_chunkArray->SetChunkValid (chunkNo);
    m_chunkWait.notify_all();

    if (m_done)
        {
        BeAssert (m_chunkArray->GetNChunks() == m_chunkArray->CountChunksValid());
        OnDownloadComplete();
        }
    }

/*---------------------------------------------------------------------------------**//**
* called from the main thread. Blocks waiting for a specific chunk to become available.
* @bsimethod                                    Keith.Bentley                   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DownloadFile::wt_WaitForChunk ()
    {
    BeAssert (g_dlVfsThreadId.InMainThread());
    BeAssert (m_waitingChunk != INVALID_CHUNK);

    BeMutexHolder _v (m_chunkWait.GetMutex());
    DBG_LOG (3,"waiting for %d \n", (unsigned int) m_waitingChunk);

    while (!IsChunkValid(m_waitingChunk))
        {
        if (BeTimeUtilities::QueryMillisecondsCounter()-m_lastChunkRequest > GetDownloadTimeout())
            {
            if (!m_admin._OnDownloadTimeout(m_requestQueue->GetConnection()))
                {
                DBG_LOG (1,"download timeout\n");
                break;
                }
            }

        m_chunkWait.ProtectedWaitOnCondition(_v, NULL, GetDownloadTimeout()/2);
        }

    DBG_LOG (3,"got chunk %d \n",(unsigned int) m_waitingChunk);
    m_waitingChunk = INVALID_CHUNK;
    }

/*---------------------------------------------------------------------------------**//**
* Request a specific chunk from the server. Does not wait for response. This function is used by both the
* sqlite threads for a needed page, and by the preemptive downloading logic. If "directRequest" is true
* this was called directly from an sqlite thread and is not a preemptive request. If directRequest
* is true m_waitingChunk hold the chunk number of the requested chunk.
* @bsimethod                                    Keith.Bentley                   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DownloadFile::RequestChunk(uint32_t chunkNo, bool directRequest)
    {
    BeAssert (g_dlVfsThreadId.InMainThread() || g_dlVfsThreadId.InReceiverThread());
    BeMutexHolder _v (m_chunkWait.GetMutex());

    if (m_waitingChunk != INVALID_CHUNK)
        {
        if (!directRequest)
            return;  // we thought we could ask for a preemptive chunk but other thread beat us here

        BeAssert (false && "asking for chunk while waiting for chunk");
        }

    if (directRequest)
        {
        m_lastChunkRequest = BeTimeUtilities::QueryMillisecondsCounter();
        m_waitingChunk = chunkNo;
        }

    m_lastRequestedChunkNo = chunkNo;
    if (m_requestQueue) // may have been aborted
        m_requestQueue->QueueChunkRequest(chunkNo);
    }

/*---------------------------------------------------------------------------------**//**
* request and wait for a specific chunk to become available.
* @bsimethod                                    Keith.Bentley                   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool DownloadFile::wt_ReadyChunk (uint32_t chunkNo)
    {
    BeAssert (chunkNo < m_chunkArray->GetNChunks());

    if (IsChunkValid(chunkNo))
        return true;

    BeAssert (g_dlVfsThreadId.InMainThread());
    RequestChunk(chunkNo, true);
    wt_WaitForChunk();

    if (m_done)
        {
        // no more work to do, stop download threads.
        AbortQueues();
        return true;
        }

    // if the chunk is invalid, the download failed (timed out?) and we're going to report a "disk error"
    return IsChunkValid(chunkNo);
    }

/*---------------------------------------------------------------------------------**//**
* called in a sqlite thread to ensure a database page is valid. If not, it will be requested
* from the server and this method will chunk until it is ready to be read from the file.
* This method can return false, if the server times out and the database cannot be read.
* @bsimethod                                    Keith.Bentley                   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool DownloadFile::wt_ReadyBytes(int64_t offset, int32_t numBytes)
    {
    if (m_done) // all chunks already valid. NOTE: m_chunkArray can be NULL!
        return  true;

    int32_t chunkSize = m_chunkArray->GetChunkSize();
    do
        {
        BeAssert (!m_done); // we're in this loop a second time, yet we think we're done???

        uint32_t chunkNo = (uint32_t) (offset / chunkSize);
        if (!wt_ReadyChunk(chunkNo))
            return false; // download failed.

        int32_t bytesThisChunk = chunkSize - (offset%chunkSize);
        BeAssert (bytesThisChunk>0);
        numBytes -= bytesThisChunk;
        offset   += bytesThisChunk;
        } while (numBytes > 0);

    return true;
    }

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   10/13
//=======================================================================================
struct DownloadVfs : sqlite3_vfs
{
    sqlite3_vfs* m_root;      // points to the default vfs
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   10/13
//=======================================================================================
struct DownloadVfsFile : sqlite3_file
{
    DownloadFile* m_dlFile;    // all of our C++ objects are found via this pointer
};

/*---------------------------------------------------------------------------------**//**
* This weird subclassing is just the way SQLite works. We have to put the subclass data first, followed by the superclass data.
* Note: "real file" == "default sqlite vfs file"
* @bsimethod                                    Keith.Bentley                   11/13
+---------------+---------------+---------------+---------------+---------------+------*/
static sqlite3_file* toRealFile(sqlite3_file* in) {return (sqlite3_file*)((Byte*)in+sizeof(DownloadVfsFile));}

/*---------------------------------------------------------------------------------**//**
* SQLite has been asked to close this file. We delete its DownloadFile object, which will kill any in-process downloading
* and terminate the corresponding threads.
* @bsimethod                                    Keith       .Bentley                   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
static int dlvfsClose (sqlite3_file* inFile)
    {
    DownloadVfsFile* dlFile = (DownloadVfsFile*) inFile;

    delete dlFile->m_dlFile;     // kills other threads, but doesn't wait for them to die.
    dlFile->m_dlFile = NULL;

    sqlite3_file* pFile = toRealFile(inFile);
    return  pFile->pMethods->xClose(pFile);
    }

/*---------------------------------------------------------------------------------**//**
* called by SQLite to read a page from the database. We first make sure all of the required bytes are "ready" (downloaded)
* before allowing SQLite to proceed.
* @bsimethod                                    Keith.Bentley                   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
static int dlvfsRead(sqlite3_file* inFile, void* zBuf, int iAmt, sqlite_int64 iOfst)
    {
    DownloadVfsFile* dlFile = (DownloadVfsFile*) inFile;

    // if the bytes are not valid, wait for them to be downloaded before proceeding
    if ((NULL != dlFile->m_dlFile) && (!dlFile->m_dlFile->wt_ReadyBytes(iOfst, iAmt)))
        return  SQLITE_IOERR; // this is what we can return for "timeout"

    // at this point the local file has all the bytes SQLite is going to read. Just call the superclass' xRead method
    sqlite3_file* pFile = toRealFile(inFile);
    int rc = pFile->pMethods->xRead(pFile, zBuf, iAmt, iOfst);

    // for first block in the file, we need to replace our "Download SQLite" signature with the standard "SQLite format 3" signature
    // so SQLite won't think there's anything amiss with the file.
    if (iOfst == 0 && rc==SQLITE_OK)
        memcpy (zBuf, SQLITE_FORMAT_SIGNATURE, sizeof(SQLITE_FORMAT_SIGNATURE));

    return rc;
    }

/*---------------------------------------------------------------------------------**//**
* When SQLite asks for the file size, we can't let it look at the local file, since it may not be full sized yet.
* just return the value we got from the server.
* @bsimethod                                    Keith.Bentley                   11/13
+---------------+---------------+---------------+---------------+---------------+------*/
static int  dlvfsFileSize(sqlite3_file* inFile, sqlite3_int64 *pSize)
    {
    DownloadVfsFile* dlFile = (DownloadVfsFile*) inFile;
    if (!dlFile->m_dlFile)
        return  SQLITE_IOERR;

    *pSize = dlFile->m_dlFile->GetFileSize();
    return  SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* all of the methods below are just "thunks" to the superclass implementations- we don't need to do any work in them.
+---------------+---------------+---------------+---------------+---------------+------*/
static int  dlvfsWrite(sqlite3_file* inFile, const void* zBuf, int iAmt, sqlite3_int64 iOfst)
    {
    sqlite3_file* pFile = toRealFile(inFile);
    return pFile->pMethods->xWrite(pFile, zBuf, iAmt, iOfst);
    }

static int  dlvfsTruncate(sqlite3_file* inFile, sqlite3_int64 size)
    {
    sqlite3_file* pFile = toRealFile(inFile);
    return pFile->pMethods->xTruncate(pFile, size);
    }

static int  dlvfsSync(sqlite3_file* inFile, int flags)
    {
    sqlite3_file* pFile = toRealFile(inFile);
    return pFile->pMethods->xSync(pFile, flags);
    }

static int  dlvfsLock(sqlite3_file* inFile, int eLock)
    {
    sqlite3_file* pFile = toRealFile(inFile);
    return pFile->pMethods->xLock(pFile, eLock);
    }

static int  dlvfsUnlock(sqlite3_file* inFile, int eLock)
    {
    sqlite3_file* pFile = toRealFile(inFile);
    return pFile->pMethods->xUnlock(pFile, eLock);
    }

static int  dlvfsCheckReservedLock(sqlite3_file* inFile, int *pResOut)
    {
    sqlite3_file* pFile = toRealFile(inFile);
    return pFile->pMethods->xCheckReservedLock(pFile, pResOut);
    }

static int  dlvfsFileControl(sqlite3_file* inFile, int op, void *pArg)
    {
    sqlite3_file* pFile = toRealFile(inFile);
    return pFile->pMethods->xFileControl(pFile, op, pArg);
    }

static int  dlvfsSectorSize(sqlite3_file* inFile)
    {
    sqlite3_file* pFile = toRealFile(inFile);
    return pFile->pMethods->xSectorSize(pFile);
    }

static int  dlvfsDeviceCharacteristics(sqlite3_file* inFile)
    {
    sqlite3_file* pFile = toRealFile(inFile);
    return pFile->pMethods->xDeviceCharacteristics(pFile);
   }

static int  dlvfsShmMap(sqlite3_file* inFile, int iPg, int pgsz, int isWrite, void volatile** pp)
    {
    sqlite3_file* pFile = toRealFile(inFile);
    return pFile->pMethods->xShmMap(pFile, iPg, pgsz, isWrite, pp);
    }

static int  dlvfsShmLock(sqlite3_file* inFile, int offset, int n, int flags)
    {
    sqlite3_file* pFile = toRealFile(inFile);
    return pFile->pMethods->xShmLock(pFile, offset, n, flags);
    }

static void dlvfsShmBarrier(sqlite3_file* inFile)
    {
    sqlite3_file* pFile = toRealFile(inFile);
    return pFile->pMethods->xShmBarrier(pFile);
    }

static int  dlvfsShmUnmap(sqlite3_file* inFile, int deleteFlag)
    {
    sqlite3_file* pFile = toRealFile(inFile);
    return pFile->pMethods->xShmUnmap(pFile, deleteFlag);
    }

static int  dlvfsFetch(sqlite3_file* inFile, sqlite3_int64 iOfst, int iAmt, void **pp)
    {
    sqlite3_file* pFile = toRealFile(inFile);
    return pFile->pMethods->xFetch(pFile, iOfst, iAmt, pp);
    }

static int  dlvfsUnfetch(sqlite3_file* inFile, sqlite3_int64 iOfst, void *p)
    {
    sqlite3_file* pFile = toRealFile(inFile);
    return pFile->pMethods->xUnfetch(pFile, iOfst, p);
    }

static sqlite3_io_methods dlvfs_io_methods = {
  3,                              /* iVersion */
  dlvfsClose,                      /* xClose */
  dlvfsRead,                       /* xRead */
  dlvfsWrite,                      /* xWrite */
  dlvfsTruncate,                   /* xTruncate */
  dlvfsSync,                       /* xSync */
  dlvfsFileSize,                   /* xFileSize */
  dlvfsLock,                       /* xLock */
  dlvfsUnlock,                     /* xUnlock */
  dlvfsCheckReservedLock,          /* xCheckReservedLock */
  dlvfsFileControl,                /* xFileControl */
  dlvfsSectorSize,                 /* xSectorSize */
  dlvfsDeviceCharacteristics,      /* xDeviceCharacteristics */
  dlvfsShmMap,                     /* xShmMap */
  dlvfsShmLock,                    /* xShmLock */
  dlvfsShmBarrier,                 /* xShmBarrier */
  dlvfsShmUnmap,                   /* xShmUnmap */
  dlvfsFetch,
  dlvfsUnfetch
};

/*---------------------------------------------------------------------------------**//**
* Called by SQLite via DownloadVFS to initialize the sqlite3_file object for a download file.
* NOTE: sqlite calls this method to open various types of files (e.g. "-journal" files).
* We only want to use our subclass for the main db.
* @bsimethod                                    Keith.Bentley                   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
static int dlvfsOpen(sqlite3_vfs* pVfs, Utf8CP zName, sqlite3_file* inFile, int flags, int *pOutFlags)
    {
    DownloadVfs* dlVfs = (DownloadVfs*) pVfs;
    sqlite3_vfs* rootVfs = dlVfs->m_root;

    // We only want to create a download file for main db.
    if (0 == (flags & SQLITE_OPEN_MAIN_DB))
        return rootVfs->xOpen(rootVfs, zName, inFile, flags, pOutFlags);

    DownloadVfsFile* dlFile = (DownloadVfsFile*) inFile;
    dlFile->pMethods = &dlvfs_io_methods;
    dlFile->m_dlFile = NULL;

    // all download files must be opened readonly with SQLite. We can't allow them to be modified.
    flags &= ~(SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE);
    flags |= SQLITE_OPEN_READONLY;

    int rc = rootVfs->xOpen(rootVfs, zName, toRealFile(inFile), flags, pOutFlags);
    if (rc == SQLITE_OK)
        {
        IDownloadAdmin& admin = *(IDownloadAdmin*) dlVfs->pAppData;
        dlFile->m_dlFile = new DownloadFile(admin);
        dlFile->m_dlFile->Restart (zName, admin);
        }

    return rc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
static int registerDownloadVfs(Utf8CP dlVfsName, IDownloadAdmin* admin)
    {
    sqlite3_vfs* rootVfs = sqlite3_vfs_find(NULL);   // this will be the platform-specific default vfs
    if (NULL == rootVfs)
        return SQLITE_NOTFOUND;

    int nameLen = (int) strlen(dlVfsName) + 1;
    int nByte = sizeof(DownloadVfs) + nameLen;

    DownloadVfs* dlVfs = (DownloadVfs*) sqlite3_malloc(nByte);
    if (dlVfs==0) return SQLITE_NOMEM;
    memset(dlVfs, 0, nByte);

    dlVfs->m_root = rootVfs;
    dlVfs->xOpen  = dlvfsOpen;
    dlVfs->szOsFile = rootVfs->szOsFile + sizeof(DownloadVfsFile); // see note about SQLite subclassing at toRealFile
    dlVfs->zName = (char*) &dlVfs[1]; // to save a malloc
    memcpy ((char*) dlVfs->zName, dlVfsName, nameLen);
    dlVfs->pAppData      = admin;
    dlVfs->iVersion      = rootVfs->iVersion;
    dlVfs->mxPathname    = rootVfs->mxPathname;
    dlVfs->xDelete       = rootVfs->xDelete;
    dlVfs->xAccess       = rootVfs->xAccess;
    dlVfs->xFullPathname = rootVfs->xFullPathname;
    dlVfs->xDlOpen       = rootVfs->xDlOpen;
    dlVfs->xDlError      = rootVfs->xDlError;
    dlVfs->xDlSym        = rootVfs->xDlSym;
    dlVfs->xDlClose      = rootVfs->xDlClose;
    dlVfs->xRandomness   = rootVfs->xRandomness;
    dlVfs->xSleep        = rootVfs->xSleep;
    dlVfs->xCurrentTime  = rootVfs->xCurrentTime;
    dlVfs->xGetLastError = rootVfs->xGetLastError;
    dlVfs->xCurrentTimeInt64 = rootVfs->xCurrentTimeInt64;
    dlVfs->xSetSystemCall  = rootVfs->xSetSystemCall;
    dlVfs->xGetSystemCall  = rootVfs->xGetSystemCall;
    dlVfs->xNextSystemCall = rootVfs->xNextSystemCall;

    return sqlite3_vfs_register(dlVfs, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/11
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP IDownloadAdmin::GetVfs()
    {
    static Utf8CP s_vfsDl = "download";
    if (0 == sqlite3_vfs_find(s_vfsDl))
        registerDownloadVfs(s_vfsDl, this);

    return s_vfsDl;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
DownloadStatus IDownloadAdmin::CreateLocalDownloadFile (Utf8String localFileName, Utf8String serverUrl, Utf8String serverFileSpec, uint32_t prefetchBytes)
    {
    // NOTE: DownloadFile is not in public api.
    DownloadFile tmp (*this);
    return tmp.CreateDownloadFile (localFileName, serverUrl, serverFileSpec, *this, prefetchBytes);
    }
