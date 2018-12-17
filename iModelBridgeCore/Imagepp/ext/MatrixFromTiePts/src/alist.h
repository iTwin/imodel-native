/*--------------------------------------------------------------------------------------+
|
|     $Source: ext/MatrixFromTiePts/src/alist.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*hh================================================================ SAIG inc.
**  alist.h (list.lib)
**
**  This header file defines the private functions to be used on
**  the following data types:
**
**      LIST    -   Generic list
**      DALIST  -   Dynamic array list
**
**  HMR FUNCTIONS
**         alist_create
**         alist_destroy
**         alist_append
**         alist_insert
**         alist_delete
**         alist_get
**         alist_set
**         alist_get_addr
**         alist_position
**         alist_first
**         alist_next
**         alist_prev
**         alist_last
**         alist_goto
**         alist_count
**  END HMR FUNCTIONS
**
**  Operations on these data types are functions in the list.lib library.
**===========================================================================*/
#ifndef __ALIST_H__
#define __ALIST_H__

#include "hlist.h"


/* Private functions prototypes */

int8_t alist_create(LIST *list,int32_t init_size,int32_t alloc_size,
                    int32_t free_size,uint32_t object_size);
int8_t alist_destroy(LIST *list);
int8_t alist_append(LIST *list, void * object_ptr);
int8_t alist_insert(LIST *list, void * object_ptr);
int8_t alist_delete(LIST *list);
int8_t alist_get(const LIST *list,void * object);
int8_t alist_set(LIST *list, void * object_ptr);
int8_t alist_get_addr(const LIST *list, void * *object_ptr);
int32_t alist_position(const LIST *list);
int32_t alist_first(LIST *list);
int32_t alist_next(LIST *list);
int32_t alist_prev(LIST *list);
int32_t alist_last(LIST *list);
int32_t alist_goto(LIST *list, int32_t position);
int32_t alist_count(const LIST *list);

#endif  /* __ALIST_H__ */
