/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshInfo.cpp $
|    $RCSfile: ScalableMesh.cpp,v $
|   $Revision: 1.106 $
|       $Date: 2012/01/06 16:30:15 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
  
#include <ScalableMeshPCH.h>
#include "ScalableMeshInfo.h"


BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

/*----------------------------------------------------------------------------+
|IScalableMeshTextureInfo Method Definition Section - Begin
+----------------------------------------------------------------------------*/
SMTextureType IScalableMeshTextureInfo::GetTextureType() const
    {
    return _GetTextureType();
    }

bool IScalableMeshTextureInfo::IsUsingBingMap() const
    {
    return _IsUsingBingMap();
    }

/*----------------------------------------------------------------------------+
|IScalableMeshTextureInfo Method Definition Section - End
+----------------------------------------------------------------------------*/
ScalableMeshTextureInfo::ScalableMeshTextureInfo(SMTextureType textureType, bool isUsingBingMap)
    {
    m_textureType = textureType;
    m_isUsingBingMap = isUsingBingMap;
    }

SMTextureType ScalableMeshTextureInfo::_GetTextureType() const
    {
    return m_textureType;
    }

bool ScalableMeshTextureInfo::_IsUsingBingMap() const
    {
    return m_isUsingBingMap; 
    }        

END_BENTLEY_SCALABLEMESH_NAMESPACE
