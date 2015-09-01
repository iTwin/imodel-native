//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfc/src/HFCExceptionHandler.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class: HFCExceptionHandler
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>
 // Must be the first include.
#include <ImagePP/all/h/HFCException.h>
#include <Imagepp/all/h/HFCExceptionHandler.h>
#include <Logging/bentleylogging.h>

USING_NAMESPACE_BENTLEY_LOGGING

// Initialisation of the static member.

HFCExceptionHandler* HFCExceptionHandler::m_pHandler = 0;

/**----------------------------------------------------------------------------
 This static method must be called to handle an exception event, during
 exception handling (when a signaler object has been caught).  If no
 exception handler have been set, nothing happens and H_ERROR is returned
 as default value.

 This version of the function does not require any parameter and is
 purposed for ultra-generic exception handling.  Another version of this
 function is provided for handling exceptions signaled with the
 HFCException model.

 Since the handler throws no exceptions it is safe to use it in class
 destructors to signal an error.

 @return  A status ID returned by the local handler.  Programmer may use it
          in any way, except for the H_ERROR value that tells that the exception
          have not been managed.
-----------------------------------------------------------------------------*/
HSTATUS HFCExceptionHandler::HandleException()
    {
    HSTATUS Result = H_ERROR;
    if (m_pHandler != 0)
        Result = m_pHandler->LocalHandler();

    return Result;
    }

/**----------------------------------------------------------------------------
 This static method must be called to handle an exception event, during
 exception handling (when a signaler object has been caught).  If no
 exception handler have been set, nothing happens and H_ERROR is returned
 as default value.

 This is the version of this function to call when the signaler object's class
 is or derives from HFCException.  The local handler will be able to recognize
 the exception sub-type and react accordingly.

 Since the handler throws no exceptions it is safe to use it in class
 destructors to signal an error.

 @param pi_pException  A pointer to the HFCException-type signaler object
                       that had been thrown when the exception occurred.

 @return  A status ID returned by the local handler.  Programmer may use it
          in any way, except for the H_ERROR value that tells that the exception
          have not been managed.
-----------------------------------------------------------------------------*/
HSTATUS HFCExceptionHandler::HandleException(const HFCException* pi_pException)
    {
    HSTATUS Result = H_ERROR;
    if (m_pHandler != 0)
        Result = m_pHandler->LocalHandler(pi_pException);
    
    LoggingManager::GetLogger (L"ImagePP")->message (LOG_ERROR, pi_pException->GetExceptionMessage().c_str());
    
    return Result;
    }

/**----------------------------------------------------------------------------
 This method is used to set the local exception handler used by the
 static method HandleException.  This method must be called at the
 beginning of the execution of a program.

 The exception handler is an object of a class that derives from
 HFCExceptionHandler. This class has to be defined by the programmer of
 the application, to provide proper reaction to abnormal events: that
 code has to be placed in the @c{LocalHandler} virtual method of the new
 class.
-----------------------------------------------------------------------------*/
void HFCExceptionHandler::SetExceptionHandler(HFCExceptionHandler* pi_pHandler)
    {
    m_pHandler = pi_pHandler;
    }