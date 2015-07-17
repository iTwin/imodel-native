/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/2d/bcdtmInitialise.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "bcDTMBaseDef.h"
#include <direct.h>
#include <tchar.h>
#include <thread>

#include "dtmvars.h"
FILE    *fpLOG = NULL;

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmInitialise(void)
{
 long dbg=DTM_TRACE_VALUE(0) ;
 static long firstCall=1 ;
/*
** Only Process For The First Call
*/
 if( firstCall )
   {
    firstCall = 0 ;
/*
** Open Log File
*/
    bcdtmInitialise_openGpkDtmLogFile() ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Initialising DTM") ;
/*
** Set Value of DTM_PYE
*/
    DTM_PYE = atan2(0.0,-1.0) ;
    if( DTM_PYE < 0.0 ) DTM_PYE = -DTM_PYE ;
    DTM_2PYE = DTM_PYE * 2.0 ;
/*
** Set Number Of CPU Processors
*/
    DTM_NUM_PROCESSORS = std::thread::hardware_concurrency();
    if( DTM_NUM_PROCESSORS > DTM_MAX_PROCESSORS ) DTM_NUM_PROCESSORS = DTM_MAX_PROCESSORS ; 
    if( dbg ) bcdtmWrite_message(0,0,0,"DTM_NUM_PROCESSORS = %2ld",DTM_NUM_PROCESSORS) ; 
/*
** Set Floating Point Control Word To 64 Bit Precision
*/
//    bcdtmWrite_message(11,0,0,"FCW = 0x%.4x ",_control87(0,0)) ;
//
//    #ifdef winNT   
//    bcdtmWrite_message(0,0,0,"Set    I1 - FCW = 0x%.4x ",_control87(0,0)) ;
//    _control87(_PC_64,MCW_PC) ;  
//    bcdtmWrite_message(0,0,0,"64 Bit I2 - FCW = 0x%.4x ",_control87(0,0)) ;
//    _control87(_PC_53,MCW_PC) ;  
//    bcdtmWrite_message(0,0,0,"53 Bit I2 - FCW = 0x%.4x ",_control87(0,0)) ;
//    _control87(_PC_24,MCW_PC) ;  
//    bcdtmWrite_message(0,0,0,"24 Bit I2 - FCW = 0x%.4x ",_control87(0,0)) ;
//    #endif 
   } 
/*
** Job Completed
*/
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmInitialise_openGpkDtmLogFile(void)
{
 char  Dstr[50],Tstr[50] ;
 char   *logFileEnvP=NULL ;
/*
** Check For bcDTM Log File
*/
 if( fpLOG == NULL )
   {
    // Modified by Joe Roberts on 10/15/2008 for D-28436
	// This is the GEOPAK/PW problem caused by inclusion of Civil Accudraw, GPS, and Roundabouts 
	// into the GEOPAK install.  The inclusion caused the BCivilDTM.2.0.dll to load earlier - before PW is ready.  
	// The first thing the init function for the DTM dll did was to try to create a log file in PW.  
	// But PW wasn't ready, so the connection failed and because of the optimization of the connection function, 
	// it did not retry.  So as far as GEOPAK was concerned, PW was not active. The solution was since the dtm 
	// log file truly did not need to be created in PW, then don't do it.  Create the log file on the local disk.  
	// End of problem.
/*
**  Check Log File Environment Variable Exists
*/
    logFileEnvP = getenv("BCDTM_LOG_FILE"); 
/*
**  If Environment Variable Exists Try And Open Log File
*/ 
    if( logFileEnvP != NULL )
      { 
       if( ( fpLOG = fopen(logFileEnvP,"r")) != NULL )
         {
          fclose(fpLOG) ;
          fpLOG = fopen(logFileEnvP,"w") ;
          if( fpLOG != NULL )
            { 
             fprintf(fpLOG,"Bentley Civil DTM LOG FILE\n") ;
             fprintf(fpLOG,"==========================\n") ;
             fprintf(fpLOG,"SS3 BcDTM Server Running\n") ;
             bcdtmUtl_getDateAndTime(Dstr,Tstr) ;
             fprintf(fpLOG,"Start = %s %s\n",Dstr,Tstr) ;
             fflush(fpLOG) ;  
            }
         }
      }
   }
/*
** Job Completed
*/
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmInitialise_openLogFile(WCharCP LogFileName)
{
 char  Dstr[50],Tstr[50] ;
/*
** Check For bcDTM Log File And Close If Open
*/
 if( fpLOG != NULL )
   {
     fclose(fpLOG) ;
     fpLOG=NULL ;
   }
/*
** Open Application Log File
*/
 if( fpLOG == NULL )
   {
    if( ( fpLOG = bcdtmFile_open(LogFileName,L"w")) != NULL )
      {
       fprintf(fpLOG,"Bentley Civil DTM LOG FILE\n") ;
       fprintf(fpLOG,"==========================\n") ;
       fprintf(fpLOG,"bcDTM Server Running\n") ;
       bcdtmUtl_getDateAndTime(Dstr,Tstr) ;
       fprintf(fpLOG,"Start = %s %s\n",Dstr,Tstr) ;
       fflush(fpLOG) ;  
      }
   }
/*
** Job Completed
*/
 return(0) ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_EXPORT int bcdtmInitialise_closeLogFile(void)
{
 char  Dstr[50],Tstr[50] ;
/*
** Check For bcDTM Log File
*/
 if( fpLOG != NULL )
   {
    bcdtmUtl_getDateAndTime(Dstr,Tstr) ;
    fprintf(fpLOG,"End = %s %s\n",Dstr,Tstr) ;
    fflush(fpLOG) ;  
    fclose(fpLOG) ;
    fpLOG = NULL  ;
   }
/*
**  Job Completed
*/
 return(0) ;
}
