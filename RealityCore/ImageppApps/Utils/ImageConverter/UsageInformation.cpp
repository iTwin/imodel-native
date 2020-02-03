/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ImageConverter/UsageInformation.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
// $Header: /imagepp-root/Apps/Utils/ImageConverter/UsageInformation.cpp,v 1.4 2008/12/22 16:24:44 Mathieu.St-Pierre Exp $
//-----------------------------------------------------------------------------
// Implementation of UsageInformation
//-----------------------------------------------------------------------------

#include "ImageConverterPch.h"

#include <iostream>
#include <tchar.h>


#include "ConverterUtilities.h"

//-----------------------------------------------------------------------------
// This function print out the version information for the current program.
//-----------------------------------------------------------------------------

#define BUILD_CONSOLE_VERSION_STR(ProductName) ProductName L"Version 8,11,99,99, Bentley Systems Incorporated, Copyright(c) 2013"

void PrintVersionInfo()
{
     wcout << "==========================================================================" << endl
         #ifdef _HMR_ALTAPHOTO
             << BUILD_CONSOLE_VERSION_STR(L"AltaPhotoImageConverter") << endl
         #elif __HMR_EIMAGE
            << BUILD_CONSOLE_VERSION_STR(L"E-Image ImageConverter") << endl
         #elif THUMBNAIL_GENERATOR // HMR ImageConverter thumbnail.
            << BUILD_CONSOLE_VERSION_STR(L"Thumbnail creator") << endl
         #else
            << BUILD_CONSOLE_VERSION_STR(L"MSI ImageConverter") << endl
         #endif
          << "==========================================================================" << endl;
}


#ifdef _HMR_ALTAPHOTO  //  AltaPhotoImageConverter version (!HMR ImageConverter)

//-----------------------------------------------------------------------------
// This function print out the short version of the current program usage.
//
// IMPORTANT: This function never return, it exit from the program.
//-----------------------------------------------------------------------------
void PrintShortUsage()
{
    PrintVersionInfo();

    // Check that we have the right number of parameters.
    cout << endl
         << "AltaPhotoImageConverter [options] <source> [<destination>]" << endl
         << endl
         << "[options]"                                         << endl
         << "-ow               -c<codec>     -q<quality>"       << endl 
         << "-ss               -v            -b<blocktype>"     << endl 
         << "-p<colorspace>    -ab<Blob file name>"             << endl 
         << "-rp<SourceProjection:DestinationProjection:PRECISION>"  << endl 
#ifdef SEB_HMR_ALTAPHOTO
         << "-af<weights>      -at<threshold>"                  << endl 
#endif
         << "-?"                                                << endl 
                                                                << endl
         << "EXAMPLES          The following example displays more help about"     << endl
         << "                  AltaPhotoImageConverter."        << endl 
                                                                << endl
         << "                    AltaPhotoImageConverter -?"             << endl 
                                                                << endl
         << "                  The following example converts the specified JPEG"  << endl
         << "                  image to iTIFF format using the default settings."  << endl
                                                                << endl
         << "                    AltaPhotoImageConverter c:\\images\\i1.jpg c:\\images\\i1.itiff" << endl
                                                                << endl
         << "                  The following example converts the all JPEG images" << endl
         << "                  in c:\\images and its sub-directories to grayscale"  << endl
         << "                  iTIFF image files."              << endl 
                                                                << endl
         << "                    AltaPhotoImageConverter -pGrayscale8 -ss c:\\images\\*.jpg c:\\images "    << endl 
         << endl
         <<  endl;
     exit(1);
}

#elif THUMBNAIL_GENERATOR // HMR ImageConverter thumbnail.

//-----------------------------------------------------------------------------
// This function print out the short version of the current program usage.
//
// IMPORTANT: This function never return, it exit from the program.
//-----------------------------------------------------------------------------
void PrintShortUsage()
{
    PrintVersionInfo();

    // Check that we have the right number of parameters.
    cout << endl
         << "ICThumbnail [options] <source> [<destination>]" << endl
         << endl
         << "[options]"                                         << endl
         << "-t<width,height>"                                  << endl
         << "-ow       -v         -ss            -?"            << endl 
         << endl
         << endl
         << "EXAMPLES          The following example displays more help about"           << endl
         << "                  ICThumbnail."                                            << endl 
                                                                                         << endl
         << "                    ICThumbnail -?"                                         << endl 
                                                                                         << endl
         << "                  The following example create a thumbnail for"             << endl
         << "                  the specified JPEG using the default settings."           << endl
                                                                                         << endl
         << "                    ICThumbnail -t96,96 c:\\images\\i1.jpg"                 << endl
                                                                                         << endl                  
         << "                  The following example create a thumbnail for all JPEG images in" << endl
         << "                  c:\\images and its sub-directories"                    << endl
                                                                                         << endl
         << "                    ICThumbnail -ss -t96,96 c:\\images\\*.jpg c:\\images"  << endl
         << endl
         << endl;
     exit(1);
}

#else    //  HMR ImageConverter version (!AltaPhotoImageConverter)

//-----------------------------------------------------------------------------
// This function print out the short version of the current program usage.
//
// IMPORTANT: This function never return, it exit from the program.
//-----------------------------------------------------------------------------
void PrintShortUsage()
{
    PrintVersionInfo();

    // Check that we have the right number of parameters.
    cout << endl
         << "ImageConverter [options] <source> [<destination>]" << endl
         << endl
         << "[options]"                                         << endl
         << "-ow               -c<codec>         -q<quality>"   << endl 
         << "-ss               -v                -b<blocktype>" << endl 
         << "-p<colorspace>    -cpss             -cpssc"        << endl 
         << "-cdim<dimension>  -mdim<dimension>  -?"            << endl          
         << endl
         << endl
         << "EXAMPLES          The following example displays more help about"     << endl
         << "                  ImageConverter."                 << endl 
                                                                << endl
         << "                    imageconverter -?"             << endl 
                                                                << endl
         << "                  The following example converts the specified JPEG"  << endl
         << "                  image to iTIFF format using the default settings."  << endl
                                                                << endl
         << "                    imageconverter c:\\images\\i1.jpg c:\\images\\i1.itiff" << endl
                                                                << endl
         << "                  The following example converts the all JPEG images" << endl
         << "                  in c:\\images and its sub-directories to grayscale"  << endl
         << "                  iTIFF image files."              << endl 
                                                                << endl
         << "                    imageconverter -pGrayscale8 -ss c:\\images\\*.jpg c:\\images "    << endl 
         << endl
         <<  endl;
     exit(1);
}
#endif


#ifdef _HMR_ALTAPHOTO  //  AltaPhotoImageConverter version (!HMR ImageConverter)

//-----------------------------------------------------------------------------
// This function print out the complete usage for the current program.
//
// IMPORTANT: This function never return, it exit from the program.
//-----------------------------------------------------------------------------

void PrintExtendedUsage()
{
    PrintVersionInfo();

    // Check that we have the right number of parameters.
    cout << endl
         << "AltaPhotoImageConverter [options] <source> [<destination>]"                          << endl
                                                                                                  << endl
         << "<source>          Name of source file(s) to convert. Must be"                        << endl
         << "                  specified in one of the following ways:"                           << endl
                                                                                                  << endl
         << "                  - Path name to single image"                                       << endl
         << "                  - Directory name containing set of images to convert"              << endl
         << "                  - Wildcard path name specifying set of images to convert"          << endl
                                                                                                  << endl
         << "                  Supported input formats:"                                          << endl
                                                                                                  << endl
         << "                  HMR        iTIFF     TIF     GeoTIFF"                              << endl
         << "                  JPEG       GIF       BMP"                                          << endl
         << "                  CALS       INTERGRAPH (cit, cot, rle, tg4, rgb)"                   << endl
                                                                                                  << endl
         << "[<destination>]   Name of the output iTIFF file or of the output directory"          << endl
                                                                                                  << endl
         << "                  If the source is a single image, this can be"                      << endl
         << "                  the name of a file or a directory. If the source"                  << endl
         << "                  is a wildcard or directory, this must be the name"                 << endl
         << "                  of a directory. The default is the directory of the"               << endl
         << "                  source."                                                           << endl
                                                                                                  << endl
         << "[options]"                                                                           << endl
                                                                                                  << endl
         << "-?                Displays this page of help."                                       << endl
                                                                                                  << endl
         << "-ss               If this switch is specified and if a wildcard or"                  << endl
         << "                  directory is specified as the source, the converter"               << endl
         << "                  will scan the sub-directories in addition to the"                  << endl
         << "                  specified directory."                                              << endl
                                                                                                  << endl
         << "-dn<namingrule>   Specifies the rule to be used to name the output"                  << endl
         << "                  files. By default the ReplaceExt rule applies. Must"               << endl
         << "                  be one of the following:"                                          << endl
                                                                                                  << endl
         << "                  ReplaceExt   (ex: a.bmp --> a.itiff)"                              << endl
         << "                  RetainExt    (ex: a.bmp --> a_bmp.itiff)"                          << endl
                                                                                                  << endl
         << "-ow               If this switch is specified, any existing output"                  << endl
         << "                  file will be overwritten. Otherwise, a message will"               << endl
         << "                  indicate that the output file already exists, and"                 << endl
         << "                  the conversion of that file will not be performed."                << endl
                                                                                                  << endl
         << "-v                If this switch is specified, a progression time"                   << endl
         << "                  is displayed on the screen."                                       << endl
                                                                                                  << endl
         << "-p<colorspace>    Specifies the color space of the output image."                    << endl
         << "                  The default is the best match for the color space"                 << endl
         << "                  of the input. Must be one of the following:"                       << endl
                                                                                                  << endl
         << "                  Grayscale1"                                                        << endl
         << "                  256Colors"                                                         << endl
         << "                  TrueColor24"                                                       << endl
         << "                  Grayscale8"                                                        << endl
                                                                                                  << endl                            
         << "-c<codec>         Specifies the type of image compression of the"                    << endl
         << "                  output image. The default is -cNone."                              << endl
         << "                  Must be one of the following:"                                     << endl
                                                                                                  << endl
         << "                  None          (applies to all color spaces)"                       << endl
         << "                  Deflate       (applies to all color spaces)"                       << endl
         << "                  CCITT         (applies to 1 bit)"                                  << endl
         << "                  JPEG          (applies to 24 bits and grayscale8)"                 << endl
         << "                  AltaPhotoJPEG (applies to 24 bits only)"                           << endl
                                                                                                  << endl
         << "-q<quality>       Specifies the percentage of quality between"                       << endl
         << "                  1 and 100 for the JPEG codec. Default is 80."                      << endl
                                                                                                  << endl
         << "-e<encoding>      Specifies the depth organization of the output"                    << endl
         << "                  file. Default is -eMultires. Must be one"                          << endl
         << "                  of the following:"                                                 << endl
                                                                                                  << endl
         << "                  Multires"                                                          << endl
                                                                                                  << endl
#if 0 // Not supported by ITiff
         << "                  Standard"                                                          << endl
#endif
         << "-mr<method>       Specifies the method for resampling overviews when"                << endl
         << "                  Multires is the selected encoding. Must be one of"                 << endl
         << "                  the following:"                                                    << endl
                                                                                                  << endl
         << "                  NearestNeighbor    (applies to all color spaces)"                  << endl
         << "                  Average            (applies to any color spaces without palette)"  << endl

         << "                  oring4,<0 or 1>    (specify the forground index, applies to 1 bit palette)" << endl

                                                                                                  << endl
         << "-b<blocktype>     Specifies the planar organization of the output"                   << endl
         << "                  file. Default is -bTile. Must be one of the"                       << endl
         << "                  following:"                                                        << endl
                                                                                                  << endl
         << "                  Tile"                                                              << endl
         << "                  Strip"                                                             << endl
                                                                                                  << endl
         << "-ab<Blob fileName>"                                                                  << endl
         << "                  Specifies the source file containing the blob information"         << endl
         << "                  to inject in the Alta Photo blob field of the output file."        << endl
                                                                                                  << endl
#ifdef SEB_HMR_ALTAPHOTO
         << "-af<weights>      Specifies a perfectly symmetric 3 by 3 filter to apply during"     << endl
         << "                  copy of the base resolution.  This parameter is incompatible"      << endl
         << "                  with use of -mrCopyPyramid sampling method.  Weights are"          << endl
         << "                  specified by 3 numbers placed between quotes such as for"          << endl
         << "                  following example:"                                                << endl
         << "                       -af\"0.5, 0.12, -0.17\""                                      << endl
                                                                                                  << endl
         << "-at<threshold>    When the -at flag is used, it is possible to specifiy a threshold" << endl
         << "                  that will indicate for each and every pixel filtered if the"       << endl
         << "                  original value must remain or replaced by the result of the"       << endl
         << "                  convolution filter.  The threshold is a value between 0.0 and"     << endl
         << "                  255.0 inclusive.  If no threshold is specified, the pixel value"   << endl
         << "                  will always be replaced by the result of the convolution filter."  << endl
#endif
         << "-rp<DatabaseFileName,SourceProjection,DestinationProjection:PRECISION>            "  << endl 
         << "                  Allows reprojection during export process from one cartographic "  << endl
         << "                  projection to another. If the input file contains projection    "  << endl
         << "                  information and this information is incompatible with indicated "  << endl
         << "                  SourceProjection, an error is reported. Specifications of       "  << endl
         << "                  source and destination projection is done by indicating the     "  << endl
         << "                  EPSG code corresponding to the projection.                      "  << endl
         << "                  The precision allows to validate the transformation by          "  << endl
         << "                  indicating the maximum deviation acceptable and thus enables    "  << endl
         << "                  the program to accelerate the transformation by making linearity"  << endl
         << "                  assumptions. This precision is indicated in meters for the"        << endl
         << "                  destination projection. Use of this option implies use of the -rf" << endl
         << "                  option"                                                            << endl
         << "-rf<DatabaseFileName>                                          "  << endl 
         << "                  The projection database file name must be provided"   << endl
         << "                  as an absolute path in the form e:\\DataGeo\\GeoCalc.dat"          << endl
                                                                                                  << endl
         << "EXAMPLES          The following example converts the specified JPEG"                 << endl
         << "                  image to iTIFF format using the default settings."                 << endl
                                                                                                  << endl
         << "                    AltaPhotoImageConverter c:\\images\\i1.jpg c:\\images\\i1.itiff" << endl
                                                                                                  << endl                  
         << "                  The following example converts the all JPEG images"                << endl
         << "                  in c:\\images and its sub-directories to grayscale "               << endl
         << "                  iTIFF image files."                                                << endl
                                                                                                  << endl
         << "                    AltaPhotoImageConverter -cAltaPhotoJPEG -ss c:\\images\\*.jpg c:\\images"  << endl
                                                                                                  << endl                  
         << "                  The following example converts all images (TIF or GeoTIFF)  "      << endl
         << "                  in c:\\images and its sub-directories to iTIFF files"              << endl
         << "                  performing in the same process a reprojection from UTM-01N "       << endl
         << "                  to UTM-02N projection with an acceptable precision of 0.15 meters " << endl
                                                                                                  << endl
         << "                    AltaPhotoImageConverter -cAltaPhotoJPEG -rf e:\\DataGeo\\GeoCalc.dat -rp \"32601, 32602, 0.15\" -ss c:\\images\\*.tif c:\\images"  << endl
         
                                                                                                  <<  endl;
     exit(1);
}

#elif THUMBNAIL_GENERATOR // HMR ImageConverter Thumbnail.

void PrintExtendedUsage()
{
    PrintVersionInfo();

    // Check that we have the right number of parameters.
    cout << endl
         << "ICThumbnail [options] <source> [<destination>]"                          << endl
                                                                                         << endl
         << "<source>          Name of source file(s) to convert. Must be"               << endl
         << "                  specified in one of the following ways:"                  << endl
                                                                                         << endl
         << "                  - Path name to single image"                              << endl
         << "                  - Directory name containing set of images to create thumbnail"     << endl
         << "                  - Wildcard path name specifying set of images to create thumbnail" << endl
                                                                                         << endl
         << "                  Supported input formats:"                                 << endl
                                                                                         << endl
#ifdef __HMR_EIMAGE
         << "                  HMR        iTIFF     TIF     GeoTIFF"                     << endl
         << "                  JPEG       GIF       BMP"                                 << endl
         << "                  FlashPix   PNG"                                           << endl
                                                                                         << endl
#else
         << "                  HMR        iTIFF     TIF     GeoTIFF"                     << endl
         << "                  JPEG       GIF       BMP"                                 << endl
         << "                  CALS       INTERGRAPH (cit, cot, rle, tg4, rgb)"          << endl
#endif
                                                                                         << endl
         << "[<destination>]   Name of the output jpg file or of the output directory"   << endl
                                                                                         << endl
         << "                  If the source is a single image, this can be"             << endl
         << "                  the name of a file or a directory. If the source"         << endl
         << "                  is a wildcard or directory, this must be the name"        << endl
         << "                  of a directory. The default is the directory of the"      << endl
         << "                  source."                                                  << endl
                                                                                         << endl
         << "[options]"                                                                  << endl
                                                                                         << endl
         << "-?                Displays this page of help."                              << endl
                                                                                         << endl
         << "-ss               If this switch is specified and if a wildcard or"         << endl
         << "                  directory is specified as the source, the converter"      << endl
         << "                  will scan the sub-directories in addition to the"         << endl
         << "                  specified directory."                                     << endl
                                                                                         << endl
         << "-ow               If this switch is specified, any existing output"         << endl
         << "                  file will be overwritten. Otherwise, a message will"      << endl
         << "                  indicate that the output file already exists, and"        << endl
         << "                  the conversion of that file will not be performed."       << endl
                                                                                         << endl
         << "-v                If this switch is specified, a progression time"                   << endl
         << "                  is displayed on the screen."                                       << endl
                                                                                                  << endl
         << "-t<width,height>  Specifies the width and the height of the ouput file."    << endl
         << "                  Width and height must be less than 257. The aspect ratio" << endl
         << "                  is always maintained. Default value is 96,96."            << endl
                                                                                         << endl
         << "EXAMPLES          The following example create a thumbnail for"             << endl
         << "                  the specified JPEG using the default settings."           << endl
                                                                                         << endl
         << "                    ICThumbnail -t96,96 c:\\images\\i1.jpg"              << endl
                                                                                         << endl                  
         << "                  The following example create a thumbnail for all JPEG images" << endl
         << "                  in c:\\images and its sub-directories"                    << endl
                                                                                         << endl
         << "                    ICThumbnail -ss -t96,96 c:\\images\\*.jpg c:\\images"  << endl
         <<  endl;
     exit(1);
}

#else    //  HMR ImageConverter version (!AltaPhotoImageConverter)

//-----------------------------------------------------------------------------
// This function print out the complete usage for the current program.
//
// IMPORTANT: This function never return, it exit from the program.
//-----------------------------------------------------------------------------

void PrintExtendedUsage()
{
    PrintVersionInfo();

    // Check that we have the right number of parameters.
    cout << endl
         << "ImageConverter [options] <source> [<destination>]"                          << endl
                                                                                         << endl
         << "<source>          Name of source file(s) to convert. Must be"               << endl
         << "                  specified in one of the following ways:"                  << endl
                                                                                         << endl
         << "                  - Path name to single image"                              << endl
         << "                  - Directory name containing set of images to convert"     << endl
         << "                  - Wildcard path name specifying set of images to convert" << endl
                                                                                         << endl
         << "                  Supported input formats:"                                 << endl
                                                                                         << endl
#ifdef __HMR_EIMAGE
         << "                  HMR        iTIFF     TIF     GeoTIFF"                     << endl
         << "                  JPEG       GIF       BMP"                                 << endl
         << "                  FlashPix   PNG"                                           << endl
                                                                                         << endl
#else
         << "                  HMR        iTIFF     TIF     GeoTIFF"                     << endl
         << "                  JPEG       GIF       BMP     PSS"                                 << endl
         << "                  CALS       INTERGRAPH (cit, cot, rle, tg4, rgb)"          << endl
#endif
                                                                                         << endl
         << "[<destination>]   Name of the output iTIFF file, the output directory or"   << endl
         << "                  the output PSS's cTiff cache file name."                                << endl
                                                                                         << endl
         << "                  If the source is a single image, this can be"             << endl
         << "                  the name of a file or a directory. If the source"         << endl
         << "                  is a wildcard or directory, this must be the name"        << endl
         << "                  of a directory. If the option -pss is specified,"         << endl
         << "                  this must be the name of the generated PSS file."         << endl                  
         << "                  The default is the directory of the source."              << endl                  
                                                                                         << endl
         << "[options]"                                                                  << endl
                                                                                         << endl
         << "-?                Displays this page of help."                              << endl
                                                                                         << endl
         << "-ss               If this switch is specified and if a wildcard or"         << endl
         << "                  directory is specified as the source, the converter"      << endl
         << "                  will scan the sub-directories in addition to the"         << endl
         << "                  specified directory."                                     << endl
                                                                                         << endl
         << "-dn<namingrule>   Specifies the rule to be used to name the output"         << endl
         << "                  files. By default the ReplaceExt rule applies. Must"      << endl
         << "                  be one of the following:"                                 << endl
                                                                                         << endl
         << "                  ReplaceExt   (ex: a.bmp --> a.itiff)"                     << endl
         << "                  RetainExt    (ex: a.bmp --> a_bmp.itiff)"                 << endl
                                                                                         << endl
         << "-ow               If this switch is specified, any existing output"         << endl
         << "                  file will be overwritten. Otherwise, a message will"      << endl
         << "                  indicate that the output file already exists, and"        << endl
         << "                  the conversion of that file will not be performed."       << endl
                                                                                         << endl
         << "-v                If this switch is specified, a progression time"                   << endl
         << "                  is displayed on the screen."                                       << endl
                                                                                                  << endl
         << "-p<colorspace>    Specifies the color space of the output image."           << endl
         << "                  The default is the best match for the color space"        << endl
         << "                  of the input. Must be one of the following:"              << endl
                                                                                         << endl
         << "                  Grayscale1"                                               << endl
         << "                  256Colors"                                                << endl
         << "                  TrueColor24"                                              << endl
         << "                  Grayscale8"                                               << endl
                                                                                         << endl                            
         << "-c<codec>         Specifies the type of image compression of the"           << endl
         << "                  output image. The default is -cNone."                     << endl
         << "                  Must be one of the following:"                            << endl
                                                                                         << endl
         << "                  None         (applies to all color spaces)"               << endl
         << "                  Deflate      (applies to all color spaces)"               << endl
         << "                  CCITT        (applies to 1 bit)"                          << endl
         << "                  JPEG         (applies to 24 bits and grayscale8)"         << endl
                                                                                         << endl
         << "-q<quality>       Specifies the percentage of quality between"              << endl
         << "                  1 and 100 for the JPEG codec. Default is 80."             << endl
                                                                                         << endl
         << "-e<encoding>      Specifies the depth organization of the output"           << endl
         << "                  file. Default is -eMultires. Must be one"                 << endl
         << "                  of the following:"                                        << endl
                                                                                         << endl
         << "                  Multires"                                                 << endl
#if 0 // Not supported by ITiff
         << "                  Standard"                                                 << endl
                                                                                         << endl
#endif
         << "-mr<method>       Specifies the method for resampling overviews when"       << endl
         << "                  Multires is the selected encoding. Must be one of"        << endl
         << "                  the following:"                                           << endl
                                                                                         << endl
         << "                  NearestNeighbor    (applies to all color spaces)"         << endl
         << "                  Average            (applies to any color spaces without palette)" << endl

         << "                  oring4,<0 or 1>    (specify the forground index, applies to 1 bit palette)" << endl

                                                                                         << endl                   
         << "-b<blocktype>     Specifies the planar organization of the output"          << endl
         << "                  file. Default is -bTile. Must be one of the"              << endl
         << "                  following:"                                               << endl
                                                                                         << endl
         << "                  Tile"                                                     << endl
         << "                  Strip"                                                    << endl
                                                                                         << endl
         << "-cpss             Specifies the creation of a PSS file describing a mosaic" << endl
         << "                  whose images are those found in the specified source"     << endl
         << "                  directory."                                               << endl         
                                                                                         << endl                                                                                                                                                                                         
         << "-cpssc            Specifies the creation of a cache file for a PSS file"    << endl
         << "                  with one page that is an on-demand mosaic raster. To "    << endl         
         << "                  control the generation of the cache file, the        "    << endl         
         << "                  following options can be specified : "                    << endl         
         << "                  <options>                            "                    << endl         
         << "                  [cdim=<dim in pixel|percentage>]     "                    << endl         
         << "                  [mdim=<dim in pixel>]                "                    << endl                           
                                                                                         << endl                  
         << "-cdim<dimension>  Specifies the dimension of the cache file. The value "    << endl
         << "                  can be an integer or a percentage as shown below. The "   << endl
         << "                  default value is "                                        << 
                               PSS_MIN_CACHE_DIMENSION_IN_PERCENT << "%."                << endl                                    
         << "                  (ex: for a 5000 X 2000 PSS file "                         << endl                  
         << "                       cdim500 --> a 500 X 200 cache file "                 << endl                  
         << "                       cdim50% --> a 2500 X 1000 cache file)"               << endl                  
                                                                                         << endl                  
         << "-mdim<dimension>  Specifies the minimum number of pixels one of the PSS "   << endl                  
         << "                  dimension must have in order for a cache to be created."  << endl                          
         << "                  The default value is " << PSS_MIN_DIMENSION_FOR_CACHE     << 
                                                                                     "." << endl         
         << "                  (ex: for a 5000 X 2000 PSS file"                          << endl                  
         << "                       mdim=4000 --> a cache file is generated"             << endl                  
         << "                       mdim=2000 --> no cache file is generated)"           << endl                                                                                                                    
                                                                                         << endl                                                                                                                                                                                                             
         << "EXAMPLES          The following example converts the specified JPEG"        << endl
         << "                  image to iTIFF format using the default settings."        << endl
                                                                                         << endl
         << "                    imageconverter c:\\images\\i1.jpg c:\\images\\i1.itiff" << endl
                                                                                         << endl                  
         << "                  The following example converts the all JPEG images"       << endl
         << "                  in c:\\images and its sub-directories to grayscale "       << endl
         << "                  iTIFF image files."                                       << endl
                                                                                         << endl
         << "                    imageconverter -pGrayscale8 -ss c:\\images\\*.jpg c:\\images"  << endl
         <<  endl;
     exit(1);
}

#endif