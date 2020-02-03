/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ImageConverter/Parser.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//---------------------------------------------------------------------------
// $Header
//---------------------------------------------------------------------------
// Class : HPSParser
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

#ifndef __GTIFFParser_H__
#define __GTIFFParser_H__

#if 0

#include <HPAParser.h>
#include <HTIFFFile.h>
#include <HTIFFGeoKey.h>

class GTIFFParser : public HPAParser
{
    public:

                            GTIFFParser(HTIFFFile& pi_rTiffFile, HTIFFGeoKey& pi_rGeoKey);
        virtual             ~GTIFFParser();   

    protected:

    private:

        friend class IdentifierNode;
        friend class TagInfoNode;
        friend class KeyInfoNode;
        friend class GTIFFTokenizer;

        // Tokens

        HPAToken            LP_tk;
        HPAToken            RP_tk;
        HPAToken            COMMA_tk;
        HPAToken            COLON_tk;
        HPAToken            MINUS_tk;
        HPAToken            DOT_tk;
        HPAToken            Error_tk;

        HPAToken            GEOTIFF_INFORMATION_tk;
        HPAToken            END_OF_GEOTIFF_tk;
        HPAToken            VERSION_tk;
        HPAToken            KEY_REVISION_tk;
        HPAToken            TAGGED_INFORMATION_tk;
        HPAToken            END_OF_TAGS_tk;
        HPAToken            KEYED_INFORMATION_tk;
        HPAToken            END_OF_KEYS_tk;
        HPAToken            SHORT_tk;
        HPAToken            ASCII_tk;
        HPAToken            DOUBLE_tk;

        HPAToken            Number_tk;
        HPAToken            String_tk;
        HPAToken            Identifier_tk;

       // Rules

        HPARule             GeoTIFF;
        HPARule             Body;
        HPARule             Header;
        HPARule             HeaderEntry;
        HPARule             VersionExpression;
        HPARule             KeyRevisionExpression;
        HPARule             Sections;
        HPARule             Section;
        HPARule             TaggedInformation;
        HPARule             KeyedInformation;
        HPARule             TagList;
        HPARule             TagInfo;
        HPARule             NumberList;
        HPARule             NumericExpression;
        HPARule             KeyList;
        HPARule             KeyInfo;
        HPARule             TypeExpression;
        HPARule             ValueExpression;

        HTIFFGeoKey&        m_rGeoKey;
        HTIFFFile&          m_rTiffFile;
};


#endif

#endif
