/*--------------------------------------------------------------------------------------+
|
|     $Source: Foundations/Errors.cpp $
|    $RCSfile: Errors.cpp,v $
|   $Revision: 1.7 $
|       $Date: 2011/10/25 18:53:51 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableMeshPCH.h>

#include <ScalableMesh/Foundations/Log.h>
#include <ScalableMesh/Foundations/Error.h>
#include <ScalableMesh/Foundations/Warning.h>
#include <ScalableMesh/Foundations/Message.h>

#include <STMInternal/Foundations/PrivateStringTools.h>

BEGIN_BENTLEY_SCALABLEMESH_FOUNDATIONS_NAMESPACE


struct Error::Impl
    {
    WString            m_msg;
    StatusInt               m_errorCode;

    explicit        Impl           (const WChar*         msg,
                                    StatusInt           errorCode)
        :   m_msg(msg),
            m_errorCode(errorCode)
        {

        }

    explicit        Impl           (const Impl&         rhs)
        :   m_msg(rhs.m_msg),
            m_errorCode(rhs.m_errorCode)
        {

        }

    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Error::Error   (const WChar*   msg, 
                StatusInt     code)
    :   m_implP(new Impl(msg, code))
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
 Error& Error::operator= (const Error& rhs)
    {
    m_implP.reset(new Impl(*rhs.m_implP));
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Error::~Error ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Error::Error (const Error& rhs)
    :   m_implP(new Impl(*rhs.m_implP))
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt Error::GetErrorCode () const 
    { 
    return m_implP->m_errorCode; 
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const WChar* Error::what () const
    {
    return m_implP->m_msg.c_str();
    }




struct Warning::Impl
    {
    WString            m_msg;
    StatusInt               m_errorCode;

    explicit        Impl           (const WChar*      msg,
                                    StatusInt           errorCode)
        :   m_msg(msg),
            m_errorCode(errorCode)
        {

        }

    explicit        Impl           (const Impl&         rhs)
        :   m_msg(rhs.m_msg),
            m_errorCode(rhs.m_errorCode)
        {

        }

    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Warning::Warning   (const WChar*  msg,
                    Level           level,
                    StatusInt       errorCode)
    :   m_implP(new Impl(msg, errorCode)),
        m_level(level)
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Warning::~Warning () 
    {
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Warning::Warning (const Warning& rhs)
    :   m_implP(new Impl(*rhs.m_implP)),
        m_level(rhs.m_level)
    {
    
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt Warning::GetErrorCode () const 
    { 
    return m_implP->m_errorCode; 
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const WChar* Warning::what () const 
    { 
    return m_implP->m_msg.c_str(); 
    }




struct Message::Impl
    {
    WString            m_msg;
    StatusInt               m_msgID;

    explicit        Impl           (const WChar*      msg,
                                    StatusInt           msgID)
        :   m_msg(msg),
            m_msgID(msgID)
        {

        }

    explicit        Impl           (const Impl&         rhs)
        :   m_msg(rhs.m_msg),
            m_msgID(rhs.m_msgID)
        {

        }

    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Message::Message   (const WChar* msg,
                    uint32_t        id)
    :   m_implP(new Impl(msg, id))
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Message::~Message () 
    {
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Message::Message (const Message& rhs)
    :   m_implP(new Impl(*rhs.m_implP))
    {
    
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Message& Message::operator= (const Message& rhs)
    {
    *m_implP = *rhs.m_implP;
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t Message::GetID () const 
    { 
    return m_implP->m_msgID; 
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const WChar* Message::what () const 
    { 
    return m_implP->m_msg.c_str(); 
    }






/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Log& GetDefaultLog ()
    {
    class DefaultLog : public Log
        {
        virtual Log* _Clone () const override
            {
            return new DefaultLog;
            }

        virtual void _Add (const Error& error) override
            {
            std::cerr << "Error: " << error.what() << "(Error code: " << error.GetErrorCode() << ")" << std::endl;
            }

        virtual void _Add (const Warning& warning) override
            {
            std::cerr << "Warning: " << warning.what() << "(Error code: " << warning.GetErrorCode() << ")" << std::endl;
            }

       virtual void _Add (const Message& message) override
            {
            std::cout << "Message: " << message.what() << "(ID: " << message.GetID() << ")" << std::endl;
            }
        };

    static DefaultLog DEFAULT_LOG;
    return DEFAULT_LOG;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Log::Log ()
    :   m_implP(0)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Log::Log (const Log& rhs)
    :   m_implP(0)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Log::~Log () 
    {
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Log* Log::Clone () const
    {
    return _Clone();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void Log::Add (const Error& error)
    {
    _Add(error);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void Log::Add (const Warning& warning)
    {
    _Add(warning);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void Log::Add (const Message& message)
    {
    _Add(message);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
WarningItem::WarningItem (const Warning& warning)
    :   m_warningP(warning._Clone())
    {
    
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
WarningItem::~WarningItem ()
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
WarningItem::WarningItem (const WarningItem& rhs)
    :   m_warningP(rhs.m_warningP->_Clone())
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
WarningItem& WarningItem::operator= (const WarningItem& rhs)
    {
    m_warningP.reset(rhs.m_warningP->_Clone());
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt WarningItem::GetErrorCode () const
    {
    return m_warningP->GetErrorCode();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const WChar* WarningItem::what () const
    {
    return m_warningP->what();
    }










/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ErrorItem::ErrorItem (const Error& error)
    :   m_errorP(error._Clone())
    {
    
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ErrorItem::~ErrorItem ()
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ErrorItem::ErrorItem (const ErrorItem& rhs)
    :   m_errorP(rhs.m_errorP->_Clone())
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ErrorItem& ErrorItem::operator= (const ErrorItem& rhs)
    {
    m_errorP.reset(rhs.m_errorP->_Clone());
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ErrorItem::GetErrorCode () const
    {
    return m_errorP->GetErrorCode();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const WChar* ErrorItem::what () const
    {
    return m_errorP->what();
    }






END_BENTLEY_SCALABLEMESH_FOUNDATIONS_NAMESPACE
