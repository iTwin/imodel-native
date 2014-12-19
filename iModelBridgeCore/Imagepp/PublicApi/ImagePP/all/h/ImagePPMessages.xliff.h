/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ImagePP/all/h/ImagePPMessages.xliff.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/WString.h>
#include <BeSQLite/L10N.h>

BENTLEY_TRANSLATABLE_STRINGS2_START(ImagePPMessages,ImagePPMessage,ImagePP)
    {
    // Message IDs 
    IDS_IMAGEPP_NotFound=1000,                  // =="Not found!"==

    // Codecs
    IDS_CODEC_Identity,                         // =="None"==
    IDS_CODEC_CCITT3,                           // =="CCITT3"==
    IDS_CODEC_CCITT4,                           // =="CCITT4"==
    IDS_CODEC_Deflate,                          // =="Deflate"==
    IDS_CODEC_Packbits,                         // =="Packbits"==
    IDS_CODEC_LZW,                              // =="LZW"==
    IDS_CODEC_JPEG,                             // =="JPEG"==
    IDS_CODEC_FlashPix,                         // =="FlashPix"==
    IDS_CODEC_RLE1,                             // =="RLE 1"==
    IDS_CODEC_RLE8,                             // =="RLE8"==
    IDS_CODEC_RGBRLE8,                          // =="RGBRLE8"==
    IDS_CODEC_BMPRLE8,                          // =="BMPRLE8"==
    IDS_CODEC_BMPRLE4,                          // =="BMPRLE4"==
    IDS_CODEC_Gif,                              // =="Gif"==
    IDS_CODEC_FlashPixSingleColor,              // =="FlashPix (single color)"==
    IDS_CODEC_Jpeg2000,                         // =="JPEG 2000"==
    IDS_CODEC_CCITTRLE,                         // =="CCITTRLE"==
    IDS_CODEC_TgaRLE,                           // =="Tga RLE"==
    IDS_CODEC_PCX,                              // =="PCX"==
    IDS_CODEC_TGARLE,                           // =="TGARLE"==
    IDS_CODEC_ECW,                              // =="ECW"==
    IDS_CODEC_CRL8,                             // =="CRL8"==
    IDS_CODEC_CCITTFax4,                        // =="CCITTFax4"==
    IDS_CODEC_JpegAlpha,                        // =="JpegAlpha"==
    IDS_CODEC_FLIRLE8,                          // =="FLI_RLE8bits"==
    IDS_CODEC_LRDRLE1,                          // =="LRD RLE 1bit"==
    IDS_CODEC_JBIG,                             // =="JBIG"==

    // Pixeltypes
    IDS_PIXELTYPE_BlackAndWhite,                // =="Black & white"==
    IDS_PIXELTYPE_WhiteAndBlack,                // =="White & black"==
    IDS_PIXELTYPE_2ColorsPalette,               // =="2 colors"==
    IDS_PIXELTYPE_2ColorsPaletteRLE,            // =="2 colors RLE"==
    IDS_PIXELTYPE_2ColorsPaletteAlpha,          // =="2 colors alpha"==
    IDS_PIXELTYPE_2ColorsPaletteAlphaRLE,       // =="2 colors alpha RLE"==
    IDS_PIXELTYPE_4Colors,                      // =="4 colors"==
    IDS_PIXELTYPE_16Colors,                     // =="16 colors"==
    IDS_PIXELTYPE_16ColorsAlpha,                // =="16 colors alpha"==
    IDS_PIXELTYPE_256Colors,                    // =="256 colors"==
    IDS_PIXELTYPE_256ColorsAlpha,               // =="256 colors alpha"==
    IDS_PIXELTYPE_256ColorsARGB,                // =="256 colors ARGB"==
    IDS_PIXELTYPE_256ColorsDCMask,              // =="256 colors DC Mask"==
    IDS_PIXELTYPE_Gray8Palette,                 // =="Gray 8 Palette"==
    IDS_PIXELTYPE_Gray8,                        // =="Gray 8"==
    IDS_PIXELTYPE_Gray8White,                   // =="Gray 8 inv."==
    IDS_PIXELTYPE_Gray8Alpha,                   // =="Gray 8 alpha"==
    IDS_PIXELTYPE_Gray16,                       // =="Gray 16"==
    IDS_PIXELTYPE_Integer16,                    // =="Integer 16"==
    IDS_PIXELTYPE_TrueColors16,                 // =="True colors 16"==
    IDS_PIXELTYPE_TrueColors24,                 // =="True colors 24"==
    IDS_PIXELTYPE_TrueColors24BGR,              // =="True colors 24 BGR"==
    IDS_PIXELTYPE_TrueColors32,                 // =="True colors 32"==
    IDS_PIXELTYPE_TrueColors32ARGB,             // =="True colors 32 ARGB"==
    IDS_PIXELTYPE_TrueColors32BGRX,             // =="True colors 32 BGRX"==
    IDS_PIXELTYPE_TrueColors32RGBX,             // =="True colors 32 no Alpha"==
    IDS_PIXELTYPE_TrueColors32PreMult,          // =="True colors 32 PR"==
    IDS_PIXELTYPE_TrueColors48,                 // =="True colors 48"==
    IDS_PIXELTYPE_TrueColors64,                 // =="True colors 64"==
    IDS_PIXELTYPE_TrueColors64RGBX,             // =="True colors 64 no Alpha"==
    IDS_PIXELTYPE_TrueColors96,                 // =="True Colors 96"==
    IDS_PIXELTYPE_Float32,                      // =="Float 32"==
    IDS_PIXELTYPE_CMYK,                         // =="CMYK"==
    IDS_PIXELTYPE_YCC,                          // =="YCC"==
    IDS_PIXELTYPE_YCCAlpha,                     // =="YCC alpha"==

    // File formats    
    IDS_FILEFORMAT_ArcInfoASCII,                // =="ArcInfo ASCII Grid"==
    IDS_FILEFORMAT_ArcInfoBinary,               // =="ArcInfo Binary Grid"==
    IDS_FILEFORMAT_Bil,                         // =="ESRI BIL"==
    IDS_FILEFORMAT_BingMaps,                    // =="BingMaps Server"==
    IDS_FILEFORMAT_BMP,                         // =="Window BMP"==
    IDS_FILEFORMAT_BSB,                         // =="BSB Navigation Chart"==
    IDS_FILEFORMAT_C29,                         // =="C29"==
    IDS_FILEFORMAT_C30,                         // =="C30"==
    IDS_FILEFORMAT_C31,                         // =="C31"==
    IDS_FILEFORMAT_CALS,                        // =="Cals Type 1 CCITT4"==
    IDS_FILEFORMAT_CIT,                         // =="Cit"==
    IDS_FILEFORMAT_COT,                         // =="Cot"==
    IDS_FILEFORMAT_CRL,                         // =="CRL"==
    IDS_FILEFORMAT_cTiff,                       // =="Cache TIFF"==
    IDS_FILEFORMAT_DTED,                        // =="Digital Terrain Elevation Data"==
    IDS_FILEFORMAT_ECW,                         // =="ERMapper Compressed Wavelets"==
    IDS_FILEFORMAT_EncapsulatedPostScript,      // =="Encapsulated PostScript"==
    IDS_FILEFORMAT_ErdasImg,                    // =="Erdas IMG"==
    IDS_FILEFORMAT_FLI,                         // =="FLI Animation Format"==
    IDS_FILEFORMAT_GeoTiff,                     // =="GEOTIFF"==
    IDS_FILEFORMAT_GIF,                         // =="Gif"==
    IDS_FILEFORMAT_HMR,                         // =="Bentley HMR"==
    IDS_FILEFORMAT_ImgMapped,                   // =="Img"==
    IDS_FILEFORMAT_ImgRGB,                      // =="Image RGB"==
    IDS_FILEFORMAT_IngrTiff,                    // =="Ingr. TIFF"==
    IDS_FILEFORMAT_IrasbRST,                    // =="Raster save set"==
    IDS_FILEFORMAT_iTiff,                       // =="Internet TIFF"==
    IDS_FILEFORMAT_iTiff64,                     // =="Internet TIFF64"==
    IDS_FILEFORMAT_Jpeg,                        // =="JPEG"==
    IDS_FILEFORMAT_Jpeg2000,                    // =="JPEG 2000"==
    IDS_FILEFORMAT_LRD,                         // =="Anatech LRD"==
    IDS_FILEFORMAT_MPF,                         // =="MPF"==
    IDS_FILEFORMAT_MrSid,                       // =="MrSID"==
    IDS_FILEFORMAT_MultiChannel,                // =="Multi Channel"==
    IDS_FILEFORMAT_NITF,                        // =="National Imagery Transmission"==
    IDS_FILEFORMAT_PCX,                         // =="PCX"==
    IDS_FILEFORMAT_PDF,                         // =="Adobe PDF"==
    IDS_FILEFORMAT_PICT,                        // =="Apple PICT"==
    IDS_FILEFORMAT_PNG,                         // =="Portable Network Graphics"==
    IDS_FILEFORMAT_PRJ,                         // =="PRJ"==
    IDS_FILEFORMAT_PSS,                         // =="Picture Script Scene"==
    IDS_FILEFORMAT_RAW,                         // =="Raw Data"==
    IDS_FILEFORMAT_RGB,                         // =="RGB"==
    IDS_FILEFORMAT_RGBCompressed,               // =="RGB compressed"==
    IDS_FILEFORMAT_RLC,                         // =="RLC"==
    IDS_FILEFORMAT_RLE,                         // =="RLE"==
    IDS_FILEFORMAT_SDOGeoRaster,                // =="SDO GeoRaster"==
    IDS_FILEFORMAT_SpotCap,                     // =="SPOT CAP"==
    IDS_FILEFORMAT_SpotDigital,                 // =="SPOT Digital Image Map"==
    IDS_FILEFORMAT_SunRaster,                   // =="Sun Raster"==
    IDS_FILEFORMAT_Targa,                       // =="Targa"==
    IDS_FILEFORMAT_TG4,                         // =="TG4"==
    IDS_FILEFORMAT_Tiff,                        // =="Tagged Image"==
    IDS_FILEFORMAT_USGS_ASCII,                  // =="USGS DEM ASCII"==
    IDS_FILEFORMAT_USGS_DOQ,                    // =="USGS Digital Ortho Quad"==
    IDS_FILEFORMAT_USGS_FastL7A,                // =="Landsat TM FAST-L7A"==
    IDS_FILEFORMAT_USGS_NDF,                    // =="USGS NDF"==
    IDS_FILEFORMAT_USGS_SDTS,                   // =="USGS SDTS DEM"==
    IDS_FILEFORMAT_WCS,                         // =="Web Coverage Service"==
    IDS_FILEFORMAT_WirelessBitmap,              // =="Wireless BitMap"==
    IDS_FILEFORMAT_WMS,                         // =="Web Map Server"==

    // Transfo models
    IDS_TRANSFO_Identity,                       // =="Identity"==
    IDS_TRANSFO_Translation,                    // =="Translation"==
    IDS_TRANSFO_Strech,                         // =="Stretch"==
    IDS_TRANSFO_Similitude,                     // =="Similitude"==
    IDS_TRANSFO_Helmert,                        // =="Helmert"==
    IDS_TRANSFO_Projective,                     // =="Projective"==
    IDS_TRANSFO_Affine,                         // =="Affine"==
    IDS_TRANSFO_ProjectionCoordModel,           // =="CoordModel"==
    IDS_TRANSFO_LinearModelAdapter,             // =="LinearModelAdapter"==
    IDS_TRANSFO_ProjectiveGrid,                 // =="ProjectiveGrid"==
    IDS_TRANSFO_ComplexModel,                   // =="ComplexTransfoModel"==
    
    // Resampling 
    IDS_RESAMPLING_NearestNeighbour,            // =="Nearest Neighbour"==
    IDS_RESAMPLING_Average,                     // =="Average"==
    IDS_RESAMPLING_VectorAwareness,             // =="Vector Awareness"==
    IDS_RESAMPLING_Unknown,                     // =="Unknown"==
    IDS_RESAMPLING_Oring4,                      // =="Oring 4"==
    IDS_RESAMPLING_None,                        // =="None"==

    // Resolution encoding
    IDS_ENCODING_Standard,                      // =="Standard"==
    IDS_ENCODING_MultiResolution,               // =="Multi-resolution"==
    IDS_ENCODING_Progressive,                   // =="Progressive"==

    // Block Type
    IDS_BLOCKTYPE_Tile,                         // =="Tile"==
    IDS_BLOCKTYPE_Strip,                        // =="Strip"==
    IDS_BLOCKTYPE_Line,                         // =="Line"==
    IDS_BLOCKTYPE_Image,                        // =="Image"==

    // Georefence provenance
    IDS_GEOREFERENCE_InImage,                   // =="None"==
    IDS_GEOREFERENCE_HGR,                       // =="In a HGR file"==
    IDS_GEOREFERENCE_WorldFile,                 // =="In a World file"==

    // Filters
    IDS_FILTER_Blur,                            // =="Blur"==
    IDS_FILTER_BlurDescription,                 // =="Blur Filter Description"==
    IDS_FILTER_Sharpen,                         // =="Sharpen"==
    IDS_FILTER_SharpenDescription,              // =="Sharpen Filter Description"==
    IDS_FILTER_Average,                         // =="Average"==
    IDS_FILTER_AverageDescription,              // =="Average Filter Description"==
    IDS_FILTER_Smooth,                          // =="Smooth"==
    IDS_FILTER_SmoothDescription,               // =="Smooth Filter Description"==
    IDS_FILTER_Detail,                          // =="Detail"==
    IDS_FILTER_DetailDescription,               // =="Detail Filter Description"==
    IDS_FILTER_EdgeEnhance,                     // =="Edge Enhance"==
    IDS_FILTER_EdgeEnhanceDescription,          // =="Edge Enhance Filter Description"==
    IDS_FILTER_FindEdges,                       // =="Find Edges"==
    IDS_FILTER_FindEdgesDescription,            // =="Find Edges Filter Description"==
    IDS_FILTER_CustomConvolution,               // =="Custom Convolution"==
    IDS_FILTER_CustomConvolutionDescription,    // =="Custom Convolution Filter Description"==
    IDS_FILTER_ColorTwist,                      // =="Color Twist"==
    IDS_FILTER_ColorTwistDescription,           // =="Color Twist Filter Description"==
    IDS_FILTER_AlphaReplacer,                   // =="Alpha Replacer"==
    IDS_FILTER_AlphaReplacerDescription,        // =="Alpha Replacer Filter Description"==
    IDS_FILTER_AlphaComposer,                   // =="Alpha Composer"==
    IDS_FILTER_AlphaComposerDescription,        // =="Alpha Composer Filter Description"==
    IDS_FILTER_ColorReplacer,                   // =="Color Replacer"==
    IDS_FILTER_ColorReplacerDescription,        // =="Color Replacer Filter Description"==
    IDS_FILTER_ColorBalance,                    // =="Color Balance"==
    IDS_FILTER_ColorBalanceDescription,         // =="Color Balance Filter Description"==
    IDS_FILTER_Contrast,                        // =="Contrast"==
    IDS_FILTER_ContrastDescription,             // =="Contrast Filter Description"==
    IDS_FILTER_HistogramScaling,                // =="Histogram Scaling"==
    IDS_FILTER_HistogramScalingDescription,     // =="Histogram Scaling Filter Description"==
    IDS_FILTER_Gamma,                           // =="Gamma"==
    IDS_FILTER_GammaDescription,                // =="Gamma Filter Description"==
    IDS_FILTER_Invert,                          // =="Invert"==
    IDS_FILTER_invertDescription,               // =="Invert Filter Description"==
    IDS_FILTER_Tint,                            // =="Tint"==
    IDS_FILTER_TintDescription,                 // =="Tint Filter Description"==
    IDS_FILTER_CustomMap8,                      // =="Custom Map8"==
    IDS_FILTER_CustomMap8Description,           // =="Custom Map8 Description"==
    IDS_FILTER_CustomMap16,                     // =="Custom Map16"==
    IDS_FILTER_CustomMap16Description,          // =="Custom Map16 Description"==
    IDS_FILTER_Complex,                         // =="Complex"==

    // SLO orientation
    IDS_ORIENTATION_UpperLeftVertial,           // =="Upper Left Vertical"==
    IDS_ORIENTATION_UpperRightVertial,          // =="Upper Right Vertical"==
    IDS_ORIENTATION_LowerLeftVertial,           // =="Lower Left Vertical"==
    IDS_ORIENTATION_LowerRightVertial,          // =="Lower Right Vertical"==
    IDS_ORIENTATION_UpperLeftHorizontal,        // =="Upper Left Horizontal"==
    IDS_ORIENTATION_UpperRightHorizontal,       // =="Upper Right Horizontal"==
    IDS_ORIENTATION_LowerLeftHorizontal,        // =="Lower Left Horizontal"==
    IDS_ORIENTATION_LowerRightHorizontal,       // =="Lower Right Horizontal"==

    // HTIFF errors
    IDS_HTIFF_UnknownError,                     // =="unknown error"==
    IDS_HTIFF_CannotReadDirCount,               // =="cannot read directory count"==
    IDS_HTIFF_CannotWriteDirBuffer,             // =="cannot write directory buffer"==
    IDS_HTIFF_CannotWriteDirCount,              // =="cannot write directory count"==
    IDS_HTIFF_CannotWriteDirOffset,             // =="cannot write directory's next directory offset"==
    IDS_HTIFF_CannotWriteDirLink,               // =="cannot write directory link"==
    IDS_HTIFF_NegativeValueForRational,         // =="cannot write the negative value %g as unsigned RATIONAL (tag : %s)"==
    IDS_HTIFF_HMRDirAlreadyExist,               // =="HMR directory already exists for the page"==
    IDS_HTIFF_CannotWritePWBlob,                // =="cannot write the ProjectWise blob (offset : %0I64x, lenght : %I64d)"==
    IDS_HTIFF_CannotOpenFile,                   // =="cannot open file"==
    IDS_HTIFF_CannotReadHeader,                 // =="cannot read the header"==
    IDS_HTIFF_BadMagicNumber,                   // =="the following TIFF file's magic number is incorrect : 0x%x"==
    IDS_HTIFF_BadFileVersion,                   // =="the following TIFF file's version number is incorrect : 0x%x"==
    IDS_HTIFF_CannotCreateFile,                 // =="cannot create file"==
    IDS_HTIFF_CannotWriteHeader,                // =="cannot write header"==
    IDS_HTIFF_ErrorFoundInDirectory,            // =="an error was found in a TIFF directory"==
    IDS_HTIFF_ImageDepthDifferentThan1,         // =="an image depth other than '1' is not supported"==
    IDS_HTIFF_ImageWidthAndLengthRequired,      // =="the image's length and width are required"==
    IDS_HTIFF_MissingStripOrTileOffset,         // =="the strip or tile offset is missing"==
    IDS_HTIFF_MissingImagePlanarConfig,         // =="the image's planar configuration is missing (PLANARCONFIG_CONTIG should be used by default)"==
    IDS_HTIFF_MissingColorMap,                  // =="the color map is missing"==
    IDS_HTIFF_BadBlockNumber,                   // =="bad strip or tile number (%ld)"==
    IDS_HTIFF_CannotWriteSeparatePlane,         // =="cannot write separate planes because the file?s planar configuration is contiguous"==
    IDS_HTIFF_CannotWriteBlock,                 // =="cannot write a strip or tile (index : %ld, offset : %0I64x, length : %ld)"==
    IDS_HTIFF_CannotReadSeparatePlane,          // =="Cannot read separate planes because the file?s planar configuration is contiguous"==
    IDS_HTIFF_CannotReadBlock,                  // =="cannot read a strip or tile (index : %ld, offset : %0I64x, length : %ld)"==
    IDS_HTIFF_UnknownCompressType,              // =="the following compression type is unknown : %hu"==
    IDS_HTIFF_SeparatePlanarConfigNotSuported,  // =="separate image planar configuration is not supported."==
    IDS_HTIFF_UnsupportedGeotiffVersion,        // =="the GeoTIFF version is not supported"==
    IDS_HTIFF_InvalidGeotiffCount,              // =="a GeoTIFF count is invalid (geoKey : %d)"==
    IDS_HTIFF_InvalidGeotiffIndex,              // =="a GeoTIFF index or count is invalid (geoKey : %d)"==
    IDS_HTIFF_InvalidGeotiffTag,                // =="a GeoTIFF tag is invalid (geoKey : %d, tag : %d)"==
    IDS_HTIFF_UnknownTagEnumeration,            // =="the following unknown tag enumeration is ignored : %ld"==
    IDS_HTIFF_WrongDataTypeForTag,              // =="a tag is ignored because it has a wrong data type (tag : %d, data type : %s)"==
    IDS_HTIFF_UnknownTag,                       // =="a tag is unknown (tag file : %u, data type : %s, length : %lu, offset : %lu)"==
    IDS_HTIFF_UnknownFirstTag,                  // =="the following unknown tag is ignored : %ld"==
    IDS_HTIFF_DirectoryEntryReadError,          // =="directory entry read error"==
    IDS_HTIFF_TagReadError,                     // =="an error occurred while reading a tag (tag name : %s, tag file : %ld, data length : %ld)"==
    IDS_HTIFF_DirectoryEntryWriteError,         // =="directory entry write error"==
    IDS_HTIFF_TagWriteError,                    // =="an error occurred while writing a tag (tag name : %s, tag file : %ld, data length : %ld)"==
    IDS_HTIFF_IncorrectCountForTagRead,         // =="incorrect count for tag read (tag name : %s, count : %I64u, expected count : %hu)"==
    IDS_HTIFF_IncorrectCountForTagWritten,      // =="incorrect count for tag written (tag name : %s, count : %I64u, expected count : %hu)"==
    IDS_HTIFF_SizeOutOfRange,                   // =="the image size is too big to be saved using the selected file format"==

    // Misc
    IDS_BingMapsCultureId,                      // =="en-US"==
    IDS_PSS_SyntaxError,                        // =="Syntax error."==
    IDS_PSS_PrematureEndOfFile,                 // =="Can't get token / Premature end of file found."==
    IDS_PSS_FileAlreadyIncluded,                // =="File or stream has already been included."==
    IDS_MODULE_NoDeviceName,                    // =="No Device Name"==
    IDS_MODULE_NoFile,                          // =="No file"==

    IDS_TWF_PixelSizeX,                         // =="Line 1 : pixel size in the x-direction"==
    IDS_TWF_PixelSizeY,                         // =="Line 4 : pixel size in the y-direction"==
    IDS_TWF_RotationAboutX,                     // =="Line 3 : rotation about x-axis"==
    IDS_TWF_RotationAboutY,                     // =="Line 2 : rotation about y-axis"==
    IDS_TWF_TranslationX,                       // =="Line 5 : x-coordinate of the center of the upper left pixel"==
    IDS_TWF_TranslationY,                       // =="Line 6 : y-coordinate of the center of the upper left pixel"==
    };
BENTLEY_TRANSLATABLE_STRINGS2_END
