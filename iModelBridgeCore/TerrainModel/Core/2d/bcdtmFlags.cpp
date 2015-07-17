/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/2d/bcdtmFlags.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "bcDTMBaseDef.h"
#include "dtmevars.h"
#include "bcdtminlines.h" 

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmFlag_setFlag(unsigned char *Flag, long Offset )
{
 long Byte,Bit ;
 char mask=0x1   ;
 Bit  = Offset % 8 ;
 Byte = Offset / 8 ;
 mask = mask << ( char ) Bit ;
 *(Flag+Byte) = *(Flag+Byte) | mask ;
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmFlag_clearFlag(unsigned char *Flag, long Offset )
{
 long Byte,Bit ;
 unsigned char mask=0x1 ;
 if( bcdtmFlag_testFlag(Flag,Offset ))
   {
    Bit  = Offset % 8 ;
    Byte = Offset / 8 ;
    mask = mask << ( char ) Bit ;
    *(Flag+Byte) = *(Flag+Byte) ^ mask ;    // XOR 
   }
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmFlag_testFlag(unsigned char *Flag, long Offset )
{
 long Byte,Bit ;
 char mask=0x1,mbyte=0   ;
 Byte = Offset / 8 ;
 Bit  = Offset % 8 ;
 mask = mask << ( char ) Bit ;
 mbyte = *(Flag+Byte) & mask ;
 if( mbyte == 0 ) return(0) ;
 else             return(1) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmFlag_setVoidBitPCWD(unsigned short *Pcwd )
{
 char mask=0x1,*Flag ;
 Flag = ( char * ) Pcwd ; 
 *Flag = *Flag | mask ;
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmFlag_clearVoidBitPCWD(unsigned short *Pcwd)
{
 unsigned char mask=0xFE,*Flag ;
 Flag = ( unsigned char * ) Pcwd ; 
 *Flag = *Flag & mask ;
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmFlag_testVoidBitPCWD(unsigned short *Pcwd )
{
 char mask=0x1,mbyte=0,*Flag  ;
 Flag = ( char * ) Pcwd ; 
 mbyte = *Flag & mask ;
 if( mbyte == 0 ) return(0) ;
 else             return(1) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmFlag_setDeletePointBitPCWD(unsigned short *Pcwd )
{
 char mask=0x2,*Flag ;
 Flag = ( char * ) Pcwd ; 
 *Flag = *Flag | mask ;
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmFlag_clearDeletePointBitPCWD(unsigned short *Pcwd)
{
 unsigned char mask=0xFD,*Flag ;
 Flag = ( unsigned char * ) Pcwd ; 
 *Flag = *Flag & mask ;
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmFlag_testDeletePointBitPCWD(unsigned short *Pcwd )
{
 char mask=0x2,mbyte=0,*Flag  ;
 Flag = ( char * ) Pcwd ; 
 mbyte = *Flag & mask ;
 if( mbyte == 0 ) return(0) ;
 else             return(1) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmFlag_setBitPCWD(unsigned short *Pcwd,long Offset )
{
 long Byte,Bit ;
 char mask=0x1,*Flag ;
 Flag = ( char * ) Pcwd ; 
 Bit  = Offset % 8 ;
 Byte = Offset / 8 ;
 mask = mask << ( char) Bit ;
 *(Flag+Byte) = *(Flag+Byte) | mask ;
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmFlag_clearBitPCWD(unsigned short *Pcwd,long Offset)
{
 long Byte,Bit ;
 unsigned char mask=0xFE,*Flag ;

 Flag = ( unsigned char * ) Pcwd ; 
 Bit  = Offset % 8 ;
 Byte = Offset / 8 ;
 mask = mask <<( unsigned char ) Bit ;
 *(Flag+Byte) = *(Flag+Byte) & mask ;
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmFlag_testBitPCWD(unsigned short *Pcwd,long Offset )
{
 long Byte,Bit ;
 char mask=0x1,mbyte=0,*Flag  ;
 Flag = ( char * ) Pcwd ; 
 Byte = Offset / 8 ;
 Bit  = Offset % 8 ;
 mask = mask << ( char ) Bit ;
 mbyte = *(Flag+Byte) & mask ;
 if( mbyte == 0 ) return(0) ;
 else             return(1) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmFlag_setInsertPoint(BC_DTM_OBJ *dtmP,long point )
{
/*
** Sets The 2ND Bit Of PCWD
*/
 unsigned char mask=0x2,*flagP ;
 flagP = ( unsigned char * ) nodeAddrP(dtmP,point) ; 
 *flagP = *flagP | mask ;
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmFlag_clearInsertPoint(BC_DTM_OBJ *dtmP,long point)
{
/*
** Clears The 2ND Bit Of PCWD
*/
 unsigned char mask=0xFD,*flagP ;
 flagP = ( unsigned char * ) nodeAddrP(dtmP,point) ; 
 *flagP = *flagP & mask ;
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Public int bcdtmFlag_testInsertPoint(BC_DTM_OBJ *dtmP,long point )
{
/*
** Tests The 2ND Bit Of PCWD
*/
 unsigned char mask=0x2,mbyte=0,*flagP  ;
 flagP = ( unsigned char * ) nodeAddrP(dtmP,point) ; 
 mbyte = *flagP & mask ;
 if( mbyte == 0 ) return(0) ;
 else             return(1) ;
}
