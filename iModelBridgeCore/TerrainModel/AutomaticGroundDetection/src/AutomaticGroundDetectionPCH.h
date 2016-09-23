/*--------------------------------------------------------------------------------------+
|
|     $Source: AutomaticGroundDetection/src/AutomaticGroundDetectionPCH.h $
|    $RCSfile: stdafx.h,v $
|   $Revision: 1.11 $
|       $Date: 2012/07/25 14:13:37 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <amp.h> 
#include <amp_math.h>
#include <thread>
#include <atomic>
#include <mutex> 
#include <excpt.h> 
#include <map>


/*
#include <Bentley\Bentley.h>
#include <Bentley\Bentley.r.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM


/*----------------------------------------------------------------------+
| Include TerrainModel general header files                             |
+----------------------------------------------------------------------*/
#include <TerrainModel/TerrainModel.h>
#include <TerrainModel/Core/bcDTMBaseDef.h>
#include <TerrainModel/Core/bcDTMClass.h>
#include <TerrainModel/Core/bcdtminlines.h>
#include <TerrainModel\Formats\InroadsImporter.h>


