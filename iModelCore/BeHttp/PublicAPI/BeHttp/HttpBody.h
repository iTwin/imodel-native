/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/BeHttp/HttpBody.h $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/BeFile.h>
#include <Bentley/RefCounted.h>
#include <Bentley/ByteStream.h>
#include <Bentley/bmap.h>
#include <BeHttp/Http.h>

BEGIN_BENTLEY_HTTP_NAMESPACE

DEFINE_POINTER_SUFFIX_TYPEDEFS(HttpBody)
DEFINE_REF_COUNTED_PTR(HttpBody)

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct HttpBody : RefCountedBase
{
public:
    // Open for Read/Write
    virtual void Open() = 0;

    // Close and release resources
    virtual void Close() = 0;

    // Set Read/Write position
    virtual BentleyStatus SetPosition(uint64_t position) = 0;

    // Get current position
    virtual BentleyStatus GetPosition(uint64_t& position) = 0;

    // Reset position and clear body contents
    virtual BentleyStatus Reset() = 0;

    // Write contents of buffer, return bytes successfully written
    virtual size_t Write(const char* buffer, size_t bufferSize) = 0;

    // Read maximum bytes of bufferSize to bufferOut
    virtual size_t Read(char* bufferOut, size_t bufferSize) = 0;

    // Get length of contents
    virtual uint64_t GetLength() = 0;

    virtual Utf8String AsString() const = 0;
};

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
typedef RefCountedPtr<struct HttpFileBody> HttpFileBodyPtr;

struct EXPORT_VTABLE_ATTRIBUTE HttpFileBody : HttpBody
{
private:
    mutable BeFile m_file;
    BeFileName m_filePath;
    bool m_fileCreated = false;

private:
    BentleyStatus CreateFile();

    HttpFileBody(BeFileNameCR filePath) : m_filePath (filePath) {}
    virtual ~HttpFileBody() {Close ();}

public:
    static HttpFileBodyPtr Create(BeFileNameCR filePath) {return new HttpFileBody(filePath);}
    BeFileNameCR GetFilePath() const {return m_filePath;}

    BEHTTP_EXPORT void Open();
    BEHTTP_EXPORT void Close();

    BEHTTP_EXPORT BentleyStatus SetPosition(uint64_t position);
    BEHTTP_EXPORT BentleyStatus GetPosition(uint64_t& position);
    BEHTTP_EXPORT BentleyStatus Reset();
    BEHTTP_EXPORT size_t Write(const char* buffer, size_t bufferSize);
    BEHTTP_EXPORT size_t Read(char* bufferOut, size_t bufferSize);
    BEHTTP_EXPORT uint64_t GetLength();
    BEHTTP_EXPORT Utf8String AsString() const;
};

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
typedef RefCountedPtr<struct HttpStringBody> HttpStringBodyPtr;

struct EXPORT_VTABLE_ATTRIBUTE HttpStringBody : HttpBody
{
protected:
    uint64_t m_position;
    std::shared_ptr<Utf8String> m_string;

    HttpStringBody(std::shared_ptr<Utf8String> content);
    virtual ~HttpStringBody() {}

public:
    BEHTTP_EXPORT static HttpStringBodyPtr Create(Utf8StringCR content = "");
    BEHTTP_EXPORT static HttpStringBodyPtr Create(std::shared_ptr<Utf8String> content);

    void Open() override {}
    void Close() override {}

    BEHTTP_EXPORT BentleyStatus SetPosition(uint64_t position);
    BentleyStatus GetPosition(uint64_t& position) {position = m_position; return SUCCESS;}

    BentleyStatus Reset() {m_string->clear(); m_position = 0; return SUCCESS;}
    BEHTTP_EXPORT size_t Write(const char* buffer, size_t bufferSize);
    BEHTTP_EXPORT size_t Read(char* bufferOut, size_t bufferSize);
    uint64_t GetLength() {return m_string->length();}

    Utf8String AsString() const {return *m_string;}
};

typedef RefCountedPtr<struct HttpByteStreamBody> HttpByteStreamBodyPtr;

/*--------------------------------------------------------------------------------------+
* @bsiclass                                     Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE HttpByteStreamBody : public HttpBody
{
protected:
    uint64_t m_position;
    ByteStream m_stream;

public:
    HttpByteStreamBody() {}
    static HttpByteStreamBodyPtr Create() {return new HttpByteStreamBody();}

    void Open() override {}
    void Close() override {}

    BEHTTP_EXPORT BentleyStatus SetPosition(uint64_t position) override;
    BentleyStatus GetPosition(uint64_t& position) override {position = m_position; return SUCCESS;}
    BentleyStatus Reset() override {m_stream.Clear(); m_position = 0; return SUCCESS;}
    size_t Write(const char* buffer, size_t bufferSize) override {m_stream.Append((uint8_t const*)buffer, (uint32_t)bufferSize); m_position += bufferSize; return bufferSize;}
    BEHTTP_EXPORT size_t Read(char* bufferOut, size_t bufferSize) override;
    uint64_t GetLength() override {return m_stream.GetSize();}
    ByteStream const& GetByteStream() const {return m_stream;}
    Utf8String AsString() const override {return "";}
};

typedef bmap<Utf8String, Utf8String> HttpMultipartProperties;
typedef HttpMultipartProperties& HttpMultipartPropertiesR;
typedef HttpMultipartProperties const& HttpMultipartPropertiesCR;
typedef RefCountedPtr<struct HttpMultipartBody> HttpMultipartBodyPtr;

/*--------------------------------------------------------------------------------------+
 Multipart body that can only be used as request body.
* @bsiclass                                                     Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
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

    BEHTTP_EXPORT HttpMultipartBody(Utf8StringCR boundary);
    virtual ~HttpMultipartBody() {}

public:
    BEHTTP_EXPORT static Utf8String DefaultBoundary();

    static HttpMultipartBodyPtr Create() {return new HttpMultipartBody(DefaultBoundary());}
    static HttpMultipartBodyPtr Create(Utf8StringCR customBoundary) {return new HttpMultipartBody(DefaultBoundary());}

    // Add multipart content part with specified contentType and properties
    void AddPart(HttpBodyPtr bodyPart, Utf8StringCR contentType) {AddPart(bodyPart, contentType, HttpMultipartProperties());}
    BEHTTP_EXPORT void AddPart(HttpBodyPtr bodyPart, Utf8StringCR contentType, HttpMultipartPropertiesCR properties);

    // Get correct content type value for this content
    BEHTTP_EXPORT Utf8String GetContentType();

    BEHTTP_EXPORT BeFileNameCR GetFilePath() const;

    BEHTTP_EXPORT void Open() override;
    BEHTTP_EXPORT void Close() override;

    BEHTTP_EXPORT BentleyStatus SetPosition(uint64_t position) override;
    BentleyStatus GetPosition(uint64_t& position) override {position = m_position; return SUCCESS;}
    BEHTTP_EXPORT size_t Read(char* bufferOut, size_t bufferSize) override;
    BEHTTP_EXPORT uint64_t GetLength();

    // Read only - will only reset position
    BentleyStatus Reset() override{return SetPosition(0);}

    // Read only - disabled
    size_t Write(const char* buffer, size_t bufferSize) {BeAssert(false && "Write is not supported"); return 0;}
    BEHTTP_EXPORT Utf8String AsString() const;
};

typedef RefCountedPtr<struct HttpRangeBody> HttpRangeBodyPtr;

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE HttpRangeBody : public HttpBody
{
private:
    size_t TrimBufferSize(size_t bufferSize);
    Utf8String ReadAllAsString();

protected:
    HttpBodyPtr m_body;
    uint64_t m_bytesFrom;
    uint64_t m_bytesTo;

    BEHTTP_EXPORT HttpRangeBody(HttpBodyPtr body, uint64_t bytesFrom, uint64_t bytesTo);
    virtual ~HttpRangeBody() {}

public:
    static HttpRangeBodyPtr Create(HttpBodyPtr body, uint64_t bytesFrom, uint64_t bytesTo) {return new HttpRangeBody(body, bytesFrom, bytesTo);}

    void Open() {m_body->Open(); m_body->SetPosition(m_bytesFrom);}
    void Close() {m_body->Close();}

    BentleyStatus SetPosition(uint64_t position) {return m_body->SetPosition(m_bytesFrom + position);}
    BEHTTP_EXPORT BentleyStatus GetPosition(uint64_t& position);
    BEHTTP_EXPORT BentleyStatus Reset();
    BEHTTP_EXPORT size_t Write(const char* buffer, size_t bufferSize);
    BEHTTP_EXPORT size_t Read(char* bufferOut, size_t bufferSize);
    uint64_t GetLength() {return m_bytesTo - m_bytesFrom + 1;}

    BEHTTP_EXPORT Utf8String AsString() const;
};

END_BENTLEY_HTTP_NAMESPACE
