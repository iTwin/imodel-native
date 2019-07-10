//:>--------------------------------------------------------------------------------------+
//:>
//:>    $RCSfile: ScalableMeshQuadTreeBCLIBFilters.hpp,v $
//:>   $Revision: 1.28 $
//:>       $Date: 2011/04/27 17:17:56 $
//:>     $Author: Alain.Robert $
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
#include <windows.h> //for showing info.
#include <ImagePP/all/h/HFCException.h>

#include "ScalableMesh\ScalableMeshGraph.h"
#include "CGALEdgeCollapse.h"
#include "ScalableMeshMesher.h"
#include "LogUtils.h"

//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================

// ScalableMeshQuadTreeBCLIBFilter1 Class

//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================
//=====================================================================================================================


//If a metric cannot be computed, set the metric to a very large number. This 
//will ensure that points in such leaf node are used (i.e. : always greater than the 
//minimum metric threshold) and that points in such parent node are not used. 
//Note that HDOUBLE_MAX is not used to avoid overflow when computing the metric depending 
//on the current root to view transformation matrix.
#ifndef DEFAULT_VIEW_DEPENDENT_METRIC 
    #define DEFAULT_VIEW_DEPENDENT_METRIC HFLOAT_MAX
#endif


/**----------------------------------------------------------------------------
 Initiates a filtering of the node. Ther filtering process
 will compute the sub-resolution and the view oriented parameters.
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> bool ScalableMeshQuadTreeBCLIBFilter1<POINT, EXTENT>::Filter(
                                    HFCPtr<SMPointIndexNode<POINT, EXTENT> > parentNode, 
                                    std::vector<HFCPtr<SMPointIndexNode<POINT, EXTENT> >>& subNodes,
                                    size_t numSubNodes) const

    {
    // Compute the number of points in sub-nodes
    size_t totalNumberOfPoints = 0;
    for (size_t indexNodes = 0; indexNodes < numSubNodes; indexNodes++)
        {
        if (subNodes[indexNodes] != NULL)
            {
            RefCountedPtr<SMMemoryPoolVectorItem<POINT>> pointsPtr(subNodes[indexNodes]->GetPointsPtr());
            totalNumberOfPoints += pointsPtr->size();
            }
        }

    if (totalNumberOfPoints < 10)
    {
        // There are far too few points to start decimating them towards the root.
        // We then promote then all so they are given a high importance to make sure some terrain
        // representativity is retained in this area.
        for (size_t indexNodes = 0; indexNodes < numSubNodes ; indexNodes++)
        {
            if (subNodes[indexNodes] != NULL)
                {                                
                RefCountedPtr<SMMemoryPoolVectorItem<POINT>> subNodePointsPtr(subNodes[indexNodes]->GetPointsPtr());
                RefCountedPtr<SMMemoryPoolVectorItem<POINT>> parentPointsPtr(parentNode->GetPointsPtr());
                parentPointsPtr->push_back(&(*subNodePointsPtr)[0], subNodePointsPtr->size());                                    
                }
        }
    }
    else
    {    
        size_t pointArrayInitialNumber[8];
        RefCountedPtr<SMMemoryPoolVectorItem<POINT>> parentPointsPtr(parentNode->GetPointsPtr());
        parentPointsPtr->reserve (parentPointsPtr->size() + (totalNumberOfPoints * 1 /8) + 20);
        for (size_t indexNodes = 0; indexNodes < numSubNodes ; indexNodes++)
        {
            if (subNodes[indexNodes] != NULL)
            {
                // The value of 10 here is required. The alternative path use integer division (*3/4 +1) that will take all points anyway
                // In reality starting at 9 not all points are used but let's gives us a little margin.
                RefCountedPtr<SMMemoryPoolVectorItem<POINT>> subNodePointsPtr(subNodes[indexNodes]->GetPointsPtr());

                if (subNodePointsPtr->size() <= 10)
                    {
                    // Too few content in node ... promote them all                                        
                    parentPointsPtr = parentNode->GetPointsPtr();
                    parentPointsPtr->push_back(&(*subNodePointsPtr)[0], subNodePointsPtr->size());                           
                    }
                else
                    {
                    pointArrayInitialNumber[indexNodes] = subNodePointsPtr->size();

                    // Randomize the node content                                        
                    subNodePointsPtr->random_shuffle();
                                                                   
                    size_t indexStart = (subNodePointsPtr->size() * 7 / 8) + 1;
                    HASSERT ((indexStart > 0) && (indexStart <= (subNodePointsPtr->size() - 1)));
                    
                    parentPointsPtr = parentNode->GetPointsPtr();
                    parentPointsPtr->push_back(&(*subNodePointsPtr)[indexStart], subNodePointsPtr->size() - 1);                                               
                }
            }
        }
    }

    return true;
    }

/**----------------------------------------------------------------------------
 Indicates if the filtering is progressinve or not.
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> bool ScalableMeshQuadTreeBCLIBProgressiveFilter1<POINT, EXTENT>::IsProgressiveFilter() const
{
    return true;
}



/**----------------------------------------------------------------------------
 Initiates a filtering of the node. Ther filtering process
 will compute the sub-resolution and the view oriented parameters.
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> bool ScalableMeshQuadTreeBCLIBProgressiveFilter1<POINT, EXTENT>::Filter(
                                    HFCPtr<SMPointIndexNode<POINT, EXTENT> > parentNode, 
                                    std::vector<HFCPtr<SMPointIndexNode<POINT, EXTENT> >>& subNodes,
                                    size_t numSubNodes) const

{
#if 0 //Currently deactivated, could be useful for handling point cloud with ScalableMesh technology.
    // Compute the number of points in sub-nodes
    size_t totalNumberOfPoints = 0;
    for (size_t indexNodes = 0; indexNodes < numSubNodes; indexNodes++)
    {
        if (subNodes[indexNodes] != NULL)
        {
            totalNumberOfPoints += subNodes[indexNodes]->size();
        }
    }

    if (totalNumberOfPoints < 10)
    {
        // There are far too few points to start decimating them towards the root.
        // We then promote then all so they are given a high importance to make sure some terrain
        // representativity is retained in this area.
        for (size_t indexNodes = 0; indexNodes < numSubNodes ; indexNodes++)
        {
            if (subNodes[indexNodes] != NULL)
            {                
                size_t initialNodeCount = subNodes[indexNodes]->size();
                parentNode->push_back(subNodes[indexNodes]);
                subNodes[indexNodes]->m_nodeHeader.m_totalCount -= initialNodeCount;
                subNodes[indexNodes]->clear();                
            }
        }
    }
    else
    {

    
#if (1)
        size_t pointArrayInitialNumber[8];
        parentNode->reserve (parentNode->size() + (totalNumberOfPoints * 3 /4) + 20);
        for (size_t indexNodes = 0; indexNodes < numSubNodes ; indexNodes++)
        {
            if (subNodes[indexNodes] != NULL)
            {
                // The value of 10 here is required. The alternative path use integer division (*3/4 +1) that will take all points anyway
                // In reality starting at 9 not all points are used but let's gives us a little margin.
                if (subNodes[indexNodes]->size() <= 10)
                {
                    // Too few content in node ... promote them all
                    size_t initialNodeCount = subNodes[indexNodes]->size();
                    parentNode->push_back (subNodes[indexNodes]);
                    subNodes[indexNodes]->m_nodeHeader.m_totalCount -= initialNodeCount;
                    subNodes[indexNodes]->clear();
                }
                else
                {
                    pointArrayInitialNumber[indexNodes] = subNodes[indexNodes]->size();

                    // Randomize the node content
                    subNodes[indexNodes]->random_shuffle();
                                                                   
                    size_t indexStart = (subNodes[indexNodes]->size() * 3 / 4) + 1;
                    HASSERT ((indexStart > 0) && (indexStart <= (subNodes[indexNodes]->size() - 1)));

                    parentNode->push_back(subNodes[indexNodes], indexStart, subNodes[indexNodes]->size() - 1);
                    subNodes[indexNodes]->clearFrom (indexStart);
                    subNodes[indexNodes]->m_nodeHeader.m_totalCount -= pointArrayInitialNumber[indexNodes] - subNodes[indexNodes]->size();
                }
            }
        }

#else

        POINT* pointArray[8];
        POINT* pointArrayPromoted = new POINT[totalNumberOfPoints / 4 + 10];
        size_t pointArrayPromotedNumber = 0;
        size_t pointArrayNumber[8];
        size_t pointArrayInitialNumber[8];

        for (size_t indexNodes = 0; indexNodes < numSubNodes ; indexNodes++)
        {
            pointArray[indexNodes] = NULL;
            if (subNodes[indexNodes] != NULL)
            {
                pointArray[indexNodes] = new POINT[subNodes[indexNodes]->size()];
                pointArrayNumber[indexNodes] = 0;
                pointArrayInitialNumber[indexNodes] = subNodes[indexNodes]->size();
            }
        }


        for (size_t indexNodes = 0; indexNodes < numSubNodes ; indexNodes++)
        {
            if (subNodes[indexNodes] != NULL)
            {
                size_t numPoints = subNodes[indexNodes]->size();
                POINT* INPointArray = new POINT[numPoints];
                subNodes[indexNodes]->get(INPointArray, numPoints);

                for (size_t indexPoint = 0; indexPoint < numPoints  ; indexPoint++)
                {
                    if ((indexPoint % 4) == 0)
                    {
                        pointArrayPromoted[pointArrayPromotedNumber] = INPointArray[indexPoint];
                        pointArrayPromotedNumber++;
                    }
                    else
                    {
                        pointArray[indexNodes][pointArrayNumber[indexNodes]] = INPointArray[indexPoint];
                        (pointArrayNumber[indexNodes])++;
                    }
                }
                delete [] INPointArray;
            }
        }

        // Although doubtful, if there are already points in the parent node then we should assume 
        // they were previously promoted somehow and should be retained.

        for (size_t indexNodes = 0; indexNodes < numSubNodes ; indexNodes++)
        {
            if (subNodes[indexNodes] != NULL)
            {
                subNodes[indexNodes]->clear();
                subNodes[indexNodes]->reserve(pointArrayNumber[indexNodes] + 1);
                subNodes[indexNodes]->push_back(pointArray[indexNodes], pointArrayNumber[indexNodes]);
                subNodes[indexNodes]->m_nodeHeader.m_totalCount -= pointArrayInitialNumber[indexNodes] - subNodes[indexNodes]->size();

            }
        }

        parentNode->push_back(pointArrayPromoted, pointArrayPromotedNumber);
        

        delete [] pointArrayPromoted;

        for (size_t indexNodes = 0; indexNodes < numSubNodes ; indexNodes++)
        {
            if (subNodes[indexNodes] != NULL)
            {
                delete [] pointArray[indexNodes];
            }
        }
#endif
    }
#endif
    return true;

    }


/**----------------------------------------------------------------------------
 Initiates a filtering of the node. Ther filtering process
 will compute the sub-resolution and the view oriented parameters.
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> bool ScalableMeshQuadTreeBCLIBMeshFilter1<POINT, EXTENT>::Filter(
	HFCPtr<SMPointIndexNode<POINT, EXTENT> > parentNode,
	std::vector<HFCPtr<SMPointIndexNode<POINT, EXTENT> >>& subNodes,
	size_t numSubNodes) const

{

	HFCPtr<SMMeshIndexNode<POINT, EXTENT> > pParentMeshNode = dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(parentNode);

	RefCountedPtr<SMMemoryPoolVectorItem<POINT>> parentPointsPtr(parentNode->GetPointsPtr());
    parentPointsPtr->clear();

	// Compute the number of points in sub-nodes
	size_t totalNumberOfPoints = 0;
	bvector<bvector<DPoint3d>> polylines;
	bvector<DTMFeatureType> types;
	bvector<bool> anyHull(numSubNodes, false);
	bvector<int> beginIdx(numSubNodes, -1);
    std::set<DPoint3d, DPoint3dZYXTolerancedSortComparison> featurePtsToInclude(DPoint3dZYXTolerancedSortComparison(1e-6, 0));
    bvector<POINT> ptsToInclude;


    // Get children linear features
	for (size_t indexNodes = 0; indexNodes < numSubNodes; indexNodes++)
	{
		if (subNodes[indexNodes] != NULL) 
		{
            if (!subNodes[indexNodes]->IsLoaded())
                subNodes[indexNodes]->Load();
                    
			RefCountedPtr<SMMemoryPoolVectorItem<POINT>> subNodePointsPtr(subNodes[indexNodes]->GetPointsPtr());
			totalNumberOfPoints += subNodePointsPtr->size();
        
			HFCPtr<SMMeshIndexNode<POINT, EXTENT>> subMeshNode = dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(subNodes[indexNodes]);

			bvector<bvector<DPoint3d>> polylinesNode;
			bvector<DTMFeatureType> typesNode;
			subMeshNode->ReadFeatureDefinitions(polylinesNode, typesNode, false);

            auto beginHullIdx = (int)polylines.size();
            for(size_t i = 0; i < polylinesNode.size(); i++)
                {
                auto const& type = typesNode[i];
                auto const& polyline = polylinesNode[i];
                if(type == DTMFeatureType::Hull || type == DTMFeatureType::TinHull)
                    anyHull[indexNodes] = true;
                else if(IsClosedFeature((ISMStore::FeatureType)type) && polyline.size() <= 4)
                    continue;
                else if(polyline.size() <= 3)
                    continue;
                types.push_back(type);
                polylines.push_back(polyline);
                for(auto const& pt : polyline)
                    if(featurePtsToInclude.count(pt) == 0) featurePtsToInclude.insert(pt);
                }

			if(anyHull[indexNodes])
				beginIdx[indexNodes] = beginHullIdx;
		}
	}


	if (totalNumberOfPoints < 10)
	{
		// There are far too few points to start decimating them towards the root.
		// We then promote then all so they are given a high importance to make sure some terrain
		// representativity is retained in this area.
		DRange3d extent = DRange3d::NullRange();
		for (size_t indexNodes = 0; indexNodes < numSubNodes; indexNodes++)
		{            			
			if (subNodes[indexNodes] != NULL)
			{
				if (subNodes[indexNodes]->GetNbObjects() == 0) continue;

                extent.Extend(subNodes[indexNodes]->m_nodeHeader.m_contentExtent);

				RefCountedPtr<SMMemoryPoolVectorItem<POINT>> subNodesPointsPtr(subNodes[indexNodes]->GetPointsPtr());
                ptsToInclude.resize(subNodesPointsPtr->size());
                memcpy(&ptsToInclude[0], &(*subNodesPointsPtr)[0], ptsToInclude.size() * sizeof(POINT));

				if (std::any_of(anyHull.begin(), anyHull.end(), [](bool val) {return val; }))
				{
					if (!anyHull[indexNodes])
					{
						types.push_back(DTMFeatureType::Hull);
						bvector<DPoint3d> boxPts;
						boxPts.push_back(DPoint3d::From(subNodes[indexNodes]->m_nodeHeader.m_contentExtent.low.x, subNodes[indexNodes]->m_nodeHeader.m_contentExtent.low.y, 0));
						boxPts.push_back(DPoint3d::From(subNodes[indexNodes]->m_nodeHeader.m_contentExtent.low.x, subNodes[indexNodes]->m_nodeHeader.m_contentExtent.high.y, 0));
						boxPts.push_back(DPoint3d::From(subNodes[indexNodes]->m_nodeHeader.m_contentExtent.high.x, subNodes[indexNodes]->m_nodeHeader.m_contentExtent.high.y, 0));
						boxPts.push_back(DPoint3d::From(subNodes[indexNodes]->m_nodeHeader.m_contentExtent.high.x, subNodes[indexNodes]->m_nodeHeader.m_contentExtent.low.y, 0));
						boxPts.push_back(DPoint3d::From(subNodes[indexNodes]->m_nodeHeader.m_contentExtent.low.x, subNodes[indexNodes]->m_nodeHeader.m_contentExtent.low.y, 0));

						auto nodePtr = HFCPtr<SMPointIndexNode<POINT, EXTENT>>(subNodes[indexNodes]);
						IScalableMeshNodePtr nodeP(
#ifndef VANCOUVER_API
							new ScalableMeshNode<POINT>(nodePtr)
#else
							ScalableMeshNode<POINT>::CreateItem(nodePtr)
#endif
						);
						BcDTMPtr dtm = nodeP->GetBcDTM().get();
						for (auto& pt : boxPts)
						{
							int drapedTypeP;
							dtm->GetDTMDraping()->DrapePoint(&pt.z, nullptr, nullptr, nullptr, drapedTypeP, pt);
						}
						polylines.push_back(boxPts);
					}
					else
					{
						types.push_back(types[beginIdx[indexNodes]]);
						polylines.push_back(polylines[beginIdx[indexNodes]]);
					}
				}
			}
		}   
		if (!extent.IsNull()) parentNode->m_nodeHeader.m_contentExtent = extent;
		if (pParentMeshNode->m_nodeHeader.m_contentExtent.low.x == 0 && pParentMeshNode->m_nodeHeader.m_contentExtent.high.x != 0)
		{
			std::cout << " FILTERING NODE " << pParentMeshNode->GetBlockID().m_integerID << " WRONG EXTENT " << std::endl;
		}
	}
	else
	{
		size_t pointArrayInitialNumber[8];
		DRange3d extent = DRange3d::NullRange();

		parentPointsPtr->reserve(parentPointsPtr->size() + (totalNumberOfPoints * 1 / pParentMeshNode->m_nodeHeader.m_numberOfSubNodesOnSplit) + 20);

        // Get children points
		for (size_t indexNodes = 0; indexNodes < numSubNodes; indexNodes++)
		{
			if (subNodes[indexNodes] != NULL)
			{
				if (!subNodes[indexNodes]->IsLoaded()) subNodes[indexNodes]->Load();
				if (subNodes[indexNodes]->m_nodeHeader.m_contentExtentDefined) extent.Extend(subNodes[indexNodes]->m_nodeHeader.m_contentExtent);
				RefCountedPtr<SMMemoryPoolVectorItem<POINT>> subNodePointsPtr(subNodes[indexNodes]->GetPointsPtr());


				if (subNodePointsPtr->size() == 0)
					continue;

				if (std::any_of(anyHull.begin(), anyHull.end(), [](bool val) {return val; }))
				{
					if (!anyHull[indexNodes])
					{

						types.push_back(DTMFeatureType::Hull);
						bvector<DPoint3d> boxPts;
						boxPts.push_back(DPoint3d::From(subNodes[indexNodes]->m_nodeHeader.m_contentExtent.low.x, subNodes[indexNodes]->m_nodeHeader.m_contentExtent.low.y, 0));
						boxPts.push_back(DPoint3d::From(subNodes[indexNodes]->m_nodeHeader.m_contentExtent.low.x, subNodes[indexNodes]->m_nodeHeader.m_contentExtent.high.y, 0));
						boxPts.push_back(DPoint3d::From(subNodes[indexNodes]->m_nodeHeader.m_contentExtent.high.x, subNodes[indexNodes]->m_nodeHeader.m_contentExtent.high.y, 0));
						boxPts.push_back(DPoint3d::From(subNodes[indexNodes]->m_nodeHeader.m_contentExtent.high.x, subNodes[indexNodes]->m_nodeHeader.m_contentExtent.low.y, 0));
						boxPts.push_back(DPoint3d::From(subNodes[indexNodes]->m_nodeHeader.m_contentExtent.low.x, subNodes[indexNodes]->m_nodeHeader.m_contentExtent.low.y, 0));

						auto nodePtr = HFCPtr<SMPointIndexNode<POINT, EXTENT>>(subNodes[indexNodes]);
						IScalableMeshNodePtr nodeP(
#ifndef VANCOUVER_API
							new ScalableMeshNode<POINT>(nodePtr)
#else
							ScalableMeshNode<POINT>::CreateItem(nodePtr)
#endif
						);
						BcDTMPtr dtm = nodeP->GetBcDTM().get();
						for (auto& pt : boxPts)
						{
							int drapedTypeP;
							if(dtm.IsValid()) 
								dtm->GetDTMDraping()->DrapePoint(&pt.z, nullptr, nullptr, nullptr, drapedTypeP, pt);
						}

						polylines.push_back(boxPts);
					}
					else
					{
						types.push_back(types[beginIdx[indexNodes]]);
						polylines.push_back(polylines[beginIdx[indexNodes]]);
					}
				}

                // The value of 10 here is required. The alternative path use integer division (*3/4 +1) that will take all points anyway
                // In reality starting at 9 not all points are used but let's gives us a little margin.
                if (subNodePointsPtr->size() <= 10)
				{
					// Too few content in node ... promote them all                           
                    ptsToInclude.resize(subNodePointsPtr->size());
                    memcpy(&ptsToInclude[0], &(*subNodePointsPtr)[0], ptsToInclude.size() * sizeof(POINT));

				}
				else
				{
					subNodePointsPtr = subNodes[indexNodes]->GetPointsPtr();
					pointArrayInitialNumber[indexNodes] = subNodePointsPtr->size();


					vector<POINT> points;

                    for(size_t i = 0; i < subNodePointsPtr->size(); i++)
                        {
                        DPoint3d pt3d = DPoint3d::From(PointOp<POINT>::GetX((*subNodePointsPtr)[i]), PointOp<POINT>::GetY((*subNodePointsPtr)[i]), PointOp<POINT>::GetZ((*subNodePointsPtr)[i]));
                        if(featurePtsToInclude.count(pt3d) == 0)
                            points.push_back((*subNodePointsPtr)[i]);
                        }
					//memcpy(&points[0], &(*subNodePointsPtr)[0], points.size() * sizeof(POINT));

					std::random_shuffle(points.begin(), points.end());

					size_t count = (subNodePointsPtr->size() / pParentMeshNode->m_nodeHeader.m_numberOfSubNodesOnSplit) + 1;

                    ptsToInclude.resize(points.size());
                    memcpy(&ptsToInclude[0], &points[0], std::min(count, points.size()) * sizeof(POINT));

				}
			}
			if (!extent.IsNull())  parentNode->m_nodeHeader.m_contentExtent = extent;

		}

	}
   
	if (std::any_of(anyHull.begin(), anyHull.end(), [](bool val) {return val; }))
	{
		size_t nRemoved = 0;
		for (size_t indexNodes = 0; indexNodes < numSubNodes; indexNodes++)
		{
			if (beginIdx[indexNodes] != -1)
			{
				polylines.erase(polylines.begin() + beginIdx[indexNodes] - nRemoved);
				types.erase(types.begin() + beginIdx[indexNodes] - nRemoved);
				nRemoved++;
			}
		}
	}

    // Merge and simplify children linear features
	if (polylines.size() > 0)
	{
		bvector<DTMFeatureType> newTypes;
		bvector<DTMFeatureType> otherNewTypes;
		bvector<bvector<DPoint3d>> newLines;

#if 0
        LOG_SET_PATH("e:\\Elenie\\mesh\\")
            LOG_SET_PATH_W("e:\\Elenie\\mesh\\")
            BC_DTM_OBJ* dtmObjP = 0;
        bcdtmObject_createDtmObject(&dtmObjP);
        for(auto& feature: polylines)
            if(IsVoidFeature((ISMStore::FeatureType)types[&feature-polylines.data()]))
                bcdtmObject_storeDtmFeatureInDtmObject(dtmObjP, DTMFeatureType::Breakline, dtmObjP->nullUserTag, 1, &dtmObjP->nullFeatureId, &feature[0], (long)feature.size());

        WString dtmFileName(LOG_PATH_STR_W + L"ptile_");
        LOGSTRING_NODE_INFO_W(pParentMeshNode, dtmFileName)
            dtmFileName.append(L".tin");
        bcdtmWrite_toFileDtmObject(dtmObjP, dtmFileName.c_str());
#endif

        // Merge VOIDS
		MergePolygonSets(polylines, [&newTypes, &newLines, &types](const size_t i, const bvector<DPoint3d>& vec)
		{
			if (!IsVoidFeature((ISMStore::FeatureType)types[i]))
			{
				newLines.push_back(vec);
				newTypes.push_back(types[i]);
				return false;
			}
			else return true;
		},
			[&otherNewTypes](const bvector<DPoint3d>& vec)
		{
			otherNewTypes.push_back(DTMFeatureType::DrapeVoid);
		});
		otherNewTypes.insert(otherNewTypes.end(), newTypes.begin(), newTypes.end());
		polylines.insert(polylines.end(), newLines.begin(), newLines.end());
		types = otherNewTypes;

		newTypes.clear();
		otherNewTypes.clear();
		newLines.clear();

        // Merge ISLANDS
		MergePolygonSets(polylines, [&newTypes, &newLines, &types](const size_t i, const bvector<DPoint3d>& vec)
		{
			if (types[i] != DTMFeatureType::Island)
			{
				newLines.push_back(vec);
				newTypes.push_back(types[i]);
				return false;
			}
			else return true;
		},
			[&otherNewTypes](const bvector<DPoint3d>& vec)
		{
			otherNewTypes.push_back(DTMFeatureType::Island);
		});
		otherNewTypes.insert(otherNewTypes.end(), newTypes.begin(), newTypes.end());
		polylines.insert(polylines.end(), newLines.begin(), newLines.end());
		types = otherNewTypes;

		newTypes.clear();
		otherNewTypes.clear();
		newLines.clear();

        if (polylines.size() > 0)
            {
		    for (size_t i = polylines.size() - 1; i > 0; i--)
		        {
                newTypes.clear();
                otherNewTypes.clear();
                newLines.clear();
			    bvector<bvector<DPoint3d>> defsHull;
			    defsHull.push_back(polylines[i]);
			    defsHull.push_back(polylines[i - 1]);

                // Merge HULLS
			    MergePolygonSets(defsHull, [&newTypes, &newLines, &types, &i](const size_t j, const bvector<DPoint3d>& vec)
			        {
				    if (types[i-j] != DTMFeatureType::Hull &&  types[i-j] != DTMFeatureType::TinHull)
				        {
					    newLines.push_back(vec);
					    newTypes.push_back(types[i-j]);
					    return false;
				    }
				    else return true;
			        },
				    [&otherNewTypes](const bvector<DPoint3d>& vec)
			        {
				    otherNewTypes.push_back(DTMFeatureType::Hull);
			        });
			    if (defsHull.size() > 0)
			        {
				    polylines[i - 1] = defsHull[0];
				    types[i - 1] = otherNewTypes[0];
			        }
			    if (defsHull.size() > 1)
			        {
				    polylines[i] = defsHull[1];
				    types[i] = otherNewTypes[1];
			        }
			    if (newLines.size() > 0)
			        {
				    polylines[i] = newLines[0];
				    types[i] = newTypes[0];
			        }
			    if (newLines.size() > 1)
			        {
				    polylines[i - 1] = newLines[1];
				    types[i - 1] = newTypes[1];
			        }
			    if (defsHull.size() + newLines.size() < 2) polylines[i].clear();
		        }

		    newLines = polylines;

            size_t totalNumberOfFeaturePoints = 0;
            for(const auto& line : polylines)
                totalNumberOfFeaturePoints += line.size();

            // backed out  part of changeset "beab95dcece7" to prevent crash in imodel publish (using IMBOB)
            // files: $(atp-serv.bentley.com:/atp-root)\atp\building\development\tfbridge\scenario32\1MC08-BBV-DS-DMB-NS02_NL07-100005.dgn 
            // see discussion in BUG https://bentleycs.visualstudio.com/iModelTechnologies/_workitems/edit/149245 

            // WIP_NEEDS_WORK_2017 (original comment removed from changeset beab95dcece7)
            //bool shouldSimplifyFeatures = totalNumberOfPoints * 0.2 <= totalNumberOfFeaturePoints;
            //  if(shouldSimplifyFeatures)
                //SimplifyPolylines(polylines);

            // Keep non simplified Hulls
		    std::transform(polylines.begin(), polylines.end(), newLines.begin(), polylines.begin(),
			    [&types, &polylines](const bvector<DPoint3d>&vec, const bvector<DPoint3d>& vec2)
		        {
			    if (types[&vec - &polylines[0]] == DTMFeatureType::Hull || types[&vec - &polylines[0]] == DTMFeatureType::TinHull)
				    return vec2;
			    else return vec;
		        });
            }

        std::unique(polylines.begin(), polylines.end(), [] (const bvector<DPoint3d>& a, const bvector<DPoint3d>&b) {
            if(a.size() != b.size()) return false;
            for(size_t i = 0; i < a.size(); ++i)
                if(!a[i].IsEqual(b[i]))
                    return false;

            return true;
                    });
	    }

    // Only points which are not feature points should be included
    bvector<POINT> pts2 = ptsToInclude;
    ptsToInclude.clear();
    for(auto const& pt : pts2)
        {
        DPoint3d pt3d = DPoint3d::From(PointOp<POINT>::GetX(pt), PointOp<POINT>::GetY(pt), PointOp<POINT>::GetZ(pt));

        if(featurePtsToInclude.count(pt3d) == 0)
            ptsToInclude.push_back(pt);
        }

    // Add points and features
    parentPointsPtr->push_back(ptsToInclude.begin(), ptsToInclude.size());

	for (auto& polyline : polylines)
	{
		if (polyline.empty()) continue;
		DRange3d extent2 = DRange3d::From(polyline);
		pParentMeshNode->AddFeatureDefinitionSingleNode((ISMStore::FeatureType)types[&polyline - &polylines.front()], polyline, extent2);
	}

    //In multi-process flushing to disk needs to be controlled by the the workers.
    if(!m_isMultiProcessGeneration)
        {
        SMMemoryPool::GetInstance()->RemoveItem(pParentMeshNode->m_pointsPoolItemId, pParentMeshNode->GetBlockID().m_integerID, SMStoreDataType::Points, (uint64_t)pParentMeshNode->m_SMIndex);
        parentPointsPtr = 0;
        pParentMeshNode->m_pointsPoolItemId = SMMemoryPool::s_UndefinedPoolItemId;
        }
    
    // Mesh
    if (pParentMeshNode->m_nodeHeader.m_arePoints3d)
	{
		pParentMeshNode->GetMesher3d()->Mesh(pParentMeshNode);
	}
	else
	{
		pParentMeshNode->GetMesher2_5d()->Mesh(pParentMeshNode);
	}

	if (pParentMeshNode->GetPointsPtr()->size() > 10 && pParentMeshNode->GetPtsIndicePtr()->size() == 0)
	{
		std::cout << "NODE " << pParentMeshNode->GetBlockID().m_integerID << " SHOULD HAVE FACES " << std::endl;
#if FILTER_DBG_WHEN_NODE_HAS_NO_FACE
		for (size_t indexNodes = 0; indexNodes < numSubNodes; indexNodes++)
		{
			if (subNodes[indexNodes] != NULL)
			{
				RefCountedPtr<SMMemoryPoolVectorItem<POINT>> subNodePointsPtr(subNodes[indexNodes]->GetPointsPtr());
				Utf8String namePts = "e:\\output\\scmesh\\2016-08-24\\sub_mesh_tile_";
				LOGSTRING_NODE_INFO(subNodes[indexNodes], namePts)
					namePts.append(".pts");
				size_t _nVertices = subNodePointsPtr->size();
				FILE* _meshFile = fopen(namePts.c_str(), "wb");
				fwrite(&_nVertices, sizeof(size_t), 1, _meshFile);
				fwrite(&((*subNodePointsPtr)[0]), sizeof(DPoint3d), _nVertices, _meshFile);
				fclose(_meshFile);
			}
		}
#endif
	}

	//In multi-process flushing to disk needs to be controlled by the the workers.
    if (!m_isMultiProcessGeneration)    
        {
        //Flushing tiles to disk during filtering result in less tiles being flushed sequentially at the end of the generation process, leading to potential huge performance gain.
        for (size_t indexNodes = 0; indexNodes < numSubNodes; indexNodes++)
            {
            if (subNodes[indexNodes] != NULL)
                {
                subNodes[indexNodes]->Discard();
                }
            }
        }
    
	return true;
}

//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 11/15
//=======================================================================================
template<class POINT, class EXTENT> bool ScalableMeshQuadTreeBCLIB_CGALMeshFilter<POINT, EXTENT>::Filter(
    HFCPtr<SMPointIndexNode<POINT, EXTENT> > parentNode,
    std::vector<HFCPtr<SMPointIndexNode<POINT, EXTENT> >>&  subNodes,
    size_t numSubNodes) const
    {
    HFCPtr<SMMeshIndexNode<POINT, EXTENT> > pParentMeshNode = dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(parentNode);    
  
    RefCountedPtr<SMMemoryPoolGenericBlobItem<MTGGraph>> graphPtr(pParentMeshNode->GetGraphPtr());
    MTGGraph* meshInput = nullptr;
    std::vector<DPoint3d> inputPts;
    DPoint3d extentMin, extentMax;
    extentMin = DPoint3d::FromXYZ(ExtentOp<EXTENT>::GetXMin(pParentMeshNode->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMin(pParentMeshNode->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMin(pParentMeshNode->m_nodeHeader.m_nodeExtent));
    extentMax = DPoint3d::FromXYZ(ExtentOp<EXTENT>::GetXMax(pParentMeshNode->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMax(pParentMeshNode->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMax(pParentMeshNode->m_nodeHeader.m_nodeExtent));


    bvector<bvector<DPoint3d>> polylines;
    bvector<DTMFeatureType> types;
    for (size_t indexNodes = 0; indexNodes < numSubNodes; indexNodes++)
        {
        if (subNodes[indexNodes] != NULL)
            {
            size_t numFaceIndexes = subNodes[indexNodes]->m_nodeHeader.m_nbFaceIndexes;

            if (numFaceIndexes > 0)
                {
                HFCPtr<SMMeshIndexNode<POINT, EXTENT>> subMeshNode = dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(subNodes[indexNodes]);                
              
                RefCountedPtr<SMMemoryPoolGenericBlobItem<MTGGraph>> subMeshGraphPtr(subMeshNode->GetGraphPtr());
                if (meshInput == nullptr)
                    {
                    meshInput = new MTGGraph();
                    if(nullptr != subMeshGraphPtr->GetData()) *meshInput = *subMeshGraphPtr->GetData();
                    RefCountedPtr<SMMemoryPoolVectorItem<POINT>> subMeshPointsPtr(subMeshNode->GetPointsPtr());
                    inputPts.resize(subMeshPointsPtr->size());

                    PtToPtConverter::Transform(&inputPts[0], &(*subMeshPointsPtr)[0], inputPts.size());        
                    subMeshNode->ReadFeatureDefinitions(polylines, types, false);

                    }
                else
                    {
                    RefCountedPtr<SMMemoryPoolVectorItem<POINT>> pointsPtr(subMeshNode->GetPointsPtr());                    
                    std::vector<DPoint3d> pts(pointsPtr->size());                    
                    PtToPtConverter::Transform(&pts[0], &(*pointsPtr)[0], pts.size());
                    subMeshNode->ReadFeatureDefinitions(polylines, types, false);

                    std::vector<int> pointsToDestPointsMap(pts.size());
                    std::fill_n(pointsToDestPointsMap.begin(), pointsToDestPointsMap.size(), -1);
                    std::map<DPoint3d, int, DPoint3dZYXTolerancedSortComparison> stitchedSet(DPoint3dZYXTolerancedSortComparison(1e-3, 0));

                    for (int i = 0; i < (int)inputPts.size(); i++)
                        {
                        stitchedSet.insert(std::make_pair(inputPts[i], i));
                        
                        }

                    for (size_t i = 0; i < pts.size(); i++)
                        {
                        DPoint3d pt = pts[i];
                        if (stitchedSet.count(pt) != 0) pointsToDestPointsMap[i] = stitchedSet[pt];
                        else pointsToDestPointsMap[i] = -1;
                        }
                    bvector<int> contours;

                    subMeshGraphPtr = subMeshNode->GetGraphPtr();
                    if(nullptr != subMeshGraphPtr->GetData()) MergeGraphs(meshInput, inputPts, subMeshGraphPtr->EditData(), pts, extentMin, extentMax, pointsToDestPointsMap, contours);

                    }                
                }
            }
        }
    if (meshInput == nullptr) return false;
    ResolveUnmergedBoundaries(meshInput);

// WIP_NEEDS_WORK_2017
//    bool ret = CGALEdgeCollapse(meshInput, inputPts, pParentMeshNode->GetBlockID().m_integerID);
    bvector<DPoint3d> vecPts;
    vecPts.assign(inputPts.begin(), inputPts.end());    
    if (polylines.size() > 0)
        {

        bvector<DTMFeatureType> newTypes;
        bvector<DTMFeatureType> otherNewTypes;
        bvector<bvector<DPoint3d>> newLines;
        MergePolygonSets(polylines, [&newTypes, &newLines, &types] (const size_t i, const bvector<DPoint3d>& vec)
            {
            if (!IsVoidFeature((ISMStore::FeatureType)types[i]))
                {
                newLines.push_back(vec);
                newTypes.push_back(types[i]);
                return false;
                }
            else return true;
            },
                [&otherNewTypes] (const bvector<DPoint3d>& vec)
                {
                otherNewTypes.push_back(DTMFeatureType::DrapeVoid);
                });
            otherNewTypes.insert(otherNewTypes.end(), newTypes.begin(), newTypes.end());
            polylines.insert(polylines.end(), newLines.begin(), newLines.end());
                types = otherNewTypes;
            SimplifyPolylines(polylines);
        }


    //if (ret)
    //    {
    //    volatile size_t n2 = 0;
    //    MTGARRAY_SET_LOOP(edgeID, meshInput)
    //        {
    //        int tag = -1;
    //        meshInput->TryGetLabel(edgeID, 2, tag);
    //        if (tag != -1) n2++;
    //        }
    //    MTGARRAY_END_SET_LOOP(edgeID, meshInput)
    //            
    //    dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(parentNode)->UpdateFromGraph(meshInput, vecPts);
    //    for (auto& polyline : polylines)
    //        {
    //        DRange3d extent = DRange3d::From(polyline);
    //        pParentMeshNode->AddFeatureDefinitionSingleNode((ISMStore::FeatureType)types[&polyline - &polylines.front()], polyline, extent);
    //        }
    //    }
    //else
    //    {
        std::random_shuffle(vecPts.begin(), vecPts.end());
        vecPts.resize(vecPts.size() / pParentMeshNode->GetNumberOfSubNodesOnSplit());
        //pParentMeshNode->clear();

        RefCountedPtr<SMMemoryPoolVectorItem<POINT>> parentPointsPtr(pParentMeshNode->GetPointsPtr());

        parentPointsPtr->push_back(&vecPts[0], vecPts.size());
        for (auto& polyline : polylines)
            {
            DRange3d extent = DRange3d::From(polyline);
            pParentMeshNode->AddFeatureDefinitionSingleNode((ISMStore::FeatureType)types[&polyline - &polylines.front()], polyline, extent);
            }

        if (pParentMeshNode->m_nodeHeader.m_arePoints3d)
            {
            pParentMeshNode->GetMesher3d()->Mesh(pParentMeshNode);
            }
        else
            {
            pParentMeshNode->GetMesher2_5d()->Mesh(pParentMeshNode);
            }
        //}
    delete meshInput;
    if (pParentMeshNode->GetPointsPtr()->size() > 10 && pParentMeshNode->GetPtsIndicePtr()->size() == 0)
        {
        std::cout << "NODE " << pParentMeshNode->GetBlockID().m_integerID << " SHOULD HAVE FACES " << std::endl;
        }
    return true;
    }

//#ifdef WIP_MESH_IMPORT
//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 07/16
//=======================================================================================
template<class POINT, class EXTENT> bool ScalableMeshQuadTreeBCLIB_UserMeshFilter<POINT, EXTENT>::Filter(
    HFCPtr<SMPointIndexNode<POINT, EXTENT> > parentNode,
    std::vector<HFCPtr<SMPointIndexNode<POINT, EXTENT> >>&  subNodes,
    size_t numSubNodes) const
    {
    assert(!"not yet implemented");
    return false;

#ifdef WIP_MESH_IMPORT
    HFCPtr<SMMeshIndexNode<POINT, EXTENT> > pParentMeshNode = dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(parentNode);    
    DRange3d extent = DRange3d::NullRange();
    bvector<IScalableMeshMeshPtr> subMeshes;
    bvector<Utf8String> subMetadata;
    for (size_t indexNodes = 0; indexNodes < numSubNodes; indexNodes++)
        {
        if (subNodes[indexNodes] != NULL)
            {
            if (subNodes[indexNodes]->m_nodeHeader.m_contentExtentDefined) extent.Extend(subNodes[indexNodes]->m_nodeHeader.m_contentExtent);
            size_t numFaceIndexes = subNodes[indexNodes]->m_nodeHeader.m_nbFaceIndexes;

            if (numFaceIndexes > 0)
                {
                HFCPtr<SMMeshIndexNode<POINT, EXTENT>> subMeshNode = dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(subNodes[indexNodes]);          
                if(!subMeshNode->IsLoaded())subMeshNode->Load();
                bvector<IScalableMeshMeshPtr> meshParts;
                bvector<Utf8String> meshMetadata;
                bvector<bvector<uint8_t>> texData;
                RefCountedPtr<SMMemoryPoolVectorItem<POINT>> pointsPtr(subMeshNode->GetPointsPtr());
                if (pointsPtr->size() > 65000)
                    std::cout << " TOO MANY POINTS BEFORE FILTER :" << pointsPtr->size() << std::endl;
                subMeshNode->GetMetadata();
                subMeshNode->GetMeshParts();
                subMeshNode->GetMeshParts(meshParts, meshMetadata, texData);
                if (meshParts.size() > 0)
                    {
                    subMeshes.insert(subMeshes.end(), meshParts.begin(), meshParts.end());
                    subMetadata.insert(subMetadata.end(), meshMetadata.begin(), meshMetadata.end());
                    }
                }
            }
        }
    if (!extent.IsNull()) pParentMeshNode->m_nodeHeader.m_contentExtent = extent;
    if (m_callback != nullptr)
        {
        bool shouldCreateGraph = false;
        bvector<bvector<DPoint3d>> parentMeshPts;
        bvector<bvector<int32_t>> parentMeshIdx;
        bvector<Utf8String> parentMetadata;
        bvector<bvector<DPoint2d>> parentMeshUvs;
        bvector<bvector<uint8_t>> parentMeshTex;
        bool filterSuccess = m_callback(shouldCreateGraph, parentMeshPts, parentMeshIdx, parentMetadata,parentMeshUvs, parentMeshTex, subMeshes, subMetadata, pParentMeshNode->m_nodeHeader.m_nodeExtent);
        if (filterSuccess)
            {
            //user function needs to provide info for all mesh parts
            assert(parentMeshPts.size() == parentMeshIdx.size() && parentMeshPts.size() == parentMetadata.size());
            size_t partMeshId =0;
            for(auto& parentData: parentMetadata)
                {
                Json::Value val;
                Json::Reader reader;
                reader.parse(parentData, val);
                bvector<int> parts;
                bvector<int64_t> texId;
                bmap<int64_t,uint64_t> dgnIds;
                for (Json::Value& id : val["texId"])
                    {
                    texId.push_back(id.asInt64());
                    }
                for (Json::Value& id : val["mapIds"])
                    {
                    dgnIds[id["dgn"].asUInt64()] = id["SM"].asInt64();
                    }

                for (const Json::Value& id : val["parts"])
                    {
                    parts.push_back(id.asInt());
                    }
                if(!parentMeshTex[partMeshId].empty())
                    {
                    int width, height, nChannels;
                    memcpy(&width, &parentMeshTex[partMeshId][0],sizeof(int));
                    memcpy(&height, &parentMeshTex[partMeshId][0]+sizeof(int),sizeof(int));
                    memcpy(&nChannels, &parentMeshTex[partMeshId][0]+2*sizeof(int),sizeof(int));
                    int64_t newTexId = ((SMMeshIndex<POINT,EXTENT>*)pParentMeshNode->m_SMIndex)->AddTexture(width, height, nChannels, &parentMeshTex[partMeshId][0]+3*sizeof(int), parentMeshTex[partMeshId].size()-3*sizeof(int));
                    uint64_t dgnId = texId[0];
                    dgnIds[dgnId] = newTexId;
                    texId[0] = newTexId;
                    parentMeshTex[partMeshId].clear();
                    }
                partMeshId++;
                val["texId"] = Json::arrayValue;

                for(auto& id: texId) val["texId"].append(id);
                val["mapIds"] = Json::arrayValue;

                for(auto& mapId: dgnIds)
                    {
                    Json::Value newEntry = Json::objectValue;
                    newEntry["dgn"] = mapId.first;
                    newEntry["SM"] = mapId.second;
                    val["mapIds"].append(newEntry);
                    }

                parentData=Json::FastWriter().write(val);
                }
            pParentMeshNode->AppendMeshParts(parentMeshPts, parentMeshIdx, parentMetadata, parentMeshUvs, parentMeshTex,shouldCreateGraph);
            }
        return filterSuccess;
        }

    return true;
#endif
    }
//#endif