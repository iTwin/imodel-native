/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/ImageSource.cpp $
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
struct PngData
{
private:
    Byte const* m_pData;
    size_t      m_size;
    size_t      m_position;

public:
    PngData(Byte const* pPngData, size_t dataSize) : m_pData(pPngData), m_size(dataSize), m_position(0){}

    static void Read(png_structp png_ptr, png_bytep data, png_size_t length)
        {
        PngData* pngData = (PngData*)(png_ptr->io_ptr);

        if (pngData->m_position + length > pngData->m_size)
            png_error(png_ptr, "Read Error");   // >>>>  Will long jump and not return.

        memcpy(data, pngData->m_pData+pngData->m_position, length);
        pngData->m_position+=length;
        }

    BentleyStatus DoRead(Image& image);
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   05/16
//=======================================================================================
struct PngReader
{
    png_structp m_png = nullptr;
    png_infop m_info = nullptr;

    PngReader(PngData& data) 
        {
        m_png = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0); 
        if (nullptr == m_png)
            return;

        m_info = png_create_info_struct(m_png);

        // Register our png data reader.
        png_set_read_fn(m_png, &data, PngData::Read);

        // set no signature
        png_set_sig_bytes(m_png, 0);
        }

    ~PngReader() {if (m_png) png_destroy_read_struct(&m_png, &m_info, png_infopp_NULL);}
    void Read(Image::Format format) 
        {
        // PNG_TRANSFORM_STRIP_16       >> strip 16 bit/color files down to 8 bits per color.
        // PNG_TRANSFORM_PACKING        >> forces 8 bit
        // PNG_TRANSFORM_EXPAND         >> forces to expand a palette into RGB
        // PNG_TRANSFORM_GRAY_TO_RGB    >> convert grayscale to rgb.
        int imgTransforms = PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_EXPAND | PNG_TRANSFORM_GRAY_TO_RGB | PNG_TRANSFORM_PACKING;

        if (format == Image::Format::Rgb)
            imgTransforms |= PNG_TRANSFORM_STRIP_ALPHA; // we don't want alpha channel
        else
            png_set_add_alpha(m_png, 0xff, PNG_FILLER_AFTER);

        png_read_png(m_png, m_info, imgTransforms, png_voidp_NULL);
        }

    Byte** GetRows() const {return png_get_rows(m_png, m_info);}
    int GetBytesPerRow() const {return png_get_rowbytes(m_png, m_info);}
    int GetPixelDepth() const {return m_info->pixel_depth;}
    bool IsValid() const{return nullptr != m_png;}
    int GetWidth() const {return m_info->width;}
    int GetHeight()const {return m_info->height;}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus PngData::DoRead(Image& image)
    {
    // Create and initialize the png_struct with default error handler functions(setjmp/longjmp).
    PngReader pngReader(*this);
    if (!pngReader.IsValid())
        return ERROR;

    // Set error handling. setjmp/longjmp method (this is the normal method of doing things with libpng)
    if (setjmp(png_jmpbuf(pngReader.m_png)))
        {
        BeAssert(false);
        return ERROR;   // If we get here, we had a problem reading the file
        }

    pngReader.Read(image.GetFormat());

    Byte** rowPointers = pngReader.GetRows();
    int bytesPerRow = pngReader.GetBytesPerRow();

    // make sure we got our expected output
    if (!((image.GetFormat()==Image::Format::Rgb && 24 == pngReader.GetPixelDepth()) || 32 == pngReader.GetPixelDepth()))
        {
        BeAssert(false);
        return ERROR;
        }
        
    // Fill in the outputs.
    image.SetSize(pngReader.GetWidth(), pngReader.GetHeight());
    image.GetByteStreamR().Resize(pngReader.GetHeight() * bytesPerRow);

    Byte* out = image.GetByteStreamR().GetDataP();
    for (int line=0; line < pngReader.GetHeight(); ++line)
        memcpy(out + line*bytesPerRow, rowPointers[line], bytesPerRow);

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

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson  10/2013
//---------------------------------------------------------------------------------------
static BentleyStatus prepareForPng(int& pngformat, bvector <png_bytep>& rows, ImageCR img, ImageSource const& source, Image::BottomUp bottomUp)
    {
    int bytesPerPixel;
    if (img.GetFormat() == Image::Format::Rgb)
        {
        pngformat = PNG_COLOR_TYPE_RGB;
        bytesPerPixel = 3;
        }
    else if (img.GetFormat() == Image::Format::Rgba)
        {
        pngformat = PNG_COLOR_TYPE_RGBA;
        bytesPerPixel = 4;
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
    bool isTopDown = bottomUp==Image::BottomUp::No;
    uint8_t* row = isTopDown ? img.GetByteStream().GetDataP() : img.GetByteStream().GetDataP() + (img.GetHeight()-1)*rowSize;
    int    rowStep = isTopDown ? rowSize : -rowSize;
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
static BentleyStatus writeImageToPng(BufferWriter& writer, ImageCR imageData, ImageSource const& source, Image::BottomUp bottomUp)
    {
    int pngFormat;
    bvector<png_bytep> rowPointers;
    if (prepareForPng(pngFormat, rowPointers, imageData, source, bottomUp) != BSISUCCESS)
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

END_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
Image::Image(ImageSourceCR source, Format targetFormat, Image::BottomUp bottomUp)
    {
    m_format = targetFormat;

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

        BeJpegPixelType fmt = m_format == Format::Rgb ? BE_JPEG_PIXELTYPE_Rgb : BE_JPEG_PIXELTYPE_RgbA;
        if (SUCCESS != reader.Decompress(m_image.GetDataP(), m_image.GetSize(), input.GetData(), input.GetSize(), fmt, bottomUp==Image::BottomUp::Yes ? BeJpegBottomUp::Yes : BeJpegBottomUp::No))
            Invalidate();

        return;
        }

    PngData pngData(input.GetData(), input.GetSize());
    if (SUCCESS != pngData.DoRead(*this))
        Invalidate();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
ImageSource::ImageSource(ImageCR image, Format format, int quality, Image::BottomUp bottomUp)
    {
    m_format = format;
    if (m_format == Format::Jpeg)
        {
        BeJpegCompressor writer;
        if (SUCCESS == writer.Compress(m_stream, image.GetByteStream().GetData(), image.GetWidth(), image.GetHeight(), 
                                       image.GetFormat()==Image::Format::Rgb ? BE_JPEG_PIXELTYPE_Rgb : BE_JPEG_PIXELTYPE_RgbA, 
                                       quality, 
                                       Image::BottomUp::Yes==bottomUp ? BeJpegBottomUp::Yes : BeJpegBottomUp::No))
            return;

        }
    else if (m_format==Format::Png)
        {
        BufferWriter writer(m_stream);
        if (SUCCESS == writeImageToPng(writer, image, *this, bottomUp))
            return;
        }

    m_stream.Clear();
    }
