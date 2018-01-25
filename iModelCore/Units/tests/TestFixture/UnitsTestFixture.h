/*--------------------------------------------------------------------------------------+
|
|  $Source: tests/TestFixture/UnitsTestFixture.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "../UnitsTestsPch.h"

BEGIN_UNITS_UNITTESTS_NAMESPACE

//=======================================================================================    
// @bsiclass                                                 Basanta.Kharel     12/2015
//=======================================================================================    
struct UnitsTestFixture : public ::testing::Test
    {
    public:
        using CSVLineProcessor = std::function<void(bvector<Utf8String>&)>;

        static Utf8String GetConversionDataPath(WCharCP dataFile);
        static Utf8String GetOutputDataPath(WCharCP dataFile);

        static void WriteLine(BeFile& file, Utf8CP line = nullptr);
        static void WriteToFile(Utf8CP fileName, bvector<bpair<Utf8String, Utf8String>> lines);

        static void ReadConversionCsvFile(WCharCP file, CSVLineProcessor lineProcessor);

        static Utf8String ParseUOM(Utf8CP unitName, bset<Utf8String>& notMapped);
        static UnitCP LocateUOM(Utf8CP unitName, bool useLegacyNames);

        template<class T> typename std::enable_if<!std::numeric_limits<T>::is_integer, bool>::type
        static almost_equal(const T x, const T y, int ulp)
            {
            // the machine epsilon has to be scaled to the magnitude of the values used
            // and multiplied by the desired precision in ULPs (units in the last place)
            return fabs(x - y) < std::numeric_limits<T>::epsilon() * fabs(x + y) * ulp
                // unless the result is subnormal
                || fabs(x - y) < std::numeric_limits<T>::min();
            }
    };

END_UNITS_UNITTESTS_NAMESPACE