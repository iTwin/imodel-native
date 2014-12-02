/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/DgnProject/BackDoor/PublicAPI/BackDoor/DgnProject/BackDoor.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

// BackDoor is built with the non-published the DgnPlatform APIs. BackDoor then presents its API to the published unit test code, which is build with the published API. 
// The purpose of BackDoor is to provide the published unit tests with access to selected non-published functions so that they can do necessary set-up or result-checking 
// as part of the testing mechanism, not as part of the API or functionality that is tested.
// NB: Even though it uses the Bentley-internal API, BackDoor must be portable to other platforms.

#include <Bentley/Bentley.h>
//#include <Bentley/CatchNonPortable.h>
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/DgnCore/TextParam.h>
#include <DgnPlatform/DgnHandlers/ScopedDgnHost.h>
#include <Bentley/BeTest.h>

#define BEGIN_DGNDB_UNIT_TESTS_NAMESPACE BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE namespace DgnDbUnitTests {
#define END_DGNDB_UNIT_TESTS_NAMESPACE   } END_BENTLEY_DGNPLATFORM_NAMESPACE
#define USING_DGNDB_UNIT_TESTS_NAMESPACE using namespace BentleyApi::DgnPlatform::DgnDbUnitTests;

BEGIN_DGNDB_UNIT_TESTS_NAMESPACE

struct DgnDbTestDgnManager : TestDgnManager
{
    DgnDbTestDgnManager (WCharCP dgnfilename, CharCP callerSourceFile="", FileOpenMode omode=OPENMODE_READWRITE, DgnInitializeMode imode=DGNINITIALIZEMODE_FillModel, bool forceMakeCopy=false);
    static BeFileName GetUtDatPath (CharCP callerSourceFile);
    static StatusInt FindTestData (BeFileName& fullFileName, WCharCP fileName, CharCP callerSourceFile);
    static StatusInt GetTestDataOut (BeFileName& outFullFileName, WCharCP fileName, WCharCP outName, CharCP callerSourceFile);
    static BeFileName GetWritableTestData (WCharCP fileName, CharCP callerSourceFile);
    static BeFileName GetReadOnlyTestData (WCharCP fileName, CharCP callerSourceFile, bool forceMakeCopy);

#if defined (DGNPLATFORM_HAVE_DGN_IMPORTER)
    static void CreateProjectFromDgn (DgnProjectPtr& project, BeFileName const& projectFileName, BeFileName const& dgnFileName, BeFileName const& refFileName = BeFileName(), bool deleteIfExists = true);
#endif

    static BeFileName GetOutputFilePath (WCharCP fn)
        {
        BeFileName projectFileName;
        BeTest::GetHost().GetOutputRoot (projectFileName);
        projectFileName.AppendToPath (fn);
        return projectFileName;
        }

    static BeFileName GetSeedFilePath (WCharCP fn)
        {
        BeFileName fullFileName;
        FindTestData (fullFileName, fn, __FILE__);
        return fullFileName;
        }
};

namespace BackDoor
{
/*---------------------------------------------------------------------------------**//**
* @bsinamespace                                                 KevinNyman      07/09
+---------------+---------------+---------------+---------------+---------------+------*/
    namespace DirectionParser 
    {
        void SetTrueNorth (DgnPlatform::DirectionParser& parser, double const& trueNorth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsinamespace                                                 KevinNyman      07/09
+---------------+---------------+---------------+---------------+---------------+------*/
    namespace ArcHandler
    {
        BentleyStatus Extract
            (
            DPoint3dP       start_pt,
            DPoint3dP       end_pt,
            double*         start,
            double*         sweep,
            double*         x1,
            double*         x2,
            RotMatrixP      trans,
            double*         rot,
            DPoint3dP       center,
            ElementHandleCR    eh
            );
    };

/*---------------------------------------------------------------------------------**//**
* @bsinamespace                                                 KevinNyman      07/09
+---------------+---------------+---------------+---------------+---------------+------*/
    namespace EllipseHandler
    {
        BentleyStatus   Extract
            (
            DPoint3dP       sf_pt,
            double*         x1,
            double*         x2,
            RotMatrixP      trans,
            double*         rot,
            DPoint3dP       center,
            ElementHandleCR    eh
            );
    };

/*---------------------------------------------------------------------------------**//**
* @bsinamespace                                                 KevinNyman      07/09
+---------------+---------------+---------------+---------------+---------------+------*/
    namespace MeshHeaderHandler
    {
        /*-------------------------------------------------------------------------**//**
        * @bsimethod                                            KevinNyman      07/09
        +-------+---------------+---------------+---------------+---------------+------*/
        BentleyStatus PolyfaceFromElement (PolyfaceHeaderPtr & pr, ElementHandleCR eh);
    };


/*---------------------------------------------------------------------------------**//**
* @bsinamespace                                                 KevinNyman      11/09
+---------------+---------------+---------------+---------------+---------------+------*/
    namespace DgnModel
    {
        /*-------------------------------------------------------------------------**//**
        * @bsimethod                                            KevinNyman      11/09
        +-------+---------------+---------------+---------------+---------------+------*/
        DgnModelType GetModelType (DgnModelCR model);

        /*-------------------------------------------------------------------------**//**
        * @bsimethod                                            KevinNyman      08/10
        +-------+---------------+---------------+---------------+---------------+------*/
        void SetReadOnly (DgnPlatform::DgnModel& model, bool isReadOnly);

        /*-------------------------------------------------------------------------**//**
        * @bsimethod                                            KevinNyman      11/09
        +-------+---------------+---------------+---------------+---------------+------*/
        PersistentElementRefP FindByElementId (DgnModelR model, DgnPlatform::ElementId id);

        /*-------------------------------------------------------------------------**//**
        * @bsimethod                                            KevinNyman      11/09
        +-------+---------------+---------------+---------------+---------------+------*/
        DgnPlatform::DgnModelId GetModelId (DgnModelR model);

    };

/*---------------------------------------------------------------------------------**//**
* @bsinamespace                                                 KevinNyman      07/09
+---------------+---------------+---------------+---------------+---------------+------*/
  namespace SurfaceOrSolidHandler
    {
        /*-------------------------------------------------------------------------**//**
        * @bsimethod                                            KevinNyman      07/09
        +-------+---------------+---------------+---------------+---------------+------*/
        bool IsSurfaceOfProjection (ElementHandleCR eh, bool* isCapped);

        /*-------------------------------------------------------------------------**//**
        * @bsimethod                                            KevinNyman      07/09
        +-------+---------------+---------------+---------------+---------------+------*/
        bool IsSurfaceOfRevolution (ElementHandleCR eh, bool* isCapped);

        /*-------------------------------------------------------------------------**//**
        * @bsimethod                                            KevinNyman      07/09
        +-------+---------------+---------------+---------------+---------------+------*/
        BentleyStatus  ExtractProjectionParameters (ElementHandleCR eh,
                                                    EditElementHandleR profileEeh,
                                                    DVec3dR direction,
                                                    double& distance,
                                                    bool ignoreSkew);

        /*-------------------------------------------------------------------------**//**
        * @bsimethod                                            KevinNyman      07/09
        +-------+---------------+---------------+---------------+---------------+------*/
        BentleyStatus  ExtractRevolutionParameters (ElementHandleCR eh,
                                                    EditElementHandleR profileEeh,
                                                    DPoint3dR center,
                                                    DVec3dR axis,
                                                    double& sweep);

        /*-------------------------------------------------------------------------**//**
        * @bsimethod                                            KevinNyman      07/09
        +-------+---------------+---------------+---------------+---------------+------*/
        BentleyStatus  ExtractProfiles (ElementHandleCR eh, ElementAgendaR profiles);
    };

/*---------------------------------------------------------------------------------**//**
* @bsinamespace                                                 KevinNyman      07/09
+---------------+---------------+---------------+---------------+---------------+------*/
    namespace ConeHandler
    {
        /*-------------------------------------------------------------------------**//**
        * @bsimethod                                            KevinNyman      07/09
        +-------+---------------+---------------+---------------+---------------+------*/
        double ExtractBottomRadius (ElementHandleCR eh);

        /*-------------------------------------------------------------------------**//**
        * @bsimethod                                            KevinNyman      07/09
        +-------+---------------+---------------+---------------+---------------+------*/
        double ExtractTopRadius (ElementHandleCR eh);

        /*-------------------------------------------------------------------------**//**
        * @bsimethod                                            KevinNyman      07/09
        +-------+---------------+---------------+---------------+---------------+------*/
        DPoint3dCP ExtractBottomCenter (ElementHandleCR eh);

        /*-------------------------------------------------------------------------**//**
        * @bsimethod                                            KevinNyman      07/09
        +-------+---------------+---------------+---------------+---------------+------*/
        DPoint3dCP ExtractTopCenter (ElementHandleCR eh);

        /*-------------------------------------------------------------------------**//**
        * @bsimethod                                            KevinNyman      07/09
        +-------+---------------+---------------+---------------+---------------+------*/
        bool ExtractCapFlag (ElementHandleCR eh);

        /*-------------------------------------------------------------------------**//**
        * @bsimethod                                            KevinNyman      07/09
        +-------+---------------+---------------+---------------+---------------+------*/
        void ExtractRotation (RotMatrixR rMatrix, ElementHandleCR eh);
    };

    /*---------------------------------------------------------------------------------**//**
    * @bsinamespace                                                 Jeff.Marker     10/2009
    +---------------+---------------+---------------+---------------+---------------+------*/
    namespace TextBlock
    {
        /*-------------------------------------------------------------------------**//**
        * @bsimethod                                            Jeff.Marker     10/2009
        +-------+---------------+---------------+---------------+---------------+------*/
        bool TextBlock_EqualsWithCompareContentAndLocation (TextBlockCR lhs, TextBlockCR rhs);

        /*-------------------------------------------------------------------------**//**
        * @bsimethod                                            Jeff.Marker     10/2009
        +-------+---------------+---------------+---------------+---------------+------*/
        bool TextBlockProperties_Equals (TextBlockPropertiesCR lhs, TextBlockPropertiesCR rhs);

        /*-------------------------------------------------------------------------**//**
        * @bsimethod                                            Jeff.Marker     10/2009
        +-------+---------------+---------------+---------------+---------------+------*/
        bool ParagraphProperties_Equals (ParagraphPropertiesCR lhs, ParagraphPropertiesCR rhs);

        /*-------------------------------------------------------------------------**//**
        * @bsimethod                                            Jeff.Marker     10/2009
        +-------+---------------+---------------+---------------+---------------+------*/
        bool RunProperties_Equals (RunPropertiesCR lhs, RunPropertiesCR rhs);

        /*-------------------------------------------------------------------------**//**
        * @bsimethod                                            Jeff.Marker     12/2009
        +-------+---------------+---------------+---------------+---------------+------*/
        void TextBlock_SetNodeProperties (TextBlockR textBlock, RunPropertiesCR runProps);

        /*-------------------------------------------------------------------------**//**
        * @bsimethod                                            Jeff.Marker     12/2009
        +-------+---------------+---------------+---------------+---------------+------*/
        bool TextBlock_DoesStartFieldXAttributeExistOnElement (ElementHandle);

        /*-------------------------------------------------------------------------**//**
        * @bsimethod                                            Jeff.Marker     12/2009
        +-------+---------------+---------------+---------------+---------------+------*/
        bool TextBlock_DoesEndFieldLinkageExistOnElement (ElementHandle);

    }; // TextBlock

    /*---------------------------------------------------------------------------------**//**
    * @bsinamespace                                                 JoshSchifter    11/09
    +---------------+---------------+---------------+---------------+---------------+------*/
    namespace NonVisibleViewport
    {
        /*-------------------------------------------------------------------------**//**
        * @bsimethod                                            JoshSchifter    11/09
        +-------+---------------+---------------+---------------+---------------+------*/
        ViewportP Create (ViewControllerR viewInfo);

        /*-------------------------------------------------------------------------**//**
        * @bsimethod                                            JoshSchifter    11/09
        +-------+---------------+---------------+---------------+---------------+------*/
        void Delete (ViewportP viewport);

    }; // NonVisibleViewport
    
    /*-------------------------------------------------------------------------**//**
    * @bsinamespace                                                 KevinNyman      07/09
    +---------------+---------------+---------------+---------------+---------------+------*/
    namespace PolyfaceQueryP
    {
        /*-------------------------------------------------------------------------**//**
        * @bsimethod                                            KevinNyman      07/09
        +-------+---------------+---------------+---------------+---------------+------*/
        double SumTetrahedralVolumes (BentleyApi::PolyfaceQueryP p, DPoint3dCR origin);
    };
}

END_DGNDB_UNIT_TESTS_NAMESPACE
