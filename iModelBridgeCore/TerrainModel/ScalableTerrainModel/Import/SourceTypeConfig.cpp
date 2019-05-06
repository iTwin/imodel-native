/*--------------------------------------------------------------------------------------+
|    $RCSfile: SourceTypeConfig.cpp,v $
|   $Revision: 1.2 $
|       $Date: 2011/04/20 19:39:03 $
|     $Author: Raymond.Gauthier $
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <ScalableTerrainModelPCH.h>

#include <ScalableTerrainModel/Import/Config/SourceTypeConfig.h>
#include <ScalableTerrainModel/Import/ConfigVisitor.h>

BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                    Raymond.Gauthier  04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SourceTypeConfig::SourceTypeConfig     (UInt                layerID,
                                        const DataType&     type)
    :   m_layerID(layerID),
        m_type(type)
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                    Raymond.Gauthier  04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void SourceTypeConfig::_Accept (ConfigVisitor& visitor) const
    {
    visitor.Visit(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                    Raymond.Gauthier  04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ConfigComponent SourceTypeConfig::Create   (UInt            layerID,
                                            const DataType& type)
    {
    return CreateFromBase(new SourceTypeConfig(layerID, type));
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                    Raymond.Gauthier  04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
UInt SourceTypeConfig::GetLayerID () const
    {
    return m_layerID;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                    Raymond.Gauthier  04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const DataType& SourceTypeConfig::GetType () const
    {
    return m_type;
    }



END_BENTLEY_MRDTM_IMPORT_NAMESPACE