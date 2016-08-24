/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ImagePP/all/h/ImagePPMessages.xliff.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/WString.h>
#include <BeSQLite/L10N.h>

BENTLEY_TRANSLATABLE_STRINGS_START(ImagePPMessages,ImagePPMessages)
    L10N_STRING(IMAGEPP_NotFound)                        // =="Not found!"==
    // Codecs
    L10N_STRING(CODEC_Identity)                          // =="None"==
    L10N_STRING(CODEC_CCITT3)                            // =="CCITT3"==
    L10N_STRING(CODEC_CCITT4)                            // =="CCITT4"==
    L10N_STRING(CODEC_Deflate)                           // =="Deflate"==
    L10N_STRING(CODEC_Packbits)                          // =="Packbits"==
    L10N_STRING(CODEC_LZW)                               // =="LZW"==
    L10N_STRING(CODEC_JPEG)                              // =="JPEG"==
    L10N_STRING(CODEC_FlashPix)                          // =="FlashPix"==
    L10N_STRING(CODEC_RLE1)                              // =="RLE 1"==
    L10N_STRING(CODEC_RLE8)                              // =="RLE8"==
    L10N_STRING(CODEC_BMPRLE8)                           // =="BMPRLE8"==
    L10N_STRING(CODEC_BMPRLE4)                           // =="BMPRLE4"==
    L10N_STRING(CODEC_Gif)                               // =="Gif"==
    L10N_STRING(CODEC_FlashPixSingleColor)               // =="FlashPix (single color)"==
    L10N_STRING(CODEC_Jpeg2000)                          // =="JPEG 2000"==
    L10N_STRING(CODEC_CCITTRLE)                          // =="CCITTRLE"==
    L10N_STRING(CODEC_TgaRLE)                            // =="Tga RLE"==
    L10N_STRING(CODEC_PCX)                               // =="PCX"==
    L10N_STRING(CODEC_TGARLE)                            // =="TGARLE"==
    L10N_STRING(CODEC_ECW)                               // =="ECW"==
    L10N_STRING(CODEC_CRL8)                              // =="CRL8"==
    L10N_STRING(CODEC_CCITTFax4)                         // =="CCITTFax4"==
    L10N_STRING(CODEC_JpegAlpha)                         // =="JpegAlpha"==
    L10N_STRING(CODEC_FLIRLE8)                           // =="FLI_RLE8bits"==
    L10N_STRING(CODEC_LRDRLE1)                           // =="LRD RLE 1bit"==
    L10N_STRING(CODEC_JBIG)                              // =="JBIG"==

    // Pixeltypes
    L10N_STRING(PIXELTYPE_BlackAndWhite)                 // =="Black & white"==
    L10N_STRING(PIXELTYPE_WhiteAndBlack)                 // =="White & black"==
    L10N_STRING(PIXELTYPE_2ColorsPalette)                // =="2 colors"==
    L10N_STRING(PIXELTYPE_2ColorsPaletteRLE)             // =="2 colors RLE"==
    L10N_STRING(PIXELTYPE_2ColorsPaletteAlpha)           // =="2 colors alpha"==
    L10N_STRING(PIXELTYPE_2ColorsPaletteAlphaRLE)        // =="2 colors alpha RLE"==
    L10N_STRING(PIXELTYPE_4Colors)                       // =="4 colors"==
    L10N_STRING(PIXELTYPE_16Colors)                      // =="16 colors"==
    L10N_STRING(PIXELTYPE_16ColorsAlpha)                 // =="16 colors alpha"==
    L10N_STRING(PIXELTYPE_256Colors)                     // =="256 colors"==
    L10N_STRING(PIXELTYPE_256ColorsAlpha)                // =="256 colors alpha"==
    L10N_STRING(PIXELTYPE_256ColorsARGB)                 // =="256 colors ARGB"==
    L10N_STRING(PIXELTYPE_256ColorsDCMask)               // =="256 colors DC Mask"==
    L10N_STRING(PIXELTYPE_Gray8Palette)                  // =="Gray 8 Palette"==
    L10N_STRING(PIXELTYPE_Gray8)                         // =="Gray 8"==
    L10N_STRING(PIXELTYPE_Gray8White)                    // =="Gray 8 inv."==
    L10N_STRING(PIXELTYPE_Gray8Alpha)                    // =="Gray 8 alpha"==
    L10N_STRING(PIXELTYPE_Gray16)                        // =="Gray 16"==
    L10N_STRING(PIXELTYPE_Integer16)                     // =="Integer 16"==
    L10N_STRING(PIXELTYPE_TrueColors16)                  // =="True colors 16"==
    L10N_STRING(PIXELTYPE_TrueColors24)                  // =="True colors 24"==
    L10N_STRING(PIXELTYPE_TrueColors24BGR)               // =="True colors 24 BGR"==
    L10N_STRING(PIXELTYPE_TrueColors32)                  // =="True colors 32"==
    L10N_STRING(PIXELTYPE_TrueColors32ARGB)              // =="True colors 32 ARGB"==
    L10N_STRING(PIXELTYPE_TrueColors32BGRX)              // =="True colors 32 BGRX"==
    L10N_STRING(PIXELTYPE_TrueColors32RGBX)              // =="True colors 32 no Alpha"==
    L10N_STRING(PIXELTYPE_TrueColors32PreMult)           // =="True colors 32 PR"==
    L10N_STRING(PIXELTYPE_TrueColors48)                  // =="True colors 48"==
    L10N_STRING(PIXELTYPE_TrueColors64)                  // =="True colors 64"==
    L10N_STRING(PIXELTYPE_TrueColors64RGBX)              // =="True colors 64 no Alpha"==
    L10N_STRING(PIXELTYPE_TrueColors96)                  // =="True Colors 96"==
    L10N_STRING(PIXELTYPE_Float32)                       // =="Float 32"==
    L10N_STRING(PIXELTYPE_CMYK)                          // =="CMYK"==
    L10N_STRING(PIXELTYPE_YCC)                           // =="YCC"==
    L10N_STRING(PIXELTYPE_YCCAlpha)                      // =="YCC alpha"==

    // File formats
    L10N_STRING(FILEFORMAT_ArcInfoASCII)                 // =="ArcInfo ASCII Grid"==
    L10N_STRING(FILEFORMAT_ArcInfoBinary)                // =="ArcInfo Binary Grid"==
    L10N_STRING(FILEFORMAT_Bil)                          // =="ESRI BIL"==
    L10N_STRING(FILEFORMAT_BingMaps)                     // =="BingMaps Server"==
    L10N_STRING(FILEFORMAT_BMP)                          // =="Window BMP"==
    L10N_STRING(FILEFORMAT_BSB)                          // =="BSB Navigation Chart"==
    L10N_STRING(FILEFORMAT_C29)                          // =="C29"==
    L10N_STRING(FILEFORMAT_C30)                          // =="C30"==
    L10N_STRING(FILEFORMAT_C31)                          // =="C31"==
    L10N_STRING(FILEFORMAT_CALS)                         // =="Cals Type 1 CCITT4"==
    L10N_STRING(FILEFORMAT_CIT)                          // =="Cit"==
    L10N_STRING(FILEFORMAT_COT)                          // =="Cot"==
    L10N_STRING(FILEFORMAT_CRL)                          // =="CRL"==
    L10N_STRING(FILEFORMAT_cTiff)                        // =="Cache TIFF"==
    L10N_STRING(FILEFORMAT_DTED)                         // =="Digital Terrain Elevation Data"==
    L10N_STRING(FILEFORMAT_ECW)                          // =="ERMapper Compressed Wavelets"==
    L10N_STRING(FILEFORMAT_EncapsulatedPostScript)       // =="Encapsulated PostScript"==
    L10N_STRING(FILEFORMAT_ErdasImg)                     // =="Erdas IMG"==
    L10N_STRING(FILEFORMAT_FLI)                          // =="FLI Animation Format"==
    L10N_STRING(FILEFORMAT_GeoTiff)                      // =="GEOTIFF"==
    L10N_STRING(FILEFORMAT_GIF)                          // =="Gif"==
    L10N_STRING(FILEFORMAT_Hgt)                          // =="Hgt"==
    L10N_STRING(FILEFORMAT_HMR)                          // =="Bentley HMR"==
    L10N_STRING(FILEFORMAT_ImgMapped)                    // =="Img"==
    L10N_STRING(FILEFORMAT_ImgRGB)                       // =="Image RGB"==
    L10N_STRING(FILEFORMAT_IngrTiff)                     // =="Ingr. TIFF"==
    L10N_STRING(FILEFORMAT_IrasbRST)                     // =="Raster save set"==
    L10N_STRING(FILEFORMAT_iTiff)                        // =="Internet TIFF"==
    L10N_STRING(FILEFORMAT_iTiff64)                      // =="Internet TIFF64"==
    L10N_STRING(FILEFORMAT_Jpeg)                         // =="JPEG"==
    L10N_STRING(FILEFORMAT_Jpeg2000)                     // =="JPEG 2000"==
    L10N_STRING(FILEFORMAT_LRD)                          // =="Anatech LRD"==
    L10N_STRING(FILEFORMAT_MPF)                          // =="MPF"==
    L10N_STRING(FILEFORMAT_MrSid)                        // =="MrSID"==
    L10N_STRING(FILEFORMAT_MultiChannel)                 // =="Multi Channel"==
    L10N_STRING(FILEFORMAT_NITF)                         // =="National Imagery Transmission"==
    L10N_STRING(FILEFORMAT_PCX)                          // =="PCX"==
    L10N_STRING(FILEFORMAT_PDF)                          // =="Adobe PDF"==
    L10N_STRING(FILEFORMAT_PICT)                         // =="Apple PICT"==
    L10N_STRING(FILEFORMAT_PNG)                          // =="Portable Network Graphics"==
    L10N_STRING(FILEFORMAT_PRJ)                          // =="PRJ"==
    L10N_STRING(FILEFORMAT_PSS)                          // =="Picture Script Scene"==
    L10N_STRING(FILEFORMAT_RAW)                          // =="Raw Data"==
    L10N_STRING(FILEFORMAT_RGB)                          // =="RGB"==
    L10N_STRING(FILEFORMAT_RLC)                          // =="RLC"==
    L10N_STRING(FILEFORMAT_RLE)                          // =="RLE"==
    L10N_STRING(FILEFORMAT_SDOGeoRaster)                 // =="SDO GeoRaster"==
    L10N_STRING(FILEFORMAT_SpotCap)                      // =="SPOT CAP"==
    L10N_STRING(FILEFORMAT_SpotDigital)                  // =="SPOT Digital Image Map"==
    L10N_STRING(FILEFORMAT_SunRaster)                    // =="Sun Raster"==
    L10N_STRING(FILEFORMAT_Targa)                        // =="Targa"==
    L10N_STRING(FILEFORMAT_TG4)                          // =="TG4"==
    L10N_STRING(FILEFORMAT_Tiff)                         // =="Tagged Image"==
    L10N_STRING(FILEFORMAT_USGS_ASCII)                   // =="USGS DEM ASCII"==
    L10N_STRING(FILEFORMAT_USGS_DOQ)                     // =="USGS Digital Ortho Quad"==
    L10N_STRING(FILEFORMAT_USGS_FastL7A)                 // =="Landsat TM FAST-L7A"==
    L10N_STRING(FILEFORMAT_USGS_NDF)                     // =="USGS NDF"==
    L10N_STRING(FILEFORMAT_USGS_SDTS)                    // =="USGS SDTS DEM"==
    L10N_STRING(FILEFORMAT_WCS)                          // =="Web Coverage Service"==
    L10N_STRING(FILEFORMAT_WirelessBitmap)               // =="Wireless BitMap"==
    L10N_STRING(FILEFORMAT_WMS)                          // =="Web Map Server"==

    // Transfo models
    L10N_STRING(TRANSFO_Identity)                        // =="Identity"==
    L10N_STRING(TRANSFO_Translation)                     // =="Translation"==
    L10N_STRING(TRANSFO_Strech)                          // =="Stretch"==
    L10N_STRING(TRANSFO_Similitude)                      // =="Similitude"==
    L10N_STRING(TRANSFO_Helmert)                         // =="Helmert"==
    L10N_STRING(TRANSFO_Projective)                      // =="Projective"==
    L10N_STRING(TRANSFO_Affine)                          // =="Affine"==
    L10N_STRING(TRANSFO_ProjectionCoordModel)            // =="CoordModel"==
    L10N_STRING(TRANSFO_LinearModelAdapter)              // =="LinearModelAdapter"==
    L10N_STRING(TRANSFO_ProjectiveGrid)                  // =="ProjectiveGrid"==
    L10N_STRING(TRANSFO_ComplexModel)                    // =="ComplexTransfoModel"==

    // Resampling 
    L10N_STRING(RESAMPLING_NearestNeighbour)             // =="Nearest Neighbour"==
    L10N_STRING(RESAMPLING_Average)                      // =="Average"==
    L10N_STRING(RESAMPLING_VectorAwareness)              // =="Vector Awareness"==
    L10N_STRING(RESAMPLING_Unknown)                      // =="Unknown"==
    L10N_STRING(RESAMPLING_Oring4)                       // =="Oring 4"==
    L10N_STRING(RESAMPLING_None)                         // =="None"==

    // Resolution encoding
    L10N_STRING(ENCODING_Standard)                       // =="Standard"==
    L10N_STRING(ENCODING_MultiResolution)                // =="Multi-resolution"==
    L10N_STRING(ENCODING_Progressive)                    // =="Progressive"==

    // Block Type
    L10N_STRING(BLOCKTYPE_Tile)                          // =="Tile"==
    L10N_STRING(BLOCKTYPE_Strip)                         // =="Strip"==
    L10N_STRING(BLOCKTYPE_Line)                          // =="Line"==
    L10N_STRING(BLOCKTYPE_Image)                         // =="Image"==

    // Georefence provenance
    L10N_STRING(GEOREFERENCE_InImage)                    // =="None"==
    L10N_STRING(GEOREFERENCE_HGR)                        // =="In a HGR file"==
    L10N_STRING(GEOREFERENCE_WorldFile)                  // =="In a World file"==

    // Filters
    L10N_STRING(FILTER_Blur)                             // =="Blur"==
    L10N_STRING(FILTER_BlurDescription)                  // =="Blur Filter Description"==
    L10N_STRING(FILTER_Sharpen)                          // =="Sharpen"==
    L10N_STRING(FILTER_SharpenDescription)               // =="Sharpen Filter Description"==
    L10N_STRING(FILTER_Average)                          // =="Average"==
    L10N_STRING(FILTER_AverageDescription)               // =="Average Filter Description"==
    L10N_STRING(FILTER_Smooth)                           // =="Smooth"==
    L10N_STRING(FILTER_SmoothDescription)                // =="Smooth Filter Description"==
    L10N_STRING(FILTER_Detail)                           // =="Detail"==
    L10N_STRING(FILTER_DetailDescription)                // =="Detail Filter Description"==
    L10N_STRING(FILTER_EdgeEnhance)                      // =="Edge Enhance"==
    L10N_STRING(FILTER_EdgeEnhanceDescription)           // =="Edge Enhance Filter Description"==
    L10N_STRING(FILTER_FindEdges)                        // =="Find Edges"==
    L10N_STRING(FILTER_FindEdgesDescription)             // =="Find Edges Filter Description"==
    L10N_STRING(FILTER_CustomConvolution)                // =="Custom Convolution"==
    L10N_STRING(FILTER_CustomConvolutionDescription)     // =="Custom Convolution Filter Description"==
    L10N_STRING(FILTER_ColorTwist)                       // =="Color Twist"==
    L10N_STRING(FILTER_ColorTwistDescription)            // =="Color Twist Filter Description"==
    L10N_STRING(FILTER_AlphaReplacer)                    // =="Alpha Replacer"==
    L10N_STRING(FILTER_AlphaReplacerDescription)         // =="Alpha Replacer Filter Description"==
    L10N_STRING(FILTER_AlphaComposer)                    // =="Alpha Composer"==
    L10N_STRING(FILTER_AlphaComposerDescription)         // =="Alpha Composer Filter Description"==
    L10N_STRING(FILTER_ColorReplacer)                    // =="Color Replacer"==
    L10N_STRING(FILTER_ColorReplacerDescription)         // =="Color Replacer Filter Description"==
    L10N_STRING(FILTER_ColorBalance)                     // =="Color Balance"==
    L10N_STRING(FILTER_ColorBalanceDescription)          // =="Color Balance Filter Description"==
    L10N_STRING(FILTER_Contrast)                         // =="Contrast"==
    L10N_STRING(FILTER_ContrastDescription)              // =="Contrast Filter Description"==
    L10N_STRING(FILTER_HistogramScaling)                 // =="Histogram Scaling"==
    L10N_STRING(FILTER_HistogramScalingDescription)      // =="Histogram Scaling Filter Description"==
    L10N_STRING(FILTER_Gamma)                            // =="Gamma"==
    L10N_STRING(FILTER_GammaDescription)                 // =="Gamma Filter Description"==
    L10N_STRING(FILTER_Invert)                           // =="Invert"==
    L10N_STRING(FILTER_invertDescription)                // =="Invert Filter Description"==
    L10N_STRING(FILTER_Tint)                             // =="Tint"==
    L10N_STRING(FILTER_TintDescription)                  // =="Tint Filter Description"==
    L10N_STRING(FILTER_CustomMap8)                       // =="Custom Map8"==
    L10N_STRING(FILTER_CustomMap8Description)            // =="Custom Map8 Description"==
    L10N_STRING(FILTER_CustomMap16)                      // =="Custom Map16"==
    L10N_STRING(FILTER_CustomMap16Description)           // =="Custom Map16 Description"==
    L10N_STRING(FILTER_Complex)                          // =="Complex"==

    // SLO orientation
    L10N_STRING(ORIENTATION_UpperLeftVertial)            // =="Upper Left Vertical"==
    L10N_STRING(ORIENTATION_UpperRightVertial)           // =="Upper Right Vertical"==
    L10N_STRING(ORIENTATION_LowerLeftVertial)            // =="Lower Left Vertical"==
    L10N_STRING(ORIENTATION_LowerRightVertial)           // =="Lower Right Vertical"==
    L10N_STRING(ORIENTATION_UpperLeftHorizontal)         // =="Upper Left Horizontal"==
    L10N_STRING(ORIENTATION_UpperRightHorizontal)        // =="Upper Right Horizontal"==
    L10N_STRING(ORIENTATION_LowerLeftHorizontal)         // =="Lower Left Horizontal"==
    L10N_STRING(ORIENTATION_LowerRightHorizontal)        // =="Lower Right Horizontal"==

    // HTiff errors
    L10N_STRING(HTIFF_UnknownError)                      // =="unknown error"==
    L10N_STRING(HTIFF_CannotReadDirCount)                // =="cannot read directory count"==
    L10N_STRING(HTIFF_CannotWriteDirBuffer)              // =="cannot write directory buffer"==
    L10N_STRING(HTIFF_CannotWriteDirCount)               // =="cannot write directory count"==
    L10N_STRING(HTIFF_CannotWriteDirOffset)              // =="cannot write directory's next directory offset"==
    L10N_STRING(HTIFF_CannotWriteDirLink)                // =="cannot write directory link"==
    L10N_STRING(HTIFF_NegativeValueForRational)          // =="cannot write the negative value %g as unsigned RATIONAL (tag : %s)"==
    L10N_STRING(HTIFF_HMRDirAlreadyExist)                // =="HMR directory already exists for the page"==
    L10N_STRING(HTIFF_CannotWritePWBlob)                 // =="cannot write the ProjectWise blob (offset : %0I64x, length : %I64d)"==
    L10N_STRING(HTIFF_CannotOpenFile)                    // =="cannot open file"==
    L10N_STRING(HTIFF_CannotReadHeader)                  // =="cannot read the header"==
    L10N_STRING(HTIFF_BadMagicNumber)                    // =="the following TIFF file's magic number is incorrect : 0x%x"==
    L10N_STRING(HTIFF_BadFileVersion)                    // =="the following TIFF file's version number is incorrect : 0x%x"==
    L10N_STRING(HTIFF_CannotCreateFile)                  // =="cannot create file"==
    L10N_STRING(HTIFF_CannotWriteHeader)                 // =="cannot write header"==
    L10N_STRING(HTIFF_ErrorFoundInDirectory)             // =="an error was found in a TIFF directory"==
    L10N_STRING(HTIFF_ImageDepthDifferentThan1)          // =="an image depth other than '1' is not supported"==
    L10N_STRING(HTIFF_ImageWidthAndLengthRequired)       // =="the image's length and width are required"==
    L10N_STRING(HTIFF_MissingStripOrTileOffset)          // =="the strip or tile offset is missing"==
    L10N_STRING(HTIFF_MissingImagePlanarConfig)          // =="the image's planar configuration is missing (PLANARCONFIG_CONTIG should be used by default)"==
    L10N_STRING(HTIFF_MissingColorMap)                   // =="the color map is missing"==
    L10N_STRING(HTIFF_BadBlockNumber)                    // =="bad strip or tile number (%ld)"==
    L10N_STRING(HTIFF_CannotWriteSeparatePlane)          // =="cannot write separate planes because the file?s planar configuration is contiguous"==
    L10N_STRING(HTIFF_CannotWriteBlock)                  // =="cannot write a strip or tile (index : %ld, offset : %0I64x, length : %ld)"==
    L10N_STRING(HTIFF_CannotReadSeparatePlane)           // =="Cannot read separate planes because the file?s planar configuration is contiguous"==
    L10N_STRING(HTIFF_CannotReadBlock)                   // =="cannot read a strip or tile (index : %ld, offset : %0I64x, length : %ld)"==
    L10N_STRING(HTIFF_UnknownCompressType)               // =="the following compression type is unknown : %hu"==
    L10N_STRING(HTIFF_SeparatePlanarConfigNotSuported)   // =="separate image planar configuration is not supported."==
    L10N_STRING(HTIFF_UnsupportedGeotiffVersion)         // =="the GeoTIFF version is not supported"==
    L10N_STRING(HTIFF_InvalidGeotiffCount)               // =="a GeoTIFF count is invalid (geoKey : %d)"==
    L10N_STRING(HTIFF_InvalidGeotiffIndex)               // =="a GeoTIFF index or count is invalid (geoKey : %d)"==
    L10N_STRING(HTIFF_InvalidGeotiffTag)                 // =="a GeoTIFF tag is invalid (geoKey : %d, tag : %d)"==
    L10N_STRING(HTIFF_UnknownTagEnumeration)             // =="the following unknown tag enumeration is ignored : %ld"==
    L10N_STRING(HTIFF_WrongDataTypeForTag)               // =="a tag is ignored because it has a wrong data type (tag : %d, data type : %s)"==
    L10N_STRING(HTIFF_UnknownTag)                        // =="a tag is unknown (tag file : %u, data type : %s, length : %lu, offset : %lu)"==
    L10N_STRING(HTIFF_UnknownFirstTag)                   // =="the following unknown tag is ignored : %ld"==
    L10N_STRING(HTIFF_DirectoryEntryReadError)           // =="directory entry read error"==
    L10N_STRING(HTIFF_TagReadError)                      // =="an error occurred while reading a tag (tag name : %s, tag file : %ld, data length : %ld)"==
    L10N_STRING(HTIFF_DirectoryEntryWriteError)          // =="directory entry write error"==
    L10N_STRING(HTIFF_TagWriteError)                     // =="an error occurred while writing a tag (tag name : %s, tag file : %ld, data length : %ld)"==
    L10N_STRING(HTIFF_IncorrectCountForTagRead)          // =="incorrect count for tag read (tag name : %s, count : %I64u, expected count : %hu)"==
    L10N_STRING(HTIFF_IncorrectCountForTagWritten)       // =="incorrect count for tag written (tag name : %s, count : %I64u, expected count : %hu)"==
    L10N_STRING(HTIFF_SizeOutOfRange)                    // =="the image size is too big to be saved using the selected file format"==

    // Misc
    L10N_STRING(BingMapsCultureId)                       // =="en-US"== // The complete list of supported culture by the Bing Maps server is available at http://msdn.microsoft.com/en-us/library/cc981048.aspx
    L10N_STRING(PSS_SyntaxError)                         // =="Syntax error."==
    L10N_STRING(PSS_PrematureEndOfFile)                  // =="Can't get token / Premature end of file found."==
    L10N_STRING(PSS_FileAlreadyIncluded)                 // =="File or stream has already been included."==
    L10N_STRING(MODULE_NoDeviceName)                     // =="No Device Name"==
    L10N_STRING(MODULE_NoFile)                           // =="No file"==
    L10N_STRING(TWF_PixelSizeX)                          // =="Line 1 : pixel size in the x-direction"==
    L10N_STRING(TWF_PixelSizeY)                          // =="Line 4 : pixel size in the y-direction"==
    L10N_STRING(TWF_RotationAboutX)                      // =="Line 3 : rotation about x-axis"==
    L10N_STRING(TWF_RotationAboutY)                      // =="Line 2 : rotation about y-axis"==
    L10N_STRING(TWF_TranslationX)                        // =="Line 5 : x-coordinate of the center of the upper left pixel"==
    L10N_STRING(TWF_TranslationY)                        // =="Line 6 : y-coordinate of the center of the upper left pixel"==
BENTLEY_TRANSLATABLE_STRINGS_END

