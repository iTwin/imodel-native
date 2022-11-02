//:>--------------------------------------------------------------------------------------+
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See LICENSE.md in the repository root for full copyright notice.
//:>+--------------------------------------------------------------------------------------

#pragma once
#include <Bentley/bvector.h>

class GeoCoordTestCommon
    {
    public:
        static bool Initialize(bool doNotUseAllEarth = false);
        static void Shutdown();
        
        static const Utf8String& InitializedLibraryPath();

        static const bvector<Utf8String>& GetListOfGCS();
        static const bvector<Utf8String>& GetSmallListOfGCS();
        static const bvector<Utf8String>& GetRepresentativeListOfGCS();
        

        static const bvector<Utf8String>& GetListOfDatums();
        static const bvector<Utf8String>& GetListOfUnsupportedDatums();
        static const bvector<Utf8String>& GetListOfDatumsWithAdditionalPaths();
        
        static bool doubleSame(double val1, double val2) {
            double denom = fabs(val1) + fabs(val2);

            if (denom == 0)
                return true;
        
            double scaledDiff = fabs((val1 - val2) / denom);
        
            if (scaledDiff < 1e-11)
                return true;
        
            // This may mean that values are near 0 in that case the scale difference can be 1.0 (example 0.0 and 1e-30 results in a scale diff of 1.0)
            if (denom < 1e-11)
                return (fabs(val1 - val2) <= 1e-11);
        
            return false;
        }
    };