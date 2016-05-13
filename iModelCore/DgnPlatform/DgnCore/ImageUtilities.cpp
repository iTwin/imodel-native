/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/ImageUtilities.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

#include <DgnPlatform/ImageUtilities.h>

#if defined (BENTLEY_TOOL_CONTEXT_IsLinuxGcc)
// See pngconf.h
#define PNG_SKIP_SETJMP_CHECK
#endif

#include <png/png.h>
#include <BeJpeg/BeJpeg.h>

namespace {
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
* @bsiclass                                                     Sam.Wilson      08/13
+---------------+---------------+---------------+---------------+---------------+------*/
struct PngReadFile
{
    BeFile& m_file;

    PngReadFile(BeFile& file) : m_file(file) {;}

    static void Read(png_structp png_ptr, png_bytep data, png_size_t length)
        {
        PngReadFile* pngFile = (PngReadFile*)(png_ptr->io_ptr);

        uint32_t bytesRead;
        if (pngFile->m_file.Read(data, &bytesRead, (uint32_t)length) != BeFileStatus::Success || bytesRead != length)
            png_error(png_ptr, "Read Error");   // >>>>  Will long jump and not return.
        }
};
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename READER> static BentleyStatus readPngToBuffer(ByteStream& outPixels, RgbImageInfo& info, READER& reader)
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
    png_set_read_fn(png_ptr, &reader, READER::Read);

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
    info.m_width = info_ptr->width;
    info.m_height = info_ptr->height;
    info.m_hasAlpha = (32 == info_ptr->pixel_depth);     // either RGB or RGBA
    info.m_isBGR = false;
    info.SetTopDown();
    outPixels.Resize((uint32_t) (info.m_height*bytesPerRow));

    for (uint32_t line=0; line < info.m_height; ++line)
        memcpy(outPixels.GetDataP() + line*bytesPerRow, rows_pointers[line], bytesPerRow);

#if 0 //DUMP_TO_FILE
    WString filename;
    filename.Sprintf(L"c:\\pngDump_%dx%d.raw", info_ptr->width, info_ptr->height);
    BeFile dumpFile;
    dumpFile.Create(filename.c_str());
    uint32_t bytesWritten;
    dumpFile.Write(&bytesWritten, &outPixels[0], (uint32_t)outPixels.size());
    dumpFile.Close();
#endif

    // Clean up after the read, and free any memory allocated 
    png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2013
//---------------------------------------------------------------------------------------
BentleyStatus RgbImageInfo::ReadImageFromPngBuffer(Render::Image& image, Byte const*inputBuffer, size_t numberBytes)
    {
    image.Initialize(0, 0, m_hasAlpha ? Render::Image::Format::Rgba : Render::Image::Format::Rgb); // zero...we haven't read the dimensions yet...
    PngReadData pngData(inputBuffer, numberBytes);
    auto status = readPngToBuffer(image.GetByteStreamR(), *this, pngData);
    if (SUCCESS == status)
        image.SetSize(m_width, m_height);   // now we know the dimensions...

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      10/2013
//---------------------------------------------------------------------------------------
BentleyStatus RgbImageInfo::ReadImageFromPngFile(ByteStream& rgbaBuffer, BeFile& file)
    {
    PngReadFile pngFile(file);
    return readPngToBuffer(rgbaBuffer, *this, pngFile);
    }

BEGIN_UNNAMED_NAMESPACE
//=======================================================================================
// @bsiclass                                                    Sam.Wilson  10/13
//=======================================================================================
struct FileWriter
{
    BeFile& m_pngFile;          // The output file

    FileWriter(BeFile& f) : m_pngFile(f) {;}

    static void Write(png_structp png_ptr, png_bytep data, png_size_t length)
        {
        FileWriter* app = (FileWriter*) png_get_io_ptr(png_ptr);
        uint32_t bytesWritten = 0;
        if (app->m_pngFile.Write(&bytesWritten, data, (uint32_t)length) != BeFileStatus::Success || length != bytesWritten)
            {
            png_error(png_ptr, "Write Error");   // >>>>  Will long jump and not return.
            }
        }
};

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

END_UNNAMED_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson  10/2013
//---------------------------------------------------------------------------------------
static BentleyStatus prepareForPng(int& pngformat, bvector <png_bytep>& rows, ByteStream& img, RgbImageInfo const& pngInfo)
    {
    uint32_t bytesPerPixel = pngInfo.m_hasAlpha? 4: 3;
    size_t  lengthInPixels = pngInfo.m_width*pngInfo.m_height;

    //  ---------------------------------------------------------------------------------------------------
    //  PNG does not support BGR*, only RGB*. If the input is BGR, we must swap B and R.
    //  PNG does support formats with an "alpha" byte. This value is the inverse of transparency. 
    //  On Windows, we usually get BGRA images from QV with 0 for the alpha value. That would make 
    //  the image invisible. We fix that in the loop below.
    //  ---------------------------------------------------------------------------------------------------

    if (bytesPerPixel == 3)
        {
        pngformat = PNG_COLOR_TYPE_RGB;
        if (pngInfo.m_isBGR)
            {
            Bgr* pixel = (Bgr*) img.GetData();
            Bgr* pixelEnd = pixel + lengthInPixels;
            for (; pixel < pixelEnd; ++pixel)
                std::swap(pixel->r, pixel->b);
            }
        }
    else if (bytesPerPixel == 4)
        {
        pngformat = PNG_COLOR_TYPE_RGBA;
        
        if (pngInfo.m_isBGR)
            {
            Bgra* pixel = (Bgra*) img.GetData();
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
    rows.resize(pngInfo.m_height);

    int    rowSize = pngInfo.m_width * bytesPerPixel;
    uint8_t* row     = pngInfo.IsTopDown() ? img.GetDataP() : img.GetDataP() + (pngInfo.m_height-1)*rowSize;
    int    rowStep = pngInfo.IsTopDown() ? rowSize : -rowSize;
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
template<typename WRITER> BentleyStatus writeImageToPng(WRITER& writer, ByteStream& imageData, RgbImageInfo const& pngInfo)
    {
    int pngFormat;
    bvector<png_bytep> rowPointers;
    if (prepareForPng(pngFormat, rowPointers, imageData, pngInfo) != BSISUCCESS)
        return BSIERROR;

    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr)
        return BSIERROR;

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
        return BSIERROR;

    if (setjmp(png_jmpbuf(png_ptr)))
        return BSIERROR;

    png_set_write_fn(png_ptr, &writer, WRITER::Write, NULL);

    png_set_IHDR (png_ptr, info_ptr, pngInfo.m_width, pngInfo.m_height, 8, pngFormat, PNG_INTERLACE_NONE,
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
// @bsimethod                                                   Sam.Wilson      10/13
//---------------------------------------------------------------------------------------
BentleyStatus RgbImageInfo::WriteImageToPngFormat(ByteStream& pngData, ByteStream& imageData) const
    {
    BufferWriter writer(pngData);
    return writeImageToPng(writer, imageData, *this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      10/13
//---------------------------------------------------------------------------------------
BentleyStatus RgbImageInfo::WriteImageToPngFile(BeFile& pngFile, ByteStream& imageData) const
    {
    FileWriter writer(pngFile);
    return writeImageToPng(writer, imageData, *this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      10/2013
//---------------------------------------------------------------------------------------
static BeJpegPixelType computePixelFormat(RgbImageInfo const& infoIn)
    {
    if (infoIn.m_hasAlpha)
        return infoIn.m_isBGR ? BE_JPEG_PIXELTYPE_BgrA : BE_JPEG_PIXELTYPE_RgbA;
        
    return infoIn.m_isBGR ? BE_JPEG_PIXELTYPE_Bgr : BE_JPEG_PIXELTYPE_Rgb;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      10/2013
//---------------------------------------------------------------------------------------
static size_t computeBytesPerPixel(RgbImageInfo const& infoIn)
    {
    return infoIn.m_hasAlpha ? 4 : 3;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      10/2013
//---------------------------------------------------------------------------------------
BentleyStatus RgbImageInfo::WriteImageToJpgBuffer(ByteStream& jpegData, ByteStream const& imageData, int quality, BottomUp bottomUp) const
    {
    BeJpegCompressor writer;
    return writer.Compress(jpegData, imageData.GetData(), m_width, m_height, computePixelFormat(*this), quality, BottomUp::Yes==bottomUp ? BeJpegBottomUp::Yes : BeJpegBottomUp::No);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      10/2013
//---------------------------------------------------------------------------------------
BentleyStatus RgbImageInfo::ReadImageFromJpgBuffer(Render::Image& image, Byte const*inputBuffer, size_t inputBufferSize, BottomUp bottomUp)
    {
    BeJpegDecompressor reader;
    if (SUCCESS != reader.ReadHeader(m_width, m_height, inputBuffer, inputBufferSize))
        return ERROR;

    image.Initialize(m_width, m_height, m_hasAlpha ? Render::Image::Format::Rgba : Render::Image::Format::Rgb);
    ByteStream& rgbaBuffer = image.GetByteStreamR();
    rgbaBuffer.Resize((uint32_t) (m_width*m_height*computeBytesPerPixel(*this)));
    return reader.Decompress(rgbaBuffer.GetDataP(), rgbaBuffer.GetSize(), inputBuffer, inputBufferSize, computePixelFormat(*this), BottomUp::Yes==bottomUp ? BeJpegBottomUp::Yes : BeJpegBottomUp::No);
    }

