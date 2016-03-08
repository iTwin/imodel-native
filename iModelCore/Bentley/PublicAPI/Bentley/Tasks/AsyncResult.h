/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Bentley/Tasks/AsyncResult.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/Tasks/Tasks.h>
#include <memory>

BEGIN_BENTLEY_TASKS_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma   05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename ValueType, typename ErrorType>
//! Class for returning success Value or Error from async operations
struct AsyncResult
    {
    template <typename AnyValue, typename AnyError> friend struct AsyncResult;

    private:
        bool m_isSuccess;
        mutable std::shared_ptr<ValueType> m_value;
        mutable std::shared_ptr<ErrorType> m_error;

    protected:
        AsyncResult (bool isSuccess, std::shared_ptr<ValueType> value, std::shared_ptr<ErrorType> error) :
            m_isSuccess (isSuccess),
            m_value (value),
            m_error (error)
            {
            };

    public:
        AsyncResult () :
            m_isSuccess (false)
            {
            };

        template<typename AnyValue>
        static AsyncResult<ValueType, ErrorType> Error (const AsyncResult<AnyValue, ErrorType>& error)
            {
            return AsyncResult<ValueType, ErrorType> (false, nullptr, error.m_error);
            }

        static AsyncResult<ValueType, ErrorType> Error (const ErrorType& error)
            {
            return AsyncResult<ValueType, ErrorType> (false, nullptr, std::make_shared<ErrorType> (error));
            }

        static AsyncResult<ValueType, ErrorType> Error (const ErrorType& error, const ValueType& value)
            {
            return AsyncResult<ValueType, ErrorType> (false, std::make_shared<ValueType> (value), std::make_shared<ErrorType> (error));
            }

        static AsyncResult<ValueType, ErrorType> Success (const ValueType& value)
            {
            return AsyncResult<ValueType, ErrorType> (true, std::make_shared<ValueType> (value), nullptr);
            }

        AsyncResult<ValueType, ErrorType>& SetSuccess (const ValueType& value)
            {
            m_isSuccess = true;
            m_value = std::make_shared<ValueType> (value);
            m_error = nullptr;
            return *this;
            };

        AsyncResult<ValueType, ErrorType>& SetError (const ErrorType& error)
            {
            m_isSuccess = false;
            m_value = nullptr;
            m_error = std::make_shared<ErrorType> (error);
            return *this;
            };

        AsyncResult<ValueType, ErrorType>& SetError (const ErrorType& error, const ValueType& value)
            {
            m_isSuccess = false;
            m_value = std::make_shared<ValueType> (value);
            m_error = std::make_shared<ErrorType> (error);
            return *this;
            };

        bool IsSuccess () const
            {
            return m_isSuccess;
            };

        ValueType& GetValue ()
            {
            if (!m_value)
                {
                m_value = std::make_shared<ValueType> ();
                }
            return *m_value;
            };

        const ValueType& GetValue () const
            {
            if (!m_value)
                {
                m_value = std::make_shared<ValueType> ();
                }
            return *m_value;
            };

        ErrorType& GetError ()
            {
            if (!m_error)
                {
                m_error = std::make_shared<ErrorType> ();
                }
            return *m_error;
            };

        const ErrorType& GetError () const
            {
            if (!m_error)
                {
                m_error = std::make_shared<ErrorType> ();
                }
            return *m_error;
            };
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma   05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename ErrorType>
//! Class for returning success or Error from async operations
struct AsyncResult<void, ErrorType>
    {
    template <typename AnyValue, typename AnyError> friend struct AsyncResult;

    private:
        bool m_isSuccess;
        mutable std::shared_ptr<ErrorType> m_error;

    protected:
        AsyncResult (bool isSuccess, std::shared_ptr<ErrorType> error) :
            m_isSuccess (isSuccess),
            m_error (error)
            {
            };

    public:
        AsyncResult () :
            m_isSuccess (false)
            {
            };

        template<typename AnyValue>
        static AsyncResult<void, ErrorType> Error (const AsyncResult<AnyValue, ErrorType>& error)
            {
            return AsyncResult<void, ErrorType> (false, error.m_error);
            }

        static AsyncResult<void, ErrorType> Error (const ErrorType& error)
            {
            return AsyncResult<void, ErrorType> (false, std::make_shared<ErrorType> (error));
            }

        static AsyncResult<void, ErrorType> Success ()
            {
            return AsyncResult<void, ErrorType> (true, nullptr);
            }

        AsyncResult<void, ErrorType>& SetSuccess ()
            {
            m_isSuccess = true;
            m_error = nullptr;
            return *this;
            };

        AsyncResult<void, ErrorType>& SetError (const ErrorType& error)
            {
            m_isSuccess = false;
            m_error = std::make_shared<ErrorType> (error);
            return *this;
            };

        bool IsSuccess () const
            {
            return m_isSuccess;
            };

        ErrorType& GetError ()
            {
            if (!m_error)
                {
                m_error = std::make_shared<ErrorType> ();
                }
            return *m_error;
            };

        const ErrorType& GetError () const
            {
            if (!m_error)
                {
                m_error = std::make_shared<ErrorType> ();
                }
            return *m_error;
            };
    };

END_BENTLEY_TASKS_NAMESPACE
