//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCExceptionHandler.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class: HFCExceptionHandler
//-----------------------------------------------------------------------------
#pragma once

BEGIN_IMAGEPP_NAMESPACE

class HFCException;

/**

    This class is a two-purpose one provided to ease the installation of a
    global, generic exception handling mechanism in an application.

    First it defines a simple common interface for generic exception handling.
    The programmer will have to derive a new class from this one, in order to
    put in it the code for handling exceptions at the application level.
    For example, display a dialog with informational message is the kind of
    job to be performed there.

    Second, the "HFCExceptionHandler" label is used more as a scope name
    than as a class name.  It is used to scope two static functions (and
    an internal static variable). One of them is used to ease exception
    handling from the inner side of the application (from the library
    code): @c{HandleException}, the  function to call to delegate
    handling of an exception.  The other is the @c{SetExceptionHandler}
    function, to be called at the beginning of the execution of an
    application in order to set in place the exception handler developped
    specifically for that application.

    The role of a global handling mechanism is to regroup in a single place
    all exception handling code that does not have to know a specific context to
    deal with the situation.  This let the application the possibility to customize
    such management.  This provides also a way to deal with exceptions occurring
    inside destructors, which are places to should never let an exception go out.

*/

class HFCExceptionHandler
    {
public:

    //:> Static interface for exception handling in destructors and elsewhere needed

    IMAGEPP_EXPORT static HSTATUS  HandleException();
    IMAGEPP_EXPORT static HSTATUS  HandleException(const HFCException* pi_pException);
    IMAGEPP_EXPORT static void     SetExceptionHandler(HFCExceptionHandler* pi_pHandler);

    // The interface that must be implemented by application's handler.
    // If pi_pException is set to 0, handles unknown or unsupported exception.

    virtual HSTATUS LocalHandler(const HFCException* pi_pException = 0) = 0;

private:

    static HFCExceptionHandler* m_pHandler;
    };

END_IMAGEPP_NAMESPACE