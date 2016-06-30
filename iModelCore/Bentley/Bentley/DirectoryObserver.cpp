/*--------------------------------------------------------------------------------------+
|
|     $Source: Bentley/DirectoryObserver.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/DirectoryObserver.h>
#include <Bentley/BeTimeUtilities.h>
#include <Logging/bentleylogging.h>

#if defined (BENTLEY_WIN32)
#include <windows.h>

#define EVENT_SIZE (sizeof(FILE_NOTIFY_INFORMATION ) + (2 * MAX_PATH))
#endif

#if defined (ANDROID)
#include <sys/types.h>
#include <sys/inotify.h>
#include <sys/select.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#define EVENT_SIZE  (sizeof(struct inotify_event))
#define BUF_LEN     (1024 * (EVENT_SIZE + 16))
#endif

#if defined (__APPLE__)
#include <sys/event.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <Bentley/BeDirectoryIterator.h>
#endif

#if defined (BENTLEY_WINRT)
#include <Windows.h>
#include <ppltasks.h>
#include "collection.h"
#include <Bentley/BeDirectoryIterator.h>

using namespace Windows::Storage;
using namespace Windows::Storage::Search;
using namespace Windows::Storage::FileProperties;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Platform;
using namespace concurrency;
#endif

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_TASKS

#define LOGGER_NAMESPACE_BENTLEY  "Bentley"
#define LOG (*BentleyApi::NativeLogging::LoggingManager::GetLogger (LOGGER_NAMESPACE_BENTLEY))

//#define DEBUG_CALLBACKS 1
//#define DEBUG_DIROBSERVER 1
#if defined (DEBUG_DIROBSERVER)
    #if defined (ANDROID)
        #define DIROBSERVER_DEBUGMSG(...)   __android_log_print(ANDROID_LOG_DEBUG, "DirectoryObserver", __VA_ARGS__);
    #else
        #define DIROBSERVER_DEBUGMSG(...)   NativeLogging::LoggingManager::GetLogger("DirectoryObserver")->debugv(__VA_ARGS__);
        struct DirObserverLoggingEnabler
            {
            DirObserverLoggingEnabler() 
                {
                if (!NativeLogging::LoggingConfig::IsProviderActive())
                    NativeLogging::LoggingConfig::ActivateProvider(NativeLogging::CONSOLE_LOGGING_PROVIDER);
                
                NativeLogging::LoggingConfig::SetSeverity(L"DirectoryObserver", NativeLogging::LOG_DEBUG); 
                }
            };
        static DirObserverLoggingEnabler s_enableDirObserverLogging;
    #endif
#else
    #define DIROBSERVER_DEBUGMSG(...)   {}
#endif

//=======================================================================================
// Cross-platform part of the observer
//=======================================================================================
/*--------------------------------------------------------------------------------------+
* @bsiclass                                                  Mantas.Ragauskas    06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
struct ChangesPredicate : IConditionVariablePredicate
    {
    BeAtomic<bool>& m_isChangeResolved;

    ChangesPredicate(BeAtomic<bool>& m_isChangeResolved) :m_isChangeResolved(m_isChangeResolved) { }
    virtual bool _TestCondition(struct BeConditionVariable &cv) override { return m_isChangeResolved; }
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Mantas.Ragauskas       06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
DirectoryObserver::DirectoryObserver(bset<BeFileName> const& directories, ObserverCallback callback)
    {
    m_directories = directories;
    m_callback = callback;
    m_isObserving.store(false);
    m_waitingThread = WorkerThread::Create("DirObserver_WaitingThread");
    m_callbackThread = WorkerThread::Create("DirObserver_CallbackThread");
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Mantas.Ragauskas       06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
DirectoryObserver::DirectoryObserver(ObserverCallback callback)
    {
    m_directories.clear();
    m_callback = callback;
    m_isObserving.store(false);
    m_waitingThread = WorkerThread::Create("DirObserver_WaitingThread");
    m_callbackThread = WorkerThread::Create("DirObserver_CallbackThread");
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Mantas.Ragauskas       06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
DirectoryObserver::~DirectoryObserver()
    {
    StopObserving();
    m_waitingThread->OnEmpty()->Wait();
    m_callbackThread->OnEmpty()->Wait();
    DIROBSERVER_DEBUGMSG("[Main][Observer deleted]");
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Mantas.Ragauskas       06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bset<BeFileName>const& DirectoryObserver::GetObservedDirectories() const { return m_directories; }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Mantas.Ragauskas       06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool DirectoryObserver::GetObserverStatus() const { return m_isObserving; }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Mantas.Ragauskas       06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void DirectoryObserver::RunCallbackAsync(BeFileNameCR changedDir, FileChanges changeStatus)
    {
    m_callbackThread->ExecuteAsync([=]()
        {
        m_callback(changedDir, changeStatus);
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Mantas.Ragauskas       06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DirectoryObserver::AddObservedDirectory(BeFileNameCR fileName)
    {
    if (!fileName.DoesPathExist())
        {
        LOG.errorv("[AddObservedDirectory]: Path does not exist: %s", fileName.GetNameUtf8().c_str());
        return BentleyStatus::ERROR;
        }

    DIROBSERVER_DEBUGMSG("[Main][Directory adding]: %llu %s", BeTimeUtilities::GetCurrentTimeAsUnixMillis(),  fileName.GetNameUtf8().c_str());

    auto it = m_directories.find(fileName);
    if (it != m_directories.end())
        {
        LOG.errorv("[AddObservedDirectory]: Directory already observed: %s", fileName.GetNameUtf8().c_str());
        return BentleyStatus::SUCCESS;
        }

    m_directories.insert(fileName);

    StopObserving();
    return StartObserving();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Mantas.Ragauskas       07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DirectoryObserver::AddObservedDirectories(bset<BeFileName>const& fileNames)
    {
    for (BeFileNameCR fileName : fileNames)
        {
        if (fileName.DoesPathExist() && (m_directories.end() == m_directories.find(fileName)))
            m_directories.insert(fileName);
        else
            LOG.errorv("[AddObservedDirectories]: Path does not exist or directory is already observed: %s", fileName.GetNameUtf8().c_str());
        }

    StopObserving();
    return StartObserving();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Mantas.Ragauskas       06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DirectoryObserver::RemoveObservedDirectory(BeFileNameCR fileName)
    {
    auto it = m_directories.find(fileName);
    if (it == m_directories.end())
        {
        LOG.errorv("[RemoveObservedDirectory]: Directory is not observed: %s", fileName.GetNameUtf8().c_str());
        return BentleyStatus::SUCCESS;
        }

    DIROBSERVER_DEBUGMSG("[Main][Directory removing]: %s", Utf8String(fileName).c_str());
    m_directories.erase(it);

    StopObserving();
    if (0 == m_directories.size())
        return BentleyStatus::SUCCESS;

    return StartObserving();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Mantas.Ragauskas       07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DirectoryObserver::RemoveObservedDirectories(bset<BeFileName>const& fileNames)
    {
    for (auto fileName : fileNames)
        {
        auto it = m_directories.find(fileName);
        if (it != m_directories.end())
            m_directories.erase(it);
        else
            LOG.errorv("[RemoveObservedDirectory]: Directory is not observed");
        }

    StopObserving();
    if (0 == m_directories.size())
        return BentleyStatus::SUCCESS;

    return StartObserving();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Mantas.Ragauskas       07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DirectoryObserver::RemoveAllObservedDirectories()
    {
    m_directories.clear();
    StopObserving();
    return BentleyStatus::SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Mantas.Ragauskas       06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DirectoryObserver::StartObserving()
    {
    if (m_isObserving)
        return BentleyStatus::SUCCESS;

    if (0 == m_directories.size())
        {
        LOG.errorv("[StartObserving]: No directories to observe");
        return BentleyStatus::ERROR;
        }
    DIROBSERVER_DEBUGMSG("[Main][Observer starting]: %llu", BeTimeUtilities::GetCurrentTimeAsUnixMillis());
    m_isObserving.store(true);
    m_startedObserving.store(false);

    m_waitingThread->ExecuteAsync([&]()
        {
        _StartObserving();
        while (m_isObserving)
            {
            _Observe();
            }

        BeMutexHolder lock(m_stoppedObservingLock.GetMutex());
        m_stoppedObserving.store(true);
        m_stoppedObservingLock.notify_all();
        });

    ChangesPredicate predicate(m_startedObserving);
    m_startedObservingLock.WaitOnCondition(&predicate, BeConditionVariable::Infinite);
    DIROBSERVER_DEBUGMSG("[Main][Observer started]: %llu", BeTimeUtilities::GetCurrentTimeAsUnixMillis());
    return BentleyStatus::SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Mantas.Ragauskas       06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DirectoryObserver::StopObserving()
    {
    BeMutexHolder lock(m_stoppedObservingLock.GetMutex());
    if (!m_isObserving)
        return BentleyStatus::SUCCESS;

    m_isObserving.store(false);
    m_stoppedObserving.store(false);
    _StopObserving();

    ChangesPredicate predicate(m_stoppedObserving);
    m_stoppedObservingLock.ProtectedWaitOnCondition(lock, &predicate, BeConditionVariable::Infinite);
    DIROBSERVER_DEBUGMSG("[Main][Observer Stopped]: %llu", BeTimeUtilities::GetCurrentTimeAsUnixMillis());

    _OnObservingStopped();

    return BentleyStatus::SUCCESS;
    }

//=======================================================================================
// Platform-specific part of the observer
//=======================================================================================
#if defined (BENTLEY_WIN32)
/*--------------------------------------------------------------------------------------+
* @bsiclass                                              Mantas.Ragauskas       07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
struct ObservedDir
    {
    OVERLAPPED m_overlapped;
    HANDLE     m_dirHandle;
    BeFileName m_dirName;
    DWORD      m_buffer[EVENT_SIZE];

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                              Mantas.Ragauskas       07/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    ObservedDir(HANDLE dirHandle, BeFileNameCR dirName) : m_dirHandle (dirHandle), m_dirName (dirName)
        {
        m_overlapped = {};
        memset(m_buffer, 0, EVENT_SIZE);
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                              Mantas.Ragauskas       07/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    ~ObservedDir()
        {
        CloseHandle(m_dirHandle);
        }
    };
//=======================================================================================
// @bsiclass                                              Mantas.Ragauskas       06/2015
//=======================================================================================
struct DirectoryObserverWin : public DirectoryObserver
    {
private:
    HANDLE  m_completionPort;
    BeMutex m_mutex;
    bmap <ULONG_PTR, ObservedDir*> m_mappedDirectories;

    virtual void _StartObserving() override;
    virtual void _StopObserving() override;
    virtual void _OnObservingStopped() override;
    virtual void _Observe()override;

public:
    DirectoryObserverWin(bset<BeFileName>const& directories, ObserverCallback callback);
    DirectoryObserverWin(ObserverCallback callback);
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Mantas.Ragauskas       06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
DirectoryObserverWin::DirectoryObserverWin(ObserverCallback callback) : DirectoryObserver(callback) {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Mantas.Ragauskas       06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
DirectoryObserverWin::DirectoryObserverWin(bset<BeFileName>const& directories, ObserverCallback callback) : DirectoryObserver(directories, callback) { }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Mantas.Ragauskas       06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void DirectoryObserverWin::_StartObserving()
    {
    BeMutexHolder holder(m_mutex);
    ULONG_PTR id = 0;

    m_completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    BeAssert(NULL != m_completionPort);

    for (BeFileNameCR directory : m_directories)
        {
        id++;

        HANDLE dirHandle = CreateFileW(
            directory.c_str(),                                      // pointer to the file name
            FILE_LIST_DIRECTORY,                                    // access (read/write) mode
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, // share mode
            NULL,                                                   // security descriptor
            OPEN_EXISTING,                                          // how to create
            FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,      // file attributes
            NULL);                                                  // file with attributes to copy

        HANDLE assosiatedHandle = CreateIoCompletionPort(dirHandle, m_completionPort, id, 0);
        BeAssert(m_completionPort == assosiatedHandle);

         m_mappedDirectories[id] = new ObservedDir(dirHandle, directory);

         BOOL readResult = ReadDirectoryChangesW(
             dirHandle,                                             // handle to directory                       
             m_mappedDirectories[id]->m_buffer,                     // read results buffer
             sizeof(m_mappedDirectories[id]->m_buffer),             // length of buffer
             FALSE,                                                 // monitoring option (watch subtree)
             FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION | FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME, // filter conditions
             NULL,                                                  // bytes returned
             &(m_mappedDirectories[id]->m_overlapped),              // overlapped buffer
             NULL);                                                 // completion routine

         BeAssert(0 != readResult);
        }
    }
/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Mantas.Ragauskas       06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void DirectoryObserverWin::_StopObserving()
    {
    BeMutexHolder holder(m_mutex);
    CloseHandle(m_completionPort);
    m_completionPort = nullptr;
    }
/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Grigas.Petraitis                11/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void DirectoryObserverWin::_OnObservingStopped()
    {
    BeMutexHolder holder(m_mutex);
    for (auto pair : m_mappedDirectories)
        delete pair.second;

    m_mappedDirectories.clear();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Mantas.Ragauskas       06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static FileChanges ResolveChangeSatusWin(DWORD action)
    {
    switch (action)
        {
        case FILE_ACTION_ADDED:
            return FileChanges::FileAdded;
        case FILE_ACTION_REMOVED:
            return FileChanges::FileRemoved;
        case FILE_ACTION_MODIFIED:
            return FileChanges::FileMofified;
#if defined(ENABLE_RENAME)
        case FILE_ACTION_RENAMED_OLD_NAME:
            return FileChanges::FileRenamed_oldName;
        case FILE_ACTION_RENAMED_NEW_NAME:
            return FileChanges::FileRenamed_newName;
#else
        case FILE_ACTION_RENAMED_OLD_NAME:
            return FileChanges::FileRemoved;
        case FILE_ACTION_RENAMED_NEW_NAME:
            return FileChanges::FileAdded;
#endif
        default:  
            {
            BeAssert(false && "we should not get notifications about unknown actions");
            return FileChanges::FileMofified;
            }
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Mantas.Ragauskas       06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void DirectoryObserverWin::_Observe()
    {
    DWORD buffSize         = 0;
    ULONG_PTR completionKey = 0;
    OVERLAPPED* overlapped = NULL;

    m_startedObserving.store(true);
    m_startedObservingLock.notify_all();

    BeMutexHolder lock(m_mutex);
    HANDLE port = m_completionPort;
    lock.unlock();

    BOOL waitResult = FALSE;
    if (nullptr != port)
        waitResult = GetQueuedCompletionStatus(port, &buffSize, &completionKey, &overlapped, INFINITE);

    DIROBSERVER_DEBUGMSG("[Win][Observer woke up]");
    if (FALSE == waitResult || nullptr == overlapped)
        {
        DIROBSERVER_DEBUGMSG("[Win][Observer]: No callback");
        return;
        }
    
    lock.lock();
    auto it = m_mappedDirectories.find(completionKey);
    if (m_mappedDirectories.end() == it)
        {
        DIROBSERVER_DEBUGMSG("[Win][Observer]: No mapped directory");
        return;
        }
    ObservedDir* dirTriggered = it->second;

    DWORD byteCount;    
    if (!GetOverlappedResult(dirTriggered->m_dirHandle, overlapped, &byteCount, FALSE) || 0 == byteCount)
        return;

    bool isChangeNotificationPending = true;
    size_t offset = 0;

    while (isChangeNotificationPending)
        {
        FILE_NOTIFY_INFORMATION* fni = (FILE_NOTIFY_INFORMATION*) &dirTriggered->m_buffer[offset];

        WString fileNameW(&fni->FileName[0], ((size_t) fni->FileNameLength / 2)); // FileNameLength is given in bytes, not symbols
        BeFileName fullFileName = dirTriggered->m_dirName;
        fullFileName.AppendToPath(fileNameW.GetWCharCP());

        DIROBSERVER_DEBUGMSG("[Win][Callback]:     [%s] [Status %u]", Utf8String(fullFileName.Abbreviate(25)).c_str(), ResolveChangeSatusWin(fni->Action));
        RunCallbackAsync(fullFileName, ResolveChangeSatusWin(fni->Action));

        if (0 != fni->NextEntryOffset)
            offset += (fni->NextEntryOffset / sizeof(DWORD));   //Buffer is DWORD aligned (offset is given in bytes)
        else
            isChangeNotificationPending = false;
        }

    if (m_isObserving)                          //Restart observing triggered handle
        {
        BOOL readResult = ReadDirectoryChangesW(
            dirTriggered->m_dirHandle,           // handle to directory                       
            dirTriggered->m_buffer,              // read results buffer
            sizeof(dirTriggered->m_buffer),      // length of buffer
            FALSE,                              // monitoring option (watch subtree)
            FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION | FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME, // filter conditions
            NULL,                               // bytes returned                                                           
            &(dirTriggered->m_overlapped),       // overlapped buffer                                                       
            NULL);                              // completion routine 

        BeAssert(0 != readResult);
        }
    }

#endif // BENTLEY_WIN32

#if defined (ANDROID)
//=======================================================================================
// @bsiclass                                             Mantas.Ragauskas       06 / 2015
//=======================================================================================
struct DirectoryObserverLinux : DirectoryObserver
    {
private:
    int     m_inotifyInstance;
    int     m_resetDescriptor[2];      //Pipe serves as a reset; destriptor [0] for reading, [1] for writing to a pipe
    char    m_inotifyBuffer[BUF_LEN];
    fd_set  m_fileDescriptorSet;

    BeMutex m_mutex; 
    bmap<int, BeFileName> m_descriptors;

    void ProcessNotification(inotify_event& inotifyEvent);
    void Reset();

    virtual void _StartObserving() override;
    virtual void _StopObserving() override;
    virtual void _Observe()override;

public:
    DirectoryObserverLinux(bset<BeFileName>const& directories, ObserverCallback callback);
    DirectoryObserverLinux(ObserverCallback callback);
    ~DirectoryObserverLinux();
    };


/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Mantas.Ragauskas       06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
DirectoryObserverLinux::DirectoryObserverLinux(bset<BeFileName>const& directories, ObserverCallback callback) : DirectoryObserver(directories, callback)
    {
    m_inotifyInstance = inotify_init();

    BeAssert(-1 != m_inotifyInstance);
    
    int pipeCreationResult = pipe(m_resetDescriptor);
    BeAssert(-1 != pipeCreationResult);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Mantas.Ragauskas       06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
DirectoryObserverLinux::DirectoryObserverLinux(ObserverCallback callback) : DirectoryObserver(callback)
    {
    m_inotifyInstance = inotify_init();

    BeAssert(-1 != m_inotifyInstance);

    int pipeCreationResult = pipe(m_resetDescriptor);
    BeAssert(-1 != pipeCreationResult);
    }
/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Mantas.Ragauskas       06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
DirectoryObserverLinux::~DirectoryObserverLinux( )
    {
    int closeResult;
    closeResult = close(m_inotifyInstance);
    BeAssert(-1 != closeResult);

    closeResult = close(m_resetDescriptor[0]);
    BeAssert(-1 != closeResult);

    closeResult = close(m_resetDescriptor[1]);
    BeAssert(-1 != closeResult);

    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Mantas.Ragauskas       06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void DirectoryObserverLinux::_StartObserving()
    {
    BeMutexHolder holder(m_mutex);
    m_descriptors.clear();
    for (BeFileNameCR directory : m_directories)
        {
        int  descriptor = inotify_add_watch(
            m_inotifyInstance,                  // inotify instance   
            directory.GetNameUtf8().c_str(),            // directory name
            IN_MODIFY | IN_CREATE | IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO); // filter ocnditions
        
        
        BeAssert(-1 != descriptor);
        m_descriptors[descriptor] = directory;
        DIROBSERVER_DEBUGMSG("    [Dir added] [wd: %d] [%s] ", descriptor, Utf8String(directory.Abbreviate(25)).c_str());
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Mantas.Ragauskas       06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void DirectoryObserverLinux::_StopObserving()
    {
    Reset();
    BeMutexHolder holder(m_mutex);

    for (bpair<int, BeFileName> const& descriptor : m_descriptors)
        {
        int rmResult = inotify_rm_watch(m_inotifyInstance, descriptor.first);
        BeAssert(-1 != rmResult);
        }
    m_descriptors.clear();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Mantas.Ragauskas       06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void DirectoryObserverLinux::_Observe()
    {
    FD_ZERO(&m_fileDescriptorSet);
    FD_SET(m_resetDescriptor[0], &m_fileDescriptorSet);
    FD_SET(m_inotifyInstance, &m_fileDescriptorSet);

    int highestFileDescriptor = (m_inotifyInstance > m_resetDescriptor[0]) ? m_inotifyInstance : m_resetDescriptor[0]; // select requires highes numbered file descriptor passed as argument 

    m_startedObserving.store(true);
    m_startedObservingLock.notify_all();
    int eventNum = select(1 + highestFileDescriptor, &m_fileDescriptorSet, NULL, NULL, NULL);
    
    if (-1 == eventNum || 0 == eventNum)
        {
        DIROBSERVER_DEBUGMSG("    [Observer unkown eventNum]: %d", eventNum);
        return;
        }

    if (FD_ISSET(m_resetDescriptor[0], &m_fileDescriptorSet))
        {
        BeMutexHolder holder(m_mutex);
        DIROBSERVER_DEBUGMSG("    [Observer reset]");
        char buffer[8];
        read(m_resetDescriptor[0], &buffer, sizeof(buffer)); //Read the buffer so that the pipe does not get full
        }
    else if (FD_ISSET(m_inotifyInstance, &m_fileDescriptorSet))
        {
        int length = read(m_inotifyInstance, m_inotifyBuffer, BUF_LEN);
        if (length < 0)
            {
            DIROBSERVER_DEBUGMSG("    [Observer inotify read error]");
            return;
            }

        int i = 0;
        while (i < length)
            {
            struct inotify_event* inotifyEvent = (struct inotify_event*) &m_inotifyBuffer[i];
            i += EVENT_SIZE + inotifyEvent->len;
            if (inotifyEvent->len)
                ProcessNotification(*inotifyEvent);
            }
        memset(m_inotifyBuffer, 0, sizeof(m_inotifyBuffer));
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Mantas.Ragauskas       06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void DirectoryObserverLinux::Reset()
    {
    Utf8String message = "Reset";
    write(m_resetDescriptor[1], message.c_str(), message.length());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Mantas.Ragauskas       06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static FileChanges ResolveChangeSatusLinux(uint32_t action)
    {
    if (action &  IN_CREATE)
        return FileChanges::FileAdded;
    else if (action &  IN_DELETE)
        return FileChanges::FileRemoved;
    else if (action &  IN_MODIFY)
        return FileChanges::FileMofified;
#if defined(ENABLE_RENAME)
    else if (action &  IN_MOVED_FROM)
        return FileChanges::FileRenamed_oldName;
    else if (action &  IN_MOVED_TO)
        return FileChanges::FileRenamed_newName;
#else
    else if (action &  IN_MOVED_FROM)
        return FileChanges::FileRemoved;
    else if (action &  IN_MOVED_TO)
        return FileChanges::FileAdded;
#endif
    BeAssert(false && "we should not get notifications about unknown actions");
    return FileChanges::FileMofified;
    }
/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Mantas.Ragauskas       06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void DirectoryObserverLinux::ProcessNotification(inotify_event& inotifyEvent)
    {
    WString    fileNameW(Utf8String(inotifyEvent.name, inotifyEvent.len).c_str(), true);
    BeFileName fullName = m_descriptors[inotifyEvent.wd];
    fullName.AppendToPath(fileNameW.GetWCharCP());

#ifdef DEBUG_CALLBACKS
    DIROBSERVER_DEBUGMSG("    [wd received]     [%d] ", inotifyEvent.wd);   
    DIROBSERVER_DEBUGMSG("    [mask received]   [%u] ", inotifyEvent.mask); 
    DIROBSERVER_DEBUGMSG("    [cookie received] [%u] ", inotifyEvent.cookie); 
    DIROBSERVER_DEBUGMSG("    [len received]    [%u] ", inotifyEvent.len); 
    DIROBSERVER_DEBUGMSG("    [Name in map]     [%s] ", Utf8String(fullName.Abbreviate(25)).c_str()); 
    DIROBSERVER_DEBUGMSG("    [Callback]        [%s] [Status %u]", Utf8String(fullName.Abbreviate(25)).c_str(), ResolveChangeSatusLinux(inotifyEvent.mask));
#endif

    RunCallbackAsync(fullName, ResolveChangeSatusLinux(inotifyEvent.mask));
    }
#endif //ANDROID

#if defined (__APPLE__)
/*--------------------------------------------------------------------------------------+
 * @bsimethod                                              Mantas.Ragauskas       07/2015
 +---------------+---------------+---------------+---------------+---------------+------*/
struct DirectorySnapshot
    {
    bmap<ino_t, BeFileName> m_files;
    BeFileNameCR m_directoryName;
        
private:
    /*--------------------------------------------------------------------------------------+
     * @bsimethod                                              Mantas.Ragauskas       07/2015
     +---------------+---------------+---------------+---------------+---------------+------*/
    DirectorySnapshot(bmap<ino_t, BeFileName>& files, BeFileNameCR directoryName) : m_directoryName(directoryName) { m_files = files; }
        
public:
    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                              Mantas.Ragauskas       07/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    static DirectorySnapshot* MakeSnapshot (BeFileNameCR directoryName)
        {
        bmap<ino_t, BeFileName> dirInfo;
        BeDirectoryIterator it (directoryName);
        
        StatusInt status = BentleyStatus::SUCCESS;
        BeFileName name;
        while (BentleyStatus::ERROR != status)
            {
            bool isDir;
            status = it.GetCurrentEntry(name, isDir);
            if (BentleyStatus::ERROR == status)
                break;
                
            struct stat statStruct;
            if (-1 !=  stat(name.GetNameUtf8().c_str(), &statStruct))
                {
                dirInfo[statStruct.st_ino] = name;
                }
            else
                {
                BeAssert(false);
                }
            status = it.ToNext();
            }
        return new DirectorySnapshot(dirInfo, directoryName);
        }
    };
//=======================================================================================
// @bsiclass                                             Mantas.Ragauskas       07/ 2015
//=======================================================================================
struct DirectoryObserverIOS : DirectoryObserver
    {
private:
    int     m_resetDescriptor[2];      //Pipe serves as reset; destriptor [0] for reading, [1] for writing to a pipe  
    int     m_kqueue;
    BeMutex m_mutex;

    bmap<int, DirectorySnapshot*> m_descriptors;
    struct kevent* m_events;
    struct kevent* m_eventResults;


    void ProcessNotification(int dirDescriptor);
    void Reset();

    virtual void _StartObserving() override;
    virtual void _StopObserving() override;
    virtual void _Observe() override;

public:
    DirectoryObserverIOS(bset<BeFileName>const& directories, ObserverCallback callback);
    DirectoryObserverIOS(ObserverCallback callback);
    ~DirectoryObserverIOS();
    };


/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Mantas.Ragauskas       06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
DirectoryObserverIOS::DirectoryObserverIOS(bset<BeFileName>const& directories, ObserverCallback callback) : DirectoryObserver(directories, callback)
    {
    int pipeCreationResult = pipe(m_resetDescriptor);
    BeAssert(-1 != pipeCreationResult);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Mantas.Ragauskas       06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
DirectoryObserverIOS::DirectoryObserverIOS(ObserverCallback callback) : DirectoryObserver(callback)
    {
    int pipeCreationResult = pipe(m_resetDescriptor);
    BeAssert(-1 != pipeCreationResult);
    }
/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Mantas.Ragauskas       06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
DirectoryObserverIOS::~DirectoryObserverIOS()
    {
    int closeResult = close(m_resetDescriptor[0]);
    BeAssert(-1 != closeResult);

    closeResult = close(m_resetDescriptor[1]);
    BeAssert(-1 != closeResult);

    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Mantas.Ragauskas       06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void DirectoryObserverIOS::_StartObserving()
    {
    BeMutexHolder holder(m_mutex);
    m_descriptors.clear();

    if ((m_kqueue = kqueue()) == -1)
        BeAssert(false);

    m_events = new struct kevent[m_directories.size() + 1];
    m_eventResults = new struct kevent[m_directories.size() + 1];

    struct kevent ev;

    m_events[0] = ev;// reset
    EV_SET(&m_events[0], m_resetDescriptor[0], EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, 0);
    
    int i = 1;
    for (BeFileNameCR directory : m_directories)
        {
        int fileDescriptor = open(directory.GetNameUtf8().c_str(), O_RDONLY);
        BeAssert(-1 != fileDescriptor);

        m_descriptors[fileDescriptor] = DirectorySnapshot::MakeSnapshot(directory);

        m_events[i] = ev;
        EV_SET(&m_events[i],                                          // &kev
               fileDescriptor,                                        // ident
               EVFILT_VNODE,                                          // filter
               EV_ADD | EV_ENABLE | EV_CLEAR,                         // flags
               NOTE_DELETE | NOTE_WRITE | NOTE_EXTEND | NOTE_RENAME,  // fflags
               0,                                                     // data
               0);                                                    // udata
        i++;
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Mantas.Ragauskas       06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void DirectoryObserverIOS::_StopObserving()
    {
    Reset();
    BeMutexHolder holder(m_mutex);

    for (bpair<int, DirectorySnapshot*>const& descriptor : m_descriptors)
        {
        delete descriptor.second;
        close(descriptor.first);
        }

    delete[]m_events;
    delete[]m_eventResults;
    
    close(m_kqueue);
    m_descriptors.clear();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Mantas.Ragauskas       06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void DirectoryObserverIOS::_Observe()
    {
    m_startedObserving.store(true);
    m_startedObservingLock.notify_all();
    
    int numberOfEvents = kevent(m_kqueue, m_events, m_directories.size() + 1, m_eventResults, m_directories.size() + 1, NULL);
    BeMutexHolder holder(m_mutex);

    if (-1 == numberOfEvents)
        {
        DIROBSERVER_DEBUGMSG("    [Observer kevent wait error]");
        return;
        }

        
    bset<int> triggeredDirectoies;
    for (int i = 0; i < numberOfEvents; i++)
        {
        bool isReset = (m_eventResults[i].ident == m_resetDescriptor[0]);
        bool isError = (m_eventResults[i].flags & EV_ERROR);
        bool isKnownDescriptor = (m_descriptors.find(m_eventResults[i].ident) != m_descriptors.end());
        
        if (isError)
            continue;

        if (isReset)
            {
            char buffer[8];
            read(m_resetDescriptor[0], &buffer, sizeof(buffer)); //Read the buffer so that the pipe does not get full
            }
        else if (isKnownDescriptor)
            {
            triggeredDirectoies.insert(m_eventResults[i].ident);
            }
        }
        
    for (int descriptor : triggeredDirectoies)
        ProcessNotification(descriptor);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Mantas.Ragauskas       06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void DirectoryObserverIOS::Reset()
    {
    Utf8String message = "Reset";
    write(m_resetDescriptor[1], message.c_str(), message.length());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Mantas.Ragauskas       06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void DirectoryObserverIOS::ProcessNotification(int dirDescriptor)
    {
    DirectorySnapshot* oldSnapshot = m_descriptors[dirDescriptor];
    DirectorySnapshot* newSnapshot = DirectorySnapshot::MakeSnapshot(oldSnapshot->m_directoryName);

    for (bpair<ino_t, BeFileName>const& it : oldSnapshot->m_files)
        {
        if (newSnapshot->m_files.find(it.first) != newSnapshot->m_files.end())
            {
#if defined(ENABLE_RENAME)
            RunCallbackAsync(it.second, FileChanges::FileRenamed_oldName);
            RunCallbackAsync(newSnapshot->m_files[it.first], FileChanges::FileRenamed_newName);
#else
            RunCallbackAsync(it.second, FileChanges::FileRemoved);
            RunCallbackAsync(newSnapshot->m_files[it.first], FileChanges::FileAdded);
#endif
            }
        else
            {
            RunCallbackAsync(it.second, FileChanges::FileRemoved);
            }
        }
     
    for (bpair<ino_t, BeFileName>const& it : newSnapshot->m_files)
        {
        if (oldSnapshot->m_files.find(it.first) == oldSnapshot->m_files.end())
            RunCallbackAsync(it.second, FileChanges::FileAdded);
        }
    delete oldSnapshot;
    m_descriptors[dirDescriptor] = newSnapshot;
    }
#endif // __APPLE__

#if defined (BENTLEY_WINRT)
/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Mantas.Ragauskas       07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
struct DirectorySnapshot
    {
    bmap<BeFileName, uint64_t> m_files;
    BeFileNameCR m_directory;

private:
    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                              Mantas.Ragauskas       07/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    DirectorySnapshot(bmap<BeFileName, uint64_t>const& files, BeFileNameCR directory) : m_directory(directory)
        { 
        m_files = files; 
        }

public:
    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                              Mantas.Ragauskas       07/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    static DirectorySnapshot* MakeSnapshot(BeFileNameCR directory)
        {
        bmap<BeFileName, uint64_t> directoryInfo;
        
        BeDirectoryIterator it(directory);

        StatusInt status = BentleyStatus::SUCCESS;
        BeFileName name;
        while (BentleyStatus::ERROR != status)
            {
            bool isDir;
            status = it.GetCurrentEntry(name, isDir);
            if (BentleyStatus::ERROR == status)
                break;

            struct _stat statStruct;

            if (-1 == _wstat(name, &statStruct))
                {
                BeAssert(false && "Should not fail getting information on files");
                }

            directoryInfo[name] = statStruct.st_mtime;
            status = it.ToNext();
            }

        return new DirectorySnapshot(directoryInfo, directory);
        }
    };

//=======================================================================================
// @bsiclass                                             Mantas.Ragauskas       07/ 2015
//=======================================================================================
typedef RefCountedPtr<struct DirectoryObserverWinRT> DirectoryObserverWinRTPtr;
struct DirectoryObserverWinRT : DirectoryObserver
    {
    private:
        BeAtomic<bool>       m_reset;
        BeConditionVariable  m_resetCV;
        ChangesPredicate     m_resetPredicate;
        BeMutex              m_mutex;

        bmap<BeFileName, DirectorySnapshot*> m_snapshots;
        
        void Reset();
        void ProcessNotification();

        virtual void _StartObserving() override;
        virtual void _StopObserving() override;
        virtual void _Observe()override;
    public:
        DirectoryObserverWinRT(bset<BeFileName>const& directories, ObserverCallback callback);
        DirectoryObserverWinRT(ObserverCallback callback);
        ~DirectoryObserverWinRT();
    };


/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Mantas.Ragauskas       07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
DirectoryObserverWinRT::DirectoryObserverWinRT(bset<BeFileName>const& directories, ObserverCallback callback) : DirectoryObserver(directories, callback), m_resetPredicate(m_reset) { }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Mantas.Ragauskas       07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
DirectoryObserverWinRT::DirectoryObserverWinRT(ObserverCallback callback) : DirectoryObserver(callback), m_resetPredicate(m_reset) { }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Mantas.Ragauskas       07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
DirectoryObserverWinRT::~DirectoryObserverWinRT()   
    {
    DIROBSERVER_DEBUGMSG("[WinRT][Observer deleted]: %llu", BeTimeUtilities::GetCurrentTimeAsUnixMillis());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Mantas.Ragauskas       07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void DirectoryObserverWinRT::Reset()
    {
    DIROBSERVER_DEBUGMSG("[WinRT][Observer reseting]: %llu", BeTimeUtilities::GetCurrentTimeAsUnixMillis());
    m_reset.store(true);
    m_resetCV.notify_all();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Mantas.Ragauskas       07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void DirectoryObserverWinRT::_StartObserving()
    {
    DIROBSERVER_DEBUGMSG("[WinRT][Starting]: %llu",BeTimeUtilities::GetCurrentTimeAsUnixMillis());

    BeMutexHolder holder(m_mutex);

    for (BeFileNameCR directory : m_directories)
        m_snapshots[directory] = DirectorySnapshot::MakeSnapshot(directory);

    DIROBSERVER_DEBUGMSG("[WinRT][Started]:  %llu",BeTimeUtilities::GetCurrentTimeAsUnixMillis());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Mantas.Ragauskas       07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void DirectoryObserverWinRT::_StopObserving()
    {
    Reset();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Mantas.Ragauskas       07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void DirectoryObserverWinRT::_Observe()
    {
    m_startedObserving.store(true);
    m_startedObservingLock.notify_all();

    m_reset.store(false);
    DIROBSERVER_DEBUGMSG("[WinRT][Observer wait]: %llu", BeTimeUtilities::GetCurrentTimeAsUnixMillis());
    m_resetCV.WaitOnCondition(&m_resetPredicate, 1000);
    DIROBSERVER_DEBUGMSG("[WinRT][Observer reset done]: %llu", BeTimeUtilities::GetCurrentTimeAsUnixMillis());    BeMutexHolder holder(m_mutex);
    
    if (!m_reset)
        ProcessNotification();

    if (!m_isObserving)
        {
        for (bpair<BeFileName, DirectorySnapshot*>const& snapShot : m_snapshots)
            delete snapShot.second;

        m_snapshots.clear();
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Mantas.Ragauskas       07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void DirectoryObserverWinRT::ProcessNotification()
    {
    DIROBSERVER_DEBUGMSG("[WinRT][Processing start]: %llu",BeTimeUtilities::GetCurrentTimeAsUnixMillis());  
    for (BeFileNameCR directory : m_directories)
        {
        DirectorySnapshot* oldSnapshot = m_snapshots[directory];
        DirectorySnapshot* newSnapshot = DirectorySnapshot::MakeSnapshot(oldSnapshot->m_directory);
        
        for (bpair <BeFileName, uint64_t>const& it : oldSnapshot->m_files)
            {
            BeFileNameCR fileName = it.first;
            if (newSnapshot->m_files.find(fileName) != newSnapshot->m_files.end())
                {                  
                if (it.second != newSnapshot->m_files[fileName])
                    RunCallbackAsync(fileName, FileChanges::FileMofified);
                }
            else
                {
                RunCallbackAsync(fileName, FileChanges::FileRemoved);
                }
            }
        
        for (bpair<BeFileName, uint64_t>const& it : newSnapshot->m_files)
            {
            if (oldSnapshot->m_files.find(it.first) == oldSnapshot->m_files.end())
                RunCallbackAsync(it.first, FileChanges::FileAdded);
            }
        delete oldSnapshot;
        m_snapshots[directory] = newSnapshot;
        }   
    DIROBSERVER_DEBUGMSG("[WinRT][Processing end]:   %llu",BeTimeUtilities::GetCurrentTimeAsUnixMillis());
    }

#endif // BENTLEY_WINRT

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Mantas.Ragauskas       06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
DirectoryObserverPtr DirectoryObserver::Create(bset<BeFileName>const& directories, ObserverCallback callback)
    {
    
#if defined (BENTLEY_WIN32)
    DirectoryObserverPtr observer = new DirectoryObserverWin(directories, callback);
#elif defined (ANDROID)
    DirectoryObserverPtr observer =  new DirectoryObserverLinux(directories, callback);
#elif defined (__APPLE__)
    DirectoryObserverPtr observer =  new DirectoryObserverIOS(directories, callback);
#elif defined (BENTLEY_WINRT)
    DirectoryObserverPtr observer =  new DirectoryObserverWinRT(directories, callback);
#else
    BeAssert(false && "Not implemented.");
    DirectoryObserverPtr observer =  nullptr;
#endif
    observer->StartObserving();
    return observer;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              Mantas.Ragauskas       06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
DirectoryObserverPtr DirectoryObserver::Create(ObserverCallback callback) 
    {
#if defined (BENTLEY_WIN32)
    DirectoryObserverPtr observer = new DirectoryObserverWin(callback);
#elif defined (ANDROID)
    DirectoryObserverPtr observer = new DirectoryObserverLinux(callback);
#elif defined (__APPLE__)
    DirectoryObserverPtr observer = new DirectoryObserverIOS(callback);
#elif defined (BENTLEY_WINRT)
    DirectoryObserverPtr observer = new DirectoryObserverWinRT(callback);
#else
    BeAssert(false && "Not implemented.");
    DirectoryObserverPtr observer = nullptr;
#endif 
    return observer;
    }
