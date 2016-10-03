/*--------------------------------------------------------------------------------------+
|
|     $Source: AutomaticGroundDetection/ScalableMeshGroundExtractor.cpp $
|    $RCSfile: ScalableMesh.cpp,v $
|   $Revision: 1.106 $
|       $Date: 2012/01/06 16:30:15 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
  
#include <ScalableMeshPCH.h>
#include "ScalableMeshGroundExtractor.h"
#include "ScalableMeshPointsProvider.h"

#include <TerrainModel\AutomaticGroundDetection\GroundDetectionMacros.h>
#include <TerrainModel\AutomaticGroundDetection\GroundDetectionManager.h>
#include <TerrainModel\AutomaticGroundDetection\IGroundDetectionServices.h>

USING_NAMESPACE_GROUND_DETECTION

/*----------------------------------------------+
| Constant definitions                          |
+----------------------------------------------*/
BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE


/*----------------------------------------------------------------------------+
|IScalableMeshGroundExtractor - Begin
+----------------------------------------------------------------------------*/
IScalableMeshGroundExtractorPtr IScalableMeshGroundExtractor::Create(IScalableMeshPtr& scalableMesh)
	{
	IScalableMeshGroundExtractorPtr groundExtractor(ScalableMeshGroundExtractor::Create(scalableMesh).get());
	return groundExtractor;
	}

StatusInt IScalableMeshGroundExtractor::ExtractAndEmbed()
	{
	return _ExtractAndEmbed();
	}		
/*----------------------------------------------------------------------------+
|IScalableMeshGroundExtractor Method Definition Section - End
+----------------------------------------------------------------------------*/
ScalableMeshGroundExtractorPtr ScalableMeshGroundExtractor::Create(IScalableMeshPtr& scalableMesh)
	{
	return new ScalableMeshGroundExtractor(scalableMesh);
	}

ScalableMeshGroundExtractor::ScalableMeshGroundExtractor(IScalableMeshPtr& scalableMesh)
	{
	m_scalableMesh = scalableMesh;
	}

ScalableMeshGroundExtractor::~ScalableMeshGroundExtractor()
	{
	}

StatusInt ScalableMeshGroundExtractor::_ExtractAndEmbed()
	{
	/*
	IGroundDetectionServices* serviceP(GroundDetectionManager::GetServices());

	bvector<DPoint3d> seedpoints;	
	GroundDetectionParametersPtr params;

	ScalableMeshPointsProviderCreatorPtr smPtsProviderCreator(ScalableMeshPointsProviderCreator::Create(m_scalableMesh));	

	IPointsProviderCreatorPtr ptsProviderCreator(smPtsProviderCreator.get()); 	
	params->SetPointsProviderCreator(ptsProviderCreator);        

	StatusInt status = serviceP->_GetSeedPointsFromTIN(seedpoints, *params.get());
	assert(status == SUCCESS);
	return status;
	*/
	return SUCCESS;

	}

END_BENTLEY_SCALABLEMESH_NAMESPACE
