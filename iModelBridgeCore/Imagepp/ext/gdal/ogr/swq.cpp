/******************************************************************************
 *
 * Component: OGDI Driver Support Library
 * Purpose: Generic SQL WHERE Expression Implementation.
 * Author: Frank Warmerdam <warmerdam@pobox.com>
 * 
 ******************************************************************************
 * Copyright (C) 2001 Information Interoperability Institute (3i)
 *
 * Permission to use, copy, modify and distribute this software and
 * its documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies, that
 * both the copyright notice and this permission notice appear in
 * supporting documentation, and that the name of 3i not be used 
 * in advertising or publicity pertaining to distribution of the software 
 * without specific, written prior permission.  3i makes no
 * representations about the suitability of this software for any purpose.
 * It is provided "as is" without express or implied warranty.
 ****************************************************************************/

#include <ctype.h>

#include "cpl_conv.h"
#include "cpl_multiproc.h"
#include "swq.h"
#include "swq_parser.hpp"

#define YYSTYPE  swq_expr_node*

/************************************************************************/
/*                               swqlex()                               */
/*                                                                      */
/*      Read back a token from the input.                               */
/************************************************************************/

int swqlex( YYSTYPE *ppNode, swq_parse_context *context )
{
    const char *pszInput = context->pszNext;

    *ppNode = NULL;

/* -------------------------------------------------------------------- */
/*      Do we have a start symbol to return?                            */
/* -------------------------------------------------------------------- */
    if( context->nStartToken != 0 )
    {
        int nRet = context->nStartToken;
        context->nStartToken = 0;
        return nRet;
    }

/* -------------------------------------------------------------------- */
/*      Skip white space.                                               */
/* -------------------------------------------------------------------- */
    while( *pszInput == ' ' || *pszInput == '\t'
           || *pszInput == 10 || *pszInput == 13 )
        pszInput++;

    if( *pszInput == '\0' )
    {
        context->pszNext = pszInput;
        return EOF; 
    }

/* -------------------------------------------------------------------- */
/*      Handle string constants.                                        */
/* -------------------------------------------------------------------- */
    if( *pszInput == '"' || *pszInput == '\'' )
    {
        char *token;
        int i_token;

        pszInput++;

        token = (char *) CPLMalloc(strlen(pszInput)+1);
        i_token = 0;

        while( *pszInput != '\0' )
        {
            if( *pszInput == '\\' && pszInput[1] == '"' )
                pszInput++;
            else if( *pszInput == '\\' && pszInput[1] == '\'' )
                pszInput++;
            else if( *pszInput == '\'' && pszInput[1] == '\'' )
                pszInput++;
            else if( *pszInput == '"' )
            {
                pszInput++;
                break;
            }
            else if( *pszInput == '\'' )
            {
                pszInput++;
                break;
            }
            
            token[i_token++] = *(pszInput++);
        }
        token[i_token] = '\0';

        *ppNode = new swq_expr_node( token );
        CPLFree( token );

        context->pszNext = pszInput;

        return SWQT_STRING;
    }

/* -------------------------------------------------------------------- */
/*      Handle numbers.                                                 */
/* -------------------------------------------------------------------- */
    else if( *pszInput >= '0' && *pszInput <= '9' )
    {
        CPLString osToken;
        const char *pszNext = pszInput + 1;

        osToken += *pszInput;

        // collect non-decimal part of number
        while( *pszNext >= '0' && *pszNext <= '9' )
            osToken += *(pszNext++);

        // collect decimal places.
        if( *pszNext == '.' )
        {
            osToken += *(pszNext++);
            while( *pszNext >= '0' && *pszNext <= '9' )
                osToken += *(pszNext++);
        }

        // collect exponent
        if( *pszNext == 'e' || *pszNext == 'E' )
        {
            osToken += *(pszNext++);
            if( *pszNext == '-' || *pszNext == '+' )
                osToken += *(pszNext++);
            while( *pszNext >= '0' && *pszNext <= '9' )
                osToken += *(pszNext++);
        }

        context->pszNext = pszNext;

        if( strstr(osToken,".") 
            || strstr(osToken,"e") 
            || strstr(osToken,"E") )
        {
            *ppNode = new swq_expr_node( CPLAtof(osToken) );
        }
        else
        {
            *ppNode = new swq_expr_node( atoi(osToken) );
        }

        return SWQT_NUMBER;
    }

/* -------------------------------------------------------------------- */
/*      Handle alpha-numerics.                                          */
/* -------------------------------------------------------------------- */
    else if( isalnum( *pszInput ) )
    {
        int nReturn = SWQT_IDENTIFIER;
        CPLString osToken;
        const char *pszNext = pszInput + 1;

        osToken += *pszInput;

        // collect text characters
        while( isalnum( *pszNext ) || *pszNext == '_' 
               || ((unsigned char) *pszNext) > 127 )
            osToken += *(pszNext++);

        context->pszNext = pszNext;

        if( EQUAL(osToken,"IN") )
            nReturn = SWQT_IN;
        else if( EQUAL(osToken,"LIKE") )
            nReturn = SWQT_LIKE;
        else if( EQUAL(osToken,"ILIKE") )
            nReturn = SWQT_LIKE;
        else if( EQUAL(osToken,"NULL") )
            nReturn = SWQT_NULL;
        else if( EQUAL(osToken,"IS") )
            nReturn = SWQT_IS;
        else if( EQUAL(osToken,"NOT") )
            nReturn = SWQT_NOT;
        else if( EQUAL(osToken,"AND") )
            nReturn = SWQT_AND;
        else if( EQUAL(osToken,"OR") )
            nReturn = SWQT_OR;
        else if( EQUAL(osToken,"BETWEEN") )
            nReturn = SWQT_BETWEEN;
        else if( EQUAL(osToken,"SELECT") )
            nReturn = SWQT_SELECT;
        else if( EQUAL(osToken,"LEFT") )
            nReturn = SWQT_LEFT;
        else if( EQUAL(osToken,"JOIN") )
            nReturn = SWQT_JOIN;
        else if( EQUAL(osToken,"WHERE") )
            nReturn = SWQT_WHERE;
        else if( EQUAL(osToken,"ON") )
            nReturn = SWQT_ON;
        else if( EQUAL(osToken,"ORDER") )
            nReturn = SWQT_ORDER;
        else if( EQUAL(osToken,"BY") )
            nReturn = SWQT_BY;
        else if( EQUAL(osToken,"FROM") )
            nReturn = SWQT_FROM;
        else if( EQUAL(osToken,"AS") )
            nReturn = SWQT_AS;
        else if( EQUAL(osToken,"ASC") )
            nReturn = SWQT_ASC;
        else if( EQUAL(osToken,"DESC") )
            nReturn = SWQT_DESC;
        else if( EQUAL(osToken,"DISTINCT") )
            nReturn = SWQT_DISTINCT;
        else if( EQUAL(osToken,"CAST") )
            nReturn = SWQT_CAST;
        else
        {
            *ppNode = new swq_expr_node( osToken );
            nReturn = SWQT_IDENTIFIER;
        }

        return nReturn;
    }

/* -------------------------------------------------------------------- */
/*      Handle special tokens.                                          */
/* -------------------------------------------------------------------- */
    else
    {
        context->pszNext = pszInput+1;
        return *pszInput;
    }
}

/************************************************************************/
/*                        swq_select_summarize()                        */
/************************************************************************/

const char *
swq_select_summarize( swq_select *select_info, 
                      int dest_column, const char *value )

{
    swq_col_def *def = select_info->column_defs + dest_column;
    swq_summary *summary;

/* -------------------------------------------------------------------- */
/*      Do various checking.                                            */
/* -------------------------------------------------------------------- */
    if( select_info->query_mode == SWQM_RECORDSET )
        return "swq_select_summarize() called on non-summary query.";

    if( dest_column < 0 || dest_column >= select_info->result_columns )
        return "dest_column out of range in swq_select_summarize().";

    if( def->col_func == SWQCF_NONE && !def->distinct_flag )
        return NULL;

/* -------------------------------------------------------------------- */
/*      Create the summary information if this is the first row         */
/*      being processed.                                                */
/* -------------------------------------------------------------------- */
    if( select_info->column_summary == NULL )
    {
        int i;

        select_info->column_summary = (swq_summary *) 
            CPLMalloc(sizeof(swq_summary) * select_info->result_columns);
        memset( select_info->column_summary, 0, 
                sizeof(swq_summary) * select_info->result_columns );

        for( i = 0; i < select_info->result_columns; i++ )
        {
            select_info->column_summary[i].min = 1e20;
            select_info->column_summary[i].max = -1e20;
        }
    }

/* -------------------------------------------------------------------- */
/*      If distinct processing is on, process that now.                 */
/* -------------------------------------------------------------------- */
    summary = select_info->column_summary + dest_column;
    
    if( def->distinct_flag )
    {
        int  i;

        /* This should be implemented with a much more complicated
           data structure to achieve any sort of efficiency. */
        for( i = 0; i < summary->count; i++ )
        {
            if( strcmp(value,summary->distinct_list[i]) == 0 )
                break;
        }
        
        if( i == summary->count )
        {
            char  **old_list = summary->distinct_list;
            
            summary->distinct_list = (char **) 
                CPLMalloc(sizeof(char *) * (summary->count+1));
            memcpy( summary->distinct_list, old_list, 
                    sizeof(char *) * summary->count );
            summary->distinct_list[(summary->count)++] = 
                CPLStrdup( value );

            CPLFree(old_list);
        }
    }

/* -------------------------------------------------------------------- */
/*      Process various options.                                        */
/* -------------------------------------------------------------------- */

    switch( def->col_func )
    {
      case SWQCF_MIN:
        if( value != NULL && value[0] != '\0' )
        {
            double df_val = CPLAtof(value);
            if( df_val < summary->min )
                summary->min = df_val;
        }
        break;
      case SWQCF_MAX:
        if( value != NULL && value[0] != '\0' )
        {
            double df_val = CPLAtof(value);
            if( df_val > summary->max )
                summary->max = df_val;
        }
        break;
      case SWQCF_AVG:
      case SWQCF_SUM:
        if( value != NULL && value[0] != '\0' )
        {
            summary->count++;
            summary->sum += CPLAtof(value);
        }
        break;

      case SWQCF_COUNT:
        if( value != NULL && !def->distinct_flag )
            summary->count++;
        break;

      case SWQCF_NONE:
        break;

      case SWQCF_CUSTOM:
        return "swq_select_summarize() called on custom field function.";

      default:
        return "swq_select_summarize() - unexpected col_func";
    }

    return NULL;
}
/************************************************************************/
/*                      sort comparison functions.                      */
/************************************************************************/

static int FORCE_CDECL swq_compare_int( const void *item1, const void *item2 )
{
    int  v1, v2;

    v1 = atoi(*((const char **) item1));
    v2 = atoi(*((const char **) item2));

    if( v1 < v2 )
        return -1;
    else if( v1 == v2 )
        return 0;
    else
        return 1;
}

static int FORCE_CDECL swq_compare_real( const void *item1, const void *item2 )
{
    double  v1, v2;

    v1 = CPLAtof(*((const char **) item1));
    v2 = CPLAtof(*((const char **) item2));

    if( v1 < v2 )
        return -1;
    else if( v1 == v2 )
        return 0;
    else
        return 1;
}

static int FORCE_CDECL swq_compare_string( const void *item1, const void *item2 )
{
    return strcmp( *((const char **) item1), *((const char **) item2) );
}

/************************************************************************/
/*                    swq_select_finish_summarize()                     */
/*                                                                      */
/*      Call to complete summarize work.  Does stuff like ordering      */
/*      the distinct list for instance.                                 */
/************************************************************************/

const char *swq_select_finish_summarize( swq_select *select_info )

{
    int (FORCE_CDECL *compare_func)(const void *, const void*);
    int count = 0;
    char **distinct_list = NULL;

    if( select_info->query_mode != SWQM_DISTINCT_LIST 
        || select_info->order_specs == 0 )
        return NULL;

    if( select_info->order_specs > 1 )
        return "Can't ORDER BY a DISTINCT list by more than one key.";

    if( select_info->order_defs[0].field_index != 
        select_info->column_defs[0].field_index )
        return "Only selected DISTINCT field can be used for ORDER BY.";

    if( select_info->column_summary == NULL )
        return NULL;

    if( select_info->column_defs[0].field_type == SWQ_INTEGER )
        compare_func = swq_compare_int;
    else if( select_info->column_defs[0].field_type == SWQ_FLOAT )
        compare_func = swq_compare_real;
    else
        compare_func = swq_compare_string;

    distinct_list = select_info->column_summary[0].distinct_list;
    count = select_info->column_summary[0].count;

    qsort( distinct_list, count, sizeof(char *), compare_func );

/* -------------------------------------------------------------------- */
/*      Do we want the list ascending in stead of descending?           */
/* -------------------------------------------------------------------- */
    if( !select_info->order_defs[0].ascending_flag )
    {
        char *saved;
        int i;

        for( i = 0; i < count/2; i++ )
        {
            saved = distinct_list[i];
            distinct_list[i] = distinct_list[count-i-1];
            distinct_list[count-i-1] = saved;
        }
    }

    return NULL;
}

/************************************************************************/
/*                          swq_select_free()                           */
/************************************************************************/

void swq_select_free( swq_select *select_info )

{
    delete select_info;
}

/************************************************************************/
/*                         swq_identify_field()                         */
/************************************************************************/

int swq_identify_field( const char *token, swq_field_list *field_list,
                        swq_field_type *this_type, int *table_id )

{
    int i;
    char table_name[128];
    const char *field_token = token;
    int   tables_enabled;

    if( field_list->table_count > 0 && field_list->table_ids != NULL )
        tables_enabled = TRUE;
    else
        tables_enabled = FALSE;

/* -------------------------------------------------------------------- */
/*      Parse out table name if present, and table support enabled.     */
/* -------------------------------------------------------------------- */
    table_name[0] = '\0';
    if( tables_enabled && strchr(token, '.') != NULL )
    {
        int dot_offset = (int)(strchr(token,'.') - token);

        if( dot_offset < (int) sizeof(table_name) )
        {
            strncpy( table_name, token, dot_offset );
            table_name[dot_offset] = '\0';
            field_token = token + dot_offset + 1;
        }
    }

/* -------------------------------------------------------------------- */
/*      Search for matching field.                                      */
/* -------------------------------------------------------------------- */
    for( i = 0; i < field_list->count; i++ )
    {
        int  t_id = 0;

        if( !EQUAL( field_list->names[i], field_token ) )
            continue;

        /* Do the table specifications match? */
        if( tables_enabled )
        {
            t_id = field_list->table_ids[i];
            if( table_name[0] != '\0' 
                && !EQUAL(table_name,field_list->table_defs[t_id].table_alias))
                continue;

//            if( t_id != 0 && table_name[0] == '\0' )
//                continue;
        }

        /* We have a match, return various information */
        if( this_type != NULL )
        {
            if( field_list->types != NULL )
                *this_type = field_list->types[i];
            else
                *this_type = SWQ_OTHER;
        }
        
        if( table_id != NULL )
            *table_id = t_id;

        if( field_list->ids == NULL )
            return i;
        else
            return field_list->ids[i];
    }

/* -------------------------------------------------------------------- */
/*      No match, return failure.                                       */
/* -------------------------------------------------------------------- */
    if( this_type != NULL )
        *this_type = SWQ_OTHER;

    if( table_id != NULL )
        *table_id = 0;

    return -1;
}

/************************************************************************/
/*                          swq_expr_compile()                          */
/************************************************************************/

CPLErr swq_expr_compile( const char *where_clause,
                         int field_count,
                         char **field_names, 
                         swq_field_type *field_types, 
                         swq_expr_node **expr_out )

{
    swq_field_list  field_list;

    field_list.count = field_count;
    field_list.names = field_names;
    field_list.types = field_types;
    field_list.table_ids = NULL;
    field_list.ids = NULL;
    
    field_list.table_count = 0;
    field_list.table_defs = NULL;

    return swq_expr_compile2( where_clause, &field_list, expr_out );
}


/************************************************************************/
/*                         swq_expr_compile2()                          */
/************************************************************************/

CPLErr swq_expr_compile2( const char *where_clause, 
                          swq_field_list *field_list,
                          swq_expr_node **expr_out )

{

    swq_parse_context context;

    context.pszInput = where_clause;
    context.pszNext = where_clause;
    context.nStartToken = SWQT_LOGICAL_START;
    
    if( swqparse( &context ) == 0 
        && context.poRoot->Check( field_list ) != SWQ_ERROR )
    {
        *expr_out = context.poRoot;

        return CE_None;
    }
    else
    {
        delete context.poRoot;
        *expr_out = NULL;
        return CE_Failure;
    }
}
