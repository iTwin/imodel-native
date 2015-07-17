/*--------------------------------------------------------------------------------------+
|
|     $Source: Drainage/bcdtmDrainage.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

extern double DTM_PYE;
extern double DTM_2PYE;

//#include <bcBaseDefs.h>
#include <TerrainModel/Drainage/Drainage.h>
#include <TerrainModel/TerrainModel.h>


//#include "transformhelper.h"

#include "bcdtmDrainageTables.h"
#include "bcdtmDrainageTrace.h"
#include "bcdtmDrainagePond.h"
#include "bcdtmDrainageCatchment.h"
#include "bcdtmDrainageList.h"
#include "bcdtmDrainageUtility.h"
#include "bcdtmDrainageFeatures.h"

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/

class DTMDrainage
{
 public :
 
    DTMDrainage(void) 
       {
        m_pointCache = new DTMPointCache() ;
       }
      
    ~DTMDrainage()
       {
        m_pointCache->FreeCache() ;
       }  
 
 private :
 
    DTMPointCache*  m_pointCache ;
    
} ;   
