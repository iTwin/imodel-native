/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/2d/bcdtmDllMain.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
 int  bcdtmInitialise(void) ; 

 class DTMDll
     {
     public: DTMDll()
                 {
                 bcdtmInitialise() ;
                 }
     };

 DTMDll dllInst;
