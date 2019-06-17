/*--------------------------------------------------------------------------*/ 
/*  BoundingBox.h															*/ 
/*	Axis Aligned Bounding Rect class definition								*/ 
/*  (C) 2003 Copyright Pointools Ltd, UK | All Rights Reserved				*/ 
/*																			*/ 
/*	Adapted from OSG::Timer class											*/ 
/*  Last Updated 12 Oct 2003 Faraz Ravi										*/ 
/*--------------------------------------------------------------------------*/ 

#pragma once

namespace pt
    {

    /** A high resolution, low latency time stamper.*/
    class CCLASSES_API Timer
        {

        public:

            typedef uint64_t TimeMs;

            static inline TimeMs tick() { return BeTimeUtilities::QueryMillisecondsCounter(); }

            static inline TimeMs delta_s(TimeMs t1, TimeMs t2) { return delta_m(t1, t2) * 1000; }
            static inline TimeMs delta_m(TimeMs t1, TimeMs t2) { return t2 - t1; }
            //inline double delta_u(TimeMs t1, TimeMs t2 ) const { return delta_s(t1,t2)*1e6; }
           // inline double delta_n(TimeMs t1, TimeMs t2 ) const { return delta_s(t1,t2)*1e9; }
        };
    }

