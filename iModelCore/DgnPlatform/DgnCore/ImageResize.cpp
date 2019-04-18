/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

struct Squetch
{
    int32_t    pa, wa, pb, wb;
};

/* Squetch Modes */
enum    SquetchMode
{
    SAMESIZE        = 0,
    EXPAND          = 1,
    RGBCOMPRESS     = 2
};

#define SQ_ONE 512

typedef  bvector<Squetch> T_SquetchTable;

/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     10/2016
+===============+===============+===============+===============+===============+======*/
struct Resizer
{
    Byte const*             m_input;
    ByteStream&             m_output;
    Point2d                 m_inSize;
    Point2d                 m_outSize;
    T_SquetchTable          m_xTable;
    SquetchMode             m_xMode;
    T_SquetchTable          m_yTable;
    SquetchMode             m_yMode;
    int                     m_sourceRow;                        // The row number to get from the source image.
    int                     m_currentInputRow;                  // The index into the Squetch table.
    int                     m_currentOutputRow;         
    int                     m_bytesPerPixel;
    bvector<int32_t>        m_pBuf, m_oBuf1, m_oBuf2;

    Byte*                   GetCurrentOutputRow() {return m_output.data() + m_currentOutputRow * m_bytesPerPixel * m_outSize.x;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Resizer(ByteStream& output, uint32_t targetWidth, uint32_t targetHeight, ImageCR sourceImage) : m_output(output), m_currentInputRow(0), m_currentOutputRow(0), m_sourceRow(0)
    {
    m_input = sourceImage.GetByteStream().data();
    m_inSize.x = (int32_t) sourceImage.GetWidth();
    m_inSize.y = (int32_t) sourceImage.GetHeight();
    m_outSize.x = (int32_t) targetWidth;
    m_outSize.y = (int32_t) targetHeight;

    m_bytesPerPixel = sourceImage.GetBytesPerPixel();
    m_pBuf.resize(m_bytesPerPixel * targetWidth);
    m_oBuf1.resize(m_bytesPerPixel * targetWidth);
    m_oBuf2.resize(m_bytesPerPixel * targetWidth);

    m_xMode = BuildSquetchTable(m_inSize.x, m_outSize.x, m_xTable);
    m_yMode = BuildSquetchTable(m_inSize.y, m_outSize.y, m_yTable);

    if (m_yMode == EXPAND)
        {
        LoadLineSquetch(m_oBuf1.data());
        LoadLineSquetch(m_oBuf2.data());
        }

    m_output.Resize(m_bytesPerPixel * targetWidth * targetHeight);
    }
 
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
SquetchMode BuildSquetchTable(int32_t n, /* go from n pixels */ int32_t m, /* to m pixels */ T_SquetchTable&   squetchTable)
    {
    int         i,j, count=0;
    double      x, dx;

    if (n == m)
        return SAMESIZE;

    if (n > m)  /* compression */
        {
        squetchTable.resize(n);
        dx = (double) m/(double) n;
        x = 0;
        for (i=0,j=0;i<n;i++)
            {
            x += dx;
            if (x <= 1.0)
                {
                squetchTable[i].pa=j;   
                squetchTable[i].wa=(int)(dx*SQ_ONE+0.5);
                squetchTable[i].pb=j+1; squetchTable[i].wb=0;
                }
            else
                {
                x -= 1.0;
                squetchTable[i].pa=j;   
                squetchTable[i].wa=(int)((dx-x)*SQ_ONE+0.5);
                squetchTable[i].pb=j+1; 
                squetchTable[i].wb=(int)((x*SQ_ONE)+0.5);
                j++;
                }
            if (squetchTable[i].wb != 0) count++;
            }
        return RGBCOMPRESS;
        }
    else        /* expansion */
        {
        squetchTable.resize(m);
        dx = (double)(n-1) / (double)m;
        for (i=0,x=0;i<m-1;i++,x+=dx)
            {
            squetchTable[i].pa = (int32_t)x;
            squetchTable[i].pb = squetchTable[i].pa + 1;
            squetchTable[i].wb = (int32_t)((x - (double)((int)x))*SQ_ONE);
            squetchTable[i].wa = SQ_ONE - squetchTable[i].wb;
            }
        squetchTable[m-1].pb = n-1;
        squetchTable[m-1].wb = SQ_ONE;
        squetchTable[m-1].pa = n-2;
        squetchTable[m-1].wa = 0;
        return (EXPAND);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     01/1990
+---------------+---------------+---------------+---------------+---------------+------*/
void LoadSquetch
(
int                 mode,           /* COMPRESS, EXPAND, or SAMESIZE */
int32_t             n,              /* from size */
int32_t             m,              /* to size */
T_SquetchTable&     squetchTable,   /* squetch table */
Byte const*         inBuf,          /* input Buffer */
int32_t*            outBuf          /* output Buffer */
)
    {
    switch (mode)
        {
        case SAMESIZE:
            for (int32_t i=0; i<n; i++)
                {
                *outBuf++ = *inBuf * SQ_ONE;
                inBuf += m_bytesPerPixel;
                }
            break;

        case RGBCOMPRESS:
            {
            Squetch *tab = squetchTable.data();

            *outBuf = 0;
            for (int32_t i=1; i < n; i++)
                {
                *outBuf += (*inBuf * tab->wa);
                if (tab->pa != tab[1].pa)
                    *(++outBuf) = (*inBuf * tab->wb); 

                inBuf += m_bytesPerPixel;
                tab++;
                }
            *outBuf += (*inBuf * tab->wa);
            break;
            }

        case EXPAND:
            {
            Squetch *tab = squetchTable.data();

            for (int32_t i=1; i < m; i++)
                {
                *outBuf++ = *inBuf * tab->wa + inBuf[m_bytesPerPixel]*tab->wb;
                if ((tab->pa) != (tab[1].pa))
                    inBuf += m_bytesPerPixel;

                tab++;
                }
            *outBuf++ = *inBuf * tab->wa + inBuf[m_bytesPerPixel]*tab->wb;
            break;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     01/1990
+---------------+---------------+---------------+---------------+---------------+------*/
void LoadLineSquetch(int32_t*  pBuf)
    {
    Byte const*     pInputRow = m_input + m_inSize.x * m_bytesPerPixel * m_sourceRow++;

    for (int i=0; i<m_bytesPerPixel; i++)
        LoadSquetch(m_xMode, m_inSize.x, m_outSize.x, m_xTable, pInputRow + i, pBuf + i * m_outSize.x);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ExtractLineSquetch(int32_t *pBuf, Byte* pOutput, int scale) 
    {
    for (int i=0; i<m_bytesPerPixel; i++)
        {
        int32_t*    pThisBuffer = pBuf + i * m_outSize.x;
        Byte*       pOutputBuffer = pOutput + i;
        
        for (int32_t j=0; j<m_outSize.x; j++)
            {
            int32_t t = (*pThisBuffer++) >> scale;
            *pOutputBuffer = (t>255) ? 255 : (Byte)(t & 0xff);
            pOutputBuffer += m_bytesPerPixel;
            }
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     01/1990
+---------------+---------------+---------------+---------------+---------------+------*/
void SwapOutputBuffers()
    {
    m_oBuf1.swap(m_oBuf2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     01/1990
+---------------+---------------+---------------+---------------+---------------+------*/
int ExpandRows(int32_t* inBufA, int32_t* inBufB, int32_t* outBuf)
    {
    int32_t wa = m_yTable[m_currentInputRow].wa;
    int32_t wb = m_yTable[m_currentInputRow].wb;

    for (int i=0; i<m_outSize.x; i++)
        *outBuf++ = (*inBufA++ * wa + *inBufB++ * wb);

    if (m_currentInputRow == m_yTable.size() - 1)
        return (0);

    if (m_yTable[m_currentInputRow].pa != m_yTable[m_currentInputRow+1].pa)
        return (1);

    return (0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     01/1990
+---------------+---------------+---------------+---------------+---------------+------*/
int CompressRows(int32_t* inBuf, int32_t* outBufA, int32_t* outBufB)
    {
    int         w;
    int32_t*    t;

    w = m_yTable[m_currentInputRow].wa;
    t = inBuf;

    for (int i=0; i < m_outSize.x; i++)
        {
        *outBufA += (*t++ * w);
        outBufA++;
        };
    if ((m_currentInputRow == m_yTable.size() - 1) || (m_yTable[m_currentInputRow].pa != m_yTable[m_currentInputRow+1].pa))
        {
        w = m_yTable[m_currentInputRow].wb;
        for (int i=0; i < m_outSize.x; i++)
            *outBufB++ = (*inBuf++ * w);

        return (1);
        }
    return (0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     01/1990
+---------------+---------------+---------------+---------------+---------------+------*/
bool GetRow()
    {
    if (m_yMode == RGBCOMPRESS)
        {
        while (true)
            {
            if (m_currentInputRow > m_inSize.y)
                break;

            LoadLineSquetch(m_pBuf.data());

            int s = 0;

            for (int i=0; i<m_bytesPerPixel; i++)
                s |= CompressRows(m_pBuf.data()  + i * m_outSize.x, m_oBuf1.data() + i * m_outSize.x, m_oBuf2.data() + i * m_outSize.x);

            if (s)
                {
                ExtractLineSquetch(m_oBuf1.data(), GetCurrentOutputRow(), 18);
                SwapOutputBuffers();
                m_currentInputRow++;
                return (0);
                }
            m_currentInputRow++;
            }
        ExtractLineSquetch(m_oBuf1.data(), GetCurrentOutputRow(), 18);
        return (0);
        }
    else if (m_yMode == EXPAND)
        {
        int s = 0;

        for (int i=0; i<m_bytesPerPixel; i++)
            s |= ExpandRows(m_oBuf1.data() + i * m_outSize.x,  m_oBuf2.data() + i * m_outSize.x, m_pBuf.data() + i * m_outSize.x);

         if (s)
            {
            SwapOutputBuffers();
            LoadLineSquetch(m_oBuf2.data());
            }
        m_currentInputRow++;
        ExtractLineSquetch(m_pBuf.data(), GetCurrentOutputRow(), 18);
        return (0);
        }
    else
        {
        LoadLineSquetch(m_pBuf.data());
        ExtractLineSquetch(m_pBuf.data(), GetCurrentOutputRow(), 9);
        m_currentInputRow++;
        }
    return m_currentInputRow < m_inSize.y;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void    DoResize()
    {
    for (m_currentOutputRow=0; m_currentOutputRow < m_outSize.y; m_currentOutputRow++)
        GetRow();

    }

};      // Resizer


#ifdef DEBUG_IMAGES
static void writeImageFile(Byte const* data, uint32_t width, uint32_t height, char* name, int32_t bytesPerPixel)
    {
    bool            rgba = 4 == bytesPerPixel;
    Image           image(width, height, ByteStream(data, width * height * bytesPerPixel), (rgba) ? Image::Format::Rgba : Image::Format::Rgb);
    ImageSource     imageSource(image, rgba ? ImageSource::Format::Png: ImageSource::Format::Jpeg);

    FILE*               file = fopen(name, "wb");
    ByteStream const&   byteStream = imageSource.GetByteStream();

    fwrite(byteStream.data(), 1, byteStream.size(), file);
    fclose(file);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Image Image::FromResizedImage(uint32_t targetWidth, uint32_t targetHeight, ImageCR sourceImage)
    {
    ByteStream  outputImage;
    Resizer     resizer(outputImage, targetWidth, targetHeight, sourceImage);

    resizer.DoResize();

#ifdef DEBUG_IMAGES
    writeImageFile(sourceImage.GetByteStream().data(), sourceImage.GetWidth(), sourceImage.GetHeight(), "d:\\tmp\\inputImage.png", sourceImage.GetBytesPerPixel());
    writeImageFile(outputImage.data(), targetWidth, targetHeight, "d:\\tmp\\resizedImage.png", sourceImage.GetBytesPerPixel());
#endif

    return Image(targetWidth, targetHeight, std::move(outputImage), sourceImage.GetFormat());
    }
