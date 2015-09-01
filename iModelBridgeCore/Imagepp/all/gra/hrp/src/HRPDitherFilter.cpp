//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPDitherFilter.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HRPDitherFilter
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRPDitherFilter.h>

#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8.h>
#include <Imagepp/all/h/HRPMessages.h>
#include <Imagepp/all/h/HMGMessageDuplex.h>

#define OCTREE_LEVEL 4


// Messaging setup
HMG_BEGIN_RECEIVER_MESSAGE_MAP(HRPDitherFilter, HMGMessageReceiver, HMG_NO_NEED_COHERENCE_SECURITY)
HMG_REGISTER_MESSAGE(HRPDitherFilter, HRPPaletteChangedMsg, NotifyPaletteChanged)
HMG_END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
// public
// Default constructor
//-----------------------------------------------------------------------------
HRPDitherFilter::HRPDitherFilter()
    : HRPFilter(HRPPixelNeighbourhood(3, 2, 1, 0)),
      m_pPixelType(new HRPPixelTypeI8R8G8B8())
    {
    // init the filter
    InitObject();

    // link to pixel type
    LinkTo(m_pPixelType);
    }

//-----------------------------------------------------------------------------
// public
// constructor
//-----------------------------------------------------------------------------
HRPDitherFilter::HRPDitherFilter(const HFCPtr<HRPPixelType>& pi_pPixelType)
    : HRPFilter(HRPPixelNeighbourhood(3, 2, 1, 0)),
      m_pPixelType(pi_pPixelType)
    {
    // init the filter
    InitObject();

    // link to pixel type
    LinkTo(m_pPixelType);
    }

//-----------------------------------------------------------------------------
// public
// Copy constructor
//-----------------------------------------------------------------------------
HRPDitherFilter::HRPDitherFilter(const HRPDitherFilter& pi_rFilter)
    : HRPFilter(pi_rFilter),
      m_pPixelType(pi_rFilter.m_pPixelType)
    {
    // init the filter
    InitObject();

    // link to pixel type
    LinkTo(m_pPixelType);
    }

//-----------------------------------------------------------------------------
// public
// destructor
//-----------------------------------------------------------------------------
HRPDitherFilter::~HRPDitherFilter()
    {
    // unlink from pixel type
    UnlinkFrom(m_pPixelType);
    }

//-----------------------------------------------------------------------------
// public
// Clone
//-----------------------------------------------------------------------------
HRPFilter* HRPDitherFilter::Clone() const
    {
    return new HRPDitherFilter(*this);
    }

//-----------------------------------------------------------------------------
// public
// Convert
//-----------------------------------------------------------------------------
void HRPDitherFilter::Convert(HRPPixelBuffer* pi_pInputBuffer,
                              HRPPixelBuffer* pio_pOutputBuffer)
    {
    // The input and output pixel types must have been defined
    HPRECONDITION(GetInputPixelType().GetPtr() != 0);
    HPRECONDITION(GetOutputPixelType().GetPtr() != 0);

    // Obtain pointer to source buffer
    Byte* pSrcRawData = (Byte*)pi_pInputBuffer->GetBufferPtr();

    // Obtain pointer to final destination of the filtered data (HRPPixelBuffer output object)
    Byte* pDestRawData = (Byte*)pio_pOutputBuffer->GetBufferPtr();

    // Extract the number of lines to process
    uint32_t LinesCount = pi_pInputBuffer->GetHeight();

    // Declare count variable
    uint32_t PixelsCount;

    // Extract the number of bytes per line for both input and output buffers
    uint32_t InBytes = pi_pInputBuffer->GetBytesPerLine();
    uint32_t OutBytes = pio_pOutputBuffer->GetBytesPerLine();

    // Declare work variables
    Byte* pApproximation;
    short Error;
    uint32_t ChannelsCount;
    Byte EntryIndex;

    // Set the work pointers
    // Current pixel pointer
    Byte* pPixel;
    // Pixel to the right of current pixel
    Byte* pRightPixel;
    // Pixel to the left and under of current pixel
    Byte* pBottomLeftPixel;
    // Pixel under the current pixel
    Byte* pBottomPixel;
    // Pixel to the right and under current pixel
    Byte* pBottomRightPixel;
    // Destination pixel
    Byte* pDest;

    // Get the min max table. Notice that the origin of the table is at entry 112
    Byte* pMinMaxTable = m_MinMaxTable + 112;

    // Get the palette
    const HRPPixelPalette& rPalette = m_pPixelType->GetPalette();

    // Filter all the lines in the buffer
    while(LinesCount != 1)
        {
        // Set the number of pixels in each line
        PixelsCount = pi_pInputBuffer->GetWidth();

        // Set the work pointers
        // Current pixel is initialized to start of soruce data
        pPixel = pSrcRawData;
        // The pixel to the right is 3 bytes further (RGB data)
        pRightPixel = pPixel + 3;
        // Bottom left pixel is on the next line to current pixel (InBytes contain the width of lines in BYTES)
        pBottomLeftPixel = pSrcRawData + InBytes - 3;
        // Bottom pixel is one step further from bottom left pixel
        pBottomPixel = pBottomLeftPixel + 3;
        // Bottom right pixel is yet another step further
        pBottomRightPixel = pBottomPixel + 3;
        // Set destination pixel pointer
        pDest = pDestRawData;

        // Process all the pixels in the line
        while(PixelsCount)
            {
            // Get the best representative entry in the palette for the real pixel value
            EntryIndex = m_QuantizedPalette.GetIndex(pPixel[0], pPixel[1], pPixel[2]);
            // And calculate the RGB value of rPalette
            pApproximation = (Byte*)(rPalette.GetCompositeValue(EntryIndex));

            // Set destination pixel to the approximation of source pixel in palette
            *pDest = EntryIndex;

            // Set the channels count to 3 for RGB
            ChannelsCount = 3;

            // process the 3 channels (RGB)
            while(ChannelsCount)
                {
                // Calculate the error for the current channel
                // This error is the difference between source pixel and approximation

                // RED/GREEN or BLUE Channel depending if it is the first, second or third iteration
                Error = *pPixel - *pApproximation;

                // Add an index to position the error correctly in the tables
                Error += 256;

                // set the approximated channel value in the output
                //*pDest = *pApproximation;

                // diffuse the error
                // calculate the new values for the neighbourhood pixels
                // clamp and set these values

                // THE FOLLOWING MODIFIES THE INPUT BUFFER
                *pBottomPixel = pMinMaxTable[*pBottomPixel + m_BottomError[Error]];

                *pBottomLeftPixel = pMinMaxTable[*pBottomLeftPixel + m_BottomLeftError[Error]] ;

                *pBottomRightPixel = pMinMaxTable[*pBottomRightPixel + m_BottomRightError[Error]];

                // Check if there is a pixel to the right
                if (PixelsCount > 1)
                    {
                    // There is a pixel to the right ... diffuse error
                    *pRightPixel = pMinMaxTable[*pRightPixel + m_RightError[Error]];
                    }

                // increment the pixel pointers to the next CHANNEL
                pPixel++;
                pRightPixel++;
                pBottomLeftPixel++;
                pBottomPixel++;
                pBottomRightPixel++;

                // advance the destination pointer
                //pDest++;

                // go to the next channel in the approximated composite value
                pApproximation++;

                // decrement the channels count
                ChannelsCount--;
                }

            // Advance to next pixel in destination
            pDest++;

            PixelsCount--;

            }

        // All pixels of the line have been processed

        // Advance source to next line
        pSrcRawData += InBytes;
        // Advance destination to next line
        pDestRawData += OutBytes;

        // Another line has been processed
        LinesCount--;
        }

    // At this point there is still the last line to process ...
    // But for this line there is no need to difuse the error to next line only pixel to the right
    // since there is no line
    // Set the number of pixels in each line
    PixelsCount = pi_pInputBuffer->GetWidth();

    // Set the work pointers
    // Current pixel is initialized to start of soruce data
    pPixel = pSrcRawData;
    // The pixel to the right is 3 bytes further (RGB data)
    pRightPixel = pPixel + 3;
    // Set destination pixel pointer
    pDest = pDestRawData;

    // Process all the pixels in the line
    while(PixelsCount)
        {
        // Get the best representative entry in the palette for the real pixel value
        EntryIndex = m_QuantizedPalette.GetIndex(pPixel[0], pPixel[1], pPixel[2]);
        // And calculate the RGB value of rPalette
        pApproximation = (Byte*)(rPalette.GetCompositeValue(EntryIndex));

        // Set destination pixel to the approximation of source pixel in palette
        *pDest = EntryIndex;

        // Set the channels count to 3 for RGB
        ChannelsCount = 3;

        // process the 3 channels (RGB)
        while(ChannelsCount)
            {
            // Calculate the error for the current channel
            // This error is the difference between source pixel and approximation

            // RED/GREEN or BLUE Channel depending if it is the first, second or third iteration
            Error = *pPixel - *pApproximation;

            // Add an index to position the error correctly in the tables
            Error += 256;

            // THE FOLLOWING MODIFIES THE INPUT BUFFER
            // Check if there is a pixel to the right
            if (PixelsCount > 1)
                {
                // There is a pixel to the right ... diffuse error
                *pRightPixel = pMinMaxTable[*pRightPixel + m_RightError[Error]];
                }

            // increment the pixel pointers to the next CHANNEL
            pPixel++;
            pRightPixel++;

            // go to the next channel in the approximated composite value
            pApproximation++;

            // decrement the channels count
            ChannelsCount--;
            }

        // Advance to next pixel in destination
        pDest++;

        PixelsCount--;
        }


    }

//-----------------------------------------------------------------------------
// public
// Notify palette changed
//-----------------------------------------------------------------------------
bool HRPDitherFilter::NotifyPaletteChanged(const HMGMessage& pi_rMessage)
    {
    // flush the current quantized palette
    m_QuantizedPalette.FlushEntries();

    // update the quantized palette
    FillQuantizedPalette();

    // propagate the message
    return true;
    }

//-----------------------------------------------------------------------------
// private
// Process
//-----------------------------------------------------------------------------
void HRPDitherFilter::Process(Byte* pi_pSrcRawData[],
                              Byte* po_pDestRawData,
                              uint32_t pi_Width)
    {
//HChk BB by AR Why the f..k is it defined ???
    HASSERT(0);
    }


//-----------------------------------------------------------------------------
// private
// InitObject
//-----------------------------------------------------------------------------
void HRPDitherFilter::InitObject()
    {
    // Fill quantized palette
    FillQuantizedPalette();

    // Create four pre-calculated tables for the error
    short Error = -256;

    for(uint32_t EntryIndex = 0; EntryIndex < 512; EntryIndex++)
        {
        m_RightError[EntryIndex] = 7 * Error / 16;
        m_BottomLeftError[EntryIndex] = 3 * Error / 16;
        m_BottomError[EntryIndex] = 5 * Error / 16;
        m_BottomRightError[EntryIndex] = Error / 16;

        Error++;
        }

    // Setup the minmax table
    // This table contains 480 entries where the first 112 entries are set at 0,
    // the lass 112 entries are set to 255 while the 256 in between entries are set
    // from 0 to 255. The global figure is a gradual step saturating
    // where the origin is at entry 112
    Byte* pMinMaxTable = m_MinMaxTable + 112;
    memset(pMinMaxTable - 112, 0, 112);
    memset(pMinMaxTable + 256, 255, 112);
    for(uint32_t EntryIndex = 0; EntryIndex < 256; EntryIndex++)
        pMinMaxTable[EntryIndex] = (Byte)EntryIndex;
    }

//-----------------------------------------------------------------------------
// private
// FillQuantizedPalette
//-----------------------------------------------------------------------------
void HRPDitherFilter::FillQuantizedPalette()
    {
    // get the palette
    const HRPPixelPalette& rPalette = m_pPixelType->GetPalette();

    // fill the octree with the destination palette entries
    for(Byte Index = 0; Index < 256; Index++)
        {
        m_QuantizedPalette.AddCompositeValue(rPalette.GetCompositeValue(Index),
                                             Index);
        }
    }
