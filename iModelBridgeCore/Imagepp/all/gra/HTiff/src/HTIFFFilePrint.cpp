//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/HTiff/src/HTIFFFilePrint.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HTIFFFilePrint
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>

#include <Imagepp/all/h/HTIFFFile.h>
#include <ImagePP/all/h/HTIFFTag.h>
#include <ImagePP/all/h/HTIFFGeoKey.h>
#include <Imagepp/all/h/HTIFFUtils.h>
#include <Imagepp/all/h/HTIFFDirectory.h>

USING_NAMESPACE_IMAGEPP

static void s_PrintAscii    (FILE* po_pOutput, const char* pi_pCar, int32_t pi_NbCars = -1);
static void s_PrintAsciiTag (FILE* po_pOutput, const char* pi_pTagName, const char* pi_pString);

static void s_PrintDoubleArray   (FILE* po_pOutput, const double* pi_pArray, uint32_t pi_NbLongs);


//---------------------------------------------------------------------------
// Print value functor that accept custom separator
//---------------------------------------------------------------------------
class PrintValue
    {
public:
    PrintValue(FILE* const po_pOutputFile, const WChar* const pi_Separator)
        :   m_pOutputFile(po_pOutputFile), m_Separator(pi_Separator)
        {
        HPRECONDITION(po_pOutputFile != 0);
        HPRECONDITION(pi_Separator != 0);
        }

    void operator() (const double pi_rValue)
        {
        fprintf(m_pOutputFile, "%8.6lf%s", pi_rValue, AString(m_Separator).c_str());
        }
    void operator() (const float pi_rValue)
        {
        fprintf(m_pOutputFile, "%8.6f%s", pi_rValue, AString(m_Separator).c_str());
        }
    void operator() (const unsigned short pi_rValue)
        {
        fprintf(m_pOutputFile, "%u%s", pi_rValue, AString(m_Separator).c_str());
        }
    void operator() (const uint32_t pi_rValue)
        {
        fprintf(m_pOutputFile, "%u%s", pi_rValue, AString(m_Separator).c_str());
        }

    // Add more types here as needed...


private:
    FILE* const         m_pOutputFile;
    const WChar* const m_Separator;
    };


static const char* s_PhotoNames[] = {
    "min-is-white",                         // PHOTOMETRIC_MINISWHITE
    "min-is-black",                         // PHOTOMETRIC_MINISBLACK
    "RGB color",                            // PHOTOMETRIC_RGB
    "palette color (RGB from colormap)",    // PHOTOMETRIC_PALETTE
    "transparency mask",                    // PHOTOMETRIC_MASK
    "separated",                            // PHOTOMETRIC_SEPARATED
    "YCbCr",                                // PHOTOMETRIC_YCBCR
    "7 (0x7)",
    "CIE L*a*b*",                           // PHOTOMETRIC_CIELAB
    };
#define NPHOTONAMES (sizeof (s_PhotoNames) / sizeof (s_PhotoNames[0]))

static const char* s_OrientNames[] = {
    "0 (0x0)",
    "row 0 top, col 0 lhs",                 // ORIENTATION_TOPLEFT
    "row 0 top, col 0 rhs",                 // ORIENTATION_TOPRIGHT
    "row 0 bottom, col 0 rhs",              // ORIENTATION_BOTRIGHT
    "row 0 bottom, col 0 lhs",              // ORIENTATION_BOTLEFT
    "row 0 lhs, col 0 top",                 // ORIENTATION_LEFTTOP
    "row 0 rhs, col 0 top",                 // ORIENTATION_RIGHTTOP
    "row 0 rhs, col 0 bottom",              // ORIENTATION_RIGHTBOT
    "row 0 lhs, col 0 bottom",              // ORIENTATION_LEFTBOT
    };
#define NORIENTNAMES    (sizeof (s_OrientNames) / sizeof (s_OrientNames[0]))


void HTIFFFile::_PrintCurrentDirectory (FILE* po_pOutput, uint32_t pi_Flag)
    {
    uint32_t ValL;   // Use to read Long and Short Tag
    uint32_t ValL2;
    double ValD;
    double ValD2;
    char*  pValC;
    uint64_t Val64;
    HFCMonitor Monitor(m_Key);

    fprintf(po_pOutput, "TIFF Directory at offset 0x%I64x\n", DirectoryOffset(m_CurDir));

    if (m_pCurDir->TagIsPresent(SUBFILETYPE))
        {
        fprintf(po_pOutput, "  %s: ", GetTagNameString(SUBFILETYPE));

        GetField (SUBFILETYPE, &ValL);

        if (ValL == FILETYPE_REDUCEDIMAGE)
            fprintf(po_pOutput, "reduced-resolution image");
        if (ValL ==  FILETYPE_PAGE)
            fprintf(po_pOutput, "multi-page document");
        if (ValL == FILETYPE_MASK)
            fprintf(po_pOutput, "transparency mask");

        fprintf(po_pOutput, " (%lu = 0x%lx)\n", ValL, ValL);
        }

    if (m_pCurDir->TagIsPresent(IMAGEWIDTH))
        {
        fprintf(po_pOutput, "  %s: %lu %s: %lu",GetTagNameString(IMAGEWIDTH), m_ImageWidth,
                GetTagNameString(IMAGELENGTH), m_ImageLength);

        if (m_pCurDir->TagIsPresent(IMAGEDEPTH))
            {
            GetField (IMAGEDEPTH, &ValL);
            fprintf(po_pOutput, " %s: %lu", GetTagNameString(IMAGEDEPTH), ValL);
            }

        fprintf(po_pOutput, "\n");
        }

    if (m_pCurDir->TagIsPresent(TILEWIDTH))
        {
        GetField (TILEWIDTH, &ValL);
        GetField (TILELENGTH, &ValL2);

        fprintf(po_pOutput, "  %s: %lu %s: %lu",GetTagNameString(TILEWIDTH), ValL,
                GetTagNameString(TILELENGTH), ValL2);

        if (m_pCurDir->TagIsPresent(TILEDEPTH))
            {
            GetField (TILEDEPTH, &ValL);
            fprintf(po_pOutput, " %s: %lu", GetTagNameString(TILEDEPTH), ValL);
            }
        fprintf(po_pOutput, "\n");
        }

    if (m_pCurDir->TagIsPresent(XRESOLUTION) || m_pCurDir->TagIsPresent(YRESOLUTION))
        {
        RATIONAL V1, V2;

        GetField (XRESOLUTION, &V1);
        GetField (YRESOLUTION, &V2);

        fprintf(po_pOutput, "  Resolution: %g, %g", V1.Value, V2.Value);

        if (m_pCurDir->TagIsPresent(RESOLUTIONUNIT))
            {
            GetField (RESOLUTIONUNIT, &ValL);

            switch (ValL)
                {
                case RESUNIT_NONE:
                    fprintf(po_pOutput, " (unitless)");
                    break;
                case RESUNIT_INCH:
                    fprintf(po_pOutput, " pixels/inch");
                    break;
                case RESUNIT_CENTIMETER:
                    fprintf(po_pOutput, " pixels/cm");
                    break;
                default:
                    fprintf(po_pOutput, " (unit %u = 0x%x)", ValL, ValL);
                    break;
                }
            }
        fprintf(po_pOutput, "\n");
        }

    if (m_pCurDir->TagIsPresent(XPOSITION) || m_pCurDir->TagIsPresent(YPOSITION))
        {
        GetField (XPOSITION, &ValD);
        GetField (YPOSITION, &ValD2);

        fprintf(po_pOutput, "  Position: %g, %g\n", ValD, ValD2);
        }

    if (m_pCurDir->TagIsPresent(BITSPERSAMPLE))
        {
        fprintf(po_pOutput, "  %s: ", GetTagNameString(BITSPERSAMPLE));
        for (size_t i=0; i<m_NbSampleFromFile; ++i)
            fprintf(po_pOutput, "%u, ", m_pBitsBySample[i]);

        fprintf(po_pOutput, "\n");
        }


    if (m_pCurDir->TagIsPresent(SAMPLEFORMAT))
        {
        fprintf(po_pOutput, "  %s: ", GetTagNameString(SAMPLEFORMAT));

        GetField (SAMPLEFORMAT, &ValL);
        switch (ValL)
            {
            case SAMPLEFORMAT_VOID:
                fprintf(po_pOutput, "void\n");
                break;
            case SAMPLEFORMAT_INT:
                fprintf(po_pOutput, "signed integer\n");
                break;
            case SAMPLEFORMAT_UINT:
                fprintf(po_pOutput, "unsigned integer\n");
                break;
            case SAMPLEFORMAT_IEEEFP:
                fprintf(po_pOutput, "IEEE floating point\n");
                break;
            default:
                fprintf(po_pOutput, "%u (0x%x)\n", ValL, ValL);
                break;
            }
        }

    if (m_pCurDir->TagIsPresent(COMPRESSION))
        {
        fprintf(po_pOutput, "  %s: ", GetTagNameString(COMPRESSION));
        GetField (COMPRESSION, &ValL);

        switch (ValL)
            {
            case COMPRESSION_NONE:
                fprintf(po_pOutput, "None\n");
                break;

            case COMPRESSION_CCITTRLE:
                fprintf(po_pOutput, "CCITT-RLE\n");
                break;

            case COMPRESSION_CCITTFAX3:
                fprintf(po_pOutput, "CCITT-Fax3\n");
                break;

            case COMPRESSION_CCITTFAX4:
                fprintf(po_pOutput, "CCITT-Fax4\n");
                break;

            case COMPRESSION_LZW:
                fprintf(po_pOutput, "LZW\n");
                break;

            case COMPRESSION_OJPEG:
                fprintf(po_pOutput, "OJPEG (6.0 JPEG)\n");
                break;

            case COMPRESSION_JPEG:
                fprintf(po_pOutput, "JPEG (DCT)\n");
                break;

            case COMPRESSION_JBIG:
                fprintf(po_pOutput, "JBIG\n");
                break;

            case COMPRESSION_PACKBITS:
                fprintf(po_pOutput, "Packbits\n");
                break;

            case COMPRESSION_DEFLATE:
                fprintf(po_pOutput, "Deflate\n");
                break;

            case COMPRESSION_HMR_FLASHPIX:
                fprintf(po_pOutput, "FLASHPIX-JPEG (HMR)\n");
                break;

            case COMPRESSION_HMR_RLE1:
                fprintf(po_pOutput, "RLE1 (HMR)\n");
                break;

                // Supported Tags only for compatibility....
                // Sets 01 Mars 1999, can be remove 01 Mars 2000....
                //   Only use in the cache files system a the beginning.
            case COMPRESSION_HMR_RLE1_OLD:
                fprintf(po_pOutput, "RLE1 **Old HMR-Tag**\n");
                break;
            case COMPRESSION_HMR_FLASHPIX_OLD:
                fprintf(po_pOutput, "FLASHPIX-JPEG **Old HMR-Tag**)\n");
                break;

            default:
                fprintf(po_pOutput, "%u (0x%x)\n", ValL, ValL);
                break;
            }
        }

    if (m_pCurDir->TagIsPresent(PHOTOMETRIC))
        {
        fprintf(po_pOutput, "  %s: ", GetTagNameString(PHOTOMETRIC));

        if (m_Photometric < NPHOTONAMES)
            fprintf(po_pOutput, "%s\n", s_PhotoNames[m_Photometric]);
        else
            fprintf(po_pOutput, "%u (0x%x)\n", m_Photometric, m_Photometric);
        }

    if (m_pCurDir->TagIsPresent(EXTRASAMPLES))
        {
        unsigned short**  ppExtraSample = new unsigned short*;
        uint32_t*    NbSample      = new uint32_t[1];
        NbSample[0] = 1;

        *ppExtraSample = new unsigned short[1];
        GetField(EXTRASAMPLES, NbSample, ppExtraSample);

        fprintf(po_pOutput, "  Extra Samples: %u<", **ppExtraSample);

//     for (i = 0; i < td->td_extrasamples; i++)
//     {

        // For now we take only handel 1 extra sample.
        switch (**ppExtraSample)
            {
                // Extra sample contain alpha value.
            case EXTRASAMPLE_UNASSALPHA:
                fprintf(po_pOutput, " unassoc-alpha\n");
                break;
                // Extra sample contain pre-multiplied alpha value.
            case EXTRASAMPLE_ASSOCALPHA:
                fprintf(po_pOutput, " assoc-alpha(premultiplied)\n");
                break;
                // Extra sample does not contain alpha value .
            case EXTRASAMPLE_UNSPECIFIED:
                fprintf(po_pOutput, " unspecified\n");
                break;
            }
//      }
        }

    if (m_pCurDir->TagIsPresent(INKSET))
        {
        fprintf(po_pOutput, "  %s: ", GetTagNameString(INKSET));

        GetField (INKSET, &ValL);
        switch (ValL)
            {
            case INKSET_CMYK:
                fprintf(po_pOutput, "CMYK\n");
                break;
            default:
                fprintf(po_pOutput, "%u (0x%x)\n", ValL, ValL);
                break;
            }
        }

    if (m_pCurDir->TagIsPresent(INKNAMES))
        {
        uint32_t i = m_SamplesByPixel;
        char*  pCar;

        fprintf(po_pOutput, "  %s: ", GetTagNameString(INKNAMES));
        GetField (INKNAMES, &pValC);

        for (pCar = pValC; i > 0; pCar = strchr(pCar, '\0'), i--)
            {
            s_PrintAscii (po_pOutput, pCar);
            fprintf(po_pOutput, ", ");
            }
        }

    if (m_pCurDir->TagIsPresent(DOTRANGE))
        {
        unsigned short S1, S2;
        GetField (DOTRANGE, &S1, &S2);

        fprintf(po_pOutput, "  %s: %u-%u\n", GetTagNameString(DOTRANGE), S1, S2);
        }

    if (m_pCurDir->TagIsPresent(TARGETPRINTER))
        {
        GetField (TARGETPRINTER, &pValC);

        s_PrintAsciiTag(po_pOutput, GetTagNameString(TARGETPRINTER), pValC);
        }

    if (m_pCurDir->TagIsPresent(THRESHHOLDING))
        {
        fprintf(po_pOutput, "  %s: ", GetTagNameString(THRESHHOLDING));

        GetField (THRESHHOLDING, &ValL);
        switch (ValL)
            {
            case THRESHHOLD_BILEVEL:
                fprintf(po_pOutput, "bilevel art scan\n");
                break;
            case THRESHHOLD_HALFTONE:
                fprintf(po_pOutput, "halftone or dithered scan\n");
                break;
            case THRESHHOLD_ERRORDIFFUSE:
                fprintf(po_pOutput, "error diffused\n");
                break;
            default:
                fprintf(po_pOutput, "%u (0x%x)\n", ValL, ValL);
                break;
            }
        }

    if (m_pCurDir->TagIsPresent(FILLORDER))
        {
        fprintf(po_pOutput, "  %s: ", GetTagNameString(FILLORDER));

        GetField (FILLORDER, &ValL);
        switch (ValL)
            {
            case FILLORDER_MSB2LSB:
                fprintf(po_pOutput, "msb-to-lsb\n");
                break;
            case FILLORDER_LSB2MSB:
                fprintf(po_pOutput, "lsb-to-msb\n");
                break;
            default:
                fprintf(po_pOutput, "%u (0x%x)\n", ValL, ValL);
                break;
            }
        }

    if (m_pCurDir->TagIsPresent(YCBCRSUBSAMPLING))
        {
        unsigned short S1, S2;

        GetField (YCBCRSUBSAMPLING, &S1, &S2);
        fprintf(po_pOutput, "  %s: %u, %u\n", GetTagNameString(YCBCRSUBSAMPLING), S1, S2);
        }

    if (m_pCurDir->TagIsPresent(YCBCRPOSITIONING))
        {
        fprintf(po_pOutput, "  %s: ", GetTagNameString(YCBCRPOSITIONING));

        GetField (YCBCRPOSITIONING, &ValL);
        switch (ValL)
            {
            case YCBCRPOSITION_CENTERED:
                fprintf(po_pOutput, "centered\n");
                break;
            case YCBCRPOSITION_COSITED:
                fprintf(po_pOutput, "cosited\n");
                break;
            default:
                fprintf(po_pOutput, "%u (0x%x)\n", ValL, ValL);
                break;
            }
        }

    if (m_pCurDir->TagIsPresent(YCBCRCOEFFICIENTS))
        {
        fprintf(po_pOutput, "  %s: Not Complete\n", GetTagNameString(YCBCRCOEFFICIENTS));
        }

    if (m_pCurDir->TagIsPresent(HALFTONEHINTS))
        {
        unsigned short S1, S2;

        GetField (HALFTONEHINTS, &S1, &S2);
        fprintf(po_pOutput, "  %s: light %u dark %u\n", GetTagNameString(HALFTONEHINTS), S1, S2);
        }

    if (m_pCurDir->TagIsPresent(ARTIST))
        {
        GetField (ARTIST, &pValC);
        s_PrintAsciiTag(po_pOutput, GetTagNameString(ARTIST), pValC);
        }

    if (m_pCurDir->TagIsPresent(DATETIME))
        {
        GetField (DATETIME, &pValC);
        s_PrintAsciiTag(po_pOutput, GetTagNameString(DATETIME), pValC);
        }

    if (m_pCurDir->TagIsPresent(HOSTCOMPUTER))
        {
        GetField (HOSTCOMPUTER, &pValC);
        s_PrintAsciiTag(po_pOutput, GetTagNameString(HOSTCOMPUTER), pValC);
        }

    if (m_pCurDir->TagIsPresent(SOFTWARE))
        {
        GetField (SOFTWARE, &pValC);
        s_PrintAsciiTag(po_pOutput, GetTagNameString(SOFTWARE), pValC);
        }

    if (m_pCurDir->TagIsPresent(DOCUMENTNAME))
        {
        GetField (DOCUMENTNAME, &pValC);
        s_PrintAsciiTag(po_pOutput, GetTagNameString(DOCUMENTNAME), pValC);
        }

    if (m_pCurDir->TagIsPresent(IMAGEDESCRIPTION))
        {
        GetField (IMAGEDESCRIPTION, &pValC);
        s_PrintAsciiTag(po_pOutput, GetTagNameString(IMAGEDESCRIPTION), pValC);
        }

    if (m_pCurDir->TagIsPresent(MAKE))
        {
        GetField (MAKE, &pValC);
        s_PrintAsciiTag(po_pOutput, GetTagNameString(MAKE), pValC);
        }

    if (m_pCurDir->TagIsPresent(MODEL))
        {
        GetField (MODEL, &pValC);
        s_PrintAsciiTag(po_pOutput, GetTagNameString(MODEL), pValC);
        }

    if (m_pCurDir->TagIsPresent(ORIENTATION))
        {
        fprintf(po_pOutput, "  %s: ", GetTagNameString(ORIENTATION));

        GetField (ORIENTATION, &ValL);
        if (ValL < NORIENTNAMES)
            fprintf(po_pOutput, "%s\n", s_OrientNames[ValL]);
        else
            fprintf(po_pOutput, "%u (0x%x)\n", ValL, ValL);
        }

    if (m_pCurDir->TagIsPresent(SAMPLESPERPIXEL))
        fprintf(po_pOutput, "  %s: %u\n", GetTagNameString(SAMPLESPERPIXEL), m_SamplesByPixel);

    if (m_pCurDir->TagIsPresent(ROWSPERSTRIP))
        {
        fprintf(po_pOutput, "  %s: ",GetTagNameString(ROWSPERSTRIP));

        m_pCurDir->GetValues (ROWSPERSTRIP, &ValL);
        if (ValL == (uint32_t) -1)
            fprintf(po_pOutput, "(infinite)\n");
        else
            fprintf(po_pOutput, "%lu\n", ValL);

        if (ValL != m_RowsByStrip)
            {
            // Emulation by HTIFF
            fprintf(po_pOutput, "  Emulation by HTIFF library\n");
            fprintf(po_pOutput, "    %s: ",GetTagNameString(ROWSPERSTRIP));
            fprintf(po_pOutput, "%lu\n", m_RowsByStrip);
            }
        }

    if (m_pCurDir->TagIsPresent(MINSAMPLEVALUE))
        {
        unsigned short* pMinSampleValues;
        uint32_t Count;

        GetField (MINSAMPLEVALUE, &Count, &pMinSampleValues);
        fprintf(po_pOutput, "  %s: ", GetTagNameString(MINSAMPLEVALUE));
        for_each(pMinSampleValues, pMinSampleValues + Count, PrintValue(po_pOutput, L"\t"));
        fprintf(po_pOutput, "\n");
        }

    if (m_pCurDir->TagIsPresent(MAXSAMPLEVALUE))
        {
        unsigned short* pMaxSampleValues;
        uint32_t Count;

        GetField (MAXSAMPLEVALUE, &Count, &pMaxSampleValues);
        fprintf(po_pOutput, "  %s: ", GetTagNameString(MAXSAMPLEVALUE));
        for_each(pMaxSampleValues, pMaxSampleValues + Count, PrintValue(po_pOutput, L"\t"));
        fprintf(po_pOutput, "\n");
        }

    if (m_pCurDir->TagIsPresent(SMINSAMPLEVALUE))
        {
        vector<double> Values;
        GetConvertedField(SMINSAMPLEVALUE, Values);

        fprintf(po_pOutput, "  %s: ", GetTagNameString(SMINSAMPLEVALUE));
        for_each(Values.begin(), Values.end(), PrintValue(po_pOutput, L"\t"));
        fprintf(po_pOutput, "\n");

        }

    if (m_pCurDir->TagIsPresent(SMAXSAMPLEVALUE))
        {
        vector<double> Values;
        GetConvertedField(SMAXSAMPLEVALUE, Values);

        fprintf(po_pOutput, "  %s: ", GetTagNameString(SMAXSAMPLEVALUE));
        for_each(Values.begin(), Values.end(), PrintValue(po_pOutput, L"\t"));
        fprintf(po_pOutput, "\n");

        }

    if (m_pCurDir->TagIsPresent(PLANARCONFIG))
        {
        fprintf(po_pOutput, "  %s: ", GetTagNameString(PLANARCONFIG));
        switch (m_PlanarConfig)
            {
            case PLANARCONFIG_CONTIG:
                fprintf(po_pOutput, "single image plane\n");
                break;
            case PLANARCONFIG_SEPARATE:
                fprintf(po_pOutput, "separate image planes\n");
                break;
            default:
                fprintf(po_pOutput, "%u (0x%x)\n", m_PlanarConfig, m_PlanarConfig);
                break;
            }
        }


    if (m_pCurDir->TagIsPresent(PAGENAME))
        {
        GetField (PAGENAME, &pValC);
        s_PrintAsciiTag(po_pOutput, GetTagNameString(PAGENAME), pValC);
        }

    if (m_pCurDir->TagIsPresent(PAGENUMBER))
        {
        unsigned short S1, S2;

        GetField (PAGENUMBER, &S1, &S2);
        fprintf(po_pOutput, "  %s: %u-%u\n", GetTagNameString(PAGENUMBER), S1, S2);
        }

    if (m_pCurDir->TagIsPresent(TCOLORMAP))
        {
        unsigned short* pR, *pG, *pB;

        fprintf(po_pOutput, "  %s: ", GetTagNameString(TCOLORMAP));

        if (pi_Flag & TIFFPRINT_COLORMAP)
            {
            uint32_t Num = 1L<<m_pBitsBySample[0];
            fprintf(po_pOutput, "\n");

            GetField (TCOLORMAP, &pR, &pG, &pB);

            for (uint32_t i=0; i<Num; i++)
                fprintf(po_pOutput, "   %5lu: %5u %5u %5u\n",i, pR[i], pG[i], pB[i]);
            }
        else
            fprintf(po_pOutput, "(present)\n");
        }

    if (m_pCurDir->TagIsPresent(JPEGTABLES))
        {
        fprintf(po_pOutput, "  %s: ", GetTagNameString(JPEGTABLES));
        fprintf(po_pOutput, "(present)\n");
        }


    if (m_pCurDir->TagIsPresent(WHITEPOINT))
        {
        fprintf(po_pOutput, "  %s: not complete\n", GetTagNameString(WHITEPOINT));
        }

    if (m_pCurDir->TagIsPresent(PRIMARYCHROMATICITIES))
        {
        fprintf(po_pOutput, "  %s: not complete\n", GetTagNameString(PRIMARYCHROMATICITIES));
        }

    if (m_pCurDir->TagIsPresent(REFERENCEBLACKWHITE))
        {
        RATIONAL ValR[3];
        uint32_t Count;
        if (GetField (REFERENCEBLACKWHITE, &Count, ValR) && Count == 3)
            {
            fprintf(po_pOutput, "  %s: \n", GetTagNameString(REFERENCEBLACKWHITE));

            fprintf(po_pOutput, "    0: %g\n", ValR[0].Value);
            fprintf(po_pOutput, "    1: %g\n", ValR[1].Value);
            fprintf(po_pOutput, "    2: %g\n", ValR[2].Value);
            }
        else
            {
            uint32_t* pVal;
            uint32_t Count;
            if (GetField (REFERENCEBLACKWHITE, &Count, &pVal))
                {
                fprintf(po_pOutput, "  %s: \n", GetTagNameString(REFERENCEBLACKWHITE));

                fprintf(po_pOutput, "    0: %lu, %lu\n", pVal[0], pVal[1]);
                fprintf(po_pOutput, "    1: %lu, %lu\n", pVal[2], pVal[3]);
                fprintf(po_pOutput, "    2: %lu, %lu\n", pVal[4], pVal[5]);
                }
            }
        }

    if (m_pCurDir->TagIsPresent(TRANSFERFUNCTION))
        {
        fprintf(po_pOutput, "  %s: ", GetTagNameString(TRANSFERFUNCTION));
#if 0
        if (pi_Flags & TIFFPRINT_CURVES)
            {
            unsigned short* pVal;

            GetField (TRANSFERFUNCTION, &pVal);

            fprintf(po_pOutput, "\n");
            Num = 1L<<m_BitsPerSample;
            for (uint32_t i=0; i<Num; i++)
                {
                fprintf(po_pOutput, "    %2lu: %5u",
                        l, td->td_transferfunction[0][l]);
                for (i = 1; i < td->td_samplesperpixel; i++)
                    fprintf(po_pOutput, " %5u",
                            td->td_transferfunction[i][l]);
                fputc('\n', po_pOutput);
                }
            }
        else
#endif
            fprintf(po_pOutput, "(present)\n");
        }


    if (m_pCurDir->TagIsPresent(SUBIFD))
        {
        uint32_t* pVal;

        fprintf(po_pOutput, "  %s:", GetTagNameString(SUBIFD));

        GetField (SUBIFD, &ValL, &pVal);
        for (uint32_t i=0; i<ValL; i++)
            fprintf(po_pOutput, " %5lu", pVal[i]);
        fputc('\n', po_pOutput);
        }

    /*
    ** GeoTiff
    */
    if (m_pCurDir->TagIsPresent(GEOTIEPOINTS))
        {
        double* pVal;

        GetField (GEOTIEPOINTS, &ValL, &pVal);
        fprintf(po_pOutput, "  %s:", GetTagNameString(GEOTIEPOINTS));
        if (ValL>6) fprintf(po_pOutput,"\n    ");
        for (uint32_t i=0; i<ValL; i+=6)
            {
            fprintf(po_pOutput," (");
            for (int j=0; j<3; j++)
                fprintf(po_pOutput, " %lf", pVal[i+j]);
            fprintf(po_pOutput,")->(");
            for (int j=3; j<6; j++)
                fprintf(po_pOutput, " %lf", pVal[i+j]);
            fprintf(po_pOutput,")\n");
            }
        }

    if (m_pCurDir->TagIsPresent(GEOPIXELSCALE))
        {
        double* pVal;

        GetField (GEOPIXELSCALE, &ValL, &pVal);
        fprintf(po_pOutput, "  %s: (", GetTagNameString(GEOPIXELSCALE));
        for (uint32_t j=0; j<ValL; j++)
            fprintf(po_pOutput, " %lf", pVal[j]);
        fprintf(po_pOutput, " )\n");
        }

    if (m_pCurDir->TagIsPresent(INTERGRAPH_RAGBAG))
        {
        unsigned short* pVal;

        GetField (INTERGRAPH_RAGBAG, &ValL, &pVal);
        fprintf(po_pOutput, "  %s:\n", GetTagNameString(INTERGRAPH_RAGBAG));
        size_t k=4;
        for (size_t i=0; i<4 && (k+3<ValL); ++i)
            {
            for (uint32_t j=0; j<4; j++)
                {
                double Tmp;
                ((unsigned short*)&Tmp)[0] = pVal[k++];
                ((unsigned short*)&Tmp)[1] = pVal[k++];
                ((unsigned short*)&Tmp)[2] = pVal[k++];
                ((unsigned short*)&Tmp)[3] = pVal[k++];

                fprintf(po_pOutput, " %8.6lf", Tmp);
                }
            fprintf(po_pOutput, "\n");
            }
        }

    if (m_pCurDir->TagIsPresent(INTERGRAPH_MATRIX))
        {
        double* pVal;

        GetField (INTERGRAPH_MATRIX, &ValL, &pVal);
        fprintf(po_pOutput, "  %s:\n", GetTagNameString(INTERGRAPH_MATRIX));
        int i;
        for (i=0; ValL>3; ValL-=4)
            {
            for (uint32_t j=0; j<4; j++)
                fprintf(po_pOutput, " %8.6lf", pVal[i++]);
            fprintf(po_pOutput, "\n");
            }
        if (ValL)
            {
            for (uint32_t j=0; j<ValL; j++)
                fprintf(po_pOutput, " %8.6lf", pVal[i++]);
            fprintf(po_pOutput, "\n");
            }
        }

    if (m_pCurDir->TagIsPresent(GEOTRANSMATRIX))
        {
        double* pVal;

        GetField (GEOTRANSMATRIX, &ValL, &pVal);
        fprintf(po_pOutput, "  %s:\n", GetTagNameString(GEOTRANSMATRIX));
        for (uint32_t i=0; i<ValL; i+=4)
            {
            for (int j=0; j<4; j++)
                fprintf(po_pOutput, " %8.6lf", pVal[i+j]);
            fprintf(po_pOutput, "\n");
            }
        }

    if (m_pCurDir->TagIsPresent(HMR2_3D_TRANSFO_MATRIX))
        {
        double* pVal;

        GetField (HMR2_3D_TRANSFO_MATRIX, &ValL, &pVal);
        fprintf(po_pOutput, "  %s:\n", GetTagNameString(HMR2_3D_TRANSFO_MATRIX));
        for (uint32_t i=0; i<ValL; i+=4)
            {
            for (int j=0; j<4; j++)
                fprintf(po_pOutput, " %8.6lf", pVal[i+j]);
            fprintf(po_pOutput, "\n");
            }
        }

    if (m_pCurDir->TagIsPresent(GEOKEYDIRECTORY))
        {
        unsigned short* pVal;

        GetField (GEOKEYDIRECTORY, &ValL, &pVal);
        fprintf(po_pOutput, "  %s:", GetTagNameString(GEOKEYDIRECTORY));
        if (pi_Flag & TIFFPRINT_GEOKEYDIRECTORY)
            {
            char   aString[256];
            double aDouble[10];
            unsigned short ValShort;

            HTIFFGeoKey& GeoKey = m_pCurDir->GetGeoKeyInterpretation();

            fprintf(po_pOutput, "\n");
            for (uint32_t i=0; i<ValL; i+=4)
                {
                for (int j=0; j<4; j++)
                    fprintf(po_pOutput, "  %8hu", pVal[i+j]);

                // Decode Info
                switch(GeoKey.GetDataType((TIFFGeoKey)pVal[i]))
                    {
                    case HTagInfo::ASCII:
                        if (GeoKey.GetCount((TIFFGeoKey)pVal[i]) < 256)
                            {
                            GeoKey.GetValues((TIFFGeoKey)pVal[i], aString);
                            fprintf(po_pOutput, " (%s)", aString);
                            }
                        break;

                    case HTagInfo::SHORT:
                        GeoKey.GetValue((TIFFGeoKey)pVal[i], &ValShort);
                        fprintf(po_pOutput, " (%d)", ValShort);
                        break;

                    case HTagInfo::DOUBLE:
                        if (GeoKey.GetCount((TIFFGeoKey)pVal[i]) < 10)
                            {
                            GeoKey.GetValues((TIFFGeoKey)pVal[i], aDouble, 0, GeoKey.GetCount((TIFFGeoKey)pVal[i]));
                            fprintf(po_pOutput, " (%lf", aDouble[0]);
                            for (uint32_t k=1; k<GeoKey.GetCount((TIFFGeoKey)pVal[i]); k++)
                                fprintf(po_pOutput, " ,%lf", aDouble[k]);
                            fprintf(po_pOutput, ")");
                            }
                        break;
                    }

                // Inidcate if the type was of the type expected
                if (HTIFFGeoKey::sGetExpectedDataType((TIFFGeoKey)pVal[i]) != GeoKey.GetDataType((TIFFGeoKey)pVal[i]))
                    {
                    fprintf(po_pOutput, " ERROR: Expected type: ");
                    if (HTIFFGeoKey::sGetExpectedDataType((TIFFGeoKey)pVal[i]) == HTagInfo::SHORT)
                        fprintf(po_pOutput, "SHORT");
                    else if (HTIFFGeoKey::sGetExpectedDataType((TIFFGeoKey)pVal[i]) == HTagInfo::DOUBLE)
                        fprintf(po_pOutput, "DOUBLE");
                    else if (HTIFFGeoKey::sGetExpectedDataType((TIFFGeoKey)pVal[i]) == HTagInfo::ASCII)
                        fprintf(po_pOutput, "ASCII");
                    else
                        fprintf(po_pOutput, "***UNKNOWN***");
                    }

                fprintf(po_pOutput, "\n");
                }
            }
        else
            fprintf(po_pOutput, "(present)\n");
        }

    if (m_pCurDir->TagIsPresent(GEODOUBLEPARAMS))
        {
        double* pVal;

        GetField (GEODOUBLEPARAMS, &ValL, &pVal);
        fprintf(po_pOutput, "  %s:", GetTagNameString(GEODOUBLEPARAMS));
        if (pi_Flag & TIFFPRINT_GEOKEYPARAMS)
            {
            fprintf(po_pOutput, "\n");
            for (uint32_t i=0; i<ValL; i++)
                fprintf(po_pOutput, "  %8.6lf", pVal[i]);
            fprintf(po_pOutput, "\n");
            }
        else
            fprintf(po_pOutput, "(present)\n");

        }

    if (m_pCurDir->TagIsPresent(GEOASCIIPARAMS))
        {
        if (pi_Flag & TIFFPRINT_GEOKEYPARAMS)
            {
            GetField (GEOASCIIPARAMS, &pValC);
            s_PrintAsciiTag(po_pOutput,GetTagNameString(GEOASCIIPARAMS), pValC);
            }
        else
            fprintf(po_pOutput, "  %s:(present)\n",GetTagNameString(GEOASCIIPARAMS));
        }

    if (m_pCurDir->TagIsPresent(HMR2_WELLKNOWNTEXT))
        {
        char*  pCharVal;
        if (m_pCurDir->GetValues(HMR2_WELLKNOWNTEXT, &pCharVal))
            {
            fprintf(po_pOutput, "  %s : \n", GetTagNameString(HMR2_WELLKNOWNTEXT));
            s_PrintAscii(po_pOutput, pCharVal);
            fprintf(po_pOutput, "\"\n");
            }
        }

    if (m_pCurDir->TagIsPresent(HMR2_ONDEMANDRASTERS_INFO))
        {   //The bytes represent UTF8 data, so print them as ASCII characters.
        Byte* pBytes;
        uint32_t NbBytes;

        if (m_pCurDir->GetValues(HMR2_ONDEMANDRASTERS_INFO, &NbBytes, &pBytes))
            {
            fprintf(po_pOutput, "  %s : \n", GetTagNameString(HMR2_ONDEMANDRASTERS_INFO));
            HAutoPtr<char> pTempBuffer(new char[NbBytes + 1]);

            BeStringUtilities::Memcpy((void*)pTempBuffer.get(), NbBytes, pBytes, NbBytes);

            pTempBuffer.get()[NbBytes] = '\0';

            s_PrintAscii(po_pOutput, pTempBuffer.get(), NbBytes);
            fprintf(po_pOutput, "\"\n");
            }
        }


// HMR
//
    if (m_pCurDir->TagIsPresent(HMR_IMAGEINFORMATION))
        {
        if (m_pFile->m_IsTiff64)
            {
            GetField (HMR_IMAGEINFORMATION, &Val64);
            fprintf(po_pOutput, "  %s: 0x%I64x\n", GetTagNameString(HMR_IMAGEINFORMATION), Val64);
            }
        else
            {
            GetField (HMR_IMAGEINFORMATION, &ValL);
            fprintf(po_pOutput, "  %s: 0x%lx\n", GetTagNameString(HMR_IMAGEINFORMATION), ValL);
            }
        }
    else if (m_pCurDir->TagIsPresent(HMR2_IMAGEINFORMATION))
        {
        if (m_pFile->m_IsTiff64)
            {
            GetField (HMR2_IMAGEINFORMATION, &Val64);
            fprintf(po_pOutput, "  %s: 0x%I64x\n", GetTagNameString(HMR2_IMAGEINFORMATION), Val64);
            }
        else
            {
            GetField (HMR2_IMAGEINFORMATION, &ValL);
            fprintf(po_pOutput, "  %s: 0x%lx\n", GetTagNameString(HMR2_IMAGEINFORMATION), ValL);
            }
        }

    if (m_pCurDir->TagIsPresent(HMR_VERSION))
        {
        GetField (HMR_VERSION, &ValL);
        fprintf(po_pOutput, "  %s: %ld\n", GetTagNameString(HMR_VERSION), ValL);
        }

    if (m_pCurDir->TagIsPresent(HMR_VERSION_MINOR))
        {
        GetField (HMR_VERSION_MINOR, &ValL);
        fprintf(po_pOutput, "  %s: %ld\n", GetTagNameString(HMR_VERSION_MINOR), ValL);
        }

    if (m_pCurDir->TagIsPresent(HMR_FILTERS))
        {
        GetField(HMR_FILTERS, &pValC);
        s_PrintAsciiTag(po_pOutput, GetTagNameString(HMR_FILTERS), pValC);
        }

    if (m_pCurDir->TagIsPresent(HMR_PADDING))
        {
        GetField (HMR_PADDING, &ValL);
        fprintf(po_pOutput, "  %s: %ld\n", GetTagNameString(HMR_PADDING), ValL);
        }

    if (m_pCurDir->TagIsPresent(HMR_PIXEL_TYPE_SPEC))
        {
        GetField (HMR_PIXEL_TYPE_SPEC, &ValL);
        fprintf(po_pOutput, "  %s: %ld (0=RGB, 1=BGR\n", GetTagNameString(HMR_PIXEL_TYPE_SPEC), ValL);
        }

    if (m_pCurDir->TagIsPresent(HMR_THUMBNAIL_COMPOSED))
        {
        GetField (HMR_THUMBNAIL_COMPOSED, &ValL);
        fprintf(po_pOutput, "  %s: %ld\n", GetTagNameString(HMR_THUMBNAIL_COMPOSED), ValL);
        }

    if (m_pCurDir->TagIsPresent(HMR_TRANSPARENCY_PALETTE))
        {
        fprintf(po_pOutput, "  %s:", GetTagNameString(HMR_TRANSPARENCY_PALETTE));

        if (pi_Flag & TIFFPRINT_COLORMAP)
            {
            Byte* pVal;

            GetField (HMR_TRANSPARENCY_PALETTE, &ValL, &pVal);
            fprintf(po_pOutput, "\n");

            uint32_t Num = 1L<<m_BitsByPixel;
            fprintf(po_pOutput, "\n");

            if (Num == 2)
                fprintf(po_pOutput, "   %5lu: %5u %5u \n",2, (Byte)pVal[0], (Byte)pVal[1]);
            else
                {
                for (uint32_t i=0; i<Num; i+=8)
                    fprintf(po_pOutput, "   %5lu: %5u %5u %5u %5u %5u %5u %5u %5u\n",i,
                            (Byte)pVal[i],
                            (Byte)pVal[i+1],
                            (Byte)pVal[i+2],
                            (Byte)pVal[i+3],
                            (Byte)pVal[i+4],
                            (Byte)pVal[i+5],
                            (Byte)pVal[i+6],
                            (Byte)pVal[i+7]);

                }
            fprintf (po_pOutput, "\n");
            }
        else
            fprintf(po_pOutput, " (present)\n");
        }

    if (m_pCurDir->TagIsPresent(HMR_IMAGECOORDINATESYSTEM))
        {
        GetField (HMR_IMAGECOORDINATESYSTEM, &pValC);
        fprintf(po_pOutput, "  %s: \"%s\"\n", GetTagNameString(HMR_IMAGECOORDINATESYSTEM), pValC);
        }

    if (m_pCurDir->TagIsPresent(HMR_XORIGIN))
        {
        GetField (HMR_XORIGIN, &ValD);
        fprintf(po_pOutput, "  %s: %lf\n", GetTagNameString(HMR_XORIGIN), ValD);
        }

    if (m_pCurDir->TagIsPresent(HMR_YORIGIN))
        {
        GetField (HMR_YORIGIN, &ValD);
        fprintf(po_pOutput, "  %s: %lf\n", GetTagNameString(HMR_YORIGIN), ValD);
        }

    if (m_pCurDir->TagIsPresent(HMR_XPIXELSIZE))
        {
        GetField (HMR_XPIXELSIZE, &ValD);
        fprintf(po_pOutput, "  %s: %lf\n", GetTagNameString(HMR_XPIXELSIZE), ValD);
        }

    if (m_pCurDir->TagIsPresent(HMR_YPIXELSIZE))
        {
        GetField (HMR_YPIXELSIZE, &ValD);
        fprintf(po_pOutput, "  %s: %lf\n", GetTagNameString(HMR_YPIXELSIZE), ValD);
        }

    if (m_pCurDir->TagIsPresent(HMR_HISTOGRAMDATETIME))
        {
        GetField (HMR_HISTOGRAMDATETIME, &pValC);
        fprintf(po_pOutput, "  %s: \"%s\"\n", GetTagNameString(HMR_HISTOGRAMDATETIME), pValC);
        }

    if (m_pCurDir->TagIsPresent(HMR_HISTOGRAM))
        {
        fprintf(po_pOutput, "  %s:", GetTagNameString(HMR_HISTOGRAM));

        if (pi_Flag & TIFFPRINT_HMR_HISTOGRAM)
            {
            uint32_t* pVal;

            GetField (HMR_HISTOGRAM, &ValL, &pVal);
            fprintf(po_pOutput, "\n");

            for (size_t i=0; i<ValL;)
                {
                fprintf (po_pOutput, " [%3llu] ", (uint64_t)i);
                for (size_t j=0; (j<8) && (i<ValL); j++, i++)
                    fprintf (po_pOutput, "%8u ", pVal[i]);
                fprintf (po_pOutput, "\n");
                }
            }
        else
            fprintf(po_pOutput, " (present)\n");
        }

    if (m_pCurDir->TagIsPresent(HMR_LOGICALSHAPE))
        {
        double* pVal;

        fprintf(po_pOutput, "  %s:", GetTagNameString(HMR_LOGICALSHAPE));

        if (pi_Flag & TIFFPRINT_HMR_LOGICALSHAPE)
            {
            fprintf(po_pOutput, "\n");
            GetField (HMR_LOGICALSHAPE, &ValL, &pVal);
            for (uint32_t i=0; i<ValL; i++)
                fprintf(po_pOutput, "%lf, ", pVal[i]);
            fprintf(po_pOutput, "\n\n");
            }
        else
            fprintf(po_pOutput, " (present)\n");
        }

    if (m_pCurDir->TagIsPresent(HMR_TRANSPARENTSHAPE))
        {
        double* pVal;

        fprintf(po_pOutput, "  %s:", GetTagNameString(HMR_TRANSPARENTSHAPE));

        if (pi_Flag & TIFFPRINT_HMR_TRANSPARENTSHAPE)
            {
            fprintf(po_pOutput, "\n");
            GetField (HMR_TRANSPARENTSHAPE, &ValL, &pVal);
            for (uint32_t i=0; i<ValL; i++)
                fprintf(po_pOutput, "%lf, ", pVal[i]);
            fprintf(po_pOutput, "\n\n");
            }
        else
            fprintf(po_pOutput, " (present)\n");
        }

    if (m_pCurDir->TagIsPresent(HMR_USERDATA))
        {
        if (pi_Flag & TIFFPRINT_HMR_USERDATA)
            {
            GetField (HMR_USERDATA, &pValC);
            fprintf(po_pOutput, "  %s: \"%s\"\n\n", GetTagNameString(HMR_USERDATA), pValC);
            }
        else
            fprintf(po_pOutput, "  %s: (present)\n", GetTagNameString(HMR_USERDATA));
        }


    if (m_pCurDir->TagIsPresent(HMR2_TILEFLAG))
        {
        fprintf(po_pOutput, "  %s:", GetTagNameString(HMR2_TILEFLAG));

        if (pi_Flag & TIFFPRINT_HMR2_TILEFLAG)
            {
            Byte* pVal;
            unsigned short Cnt     = 0;
            unsigned short CntRes  = 0;

            fprintf(po_pOutput, "\n Resolution : 0\n");
            GetField (HMR2_TILEFLAG, (char**)&pVal);
            for (uint32_t i=0; i<strlen((char*)pVal); ++i, ++Cnt)
                {
                if (Cnt >= 16)
                    {
                    fprintf(po_pOutput, "\n");
                    Cnt = 0;
                    }

                // End of Tile for this Resolution.
                if (pVal[i] == 0xff)
                    {
                    CntRes++;
                    fprintf(po_pOutput, "\n Resolution : %d\n", CntRes);
                    Cnt = 0;
                    }
                else
                    fprintf(po_pOutput, "0x%2x, ", pVal[i]);
                }

            fprintf(po_pOutput, "\n\n");
            }
        else
            fprintf(po_pOutput, " (present)\n");
        }

    if (m_pCurDir->TagIsPresent(HMR2_CHANNELS_WITH_NODATAVALUE))
        {

        uint32_t*    pIndexes;
        uint32_t    Count;

        if (m_pCurDir->GetValues(HMR2_CHANNELS_WITH_NODATAVALUE, &Count, &pIndexes))
            {
            fprintf(po_pOutput, "  %s : \n", GetTagNameString(HMR2_CHANNELS_WITH_NODATAVALUE));

            fprintf(po_pOutput, "\t");
            for (uint32_t iElement = 0; iElement < Count; ++iElement)
                {
                if (CHANNEL_INDEX_ALL_CHANNELS == pIndexes[iElement])
                    fprintf(po_pOutput, "%s ", "ALL_CHANNELS");
                else
                    fprintf(po_pOutput, "%ud ", pIndexes[iElement]);
                }
            fprintf(po_pOutput, "\n");
            }
        }

    if (m_pCurDir->TagIsPresent(HMR2_CHANNELS_NODATAVALUE))
        {

        double*    pValues;
        uint32_t    Count;

        if (m_pCurDir->GetValues(HMR2_CHANNELS_NODATAVALUE, &Count, &pValues))
            {
            fprintf(po_pOutput, "  %s : \n", GetTagNameString(HMR2_CHANNELS_NODATAVALUE));

            fprintf(po_pOutput, "\t");
            s_PrintDoubleArray(po_pOutput, pValues, Count);
            fprintf(po_pOutput, "\n");
            }
        }

    if (m_pCurDir->TagIsPresent(HMR_DECIMATION_METHOD))
        {
        uint32_t Count = 0;
        Byte* pVal;
        GetField (HMR_DECIMATION_METHOD, &Count, &pVal);
        fprintf(po_pOutput, "  %s : \n", GetTagNameString(HMR_DECIMATION_METHOD));

        for (unsigned short i=0; i<Count; i++)
            {
            switch(pVal[i])
                {
                case HMR_UNDEFINED_DECIMATION:
                    fprintf(po_pOutput, "\t Res %d: %s (%d)\n", i, " Undefined", pVal[i]);
                    break;

                case HMR_NEAREST_NEIGHBOUR_DECIMATION:
                    fprintf(po_pOutput, "\t Res %d: %s (%d)\n", i, " Nearest neighbour (Standard)", pVal[i]);
                    break;

                case HMR_AVERAGE_DECIMATION:
                    fprintf(po_pOutput, "\t Res %d: %s (%d)\n", i, " Average", pVal[i]);
                    break;

                case HMR_VECTOR_AWARENESS_DECIMATION:
                    fprintf(po_pOutput, "\t Res %d: %s (%d)\n", i, " Vector Awarness", pVal[i]);
                    break;

                case HMR_ORING4_DECIMATION:
                    fprintf(po_pOutput, "\t Res %d: %s (%d)\n", i, " Oring 4", pVal[i]);
                    break;

                case HMR_NONE_DECIMATION:
                    fprintf(po_pOutput, "\t Res %d: %s (%d)\n", i, " None ", pVal[i]);
                    break;

                default:
                    fprintf(po_pOutput, "\t Res %d: %s (%d)\n", i, " Unknow", pVal[i]);
                    break;
                }
            }
        }

    if (m_pCurDir->TagIsPresent(HMR_PROJECTWISE_BLOB))
        {
        fprintf(po_pOutput, "  %s: ", GetTagNameString(HMR_PROJECTWISE_BLOB));

        if (pi_Flag & TIFFPRINT_STRIPS)
            {
            if (m_pFile->m_IsTiff64)
                {
                uint64_t* pData;
                uint32_t DataSize;

                GetField(HMR_PROJECTWISE_BLOB, &DataSize, &pData);

                fprintf(po_pOutput, "  %lu DataBlocks:\n", DataSize/2);
                for (uint32_t i = 0; i<DataSize; i+=2)
                    fprintf(po_pOutput, "    %10lu: [%10I64u, %10I64u]\n",(i/2), pData[i], pData[i+1]);
                }
            else
                {
                uint32_t* pData;
                uint32_t DataSize;

                GetField(HMR_PROJECTWISE_BLOB, &DataSize, &pData);

                fprintf(po_pOutput, "  %lu DataBlocks:\n", DataSize/2);
                for (uint32_t i = 0; i<DataSize; i+=2)
                    fprintf(po_pOutput, "    %3lu: [%8lu, %8lu]\n",(i/2), pData[i], pData[i+1]);
                }
            }
        else
            fprintf(po_pOutput, "(present)\n");
        }


    if (m_pCurDir->TagIsPresent(HMR_SOURCEFILE_CREATIONDATE))
        {
        GetField (HMR_SOURCEFILE_CREATIONDATE, &pValC);
        fprintf(po_pOutput, "  %s: \"%s\"\n", GetTagNameString(HMR_SOURCEFILE_CREATIONDATE), pValC);
        }

    if (m_pCurDir->TagIsPresent(HMR_SYNCHRONIZE_FIELD))
        {
        GetField (HMR_SYNCHRONIZE_FIELD, &ValL);
        fprintf(po_pOutput, "  %s: 0x%lx\n", GetTagNameString(HMR_SYNCHRONIZE_FIELD), ValL);
        }

    if (m_pCurDir->TagIsPresent(FREEOFFSETS))
        {
        uint32_t* pOffset;
        uint32_t* pCount;
        uint32_t NbEntry;
        uint32_t FreeBytes = 0;

        GetField (FREEOFFSETS,    &NbEntry, &pOffset);
        GetField (FREEBYTECOUNTS, &NbEntry, &pCount);

        // Compute number of bytes free
        for (uint32_t i = 0; i<NbEntry; i++)
            FreeBytes += pCount[i];
        fprintf(po_pOutput, "  BytesTotal in Freeblocks: %lu\n", FreeBytes);

        if ((pi_Flag & TIFFPRINT_FREEBLOCKS) && (NbEntry > 0))
            {
            for (uint32_t i = 0; i<NbEntry; i++)
                fprintf(po_pOutput, "    %3lu: [%8lu, %8lu]\n",i, pOffset[i], pCount[i]);
            }
        }


    if ((pi_Flag & TIFFPRINT_STRIPS) && (m_NbData32 > 0))
        {
        uint32_t NbEntry = 0;
        uint32_t NbEntry2 = 0;
        uint32_t* pCount32  = 0;
        uint32_t* pOffset32 = 0;
        uint64_t* pCount64  = 0;
        uint64_t* pOffset64 = 0;

        if (m_pFile->m_IsTiff64)
            {
            m_pCurDir->GetValues ((IsTiled() ? TILEOFFSETS : STRIPOFFSETS),   &NbEntry, &pOffset64);
            m_pCurDir->GetValues ((IsTiled() ? TILEBYTECOUNTS : STRIPBYTECOUNTS),&NbEntry2, &pCount64);
            }
        else
            {
            m_pCurDir->GetValues ((IsTiled() ? TILEOFFSETS : STRIPOFFSETS),   &NbEntry, &pOffset32);
            m_pCurDir->GetValues ((IsTiled() ? TILEBYTECOUNTS : STRIPBYTECOUNTS),&NbEntry2, &pCount32);
            }

        if (m_pCurDir->TagIsPresent(STRIPOFFSETS))
            fprintf(po_pOutput, "  %lu Strips:\n", NbEntry);
        else if (m_pCurDir->TagIsPresent(TILEOFFSETS))
            fprintf(po_pOutput, "  %lu Tiles:\n", NbEntry);
        else
            fprintf(po_pOutput, "  Not found\n");

        if (NbEntry > 0 && NbEntry2 > 0)
            {
            if (m_pFile->m_IsTiff64)
                for (uint32_t i = 0; i<NbEntry; i++)
                    fprintf(po_pOutput, "    %8lu: [%20I64u, %20I64u]\n",i, pOffset64[i], pCount64[i]);
            else
                for (uint32_t i = 0; i<NbEntry; i++)
                    fprintf(po_pOutput, "    %3lu: [%8lu, %8lu]\n",i, pOffset32[i], pCount32[i]);
            }

        if ((m_NbData32 > 0) && (m_NbData32 != NbEntry || m_NbData32 != NbEntry2))
            {
            // Emulation by HTIFF
            fprintf(po_pOutput, "  Emulation by HTIFF library\n");

            if (m_pCurDir->TagIsPresent(STRIPOFFSETS))
                fprintf(po_pOutput, "    %lu Strips:\n", m_NbData32);
            else if (m_pCurDir->TagIsPresent(TILEOFFSETS))
                fprintf(po_pOutput, "    %lu Tiles:\n", m_NbData32);

            for (uint32_t i = 0; i<m_NbData32; i++)
                fprintf(po_pOutput, "      %3lu: [%8I64u, %8u]\n",i, GetOffset(i), GetCount(i));

            }
        }
    }

void HTIFFFile::PrintEXIFDefinedGPSTags(uint32_t pi_PageDirInd,
                                        FILE*  po_pOutput)
    {
    HPRECONDITION(pi_PageDirInd < m_ListDirCount);
    HPRECONDITION(IsTiff64() == false);

    uint32_t    GPSIFDOffset32;
    uint64_t   GPSIFDOffset64=0;

    if (m_pFile->m_IsTiff64)
        {
        m_ppListDir[pi_PageDirInd]->GetValues(GPSDIRECTORY, &GPSIFDOffset64);
        }
    else
        {
        m_ppListDir[pi_PageDirInd]->GetValues(GPSDIRECTORY, &GPSIFDOffset32);
        GPSIFDOffset64 = GPSIFDOffset32;
        }

    if (GPSIFDOffset64 != 0)
        {
        HTIFFError*     pTIFFError = 0;
        double         ConvRationalToDblVals[3];
        Byte*          pByteVal;
        unsigned short*        pShortVal;
        double*        pDblVal;
        char*          pCharVal;
        uint32_t        NbVals;
        vector<Byte>   ByteVec;
        vector<double> DblVec;

        fprintf(po_pOutput, "\nEXIF Related GPS Info Directory at offset 0x%I64x\n", GPSIFDOffset64);

        HAutoPtr<HTIFFDirectory> pGPSDir(new HTIFFDirectory(m_rTagInfo, &m_ByteOrder, IsTiff64()));

        pGPSDir->ReadDirectory(m_pFile, GPSIFDOffset64);

        if ((pGPSDir->IsValid(&pTIFFError) == true) ||
            (pTIFFError->IsFatal() == false))
            {
            if (pGPSDir->GetValues(GPS_VERSIONID, &NbVals, &pByteVal))
                {
                HASSERT(NbVals == 4);

                fprintf(po_pOutput, "  %s: ", pGPSDir->GetTagNameString(GPS_VERSIONID));

                for(unsigned short DigitInd = 0; DigitInd < NbVals; DigitInd++)
                    {
                    if (DigitInd == NbVals - 1)
                        {
                        fprintf(po_pOutput, "%ui\n", pByteVal[DigitInd]);
                        }
                    else
                        {
                        fprintf(po_pOutput, "%ui.", pByteVal[DigitInd]);
                        }
                    }
                }

            if (pGPSDir->GetValues(GPS_LATITUDEREF, &pCharVal))
                {
                if (BeStringUtilities::Stricmp("n", pCharVal) == 0)
                    {
                    s_PrintAsciiTag(po_pOutput, pGPSDir->GetTagNameString(GPS_LATITUDEREF), "North");
                    }
                else if (BeStringUtilities::Stricmp("s", pCharVal) == 0)
                    {
                    s_PrintAsciiTag(po_pOutput, pGPSDir->GetTagNameString(GPS_LATITUDEREF), "South");
                    }
                else
                    {
                    //Unknown units. Display only the abbreviation
                    HASSERT(0);
                    s_PrintAsciiTag(po_pOutput, pGPSDir->GetTagNameString(GPS_LATITUDEREF), pCharVal);
                    }
                }

            if (pGPSDir->GetValues(GPS_LATITUDE, &NbVals, &pDblVal))
                {
                HASSERT(NbVals == 3);

                pGPSDir->ConvertRationalToDblValues(NbVals,
                                                    pDblVal,
                                                    ConvRationalToDblVals);

                fprintf(po_pOutput,
                        "  %s: %.2f degrees, %.2f minutes, %.2f seconds\n",
                        pGPSDir->GetTagNameString(GPS_LATITUDE),
                        ConvRationalToDblVals[0],
                        ConvRationalToDblVals[1],
                        ConvRationalToDblVals[2]);
                }

            if (pGPSDir->GetValues(GPS_LONGITUDEREF, &pCharVal))
                {
                if (BeStringUtilities::Stricmp("e", pCharVal) == 0)
                    {
                    s_PrintAsciiTag(po_pOutput, pGPSDir->GetTagNameString(GPS_LONGITUDEREF), "East");
                    }
                else if (BeStringUtilities::Stricmp("w", pCharVal) == 0)
                    {
                    s_PrintAsciiTag(po_pOutput, pGPSDir->GetTagNameString(GPS_LONGITUDEREF), "West");
                    }
                else
                    {
                    //Unknown units. Display only the abbreviation
                    HASSERT(0);
                    s_PrintAsciiTag(po_pOutput, pGPSDir->GetTagNameString(GPS_LONGITUDEREF), pCharVal);
                    }
                }

            if (pGPSDir->GetValues(GPS_LONGITUDE, &NbVals, &pDblVal))
                {
                HASSERT(NbVals == 3);

                pGPSDir->ConvertRationalToDblValues(NbVals,
                                                    pDblVal,
                                                    ConvRationalToDblVals);

                fprintf(po_pOutput,
                        "  %s: %.2f degrees, %.2f minutes, %.2f seconds\n",
                        pGPSDir->GetTagNameString(GPS_LONGITUDE),
                        ConvRationalToDblVals[0],
                        ConvRationalToDblVals[1],
                        ConvRationalToDblVals[2]);
                }

            if (pGPSDir->GetValues(GPS_ALTITUDEREF, &NbVals, &pByteVal))
                {
                HASSERT(NbVals == 1);

                if (*pByteVal == 0)
                    {
                    s_PrintAsciiTag(po_pOutput, pGPSDir->GetTagNameString(GPS_ALTITUDEREF), "Above sea level");
                    }
                else if (*pByteVal == 1)
                    {
                    s_PrintAsciiTag(po_pOutput, pGPSDir->GetTagNameString(GPS_ALTITUDEREF), "Below sea level");
                    }
                else
                    {
                    //Unknown altitude reference
                    HASSERT(0);

                    fprintf(po_pOutput,
                            "  %s: %ul (Unknown altitude reference)\n",
                            pGPSDir->GetTagNameString(GPS_LATITUDEREF),
                            pByteVal[0]);
                    }
                }

            if (pGPSDir->GetValues(GPS_ALTITUDE, &NbVals, &pDblVal))
                {
                HASSERT(NbVals == 1);

                pGPSDir->ConvertRationalToDblValues(NbVals,
                                                    pDblVal,
                                                    ConvRationalToDblVals);

                fprintf(po_pOutput,
                        "  %s: %.2f meters\n",
                        pGPSDir->GetTagNameString(GPS_ALTITUDE),
                        ConvRationalToDblVals[0]);
                }

            if (pGPSDir->GetValues(GPS_TIMESTAMP, &NbVals, &pDblVal))
                {
                HASSERT(NbVals == 3);

                pGPSDir->ConvertRationalToDblValues(NbVals,
                                                    pDblVal,
                                                    ConvRationalToDblVals);

                fprintf(po_pOutput,
                        "  %s: %.2f hours, %.2f minutes, %.2f seconds\n",
                        pGPSDir->GetTagNameString(GPS_TIMESTAMP),
                        pDblVal[0],
                        pDblVal[1],
                        pDblVal[2]);
                }

            if (pGPSDir->GetValues(GPS_SATELLITES, &pCharVal))
                {
                s_PrintAsciiTag(po_pOutput, pGPSDir->GetTagNameString(GPS_SATELLITES), pCharVal);
                }

            if (pGPSDir->GetValues(GPS_STATUS, &pCharVal))
                {
                if (BeStringUtilities::Stricmp("a", pCharVal) == 0)
                    {
                    s_PrintAsciiTag(po_pOutput, pGPSDir->GetTagNameString(GPS_STATUS), "Measurement is in progress");
                    }
                else if (BeStringUtilities::Stricmp("v", pCharVal) == 0)
                    {
                    s_PrintAsciiTag(po_pOutput, pGPSDir->GetTagNameString(GPS_STATUS), "Measurement is Interoperability");
                    }
                else
                    {
                    //Unknown status
                    HASSERT(0);
                    s_PrintAsciiTag(po_pOutput, pGPSDir->GetTagNameString(GPS_STATUS), pCharVal);
                    }
                }

            if (pGPSDir->GetValues(GPS_MEASUREMODE, &pCharVal))
                {
                if (BeStringUtilities::Stricmp("2", pCharVal) == 0)
                    {
                    s_PrintAsciiTag(po_pOutput, pGPSDir->GetTagNameString(GPS_MEASUREMODE), "2-dimensional measurement");
                    }
                else if (BeStringUtilities::Stricmp("3", pCharVal) == 0)
                    {
                    s_PrintAsciiTag(po_pOutput, pGPSDir->GetTagNameString(GPS_MEASUREMODE), "3-dimensional measurement");
                    }
                else
                    {
                    //Unknown measure mode
                    HASSERT(0);
                    s_PrintAsciiTag(po_pOutput, pGPSDir->GetTagNameString(GPS_MEASUREMODE), pCharVal);
                    }
                }

            if (pGPSDir->GetValues(GPS_DOP, &NbVals, &pDblVal))
                {
                HASSERT(NbVals == 1);

                pGPSDir->ConvertRationalToDblValues(NbVals,
                                                    pDblVal,
                                                    ConvRationalToDblVals);

                fprintf(po_pOutput,
                        "  %s: %.2f\n",
                        pGPSDir->GetTagNameString(GPS_DOP),
                        ConvRationalToDblVals[0]);
                }

            if (pGPSDir->GetValues(GPS_SPEEDREF, &pCharVal))
                {
                if (BeStringUtilities::Stricmp("K", pCharVal) == 0)
                    {
                    s_PrintAsciiTag(po_pOutput, pGPSDir->GetTagNameString(GPS_SPEEDREF), "Kilometers per hour");
                    }
                else if (BeStringUtilities::Stricmp("M", pCharVal) == 0)
                    {
                    s_PrintAsciiTag(po_pOutput, pGPSDir->GetTagNameString(GPS_SPEEDREF), "Miles per hour");
                    }
                else if (BeStringUtilities::Stricmp("N", pCharVal) == 0)
                    {
                    s_PrintAsciiTag(po_pOutput, pGPSDir->GetTagNameString(GPS_SPEEDREF), "Knots");
                    }
                else
                    {
                    //Unknown units. Display only the abbreviation
                    HASSERT(0);
                    s_PrintAsciiTag(po_pOutput, pGPSDir->GetTagNameString(GPS_SPEEDREF), pCharVal);
                    }
                }

            if (pGPSDir->GetValues(GPS_SPEED, &NbVals, &pDblVal))
                {
                HASSERT(NbVals == 1);

                pGPSDir->ConvertRationalToDblValues(NbVals,
                                                    pDblVal,
                                                    ConvRationalToDblVals);

                fprintf(po_pOutput,
                        "  %s: %.2f\n",
                        pGPSDir->GetTagNameString(GPS_SPEED),
                        ConvRationalToDblVals[0]);
                }

            if (pGPSDir->GetValues(GPS_TRACKREF, &pCharVal))
                {
                if (BeStringUtilities::Stricmp("T", pCharVal) == 0)
                    {
                    s_PrintAsciiTag(po_pOutput, pGPSDir->GetTagNameString(GPS_TRACKREF), "True direction");
                    }
                else if (BeStringUtilities::Stricmp("M", pCharVal) == 0)
                    {
                    s_PrintAsciiTag(po_pOutput, pGPSDir->GetTagNameString(GPS_TRACKREF), "Magnetic direction");
                    }
                else
                    {
                    //Unknown track reference
                    HASSERT(0);
                    s_PrintAsciiTag(po_pOutput, pGPSDir->GetTagNameString(GPS_TRACKREF), pCharVal);
                    }
                }

            if (pGPSDir->GetValues(GPS_TRACK, &NbVals, &pDblVal))
                {
                HASSERT(NbVals == 1);

                pGPSDir->ConvertRationalToDblValues(NbVals,
                                                    pDblVal,
                                                    ConvRationalToDblVals);

                fprintf(po_pOutput,
                        "  %s: %.2f\n",
                        pGPSDir->GetTagNameString(GPS_TRACK),
                        ConvRationalToDblVals[0]);
                }

            if (pGPSDir->GetValues(GPS_IMGDIRECTIONREF, &pCharVal))
                {
                if (BeStringUtilities::Stricmp("T", pCharVal) == 0)
                    {
                    s_PrintAsciiTag(po_pOutput, pGPSDir->GetTagNameString(GPS_IMGDIRECTIONREF), "True direction");
                    }
                else if (BeStringUtilities::Stricmp("M", pCharVal) == 0)
                    {
                    s_PrintAsciiTag(po_pOutput, pGPSDir->GetTagNameString(GPS_IMGDIRECTIONREF), "Magnetic direction");
                    }
                else
                    {
                    //Unknown image direction reference
                    HASSERT(0);
                    s_PrintAsciiTag(po_pOutput, pGPSDir->GetTagNameString(GPS_IMGDIRECTIONREF), pCharVal);
                    }
                }

            if (pGPSDir->GetValues(GPS_IMGDIRECTION, &NbVals, &pDblVal))
                {
                HASSERT(NbVals == 1);

                pGPSDir->ConvertRationalToDblValues(NbVals,
                                                    pDblVal,
                                                    ConvRationalToDblVals);

                fprintf(po_pOutput,
                        "  %s: %.2f\n",
                        pGPSDir->GetTagNameString(GPS_IMGDIRECTION),
                        ConvRationalToDblVals[0]);
                }

            if (pGPSDir->GetValues(GPS_MAPDATUM, &pCharVal))
                {
                s_PrintAsciiTag(po_pOutput, pGPSDir->GetTagNameString(GPS_MAPDATUM), pCharVal);
                }

            if (pGPSDir->GetValues(GPS_DESTLATITUDEREF, &pCharVal))
                {
                if (BeStringUtilities::Stricmp("N", pCharVal) == 0)
                    {
                    s_PrintAsciiTag(po_pOutput, pGPSDir->GetTagNameString(GPS_DESTLATITUDEREF), "North latitude");
                    }
                else if (BeStringUtilities::Stricmp("S", pCharVal) == 0)
                    {
                    s_PrintAsciiTag(po_pOutput, pGPSDir->GetTagNameString(GPS_DESTLATITUDEREF), "South latitude");
                    }
                else
                    {
                    //Unknown destination latitude reference
                    HASSERT(0);
                    s_PrintAsciiTag(po_pOutput, pGPSDir->GetTagNameString(GPS_DESTLATITUDEREF), pCharVal);
                    }
                }

            if (pGPSDir->GetValues(GPS_DESTLATITUDE, &NbVals, &pDblVal))
                {
                HASSERT(NbVals == 3);

                pGPSDir->ConvertRationalToDblValues(NbVals,
                                                    pDblVal,
                                                    ConvRationalToDblVals);

                fprintf(po_pOutput,
                        "  %s: %.2f degrees, %.2f minutes, %.2f seconds\n",
                        pGPSDir->GetTagNameString(GPS_DESTLATITUDE),
                        ConvRationalToDblVals[0],
                        ConvRationalToDblVals[1],
                        ConvRationalToDblVals[2]);
                }

            if (pGPSDir->GetValues(GPS_DESTLONGITUDEREF, &pCharVal))
                {
                if (BeStringUtilities::Stricmp("E", pCharVal) == 0)
                    {
                    s_PrintAsciiTag(po_pOutput, pGPSDir->GetTagNameString(GPS_DESTLONGITUDEREF), "East longitude");
                    }
                else if (BeStringUtilities::Stricmp("W", pCharVal) == 0)
                    {
                    s_PrintAsciiTag(po_pOutput, pGPSDir->GetTagNameString(GPS_DESTLONGITUDEREF), "West longitude");
                    }
                else
                    {
                    //Unknown destination longitude reference
                    HASSERT(0);
                    s_PrintAsciiTag(po_pOutput, pGPSDir->GetTagNameString(GPS_DESTLONGITUDEREF), pCharVal);
                    }
                }

            if (pGPSDir->GetValues(GPS_DESTLONGITUDE, &NbVals, &pDblVal))
                {
                HASSERT(NbVals == 3);

                pGPSDir->ConvertRationalToDblValues(NbVals,
                                                    pDblVal,
                                                    ConvRationalToDblVals);

                fprintf(po_pOutput,
                        "  %s: %.2f degrees, %.2f minutes, %.2f seconds\n",
                        pGPSDir->GetTagNameString(GPS_DESTLONGITUDE),
                        ConvRationalToDblVals[0],
                        ConvRationalToDblVals[1],
                        ConvRationalToDblVals[2]);
                }

            if (pGPSDir->GetValues(GPS_DESTBEARINGREF, &pCharVal))
                {
                if (BeStringUtilities::Stricmp("T", pCharVal) == 0)
                    {
                    s_PrintAsciiTag(po_pOutput, pGPSDir->GetTagNameString(GPS_DESTBEARINGREF), "True direction");
                    }
                else if (BeStringUtilities::Stricmp("M", pCharVal) == 0)
                    {
                    s_PrintAsciiTag(po_pOutput, pGPSDir->GetTagNameString(GPS_DESTBEARINGREF), "Magnetic direction");
                    }
                else
                    {
                    //Unknown destination bearing reference
                    HASSERT(0);
                    s_PrintAsciiTag(po_pOutput, pGPSDir->GetTagNameString(GPS_DESTBEARINGREF), pCharVal);
                    }
                }

            if (pGPSDir->GetValues(GPS_DESTBEARING, &NbVals, &pDblVal))
                {
                HASSERT(NbVals == 1);

                pGPSDir->ConvertRationalToDblValues(NbVals,
                                                    pDblVal,
                                                    ConvRationalToDblVals);

                fprintf(po_pOutput,
                        "  %s: %.2f\n",
                        pGPSDir->GetTagNameString(GPS_DESTBEARING),
                        ConvRationalToDblVals[0]);
                }

            if (pGPSDir->GetValues(GPS_DESTDISTANCEREF, &pCharVal))
                {
                if (BeStringUtilities::Stricmp("K", pCharVal) == 0)
                    {
                    s_PrintAsciiTag(po_pOutput, pGPSDir->GetTagNameString(GPS_DESTDISTANCEREF), "Kilometers");
                    }
                else if (BeStringUtilities::Stricmp("M", pCharVal) == 0)
                    {
                    s_PrintAsciiTag(po_pOutput, pGPSDir->GetTagNameString(GPS_DESTDISTANCEREF), "Miles");
                    }
                else if (BeStringUtilities::Stricmp("N", pCharVal) == 0)
                    {
                    s_PrintAsciiTag(po_pOutput, pGPSDir->GetTagNameString(GPS_DESTDISTANCEREF), "Knots");
                    }
                else
                    {
                    //Unknown destination distance reference
                    HASSERT(0);
                    s_PrintAsciiTag(po_pOutput, pGPSDir->GetTagNameString(GPS_DESTDISTANCEREF), pCharVal);
                    }
                }

            if (pGPSDir->GetValues(GPS_DESTDISTANCE, &NbVals, &pDblVal))
                {
                HASSERT(NbVals == 1);

                pGPSDir->ConvertRationalToDblValues(NbVals,
                                                    pDblVal,
                                                    ConvRationalToDblVals);

                fprintf(po_pOutput,
                        "  %s: %.2f\n",
                        pGPSDir->GetTagNameString(GPS_DESTDISTANCE),
                        ConvRationalToDblVals[0]);
                }

            if (pGPSDir->GetValues(GPS_PROCESSINGMETHOD, &NbVals, &pByteVal))
                {
                //Character code
                fprintf(po_pOutput, "%c ", pByteVal[0]);

                //Processing method name
                for(unsigned short ValInd = 0; ValInd < NbVals; ValInd++)
                    {
                    fprintf(po_pOutput, "%c", pByteVal[ValInd]);
                    }

                fprintf(po_pOutput, "\n");
                }

            if (pGPSDir->GetValues(GPS_AREAINFORMATION, &NbVals, &pByteVal))
                {
                //Character code
                fprintf(po_pOutput, "%c ", pByteVal[0]);

                //GPS Area information
                for(unsigned short ValInd = 0; ValInd < NbVals; ValInd++)
                    {
                    fprintf(po_pOutput, "%c", pByteVal[ValInd]);
                    }

                fprintf(po_pOutput, "\n");
                }

            if (pGPSDir->GetValues(GPS_DATESTAMP, &pCharVal))
                {
                s_PrintAsciiTag(po_pOutput, pGPSDir->GetTagNameString(GPS_DATESTAMP), pCharVal);
                }

            if (pGPSDir->GetValues(GPS_DIFFERENTIAL, &NbVals, &pShortVal))
                {
                HASSERT(NbVals == 1);

                if (*pShortVal == 0)
                    {
                    s_PrintAsciiTag(po_pOutput, pGPSDir->GetTagNameString(GPS_DIFFERENTIAL), "Measurement without differential correction");
                    }
                else if (*pShortVal == 1)
                    {
                    s_PrintAsciiTag(po_pOutput, pGPSDir->GetTagNameString(GPS_DIFFERENTIAL), "Differential correction applied");
                    }
                else
                    {
                    //Unknown differential correction code
                    HASSERT(0);

                    fprintf(po_pOutput,
                            "  %s: %ul (Unknown differential correction code)\n",
                            pGPSDir->GetTagNameString(GPS_DIFFERENTIAL),
                            pShortVal[0]);
                    }
                }
            }
        }
    }


void HTIFFFile::PrintEXIFTags(uint32_t pi_PageDirInd,
                              FILE*  po_pOutput)
    {
    HPRECONDITION(pi_PageDirInd < m_ListDirCount);
    HPRECONDITION(IsTiff64() == false);

    uint32_t    EXIFIFDOffset;

    //By default, search the EXIF tags in the base directory
    HTIFFDirectory*          pDirToSearch(m_ppListDir[pi_PageDirInd]);
    HAutoPtr<HTIFFDirectory> pExifDir;
    HTIFFError*              pTIFFError = 0;
    double                  ConvRationalToDblVals[3];

    Byte                   UByteVal;
    unsigned short          UShortVal;
    uint32_t                 ULongVal;
    double                  DblVal;

    Byte*                   pByteVal;
    unsigned short*                 pUShortVal;
    double*                 pDblVal;
    char*                   pCharVal;

    uint32_t                 NbVals;
    vector<Byte>            ByteVec;
    vector<double>          DblVec;

    fprintf(po_pOutput, "\n--EXIF Tags--\n\n");

    //Check for a private IFD
    if (m_ppListDir[pi_PageDirInd]->GetValues(EXIFDIRECTORY, &EXIFIFDOffset))
        {
        fprintf(po_pOutput, "\nEXIF Directory at offset 0x%I32x\n", EXIFIFDOffset);

        pExifDir = new HTIFFDirectory(m_rTagInfo, &m_ByteOrder, IsTiff64());
        pExifDir->ReadDirectory(m_pFile, EXIFIFDOffset);

        if ((pExifDir->IsValid(&pTIFFError) == true) ||
            (pTIFFError->IsFatal() == false))
            {
            //If a private IFD is found, search the EXIF tags from that directory instead.
            pDirToSearch = pExifDir;
            }
        }

    if (pDirToSearch->GetValues(EXIF_EXPOSURETIME, &NbVals, &pDblVal))
        {
        HASSERT(NbVals == 1);

        pDirToSearch->ConvertRationalToDblValues(NbVals,
                                                 pDblVal,
                                                 ConvRationalToDblVals);

        fprintf(po_pOutput,
                "  %s: %.9f\n",
                pDirToSearch->GetTagNameString(EXIF_EXPOSURETIME),
                ConvRationalToDblVals[0]);
        }

    if (pDirToSearch->GetValues(EXIF_FNUMBER, &NbVals, &pDblVal))
        {
        HASSERT(NbVals == 1);

        pDirToSearch->ConvertRationalToDblValues(NbVals,
                                                 pDblVal,
                                                 ConvRationalToDblVals);

        fprintf(po_pOutput,
                "  %s: %.1f\n",
                pDirToSearch->GetTagNameString(EXIF_FNUMBER),
                ConvRationalToDblVals[0]);
        }

    if (pDirToSearch->GetValues(EXIF_EXPOSUREPROGRAM, &UShortVal))
        {
        string ExposureProgram;

        switch (UShortVal)
            {
            case 0 :
                ExposureProgram = "Not defined";
                break;
            case 1 :
                ExposureProgram = "Manual";
                break;
            case 2 :
                ExposureProgram = "Normal program";
                break;
            case 3 :
                ExposureProgram = "Aperture priority";
                break;
            case 4 :
                ExposureProgram = "Shutter priority";
                break;
            case 5 :
                ExposureProgram = "Creative program (biased toward depth of field)";
                break;
            case 6 :
                ExposureProgram = "Action program (biased toward fast shutter speed)";
                break;
            case 7 :
                ExposureProgram = "Portrait mode (for closeup photos with the background out of focus)";
                break;
            case 8 :
                ExposureProgram = "Landscape mode (for landscape photos with the background in focus)";
                break;
            default :
                HASSERT(0); //Unknown (new?) option
                ExposureProgram = "";
                break;
            }

        s_PrintAsciiTag(po_pOutput, pDirToSearch->GetTagNameString(EXIF_EXPOSUREPROGRAM), ExposureProgram.c_str());
        }

    if (pDirToSearch->GetValues(EXIF_SPECTRALSENSITIVITY, &pCharVal))
        {
        s_PrintAsciiTag(po_pOutput, pDirToSearch->GetTagNameString(EXIF_SPECTRALSENSITIVITY), pCharVal);
        }

    if (pDirToSearch->GetValues(EXIF_ISOSPEEDRATINGS, &NbVals, &pUShortVal))
        {
        //Formatted as specified in ISO 12232. Currently display unformatted
        fprintf(po_pOutput,
                "  %s: ",
                pDirToSearch->GetTagNameString(EXIF_ISOSPEEDRATINGS));

        for (uint32_t IndVal = 0; IndVal < NbVals; IndVal++)
            {
            fprintf(po_pOutput, "%u ", pUShortVal[IndVal]);
            }

        fprintf(po_pOutput, "\n");
        }

    if (pDirToSearch->GetValues(EXIF_OECF, &NbVals, &pByteVal))
        {
        fprintf(po_pOutput,
                "  %s (see the ISO 14524 specification for interpretation purpose) : ",
                pDirToSearch->GetTagNameString(EXIF_OECF));

        for (uint32_t IndVal = 0; IndVal < NbVals; IndVal++)
            {
            fprintf(po_pOutput, "%u ", pByteVal[IndVal]);
            }

        fprintf(po_pOutput, "\n");
        }

    if (pDirToSearch->GetValues(EXIF_EXIFVERSION, &NbVals, &pByteVal))
        {
        HASSERT(NbVals == 4);

        fprintf(po_pOutput,
                "  %s: %c%c.%c%c\n",
                pDirToSearch->GetTagNameString(EXIF_EXIFVERSION),
                pByteVal[0], pByteVal[1], pByteVal[2], pByteVal[3]);
        }

    if (pDirToSearch->GetValues(EXIF_DATETIMEORIGINAL, &pCharVal))
        {
        s_PrintAsciiTag(po_pOutput, pDirToSearch->GetTagNameString(EXIF_DATETIMEORIGINAL), pCharVal);
        }

    if (pDirToSearch->GetValues(EXIF_DATETIMEDIGITIZED, &pCharVal))
        {
        s_PrintAsciiTag(po_pOutput, pDirToSearch->GetTagNameString(EXIF_DATETIMEDIGITIZED), pCharVal);
        }

    if (pDirToSearch->GetValues(EXIF_COMPONENTSCONFIGURATION, &NbVals, &pByteVal))
        {
        HASSERT(NbVals == 4);

        fprintf(po_pOutput,
                "  %s: \n",
                pDirToSearch->GetTagNameString(EXIF_COMPONENTSCONFIGURATION));

        string PhotometricIntr;
        for (unsigned short ChInd = 0; ChInd < NbVals; ChInd++)
            {
            switch (pByteVal[ChInd])
                {
                case 0 :
                    PhotometricIntr = "does not exist";
                    break;
                case 1 :
                    PhotometricIntr = "Y";
                    break;
                case 2 :
                    PhotometricIntr = "Cb";
                    break;
                case 3 :
                    PhotometricIntr = "Cr";
                    break;
                case 4 :
                    PhotometricIntr = "R";
                    break;
                case 5 :
                    PhotometricIntr = "G";
                    break;
                case 6 :
                    PhotometricIntr = "B";
                    break;
                default :
                    PhotometricIntr = "reserved";
                    break;
                }
            fprintf(po_pOutput,
                    "    Channel %u: %s\n",
                    ChInd,
                    PhotometricIntr.c_str());
            }

        fprintf(po_pOutput, "\n");
        }

    if (pDirToSearch->GetValues(EXIF_COMPRESSEDBITSPERPIXEL, &NbVals, &pDblVal))
        {
        HASSERT(NbVals == 1);

        pDirToSearch->ConvertRationalToDblValues(NbVals,
                                                 pDblVal,
                                                 ConvRationalToDblVals);

        fprintf(po_pOutput,
                "  %s: %.2f\n",
                pDirToSearch->GetTagNameString(EXIF_COMPRESSEDBITSPERPIXEL),
                ConvRationalToDblVals[0]);
        }

    if (pDirToSearch->GetValues(EXIF_SHUTTERSPEEDVALUE, &NbVals, &pDblVal))
        {
        HASSERT(NbVals == 1);

        pDirToSearch->ConvertRationalToDblValues(NbVals,
                                                 pDblVal,
                                                 ConvRationalToDblVals,
                                                 true);

        fprintf(po_pOutput,
                "  %s: %.2f\n",
                pDirToSearch->GetTagNameString(EXIF_SHUTTERSPEEDVALUE),
                ConvRationalToDblVals[0]);
        }


    if (pDirToSearch->GetValues(EXIF_APERTUREVALUE, &NbVals, &pDblVal))
        {
        HASSERT(NbVals == 1);

        pDirToSearch->ConvertRationalToDblValues(NbVals,
                                                 pDblVal,
                                                 ConvRationalToDblVals);

        fprintf(po_pOutput,
                "  %s: %.2f\n",
                pDirToSearch->GetTagNameString(EXIF_APERTUREVALUE),
                ConvRationalToDblVals[0]);
        }

    if (pDirToSearch->GetValues(EXIF_BRIGHTNESSVALUE, &NbVals, &pDblVal))
        {
        HASSERT(NbVals == 1);

        if (*((uint64_t*)pDblVal) == 0xFFFFFFFF)
            {
            s_PrintAsciiTag(po_pOutput, pDirToSearch->GetTagNameString(EXIF_BRIGHTNESSVALUE), "Unknown value");
            }
        else
            {
            pDirToSearch->ConvertRationalToDblValues(NbVals,
                                                     pDblVal,
                                                     ConvRationalToDblVals,
                                                     true);

            fprintf(po_pOutput,
                    "  %s: %.2f\n",
                    pDirToSearch->GetTagNameString(EXIF_BRIGHTNESSVALUE),
                    ConvRationalToDblVals[0]);
            }
        }

    if (pDirToSearch->GetValues(EXIF_EXPOSUREBIASVALUE, &NbVals, &pDblVal))
        {
        HASSERT(NbVals == 1);

        pDirToSearch->ConvertRationalToDblValues(NbVals,
                                                 pDblVal,
                                                 ConvRationalToDblVals,
                                                 true);

        fprintf(po_pOutput,
                "  %s: %.2f\n",
                pDirToSearch->GetTagNameString(EXIF_EXPOSUREBIASVALUE),
                ConvRationalToDblVals[0]);
        }

    if (pDirToSearch->GetValues(EXIF_MAXAPERTUREVALUE, &NbVals, &pDblVal))
        {
        pDirToSearch->ConvertRationalToDblValues(NbVals,
                                                 pDblVal,
                                                 ConvRationalToDblVals);

        fprintf(po_pOutput,
                "  %s: %.2f\n",
                pDirToSearch->GetTagNameString(EXIF_MAXAPERTUREVALUE),
                ConvRationalToDblVals[0]);
        }

    if (pDirToSearch->GetValues(EXIF_SUBJECTDISTANCE, &NbVals, &pDblVal))
        {
        if (*((uint64_t*)pDblVal) == 0xFFFFFFFF)
            {
            s_PrintAsciiTag(po_pOutput, pDirToSearch->GetTagNameString(EXIF_SUBJECTDISTANCE), "Infinity");
            }
        else if (*((uint64_t*)pDblVal) == 0)
            {
            s_PrintAsciiTag(po_pOutput, pDirToSearch->GetTagNameString(EXIF_SUBJECTDISTANCE), "Distance unknown");
            }
        else
            {
            pDirToSearch->ConvertRationalToDblValues(NbVals,
                                                     pDblVal,
                                                     ConvRationalToDblVals,
                                                     true);

            fprintf(po_pOutput,
                    "  %s: %.2f\n",
                    pDirToSearch->GetTagNameString(EXIF_SUBJECTDISTANCE),
                    ConvRationalToDblVals[0]);
            }
        }

    if (pDirToSearch->GetValues(EXIF_METERINGMODE, &UShortVal))
        {
        string MeteringMode;

        switch (UShortVal)
            {
            case 0 :
                MeteringMode = "Unknown";
                break;
            case 1 :
                MeteringMode = "Average";
                break;
            case 2 :
                MeteringMode = "CenterWeightedAverage";
                break;
            case 3 :
                MeteringMode = "Spot";
                break;
            case 4 :
                MeteringMode = "MultiSpot";
                break;
            case 5 :
                MeteringMode = "Pattern";
                break;
            case 6 :
                MeteringMode = "Partial";
                break;
            case 255 :
                MeteringMode = "other";
                break;
            default :
                HASSERT (0); //Unknown (new?) mode
                MeteringMode = "";
                break;
            }

        fprintf(po_pOutput,
                "  %s: %s\n",
                pDirToSearch->GetTagNameString(EXIF_METERINGMODE),
                MeteringMode.c_str());
        }

    if (pDirToSearch->GetValues(EXIF_LIGHTSOURCE, &UShortVal))
        {
        string LightSource;

        switch (UShortVal)
            {
            case 0 :
                LightSource = "Unknown";
                break;
            case 1 :
                LightSource = "Daylight";
                break;
            case 2 :
                LightSource = "Fluorescent";
                break;
            case 3 :
                LightSource = "Tungsten (incandescent light)";
                break;
            case 4 :
                LightSource = "Flash";
                break;
            case 9 :
                LightSource = "Fine weather";
                break;
            case 10 :
                LightSource = "Cloudy weather";
                break;
            case 11 :
                LightSource = "Shade";
                break;
            case 12 :
                LightSource = "Daylight fluorescent (D 5700 - 7100K)";
                break;
            case 13 :
                LightSource = "Day white fluorescent (N 4600 - 5400K)";
                break;
            case 14 :
                LightSource = "Cool white fluorescent (W 3900 - 4500K)";
                break;
            case 15 :
                LightSource = "White fluorescent (WW 3200 - 3700K)";
                break;
            case 17 :
                LightSource = "Standard light A";
                break;
            case 18 :
                LightSource = "Standard light B";
                break;
            case 19 :
                LightSource = "Standard light C";
                break;
            case 20 :
                LightSource = "D55";
                break;
            case 21 :
                LightSource = "D65";
                break;
            case 22 :
                LightSource = "D75";
                break;
            case 23 :
                LightSource = "D50";
                break;
            case 24 :
                LightSource = "ISO studio tungsten";
                break;
            case 255 :
                LightSource = "Other light source";
                break;
            default :
                HASSERT (0); //Unknown (new?) mode
                LightSource = "";
                break;
            }

        fprintf(po_pOutput,
                "  %s: %s\n",
                pDirToSearch->GetTagNameString(EXIF_LIGHTSOURCE),
                LightSource.c_str());
        }

    if (pDirToSearch->GetValues(EXIF_FLASH, &UShortVal))
        {
        string Flash;

        switch (UShortVal)
            {
            case 0x0000 :
                Flash = "Flash did not fire";
                break;
            case 0x0001 :
                Flash = "Flash fired";
                break;
            case 0x0005 :
                Flash = "Strobe return light not detected";
                break;
            case 0x0007 :
                Flash = "Strobe return light detected";
                break;
            case 0x0009 :
                Flash = "Flash fired, compulsory flash mode";
                break;
            case 0x000D :
                Flash = "Flash fired, compulsory flash mode, return light not detected";
                break;
            case 0x000F :
                Flash = "Flash fired, compulsory flash mode, return light detected";
                break;
            case 0x0010 :
                Flash = "Flash did not fire, compulsory flash mode";
                break;
            case 0x0018 :
                Flash = "Flash did not fire, auto mode";
                break;
            case 0x0019 :
                Flash = "Flash fired, auto mode";
                break;
            case 0x001D :
                Flash = "Flash fired, auto mode, return light not detected";
                break;
            case 0x001F :
                Flash = "Flash fired, auto mode, return light detected";
                break;
            case 0x0020 :
                Flash = "No flash function";
                break;
            case 0x0041 :
                Flash = "Flash fired, red-eye reduction mode";
                break;
            case 0x0045 :
                Flash = "Flash fired, red-eye reduction mode, return light not detected";
                break;
            case 0x0047 :
                Flash = "Flash fired, red-eye reduction mode, return light detected";
                break;
            case 0x0049 :
                Flash = "Flash fired, compulsory flash mode, red-eye reduction mode";
                break;
            case 0x004D :
                Flash = "Flash fired, compulsory flash mode, red-eye reduction mode, return light not detected";
                break;
            case 0x004F :
                Flash = "Flash fired, compulsory flash mode, red-eye reduction mode, return light detected";
                break;
            case 0x0059 :
                Flash = "Flash fired, auto mode, red-eye reduction mode";
                break;
            case 0x005D :
                Flash = "Flash fired, auto mode, return light not detected, red-eye reduction mode";
                break;
            case 0x005F :
                Flash = "Flash fired, auto mode, return light detected, red-eye reduction mode ";
                break;
            default :
                {
                if ((UShortVal & 0x0001) == 0)
                    {
                    Flash += "Flash did not fire";
                    }
                else
                    {
                    Flash += "Flash fired";
                    }

                switch ((UShortVal & 0x0006) >> 1)
                    {
                    case 0x00 :
                        Flash += ", No strobe return detection function";
                        break;
                    case 0x01 :
                        HASSERT(0); //reserved
                        break;
                    case 0x10 :
                        Flash += ", Strobe return light not detected";
                        break;
                    case 0x11 :
                        Flash += ", Strobe return light detected ";
                        break;
                    }

                switch ((UShortVal & 0x0018) >> 3)
                    {
                    case 0x00 :
                        Flash += ", unknown";
                        break;
                    case 0x01 :
                        Flash += ", Compulsory flash firing";
                        break;
                    case 0x10 :
                        Flash += ", Compulsory flash suppression";
                        break;
                    case 0x11 :
                        Flash += ", Auto mode";
                        break;
                    }

                switch ((UShortVal & 0x0020) >> 6)
                    {
                    case 0x0 :
                        Flash += ", No red-eye reduction mode or unknown";
                        break;
                    case 0x1 :
                        Flash += ", Red-eye reduction supported";
                        break;
                    }
                }
            }

        fprintf(po_pOutput,
                "  %s: %s\n",
                pDirToSearch->GetTagNameString(EXIF_FLASH),
                Flash.c_str());
        }

    if (pDirToSearch->GetValues(EXIF_FOCALLENGTH, &NbVals, &pDblVal))
        {
        HASSERT(NbVals == 1);

        pDirToSearch->ConvertRationalToDblValues(NbVals,
                                                 pDblVal,
                                                 ConvRationalToDblVals);

        fprintf(po_pOutput,
                "  %s: %.2f\n",
                pDirToSearch->GetTagNameString(EXIF_FOCALLENGTH),
                ConvRationalToDblVals[0]);
        }

    if (pDirToSearch->GetValues(EXIF_SUBJECTAREA, &NbVals, &pUShortVal))
        {
        HASSERT((NbVals >= 2) && (NbVals <= 4));

        switch (NbVals)
            {
            case 2 :
                fprintf(po_pOutput,
                        "  %s: \n    Center X : %u\n    Center Y : %u",
                        pDirToSearch->GetTagNameString(EXIF_FOCALLENGTH),
                        pUShortVal[0],
                        pUShortVal[1]);
                break;
            case 3 :
                fprintf(po_pOutput,
                        "  %s: \n    Center X : %u\n    Center Y : %u\n    Diameter : %u",
                        pDirToSearch->GetTagNameString(EXIF_FOCALLENGTH),
                        pUShortVal[0],
                        pUShortVal[1],
                        pUShortVal[2]);
                break;
            case 4 :
                fprintf(po_pOutput,
                        "  %s: \n    Center X : %u\n    Center Y : %u\n",
                        pDirToSearch->GetTagNameString(EXIF_FOCALLENGTH),
                        pUShortVal[0],
                        pUShortVal[1]);

                fprintf(po_pOutput,
                        "    Width : %u\n    Height : %u\n",
                        pUShortVal[2],
                        pUShortVal[3]);
                break;
            }
        }

    if (pDirToSearch->GetValues(EXIF_MAKERNOTE, &NbVals, &pByteVal))
        {
        fprintf(po_pOutput,
                "  %s: ",
                pDirToSearch->GetTagNameString(EXIF_MAKERNOTE));

        for (uint32_t IndVal = 0; IndVal < NbVals; IndVal++)
            {
            fprintf(po_pOutput, "%u ", pByteVal[IndVal]);
            }

        fprintf(po_pOutput, "\n");
        }

    if (pDirToSearch->GetValues(EXIF_USERCOMMENT, &NbVals, &pByteVal))
        {
        HASSERT(NbVals >= 8);

        fprintf(po_pOutput,
                "  %s: ",
                pDirToSearch->GetTagNameString(EXIF_USERCOMMENT));

        //ASCII comment
        if (strcmp((char*)pByteVal, "ASCII") == 0)
            {
            HAutoPtr<char> pUserComment(new char[NbVals - 7]);
            strncpy(pUserComment, (char*)(pByteVal + 8), NbVals - 8);
            pUserComment[NbVals - 8] = '\0';

            fprintf(po_pOutput, "%s\n", pUserComment.get());
            }

        if ((strcmp((char*)pByteVal, "JIS") == 0) ||
            (strcmp((char*)pByteVal, "") == 0))
            {
            for (uint32_t IndVal = 8; IndVal < NbVals; IndVal++)
                {
                fprintf(po_pOutput, "%u ", pByteVal[IndVal]);
                }
            fprintf(po_pOutput, "\n");
            }

        //Unicode comment
        if (strcmp((char*)pByteVal, "UNICODE") == 0)
            {
            //Unicode characters are encoded with 2 bytes,
            //the number of bytes remaining should be even.
            HASSERT(((NbVals - 8) % 2) != 0);

            uint32_t NbChars = (NbVals - 8) / 2;
            HAutoPtr<WChar> pUserComment(new WChar[NbChars + 1]);
            wcsncpy(pUserComment, (WChar*)(pByteVal + 8), NbChars);
            pUserComment[NbChars] = L'\0';

            fwprintf(po_pOutput, L"%s\n", pUserComment.get());
            }
        }

    if (pDirToSearch->GetValues(EXIF_SUBSECTIME, &pCharVal))
        {
        s_PrintAsciiTag(po_pOutput, pDirToSearch->GetTagNameString(EXIF_SUBSECTIME), pCharVal);
        }

    if (pDirToSearch->GetValues(EXIF_SUBSECTIME_ORIGINAL, &pCharVal))
        {
        s_PrintAsciiTag(po_pOutput, pDirToSearch->GetTagNameString(EXIF_SUBSECTIME_ORIGINAL), pCharVal);
        }

    if (pDirToSearch->GetValues(EXIF_SUBSECTIME_DIGITIZED, &pCharVal))
        {
        s_PrintAsciiTag(po_pOutput, pDirToSearch->GetTagNameString(EXIF_SUBSECTIME_DIGITIZED), pCharVal);
        }

    if (pDirToSearch->GetValues(EXIF_FLASHPIXVERSION, &NbVals, &pByteVal))
        {
        HASSERT(NbVals == 4);

        fprintf(po_pOutput,
                "  %s: %c%c.%c%c\n",
                pDirToSearch->GetTagNameString(EXIF_FLASHPIXVERSION),
                pByteVal[0], pByteVal[1], pByteVal[2], pByteVal[3]);
        }

    if (pDirToSearch->GetValues(EXIF_COLORSPACE, &NbVals, &pUShortVal))
        {
        switch (*pUShortVal)
            {
            case 1 :
                s_PrintAsciiTag(po_pOutput, pDirToSearch->GetTagNameString(EXIF_COLORSPACE), "RGB");
                break;

            case 65535 :
                s_PrintAsciiTag(po_pOutput, pDirToSearch->GetTagNameString(EXIF_COLORSPACE), "Uncalibrated");
                break;

            default :
                HASSERT(0); //Unknown (new?) value
                s_PrintAsciiTag(po_pOutput, pDirToSearch->GetTagNameString(EXIF_COLORSPACE), "");
                break;
            }
        }

    if (pDirToSearch->GetValues(EXIF_PIXELXDIMENSION, &ULongVal))
        {
        fprintf(po_pOutput,
                "  %s: %u\n",
                pDirToSearch->GetTagNameString(EXIF_PIXELXDIMENSION),
                ULongVal);
        }

    if (pDirToSearch->GetValues(EXIF_PIXELYDIMENSION, &ULongVal))
        {
        fprintf(po_pOutput,
                "  %s: %u\n",
                pDirToSearch->GetTagNameString(EXIF_PIXELYDIMENSION),
                ULongVal);
        }

    if (pDirToSearch->GetValues(EXIF_RELATEDSOUNDFILE, &pCharVal))
        {
        s_PrintAsciiTag(po_pOutput, pDirToSearch->GetTagNameString(EXIF_RELATEDSOUNDFILE), pCharVal);
        }

    if (pDirToSearch->GetValues(EXIF_FLASHENERGY, &NbVals, &pDblVal))
        {
        HASSERT(NbVals == 1);

        pDirToSearch->ConvertRationalToDblValues(NbVals,
                                                 pDblVal,
                                                 ConvRationalToDblVals);

        fprintf(po_pOutput,
                "  %s: %.2f\n",
                pDirToSearch->GetTagNameString(EXIF_FLASHENERGY),
                ConvRationalToDblVals[0]);
        }

    if (pDirToSearch->GetValues(EXIF_SPATIALFREQUENCYRESPONSE, &NbVals, &pByteVal))
        {
        //Formatted as specified in ISO 12232. Currently display unformatted
        fprintf(po_pOutput,
                "  %s: ",
                pDirToSearch->GetTagNameString(EXIF_SPATIALFREQUENCYRESPONSE));

        for (uint32_t IndVal = 0; IndVal < NbVals; IndVal++)
            {
            fprintf(po_pOutput, "%u ", pUShortVal[IndVal]);
            }

        fprintf(po_pOutput, "\n");
        }

    if (pDirToSearch->GetValues(EXIF_FOCALPLANEXRESOLUTION, &NbVals, &pDblVal))
        {
        HASSERT(NbVals == 1);

        pDirToSearch->ConvertRationalToDblValues(NbVals,
                                                 pDblVal,
                                                 ConvRationalToDblVals);

        fprintf(po_pOutput,
                "  %s: %.2f\n",
                pDirToSearch->GetTagNameString(EXIF_FOCALPLANEXRESOLUTION),
                ConvRationalToDblVals[0]);
        }

    if (pDirToSearch->GetValues(EXIF_FOCALPLANEYRESOLUTION, &NbVals, &pDblVal))
        {
        HASSERT(NbVals == 1);

        pDirToSearch->ConvertRationalToDblValues(NbVals,
                                                 pDblVal,
                                                 ConvRationalToDblVals);

        fprintf(po_pOutput,
                "  %s: %.2f\n",
                pDirToSearch->GetTagNameString(EXIF_FOCALPLANEYRESOLUTION),
                ConvRationalToDblVals[0]);
        }

    if (pDirToSearch->GetValues(EXIF_FOCALPLANERESOLUTIONUNIT, &UShortVal))
        {
        switch (*pUShortVal)
            {
            case 1 :
                s_PrintAsciiTag(po_pOutput, pDirToSearch->GetTagNameString(EXIF_FOCALPLANERESOLUTIONUNIT), "No absolute unit of measurement");
                break;

            case 2 :
                s_PrintAsciiTag(po_pOutput, pDirToSearch->GetTagNameString(EXIF_FOCALPLANERESOLUTIONUNIT), "Inch");
                break;

            case 3 :
                s_PrintAsciiTag(po_pOutput, pDirToSearch->GetTagNameString(EXIF_FOCALPLANERESOLUTIONUNIT), "Centimeter");
                break;

            default :
                HASSERT(0); //Unknown (new?) value
                s_PrintAsciiTag(po_pOutput, pDirToSearch->GetTagNameString(EXIF_FOCALPLANERESOLUTIONUNIT), "");
                break;
            }
        }

    if (pDirToSearch->GetValues(EXIF_SUBJECTLOCATION, &NbVals, &pUShortVal))
        {
        HASSERT(NbVals == 2);

        fprintf(po_pOutput,
                "  %s: column %u, row %u\n",
                pDirToSearch->GetTagNameString(EXIF_SUBJECTLOCATION),
                pUShortVal[0],
                pUShortVal[1]);
        }

    if (pDirToSearch->GetValues(EXIF_EXPOSUREINDEX, &NbVals, &pDblVal))
        {
        HASSERT(NbVals == 1);

        pDirToSearch->ConvertRationalToDblValues(NbVals,
                                                 pDblVal,
                                                 ConvRationalToDblVals);

        fprintf(po_pOutput,
                "  %s: %.2f\n",
                pDirToSearch->GetTagNameString(EXIF_EXPOSUREINDEX),
                ConvRationalToDblVals[0]);
        }

    if (pDirToSearch->GetValues(EXIF_SENSINGMETHOD, &UShortVal))
        {
        string SensingMethod;

        switch (UShortVal)
            {
            case 1 :
                SensingMethod = "Not defined";
                break;
            case 2 :
                SensingMethod = "One-chip color area sensor";
                break;
            case 3 :
                SensingMethod = "Two-chip color area sensor";
                break;
            case 4 :
                SensingMethod = "Three-chip color area sensor";
                break;
            case 5 :
                SensingMethod = "Color sequential area sensor";
                break;
            case 7 :
                SensingMethod = "Trilinear sensor";
                break;
            case 8 :
                SensingMethod = "Color sequential linear sensor";
                break;
            default :
                HASSERT(0); //Unknown (new?) sensing method
                SensingMethod = "";
                break;
            }

        fprintf(po_pOutput,
                "  %s: %s\n",
                pDirToSearch->GetTagNameString(EXIF_SENSINGMETHOD),
                SensingMethod.c_str());
        }

    if (pDirToSearch->GetValues(EXIF_FILESOURCE, &UByteVal))
        {
        if (UByteVal == 3)
            {
            fprintf(po_pOutput,
                    "  %s: 3 (Digital Still Camera)\n",
                    pDirToSearch->GetTagNameString(EXIF_FILESOURCE));
            }
        else
            {
            fprintf(po_pOutput,
                    "  %s: %u\n",
                    pDirToSearch->GetTagNameString(EXIF_FILESOURCE),
                    UByteVal);
            }
        }

    if (pDirToSearch->GetValues(EXIF_SCENETYPE, &UByteVal))
        {
        if (UByteVal == 1)
            {
            fprintf(po_pOutput,
                    "  %s: 1 (Directly photographed image)\n",
                    pDirToSearch->GetTagNameString(EXIF_SCENETYPE));
            }
        else
            {
            fprintf(po_pOutput,
                    "  %s: %u\n",
                    pDirToSearch->GetTagNameString(EXIF_SCENETYPE),
                    UByteVal);
            }
        }

    if (pDirToSearch->GetValues(EXIF_CFAPATTERN, &NbVals, &pByteVal))
        {
        //Check the EXIF specification to interpret this tag's data.
        fprintf(po_pOutput,
                "  %s: ",
                pDirToSearch->GetTagNameString(EXIF_CFAPATTERN));

        for (uint32_t IndVal = 0; IndVal < NbVals; IndVal++)
            {
            fprintf(po_pOutput, "%u ", pByteVal[IndVal]);
            }

        fprintf(po_pOutput, "\n");
        }

    if (pDirToSearch->GetValues(EXIF_CUSTOMRENDERED, &UShortVal))
        {
        switch (UShortVal)
            {
            case 0 :
                s_PrintAsciiTag(po_pOutput, pDirToSearch->GetTagNameString(EXIF_CUSTOMRENDERED), "Normal process");
                break;

            case 1 :
                s_PrintAsciiTag(po_pOutput, pDirToSearch->GetTagNameString(EXIF_CUSTOMRENDERED), "Custom process");
                break;

            default :
                HASSERT(0); //Unknown (new?) value
                s_PrintAsciiTag(po_pOutput, pDirToSearch->GetTagNameString(EXIF_CUSTOMRENDERED), "");
                break;
            }
        }

    if (pDirToSearch->GetValues(EXIF_EXPOSUREMODE, &UShortVal))
        {
        switch (UShortVal)
            {
            case 0 :
                s_PrintAsciiTag(po_pOutput, pDirToSearch->GetTagNameString(EXIF_EXPOSUREMODE), "Auto exposure");
                break;

            case 1 :
                s_PrintAsciiTag(po_pOutput, pDirToSearch->GetTagNameString(EXIF_EXPOSUREMODE), "Manual exposure");
                break;

            case 2 :
                s_PrintAsciiTag(po_pOutput, pDirToSearch->GetTagNameString(EXIF_EXPOSUREMODE), "Auto bracket");
                break;

            default :
                HASSERT(0); //Unknown (new?) value
                s_PrintAsciiTag(po_pOutput, pDirToSearch->GetTagNameString(EXIF_EXPOSUREMODE), "");
                break;
            }
        }

    if (pDirToSearch->GetValues(EXIF_WHITEBALANCE, &UShortVal))
        {
        switch (UShortVal)
            {
            case 0 :
                s_PrintAsciiTag(po_pOutput, pDirToSearch->GetTagNameString(EXIF_WHITEBALANCE), "Auto white balance");
                break;

            case 1 :
                s_PrintAsciiTag(po_pOutput, pDirToSearch->GetTagNameString(EXIF_WHITEBALANCE), "Manual white balance ");
                break;

            default :
                HASSERT(0); //Unknown (new?) value
                s_PrintAsciiTag(po_pOutput, pDirToSearch->GetTagNameString(EXIF_WHITEBALANCE), "");
                break;
            }
        }

    if (pDirToSearch->GetValues(EXIF_DIGITALZOOMRATIO, &DblVal))
        {
        HASSERT(NbVals == 1);

        pDirToSearch->ConvertRationalToDblValues(NbVals,
                                                 &DblVal,
                                                 ConvRationalToDblVals);

        fprintf(po_pOutput,
                "  %s: %.2f\n",
                pDirToSearch->GetTagNameString(EXIF_DIGITALZOOMRATIO),
                ConvRationalToDblVals[0]);
        }

    if (pDirToSearch->GetValues(EXIF_FOCALLENGTHIN35MMFILM, &UShortVal))
        {
        fprintf(po_pOutput,
                "  %s: %u\n",
                pDirToSearch->GetTagNameString(EXIF_FOCALLENGTHIN35MMFILM),
                UShortVal);
        }

    if (pDirToSearch->GetValues(EXIF_SCENECAPTURETYPE, &UShortVal))
        {
        switch (UShortVal)
            {
            case 0 :
                s_PrintAsciiTag(po_pOutput, pDirToSearch->GetTagNameString(EXIF_SCENECAPTURETYPE), "Standard");
                break;

            case 1 :
                s_PrintAsciiTag(po_pOutput, pDirToSearch->GetTagNameString(EXIF_SCENECAPTURETYPE), "Landscape");
                break;

            case 2 :
                s_PrintAsciiTag(po_pOutput, pDirToSearch->GetTagNameString(EXIF_SCENECAPTURETYPE), "Portrait");
                break;

            case 3 :
                s_PrintAsciiTag(po_pOutput, pDirToSearch->GetTagNameString(EXIF_SCENECAPTURETYPE), "Night scene");
                break;

            default :
                HASSERT(0); //Unknown (new?) value
                s_PrintAsciiTag(po_pOutput, pDirToSearch->GetTagNameString(EXIF_SCENECAPTURETYPE), "");
                break;
            }
        }

    if (pDirToSearch->GetValues(EXIF_GAINCONTROL, &UShortVal))
        {
        switch (UShortVal)
            {
            case 0 :
                s_PrintAsciiTag(po_pOutput, pDirToSearch->GetTagNameString(EXIF_GAINCONTROL), "None");
                break;

            case 1 :
                s_PrintAsciiTag(po_pOutput, pDirToSearch->GetTagNameString(EXIF_GAINCONTROL), "Low gain up");
                break;

            case 2 :
                s_PrintAsciiTag(po_pOutput, pDirToSearch->GetTagNameString(EXIF_GAINCONTROL), "High gain up");
                break;

            case 3 :
                s_PrintAsciiTag(po_pOutput, pDirToSearch->GetTagNameString(EXIF_GAINCONTROL), "Low gain down");
                break;

            case 4 :
                s_PrintAsciiTag(po_pOutput, pDirToSearch->GetTagNameString(EXIF_GAINCONTROL), "High gain down");
                break;

            default :
                HASSERT(0); //Unknown (new?) value
                s_PrintAsciiTag(po_pOutput, pDirToSearch->GetTagNameString(EXIF_GAINCONTROL), "");
                break;
            }
        }

    if (pDirToSearch->GetValues(EXIF_CONTRAST, &UShortVal))
        {
        switch (UShortVal)
            {
            case 0 :
                s_PrintAsciiTag(po_pOutput, pDirToSearch->GetTagNameString(EXIF_CONTRAST), "Normal");
                break;

            case 1 :
                s_PrintAsciiTag(po_pOutput, pDirToSearch->GetTagNameString(EXIF_CONTRAST), "Soft");
                break;

            case 2 :
                s_PrintAsciiTag(po_pOutput, pDirToSearch->GetTagNameString(EXIF_CONTRAST), "Hard");
                break;

            default :
                HASSERT(0); //Unknown (new?) value
                s_PrintAsciiTag(po_pOutput, pDirToSearch->GetTagNameString(EXIF_CONTRAST), "");
                break;
            }
        }

    if (pDirToSearch->GetValues(EXIF_SATURATION, &UShortVal))
        {
        switch (UShortVal)
            {
            case 0 :
                s_PrintAsciiTag(po_pOutput, pDirToSearch->GetTagNameString(EXIF_SATURATION), "Normal");
                break;

            case 1 :
                s_PrintAsciiTag(po_pOutput, pDirToSearch->GetTagNameString(EXIF_SATURATION), "Low saturation");
                break;

            case 2 :
                s_PrintAsciiTag(po_pOutput, pDirToSearch->GetTagNameString(EXIF_SATURATION), "High saturation");
                break;

            default :
                HASSERT(0); //Unknown (new?) value
                s_PrintAsciiTag(po_pOutput, pDirToSearch->GetTagNameString(EXIF_SATURATION), "");
                break;
            }
        }

    if (pDirToSearch->GetValues(EXIF_SHARPNESS, &UShortVal))
        {
        switch (UShortVal)
            {
            case 0 :
                s_PrintAsciiTag(po_pOutput, pDirToSearch->GetTagNameString(EXIF_SHARPNESS), "Normal");
                break;

            case 1 :
                s_PrintAsciiTag(po_pOutput, pDirToSearch->GetTagNameString(EXIF_SHARPNESS), "Soft");
                break;

            case 2 :
                s_PrintAsciiTag(po_pOutput, pDirToSearch->GetTagNameString(EXIF_SHARPNESS), "Hard");
                break;

            default :
                HASSERT(0); //Unknown (new?) value
                s_PrintAsciiTag(po_pOutput, pDirToSearch->GetTagNameString(EXIF_SHARPNESS), "");
                break;
            }
        }

    if (pDirToSearch->GetValues(EXIF_DEVICESETTINGDESCRIPTION, &NbVals, &pByteVal))
        {
        fprintf(po_pOutput,
                "  %s: ",
                pDirToSearch->GetTagNameString(EXIF_DEVICESETTINGDESCRIPTION));

        for (uint32_t IndVal = 0; IndVal < NbVals; IndVal++)
            {
            fprintf(po_pOutput, "%u ", pByteVal[IndVal]);
            }

        fprintf(po_pOutput, "\n");
        }

    if (pDirToSearch->GetValues(EXIF_SUBJECTDISTANCERANGE, &UShortVal))
        {
        switch (UShortVal)
            {
            case 0 :
                s_PrintAsciiTag(po_pOutput, pDirToSearch->GetTagNameString(EXIF_SUBJECTDISTANCERANGE), "Unknown");
                break;

            case 1 :
                s_PrintAsciiTag(po_pOutput, pDirToSearch->GetTagNameString(EXIF_SUBJECTDISTANCERANGE), "Macro");
                break;

            case 2 :
                s_PrintAsciiTag(po_pOutput, pDirToSearch->GetTagNameString(EXIF_SUBJECTDISTANCERANGE), "Close view");
                break;

            case 3 :
                s_PrintAsciiTag(po_pOutput, pDirToSearch->GetTagNameString(EXIF_SUBJECTDISTANCERANGE), "Distant view");
                break;

            default :
                HASSERT(0); //Unknown (new?) value
                s_PrintAsciiTag(po_pOutput, pDirToSearch->GetTagNameString(EXIF_SUBJECTDISTANCERANGE), "");
                break;
            }
        }

    if (pDirToSearch->GetValues(EXIF_IMAGEUNIQUEID, &pCharVal))
        {
        s_PrintAsciiTag(po_pOutput, pDirToSearch->GetTagNameString(EXIF_IMAGEUNIQUEID), pCharVal);
        }
    }


//-------------------------------------------------------- Privates

static
void s_PrintAscii(FILE* po_pOutput, const char* pi_pCar, int32_t pi_NbCars)
    {
    for (; pi_NbCars == -1 ? *pi_pCar != '\0' : pi_NbCars-- > 0; pi_pCar++) {
        const char* pTmp;

        if (isprint((unsigned char)*pi_pCar)) {
            fputc(*pi_pCar, po_pOutput);
            continue;
            }
        for (pTmp = "\tt\bb\rr\nn\vv"; *pTmp; pTmp++)
            if (*pTmp++ == *pi_pCar)
                break;
        if (*pTmp)
            fprintf(po_pOutput, "\\%c", *pTmp);
        else
            fprintf(po_pOutput, "\\%03o", (unsigned char)*pi_pCar & 0xff);
        }
    }

static
void s_PrintAsciiTag(FILE* po_pOutput, const char* pi_pTagName, const char* pi_pString)
    {
    fprintf(po_pOutput, "  %s: \"", pi_pTagName);
    s_PrintAscii(po_pOutput, pi_pString);
    fprintf(po_pOutput, "\"\n");
    }


static
void s_PrintDoubleArray(FILE* po_pOutput, const double* pi_pArray, uint32_t pi_NbLongs)
    {
    HPRECONDITION(0 != pi_pArray);
    HPRECONDITION(0 != po_pOutput);

    for (uint32_t iElement = 0; iElement < pi_NbLongs; ++iElement)
        {
        fprintf(po_pOutput, "%8.6lf ", pi_pArray[iElement]);
        }
    }