/******************************************************************************
 * $Id: gdalmultidomainmetadata.cpp 21812 2011-02-23 21:47:23Z rouault $
 *
 * Project:  GDAL Core
 * Purpose:  Implementation of GDALMultiDomainMetadata class.  This class
 *           manages metadata items for a variable list of domains. 
 * Author:   Frank Warmerdam, warmerdam@pobox.com
 *
 ******************************************************************************
 * Copyright (c) 2005, Frank Warmerdam <warmerdam@pobox.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 ****************************************************************************/

#include "gdal_pam.h"
#include "cpl_string.h"
#include <map>

CPL_CVSID("$Id: gdalmultidomainmetadata.cpp 21812 2011-02-23 21:47:23Z rouault $");

/************************************************************************/
/*                      GDALMultiDomainMetadata()                       */
/************************************************************************/

GDALMultiDomainMetadata::GDALMultiDomainMetadata()

{
    papszDomainList = NULL;
    papapszMetadataLists = NULL;
}

/************************************************************************/
/*                      ~GDALMultiDomainMetadata()                      */
/************************************************************************/

GDALMultiDomainMetadata::~GDALMultiDomainMetadata()

{
    Clear();
}

/************************************************************************/
/*                               Clear()                                */
/************************************************************************/

void GDALMultiDomainMetadata::Clear()

{
    int i, nDomainCount;

    nDomainCount = CSLCount( papszDomainList );
    CSLDestroy( papszDomainList );
    papszDomainList = NULL;

    for( i = 0; i < nDomainCount; i++ )
    {
        CSLDestroy( papapszMetadataLists[i] );
    }
    CPLFree( papapszMetadataLists );
    papapszMetadataLists = NULL;
}


/************************************************************************/
/*                            GetMetadata()                             */
/************************************************************************/

char **GDALMultiDomainMetadata::GetMetadata( const char *pszDomain )

{
    if( pszDomain == NULL )
        pszDomain = "";

    int iDomain = CSLFindString( papszDomainList, pszDomain );

    if( iDomain == -1 )
        return NULL;
    else
        return papapszMetadataLists[iDomain];
}

/************************************************************************/
/*                            SetMetadata()                             */
/************************************************************************/

CPLErr GDALMultiDomainMetadata::SetMetadata( char **papszMetadata, 
                                             const char *pszDomain )

{
    if( pszDomain == NULL )
        pszDomain = "";

    int iDomain = CSLFindString( papszDomainList, pszDomain );

    if( iDomain == -1 )
    {
        int nDomainCount;

        papszDomainList = CSLAddString( papszDomainList, pszDomain );
        nDomainCount = CSLCount( papszDomainList );

        papapszMetadataLists = (char ***) 
            CPLRealloc( papapszMetadataLists, sizeof(char*)*(nDomainCount+1) );
        papapszMetadataLists[nDomainCount] = NULL;
        papapszMetadataLists[nDomainCount-1] = CSLDuplicate( papszMetadata );
    }
    else
    {
        CSLDestroy( papapszMetadataLists[iDomain] );
        papapszMetadataLists[iDomain] = CSLDuplicate( papszMetadata );
    }

    return CE_None;
}

/************************************************************************/
/*                          GetMetadataItem()                           */
/************************************************************************/

const char *GDALMultiDomainMetadata::GetMetadataItem( const char *pszName, 
                                                      const char *pszDomain )

{
    char **papszMD = GetMetadata( pszDomain );
    if( papszMD != NULL )
        return CSLFetchNameValue( papszMD, pszName );
    else
        return NULL;
}

/************************************************************************/
/*                          SetMetadataItem()                           */
/************************************************************************/

CPLErr GDALMultiDomainMetadata::SetMetadataItem( const char *pszName,
                                                 const char *pszValue,
                                                 const char *pszDomain )

{
    if( pszDomain == NULL )
        pszDomain = "";

    int iDomain = CSLFindString( papszDomainList, pszDomain );

/* -------------------------------------------------------------------- */
/*      Create the domain if it does not already exist.                 */
/* -------------------------------------------------------------------- */
    if( iDomain == -1 )
    {
        int nDomainCount;

        papszDomainList = CSLAddString( papszDomainList, pszDomain );
        nDomainCount = CSLCount( papszDomainList );

        papapszMetadataLists = (char ***) 
            CPLRealloc( papapszMetadataLists, sizeof(char*)*(nDomainCount+1) );
        papapszMetadataLists[nDomainCount] = NULL;
        iDomain = nDomainCount-1;
        papapszMetadataLists[iDomain] = NULL;
    }

/* -------------------------------------------------------------------- */
/*      Set the value in the domain list.                               */
/* -------------------------------------------------------------------- */
    if( pszValue != NULL )
    {
        papapszMetadataLists[iDomain] = 
            CSLSetNameValue( papapszMetadataLists[iDomain], 
                             pszName, pszValue );
    }

/* -------------------------------------------------------------------- */
/*      Remove the target key from the domain list.                     */
/* -------------------------------------------------------------------- */
    else
    {
        int iKey = CSLFindName( papapszMetadataLists[iDomain], pszName );

        if( iKey != -1 )
            papapszMetadataLists[iDomain] = 
                CSLRemoveStrings(papapszMetadataLists[iDomain],iKey,1,NULL);
    }

    return CE_None;
}

/************************************************************************/
/*                              XMLInit()                               */
/*                                                                      */
/*      This method should be invoked on the parent of the              */
/*      <Metadata> elements.                                            */
/************************************************************************/

int GDALMultiDomainMetadata::XMLInit( CPLXMLNode *psTree, int bMerge )

{
    CPLXMLNode *psMetadata;

/* ==================================================================== */
/*      Process all <Metadata> elements, each for one domain.           */
/* ==================================================================== */
    for( psMetadata = psTree->psChild; 
         psMetadata != NULL; psMetadata = psMetadata->psNext )
    {
        char **papszMD = NULL;
        CPLXMLNode *psMDI;
        const char *pszDomain, *pszFormat;

        if( psMetadata->eType != CXT_Element
            || !EQUAL(psMetadata->pszValue,"Metadata") )
            continue;

        pszDomain = CPLGetXMLValue( psMetadata, "domain", "" );
        pszFormat = CPLGetXMLValue( psMetadata, "format", "" );

/* -------------------------------------------------------------------- */
/*      XML format subdocuments.                                        */
/* -------------------------------------------------------------------- */
        if( EQUAL(pszFormat,"xml") )
        {
            CPLXMLNode *psSubDoc;

            /* find first non-attribute child of current element */
            psSubDoc = psMetadata->psChild;
            while( psSubDoc != NULL && psSubDoc->eType == CXT_Attribute )
                psSubDoc = psSubDoc->psNext;
            
            char *pszDoc = CPLSerializeXMLTree( psSubDoc );

            papszMD = (char **) CPLCalloc(sizeof(char*),2);
            papszMD[0] = pszDoc;
        }

/* -------------------------------------------------------------------- */
/*      Name value format.                                              */
/*      <MDI key="...">value_Text</MDI>                                 */
/* -------------------------------------------------------------------- */
        else
        {
            /* Keep a map of keys to ensure that if duplicate keys are found */
            /* in the metadata, the newer values will replace the older */
            /* ones, as done with CSLSetNameValue() before r21714 */
            std::map<CPLString, int> oMap;
            if( bMerge )
            {
                papszMD = GetMetadata( pszDomain );
                if( papszMD != NULL )
                {
                    papszMD = CSLDuplicate( papszMD );
                    for(int i=0;papszMD[i] != NULL;i++)
                    {
                        char* pszKey = NULL;
                        CPLParseNameValue(papszMD[i], &pszKey);
                        if (pszKey)
                        {
                            oMap[pszKey] = i;
                            CPLFree(pszKey);
                        }
                    }
                }
            }

            int nCount = 0;
            for( psMDI = psMetadata->psChild; psMDI != NULL; 
                 psMDI = psMDI->psNext )
            {
                if( !EQUAL(psMDI->pszValue,"MDI") 
                    || psMDI->eType != CXT_Element 
                    || psMDI->psChild == NULL 
                    || psMDI->psChild->psNext == NULL 
                    || psMDI->psChild->eType != CXT_Attribute
                    || psMDI->psChild->psChild == NULL )
                    continue;
                nCount ++;
            }

            if( nCount > 0 )
            {
                int nPrevSize = CSLCount(papszMD);
                papszMD = (char**)CPLRealloc(papszMD,
                            (nPrevSize + nCount + 1) * sizeof(char*));
                int i = nPrevSize;
                for( psMDI = psMetadata->psChild; psMDI != NULL;
                     psMDI = psMDI->psNext )
                {
                    if( !EQUAL(psMDI->pszValue,"MDI")
                        || psMDI->eType != CXT_Element
                        || psMDI->psChild == NULL
                        || psMDI->psChild->psNext == NULL
                        || psMDI->psChild->eType != CXT_Attribute
                        || psMDI->psChild->psChild == NULL )
                        continue;

                    char* pszName = psMDI->psChild->psChild->pszValue;
                    char* pszValue = psMDI->psChild->psNext->pszValue;
                    if( pszName != NULL && pszValue != NULL )
                    {
                        char* pszLine = (char *) CPLMalloc(strlen(pszName)+
                                                        strlen(pszValue)+2);
                        sprintf( pszLine, "%s=%s", pszName, pszValue );
                        std::map<CPLString, int>::iterator iter = oMap.find(pszName);
                        if (iter == oMap.end())
                        {
                            oMap[pszName] = i;
                            papszMD[i++] = pszLine;
                        }
                        else
                        {
                            int iToReplace = iter->second;
                            CPLFree(papszMD[iToReplace]);
                            papszMD[iToReplace] = pszLine;
                        }
                    }
                }
                papszMD[i] = NULL;
            }
        }

        SetMetadata( papszMD, pszDomain );
        CSLDestroy( papszMD );
    }

    return CSLCount(papszDomainList) != 0;
}

/************************************************************************/
/*                             Serialize()                              */
/************************************************************************/

CPLXMLNode *GDALMultiDomainMetadata::Serialize()

{
    CPLXMLNode *psFirst = NULL;

    for( int iDomain = 0; 
         papszDomainList != NULL && papszDomainList[iDomain] != NULL; 
         iDomain++)
    {
        char **papszMD = papapszMetadataLists[iDomain];
        CPLXMLNode *psMD;
        int bFormatXML = FALSE;
        
        psMD = CPLCreateXMLNode( NULL, CXT_Element, "Metadata" );

        if( strlen( papszDomainList[iDomain] ) > 0 )
            CPLCreateXMLNode( 
                CPLCreateXMLNode( psMD, CXT_Attribute, "domain" ), 
                CXT_Text, papszDomainList[iDomain] );

        if( EQUALN(papszDomainList[iDomain],"xml:",4) 
            && CSLCount(papszMD) == 1 )
        {
            CPLXMLNode *psValueAsXML = CPLParseXMLString( papszMD[0] );
            if( psValueAsXML != NULL )
            {
                bFormatXML = TRUE;

                CPLCreateXMLNode( 
                    CPLCreateXMLNode( psMD, CXT_Attribute, "format" ), 
                    CXT_Text, "xml" );
                
                CPLAddXMLChild( psMD, psValueAsXML );
            }
        }

        if( !bFormatXML )
        {
            CPLXMLNode* psLastChild = NULL;
            if( psMD->psChild != NULL )
            {
                psLastChild = psMD->psChild;
                while( psLastChild->psNext != NULL )
                    psLastChild = psLastChild->psNext; 
            }
            for( int i = 0; papszMD != NULL && papszMD[i] != NULL; i++ )
            {
                const char *pszRawValue;
                char *pszKey = NULL;
                CPLXMLNode *psMDI;
                
                pszRawValue = CPLParseNameValue( papszMD[i], &pszKey );
                
                psMDI = CPLCreateXMLNode( NULL, CXT_Element, "MDI" );
                if( psLastChild == NULL )
                    psMD->psChild = psMDI;
                else
                    psLastChild->psNext = psMDI;
                psLastChild = psMDI;

                CPLSetXMLValue( psMDI, "#key", pszKey );
                CPLCreateXMLNode( psMDI, CXT_Text, pszRawValue );
                
                CPLFree( pszKey );
            }
        }
            
        if( psFirst == NULL )
            psFirst = psMD;
        else
            CPLAddXMLSibling( psFirst, psMD );
    }

    return psFirst;
}

