/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <BeHttp/HttpBody.h>

#include <zlib/zlib.h>
#include <zlib/zip/zip.h>
#include <zlib/zip/unzip.h>

#define HttpCompressedBody_BufferSize 16384

USING_NAMESPACE_BENTLEY_HTTP

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpFileBody::Open()
    {
    OpenFile();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus HttpFileBody::OpenFile(BeFileAccess access)
    {
    if (m_file.IsOpen ())
        return SUCCESS;

    if (!m_filePath.DoesPathExist())
        {
        return CreateFile();
        }
        
    BeFileStatus status = m_file.Open(m_filePath, access);
    if (BeFileStatus::Success != status)
        {
        BeAssert(false);
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpFileBody::Close()
    {
    if (m_file.IsOpen())
        m_file.Close();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus HttpFileBody::CreateFile()
    {
    if (m_filePath.DoesPathExist ())
        return SUCCESS;

    BeFileStatus status = m_file.Create (m_filePath);
    if (BeFileStatus::Success != status)
        {
        BeAssert (false);
        return ERROR;
        }
     
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus HttpFileBody::PrepareForWrite()
    {
    if (BeFileAccess::ReadWrite == m_file.GetAccess())
        return SUCCESS;

    uint64_t position = 0;
    if (m_file.IsOpen() && BeFileStatus::Success != m_file.GetPointer(position))
        return ERROR;

    Close();
    if (SUCCESS != OpenFile(BeFileAccess::ReadWrite))
        return ERROR;

    if (BeFileStatus::Success != m_file.SetPointer(position, BeFileSeekOrigin::Begin))
        return ERROR;

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus HttpFileBody::SetPosition(uint64_t position)
    {
    if (SUCCESS != OpenFile())
        {
        return ERROR;
        }

    auto status = m_file.SetPointer(position, BeFileSeekOrigin::Begin);
    if (BeFileStatus::Success != status)
        {
        return ERROR;
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus HttpFileBody::GetPosition(uint64_t& position)
    {
    if (SUCCESS != OpenFile())
        {
        return ERROR;
        }

    auto status = m_file.GetPointer(position);
    if (BeFileStatus::Success != status)
        {
        return ERROR;
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus HttpFileBody::Reset()
    {
    m_file.Close ();

    BeFileNameStatus status = m_filePath.BeDeleteFile ();
    if (BeFileNameStatus::Success != status &&
        BeFileNameStatus::FileNotFound != status)
        {
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
size_t HttpFileBody::Write(const char* buffer, size_t bufferSize)
    {
    if (SUCCESS != PrepareForWrite())
        {
        return 0;
        }
    unsigned bytesWritten = 0;
    BeFileStatus status = m_file.Write (&bytesWritten, buffer, (unsigned)bufferSize);
    if (BeFileStatus::Success != status)
        {
        return 0;
        }
    return bytesWritten;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
size_t HttpFileBody::Read(char* bufferOut, size_t bufferSize)
    {
    unsigned bytesRead = 0;
    BeFileStatus status = m_file.Read(bufferOut, &bytesRead, (unsigned)bufferSize);
    if (BeFileStatus::Success != status)
        {
        return 0;
        }
    return bytesRead;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t HttpFileBody::GetLength()
    {
    bool wasOpen = m_file.IsOpen ();
    if (!wasOpen)
        Open ();

    uint64_t length;
    auto status = m_file.GetSize(length);
    if (BeFileStatus::Success != status)
        return 0;

    if (!wasOpen)
        Close ();

    return length;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String HttpFileBody::AsString() const
    {
    bvector<Byte> fileContents;

    m_file.Open(m_filePath, BeFileAccess::Read);
    m_file.ReadEntireFile(fileContents);
    m_file.Close();

    Utf8String stringContents;
    stringContents.append(fileContents.begin(), fileContents.end());
    return stringContents;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HttpStringBody::HttpStringBody(std::shared_ptr<Utf8String> content) :
m_string(content ? content : std::make_shared<Utf8String>()),
m_position(0)
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HttpStringBodyPtr HttpStringBody::Create(Utf8StringCR content)
    {
    return new HttpStringBody(std::make_shared<Utf8String>(content));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HttpStringBodyPtr HttpStringBody::Create(std::shared_ptr<Utf8String> content)
    {
    return new HttpStringBody(content);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus HttpStringBody::SetPosition(uint64_t position)
    {
    uint64_t currentLength = GetLength();
    if (position > currentLength)
        return ERROR;

    m_position = position;
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
size_t HttpStringBody::Write(const char* buffer, size_t bufferSize)
    {
    uint64_t currentLength = GetLength();

    if (currentLength < m_position + bufferSize)
        {
        m_string->resize(static_cast<Utf8String::size_type>(m_position + bufferSize));
        }

    m_string->replace(static_cast<Utf8String::size_type>(m_position), bufferSize, buffer, bufferSize);
    m_position += bufferSize;

    return bufferSize;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
size_t HttpStringBody::Read(char* bufferOut, size_t bufferSize)
    {
    uint64_t bytesCopied = m_string->copy(bufferOut, bufferSize, static_cast<Utf8String::size_type>(m_position));
    m_position += bytesCopied;
    return static_cast<size_t>(bytesCopied);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                julius.cepukenas    11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
HttpBinaryBody::HttpBinaryBody(std::shared_ptr<bvector<char>> data) :
m_data(data)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                julius.cepukenas    11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
HttpBinaryBodyPtr HttpBinaryBody::Create(std::shared_ptr<bvector<char>> data)
    {
    return new HttpBinaryBody(data);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    julius.cepukenas 11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus HttpBinaryBody::SetPosition(uint64_t position)
    {
    uint64_t currentLength = GetLength();
    if (position > currentLength)
        return ERROR;

    m_position = position;
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    julius.cepukenas 11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
size_t HttpBinaryBody::Write(const char* buffer, size_t bufferSize)
    {
    if (bufferSize == 0)
        return 0;

    uint64_t currentLength = GetLength();

    if (currentLength < m_position + bufferSize)
        {
        if (std::numeric_limits<size_t>::max() < m_position + bufferSize)
            {
            BeAssert(false && "Body limit reached");
            return 0;
            }

        m_data->resize(static_cast<size_t>(m_position + bufferSize), '\0');
        }

    memcpy(m_data->data() + m_position, buffer, bufferSize);
    m_position += bufferSize;

    return bufferSize;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    julius.cepukenas 11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
size_t HttpBinaryBody::Read(char* bufferOut, size_t bufferSize)
    {
    size_t copyBytesCount = bufferSize;

    if (GetLength() - m_position < copyBytesCount)
        copyBytesCount = (size_t)(GetLength() - m_position);

    if (0 == copyBytesCount)
        return 0;

    memcpy(bufferOut, m_data->data() + m_position, copyBytesCount);
    m_position += copyBytesCount;
    return copyBytesCount;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    julius.cepukenas 11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t HttpBinaryBody::GetLength()
    {
    return m_data->size();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    julius.cepukenas 11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus HttpBinaryBody::Reset()
    {
    m_data->clear();
    return SetPosition(0);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus HttpByteStreamBody::SetPosition(uint64_t position)
    {
    uint64_t currentLength = GetLength();
    if (position > currentLength)
        return ERROR;

    m_position = position;
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
size_t HttpByteStreamBody::Read(char* bufferOut, size_t bufferSize)
    {
    size_t copyBytesCount = bufferSize;
    if (m_stream.GetSize() < copyBytesCount)
        copyBytesCount = (size_t)m_stream.GetSize();

    memcpy(bufferOut, m_stream.GetData(), copyBytesCount);
    return copyBytesCount;
    }

Utf8String HttpMultipartBody::DefaultBoundary() {return "----------------c74c9f339ca44dd48ee4b45a9dedd811";}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HttpMultipartBody::HttpMultipartBody(Utf8StringCR boundary) :
m_totalPartsLength(0),
m_position(0),
m_boundary(boundary),
m_boundaryFinish("\r\n--" + boundary + "--\r\n")
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String HttpMultipartBody::GetContentType()
    {
    return Utf8PrintfString("multipart/form-data; boundary=%s", m_boundary.c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String GetPropertyStringList(HttpMultipartPropertiesCR properties)
    {
    Utf8String list;

    Utf8String temp;
    for (auto iter : properties)
        {
        temp.Sprintf("; %s=\"%s\"", iter.first.c_str(), iter.second.c_str());
        list.append(temp);
        }

    return list;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpMultipartBody::AddPart(HttpBodyPtr bodyPart, Utf8StringCR contentType, HttpMultipartPropertiesCR properties)
    {
    Utf8String headerStart = m_parts.size() == 0 ? "--" : "\r\n--";

    m_parts.push_back(HttpMPart());
    HttpMPart& multipartPart = m_parts.back();

    Utf8String propertyList = GetPropertyStringList(properties);

    multipartPart.content = bodyPart;
    multipartPart.header.Sprintf(
            "%s%s\r\n"
            "Content-Disposition: form-data%s\r\n"
            "Content-Type: %s\r\n"
            "\r\n",
            headerStart.c_str(), m_boundary.c_str(),
            propertyList.c_str(),
            contentType.c_str());

    multipartPart.length = multipartPart.content->GetLength() + multipartPart.header.length();

    m_totalPartsLength += multipartPart.length;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpMultipartBody::Open()
    {
    for (HttpMPart& part : m_parts)
        part.content->Open();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpMultipartBody::Close()
    {
    for (HttpMPart& part : m_parts)
        part.content->Close();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus HttpMultipartBody::SetPosition(uint64_t position)
    {
    uint64_t currentLength = GetLength();
    if (position > currentLength)
        {
        return ERROR;
        }
    m_position = position;
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
size_t HttpMultipartBody::Read(char* bufferOut, size_t bufferSize)
    {
    if (m_position >= GetLength())
        {
        return 0;
        }

    HttpMPart* foundPart = nullptr;
    uint64_t positionInPart = 0;

    uint64_t previousPartsLength = 0;
    for (HttpMPart& part : m_parts)
        {
        uint64_t totalCurrentLength = previousPartsLength + part.length;

        if (m_position < totalCurrentLength)
            {
            positionInPart = m_position + part.length - totalCurrentLength;
            foundPart = &part;
            break;
            }

        previousPartsLength = totalCurrentLength;
        }

    size_t bytesRead = 0;
    if (nullptr != foundPart)
        {
        size_t partHeaderLength = foundPart->header.length();
        if (positionInPart < partHeaderLength)
            {
            bytesRead = foundPart->header.copy(bufferOut, bufferSize, static_cast<Utf8String::size_type>(positionInPart));
            }
        else
            {
            foundPart->content->SetPosition(positionInPart - partHeaderLength);
            bytesRead = foundPart->content->Read(bufferOut, bufferSize);
            }
        }
    else
        {
        Utf8String::size_type stringPosition = static_cast<Utf8String::size_type>(m_position - m_totalPartsLength);
        bytesRead = m_boundaryFinish.copy(bufferOut, bufferSize, stringPosition);
        }

    m_position += bytesRead;
    return bytesRead;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t HttpMultipartBody::GetLength()
    {
    if (m_parts.size() == 0)
        {
        return 0;
        }
    return m_totalPartsLength + m_boundaryFinish.size();
    }

#define IsInstanceOf(instance, typeof) nullptr != dynamic_cast<typeof*>(instance)

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String HttpMultipartBody::AsString() const
    {
    Utf8String result;
    for (const HttpMPart& part : m_parts)
        {
        result += part.header;

        if (IsInstanceOf(part.content.get(), HttpFileBody))
            {
            result += Utf8PrintfString("<< file %llu bytes >>", part.content->GetLength());
            }
        else
            {
            result += part.content->AsString();
            }
        }
    result += m_boundaryFinish;
    return result;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HttpRangeBody::HttpRangeBody(HttpBodyPtr body, uint64_t bytesFrom, uint64_t bytesTo) :
m_body(body),
m_bytesFrom(bytesFrom),
m_bytesTo(bytesTo)
    {
    BeAssert(bytesFrom <= bytesTo);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus HttpRangeBody::GetPosition(uint64_t& position)
    {
    uint64_t bodyPosition;
    if (SUCCESS != m_body->GetPosition(bodyPosition))
        {
        return ERROR;
        }
    position = bodyPosition - m_bytesFrom;
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus HttpRangeBody::Reset()
    {
    if (SUCCESS != SetPosition(0))
        {
        return ERROR;
        }

    char buffer[1] = {0};

    for (uint64_t i = 0; i < GetLength(); i++)
        {
        if (1 != Write(buffer, 1))
            {
            return ERROR;
            }
        }

    if (SUCCESS != SetPosition(0))
        {
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
size_t HttpRangeBody::Write(const char* buffer, size_t bufferSize)
    {
    bufferSize = TrimBufferSize(bufferSize);
    if (0 == bufferSize)
        {
        return 0;
        }

    return m_body->Write(buffer, bufferSize);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
size_t HttpRangeBody::Read(char* bufferOut, size_t bufferSize)
    {
    bufferSize = TrimBufferSize(bufferSize);
    if (0 == bufferSize)
        {
        return 0;
        }

    return m_body->Read(bufferOut, bufferSize);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
size_t HttpRangeBody::TrimBufferSize(size_t bufferSize)
    {
    uint64_t length = GetLength();
    uint64_t position;
    if (SUCCESS != GetPosition(position))
        {
        return 0;
        }

    if (position >= length)
        {
        return 0;
        }
    if (position + bufferSize > length)
        {
        bufferSize = static_cast<size_t>(length - position);
        }
    return bufferSize;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String HttpRangeBody::AsString() const
    {
    return const_cast<HttpRangeBody*>(this)->ReadAllAsString();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String HttpRangeBody::ReadAllAsString()
    {
    uint64_t initialPosition;
    if (SUCCESS != GetPosition(initialPosition))
        return nullptr;

    if (SUCCESS != SetPosition(0))
        return nullptr;

    Utf8String str;

    size_t bufferSize = 256;
    char* buffer = new char[bufferSize];

    size_t bytesRead;
    while (bytesRead = Read(buffer, bufferSize))
        {
        str.append(buffer, bytesRead);
        }

    delete[] buffer;

    if (SUCCESS != SetPosition(initialPosition))
        return nullptr;

    return str;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                julius.cepukenas    11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
HttpCompressedBody::HttpCompressedBody(HttpBodyPtr httpBody) :
m_data(std::make_shared<bvector<char>>()),
m_dataBody(HttpBinaryBody::Create(m_data)),
m_contentBody(httpBody)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                julius.cepukenas    11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpCompressedBody::Open()
    {
    if (!m_data->empty())
        return;

    if (m_contentBody == nullptr)
        return;

    m_contentBody->Open();
    if (SUCCESS != m_contentBody->SetPosition(0))
        {
        BeAssert(false);
        return;
        }

    if (SUCCESS != Compress())
        {
        BeAssert(false);
        return;
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                julius.cepukenas    11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpCompressedBody::Close()
    {
    if (m_contentBody != nullptr)
        m_contentBody->Close();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                julius.cepukenas    11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus HttpCompressedBody::Compress()
    {
    size_t dataLenght = 0;

    //initialise z_stream
    z_stream strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;

    int result;
    //create GZip stream    
    result = deflateInit2(&strm, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 15 | 16, 8, Z_DEFAULT_STRATEGY);
    if (Z_OK != result)
        return ERROR;

    unsigned char out[HttpCompressedBody_BufferSize];
    unsigned char in[HttpCompressedBody_BufferSize];

    int flush = Z_NO_FLUSH;

    while (Z_FINISH != flush)
        {
        strm.avail_in = static_cast<uInt>(m_contentBody->Read(reinterpret_cast<char*>(in), HttpCompressedBody_BufferSize));

        uint64_t position;
        if (SUCCESS != m_contentBody->GetPosition(position))
            break;

        flush = position >= m_contentBody->GetLength() ? Z_FINISH : Z_NO_FLUSH;

        strm.next_in = in;
        strm.avail_out = 0;

        while (0 == strm.avail_out)
            {
            strm.avail_out = HttpCompressedBody_BufferSize;
            strm.next_out = out;
            result = deflate(&strm, flush);
            if(Z_STREAM_ERROR == result)
                break;

            int have = HttpCompressedBody_BufferSize - strm.avail_out;
            dataLenght += have;

            m_data->resize(dataLenght);
            memcpy(m_data->data() + (dataLenght - have), out, have);
            }
        }

    if (Z_STREAM_END != result)
        {
        deflateEnd(&strm);
        return ERROR;
        }

    result = deflateEnd(&strm);
    if (Z_OK != result)
        return ERROR;

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                julius.cepukenas    11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
HttpCompressedBodyPtr HttpCompressedBody::Create(HttpBodyPtr content)
    {
    return new HttpCompressedBody(content);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    julius.cepukenas 11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus HttpCompressedBody::SetPosition(uint64_t position)
    {
    return m_dataBody->SetPosition(position);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    julius.cepukenas 11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus HttpCompressedBody::GetPosition(uint64_t& position)
    {
    return m_dataBody->GetPosition(position);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    julius.cepukenas 11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
size_t HttpCompressedBody::Write(const char* buffer, size_t bufferSize)
    {
    BeAssert(false && "Not supported");
    return 0;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    julius.cepukenas 11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
size_t HttpCompressedBody::Read(char* bufferOut, size_t bufferSize)
    {
    return m_dataBody->Read(bufferOut, bufferSize);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    julius.cepukenas 11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus HttpCompressedBody::Reset()
    {
    return m_dataBody->Reset();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    julius.cepukenas 11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t HttpCompressedBody::GetLength()
    { 
    return m_dataBody->GetLength();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                 julius.cepukenas 11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String HttpCompressedBody::AsString() const
    {
    BeAssert(false && "Not supported");
    return "";
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                julius.cepukenas    11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
HttpDummyBody::HttpDummyBody()
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                julius.cepukenas    11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpDummyBody::Open()
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                julius.cepukenas    11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpDummyBody::Close()
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                julius.cepukenas    11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
HttpDummyBodyPtr HttpDummyBody::Create()
    {
    return new HttpDummyBody();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    julius.cepukenas 11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus HttpDummyBody::SetPosition(uint64_t position)
    {
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    julius.cepukenas 11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus HttpDummyBody::GetPosition(uint64_t& position)
    {
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    julius.cepukenas 11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
size_t HttpDummyBody::Write(const char* buffer, size_t bufferSize)
    {
    return bufferSize;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    julius.cepukenas 11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
size_t HttpDummyBody::Read(char* bufferOut, size_t bufferSize)
    {
    return 0;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    julius.cepukenas 11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus HttpDummyBody::Reset()
    {
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    julius.cepukenas 11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t HttpDummyBody::GetLength()
    {
    return 0;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                 julius.cepukenas 11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String HttpDummyBody::AsString() const
    {
    BeAssert(false && "Not supported");
    return "";
    }
