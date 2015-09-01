/******************************************************************************
 * $Id: ogrfeaturequery.cpp 27044 2014-03-16 23:41:27Z rouault $
 * 
 * Project:  OpenGIS Simple Features Reference Implementation
 * Purpose:  Implementation of simple SQL WHERE style attributes queries
 *           for OGRFeatures.  
 * Author:   Frank Warmerdam, warmerdam@pobox.com
 *
 ******************************************************************************
 * Copyright (c) 2001, Frank Warmerdam <warmerdam@pobox.com>
 * Copyright (c) 2008-2014, Even Rouault <even dot rouault at mines-paris dot org>
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

#include <assert.h>
#include "swq.h"
#include "ogr_feature.h"
#include "ogr_p.h"
#include "ogr_attrind.h"

CPL_CVSID("$Id: ogrfeaturequery.cpp 27044 2014-03-16 23:41:27Z rouault $");

/************************************************************************/
/*     Support for special attributes (feature query and selection)     */
/************************************************************************/

const char* SpecialFieldNames[SPECIAL_FIELD_COUNT] 
= {"FID", "OGR_GEOMETRY", "OGR_STYLE", "OGR_GEOM_WKT", "OGR_GEOM_AREA"};
const swq_field_type SpecialFieldTypes[SPECIAL_FIELD_COUNT] 
= {SWQ_INTEGER, SWQ_STRING, SWQ_STRING, SWQ_STRING, SWQ_FLOAT};

/************************************************************************/
/*                          OGRFeatureQuery()                           */
/************************************************************************/

OGRFeatureQuery::OGRFeatureQuery()

{
    poTargetDefn = NULL;
    pSWQExpr = NULL;
}

/************************************************************************/
/*                          ~OGRFeatureQuery()                          */
/************************************************************************/

OGRFeatureQuery::~OGRFeatureQuery()

{
    delete (swq_expr_node *) pSWQExpr;
}

/************************************************************************/
/*                                Parse                                 */
/************************************************************************/

OGRErr OGRFeatureQuery::Compile( OGRFeatureDefn *poDefn, 
                                 const char * pszExpression )

{
/* -------------------------------------------------------------------- */
/*      Clear any existing expression.                                  */
/* -------------------------------------------------------------------- */
    if( pSWQExpr != NULL )
    {
        delete (swq_expr_node *) pSWQExpr;
        pSWQExpr = NULL;
    }

/* -------------------------------------------------------------------- */
/*      Build list of fields.                                           */
/* -------------------------------------------------------------------- */
    char        **papszFieldNames;
    swq_field_type *paeFieldTypes;
    int         iField;
    int         nFieldCount = poDefn->GetFieldCount() + SPECIAL_FIELD_COUNT +
                              poDefn->GetGeomFieldCount();

    papszFieldNames = (char **) 
        CPLMalloc(sizeof(char *) * nFieldCount );
    paeFieldTypes = (swq_field_type *) 
        CPLMalloc(sizeof(swq_field_type) * nFieldCount );

    for( iField = 0; iField < poDefn->GetFieldCount(); iField++ )
    {
        OGRFieldDefn    *poField = poDefn->GetFieldDefn( iField );

        papszFieldNames[iField] = (char *) poField->GetNameRef();

        switch( poField->GetType() )
        {
          case OFTInteger:
            paeFieldTypes[iField] = SWQ_INTEGER;
            break;

          case OFTReal:
            paeFieldTypes[iField] = SWQ_FLOAT;
            break;

          case OFTString:
            paeFieldTypes[iField] = SWQ_STRING;
            break;

          case OFTDate:
          case OFTTime:
          case OFTDateTime:
            paeFieldTypes[iField] = SWQ_TIMESTAMP;
            break;

          default:
            paeFieldTypes[iField] = SWQ_OTHER;
            break;
        }
    }

    iField = 0;
    while (iField < SPECIAL_FIELD_COUNT)
    {
        papszFieldNames[poDefn->GetFieldCount() + iField] = (char *) SpecialFieldNames[iField];
        paeFieldTypes[poDefn->GetFieldCount() + iField] = SpecialFieldTypes[iField];
        ++iField;
    }

    for( iField = 0; iField < poDefn->GetGeomFieldCount(); iField++ )
    {
        OGRGeomFieldDefn    *poField = poDefn->GetGeomFieldDefn( iField );
        int iDstField = poDefn->GetFieldCount() + SPECIAL_FIELD_COUNT + iField;

        papszFieldNames[iDstField] = (char *) poField->GetNameRef();
        if( *papszFieldNames[iDstField] == '\0' )
            papszFieldNames[iDstField] = (char*) OGR_GEOMETRY_DEFAULT_NON_EMPTY_NAME;
        paeFieldTypes[iDstField] = SWQ_GEOMETRY;
    }

/* -------------------------------------------------------------------- */
/*      Try to parse.                                                   */
/* -------------------------------------------------------------------- */
    OGRErr      eErr = OGRERR_NONE;
    CPLErr      eCPLErr;

    poTargetDefn = poDefn;
    eCPLErr = swq_expr_compile( pszExpression, nFieldCount,
                                papszFieldNames, paeFieldTypes, 
                                (swq_expr_node **) &pSWQExpr );
    if( eCPLErr != CE_None )
    {
        eErr = OGRERR_CORRUPT_DATA;
        pSWQExpr = NULL;
    }

    CPLFree( papszFieldNames );
    CPLFree( paeFieldTypes );


    return eErr;
}

/************************************************************************/
/*                         OGRFeatureFetcher()                          */
/************************************************************************/

static swq_expr_node *OGRFeatureFetcher( swq_expr_node *op, void *pFeatureIn )

{
    OGRFeature *poFeature = (OGRFeature *) pFeatureIn;
    swq_expr_node *poRetNode = NULL;

    if( op->field_type == SWQ_GEOMETRY )
    {
        int iField = op->field_index - (poFeature->GetFieldCount() + SPECIAL_FIELD_COUNT);
        poRetNode = new swq_expr_node( poFeature->GetGeomFieldRef(iField) );
        return poRetNode;
    }

    switch( op->field_type )
    {
      case SWQ_INTEGER:
      case SWQ_BOOLEAN:
        poRetNode = new swq_expr_node( 
            poFeature->GetFieldAsInteger(op->field_index) );
        break;

      case SWQ_FLOAT:
        poRetNode = new swq_expr_node( 
            poFeature->GetFieldAsDouble(op->field_index) );
        break;

      default:
        poRetNode = new swq_expr_node( 
            poFeature->GetFieldAsString(op->field_index) );
        break;
    }

    poRetNode->is_null = !(poFeature->IsFieldSet(op->field_index));

    return poRetNode;
}

/************************************************************************/
/*                              Evaluate()                              */
/************************************************************************/

int OGRFeatureQuery::Evaluate( OGRFeature *poFeature )

{
    if( pSWQExpr == NULL )
        return FALSE;

    swq_expr_node *poResult;

    poResult = ((swq_expr_node *) pSWQExpr)->Evaluate( OGRFeatureFetcher,
                                                       (void *) poFeature );

    if( poResult == NULL )
        return FALSE;

    CPLAssert( poResult->field_type == SWQ_BOOLEAN );

    int bLogicalResult = poResult->int_value;

    delete poResult;

    return bLogicalResult;
}

/************************************************************************/
/*                            CanUseIndex()                             */
/************************************************************************/

int OGRFeatureQuery::CanUseIndex( OGRLayer *poLayer )
{
    swq_expr_node *psExpr = (swq_expr_node *) pSWQExpr;

/* -------------------------------------------------------------------- */
/*      Do we have an index on the targetted layer?                     */
/* -------------------------------------------------------------------- */
    if ( poLayer->GetIndex() == FALSE )
        return FALSE;

    return CanUseIndex( psExpr, poLayer );
}

int OGRFeatureQuery::CanUseIndex( swq_expr_node *psExpr,
                                  OGRLayer *poLayer )
{
    OGRAttrIndex *poIndex;

/* -------------------------------------------------------------------- */
/*      Does the expression meet our requirements?                      */
/* -------------------------------------------------------------------- */
    if( psExpr == NULL ||
        psExpr->eNodeType != SNT_OPERATION )
        return FALSE;

    if ((psExpr->nOperation == SWQ_OR || psExpr->nOperation == SWQ_AND) &&
         psExpr->nSubExprCount == 2)
    {
        return CanUseIndex( psExpr->papoSubExpr[0], poLayer ) &&
               CanUseIndex( psExpr->papoSubExpr[1], poLayer );
    }

    if( !(psExpr->nOperation == SWQ_EQ || psExpr->nOperation == SWQ_IN)
        || psExpr->nSubExprCount < 2 )
        return FALSE;

    swq_expr_node *poColumn = psExpr->papoSubExpr[0];
    swq_expr_node *poValue = psExpr->papoSubExpr[1];
    
    if( poColumn->eNodeType != SNT_COLUMN
        || poValue->eNodeType != SNT_CONSTANT )
        return FALSE;

    poIndex = poLayer->GetIndex()->GetFieldIndex( poColumn->field_index );
    if( poIndex == NULL )
        return FALSE;

/* -------------------------------------------------------------------- */
/*      OK, we have an index                                            */
/* -------------------------------------------------------------------- */
    return TRUE;
}

/************************************************************************/
/*                       EvaluateAgainstIndices()                       */
/*                                                                      */
/*      Attempt to return a list of FIDs matching the given             */
/*      attribute query conditions utilizing attribute indices.         */
/*      Returns NULL if the result cannot be computed from the          */
/*      available indices, or an "OGRNullFID" terminated list of        */
/*      FIDs if it can.                                                 */
/*                                                                      */
/*      For now we only support equality tests on a single indexed      */
/*      attribute field.  Eventually we should make this support        */
/*      multi-part queries with ranges.                                 */
/************************************************************************/

static int CompareLong(const void *a, const void *b)
{
	return (*(const long *)a) - (*(const long *)b);
}

long *OGRFeatureQuery::EvaluateAgainstIndices( OGRLayer *poLayer, 
                                               OGRErr *peErr )

{
    swq_expr_node *psExpr = (swq_expr_node *) pSWQExpr;

    if( peErr != NULL )
        *peErr = OGRERR_NONE;

/* -------------------------------------------------------------------- */
/*      Do we have an index on the targetted layer?                     */
/* -------------------------------------------------------------------- */
    if ( poLayer->GetIndex() == NULL )
        return NULL;

    int nFIDCount = 0;
    return EvaluateAgainstIndices(psExpr, poLayer, nFIDCount);
}

/* The input arrays must be sorted ! */
static
long* OGRORLongArray(long panFIDList1[], int nFIDCount1,
                     long panFIDList2[], int nFIDCount2, int& nFIDCount)
{
    int nMaxCount = nFIDCount1 + nFIDCount2;
    long* panFIDList = (long*) CPLMalloc((nMaxCount+1) * sizeof(long));
    nFIDCount = 0;

    int i1 = 0, i2 =0;
    for(;i1<nFIDCount1 || i2<nFIDCount2;)
    {
        if (i1 < nFIDCount1 && i2 < nFIDCount2)
        {
            long nVal1 = panFIDList1[i1];
            long nVal2 = panFIDList2[i2];
            if (nVal1 < nVal2)
            {
                if (i1+1 < nFIDCount1 && panFIDList1[i1+1] <= nVal2)
                {
                    panFIDList[nFIDCount ++] = nVal1;
                    i1 ++;
                }
                else
                {
                    panFIDList[nFIDCount ++] = nVal1;
                    panFIDList[nFIDCount ++] = nVal2;
                    i1 ++;
                    i2 ++;
                }
            }
            else if (nVal1 == nVal2)
            {
                panFIDList[nFIDCount ++] = nVal1;
                i1 ++;
                i2 ++;
            }
            else
            {
                if (i2+1 < nFIDCount2 && panFIDList2[i2+1] <= nVal1)
                {
                    panFIDList[nFIDCount ++] = nVal2;
                    i2 ++;
                }
                else
                {
                    panFIDList[nFIDCount ++] = nVal2;
                    panFIDList[nFIDCount ++] = nVal1;
                    i1 ++;
                    i2 ++;
                }
            }
        }
        else if (i1 < nFIDCount1)
        {
            long nVal1 = panFIDList1[i1];
            panFIDList[nFIDCount ++] = nVal1;
            i1 ++;
        }
        else if (i2 < nFIDCount2)
        {
            long nVal2 = panFIDList2[i2];
            panFIDList[nFIDCount ++] = nVal2;
            i2 ++;
        }
    }

    panFIDList[nFIDCount] = OGRNullFID;

    return panFIDList;
}

/* The input arrays must be sorted ! */
static
long* OGRANDLongArray(long panFIDList1[], int nFIDCount1,
                      long panFIDList2[], int nFIDCount2, int& nFIDCount)
{
    int nMaxCount = MAX(nFIDCount1, nFIDCount2);
    long* panFIDList = (long*) CPLMalloc((nMaxCount+1) * sizeof(long));
    nFIDCount = 0;

    int i1 = 0, i2 =0;
    for(;i1<nFIDCount1 && i2<nFIDCount2;)
    {
        long nVal1 = panFIDList1[i1];
        long nVal2 = panFIDList2[i2];
        if (nVal1 < nVal2)
        {
            if (i1+1 < nFIDCount1 && panFIDList1[i1+1] <= nVal2)
            {
                i1 ++;
            }
            else
            {
                i1 ++;
                i2 ++;
            }
        }
        else if (nVal1 == nVal2)
        {
            panFIDList[nFIDCount ++] = nVal1;
            i1 ++;
            i2 ++;
        }
        else
        {
            if (i2+1 < nFIDCount2 && panFIDList2[i2+1] <= nVal1)
            {
                i2 ++;
            }
            else
            {
                i1 ++;
                i2 ++;
            }
        }
    }

    panFIDList[nFIDCount] = OGRNullFID;

    return panFIDList;
}

long *OGRFeatureQuery::EvaluateAgainstIndices( swq_expr_node *psExpr,
                                               OGRLayer *poLayer,
                                               int& nFIDCount )
{
    OGRAttrIndex *poIndex;

/* -------------------------------------------------------------------- */
/*      Does the expression meet our requirements?                      */
/* -------------------------------------------------------------------- */
    if( psExpr == NULL ||
        psExpr->eNodeType != SNT_OPERATION )
        return NULL;

    if ((psExpr->nOperation == SWQ_OR || psExpr->nOperation == SWQ_AND) &&
         psExpr->nSubExprCount == 2)
    {
        int nFIDCount1 = 0, nFIDCount2 = 0;
        long* panFIDList1 = EvaluateAgainstIndices( psExpr->papoSubExpr[0], poLayer, nFIDCount1 );
        long* panFIDList2 = panFIDList1 == NULL ? NULL :
                            EvaluateAgainstIndices( psExpr->papoSubExpr[1], poLayer, nFIDCount2 );
        long* panFIDList = NULL;
        if (panFIDList1 != NULL && panFIDList2 != NULL)
        {
            if (psExpr->nOperation == SWQ_OR )
                panFIDList = OGRORLongArray(panFIDList1, nFIDCount1,
                                            panFIDList2, nFIDCount2, nFIDCount);
            else if (psExpr->nOperation == SWQ_AND )
                panFIDList = OGRANDLongArray(panFIDList1, nFIDCount1,
                                            panFIDList2, nFIDCount2, nFIDCount);

        }
        CPLFree(panFIDList1);
        CPLFree(panFIDList2);
        return panFIDList;
    }

    if( !(psExpr->nOperation == SWQ_EQ || psExpr->nOperation == SWQ_IN)
        || psExpr->nSubExprCount < 2 )
        return NULL;

    swq_expr_node *poColumn = psExpr->papoSubExpr[0];
    swq_expr_node *poValue = psExpr->papoSubExpr[1];
    
    if( poColumn->eNodeType != SNT_COLUMN
        || poValue->eNodeType != SNT_CONSTANT )
        return NULL;

    poIndex = poLayer->GetIndex()->GetFieldIndex( poColumn->field_index );
    if( poIndex == NULL )
        return NULL;

/* -------------------------------------------------------------------- */
/*      OK, we have an index, now we need to query it.                  */
/* -------------------------------------------------------------------- */
    OGRField sValue;
    OGRFieldDefn *poFieldDefn;

    poFieldDefn = poLayer->GetLayerDefn()->GetFieldDefn(poColumn->field_index);

/* -------------------------------------------------------------------- */
/*      Handle the case of an IN operation.                             */
/* -------------------------------------------------------------------- */
    if (psExpr->nOperation == SWQ_IN)
    {
        int nLength;
        long *panFIDs = NULL;
        int iIN;

        for( iIN = 1; iIN < psExpr->nSubExprCount; iIN++ )
        {
            switch( poFieldDefn->GetType() )
            {
              case OFTInteger:
                if (psExpr->papoSubExpr[iIN]->field_type == SWQ_FLOAT)
                    sValue.Integer = (int) psExpr->papoSubExpr[iIN]->float_value;
                else
                    sValue.Integer = psExpr->papoSubExpr[iIN]->int_value;
                break;

              case OFTReal:
                sValue.Real = psExpr->papoSubExpr[iIN]->float_value;
                break;

              case OFTString:
                sValue.String = psExpr->papoSubExpr[iIN]->string_value;
                break;

              default:
                CPLAssert( FALSE );
                return NULL;
            }

            panFIDs = poIndex->GetAllMatches( &sValue, panFIDs, &nFIDCount, &nLength );
        }

        if (nFIDCount > 1)
        {
            /* the returned FIDs are expected to be in sorted order */
            qsort(panFIDs, nFIDCount, sizeof(long), CompareLong);
        }
        return panFIDs;
    }

/* -------------------------------------------------------------------- */
/*      Handle equality test.                                           */
/* -------------------------------------------------------------------- */
    switch( poFieldDefn->GetType() )
    {
      case OFTInteger:
        if (poValue->field_type == SWQ_FLOAT)
            sValue.Integer = (int) poValue->float_value;
        else
            sValue.Integer = poValue->int_value;
        break;
        
      case OFTReal:
        sValue.Real = poValue->float_value;
        break;
        
      case OFTString:
        sValue.String = poValue->string_value;
        break;

      default:
        CPLAssert( FALSE );
        return NULL;
    }

    int nLength = 0;
    long *panFIDs = poIndex->GetAllMatches( &sValue, NULL, &nFIDCount, &nLength );
    if (nFIDCount > 1)
    {
        /* the returned FIDs are expected to be in sorted order */
        qsort(panFIDs, nFIDCount, sizeof(long), CompareLong);
    }
    return panFIDs;
}

/************************************************************************/
/*                         OGRFieldCollector()                          */
/*                                                                      */
/*      Helper function for recursing through tree to satisfy           */
/*      GetUsedFields().                                                */
/************************************************************************/

char **OGRFeatureQuery::FieldCollector( void *pBareOp, 
                                        char **papszList )

{
    swq_expr_node *op = (swq_expr_node *) pBareOp;

/* -------------------------------------------------------------------- */
/*      References to tables other than the primarily are currently     */
/*      unsupported. Error out.                                         */
/* -------------------------------------------------------------------- */
    if( op->eNodeType == SNT_COLUMN )
    {
        if( op->table_index != 0 )
        {
            CSLDestroy( papszList );
            return NULL;
        }

/* -------------------------------------------------------------------- */
/*      Add the field name into our list if it is not already there.    */
/* -------------------------------------------------------------------- */
        const char *pszFieldName;

        if( op->field_index >= poTargetDefn->GetFieldCount()
            && op->field_index < poTargetDefn->GetFieldCount() + SPECIAL_FIELD_COUNT)
            pszFieldName = SpecialFieldNames[op->field_index - poTargetDefn->GetFieldCount()];
        else if( op->field_index >= 0
                 && op->field_index < poTargetDefn->GetFieldCount() )
            pszFieldName = 
                poTargetDefn->GetFieldDefn(op->field_index)->GetNameRef();
        else
        {
            CSLDestroy( papszList );
            return NULL;
        }
        
        if( CSLFindString( papszList, pszFieldName ) == -1 )
            papszList = CSLAddString( papszList, pszFieldName );
    }

/* -------------------------------------------------------------------- */
/*      Add in fields from subexpressions.                              */
/* -------------------------------------------------------------------- */
    if( op->eNodeType == SNT_OPERATION )
    {
        for( int iSubExpr = 0; iSubExpr < op->nSubExprCount; iSubExpr++ )
        {
            papszList = FieldCollector( op->papoSubExpr[iSubExpr], papszList );
        }
    }

    return papszList;
}

/************************************************************************/
/*                           GetUsedFields()                            */
/************************************************************************/

/**
 * Returns lists of fields in expression.
 *
 * All attribute fields are used in the expression of this feature
 * query are returned as a StringList of field names.  This function would
 * primarily be used within drivers to recognise special case conditions
 * depending only on attribute fields that can be very efficiently 
 * fetched. 
 *
 * NOTE: If any fields in the expression are from tables other than the
 * primary table then NULL is returned indicating an error.  In succesful
 * use, no non-empty expression should return an empty list.
 *
 * @return list of field names.  Free list with CSLDestroy() when no longer
 * required.
 */

char **OGRFeatureQuery::GetUsedFields( )

{
    if( pSWQExpr == NULL )
        return NULL;

    
    return FieldCollector( pSWQExpr, NULL );
}



