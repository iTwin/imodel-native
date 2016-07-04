/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/BeHttp/HttpBody.h $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <BeJsonCpp/BeJsonUtilities.h>
#include <Bentley/BeFile.h>
#include <Bentley/RefCounted.h>
#include <Bentley/ByteStream.h>
#include <rapidjson/BeRapidJson.h>

#include <BeHttp/Http.h>

BEGIN_BENTLEY_HTTP_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
typedef struct HttpBody& HttpBodyR;
typedef const struct HttpBody& HttpBodyCR;
typedef RefCountedPtr<struct HttpBody> HttpBodyPtr;

struct EXPORT_VTABLE_ATTRIBUTE HttpBody : public RefCountedBase
    {
    protected:
        HttpBody ();
        virtual ~HttpBody ();

    public:
        // Open for Read/Write
        BEHTTP_EXPORT virtual void Open () = 0;

        // Close and release resources
        BEHTTP_EXPORT virtual void Close () = 0;

        // Set Read/Write position
        BEHTTP_EXPORT virtual BentleyStatus SetPosition (uint64_t position) = 0;

        // Get current position
        BEHTTP_EXPORT virtual BentleyStatus GetPosition (uint64_t& position) = 0;

        // Reset position and clear body contents
        BEHTTP_EXPORT virtual BentleyStatus Reset () = 0;

        // Write contents of buffer, return bytes successfully written
        BEHTTP_EXPORT virtual size_t Write (const char* buffer, size_t bufferSize) = 0;

        // Read maximum bytes of bufferSize to bufferOut
        BEHTTP_EXPORT virtual size_t Read (char* bufferOut, size_t bufferSize) = 0;

        // Get length of contents
        BEHTTP_EXPORT virtual uint64_t GetLength () = 0;

        // Convenience methods to read content
        BEHTTP_EXPORT virtual Json::Value AsJson () const = 0;
        BEHTTP_EXPORT virtual void AsRapidJson (rapidjson::Document& documentOut) const = 0;
        BEHTTP_EXPORT virtual Utf8String AsString () const = 0;
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
typedef RefCountedPtr<struct HttpFileBody> HttpFileBodyPtr;

struct EXPORT_VTABLE_ATTRIBUTE HttpFileBody : public HttpBody
    {
    private:
        mutable BeFile  m_file;
        BeFileName      m_filePath;
        bool            m_fileCreated;

    private:
        BentleyStatus CreateFile ();

        HttpFileBody (BeFileNameCR filePath);
        virtual ~HttpFileBody ();

    public:
        BEHTTP_EXPORT static HttpFileBodyPtr Create (BeFileNameCR filePath);
        BEHTTP_EXPORT BeFileNameCR GetFilePath () const;

        BEHTTP_EXPORT void Open ();
        BEHTTP_EXPORT void Close ();

        BEHTTP_EXPORT BentleyStatus SetPosition (uint64_t position);
        BEHTTP_EXPORT BentleyStatus GetPosition (uint64_t& position);
        BEHTTP_EXPORT BentleyStatus Reset ();
        BEHTTP_EXPORT size_t Write (const char* buffer, size_t bufferSize);
        BEHTTP_EXPORT size_t Read (char* bufferOut, size_t bufferSize);
        BEHTTP_EXPORT uint64_t GetLength ();

        BEHTTP_EXPORT Json::Value AsJson () const;
        BEHTTP_EXPORT void AsRapidJson (rapidjson::Document& documentOut) const;
        BEHTTP_EXPORT Utf8String AsString () const;
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
typedef RefCountedPtr<struct HttpStringBody> HttpStringBodyPtr;

struct EXPORT_VTABLE_ATTRIBUTE HttpStringBody : public HttpBody
    {
    protected:
        uint64_t m_position;
        std::shared_ptr<Utf8String> m_string;

        HttpStringBody (std::shared_ptr<Utf8String> content);
        virtual ~HttpStringBody ();

    public:
        BEHTTP_EXPORT static HttpStringBodyPtr Create (Utf8StringCR content = "");
        BEHTTP_EXPORT static HttpStringBodyPtr Create (std::shared_ptr<Utf8String> content);

        BEHTTP_EXPORT void Open ();
        BEHTTP_EXPORT void Close ();

        BEHTTP_EXPORT BentleyStatus SetPosition (uint64_t position);
        BEHTTP_EXPORT BentleyStatus GetPosition (uint64_t& position);
        BEHTTP_EXPORT BentleyStatus Reset ();
        BEHTTP_EXPORT size_t Write (const char* buffer, size_t bufferSize);
        BEHTTP_EXPORT size_t Read (char* bufferOut, size_t bufferSize);
        BEHTTP_EXPORT uint64_t GetLength ();

        BEHTTP_EXPORT Json::Value AsJson () const;
        BEHTTP_EXPORT void AsRapidJson (rapidjson::Document& documentOut) const;
        BEHTTP_EXPORT Utf8String AsString () const;
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                     Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
typedef RefCountedPtr<struct HttpByteStreamBody> HttpByteStreamBodyPtr;

struct EXPORT_VTABLE_ATTRIBUTE HttpByteStreamBody : public HttpBody
    {
    protected:
        uint64_t m_position;
        ByteStream m_stream;

        HttpByteStreamBody() {}

    public:
        BEHTTP_EXPORT static HttpByteStreamBodyPtr Create();

        ByteStream const& GetByteStream() const {return m_stream;}

        BEHTTP_EXPORT void Open() override;
        BEHTTP_EXPORT void Close() override;

        BEHTTP_EXPORT BentleyStatus SetPosition(uint64_t position) override;
        BEHTTP_EXPORT BentleyStatus GetPosition(uint64_t& position) override;
        BEHTTP_EXPORT BentleyStatus Reset() override;
        BEHTTP_EXPORT size_t Write(const char* buffer, size_t bufferSize) override;
        BEHTTP_EXPORT size_t Read(char* bufferOut, size_t bufferSize) override;
        BEHTTP_EXPORT uint64_t GetLength() override;

        BEHTTP_EXPORT Json::Value AsJson() const override;
        BEHTTP_EXPORT void AsRapidJson(rapidjson::Document& documentOut) const override;
        BEHTTP_EXPORT Utf8String AsString() const override;
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
typedef RefCountedPtr<struct HttpMultipartBody> HttpMultipartBodyPtr;

typedef bmap<Utf8String, Utf8String> HttpMultipartProperties;
typedef HttpMultipartProperties& HttpMultipartPropertiesR;
typedef const HttpMultipartProperties& HttpMultipartPropertiesCR;

// Multipart body that can only be used as request body.
struct EXPORT_VTABLE_ATTRIBUTE HttpMultipartBody : public HttpBody
    {
    protected:
        struct HttpMPart
            {
            Utf8String header;
            HttpBodyPtr content;
            uint64_t length;
            };

        uint64_t m_position;
        uint64_t m_totalPartsLength;
        Utf8String m_boundary;
        Utf8String m_boundaryFinish;
        bvector<HttpMPart> m_parts;

        HttpMultipartBody (Utf8StringCR boundary);
        virtual ~HttpMultipartBody ();

    public:
        static const Utf8String DefaultBoundary;

        BEHTTP_EXPORT static HttpMultipartBodyPtr Create ();
        BEHTTP_EXPORT static HttpMultipartBodyPtr Create (Utf8StringCR customBoundary);

        // Add multipart content part with specified contentType and properties
        BEHTTP_EXPORT void AddPart (HttpBodyPtr bodyPart, Utf8StringCR contentType);
        BEHTTP_EXPORT void AddPart (HttpBodyPtr bodyPart, Utf8StringCR contentType, HttpMultipartPropertiesCR properties);

        // Get correct content type value for this content
        BEHTTP_EXPORT Utf8String GetContentType ();

        BEHTTP_EXPORT BeFileNameCR GetFilePath () const;

        BEHTTP_EXPORT void Open ();
        BEHTTP_EXPORT void Close ();

        BEHTTP_EXPORT BentleyStatus SetPosition (uint64_t position);
        BEHTTP_EXPORT BentleyStatus GetPosition (uint64_t& position);
        BEHTTP_EXPORT size_t Read (char* bufferOut, size_t bufferSize);
        BEHTTP_EXPORT uint64_t GetLength ();

        // Read only - will only reset position
        BEHTTP_EXPORT BentleyStatus Reset ();

        // Read only - disabled
        BEHTTP_EXPORT size_t Write (const char* buffer, size_t bufferSize);
        BEHTTP_EXPORT Json::Value AsJson () const;
        BEHTTP_EXPORT void AsRapidJson (rapidjson::Document& documentOut) const;
        BEHTTP_EXPORT Utf8String AsString () const;
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
typedef RefCountedPtr<struct HttpRangeBody> HttpRangeBodyPtr;

struct EXPORT_VTABLE_ATTRIBUTE HttpRangeBody : public HttpBody
    {
    private:
        size_t TrimBufferSize (size_t bufferSize);
        Utf8String ReadAllAsString ();

    protected:
        HttpBodyPtr m_body;
        uint64_t m_bytesFrom;
        uint64_t m_bytesTo;

        HttpRangeBody (HttpBodyPtr body, uint64_t bytesFrom, uint64_t bytesTo);
        virtual ~HttpRangeBody ();

    public:
        BEHTTP_EXPORT static HttpRangeBodyPtr Create (HttpBodyPtr body, uint64_t bytesFrom, uint64_t bytesTo);

        BEHTTP_EXPORT void Open ();
        BEHTTP_EXPORT void Close ();

        BEHTTP_EXPORT BentleyStatus SetPosition (uint64_t position);
        BEHTTP_EXPORT BentleyStatus GetPosition (uint64_t& position);
        BEHTTP_EXPORT BentleyStatus Reset ();
        BEHTTP_EXPORT size_t Write (const char* buffer, size_t bufferSize);
        BEHTTP_EXPORT size_t Read (char* bufferOut, size_t bufferSize);
        BEHTTP_EXPORT uint64_t GetLength ();

        BEHTTP_EXPORT Json::Value AsJson () const;
        BEHTTP_EXPORT void AsRapidJson (rapidjson::Document& documentOut) const;
        BEHTTP_EXPORT Utf8String AsString () const;
    };

END_BENTLEY_HTTP_NAMESPACE
