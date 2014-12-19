//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFInternetHIPLogicalShapeHandler.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFInternetHIPLogicalShapeHandler
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HRFInternetHIPLogicalShapeHandler.h>
#include <Imagepp/all/h/HRFInternetImagingFile.h>

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

const static string s_Label("hip-logical-shape");


//-----------------------------------------------------------------------------
// Public
// Constructor - Do not use s_Label as the parameter of the parent class,
// because this object is contructed statically and the label may be empty
//-----------------------------------------------------------------------------
HRFInternetHIPLogicalShapeHandler::HRFInternetHIPLogicalShapeHandler()
    : HRFInternetASCIIHandler("hip-logical-shape")
    {
    }


//-----------------------------------------------------------------------------
// Public
// Destructor
//-----------------------------------------------------------------------------
HRFInternetHIPLogicalShapeHandler::~HRFInternetHIPLogicalShapeHandler()
    {
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
void HRFInternetHIPLogicalShapeHandler::Handle(HRFInternetImagingFile& pi_rFile,
                                               HFCBuffer&              pio_rBuffer,
                                               HFCInternetConnection&  pi_rConnection)
    {
    // Clear the current buffer
    pio_rBuffer.Clear();

    // Scan the connection untilt the end marker
    ReadUntilMarker(pi_rConnection, pio_rBuffer);

    // Put the data in a string
    size_t Start = 0;
    while ((*(pio_rBuffer.GetData() + Start) == ' ') ||
           (*(pio_rBuffer.GetData() + Start) == ':') )
        Start++;
    string LogicalString((const char*)pio_rBuffer.GetData() + Start,
                         pio_rBuffer.GetDataSize() - Start - s_Marker.size());

    // Extract the size of the array.  If greater than zero, continue
    size_t ArraySize = atoi(LogicalString.c_str());
    if (ArraySize > 0)
        {
        // Build the array of doubles
        HArrayAutoPtr<double> pArray(new double[ArraySize]);
        HASSERT(pArray.get() != 0);

        size_t NumEntries = 0;
        string::size_type StartPos = 0;
        string::size_type EndPos   = string::npos;
        string::size_type Length;

        // position the string after the first number (the count of items)
        StartPos = LogicalString.find_first_of(" ", StartPos);
        if (StartPos != string::npos)
            StartPos++;

        // The data will be ordered after the array in the format "<space>data" and so on.
        // Search for a space and the next data will be right after it
        while (StartPos != string::npos)
            {
            // find the next separator from the current start pos
            EndPos = LogicalString.find_first_of(" ", StartPos);

            // build the request if any
            if (EndPos != StartPos)
                {
                // get a string on the current data
                Length = (EndPos != string::npos ? EndPos - StartPos : EndPos);
                string CurrentData(LogicalString, StartPos, Length);

                // Convert to a double directly in the array.  If the actual number
                // of entry found is higher that ArraySize, do not add to the array
                // but continue counting.  This will help us detect an error, if any.
                if (NumEntries < ArraySize)
                    pArray[NumEntries] = atof(CurrentData.c_str());
                NumEntries++;
                }

            // setup for the next iteration
            StartPos = (EndPos != string::npos ? EndPos + 1 : EndPos);
            }

        // continue if there were the actual announced number of numbers
        if (NumEntries == ArraySize)
            {
            HFCMonitor Monitor(pi_rFile.m_DataKey);

            // transfer the auto ptr ownership
            pi_rFile.m_pClipShape    = pArray;
            HASSERT_X64(ArraySize < ULONG_MAX);
            pi_rFile.m_ClipShapeSize = (uint32_t)ArraySize;
            }
        }
    }
