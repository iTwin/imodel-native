/******************************************************************************
 * $Id: sdtsattrreader.cpp 10645 2007-01-18 02:22:39Z warmerdam $
 *
 * Project:  SDTS Translator
 * Purpose:  Implementation of SDTSAttrReader class.
 * Author:   Frank Warmerdam, warmerdam@pobox.com
 *
 ******************************************************************************
 * Copyright (c) 1999, Frank Warmerdam
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

#include "sdts_al.h"

CPL_CVSID("$Id: sdtsattrreader.cpp 10645 2007-01-18 02:22:39Z warmerdam $");


/************************************************************************/
/* ==================================================================== */
/*                             SDTSAttrRecord                           */
/* ==================================================================== */
/************************************************************************/

/************************************************************************/
/*                           SDTSAttrRecord()                           */
/************************************************************************/

SDTSAttrRecord::SDTSAttrRecord()

{
    poWholeRecord = NULL;
    poATTR = NULL;
}

/************************************************************************/
/*                          ~SDTSAttrRecord()                           */
/************************************************************************/

SDTSAttrRecord::~SDTSAttrRecord()

{
    if( poWholeRecord != NULL )
        delete poWholeRecord;
}

/************************************************************************/
/*                                Dump()                                */
/************************************************************************/

void SDTSAttrRecord::Dump( FILE * fp )

{
    if( poATTR != NULL )
        poATTR->Dump( fp );
}


/************************************************************************/
/* ==================================================================== */
/*                             SDTSAttrReader                           */
/*                                                                      */
/*      This is the class used to read a primary attribute module.      */
/* ==================================================================== */
/************************************************************************/

/************************************************************************/
/*                           SDTSAttrReader()                           */
/************************************************************************/

SDTSAttrReader::SDTSAttrReader( SDTS_IREF * poIREFIn )

{
    poIREF = poIREFIn;
}

/************************************************************************/
/*                          ~SDTSAttrReader()                           */
/************************************************************************/

SDTSAttrReader::~SDTSAttrReader()
{
    Close();
}

/************************************************************************/
/*                               Close()                                */
/************************************************************************/

void SDTSAttrReader::Close()

{
    ClearIndex();
    oDDFModule.Close();
}

/************************************************************************/
/*                                Open()                                */
/*                                                                      */
/*      Open the requested attr file, and prepare to start reading      */
/*      data records.                                                   */
/************************************************************************/

int SDTSAttrReader::Open( const char *pszFilename )

{
    int         bSuccess;

    bSuccess = oDDFModule.Open( pszFilename );

    if( bSuccess )
        bIsSecondary = (oDDFModule.FindFieldDefn("ATTS") != NULL);

    return bSuccess;
}

/************************************************************************/
/*                           GetNextRecord()                            */
/************************************************************************/

DDFField *SDTSAttrReader::GetNextRecord( SDTSModId * poModId,
                                         DDFRecord ** ppoRecord,
                                         int bDuplicate )

{
    DDFRecord   *poRecord;
    DDFField    *poATTP;
    
/* -------------------------------------------------------------------- */
/*      Fetch a record.                                                 */
/* -------------------------------------------------------------------- */
    if( ppoRecord != NULL )
        *ppoRecord = NULL;
    
    if( oDDFModule.GetFP() == NULL )
        return NULL;

    poRecord = oDDFModule.ReadRecord();

    if( poRecord == NULL )
        return NULL;

/* -------------------------------------------------------------------- */
/*      Make a copy of the record for persistent use if requested by    */
/*      the caller.                                                     */
/* -------------------------------------------------------------------- */
    if( bDuplicate )
        poRecord = poRecord->Clone();

/* -------------------------------------------------------------------- */
/*      Find the ATTP field.                                            */
/* -------------------------------------------------------------------- */
    poATTP = poRecord->FindField( "ATTP", 0 );
    if( poATTP == NULL )
    {
        poATTP = poRecord->FindField( "ATTS", 0 );
    }

    if( poATTP == NULL )
        return NULL;

/* -------------------------------------------------------------------- */
/*      Update the module ID if required.                               */
/* -------------------------------------------------------------------- */
    if( poModId != NULL )
    {
        DDFField        *poATPR = poRecord->FindField( "ATPR" );

        if( poATPR == NULL )
            poATPR = poRecord->FindField( "ATSC" );

        if( poATPR != NULL )
            poModId->Set( poATPR );
    }

/* -------------------------------------------------------------------- */
/*      return proper answer.                                           */
/* -------------------------------------------------------------------- */
    if( ppoRecord != NULL )
        *ppoRecord = poRecord;

    return poATTP;
}

/************************************************************************/
/*                         GetNextAttrRecord()                          */
/************************************************************************/

SDTSAttrRecord *SDTSAttrReader::GetNextAttrRecord()

{
    DDFRecord   *poRawRecord;
    DDFField    *poATTRField;
    SDTSModId   oModId;
    SDTSAttrRecord *poAttrRecord;

    poATTRField = GetNextRecord( &oModId, &poRawRecord, TRUE );

    if( poATTRField == NULL )
        return NULL;

    poAttrRecord = new SDTSAttrRecord();

    poAttrRecord->poWholeRecord = poRawRecord;
    poAttrRecord->poATTR = poATTRField;
    memcpy( &(poAttrRecord->oModId), &oModId, sizeof(SDTSModId) );

    return poAttrRecord;
}

