//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hcd/src/HCDCodecCCITTRLE.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HCDCodecCCITTRLE
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HCDCodecCCITTRLE.h>
#include <Imagepp/all/h/HCDCodecHMRRLE1.h>
#include <Imagepp/all/h/HCDPacketRLE.h>



//#define SUPPORT_REVERSE_BIT_ONLY 1

#define FillPixels(count) \
    while(count > 32767) \
    {\
        m_RLEBuffer[m_RLEBufferIndex] = 32767;\
        m_RLEBuffer[m_RLEBufferIndex+1] = 0;\
        m_RLEBufferIndex+=2;\
        count-=32767;\
    }\
    m_RLEBuffer[m_RLEBufferIndex] = (unsigned short)count;\
    ++m_RLEBufferIndex;

// Assume black state at start and white at the end
#define FillBlack(pixels) \
    int BlackCount(pixels); \
    FillPixels(BlackCount); \
 
// Assume white state at start and black at the end
#define FillWhite(pixels) \
    int WhiteCount(pixels); \
    FillPixels(WhiteCount); \
 
#define CCITTRLE_MAXLRS_GROW_RATE 2048

#define CCITTRLE_NOERROR                   0
#define CCITTRLE_ERROR_READ_DATA           4
#define CCITTRLE_ERROR_INVALID_CODE        5
#define CCITTRLE_ERROR_NO_MORE_INPUT_DATA  6

// From HCDCodecHMRCCITT.cpp
extern unsigned char BitRevTable[256];
extern Byte        zeroruns[256];
extern Byte        oneruns[256];

struct White
    {
    HArrayAutoPtr<unsigned short>   table;
    HArrayAutoPtr<char>             cnt;
    };

struct Black
    {
    HArrayAutoPtr<unsigned short>   table;
    HArrayAutoPtr<char>             cnt;
    };

static White s_White;
static Black s_Black;

//-----------------------------------------------------------------------------
// Methods for CCITTRLEState
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// public
//-----------------------------------------------------------------------------
HCDCodecCCITTRLE::CCITTRLEState::CCITTRLEState()
    {
    m_bitrevtable = false;
    m_invertResult = false;

    m_buf1   = 0;
    m_buf1c  = 0;
    m_buf2   = 0;
    m_buf2c  = 0;

    m_ccittrleBuffer = 0;
    m_ccittrleBufferSize = 0;
    m_ccittrleBufferIndex = 0;

    m_RLEBuffer = 0;
    m_RLEBufferSize = 0;
    m_RLEBufferIndex = 0;

    m_UncompressBuffer      = 0;
    m_UncompressBufferSize  = 0;
    m_UncompressBufferIndex = 0;
    m_UncompressBufferLineSize = 0;

    m_bufferAllocated = false;
    }

//-----------------------------------------------------------------------------
// public
//-----------------------------------------------------------------------------
HCDCodecCCITTRLE::CCITTRLEState::CCITTRLEState(const CCITTRLEState& pi_rObj)
    {
    m_max_lrs = pi_rObj.m_max_lrs;
    m_ccittrleMask = pi_rObj.m_ccittrleMask;
    m_ccittrleWord = pi_rObj.m_ccittrleWord;
    m_ccittrleError = pi_rObj.m_ccittrleError;
    m_code = pi_rObj.m_code;
    m_bufferAllocated = pi_rObj.m_bufferAllocated;

    // Copy internal state buffer.
    if(m_bufferAllocated)
        {
        m_buf1  = (long*)malloc((m_max_lrs*2+12)*sizeof(long));
        m_buf1c = (char*)malloc((m_max_lrs*2+12)*sizeof(char));
        m_buf2  = (long*)malloc((m_max_lrs*2+12)*sizeof(long));
        m_buf2c = (char*)malloc((m_max_lrs*2+12)*sizeof(char));

        memcpy(m_buf1, pi_rObj.m_buf1, m_max_lrs*2+12*sizeof(long));
        memcpy(m_buf1c, pi_rObj.m_buf1c, m_max_lrs*2+12*sizeof(char));
        memcpy(m_buf2, pi_rObj.m_buf2, m_max_lrs*2+12*sizeof(long));
        memcpy(m_buf2c, pi_rObj.m_buf2c, m_max_lrs*2+12*sizeof(char));
        }
    else
        {
        m_buf1   = 0;
        m_buf1c  = 0;
        m_buf2   = 0;
        m_buf2c  = 0;
        }

    // internal buffer pointer. Always point to the beginning.
    m_a      = m_buf1;
    m_acolor = m_buf1c;
    m_b      = m_buf2;
    m_bcolor = m_buf2c;

    m_bstop = pi_rObj.m_bstop;
    m_bstopp1 = pi_rObj.m_bstopp1;
    m_width = pi_rObj.m_width;
    m_height = pi_rObj.m_height;
    m_bufcode = pi_rObj.m_bufcode;

    // We can't copy this information because a CCITTRLEState object does not owned this buffer.
    // It should always be NULL when we end up here anyways.
    // If not it means we are copy constructing a HCDCodecCCITTRLE while encoding/decoding.
    HASSERT(pi_rObj.m_ccittrleBuffer == 0);
    HASSERT(pi_rObj.m_ccittrleBufferSize == 0);
    HASSERT(pi_rObj.m_ccittrleBufferIndex == 0);
    HASSERT(pi_rObj.m_RLEBuffer == 0);
    HASSERT(pi_rObj.m_RLEBufferSize == 0);
    HASSERT(pi_rObj.m_RLEBufferIndex == 0);
    HASSERT(pi_rObj.m_UncompressBuffer == 0);
    HASSERT(pi_rObj.m_UncompressBufferSize == 0);
    HASSERT(pi_rObj.m_UncompressBufferIndex == 0);

    m_ccittrleBuffer = 0;
    m_ccittrleBufferSize = 0;
    m_ccittrleBufferIndex = 0;

    m_RLEBuffer = 0;
    m_RLEBufferSize = 0;
    m_RLEBufferIndex = 0;

    m_UncompressBuffer      = 0;
    m_UncompressBufferSize  = 0;
    m_UncompressBufferIndex = 0;
    m_UncompressBufferLineSize = 0;

    m_decodeMode = pi_rObj.m_decodeMode;
    m_bitrevtable = pi_rObj.m_bitrevtable;
    m_invertResult = pi_rObj.m_invertResult;
    }


//-----------------------------------------------------------------------------
// public
//-----------------------------------------------------------------------------
HCDCodecCCITTRLE::CCITTRLEState::~CCITTRLEState()
    {
    Reset();
    }

//-----------------------------------------------------------------------------
// public
//-----------------------------------------------------------------------------
void HCDCodecCCITTRLE::CCITTRLEState::Pre(long pi_Width, long pi_Height, bool pi_Decode,
                                          bool pi_UseBitRevTable, bool pi_InvertResult)
    {
    Reset();

    m_bitrevtable = pi_UseBitRevTable;
    m_invertResult = pi_InvertResult;
    m_decodeMode = pi_Decode;

    m_max_lrs = CCITTRLE_MAXLRS_GROW_RATE;
    m_ccittrleError = CCITTRLE_NOERROR;
    m_code = 0;
    m_ccittrleWord = 0;
    m_ccittrleMask = m_decodeMode ? 0 : 0x80L;
    m_eol = false;

    m_height = pi_Height;
    m_width  = pi_Width;
    m_bstop  = pi_Width;
    m_bstopp1= pi_Width + 1;

    m_buf1  = (long*)malloc((m_max_lrs*2+12)*sizeof(long));
    m_buf1c = (char*)malloc((m_max_lrs*2+12)*sizeof(char));
    m_buf2  = (long*)malloc((m_max_lrs*2+12)*sizeof(long));
    m_buf2c = (char*)malloc((m_max_lrs*2+12)*sizeof(char));

    m_a      = m_buf1;
    m_acolor = m_buf1c;
    m_b      = m_buf2;
    m_bcolor = m_buf2c;

    m_b[0] = m_bstopp1;
    m_bcolor[0] = 0;
    m_b[1] = m_bstopp1;
    m_bcolor[1] = 1;

    m_bufcode= false;

    m_bufferAllocated = true;
    }

//-----------------------------------------------------------------------------
// public
//-----------------------------------------------------------------------------
void HCDCodecCCITTRLE::CCITTRLEState::Post()
    {
    if(!m_decodeMode)
        {
        if (m_ccittrleError == CCITTRLE_NOERROR)
            {
            // flush buffer
            if (m_ccittrleMask != 0x80L)
                {
                HASSERT(m_ccittrleBufferIndex < m_ccittrleBufferSize);
                if (m_bitrevtable)
                    {
                    m_ccittrleBuffer[m_ccittrleBufferIndex] = BitRevTable[m_ccittrleWord];
                    }
                else
                    {
                    m_ccittrleBuffer[m_ccittrleBufferIndex] = m_ccittrleWord;
                    }

                ++m_ccittrleBufferIndex;
                }

            if (m_ccittrleBufferIndex > 0)
                {
                if ((m_ccittrleBufferIndex & 1) == 1)
                    {
                    HASSERT(m_ccittrleBufferIndex < m_ccittrleBufferSize);
                    m_ccittrleBuffer[m_ccittrleBufferIndex] = 0;
                    ++m_ccittrleBufferIndex;
                    }
                }
            }
        }
    }
//-----------------------------------------------------------------------------
// public
//-----------------------------------------------------------------------------
void HCDCodecCCITTRLE::CCITTRLEState::Reset()
    {
    // We do not owned memory for these buffers.
    m_ccittrleBuffer = 0;
    m_ccittrleBufferIndex = 0;
    m_ccittrleBufferSize = 0;
    m_RLEBuffer = 0;
    m_RLEBufferSize = 0;
    m_RLEBufferIndex = 0;
    m_UncompressBuffer = 0;
    m_UncompressBufferSize = 0;
    m_UncompressBufferIndex = 0;
    m_UncompressBufferLineSize = 0;

    if(m_bufferAllocated)
        {
        free(m_buf1);
        m_buf1 = 0;
        free(m_buf1c);
        m_buf1c = 0;
        free(m_buf2);
        m_buf2 = 0;
        free(m_buf2c);
        m_buf2c = 0;

        m_bufferAllocated = false;
        }
    }

//-----------------------------------------------------------------------------
// public
//-----------------------------------------------------------------------------
void HCDCodecCCITTRLE::CCITTRLEState::GrowMaxLRS()
    {
    HPRECONDITION(m_bufferAllocated);
    // All buffers are not inverted or all buffers are inverted.
    HPRECONDITION((m_a == m_buf1 && m_acolor == m_buf1c && m_b == m_buf2 && m_bcolor == m_buf2c) ||
                  (m_a == m_buf2 && m_acolor == m_buf2c && m_b == m_buf1 && m_bcolor == m_buf1c));

    m_max_lrs+=CCITTRLE_MAXLRS_GROW_RATE;

    // Buffers can be inverted, make sure they point to the right buffer after the reallocation.
    bool inverted(m_buf1 == m_a ? false : true);

    m_buf1   = (long*)realloc(m_buf1, (m_max_lrs*2+12)*sizeof(long));
    m_buf1c  = (char*)realloc(m_buf1c, (m_max_lrs*2+12)*sizeof(char));
    m_buf2   = (long*)realloc(m_buf2, (m_max_lrs*2+12)*sizeof(long));
    m_buf2c  = (char*)realloc(m_buf2c, (m_max_lrs*2+12)*sizeof(char));

    // Reassign buffer pointer. It is OK since we are indexing these buffers.
    m_a      = m_buf1;
    m_acolor = m_buf1c;
    m_b      = m_buf2;
    m_bcolor = m_buf2c;

    // Invert buffer pointers if needed.
    if(inverted)
        {
        std::swap(m_a, m_b);
        std::swap(m_acolor, m_bcolor);
        }
    }

//-----------------------------------------------------------------------------
// public
// Set buffers use for input(decode) or output(encode)
//-----------------------------------------------------------------------------
void HCDCodecCCITTRLE::CCITTRLEState::SetupForRLEMode(Byte* pio_pCCITTRLEBuffer, size_t pi_CCITTRLEBufferSize,
                                                      unsigned short* pio_pRLEBuffer, size_t pi_RLEBufferSize)
    {
    m_ccittrleBuffer      = pio_pCCITTRLEBuffer;
    m_ccittrleBufferSize  = pi_CCITTRLEBufferSize;
    m_ccittrleBufferIndex = 0;

    m_RLEBuffer        = pio_pRLEBuffer;
    m_RLEBufferSize    = pi_RLEBufferSize;
    m_RLEBufferIndex   = 0;
    }

//-----------------------------------------------------------------------------
// public
// Set buffers use for input(decode) or output(encode)
//-----------------------------------------------------------------------------
void HCDCodecCCITTRLE::CCITTRLEState::SetRLEBuffer(unsigned short* pio_pRLEBuffer, size_t pi_RLEBufferSize)
    {
    m_RLEBuffer        = pio_pRLEBuffer;
    m_RLEBufferSize    = pi_RLEBufferSize;
    m_RLEBufferIndex   = 0;
    }

//-----------------------------------------------------------------------------
// public
// Set buffers use for input(decode) or output(encode)
//-----------------------------------------------------------------------------
void HCDCodecCCITTRLE::CCITTRLEState::SetupForUncompressMode(Byte* pio_pCCITTRLEBuffer, size_t pi_CCITTRLEBufferSize,
                                                             Byte* pio_pUncompressBuffer, size_t pi_UncompressBufferSize,
                                                             size_t pi_UncompressLineSize)
    {
    HPRECONDITION(pi_UncompressBufferSize % pi_UncompressLineSize == 0);

    m_ccittrleBuffer      = pio_pCCITTRLEBuffer;
    m_ccittrleBufferSize  = pi_CCITTRLEBufferSize;
    m_ccittrleBufferIndex = 0;

    m_UncompressBuffer      = pio_pUncompressBuffer;
    m_UncompressBufferSize  = pi_UncompressBufferSize;
    m_UncompressBufferIndex = 0;
    m_UncompressBufferLineSize = pi_UncompressLineSize;

    if(m_decodeMode)
        {
        if(m_invertResult)
            {
            // Init buffer to all ones
            memset(m_UncompressBuffer, 0xFF, pi_UncompressBufferSize);
            }
        else
            {
            // Init buffer to all zeros
            memset(m_UncompressBuffer, 0x00, pi_UncompressBufferSize);
            }
        }
    }

//-----------------------------------------------------------------------------
// public
// Byte(s) read(decode) or write(encode) in ccittrleBuffer.
//-----------------------------------------------------------------------------
size_t HCDCodecCCITTRLE::CCITTRLEState::GetCCITTRLEBufferDataSize() const
    {
    return m_ccittrleBufferIndex;
    }

//-----------------------------------------------------------------------------
// public
// Byte(s) write(decode) or read(encode) in RLEBuffer.
//-----------------------------------------------------------------------------
size_t HCDCodecCCITTRLE::CCITTRLEState::GetRLEBufferDataSize() const
    {
    HASSERT(sizeof(short) == 2);
    // Short index * 2.
    return m_RLEBufferIndex << 1;
    }

//-----------------------------------------------------------------------------
// public
// Byte(s) write(decode) or read(encode) in uncompress buffer.
//-----------------------------------------------------------------------------
size_t HCDCodecCCITTRLE::CCITTRLEState::GetUncompressBufferDataSize() const
    {
    return m_UncompressBufferIndex;
    }

//-----------------------------------------------------------------------------
// public
// Get next code
//-----------------------------------------------------------------------------
void HCDCodecCCITTRLE::CCITTRLEState::GetCode(long pi_color)
    {
    long makeup;

    m_eol = false;

    if (pi_color == 0)
        {
        GetWhite();
        }
    else
        {
        GetBlack();
        }

    if (m_code == -1)
        {
        m_eol = true;
        m_code = 0;
        return;
        }
    else
        {
        makeup = m_code;
        while (m_code > 63)
            {
            if (pi_color == 0)
                {
                GetWhite();
                }
            else
                {
                GetBlack();
                }
            makeup += m_code;
            } /* end while */
        if (m_code < 0)
            {
            m_ccittrleError = CCITTRLE_ERROR_INVALID_CODE;
            } /* end if */
        m_code = makeup;
        }
    }

//-----------------------------------------------------------------------------
// public
// Get next white code from ccittrle buffer
//-----------------------------------------------------------------------------
void HCDCodecCCITTRLE::CCITTRLEState::GetWhite()
    {
    if (s_White.table == NULL)
        {
        size_t length = 0x10000L;

        s_White.table = new unsigned short[length];
        memset(s_White.table, 0, length * sizeof (unsigned short));

        s_White.cnt = new char[length];
        memset(s_White.cnt, 0, length*sizeof(char));

        s_White.cnt[ 0x7000L ] = 4;
        s_White.cnt[ 0x8000L ] = 4;
        s_White.cnt[ 0xb000L ] = 4;
        s_White.cnt[ 0xc000L ] = 4;
        s_White.cnt[ 0xe000L ] = 4;
        s_White.cnt[ 0xf000L ] = 4;

        s_White.table[ 0x7000L ] = 2;
        s_White.table[ 0x8000L ] = 3;
        s_White.table[ 0xb000L ] = 4;
        s_White.table[ 0xc000L ] = 5;
        s_White.table[ 0xe000L ] = 6;
        s_White.table[ 0xf000L ] = 7;

        s_White.cnt[ 0x9800L ] = 5;
        s_White.cnt[ 0xa000L ] = 5;
        s_White.cnt[ 0x3800L ] = 5;
        s_White.cnt[ 0x4000L ] = 5;
        s_White.cnt[ 0xd800L ] = 5;
        s_White.cnt[ 0x9000L ] = 5;

        s_White.table[ 0x9800L ] = 8;
        s_White.table[ 0xa000L ] = 9;
        s_White.table[ 0x3800L ] = 10;
        s_White.table[ 0x4000L ] = 11;
        s_White.table[ 0xd800L ] = 64;
        s_White.table[ 0x9000L ] = 128;

        s_White.cnt[ 0x1c00L ] = 6;
        s_White.cnt[ 0x2000L ] = 6;
        s_White.cnt[ 0x0c00L ] = 6;
        s_White.cnt[ 0xd000L ] = 6;
        s_White.cnt[ 0xd400L ] = 6;
        s_White.cnt[ 0xa800L ] = 6;
        s_White.cnt[ 0xac00L ] = 6;
        s_White.cnt[ 0x5c00L ] = 6;
        s_White.cnt[ 0x6000L ] = 6;

        s_White.table[ 0x1c00L ] = 1;
        s_White.table[ 0x2000L ] = 12;
        s_White.table[ 0x0c00L ] = 13;
        s_White.table[ 0xd000L ] = 14;
        s_White.table[ 0xd400L ] = 15;
        s_White.table[ 0xa800L ] = 16;
        s_White.table[ 0xac00L ] = 17;
        s_White.table[ 0x5c00L ] = 192;
        s_White.table[ 0x6000L ] = 1664;

        s_White.cnt[ 0x4e00L ] = 7;
        s_White.cnt[ 0x1800L ] = 7;
        s_White.cnt[ 0x1000L ] = 7;
        s_White.cnt[ 0x2e00L ] = 7;
        s_White.cnt[ 0x0600L ] = 7;
        s_White.cnt[ 0x0800L ] = 7;
        s_White.cnt[ 0x5000L ] = 7;
        s_White.cnt[ 0x5600L ] = 7;
        s_White.cnt[ 0x2600L ] = 7;
        s_White.cnt[ 0x4800L ] = 7;
        s_White.cnt[ 0x3000L ] = 7;
        s_White.cnt[ 0x6e00L ] = 7;

        s_White.table[ 0x4e00L ] = 18;
        s_White.table[ 0x1800L ] = 19;
        s_White.table[ 0x1000L ] = 20;
        s_White.table[ 0x2e00L ] = 21;
        s_White.table[ 0x0600L ] = 22;
        s_White.table[ 0x0800L ] = 23;
        s_White.table[ 0x5000L ] = 24;
        s_White.table[ 0x5600L ] = 25;
        s_White.table[ 0x2600L ] = 26;
        s_White.table[ 0x4800L ] = 27;
        s_White.table[ 0x3000L ] = 28;
        s_White.table[ 0x6e00L ] = 256;

        s_White.cnt[ 0x3500L ] = 8;
        s_White.cnt[ 0x0200L ] = 8;
        s_White.cnt[ 0x0300L ] = 8;
        s_White.cnt[ 0x1a00L ] = 8;
        s_White.cnt[ 0x1b00L ] = 8;
        s_White.cnt[ 0x1200L ] = 8;
        s_White.cnt[ 0x1300L ] = 8;
        s_White.cnt[ 0x1400L ] = 8;
        s_White.cnt[ 0x1500L ] = 8;
        s_White.cnt[ 0x1600L ] = 8;
        s_White.cnt[ 0x1700L ] = 8;
        s_White.cnt[ 0x2800L ] = 8;
        s_White.cnt[ 0x2900L ] = 8;
        s_White.cnt[ 0x2a00L ] = 8;
        s_White.cnt[ 0x2b00L ] = 8;
        s_White.cnt[ 0x2c00L ] = 8;
        s_White.cnt[ 0x2d00L ] = 8;
        s_White.cnt[ 0x0400L ] = 8;
        s_White.cnt[ 0x0500L ] = 8;
        s_White.cnt[ 0x0a00L ] = 8;
        s_White.cnt[ 0x0b00L ] = 8;
        s_White.cnt[ 0x5200L ] = 8;
        s_White.cnt[ 0x5300L ] = 8;
        s_White.cnt[ 0x5400L ] = 8;
        s_White.cnt[ 0x5500L ] = 8;
        s_White.cnt[ 0x2400L ] = 8;
        s_White.cnt[ 0x2500L ] = 8;
        s_White.cnt[ 0x5800L ] = 8;
        s_White.cnt[ 0x5900L ] = 8;
        s_White.cnt[ 0x5a00L ] = 8;
        s_White.cnt[ 0x5b00L ] = 8;
        s_White.cnt[ 0x4a00L ] = 8;
        s_White.cnt[ 0x4b00L ] = 8;
        s_White.cnt[ 0x3200L ] = 8;
        s_White.cnt[ 0x3300L ] = 8;
        s_White.cnt[ 0x3400L ] = 8;
        s_White.cnt[ 0x3600L ] = 8;
        s_White.cnt[ 0x3700L ] = 8;
        s_White.cnt[ 0x6400L ] = 8;
        s_White.cnt[ 0x6500L ] = 8;
        s_White.cnt[ 0x6800L ] = 8;
        s_White.cnt[ 0x6700L ] = 8;

        s_White.table[ 0x3500L ] = 0;
        s_White.table[ 0x0200L ] = 29;
        s_White.table[ 0x0300L ] = 30;
        s_White.table[ 0x1a00L ] = 31;
        s_White.table[ 0x1b00L ] = 32;
        s_White.table[ 0x1200L ] = 33;
        s_White.table[ 0x1300L ] = 34;
        s_White.table[ 0x1400L ] = 35;
        s_White.table[ 0x1500L ] = 36;
        s_White.table[ 0x1600L ] = 37;
        s_White.table[ 0x1700L ] = 38;
        s_White.table[ 0x2800L ] = 39;
        s_White.table[ 0x2900L ] = 40;
        s_White.table[ 0x2a00L ] = 41;
        s_White.table[ 0x2b00L ] = 42;
        s_White.table[ 0x2c00L ] = 43;
        s_White.table[ 0x2d00L ] = 44;
        s_White.table[ 0x0400L ] = 45;
        s_White.table[ 0x0500L ] = 46;
        s_White.table[ 0x0a00L ] = 47;
        s_White.table[ 0x0b00L ] = 48;
        s_White.table[ 0x5200L ] = 49;
        s_White.table[ 0x5300L ] = 50;
        s_White.table[ 0x5400L ] = 51;
        s_White.table[ 0x5500L ] = 52;
        s_White.table[ 0x2400L ] = 53;
        s_White.table[ 0x2500L ] = 54;
        s_White.table[ 0x5800L ] = 55;
        s_White.table[ 0x5900L ] = 56;
        s_White.table[ 0x5a00L ] = 57;
        s_White.table[ 0x5b00L ] = 58;
        s_White.table[ 0x4a00L ] = 59;
        s_White.table[ 0x4b00L ] = 60;
        s_White.table[ 0x3200L ] = 61;
        s_White.table[ 0x3300L ] = 62;
        s_White.table[ 0x3400L ] = 63;
        s_White.table[ 0x3600L ] = 320;
        s_White.table[ 0x3700L ] = 384;
        s_White.table[ 0x6400L ] = 448;
        s_White.table[ 0x6500L ] = 512;
        s_White.table[ 0x6800L ] = 576;
        s_White.table[ 0x6700L ] = 640;

        s_White.cnt[ 0x6600L ] = 9;
        s_White.cnt[ 0x6680L ] = 9;
        s_White.cnt[ 0x6900L ] = 9;
        s_White.cnt[ 0x6980L ] = 9;
        s_White.cnt[ 0x6a00L ] = 9;
        s_White.cnt[ 0x6a80L ] = 9;
        s_White.cnt[ 0x6b00L ] = 9;
        s_White.cnt[ 0x6b80L ] = 9;
        s_White.cnt[ 0x6c00L ] = 9;
        s_White.cnt[ 0x6c80L ] = 9;
        s_White.cnt[ 0x6d00L ] = 9;
        s_White.cnt[ 0x6d80L ] = 9;
        s_White.cnt[ 0x4c00L ] = 9;
        s_White.cnt[ 0x4c80L ] = 9;
        s_White.cnt[ 0x4d00L ] = 9;
        s_White.cnt[ 0x4d80L ] = 9;

        s_White.table[ 0x6600L ] = 704;
        s_White.table[ 0x6680L ] = 768;
        s_White.table[ 0x6900L ] = 832;
        s_White.table[ 0x6980L ] = 896;
        s_White.table[ 0x6a00L ] = 960;
        s_White.table[ 0x6a80L ] = 1024;
        s_White.table[ 0x6b00L ] = 1088;
        s_White.table[ 0x6b80L ] = 1152;
        s_White.table[ 0x6c00L ] = 1216;
        s_White.table[ 0x6c80L ] = 1280;
        s_White.table[ 0x6d00L ] = 1344;
        s_White.table[ 0x6d80L ] = 1408;
        s_White.table[ 0x4c00L ] = 1472;
        s_White.table[ 0x4c80L ] = 1536;
        s_White.table[ 0x4d00L ] = 1600;
        s_White.table[ 0x4d80L ] = 1728;

        s_White.cnt[ 0x0100L ] = 11;
        s_White.cnt[ 0x0180L ] = 11;
        s_White.cnt[ 0x01a0L ] = 11;

        s_White.table[ 0x0100L ] = 1792;
        s_White.table[ 0x0180L ] = 1856;
        s_White.table[ 0x01a0L ] = 1920;

        s_White.cnt[ 0x0120L ] = 12;
        s_White.cnt[ 0x0130L ] = 12;
        s_White.cnt[ 0x0140L ] = 12;
        s_White.cnt[ 0x0150L ] = 12;
        s_White.cnt[ 0x0160L ] = 12;
        s_White.cnt[ 0x0170L ] = 12;
        s_White.cnt[ 0x01c0L ] = 12;
        s_White.cnt[ 0x01d0L ] = 12;
        s_White.cnt[ 0x01e0L ] = 12;
        s_White.cnt[ 0x01f0L ] = 12;

        s_White.table[ 0x0120L ] = 1984;
        s_White.table[ 0x0130L ] = 2048;
        s_White.table[ 0x0140L ] = 2112;
        s_White.table[ 0x0150L ] = 2176;
        s_White.table[ 0x0160L ] = 2240;
        s_White.table[ 0x0170L ] = 2304;
        s_White.table[ 0x01c0L ] = 2368;
        s_White.table[ 0x01d0L ] = 2432;
        s_White.table[ 0x01e0L ] = 2496;
        s_White.table[ 0x01f0L ] = 2560;
        }

    unsigned char  ccittrleMask = m_ccittrleMask;
    unsigned char  ccittrleWord = m_ccittrleWord;
    unsigned short word   = 0;
    unsigned short wmask  = 0x8000L;

    for (char bit(1); bit<=12; ++bit)
        {
        if (!(ccittrleMask >>= 1))
            {
            if(m_ccittrleBufferIndex >= m_ccittrleBufferSize)
                {
                m_ccittrleError = CCITTRLE_ERROR_NO_MORE_INPUT_DATA;
                m_code  = 0;
                return;
                }
#ifdef SUPPORT_REVERSE_BIT_ONLY
            ccittrleWord = BitRevTable[m_ccittrleBuffer[m_ccittrleBufferIndex]];
            ++m_ccittrleBufferIndex;
#else
            ccittrleWord = m_ccittrleBuffer[m_ccittrleBufferIndex];
            ++m_ccittrleBufferIndex;
            if (m_bitrevtable)
                {
                ccittrleWord = BitRevTable[ ccittrleWord ];
                }
#endif
            ccittrleMask = 0x80;
            }
        if (ccittrleWord & ccittrleMask)
            {
            word |= wmask;
            }
        wmask >>= 1;

        if (s_White.cnt[ word ] == bit)
            {
            m_code = s_White.table[ word ];
            m_ccittrleMask = ccittrleMask;
            m_ccittrleWord = ccittrleWord;
            return;
            }
        }

    m_ccittrleError = CCITTRLE_ERROR_INVALID_CODE;
    m_code  = 0;
    }


//-----------------------------------------------------------------------------
// public
// Get next black code from ccittrle buffer
//-----------------------------------------------------------------------------
void HCDCodecCCITTRLE::CCITTRLEState::GetBlack()
    {
    if (s_Black.table == NULL)
        {
        size_t length = 0x10000L;

        s_Black.table = new unsigned short[length];
        memset(s_Black.table, 0, length*sizeof(unsigned short));

        s_Black.cnt = new char[length];
        memset(s_Black.cnt, 0, length*sizeof(char));

        s_Black.cnt[ 0xc000L ] = 2;
        s_Black.cnt[ 0x8000L ] = 2;

        s_Black.table[ 0xc000L ] = 2;
        s_Black.table[ 0x8000L ] = 3;

        s_Black.cnt[ 0x4000L ] = 3;
        s_Black.cnt[ 0x6000L ] = 3;

        s_Black.table[ 0x4000L ] = 1;
        s_Black.table[ 0x6000L ] = 4;

        s_Black.cnt[ 0x3000L ] = 4;
        s_Black.cnt[ 0x2000L ] = 4;

        s_Black.table[ 0x3000L ] = 5;
        s_Black.table[ 0x2000L ] = 6;

        s_Black.cnt[ 0x1800L ] = 5;

        s_Black.table[ 0x1800L ] = 7;

        s_Black.cnt[ 0x1400L ] = 6;
        s_Black.cnt[ 0x1000L ] = 6;

        s_Black.table[ 0x1400L ] = 8;
        s_Black.table[ 0x1000L ] = 9;

        s_Black.cnt[ 0x0800L ] = 7;
        s_Black.cnt[ 0x0a00L ] = 7;
        s_Black.cnt[ 0x0e00L ] = 7;

        s_Black.table[ 0x0800L ] = 10;
        s_Black.table[ 0x0a00L ] = 11;
        s_Black.table[ 0x0e00L ] = 12;

        s_Black.cnt[ 0x0400L ] = 8;
        s_Black.cnt[ 0x0700L ] = 8;

        s_Black.table[ 0x0400L ] = 13;
        s_Black.table[ 0x0700L ] = 14;

        s_Black.cnt[ 0x0c00L ] = 9;

        s_Black.table[ 0x0c00L ] = 15;

        s_Black.cnt[ 0x0dc0L ] = 10;
        s_Black.cnt[ 0x05c0L ] = 10;
        s_Black.cnt[ 0x0600L ] = 10;
        s_Black.cnt[ 0x0200L ] = 10;
        s_Black.cnt[ 0x03c0L ] = 10;

        s_Black.table[ 0x0dc0L ] = 0;
        s_Black.table[ 0x05c0L ] = 16;
        s_Black.table[ 0x0600L ] = 17;
        s_Black.table[ 0x0200L ] = 18;
        s_Black.table[ 0x03c0L ] = 64;

        s_Black.cnt[ 0x0ce0L ] = 11;
        s_Black.cnt[ 0x0d00L ] = 11;
        s_Black.cnt[ 0x0d80L ] = 11;
        s_Black.cnt[ 0x06e0L ] = 11;
        s_Black.cnt[ 0x0500L ] = 11;
        s_Black.cnt[ 0x02e0L ] = 11;
        s_Black.cnt[ 0x0300L ] = 11;
        s_Black.cnt[ 0x0100L ] = 11;
        s_Black.cnt[ 0x0180L ] = 11;
        s_Black.cnt[ 0x01a0L ] = 11;

        s_Black.table[ 0x0ce0L ] = 19;
        s_Black.table[ 0x0d00L ] = 20;
        s_Black.table[ 0x0d80L ] = 21;
        s_Black.table[ 0x06e0L ] = 22;
        s_Black.table[ 0x0500L ] = 23;
        s_Black.table[ 0x02e0L ] = 24;
        s_Black.table[ 0x0300L ] = 25;
        s_Black.table[ 0x0100L ] = 1792;
        s_Black.table[ 0x0180L ] = 1856;
        s_Black.table[ 0x01a0L ] = 1920;

        s_Black.cnt[ 0x0ca0L ] = 12;
        s_Black.cnt[ 0x0cb0L ] = 12;
        s_Black.cnt[ 0x0cc0L ] = 12;
        s_Black.cnt[ 0x0cd0L ] = 12;
        s_Black.cnt[ 0x0680L ] = 12;
        s_Black.cnt[ 0x0690L ] = 12;
        s_Black.cnt[ 0x06a0L ] = 12;
        s_Black.cnt[ 0x06b0L ] = 12;
        s_Black.cnt[ 0x0d20L ] = 12;
        s_Black.cnt[ 0x0d30L ] = 12;
        s_Black.cnt[ 0x0d40L ] = 12;
        s_Black.cnt[ 0x0d50L ] = 12;
        s_Black.cnt[ 0x0d60L ] = 12;
        s_Black.cnt[ 0x0d70L ] = 12;
        s_Black.cnt[ 0x06c0L ] = 12;
        s_Black.cnt[ 0x06d0L ] = 12;
        s_Black.cnt[ 0x0da0L ] = 12;
        s_Black.cnt[ 0x0db0L ] = 12;
        s_Black.cnt[ 0x0540L ] = 12;
        s_Black.cnt[ 0x0550L ] = 12;
        s_Black.cnt[ 0x0560L ] = 12;
        s_Black.cnt[ 0x0570L ] = 12;
        s_Black.cnt[ 0x0640L ] = 12;
        s_Black.cnt[ 0x0650L ] = 12;
        s_Black.cnt[ 0x0520L ] = 12;
        s_Black.cnt[ 0x0530L ] = 12;
        s_Black.cnt[ 0x0240L ] = 12;
        s_Black.cnt[ 0x0370L ] = 12;
        s_Black.cnt[ 0x0380L ] = 12;
        s_Black.cnt[ 0x0270L ] = 12;
        s_Black.cnt[ 0x0280L ] = 12;
        s_Black.cnt[ 0x0580L ] = 12;
        s_Black.cnt[ 0x0590L ] = 12;
        s_Black.cnt[ 0x02b0L ] = 12;
        s_Black.cnt[ 0x02c0L ] = 12;
        s_Black.cnt[ 0x05a0L ] = 12;
        s_Black.cnt[ 0x0660L ] = 12;
        s_Black.cnt[ 0x0670L ] = 12;
        s_Black.cnt[ 0x0c80L ] = 12;
        s_Black.cnt[ 0x0c90L ] = 12;
        s_Black.cnt[ 0x05b0L ] = 12;
        s_Black.cnt[ 0x0330L ] = 12;
        s_Black.cnt[ 0x0340L ] = 12;
        s_Black.cnt[ 0x0350L ] = 12;
        s_Black.cnt[ 0x0120L ] = 12;
        s_Black.cnt[ 0x0130L ] = 12;
        s_Black.cnt[ 0x0140L ] = 12;
        s_Black.cnt[ 0x0150L ] = 12;
        s_Black.cnt[ 0x0160L ] = 12;
        s_Black.cnt[ 0x0170L ] = 12;
        s_Black.cnt[ 0x01c0L ] = 12;
        s_Black.cnt[ 0x01d0L ] = 12;
        s_Black.cnt[ 0x01e0L ] = 12;
        s_Black.cnt[ 0x01f0L ] = 12;

        s_Black.table[ 0x0ca0L ] = 26;
        s_Black.table[ 0x0cb0L ] = 27;
        s_Black.table[ 0x0cc0L ] = 28;
        s_Black.table[ 0x0cd0L ] = 29;
        s_Black.table[ 0x0680L ] = 30;
        s_Black.table[ 0x0690L ] = 31;
        s_Black.table[ 0x06a0L ] = 32;
        s_Black.table[ 0x06b0L ] = 33;
        s_Black.table[ 0x0d20L ] = 34;
        s_Black.table[ 0x0d30L ] = 35;
        s_Black.table[ 0x0d40L ] = 36;
        s_Black.table[ 0x0d50L ] = 37;
        s_Black.table[ 0x0d60L ] = 38;
        s_Black.table[ 0x0d70L ] = 39;
        s_Black.table[ 0x06c0L ] = 40;
        s_Black.table[ 0x06d0L ] = 41;
        s_Black.table[ 0x0da0L ] = 42;
        s_Black.table[ 0x0db0L ] = 43;
        s_Black.table[ 0x0540L ] = 44;
        s_Black.table[ 0x0550L ] = 45;
        s_Black.table[ 0x0560L ] = 46;
        s_Black.table[ 0x0570L ] = 47;
        s_Black.table[ 0x0640L ] = 48;
        s_Black.table[ 0x0650L ] = 49;
        s_Black.table[ 0x0520L ] = 50;
        s_Black.table[ 0x0530L ] = 51;
        s_Black.table[ 0x0240L ] = 52;
        s_Black.table[ 0x0370L ] = 53;
        s_Black.table[ 0x0380L ] = 54;
        s_Black.table[ 0x0270L ] = 55;
        s_Black.table[ 0x0280L ] = 56;
        s_Black.table[ 0x0580L ] = 57;
        s_Black.table[ 0x0590L ] = 58;
        s_Black.table[ 0x02b0L ] = 59;
        s_Black.table[ 0x02c0L ] = 60;
        s_Black.table[ 0x05a0L ] = 61;
        s_Black.table[ 0x0660L ] = 62;
        s_Black.table[ 0x0670L ] = 63;
        s_Black.table[ 0x0c80L ] = 128;
        s_Black.table[ 0x0c90L ] = 192;
        s_Black.table[ 0x05b0L ] = 256;
        s_Black.table[ 0x0330L ] = 320;
        s_Black.table[ 0x0340L ] = 384;
        s_Black.table[ 0x0350L ] = 448;
        s_Black.table[ 0x0120L ] = 1984;
        s_Black.table[ 0x0130L ] = 2048;
        s_Black.table[ 0x0140L ] = 2112;
        s_Black.table[ 0x0150L ] = 2176;
        s_Black.table[ 0x0160L ] = 2240;
        s_Black.table[ 0x0170L ] = 2304;
        s_Black.table[ 0x01c0L ] = 2368;
        s_Black.table[ 0x01d0L ] = 2432;
        s_Black.table[ 0x01e0L ] = 2496;
        s_Black.table[ 0x01f0L ] = 2560;

        s_Black.cnt[ 0x0360L ] = 13;
        s_Black.cnt[ 0x0368L ] = 13;
        s_Black.cnt[ 0x0250L ] = 13;
        s_Black.cnt[ 0x0258L ] = 13;
        s_Black.cnt[ 0x0260L ] = 13;
        s_Black.cnt[ 0x0268L ] = 13;
        s_Black.cnt[ 0x0390L ] = 13;
        s_Black.cnt[ 0x0398L ] = 13;
        s_Black.cnt[ 0x03a0L ] = 13;
        s_Black.cnt[ 0x03a8L ] = 13;
        s_Black.cnt[ 0x03b0L ] = 13;
        s_Black.cnt[ 0x03b8L ] = 13;
        s_Black.cnt[ 0x0290L ] = 13;
        s_Black.cnt[ 0x0298L ] = 13;
        s_Black.cnt[ 0x02a0L ] = 13;
        s_Black.cnt[ 0x02a8L ] = 13;
        s_Black.cnt[ 0x02d0L ] = 13;
        s_Black.cnt[ 0x02d8L ] = 13;
        s_Black.cnt[ 0x0320L ] = 13;
        s_Black.cnt[ 0x0328L ] = 13;

        s_Black.table[ 0x0360L ] = 512;
        s_Black.table[ 0x0368L ] = 576;
        s_Black.table[ 0x0250L ] = 640;
        s_Black.table[ 0x0258L ] = 704;
        s_Black.table[ 0x0260L ] = 768;
        s_Black.table[ 0x0268L ] = 832;
        s_Black.table[ 0x0390L ] = 896;
        s_Black.table[ 0x0398L ] = 960;
        s_Black.table[ 0x03a0L ] = 1024;
        s_Black.table[ 0x03a8L ] = 1088;
        s_Black.table[ 0x03b0L ] = 1152;
        s_Black.table[ 0x03b8L ] = 1216;
        s_Black.table[ 0x0290L ] = 1280;
        s_Black.table[ 0x0298L ] = 1344;
        s_Black.table[ 0x02a0L ] = 1408;
        s_Black.table[ 0x02a8L ] = 1472;
        s_Black.table[ 0x02d0L ] = 1536;
        s_Black.table[ 0x02d8L ] = 1600;
        s_Black.table[ 0x0320L ] = 1664;
        s_Black.table[ 0x0328L ] = 1728;
        }

    unsigned char  ccittrleMask = m_ccittrleMask;
    unsigned char  ccittrleWord = m_ccittrleWord;
    unsigned short word   = 0;
    unsigned short wmask  = 0x8000L;

    for (char bit(1); bit <= 13; ++bit)
        {
        if (!(ccittrleMask >>= 1))
            {
            if(m_ccittrleBufferIndex >= m_ccittrleBufferSize)
                {
                m_ccittrleError = CCITTRLE_ERROR_NO_MORE_INPUT_DATA;
                m_code  = 0;
                return;
                }
#ifdef SUPPORT_REVERSE_BIT_ONLY
            ccittrleWord = BitRevTable[m_ccittrleBuffer[m_ccittrleBufferIndex]];
            ++m_ccittrleBufferIndex;
#else
            ccittrleWord = m_ccittrleBuffer[m_ccittrleBufferIndex];
            ++m_ccittrleBufferIndex;
            if (m_bitrevtable)
                {
                ccittrleWord = BitRevTable[ ccittrleWord ];
                }
#endif
            ccittrleMask = 0x80;
            }
        if (ccittrleWord & ccittrleMask)
            {
            word |= wmask;
            }
        wmask >>= 1;

        if (s_Black.cnt[ word ] == bit)
            {
            m_code = s_Black.table[ word ];
            m_ccittrleMask = ccittrleMask;
            m_ccittrleWord = ccittrleWord;
            return;
            }
        }

    m_ccittrleError = CCITTRLE_ERROR_INVALID_CODE;
    m_code  = 0;
    }

//-----------------------------------------------------------------------------
// public
// Output bits from word to output buffer
//-----------------------------------------------------------------------------
void HCDCodecCCITTRLE::CCITTRLEState::OutputBits(long word, long nbits)
    {
    long wmask;
    unsigned char ccittrleWord, ccittrleMask;

    ccittrleWord = m_ccittrleWord;
    ccittrleMask = m_ccittrleMask;

    wmask = 0x8000L;

    for (long i(0); i<nbits; ++i)
        {
        if (word & wmask)
            {
            ccittrleWord |= ccittrleMask;
            }
        wmask >>= 1;
        if (!(ccittrleMask >>= 1))
            {
            HASSERT(m_ccittrleBufferIndex < m_ccittrleBufferSize);
#ifdef SUPPORT_REVERSE_BIT_ONLY
            m_ccittrleBuffer[ m_ccittrleBufferIndex ] = BitRevTable[ ccittrleWord ];
            ++m_ccittrleBufferIndex;
#else
            if (m_bitrevtable)
                {
                m_ccittrleBuffer[ m_ccittrleBufferIndex ] = BitRevTable[ ccittrleWord ];
                }
            else
                {
                m_ccittrleBuffer[ m_ccittrleBufferIndex ] = ccittrleWord;
                }
            ++m_ccittrleBufferIndex;
#endif

            ccittrleWord = 0;
            ccittrleMask = 0x80;
            }
        }

    m_ccittrleWord = ccittrleWord;
    m_ccittrleMask = ccittrleMask;
    }
/**************************************************************************/

//-----------------------------------------------------------------------------
// public
// Output code for white run length
// long value;    // length of run length
//-----------------------------------------------------------------------------
void HCDCodecCCITTRLE::CCITTRLEState::CodeWhite(long value)
    {
    long makeup;

    static const long white[ 64 ] = {
        0x3500L, /*  0, 0011 0101 ---- ---- */
        0x1c00L, /*  1, 0001 11-- ---- ---- */
        0x7000L, /*  2, 0111 ---- ---- ---- */
        0x8000L, /*  3, 1000 ---- ---- ---- */
        0xb000L, /*  4, 1011 ---- ---- ---- */
        0xc000L, /*  5, 1100 ---- ---- ---- */
        0xe000L, /*  6, 1110 ---- ---- ---- */
        0xf000L, /*  7, 1111 ---- ---- ---- */
        0x9800L, /*  8, 1001 1--- ---- ---- */
        0xa000L, /*  9, 1010 0--- ---- ---- */
        0x3800L, /* 10, 0011 1--- ---- ---- */
        0x4000L, /* 11, 0100 0--- ---- ---- */
        0x2000L, /* 12, 0010 00-- ---- ---- */
        0x0c00L, /* 13, 0000 11-- ---- ---- */
        0xd000L, /* 14, 1101 00-- ---- ---- */
        0xd400L, /* 15, 1101 01-- ---- ---- */
        0xa800L, /* 16, 1010 10-- ---- ---- */
        0xac00L, /* 17, 1010 11-- ---- ---- */
        0x4e00L, /* 18, 0100 111- ---- ---- */
        0x1800L, /* 19, 0001 100- ---- ---- */
        0x1000L, /* 20, 0001 000- ---- ---- */
        0x2e00L, /* 21, 0010 111- ---- ---- */
        0x0600L, /* 22, 0000 011- ---- ---- */
        0x0800L, /* 23, 0000 100- ---- ---- */
        0x5000L, /* 24, 0101 000- ---- ---- */
        0x5600L, /* 25, 0101 011- ---- ---- */
        0x2600L, /* 26, 0010 011- ---- ---- */
        0x4800L, /* 27, 0100 100- ---- ---- */
        0x3000L, /* 28, 0011 000- ---- ---- */
        0x0200L, /* 29, 0000 0010 ---- ---- */
        0x0300L, /* 30, 0000 0011 ---- ---- */
        0x1a00L, /* 31, 0001 1010 ---- ---- */
        0x1b00L, /* 32, 0001 1011 ---- ---- */
        0x1200L, /* 33, 0001 0010 ---- ---- */
        0x1300L, /* 34, 0001 0011 ---- ---- */
        0x1400L, /* 35, 0001 0100 ---- ---- */
        0x1500L, /* 36, 0001 0101 ---- ---- */
        0x1600L, /* 37, 0001 0110 ---- ---- */
        0x1700L, /* 38, 0001 0111 ---- ---- */
        0x2800L, /* 39, 0010 1000 ---- ---- */
        0x2900L, /* 40, 0010 1001 ---- ---- */
        0x2a00L, /* 41, 0010 1010 ---- ---- */
        0x2b00L, /* 42, 0010 1011 ---- ---- */
        0x2c00L, /* 43, 0010 1100 ---- ---- */
        0x2d00L, /* 44, 0010 1101 ---- ---- */
        0x0400L, /* 45, 0000 0100 ---- ---- */
        0x0500L, /* 46, 0000 0101 ---- ---- */
        0x0a00L, /* 47, 0000 1010 ---- ---- */
        0x0b00L, /* 48, 0000 1011 ---- ---- */
        0x5200L, /* 49, 0101 0010 ---- ---- */
        0x5300L, /* 50, 0101 0011 ---- ---- */
        0x5400L, /* 51, 0101 0100 ---- ---- */
        0x5500L, /* 52, 0101 0101 ---- ---- */
        0x2400L, /* 53, 0010 0100 ---- ---- */
        0x2500L, /* 54, 0010 0101 ---- ---- */
        0x5800L, /* 55, 0101 1000 ---- ---- */
        0x5900L, /* 56, 0101 1001 ---- ---- */
        0x5a00L, /* 57, 0101 1010 ---- ---- */
        0x5b00L, /* 58, 0101 1011 ---- ---- */
        0x4a00L, /* 59, 0100 1010 ---- ---- */
        0x4b00L, /* 60, 0100 1011 ---- ---- */
        0x3200L, /* 61, 0011 0010 ---- ---- */
        0x3300L, /* 62, 0011 0011 ---- ---- */
        0x3400L  /* 63, 0011 0100 ---- ---- */
        };

    static const long lwhite[ 64 ] = {
        8, /*  0 */
        6, /*  1 */
        4, /*  2 */
        4, /*  3 */
        4, /*  4 */
        4, /*  5 */
        4, /*  6 */
        4, /*  7 */
        5, /*  8 */
        5, /*  9 */
        5, /* 10 */
        5, /* 11 */
        6, /* 12 */
        6, /* 13 */
        6, /* 14 */
        6, /* 15 */
        6, /* 16 */
        6, /* 17 */
        7, /* 18 */
        7, /* 19 */
        7, /* 20 */
        7, /* 21 */
        7, /* 22 */
        7, /* 23 */
        7, /* 24 */
        7, /* 25 */
        7, /* 26 */
        7, /* 27 */
        7, /* 28 */
        8, /* 29 */
        8, /* 30 */
        8, /* 31 */
        8, /* 32 */
        8, /* 33 */
        8, /* 34 */
        8, /* 35 */
        8, /* 36 */
        8, /* 37 */
        8, /* 38 */
        8, /* 39 */
        8, /* 40 */
        8, /* 41 */
        8, /* 42 */
        8, /* 43 */
        8, /* 44 */
        8, /* 45 */
        8, /* 46 */
        8, /* 47 */
        8, /* 48 */
        8, /* 49 */
        8, /* 50 */
        8, /* 51 */
        8, /* 52 */
        8, /* 53 */
        8, /* 54 */
        8, /* 55 */
        8, /* 56 */
        8, /* 57 */
        8, /* 58 */
        8, /* 59 */
        8, /* 60 */
        8, /* 61 */
        8, /* 62 */
        8  /* 63 */
        };

    static const long mwhite[ 40 ] = {
        0xd800L, /*   64, 1101 1--- ---- ---- */
        0x9000L, /*  128, 1001 0--- ---- ---- */
        0x5c00L, /*  192, 0101 11-- ---- ---- */
        0x6e00L, /*  256, 0110 111- ---- ---- */
        0x3600L, /*  320, 0011 0110 ---- ---- */
        0x3700L, /*  384, 0011 0111 ---- ---- */
        0x6400L, /*  448, 0110 0100 ---- ---- */
        0x6500L, /*  512, 0110 0101 ---- ---- */
        0x6800L, /*  576, 0110 1000 ---- ---- */
        0x6700L, /*  640, 0110 0111 ---- ---- */
        0x6600L, /*  704, 0110 0110 0--- ---- */
        0x6680L, /*  768, 0110 0110 1--- ---- */
        0x6900L, /*  832, 0110 1001 0--- ---- */
        0x6980L, /*  896, 0110 1001 1--- ---- */
        0x6a00L, /*  960, 0110 1010 0--- ---- */
        0x6a80L, /* 1024, 0110 1010 1--- ---- */
        0x6b00L, /* 1088, 0110 1011 0--- ---- */
        0x6b80L, /* 1152, 0110 1011 1--- ---- */
        0x6c00L, /* 1216, 0110 1100 0--- ---- */
        0x6c80L, /* 1280, 0110 1100 1--- ---- */
        0x6d00L, /* 1344, 0110 1101 0--- ---- */
        0x6d80L, /* 1408, 0110 1101 1--- ---- */
        0x4c00L, /* 1472, 0100 1100 0--- ---- */
        0x4c80L, /* 1536, 0100 1100 1--- ---- */
        0x4d00L, /* 1600, 0100 1101 0--- ---- */
        0x6000L, /* 1664, 0110 00-- ---- ---- */
        0x4d80L, /* 1728, 0100 1101 1--- ---- */
        0x0100L, /* 1792, 0000 0001 000- ---- */
        0x0180L, /* 1856, 0000 0001 100- ---- */
        0x01a0L, /* 1920, 0000 0001 101- ---- */
        0x0120L, /* 1984, 0000 0001 0010 ---- */
        0x0130L, /* 2048, 0000 0001 0011 ---- */
        0x0140L, /* 2112, 0000 0001 0100 ---- */
        0x0150L, /* 2176, 0000 0001 0101 ---- */
        0x0160L, /* 2240, 0000 0001 0110 ---- */
        0x0170L, /* 2304, 0000 0001 0111 ---- */
        0x01c0L, /* 2368, 0000 0001 1100 ---- */
        0x01d0L, /* 2432, 0000 0001 1101 ---- */
        0x01e0L, /* 2496, 0000 0001 1110 ---- */
        0x01f0L  /* 2560, 0000 0001 1111 ---- */
        };

    static const long lmwhite[ 40 ] = {
        5, /*   64 */
        5, /*  128 */
        6, /*  192 */
        7, /*  256 */
        8, /*  320 */
        8, /*  384 */
        8, /*  448 */
        8, /*  512 */
        8, /*  576 */
        8, /*  640 */
        9, /*  704 */
        9, /*  768 */
        9, /*  832 */
        9, /*  896 */
        9, /*  960 */
        9, /* 1024 */
        9, /* 1088 */
        9, /* 1152 */
        9, /* 1216 */
        9, /* 1280 */
        9, /* 1344 */
        9, /* 1408 */
        9, /* 1472 */
        9, /* 1536 */
        9, /* 1600 */
        6, /* 1664 */
        9, /* 1728 */
        11, /* 1792 */
        11, /* 1856 */
        11, /* 1920 */
        12, /* 1984 */
        12, /* 2048 */
        12, /* 2112 */
        12, /* 2176 */
        12, /* 2240 */
        12, /* 2304 */
        12, /* 2368 */
        12, /* 2432 */
        12, /* 2496 */
        12  /* 2560 */
        };

    static const long black[ 64 ] = {
        0x0dc0L, /*  0, 0000 1101 11-- ---- */
        0x4000L, /*  1, 010- ---- ---- ---- */
        0xc000L, /*  2, 11-- ---- ---- ---- */
        0x8000L, /*  3, 10-- ---- ---- ---- */
        0x6000L, /*  4, 011- ---- ---- ---- */
        0x3000L, /*  5, 0011 ---- ---- ---- */
        0x2000L, /*  6, 0010 ---- ---- ---- */
        0x1800L, /*  7, 0001 1--- ---- ---- */
        0x1400L, /*  8, 0001 01-- ---- ---- */
        0x1000L, /*  9, 0001 00-- ---- ---- */
        0x0800L, /* 10, 0000 100- ---- ---- */
        0x0a00L, /* 11, 0000 101- ---- ---- */
        0x0e00L, /* 12, 0000 111- ---- ---- */
        0x0400L, /* 13, 0000 0100 ---- ---- */
        0x0700L, /* 14, 0000 0111 ---- ---- */
        0x0c00L, /* 15, 0000 1100 0--- ---- */
        0x05c0L, /* 16, 0000 0101 11-- ---- */
        0x0600L, /* 17, 0000 0110 00-- ---- */
        0x0200L, /* 18, 0000 0010 00-- ---- */
        0x0ce0L, /* 19, 0000 1100 111- ---- */
        0x0d00L, /* 20, 0000 1101 000- ---- */
        0x0d80L, /* 21, 0000 1101 100- ---- */
        0x06e0L, /* 22, 0000 0110 111- ---- */
        0x0500L, /* 23, 0000 0101 000- ---- */
        0x02e0L, /* 24, 0000 0010 111- ---- */
        0x0300L, /* 25, 0000 0011 000- ---- */
        0x0ca0L, /* 26, 0000 1100 1010 ---- */
        0x0cb0L, /* 27, 0000 1100 1011 ---- */
        0x0cc0L, /* 28, 0000 1100 1100 ---- */
        0x0cd0L, /* 29, 0000 1100 1101 ---- */
        0x0680L, /* 30, 0000 0110 1000 ---- */
        0x0690L, /* 31, 0000 0110 1001 ---- */
        0x06a0L, /* 32, 0000 0110 1010 ---- */
        0x06b0L, /* 33, 0000 0110 1011 ---- */
        0x0d20L, /* 34, 0000 1101 0010 ---- */
        0x0d30L, /* 35, 0000 1101 0011 ---- */
        0x0d40L, /* 36, 0000 1101 0100 ---- */
        0x0d50L, /* 37, 0000 1101 0101 ---- */
        0x0d60L, /* 38, 0000 1101 0110 ---- */
        0x0d70L, /* 39, 0000 1101 0111 ---- */
        0x06c0L, /* 40, 0000 0110 1100 ---- */
        0x06d0L, /* 41, 0000 0110 1101 ---- */
        0x0da0L, /* 42, 0000 1101 1010 ---- */
        0x0db0L, /* 43, 0000 1101 1011 ---- */
        0x0540L, /* 44, 0000 0101 0100 ---- */
        0x0550L, /* 45, 0000 0101 0101 ---- */
        0x0560L, /* 46, 0000 0101 0110 ---- */
        0x0570L, /* 47, 0000 0101 0111 ---- */
        0x0640L, /* 48, 0000 0110 0100 ---- */
        0x0650L, /* 49, 0000 0110 0101 ---- */
        0x0520L, /* 50, 0000 0101 0010 ---- */
        0x0530L, /* 51, 0000 0101 0011 ---- */
        0x0240L, /* 52, 0000 0010 0100 ---- */
        0x0370L, /* 53, 0000 0011 0111 ---- */
        0x0380L, /* 54, 0000 0011 1000 ---- */
        0x0270L, /* 55, 0000 0010 0111 ---- */
        0x0280L, /* 56, 0000 0010 1000 ---- */
        0x0580L, /* 57, 0000 0101 1000 ---- */
        0x0590L, /* 58, 0000 0101 1001 ---- */
        0x02b0L, /* 59, 0000 0010 1011 ---- */
        0x02c0L, /* 60, 0000 0010 1100 ---- */
        0x05a0L, /* 61, 0000 0101 1010 ---- */
        0x0660L, /* 62, 0000 0110 0110 ---- */
        0x0670L  /* 63, 0000 0110 0111 ---- */
        };

    static const long lblack[ 64 ] = {
        10, /*  0 */
        3, /*  1 */
        2, /*  2 */
        2, /*  3 */
        3, /*  4 */
        4, /*  5 */
        4, /*  6 */
        5, /*  7 */
        6, /*  8 */
        6, /*  9 */
        7, /* 10 */
        7, /* 11 */
        7, /* 12 */
        8, /* 13 */
        8, /* 14 */
        9, /* 15 */
        10, /* 16 */
        10, /* 17 */
        10, /* 18 */
        11, /* 19 */
        11, /* 20 */
        11, /* 21 */
        11, /* 22 */
        11, /* 23 */
        11, /* 24 */
        11, /* 25 */
        12, /* 26 */
        12, /* 27 */
        12, /* 28 */
        12, /* 29 */
        12, /* 30 */
        12, /* 31 */
        12, /* 32 */
        12, /* 33 */
        12, /* 34 */
        12, /* 35 */
        12, /* 36 */
        12, /* 37 */
        12, /* 38 */
        12, /* 39 */
        12, /* 40 */
        12, /* 41 */
        12, /* 42 */
        12, /* 43 */
        12, /* 44 */
        12, /* 45 */
        12, /* 46 */
        12, /* 47 */
        12, /* 48 */
        12, /* 49 */
        12, /* 50 */
        12, /* 51 */
        12, /* 52 */
        12, /* 53 */
        12, /* 54 */
        12, /* 55 */
        12, /* 56 */
        12, /* 57 */
        12, /* 58 */
        12, /* 59 */
        12, /* 60 */
        12, /* 61 */
        12, /* 62 */
        12  /* 63 */
        };

    if (value <= 63)
        {
        OutputBits(white[ value ], lwhite[ value ]);
        return;
        }

    if (value <= 2623)
        {
        makeup = value >> 6L;
        value = value - (makeup << 6L);
        OutputBits(mwhite[ makeup-1 ], lmwhite[ makeup-1 ]);
        OutputBits(white[ value ], lwhite[ value ]);
        return;
        }

    while (value > 2623)
        {
        OutputBits(mwhite[39],lmwhite[39]);
        OutputBits(white[63],lwhite[63]);
        OutputBits(black[0],lblack[0]);
        value = value - 2623;
        }

    if (value <= 63)
        {
        OutputBits(white[ value ], lwhite[ value ]);
        return;
        }

    makeup = value >> 6L;
    value = value - (makeup << 6L);
    OutputBits(mwhite[ makeup-1 ], lmwhite[ makeup-1 ]);
    OutputBits(white[ value ], lwhite[ value ]);
    }


/**************************************************************************/

//-----------------------------------------------------------------------------
// public
// Output code for black run length
// long value;    // length of run length
//-----------------------------------------------------------------------------
void HCDCodecCCITTRLE::CCITTRLEState::CodeBlack(long value)
    {
    long makeup;

    static long black[ 64 ] = {
        0x0dc0L, /*  0, 0000 1101 11-- ---- */
        0x4000L, /*  1, 010- ---- ---- ---- */
        0xc000L, /*  2, 11-- ---- ---- ---- */
        0x8000L, /*  3, 10-- ---- ---- ---- */
        0x6000L, /*  4, 011- ---- ---- ---- */
        0x3000L, /*  5, 0011 ---- ---- ---- */
        0x2000L, /*  6, 0010 ---- ---- ---- */
        0x1800L, /*  7, 0001 1--- ---- ---- */
        0x1400L, /*  8, 0001 01-- ---- ---- */
        0x1000L, /*  9, 0001 00-- ---- ---- */
        0x0800L, /* 10, 0000 100- ---- ---- */
        0x0a00L, /* 11, 0000 101- ---- ---- */
        0x0e00L, /* 12, 0000 111- ---- ---- */
        0x0400L, /* 13, 0000 0100 ---- ---- */
        0x0700L, /* 14, 0000 0111 ---- ---- */
        0x0c00L, /* 15, 0000 1100 0--- ---- */
        0x05c0L, /* 16, 0000 0101 11-- ---- */
        0x0600L, /* 17, 0000 0110 00-- ---- */
        0x0200L, /* 18, 0000 0010 00-- ---- */
        0x0ce0L, /* 19, 0000 1100 111- ---- */
        0x0d00L, /* 20, 0000 1101 000- ---- */
        0x0d80L, /* 21, 0000 1101 100- ---- */
        0x06e0L, /* 22, 0000 0110 111- ---- */
        0x0500L, /* 23, 0000 0101 000- ---- */
        0x02e0L, /* 24, 0000 0010 111- ---- */
        0x0300L, /* 25, 0000 0011 000- ---- */
        0x0ca0L, /* 26, 0000 1100 1010 ---- */
        0x0cb0L, /* 27, 0000 1100 1011 ---- */
        0x0cc0L, /* 28, 0000 1100 1100 ---- */
        0x0cd0L, /* 29, 0000 1100 1101 ---- */
        0x0680L, /* 30, 0000 0110 1000 ---- */
        0x0690L, /* 31, 0000 0110 1001 ---- */
        0x06a0L, /* 32, 0000 0110 1010 ---- */
        0x06b0L, /* 33, 0000 0110 1011 ---- */
        0x0d20L, /* 34, 0000 1101 0010 ---- */
        0x0d30L, /* 35, 0000 1101 0011 ---- */
        0x0d40L, /* 36, 0000 1101 0100 ---- */
        0x0d50L, /* 37, 0000 1101 0101 ---- */
        0x0d60L, /* 38, 0000 1101 0110 ---- */
        0x0d70L, /* 39, 0000 1101 0111 ---- */
        0x06c0L, /* 40, 0000 0110 1100 ---- */
        0x06d0L, /* 41, 0000 0110 1101 ---- */
        0x0da0L, /* 42, 0000 1101 1010 ---- */
        0x0db0L, /* 43, 0000 1101 1011 ---- */
        0x0540L, /* 44, 0000 0101 0100 ---- */
        0x0550L, /* 45, 0000 0101 0101 ---- */
        0x0560L, /* 46, 0000 0101 0110 ---- */
        0x0570L, /* 47, 0000 0101 0111 ---- */
        0x0640L, /* 48, 0000 0110 0100 ---- */
        0x0650L, /* 49, 0000 0110 0101 ---- */
        0x0520L, /* 50, 0000 0101 0010 ---- */
        0x0530L, /* 51, 0000 0101 0011 ---- */
        0x0240L, /* 52, 0000 0010 0100 ---- */
        0x0370L, /* 53, 0000 0011 0111 ---- */
        0x0380L, /* 54, 0000 0011 1000 ---- */
        0x0270L, /* 55, 0000 0010 0111 ---- */
        0x0280L, /* 56, 0000 0010 1000 ---- */
        0x0580L, /* 57, 0000 0101 1000 ---- */
        0x0590L, /* 58, 0000 0101 1001 ---- */
        0x02b0L, /* 59, 0000 0010 1011 ---- */
        0x02c0L, /* 60, 0000 0010 1100 ---- */
        0x05a0L, /* 61, 0000 0101 1010 ---- */
        0x0660L, /* 62, 0000 0110 0110 ---- */
        0x0670L  /* 63, 0000 0110 0111 ---- */
        };

    static long lblack[ 64 ] = {
        10, /*  0 */
        3, /*  1 */
        2, /*  2 */
        2, /*  3 */
        3, /*  4 */
        4, /*  5 */
        4, /*  6 */
        5, /*  7 */
        6, /*  8 */
        6, /*  9 */
        7, /* 10 */
        7, /* 11 */
        7, /* 12 */
        8, /* 13 */
        8, /* 14 */
        9, /* 15 */
        10, /* 16 */
        10, /* 17 */
        10, /* 18 */
        11, /* 19 */
        11, /* 20 */
        11, /* 21 */
        11, /* 22 */
        11, /* 23 */
        11, /* 24 */
        11, /* 25 */
        12, /* 26 */
        12, /* 27 */
        12, /* 28 */
        12, /* 29 */
        12, /* 30 */
        12, /* 31 */
        12, /* 32 */
        12, /* 33 */
        12, /* 34 */
        12, /* 35 */
        12, /* 36 */
        12, /* 37 */
        12, /* 38 */
        12, /* 39 */
        12, /* 40 */
        12, /* 41 */
        12, /* 42 */
        12, /* 43 */
        12, /* 44 */
        12, /* 45 */
        12, /* 46 */
        12, /* 47 */
        12, /* 48 */
        12, /* 49 */
        12, /* 50 */
        12, /* 51 */
        12, /* 52 */
        12, /* 53 */
        12, /* 54 */
        12, /* 55 */
        12, /* 56 */
        12, /* 57 */
        12, /* 58 */
        12, /* 59 */
        12, /* 60 */
        12, /* 61 */
        12, /* 62 */
        12  /* 63 */
        };

    static long mblack[ 40 ] = {
        0x03c0L, /*   64, 0000 0011 11-- ---- */
        0x0c80L, /*  128, 0000 1100 1000 ---- */
        0x0c90L, /*  192, 0000 1100 1001 ---- */
        0x05b0L, /*  256, 0000 0101 1011 ---- */
        0x0330L, /*  320, 0000 0011 0011 ---- */
        0x0340L, /*  384, 0000 0011 0100 ---- */
        0x0350L, /*  448, 0000 0011 0101 ---- */
        0x0360L, /*  512, 0000 0011 0110 0--- */
        0x0368L, /*  576, 0000 0011 0110 1--- */
        0x0250L, /*  640, 0000 0010 0101 0--- */
        0x0258L, /*  704, 0000 0010 0101 1--- */
        0x0260L, /*  768, 0000 0010 0110 0--- */
        0x0268L, /*  832, 0000 0010 0110 1--- */
        0x0390L, /*  896, 0000 0011 1001 0--- */
        0x0398L, /*  960, 0000 0011 1001 1--- */
        0x03a0L, /* 1024, 0000 0011 1010 0--- */
        0x03a8L, /* 1088, 0000 0011 1010 1--- */
        0x03b0L, /* 1152, 0000 0011 1011 0--- */
        0x03b8L, /* 1216, 0000 0011 1011 1--- */
        0x0290L, /* 1280, 0000 0010 1001 0--- */
        0x0298L, /* 1344, 0000 0010 1001 1--- */
        0x02a0L, /* 1408, 0000 0010 1010 0--- */
        0x02a8L, /* 1472, 0000 0010 1010 1--- */
        0x02d0L, /* 1536, 0000 0010 1101 0--- */
        0x02d8L, /* 1600, 0000 0010 1101 1--- */
        0x0320L, /* 1664, 0000 0011 0010 0--- */
        0x0328L, /* 1728, 0000 0011 0010 1--- */
        0x0100L, /* 1792, 0000 0001 000- ---- */
        0x0180L, /* 1856, 0000 0001 100- ---- */
        0x01a0L, /* 1920, 0000 0001 101- ---- */
        0x0120L, /* 1984, 0000 0001 0010 ---- */
        0x0130L, /* 2048, 0000 0001 0011 ---- */
        0x0140L, /* 2112, 0000 0001 0100 ---- */
        0x0150L, /* 2176, 0000 0001 0101 ---- */
        0x0160L, /* 2240, 0000 0001 0110 ---- */
        0x0170L, /* 2304, 0000 0001 0111 ---- */
        0x01c0L, /* 2368, 0000 0001 1100 ---- */
        0x01d0L, /* 2432, 0000 0001 1101 ---- */
        0x01e0L, /* 2496, 0000 0001 1110 ---- */
        0x01f0L  /* 2560, 0000 0001 1111 ---- */
        };

    static long lmblack[ 40 ] = {
        10, /*   64 */
        12, /*  128 */
        12, /*  192 */
        12, /*  256 */
        12, /*  320 */
        12, /*  384 */
        12, /*  448 */
        13, /*  512 */
        13, /*  576 */
        13, /*  640 */
        13, /*  704 */
        13, /*  768 */
        13, /*  832 */
        13, /*  896 */
        13, /*  960 */
        13, /* 1024 */
        13, /* 1088 */
        13, /* 1152 */
        13, /* 1216 */
        13, /* 1280 */
        13, /* 1344 */
        13, /* 1408 */
        13, /* 1472 */
        13, /* 1536 */
        13, /* 1600 */
        13, /* 1664 */
        13, /* 1728 */
        11, /* 1792 */
        11, /* 1856 */
        11, /* 1920 */
        12, /* 1984 */
        12, /* 2048 */
        12, /* 2112 */
        12, /* 2176 */
        12, /* 2240 */
        12, /* 2304 */
        12, /* 2368 */
        12, /* 2432 */
        12, /* 2496 */
        12  /* 2560 */
        };

    static long white[ 64 ] = {
        0x3500L, /*  0, 0011 0101 ---- ---- */
        0x1c00L, /*  1, 0001 11-- ---- ---- */
        0x7000L, /*  2, 0111 ---- ---- ---- */
        0x8000L, /*  3, 1000 ---- ---- ---- */
        0xb000L, /*  4, 1011 ---- ---- ---- */
        0xc000L, /*  5, 1100 ---- ---- ---- */
        0xe000L, /*  6, 1110 ---- ---- ---- */
        0xf000L, /*  7, 1111 ---- ---- ---- */
        0x9800L, /*  8, 1001 1--- ---- ---- */
        0xa000L, /*  9, 1010 0--- ---- ---- */
        0x3800L, /* 10, 0011 1--- ---- ---- */
        0x4000L, /* 11, 0100 0--- ---- ---- */
        0x2000L, /* 12, 0010 00-- ---- ---- */
        0x0c00L, /* 13, 0000 11-- ---- ---- */
        0xd000L, /* 14, 1101 00-- ---- ---- */
        0xd400L, /* 15, 1101 01-- ---- ---- */
        0xa800L, /* 16, 1010 10-- ---- ---- */
        0xac00L, /* 17, 1010 11-- ---- ---- */
        0x4e00L, /* 18, 0100 111- ---- ---- */
        0x1800L, /* 19, 0001 100- ---- ---- */
        0x1000L, /* 20, 0001 000- ---- ---- */
        0x2e00L, /* 21, 0010 111- ---- ---- */
        0x0600L, /* 22, 0000 011- ---- ---- */
        0x0800L, /* 23, 0000 100- ---- ---- */
        0x5000L, /* 24, 0101 000- ---- ---- */
        0x5600L, /* 25, 0101 011- ---- ---- */
        0x2600L, /* 26, 0010 011- ---- ---- */
        0x4800L, /* 27, 0100 100- ---- ---- */
        0x3000L, /* 28, 0011 000- ---- ---- */
        0x0200L, /* 29, 0000 0010 ---- ---- */
        0x0300L, /* 30, 0000 0011 ---- ---- */
        0x1a00L, /* 31, 0001 1010 ---- ---- */
        0x1b00L, /* 32, 0001 1011 ---- ---- */
        0x1200L, /* 33, 0001 0010 ---- ---- */
        0x1300L, /* 34, 0001 0011 ---- ---- */
        0x1400L, /* 35, 0001 0100 ---- ---- */
        0x1500L, /* 36, 0001 0101 ---- ---- */
        0x1600L, /* 37, 0001 0110 ---- ---- */
        0x1700L, /* 38, 0001 0111 ---- ---- */
        0x2800L, /* 39, 0010 1000 ---- ---- */
        0x2900L, /* 40, 0010 1001 ---- ---- */
        0x2a00L, /* 41, 0010 1010 ---- ---- */
        0x2b00L, /* 42, 0010 1011 ---- ---- */
        0x2c00L, /* 43, 0010 1100 ---- ---- */
        0x2d00L, /* 44, 0010 1101 ---- ---- */
        0x0400L, /* 45, 0000 0100 ---- ---- */
        0x0500L, /* 46, 0000 0101 ---- ---- */
        0x0a00L, /* 47, 0000 1010 ---- ---- */
        0x0b00L, /* 48, 0000 1011 ---- ---- */
        0x5200L, /* 49, 0101 0010 ---- ---- */
        0x5300L, /* 50, 0101 0011 ---- ---- */
        0x5400L, /* 51, 0101 0100 ---- ---- */
        0x5500L, /* 52, 0101 0101 ---- ---- */
        0x2400L, /* 53, 0010 0100 ---- ---- */
        0x2500L, /* 54, 0010 0101 ---- ---- */
        0x5800L, /* 55, 0101 1000 ---- ---- */
        0x5900L, /* 56, 0101 1001 ---- ---- */
        0x5a00L, /* 57, 0101 1010 ---- ---- */
        0x5b00L, /* 58, 0101 1011 ---- ---- */
        0x4a00L, /* 59, 0100 1010 ---- ---- */
        0x4b00L, /* 60, 0100 1011 ---- ---- */
        0x3200L, /* 61, 0011 0010 ---- ---- */
        0x3300L, /* 62, 0011 0011 ---- ---- */
        0x3400L  /* 63, 0011 0100 ---- ---- */
        };

    static long lwhite[ 64 ] = {
        8, /*  0 */
        6, /*  1 */
        4, /*  2 */
        4, /*  3 */
        4, /*  4 */
        4, /*  5 */
        4, /*  6 */
        4, /*  7 */
        5, /*  8 */
        5, /*  9 */
        5, /* 10 */
        5, /* 11 */
        6, /* 12 */
        6, /* 13 */
        6, /* 14 */
        6, /* 15 */
        6, /* 16 */
        6, /* 17 */
        7, /* 18 */
        7, /* 19 */
        7, /* 20 */
        7, /* 21 */
        7, /* 22 */
        7, /* 23 */
        7, /* 24 */
        7, /* 25 */
        7, /* 26 */
        7, /* 27 */
        7, /* 28 */
        8, /* 29 */
        8, /* 30 */
        8, /* 31 */
        8, /* 32 */
        8, /* 33 */
        8, /* 34 */
        8, /* 35 */
        8, /* 36 */
        8, /* 37 */
        8, /* 38 */
        8, /* 39 */
        8, /* 40 */
        8, /* 41 */
        8, /* 42 */
        8, /* 43 */
        8, /* 44 */
        8, /* 45 */
        8, /* 46 */
        8, /* 47 */
        8, /* 48 */
        8, /* 49 */
        8, /* 50 */
        8, /* 51 */
        8, /* 52 */
        8, /* 53 */
        8, /* 54 */
        8, /* 55 */
        8, /* 56 */
        8, /* 57 */
        8, /* 58 */
        8, /* 59 */
        8, /* 60 */
        8, /* 61 */
        8, /* 62 */
        8  /* 63 */
        };

    if (value <= 63)
        {
        OutputBits(black[ value ], lblack[ value ]);
        return;
        }

    if (value <= 2623)
        {
        makeup = value >> 6L;
        value = value - (makeup << 6L);
        OutputBits(mblack[ makeup-1 ], lmblack[ makeup-1 ]);
        OutputBits(black[ value ], lblack[ value ]);
        return;
        }

    while (value > 2623)
        {
        OutputBits(mblack[39],lmblack[39]);
        OutputBits(black[63],lblack[63]);
        OutputBits(white[0],lwhite[0]);
        value = value - 2623;
        }

    if (value <= 63)
        {
        OutputBits(black[ value ], lblack[ value ]);
        return;
        }

    makeup = value >> 6L;
    value = value - (makeup << 6L);
    OutputBits(mblack[ makeup-1 ], lmblack[ makeup-1 ]);
    OutputBits(black[ value ], lblack[ value ]);
    }

//-----------------------------------------------------------------------------
// public
// Decode one line of CCITT.
//-----------------------------------------------------------------------------
void HCDCodecCCITTRLE::CCITTRLEState::DecodeLine()
    {
    HPRECONDITION(m_UncompressBuffer != 0);
    HPRECONDITION(m_UncompressBufferSize != 0);

    Byte* pLineBuffer = &m_UncompressBuffer[m_UncompressBufferIndex];

    m_ccittrleError = CCITTRLE_NOERROR;

    m_lrsCount = 0;

    long x, l, r;
    long color;

    color = 0L;
    r = 0;
    x = 1;
    l = 0;

    m_eol = false;

    for(;;)
        {
        GetCode(color);

        if (m_ccittrleError != CCITTRLE_NOERROR)
            goto ERRORX;

        if (m_eol) // eod
            {
            m_ccittrleError = CCITTRLE_ERROR_READ_DATA;
            goto ERRORX;
            }

        if (color == 0)
            {
            if ((m_code > 0) && (r > 0))
                {
                HASSERT(m_lrsCount < m_max_lrs);
                FillSpan(pLineBuffer, l-1, r - (l- 1));
                ++m_lrsCount;

                r = 0;
                }
            x += m_code;
            color = 1;
            }
        else
            {
            if (m_code > 0)
                {
                if (r == 0)
                    {
                    l = x;
                    }
                x = x + m_code;
                r = x - 1;
                }
            color = 0;
            }

        if (x > m_bstop) break;

        // Make sure we have space for the next LRS
        if (m_lrsCount >= m_max_lrs)
            {
            GrowMaxLRS();
            }
        }

    if (r != 0)
        {
        FillSpan(pLineBuffer, l-1, r - (l- 1));
        ++m_lrsCount;
        }

    // Move index to the next line in the uncompress buffer.
    m_UncompressBufferIndex+=m_UncompressBufferLineSize;

    m_ccittrleMask = 0;

    return;

ERRORX:
    m_lrsCount = 0;
    return;
    }

//-----------------------------------------------------------------------------
// public
// Decode one line of CCITT.
//-----------------------------------------------------------------------------
void HCDCodecCCITTRLE::CCITTRLEState::DecodeLineRLE()
    {
    HPRECONDITION(m_RLEBuffer != 0);
    HPRECONDITION(m_RLEBufferSize);

    long  lastRightValue = 0;

    m_ccittrleError = CCITTRLE_NOERROR;

    m_lrsCount = 0;

    long x, l, r;
    long color;

    color = 0L;
    r = 0;
    x = 1;
    l = 0;

    m_eol = false;

    if(m_invertResult)
        {
        // Fill 0 black will make us start on white.
        m_RLEBuffer[m_RLEBufferIndex++] = 0;
        }

    for(;;)
        {
        GetCode(color);

        if (m_ccittrleError != CCITTRLE_NOERROR)
            goto ERRORX;

        if (m_eol) // eod
            {
            m_ccittrleError = CCITTRLE_ERROR_READ_DATA;
            goto ERRORX;
            }

        if (color == 0)
            {
            if ((m_code > 0) && (r > 0))
                {
                HASSERT(m_lrsCount < m_max_lrs);
                HASSERT(m_lrsCount < m_max_lrs);
                FillBlack(l - lastRightValue - 1);
                FillWhite(r - (l- 1));
                lastRightValue = r;
                ++m_lrsCount;

                r = 0;
                }
            x += m_code;
            color = 1;
            }
        else
            {
            if (m_code > 0)
                {
                if (r == 0)
                    {
                    l = x;
                    }
                x = x + m_code;
                r = x - 1;
                }
            color = 0;
            }

        // Make sure we have space for the next LRS
        if (m_lrsCount >= m_max_lrs)
            {
            GrowMaxLRS();
            }

        if (x > m_bstop) break;
        }

    if (r != 0)
        {
        HASSERT(m_lrsCount < m_max_lrs);
        FillBlack(l - lastRightValue - 1);
        FillWhite(r - (l- 1));
        lastRightValue = r;
        ++m_lrsCount;
        }

ERRORX:

    // Fill remaining black
    FillBlack(m_width - lastRightValue);

    if(m_invertResult)
        {
        // Fill 0 black will make us end on black.
        m_RLEBuffer[m_RLEBufferIndex++] = 0;
        }

    m_ccittrleMask = 0;

    m_lrsCount = 0;
    return;
    }

//-----------------------------------------------------------------------------
// public
// Encode lbuf and rbuf
//-----------------------------------------------------------------------------
void HCDCodecCCITTRLE::CCITTRLEState::EncodeLineFromRLE()
    {
    m_lrsCount = 0;

    m_a[0] = 0;
    m_acolor[0] = 0;
    long apt = 1;

    long PixelsFromRun = 0;

    if(m_invertResult)
        {
        while(PixelsFromRun < m_width)
            {
            // Skip empty runs.
            if(!m_RLEBuffer[m_RLEBufferIndex])
                {
                ++m_RLEBufferIndex;
                continue;
                }

            long StartRun = PixelsFromRun;

            PixelsFromRun += m_RLEBuffer[m_RLEBufferIndex];
            ++m_RLEBufferIndex;

            // Append all pixels of the same state. That take care of 32767,0,123,0,456..
            while(PixelsFromRun < m_width && m_RLEBuffer[m_RLEBufferIndex] == 0)
                {
                PixelsFromRun += m_RLEBuffer[m_RLEBufferIndex+1];
                m_RLEBufferIndex+=2;
                }

            // Encode black in invert mode. Black runs are ON even number. 0,2,4,6...
            if(m_RLEBufferIndex & 0x00000001)    // m_RLEBufferIndex is 1 beyond current state but if next is white that means previous was black.
                {
                HASSERT(StartRun < m_width);

                // Left entry, add one for the CCITT algo
                m_a[apt]      = StartRun + 1;
                m_acolor[apt] = 1;

                // Right entry, add one for the CCITT algo
                m_a[apt+1]      = PixelsFromRun + 1;
                m_acolor[apt+1] = 0;

                // Setup for next LRS
                ++m_lrsCount;
                apt+=2;

                if(m_lrsCount >= m_max_lrs)
                    GrowMaxLRS();
                }
            }
        }
    else
        {
        while(PixelsFromRun < m_width)
            {
            // Skip empty runs.
            if(!m_RLEBuffer[m_RLEBufferIndex])
                {
                ++m_RLEBufferIndex;
                continue;
                }

            long StartRun = PixelsFromRun;

            PixelsFromRun += m_RLEBuffer[m_RLEBufferIndex];
            ++m_RLEBufferIndex;

            // Append all pixels of the same state. That take care of 32767,0,123,0,456..
            while(PixelsFromRun < m_width && m_RLEBuffer[m_RLEBufferIndex] == 0)
                {
                PixelsFromRun += m_RLEBuffer[m_RLEBufferIndex+1];
                m_RLEBufferIndex+=2;
                }

            // Encode white only. Black runs are ON even number. 0,2,4,6...
            if(!(m_RLEBufferIndex & 0x00000001))    // m_RLEBufferIndex is 1 beyond current state but if next is white that means previous was black.
                {
                HASSERT(StartRun < m_width);

                // Left entry, add one for the CCITT algo
                m_a[apt]      = StartRun + 1;
                m_acolor[apt] = 1;

                // Right entry, add one for the CCITT algo
                m_a[apt+1]      = PixelsFromRun + 1;
                m_acolor[apt+1] = 0;

                // Setup for next LRS
                ++m_lrsCount;
                apt+=2;

                if(m_lrsCount >= m_max_lrs)
                    GrowMaxLRS();
                }
            }
        }

    m_a[apt] = m_width + 1;
    m_acolor[apt] = 0;
    ++apt;
    m_a[apt] = m_width + 1;
    m_acolor[apt] = 1;

    // Encode current LRS.
    EncodeLine();
    }

//-----------------------------------------------------------------------------
// public
// Encode lbuf and rbuf
//-----------------------------------------------------------------------------
void HCDCodecCCITTRLE::CCITTRLEState::EncodeLineFromBit()
    {
    CreateLRSForEncode();

    EncodeLine();
    }

//-----------------------------------------------------------------------------
// private
// Encode lbuf and rbuf
//-----------------------------------------------------------------------------
void HCDCodecCCITTRLE::CCITTRLEState::EncodeLine()
    {
    long oldr = 0;
    long apt = 1;
    long value;

    for(int i(0); i < m_lrsCount; ++i)
        {
        value = m_a[apt+0] - oldr - 1;
        CodeWhite(value);
        value = m_a[apt+1] - m_a[apt+0];
        CodeBlack(value);
        oldr = m_a[apt+1] - 1;
        apt+=2;
        }

    value = m_width - oldr;
    if (value > 0)
        {
        CodeWhite(value);
        }

    if (m_ccittrleMask != 0x80L)
        {
        if (m_ccittrleWord != 'p')
            {
            int i;
            i = 1;
            }
        HASSERT(m_ccittrleBufferIndex < m_ccittrleBufferSize);
        /* go to next byte */
        m_ccittrleBuffer[m_ccittrleBufferIndex] = (char) m_ccittrleWord;
        ++m_ccittrleBufferIndex;

        m_ccittrleWord = 0;
        m_ccittrleMask = 0x80L;
        }
    }

//-----------------------------------------------------------------------------
// findspan: Find a span of ones or zeros using the supplied
//           table.  The byte-aligned start of the bit string
//           is supplied along with the start+end bit indices.
//           The table gives the number of consecutive ones or
//           zeros starting from the msb and is indexed by byte
//           value.
//-----------------------------------------------------------------------------
int HCDCodecCCITTRLE::CCITTRLEState::FindSpan(Byte** bpp, int bs, int be, register Byte const* tab) const
    {
    register Byte* bp = *bpp;
    register int bits = be - bs;
    register int n, span;

    /*
    * Check partial byte on lhs.
    */
    if (bits > 0 && (n = (bs & 7)))
        {
        span = tab[(*bp << n) & 0xff];
        if (span > 8-n)     /* table value too generous */
            span = 8-n;
        if (span > bits)    /* constrain span to bit range */
            span = bits;
        if (n+span < 8)     /* doesn't extend to edge of Byte */
            goto done;
        bits -= span;
        bp++;
        }
    else
        span = 0;
    /*
     * Scan full bytes for all 1's or all 0's.
     */
    while (bits >= 8)
        {
        n = tab[*bp];
        span += n;
        bits -= n;
        if (n < 8)      /* end of run */
            goto done;
        bp++;
        }
    /*
     * Check partial byte on rhs.
     */
    if (bits > 0) {
        n = tab[*bp];
        span += (n > bits ? bits : n);
        }
done:
    *bpp = bp;
    return (span);
    }

//-----------------------------------------------------------------------------
// finddiff: Return the offset of the next bit in the range
//           [bs..be] that is different from the specified
//           color.  The end, be, is returned if no such bit
//           exists.
//-----------------------------------------------------------------------------
int HCDCodecCCITTRLE::CCITTRLEState::FindDiff(Byte* cp, int bitStart, int bitEnd, int color) const
    {
    cp += bitStart >> 3;            /* adjust Byte offset */
    return (bitStart + FindSpan(&cp, bitStart, bitEnd, color ? oneruns : zeroruns));
    }

//-----------------------------------------------------------------------------
// FillSpan: Fill a span with ones.
// cp    = buffer pointing to the beginning of the line
// x     = position in the line
// count = number of white starting at x
//-----------------------------------------------------------------------------
void HCDCodecCCITTRLE::CCITTRLEState::FillSpan(Byte* cp, int x, int count)
    {
    static const unsigned char masks[] =  { 0, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff };

    if (count <= 0)
        return;

    if(m_invertResult)
        {
        cp += x>>3;
        if (x &= 7)             /* align to Byte boundary */
            {
            if (count < 8 - x)
                {
                *cp++ ^= masks[count] >> x;
                return;
                }
            *cp++ ^= 0xff >> x;
            count -= 8 - x;
            }

        while (count >= 8)
            {
            *cp++ = 0x00;
            count -= 8;
            }

        if (count > 0)
            *cp ^= masks[count];
        }
    else
        {
        cp += x>>3;
        if (x &= 7)             /* align to Byte boundary */
            {
            if (count < 8 - x)
                {
                *cp++ |= masks[count] >> x;
                return;
                }
            *cp++ |= 0xff >> x;
            count -= 8 - x;
            }

        while (count >= 8)
            {
            *cp++ = 0xff;
            count -= 8;
            }

        if (count > 0)
            *cp |= masks[count];
        }
    }


//-----------------------------------------------------------------------------
// private
// CreateLRS
//-----------------------------------------------------------------------------
void HCDCodecCCITTRLE::CCITTRLEState::CreateLRSForEncode()
    {
    m_lrsCount = 0;

    m_a[0] = 0;
    m_acolor[0] = 0;
    long apt = 1;

    Byte* pLineBuffer = &m_UncompressBuffer[m_UncompressBufferIndex];

    if(m_invertResult)
        {
        for(int i(0); i < m_width; ++i)
            {
            // Left entry
            m_a[apt+0] = FindDiff(pLineBuffer, i, m_width, 1);
            if(m_width == m_a[apt])
                {
                // We can't start a run at the end.  That means only blacks are remaining.
                break;
                }
            m_acolor[apt+0] = 1;

            // Right entry
            m_a[apt+1]      = FindDiff(pLineBuffer, m_a[apt+0], m_width, 0);
            m_acolor[apt+1] = 0;

            // Setup for next LRS
            i = m_a[apt+1];
            ++m_lrsCount;

            // Add one for the CCITT algo
            m_a[apt+0] += 1;
            m_a[apt+1] += 1;
            apt+=2;

            if(m_lrsCount >= m_max_lrs)
                GrowMaxLRS();
            }
        }
    else
        {
        for(int i(0); i < m_width; ++i)
            {
            // Left entry
            m_a[apt+0] = FindDiff(pLineBuffer, i, m_width, 0);
            if(m_width == m_a[apt])
                {
                // We can't start a run at the end.  That means only blacks are remaining.
                break;
                }
            m_acolor[apt+0] = 1;

            // Right entry
            m_a[apt+1]      = FindDiff(pLineBuffer, m_a[apt+0], m_width, 1);
            m_acolor[apt+1] = 0;

            // Setup for next LRS
            i = m_a[apt+1];
            ++m_lrsCount;

            // Add one for the CCITT algo
            m_a[apt+0] += 1;
            m_a[apt+1] += 1;
            apt+=2;

            if(m_lrsCount >= m_max_lrs)
                GrowMaxLRS();
            }
        }

    // Move index to the next line in the uncompress buffer.
    m_UncompressBufferIndex+=m_UncompressBufferLineSize;

    m_a[apt] = m_width + 1;
    m_acolor[apt] = 0;
    apt++;
    m_a[apt] = m_width + 1;
    m_acolor[apt] = 1;
    }

/**************************************************************************/

//-----------------------------------------------------------------------------
// public
// Default constructor
//-----------------------------------------------------------------------------
HCDCodecCCITTRLE::HCDCodecCCITTRLE()
    : HCDCodecCCITT()
    {
    }

//-----------------------------------------------------------------------------
// public
// Default constructor
//-----------------------------------------------------------------------------
HCDCodecCCITTRLE::HCDCodecCCITTRLE(uint32_t pi_Width, uint32_t pi_Height)
    : HCDCodecCCITT(pi_Width, pi_Height)
    {
    }

//-----------------------------------------------------------------------------
// public
// Copy constructor
//-----------------------------------------------------------------------------
HCDCodecCCITTRLE::HCDCodecCCITTRLE(const HCDCodecCCITTRLE& pi_rObj)
    : HCDCodecCCITT(pi_rObj),
      m_CCITTRLEState(pi_rObj.m_CCITTRLEState)
    {
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HCDCodecCCITTRLE::~HCDCodecCCITTRLE()
    {
    }

//-----------------------------------------------------------------------------
// public
// Clone
//-----------------------------------------------------------------------------
HCDCodec* HCDCodecCCITTRLE::Clone() const
    {
    return new HCDCodecCCITTRLE(*this);
    }

//-----------------------------------------------------------------------------
// public
// CompressSubset
//-----------------------------------------------------------------------------
size_t HCDCodecCCITTRLE::CompressSubset(const void* pi_pInData, size_t pi_InDataSize,
                                        void* po_pOutBuffer,    size_t po_OutBufferSize)
    {
    HPRECONDITION(GetWidth() == GetSubsetWidth());

    size_t OutDataSize(0);

    // is it the first subset?
    if(GetSubsetPosY() == 0)
        {
        SetCurrentState(STATE_COMPRESS);
        m_CCITTRLEState.Pre(static_cast<long>(GetWidth()), static_cast<long>(GetHeight()), false, m_bitrevtable, CCITT_PHOTOMETRIC_MINISBLACK == m_photometric);
        }

    size_t  LineSize(((GetSubsetWidth() * GetBitsPerPixel() + 7) / 8) + GetLinePaddingBits() / 8);

    // Set read/write buffers for the current line(s)
    m_CCITTRLEState.SetupForUncompressMode((Byte*)po_pOutBuffer, po_OutBufferSize, (Byte*)pi_pInData, pi_InDataSize, LineSize);

    // Compress subset
    for(uint32_t Line(0); Line < GetSubsetHeight(); ++Line)
        {
        m_CCITTRLEState.EncodeLineFromBit();

        HASSERT(m_CCITTRLEState.GetCCITTRLEError() == CCITTRLE_NOERROR);
        }

    OutDataSize = m_CCITTRLEState.GetCCITTRLEBufferDataSize();

    SetSubsetPosY(GetSubsetPosY() + GetSubsetHeight());

    // is it the last subset?
    if(GetSubsetPosY() == GetHeight())
        {
        m_CCITTRLEState.Post();

        // Update outDataSize before the reset occurs.
        OutDataSize = m_CCITTRLEState.GetCCITTRLEBufferDataSize();

        Reset();
        }

    return OutDataSize;
    }

//-----------------------------------------------------------------------------
// public
// GetRLEInterface
//-----------------------------------------------------------------------------
HCDCodecRLEInterface* HCDCodecCCITTRLE::GetRLEInterface()
    {
    return this;
    }

//-----------------------------------------------------------------------------
// public
// CompressSubsetFromRLE
//-----------------------------------------------------------------------------
size_t HCDCodecCCITTRLE::CompressSubsetFromRLE(HFCPtr<HCDPacketRLE> const& pi_rpPacketRLE, void* po_pOutBuffer, size_t po_OutBufferSize)
    {
    HPRECONDITION(GetWidth() == GetSubsetWidth());

    size_t OutDataSize(0);

    // is it the first subset?
    if(GetSubsetPosY() == 0)
        {
        SetCurrentState(STATE_COMPRESS);
        m_CCITTRLEState.Pre(static_cast<long>(GetWidth()), static_cast<long>(GetHeight()), false, m_bitrevtable, CCITT_PHOTOMETRIC_MINISBLACK == m_photometric);
        }

    //size_t  LineSize(((GetSubsetWidth() * GetBitsPerPixel() + 7) / 8) + GetLinePaddingBits() / 8);

    // Set read/write buffers for the current line(s)
    m_CCITTRLEState.SetupForRLEMode((Byte*)po_pOutBuffer, po_OutBufferSize, (unsigned short*)pi_rpPacketRLE->GetLineBuffer(0), pi_rpPacketRLE->GetLineDataSize(0));

    // Compress subset
    for(uint32_t Line(0); Line < GetSubsetHeight(); ++Line)
        {
        // Set input RLE buffer for current line
        m_CCITTRLEState.SetRLEBuffer((unsigned short*)pi_rpPacketRLE->GetLineBuffer(Line), pi_rpPacketRLE->GetLineDataSize(Line));

        m_CCITTRLEState.EncodeLineFromRLE();

        HASSERT(m_CCITTRLEState.GetCCITTRLEError() == CCITTRLE_NOERROR);
        }

    OutDataSize = m_CCITTRLEState.GetCCITTRLEBufferDataSize();

    SetSubsetPosY(GetSubsetPosY() + GetSubsetHeight());

    // is it the last subset?
    if(GetSubsetPosY() == GetHeight())
        {
        m_CCITTRLEState.Post();

        // Update outDataSize before the reset occurs.
        OutDataSize = m_CCITTRLEState.GetCCITTRLEBufferDataSize();

        Reset();
        }

    return OutDataSize;
    }

//-----------------------------------------------------------------------------
// public
// DecompressSubsetToRLE
//-----------------------------------------------------------------------------
void HCDCodecCCITTRLE::DecompressSubsetToRLE(const void* pi_pInData, size_t pi_InDataSize, HFCPtr<HCDPacketRLE>& pio_rpRLEPacket)
    {
    HPRECONDITION(pi_pInData != 0);
    HPRECONDITION(pio_rpRLEPacket->GetCodec()->GetHeight() >= GetSubsetHeight());
    HPRECONDITION(pio_rpRLEPacket->GetCodec()->GetWidth() == GetSubsetWidth());
    HPRECONDITION(GetWidth() == GetSubsetWidth());
    HPRECONDITION(pio_rpRLEPacket->HasBufferOwnership());    // Must be owner of buffers.

    // Is it the first subset?
    if(GetSubsetPosY() == 0)
        {
        SetCurrentState(STATE_DECOMPRESS);
        m_CCITTRLEState.Pre(static_cast<long>(GetWidth()), static_cast<long>(GetHeight()), true, m_bitrevtable, CCITT_PHOTOMETRIC_MINISBLACK == m_photometric);
        }

    // Alloc a working buffer
    size_t                  WorkLineBufferSize = (GetSubsetWidth()* 2 + 2)*sizeof(unsigned short);   // Worst case for one line.
    HArrayAutoPtr<Byte>   pWorkLineBuffer (new Byte[WorkLineBufferSize]);

    // Set read buffer for the current line(s)
    m_CCITTRLEState.SetupForRLEMode((Byte*)pi_pInData, pi_InDataSize, (unsigned short*)pWorkLineBuffer.get(), WorkLineBufferSize);

    // Decompress subset
    for(uint32_t Line(0); Line < GetSubsetHeight(); ++Line)
        {
        // Get white line runs.
        m_CCITTRLEState.DecodeLineRLE();

        size_t LineDataSize = m_CCITTRLEState.GetRLEBufferDataSize();

        // Alloc buffer if it is not large enough.
        if(pio_rpRLEPacket->GetLineBufferSize(Line) < LineDataSize)
            {
            HASSERT(pio_rpRLEPacket->HasBufferOwnership());    // Must be owner of buffer.
            pio_rpRLEPacket->SetLineBuffer(Line, new Byte[LineDataSize], LineDataSize, 0/*pi_DataSize*/);
            }

        // Copy from workBuffer to output packet.
        memcpy(pio_rpRLEPacket->GetLineBuffer(Line), pWorkLineBuffer, LineDataSize);
        pio_rpRLEPacket->SetLineDataSize(Line, LineDataSize);

        // RLE runs are encoded by line and always start and end with a black state so the current state should be a white state.
        HASSERT((m_CCITTRLEState.GetRLEBufferDataSize() >> 1) % 2 == 1);
        HASSERT(m_CCITTRLEState.GetCCITTRLEError() == CCITTRLE_NOERROR);

        // Reset RLE buffer for next line.
        m_CCITTRLEState.SetRLEBuffer((unsigned short*)pWorkLineBuffer.get(), WorkLineBufferSize);
        }

    SetSubsetPosY(GetSubsetPosY() + GetSubsetHeight());

    // Is it the last subset?
    if(GetSubsetPosY() == GetHeight())
        {
        m_CCITTRLEState.Post();

        Reset();
        }
    else
        {
        // Caller need this to increment the input buffer for the next subset.
        SetCompressedImageIndex(GetCompressedImageIndex() + m_CCITTRLEState.GetCCITTRLEBufferDataSize());
        }
    }

//-----------------------------------------------------------------------------
// public
// DecompressSubset
//-----------------------------------------------------------------------------
size_t HCDCodecCCITTRLE::DecompressSubset(const void* pi_pInData, size_t pi_InDataSize,
                                          void* po_pOutBuffer, size_t pi_OutBufferSize)
    {
    HPRECONDITION(GetWidth() == GetSubsetWidth());

    size_t SubsetSize = (GetSubsetWidth() * GetBitsPerPixel() + GetLinePaddingBits()) / 8 * GetSubsetHeight();

    HASSERT(SubsetSize <= pi_OutBufferSize);

    // Is it the first subset?
    if(GetSubsetPosY() == 0)
        {
        SetCurrentState(STATE_DECOMPRESS);
        m_CCITTRLEState.Pre(static_cast<long>(GetWidth()), static_cast<long>(GetHeight()), true, m_bitrevtable, CCITT_PHOTOMETRIC_MINISBLACK == m_photometric);
        }

    // Set read buffer for the current line(s)
    size_t  LineSize(((GetSubsetWidth() * GetBitsPerPixel() + 7) / 8) + GetLinePaddingBits() / 8);

    // Give only what the decompressor needs as the output buffer size.
    m_CCITTRLEState.SetupForUncompressMode((Byte*)pi_pInData, pi_InDataSize, (Byte*)po_pOutBuffer, MIN(pi_OutBufferSize, SubsetSize), LineSize);

    // Decompress subset
    for(uint32_t Line(0); Line < GetSubsetHeight(); ++Line)
        {
        m_CCITTRLEState.DecodeLine();
        }

    SetSubsetPosY(GetSubsetPosY() + GetSubsetHeight());

    // Is it the last subset?
    if(GetSubsetPosY() == GetHeight())
        {
        m_CCITTRLEState.Post();
        Reset();
        }
    else
        {
        // Caller need this to increment the input buffer for the next subset.
        SetCompressedImageIndex(GetCompressedImageIndex() + m_CCITTRLEState.GetCCITTRLEBufferDataSize());
        }

    return SubsetSize;
    }

//-----------------------------------------------------------------------------
// public
// HasLineAccess
//-----------------------------------------------------------------------------
bool HCDCodecCCITTRLE::HasLineAccess() const
    {
    return true;
    }

//-----------------------------------------------------------------------------
// public
// Reset
//-----------------------------------------------------------------------------
void HCDCodecCCITTRLE::Reset()
    {
    HCDCodecCCITT::Reset();
    m_CCITTRLEState.Reset();
    }
