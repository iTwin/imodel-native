//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hve/src/HVEBreakLines.cpp $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Inline methods for class HVEBreakLines
//-----------------------------------------------------------------------------

#include "hstdcpp.h"
#include <strstream>
#include "HVEBreakLines.h"
#include "HFCException.h"

//-----------------------------------------------------------------------------
//    GLOBAL FUNCTIONS OPERATING ON OBJECT
//-----------------------------------------------------------------------------

/** -----------------------------------------------------------------------------
    Validate
    This method validates the current breaklines loaded in the object.
    The validation will result in one of three possibilities
     0 - Valid
     1 - Error
     2 - Warnings

    The result is returned by the method, but the specifics concerning warnings
    and errors are written to the provided output stream in human readable language.

    The list of defects validated is:
    1.  0x00002000 - No breaklines nor points present.
    1.  0x00000001 - A breakline may not cross itself
    2.  0x00000002 - A vertex must have a minimal length
    3.  0x00000004 - A breakline may not have two consecutive identical points
    4.  0x00000008 - A breakline must have at least two non-dentical points
    5.  0x00000010 - A vertex must not have too long a length
    6.  0x00000020 - The complete length of a breakline must not exceed some reasonable amount
    7.  0x00000040 - Two breaklines may not cross each other.
    8.  0x00000080 - A breakline may not start on another breakline unless at a vertex point (including extremities)
    9.  0x00000100 - Two breaklines may not be contiguous
    10. 0x00000200 - Two breaklines may not flirt other than at a vertex point
    11. 0x00000400 - An elevation point may not be located on a breakline
    12. 0x00000800 - Two breaklines linked at extremity or other vertes point must have the
                     same elevation at these points
    13. 0x00001000 - Two points may not be identical



    @return 0 if the breaklines are valid
            1 if there are errors in the breakline dataset
            2 if the dataset has no errors but warnings were issued.

    Example:
    @code
    KGIBreakLines      MyBreaklines;

    ifstream BreakLineFile("C:\\Data\\Breaklines.wrk");

    BreakLineFile >> MyBreakLines;

    int Result == MyBreakLines.Validate(cout);

    if (Result == 0)
    {
        cout << "DTM is valid" << endl;
    }
    if (Result == 1)
    {
        cout << "DTM is invalid" << endl;
    }
    if (Result == 2)
    {
        cout << "DTM is limitly valid" << endl;
    }

    @end
    -----------------------------------------------------------------------------
*/
int HVEBreakLines::Validate(ostream& o_rErrorStream, string i_Prefix, long i_ValidationMask, bool i_Verbose) const
    {
    // Init result value to valid
    int Result = 0;

    list<HVEBreakLineDefect> ListOfDefects;

    Result = Validate(&ListOfDefects, i_ValidationMask);

    if (Result != 0)
        {
        list<HVEBreakLineDefect>::iterator DefectItr;

        for (DefectItr = ListOfDefects.begin() ; DefectItr != ListOfDefects.end() ; ++DefectItr)
            {
            long DefectCode = DefectItr->GetCode();

            switch (DefectCode)
                {

                case 0x00000001:
                    o_rErrorStream << i_Prefix << "ERR0802 - BreakLine starting at "
                                   << DefectItr->GetLocation().GetX() << ", " << DefectItr->GetLocation().GetY()
                                   << " crosses itself" << endl;
                    break;
                case 0x00000002:
                    o_rErrorStream << i_Prefix << "ERR0822 - Segment is too short. Segment starts at : "
                                   << DefectItr->GetLocation().GetX() << " , "
                                   << DefectItr->GetLocation().GetY() << endl;
                    break;

                case 0x00000004:
                    o_rErrorStream << i_Prefix << "WARNING0800 - Breakline starting at : " << DefectItr->GetLocation().GetX() << " , "
                                   << DefectItr->GetLocation().GetY() << " has double points" << endl;
                    break;
                case 0x00000008:
                    o_rErrorStream << i_Prefix << "ERR0803 - Breakline starting at : "
                                   << DefectItr->GetLocation().GetX() << " , " << DefectItr->GetLocation().GetY()
                                   << " has less than 2 points" << endl;
                    break;

                case 0x00000010:
                    o_rErrorStream << i_Prefix << "ERR0804 - Segment is too long. Segment starts at : " << DefectItr->GetLocation().GetX() << " , "
                                   << DefectItr->GetLocation().GetY() << endl;
                    break;

                case 0x00000020:
                    o_rErrorStream << i_Prefix << "ERR0823 - Breakline starting at : X = " << DefectItr->GetLocation().GetX()
                                   << " , Y = " << DefectItr->GetLocation().GetY() << "is too long" << endl;
                    break;

                case 0x00000040:
                    o_rErrorStream << i_Prefix << "ERR0805 - BreakLines Intersect at point: " << DefectItr->GetLocation().GetX()
                                   << " , " << DefectItr->GetLocation().GetY() << endl;
                    break;

                case 0x00000080:
                    o_rErrorStream << i_Prefix << "ERR0806 - A breakline connects on another at X : "
                                   << DefectItr->GetLocation().GetX() << " Y : " << DefectItr->GetLocation().GetY() << endl;
                    break;

                case 0x00000100:
                    o_rErrorStream << i_Prefix << "ERR0807 - Breaklines are contiguous one to another at X = "
                                   << DefectItr->GetLocation().GetX() << " Y : " << DefectItr->GetLocation().GetY() << endl;
                    break;

                case 0x00000200:
                    o_rErrorStream << i_Prefix << "ERR0822 - Two breaklines flirt other than at a vertex point at X = "
                                   << DefectItr->GetLocation().GetX() << " Y : " << DefectItr->GetLocation().GetY() << endl;
                    break;

                case 0x00000400:
                    o_rErrorStream << i_Prefix << "ERR0820 - Point X = "
                                   << DefectItr->GetLocation().GetX() << " Y : " << DefectItr->GetLocation().GetY()
                                   << " is located on a breakline" << endl;
                    break;

                case 0x00000800:
                    o_rErrorStream << i_Prefix << "ERR0822 - Two breaklines linked at a vertex point have diferent elevation at X = "
                                   << DefectItr->GetLocation().GetX() << " Y : " << DefectItr->GetLocation().GetY() << endl;
                    break;

                case 0x00001000:
                    o_rErrorStream << i_Prefix << "ERR0821 - Double point at X = "
                                   << DefectItr->GetLocation().GetX() << " Y : " << DefectItr->GetLocation().GetY() << endl;
                    break;

                case 0x00002000:
                    o_rErrorStream << i_Prefix << "ERR0800 - No breaklines nor elevation points defined" << endl;
                    break;

                }

            if (i_Verbose &&
                (DefectItr->GetBreakLineIndex() >= 0) &&
                ((DefectCode == 0x00000001) ||
                 (DefectCode == 0x00000002) ||
                 (DefectCode == 0x00000004) ||
                 (DefectCode == 0x00000008) ||
                 (DefectCode == 0x00000010) ||
                 (DefectCode == 0x00000020) ||
                 (DefectCode == 0x00000040) ||
                 (DefectCode == 0x00000080))
               )
                {
                o_rErrorStream << "Following is dump of faulty breakline: " << endl;
                HFCPtr<HVE3DPolyLine> pBreakLine = m_BreakLineList[DefectItr->GetBreakLineIndex()];

                size_t NumPoints = pBreakLine->GetSize();
                size_t i;
                HGF3DCoord<double> MyPoint;
                for (i = 0 ; i < NumPoints ; ++i)
                    {
                    MyPoint = pBreakLine->GetPoint(i);
                    o_rErrorStream << "X : " << MyPoint.GetX() << " Y : " << MyPoint.GetY() << endl;
                    }
                }

            }
        }

    return(Result);
    }



/** -----------------------------------------------------------------------------
    Validate
    This method validates the current breaklines loaded in the object.
    The validation will result in one of three possibilities
     0 - Valid
     1 - Error
     2 - Warnings

    The result is returned by the method in the form of a list of defects

    The list of defects validated is:
    1.  0x00002000 - (Always validated) There must be at least a point or breakline defined

    1.  0x00000001 - A breakline may not cross itself
    2.  0x00000002 - A vertex must have a minimal length
    3.  0x00000004 - A breakline may not have two consecutive identical points
    4.  0x00000008 - A breakline must have at least two non-dentical points
    5.  0x00000010 - A vertex must not have too long a length
    6.  0x00000020 - The complete length of a breakline must not exceed some reasonable amount
    7.  0x00000040 - Two breaklines may not cross each other.
    8.  0x00000080 - A breakline may not start on another breakline unless at a vertex point (including extremities)
    9.  0x00000100 - Two breaklines may not be contiguous
    10. 0x00000200 - Two breaklines may not flirt other than at a vertex point
    11. 0x00000400 - An elevation point may not be located on a breakline
    12. 0x00000800 - Two breaklines linked at extremity or other vertes point must have the
                     same elevation at these points
    13. 0x00001000 - Two points may not be identical



    @return 0 if the breaklines are valid
            1 if there are errors in the breakline dataset
            2 if the dataset has no errors but warnings were issued.

    Example:
    @code
    KGIBreakLines      MyBreaklines;

    ifstream BreakLineFile("C:\\Data\\Breaklines.wrk");

    BreakLineFile >> MyBreakLines;

    list<HVEBreakLineDefect> ListOfDefects;

    int Result == MyBreakLines.Validate(&ListOfDefects);

    if (Result == 0)
    {
        cout << "DTM is valid" << endl;
    }
    if (Result == 1)
    {
        cout << "DTM is invalid" << endl;
    }
    if (Result == 2)
    {
        cout << "DTM is limitly valid" << endl;
    }

    @end
    -----------------------------------------------------------------------------
*/
int HVEBreakLines::Validate(list<HVEBreakLineDefect>* o_pListOfDefects, long i_ValidationMask) const
    {
    // Init result value to valid
    int Result = 0;

    // Check for breaklines crossing
    unsigned long BLIndex;

    //=========================================================================
    // Check for empty DTM
    //=========================================================================
    if (m_BreakLineList.size() == 0 && m_DTMPoints.size() == 0)
        {
        // Indicate the breaklines are invalid
        Result = 1;

        o_pListOfDefects->push_back(HVEBreakLineDefect(0x00002000, HGF3DCoord<double>(0.0, 0.0)));
        }

    //=========================================================================
    // Validation of breakline itself
    // Check if it auto crosses (mask 0x00000001)
    //=========================================================================
    if (i_ValidationMask & 0x00000001)
        {
        // For each breakline in list
        for (BLIndex = 0 ; BLIndex < m_BreakLineList.size() ; ++BLIndex)
            {
            // Validate this breakline
            if (m_BreakLineList[BLIndex]->AutoCrosses2D())
                {
                // Indicate the breaklines are invalid
                Result = 1;

                o_pListOfDefects->push_back(HVEBreakLineDefect(0x00000001,
                                                               m_BreakLineList[BLIndex]->GetPoint(0),
                                                               BLIndex));
                }
            }
        }


    //=========================================================================
    // Validation of breakline itself
    // Check if double points this is a warning unless a single point ofter
    // double removal
    //=========================================================================
    if (i_ValidationMask & (0x00000004 | 0x00000008))
        {
        // For each breakline in list
        for (BLIndex = 0 ; BLIndex < m_BreakLineList.size() ; ++BLIndex)
            {
            size_t NumPoints = m_BreakLineList[BLIndex]->GetSize();
            size_t i;

            // Make sure that every breakline has at least two points
            if (NumPoints < 2 && (i_ValidationMask & 0x00000008))
                {
                // Indicate error
                Result = 1;

                o_pListOfDefects->push_back(HVEBreakLineDefect(0x00000008,
                                                               m_BreakLineList[BLIndex]->GetPoint(0),
                                                               BLIndex));
                }
            else if (i_ValidationMask & 0x00000004)
                {
                bool DBLPoint = false;

                HGF3DCoord<double> MyPoint2;
                HGF3DCoord<double> MyPoint = m_BreakLineList[BLIndex]->GetPoint(0);
                for (i = 1 ; !DBLPoint && i < NumPoints ; ++i)
                    {
                    MyPoint2 = m_BreakLineList[BLIndex]->GetPoint(i);

                    if (MyPoint.IsEqualTo(MyPoint2, m_BreakLineList[BLIndex]->GetTolerance()))
                        {
                        DBLPoint = true;
                        }

                    // Change current point
                    MyPoint = MyPoint2;
                    }

                if (DBLPoint)
                    {
                    Result = 1;

                    o_pListOfDefects->push_back(HVEBreakLineDefect(0x00000004,
                                                                   MyPoint2,
                                                                   BLIndex));
                    }
                }
            }
        }


    //=========================================================================
    // Validation of breakline itself
    // Check if breaklines are too long or too short
    //=========================================================================
    if (i_ValidationMask & (0x00000010 | 0x00000002))
        {
        // For each breakline in list
        for (BLIndex = 0 ; BLIndex < m_BreakLineList.size() ; ++BLIndex)
            {
            size_t NumPoints = m_BreakLineList[BLIndex]->GetSize();
            size_t i;

            // Make sure that every breakline has at least two points
            if (NumPoints >= 2)
                {
                HGF3DCoord<double> MyPoint2;
                HGF3DCoord<double> MyPoint;
                for (i = 1 ; i < NumPoints ; ++i)
                    {
                    MyPoint = m_BreakLineList[BLIndex]->GetPoint(i-1);
                    MyPoint2 = m_BreakLineList[BLIndex]->GetPoint(i);

                    // Compute length of segment
                    double length = ((MyPoint - MyPoint2).Calculate2DLength());

                    if ((i_ValidationMask & 0x00000010) && (length > 4000))
                        {
                        // Indicate that there is an error
                        Result = 1;

                        o_pListOfDefects->push_back(HVEBreakLineDefect(0x00000010,
                                                                       MyPoint,
                                                                       BLIndex));
                        }
                    if ((i_ValidationMask & 0x00000002) && (length < m_Tolerance))
                        {
                        // Indicate that there is an error
                        Result = 1;

                        // Print message
                        o_pListOfDefects->push_back(HVEBreakLineDefect(0x00000002,
                                                                       MyPoint,
                                                                       BLIndex));
                        }
                    }
                }
            }
        }


    //=========================================================================
    // Validation of breakline itself
    // Check if breaklines are too long or too short
    //=========================================================================
    if (i_ValidationMask & 0x00000020)
        {
        // For each breakline in list
        for (BLIndex = 0 ; BLIndex < m_BreakLineList.size() ; ++BLIndex)
            {
            size_t NumPoints = m_BreakLineList[BLIndex]->GetSize();

            // Make sure that every breakline has at least two points
            if (NumPoints >= 2)
                {
                if (m_BreakLineList[BLIndex]->CalculateLength2D() > 4000)
                    {
                    // Breakline is too long
                    // Indicate that there is an error
                    Result = 1;

                    // Print message
                    o_pListOfDefects->push_back(HVEBreakLineDefect(0x00000020,
                                                                   m_BreakLineList[BLIndex]->GetPoint(0),
                                                                   BLIndex));
                    }
                }
            }
        }



    //=========================================================================
    // Breakline crossing test
    //=========================================================================
    if (i_ValidationMask & 0x00000040)
        {
        // For each breakline in list
        for (BLIndex = 0 ; BLIndex < m_BreakLineList.size() ; ++BLIndex)
            {
            // For each other breaklines not yet processed
            size_t BLIndex2;

            for (BLIndex2 = BLIndex, ++BLIndex2 ; BLIndex2 < m_BreakLineList.size() ; ++BLIndex2)
                {
                // Check if the two line cross
                if (m_BreakLineList[BLIndex]->Crosses2D(*m_BreakLineList[BLIndex2]))
                    {
                    // Indicate the breaklines are invalid
                    Result = 1;

                    // The two lines cross ... obtain cross coordinates
                    HGF3DPointCollection CrossPoints;
                    m_BreakLineList[BLIndex]->Intersect2D(*m_BreakLineList[BLIndex2], &CrossPoints);

                    // Print out message

                    // We only indicate the first cross
                    o_pListOfDefects->push_back(HVEBreakLineDefect(0x00000040,
                                                                   CrossPoints[0],
                                                                   BLIndex));
                    }
                }
            }
        }

    //==========================================================================
    // Breakline connection error
    //===========================================================================
    if (i_ValidationMask & 0x00000080)
        {
        // For each breakline in list
        for (BLIndex = 0 ; BLIndex < m_BreakLineList.size() ; ++BLIndex)
            {
            // For each other breaklines not yet processed
            size_t BLIndex2;

            for (BLIndex2 = 0 ; BLIndex2 < m_BreakLineList.size() ; ++BLIndex2)
                {
                // Check if self
                if (BLIndex != BLIndex2)
                    {
                    // Check if the two line are connected but not linked (flirt)
                    if ((m_BreakLineList[BLIndex]->ConnectsTo2D(*m_BreakLineList[BLIndex2])) &&
                        (!m_BreakLineList[BLIndex]->LinksTo2D(*m_BreakLineList[BLIndex2])))
                        {
                        // We connect upon a line ... this may be valid if connection is upon a segment point
                        // Extract the two extremities
                        HGF3DPoint StartPoint = m_BreakLineList[BLIndex]->GetStartPoint();
                        HGF3DPoint EndPoint = m_BreakLineList[BLIndex]->GetEndPoint();

                        // Check if start point is on other breakline
                        if (m_BreakLineList[BLIndex2]->IsPointOn2D(StartPoint))
                            {
                            // Start point is on the breakline ... check if it is equal to one segment point

                            // Obtain the number of points in breakline 2
                            size_t NumPointsBL2 = m_BreakLineList[BLIndex2]->GetSize();
                            size_t i;
                            HGF3DPoint MyPointBL2;
                            bool InternalLink = false;
                            for (i = 0 ; i < NumPointsBL2 && !InternalLink ; ++i)
                                {
                                MyPointBL2 = m_BreakLineList[BLIndex2]->GetPoint(i);

                                //Check is start point is equal to current point
                                InternalLink = (MyPointBL2.IsEqualTo(StartPoint, m_BreakLineList[BLIndex]->GetTolerance()));
                                }

                            // If the internal link was not detected, then we have an illegal connection
                            if (!InternalLink)
                                {
                                // Indicate the breaklines are invalid
                                Result = 1;

                                o_pListOfDefects->push_back(HVEBreakLineDefect(0x00000080,
                                                                               m_BreakLineList[BLIndex]->GetPoint(0),
                                                                               BLIndex));
                                }
                            }

                        // Check if start point is on other breakline
                        if (m_BreakLineList[BLIndex2]->IsPointOn2D(EndPoint))
                            {
                            // Start point is on the breakline ... check if it is equal to one segment point

                            // Obtain the number of points in breakline 2
                            size_t NumPointsBL2 = m_BreakLineList[BLIndex2]->GetSize();
                            size_t i;
                            HGF3DPoint MyPointBL2;
                            bool InternalLink = false;
                            for (i = 0 ; i < NumPointsBL2 && !InternalLink ; ++i)
                                {
                                MyPointBL2 = m_BreakLineList[BLIndex2]->GetPoint(i);

                                //Check is start point is equal to current point
                                InternalLink = (MyPointBL2.IsEqualTo(EndPoint, m_BreakLineList[BLIndex]->GetTolerance()));
                                }

                            // If the internal link was not detected, then we have an illegal connection
                            if (!InternalLink)
                                {
                                // Indicate the breaklines are invalid
                                Result = 1;

                                o_pListOfDefects->push_back(HVEBreakLineDefect(0x00000080,
                                                                               m_BreakLineList[BLIndex]->GetEndPoint(),
                                                                               BLIndex));
                                }

                            }
                        }
                    }
                }
            }
        }


    //==========================================================================
    // Breaklines comtuguousness error
    //===========================================================================
    if (i_ValidationMask & 0x00000100)
        {
        // For each breakline in list
        for (BLIndex = 0 ; BLIndex < m_BreakLineList.size() ; ++BLIndex)
            {
            // For each other breaklines not yet processed
            size_t BLIndex2;

            for (BLIndex2 = BLIndex, ++BLIndex2 ; BLIndex2 < m_BreakLineList.size() ; ++BLIndex2)
                {
                // Check if the two line are connected but not linked (flirt)
                if (m_BreakLineList[BLIndex]->IsContiguousTo2D(*m_BreakLineList[BLIndex2]))
                    {
                    // Indicate the breaklines are invalid
                    Result = 1;

                    // Obtain contiguousness points
                    HGF3DPointCollection ContPts;

                    // Obtain points
                    if (m_BreakLineList[BLIndex]->GetContiguousPointsTo2D(*m_BreakLineList[BLIndex2], &ContPts) >= 2)
                        {
                        // We indicate the first contiguousness region point as defect location
                        o_pListOfDefects->push_back(HVEBreakLineDefect(0x00000100, ContPts[0], BLIndex));
                        }
                    else
                        {
                        // Some internal error occured... There should be at least 2 point found!
                        // We place the first point of line (Quite and arbitrary choice)
                        o_pListOfDefects->push_back(HVEBreakLineDefect(0x00000100,m_BreakLineList[BLIndex]->GetStartPoint(), BLIndex));
                        }
                    }
                }
            }
        }

    //=========================================================================
    // Validation of point against breaklines
    // Check if a point is on a breakline
    //=========================================================================
    if (i_ValidationMask & 0x00000400)
        {
        // For each breakline in list
        vector<HGF3DPoint, allocator<HGF3DPoint> >::const_iterator PTItr;
        for (BLIndex = 0 ; BLIndex < m_BreakLineList.size() ; ++BLIndex)
            {

            // For every point in list of points
            for (PTItr = m_DTMPoints.begin() ; PTItr != m_DTMPoints.end() ; ++PTItr)
                {

                // Validate this point is not on breakline
                HGF2DPosition Pt1(PTItr->GetX(), PTItr->GetY());

                if ((m_BreakLineList[BLIndex])->IsPointOn2D(*PTItr))
                    {
                    // Indicate the breaklines are invalid
                    Result = 1;

                    o_pListOfDefects->push_back(HVEBreakLineDefect(0x00000400, *PTItr, BLIndex));
                    }
                }
            }
        }


    //==========================================================================
    // Invalid connection
    //===========================================================================
    if (i_ValidationMask & (0x00000200 | 0x00000800))
        {
        // For each breakline in list
        for (BLIndex = 0 ; BLIndex < m_BreakLineList.size() ; ++BLIndex)
            {
            // For each other breaklines not yet processed
            size_t BLIndex2;

            for (BLIndex2 = BLIndex, ++BLIndex2 ; BLIndex2 < m_BreakLineList.size() ; ++BLIndex2)
                {
                // For every point of line 1
                size_t NumPoints1 = m_BreakLineList[BLIndex]->GetSize();
                size_t PtIndex1;
                for (PtIndex1 = 0 ; PtIndex1 < NumPoints1 ; ++PtIndex1)
                    {
                    // Obtain the point for breakline
                    HGF3DPoint MyCurrentPoint1 = m_BreakLineList[BLIndex]->GetPoint(PtIndex1);

                    // Check if current point is located on other breakline
                    if (m_BreakLineList[BLIndex2]->IsPointOn2D(MyCurrentPoint1))
                        {
                        // The point is located upon the other breakline
                        // We need to check if the it flirts on a point of this breakline
                        // For every point of line 1
                        HGF3DPoint MyCurrentPoint2;
                        size_t NumPoints2 = m_BreakLineList[BLIndex2]->GetSize();
                        size_t PtIndex2;
                        bool EqualPoint = false;
                        for (PtIndex2 = 0 ; !EqualPoint && PtIndex2 < NumPoints2 ; ++PtIndex2)
                            {
                            // Obtain the point for breakline
                            MyCurrentPoint2 = m_BreakLineList[BLIndex2]->GetPoint(PtIndex2);

                            // check if the points are equal 2D
                            EqualPoint = MyCurrentPoint1.IsEqualTo2D(MyCurrentPoint2, m_BreakLineList[BLIndex]->GetTolerance());
                            }

                        // Check if an equal point was found
                        if (EqualPoint)
                            {
                            // A point equal in two 2d was obtained ... if this validation is asked for ...
                            if (i_ValidationMask & 0x00000800)
                                {
                                // An equal point was found ... check if they have same elevation
                                if (!MyCurrentPoint1.IsEqualTo(MyCurrentPoint2, m_BreakLineList[BLIndex]->GetTolerance()))
                                    {
                                    // Indicate the breaklines are invalid
                                    Result = 1;

                                    o_pListOfDefects->push_back(HVEBreakLineDefect(0x00000800, MyCurrentPoint1, BLIndex));
                                    }
                                }
                            }
                        else
                            {
                            // A breakline point is on another breakline but not at a vertex
                            if (i_ValidationMask & 0x00000200)
                                {
                                o_pListOfDefects->push_back(HVEBreakLineDefect(0x00000200, MyCurrentPoint1, BLIndex));
                                }
                            }
                        }
                    }
                }
            }
        }


    //=========================================================================
    // Validation of point against other points
    // Check if two points are equal
    //=========================================================================
    if (i_ValidationMask & 0x00001000)
        {
        vector<HGF3DPoint, allocator<HGF3DPoint> >::const_iterator PTItr;

        // For each breakline in list
        for (PTItr = m_DTMPoints.begin() ; PTItr != m_DTMPoints.end() ; ++PTItr)
            {
            vector<HGF3DPoint, allocator<HGF3DPoint> >::const_iterator PTItr2;

            HGF2DPosition Pt1(PTItr->GetX(), PTItr->GetY());

            // For every point in list of points
            for (PTItr2 = PTItr, ++PTItr2 ; PTItr2 != m_DTMPoints.end() ; ++PTItr2)
                {

                // Validate this point is not on breakline
                HGF2DPosition Pt2(PTItr2->GetX(), PTItr2->GetY());

                if (Pt1.IsEqualTo(Pt2))
                    {
                    // Indicate the breaklines are invalid
                    Result = 1;

                    o_pListOfDefects->push_back(HVEBreakLineDefect(0x00001000, Pt1, -1));
                    }
                }
            }
        }


    return(Result);
    }





/** -----------------------------------------------------------------------------
    @author Alain Robert

    This static method pumps the DTM points (Breaklines) from the stream


    @param i_InStream The input stream from which is loaded the DTM points (breaklines)

    @param i_rPoints The DTM point collection to insert loaded points into

    @return Reference to input stream to be used afterwards.
    -----------------------------------------------------------------------------
*/
istream& operator>>(istream& i_InStream, HVEBreakLines& io_rBreakLines)
    {
    // Declare input buffer
    char MyInLine[1024];
    double X;
    double Y;
    double Z;
    int UserFlag;
    long LineNumber = 0;
    int ResSScanf;


    // While the end of file is not reached
    while (i_InStream.good())
        {
        // Input the line from stream
        i_InStream.getline(MyInLine, 1024);
        ++LineNumber;

        // Check if it is a comment
        if (MyInLine[0] != '#' && strlen(MyInLine) > 5)
            {
            // Validate line ...
            ResSScanf = sscanf(MyInLine,"%lf %lf %lf %i", &X, &Y, &Z, &UserFlag);
            if (ResSScanf != 4)
                {
                // the input line is invalid
                char MyString[1024];
                ostrstream MyLineStream(MyString, 1024);
                MyLineStream << "breakline file (line : " << LineNumber << ") " << ends;

                throw HFCFileException(HFC_CORRUPTED_FILE_EXCEPTION, MyString);
                }

            HGF3DPoint MyPoint(X, Y, Z);

            // Check if user flag is start of breakline
            if (UserFlag == 1)
                {
                // It is a start of breaklines

                // For every breaklines following
                while ((UserFlag == 1) && (i_InStream.good()))
                    {
                    // Create polysegment
                    HFCPtr<HVE3DPolyLine> pNewBreakLine = new HVE3DPolyLine(io_rBreakLines.GetCoordSys());

                    // Check if tolerance specified
                    // The EXACT COMPARE IS INTENTIONAL
                    if (io_rBreakLines.m_Tolerance != 0.0)
                        {
                        // Make sure the tolerance is positive
                        HASSERT(io_rBreakLines.m_Tolerance > 0.0);

                        // Desactivate auto-tolerane
                        pNewBreakLine->SetAutoToleranceActive(false);

                        // Set specified tolerance
                        pNewBreakLine->SetTolerance(io_rBreakLines.m_Tolerance);
                        }

                    // Add first point
                    pNewBreakLine->AppendPoint(HGF3DPoint(X, Y, Z));

                    // This line is essential in case comment after first point
                    UserFlag = 2;

                    // For every point of the current breakline
                    do
                        {
                        // Input the line from stream
                        i_InStream.getline(MyInLine, 1024);
                        ++LineNumber;

                        // Check if it is a comment
                        if (MyInLine[0] != '#' && strlen(MyInLine) > 5)
                            {

                            // Validate line ...
                            ResSScanf = sscanf(MyInLine,"%lf %lf %lf %i", &X, &Y, &Z, &UserFlag);
                            if (ResSScanf != 4)
                                {

                                // the input line is invalid
                                char MyString[1024];
                                ostrstream MyLineStream(MyString, 1024);
                                MyLineStream << "breakline file (line : " << LineNumber << ") " << ends;

                                throw HFCFileException(HFC_CORRUPTED_FILE_EXCEPTION, MyString);
                                }

                            if (UserFlag == 2)
                                {
                                // Add next point
                                pNewBreakLine->AppendPoint(HGF3DPoint(X, Y, Z));
                                }
                            }

                        }
                    while((UserFlag == 2) && (i_InStream.good()));

                    // Add the breakline
                    io_rBreakLines.m_BreakLineList.push_back(pNewBreakLine);
                    }

                // Check if if is end of file
                if (UserFlag == 0)
                    {
                    // It is not the end of file ... last point is a plain DTM point to be added

                    HGF3DPoint My3DPoint(X, Y, Z);

                    // Add DTM point to list of points
                    io_rBreakLines.m_DTMPoints.push_back(MyPoint);
                    }
                }
            else
                {
                // Check if the user flag is for a point ... only possibility
                if (UserFlag != 0)
                    {
                    char MyString[1024];
                    ostrstream MyLineStream(MyString, 1024);
                    MyLineStream << "breakline file (line : " << LineNumber << ") " << ends;

                    throw HFCFileException(HFC_CORRUPTED_FILE_EXCEPTION, MyString);
                    }
                else
                    {
                    // Add DTM point to list of points
                    io_rBreakLines.m_DTMPoints.push_back(MyPoint);
                    }
                }
            }
        }

    return(i_InStream);
    }

#if (0)

/** -----------------------------------------------------------------------------
    @author Alain Robert

    This static method pushes the DTM points (Breaklines) to the stream

    @param po_OutStream The output stream to which to write DTM points (breaklines)

    @param i_rPoints The DTM points to write to stream

    @return Reference to output stream to be used afterwards.
    -----------------------------------------------------------------------------
*/
ostream& operator<<(ostream& i_OutStream, const KGIBreakLinesCollection& i_rPoints)
    {

    // Write points
    KGIBreakLinesCollection::const_iterator Itr;

    for (Itr = i_rPoints.begin() ; Itr != i_rPoints.end() ; ++Itr)
        {
        i_OutStream << Itr->GetX() << " "
                    << Itr->GetY() << " "
                    << Itr->GetZ() << " "
                    << Itr->GetUserFlag() << endl;
        }

    return(i_OutStream);
    }

#endif




//-----------------------------------------------------------------------------
// PrintState
// This method dumps the content of the object in the given output stream
// in text format
//-----------------------------------------------------------------------------
void HVEBreakLines::PrintState(ostream& po_rOutput) const
    {
#ifdef __HMR_PRINTSTATE
    char       DumString[256];
    sprintf(DumString, "Object is a HVEBreakLines");
    HDUMP0(DumString);
    HDUMP0("\n");
    po_rOutput << DumString << endl;
#endif
    }











