/*--------------------------------------------------------------------------------------+
 |
 |     $Source: BeHttp/HttpBody.cpp $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#include <BeHttp/HttpBody.h>

#pragma mark - HttpBody

USING_NAMESPACE_BENTLEY_HTTP

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HttpBody::HttpBody ()
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HttpBody::~HttpBody ()
    {
    }

#pragma mark - HttpFileBody

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HttpFileBody::HttpFileBody (BeFileNameCR filePath) :
m_filePath (filePath), 
m_fileCreated (false)
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HttpFileBody::~HttpFileBody ()
    {
    Close ();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HttpFileBodyPtr HttpFileBody::Create (BeFileNameCR filePath)
    {
    return new HttpFileBody (filePath);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Jahan.Zeb    07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileNameCR HttpFileBody::GetFilePath () const
    {
    return m_filePath;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpFileBody::Open ()
    {
    if (!m_file.IsOpen ())
        {
        m_file.Open (m_filePath, BeFileAccess::ReadWrite);
        CreateFile ();
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpFileBody::Close ()
    {
    if (m_file.IsOpen ())
        {
        m_file.Close ();
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus HttpFileBody::CreateFile ()
    {
    if (!m_fileCreated)
        {
        if (!m_filePath.DoesPathExist ())
            {
            BeFileStatus status = m_file.Create (m_filePath);
            if (BeFileStatus::Success != status)
                {
                BeAssert (false);
                return ERROR;
                }
            }
        m_fileCreated = true;
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus HttpFileBody::SetPosition (uint64_t position)
    {
    Open();
    if (BeFileStatus::Success != m_file.SetPointer (position, BeFileSeekOrigin::Begin))
        {
        return ERROR;
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus HttpFileBody::GetPosition (uint64_t& position)
    {
    Open();
    if (BeFileStatus::Success != m_file.GetPointer (position))
        {
        return ERROR;
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus HttpFileBody::Reset ()
    {
    m_file.Close ();

    BeFileNameStatus status = m_filePath.BeDeleteFile ();

    if (BeFileNameStatus::Success != status &&
        BeFileNameStatus::FileNotFound != status)
        {
        return ERROR;
        }

    m_fileCreated = false;

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
size_t HttpFileBody::Write (const char* buffer, size_t bufferSize)
    {
    Open();
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
size_t HttpFileBody::Read (char* bufferOut, size_t bufferSize)
    {
    Open();
    unsigned bytesRead = 0;
    BeFileStatus status = m_file.Read (bufferOut, &bytesRead, (unsigned)bufferSize);
    if (BeFileStatus::Success != status)
        {
        return 0;
        }
    return bytesRead;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t HttpFileBody::GetLength ()
    {
    bool wasOpen = m_file.IsOpen ();
    if (!wasOpen)
        {
        Open ();
        }

    uint64_t length;
    if (BeFileStatus::Success != m_file.GetSize (length))
        {
        return 0;
        }

    if (!wasOpen)
        {
        Close ();
        }
    return length;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value HttpFileBody::AsJson () const
    {
    Json::Value json;
    if (!Json::Reader ().parse (AsString (), json, false))
        {
        json = Json::Value::null;
        }
    return json;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Benjamin.Brown  01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpFileBody::AsRapidJson (rapidjson::Document& document) const
    {
    unsigned status = !document.Parse<0> (AsString ().c_str ()).HasParseError ();
    BeAssert (status);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String HttpFileBody::AsString () const
    {
    bvector<Byte> fileContents;

    m_file.Open (m_filePath, BeFileAccess::Read);
    m_file.ReadEntireFile (fileContents);
    m_file.Close ();

    Utf8String stringContents;
    stringContents.append (fileContents.begin (), fileContents.end ());
    return stringContents;
    }

#pragma mark - HttpStringBody

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HttpStringBody::HttpStringBody (std::shared_ptr<Utf8String> content) :
m_string (content ? content : std::make_shared<Utf8String>()),
m_position (0)
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HttpStringBody::~HttpStringBody ()
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HttpStringBodyPtr HttpStringBody::Create (Utf8StringCR content)
    {
    return new HttpStringBody (std::make_shared<Utf8String>(content));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HttpStringBodyPtr HttpStringBody::Create (std::shared_ptr<Utf8String> content)
    {
    return new HttpStringBody (content);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpStringBody::Open ()
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpStringBody::Close ()
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus HttpStringBody::SetPosition (uint64_t position)
    {
    uint64_t currentLength = GetLength ();
    if (position > currentLength)
        {
        return ERROR;
        }
    m_position = position;
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus HttpStringBody::GetPosition (uint64_t& position)
    {
    position = m_position;
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus HttpStringBody::Reset ()
    {
    m_string->clear ();
    m_position = 0;
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
size_t HttpStringBody::Write (const char* buffer, size_t bufferSize)
    {
    uint64_t currentLength = GetLength ();

    if (currentLength < m_position + bufferSize)
        {
        m_string->resize (static_cast<Utf8String::size_type>(m_position + bufferSize));
        }

    m_string->replace (static_cast<Utf8String::size_type>(m_position), bufferSize, buffer, bufferSize);
    m_position += bufferSize;

    return bufferSize;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
size_t HttpStringBody::Read (char* bufferOut, size_t bufferSize)
    {
    uint64_t bytesCopied = m_string->copy (bufferOut, bufferSize, static_cast<Utf8String::size_type>(m_position));
    m_position += bytesCopied;
    return static_cast<size_t>(bytesCopied);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t HttpStringBody::GetLength ()
    {
    return m_string->length ();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value HttpStringBody::AsJson () const
    {
    Json::Value json;
    if (!Json::Reader ().parse (*m_string, json, false))
        {
        json = Json::Value::null;
        }
    return json;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Benjamin.Brown  01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpStringBody::AsRapidJson (rapidjson::Document& document) const
    {
    unsigned status = !document.Parse<0> (m_string->c_str ()).HasParseError ();
    BeAssert (status);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String HttpStringBody::AsString () const
    {
    return *m_string;
    }

#pragma mark - HttpByteStreamBody

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
HttpByteStreamBodyPtr HttpByteStreamBody::Create() {return new HttpByteStreamBody();}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpByteStreamBody::Open() {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpByteStreamBody::Close() {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus HttpByteStreamBody::SetPosition (uint64_t position)
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
BentleyStatus HttpByteStreamBody::GetPosition (uint64_t& position)
    {
    position = m_position;
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus HttpByteStreamBody::Reset ()
    {
    m_stream.Clear();
    m_position = 0;
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
size_t HttpByteStreamBody::Write (const char* buffer, size_t bufferSize)
    {
    m_stream.Append((uint8_t const*)buffer, (uint32_t)bufferSize);
    m_position += bufferSize;
    return bufferSize;
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

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t HttpByteStreamBody::GetLength() {return m_stream.GetSize();}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value HttpByteStreamBody::AsJson() const {return Json::Value::null;}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpByteStreamBody::AsRapidJson(rapidjson::Document& document) const {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Grigas.Petraitis                07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String HttpByteStreamBody::AsString() const {return "";}

#pragma mark - HttpMultipartBody

const Utf8String HttpMultipartBody::DefaultBoundary = "----------------c74c9f339ca44dd48ee4b45a9dedd811";

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HttpMultipartBody::HttpMultipartBody (Utf8StringCR boundary) :
m_totalPartsLength (0),
m_position (0),
m_boundary (boundary),
m_boundaryFinish ("\r\n--" + boundary + "--\r\n")
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HttpMultipartBody::~HttpMultipartBody ()
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HttpMultipartBodyPtr HttpMultipartBody::Create ()
    {
    return new HttpMultipartBody (HttpMultipartBody::DefaultBoundary);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HttpMultipartBodyPtr HttpMultipartBody::Create (Utf8StringCR customBoundary)
    {
    return new HttpMultipartBody (customBoundary);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String HttpMultipartBody::GetContentType ()
    {
    return Utf8PrintfString ("multipart/form-data; boundary=%s", m_boundary.c_str ());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String GetPropertyStringList (HttpMultipartPropertiesCR properties)
    {
    Utf8String list;

    Utf8String temp;
    for (auto iter : properties)
        {
        temp.Sprintf ("; %s=\"%s\"", iter.first.c_str (), iter.second.c_str ());
        list.append (temp);
        }

    return list;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpMultipartBody::AddPart (HttpBodyPtr bodyPart, Utf8StringCR contentType)
    {
    AddPart (bodyPart, contentType, HttpMultipartProperties ());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpMultipartBody::AddPart (HttpBodyPtr bodyPart, Utf8StringCR contentType, HttpMultipartPropertiesCR properties)
    {
    Utf8String headerStart = m_parts.size () == 0 ? "--" : "\r\n--";

    m_parts.push_back (HttpMPart ());
    HttpMPart& multipartPart = m_parts.back ();

    Utf8String propertyList = GetPropertyStringList (properties);

    multipartPart.content = bodyPart;
    multipartPart.header.Sprintf (
            "%s%s\r\n"
            "Content-Disposition: form-data%s\r\n"
            "Content-Type: %s\r\n"
            "\r\n",
            headerStart.c_str (), m_boundary.c_str (),
            propertyList.c_str (),
            contentType.c_str ());

    multipartPart.length = multipartPart.content->GetLength () + multipartPart.header.length ();

    m_totalPartsLength += multipartPart.length;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpMultipartBody::Open ()
    {
    for (HttpMPart& part : m_parts)
        {
        part.content->Open ();
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpMultipartBody::Close ()
    {
    for (HttpMPart& part : m_parts)
        {
        part.content->Close ();
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus HttpMultipartBody::SetPosition (uint64_t position)
    {
    uint64_t currentLength = GetLength ();
    if (position > currentLength)
        {
        return ERROR;
        }
    m_position = position;
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus HttpMultipartBody::GetPosition (uint64_t& position)
    {
    position = m_position;
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus HttpMultipartBody::Reset ()
    {
    return SetPosition (0);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
size_t HttpMultipartBody::Write (const char* buffer, size_t bufferSize)
    {
    BeAssert (false && "Write is not supported");
    return 0;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
size_t HttpMultipartBody::Read (char* bufferOut, size_t bufferSize)
    {
    if (m_position >= GetLength ())
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
        size_t partHeaderLength = foundPart->header.length ();
        if (positionInPart < partHeaderLength)
            {
            bytesRead = foundPart->header.copy (bufferOut, bufferSize, static_cast<Utf8String::size_type>(positionInPart));
            }
        else
            {
            foundPart->content->SetPosition (positionInPart - partHeaderLength);
            bytesRead = foundPart->content->Read (bufferOut, bufferSize);
            }
        }
    else
        {
        Utf8String::size_type stringPosition = static_cast<Utf8String::size_type>(m_position - m_totalPartsLength);
        bytesRead = m_boundaryFinish.copy (bufferOut, bufferSize, stringPosition);
        }

    m_position += bytesRead;
    return bytesRead;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t HttpMultipartBody::GetLength ()
    {
    if (m_parts.size () == 0)
        {
        return 0;
        }
    return m_totalPartsLength + m_boundaryFinish.size ();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value HttpMultipartBody::AsJson () const
    {
    return Json::nullValue;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Benjamin.Brown  01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpMultipartBody::AsRapidJson (rapidjson::Document& document) const
    {
    }

#define IsInstanceOf(instance, typeof) nullptr != dynamic_cast<typeof*>(instance)

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String HttpMultipartBody::AsString () const
    {
    Utf8String result;
    for (const HttpMPart& part : m_parts)
        {
        result += part.header;

        if (IsInstanceOf (part.content.get (), HttpFileBody))
            {
            result += Utf8PrintfString ("<< file %llu bytes >>", part.content->GetLength ());
            }
        else
            {
            result += part.content->AsString ();
            }
        }
    result += m_boundaryFinish;
    return result;
    }


/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HttpRangeBody::HttpRangeBody (HttpBodyPtr body, uint64_t bytesFrom, uint64_t bytesTo) :
m_body (body),
m_bytesFrom (bytesFrom),
m_bytesTo (bytesTo)
    {
    BeAssert (bytesFrom <= bytesTo);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HttpRangeBody::~HttpRangeBody ()
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HttpRangeBodyPtr HttpRangeBody::Create (HttpBodyPtr body, uint64_t bytesFrom, uint64_t bytesTo)
    {
    return new HttpRangeBody (body, bytesFrom, bytesTo);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpRangeBody::Open ()
    {
    m_body->Open ();
    m_body->SetPosition (m_bytesFrom);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpRangeBody::Close ()
    {
    m_body->Close ();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus HttpRangeBody::SetPosition (uint64_t position)
    {
    return m_body->SetPosition (m_bytesFrom + position);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus HttpRangeBody::GetPosition (uint64_t& position)
    {
    uint64_t bodyPosition;
    if (SUCCESS != m_body->GetPosition (bodyPosition))
        {
        return ERROR;
        }
    position = bodyPosition - m_bytesFrom;
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus HttpRangeBody::Reset ()
    {
    if (SUCCESS != SetPosition (0))
        {
        return ERROR;
        }

    char buffer[1] = {0};

    for (uint64_t i = 0; i < GetLength (); i++)
        {
        if (1 != Write (buffer, 1))
            {
            return ERROR;
            }
        }

    if (SUCCESS != SetPosition (0))
        {
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
size_t HttpRangeBody::Write (const char* buffer, size_t bufferSize)
    {
    bufferSize = TrimBufferSize (bufferSize);
    if (0 == bufferSize)
        {
        return 0;
        }

    return m_body->Write (buffer, bufferSize);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
size_t HttpRangeBody::Read (char* bufferOut, size_t bufferSize)
    {
    bufferSize = TrimBufferSize (bufferSize);
    if (0 == bufferSize)
        {
        return 0;
        }

    return m_body->Read (bufferOut, bufferSize);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
size_t HttpRangeBody::TrimBufferSize (size_t bufferSize)
    {
    uint64_t length = GetLength ();
    uint64_t position;
    if (SUCCESS != GetPosition (position))
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
uint64_t HttpRangeBody::GetLength ()
    {
    return m_bytesTo - m_bytesFrom + 1;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value HttpRangeBody::AsJson () const
    {
    Json::Value json;
    Json::Reader ().parse (AsString (), json);
    return json;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void HttpRangeBody::AsRapidJson (rapidjson::Document& document) const
    {
    unsigned status = !document.Parse<0> (AsString ().c_str ()).HasParseError ();
    BeAssert (status);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String HttpRangeBody::AsString () const
    {
    return const_cast<HttpRangeBody*>(this)->ReadAllAsString ();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String HttpRangeBody::ReadAllAsString ()
    {
    uint64_t initialPosition;
    if (SUCCESS != GetPosition (initialPosition))
        {
        return nullptr;
        }

    if (SUCCESS != SetPosition (0))
        {
        return nullptr;
        }

    Utf8String str;

    size_t bufferSize = 256;
    char* buffer = new char[bufferSize];

    size_t bytesRead;
    while (bytesRead = Read (buffer, bufferSize))
        {
        str.append (buffer, bytesRead);
        }

    delete[] buffer;

    if (SUCCESS != SetPosition (initialPosition))
        {
        return nullptr;
        }

    return str;
    }
