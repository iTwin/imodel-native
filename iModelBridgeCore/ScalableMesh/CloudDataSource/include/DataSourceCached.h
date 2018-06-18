#pragma once
#include "DataSourceDefs.h"
#include "DataSource.h"
#include "DataSourceBuffered.h"
#include <Bentley/RefCounted.h>
#include <Bentley/BeFile.h>
#include <queue>


struct CacheWriter : public RefCountedBase, std::mutex, std::condition_variable
    {
    public:

        typedef RefCountedPtr<CacheWriter> Ptr;

    struct CacheData : public RefCountedBase
        {
        DataSourceURL m_url;
        DataSourceBuffer::BufferData *m_dest;
        DataSourceBuffer::BufferSize m_size;
        CacheData(const DataSourceURL& url, DataSourceBuffer::BufferData * dest, DataSourceBuffer::BufferSize size) 
            : m_url(url),
              m_size(size)
            {
            m_dest = new DataSourceBuffer::BufferData[m_size];
            memcpy(m_dest, dest, size);
            }
        ~CacheData() 
            { 
            delete[] m_dest; 
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
        BeFileSharing streamSharing = BeFileSharing::None;
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

        if (!stream.IsOpen() && BeFileStatus::Success != stream.Open(data->m_url.c_str(), streamMode, streamSharing))
            {
            return DataSourceStatus(DataSourceStatus::Status_Error);
            }

        UInt32 bytesWritten = 0;
        if (BeFileStatus::Success != stream.Write(&bytesWritten, reinterpret_cast<const char *>(data->m_dest), data->m_size))
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
    DataSourceStatus                writeToCache            (DataSourceBuffer::BufferData *source, DataSourceBuffer::BufferSize size);

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
    DataSourceStatus                write                   (Buffer * source, DataSize size);

};