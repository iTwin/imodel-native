/******************************************************************************
 *
 * Component: OGR SQL Engine
 * Purpose: Implementation of SWQGeneralEvaluator and SWQGeneralChecker 
 *          functions used to represent functions during evaluation and
 *          parsing.
 * Author: Frank Warmerdam <warmerdam@pobox.com>
 * 
 ******************************************************************************
 * Copyright (C) 2010 Frank Warmerdam <warmerdam@pobox.com>
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

#include "cpl_conv.h"
#include "swq.h"

/************************************************************************/
/*                           swq_test_like()                            */
/*                                                                      */
/*      Does input match pattern?                                       */
/************************************************************************/

int swq_test_like( const char *input, const char *pattern )

{
    if( input == NULL || pattern == NULL )
        return 0;

    while( *input != '\0' )
    {
        if( *pattern == '\0' )
            return 0;

        else if( *pattern == '_' )
        {
            input++;
            pattern++;
        }
        else if( *pattern == '%' )
        {
            int   eat;

            if( pattern[1] == '\0' )
                return 1;

            /* try eating varying amounts of the input till we get a positive*/
            for( eat = 0; input[eat] != '\0'; eat++ )
            {
                if( swq_test_like(input+eat,pattern+1) )
                    return 1;
            }

            return 0;
        }
        else
        {
            if( tolower(*pattern) != tolower(*input) )
                return 0;
            else
            {
                input++;
                pattern++;
            }
        }
    }

    if( *pattern != '\0' && strcmp(pattern,"%") != 0 )
        return 0;
    else
        return 1;
}

/************************************************************************/
/*                        SWQGeneralEvaluator()                         */
/************************************************************************/

swq_expr_node *SWQGeneralEvaluator( swq_expr_node *node,
                                    swq_expr_node **sub_node_values )

{
    swq_expr_node *poRet = NULL;

/* -------------------------------------------------------------------- */
/*      Floating point operations.                                      */
/* -------------------------------------------------------------------- */
    if( sub_node_values[0]->field_type == SWQ_FLOAT 
        || (node->nSubExprCount > 1 
            && sub_node_values[1]->field_type == SWQ_FLOAT) )
            
    {
        poRet = new swq_expr_node(0);
        poRet->field_type = node->field_type;

        if( sub_node_values[0]->field_type == SWQ_INTEGER )
            sub_node_values[0]->float_value = sub_node_values[0]->int_value;
        if( node->nSubExprCount > 1 &&
            sub_node_values[1]->field_type == SWQ_INTEGER )
            sub_node_values[1]->float_value = sub_node_values[1]->int_value;

        switch( (swq_op) node->nOperation )
        {
          case SWQ_EQ:
            poRet->int_value = sub_node_values[0]->float_value 
                == sub_node_values[1]->float_value;
            break;

          case SWQ_NE:
            poRet->int_value = sub_node_values[0]->float_value 
                != sub_node_values[1]->float_value;
            break;

          case SWQ_GT:
            poRet->int_value = sub_node_values[0]->float_value 
                > sub_node_values[1]->float_value;
            break;

          case SWQ_LT:
            poRet->int_value = sub_node_values[0]->float_value 
                < sub_node_values[1]->float_value;
            break;

          case SWQ_GE:
            poRet->int_value = sub_node_values[0]->float_value 
                >= sub_node_values[1]->float_value;
            break;

          case SWQ_LE:
            poRet->int_value = sub_node_values[0]->float_value 
                <= sub_node_values[1]->float_value;
            break;

          case SWQ_IN:
          {
              int i;
              poRet->int_value = 0;
              for( i = 1; i < node->nSubExprCount; i++ )
              {
                  if( sub_node_values[0]->float_value 
                      == sub_node_values[i]->float_value )
                  {
                      poRet->int_value = 1;
                      break;
                  }
              }
          }
          break;

          case SWQ_BETWEEN:
            poRet->int_value = sub_node_values[0]->float_value
                                >= sub_node_values[1]->float_value &&
                               sub_node_values[0]->float_value
                                <= sub_node_values[2]->float_value;
            break;

          case SWQ_ISNULL:
            poRet->int_value = sub_node_values[0]->is_null;
            break;

          case SWQ_ADD:
            poRet->float_value = sub_node_values[0]->float_value 
                + sub_node_values[1]->float_value;
            break;
            
          case SWQ_SUBTRACT:
            poRet->float_value = sub_node_values[0]->float_value 
                - sub_node_values[1]->float_value;
            break;
            
          case SWQ_MULTIPLY:
            poRet->float_value = sub_node_values[0]->float_value 
                * sub_node_values[1]->float_value;
            break;
            
          case SWQ_DIVIDE:
            if( sub_node_values[1]->float_value == 0 )
                poRet->float_value = INT_MAX;
            else
                poRet->float_value = sub_node_values[0]->float_value 
                    / sub_node_values[1]->float_value;
            break;
            
          case SWQ_MODULUS:
          {
            int nRight = (int) sub_node_values[1]->float_value;
            poRet->field_type = SWQ_INTEGER;
            if (nRight == 0)
                poRet->int_value = INT_MAX;
            else
                poRet->int_value = ((int) sub_node_values[0]->float_value)
                    % nRight;
            break;
          }

          default:
            CPLAssert( FALSE );
            delete poRet;
            poRet = NULL;
            break;
        }
    }
/* -------------------------------------------------------------------- */
/*      integer/boolean operations.                                     */
/* -------------------------------------------------------------------- */
    else if( sub_node_values[0]->field_type == SWQ_INTEGER
        || sub_node_values[0]->field_type == SWQ_BOOLEAN )
    {
        poRet = new swq_expr_node(0);
        poRet->field_type = node->field_type;

        switch( (swq_op) node->nOperation )
        {
          case SWQ_AND:
            poRet->int_value = sub_node_values[0]->int_value 
                && sub_node_values[1]->int_value;
            break;
            
          case SWQ_OR:
            poRet->int_value = sub_node_values[0]->int_value 
                || sub_node_values[1]->int_value;
            break;
            
          case SWQ_NOT:
            poRet->int_value = !sub_node_values[0]->int_value;
            break;
            
          case SWQ_EQ:
            poRet->int_value = sub_node_values[0]->int_value 
                == sub_node_values[1]->int_value;
            break;

          case SWQ_NE:
            poRet->int_value = sub_node_values[0]->int_value 
                != sub_node_values[1]->int_value;
            break;

          case SWQ_GT:
            poRet->int_value = sub_node_values[0]->int_value 
                > sub_node_values[1]->int_value;
            break;

          case SWQ_LT:
            poRet->int_value = sub_node_values[0]->int_value 
                < sub_node_values[1]->int_value;
            break;

          case SWQ_GE:
            poRet->int_value = sub_node_values[0]->int_value 
                >= sub_node_values[1]->int_value;
            break;

          case SWQ_LE:
            poRet->int_value = sub_node_values[0]->int_value 
                <= sub_node_values[1]->int_value;
            break;

          case SWQ_IN:
          {
              int i;
              poRet->int_value = 0;
              for( i = 1; i < node->nSubExprCount; i++ )
              {
                  if( sub_node_values[0]->int_value 
                      == sub_node_values[i]->int_value )
                  {
                      poRet->int_value = 1;
                      break;
                  }
              }
          }
          break;

          case SWQ_BETWEEN:
            poRet->int_value = sub_node_values[0]->int_value
                                >= sub_node_values[1]->int_value &&
                               sub_node_values[0]->int_value
                                <= sub_node_values[2]->int_value;
            break;

          case SWQ_ISNULL:
            poRet->int_value = sub_node_values[0]->is_null;
            break;

          case SWQ_ADD:
            poRet->int_value = sub_node_values[0]->int_value 
                + sub_node_values[1]->int_value;
            break;
            
          case SWQ_SUBTRACT:
            poRet->int_value = sub_node_values[0]->int_value 
                - sub_node_values[1]->int_value;
            break;
            
          case SWQ_MULTIPLY:
            poRet->int_value = sub_node_values[0]->int_value 
                * sub_node_values[1]->int_value;
            break;
            
          case SWQ_DIVIDE:
            if( sub_node_values[1]->int_value == 0 )
                poRet->int_value = INT_MAX;
            else
                poRet->int_value = sub_node_values[0]->int_value 
                    / sub_node_values[1]->int_value;
            break;
            
          case SWQ_MODULUS:
            if( sub_node_values[1]->int_value == 0 )
                poRet->int_value = INT_MAX;
            else
                poRet->int_value = sub_node_values[0]->int_value
                    % sub_node_values[1]->int_value;
            break;
            
          default:
            CPLAssert( FALSE );
            delete poRet;
            poRet = NULL;
            break;
        }
    }

/* -------------------------------------------------------------------- */
/*      String operations.                                              */
/* -------------------------------------------------------------------- */
    else
    {
        poRet = new swq_expr_node(0);
        poRet->field_type = node->field_type;

        switch( (swq_op) node->nOperation )
        {
          case SWQ_EQ:
            poRet->int_value = 
                strcasecmp(sub_node_values[0]->string_value,
                           sub_node_values[1]->string_value) == 0;
            break;

          case SWQ_NE:
            poRet->int_value = 
                strcasecmp(sub_node_values[0]->string_value,
                           sub_node_values[1]->string_value) != 0;
            break;

          case SWQ_GT:
            poRet->int_value = 
                strcasecmp(sub_node_values[0]->string_value,
                           sub_node_values[1]->string_value) > 0;
            break;

          case SWQ_LT:
            poRet->int_value = 
                strcasecmp(sub_node_values[0]->string_value,
                           sub_node_values[1]->string_value) < 0;
            break;

          case SWQ_GE:
            poRet->int_value = 
                strcasecmp(sub_node_values[0]->string_value,
                           sub_node_values[1]->string_value) >= 0;
            break;

          case SWQ_LE:
            poRet->int_value = 
                strcasecmp(sub_node_values[0]->string_value,
                           sub_node_values[1]->string_value) <= 0;
            break;

          case SWQ_IN:
          {
              int i;
              poRet->int_value = 0;
              for( i = 1; i < node->nSubExprCount; i++ )
              {
                  if( strcasecmp(sub_node_values[0]->string_value,
                                 sub_node_values[i]->string_value) == 0 )
                  {
                      poRet->int_value = 1;
                      break;
                  }
              }
          }
          break;

          case SWQ_BETWEEN:
            poRet->int_value =
                strcasecmp(sub_node_values[0]->string_value,
                           sub_node_values[1]->string_value) >= 0 &&
                strcasecmp(sub_node_values[0]->string_value,
                           sub_node_values[2]->string_value) <= 0;
            break;

          case SWQ_LIKE:
            poRet->int_value = swq_test_like(sub_node_values[0]->string_value,
                                             sub_node_values[1]->string_value);
            break;

          case SWQ_ISNULL:
            poRet->int_value = sub_node_values[0]->is_null;
            break;

          case SWQ_CONCAT:
          case SWQ_ADD:
          {
              CPLString osResult = sub_node_values[0]->string_value;
              int i;

              for( i = 1; i < node->nSubExprCount; i++ )
                  osResult += sub_node_values[i]->string_value;
              
              poRet->string_value = CPLStrdup(osResult);
              break;
          }
            
          case SWQ_SUBSTR:
          {
              int nOffset, nSize;
              const char *pszSrcStr = sub_node_values[0]->string_value;

              if( sub_node_values[1]->field_type == SWQ_INTEGER )
                  nOffset = sub_node_values[1]->int_value;
              else if( sub_node_values[1]->field_type == SWQ_FLOAT )
                  nOffset = (int) sub_node_values[1]->float_value; 
              else
                  nOffset = 0;

              if( node->nSubExprCount < 3 )
                  nSize = 100000;
              else if( sub_node_values[2]->field_type == SWQ_INTEGER )
                  nSize = sub_node_values[2]->int_value;
              else if( sub_node_values[2]->field_type == SWQ_FLOAT )
                  nSize = (int) sub_node_values[2]->float_value; 
              else
                  nSize = 0;

              int nSrcStrLen = (int)strlen(pszSrcStr);
              if( nOffset < 0 || nSize < 0 || nOffset > nSrcStrLen )
              {
                  nOffset = 0;
                  nSize = 0;
              }
              else if( nOffset + nSize > nSrcStrLen )
                  nSize = nSrcStrLen - nOffset;

              CPLString osResult = pszSrcStr + nOffset;
              if( (int)osResult.size() > nSize )
                  osResult.resize( nSize );
              
              poRet->string_value = CPLStrdup(osResult);
              break;
          }

          default:
            CPLAssert( FALSE );
            delete poRet;
            poRet = NULL;
            break;
        }
    }

    return poRet;
}

/************************************************************************/
/*                    SWQAutoPromoteIntegerToFloat()                    */
/************************************************************************/

static void SWQAutoPromoteIntegerToFloat( swq_expr_node *poNode )

{
    if( poNode->nSubExprCount < 2 )
        return;

    swq_field_type eArgType = poNode->papoSubExpr[0]->field_type;
    int i;

    // We allow mixes of integer and float, and string and dates.
    // When encountered, we promote integers to floats, and strings to
    // dates.  We do that now.
    for( i = 1; i < poNode->nSubExprCount; i++ )
    {
        if( eArgType == SWQ_INTEGER
            && poNode->papoSubExpr[i]->field_type == SWQ_FLOAT )
            eArgType = SWQ_FLOAT;
    }
    
    for( i = 0; i < poNode->nSubExprCount; i++ )
    {
        swq_expr_node *poSubNode = poNode->papoSubExpr[i];

        if( eArgType == SWQ_FLOAT
            && poSubNode->field_type == SWQ_INTEGER )
        {
            if( poSubNode->eNodeType == SNT_CONSTANT )
            {
                poSubNode->float_value = poSubNode->int_value;
                poSubNode->field_type = SWQ_FLOAT;
            }
        }
    }
}

/************************************************************************/
/*                    SWQAutoPromoteStringToDateTime()                  */
/************************************************************************/

static void SWQAutoPromoteStringToDateTime( swq_expr_node *poNode )

{
    if( poNode->nSubExprCount < 2 )
        return;

    swq_field_type eArgType = poNode->papoSubExpr[0]->field_type;
    int i;

    // We allow mixes of integer and float, and string and dates.
    // When encountered, we promote integers to floats, and strings to
    // dates.  We do that now.
    for( i = 1; i < poNode->nSubExprCount; i++ )
    {
        swq_expr_node *poSubNode = poNode->papoSubExpr[i];

        if( eArgType == SWQ_STRING
            && (poSubNode->field_type == SWQ_DATE
                || poSubNode->field_type == SWQ_TIME
                || poSubNode->field_type == SWQ_TIMESTAMP) )
            eArgType = SWQ_TIMESTAMP;
    }
    
    for( i = 0; i < poNode->nSubExprCount; i++ )
    {
        swq_expr_node *poSubNode = poNode->papoSubExpr[i];

        if( eArgType == SWQ_TIMESTAMP
            && (poSubNode->field_type == SWQ_STRING 
                || poSubNode->field_type == SWQ_DATE
                || poSubNode->field_type == SWQ_TIME) )
        {
            if( poSubNode->eNodeType == SNT_CONSTANT )
            {
                poSubNode->field_type = SWQ_TIMESTAMP;
            }
        }
    }
}

/************************************************************************/
/*                         SWQGeneralChecker()                          */
/*                                                                      */
/*      Check the general purpose functions have appropriate types,     */
/*      and count and indicate the function return type under the       */
/*      circumstances.                                                  */
/************************************************************************/

swq_field_type SWQGeneralChecker( swq_expr_node *poNode )

{									
    swq_field_type eRetType = SWQ_ERROR;
    swq_field_type eArgType = SWQ_OTHER;
    int nArgCount = -1;

    switch( (swq_op) poNode->nOperation )
    {
      case SWQ_AND:
      case SWQ_OR:
      case SWQ_NOT:
        eRetType = SWQ_BOOLEAN;
        break;

      case SWQ_EQ:
      case SWQ_NE:
      case SWQ_GT:
      case SWQ_LT:
      case SWQ_GE:
      case SWQ_LE:
      case SWQ_IN:
      case SWQ_BETWEEN:
        eRetType = SWQ_BOOLEAN;
        SWQAutoPromoteIntegerToFloat( poNode );
        SWQAutoPromoteStringToDateTime( poNode );
        eArgType = poNode->papoSubExpr[0]->field_type;
        break;

      case SWQ_ISNULL:
        eRetType = SWQ_BOOLEAN;
        break;

      case SWQ_LIKE:
        eRetType = SWQ_BOOLEAN;
        eArgType = SWQ_STRING;
        break;

      case SWQ_MODULUS:
        eRetType = SWQ_INTEGER;
        eArgType = SWQ_INTEGER;
        break;

      case SWQ_ADD:
        SWQAutoPromoteIntegerToFloat( poNode );
        if( poNode->papoSubExpr[0]->field_type == SWQ_STRING )
            eRetType = eArgType = SWQ_STRING;
        else if( poNode->papoSubExpr[0]->field_type == SWQ_FLOAT )
            eRetType = eArgType = SWQ_FLOAT;
        else
            eRetType = eArgType = SWQ_INTEGER;
        break;

      case SWQ_SUBTRACT:
      case SWQ_MULTIPLY:
      case SWQ_DIVIDE:
        SWQAutoPromoteIntegerToFloat( poNode );
        if( poNode->papoSubExpr[0]->field_type == SWQ_FLOAT )
            eRetType = eArgType = SWQ_FLOAT;
        else
            eRetType = eArgType = SWQ_INTEGER;
        break;

      case SWQ_CONCAT:
        eRetType = SWQ_STRING;
        eArgType = SWQ_STRING;
        break;

      case SWQ_SUBSTR:
        eRetType = SWQ_STRING;
        if( poNode->nSubExprCount > 3 || poNode->nSubExprCount < 2 )
        {
            CPLError( CE_Failure, CPLE_AppDefined, 
                      "Expected 2 or 3 arguments to SUBSTR(), but got %d.",
                      poNode->nSubExprCount );
            return SWQ_ERROR;
        }
        if( poNode->papoSubExpr[0]->field_type != SWQ_STRING 
            || poNode->papoSubExpr[1]->field_type != SWQ_INTEGER
            || (poNode->nSubExprCount > 2 
                && poNode->papoSubExpr[2]->field_type != SWQ_INTEGER) )
        {
            CPLError( CE_Failure, CPLE_AppDefined, 
                      "Wrong argument type for SUBSTR(), expected SUBSTR(string,int,int) or SUBSTR(string,int)." );
            return SWQ_ERROR;
        }
        break;

      default:
      {
          const swq_operation *poOp = 
              swq_op_registrar::GetOperator((swq_op)poNode->nOperation);
          
          CPLError( CE_Failure, CPLE_AppDefined,
                    "SWQGeneralChecker() called on unsupported operation %s.",
                    poOp->osName.c_str());
          return SWQ_ERROR;
      }
    }
/* -------------------------------------------------------------------- */
/*      Check argument types.                                           */
/* -------------------------------------------------------------------- */
    if( eArgType != SWQ_OTHER )
    {
        int i;

        if( eArgType == SWQ_INTEGER )
            eArgType = SWQ_FLOAT;

        for( i = 0; i < poNode->nSubExprCount; i++ )
        {
            swq_field_type eThisArgType = poNode->papoSubExpr[i]->field_type;
            if( eThisArgType == SWQ_INTEGER )
                eThisArgType = SWQ_FLOAT;

            if( eArgType != eThisArgType )
            {
                const swq_operation *poOp = 
                    swq_op_registrar::GetOperator((swq_op)poNode->nOperation);
                
                CPLError( CE_Failure, CPLE_AppDefined, 
                          "Type mismatch or improper type of arguments to %s operator.",
                          poOp->osName.c_str() );
                return SWQ_ERROR;
            }
        }
    }

/* -------------------------------------------------------------------- */
/*      Validate the arg count if requested.                            */
/* -------------------------------------------------------------------- */
    if( nArgCount != -1 
        && nArgCount != poNode->nSubExprCount )
    {
        const swq_operation *poOp = 
            swq_op_registrar::GetOperator((swq_op)poNode->nOperation);
                
        CPLError( CE_Failure, CPLE_AppDefined, 
                  "Expected %d arguments to %s, but got %d arguments.",
                  nArgCount,
                  poOp->osName.c_str(),
                  poNode->nSubExprCount );
        return SWQ_ERROR;
    }

    return eRetType;
}

/************************************************************************/
/*                          SWQCastEvaluator()                          */
/************************************************************************/

swq_expr_node *SWQCastEvaluator( swq_expr_node *node,
                                 swq_expr_node **sub_node_values )

{
    swq_expr_node *poRetNode = NULL;
    swq_expr_node *poSrcNode = sub_node_values[0];

    switch( node->field_type )
    {
        case SWQ_INTEGER:
        {
            poRetNode = new swq_expr_node( 0 );

            switch( poSrcNode->field_type )
            {
                case SWQ_INTEGER:
                case SWQ_BOOLEAN:
                    poRetNode->int_value = poSrcNode->int_value;
                    break;

                case SWQ_FLOAT:
                    poRetNode->int_value = (int) poSrcNode->float_value;
                    break;

                default:
                    poRetNode->int_value = atoi(poSrcNode->string_value);
                    break;
            }
        }
        break;

        case SWQ_FLOAT:
        {
            poRetNode = new swq_expr_node( 0.0 );

            switch( poSrcNode->field_type )
            {
                case SWQ_INTEGER:
                case SWQ_BOOLEAN:
                    poRetNode->float_value = poSrcNode->int_value;
                    break;

                case SWQ_FLOAT:
                    poRetNode->float_value = poSrcNode->float_value;
                    break;

                default:
                    poRetNode->float_value = atof(poSrcNode->string_value);
                    break;
            }
        }
        break;

        // everything else is a string.
        default:
        {
            CPLString osRet;

            switch( poSrcNode->field_type )
            {
                case SWQ_INTEGER:
                case SWQ_BOOLEAN:
                    osRet.Printf( "%d", poSrcNode->int_value );
                    break;

                case SWQ_FLOAT:
                    osRet.Printf( "%.15g", poSrcNode->float_value );
                    break;

                default:
                    osRet = poSrcNode->string_value;
                    break;
            }
         
            if( node->nSubExprCount > 2 )
            {
                int nWidth;

                nWidth = sub_node_values[2]->int_value;
                if( nWidth > 0 && (int) strlen(osRet) > nWidth )
                    osRet.resize(nWidth);
            }

            poRetNode = new swq_expr_node( osRet.c_str() );
        }
    }

    return poRetNode;
}

/************************************************************************/
/*                           SWQCastChecker()                           */
/************************************************************************/

swq_field_type SWQCastChecker( swq_expr_node *poNode )

{									
    swq_field_type eType = SWQ_ERROR;
    const char *pszTypeName = poNode->papoSubExpr[1]->string_value;

    if( EQUAL(pszTypeName,"character") )
        eType = SWQ_STRING;
    else if( strcasecmp(pszTypeName,"integer") == 0 )
        eType = SWQ_INTEGER;
    else if( strcasecmp(pszTypeName,"float") == 0 )
        eType = SWQ_FLOAT;
    else if( strcasecmp(pszTypeName,"numeric") == 0 )
        eType = SWQ_FLOAT;
    else if( strcasecmp(pszTypeName,"timestamp") == 0 )
        eType = SWQ_TIMESTAMP;
    else if( strcasecmp(pszTypeName,"date") == 0 )
        eType = SWQ_DATE;
    else if( strcasecmp(pszTypeName,"time") == 0 )
        eType = SWQ_TIME;
    else
    {
        CPLError( CE_Failure, CPLE_AppDefined,
                    "Unrecognized typename %s in CAST operator.",
                    pszTypeName );
    }

    poNode->field_type = eType;

    return eType;
}
