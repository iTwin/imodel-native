/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/BeTest.h>
#include <ORDBridge/ORDBridgeApi.h>
#include <LinearReferencing/LinearReferencingApi.h>
#include <RoadRailAlignment/RoadRailAlignmentApi.h>
#include <RoadRailPhysical/RoadRailPhysicalApi.h>
#include <ORDBridge/PublishORDToBimDLL.h>

struct ORDBridgeTestsHostImpl;

//=======================================================================================
//! A DgnPlatformLib host that can be used with "Published" tests
//=======================================================================================
struct ORDBridgeTestsHost : BentleyM0200::Dgn::DgnPlatformLib::Host
    {
    friend struct CiviliModelBridgesORDBridgeTestsFixture;
    DEFINE_T_SUPER(BentleyM0200::Dgn::DgnPlatformLib::Host)

    private:
        ORDBridgeTestsHostImpl* m_pimpl;

        void CleanOutputDirectory();

    public:
        ORDBridgeTestsHost();
        ~ORDBridgeTestsHost();

        BeFileName GetTestAppProductDirectory();
        BeFileName GetOutputDirectory();
        BeFileName GetDgnPlatformAssetsDirectory();
        WString* GetInputFileArgument(BeFileName inputPath, WCharCP input);
        WString* GetOutputFileArgument(BeFileName outputPath, WCharCP bimFileName);
        BeFileName BuildProjectFileName(WCharCP);
        virtual Dgn::DgnPlatformLib::Host::IKnownLocationsAdmin& _SupplyIKnownLocationsAdmin() override;
        virtual void _SupplyProductName(Utf8StringR) override;
        virtual BeSQLite::L10N::SqlangFiles _SupplySqlangFiles() override;
    };

//=======================================================================================
// Fixture class to ensure that the host is initialized at the beginning of every test
//=======================================================================================
struct CiviliModelBridgesORDBridgeTestsFixture : ::testing::Test
    {
    private:
        static ORDBridgeTestsHost* m_host;

    protected:
        //! Called before running all tests
        static void SetUpTestCase();
        //! Called after running all tests
        static void TearDownTestCase();

        //! Called before each test
        void SetUp()
            {
            }
        //! Called after each test
        void TearDown()
            {
            }

        static bool TestFileName(Utf8CP source);
        static bool CopyTestFile(Utf8CP source, Utf8CP target);
        static bool RunTestApp(WCharCP input, WCharCP bimFileName, bool updateMode);
        static bool RunTestAppFullLocalPath(WCharCP inputFullLocalPath, WCharCP bimFileName, bool updateMode);
        static Dgn::DgnDbPtr VerifyConvertedElementCount(Utf8CP bimFileName, size_t alignmentCount, size_t corridorCount);
        static Dgn::DgnDbPtr VerifyConvertedGeometryUniqueAlignmentNameExists(Utf8CP bimFileName, Utf8CP alignmentName);
        static Dgn::DgnDbPtr VerifyConvertedGeometryTurnoutBranchCount(Utf8CP bimFileName, Utf8CP branchName, size_t branchCount);
        static Dgn::DgnDbPtr VerifyConvertedGeometrySpiralTypesAndLengths(Utf8CP bimFileName, Utf8CP alignmentName, int spiralType, double spiralLength);
        static Dgn::DgnDbPtr VerifyConvertedGeometryStationStart(Utf8CP bimFileName, Utf8CP alignmentName, double startingStation, double startingDistance);
        static Dgn::DgnDbPtr VerifyConvertedGeometryStationEquation(Utf8CP bimFileName, Utf8CP alignmentName, double startingStation, double startingDistance, int eqnCount, double eqnDistanceAlong, double eqnStationAhead);
        static Dgn::DgnDbPtr VerifyConvertedElementItemTypes(Utf8CP bimFileName, Utf8CP alignmentName, Utf8CP itemTypeLibName, Utf8CP typeClassName, size_t typePropCount, Utf8CP typePropName, Utf8CP typePropStringValue, int typePropIntegerValue, double typePropDoubleValue);

        static Dgn::DgnDbPtr VerifyConvertedGeometryElementCountAndEnds
        (
            Utf8CP bimFileName,
            Utf8CP alignmentName,
            int hElementCount,
            BentleyM0200::DPoint3dCR hBeg,
            BentleyM0200::DPoint3dCR hEnd,
            int vElementCount,
            BentleyM0200::DPoint3dCR vBeg,
            BentleyM0200::DPoint3dCR vEnd
        );

        static Dgn::DgnDbPtr VerifyConvertedGeometryElementLengths
        (
            Utf8CP bimFileName,
            Utf8CP alignmentName,
            bool checkExactValues,
            double hLineLength,
            double hArcLength,
            double hArcRadius,
            double hSpiralLength,
            double vLineLength,
            double vArcLength,
            double vParabolaLength
        );

    public:
    };

typedef CiviliModelBridgesORDBridgeTestsFixture CiviliModelBridgesORDBridgeTests;

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_LINEARREFERENCING
USING_NAMESPACE_BENTLEY_ROADRAILALIGNMENT
