#include "ScalableMeshPCH.h"
#include "ScalableMeshVolume.h"
#include <Mtg/MtgApi.h>

#ifdef SCALABLE_MESH_ATP
#include <ScalableMesh/IScalableMeshATP.h>
#endif


BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

static double s_absTol = 1.0e-8;
static double s_relTol = 1.0e-12;



/*static void StitchAndCollectVolumesFromImprintedMesh
    (
    PolyfaceQueryCR mesh,
    IFacetOptionsR options,
    bvector<PolyfaceHeaderPtr> &allVolumes
    )
    {
    allVolumes.clear ();
    DRange3d range = mesh.PointRange ();

    if (range.IsNull ())
        return;
    static double s_absTol = 1.0e-8;
    static double s_relTol = 1.0e-11;
    MTGFacets *mtgFacets = jmdlMTGFacets_new ();
    PolyfaceToMTG (mtgFacets,
        nullptr, nullptr,
        mesh,
        false, s_absTol, s_relTol, 1
        );




    bvector<bvector<MTGNodeId>> componentFaceNodes;
    mtgFacets->GetGraphP()->CollectConnectedComponents (componentFaceNodes, MTG_ScopeFace);
    size_t numShell = componentFaceNodes.size ();

        for (size_t i = 0; i < numShell; i++)
            {
                IPolyfaceConstructionPtr finalBuilder = IPolyfaceConstruction::Create (options);
            mtgFacets->AddFacesToPolyface (*finalBuilder, componentFaceNodes [i], false);
                allVolumes.push_back (finalBuilder->GetClientMeshPtr ());
            }

    }


    */
    

DRange3d RangeOfNodes (MTGFacets *facets, bvector<MTGNodeId> &nodes, size_t &numFace, size_t &numVertex, double &volumeSum, int noisy)
    {
    DRange3d range;
    range.Init ();
    size_t numError = 0;
    DPoint3d xyz;
    numFace = 0;
    numVertex = 0;
    MTGMask faceMask = MTG_CONSTU_MASK;
    MTGMask vertexMask = MTG_CONSTV_MASK;
    MTGGraph * graph = jmdlMTGFacets_getGraph(facets);
    graph->SetMask (faceMask);
    graph->SetMask (vertexMask);
    // clear masks node by node ...
    for (MTGNodeId node : nodes)
        {
        graph->ClearMaskAt (node, faceMask);
        graph->ClearMaskAt (node, vertexMask);
        }

    // confirm component includes entire face and vertex loops ..
    for (MTGNodeId node : nodes)
        {
        size_t faceMaskNodes = graph->CountMaskAroundFace (node, faceMask);
        size_t vertexMaskNodes = graph->CountMaskAroundVertex (node, vertexMask);
        if (faceMaskNodes != 0)
            numError++;
        if (vertexMaskNodes != 0)
            numError++;
        }

    DPoint3d origin = DPoint3d::From (0,0,0);
    volumeSum = 0.0;
    for (MTGNodeId node : nodes)
        {
        if (jmdlMTGFacets_getNodeCoordinates (facets, &xyz, node))
            range.Extend (xyz);
        else
            numError++;
        if (MTG_NULL_MASK == graph->GetMaskAt (node, faceMask))
            {
            numFace++;
            graph->SetMaskAroundFace (node, faceMask);
            if (noisy > 9)
                printf ("FACE ");
            int numSector = 0;
            DPoint3d xyzEdges[3];
            MTGARRAY_FACE_LOOP (sector, graph, node)
                {
                DPoint3d xyz;
                jmdlMTGFacets_getNodeCoordinates (facets, &xyz, sector);
//                if (noisy > 9)
//                    PrintNode (sector, xyz, false);
                if (numSector < 2)
                    xyzEdges[numSector] = xyz;
                else
                    {

                    xyzEdges[2] = xyz;
                    double volume = - origin.TripleProductToPoints (xyzEdges[0], xyzEdges[1], xyzEdges[2]);
                    volumeSum += volume / 6.0;
                    xyzEdges[1] = xyzEdges[2];
                    }

                numSector++;
                }
            MTGARRAY_END_FACE_LOOP (sector, graph, node)
                if (noisy > 9)
                    {
                    if (numSector != 3)
                        printf ("  (nv=%d)", numSector);
                    printf ("\n");
                    }

            }
        if (MTG_NULL_MASK == graph->GetMaskAt (node, vertexMask))
            {
            numVertex++;
            graph->SetMaskAroundVertex (node, vertexMask);
            }
        }
//    PrintNonZero ("ConnectedComponent Errors", numError);
    return range;
    }

//double ScalableMeshVolume::ComputeVolumeCutAndFillForTile(PolyfaceHeaderPtr terrainMesh, double& cut, double& fill, PolyfaceHeader& mesh, bool is2d, bvector<PolyfaceHeaderPtr>& volumeMeshVector)
DTMStatusInt ScalableMeshVolume::_ComputeVolumeCutAndFill(PolyfaceHeaderPtr& terrainMesh, double& cut, double& fill, PolyfaceHeader& mesh, bool is2d, bvector<PolyfaceHeaderPtr>& volumeMeshVector)
    {
    double totalVolume = 0.0, totalCut = 0.0, totalFill = 0.0;
#ifdef SCALABLE_MESH_ATP
    int64_t tiles;
    IScalableMeshATP::GetInt(L"nTiles", tiles);
    tiles++;
    IScalableMeshATP::StoreInt(L"nTiles", tiles);
#endif
    if (!is2d)
        {
        
        mesh.MarkTopologicalBoundariesVisible(false); // mesh->MarkTopologicalBoundaries(2pi);
//        size_t numOpen =0, numClosed = 0;
//        CurveVectorPtr curveBoundary = mesh.ExtractBoundaryStrings(numOpen, numClosed);
        bvector<DSegment3d> segments;
        bvector<DSegment3dSizeSize> segmentsTerrain;
        
        //curveBoundary->get
        mesh.CollectSegments(segments, true);
        //PolyfaceHeaderPtr meshPtr(&mesh, true);
        PolyfaceHeaderPtr meshPtr(&mesh, true);

        static double s_absTol = 1.0e-8;
        static double s_relTol = 1.0e-11;
        // short time to integrated MGGraph
/*        MTGFacets *mtgFacets = jmdlMTGFacets_new ();
        PolyfaceToMTG (mtgFacets,
            nullptr, nullptr,
            mesh,
            false, s_absTol, s_relTol, 1
            );
        bvector<bvector<MTGNodeId>> componentFaceNodes;
        mtgFacets->GetGraphP()->CollectConnectedComponents (componentFaceNodes, MTG_ScopeFace);
        size_t numShell = componentFaceNodes.size ();*/

//        MTGGraph *graph = jmdlMTGFacets_getGraph(mtgFacets);

        
        IFacetOptionsPtr options = IFacetOptions::Create();
        IPolyfaceConstructionPtr builder2 = IPolyfaceConstruction::New(*options);
        builder2->AddPolyface(mesh);

        // Mesh for ground with split edge for drapping line.
        IPolyfaceConstructionPtr builder3 = IPolyfaceConstruction::New(*options);
        //builder->AddPolyface(*terrainMesh);
        bool areTriangle = false;
        //for(int i=0; i<curveBoundary->)
        //segments.push_back(segments[0]);
/*
//        Bentley::TerrainModel::DTMPtr dtmPtr;
        //BC_DTM_OBJ* dtmObject = NULL;
        //dtmObject = dtmPtr->GetBcDTM()->GetTinHandle();
#if 0
        DPoint3d triangle[4];
        int status = SUCCESS;
        BC_DTM_OBJ* bcDtmP = 0;
        bcdtmObject_createDtmObject(&bcDtmP);

        PolyfaceVisitorPtr visitorPtr = PolyfaceVisitor::Attach(*terrainMesh, false);
        PolyfaceVisitor & visitor = *visitorPtr.get();
        const DPoint3d* ptCP = terrainMesh->GetPointCP();
        for(visitor.Reset(); visitor.AdvanceToNextFace();)
        //for(size_t n =0; n < terrainMesh->GetPointIndexCount()&& status == SUCCESS; n+=3)
            {

            triangle[0] = ptCP[visitor.ClientPointIndex()[0]];
            triangle[1] = ptCP[visitor.ClientPointIndex()[1]];
            triangle[2] = ptCP[visitor.ClientPointIndex()[2]];
            triangle[3] = triangle[0];

            status = bcdtmObject_storeDtmFeatureInDtmObject (bcDtmP, DTMFeatureType::GraphicBreak, bcDtmP->nullUserTag, 1, &bcDtmP->nullFeatureId, &triangle[0], 4);
            }
        bcdtmObject_triangulateStmTrianglesDtmObject (bcDtmP);
        //meshPtr->

        status = SUCCESS;
        BC_DTM_OBJ* bcDtmP2 = 0;
        bcdtmObject_createDtmObject(&bcDtmP2);
        PolyfaceVisitorPtr visitorPtr2 = PolyfaceVisitor::Attach(*meshPtr, false);
        PolyfaceVisitor & visitor2 = *visitorPtr2.get();
        const DPoint3d* ptCP2 = meshPtr->GetPointCP();
        for(visitor2.Reset(); visitor2.AdvanceToNextFace();)
            {
            triangle[0] = ptCP2[visitor2.ClientPointIndex()[0]];
            triangle[1] = ptCP2[visitor2.ClientPointIndex()[1]];
            triangle[2] = ptCP2[visitor2.ClientPointIndex()[2]];
            triangle[3] = triangle[0];

            status = bcdtmObject_storeDtmFeatureInDtmObject (bcDtmP2, DTMFeatureType::GraphicBreak, bcDtmP2->nullUserTag, 1, &bcDtmP2->nullFeatureId, &triangle[0], 4);
            }
        bcdtmObject_triangulateStmTrianglesDtmObject (bcDtmP2);
#endif
        size_t sizeSeg = segments.size();
        for(size_t i = 0; i < sizeSeg; i++)
            {
#if 0
            DPoint3dP linePtsP = new DPoint3d[2];
            linePtsP = segments[i].point;
            // found dtmObject
            // Found linePtsP
            

            DTM_DRAPE_POINT* drapePtsP = 0;
            long numDrapePts;

            status = bcdtmDrape_stringDtmObject(bcDtmP, linePtsP, 2, FALSE, &drapePtsP, &numDrapePts);

#else

            PolyfaceSearchContext polyfaceSearch1(terrainMesh, true, true, false);
            PolyfaceSearchContext polyfaceSearch2(meshPtr, true, true, false);

            bvector<DrapeSegment> drapeSegments1;
            
            polyfaceSearch1.DoDrapeXY(segments[i], drapeSegments1);
            size_t size = drapeSegments1.size();
            DPoint3d triangle[4];
#endif
#if 0
            for(int j=0; j<numDrapePts-1; j++)
#else
            for(int j=0; j<size; j++)
#endif
                {
                //bvector<DrapeSegment> drapeSegments2;
                //DSegment3d segment;
#if 0
                DPoint3dP linePtsP2 = new DPoint3d[2];
                linePtsP2[0].x = drapePtsP[j].drapeX;
                linePtsP2[0].y = drapePtsP[j].drapeY;
                linePtsP2[0].z = drapePtsP[j].drapeZ;
                linePtsP2[1].x = drapePtsP[j+1].drapeX;
                linePtsP2[1].y = drapePtsP[j+1].drapeY;
                linePtsP2[1].z = drapePtsP[j+1].drapeZ;
                // found dtmObject
                // Found linePtsP
                //DPoint3d triangle2[4];
                 

                DTM_DRAPE_POINT* drapePtsP2 = 0;
                long numDrapePts2;

                status = bcdtmDrape_stringDtmObject(bcDtmP2, linePtsP2, 2, FALSE, &drapePtsP2, &numDrapePts2);



#else
                
                triangle[0] = drapeSegments1[j].m_segment.point[0];
                triangle[1] = segments[i].point[0];
                triangle[2] = drapeSegments1[j].m_segment.point[1];

                builder2->AddTriStrip(triangle, NULL, NULL, 3, true);


                triangle[0] = segments[i].point[0];
                triangle[1] = drapeSegments1[j].m_segment.point[1];
                triangle[2] = segments[i].point[1];
                builder2->AddTriStrip(triangle, NULL, NULL, 3, true);
                areTriangle = true;
                




                bvector<DrapeSegment> drapeSegments2;
                DSegment3d segment;
                segment = drapeSegments1[j].m_segment;
                polyfaceSearch2.DoDrapeXY(segment, drapeSegments2);
                size_t size2 = drapeSegments2.size();
#endif
#if 0
                for(int k=0; k<numDrapePts2-1; k++)
#else
                for(int k=0; k<size2; k++)
#endif
                    {

                    //triangle[0] = drapeSegments2[k].m_segment.point[0];
#if 0
                    DPoint3d pts1, pts2;
                    pts1.x = drapePtsP2[k].drapeX;
                    pts1.y = drapePtsP2[k].drapeY;
                    pts1.z = drapePtsP2[k].drapeZ;
                    pts2.x = drapePtsP2[k+1].drapeX;
                    pts2.y = drapePtsP2[k+1].drapeY;
                    pts2.z = drapePtsP2[k+1].drapeZ;
                    triangle[0] = pts1;
                    triangle[1] = linePtsP2[0];
    //                triangle[1] = segment.point[0];
                    triangle[2] = pts2;
                    builder2->AddTriStrip(triangle, NULL, NULL, 3, true);

                    triangle[0] = linePtsP2[0];
                    triangle[1] = pts2;
                    triangle[2] = linePtsP2[1];
                    builder2->AddTriStrip(triangle, NULL, NULL, 3, true);
//#else                
                    triangle[0] = drapeSegments2[k].m_segment.point[0];
                    triangle[1] = segment.point[0];
                    triangle[2] = drapeSegments2[k].m_segment.point[1];

                    builder2->AddTriStrip(triangle, NULL, NULL, 3, true);


                    triangle[0] = segment.point[0];
                    triangle[1] = drapeSegments2[k].m_segment.point[1];
                    triangle[2] = segment.point[1];
                    builder2->AddTriStrip(triangle, NULL, NULL, 3, true);
#endif
                    //builder2->add
                    // add facet between segments and drapeSegments.
                    //PolyfaceQuery::CopyFacetsWithSegmentSplitImprint (*builder2, mesh, drapeSegments[j], true);

                    //triangle[0]
                    //builder->AddTriStrip(triangle, NULL, NULL, 3, true);
//                    DSegment3dSizeSize segmentTerrain(drapeSegments2[k].m_segment, drapeSegments2[k].m_facetReadIndex, NULL);
//                    segmentsTerrain.push_back(segmentTerrain);
                    areTriangle = true;
                    }
                }
            }


  /*      
        PolyfaceQuery::CopyFacetsWithSegmentSplitImprint(*builder3, *terrainMesh, segmentsTerrain, true);
        PolyfaceHeaderPtr meshTerrain;
        meshTerrain = builder3->GetClientMeshPtr();
*/
        PolyfaceHeaderPtr meshClosed;
        meshClosed = builder2->GetClientMeshPtr();
        if(areTriangle)
            volumeMeshVector.push_back(meshClosed);

        areTriangle = false;

        terrainMesh->MarkTopologicalBoundariesVisible(false);

        size_t numOpen = 0, numClosed = 0;
        CurveVectorPtr curveBoundary = terrainMesh->ExtractBoundaryStrings(numOpen, numClosed);

        bvector<DSegment3d> segmentsTerrainMesh;
        //bvector<DSegment3dSizeSize> segmentsTerrain;
        
        terrainMesh->CollectSegments(segmentsTerrainMesh, true);

        

        IFacetOptionsPtr option2 = IFacetOptions::Create();
        IPolyfaceConstructionPtr builder4 = IPolyfaceConstruction::New(*option2);

        builder4->AddPolyface(*meshPtr);
        segmentsTerrainMesh.push_back(segmentsTerrainMesh[0]);
        for(size_t k=0; k < segmentsTerrainMesh.size(); k++)
//        for(ICurvePrimitivePtr curvePrimitive : *curveBoundary)
            {
//            bvector<DPoint3d>* points = curvePrimitive->GetLineStringP();
//            DSegment3d segmentMesh;
 /*           size_t sizePts = points->size();
            if(!curvePrimitive->TryGetLine(segmentMesh))
                assert(true);

            for(int i=0; i<sizePts; i+=2)
                {
                //if(bfirst)
                  //  first = points->at(i);
                
                //bfirst = false;
                segmentMesh.point[0] = points->at(i);
                segmentMesh.point[0] = points->at(i+1);
              //  last = points->at(i+1);
                }
                */
            DPoint3d triangle[4];

#if 0
            DPoint3dP linePtsP = new DPoint3d[2];
            linePtsP = segmentsTerrainMesh[k].point;
            
            DTM_DRAPE_POINT* drapePtsP = 0;
            long numDrapePts;

            status = bcdtmDrape_stringDtmObject(bcDtmP2, linePtsP, 2, FALSE, &drapePtsP, &numDrapePts);
#else
            PolyfaceSearchContext polyfaceSearch1(meshPtr, true, true, false);
            PolyfaceSearchContext polyfaceSearch2(terrainMesh, true, true, false);
            bvector<DrapeSegment> drapeSegments1, drapeSegments2;

            polyfaceSearch1.DoDrapeXY(segmentsTerrainMesh[k], drapeSegments1);
#endif

#if 0
            for(size_t l=0; l<numDrapePts-1; l++)
#else
            for(size_t l=0; l<drapeSegments1.size(); l++)
#endif
                {
#if 0
                DPoint3dP linePtsP2 = new DPoint3d[2];
                linePtsP2[0].x = drapePtsP[l].drapeX;
                linePtsP2[0].y = drapePtsP[l].drapeY;
                linePtsP2[0].z = drapePtsP[l].drapeZ;
                linePtsP2[1].x = drapePtsP[l+1].drapeX;
                linePtsP2[1].y = drapePtsP[l+1].drapeY;
                linePtsP2[1].z = drapePtsP[l+1].drapeZ;
                // found dtmObject
                // Found linePtsP
                //DPoint3d triangle2[4];


                DTM_DRAPE_POINT* drapePtsP2 = 0;
                long numDrapePts2;

                status = bcdtmDrape_stringDtmObject(bcDtmP, linePtsP2, 2, FALSE, &drapePtsP2, &numDrapePts2);

#else


                /* temp */
                triangle[0] = drapeSegments1[l].m_segment.point[0];
                triangle[1] = segmentsTerrainMesh[k].point[0];
                triangle[2] = drapeSegments1[l].m_segment.point[1];

                builder4->AddTriStrip(triangle, NULL, NULL, 3, true);


                triangle[0] = segmentsTerrainMesh[k].point[0];
                triangle[1] = drapeSegments1[l].m_segment.point[1];
                triangle[2] = segmentsTerrainMesh[k].point[1];
                builder4->AddTriStrip(triangle, NULL, NULL, 3, true);
                /**/

                bvector<DrapeSegment> drapeSegments2;
                DSegment3d segment;
                segment = drapeSegments1[l].m_segment;
                polyfaceSearch2.DoDrapeXY(segment, drapeSegments2);
                size_t size = drapeSegments2.size();
#endif
#if 0
                for(int j=0; j<numDrapePts2-1; j++)
#else
                for(int j=0; j<size; j++)
#endif
                    {
#if 0
                    DPoint3d pts1, pts2;
                    pts1.x = drapePtsP2[j].drapeX;
                    pts1.y = drapePtsP2[j].drapeY;
                    pts1.z = drapePtsP2[j].drapeZ;
                    pts2.x = drapePtsP2[j+1].drapeX;
                    pts2.y = drapePtsP2[j+1].drapeY;
                    pts2.z = drapePtsP2[j+1].drapeZ;
                    triangle[0] = pts1;
                    triangle[1] = linePtsP2[0];
                    //                triangle[1] = segment.point[0];
                    triangle[2] = pts2;
                    builder4->AddTriStrip(triangle, NULL, NULL, 3, true);

                    triangle[0] = linePtsP2[0];
                    triangle[1] = pts2;
                    triangle[2] = linePtsP2[1];
                    builder4->AddTriStrip(triangle, NULL, NULL, 3, true);
#else

                    
                    triangle[0] = drapeSegments2[j].m_segment.point[0];
                    triangle[1] = segment.point[0];
                    triangle[2] = drapeSegments2[j].m_segment.point[1];

//                    builder4->AddTriStrip(triangle, NULL, NULL, 3, true);


                    triangle[0] = segment.point[0];
                    triangle[1] = drapeSegments2[j].m_segment.point[1];
                    triangle[2] = segment.point[1];
//                    builder4->AddTriStrip(triangle, NULL, NULL, 3, true);
#endif
                    /*
                    DSegment3dSizeSize segmentTerrain(drapeSegments2[j].m_segment, drapeSegments2[j].m_facetReadIndex, NULL);
                    segmentsTerrain.push_back(segmentTerrain);*/
                    areTriangle = true;
                    }
                /*if(size > 0)
                    {
                    triangle[0] = segment.point[0];
                    triangle[1] = drapeSegments2[size-1].m_segment.point[1];
                    triangle[2] = segment.point[1];
                    builder4->AddTriStrip(triangle, NULL, NULL, 3, true);
                    }*/
                }
            }
        PolyfaceHeaderPtr meshClosed2;
        meshClosed2 = builder4->GetClientMeshPtr();
        if(areTriangle)
            volumeMeshVector.push_back(meshClosed2);
/*
        bvector<DSegment3dSizeSize> segmentsTest;
        PolyfaceQuery::SearchIntersectionSegments (*meshTerrain, *meshClosed, segmentsTest);
        size_t segSize = segmentsTest.size();
        for(int i=0; i<segSize; i++)
            {
            //segmentsTest[i].
            }
      */  
        meshClosed->Triangulate();

        // Write Mesh in file
        {
        FILE* pOutputFileStream = fopen("C:\\Users\\Thomas.Butzbach\\Documents\\data_scalableMesh\\ATP\\terrainMesh.xml", "w+");

        char TempBuffer[500];
        int  NbChars;

        
        PolyfaceVisitorPtr visitorPtr = PolyfaceVisitor::Attach(*terrainMesh, false);
        PolyfaceVisitor & visitor = *visitorPtr.get();
//        DPoint3d triangle[4];
        const DPoint3d* ptCP = terrainMesh->GetPointCP();
        for(visitor.Reset(); visitor.AdvanceToNextFace();)
            {
            //WString tmp("Point(" + ptCP[visitor.ClientPointIndex()[0]].x + ", " +ptCP[visitor.ClientPointIndex()[1]].y + ", " + ptCP[visitor.ClientPointIndex()[2]].z + ")\n");
            //WString tmp += ptCP[visitor.ClientPointIndex()[0]];
            NbChars = sprintf(TempBuffer, "Point(%f, %f, %f) - Point(%f, %f, %f) - Point(%f, %f, %f)\n",
                ptCP[visitor.ClientPointIndex()[0]].x, ptCP[visitor.ClientPointIndex()[0]].y, ptCP[visitor.ClientPointIndex()[0]].z,
                ptCP[visitor.ClientPointIndex()[1]].x, ptCP[visitor.ClientPointIndex()[1]].y, ptCP[visitor.ClientPointIndex()[1]].z,
                ptCP[visitor.ClientPointIndex()[2]].x, ptCP[visitor.ClientPointIndex()[2]].y, ptCP[visitor.ClientPointIndex()[2]].z);
            size_t NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pOutputFileStream);
            HASSERT(NbWrittenChars == NbChars);
            }

        fclose(pOutputFileStream);
        }


        {
        FILE* pOutputFileStream = fopen("C:\\Users\\Thomas.Butzbach\\Documents\\data_scalableMesh\\ATP\\mesh.xml", "w+");

        char TempBuffer[500];
        int  NbChars;


        PolyfaceVisitorPtr visitorPtr = PolyfaceVisitor::Attach(*meshClosed, false);
        PolyfaceVisitor & visitor = *visitorPtr.get();
        //        DPoint3d triangle[4];
        const DPoint3d* ptCP = meshClosed->GetPointCP();
        for(visitor.Reset(); visitor.AdvanceToNextFace();)
            {
            //WString tmp("Point(" + ptCP[visitor.ClientPointIndex()[0]].x + ", " +ptCP[visitor.ClientPointIndex()[1]].y + ", " + ptCP[visitor.ClientPointIndex()[2]].z + ")\n");
            //WString tmp += ptCP[visitor.ClientPointIndex()[0]];
            NbChars = sprintf(TempBuffer, "Point(%f, %f, %f) - Point(%f, %f, %f) - Point(%f, %f, %f)\n",
                ptCP[visitor.ClientPointIndex()[0]].x, ptCP[visitor.ClientPointIndex()[0]].y, ptCP[visitor.ClientPointIndex()[0]].z,
                ptCP[visitor.ClientPointIndex()[1]].x, ptCP[visitor.ClientPointIndex()[1]].y, ptCP[visitor.ClientPointIndex()[1]].z,
                ptCP[visitor.ClientPointIndex()[2]].x, ptCP[visitor.ClientPointIndex()[2]].y, ptCP[visitor.ClientPointIndex()[2]].z);
            size_t NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pOutputFileStream);
            HASSERT(NbWrittenChars == NbChars);
            }

        fclose(pOutputFileStream);
        }






        bvector<PolyfaceHeaderPtr> enclosedVolumes;
        //PolyfaceQuery::MergeAndCollectVolumes (*meshTerrain, *meshClosed, enclosedVolumes);

        //IFacetOptionsPtr options = IFacetOptions::Create();
        IPolyfaceConstructionPtr builder = IPolyfaceConstruction::New(*options);
        builder->AddPolyface(*meshClosed);
        builder->AddPolyface(*meshClosed2);
        PolyfaceHeaderPtr finalMesh;
        finalMesh = builder->GetClientMeshPtr();

        //meshClosed->AddPolyface(meshClosed2);
        //PolyfaceQuery::MergeAndCollectVolumes (*terrainMesh, *meshClosed, enclosedVolumes);
        PolyfaceQuery::MergeAndCollectVolumes (*terrainMesh, *finalMesh, enclosedVolumes);
//        volumeMeshVector.push_back(meshClosed);
        //volumeMeshVector.push_back(&mesh);
//        volumeMeshVector.push_back(meshTerrain);

/*
        enclosedVolumes.clear ();
        bvector<DSegment3dSizeSize> segments2;
        PolyfaceQuery::SearchIntersectionSegments (*meshTerrain, *meshClosed, segments2);

        IFacetOptionsPtr options2 = IFacetOptions::Create ();
        //static int s_maxPerFace = 4;
        options2->SetMaxPerFace (4);
        IPolyfaceConstructionPtr builder4 = IPolyfaceConstruction::Create (*options2);

        PolyfaceQuery::CopyFacetsWithSegmentSplitImprint (*builder4, *meshTerrain, segments2, false);
        PolyfaceQuery::CopyFacetsWithSegmentSplitImprint (*builder4, *meshClosed, segments2, true);

        StitchAndCollectVolumesFromImprintedMesh (builder4->GetClientMeshR (), *options2, enclosedVolumes);
        */


//        volumeMeshVector.push_back(meshTerrain);
//        volumeMeshVector.push_back(meshClosed);
        //volumeMeshVector.push_back(terrainMesh);
//        PolyfaceQuery::MergeAndCollectVolumes (*terrainMesh, mesh, enclosedVolumes);
//        DPoint3d origin = DPoint3d::FromZero ();
        for (size_t i = 0; i < enclosedVolumes.size (); i++)
            {
//            volumeMeshVector.push_back(enclosedVolumes[i]);
            static double volumeScale = 1.0e12;
            DPoint3d origin;
            origin.Zero ();
            double volume1 = enclosedVolumes[i]->SumTetrahedralVolumes (origin);
            
            //PolyfaceQuery::ComputeCutAndFill(*terrainMesh, mesh, cutSections, fillSections);
            //double v = enclosedVolumes[i]->SumTetrahedralVolumes (origin);
            double sectionFill = 0.0;
            DPoint3d centroid;
            RotMatrix axes;
            DVec3d moments;
            //if (!polyfaceP->ComputePrincipalMomentsAllowMissingSideFacets(sectionFill, centroid, axes, moments, true))
            if (!enclosedVolumes[i]->ComputePrincipalMomentsAllowMissingSideFacets(sectionFill, centroid, axes, moments, true))
                {
                }

            volume1 = enclosedVolumes[i]->SumTetrahedralVolumes (centroid);

            assert(sectionFill!=volume1);
            // Si > 0 alors ouvert
            PolyfaceHeaderPtr volumeTest;
            bool test = enclosedVolumes[i]->HealVerticalPanels(*enclosedVolumes[i], true, false, volumeTest) > 0;
            assert(test);

            // Vertical direction ray
            DVec3d drapeDirection = DVec3d::From(0, 0, -1);
            DRay3d ray = DRay3d::FromOriginAndVector(centroid, drapeDirection); 
            
            // Get intersection ray with meshes
            //PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach(*enclosedVolumes[i]);
            double maxDist = std::numeric_limits<double>::max();
            PolyfaceVisitorPtr visitorTerrain = PolyfaceVisitor::Attach(*terrainMesh);
            PolyfaceVisitorPtr visitorMesh = PolyfaceVisitor::Attach(*finalMesh);
//            PolyfaceVisitorPtr visitorMesh = PolyfaceVisitor::Attach(mesh);
            DPoint3d point, edgePoint;
            double edgeFraction, edgeDistance;
            double fractionTerrain, fractionMesh;
            ptrdiff_t edgeIndex;
            FacetLocationDetail detail;
            visitorTerrain->AdvanceToFacetBySearchRay(ray, maxDist, point, fractionTerrain, edgeIndex, edgeFraction, edgePoint, edgeDistance);

            visitorMesh->AdvanceToFacetBySearchRay(ray, maxDist, point, fractionMesh, edgeIndex, edgeFraction, edgePoint, edgeDistance);

            if(fractionMesh > fractionTerrain || fractionMesh < 0)
                totalFill += fabs(sectionFill);
            else
                totalCut += fabs(sectionFill);
/*            for(; visitor->AdvanceToFacetBySearchRay(ray, detail);)
                {
                detail
                }*/

            // Cut / fill if terrain firrst => cut, if road first => fill

            //totalFill += fabs(sectionFill);
//            printf (" (Mesh %d)  (volume %.8g)\n", (int)i, v);
            //totalCut+=v;
            //totalFill+=v;
            }





        totalVolume = totalCut - totalFill;
        }
        cut = totalCut;
        fill = totalFill;
        return DTMStatusInt::DTM_SUCCESS;
    }

double ScalableMeshVolume::_ComputeVolumeCutAndFillForTile(IScalableMeshMeshPtr smTile, double& cut, double& fill, PolyfaceHeader& mesh, bool is2d, DRange3d meshExtent, bvector<PolyfaceHeaderPtr>& volumeMeshVector)
    {
    double totalVolume = 0.0, totalCut = 0.0, totalFill = 0.0;
#ifdef SCALABLE_MESH_ATP
    int64_t tiles;
    IScalableMeshATP::GetInt(L"nTiles", tiles);
    tiles++;
    IScalableMeshATP::StoreInt(L"nTiles", tiles);
#endif
    if (!is2d)
        {
        
        PolyfaceHeaderPtr terrainMesh;
        bvector<PolyfaceHeaderPtr> cutSections, fillSections;
        IFacetOptionsPtr options = IFacetOptions::Create();
        IPolyfaceConstructionPtr builder = IPolyfaceConstruction::New(*options);

        const PolyfaceQuery* polyface = smTile->GetPolyfaceQuery();
        PolyfaceVisitorPtr visitorPtr = PolyfaceVisitor::Attach(*polyface, false);
        PolyfaceVisitor & visitor = *visitorPtr.get();
        DPoint3d triangle[4];
        const DPoint3d* ptCP = polyface->GetPointCP();
        for(visitor.Reset(); visitor.AdvanceToNextFace();)
            {
            triangle[0] = ptCP[visitor.ClientPointIndex()[0]];
            triangle[1] = ptCP[visitor.ClientPointIndex()[1]];
            triangle[2] = ptCP[visitor.ClientPointIndex()[2]];
            DRange3d triRange = DRange3d::From(triangle[0], triangle[1], triangle[2]);
            if(meshExtent.high.x >= triRange.low.x && meshExtent.high.y >= triRange.low.y && meshExtent.low.x <= triRange.high.x && meshExtent.low.y <= triRange.high.y)
                builder->AddTriStrip(triangle, NULL, NULL, 3, true);
            }
        terrainMesh = builder->GetClientMeshPtr();

        /*Transform uorToMeter, meterToUor;
        GetTransformForPoints(uorToMeter, meterToUor);
        terrainMesh->Transform(uorToMeter);
        bsiTransform_multiplyDPoint3dArrayInPlace(&uorToMeter, &elemRange.low, 1);
        bsiTransform_multiplyDPoint3dArrayInPlace(&uorToMeter, &elemRange.high, 1);
        Transform refToActiveTrf;
        GetFromModelRefToActiveTransform(refToActiveTrf, terrainMesh->GetModelRef());
        terrainMesh->Transform(refToActiveTrf);
        bsiTransform_multiplyDPoint3dArrayInPlace(&refToActiveTrf, &elemRange.low, 1);
        bsiTransform_multiplyDPoint3dArrayInPlace(&refToActiveTrf, &elemRange.high, 1);*/
        PolyfaceHeaderPtr meshTerrainClosed;
            {

            // Find contour !
            
            /*MTGGraph* meshGraph = node->GetGraphPtr();
            MTGARRAY_SET_LOOP(edgeID, meshGraph)
            MTGARRAY_SET_LOOP(edgeId, graphP);
            MTGARRAY_FACE_LOOP(EdgeId, graphP, );
            MTG_EXTERIOR_MASK
            };*/
            
            /*std::vector<DPoint3d> nodePoints(node->size());

            for (size_t i = 0; i < node->size(); i++)
                {
                nodePoints[i].x = (*node)[i].x;
                nodePoints[i].y = (*node)[i].y;
                nodePoints[i].z = (*node)[i].z;
                }
                */
            bvector<bvector<DSegment3d>> bound;
//            MTGGraph* graphP = node->GetGraphPtr();
//            GetGraphExteriorBoundary(graphP, bound, points);
//            smTile->GetExteriorBoundary(bound, smTile->GetPoints());



/*        terrainMesh->MarkTopologicalBoundariesVisible(true);

        bvector<DSegment3d> segments;*/
        bvector<DSegment3dSizeSize> segmentsTerrain;
        

//        terrainMesh->CollectSegments(segments, true);

        PolyfaceHeaderPtr meshPtr(&mesh, true);

        PolyfaceSearchContext polyfaceSearch1(meshPtr, true, true, false);
        PolyfaceSearchContext polyfaceSearch2(terrainMesh, true, true, false);
        IFacetOptionsPtr options2 = IFacetOptions::Create();
        IPolyfaceConstructionPtr builder2 = IPolyfaceConstruction::New(*options2);
//        builder2->AddPolyface(*terrainMesh);
        builder2->AddPolyface(*meshPtr);

        // Mesh for ground with split edge for drapping line.
        IPolyfaceConstructionPtr builder3 = IPolyfaceConstruction::New(*options2);
        bool areTriangle = false;
        for(size_t k = 0; k < bound.size(); k++)
            {
            bvector<DrapeSegment> drapeSegments1;
            for(size_t l = 0; l < bound[k].size(); l++)
                {
                
                polyfaceSearch1.DoDrapeXY(bound[k][l], drapeSegments1);
                //}
            for(size_t i=0; i<drapeSegments1.size(); i++)
                {
                bvector<DrapeSegment> drapeSegments2;
                DSegment3d segment;
                segment = drapeSegments1[i].m_segment;
                polyfaceSearch2.DoDrapeXY(segment, drapeSegments2);
                size_t size = drapeSegments2.size();
                DPoint3d triangle[4];

                for(int j=0; j<size; j++)
                    {                
                    triangle[0] = drapeSegments2[j].m_segment.point[0];
                    triangle[1] = segment.point[0];

                    triangle[2] = drapeSegments2[j].m_segment.point[1];
                    builder2->AddTriStrip(triangle, NULL, NULL, 3, true);

                    DSegment3dSizeSize segmentTerrain(drapeSegments2[j].m_segment, drapeSegments2[j].m_facetReadIndex, NULL);
                    segmentsTerrain.push_back(segmentTerrain);
                    areTriangle = true;
                    }
                if(size>0)
                    {
                    triangle[0] = segment.point[0];
                    triangle[1] = drapeSegments2[size-1].m_segment.point[1];
                    triangle[2] = segment.point[1];
                    builder2->AddTriStrip(triangle, NULL, NULL, 3, true);
                    }
                }
                }
            }
        meshTerrainClosed = builder2->GetClientMeshPtr();
        if(areTriangle)
            {
            
            
            volumeMeshVector.push_back(meshTerrainClosed);
            }
            }

            

            PolyfaceHeaderPtr meshPtr(&mesh, true);
        mesh.MarkTopologicalBoundariesVisible(true); // mesh->MarkTopologicalBoundaries(2pi);
//        size_t numOpen =0, numClosed = 0;
//        CurveVectorPtr curveBoundary = mesh.ExtractBoundaryStrings(numOpen, numClosed);
        bvector<DSegment3d> segments;
        bvector<DSegment3dSizeSize> segmentsTerrain;
        bvector<DrapeSegment> drapeSegments1, drapeSegments2;
        //curveBoundary->get
        mesh.CollectSegments(segments, true);
        //PolyfaceHeaderPtr meshPtr(&mesh, true);
        PolyfaceSearchContext polyfaceSearch1(terrainMesh, true, true, false);
        PolyfaceSearchContext polyfaceSearch2(meshPtr, true, true, false);
        IFacetOptionsPtr options2 = IFacetOptions::Create();
        IPolyfaceConstructionPtr builder2 = IPolyfaceConstruction::New(*options2);
//        builder2->AddPolyface(mesh);
        builder2->AddPolyface(*meshTerrainClosed);

        // Mesh for ground with split edge for drapping line.
        IPolyfaceConstructionPtr builder3 = IPolyfaceConstruction::New(*options2);
        //builder->AddPolyface(*terrainMesh);
        //bvector<DPoint3d> points;
        //for(int i=0; i<curveBoundary->)
        bool areTriangle = false;
        for(size_t k = 0; k < segments.size(); k++)
/*        DPoint3d first;
        DPoint3d last;
        size_t size = curveBoundary->size();
        bool bfirst = true;*/
//        for (ICurvePrimitivePtr curvePrimitive : *curveBoundary)
            {
/*            DSegment3d segment;
            bvector<DPoint3d>* points = curvePrimitive->GetLineStringP();
            size_t sizePts = points->size();
            if(!curvePrimitive->TryGetLine(segment))
                assert(true);

            //DSegment3d segment;
            for(int i=0; i<sizePts; i+=2)
                {
                if(bfirst)
                    first =  points->at(i);
                bfirst = false;
                segment.point[0] = points->at(i);
                segment.point[1] = points->at(i+1);
                last = points->at(i+1);
                }*/

            //polyfaceSearch.DoDrapeXY(segment, drapeSegments);
            polyfaceSearch1.DoDrapeXY(segments[k], drapeSegments1);


            for(size_t l=0; l<drapeSegments1.size(); l++)
                {
                bvector<DrapeSegment> drapeSegments2;
                DSegment3d segment;
                segment = drapeSegments1[l].m_segment;
                polyfaceSearch2.DoDrapeXY(segment, drapeSegments2);
                size_t size = drapeSegments2.size();
                DPoint3d triangle[4];

            for(int j=0; j<size; j++)
                {
                /*triangle[0] = segments[i].point[0];
                triangle[1] = drapeSegments[j].m_segment.point[0];
                triangle[2] = segments[i].point[1];
                builder2->AddTriStrip(triangle, NULL, NULL, 3, true);*/
                
                triangle[0] = drapeSegments2[j].m_segment.point[0];
                triangle[1] = segment.point[0];

//                triangle[1] = drapeSegments[j].m_drapeEdgeFraction[0] < 0.5 ? segments[i].point[0] : segments[i].point[1];
//                triangle[1] = segment.point[0];
                triangle[2] = drapeSegments2[j].m_segment.point[1];
                builder2->AddTriStrip(triangle, NULL, NULL, 3, true);
                // add facet between segments and drapeSegments.
                //PolyfaceQuery::CopyFacetsWithSegmentSplitImprint (*builder2, mesh, drapeSegments[j], true);

                //triangle[0]
                //builder->AddTriStrip(triangle, NULL, NULL, 3, true);
                DSegment3dSizeSize segmentTerrain(drapeSegments2[j].m_segment, drapeSegments2[j].m_facetReadIndex, NULL);
                segmentsTerrain.push_back(segmentTerrain);
                areTriangle=true;
                //points.push_back(drapeSegments[j].m_segment.point[0]);
                //points.push_back(drapeSegments[j].m_segment.point[1]);
                }
            if(size>0)
                {
                triangle[0] = segment.point[0];
//                triangle[0] = segment.point[0];
                triangle[1] = drapeSegments2[size-1].m_segment.point[1];
                triangle[2] = segment.point[1];
//                triangle[2] = segment.point[1];
                builder2->AddTriStrip(triangle, NULL, NULL, 3, true);
                }
                }
            }

        /*smTile->

        MTGARRAY_END_SET_LOOP(edgeID, graphP)

            MTGARRAY_FACE_LOOP(edgeID, graphP, extEdge)
            {
             meshGraph->GetMaskAt(edgeID, MTG_EXTERIOR_MASK)
            }
        MTGARRAY_END_FACE_LOOP(edgeID, graphP, extEdge)*/

        //builder2->AddPolygon(points);
        /*
        DSegment3d segment;
        segment.point[0] = first;
        segment.point[1] = last;

        polyfaceSearch.DoDrapeXY(segment, drapeSegments);
            size = drapeSegments.size();
            for(int j=0; j<size; j++)
                {
                /*triangle[0] = segments[i].point[0];
                triangle[1] = drapeSegments[j].m_segment.point[0];
                triangle[2] = segments[i].point[1];
                builder2->AddTriStrip(triangle, NULL, NULL, 3, true);*/
 /*               
                triangle[0] = drapeSegments[j].m_segment.point[0];
                //triangle[1] = segments[i].point[0];
                triangle[1] = segment.point[0];
                triangle[2] = drapeSegments[j].m_segment.point[1];
                builder2->AddTriStrip(triangle, NULL, NULL, 3, true);
                // add facet between segments and drapeSegments.
                //PolyfaceQuery::CopyFacetsWithSegmentSplitImprint (*builder2, mesh, drapeSegments[j], true);

                //triangle[0]
                //builder->AddTriStrip(triangle, NULL, NULL, 3, true);
                }
            if(size>0)
                {
                //triangle[0] = segments[i].point[0];
                triangle[0] = segment.point[0];
                triangle[1] = drapeSegments[size-1].m_segment.point[1];
                //triangle[2] = segments[i].point[1];
                triangle[2] = segment.point[1];
                builder2->AddTriStrip(triangle, NULL, NULL, 3, true);
                }
            */


        /*PolyfaceQuery::CopyFacetsWithSegmentSplitImprint(*builder3, *terrainMesh, segmentsTerrain, true);
        PolyfaceHeaderPtr meshTerrain;
        meshTerrain = builder3->GetClientMeshPtr();
        */
            
        PolyfaceHeaderPtr meshClosed;
        meshClosed = builder2->GetClientMeshPtr();
        /*bvector<DSegment3dSizeSize> segmentsTest;
        PolyfaceQuery::SearchIntersectionSegments (*meshTerrain, *meshClosed, segmentsTest);
        size_t segSize = segmentsTest.size();
        for(int i=0; i<segSize; i++)
            {
            //segmentsTest[i].
            }*/

        //meshClosed->Triangulate();
        if(areTriangle)
            {
//        volumeMeshVector.push_back(meshClosed);
                }
        {
        FILE* pOutputFileStream = fopen("C:\\Users\\Thomas.Butzbach\\Documents\\data_scalableMesh\\ATP\\terrainMeshSTM.xml", "w+");

        char TempBuffer[500];
        int  NbChars;

        
        //PolyfaceVisitorPtr visitorPtr2 = PolyfaceVisitor::Attach(*terrainMesh, false);
        PolyfaceVisitorPtr visitorPtr2 = PolyfaceVisitor::Attach(*meshTerrainClosed, false);
        PolyfaceVisitor & visitor2 = *visitorPtr2.get();
        //        DPoint3d triangle[4];
        const DPoint3d* ptCP2 = meshTerrainClosed->GetPointCP();
        for(visitor2.Reset(); visitor2.AdvanceToNextFace();)
            {
            //WString tmp("Point(" + ptCP[visitor.ClientPointIndex()[0]].x + ", " +ptCP[visitor.ClientPointIndex()[1]].y + ", " + ptCP[visitor.ClientPointIndex()[2]].z + ")\n");
            //WString tmp += ptCP[visitor.ClientPointIndex()[0]];
            NbChars = sprintf(TempBuffer, "Point(%f, %f, %f) - Point(%f, %f, %f) - Point(%f, %f, %f)\n",
                ptCP2[visitor2.ClientPointIndex()[0]].x, ptCP2[visitor2.ClientPointIndex()[0]].y, ptCP2[visitor2.ClientPointIndex()[0]].z,
                ptCP2[visitor2.ClientPointIndex()[1]].x, ptCP2[visitor2.ClientPointIndex()[1]].y, ptCP2[visitor2.ClientPointIndex()[1]].z,
                ptCP2[visitor2.ClientPointIndex()[2]].x, ptCP2[visitor2.ClientPointIndex()[2]].y, ptCP2[visitor2.ClientPointIndex()[2]].z);
            size_t NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pOutputFileStream);
            HASSERT(NbWrittenChars == NbChars);
            }

        fclose(pOutputFileStream);
        }

        {
        FILE* pOutputFileStream = fopen("C:\\Users\\Thomas.Butzbach\\Documents\\data_scalableMesh\\ATP\\meshSTM.xml", "w+");

        char TempBuffer[500];
        int  NbChars;


        PolyfaceVisitorPtr visitorPtr2 = PolyfaceVisitor::Attach(*meshClosed, false);
        PolyfaceVisitor & visitor2 = *visitorPtr2.get();
        //        DPoint3d triangle[4];
        const DPoint3d* ptCP2 = meshClosed->GetPointCP();
        for(visitor2.Reset(); visitor2.AdvanceToNextFace();)
            {
            //WString tmp("Point(" + ptCP[visitor.ClientPointIndex()[0]].x + ", " +ptCP[visitor.ClientPointIndex()[1]].y + ", " + ptCP[visitor.ClientPointIndex()[2]].z + ")\n");
            //WString tmp += ptCP[visitor.ClientPointIndex()[0]];
            NbChars = sprintf(TempBuffer, "Point(%f, %f, %f) - Point(%f, %f, %f) - Point(%f, %f, %f)\n",
                ptCP2[visitor2.ClientPointIndex()[0]].x, ptCP2[visitor2.ClientPointIndex()[0]].y, ptCP2[visitor2.ClientPointIndex()[0]].z,
                ptCP2[visitor2.ClientPointIndex()[1]].x, ptCP2[visitor2.ClientPointIndex()[1]].y, ptCP2[visitor2.ClientPointIndex()[1]].z,
                ptCP2[visitor2.ClientPointIndex()[2]].x, ptCP2[visitor2.ClientPointIndex()[2]].y, ptCP2[visitor2.ClientPointIndex()[2]].z);
            size_t NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pOutputFileStream);
            HASSERT(NbWrittenChars == NbChars);
            }

        fclose(pOutputFileStream);
        }
        
        bvector<PolyfaceHeaderPtr> enclosedVolumes;
        //PolyfaceQuery::MergeAndCollectVolumes (*terrainMesh, *meshClosed, enclosedVolumes);
        PolyfaceQuery::MergeAndCollectVolumes (*terrainMesh, *meshClosed, enclosedVolumes);
///        PolyfaceQuery::MergeAndCollectVolumes (*meshClosed, *meshTerrainClosed, enclosedVolumes);
//        volumeMeshVector.push_back(meshClosed);
        //volumeMeshVector.push_back(&mesh);
//        volumeMeshVector.push_back(meshTerrain);

        /*
        enclosedVolumes.clear ();
        bvector<DSegment3dSizeSize> segments2;
        //PolyfaceQuery::SearchIntersectionSegments (*meshTerrain, *meshClosed, segments2);
        PolyfaceQuery::SearchIntersectionSegments (*meshTerrainClosed, *meshClosed, segments2);

//        IFacetOptionsPtr options2 = IFacetOptions::Create ();
        //static int s_maxPerFace = 4;
        options2->SetMaxPerFace (4);
        IPolyfaceConstructionPtr builder4 = IPolyfaceConstruction::Create (*options2);

        PolyfaceQuery::CopyFacetsWithSegmentSplitImprint (*builder4, *meshClosed, segments2, false);
        PolyfaceQuery::CopyFacetsWithSegmentSplitImprint (*builder4, *meshTerrainClosed, segments2, true);
        

        StitchAndCollectVolumesFromImprintedMesh (builder4->GetClientMeshR (), *options2, enclosedVolumes);
            
        */

        //volumeMeshVector.push_back(terrainMesh);
        //volumeMeshVector.push_back(meshClosed);
        //volumeMeshVector.push_back(terrainMesh);
//        PolyfaceQuery::MergeAndCollectVolumes (*terrainMesh, mesh, enclosedVolumes);
//        DPoint3d origin = DPoint3d::FromZero ();
        for (size_t i = 0; i < enclosedVolumes.size (); i++)
            {
//            volumeMeshVector.push_back(enclosedVolumes[i]);
            static double volumeScale = 1.0e12;
            DPoint3d origin;
            origin.Zero ();
            double volume1 = enclosedVolumes[i]->SumTetrahedralVolumes (origin);
            
            //PolyfaceQuery::ComputeCutAndFill(*terrainMesh, mesh, cutSections, fillSections);
            //double v = enclosedVolumes[i]->SumTetrahedralVolumes (origin);
            double sectionFill = 0.0;
            DPoint3d centroid;
            RotMatrix axes;
            DVec3d moments;
            //if (!polyfaceP->ComputePrincipalMomentsAllowMissingSideFacets(sectionFill, centroid, axes, moments, true))
            if (!enclosedVolumes[i]->ComputePrincipalMomentsAllowMissingSideFacets(sectionFill, centroid, axes, moments, true))
                {
                }

            volume1 = enclosedVolumes[i]->SumTetrahedralVolumes (centroid);

//            assert(sectionFill!=volume1);
            // Si > 0 alors ouvert
            PolyfaceHeaderPtr volumeTest;
            bool test = enclosedVolumes[i]->HealVerticalPanels(*enclosedVolumes[i], true, false, volumeTest) > 0;
//            assert(test);
            if(!test)
                {

            // Vertical direction ray
            DVec3d drapeDirection = DVec3d::From(0, 0, -1);
            DRay3d ray = DRay3d::FromOriginAndVector(centroid, drapeDirection); 
            
            // Get intersection ray with meshes
            //PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach(*enclosedVolumes[i]);
            double maxDist = std::numeric_limits<double>::max();
            PolyfaceVisitorPtr visitorTerrain = PolyfaceVisitor::Attach(*terrainMesh);
            PolyfaceVisitorPtr visitorMesh = PolyfaceVisitor::Attach(*meshClosed);
//            PolyfaceVisitorPtr visitorMesh = PolyfaceVisitor::Attach(mesh);
            DPoint3d point, edgePoint;
            double edgeFraction, edgeDistance;
            double fractionTerrain, fractionMesh;
            ptrdiff_t edgeIndex;
            FacetLocationDetail detail;
            visitorTerrain->AdvanceToFacetBySearchRay(ray, maxDist, point, fractionTerrain, edgeIndex, edgeFraction, edgePoint, edgeDistance);

            visitorMesh->AdvanceToFacetBySearchRay(ray, maxDist, point, fractionMesh, edgeIndex, edgeFraction, edgePoint, edgeDistance);

            if(fractionMesh > fractionTerrain || fractionMesh < 0)
                totalFill += fabs(sectionFill);
            else
                totalCut += fabs(sectionFill);
/*            for(; visitor->AdvanceToFacetBySearchRay(ray, detail);)
                {
                detail
                }*/

            // Cut / fill if terrain firrst => cut, if road first => fill

            //totalFill += fabs(sectionFill);
//            printf (" (Mesh %d)  (volume %.8g)\n", (int)i, v);
            //totalCut+=v;
            //totalFill+=v;
            }
            }
        /*int maxPerFace, noisy;
        noisy = 1;
        maxPerFace = 10; // 10 or 3 in example Test

        bvector<DSegment3dSizeSize> segments;
        PolyfaceQuery::SearchIntersectionSegments(*terrainMesh, mesh, segments);

        IFacetOptionsPtr options2 = IFacetOptions::Create();
        options2->SetMaxPerFace(maxPerFace);
        IPolyfaceConstructionPtr builder2 = IPolyfaceConstruction::Create(*options2);

        PolyfaceQuery::CopyFacetsWithSegmentSplitImprint(*builder2, *terrainMesh, segments, true);
        PolyfaceQuery::CopyFacetsWithSegmentSplitImprint(*builder2, mesh, segments, false);

        double areaTerrain = terrainMesh->SumFacetAreas();
        double areaMesh = mesh->SumFacetAreas();
        double areaC = builder->GetClientMeshR().SumFacetAreas();*/

        /*MTGFacets * mtgFacets = jmdlMTGFacets_new();
        PolyfaceToMTG(mtgFacets, nullptr, nullptr, builder2->GetClientMeshR(), false, s_absTol, s_relTol);
        
        bvector<bvector<MTGNodeId>> componentNodes, componentFaceNodes, componentVertexNodes;
        MTGGraph *graph = jmdlMTGFacets_getGraph(mtgFacets);
        graph->CollectConnectedComponents(componentNodes);
        graph->CollectConnectedComponents(componentFaceNodes, MTG_ScopeFace);
        graph->CollectConnectedComponents(componentVertexNodes, MTG_ScopeVertex);
        UsageSums volumeSum;
        UsageSums volumeSum1;
        DPoint3d origin;
        origin.Zero();
        DRange3d range1;
        for(size_t componentIndex = 0; componentIndex < componentNodes.size(); componentIndex++)
            {
            size_t numFace, numVertex;
            double volume;
            /*DRange3d range = *//*RangeOfNodes(mtgFacets, componentNodes[componentIndex], numFace, numVertex, volume, noisy);
            volumeSum.Accumulate(volume);
            double volume1 = mtgFacets->SumTetrahedralVolumeFromNodesPerFace(origin, componentFaceNodes[componentIndex]);
            range1 = mtgFacets->RangeFromNodesPerFace(componentFaceNodes[componentIndex]);
            volumeSum1.Accumulate(volume1);
            }

        jmdlMTGFacets_free(mtgFacets);

        /*PolyfaceHeaderPtr mesh1, mesh2;
        int maxPerFace, noisy;
        noisy = 1;
        maxPerFace = 10; // 10 or 3 in example Test
        TestMeshIntersection(mesh1, mesh2, maxPerFace, noisy);*/
        /*totalCut = volumeSum.m_sums[1];
        totalFill = volumeSum1.m_sums[1];*/





        totalVolume = totalCut - totalFill;
        }
    else{
        PolyfaceHeaderPtr terrainMesh;
        bvector<PolyfaceHeaderPtr> cutSections, fillSections;
        IFacetOptionsPtr  options = IFacetOptions::Create();
        IPolyfaceConstructionPtr  builder = IPolyfaceConstruction::New(*options);
        const PolyfaceQuery* polyface = smTile->GetPolyfaceQuery();
        PolyfaceVisitorPtr visitorPtr = PolyfaceVisitor::Attach(*polyface, false);
        PolyfaceVisitor & visitor = *visitorPtr.get();
        DPoint3d triangle[4];
        const DPoint3d* ptCP = polyface->GetPointCP();
        //only add those triangles that intersect the extent of the second mesh, so as to not compute volume unnecessarily on other sections
        for (visitor.Reset(); visitor.AdvanceToNextFace();)
            {
            triangle[0] = ptCP[visitor.ClientPointIndex()[0]];
            triangle[1] = ptCP[visitor.ClientPointIndex()[1]];
            triangle[2] = ptCP[visitor.ClientPointIndex()[2]];
            DRange3d triRange = DRange3d::From(triangle[0], triangle[1], triangle[2]);
            if (meshExtent.high.x >= triRange.low.x && meshExtent.high.y >= triRange.low.y && meshExtent.low.x <= triRange.high.x && meshExtent.low.y <= triRange.high.y) builder->AddTriStrip(triangle, NULL, NULL, 3, true);
            }
        terrainMesh = builder->GetClientMeshPtr();
        PolyfaceQuery::ComputeCutAndFill(*terrainMesh, mesh, cutSections, fillSections);
#ifdef SCALABLE_MESH_ATP
        if (cutSections.size() == 0 && fillSections.size() == 0)
            {
            int64_t nNoCutFillTiles;
            IScalableMeshATP::GetInt(L"nNoCutFillTiles", nNoCutFillTiles);
            nNoCutFillTiles++;
            IScalableMeshATP::StoreInt(L"nNoCutFillTiles", nNoCutFillTiles);
            }
        else
            {
            int64_t nSectionsTotal;
            IScalableMeshATP::GetInt(L"nSectionsTotal", nSectionsTotal);
            nSectionsTotal += cutSections.size() + fillSections.size();
            IScalableMeshATP::StoreInt(L"nSectionsTotal", nSectionsTotal);
            }
#endif
        for (auto& polyfaceP : cutSections)
            {
            volumeMeshVector.push_back(polyfaceP);
            double sectionCut = 0.0;
            DPoint3d centroid;
            RotMatrix axes;
            DVec3d moments;
            if (!polyfaceP->ComputePrincipalMomentsAllowMissingSideFacets(sectionCut, centroid, axes, moments, true))
                {
#ifdef SCALABLE_MESH_ATP
                int64_t nFailedComputePrincipalMoments;
                IScalableMeshATP::GetInt(L"nFailedComputePrincipalMoments", nFailedComputePrincipalMoments);
                nFailedComputePrincipalMoments++;
                IScalableMeshATP::StoreInt(L"nFailedComputePrincipalMoments", nFailedComputePrincipalMoments);
#endif
                }
            totalCut += fabs(sectionCut);
            }
        for (auto& polyfaceP : fillSections)
            {
            volumeMeshVector.push_back(polyfaceP);
            double sectionFill = 0.0;
            DPoint3d centroid;
            RotMatrix axes;
            DVec3d moments;
            if (!polyfaceP->ComputePrincipalMomentsAllowMissingSideFacets(sectionFill, centroid, axes, moments, true))
                {
#ifdef SCALABLE_MESH_ATP
                int64_t nFailedComputePrincipalMoments;
                IScalableMeshATP::GetInt(L"nFailedComputePrincipalMoments", nFailedComputePrincipalMoments);
                nFailedComputePrincipalMoments++;
                IScalableMeshATP::StoreInt(L"nFailedComputePrincipalMoments", nFailedComputePrincipalMoments);
#endif
                }
            totalFill += fabs(sectionFill);
            }
        totalVolume = totalCut - totalFill;
        }
    cut = totalCut;
    fill = totalFill;
    return totalVolume;
    }

DTMStatusInt ScalableMeshVolume::_ComputeVolumeCutAndFill(double& cut, double& fill, double& area, PolyfaceHeader& intersectingMeshSurface, DRange3d& meshRange, bvector<PolyfaceHeaderPtr>& volumeMeshVector)
    {
    double totalVolume = 0.0, totalCut = 0.0, totalFill = 0.0;

    IScalableMeshMeshQueryPtr meshQueryInterface = ((IScalableMesh*)m_scmPtr.get())->GetMeshQueryInterface(MESH_QUERY_FULL_RESOLUTION);
    bvector<IScalableMeshNodePtr> returnedNodes;
    IScalableMeshMeshQueryParamsPtr params = IScalableMeshMeshQueryParams::CreateParams();
    DRange3d fileRange;
    ((IDTM*)m_scmPtr.get())->GetRange(fileRange);
    DPoint3d box[4] = {
        DPoint3d::From(meshRange.low.x, meshRange.low.y, fileRange.low.z),
        DPoint3d::From(meshRange.low.x, meshRange.high.y, fileRange.low.z),
        DPoint3d::From(meshRange.high.x, meshRange.low.y, fileRange.high.z),
        DPoint3d::From(meshRange.high.x, meshRange.high.y, fileRange.high.z)
        };
    meshQueryInterface->Query(returnedNodes, box, 4, params);
    for (auto& node : returnedNodes)
        {
        IScalableMeshMeshPtr scalableMesh = node->GetMesh(false);
        //ScalableMeshMeshWithGraphPtr scalableMeshWithGraph((ScalableMeshMeshWithGraph*)scalableMesh.get(), true);
        double tileCut, tileFill;
        totalVolume += _ComputeVolumeCutAndFillForTile(scalableMesh, tileCut, tileFill, intersectingMeshSurface, true, meshRange, volumeMeshVector);
        totalCut += tileCut;
        totalFill += tileFill;
        }
    cut = totalCut;
    fill = totalFill;
    return DTMStatusInt::DTM_SUCCESS;
    }

ScalableMeshVolume::ScalableMeshVolume(IScalableMeshPtr scMesh) : m_scmPtr(scMesh) {}

END_BENTLEY_SCALABLEMESH_NAMESPACE
