/*--------------------------------------------------------------------------------------+
|
|     $Source: Formats/bcdtmMX.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "TerrainModel/TerrainModel.h"
#include "TerrainModel/Formats/Formats.h"
#include "TerrainModel/Core/bcDTMBaseDef.h"
#include "TerrainModel/Core/dtmdefs.h"
#include "TerrainModel/Core/dtmevars.h"
#include <TerrainModel/Core/bcdtmInlines.h> 
#include "mxtriangle.h"
#include "TerrainModel/Formats/MX.h"
#include <stack>

#define MARKER_EXTERNAL 8
#define MARKER_EXTERNAL_HULL 16

#define asLong(a) *((long*)a)


BENTLEYDTM_Private int bcdtmFormatMX_getMxTriangleNumberDtmObject(BC_DTM_OBJ *dtmP, DTM_MX_TRG_INDEX *trgIndexP, long trgPnt1, long trgPnt2, long trgPnt3, long *trgNumP);
// MX Dtm class to store all points and data structure to make it easier passing to arguments.

/* MX Specific code */
class MXDTM
    {
    private:
        const MXTriangle::TriangleArray& m_triangles;
        const MXTriangle::PointArray& m_points;
        BC_DTM_OBJ* m_dtmP;
        PartitionArray<int, 15, MAllocAllocator> m_pointMapper;

    public:
        MXDTM(const MXTriangle::TriangleArray& triPtr, const MXTriangle::PointArray& pointsPtr, BC_DTM_OBJ *dtmP) : m_triangles(triPtr), m_points(pointsPtr), m_dtmP(dtmP)
            {
            }

        inline DPoint3d& Point(long num)
            {
            return *((DPoint3d*)&m_points[num]);
            }
        inline long NumPoints()
            {
            return m_points.size();
            }

        inline long NumTriangles()
            {
            return m_triangles.size();
            }
        inline long TrianglePointNumber(long triNum, long vertex)
            {
            return m_triangles[triNum].ptNum[vertex];
            }
        inline long GetAdjTriangle(long triNum, long side)
            {
            return m_triangles[triNum].adjTri[side];
            }
        inline long TriangleStringLink(long triNum, long vertex)
            {
            return m_triangles[triNum].strings[vertex];
            }

        inline long TriangleGroupCode(long triNum)
            {
            return m_triangles[triNum].groupCode;
            }


            /*-------------------------------------------------------------------+
            |                                                                    |
            |                                                                    |
            |                                                                    |
            +-------------------------------------------------------------------*/
    private: int GetStringCodeBetweenPoints(int pnt1, int pnt2, int& stringCode)
                 {
                 DTM_TIN_POINT_FEATURES *lineFeaturesP=NULL ;
                 long numLineFeatures;

                 stringCode = 0;
                 if( bcdtmList_getDtmFeaturesForLineDtmObject(m_dtmP,pnt1,pnt2,&lineFeaturesP,&numLineFeatures) == DTM_SUCCESS)
                     {
                     if(numLineFeatures != 0)
                         {
                         long code = -1;
                         for(int i = 0; i < numLineFeatures; i++)
                             {
                             if(lineFeaturesP[i].dtmFeatureType != DTMFeatureType::Hole && 
                                 lineFeaturesP[i].dtmFeatureType != DTMFeatureType::Hull &&
                                 lineFeaturesP[i].dtmFeatureType != DTMFeatureType::Island &&
                                 lineFeaturesP[i].dtmFeatureType != DTMFeatureType::Region &&
                                 lineFeaturesP[i].dtmFeatureType != DTMFeatureType::Void)
                                 {
                                 if(lineFeaturesP[i].userTag != 0)
                                     {
                                     char* cUserTag = (char*)&lineFeaturesP[i].userTag;

                                     if(isalnum(cUserTag[0]) && isalnum(cUserTag[1]) && isalnum(cUserTag[2]) && isalnum(cUserTag[3]))
                                         {
                                         code = (long)(lineFeaturesP[i].userTag & 0xffffffff);
                                         break;
                                         }
                                     else
                                         code = asLong("_DTM");
                                     }
                                 }
                             }
                         if(code != -1)
                             stringCode = code;
                         }
                     }
                 if( lineFeaturesP != NULL ) free(lineFeaturesP) ;

                 return DTM_SUCCESS;
                 }

    public: int LoadTriangle(int trgNum, int trgPnt1, int trgPnt2,int trgPnt3, int voidTriangle, int side1Trg, int side2Trg, int side3Trg)
                {
                MXTriangle::TriangleArray& triPtr = const_cast<MXTriangle::TriangleArray&>(m_triangles);
                MXTriangle::triangle* tri = const_cast<MXTriangle::triangle*>(&triPtr[trgNum]);

                tri->ptNum[0] = m_pointMapper[trgPnt1];
                tri->ptNum[1] = m_pointMapper[trgPnt2];
                tri->ptNum[2] = m_pointMapper[trgPnt3];

                tri->adjTri[0] = side1Trg;
                tri->adjTri[1] = side2Trg;
                tri->adjTri[2] = side3Trg;

                if(voidTriangle)
                    tri->groupCode = asLong("NULL");
                else
                    tri->groupCode = 0;

                // Need to find the strings.
                GetStringCodeBetweenPoints(trgPnt1, trgPnt2, tri->strings[0]);
                GetStringCodeBetweenPoints(trgPnt2, trgPnt3, tri->strings[1]);
                GetStringCodeBetweenPoints(trgPnt3, trgPnt1, tri->strings[2]);
                return DTM_SUCCESS;
                }

             /*-------------------------------------------------------------------+
             |                                                                    |
             |                                                                    |
             |                                                                    |
             +-------------------------------------------------------------------*/
    public: int setPoints()
                {
                int ret = DTM_SUCCESS;
                MXTriangle::PointArray& pointsPtr = const_cast<MXTriangle::PointArray&>(m_points);

                pointsPtr.setPhysicalLength(m_dtmP->numPoints);
                pointsPtr.setLogicalLength(m_dtmP->numPoints);

                MXTriangle::point** pointsP = pointsPtr.getArrayPtr();
                MXTriangle::point** startPP = pointsPtr.getArrayPtr();

                PartitionArray<DTM_TIN_POINT, DTM_PARTITION_SHIFT_POINT, MAllocAllocator> pointsArray(m_dtmP->pointsPP, m_dtmP->numPoints, m_dtmP->numPointPartitions, m_dtmP->pointPartitionSize);
                PartitionArray<DTM_TIN_POINT, DTM_PARTITION_SHIFT_POINT, MAllocAllocator>::iterator pointsIter = pointsArray.start();

                m_pointMapper.resize(m_dtmP->numPoints);
                PartitionArray<int, 15, MAllocAllocator>::iterator bcPtMapperP = m_pointMapper.start();
                
                int mxPtNum = 4;
                pointsP += 4;   // Skip the first 4 points as these are the outer points.
                for(long i = 0; i < m_dtmP->numPoints; i++, pointsIter++, bcPtMapperP++)
                    {
                    if(((MXTriangle::point*)&*pointsIter)->x <= m_dtmP->xMin)
                        {
                        if(((MXTriangle::point*)&*pointsIter)->y <= m_dtmP->yMin)
                            {
                            *startPP[0] = *((MXTriangle::point*)&*pointsIter);
                            *bcPtMapperP = 0;
                            }
                        else
                            {
                            *startPP[1] = *((MXTriangle::point*)&*pointsIter);
                            *bcPtMapperP = 1;
                            }
                        }
                    else if(((MXTriangle::point*)&*pointsIter)->x >= m_dtmP->xMax)
                        {
                        if(((MXTriangle::point*)&*pointsIter)->y <= m_dtmP->yMin)
                            {
                            *startPP[3] = *((MXTriangle::point*)&*pointsIter);
                            *bcPtMapperP = 3;
                            }
                        else
                            {
                            *startPP[2] = *((MXTriangle::point*)&*pointsIter);
                            *bcPtMapperP = 2;
                            }
                        }
                    else
                        {
                        **pointsP++ = *((MXTriangle::point*)&*pointsIter);
                        *bcPtMapperP = mxPtNum++;
                        }
                    }

                return ret;
                }
    };

template<class dtm> class TriangleToDTMHelper
    {
    private:
        dtm& m_dtm;
        char* m_markers;
        long* m_pointNumbers;
        BC_DTM_OBJ* m_dtmP;
    public:
        /*-------------------------------------------------------------------+
        |                                                                    |
        |                                                                    |
        |                                                                    |
        +-------------------------------------------------------------------*/
        TriangleToDTMHelper(dtm& dtmR, BC_DTM_OBJ *dtmP) : m_dtm(dtmR), m_dtmP(dtmP)
            {
            // Allocate Markers and point number temporary array.
            m_markers = (char*)malloc(NumTriangles());
            m_pointNumbers = (long*)malloc(sizeof(long) * NumPoints());

            for(long i = 0; i < NumPoints(); i++)
                m_pointNumbers[i] = 0;

            for(long i = 0; i < NumTriangles(); i++)
                m_markers[i] = 0;
            }
        /*-------------------------------------------------------------------+
        |                                                                    |
        |                                                                    |
        |                                                                    |
        +-------------------------------------------------------------------*/
        ~TriangleToDTMHelper()
            {
            if(m_markers) free(m_markers);
            if(m_pointNumbers) free(m_pointNumbers);
            }

        /*-------------------------------------------------------------------+
        |                                                                    |
        |                                                                    |
        |                                                                    |
        +-------------------------------------------------------------------*/
        long& PointNumber(long i) { return m_pointNumbers[i]; }
        /*-------------------------------------------------------------------+
        |                                                                    |
        |                                                                    |
        |                                                                    |
        +-------------------------------------------------------------------*/
        char& Marker(long i) { return m_markers[i]; }


        inline DPoint3d& Point(long num)
            {
            return m_dtm.Point(num);
            }
        inline long NumPoints()
            {
            return m_dtm.NumPoints();
            }

        inline long NumTriangles()
            {
            return m_dtm.NumTriangles();
            }
        inline long TrianglePointNumber(long triNum, long vertex)
            {
            return m_dtm.TrianglePointNumber(triNum, vertex);
            }
        inline long GetAdjTriangle(long triNum, long side)
            {
            return m_dtm.GetAdjTriangle(triNum, side);
            }
        inline long TriangleStringLink(long triNum, long vertex)
            {
            return m_dtm.TriangleStringLink(triNum, vertex);
            }

        inline long TriangleGroupCode(long triNum)
            {
            if (m_markers[triNum] & MARKER_EXTERNAL) return asLong ("_EXT");
            return m_dtm.TriangleGroupCode(triNum);
            }



        // Gets the adjTri number and adj side which matchs the link.
        /*-------------------------------------------------------------------+
        |                                                                    |
        |                                                                    |
        |                                                                    |
        +-------------------------------------------------------------------*/
    private: void GetAdjTriangleAndSide(long triNum, long side, long& adjTri, long& adjTriSide)
                 {
                 adjTri = GetAdjTriangle(triNum, side);

                 if(adjTri > NumTriangles())
                     adjTri = adjTri;

                 if(adjTri != -1)
                     {
                     for(adjTriSide = 0; adjTriSide < 3; adjTriSide++)
                         {
                         if(GetAdjTriangle(adjTri,adjTriSide) == triNum)
                             break;
                         }
                     }
                 }

             /*-------------------------------------------------------------------+
             |                                                                    |
             |                                                                    |
             |                                                                    |
             +-------------------------------------------------------------------*/
             // Finds all of the triangles which are external and mark them.
    private: bool IsTriangleNULL(long tri)
                 {
                 if(TriangleGroupCode(tri) == asLong("NULL") || TriangleGroupCode(tri) == asLong("_EXT"))
                     return true;
                 for(int pt = 0; pt < 3; pt++)
                     {
                     if(Point(TrianglePointNumber(tri, pt)).z < -998)
                         return true;
                     }
                 return false;
                 }

struct trimEdges
{
	int tri;
//	int side;
	int marked;
};

#define MARKED_NORMAL 1
#define MARKED_MINLEN 2
#define MARKED_DONE 0

inline int FixSide(int value)
{
	if(value >= 3)
		return value - 3;
	return value;
}


        void Trim()
            {
	        int triSize = NumTriangles();

            // 1. Find a triangle at the edge.
	        int mtri = -1;
	        int side;
	        ArrayPtrClass<trimEdges> currentTrimEdges;
	        int trimEdgeNum = 0;
	        int i;

	        if(mtri == -1)
	        {
		        for(mtri = 0; mtri < triSize; mtri++)
		        {
			        for(side = 0; side < 3; side++)
			        {
				        if(GetAdjTriangle(mtri,side) == -1)
					        break;
			        }
			        if(side != 3)
				        break;
		        }
	        }

	        if(mtri >= triSize)
		        return;
	        // Get the triangles on the edges.

	        currentTrimEdges[trimEdgeNum].tri = mtri;
	        currentTrimEdges[trimEdgeNum++].marked = MARKED_NORMAL;
	        trimEdges** currentTrimEdgesP = currentTrimEdges.getArrayPtr();

	        for(i = 0; i < trimEdgeNum; i++)
	        {
		        if(currentTrimEdgesP[i]->marked == MARKED_DONE)
			        continue;
		        mtri = currentTrimEdgesP[i]->tri;
		        bool dontNullTriangle = false;
		        int prevEdgeNum = trimEdgeNum;
		        bool fixIt[3];
		        int marked[3];
		        for(side = 0; side < 3; side++)
		        {
			        fixIt[side] = TRUE;
			        marked[side] = MARKED_NORMAL;
			        int mtriadj = GetAdjTriangle(mtri, side);

		            DPoint3d *pt[3];
		            pt[0] = &Point(TrianglePointNumber(mtri, side));
		            pt[1] = &Point(TrianglePointNumber(mtri,FixSide(side + 1)));
    				double length = bcdtmMath_distanceSquared(pt[0]->x, pt[0]->y, pt[1]->x, pt[1]->y);

		            if(mtriadj != -1)
		                {
                        if (!IsTriangleNULL(mtriadj))
                            {
                            }
			            else if(IsOnEdge(mtri))
				            fixIt[side] = FALSE;
			            else
			                {
				            int side2;
				            for(side2 = 0; side2 < 3; side2++)
					            if(GetAdjTriangle(mtriadj, side2) == mtri)
						            break;

				            pt[2] = &Point(TrianglePointNumber(mtriadj,FixSide(side2 + 2)));
				            double disop = bcdtmMath_distanceSquared(pt[0]->x, pt[0]->y, pt[2]->x, pt[2]->y) + bcdtmMath_distanceSquared(pt[1]->x, pt[1]->y, pt[2]->x, pt[2]->y);

				            if(length >= disop)
					            fixIt[side] = FALSE;
			                }
		                }
		        }
		        currentTrimEdgesP[i]->marked = MARKED_DONE;
		        if(!dontNullTriangle)
		        {
			        for(side = 0; side < 3; side++)
			        {
				        if(!fixIt[side])
				        {
					        int nextTri = GetAdjTriangle(mtri, side);
					        if(nextTri != -1)
					        {
						        int j;
						        for(j = 0; j < trimEdgeNum; j++)
						        {
							        if(currentTrimEdgesP[j]->tri == nextTri)
								        break;
						        }
						        if(j == trimEdgeNum)
						        {
							        currentTrimEdges[trimEdgeNum].tri = nextTri;
							        currentTrimEdges[trimEdgeNum++].marked = marked[side];
							        currentTrimEdgesP = currentTrimEdges.getArrayPtr();
						        }
					        }
				        }
			        }
                    if (!IsTriangleNULL(mtri))
                        {
                        mtri = mtri;
                        }
                    else
                        {
                        if ((m_markers[mtri] & MARKER_EXTERNAL_HULL) == 0)
                            mtri = mtri;
                        else
			            m_markers[mtri] |= MARKER_EXTERNAL_HULL;
                        }
		        }
		        else
		        {
			        trimEdgeNum = prevEdgeNum;
		        }
	        }
        }

             /*-------------------------------------------------------------------+
             |                                                                    |
             |                                                                    |
             |                                                                    |
             +-------------------------------------------------------------------*/
    public: int markExternalTriangles()
                {
                int ret = DTM_SUCCESS;
                // Find outer triangles.
                std::stack<long> trianglesToMark;
                long triNum = 0;
                long nextTri = 0;

                Trim ();
                while(nextTri != -1)
                    {
                    triNum = nextTri;
                    long side = 0;
                    const DPoint3d* p[3];
                    p[0] = &Point(TrianglePointNumber(triNum,0));
                    p[1] = &Point(TrianglePointNumber(triNum,1));
                    p[2] = &Point(TrianglePointNumber(triNum,2));

                    if(p[1]->x < p[0]->x) side = 1;
                    if(p[2]->x < p[side]->x) side = 2;
                    nextTri = GetAdjTriangle(triNum,side);
                    }

                trianglesToMark.push(triNum);

                while(trianglesToMark.size())
                    {
                    triNum = trianglesToMark.top();
                    trianglesToMark.pop();

                    Marker(triNum) |= MARKER_EXTERNAL;

                    for(long side = 0; side < 3; side++)
                        {
                        long triNum2 = GetAdjTriangle(triNum,side);
                        PointNumber(TrianglePointNumber(triNum,side)) = -(triNum + 1);

                        if(triNum2 != -1 && (Marker(triNum2) & MARKER_EXTERNAL) != MARKER_EXTERNAL)
                            {
                            if(IsOnEdge(triNum2) || Marker(triNum2) & MARKER_EXTERNAL_HULL)
                                {
                                trianglesToMark.push(triNum2);
                                }
                            }
                        }
                    }
                return ret;
                }
            /*-------------------------------------------------------------------+
            |                                                                    |
            |                                                                    |
            |                                                                    |
            +-------------------------------------------------------------------*/
            bool IsOnEdge (long triNum)
                {
                for (int num = 0; num < 3; num++)
                    {
                    int k = TrianglePointNumber(triNum, num);
                    if (k < 4)
                        return true;
                    }
                return false;
                }


            /*-------------------------------------------------------------------+
            |                                                                    |
            |                                                                    |
            |                                                                    |
            +-------------------------------------------------------------------*/
    private: bool checkIfExternal(long ptNum, long triNum)
                 {
                 long curTri = triNum;
                 long curSide;

                 for(curSide = 0; curSide < 3; curSide++)
                     {
                     if(TrianglePointNumber(curTri,curSide) == ptNum)
                         break;
                     }

                 bool isInternal = false;
                 do
                     {
                     long adjTri;
                     long adjSide;
                     GetAdjTriangleAndSide(curTri, curSide, adjTri, adjSide);
                     if(Marker(adjTri) == 0)
                         isInternal = true;
                     curTri = adjTri;
                     curSide = (adjSide + 1) % 3;
                     }
                     while(curTri != triNum && !isInternal);

                     return !isInternal;
                 }

             /*-------------------------------------------------------------------+
             |                                                                    |
             |                                                                    |
             |                                                                    |
             +-------------------------------------------------------------------*/
    private:inline bool isLinkMarked(long triNum, long side)
                {
                char mask = 1 << side;
                return (Marker(triNum) & mask) == mask;
                }

            /*-------------------------------------------------------------------+
            |                                                                    |
            |                                                                    |
            |                                                                    |
            +-------------------------------------------------------------------*/
    private:inline void markLink(long triNum, long side)
                {
                char mask = 1 << side;
                Marker(triNum) |= mask;
                }

            /*-------------------------------------------------------------------+
            |                                                                    |
            |                                                                    |
            |                                                                    |
            +-------------------------------------------------------------------*/
    private:void findFeatureStart(long triNum, long side, long& startTri, long& startSide)
                {
                long curLink = TriangleStringLink(triNum,side);
                long curTri = triNum;
                long curSide = side;
                long adjTri;
                long adjSide;
                long endTriNum = triNum;

                do
                    {
                    curSide = (curSide + 1) % 3;

                    GetAdjTriangleAndSide(curTri, curSide, adjTri, adjSide);

                    if(adjTri == endTriNum)
                        {
                        // found the end of the feature.
                        startTri = adjTri;
                        startSide = adjSide;
                        return;
                        }

                    if(curTri == triNum && curSide == side)
                        {
                        // This feature is closed.
                        startTri = triNum;
                        startSide = side;
                        return;
                        }

                    if(!isLinkMarked(curTri, curSide) && TriangleStringLink(curTri,curSide) == curLink)
                        {
                        endTriNum = curTri;

                        // Found Link adjust search
                        }
                    else
                        {
                        // Not a match continue searching.
                        curTri = adjTri;
                        curSide = adjSide;
                        }
                    } while(true);
                }

            /*-------------------------------------------------------------------+
            |                                                                    |
            |                                                                    |
            |                                                                    |
            +-------------------------------------------------------------------*/
    private:int addFeatureToDtmObject(long triNum, long side)
                {
                int ret = DTM_SUCCESS;
                long curLink = TriangleStringLink(triNum,side);
                long curTri = triNum;
                long curSide = side;
                long adjTri;
                long adjSide;
                long endTriNum = triNum;
                long lastPoint = -1;
                long firstPoint = -1;

                // Add the first Link into the DTM.
                GetAdjTriangleAndSide(curTri, curSide, adjTri, adjSide);

                // Add the first link to the feature Table
                markLink(curTri, curSide);
                markLink(adjTri, adjSide);
                lastPoint = PointNumber(TrianglePointNumber(curTri,curSide));
                if(lastPoint != -1 && PointNumber(TrianglePointNumber(curTri, (curSide + 1) % 3)) != -1)
                    {
                    firstPoint = PointNumber(TrianglePointNumber(curTri, (curSide + 1) % 3));
                    nodeAddrP(m_dtmP, lastPoint)->tPtr = firstPoint;
                    }

                do
                    {
                    curSide = (curSide + 2) % 3;

                    GetAdjTriangleAndSide(curTri, curSide, adjTri, adjSide);

                    if(adjTri == endTriNum)
                        {
                        // found the end of the feature.
                        break;
                        }

                    if(!isLinkMarked(curTri, curSide) && TriangleStringLink(curTri,curSide) == curLink)
                        {
                        markLink(curTri, curSide);
                        markLink(adjTri, adjSide);
                        // Add this point to the features.
                        long nextPoint = PointNumber(TrianglePointNumber(curTri,curSide));

                        if(nextPoint == -1 && lastPoint != -1)
                            {
                            if(bcdtmInsert_addDtmFeatureToDtmObject(m_dtmP, NULL, 0, DTMFeatureType::Breakline, curLink, m_dtmP->dtmFeatureIndex++, lastPoint, 1) != DTM_SUCCESS)
                                goto errexit;
                            lastPoint = -1;
                            }
                        else if(nextPoint != -1)
                            {
                            if(nodeAddrP(m_dtmP, nextPoint)->tPtr != m_dtmP->nullPnt || nextPoint == firstPoint)
                                {
                                if(nextPoint == firstPoint) // Circular
                                    {
                                    nodeAddrP(m_dtmP, nextPoint)->tPtr = lastPoint;
                                    if( bcdtmList_reverseTptrPolygonDtmObject(m_dtmP,lastPoint) != DTM_SUCCESS) return DTM_ERROR;
                                    }

                                if(bcdtmInsert_addDtmFeatureToDtmObject(m_dtmP, NULL, 0, DTMFeatureType::Breakline, curLink, m_dtmP->dtmFeatureIndex++, lastPoint, 1) != DTM_SUCCESS)
                                    goto errexit;
                                firstPoint = nextPoint;
                                lastPoint = -1;
                                }
                            if(lastPoint != -1)
                                nodeAddrP(m_dtmP, nextPoint)->tPtr = lastPoint;
                            lastPoint = nextPoint;
                            }
                        // Need to check if this feature looks
                        endTriNum = curTri;
                        }
                    else
                        {
                        // Not a match continue searching.
                        curTri = adjTri;
                        curSide = adjSide;
                        }
                    } while(true);

                    if(lastPoint != -1 && lastPoint != firstPoint && bcdtmInsert_addDtmFeatureToDtmObject(m_dtmP, NULL, 0, DTMFeatureType::Breakline, curLink, m_dtmP->dtmFeatureIndex++, lastPoint, 1) != DTM_SUCCESS)
                        goto errexit;
                    /*
                    ** Cleanup
                    */
cleanup :
                    /*
                    ** Job Completed
                    */
                    return(ret) ;
                    /*
                    ** Error Exit
                    */
errexit :
                    if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
                    goto cleanup ;


                }

            /*-------------------------------------------------------------------+
            |                                                                    |
            |                                                                    |
            |                                                                    |
            +-------------------------------------------------------------------*/
    public:int traceAndAddFeaturesDtmObject()
               {
               int ret = DTM_SUCCESS;

               // Look at each triangle and each edge and look for a string link.
               for(long triNum = 0; triNum < NumTriangles(); triNum++)
                   {
                   if((Marker(triNum) & MARKER_EXTERNAL) == 0)
                       {
                       for(long side = 0; side < 3; side++)
                           {
                           // Ignore special string links.
                           if(TriangleStringLink(triNum,side) == asLong("____") || TriangleStringLink(triNum,side) == asLong("%FCE"))
                               continue;

                           if(!isLinkMarked(triNum, side) && TriangleStringLink(triNum,side))
                               {
                               long startTri;
                               long startSide;

                               // Scan along the links to find the starting point.
                               findFeatureStart(triNum, side, startTri, startSide);

                               // Now trace the features and add them to the DTM.
                               addFeatureToDtmObject(startTri, startSide);
                               }
                           }
                       }
                   }
               /*
               ** Cleanup
               */
//cleanup :
               /*
               ** Job Completed
               */
               return(ret) ;
               /*
               ** Error Exit
               */
//errexit :
//               if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
//               goto cleanup ;

//               return ret;
               }

           /*-------------------------------------------------------------------+
           |                                                                    |
           |                                                                    |
           |                                                                    |
           +-------------------------------------------------------------------*/
    public: int addPointsToDtmObject()
                {
                int ret = DTM_SUCCESS;

                // Create PartitionArrays from the dtm Arrays
                PartitionArray<DTM_TIN_POINT, DTM_PARTITION_SHIFT_POINT, MAllocAllocator> pointsArray(m_dtmP->pointsPP, m_dtmP->numPoints, m_dtmP->numPointPartitions, m_dtmP->pointPartitionSize);
                PartitionArray<DTM_TIN_POINT, DTM_PARTITION_SHIFT_POINT, MAllocAllocator>::iterator pointsIter = pointsArray.start();

                PartitionArray<DTM_TIN_NODE, DTM_PARTITION_SHIFT_NODE, MAllocAllocator> nodesArray(m_dtmP->nodesPP, m_dtmP->numNodes, m_dtmP->numNodePartitions, m_dtmP->nodePartitionSize);
                PartitionArray<DTM_TIN_NODE, DTM_PARTITION_SHIFT_NODE, MAllocAllocator>::iterator nodesIter = nodesArray.start();

                // Add the points to the array and create the point number mapper array.
                long ptNum = 0;
                for(long i = 0; i < NumPoints(); i++)
                    {
                    DTM_TIN_POINT* p3dP = (DTM_TIN_POINT*)&Point(i);

                    // If this is a point completely outside of the triangulation then ignore it.
                    if(i >= 4 && PointNumber(i) < 0)
                        {
                        if(checkIfExternal(i, (-PointNumber(i)) - 1))
                            PointNumber(i) = -1;
                        else
                            PointNumber(i) = 0;
                        }

                    if(i < 4 || PointNumber(i) == -1) // Or is external.
                        {
                        PointNumber(i) = -1;
                        }
                    else
                        {
                        PointNumber(i) = ptNum++;
                        *pointsIter = *p3dP;

                        if( ptNum == 0)
                            {
                            m_dtmP->xMin = m_dtmP->xMax = p3dP->x ;
                            m_dtmP->yMin = m_dtmP->yMax = p3dP->y ;
                            m_dtmP->zMin = m_dtmP->zMax = p3dP->z ;
                            }
                        else
                            {
                            if( p3dP->x < m_dtmP->xMin ) m_dtmP->xMin = p3dP->x ;
                            if( p3dP->x > m_dtmP->xMax ) m_dtmP->xMax = p3dP->x ;
                            if( p3dP->y < m_dtmP->yMin ) m_dtmP->yMin = p3dP->y ;
                            if( p3dP->y > m_dtmP->yMax ) m_dtmP->yMax = p3dP->y ;
                            if( p3dP->z < m_dtmP->zMin ) m_dtmP->zMin = p3dP->z ;
                            if( p3dP->z > m_dtmP->zMax ) m_dtmP->zMax = p3dP->z ;
                            }

                        nodesIter->PCWD = 0;
                        nodesIter->PRGN = 0;
                        nodesIter->hPtr = m_dtmP->nullPnt ;
                        nodesIter->tPtr = m_dtmP->nullPnt ;
                        nodesIter->sPtr = m_dtmP->nullPnt ;
                        nodesIter->cPtr = m_dtmP->nullPtr ;
                        nodesIter->fPtr = m_dtmP->nullPtr ;
                        pointsIter++;
                        nodesIter++;
                        }
                    }

                // Update the number of points.
                m_dtmP->numPoints = ptNum;
                m_dtmP->numNodes = ptNum;

                if(bcdtmObject_allocatePointsMemoryDtmObject(m_dtmP) != DTM_SUCCESS) goto errexit;
                if(bcdtmObject_allocateNodesMemoryDtmObject(m_dtmP) != DTM_SUCCESS) goto errexit;

                /*
                ** Cleanup
                */
cleanup :
                /*
                ** Job Completed
                */
                return(ret) ;
                /*
                ** Error Exit
                */
errexit :
                if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
                goto cleanup ;
                }

            /*-------------------------------------------------------------------+
            |                                                                    |
            |                                                                    |
            |                                                                    |
            +-------------------------------------------------------------------*/
    public: int addTriangleLinksToDtmObject()
                {
                int ret = DTM_SUCCESS;
                // Create Circular List
                PartitionArray<DTM_CIR_LIST, DTM_PARTITION_SHIFT_CLIST, MAllocAllocator> cListArray(m_dtmP->cListPP, m_dtmP->numClist, m_dtmP->numClistPartitions, m_dtmP->clistPartitionSize);
                PartitionArray<DTM_CIR_LIST, DTM_PARTITION_SHIFT_CLIST, MAllocAllocator>::iterator cListIter = cListArray.start();

                PartitionArray<DTM_TIN_NODE, DTM_PARTITION_SHIFT_NODE, MAllocAllocator> nodesArray(m_dtmP->nodesPP, m_dtmP->numNodes, m_dtmP->numNodePartitions, m_dtmP->nodePartitionSize);
                PartitionArray<DTM_TIN_NODE, DTM_PARTITION_SHIFT_NODE, MAllocAllocator>::iterator nodesIter = nodesArray.start();

                m_dtmP->cListPtr = 0;
                for(long i = 0; i < NumTriangles(); i++)
                    {
                    long adjTri;
                    long adjSide;
                    for(long side = 0; side < 3; side++)
                        {
                        long mxPt = TrianglePointNumber(i, side);
                        long ptNum = PointNumber(mxPt);

                        // Is this point external...
                        if(ptNum == -1)
                            {
                            // Don't need to do anything.
                            }
                        else if(nodesArray[ptNum].cPtr == m_dtmP->nullPtr)
                            {
                            long curTri = i;
                            long curSide = side;
                            long searchTri = i;
                            long thePtNum = ptNum;

                            nodesArray[ptNum].cPtr = m_dtmP->cListPtr;

                            curSide = (curSide + 2) % 3;
                            mxPt = TrianglePointNumber(curTri,curSide);
                            ptNum = PointNumber(mxPt);
                            bool skip = false;

                            if(ptNum != -1)
                                {
                                if(Marker(curTri) & MARKER_EXTERNAL)
                                    {
                                    if(!(Marker(GetAdjTriangle(curTri, curSide)) & MARKER_EXTERNAL))
                                        {
                                        if(nodesArray[ptNum].hPtr == m_dtmP->nullPnt)
                                            nodesArray[ptNum].hPtr = thePtNum;
                                        else
                                            {
                                            //Error
                                            ptNum = ptNum;
                                            }

                                        m_dtmP->hullPoint = thePtNum;
                                        }
                                    else
                                        skip = true;
                                    }
                                if(!skip)
                                    cListIter->pntNum = ptNum;
                                }
                            else
                                skip = true;

                            do
                                {
                                GetAdjTriangleAndSide(curTri, curSide, adjTri, adjSide);
                                if(adjTri == searchTri) break;

                                adjSide = (adjSide + 2) % 3;
                                mxPt = TrianglePointNumber(adjTri, adjSide);
                                ptNum = PointNumber(mxPt);

                                if(Marker(adjTri) & MARKER_EXTERNAL)
                                    {
                                    int nextTri = GetAdjTriangle(adjTri, adjSide);
                                    if(!(Marker(nextTri) & MARKER_EXTERNAL))
                                        {
                                        if(nodesArray[ptNum].hPtr == m_dtmP->nullPnt)
                                            nodesArray[ptNum].hPtr = thePtNum;
                                        else
                                            {
                                            }
                                        m_dtmP->hullPoint = thePtNum;
                                        }
                                    else
                                        {
                                        ptNum = -1;
                                        }
                                    }

                                if(adjTri != -1 && ptNum != -1)
                                    {
                                    if(!skip)
                                        {
                                        cListIter->nextPtr = m_dtmP->cListPtr + 1;
                                        cListIter++;
                                        m_dtmP->cListPtr++;
                                        }
                                    skip = false;

                                    cListIter->pntNum = ptNum;
                                    }

                                curTri = adjTri;
                                curSide = adjSide;
                                }
                                while(curTri != -1 && curTri != searchTri);
                                cListIter->nextPtr = m_dtmP->nullPtr;

                                cListIter++;
                                m_dtmP->cListPtr++;
                            }
                        }
                    }

                // Scale down the arrays.
                m_dtmP->numClist = m_dtmP->cListPtr;
                if(bcdtmObject_allocateCircularListMemoryDtmObject(m_dtmP) != DTM_SUCCESS) goto errexit;
                /*
                ** Cleanup
                */
cleanup :
                /*
                ** Job Completed
                */
                return(ret) ;
                /*
                ** Error Exit
                */
errexit :
                if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
                goto cleanup ;
                }


            /*-------------------------------------------------------------------+
            |                                                                    |
            |                                                                    |
            |                                                                    |
            +-------------------------------------------------------------------*/
    private: void resetLinkMarkers()
                 {
                 for(long i = 0; i < NumTriangles(); i++)
                     m_markers[i] &= MARKER_EXTERNAL | MARKER_EXTERNAL_HULL;
                 }
             /*-------------------------------------------------------------------+
             |                                                                    |
             |                                                                    |
             |                                                                    |
             +-------------------------------------------------------------------*/
    private: bool CheckGroupCodeEdge(long curTri, long adjTri, long groupCode)
                 {
                 if(TriangleGroupCode(curTri) == groupCode)
                     return groupCode != TriangleGroupCode(adjTri);
                 return false;//TriangleGroupCode(adjTri) == groupCode;
                 }


             /*-------------------------------------------------------------------+
             |                                                                    |
             |                                                                    |
             |                                                                    |
             +-------------------------------------------------------------------*/
    private: int addRegionFeature(long startPoint, long groupCode, bool isOnHull)
                 {
                 if(groupCode != asLong("NULL"))
                     {
                     double area ; 
                     DTMDirection direction;

                     if (*((char*)&groupCode) == 0)
                         groupCode = 0;
                     bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(m_dtmP,startPoint,&area,&direction) ;

                     // Only add Clockwise features, which are on the outside.
                     if(direction == DTMDirection::Clockwise)
                         {
                         if( bcdtmList_reverseTptrPolygonDtmObject(m_dtmP,startPoint) != DTM_SUCCESS) return DTM_ERROR;
                         return bcdtmInsert_addDtmFeatureToDtmObject(m_dtmP, NULL, 0, DTMFeatureType::Region, groupCode, m_dtmP->dtmFeatureIndex++, startPoint, 1);
                         }
                     return bcdtmList_nullTptrListDtmObject (m_dtmP, startPoint);
                     }

                 // This is either a Void or an Island.
                 double area ; 
                 DTMDirection direction;

                 bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(m_dtmP,startPoint,&area,&direction) ;

                 if(direction == DTMDirection::Clockwise)
                     {
                     if( bcdtmList_reverseTptrPolygonDtmObject(m_dtmP,startPoint) != DTM_SUCCESS) return DTM_ERROR;

                     return bcdtmInsert_addDtmFeatureToDtmObject(m_dtmP, NULL, 0, DTMFeatureType::Void, groupCode, m_dtmP->dtmFeatureIndex++, startPoint, 1);
                     }
                 else
                     {
                     if (!isOnHull)
                     return bcdtmInsert_addDtmFeatureToDtmObject(m_dtmP, NULL, isOnHull ? -1 : 0, DTMFeatureType::Island, groupCode, m_dtmP->dtmFeatureIndex++, startPoint, 1);
                     else
                         return bcdtmList_nullTptrListDtmObject(m_dtmP, startPoint);
                     }
//                 return DTM_SUCCESS;
                 }


            /*-------------------------------------------------------------------+
            |                                                                    |
            |                                                                    |
            |                                                                    |
            +-------------------------------------------------------------------*/
    private:int traceAreaToTPtr(long triNum, long side, long groupCode)
                {
                int ret = DTM_SUCCESS;
                long curTri = triNum;
                long curSide = side;
                long adjTri;
                long adjSide;
                long endTriNum = triNum;
                long lastPoint = -1;
                long firstPoint = -1;
                bool isOnHull = false;

                // Add the first Link into the DTM.
                GetAdjTriangleAndSide(curTri, curSide, adjTri, adjSide);

                // Add the first link to the feature Table
                markLink(curTri, curSide);
                lastPoint = PointNumber(TrianglePointNumber(curTri,curSide));
                if(lastPoint != -1 && PointNumber(TrianglePointNumber(curTri, (curSide + 1) % 3)) != -1)
                    {
                    firstPoint = PointNumber(TrianglePointNumber(curTri, (curSide + 1) % 3));
                    nodeAddrP(m_dtmP, lastPoint)->tPtr = firstPoint;
                    if (nodeAddrP(m_dtmP, lastPoint)->hPtr != m_dtmP->nullPnt)
                        isOnHull = true;
                    }

                do
                    {
                    curSide = (curSide + 2) % 3;

                    GetAdjTriangleAndSide(curTri, curSide, adjTri, adjSide);

                    if(adjTri == endTriNum)
                        {
                        // found the end of the feature.
                        break;
                        }

                    if(/*!isLinkMarked(curTri, curSide) && */CheckGroupCodeEdge(curTri, adjTri, groupCode))
                        {
                        markLink(curTri, curSide);
                        // Add this point to the features.
                        long nextPoint = PointNumber(TrianglePointNumber(curTri,curSide));
                       
                        if(nextPoint == -1 && lastPoint != -1)
                            {
                            // This shouldn't happen I believe....
                            goto errexit;
                            }
                        else if (nextPoint != -1)
                            {
                            if (nodeAddrP(m_dtmP, nextPoint)->hPtr != m_dtmP->nullPnt)
                                isOnHull = true;
                            if(nodeAddrP(m_dtmP, nextPoint)->tPtr != m_dtmP->nullPnt)
                                {
                                long prevLastPoint = nodeAddrP (m_dtmP, nextPoint)->tPtr;
                                nodeAddrP (m_dtmP, nextPoint)->tPtr = lastPoint;
                                // update isOnHull
                                bool wasOnHull = isOnHull;
                                if (isOnHull)
                                    {
                                    isOnHull = false;
                                    long t = nextPoint;
                                    while (t != nextPoint)
                                        {
                                        if (nodeAddrP (m_dtmP, t)->hPtr != m_dtmP->nullPnt)
                                            {
                                            isOnHull = true;
                                            break;
                                            }
                                        t = nodeAddrP (m_dtmP, t)->tPtr;
                                        }
                                    }
                                if (addRegionFeature (lastPoint, groupCode, isOnHull) != DTM_SUCCESS)
                                    goto errexit;
                                lastPoint = prevLastPoint;
                                if (wasOnHull)
                                    {
                                    isOnHull = false;
                                    long t = lastPoint;
                                    while (t != firstPoint)
                                        {
                                        if (nodeAddrP (m_dtmP, t)->hPtr != m_dtmP->nullPnt)
                                            {
                                            isOnHull = true;
                                            break;
                                            }
                                        t = nodeAddrP (m_dtmP, t)->tPtr;
                                        }
                                    }
                                }
                            else if(nextPoint == firstPoint)
                                {
                                nodeAddrP(m_dtmP, nextPoint)->tPtr = lastPoint;
                                if(addRegionFeature(lastPoint, groupCode, isOnHull) != DTM_SUCCESS)
                                    goto errexit;
                                return DTM_SUCCESS;
//                                firstPoint = nextPoint;
//                                lastPoint = -1;
                                }
                            if(lastPoint != -1)
                                nodeAddrP(m_dtmP, nextPoint)->tPtr = lastPoint;
                            lastPoint = nextPoint;
                            // Need to check if this feature looks
                            endTriNum = curTri;
                            }
                        else
                            {
                            // Not a match continue searching.
                            curTri = adjTri;
                            curSide = adjSide;
                            }
                        }
                    else
                        {
                        // Not a match continue searching.
                        curTri = adjTri;
                        curSide = adjSide;
                        }
                    } while(true);

                    // Shouldn't happen...
                    if(lastPoint != -1 && lastPoint != firstPoint)
                        goto errexit;
                    /*
                    ** Cleanup
                    */
cleanup :
                    /*
                    ** Job Completed
                    */
                    return(ret) ;
                    /*
                    ** Error Exit
                    */
errexit :
                    if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
                    bcdtmList_nullTptrValuesDtmObject (m_dtmP);
                    goto cleanup;


                }


            /*-------------------------------------------------------------------+
            |                                                                    |
            |                                                                    |
            |                                                                    |
            +-------------------------------------------------------------------*/
    public: int findRegionsAndVoids()
                {
                long groupCode;
                resetLinkMarkers();
                for(int triNum = 0; triNum < NumTriangles(); triNum++)
                    {
                    groupCode = TriangleGroupCode(triNum);
                    if((m_markers[triNum] & MARKER_EXTERNAL) == 0 && groupCode != asLong("   ") && groupCode != 0)
                        {
                        for(int side = 0; side < 3; side++)
                            {
                            long adjTri = GetAdjTriangle(triNum, side);
                            if(!isLinkMarked(triNum, side))
                                {
                                if(TriangleGroupCode(adjTri) != groupCode)
                                    {
                                    if (groupCode == asLong("NULL") && TriangleGroupCode(adjTri) == asLong("_EXT"))
                                        side = side;
                                    traceAreaToTPtr(triNum, side, groupCode);
                                    }
                                }
                            }
                        }
                    }
                return DTM_SUCCESS;
                }
    };


/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTMFORMATS_EXPORT int bcdtmImport_MXTriangulationToDtmObject(BC_DTM_OBJ* dtmP, void* triPtrP, void* pointsPtrP)
    {
    int ret = DTM_SUCCESS;
    int dbg = 0;
    int cdbg = 0;
    int tdbg = 0;
    MXTriangle::TriangleArray& triPtr = *(MXTriangle::TriangleArray*)triPtrP;
    MXTriangle::PointArray& pointsPtr = *(MXTriangle::PointArray*)pointsPtrP;
    long startTime = bcdtmClock() ;
    MXDTM mxdtmHelper(triPtr, pointsPtr, dtmP);
    TriangleToDTMHelper<MXDTM> mxdtm(mxdtmHelper, dtmP);
    // Check the inputs.

    if(dbg)
        {
        bcdtmWrite_message(0,0,0,"Importing MX Tin") ;
        bcdtmWrite_message(0,0,0," Number MX Triangles = %d", triPtr.size()) ;
        bcdtmWrite_message(0,0,0," Number MX Points = %d", pointsPtr.size());
        }

    // Initialise the tin header.
    dtmP->dtmFeatureIndex = 1;
    dtmP->dtmState = DTMState::Tin;
    dtmP->nextHullPoint = 0;

    // Create Arrays.

    dtmP->numPoints = mxdtm.NumPoints();

//    if(bcdtmObject_allocatePointsMemoryDtmObject(dtmP) != DTM_SUCCESS) goto errexit;
//    dtmP->numNodes = mxdtm.NumPoints();
//    if(bcdtmObject_allocateNodesMemoryDtmObject(dtmP) != DTM_SUCCESS) goto errexit;
//    dtmP->numClist = mxdtm.NumPoints() * 3;
//    if(bcdtmObject_allocateCircularListMemoryDtmObject(dtmP) != DTM_SUCCESS) goto errexit;

    if(bcdtmObject_setPointMemoryAllocationParametersDtmObject(dtmP,mxdtm.NumPoints(),mxdtm.NumPoints()) != DTM_SUCCESS ) goto errexit ; 
    if(bcdtmObject_allocatePointsMemoryDtmObject(dtmP) != DTM_SUCCESS) goto errexit;
    if(bcdtmObject_allocateNodesMemoryDtmObject(dtmP)  != DTM_SUCCESS) goto errexit;
    if(bcdtmObject_allocateCircularListMemoryDtmObject(dtmP) != DTM_SUCCESS) goto errexit;

    if(mxdtm.markExternalTriangles() != DTM_SUCCESS) goto errexit;

    if(mxdtm.addPointsToDtmObject()  != DTM_SUCCESS) goto errexit;

    if(mxdtm.addTriangleLinksToDtmObject() != DTM_SUCCESS) goto errexit;

    // Need to trace links and add features
    if(mxdtm.traceAndAddFeaturesDtmObject() != DTM_SUCCESS) goto errexit;

    // Need to find and add Voids and Islands and regions...
    // Find out where all the remaining NULL triangles are and add them, at the same time we should look for regions as well.
    // We need the outer boundary and inner boundary for voids and islands, and just the outer one for regions.

   if(mxdtm.findRegionsAndVoids() != DTM_SUCCESS) goto errexit;

    /*
    ** Mark Internal Void Points
    */
    //     if( numVoids || numHoles || numBreakVoids || numDrapeVoids || numIslands )
        {
        long startTime = bcdtmClock() ;
        if( dbg ) bcdtmWrite_message(0,0,0,"Marking Void Points") ;
        if( bcdtmTin_markInternalVoidPointsDtmObject(dtmP)) goto errexit ;
        /*
        ** Clip Dtm Features To Void Boundaries
        */
        if( dbg ) bcdtmWrite_message(0,0,0,"Clipping Dtm Features To Voids") ;
        if( bcdtmTin_clipDtmFeaturesToVoidsDtmObject(dtmP)) goto errexit ;
        if( tdbg ) bcdtmWrite_message(0,0,0,"** Mark Time = %8.3lf Seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;
        }

        if (bcdtmEdit_removeInsertedVoidsOnTinHullDtmObject (dtmP, 0) != DTM_SUCCESS ) goto errexit;
/*
**     Clean DTM Object
*/
        if( bcdtmList_cleanDtmObject(dtmP) != DTM_SUCCESS ) goto errexit ;

 //       if(bcdtmList_countTrianglesAndLinesDtmObject(dtmP,&dtmP->numTriangles,&dtmP->numLines) != DTM_SUCCESS) goto errexit;

 //       if(bcdtmTin_resortTinStructureDtmObject(dtmP) != DTM_SUCCESS) goto errexit;

        if(tdbg == 1) bcdtmWrite_message(0,0,0,"**** Time To Import mxTin = %12.3lf seconds",bcdtmClock_elapsedTime(bcdtmClock(),startTime)) ;

        if(cdbg == 1) if(bcdtmCheck_tinComponentDtmObject(dtmP) != DTM_SUCCESS) goto errexit;

        if( dbg ) bcdtmWrite_toFileDtmObject(dtmP,L"mxExported.bcdtm") ;
        /*
        ** Cleanup
        */
cleanup :
        // free the temporary arrays.
        /*
        ** Job Completed
        */
        return(ret) ;
        /*
        ** Error Exit
        */
errexit :
        if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
        goto cleanup ;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmFormatMX_insertRectangleAroundTinDtmObject
(
 BC_DTM_OBJ *dtmP,
 double  xdec,                     /* ==> Decrement Tin Xmin                   */
 double  xinc,                     /* ==> Increment Tin Xmax                   */  
 double  ydec,                     /* ==> Decrement Tin Ymin                   */
 double  yinc,                     /* ==> Increment Tin Ymax                   */
 DTMFeatureId *islandFeatureIdP  /* <== Island Feature Id For Prior Tin Hull */
 )
 /*
 ** This Function Adds A Surrounding Rectangle To A Triangulated DTM
 ** To Simulate An MX Triangulation In bcLIB DTM. This Is Done Prior To
 ** Exporting bcLIB Triangles To An MX Triangulation
 */
    {
    int ret=DTM_SUCCESS,dbg=0,cdbg=0 ;
    long point,closePoint ;
    long dtmFeature,numHullPts,hullFeature,rectangleFeature,tinPoints[4] ;
    long priorPnt,startPnt,nextPnt,endPnt,clkPnt ;
    long cPriorPnt,cStartPnt,cNextPnt,cClkPnt ;
//    long antPnt,padHull,hullPnt,nextHullPnt ;
    DPoint3d  rectanglePts[5],*hullPtsP=NULL ;
    DTM_TIN_POINT *pointP ;
    BC_DTM_OBJ *tempDtmP=NULL ;
    BC_DTM_FEATURE *dtmFeatureP ;

    /*
    ** Write Entry Message
    */
    if( dbg ) 
        {
        bcdtmWrite_message(0,0,0,"Inserting Rectangle Around Tin") ; 
        bcdtmWrite_message(0,0,0,"dtmP  = %p",dtmP) ; 
        bcdtmWrite_message(0,0,0,"xdec  = %12.5lf",xdec) ; 
        bcdtmWrite_message(0,0,0,"xinc  = %12.5lf",xinc) ; 
        bcdtmWrite_message(0,0,0,"ydec  = %12.5lf",ydec) ; 
        bcdtmWrite_message(0,0,0,"yinc  = %12.5lf",yinc) ; 
        } 
    /*
    ** Initialise
    */
    *islandFeatureIdP = DTM_NULL_FEATURE_ID ;
    /*
    ** Test For Valid Dtm Object
    */
    if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
    /*
    ** Check DTM Is In Triangulated State
    */
    if( dtmP->dtmState != DTMState::Tin ) 
        { 
        bcdtmWrite_message(2,0,0,"Method Requires Triangulated Dtm") ;
        goto errexit ;
        } 
    /*
    ** Write Stats Prior To Adding Rectangle
    */
    if( dbg )
        {
        bcdtmWrite_message(0,0,0,"Before Adding External Rectangle") ; 
        bcdtmWrite_message(0,0,0,"dtmP->numPoints        = %8ld",dtmP->numPoints) ; 
        bcdtmWrite_message(0,0,0,"dtmP->numSortedPoints  = %8ld",dtmP->numSortedPoints) ; 
        bcdtmWrite_message(0,0,0,"dtmP->numTriangles     = %8ld",dtmP->numTriangles) ; 
        bcdtmWrite_message(0,0,0,"dtmP->numLines         = %8ld",dtmP->numLines) ; 
        }
    /*
    ** Add Old Tin Hull As Island Feature
    */
     if( bcdtmList_copyHptrListToTptrListDtmObject(dtmP,0)) goto errexit ;
     if( bcdtmInsert_addDtmFeatureToDtmObject(dtmP,NULL,0,DTMFeatureType::Island,-9999,dtmP->dtmFeatureIndex,0,1)) goto errexit ;
     *islandFeatureIdP = dtmP->dtmFeatureIndex ;
     ++dtmP->dtmFeatureIndex ;
   /*
    ** Increment Circular List Memory
    */
    if( bcdtmObject_allocateCircularListMemoryDtmObject(dtmP)) goto errexit ;
    /*
    ** Pad Out Tin Hull Until It Is Convex
    */
/*
     if( dbg ) bcdtmWrite_message(0,0,0,"Padding Tin Hull") ; 
     padHull = TRUE ;
     while( padHull == TRUE )
       {
        padHull = FALSE ;
        hullPnt = dtmP->hullPoint ;
        do
          {
           nextHullPnt = nodeAddrP(dtmP,hullPnt)->hPtr ;
           if(( antPnt = bcdtmList_nextAntDtmObject(dtmP,nextHullPnt,hullPnt)) < 0 ) goto errexit ;
           if( bcdtmMath_pointSideOfDtmObject(dtmP,hullPnt,antPnt,nextHullPnt) > 0 )
             {
              padHull = TRUE ;
              if( bcdtmList_insertLineAfterPointDtmObject(dtmP,hullPnt,antPnt,nextHullPnt)) goto errexit ;
              if( bcdtmList_insertLineBeforePointDtmObject(dtmP,antPnt,hullPnt,nextHullPnt)) goto errexit ;
              nodeAddrP(dtmP,hullPnt)->hPtr      = antPnt ;
              nodeAddrP(dtmP,nextHullPnt)->hPtr  = dtmP->nullPnt ;
              if( nextHullPnt == dtmP->hullPoint ) dtmP->hullPoint = antPnt ;
              nextHullPnt = antPnt ;
             } 
           hullPnt = nextHullPnt ;
          } while ( hullPnt != dtmP->hullPoint ) ; 
       }        
*/
    /*
    ** Create Triangulation Of Existing Tin Hull And Surrounding Rectangle
    */
    if( bcdtmObject_createDtmObject(&tempDtmP) != DTM_SUCCESS ) goto errexit ;
    /*
    ** Extract Tin Hull
    */
    if( bcdtmList_extractHullDtmObject(dtmP,&hullPtsP,&numHullPts)) goto errexit ;
    if( dbg ) bcdtmWrite_message(0,0,0,"Number Of Hull Points = %6ld",numHullPts) ;
    /*
    ** Set Point Memory Allocation Parameters
    */
    bcdtmObject_setPointMemoryAllocationParametersDtmObject(tempDtmP,numHullPts*4+4,numHullPts) ;
    /*
    ** Store Tin Hull - Store Also As Graphic Breaks To Assist With The Constraining Of The Triangulation
    */
    if( bcdtmObject_storeDtmFeatureInDtmObject(tempDtmP,DTMFeatureType::GraphicBreak,1,1,&tempDtmP->nullFeatureId,hullPtsP,numHullPts)) goto errexit ;
    if( bcdtmObject_storeDtmFeatureInDtmObject(tempDtmP,DTMFeatureType::GraphicBreak,1,1,&tempDtmP->nullFeatureId,hullPtsP,numHullPts)) goto errexit ;
    if( bcdtmObject_storeDtmFeatureInDtmObject(tempDtmP,DTMFeatureType::GraphicBreak,1,1,&tempDtmP->nullFeatureId,hullPtsP,numHullPts)) goto errexit ;
    if( bcdtmObject_storeDtmFeatureInDtmObject(tempDtmP,DTMFeatureType::Breakline,1,1,&tempDtmP->nullFeatureId,hullPtsP,numHullPts)) goto errexit ;
    /*
    ** Store Surrounding Rectangle
    */
    rectanglePts[0].x = dtmP->xMin - xdec ; rectanglePts[0].y = dtmP->yMin - ydec ; rectanglePts[0].z = - 999 ;
    rectanglePts[1].x = dtmP->xMax + xinc ; rectanglePts[1].y = dtmP->yMin - ydec ; rectanglePts[1].z = - 999 ;
    rectanglePts[2].x = dtmP->xMax + xinc ; rectanglePts[2].y = dtmP->yMax + yinc ; rectanglePts[2].z = - 999 ;
    rectanglePts[3].x = dtmP->xMin - xdec ; rectanglePts[3].y = dtmP->yMax + yinc ; rectanglePts[3].z = - 999 ;
    rectanglePts[4].x = dtmP->xMin - xdec ; rectanglePts[4].y = dtmP->yMin - ydec ; rectanglePts[4].z = - 999 ;
    if( bcdtmObject_storeDtmFeatureInDtmObject(tempDtmP,DTMFeatureType::Breakline,2,1,&tempDtmP->nullFeatureId,rectanglePts,5)) goto errexit ;
    /*
    ** Triangulate DTM
    */
    if( dbg )  bcdtmWrite_message(0,0,0,"Triangulating Temp Dtm Object ** tempDtmP->numPoints = %8ld",tempDtmP->numPoints) ; //
    DTM_NORMALISE_OPTION  = FALSE ;             // To Inhibit Normalisation Of Coordinates  
    DTM_DUPLICATE_OPTION = FALSE ;             // To Inhibit Removal Of Near Identical Points
    dtmP->ppTol = 0.0 ;
    dtmP->plTol = 0.0 ;  
    if( bcdtmObject_createTinDtmObject(tempDtmP,1,0.0)) goto errexit ;
    DTM_NORMALISE_OPTION  = TRUE ;
    DTM_DUPLICATE_OPTION = TRUE ;
    if( dbg )  bcdtmWrite_message(0,0,0,"Triangulating Temp Dtm Object Completed ** tempDtmP->numPoints = %8ld",tempDtmP->numPoints) ; 
    /*
    ** Check Triangulation
    */
    if( cdbg )
        {
         bcdtmWrite_message(0,0,0,"Checking Temp Triangulation") ; 
        if( bcdtmCheck_tinComponentDtmObject(tempDtmP))
            {
            bcdtmWrite_message(2,0,0,"Triangulation Corrupted") ;
            goto errexit ;
            }
         bcdtmWrite_message(0,0,0,"Checking Temp Triangulation Completed") ; 
        }
    /*
    ** Find Entry In Tin For Hull And Rectangle Features
    */
    hullFeature      = tempDtmP->nullPnt ;
    rectangleFeature = tempDtmP->nullPnt ;
    for( dtmFeature = 0 ; dtmFeature < tempDtmP->numFeatures ; ++dtmFeature )
        {
        dtmFeatureP = ftableAddrP(tempDtmP,dtmFeature) ;
        if( dtmFeatureP->dtmUserTag == 1 ) hullFeature = dtmFeature ;
        if( dtmFeatureP->dtmUserTag == 2 ) rectangleFeature = dtmFeature ;
        }
    if( hullFeature == tempDtmP->nullPnt || rectangleFeature == tempDtmP->nullPnt )
        {
        bcdtmWrite_message(2,0,0,"Cannot Find Feature Entries") ;
        goto errexit ;
        }
    if( dbg ) bcdtmWrite_message(0,0,0,"hullFeature = %8ld ** rectangleFeature = %8ld",hullFeature,rectangleFeature) ;
    /*
    ** Add Rectangle Points To Tin
    */
    if( dbg ) bcdtmWrite_message(0,0,0,"Adding Rectangle Points To Tin") ;
    if( bcdtmInsert_addPointToDtmObject(dtmP,rectanglePts[0].x,rectanglePts[0].y,rectanglePts[0].z,&tinPoints[0])) goto errexit ;   
    if( bcdtmInsert_addPointToDtmObject(dtmP,rectanglePts[1].x,rectanglePts[1].y,rectanglePts[1].z,&tinPoints[1])) goto errexit ;   
    if( bcdtmInsert_addPointToDtmObject(dtmP,rectanglePts[2].x,rectanglePts[2].y,rectanglePts[2].z,&tinPoints[2])) goto errexit ;   
    if( bcdtmInsert_addPointToDtmObject(dtmP,rectanglePts[3].x,rectanglePts[3].y,rectanglePts[3].z,&tinPoints[3])) goto errexit ;   
    /*
    ** Add Rectangle Lines To Tin
    */
    if( dbg ) bcdtmWrite_message(0,0,0,"Adding Rectangle Lines To Tin") ;
    if( bcdtmList_insertLineDtmObject(dtmP,tinPoints[0],tinPoints[1])) goto errexit ;
    if( bcdtmList_insertLineDtmObject(dtmP,tinPoints[1],tinPoints[2])) goto errexit ;
    if( bcdtmList_insertLineDtmObject(dtmP,tinPoints[2],tinPoints[3])) goto errexit ;
    if( bcdtmList_insertLineDtmObject(dtmP,tinPoints[3],tinPoints[0])) goto errexit ;
    /*
    ** Scan Temp Tin And Set DTM Point Number In sPtr Array
    */
    for( point = 0 ; point < tempDtmP->numPoints ; ++point )
        {
        pointP = pointAddrP(tempDtmP,point) ;
        bcdtmFind_closestPointDtmObject(dtmP,pointP->x,pointP->y,&closePoint) ;
        nodeAddrP(tempDtmP,point)->sPtr = closePoint  ;
        }
    /*
    ** Check For Duplicate Points
    */
    if( cdbg )
        {
        for( point = 0 ; point < tempDtmP->numPoints ; ++point )
            {
            for( startPnt = point + 1 ; startPnt < tempDtmP->numPoints ; ++startPnt )
                {
                if( nodeAddrP(tempDtmP,point)->sPtr == nodeAddrP(tempDtmP,startPnt)->sPtr )
                    {
                    bcdtmWrite_message(0,0,0,"Duplicate Points %8ld %8ld ** %8ld %8ld",point,startPnt,nodeAddrP(tempDtmP,point)->sPtr,nodeAddrP(tempDtmP,startPnt)->sPtr) ;
                    ret = DTM_ERROR ;
                    }
                }  
            }   
        if( ret == DTM_ERROR )
            {
            bcdtmWrite_message(2,0,0,"Duplicate Points In Rectangle Infill") ;
            goto errexit ;
            } 
        }
    /*
    ** Fill Area External To Old Tin Hull 
    */
    if( dbg ) bcdtmWrite_message(0,0,0,"Filling Area External To Hull") ;
    if( bcdtmList_copyDtmFeatureToTptrListDtmObject(tempDtmP,hullFeature,&startPnt)) goto errexit ;
    priorPnt = startPnt ;
    startPnt = nodeAddrP(tempDtmP,startPnt)->tPtr ; 
    endPnt   = startPnt ;
    do
        {
        nextPnt   = nodeAddrP(tempDtmP,startPnt)->tPtr ; 
        cStartPnt = nodeAddrP(tempDtmP,startPnt)->sPtr ;
        cNextPnt  = nodeAddrP(tempDtmP,nextPnt)->sPtr ;
        if( ( clkPnt = bcdtmList_nextClkDtmObject(tempDtmP,startPnt,nextPnt)) < 0 ) goto errexit ;
        while( clkPnt != priorPnt )
            {
            cClkPnt = nodeAddrP(tempDtmP,clkPnt)->sPtr ;
            if( bcdtmList_insertLineAfterPointDtmObject(dtmP,cStartPnt,cClkPnt,cNextPnt)) goto errexit ;
            cNextPnt = cClkPnt ;
            if( ( clkPnt = bcdtmList_nextClkDtmObject(tempDtmP,startPnt,clkPnt)) < 0 ) goto errexit ;
            } 
        priorPnt = startPnt ;
        startPnt = nextPnt  ;
        } while (  startPnt != endPnt ) ;
        /*
        ** Null Tptr list
        */
        if( bcdtmList_nullTptrListDtmObject(dtmP,startPnt)) goto errexit ;
        /*
        ** Fill Area Internal To Rectangle 
        */
        if( dbg ) bcdtmWrite_message(0,0,0,"Filling Area Internal To Reactangle") ;
        if( bcdtmList_copyDtmFeatureToTptrListDtmObject(tempDtmP,rectangleFeature,&startPnt)) goto errexit ;
        priorPnt = startPnt ;
        startPnt = nodeAddrP(tempDtmP,startPnt)->tPtr ; 
        endPnt   = startPnt ;
        do
            {
            nextPnt  = nodeAddrP(tempDtmP,startPnt)->tPtr ; 
            cStartPnt = nodeAddrP(tempDtmP,startPnt)->sPtr ;
            cPriorPnt  = nodeAddrP(tempDtmP,priorPnt)->sPtr ;
            if( ( clkPnt = bcdtmList_nextClkDtmObject(tempDtmP,startPnt,priorPnt)) < 0 ) goto errexit ;
            while( clkPnt != nextPnt )
                {
                cClkPnt = nodeAddrP(tempDtmP,clkPnt)->sPtr ;
                if( bcdtmList_insertLineAfterPointDtmObject(dtmP,cStartPnt,cClkPnt,cPriorPnt)) goto errexit ;
                cPriorPnt = cClkPnt ;
                if( ( clkPnt = bcdtmList_nextClkDtmObject(tempDtmP,startPnt,clkPnt)) < 0 ) goto errexit ;
                } 
            priorPnt = startPnt ;
            startPnt = nextPnt  ;
            } while (  startPnt != endPnt ) ;
            /*
            ** Null Tptr list
            */
            if( bcdtmList_nullTptrListDtmObject(dtmP,startPnt)) goto errexit ;
             /*
            ** Insert Void Feature Around Hull
            */
            if( bcdtmList_setConvexHullDtmObject(dtmP)) goto errexit ;
            if( bcdtmList_copyHptrListToTptrListDtmObject(dtmP,dtmP->hullPoint)) goto errexit ;
            if( bcdtmInsert_addDtmFeatureToDtmObject(dtmP,NULL,0,DTMFeatureType::Void,-9999,dtmP->dtmFeatureIndex,dtmP->hullPoint,1)) goto errexit ;
            ++dtmP->dtmFeatureIndex ;
            /*
            ** Clean Dtm
            */
            if( dbg ) bcdtmWrite_message(0,0,0,"Cleaning DTM Object") ;
            if( bcdtmList_cleanDtmObject(dtmP)) goto errexit ;
            /*
            ** Check Tin
            */
            if( cdbg )
                {
                bcdtmWrite_message(0,0,0,"Checking Triangulation") ;
                if( bcdtmCheck_tinComponentDtmObject(dtmP))
                    {
                    bcdtmWrite_message(2,0,0,"Triangulation Corrupted") ;
                    goto errexit ;
                    }
                bcdtmWrite_message(0,0,0,"Checking Triangulation Completed") ;
                }
            /*
            ** Write Stats After Adding Rectangle
            */
            if( dbg )
                {
                bcdtmWrite_message(0,0,0,"After Adding External Rectangle") ; 
                bcdtmWrite_message(0,0,0,"dtmP->numPoints        = %8ld",dtmP->numPoints) ; 
                bcdtmWrite_message(0,0,0,"dtmP->numSortedPoints  = %8ld",dtmP->numSortedPoints) ; 
                bcdtmWrite_message(0,0,0,"dtmP->numTriangles     = %8ld",dtmP->numTriangles) ; 
                bcdtmWrite_message(0,0,0,"dtmP->numLines         = %8ld",dtmP->numLines) ; 
                }
            /*
            ** Clean Up
            */
cleanup :
            DTM_NORMALISE_OPTION  = TRUE ;
            DTM_DUPLICATE_OPTION = TRUE ;
            if( hullPtsP != NULL ) { free(hullPtsP) ; hullPtsP = NULL ; }
            /*
            ** Job Completed
            */
            if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Inserting Rectangle Around Tin Completed") ; 
            if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Inserting Rectangle Around Tin Error") ; 
            return(ret) ;
            /*
            ** Error Exit
            */
errexit :
            if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
            goto cleanup ;
    }


/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTMFORMATS_EXPORT int bcdtmFormatMX_loadMxTrianglesFromDtmObject
(
 BC_DTM_OBJ *dtmP,
 int (*loadFunctionP)(int trgNum, int trgPnt1, int trgPnt2,int trgPnt3, int voidTriangle, int side1Trg, int side2Trg, int side3Trg, void* userP),
 void* userArgP
 )
 /*
 ** This Function Loads MX Triangles From A DTM Object
 */
    {
    int  ret=DTM_SUCCESS,dbg=0 ;
    long p1,p2,p3,clc,numTriangles,voidTriangle ;
    long trgNum,side1Trg,side2Trg,side3Trg ;
    DTM_CIR_LIST *clistP ;
    DTM_MX_TRG_INDEX *indexP,*trgIndexP=NULL ;
    /*
    ** Write Entry Message
    */
    if( dbg ) 
        {
        bcdtmWrite_message(0,0,0,"Loading MX Triangles From BcCivil DTM") ; 
        bcdtmWrite_message(0,0,0,"dtmP           = %p",dtmP) ; 
        bcdtmWrite_message(0,0,0,"loadFunctionP  = %p",loadFunctionP) ; 
        } 
    /*
    ** Test For Valid Dtm Object
    */
    if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
    /*
    ** Check DTM Is In Triangulated State
    */
    if( dtmP->dtmState != DTMState::Tin ) 
        { 
        bcdtmWrite_message(2,0,0,"Method Requires Triangulated Dtm") ;
        goto errexit ;
        } 
    /*
    ** Check Load Function Is Set
    */
    if( loadFunctionP == NULL )
        {
        bcdtmWrite_message(1,0,0,"Load Function Not Set") ;
        goto errexit ; 
        }
    /*
    ** Allocate Memory For Triangle Index
    */
    trgIndexP = ( DTM_MX_TRG_INDEX * ) malloc ( dtmP->numTriangles * sizeof( DTM_MX_TRG_INDEX )) ;
    if( trgIndexP == NULL )
        {
        bcdtmWrite_message(1,0,0,"Memory Allocation Failure") ;
        goto errexit ;
        }
    /*
    ** Populate Triangle Index
    */
    numTriangles = 0 ;
    indexP = trgIndexP ;
    for( p1 = 0 ; p1 < dtmP->numPoints ; ++p1 )	       
        {
        (trgIndexP+p1)->index = numTriangles ;
        if( nodeAddrP(dtmP,p1)->cPtr != dtmP->nullPtr )
            {
            clc = nodeAddrP(dtmP,p1)->cPtr;
            if( ( p2 = bcdtmList_nextAntDtmObject(dtmP,p1,clistAddrP(dtmP,clc)->pntNum)) < 0 ) return(1) ;
            while( clc != dtmP->nullPtr )
                {
                clistP = clistAddrP(dtmP,clc) ;
                p3     = clistP->pntNum ;
                clc    = clistP->nextPtr ;
                if( p2 > p1 && p3 > p1 && nodeAddrP(dtmP,p3)->hPtr != p1 )
                    {
                    if( numTriangles >= dtmP->numTriangles ) 
                        {
                        bcdtmWrite_message(2,0,0,"Too Many Index Triangles %6ld of %6ld ",numTriangles,dtmP->numTriangles) ;
                        goto errexit ;
                        }
                    indexP->trgPnt1 = p1 ;
                    indexP->trgPnt2 = p2 ;
                    indexP->trgPnt3 = p3 ;
                    ++indexP ;
                    ++numTriangles   ;
                    }
                p2 = p3 ;
                }
            }          
        }
    /*
    ** Check Index Size
    */
    if( dbg ) bcdtmWrite_message(0,0,0,"numTriangles = %8ld ** dtmP->numTriangles = %8ld",numTriangles,dtmP->numTriangles) ;
    if( numTriangles != dtmP->numTriangles )
        {
        bcdtmWrite_message(1,0,0,"Triangle Index Size Incorrect") ;
        goto errexit ;
        }
    /*
    ** Scan Triangle Index And Load Triangles
    */
    if( dbg ) bcdtmWrite_message(0,0,0,"Loading MX Triangles") ;
    for( trgNum = 0 ,indexP = trgIndexP ; indexP < trgIndexP + numTriangles ; ++indexP , ++trgNum )
        {
        /*
        **  Get Adjancies For Each Triangle Side
        */
        side1Trg = -1 ;
        if( nodeAddrP(dtmP,indexP->trgPnt2)->hPtr != indexP->trgPnt1 )
            {
            if( ( p1 = bcdtmList_nextClkDtmObject(dtmP,indexP->trgPnt2,indexP->trgPnt1)) < 0 ) goto errexit ;
            if( bcdtmFormatMX_getMxTriangleNumberDtmObject(dtmP,trgIndexP,p1,indexP->trgPnt2,indexP->trgPnt1,&side1Trg))
                { 
                bcdtmWrite_message(1,0,0,"Failed To Get Adjacency For Side1") ;
                goto errexit ;
                }
            }
        side2Trg = -1 ;
        if( nodeAddrP(dtmP,indexP->trgPnt3)->hPtr != indexP->trgPnt2 )
            {
            if( ( p1 = bcdtmList_nextClkDtmObject(dtmP,indexP->trgPnt3,indexP->trgPnt2)) < 0 ) goto errexit ;
            if( bcdtmFormatMX_getMxTriangleNumberDtmObject(dtmP,trgIndexP,p1,indexP->trgPnt3,indexP->trgPnt2,&side2Trg) )
                {
                bcdtmWrite_message(1,0,0,"Failed To Get Adjacency For Side2") ;
                goto errexit ;
                }
            }
        side3Trg = -1 ;
        if( nodeAddrP(dtmP,indexP->trgPnt1)->hPtr != indexP->trgPnt3 )
            {
            if( ( p1 = bcdtmList_nextClkDtmObject(dtmP,indexP->trgPnt1,indexP->trgPnt3)) < 0 ) goto errexit ;
            if( bcdtmFormatMX_getMxTriangleNumberDtmObject(dtmP,trgIndexP,p1,indexP->trgPnt1,indexP->trgPnt3,&side3Trg))
                {
                bcdtmWrite_message(1,0,0,"Failed To Get Adjacency For Side3") ;
                goto errexit ;
                }
            }
        /*
        **  Test For Void triangles
        */
        if( bcdtmList_testForVoidTriangleDtmObject(dtmP,indexP->trgPnt1,indexP->trgPnt2,indexP->trgPnt3,&voidTriangle)) goto errexit ;
        /*
        **  Call Load Function
        */
        if( loadFunctionP(trgNum,indexP->trgPnt1,indexP->trgPnt2,indexP->trgPnt3,voidTriangle,side1Trg,side2Trg,side3Trg, userArgP)) goto errexit ;
        }
    /*
    ** Clean Up
    */
cleanup :
    if( trgIndexP != NULL ) { free(trgIndexP) ; trgIndexP = NULL ; }
    /*
    ** Job Completed
    */
    if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Loading MX Triangles From BcCivil DTM Completed") ; 
    if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Loading MX Triangles From BcCivil DTM Error") ; 
    return(ret) ;
    /*
    ** Error Exit
    */
errexit :
    if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
    goto cleanup ;
    }
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmFormatMX_getMxTriangleNumberDtmObject
(
 BC_DTM_OBJ       *dtmP,
 DTM_MX_TRG_INDEX *trgIndexP,
 long             trgPnt1,
 long             trgPnt2,
 long             trgPnt3,
 long             *trgNumP 
 )
 /*
 ** This Function Finds The Entry In The Triangle Index For Triangle <trgPnt1,trgPnt2,trgPnt3>
 ** And Returns The Triangle Number
 ** 
 ** Note trgPnt1,trgPnt2,trgPnt3 Must Be In A Clockwise Direction
 ** 
 */
    {  
    int     ret=DTM_SUCCESS ;
    long    sp,process ;
    DTM_MX_TRG_INDEX *bIndexP,*tIndexP    ;
    /*
    ** Initialise
    */
    *trgNumP = dtmP->nullPnt ;
    while( trgPnt1 > trgPnt2 || trgPnt1 > trgPnt3 ) { sp = trgPnt1 ; trgPnt1 = trgPnt2 ; trgPnt2 = trgPnt3 ; trgPnt3 = sp ; }
    /*
    ** Get First Entry For trgPnt1
    */
    bIndexP = trgIndexP + (trgIndexP+trgPnt1)->index  ; 
    tIndexP = trgIndexP + dtmP->numTriangles ;
    /*
    ** Scan trgPnt1 Entries Looking For trgPnt2 && trgPnt3
    */
    process = 1 ;
    while ( bIndexP < tIndexP && process && *trgNumP == dtmP->nullPnt )
        {
        if( bIndexP->trgPnt1 != trgPnt1 ) process = 0 ;
        else
            {
            if( bIndexP->trgPnt2 == trgPnt2 && bIndexP->trgPnt3 == trgPnt3 ) 
                { 
                *trgNumP = (long)(bIndexP-trgIndexP) ;
                }
            ++bIndexP ;
            }
        }
    /*
    ** If Triangle Not Found Error Exit
    */
    if( *trgNumP == dtmP->nullPnt ) goto errexit ;
    /*
    ** Cleanup
    */
cleanup :
    /*
    ** Job Completed
    */
    return(ret) ;
    /*
    ** Error Exit
    */
errexit :
    if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
    goto cleanup ;
    }

int bcdtmMXClip_toTptrPolygonLeavingFeaturesDtmObject(BC_DTM_OBJ *dtmP,long startPnt,DTMClipOption clipOption)
{
 int    ret=DTM_SUCCESS,dbg=0,cdbg=0 ;
 DTMDirection direction;//,dtmFeature ;
 double area ;
// BC_DTM_FEATURE *dtmFeatureP ;
/*
** Write Entry Message
*/
 if( dbg )
   {
    bcdtmWrite_message(0,0,0,"Clipping Dtm Object To Tptr Polygon") ;
    bcdtmWrite_message(0,0,0,"dtmP       = %p",dtmP) ;
    bcdtmWrite_message(0,0,0,"startPnt   = %8ld",startPnt) ;
    bcdtmWrite_message(0,0,0,"clipOption = %8ld",clipOption) ;
   }
/*
** Test For Valid Clip Option
*/
 if( clipOption != DTMClipOption::Internal && clipOption != DTMClipOption::External )
   {
    bcdtmWrite_message(2,0,0,"Invalid Clip Option") ;
    goto errexit ;
   }
/*
** Test For Valid Dtm  Object
*/
 if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
/*
** Check If DTM Is In Tin State
*/
 if( dtmP->dtmState != DTMState::Tin )
   {
    bcdtmWrite_message(2,0,0,"DTM Object %p Not Triangulated",dtmP) ;
    goto errexit ;
   }
/*
** Check For Valid Tptr Polygon Start Point
*/
 if( startPnt < 0 || startPnt >= dtmP->numPoints || nodeAddrP(dtmP,startPnt)->tPtr < 0 || nodeAddrP(dtmP,startPnt)->tPtr >= dtmP->numPoints )
   {
    bcdtmWrite_message(2,0,0,"Invalid Start Point For Clip Tptr Polygon") ;
    goto errexit ;
   }
/*
** Check Connectivity Of Tptr Polygon - Development Only
*/
 if( cdbg )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Checking Connectivity Tptr Polygon") ;
    if( bcdtmList_checkConnectivityTptrPolygonDtmObject(dtmP,startPnt,0)) goto errexit ;
   }
/*
** Check Direction Of Tptr Polygon And If Clockwise Set Direction Anti Clockwise
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Checking Direction Tptr Polygon") ;
 if( bcdtmMath_calculateAreaAndDirectionTptrPolygonDtmObject(dtmP,startPnt,&area,&direction) ) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Tptr Polygon Area = %15.4lf Direction = %2ld",area,direction) ;
 if( direction == DTMDirection::Clockwise )
   {
    if( bcdtmList_reverseTptrPolygonDtmObject(dtmP,startPnt)) goto errexit ;
   }
/*
** Check Integrity Of Dtm Object - Development Only
*/
 if( cdbg )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Checking Dtm") ;
    if( bcdtmCheck_tinComponentDtmObject(dtmP)) { bcdtmWrite_message(0,0,0,"DTM Corrupted") ; goto errexit ; }
    else                                          bcdtmWrite_message(0,0,0,"DTM OK") ;
   }
/*
** Clip Internal To Tptr Polygon
*/
 if( clipOption == DTMClipOption::Internal )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Clipping Dtm Object Internal") ;
    if( bcdtmClip_internalToTptrPolygonDtmObject(dtmP,startPnt,1)) goto errexit ;
   }
/*
** Clip External To Tptr Polygon
*/
 if( clipOption == DTMClipOption::External )
   {
    if( dbg ) bcdtmWrite_message(0,0,0,"Clipping Dtm Object External") ;
    if( bcdtmClip_externalToTptrPolygonDtmObject(dtmP,startPnt)) goto errexit ;
   }
/*
** Clean Dtm Object
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Cleaning Dtm Object ** dtmP->numFeatures = %8ld",dtmP->numFeatures) ;
 if( bcdtmList_cleanDtmObject(dtmP)) goto errexit ;
 if( dbg ) bcdtmWrite_message(0,0,0,"Cleaning Dtm Object Completed ** dtmP->numFeatures = %8ld",dtmP->numFeatures) ;
/*
** Check Integrity Of Dtm Object - Development Only
*/
 if( cdbg )
   {
    bcdtmWrite_message(0,0,0,"Checking Clipped Dtm") ;
    if( bcdtmCheck_tinComponentDtmObject(dtmP))
      {
       bcdtmWrite_message(0,0,0,"DTM Corrupted After Clip") ;
       goto errexit ;
      }
    else bcdtmWrite_message(0,0,0,"DTM OK") ;
   }
/*
** Clean Up
*/
 cleanup :
/*
** Job Completed
*/
 if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Clipping Dtm Object To Tptr Polygon Completed") ;
 if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Clipping Dtm Object To Tptr Polygon Error") ;
 return(ret) ;
/*
** Error Exit
*/
 errexit :
 if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
 goto cleanup ;
}
/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTM_Private int bcdtmFormatMX_clipUsingIslandFeatureIdDtmObject
(
 BC_DTM_OBJ       *dtmP,
 DTMFeatureId   dtmFeatureId  // Feature Id Of An Island Feature Used To Store The Old Tin Hull
 )
 /*
 ** This Is A Special Purpose Clean Up Function That Is Called After Exporting MX Triangles
 ** It Removes The External Void Triangles That Are Created To Simulate An MX Triangulation
 ** 
 */
    {  
    int     ret=DTM_SUCCESS,dbg=0 ;
    long    dtmFeature,hullFeature,startPnt ;
    BC_DTM_FEATURE *dtmFeatureP ;
    /*
    ** Write Entry Message
    */
    if( dbg ) bcdtmWrite_message(0,0,0,"Clipping DTM To Island Feature Id") ;
    /*
    ** Test For Valid Dtm Object
    */
    if( bcdtmObject_testForValidDtmObject(dtmP)) goto errexit ;
    /*
    ** Check DTM Is In Triangulated State
    */
    if( dtmP->dtmState != DTMState::Tin ) 
        { 
        bcdtmWrite_message(2,0,0,"Method Requires Triangulated Dtm") ;
        goto errexit ;
        } 
    /*
    ** Write Stats Prior To Clipping
    */
    if( dbg )
        {
        bcdtmWrite_message(0,0,0,"Before Clipping To Island Feature") ; 
        bcdtmWrite_message(0,0,0,"dtmP->numPoints        = %8ld",dtmP->numPoints) ; 
        bcdtmWrite_message(0,0,0,"dtmP->numSortedPoints  = %8ld",dtmP->numSortedPoints) ; 
        bcdtmWrite_message(0,0,0,"dtmP->numTriangles     = %8ld",dtmP->numTriangles) ; 
        bcdtmWrite_message(0,0,0,"dtmP->numLines         = %8ld",dtmP->numLines) ; 
        bcdtmWrite_message(0,0,0,"dtmP->numFeatures      = %8ld",dtmP->numFeatures) ; 
        }
    /*
    ** Scan For Hull Feature
    */
    hullFeature = dtmP->nullPnt ;
    for( dtmFeature = dtmP->numFeatures - 1 ; dtmFeature >= 0 && hullFeature == dtmP->nullPnt ; --dtmFeature )
        {
        dtmFeatureP = ftableAddrP(dtmP,dtmFeature) ;
        if( dtmFeatureP->dtmFeatureId == dtmFeatureId && dtmFeatureP->dtmFeatureType == DTMFeatureType::Island && dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin )
            {
            hullFeature = dtmFeature ;
            }
        }
    /*
    ** Check Hull Feature Found
    */
    if( hullFeature == dtmP->nullPnt )
        {
        bcdtmWrite_message(2,0,0,"No Island Feature With Required Feature Id Found") ;
        goto errexit ;
        }
    /*
    ** Copy Feature Points To Tptr List
    */
    if( bcdtmList_copyDtmFeatureToTptrListDtmObject(dtmP,hullFeature,&startPnt)) goto errexit ;
    /*
    ** Remove Island Feature
    */
    if( bcdtmInsert_removeDtmFeatureFromDtmObject(dtmP,hullFeature)) goto errexit ;
    /*
    ** Clip DTM To Tptr Polygon
    */
    if( bcdtmMXClip_toTptrPolygonLeavingFeaturesDtmObject(dtmP,startPnt,DTMClipOption::External)) goto errexit ;
    /*
    ** Write Stats After Clipping
    */
    if( dbg )
        {
        bcdtmWrite_message(0,0,0,"After Clipping To Island Feature") ; 
        bcdtmWrite_message(0,0,0,"dtmP->numPoints        = %8ld",dtmP->numPoints) ; 
        bcdtmWrite_message(0,0,0,"dtmP->numSortedPoints  = %8ld",dtmP->numSortedPoints) ; 
        bcdtmWrite_message(0,0,0,"dtmP->numTriangles     = %8ld",dtmP->numTriangles) ; 
        bcdtmWrite_message(0,0,0,"dtmP->numLines         = %8ld",dtmP->numLines) ; 
        bcdtmWrite_message(0,0,0,"dtmP->numFeatures      = %8ld",dtmP->numFeatures) ; 
        }
    /*
    ** Cleanup
    */
cleanup :
    /*
    ** Job Completed
    */
    if( dbg && ret == DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Clipping DTM To Island Feature Id Completed") ;
    if( dbg && ret != DTM_SUCCESS ) bcdtmWrite_message(0,0,0,"Clipping DTM To Island Feature Id Error") ;
    return(ret) ;
    /*
    ** Error Exit
    */
errexit :
    if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
    goto cleanup ;
    }


/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int mxTriangleLoadFunction(int trgNum, int trgPnt1, int trgPnt2,int trgPnt3, int voidTriangle, int side1Trg, int side2Trg, int side3Trg, void* userP)
    {
    MXDTM* mxdtm = (MXDTM*)userP;
    return mxdtm->LoadTriangle(trgNum, trgPnt1, trgPnt2, trgPnt3, voidTriangle, side1Trg, side2Trg, side3Trg);
    }

/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
BENTLEYDTMFORMATS_EXPORT int bcdtmExport_MXTriangulationFromDtmObject(BC_DTM_OBJ* dtmP, void* triPtrP, void* pointsPtrP)
    {
    int ret = DTM_SUCCESS;
    double xdec;
    double xinc;
    double ydec;
    double yinc;

    MXTriangle::TriangleArray& triPtr = *(MXTriangle::TriangleArray*)triPtrP;
    MXTriangle::PointArray& pointsPtr = *(MXTriangle::PointArray*)pointsPtrP;
    DTMFeatureId islandFeatureId;

    MXDTM mxdtmHelper(triPtr, pointsPtr, dtmP);
    TriangleToDTMHelper<MXDTM> mxdtm(mxdtmHelper, dtmP);
    /*
    ** Place Reactangle Around DTM
    */
    bcdtmWrite_message(0,0,0,"Simulating MX Triangulation") ;
    xdec = 500.0 ; xinc = 500.0 ;
    ydec = 500.0 ; yinc = 500.0 ;

    bcdtmMemory_setMemoryAccess (dtmP, DTMAccessMode::Temporary);
    if( bcdtmFormatMX_insertRectangleAroundTinDtmObject(dtmP,xdec,xinc,ydec,yinc,&islandFeatureId)) goto errexit ;

    /*
    ** Export Points to MX Triangulation
    */
    mxdtmHelper.setPoints();
    /*
    ** Load MX Triangles
    */
    bcdtmWrite_message(0,0,0,"Loading MX Triangles") ;
    if( bcdtmFormatMX_loadMxTrianglesFromDtmObject(dtmP,mxTriangleLoadFunction,&mxdtmHelper)) goto errexit ;
    /*
    ** Look at regions
    */
    triPtr.setPhysicalLength(triPtr.size());

    /*
    ** Clip Tin
    */
    bcdtmWrite_message(0,0,0,"Clipping MX DTM") ;
    if( bcdtmFormatMX_clipUsingIslandFeatureIdDtmObject(dtmP,islandFeatureId)) goto errexit ;

    /*
    ** Cleanup
    */
cleanup :
    /*
    ** Job Completed
    */
    return(ret) ;
    /*
    ** Error Exit
    */
errexit :
    if( ret == DTM_SUCCESS ) ret = DTM_ERROR ;
    goto cleanup ;
    }

