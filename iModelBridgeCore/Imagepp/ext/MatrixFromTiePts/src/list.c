/*--------------------------------------------------------------------------------------+
|
|     $Source: ext/MatrixFromTiePts/src/list.c $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*mh================================================================ SAIG inc.
**  list.c (list.lib)
**
**===========================================================================*/
#include "hlist.h"
#include "alist.h"


/*fh=========================================================================
**  list_create_darray
**
**  Martin Bussieres Dec 90 - Original version
**  list.c (list.lib)
**===========================================================================*/
int8_t list_create_darray(
LIST   *list,
int32_t init_size,
int32_t alloc_size,
int32_t free_size,
uint32_t object_size
)
{
        return(alist_create(list,init_size,alloc_size,free_size,object_size));
}


/*fh=========================================================================
**  list_destroy
**
**  Martin Bussieres Dec 90 - Original version
**  list.c (list.lib)
**===========================================================================*/
int8_t list_destroy(
LIST *list
)
{
    if(list->type == LIST_TYPE_ARRAY)
        return(alist_destroy(list));
    else
        return FALSE;
}


/*fh=========================================================================
**  list_append
**
**  Martin Bussieres Dec 90 - Original version
**  list.c (list.lib)
**===========================================================================*/
int8_t list_append(
LIST  *list,
void * object_ptr
)
{
    if(list->type == LIST_TYPE_ARRAY)
        return(alist_append(list,object_ptr));
    else
        return FALSE;
}


/*fh=========================================================================
**  list_insert
**
**  Martin Bussieres Dec 90 - Original version
**  list.c (list.lib)
**===========================================================================*/
int8_t list_insert(
LIST  *list,
void * object_ptr
)
{
    if(list->type == LIST_TYPE_ARRAY)
        return(alist_insert(list,object_ptr));
    else
        return FALSE;

}


/*fh=========================================================================
**  list_delete
**
**  Martin Bussieres Dec 90 - Original version
**  list.c (list.lib)
**===========================================================================*/
int8_t list_delete(
LIST *list
)
{
    if(list->type == LIST_TYPE_ARRAY)
        return(alist_delete(list));
    else
        return FALSE;

}


/*fh=========================================================================
**  list_get
**
**  Martin Bussieres Dec 90 - Original version
**  list.c (list.lib)
**===========================================================================*/
int8_t list_get(
const LIST  *list,
void * object_ptr
)
{
    if(list->type == LIST_TYPE_ARRAY)
        return(alist_get(list,object_ptr));
    else
        return FALSE;
}


/*fh=========================================================================
**  list_set
**
**  Martin Bussieres Dec 90 - Original version
**  list.c (list.lib)
**===========================================================================*/
int8_t list_set(
LIST  *list,
void * object_ptr
)
{
    if(list->type == LIST_TYPE_ARRAY)
        return(alist_set(list,object_ptr));
    else
        return FALSE;
}

/*fh=========================================================================
**  list_get_addr
**
**  Martin Bussieres Dec 90 - Original version
**  list.c (list.lib)
**===========================================================================*/
int8_t list_get_addr(
const LIST   *list,
void * *object_ptr
)
{
    if(list->type == LIST_TYPE_ARRAY)
                return(alist_get_addr(list,object_ptr));
    else
        return FALSE;
}


/*fh=========================================================================
**  list_position
**
**  Martin Bussieres Dec 90 - Original version
**  list.c (list.lib)
**===========================================================================*/
int32_t list_position(
const LIST *list
)
{
    if(list->type == LIST_TYPE_ARRAY)
        return(alist_position(list));
    else
        return FALSE;
}


/*fh=========================================================================
**  list_first
**
**  Martin Bussieres Dec 90 - Original version
**  list.c (list.lib)
**===========================================================================*/
int32_t list_first(
LIST *list
)
{
    if(list->type == LIST_TYPE_ARRAY)
        return(alist_first(list));
    else
        return FALSE;
}

/*fh=========================================================================
**  list_next
**
**  Martin Bussieres Dec 90 - Original version
**  list.c (list.lib)
**===========================================================================*/
int32_t list_next(
LIST *list
)
{
    if(list->type == LIST_TYPE_ARRAY)
        return(alist_next(list));
    else
        return FALSE;
}

/*fh=========================================================================
**  list_prev
**
**  Martin Bussieres Dec 90 - Original version
**  list.c (list.lib)
**===========================================================================*/
int32_t list_prev(
LIST *list
)
{
    if(list->type == LIST_TYPE_ARRAY)
        return(alist_prev(list));
    else
        return FALSE;
}

/*fh=========================================================================
**  list_last
**
**  Martin Bussieres Dec 90 - Original version
**  list.c (list.lib)
**===========================================================================*/
int32_t list_last(
LIST *list
)
{
    if(list->type == LIST_TYPE_ARRAY)
        return(alist_last(list));
    else
        return FALSE;
}

/*fh=========================================================================
**  list_goto
**
**  Martin Bussieres Dec 90 - Original version
**  list.c (list.lib)
**===========================================================================*/
int32_t list_goto(
LIST *list,
int32_t position
)
{
    if(list->type == LIST_TYPE_ARRAY)
        return(alist_goto(list,position));
    else
        return FALSE;
}


/*fh=========================================================================
**  list_count
**
**  Martin Bussieres Dec 90 - Original version
**  list.c (list.lib)
**===========================================================================*/
int32_t list_count(
const LIST *list
)
{
    if(list->type == LIST_TYPE_ARRAY)
        return(alist_count(list));
    else
        return FALSE;
}


