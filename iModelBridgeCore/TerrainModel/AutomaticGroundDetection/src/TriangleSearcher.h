/*--------------------------------------------------------------------------------------+
|
|     $Source: AutomaticGroundDetection/src/TriangleSearcher.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma  once

BEGIN_GROUND_DETECTION_NAMESPACE

struct TriangleSearcher : public RefCountedBase
    {
    private: 

        bvector<DPoint3d> m_points;

       TriangleSearcher();
        ~TriangleSearcher();


    public:

        static TriangleSearcherPtr Create();
               
        void AddTriangle(const DPoint3d& point);

        
    
    };


END_GROUND_DETECTION_NAMESPACE
