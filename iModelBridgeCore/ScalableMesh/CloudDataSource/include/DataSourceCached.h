#pragma once
#include "DataSourceDefs.h"
#include "DataSource.h"
#include "DataSourceBuffered.h"
#include <Bentley/RefCounted.h>
#include <Bentley/BeFile.h>
#include <queue>
#ifdef VANCOUVER_API
#define OPEN_FILE_WITH_SHARING(beFile, pathStr, accessMode, sharing)  beFile.Open(pathStr, accessMode, sharing)
#define OPEN_FILE(beFile, pathStr, accessMode) beFile.Open(pathStr, accessMode, BeFileSharing::None)
#define OPEN_FILE_SHARE(beFile, pathStr, accessMode) beFile.Open(pathStr, accessMode, BeFileSharing::Read)
#else
#define OPEN_FILE(beFile, pathStr, accessMode) beFile.Open(pathStr, accessMode)
#define OPEN_FILE_SHARE(beFile, pathStr, accessMode) beFile.Open(pathStr, accessMode)
#define OPEN_FILE_WITH_SHARING(beFile, pathStr, accessMode, sharing)  beFile.Open(pathStr, accessMode)
#endif

struct CacheWriter : public RefCountedBase, std::mutex, std::condition_variable
    {
    public:

        typedef RefCountedPtr<CacheWriter> Ptr;

    struct CacheData : public RefCountedBase
        {
        DataSourceURL m_url;
        DataSourceBuffer::BufferData *m_dest;
        DataSourceBuffer::BufferSize m_size;
        bool m_managesData;
        CacheData(const DataSourceURL& url, DataSourceBuffer::BufferData * dest, DataSourceBuffer::BufferSize size) 
            : m_url(url),
              m_size(size),
              m_managesData(true)
            {
            m_dest = new DataSourceBuffer::BufferData[m_size];
            memcpy(m_dest, dest, size);
            }
        CacheData(const DataSourceURL& url, std::vector<DataSourceBuffer::BufferData>& dest)
            : m_url(url),
            m_size(dest.size()),
            m_managesData(false)
            {
            m_dest = dest.data();
            }
        ~CacheData()
            { 
            if (m_managesData)
                {
                delete[] m_dest;
                }
            m_dest = nullptr;
            m_size = 0;
            }
        };

    typedef RefCountedPtr<CacheData> CacheDataPtr;

    static Ptr GetCacheWriter();

    static void ShutdownCacheWriter();

    void push(CacheDataPtr data)
        {
        std::unique_lock<std::mutex> lock(*this);
        if (!m_shutdown)
            {
            m_queue.push(data);
            notify_one();
            }
        }

    private:
    
    std::queue<CacheDataPtr> m_queue;
    static Ptr s_cacheWriter;
    std::thread m_thread;
    bool m_shutdown = false;

    CacheWriter()
        {
        m_thread = std::thread([this]() 
            {
            std::unique_lock<std::mutex> lock(*this);
            while (!m_shutdown)
                {
                while (!m_queue.empty())
                    {
                    auto data = m_queue.front();
                    m_queue.pop();
                    lock.unlock();
                    Write(data);
                    lock.lock();
                    }
                wait(lock);
                }
            });
#ifndef NDEBUG
        DWORD ThreadId = ::GetThreadId(static_cast<HANDLE>(m_thread.native_handle()));
        SetThreadName(ThreadId, "DataSourceCacheWriterThread");
#endif
        }

    ~CacheWriter()
        {
        }

    void Shutdown()
        {
            {
            std::unique_lock<std::mutex> lock(*this);
            m_shutdown = true;
            notify_one();
            }
        if (m_thread.joinable()) m_thread.join();
        }

    DataSourceStatus Write(CacheDataPtr data)
        {
        DataSourceStatus        statusErrorWrite(DataSourceStatus::Status_Error_Write);
        BeFileAccess streamMode = BeFileAccess::Write;
#ifdef VANCOUVER_API
        BeFileSharing streamSharing = BeFileSharing::None;
#endif
        DataSourceStatus            status;
        BeFile stream;

        // strip filename from path
        auto directoryPath = BeFileName::GetDirectoryName(data->m_url.c_str());
        if (!BeFileName::DoesPathExist(directoryPath.c_str()))
            {
            // Path does not exist, create it
            if (BeFileNameStatus::Success != BeFileName::CreateNewDirectory(directoryPath.c_str()))
                {
                return DataSourceStatus(DataSourceStatus::Status_Error_Not_File_Path);
                }
            }

        if (!BeFileName::DoesPathExist(data->m_url.c_str()))
            {
            // File does not exist, create it
            if (BeFileStatus::Success != stream.Create(data->m_url.c_str()))
                {
                return DataSourceStatus(DataSourceStatus::Status_Not_Found);
                }
            }

        if (!stream.IsOpen() && BeFileStatus::Success != OPEN_FILE_WITH_SHARING(stream,data->m_url.c_str(), streamMode, streamSharing))
            {
            return DataSourceStatus(DataSourceStatus::Status_Error);
            }

        uint32_t bytesWritten = 0;
        if (BeFileStatus::Success != stream.Write(&bytesWritten, reinterpret_cast<const char *>(data->m_dest), (uint32_t)data->m_size))
            {
            return DataSourceStatus(DataSourceStatus::Status_Error);
            }

        return DataSourceStatus();
        }
    };


class DataSourceCached : public DataSourceBuffered
{

protected:

    typedef    DataSourceBuffered    Super;

protected:

    bool                            cachingEnabled;
    DataSourceURL                   cacheURL;
    bool                            writeCache;

    DataSource                *     cacheDataSource;

protected:

    bool                            isCached                (void);

    DataSourceStatus                readFromCache           (DataSourceBuffer::BufferData *dest, DataSourceBuffer::BufferSize destSize, DataSourceBuffer::BufferSize &readSize, DataSourceBuffer::BufferSize size);
    DataSourceStatus                readFromCache           (std::vector<DataSourceBuffer::BufferData>& dest);
    DataSourceStatus                writeToCache            (DataSourceBuffer::BufferData *source, DataSourceBuffer::BufferSize size);
    DataSourceStatus                writeToCache            (std::vector<DataSourceBuffer::BufferData>& source);

    void                            setWriteToCache         (bool write);
    bool                            getWriteToCache         (void);

    void                            setCacheURL             (const DataSourceURL & url);
    const DataSourceURL        &    getCacheURL             (void);

    void                            setCacheDataSource      (DataSource *source);
    DataSource                *     getCacheDataSource      (void);

public:
                                    DataSourceCached        (DataSourceAccount *account, const SessionName &session);

    void                            setCachingEnabled       (bool enabled);
    bool                            getCachingEnabled       (void);
    void                            setForceWriteToCache    (void);

    DataSourceStatus                open                    (const DataSourceURL & sourceURL, DataSourceMode sourceMode);
    DataSourceStatus                close                   (void);

    DataSourceStatus                read                    (Buffer *dest, DataSize destSize, DataSize &readSize, DataSize size);
    DataSourceStatus                read                    (std::vector<Buffer>& dest);
    DataSourceStatus                write                   (Buffer * source, DataSize size);

};