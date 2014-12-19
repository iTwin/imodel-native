/******************************************************************************
 *
 * Component: OGR SQL Engine
 * Purpose: Implementation of the swq_expr_node class used to represent a
 *          node in an SQL expression.
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
#include "cpl_multiproc.h"
#include "swq.h"
#include <vector>

/************************************************************************/
/*                           swq_expr_node()                            */
/************************************************************************/

swq_expr_node::swq_expr_node()

{
    Initialize();
}

/************************************************************************/
/*                          swq_expr_node(int)                          */
/************************************************************************/

swq_expr_node::swq_expr_node( int nValueIn )

{
    Initialize();

    int_value = nValueIn;
}

/************************************************************************/
/*                        swq_expr_node(double)                         */
/************************************************************************/

swq_expr_node::swq_expr_node( double dfValueIn )

{
    Initialize();

    field_type = SWQ_FLOAT;
    float_value = dfValueIn;
}

/************************************************************************/
/*                        swq_expr_node(const char*)                    */
/************************************************************************/

swq_expr_node::swq_expr_node( const char *pszValueIn )

{
    Initialize();

    field_type = SWQ_STRING;
    string_value = CPLStrdup(pszValueIn);
}

/************************************************************************/
/*                        swq_expr_node(swq_op)                         */
/************************************************************************/

swq_expr_node::swq_expr_node( swq_op eOp )

{
    Initialize();

    eNodeType = SNT_OPERATION;
    
    nOperation = (int) eOp;
    nSubExprCount = 0;
    papoSubExpr = NULL;
}

/************************************************************************/
/*                             Initialize()                             */
/************************************************************************/

void swq_expr_node::Initialize()

{
    eNodeType = SNT_CONSTANT;
    field_type = SWQ_INTEGER;
    int_value = 0;

    is_null = FALSE;
    string_value = NULL;
    papoSubExpr = NULL;
    nSubExprCount = 0;
}

/************************************************************************/
/*                           ~swq_expr_node()                           */
/************************************************************************/

swq_expr_node::~swq_expr_node()

{
    CPLFree( string_value );

    int i;
    for( i = 0; i < nSubExprCount; i++ )
        delete papoSubExpr[i];
    CPLFree( papoSubExpr );
}

/************************************************************************/
/*                         PushSubExpression()                          */
/************************************************************************/

void swq_expr_node::PushSubExpression( swq_expr_node *child )

{
    nSubExprCount++;
    papoSubExpr = (swq_expr_node **) 
        CPLRealloc( papoSubExpr, sizeof(void*) * nSubExprCount );
    
    papoSubExpr[nSubExprCount-1] = child;
}

/************************************************************************/
/*                       ReverseSubExpressions()                        */
/************************************************************************/

void swq_expr_node::ReverseSubExpressions()

{
    int i;
    for( i = 0; i < nSubExprCount / 2; i++ )
    {
        swq_expr_node *temp;

        temp = papoSubExpr[i];
        papoSubExpr[i] = papoSubExpr[nSubExprCount - i - 1];
        papoSubExpr[nSubExprCount - i - 1] = temp;
    }
}

/************************************************************************/
/*                               Check()                                */
/*                                                                      */
/*      Check argument types, etc.                                      */
/************************************************************************/

swq_field_type swq_expr_node::Check( swq_field_list *poFieldList )

{
/* -------------------------------------------------------------------- */
/*      If something is a string constant, we must check if it is       */
/*      actually a reference to a field in which case we will           */
/*      convert it into a column type.                                  */
/* -------------------------------------------------------------------- */
    if( eNodeType == SNT_CONSTANT && field_type == SWQ_STRING )
    {
        int wrk_field_index, wrk_table_index;
        swq_field_type wrk_field_type;

        wrk_field_index = 
            swq_identify_field( string_value, poFieldList,
                                &wrk_field_type, &wrk_table_index );
        
        if( wrk_field_index >= 0 )
        {
            eNodeType = SNT_COLUMN;
            field_index = -1;
            table_index = -1;
        }
    }

/* -------------------------------------------------------------------- */
/*      Otherwise we take constants literally.                          */
/* -------------------------------------------------------------------- */
    if( eNodeType == SNT_CONSTANT )
        return field_type;

/* -------------------------------------------------------------------- */
/*      If this is intended to be a field definition, but has not       */
/*      yet been looked up, we do so now.                               */
/* -------------------------------------------------------------------- */
    if( eNodeType == SNT_COLUMN && field_index == -1 )
    {
        field_index = 
            swq_identify_field( string_value, poFieldList,
                                &field_type, &table_index );
        
        if( field_index < 0 )
        {
            CPLError( CE_Failure, CPLE_AppDefined, 
                      "'%s' not recognised as an available field.",
                      string_value );

            return SWQ_ERROR;
            
        }
    }
    
    if( eNodeType == SNT_COLUMN )
        return field_type;
    
/* -------------------------------------------------------------------- */
/*      We are dealing with an operation - fetch the definition.        */
/* -------------------------------------------------------------------- */
    const swq_operation *poOp = 
        swq_op_registrar::GetOperator((swq_op)nOperation);

    if( poOp == NULL )
    {
        CPLError( CE_Failure, CPLE_AppDefined,
                  "Check(): Unable to find definition for operator %d.",
                  nOperation );
        return SWQ_ERROR;
    }

/* -------------------------------------------------------------------- */
/*      Check subexpressions first.                                     */
/* -------------------------------------------------------------------- */
    int i;

    for( i = 0; i < nSubExprCount; i++ )
    {
        if( papoSubExpr[i]->Check(poFieldList) == SWQ_ERROR )
            return SWQ_ERROR;
    }
    
/* -------------------------------------------------------------------- */
/*      Check this node.                                                */
/* -------------------------------------------------------------------- */
    field_type = poOp->pfnChecker( this );

    return field_type;
}

/************************************************************************/
/*                                Dump()                                */
/************************************************************************/

void swq_expr_node::Dump( FILE * fp, int depth )

{
    char        spaces[60];
    int         i;

    for( i = 0; i < depth*2 && i < (int) sizeof(spaces); i++ )
        spaces[i] = ' ';
    spaces[i] = '\0';

    if( eNodeType == SNT_COLUMN )
    {
        fprintf( fp, "%s  Field %d\n", spaces, field_index );
        return;
    }

    if( eNodeType == SNT_CONSTANT )
    {
        if( field_type == SWQ_INTEGER || field_type == SWQ_BOOLEAN )
            fprintf( fp, "%s  %d\n", spaces, int_value );
        else if( field_type == SWQ_FLOAT )
            fprintf( fp, "%s  %.15g\n", spaces, float_value );
        else
            fprintf( fp, "%s  %s\n", spaces, string_value );
        return;
    }

    CPLAssert( eNodeType == SNT_OPERATION );

    const swq_operation *op_def = 
        swq_op_registrar::GetOperator( (swq_op) nOperation );

    fprintf( fp, "%s%s\n", spaces, op_def->osName.c_str() );

    for( i = 0; i < nSubExprCount; i++ )
        papoSubExpr[i]->Dump( fp, depth+1 );
}

/************************************************************************/
/*                               Quote()                                */
/*                                                                      */
/*      Add quoting necessary to unparse a string.                      */
/************************************************************************/

void swq_expr_node::Quote( CPLString &osTarget )

{
    CPLString osNew;
    int i;

    osNew = "'";

    for( i = 0; i < (int) osTarget.size(); i++ )
    {
        if( osTarget[i] == '\'' )
            osNew += "''";
        else
            osNew += osTarget[i];
    }
    osNew += "'";

    osTarget = osNew;
}

/************************************************************************/
/*                              Unparse()                               */
/************************************************************************/

char *swq_expr_node::Unparse( swq_field_list *field_list )

{
    CPLString osExpr;

/* -------------------------------------------------------------------- */
/*      Handle constants.                                               */
/* -------------------------------------------------------------------- */
    if( eNodeType == SNT_CONSTANT )
    {
        if( field_type == SWQ_INTEGER || field_type == SWQ_BOOLEAN )
            osExpr.Printf( "%d", int_value );
        else if( field_type == SWQ_FLOAT )
            osExpr.Printf( "%.15g", float_value );
        else 
        {
            osExpr = string_value;
            Quote( osExpr );
        }
        
        return CPLStrdup(osExpr);
    }

/* -------------------------------------------------------------------- */
/*      Handle columns.                                                 */
/* -------------------------------------------------------------------- */
    if( eNodeType == SNT_COLUMN )
    {
        if( field_index != -1 
            && table_index < field_list->table_count 
            && table_index > 0 )
            osExpr.Printf( "%s.%s", 
                           field_list->table_defs[table_index].table_name,
                           field_list->names[field_index] );
        else if( field_index != -1 )
            osExpr.Printf( "%s", field_list->names[field_index] );

        Quote( osExpr );

        return CPLStrdup(osExpr.c_str());
    }

/* -------------------------------------------------------------------- */
/*      Operation - start by unparsing all the subexpressions.          */
/* -------------------------------------------------------------------- */
    std::vector<char*> apszSubExpr;
    int i;

    for( i = 0; i < nSubExprCount; i++ )
        apszSubExpr.push_back( papoSubExpr[i]->Unparse(field_list) );

/* -------------------------------------------------------------------- */
/*      Put things together in a fashion depending on the operator.     */
/* -------------------------------------------------------------------- */
    const swq_operation *poOp = 
        swq_op_registrar::GetOperator( (swq_op) nOperation );

    if( poOp == NULL )
    {
        CPLAssert( FALSE );
        return CPLStrdup("");
    }

    switch( nOperation )
    {
        // binary infix operators.
      case SWQ_OR:
      case SWQ_AND:
      case SWQ_EQ:
      case SWQ_NE:
      case SWQ_GT:
      case SWQ_LT:
      case SWQ_GE:
      case SWQ_LE:
      case SWQ_LIKE:
      case SWQ_ADD:
      case SWQ_SUBTRACT:
      case SWQ_MULTIPLY:
      case SWQ_DIVIDE:
      case SWQ_MODULUS:
        CPLAssert( nSubExprCount == 2 );
        osExpr.Printf( "(%s) %s (%s)", 
                       apszSubExpr[0],
                       poOp->osName.c_str(),
                       apszSubExpr[1] );
        break;

      case SWQ_NOT:
        CPLAssert( nSubExprCount == 1 );
        osExpr.Printf( "NOT (%s)", apszSubExpr[0] );
        break;
        
      case SWQ_ISNULL:
        CPLAssert( nSubExprCount == 1 );
        osExpr.Printf( "%s IS NULL", apszSubExpr[0] );
        break;
        
      case SWQ_IN:
        osExpr.Printf( "%s IN (", apszSubExpr[0] );
        for( i = 1; i < nSubExprCount; i++ )
        {
            if( i > 1 )
                osExpr += ",";
            osExpr += "(";
            osExpr += apszSubExpr[i];
            osExpr += ")";
        }
        osExpr += ")";
        break;

      case SWQ_BETWEEN:
        CPLAssert( nSubExprCount == 3 );
        osExpr.Printf( "%s %s (%s) AND (%s)",
                       apszSubExpr[0],
                       poOp->osName.c_str(),
                       apszSubExpr[1],
                       apszSubExpr[2] );
        break;

      default: // function style.
        osExpr.Printf( "%s(", poOp->osName.c_str() );
        for( i = 0; i < nSubExprCount; i++ )
        {
            if( i > 0 )
                osExpr += ",";
            osExpr += "(";
            osExpr += apszSubExpr[i];
            osExpr += ")";
        }
        osExpr += ")";
        break;
    }

/* -------------------------------------------------------------------- */
/*      cleanup subexpressions.                                         */
/* -------------------------------------------------------------------- */
    for( i = 0; i < nSubExprCount; i++ )
        CPLFree( apszSubExpr[i] );

    return CPLStrdup( osExpr.c_str() );
}

/************************************************************************/
/*                              Evaluate()                              */
/************************************************************************/

swq_expr_node *swq_expr_node::Evaluate( swq_field_fetcher pfnFetcher,
                                        void *pRecord )

{
    swq_expr_node *poRetNode = NULL;

/* -------------------------------------------------------------------- */
/*      Duplicate ourselves if we are already a constant.               */
/* -------------------------------------------------------------------- */
    if( eNodeType == SNT_CONSTANT )
    {
        poRetNode = new swq_expr_node();

        poRetNode->eNodeType = SNT_CONSTANT;
        poRetNode->field_type = field_type;
        poRetNode->int_value = int_value;
        poRetNode->float_value = float_value;

        if( string_value )
            poRetNode->string_value = CPLStrdup(string_value);
        else
            poRetNode->string_value = NULL;

        return poRetNode;
    }

/* -------------------------------------------------------------------- */
/*      If this is a field value from a record, fetch and return it.    */
/* -------------------------------------------------------------------- */
    if( eNodeType == SNT_COLUMN )
    {
        return pfnFetcher( this, pRecord );
    }

/* -------------------------------------------------------------------- */
/*      This is an operation, collect the arguments keeping track of    */
/*      which we will need to free.                                     */
/* -------------------------------------------------------------------- */
    std::vector<swq_expr_node*> apoValues;
    std::vector<int> anValueNeedsFree;
    int i, bError = FALSE;

    for( i = 0; i < nSubExprCount && !bError; i++ )
    {
        if( papoSubExpr[i]->eNodeType == SNT_CONSTANT )
        {
            // avoid duplication.
            apoValues.push_back( papoSubExpr[i] );
            anValueNeedsFree.push_back( FALSE );
        }
        else
        {
            apoValues.push_back(papoSubExpr[i]->Evaluate(pfnFetcher,pRecord));
            anValueNeedsFree.push_back( TRUE );
        }
    }

/* -------------------------------------------------------------------- */
/*      Fetch the operator definition and function.                     */
/* -------------------------------------------------------------------- */
    if( !bError )
    {
        const swq_operation *poOp = 
            swq_op_registrar::GetOperator( (swq_op) nOperation );
        
        poRetNode = poOp->pfnEvaluator( this, &(apoValues[0]) );
    }

/* -------------------------------------------------------------------- */
/*      Cleanup                                                         */
/* -------------------------------------------------------------------- */
    for( i = 0; i < (int) apoValues.size(); i++ )
    {
        if( anValueNeedsFree[i] )
            delete apoValues[i];
    }

    return poRetNode;
}
