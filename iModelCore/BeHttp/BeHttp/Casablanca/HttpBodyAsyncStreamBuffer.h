/*--------------------------------------------------------------------------------------+
|
|     $Source: BeHttp/Casablanca/HttpBodyAsyncStreamBuffer.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include "cpprest/astreambuf.h"
#include <BeHttp/HttpBody.h>

BEGIN_BENTLEY_HTTP_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Ryan.McNulty    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename CharType>
class HttpBodyAsyncStreamBuffer
    : public Concurrency::streams::details::basic_streambuf<CharType>
    {
private:
    HttpBodyPtr m_body;
    CharType* m_block;

public:
    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                                    Ryan.McNulty    02/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    HttpBodyAsyncStreamBuffer (HttpBodyPtr body)
        : 
        m_body (body),
        m_block (nullptr)
        {
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                                    Ryan.McNulty    02/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    ~HttpBodyAsyncStreamBuffer ()
        {
        m_body = nullptr;
        if (m_block != nullptr)
            free (m_block);
        m_block = nullptr;
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                                    Ryan.McNulty    02/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    bool can_read () const
        {
        if (m_body != nullptr && !m_body.IsNull () && m_body.IsValid ())
            {
            return true;
            }
        return false;
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                                    Ryan.McNulty    02/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    bool can_write () const
        {
        if (m_body != nullptr && !m_body.IsNull () && m_body.IsValid ())
            {
            return true;
            }
        return false;
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                                    Ryan.McNulty    02/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    bool can_seek () const
        {
        if (m_body != nullptr && !m_body.IsNull () && m_body.IsValid ())
            {
            return true;
            }
        return false;
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                                    Ryan.McNulty    02/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    bool has_size () const
        {
        if (m_body != nullptr && !m_body.IsNull () && m_body.IsValid ())
            {
            return true;
            }
        return false;
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                                    Ryan.McNulty    02/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    bool is_eof () const
        {
        uint64_t position = 0;
        m_body->GetPosition (position);
        return m_body->GetLength () == position;
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                                    Ryan.McNulty    02/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    size_t buffer_size (std::ios_base::openmode direction = std::ios_base::in) const
        {
        return (size_t) m_body->GetLength ();
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                                    Ryan.McNulty    02/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    void set_buffer_size (size_t size, std::ios_base::openmode direction = std::ios_base::in)
        {
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                                    Ryan.McNulty    02/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    size_t in_avail () const
        {
        return (size_t) m_body->GetLength ();
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                                    Ryan.McNulty    02/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    bool is_open () const
        {
        if (m_body != nullptr && !m_body.IsNull () && m_body.IsValid ())
            {
            return true;
            }
        return false;
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                                    Ryan.McNulty    02/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    pplx::task<void> close (std::ios_base::openmode mode = (std::ios_base::in | std::ios_base::out))
        {
        return pplx::create_task ([this] (){});
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                                    Ryan.McNulty    02/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    pplx::task<void> close (std::ios_base::openmode mode, std::exception_ptr eptr)
        {
        return pplx::create_task ([this] (){});
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                                    Ryan.McNulty    02/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    pplx::task<int_type> putc (CharType ch)
        {
        return pplx::create_task ([this, ch] ()
            {
            const char byte = static_cast<const char>(ch);
            size_t bytesWritten = m_body->Write (&byte, 1);
            if (bytesWritten == 1)
                return 1L;
            else
                return static_cast<long>(EOF);
            });
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                                    Ryan.McNulty    02/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    pplx::task<size_t> putn (const CharType *ptr, size_t count)
        {
        return pplx::create_task ([this, ptr, count] ()
            {
            size_t bytesWritten = 0;
            const char* bytes = (const char*)ptr;
            bytesWritten = m_body->Write (bytes, count);
            return bytesWritten;
            });
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                                    Ryan.McNulty    02/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    pplx::task<int_type> bumpc ()
        {
        return pplx::create_task ([this] ()
            {
            return sbumpc ();
            });
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                                    Ryan.McNulty    02/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    int_type sbumpc ()
        {
        char* bufferOut = (char*)malloc (sizeof(char));
        size_t bytesRead = m_body->Read (bufferOut, 1);
        if (bytesRead == 0)
            memset (bufferOut, static_cast<long>(EOF), 1);
        long retValue = bufferOut[0];
        free (bufferOut);
        return retValue;
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                                    Ryan.McNulty    02/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    pplx::task<int_type> getc ()
        {
        return pplx::create_task ([this] ()
            {
            return sgetc ();
            });
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                                    Ryan.McNulty    02/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    int_type sgetc ()
        {
        char* bufferOut = (char*)malloc (sizeof(char));
        size_t bytesRead = m_body->Read (bufferOut, 1);
        if (bytesRead == 0)
            memset (bufferOut, static_cast<long>(EOF), 1);
        else
            {
            uint64_t position;
            m_body->GetPosition (position);
            m_body->SetPosition (--position);
            }
        long retValue = bufferOut[0];
        free (bufferOut);
        return retValue;
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                                    Ryan.McNulty    02/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    pplx::task<int_type> nextc ()
        {
        return pplx::create_task ([this] ()
            {
            uint64_t position;
            m_body->GetPosition (position);
            position++;
            m_body->SetPosition (position);
            return static_cast<long>(position);
            });
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                                    Ryan.McNulty    02/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    pplx::task<int_type> ungetc ()
        {
        return pplx::create_task ([this] ()
            {
            uint64_t position;
            m_body->GetPosition (position);
            m_body->SetPosition (--position);

            char* bufferOut = (char*)malloc (sizeof(char));
            size_t bytesRead = m_body->Read (bufferOut, 1);
            if (bytesRead == 0)
                memset (bufferOut, static_cast<long>(EOF), 1);
            long retValue = bufferOut[0];
            free (bufferOut);
            return retValue;
            });
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                                    Ryan.McNulty    02/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    pplx::task<size_t> getn (_Out_writes_ (count) CharType *ptr, _In_ size_t count)
        {
        return pplx::create_task ([this, ptr, count] ()
            {
            size_t bytesRead = 0;
            char* bytes = (char*)ptr;

            bytesRead = m_body->Read (bytes, count);
            return bytesRead;
            });
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                                    Ryan.McNulty    02/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    size_t scopy (_Out_writes_ (count) CharType *ptr, _In_ size_t count)
        {
        char* bytes = (char*)ptr;
        size_t bytesRead = m_body->Read (bytes, count);
        uint64_t position;
        m_body->GetPosition (position);
        m_body->SetPosition (position - count);
        return bytesRead;
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                                    Ryan.McNulty    02/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    pos_type getpos (std::ios_base::openmode direction) const
        {
        uint64_t position;
        m_body->GetPosition (position);
        if (position == m_body->GetLength ())
            return static_cast<long>(EOF);
        return position;
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                                    Ryan.McNulty    02/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    utility::size64_t size () const
        {
        return m_body->GetLength ();
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                                    Ryan.McNulty    02/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    pos_type seekpos (pos_type pos, std::ios_base::openmode direction)
        {
        if (SUCCESS != m_body->SetPosition (pos))
            return static_cast<long>(EOF);
        uint64_t position;
        m_body->GetPosition (position);
        return position;
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                                    Ryan.McNulty    02/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    pos_type seekoff (off_type offset, std::ios_base::seekdir way, std::ios_base::openmode mode)
        {
        uint64_t position;
        if (way == std::ios_base::beg)
            {
            if (SUCCESS != m_body->SetPosition (offset))
                return static_cast<long>(EOF);
            }
        else if (way == std::ios_base::cur)
            {
            m_body->GetPosition (position);
            if (SUCCESS != m_body->SetPosition (position + offset))
                return static_cast<long>(EOF);
            }
        else if (way == std::ios_base::end)
            {
            if (SUCCESS != m_body->SetPosition (m_body->GetLength () - 1))
                return static_cast<long>(EOF);
            }
        m_body->GetPosition (position);
        return position;
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                                    Ryan.McNulty    02/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    pplx::task<void> sync ()
        {
        return pplx::create_task ([this] ()
            {
            return;
            });
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                                    Ryan.McNulty    02/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    CharType* alloc (_In_ size_t count)
        {
        if (m_block != nullptr)
            {
            free (m_block);
            }
        size_t bytes = sizeof (CharType)* count;
        m_block = (CharType*)malloc (bytes);
        return m_block;
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                                    Ryan.McNulty    02/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    void commit (_In_ size_t count)
        {
        const char* bytes = (const char*)m_block;
        m_body->Write (bytes, count);
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                                    Ryan.McNulty    02/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    bool acquire (_Out_ CharType*& ptr, _Out_ size_t& count)
        {
        //This string buffer does not supported this operations
        return false;
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                                    Ryan.McNulty    02/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    void release (_Out_writes_ (count) CharType *ptr, _In_ size_t count)
        {
        free (ptr);
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                                    Ryan.McNulty    02/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    std::exception_ptr exception () const
        {
        return nullptr;
        }
    
    virtual pplx::task<size_t> putn_nocopy(const CharType *ptr, size_t count) override
        {
        BeAssert(false); // not implemented; only here to make it compile
        return pplx::create_task ([this, ptr, count] ()
            {
            BeAssert(false); // not implemented; only here to make it compile
            return (size_t)0;
            });
        }
    };

END_BENTLEY_HTTP_NAMESPACE
