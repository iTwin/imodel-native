/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "bcDTMBaseDef.h"
#include "DTMEvars.h"
int  bcdtmInitialise(void) ; 

 class DTMDll
     {
     public: DTMDll()
                 {
                 bcdtmInitialise() ;
                 }
             ~DTMDll()
                 {
                 if (glbDtmObjBtreeP != nullptr)
                     {
                     bcdtmBtree_destroyBtree(&glbDtmObjBtreeP);
                     }
                }
     };

 DTMDll dllInst;
