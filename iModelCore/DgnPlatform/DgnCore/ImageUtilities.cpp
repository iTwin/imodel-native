/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/ImageUtilities.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

#if defined (BENTLEY_TOOL_CONTEXT_IsLinuxGcc)
// See pngconf.h
#define PNG_SKIP_SETJMP_CHECK
#endif

#include <png/png.h>
#include <BeJpeg/BeJpeg.h>

BEGIN_UNNAMED_NAMESPACE
/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                     Mathieu.Marchand  07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct PngReadData
{
public:
    PngReadData(Byte const* pPngData, size_t dataSize) : m_pData(pPngData), m_size(dataSize), m_position(0){}

    static void Read(png_structp png_ptr, png_bytep data, png_size_t length)
        {
        PngReadData* pngData = (PngReadData*)(png_ptr->io_ptr);

        if (pngData->m_position + length > pngData->m_size)
            png_error(png_ptr, "Read Error");   // >>>>  Will long jump and not return.

        memcpy(data, pngData->m_pData+pngData->m_position, length);
        pngData->m_position+=length;
        }

private:
    Byte const* m_pData;
    size_t      m_size;
    size_t      m_position;
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus readPng(Image& image, PngReadData& reader)
    {
    png_structp png_ptr;
    png_infop info_ptr;
    unsigned int sig_read = 0;

    // Create and initialize the png_struct with default error handler functions(setjmp/longjmp).
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
    if (png_ptr == NULL)
        return ERROR;

    // Allocate/initialize the memory for image information.
    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL)
        {
        png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
        return ERROR;
        }

    // Set error handling. setjmp/longjmp method (this is the normal method of doing things with libpng)
    if (setjmp(png_jmpbuf(png_ptr)))
        {
        png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
        return ERROR;   // If we get here, we had a problem reading the file
        }

    // Register our png data reader.
    png_set_read_fn(png_ptr, &reader, PngReadData::Read);

    // If we have already read some of the signature */
    png_set_sig_bytes(png_ptr, sig_read);

    // We assumed that we have enough memory to read the entire image at once.
    // PNG_TRANSFORM_STRIP_16       >> strip 16 bit/color files down to 8 bits per color.   
    // PNG_TRANSFORM_PACKING        >> forces 8 bit     
    // PNG_TRANSFORM_EXPAND         >> forces to expand a palette into RGB   
    // PNG_TRANSFORM_GRAY_TO_RGB    >> convert grayscale to rgb.
    png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_EXPAND | PNG_TRANSFORM_GRAY_TO_RGB | PNG_TRANSFORM_PACKING, png_voidp_NULL);

    png_bytepp rows_pointers = png_get_rows(png_ptr, info_ptr);
    size_t bytesPerRow = png_get_rowbytes(png_ptr, info_ptr);

    // We expect RGB or RGBA output
    if (24 != info_ptr->pixel_depth && 32 != info_ptr->pixel_depth)
        {
        png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
        return ERROR;
        }
        
    // Fill in the outputs.
    image.SetSize(info_ptr->width, info_ptr->height);
    image.SetFormat((32 == info_ptr->pixel_depth) ? Image::Format::Rgba : Image::Format::Rgb); 
    image.GetByteStreamR().Resize((uint32_t) (info_ptr->height*bytesPerRow));

    Byte* out = image.GetByteStreamR().GetDataP();
    for (uint32_t line=0; line < info_ptr->height; ++line)
        memcpy(out + line*bytesPerRow, rows_pointers[line], bytesPerRow);

    // Clean up after the read, and free any memory allocated 
    png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);

    return SUCCESS;
    }

//=======================================================================================
// @bsiclass                                                    Sam.Wilson  10/13
//=======================================================================================
struct BufferWriter
{
    ByteStream& m_pngBuffer;

    BufferWriter(ByteStream& b) : m_pngBuffer(b) {;}

    static void Write(png_structp png_ptr, png_bytep data, png_size_t length)
        {
        BufferWriter* app = (BufferWriter*) png_get_io_ptr(png_ptr);
        app->m_pngBuffer.Append(data, (uint32_t) length);
        }
};

struct Bgra {uint8_t b, g, r, a;};
struct Bgr  {uint8_t b, g, r;};
struct Rgba {uint8_t r, g, b, a;};


//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson  10/2013
//---------------------------------------------------------------------------------------
static BentleyStatus prepareForPng(int& pngformat, bvector <png_bytep>& rows, ImageCR img, ImageSource const& source)
    {
    uint32_t bytesPerPixel = img.GetBytesPerPixel();
    size_t  lengthInPixels = img.GetWidth()*img.GetHeight();

    //  ---------------------------------------------------------------------------------------------------
    //  PNG does not support BGR*, only RGB*. If the input is BGR, we must swap B and R.
    //  PNG does support formats with an "alpha" byte. This value is the inverse of transparency. 
    //  On Windows, we usually get BGRA images from QV with 0 for the alpha value. That would make 
    //  the image invisible. We fix that in the loop below.
    //  ---------------------------------------------------------------------------------------------------

    if (bytesPerPixel == 3)
        {
        pngformat = PNG_COLOR_TYPE_RGB;
        if (source.IsBGR())
            {
            Bgr* pixel = (Bgr*) img.GetByteStream().GetData();
            Bgr* pixelEnd = pixel + lengthInPixels;
            for (; pixel < pixelEnd; ++pixel)
                std::swap(pixel->r, pixel->b);
            }
        }
    else if (bytesPerPixel == 4)
        {
        pngformat = PNG_COLOR_TYPE_RGBA;
        
        if (source.IsBGR())
            {
            Bgra* pixel = (Bgra*) img.GetByteStream().GetData();
            Bgra* pixelEnd = pixel + lengthInPixels;
            for (; pixel < pixelEnd; ++pixel)
                {
                std::swap(pixel->r, pixel->b);
                if (pixel->a == 0)
                    pixel->a = 0xff;
                }
            }
        }
    else
        {
        BeAssert(false && "unsupported image format");
        return BSIERROR;
        }

    //  ---------------------------------------------------------------------------------------------------
    //  PNG wants a pointer to each row, in TOP-DOWN ORDER. This is where we correct for bottom-up images. 
    //  ---------------------------------------------------------------------------------------------------
    rows.resize(img.GetHeight());

    int    rowSize = img.GetWidth() * bytesPerPixel;
    uint8_t* row = source.IsTopDown() ? img.GetByteStream().GetDataP() : img.GetByteStream().GetDataP() + (img.GetHeight()-1)*rowSize;
    int    rowStep = source.IsTopDown() ? rowSize : -rowSize;
    for (size_t i=0; i<rows.size(); ++i)
        {
        rows[i] = row;
        row += rowStep;
        }

    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     09/13
//---------------------------------------------------------------------------------------
static BentleyStatus writeImageToPng(BufferWriter& writer, ImageCR imageData, ImageSource const& source)
    {
    int pngFormat;
    bvector<png_bytep> rowPointers;
    if (prepareForPng(pngFormat, rowPointers, imageData, source) != BSISUCCESS)
        return BSIERROR;

    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr)
        return BSIERROR;

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
        return BSIERROR;

    if (setjmp(png_jmpbuf(png_ptr)))
        return BSIERROR;

    png_set_write_fn(png_ptr, &writer, BufferWriter::Write, NULL);

    png_set_IHDR (png_ptr, info_ptr, imageData.GetWidth(), imageData.GetHeight(), 8, pngFormat, PNG_INTERLACE_NONE,
                  PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    png_write_info(png_ptr, info_ptr);
    if (setjmp(png_jmpbuf(png_ptr)))
        return BSIERROR;

    png_write_image(png_ptr, &rowPointers[0]);

    if (setjmp(png_jmpbuf(png_ptr)))
        return BSIERROR;

    png_write_end(png_ptr, NULL);

    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      10/2013
//---------------------------------------------------------------------------------------
static BeJpegPixelType computePixelFormat(ImageSourceCR source)
    {
    if (source.HasAlpha())
        return source.IsBGR() ? BE_JPEG_PIXELTYPE_BgrA : BE_JPEG_PIXELTYPE_RgbA;
        
    return source.IsBGR() ? BE_JPEG_PIXELTYPE_Bgr : BE_JPEG_PIXELTYPE_Rgb;
    }
END_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
Image::Image(ImageSourceCR source)
    {
    ByteStream const& input = source.GetByteStream();
    if (source.GetFormat() == ImageSource::Format::Jpeg)
        {
        BeJpegDecompressor reader;
        if (SUCCESS != reader.ReadHeader(m_width, m_height, input.GetData(), input.GetSize()))
            {
            Invalidate();
            return;
            }

        m_image.Resize(m_width * m_height * 4);
        if (SUCCESS != reader.Decompress(m_image.GetDataP(), m_image.GetSize(), input.GetData(), input.GetSize(), computePixelFormat(source), source.IsBottomUp() ? BeJpegBottomUp::Yes : BeJpegBottomUp::No))
            Invalidate();

        return;
        }

    PngReadData pngData(input.GetData(), input.GetSize());
    if (SUCCESS != readPng(*this, pngData))
        Invalidate();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
ImageSource::ImageSource(Format format, ImageCR image, int quality, BottomUp bottomUp) : m_format(format), m_bottomUp(bottomUp)
    {
    m_hasAlpha = image.GetFormat() == Image::Format::Rgba ? Alpha::Yes : Alpha::No;
    m_isBGR = false;

    if (m_format == Format::Jpeg)
        {
        BeJpegCompressor writer;
        if (SUCCESS != writer.Compress(m_stream, image.GetByteStream().GetData(), image.GetWidth(), image.GetHeight(), computePixelFormat(*this), quality, BottomUp::Yes==bottomUp ? BeJpegBottomUp::Yes : BeJpegBottomUp::No))
            m_stream.Clear();

        return;
        }

    BufferWriter writer(m_stream);
    if (SUCCESS != writeImageToPng(writer, image, *this))
        m_stream.Clear();

    }

