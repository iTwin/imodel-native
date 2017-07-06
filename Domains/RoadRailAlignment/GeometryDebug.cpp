/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryDebug.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <RoadRailAlignmentInternal.h>

#if defined(_WIN32)
// for debugging with VS. Might have to check for Windows platform at compile time if we go cross platform
#include <Windows.h>
#endif

// initialization of debug-mutable class static ..
int GeometryDebug::s_debug = 0;

bvector<bpair<CurveVectorPtr, Dgn::ColorDef>> GeometryDebug::m_debugCurves;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz                   04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryDebug::Announce (CurveVectorCR cv, char const *name)
    {
#ifndef NDEBUG
    if (s_debug >= 10)
        {
        Utf8String s;
        CurveVectorPtr a (const_cast <CurveVectorP>(&cv));
        auto g = IGeometry::Create (a);
        BentleyGeometryJson::TryGeometryToJsonString (s, *g, true);
        printf ("\n%s\n", s.c_str ());
#if defined(_WIN32)
        OutputDebugString(s.c_str());
#endif
        }
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz                             04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryDebug::Announce (CurveVectorWithDistanceIndexCR path, char const *name)
    {
#ifndef NDEBUG
    if (s_debug > 0)
        {
        double d = path.TotalPathLength ();
        auto cv = path.GetCurveVector ();
        printf ("\n** %s  (L %.17g) (# %d)\n", name, d, (int)cv->size ());
        if (s_debug >= 5)
            {
            for (auto cp : *cv)
                {
                double a;
                cp->Length (a);
                printf (" %2d (L %.17g)\n", cp->GetCurvePrimitiveType (), a);
                }
            }
        Utf8String curves("Curves");
        Announce (*cv, curves.c_str());
        }
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz                             04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryDebug::Announce (bvector<PathLocationDetailPair> const &pathAIntervals, bvector<PathLocationDetailPair> const &pathBIntervals, char const * name)
    {
#ifndef NDEBUG
    if (s_debug > 0)
        {
        printf ("\n %s\n", name);
        for (size_t i = 0; i < pathAIntervals.size (); i++)
            {
            printf (" %d (A %d (f %.5g) %.17g ... %d (f %.5g) %.17g) (B %d (f %.5g) %.17g .. %d (f %.5g) %.17g)\n",
                (int)pathAIntervals[i].GetTagA (),

                (int)pathAIntervals[i].DetailA ().PathIndex (),
                pathAIntervals[i].DetailA ().CurveFraction (),
                pathAIntervals[i].DetailA ().DistanceFromPathStart (),
                (int)pathAIntervals[i].DetailB ().PathIndex (),
                pathAIntervals[i].DetailB ().CurveFraction (),
                pathAIntervals[i].DetailB ().DistanceFromPathStart (),

                (int)pathBIntervals[i].DetailA ().PathIndex (),
                pathBIntervals[i].DetailA ().DistanceFromPathStart (),
                pathBIntervals[i].DetailB ().CurveFraction (),
                (int)pathBIntervals[i].DetailB ().PathIndex (),
                pathBIntervals[i].DetailB ().DistanceFromPathStart (),
                pathBIntervals[i].DetailB ().CurveFraction ()
                );
            }
        }
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sandy.Bugai                      06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryDebug::ClearDebugCurves()
    { 
    m_debugCurves.clear(); 
    }