/*--------------------------------------------------------------------------------------+
|
|     $Source: ext/MatrixFromTiePts/src/alist.c $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*mh================================================================ SAIG inc.
**  alist.c
**
**      This module contains the function that perform the dynamic array
**      implementation of the list functions.  The implementation assumes
**      an array or variable size (that is the array is reallocated when
**      required).
**
**
**===========================================================================*/

#include <string.h>

#include "hlist.h"
#include "alist.h"


/*fh=========================================================================
**  alist_create
**
**  Martin Bussieres Dec 91 - Original version
**  alist.c
**===========================================================================*/
int8_t alist_create(
LIST  *list,
int32_t init_size,
int32_t alloc_size,
int32_t free_size,
uint32_t object_size
)
{
    DALIST  *alist_ptr;                    /* Pointer to array list */
    int32_t block_size;                   /* Amount of memory required */

    /*
    **  Validate parameters
    */
    if(init_size <= 0L ||
       alloc_size < 0L ||
       (free_size > 0L && free_size <= alloc_size) ||
       object_size == 0L)
        return(FALSE);

    list->type        = LIST_TYPE_ARRAY;  /* Implementation type */
    list->object_size = object_size;       /* Single object size */
    list->max_objects = init_size;         /* Maximum number of objects */
    list->nb_objects  = 0L;                /* Current number of objects */
    list->l           = NULL;

    /* Allocate the array specific header */
    if(! (list->l = (void *) malloc(sizeof(DALIST))))
        return(FALSE);

    alist_ptr = (DALIST *) list->l;
    alist_ptr->cur_object = 0;
    alist_ptr->alloc_size = alloc_size;
    alist_ptr->free_size  = free_size;

    block_size = object_size * init_size;


    /* Try to allocate memory for the initial number of objects */
    if(!(alist_ptr->mem = (void *) malloc((size_t) block_size)))
    {
        free(list->l);
        return(FALSE);
    }
    return(TRUE);
}



/*fh========================================================================
**  alist_destroy
**
**  Martin Bussieres Dec 91 - Original version
**===========================================================================*/
int8_t alist_destroy(
LIST *list
)
{
    DALIST  *alist_ptr;                             /* Pointer to array list */

    if(list->type != LIST_TYPE_ARRAY)
        return(FALSE);

    list->type = LIST_TYPE_UNDEFINED;

    alist_ptr = (DALIST *) list->l;
    free(alist_ptr->mem);
    free(list->l);
    return(TRUE);
}


/*fh========================================================================
**  alist_append
**
**  Martin Bussieres Dec 91 - Original version
**===========================================================================*/
int8_t alist_append(
LIST  *list,
void * object_ptr
)
{
    DALIST  *alist_ptr;                             /* Pointer to array list */
    char    *ptr;

    if(list->type != LIST_TYPE_ARRAY)
        return(FALSE);

    alist_ptr = (DALIST *) list->l;                         /* Setup pointers */

    if(list->nb_objects >= list->max_objects)         /* Check for overflow */
    {
        if(!alist_ptr->alloc_size)
            return(FALSE);

        if(alist_ptr->alloc_size == 0L)
            return(FALSE);

        alist_ptr->mem = realloc(alist_ptr->mem,
                                 (size_t) (list->max_objects+alist_ptr->alloc_size)*list->object_size);
        if(!alist_ptr->mem)
            return(FALSE);

        list->max_objects += alist_ptr->alloc_size;
    }

    ptr = (char *) alist_ptr->mem;
    memcpy(&ptr[list->nb_objects * list->object_size],   /* Copy the object */
           object_ptr,
           list->object_size);

    (list->nb_objects)++;                           /* Add one to the count */

    return(TRUE);
}

/*fh========================================================================
**  alist_insert
**
**  Martin Bussieres Dec 91 - Original version
**===========================================================================*/
int8_t alist_insert(
LIST  *list,
void * object_ptr
)
{
    DALIST  *alist_ptr;
                                 /* Pointer to array list */
    char    *ptr;

    if(list->type != LIST_TYPE_ARRAY)
        return(FALSE);

    alist_ptr = (DALIST *) list->l;                         /* Setup pointers */

    if(list->nb_objects >= list->max_objects)         /* Check for overflow */
    {
        if(!alist_ptr->alloc_size)
            return(FALSE);

        if(alist_ptr->alloc_size == 0L)
            return(FALSE);

        alist_ptr->mem = realloc(alist_ptr->mem,
                                 (size_t) (list->max_objects+alist_ptr->alloc_size)*list->object_size);
        if(!alist_ptr->mem)
            return(FALSE);

        list->max_objects += alist_ptr->alloc_size;
    }

    ptr = (char *) alist_ptr->mem;
    memmove (&ptr[(alist_ptr->cur_object + 1) * list->object_size],
             &ptr[alist_ptr->cur_object * list->object_size],
             (list->nb_objects - alist_ptr->cur_object) * list->object_size);


    memcpy(&ptr[alist_ptr->cur_object * list->object_size], /* Copy new object */
           object_ptr,
           list->object_size);

    (list->nb_objects)++;                           /* Add one to the count */

    return(TRUE);
}

/*fh========================================================================
**  alist_delete
**
**  Martin Bussieres Dec 91 - Original version
**===========================================================================*/
int8_t alist_delete(
LIST *list
)
{
    DALIST  *alist_ptr;                             /* Pointer to array list */
    char   * ptr;

    if(list->type != LIST_TYPE_ARRAY)
        return(FALSE);

    if(list->nb_objects == 0)                            /* Check emptyness */
        return(FALSE);

    alist_ptr = (DALIST *) list->l;                         /* Setup pointers */
    ptr = (char *) alist_ptr->mem;

    if(alist_ptr->cur_object >= list->nb_objects)  /* Can't delete past end */
        return(FALSE);

    if(--(list->nb_objects) > 0L)                 /* Substract one to count */
    {
        if(alist_ptr->cur_object == list->nb_objects)
            (alist_ptr->cur_object -= 1);
        else
        {

            memmove (&ptr[alist_ptr->cur_object * list->object_size],
                     &ptr[(alist_ptr->cur_object + 1) * list->object_size],
                     (list->nb_objects - alist_ptr->cur_object) * list->object_size);
        }

    }

    if (alist_ptr->free_size > 0L &&
        list->max_objects - list->nb_objects > alist_ptr->free_size)
    {
        /*
        **  Deallocate extra memory
        */
        alist_ptr->mem = realloc(alist_ptr->mem,
                                 (size_t) (list->max_objects-alist_ptr->free_size)*list->object_size);

        if(!alist_ptr->mem)
            return(FALSE);

        list->max_objects -= alist_ptr->free_size;
    }

    return(TRUE);
}


/*fh========================================================================
**  alist_get
**
**  Martin Bussieres Dec 91 - Original version
**===========================================================================*/
int8_t alist_get(
const LIST  *list,
void * object_ptr
)
{
    DALIST  *alist_ptr;                             /* Pointer to array list */
    char * ptr;

    if(list->type != LIST_TYPE_ARRAY)
        return(FALSE);

    if(list->nb_objects == 0)                            /* Check emptyness */
        return(FALSE);

    alist_ptr = (DALIST *) list->l;                         /* Setup pointers */
    ptr = (char *) alist_ptr->mem;

    if(alist_ptr->cur_object >= list->nb_objects)  /* Invalid at end of list */
        return(FALSE);

    memcpy(object_ptr,
           &ptr[alist_ptr->cur_object * list->object_size],
           list->object_size);

    return(TRUE);
}


/*fh========================================================================
**  alist_set
**
**  Martin Bussieres Dec 91 - Original version
**===========================================================================*/
int8_t alist_set(
LIST  *list,
void * object_ptr
)
{
    DALIST  *alist_ptr;                             /* Pointer to array list */
    char * ptr;

    if(list->type != LIST_TYPE_ARRAY)
        return(FALSE);

    if(list->nb_objects == 0)                            /* Check emptyness */
        return(FALSE);

    alist_ptr = (DALIST *) list->l;                         /* Setup pointers */
    ptr = (char *) alist_ptr->mem;

    memcpy(&ptr[alist_ptr->cur_object * list->object_size],
           object_ptr,
           list->object_size);

    return(TRUE);
}


/*fh========================================================================
**  alist_get_addr
**
**  Martin Bussieres Dec 90 - Original version
**===========================================================================*/
int8_t alist_get_addr(
const LIST  *list,
void * *object_ptr
)
{
    DALIST  *alist_ptr;                             /* Pointer to array list */
    char * ptr;

    if(list->type != LIST_TYPE_ARRAY)
        return(FALSE);

    if(list->nb_objects == 0)                            /* Check emptyness */
        return(FALSE);

    alist_ptr = (DALIST *) list->l;                         /* Setup pointers */
    ptr = (char *) alist_ptr->mem;

    if(alist_ptr->cur_object >= list->nb_objects)  /* Invalid at end of list */
        return(FALSE);

        *object_ptr = &ptr[alist_ptr->cur_object * list->object_size];

    return(TRUE);
}


/*fh========================================================================
**  alist_position
**
**  Martin Bussieres Dec 90 - Original version
**===========================================================================*/
int32_t alist_position(
const LIST *list
)
{
    DALIST  *alist_ptr;                             /* Pointer to array list */

    if(list->type != LIST_TYPE_ARRAY)
        return(-1L);

    alist_ptr = (DALIST *) list->l;                         /* Setup pointers */

    return(alist_ptr->cur_object);
}


/*fh========================================================================
**  alist_first
**
**  Martin Bussieres Dec 90 - Original version
**===========================================================================*/
int32_t alist_first(
LIST *list
)
{
    return(alist_goto(list,0L));
}

/*fh========================================================================
**  alist_next
**
**  Martin Bussieres Dec 90 - Original version
**===========================================================================*/
int32_t alist_next(
LIST *list
)
{
    DALIST  *alist_ptr;                             /* Pointer to array list */

    alist_ptr = (DALIST *) list->l;                         /* Setup pointers */

    return(alist_goto(list,alist_ptr->cur_object + 1));
}

/*fh========================================================================
**  alist_prev
**
**  Martin Bussieres Dec 90 - Original version
**===========================================================================*/
int32_t alist_prev(
LIST *list
)
{
    DALIST  *alist_ptr;                             /* Pointer to array list */

    alist_ptr = (DALIST *) list->l;                         /* Setup pointers */

    return(alist_goto(list,alist_ptr->cur_object - 1));
}

/*fh========================================================================
**  alist_last
**
**  Martin Bussieres Dec 90 - Original version
**===========================================================================*/
int32_t alist_last(
LIST *list
)
{
    return(alist_goto(list,list->nb_objects - 1));
}

/*fh========================================================================
**  alist_goto
**
**  Martin Bussieres Dec 90 - Original version
**===========================================================================*/
int32_t alist_goto(
LIST *list,
int32_t position
)
{
    DALIST  *alist_ptr;                             /* Pointer to array list */

    if(list->type != LIST_TYPE_ARRAY)
        return(-1L);

    if((position < 0L) || (position >= alist_count(list)))  /* Check bounds */
        return(-1L);

    alist_ptr = (DALIST *) list->l;                         /* Setup pointers */

    return(alist_ptr->cur_object = position);
}


/*fh========================================================================
**  alist_count
**
**  Martin Bussieres Dec 90 - Original version
**===========================================================================*/
int32_t alist_count(
const LIST *list
)
{
    if(list->type != LIST_TYPE_ARRAY)
        return(-1L);

    return(list->nb_objects);
}



