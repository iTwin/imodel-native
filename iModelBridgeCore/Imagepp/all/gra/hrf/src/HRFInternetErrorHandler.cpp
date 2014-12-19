//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFInternetErrorHandler.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFInternetErrorHandler
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HRFInternetErrorHandler.h>
#include <Imagepp/all/h/HRFInternetImagingFile.h>
#include <Imagepp/all/h/HRFInternetImagingException.h>
#include <Imagepp/all/h/HRFInternetImaging.h>
#include <Imagepp/all/h/HFCErrorCode.h>
#include <Imagepp/all/h/HFCErrorCodeReceiver.h>

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

const static string s_Label("error");


//-----------------------------------------------------------------------------
// Public
// Constructor - Do not use s_Label as the parameter of the parent class,
// because this object is contructed statically and the label may be empty
//-----------------------------------------------------------------------------
HRFInternetErrorHandler::HRFInternetErrorHandler()
    : HRFInternetBinaryHandler("error")
    {
    }


//-----------------------------------------------------------------------------
// Public
// Destructor
//-----------------------------------------------------------------------------
HRFInternetErrorHandler::~HRFInternetErrorHandler()
    {
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
void HRFInternetErrorHandler::Handle(HRFInternetImagingFile& pi_rFile,
                                     HFCBuffer&              pio_rBuffer,
                                     HFCInternetConnection&  pi_rConnection)
    {
    // Scan the connection untilt the binary data
    ReadUntilData(pi_rConnection, pio_rBuffer);

    //
    // Format:
    //
    // Error/length:errclass errnum object [message]\r\n
    //

    uint32_t              Class  = 0;
    uint32_t              Number = 0;
    WString               Object;
    WString               Message;

    // get the size of the response data
    uint32_t DataSize = atol((const char*)pio_rBuffer.GetData() + s_Label.size() + 1);

    // Read the data (clear the buffer prior to that) & clear end marker
    ReadData(pi_rConnection, pio_rBuffer, DataSize, true);
    SkipEndMarker(pi_rConnection);

    // Verify validity
    if (DataSize == 0)
        throw HRFIIHandlerException(HRFII_INVALID_VALUE_EXCEPTION,
                                    pi_rFile.GetURL()->GetURL(),
                                    HRFIIHandlerException::ERROR_HANDLER);

    uint32_t codePage;
    BeStringUtilities::GetCurrentCodePage(codePage);

    // Read the error class
    size_t Parser = 0;
    HPRECONDITION(pio_rBuffer.GetDataSize() <= INT_MAX);
    while (Parser < pio_rBuffer.GetDataSize())
        {
        // skip if space
        if (pio_rBuffer.GetData()[Parser] != ' ')
            {
            // Error Class
            if (Class == 0)
                {
                Class = atol((const char*)pio_rBuffer.GetData() + Parser);

                // skip until a space
                while (pio_rBuffer.GetData()[Parser] != ' ')
                    Parser++;
                }

            // Error Number
            else if (Number == 0)
                {
                Number = atol((const char*)pio_rBuffer.GetData() + Parser);

                // skip until a space
                while (pio_rBuffer.GetData()[Parser] != ' ')
                    Parser++;
                }

            // Error Object
            else if (Object.empty())
                {
                // find the last character
                size_t LastChar = Parser;
                while (pio_rBuffer.GetData()[LastChar] != ' ')
                    LastChar++;

                if (pi_rFile.GetProtocolVersion().GetNumber(2) == HRFInternetImagingFile::s_HIP_UTF8_Protocol)
                    BeStringUtilities::Utf8ToWChar(Object,(CharCP)pio_rBuffer.GetData() + Parser, LastChar - Parser);
                else

                    BeStringUtilities::LocaleCharToWChar(Object,(CharCP)pio_rBuffer.GetData() + Parser, codePage, LastChar - Parser);


                // Skip to space
                Parser = LastChar;
                }

            // Error message
            else if (Message.empty())
                {
                if (pi_rFile.GetProtocolVersion().GetNumber(2) == HRFInternetImagingFile::s_HIP_UTF8_Protocol)
                    BeStringUtilities::Utf8ToWChar(Message,(const char*)pio_rBuffer.GetData() + Parser, pio_rBuffer.GetDataSize() - Parser);
                else
                    BeStringUtilities::LocaleCharToWChar(Message,(CharCP)pio_rBuffer.GetData() + Parser, codePage, pio_rBuffer.GetDataSize() - Parser);

                // Skip to end
                Parser = (int32_t)pio_rBuffer.GetDataSize();
                }
            }

        // Skip to next character
        Parser++;
        }

    // specify the error to the file, it will decide what do do with it
    if ((Class  != 0) &&
        (Number != 0) &&
        (!Object.empty()) &&
        (!Message.empty()) )
        SetError(pi_rFile, Class, Number, Object, Message);
    }


//-----------------------------------------------------------------------------
// Protected
// Analyzes and throws an exception if fatal
//-----------------------------------------------------------------------------
void HRFInternetErrorHandler::SetError(HRFInternetImagingFile& pi_rFile,
                                       uint32_t                pi_ErrorClass,
                                       uint32_t                pi_ErrorNumber,
                                       const WString&          pi_rErrorObject,
                                       const WString&          pi_rErrorMessage)
    {
    // verify if the message contains an error code
    if (HFCErrorCode::IsValidErrorCodeString(pi_rErrorMessage))
        {
        // The message contains an error code.

        // Build the error code
        HFCErrorCode ErrorCode(pi_rErrorMessage);

        // try to obtain the error code from the receiver.  If no module
        // is defined to handle the error code, then the default handler
        // will be used.  This handler throws an HFCErrorCodeException
        // ONLY if the FATAL flag is set.
        HFCErrorCodeReceiver::GetInstance().Handle(ErrorCode);

        // If we pass here, it means that the Handle method did not throw an exception,
        // so it must not be that fatal, so we ignore the error.
        }
    else
        {
        // The message is a plain IIP 1.06 compatible message.

        // bring the error object to lower case
        WString ErrorObject(pi_rErrorObject);
        CaseInsensitiveStringTools().ToLower(ErrorObject);

        // Verify the error class
        switch (pi_ErrorClass)
            {
            case IIP_ERROR_CLASS_FILE:
                // any file error is fatal
                throw HRFInternetImagingServerException(pi_rFile.GetURL()->GetURL(),
                                                        pi_ErrorClass,
                                                        pi_ErrorNumber,
                                                        ErrorObject,
                                                        pi_rErrorMessage);
                break;

            case IIP_ERROR_CLASS_COMMAND:
                if ((ErrorObject.compare(L"til") == 0) ||
                    (ErrorObject.compare(L"fif") == 0) ||
                    (ErrorObject.compare(L"obj") == 0) )
                    {
                    throw HRFInternetImagingServerException(pi_rFile.GetURL()->GetURL(),
                                                            pi_ErrorClass,
                                                            pi_ErrorNumber,
                                                            ErrorObject,
                                                            pi_rErrorMessage);
                    }
                break;

            case IIP_ERROR_CLASS_OBJECT:
                // if the error object start with "obj=", remove it.
                if (ErrorObject.find(L"obj=") == 0)
                    ErrorObject.erase(0, 4);

                // Verify if the object that caused the error is an important
                // object for the creation of the image
                if ((ErrorObject.find(L"hip,") == 0) ||
                    (ErrorObject.compare(L"resolution-number") == 0) ||
                    (ErrorObject.compare(L"comp-group") == 0) ||
                    (ErrorObject.compare(L"max-size") == 0) ||
                    (ErrorObject.compare(L"colorspace") == 0) ||
                    (ErrorObject.compare(L"hip-image-size") == 0) ||
                    (ErrorObject.compare(L"hip-tile-size") == 0) ||
                    (ErrorObject.compare(L"hip-pixel-type") == 0) )
                    {
                    throw HRFInternetImagingServerException(pi_rFile.GetURL()->GetURL(),
                                                            pi_ErrorClass,
                                                            pi_ErrorNumber,
                                                            ErrorObject,
                                                            pi_rErrorMessage);
                    }

                // For the GenerateHistogram method.  So that the wait returns
                // if the histogram is not available for the image.
                if ((&pi_rFile != 0) && (ErrorObject.compare(L"hip-histogram") == 0))
                    pi_rFile.m_HistogramEvent.Signal();

                // For the GetOriginalFileType method.  So that the wait returns
                // if the file type is not available for the image.
                if ((&pi_rFile != 0) && (ErrorObject.compare(L"hip-file-type") == 0))
                    pi_rFile.m_OriginalFileTypeEvent.Signal();

                // For the GetOriginalFileType method.  So that the wait returns
                // if the file type is not available for the image.
                if ((&pi_rFile != 0) && (ErrorObject.find(L"hip-attributes") != string::npos))
                    pi_rFile.m_AttributesEvent.Signal();

                // An error occurs with the til command
                // Invalidate the tile pool to notify editor
                if ((&pi_rFile != 0) && (ErrorObject.compare(L"til") == 0))
                    {
                    pi_rFile.m_TilePool.Invalidate();
                    pi_rFile.m_TilePool.Clear();
                    }

                break;

            case IIP_ERROR_CLASS_SERVER:
                if (pi_ErrorNumber == IIP_SERVER_ERROR_DENIED)
                    {
                    throw HRFInternetImagingServerException(pi_rFile.GetURL()->GetURL(),
                                                            pi_ErrorClass,
                                                            pi_ErrorNumber,
                                                            ErrorObject,
                                                            pi_rErrorMessage);
                    }
                // Not supported for now.
                break;
            }
        }
    }
