/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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
