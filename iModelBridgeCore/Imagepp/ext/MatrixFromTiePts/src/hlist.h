/*--------------------------------------------------------------------------------------+
|
|     $Source: ext/MatrixFromTiePts/src/hlist.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*====================================================================
**  hlist.h
**
**  This header file defines the following data types:
**
**      LIST    -   Generic list
**      DALIST  -   Dynamic array list
**      STACK   -   Stack
**
**  Operations on these data types are functions in the list.lib
**  library. This header file contains the function prototypes
**  of all public functions of the list.lib library.
**
**  HMR FUNCTIONS
**         list_SCAN
**         list_create_darray
**         list_destroy
**         list_append
**         list_insert
**         list_delete
**         list_get
**         list_set
**         list_get_addr
**         list_position
**         list_first
**         list_next
**         list_prev
**         list_last
**         list_goto
**         list_count
**
**         list_find
**         list_insert_sorted
**         list_bsearch
**  END HMR FUNCTIONS
**
**====================================================================*/
#ifndef __HLIST_H__
#define __HLIST_H__

#include "oldhtypes.h"
#include "oldhmrtypes.h"

/*====================================================================
**  Defines
**====================================================================*/
#define     LIST_TYPE_ARRAY         0   /* Dynamic array implementation */
#define     LIST_TYPE_LINKED_ARRAY  2   /* Linked array                 */
#define     LIST_TYPE_UNDEFINED     99

/*====================================================================
**  LNODE
**
**  This structure records the information specific to the linked
**  implementation of a list.
**
**  previous_node   -   Pointer to the previous node in the list
**
**  object          -   Pointer to the data
**
**  next_node       -   Pointer to the next node in the list
**
**====================================================================*/
typedef struct tagLNODE
{
    struct tagLNODE   *prev_node;
    void *  mem;
    struct tagLNODE   *next_node;
} LNODE;

typedef LNODE * pLNODE;


/*====================================================================
**  DALIST
**
**  This structure records the information specific to the dynamic array
**  implementation of a list.
**
**  cur_object      -   Index of the current object in the list, starting
**                      at 0 for the first object.
**
**  alloc_size      -   Block size for reallocation.
**
**  free_size       -   Block size for deallocation.
**
**  mem             -   Pointer to a memory block that is the array of
**                      objects.
**
**====================================================================*/
typedef struct tagDALIST
{
    int32_t cur_object;
    int32_t alloc_size;
    int32_t free_size;
    void  *  mem;
} DALIST;

typedef DALIST * pDALIST;

/*====================================================================
**  LIST
**
**  This structure records the information about a list of objects.  The
**  objects can be of any size.  It supports multiple implementations
**  (array, linked list ...).
**
**  type        -   Type of implementation.  The implementation types
**                  supported are:
**
**                  LIST_TYPE_ARRAY
**                  LIST_TYPE_LINKED
**
**  object_size -   The size of a single object in the list.
**
**  nb_objects  -   The current number of objects in the list.
**
**  max_objects -   The maximum number of objects in the list.  If this
**                  number is 0, then there is no maximum other than the
**                  limit imposed by the size of the available memory.
**
**  l           -   This is a pointer to an implementation dependent structure
**                  that records the actual list of objects.
**                  Example: for an array implementation, start is a pointer
**                  to a block of memory of size (max_objects * object_size).
**                  For a linked list implementation, it could be a pointer
**                  to the first node in the list.
**
**====================================================================*/
typedef struct tagLIST
{
    uint16_t type;
    uint32_t object_size;
    int32_t nb_objects;
    int32_t max_objects;
    void   *l;
} LIST;

typedef LIST * pLIST;

/*====================================================================
**  STACK
**
**  This datatype records the information about a stack.  It is implemented
**  as a LIST.
**
**====================================================================*/
typedef LIST STACK;
typedef pLIST pSTACK;


/*====================================================================
** Prototypes
**====================================================================*/
#ifndef __STRUCTURE__

#if defined (__cplusplus)
extern "C" {
#endif

/*====================================================================
**  LIST API macros
**====================================================================*/
#define list_SCAN(l,p) for(p=list_first(l);p!=-1L;p=list_next(l))

/*====================================================================
**  LIST API functions
**====================================================================*/

/* list.c */
int8_t list_create_darray (LIST         *pList,
                                      int32_t      init_size,
                                      int32_t      alloc_size,
                                      int32_t      free_size,
                                      uint32_t      object_size);

int8_t list_destroy       (LIST         *pList);

int8_t list_append        (LIST         *pList,
                                      void        *object_ptr);

int8_t list_insert        (LIST         *pList,
                                      void        *object_ptr);

int8_t list_delete        (LIST         *pList);

int8_t list_get           (const LIST   *pList,
                                      void        *object);

int8_t list_set           (LIST         *pList,
                                      void        *object_ptr);

int8_t list_get_addr      (const LIST   *pList,
                                      void       **object_ptr);

int32_t list_position      (const LIST   *pList);

int32_t list_first         (LIST         *pList);

int32_t list_next          (LIST         *pList);

int32_t list_prev          (LIST         *pList);

int32_t list_last          (LIST         *pList);

int32_t list_goto          (LIST         *pList,
                                      int32_t      position);

int32_t list_count         (const LIST   *pList);

/* listfind.c */
int8_t list_find                 (LIST   *pList,
                                 void  *object_ptr,
                                 int   (*compare_func)(void *object1_ptr,
                                                       void *object2_ptr,
                                                       void *parms),
                                 void  *parms);

/* listsort.c */
int8_t list_insert_sorted       (LIST   *pList,
                                 void  *object_ptr,
                                 int   (*compare_func)(void *object1_ptr,
                                                       void *object2_ptr,
                                                       void *parms),
                                 void  *parms);

int8_t list_bsearch             (LIST   *pList,
                                 void  *object_ptr,
                                 int   (*compare_func)(void *object1_ptr,
                                                       void *object2_ptr,
                                                       void *parms),
                                 void  *parms);

#if defined (__cplusplus)
}
#endif

#endif /* __STRUCTURE__ */
#endif /* __HLIST_H__ */
