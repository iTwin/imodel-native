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


#include <Bentley/WString.h>


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

    struct CURLHandle
        {
        private:
        CURL*              m_curl    = nullptr;
        struct curl_slist* m_headers = nullptr;
        public:
            CURLHandle() = delete;
            CURLHandle(CURL* curl) : m_curl(curl)
                {
                }
            CURL*    get(void)
                {
                return m_curl;
                }
            struct curl_slist* get_headers(void)
                {
                return m_headers;
                }
            void add_item_to_header(const char* item)
                {
                m_headers = curl_slist_append(m_headers, item);
                }
            void free_header_list(void)
                {
                curl_slist_free_all(m_headers);
                m_headers = nullptr;
                }
				
			DataSourceStatus destroyAll(void)
				{
				return DataSourceStatus();
				}

            struct CURLDataMemoryBuffer {
                union DataType { public: DataSourceBuffer::BufferData* raw_data; DataSourceBuffer* vector; };
                DataType                      data;
                size_t                        size;
                size_t                        max_size;
                };
            struct CURLDataResponseHeader {
                std::map<std::string, std::string> data;
                };

            static size_t CURLWriteHeaderCallback(void *contents, size_t size, size_t nmemb, void *userp);
            static size_t CURLWriteDataCallbackRaw(void *contents, size_t size, size_t nmemb, void *userp);
            static size_t CURLWriteDataCallbackVector(void *contents, size_t size, size_t nmemb, void *userp);
            static size_t CURLDummyWriteDataCallback(void *contents, size_t size, size_t nmemb, void *userp);
            static size_t CURLReadDataCallback(char *bufptr, size_t size, size_t nitems, void *userp);
            static void   OpenSSLLockingFunction(int mode, int n, const char * file, int line);
        };

    class CURLHandleManager : public Manager<CURLHandle, true>
        {
        public:
            typedef std::wstring                            HandleName;

        public:

            CURLHandle*     getOrCreateCURLHandle(const HandleName &name, bool *created = nullptr);
            CURLHandle*     getOrCreateThreadCURLHandle(bool *created = nullptr);

        private:

            CURLHandle*     createCURLHandle(const HandleName &name);
        };



    CURLHandleManager m_CURLManager;

    DataSourceBuffer::BufferSize            defaultSegmentSize;
    DataSourceBuffer::Timeout               defaultTimeout;

    Utf8String proxyUser;
    Utf8String proxyPassword;
    Utf8String proxyServerUrl;
    Utf8String certificateAuthoritiesUrl;

    void setupProxyToCurl(CURL* curl);
    void setupCertificateAuthorities(CURL* curl);

public:
                                            DataSourceAccountCURL               (void) {}
                                            DataSourceAccountCURL               (const AccountName &account, const AccountIdentifier &identifier, const AccountKey &key);
        virtual                            ~DataSourceAccountCURL               (void);

        void                                setDefaultSegmentSize               (DataSourceBuffer::BufferSize size);
        DataSourceBuffer::BufferSize        getDefaultSegmentSize               (void);

        void                                setDefaultTimeout                   (DataSourceBuffer::Timeout time);
        DataSourceBuffer::Timeout           getDefaultTimeout                   (void);

        DataSourceStatus                    setAccount                          (const AccountName &account, const AccountIdentifier & identifier, const AccountKey & key);

        virtual void                        setPrefixPath                       (const DataSourceURL &prefix) override;

        DataSource                   *      createDataSource                    (const SessionName &session);
        DataSourceStatus                    destroyDataSource                   (DataSource *dataSource);

        DataSourceStatus                    downloadBlobSync                    (DataSource &dataSource, DataSourceBuffer::BufferData * dest, DataSourceBuffer::BufferSize destSize, DataSourceBuffer::BufferSize &readSize);
        DataSourceStatus                    downloadBlobSync                    (DataSourceURL &blobPath, DataSourceBuffer::BufferData * source, DataSourceBuffer::BufferSize &readSize, DataSourceBuffer::BufferSize size, const DataSource::SessionName &session);
        DataSourceStatus                    downloadBlobSync                    (DataSourceURL &url, DataSourceBuffer* buffer, const DataSource::SessionName &session);
        DataSourceStatus                    uploadBlobSync                      (DataSourceURL & url, const std::wstring &filename, DataSourceBuffer::BufferData * source, DataSourceBuffer::BufferSize size);
        DataSourceStatus                    uploadBlobSync                      (DataSource & dataSource, DataSourceBuffer::BufferData * source, DataSourceBuffer::BufferSize size);
        DataSourceStatus                    uploadBlobSync                      (const DataSourceURL & dataSource, DataSourceBuffer* source);

        CLOUD_EXPORT void                   setProxy                            (const Utf8String& proxyUserIn, const Utf8String& proxyPasswordIn, const Utf8String& proxyServerUrlIn);
        CLOUD_EXPORT void                   setCertificateAuthoritiesUrl        (const Utf8String& certificateAuthoritiesUrlIn);
    };    