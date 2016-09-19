#pragma once

#include "DataSourceDefs.h"
#include <string>
#include <map>
#include <curl/curl.h>

#include "DataSource.h"
#include "DataSourceAccount.h"
#include "DataSourceAccountCached.h"
#include "DataSourceBuffer.h"
#include "DataSourceMode.h"
#include "Manager.h"


class OpenSSLMutexes
    {
    private:
        static OpenSSLMutexes *s_instance;
        /* This array will store all of the mutexes available to OpenSSL. */
        std::mutex* m_mutexes;
        
        OpenSSLMutexes() = delete;
        OpenSSLMutexes(size_t numMutexes);

    public:
        ~OpenSSLMutexes();
        std::mutex* GetMutexes();

        static OpenSSLMutexes *CreateInstance(const size_t& numMutexes);
        static OpenSSLMutexes *Instance();
    };

class DataSourceAccountCURL : public DataSourceAccountCached
{

protected:

    class CURLHandleManager : public Manager<CURL*>
        {
        public:
            typedef std::wstring                            HandleName;

        public:

            CURL *     getOrCreateCURLHandle(const HandleName &name, bool *created = nullptr);
            CURL *     getOrCreateThreadCURLHandle(bool *created = nullptr);

        private:

            CURL *  createCURLHandle(const HandleName &name);


        };

    CURLHandleManager m_CURLManager;

    DataSourceBuffer::BufferSize            defaultSegmentSize;
    DataSourceBuffer::Timeout               defaultTimeout;


public:
                                            DataSourceAccountCURL                (const AccountName &account, const AccountIdentifier &identifier, const AccountKey &key);
        virtual                            ~DataSourceAccountCURL                (void);

        void                                setDefaultSegmentSize               (DataSourceBuffer::BufferSize size);
        DataSourceBuffer::BufferSize        getDefaultSegmentSize               (void);

        void                                setDefaultTimeout                   (DataSourceBuffer::Timeout time);
        DataSourceBuffer::Timeout           getDefaultTimeout                   (void);

        DataSourceStatus                    setAccount                          (const AccountName &account, const AccountIdentifier & identifier, const AccountKey & key);

        virtual void                        setPrefixPath                       (const DataSourceURL &prefix) override;

        DataSource                   *      createDataSource                    (void);
        DataSourceStatus                    destroyDataSource                   (DataSource *dataSource);

        DataSourceStatus                    downloadBlobSync                    (DataSource &dataSource, DataSourceBuffer::BufferData * dest, DataSourceBuffer::BufferSize destSize, DataSourceBuffer::BufferSize &readSize);
        DataSourceStatus                    downloadBlobSync                    (DataSourceURL &blobPath, DataSourceBuffer::BufferData * source, DataSourceBuffer::BufferSize &readSize, DataSourceBuffer::BufferSize size);
        DataSourceStatus                    uploadBlobSync                      (DataSourceURL & url, const std::wstring &filename, DataSourceBuffer::BufferData * source, DataSourceBuffer::BufferSize size);
        DataSourceStatus                    uploadBlobSync                      (DataSource & dataSource, DataSourceBuffer::BufferData * source, DataSourceBuffer::BufferSize size);

protected :

       struct CURLDataMemoryBuffer {
           DataSourceBuffer::BufferData* data;
           size_t                        size;
           };
       struct CURLDataResponseHeader {
           std::map<std::string, std::string> data;
           };
    
       static size_t CURLWriteHeaderCallback        (void *contents, size_t size, size_t nmemb,  void *userp);
       static size_t CURLWriteDataCallback          (void *contents, size_t size, size_t nmemb,  void *userp);
       static size_t CURLDummyWriteDataCallback     (void *contents, size_t size, size_t nmemb,  void *userp);
       static size_t CURLReadDataCallback           (char *bufptr,   size_t size, size_t nitems, void *userp);

       static void   OpenSSLLockingFunction(int mode, int n, const char * file, int line);
    };
