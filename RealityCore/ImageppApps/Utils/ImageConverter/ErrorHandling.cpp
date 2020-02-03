/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ImageConverter/ErrorHandling.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
// $Header: /imagepp-root/Apps/Utils/ImageConverter/ErrorHandling.cpp,v 1.4 2011/07/18 21:12:36 Donald.Morissette Exp $
//-----------------------------------------------------------------------------
// Implementation of ErrorHandling
//-----------------------------------------------------------------------------
#include "ImageConverterPch.h"

#include <iostream>

#include <tchar.h>
#include <stdexcpt.h>
#include <Imagepp/all/h/HFCException.h>
#include <Imagepp/all/h/HRFException.h>
#include <Imagepp/all/h/HCDException.h>


//-----------------------------------------------------------------------------
// This function print out the version information for the current program.
//-----------------------------------------------------------------------------

void MyExceptionHandler()
{
    string  Msg;
    bool   InternalError = false;
    uint32_t  InternalErrorNumber = 0;

    try
    {
        throw;
    }
    catch (HFCException& rException)
    {
        if (dynamic_cast<HFCFileNotFoundException*>(&rException) != 0)
        {
            Msg = "R002 - Input file not found.";
        }
        else if (dynamic_cast<HFCCorruptedFileException*>(&rException) != 0)
        {
            Msg = "R003 - Bad or unsupported input format.";
        }
        else if (dynamic_cast<HRFPixelTypeNotSupportedException*>(&rException) != 0 )
        {
            Msg = "R010 - Input pixel type not supported.";
        }
        else if (dynamic_cast<HFCFileNotSupportedException*>(&rException) != 0 )
        {            
            Msg = "R003 - Bad or unsupported input format.";
        }
        else if (dynamic_cast<HFCFileNotCreatedException*>(&rException) != 0 )
        {
            Msg = "R004 - No permission to write on output device.";
        }
        else if (dynamic_cast<HFCFilePermissionDeniedException*>(&rException) != 0 )
        {
            Msg = "R004 - No permission to write on output device.";
        }
        else if (dynamic_cast<HFCNoDiskSpaceLeftException*>(&rException) != 0 )
        {
            Msg = "R005 - Not enough disk space on output device.";
        }
        else if (dynamic_cast<HFCFileExistException*>(&rException) != 0 )
        {
            Msg = "R006 - Output file already exists.";
        }
        else if (dynamic_cast<HFCWriteFaultException*>(&rException) != 0 )
        {
            Msg = "R007 - Write error on output device";
        }
        else if (dynamic_cast<HRFBadPageNumberException*>(&rException) != 0)
        {
            InternalErrorNumber = 118;
            InternalError       = true;
        }    
        else if (dynamic_cast<HRFBadSubImageException*>(&rException) != 0)
        {
            InternalErrorNumber = 120;
            InternalError       = true;
        }
        /* **** Deprecated exceptions ***
        if (rException.GetID() == HRP_PALETTE_FULL_EXCEPTION)
        {
            InternalErrorNumber = 125;
            InternalError       = true;
        }       
        else
        if (rException.GetID() == HRP_NO_ENTRY_EXCEPTION)
        {
            InternalErrorNumber = 130;
            InternalError       = true;
        }*/             
        else if (dynamic_cast<HFCOutOfMemoryException*>(&rException) != 0)
        {
            Msg = "R013 - Out Of Memory.";
        }
        else if (dynamic_cast<HFCCannotLockFileException*>(&rException) != 0)
        {
            InternalErrorNumber = 145;
            InternalError       = true;
        }
        else if (dynamic_cast<HFCFileOutOfRangeException*>(&rException) != 0)
        {
            InternalErrorNumber = 150;
            InternalError       = true;
        }
        else if (dynamic_cast<HFCDeviceAbortException*>(&rException) != 0 )
        {
            InternalErrorNumber = 200;
            InternalError       = true;
        }
        else if (dynamic_cast<HFCMemoryException*>(&rException) != 0)
        {
            InternalErrorNumber = 201;
            InternalError       = true;
        }
        else
        {
            try
            {
                throw;
            }  
            catch(HFCDeviceException&)
            {             
                InternalErrorNumber = 202;
                InternalError       = true;
            }
            catch(HFCFileException&)
            {
                InternalErrorNumber = 203;
                InternalError       = true;
            }
            catch(HCDException&)
            {
                InternalErrorNumber = 204;
                InternalError       = true;
            }
            /* *** Deprecated ***
            catch(HRPException&)
            {
                InternalErrorNumber = 205;
                InternalError       = true;
            }  */  
            catch(...)
            {
                Msg = "R001 - Internal error. Contact technical support. [0]";
            }
        }                       
    }   
    catch(logic_error&)
    {
        InternalErrorNumber = 302;
        InternalError       = true;
    }
    catch(runtime_error&)
    {
        InternalErrorNumber = 301;
        InternalError       = true;
    }
    catch(exception&)
    {
        InternalErrorNumber = 300;
        InternalError       = true;
    }    
    catch(...)
    {
        Msg = "R001 - Internal error. Contact technical support. [0]";
    }
    
    //cout << endl << "FAILED : " << Msg.c_str() << endl;
    if (InternalError)
    {
        printf("\b\b\b\b\b\b\b\b\b\bFAILED : R001 - Internal error. Contact technical support. [%ld]", InternalErrorNumber);
    }
    else
        printf("\b\b\b\b\b\b\b\b\b\bFAILED : %s", Msg.c_str());
}
