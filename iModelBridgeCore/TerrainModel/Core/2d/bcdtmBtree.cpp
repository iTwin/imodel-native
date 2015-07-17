/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/2d/bcdtmBtree.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "bcDTMBaseDef.h"
#include "dtmevars.h"
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmBtree_createBtree
(
 BC_DTM_BTREE  **btreePP ,
 long          numNodes  
)
{
 int ret=DTM_SUCCESS ;
 long nullNode = -9999999 ;
 BC_DTM_BTREE_NODE *nodeP ;
/*
** Allocate Memory For Btree Header
*/
 *btreePP = ( BC_DTM_BTREE * ) malloc (sizeof(BC_DTM_BTREE)) ;
 if( *btreePP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
/*
** Initialise Btree Header Variables
*/
 (*btreePP)->nullNode    = nullNode ; 
 (*btreePP)->headNode    = nullNode ; 
 (*btreePP)->activeNodes = 0 ; 
 (*btreePP)->memNodes    = 0 ; 
 (*btreePP)->newNode     = 0 ;
 (*btreePP)->delNodePtr  = nullNode  ;
 (*btreePP)->btreeNodesP = NULL ;
/*
** Allocate Memory For Btree Nodes
*/
 (*btreePP)->btreeNodesP = ( BC_DTM_BTREE_NODE * ) malloc(numNodes*sizeof(BC_DTM_BTREE_NODE)) ;
 if( (*btreePP)->btreeNodesP == NULL )
   {
    bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
    goto errexit ;
   }
 (*btreePP)->memNodes = numNodes ; 
/*
** Initialise Node Values
*/
 for( nodeP = (*btreePP)->btreeNodesP ; nodeP < (*btreePP)->btreeNodesP + (*btreePP)->memNodes ;  ++nodeP )
   {
    nodeP->leftNode  = nullNode ;
    nodeP->rightNode = nullNode ;
    nodeP->priorNode = nullNode ;
    nodeP->dtmP      = NULL     ;
   }
/*
** Clean Up
*/
 cleanup :
/*
** Return
*/
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmBtree_destroyBtree
(
 BC_DTM_BTREE  **btreePP
)
{
/*
** Only Destroy Btree If It Exists
*/
 if( *btreePP != NULL )
   {
    if( (*btreePP)->btreeNodesP != NULL ) free((*btreePP)->btreeNodesP) ;
    free(*btreePP) ;
    *btreePP = NULL ;
   }
/*
** Return
*/
 return(DTM_SUCCESS) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmBtree_addNode
(
 BC_DTM_BTREE  *btreeP ,
 BC_DTM_OBJ    *dtmP  
)
{
 int ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long priorNode,nextNode,newNode ;
 long numActiveObjects;
 unsigned long totMemoryUsage ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Adding Node To B Tree ** btreeP = %p dtmP = %p",btreeP,dtmP) ;
/*
** Scan Btree To Entry Point
*/
 nextNode = btreeP->nullNode ;
 if( ( priorNode = btreeP->headNode ) != btreeP->nullNode )
   {
    do
      {
       if     ( dtmP <  (btreeP->btreeNodesP+priorNode)->dtmP ) nextNode = (btreeP->btreeNodesP+priorNode)->leftNode ;
       else if( dtmP >  (btreeP->btreeNodesP+priorNode)->dtmP ) nextNode = (btreeP->btreeNodesP+priorNode)->rightNode ;
       else
         {
          bcdtmWrite_message(0,0,0,"B Tree Node Entry Already Exists") ;
          if( dbg )
            {
             bcdtmObject_reportActiveDtmObjects(1,&numActiveObjects,&totMemoryUsage) ;
            }
          goto errexit ;
         }  
       if( nextNode != btreeP->nullNode ) priorNode = nextNode ; 
      } while( nextNode != btreeP->nullNode ) ;
   }
/*
** Check For Vacant Node ( Prior Deleted Node Entry )
*/
  if( btreeP->delNodePtr != btreeP->nullNode )
    {
     newNode = btreeP->delNodePtr ;
     btreeP->delNodePtr = (btreeP->btreeNodesP+newNode)->leftNode ;
    }
/*
** Allocate New Node
*/
  else
    {
     newNode = btreeP->newNode ;
     if( newNode >= btreeP->memNodes ) 
       { 
        bcdtmWrite_message(0,0,0,"B Tree Entries Exceeded") ;
        goto errexit ;
       }
     ++btreeP->newNode ;
    } 
/*
** Store New Node Entry
*/
 if( priorNode == btreeP->nullNode )
   {
    (btreeP->btreeNodesP+newNode)->dtmP = dtmP ;
    (btreeP->btreeNodesP+newNode)->priorNode = btreeP->nullNode ;
    (btreeP->btreeNodesP+newNode)->leftNode  = btreeP->nullNode ;
    (btreeP->btreeNodesP+newNode)->rightNode = btreeP->nullNode ;
    btreeP->headNode = newNode ;
    ++btreeP->activeNodes ;
   }
 else
   {
    (btreeP->btreeNodesP+newNode)->dtmP = dtmP ;
    (btreeP->btreeNodesP+newNode)->priorNode = priorNode ;
    (btreeP->btreeNodesP+newNode)->leftNode  = btreeP->nullNode ;
    (btreeP->btreeNodesP+newNode)->rightNode = btreeP->nullNode ;
    if ( dtmP <  (btreeP->btreeNodesP+priorNode)->dtmP ) (btreeP->btreeNodesP+priorNode)->leftNode  = newNode ;
    else                                                           (btreeP->btreeNodesP+priorNode)->rightNode = newNode ;
    ++btreeP->activeNodes ;
   } 
/*
** Clean Up
*/
 cleanup :
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Adding Node To B Tree Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Adding Node To B Tree Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmBtree_findNode
(
 BC_DTM_BTREE  *btreeP ,
 BC_DTM_OBJ    *dtmP,  
 long          *nodeP,
 long          *priorNodeP,
 long          *nodeFoundP,
 long          *nodeLevelP 
)
{
 int ret=DTM_SUCCESS ;
/*
** Initialise
*/
 *nodeP      = btreeP->headNode ;
 *priorNodeP = btreeP->nullNode ;
 *nodeFoundP = FALSE ;
 *nodeLevelP = btreeP->nullNode ;
/*
** Validate
*/
 if( btreeP == NULL ) goto errexit ;
/*
** Scan Btree To Node Entry 
*/
 if( *nodeP != btreeP->nullNode )
   {
    *nodeLevelP = 0 ;
    do
      {
       if( dtmP == (btreeP->btreeNodesP+*nodeP)->dtmP ) *nodeFoundP = TRUE ;
       else
         {
          ++*nodeLevelP  ;
          *priorNodeP = *nodeP ;
          if     ( dtmP < (btreeP->btreeNodesP+*nodeP)->dtmP ) *nodeP = (btreeP->btreeNodesP+*nodeP)->leftNode  ;
          else if( dtmP > (btreeP->btreeNodesP+*nodeP)->dtmP ) *nodeP = (btreeP->btreeNodesP+*nodeP)->rightNode ;
         }
      } while( *nodeFoundP == FALSE && *nodeP != btreeP->nullNode ) ;
   }
/*
** Check Entry Found
*/
  if( *nodeFoundP == FALSE )
    {
     *nodeP      = btreeP->nullNode ;
     *priorNodeP = btreeP->nullNode ;
     *nodeLevelP = btreeP->nullNode ;
    }
/*
** Clean Up
*/
 cleanup :
/*
** Return
*/
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmBtree_removeNode
(
 BC_DTM_BTREE  *btreeP ,
 long          node
)
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long priorNode,leftNode,rightNode,priorLeftNode,priorRightNode,smallNode ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Removing Node From B Tree ** btreeP = %p node = %8ld",btreeP,node) ;
/*
** Validate
*/
 if( btreeP == NULL ) goto errexit ;
/*
** Set Node Values
*/
 leftNode  = (btreeP->btreeNodesP+node)->leftNode ;
 rightNode = (btreeP->btreeNodesP+node)->rightNode ;
 priorNode = (btreeP->btreeNodesP+node)->priorNode ;
 priorLeftNode  = btreeP->nullNode ;
 priorRightNode = btreeP->nullNode ;
 if( priorNode != btreeP->nullNode )
   {
    priorLeftNode  = (btreeP->btreeNodesP+priorNode)->leftNode ;
    priorRightNode = (btreeP->btreeNodesP+priorNode)->rightNode ;
   }
/*
**  Remove Head Node
*/
 if( priorNode == btreeP->nullNode )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Removing Head Node") ;
    if     ( leftNode == btreeP->nullNode && rightNode == btreeP->nullNode ) 
      {
       btreeP->headNode = btreeP->nullNode ;
      }
    else if( leftNode != btreeP->nullNode && rightNode == btreeP->nullNode ) 
      { 
       btreeP->headNode = leftNode  ; 
       (btreeP->btreeNodesP+leftNode)->priorNode  = btreeP->nullNode ; 
      }
    else if( leftNode == btreeP->nullNode && rightNode != btreeP->nullNode ) 
      { 
       btreeP->headNode = rightNode ; 
       (btreeP->btreeNodesP+rightNode)->priorNode = btreeP->nullNode ;
      }
    else
      {
/*
**     Reset Head Node
*/
       btreeP->headNode = rightNode ;      
       (btreeP->btreeNodesP+rightNode)->priorNode = btreeP->nullNode ;
/*
**     Scan To Smallest Value Node On Right Side
*/
       smallNode = rightNode ;
       while ( (btreeP->btreeNodesP+smallNode)->leftNode != btreeP->nullNode  )
         {
          smallNode = (btreeP->btreeNodesP+smallNode)->leftNode ;
         }
/*
**     Insert Link From Small Node To Left Node
*/
       (btreeP->btreeNodesP+smallNode)->leftNode = leftNode ;       
       (btreeP->btreeNodesP+leftNode)->priorNode = smallNode ;       
      } 
   }
/*
**  Remove Tree Node
*/
 else
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Removing Tree Node") ;
    if     ( leftNode == btreeP->nullNode && rightNode == btreeP->nullNode ) 
      {
       if( priorLeftNode  == node ) (btreeP->btreeNodesP+priorNode)->leftNode  = btreeP->nullNode ;
       if( priorRightNode == node ) (btreeP->btreeNodesP+priorNode)->rightNode = btreeP->nullNode ;
      } 
    else if( leftNode != btreeP->nullNode && rightNode == btreeP->nullNode ) 
      {
       if( priorLeftNode  == node ) (btreeP->btreeNodesP+priorNode)->leftNode  = leftNode  ;       
       if( priorRightNode == node ) (btreeP->btreeNodesP+priorNode)->rightNode = leftNode  ;       
       (btreeP->btreeNodesP+leftNode)->priorNode  = priorNode ; 
      }
    else if( leftNode == btreeP->nullNode && rightNode != btreeP->nullNode ) 
      {
       if( priorLeftNode  == node ) (btreeP->btreeNodesP+priorNode)->leftNode  = rightNode  ;       
       if( priorRightNode == node ) (btreeP->btreeNodesP+priorNode)->rightNode = rightNode  ;       
       (btreeP->btreeNodesP+rightNode)->priorNode = priorNode ; 
      }
    else
      {
/*
**     Scan To Smallest Value Node From Right Side Of Node
*/
       smallNode = rightNode ;
       while ( (btreeP->btreeNodesP+smallNode)->leftNode != btreeP->nullNode  )
         {
          smallNode = (btreeP->btreeNodesP+smallNode)->leftNode ;
         }
/*
**     Insert Link From Small Node To Left Node
*/
       (btreeP->btreeNodesP+smallNode)->leftNode  = leftNode  ;       
       (btreeP->btreeNodesP+leftNode)->priorNode  = smallNode ;       
/*
**     Insert Link From Prior Node To Right Node
*/
       if( priorLeftNode  == node ) (btreeP->btreeNodesP+priorNode)->leftNode  = rightNode  ;       
       if( priorRightNode == node ) (btreeP->btreeNodesP+priorNode)->rightNode = rightNode  ;       
       (btreeP->btreeNodesP+rightNode)->priorNode = priorNode ;       
      }
   }
/*
** Add Removed Node To Empty List For Latter Re Use
*/
 (btreeP->btreeNodesP+node)->leftNode = btreeP->delNodePtr ;
 btreeP->delNodePtr = node ;
/*
** Decrement Number Of Active Nodes
*/
 --btreeP->activeNodes ;
/*
** Clean Up
*/
 cleanup :
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Removing Node From B Tree Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Removing Node From B Tree Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmBtree_getArrayOfNodeValues
(
 BC_DTM_BTREE  *btreeP ,
 BC_DTM_OBJ    ***nodeDtmPPP,
 long          *numNodeValuesP
)
{
 int  ret=DTM_SUCCESS,dbg=DTM_TRACE_VALUE(0) ;
 long node,priorNode,numNodes,scanCompleted=FALSE ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Getting An Array Of Node Values From B Tree") ;
/*
** Initialise
*/
 *numNodeValuesP = 0 ;
 if( *nodeDtmPPP != NULL ) { free(*nodeDtmPPP) ; *nodeDtmPPP = NULL ; }
/*
** Check For Nodes In Btree
*/
 if( btreeP->activeNodes )
   {
/*
**  Allocate Memory For Node Values
*/  
    *numNodeValuesP = btreeP->activeNodes ;
    *nodeDtmPPP = (BC_DTM_OBJ ** ) malloc ( *numNodeValuesP * sizeof( BC_DTM_OBJ * )) ;
    if( *nodeDtmPPP == NULL ) 
      {
       bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
       goto errexit ;
      }
/*
**  Scan Btree And Store Node Values In Array
*/
    numNodes = 0 ;
    node = btreeP->headNode ;
    do
      { 
/*
**     Scan Down Left Side Of B Tree From Node
*/
       while ( (btreeP->btreeNodesP+node)->leftNode != btreeP->nullNode ) node = (btreeP->btreeNodesP+node)->leftNode ;
/*
**     Store Node Value
*/ 
       *(*nodeDtmPPP+numNodes) = (btreeP->btreeNodesP+node)->dtmP   ;
       ++numNodes ;
/*
**     Scan Up Right Side Of B Tree From Node
*/
       while( scanCompleted == FALSE && (btreeP->btreeNodesP+node)->rightNode == btreeP->nullNode  )
         {
          priorNode = (btreeP->btreeNodesP+node)->priorNode ;
          while ( node != btreeP->headNode && (btreeP->btreeNodesP+priorNode)->rightNode == node )
            {
             node = priorNode ;
             priorNode = (btreeP->btreeNodesP+node)->priorNode ;
            }
/*
**        Check For Head Node - Scan Completed
*/ 
          if( node == btreeP->headNode ) scanCompleted = TRUE ;
/*
**        Add Node Value To Array
*/
          else
            { 
             node = priorNode ;
             *(*nodeDtmPPP+numNodes) = (btreeP->btreeNodesP+node)->dtmP   ;
             ++numNodes ;
            }
         }
/*
**     Set Next Down Scan Node To Right Node
*/
       node = (btreeP->btreeNodesP+node)->rightNode ;

      }  while ( scanCompleted == FALSE )  ;
   }
/*
** Clean Up
*/
 cleanup :
/*
** Return
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Getting An Array Of Node Values From B Tree Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Getting An Array Of Node Values From B Tree Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 *numNodeValuesP = 0 ;
 if( *nodeDtmPPP != NULL ) { free(*nodeDtmPPP) ; *nodeDtmPPP = NULL ; }
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}