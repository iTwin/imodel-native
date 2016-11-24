/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/PublicAPI/bcdtminlines.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__

/*
**   Size     Divider  Bit_Mask
**
**  131072      17      131071 
**   65536      16       65535
**   32768      15       32767
**   16384      14       16383
**    8192      13        8191 
**    4096      12        4091
**    2048      11        2047
**    1024      10        1023
**     512       9         511
**
*/    
#define NEW
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
__forceinline  BC_DTM_FEATURE * ftableAddrP(BC_DTM_OBJ *dtmP,long feature )
{
 return( dtmP->fTablePP[feature >> DTM_PARTITION_SHIFT_FEATURE ] + (feature & (DTM_PARTITION_SIZE_FEATURE - 1)) ) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
__forceinline  DPoint3d * pointAddrP(BC_DTM_OBJ *dtmP,long point )
{
 //    x = (dtmP->pointsP+offset)->x ;     old 
 //    x = pointAddrP(dtmP,offset)->x ;    new
 return( dtmP->pointsPP[point >> DTM_PARTITION_SHIFT_POINT ] + (point & (DTM_PARTITION_SIZE_POINT - 1))) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
__forceinline   DTM_TIN_NODE * nodeAddrP(BC_DTM_OBJ *dtmP,long node )
{
 return( dtmP->nodesPP[node >> DTM_PARTITION_SHIFT_NODE ] + ( node & (DTM_PARTITION_SIZE_NODE - 1) ) ) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
__forceinline   DTM_CIR_LIST * clistAddrP(BC_DTM_OBJ *dtmP,long clPtr )
{
 return( dtmP->cListPP[clPtr >> DTM_PARTITION_SHIFT_CLIST ] + ( clPtr & (DTM_PARTITION_SIZE_CLIST - 1)) ) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
__forceinline  DTM_FEATURE_LIST * flistAddrP(BC_DTM_OBJ *dtmP,long flPtr )
{
 return( dtmP->fListPP[flPtr >> DTM_PARTITION_SHIFT_FLIST ] + ( flPtr & (DTM_PARTITION_SIZE_FLIST - 1))) ;
}

__forceinline long GetNextHullPtr (BC_DTM_OBJ* dtmP, long hullPtr)
    {
    return nodeAddrP (dtmP, hullPtr)->hPtr;
    }

__forceinline long GetPointFeatureListPtr (BC_DTM_OBJ* dtmP, long point)
    {
    return nodeAddrP (dtmP, point)->fPtr;
    }

