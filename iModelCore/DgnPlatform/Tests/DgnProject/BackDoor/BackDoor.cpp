/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/DgnProject/BackDoor/BackDoor.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <UnitTests/BackDoor/DgnProject/BackDoor.h>

#include <Logging/bentleylogging.h>

USING_NAMESPACE_DGNV8
USING_NAMESPACE_FOREIGNFORMAT

BEGIN_DGNDB_UNIT_TESTS_NAMESPACE

namespace BackDoor
{

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     05/11
+---------------+---------------+---------------+---------------+---------------+------*/
namespace DirectionParser 
{
    void SetTrueNorthValue (DgnPlatform::DirectionParser& parser, double const& trueNorth)
        {
        parser.SetTrueNorthValue (trueNorth);
        }
}

#if defined (NEEDS_WORK_DGNITEM)
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
            )
                {
                return DgnPlatform::ArcHandler::Extract (start_pt, end_pt, start, sweep, x1, x2, trans, rot, center, eh);
                }
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
            )
                {
                return DgnPlatform::EllipseHandler::Extract (sf_pt, x1, x2, trans, rot, center, eh);
                }
    };

/*---------------------------------------------------------------------------------**//**
* @bsinamespace                                                 KevinNyman      07/09
+---------------+---------------+---------------+---------------+---------------+------*/
    namespace MeshHeaderHandler
    {
        /*-------------------------------------------------------------------------**//**
        * @bsimethod                                            KevinNyman      07/09
        +-------+---------------+---------------+---------------+---------------+------*/
        BentleyStatus PolyfaceFromElement (PolyfaceHeaderPtr & pr, ElementHandleCR eh)
            {
            return DgnPlatform::MeshHeaderHandler::PolyfaceFromElement (pr, eh);
            }
    };
#endif


/*---------------------------------------------------------------------------------**//**
* @bsinamespace                                                 KevinNyman      11/09
+---------------+---------------+---------------+---------------+---------------+------*/
    namespace DgnModel
    {
        /*-------------------------------------------------------------------------**//**
        * @bsimethod                                            KevinNyman      11/09
        +-------+---------------+---------------+---------------+---------------+------*/
        DgnModelType GetModelType (DgnModelCR model)
            {
            return model.GetModelType ();
            }

        /*-------------------------------------------------------------------------**//**
        * @bsimethod                                            KevinNyman      11/09
        +-------+---------------+---------------+---------------+---------------+------*/
        DgnPlatform::DgnModelId GetModelId (DgnModelR model)
            {
            return model.GetModelId();
            }

    };

#if defined (NEEDS_WORK_DGNITEM)
/*---------------------------------------------------------------------------------**//**
* @bsinamespace                                                 KevinNyman      07/09
+---------------+---------------+---------------+---------------+---------------+------*/
  namespace SurfaceOrSolidHandler
    {
        /*-------------------------------------------------------------------------**//**
        * @bsimethod                                            KevinNyman      07/09
        +-------+---------------+---------------+---------------+---------------+------*/
        bool IsSurfaceOfProjection (ElementHandleCR eh, bool* isCapped)
            {
            return DgnPlatform::SurfaceOrSolidHandler::IsSurfaceOfProjection (eh, isCapped);
            }

        /*-------------------------------------------------------------------------**//**
        * @bsimethod                                            KevinNyman      07/09
        +-------+---------------+---------------+---------------+---------------+------*/
        bool IsSurfaceOfRevolution (ElementHandleCR eh, bool* isCapped)
            {
            return DgnPlatform::SurfaceOrSolidHandler::IsSurfaceOfRevolution (eh, isCapped);
            }

        /*-------------------------------------------------------------------------**//**
        * @bsimethod                                            KevinNyman      07/09
        +-------+---------------+---------------+---------------+---------------+------*/
        BentleyStatus  ExtractProjectionParameters (ElementHandleCR eh,
                                                    EditElementHandleR profileEeh,
                                                    DVec3dR direction,
                                                    double& distance,
                                                    bool ignoreSkew)
            {
            return DgnPlatform::SurfaceOrSolidHandler::ExtractProjectionParameters (eh,
                                                                                        profileEeh,
                                                                                        direction,
                                                                                        distance,
                                                                                        ignoreSkew);
            }

        /*-------------------------------------------------------------------------**//**
        * @bsimethod                                            KevinNyman      07/09
        +-------+---------------+---------------+---------------+---------------+------*/
        BentleyStatus  ExtractRevolutionParameters (ElementHandleCR eh,
                                                    EditElementHandleR profileEeh,
                                                    DPoint3dR center,
                                                    DVec3dR axis,
                                                    double& sweep)
            {
            return DgnPlatform::SurfaceOrSolidHandler::ExtractRevolutionParameters (eh,
                                                                                            profileEeh,
                                                                                            center,
                                                                                            axis,
                                                                                            sweep);
            }

        /*-------------------------------------------------------------------------**//**
        * @bsimethod                                            KevinNyman      07/09
        +-------+---------------+---------------+---------------+---------------+------*/
        BentleyStatus  ExtractProfiles (ElementHandleCR eh, ElementAgendaR profiles)
            {
            return DgnPlatform::SurfaceOrSolidHandler::ExtractProfiles (eh, profiles);
            }
    };

/*---------------------------------------------------------------------------------**//**
* @bsinamespace                                                 KevinNyman      07/09
+---------------+---------------+---------------+---------------+---------------+------*/
    namespace ConeHandler
    {
        DgnPlatform::ConeHandler* GetHandler(ElementHandleCR eh)
            {
            return dynamic_cast<DgnPlatform::ConeHandler*>(&eh.GetHandler());
            }
        /*-------------------------------------------------------------------------**//**
        * @bsimethod                                            KevinNyman      07/09
        +-------+---------------+---------------+---------------+---------------+------*/
        double ExtractBottomRadius (ElementHandleCR eh)
            {
            return GetHandler(eh)->ExtractBottomRadius (eh);
            }

        /*-------------------------------------------------------------------------**//**
        * @bsimethod                                            KevinNyman      07/09
        +-------+---------------+---------------+---------------+---------------+------*/
        double ExtractTopRadius (ElementHandleCR eh)
            {
            return GetHandler(eh)->ExtractTopRadius (eh);
            }

        /*-------------------------------------------------------------------------**//**
        * @bsimethod                                            KevinNyman      07/09
        +-------+---------------+---------------+---------------+---------------+------*/
        DPoint3dCP ExtractBottomCenter (ElementHandleCR eh)
            {
            return GetHandler(eh)->ExtractBottomCenter (eh);
            }

        /*-------------------------------------------------------------------------**//**
        * @bsimethod                                            KevinNyman      07/09
        +-------+---------------+---------------+---------------+---------------+------*/
        DPoint3dCP ExtractTopCenter (ElementHandleCR eh)
            {
            return GetHandler(eh)->ExtractTopCenter (eh);
            }

        /*-------------------------------------------------------------------------**//**
        * @bsimethod                                            KevinNyman      07/09
        +-------+---------------+---------------+---------------+---------------+------*/
        bool ExtractCapFlag (ElementHandleCR eh)
            {
            return GetHandler(eh)->ExtractCapFlag (eh);
            }

        /*-------------------------------------------------------------------------**//**
        * @bsimethod                                            KevinNyman      07/09
        +-------+---------------+---------------+---------------+---------------+------*/
        void ExtractRotation (RotMatrixR rMatrix, ElementHandleCR eh)
            {
            return GetHandler(eh)->ExtractRotation (rMatrix, eh);
            }
    };

#endif

    /*---------------------------------------------------------------------------------**//**
    * @bsinamespace                                                 Jeff.Marker     10/2009
    +---------------+---------------+---------------+---------------+---------------+------*/
    namespace TextBlock
    {
        /*-------------------------------------------------------------------------**//**
        * @bsimethod                                            Jeff.Marker     10/2009
        +-------+---------------+---------------+---------------+---------------+------*/
        bool TextBlock_EqualsWithCompareContentAndLocation (TextBlockCR lhs, TextBlockCR rhs)
            {
            return lhs.Equals (rhs, *TextBlockCompareOptions::CreateForCompareContentAndLocation ());
            }

        /*-------------------------------------------------------------------------**//**
        * @bsimethod                                            Jeff.Marker     10/2009
        +-------+---------------+---------------+---------------+---------------+------*/
        bool TextBlockProperties_Equals (TextBlockPropertiesCR lhs, TextBlockPropertiesCR rhs)
            {
            return lhs.Equals (rhs);
            }

        /*-------------------------------------------------------------------------**//**
        * @bsimethod                                            Jeff.Marker     10/2009
        +-------+---------------+---------------+---------------+---------------+------*/
        bool ParagraphProperties_Equals (ParagraphPropertiesCR lhs, ParagraphPropertiesCR rhs)
            {
            return lhs.Equals (rhs);
            }

        /*-------------------------------------------------------------------------**//**
        * @bsimethod                                            Jeff.Marker     10/2009
        +-------+---------------+---------------+---------------+---------------+------*/
        bool RunProperties_Equals (RunPropertiesCR lhs, RunPropertiesCR rhs)
            {
            return lhs.Equals (rhs);
            }

        /*-------------------------------------------------------------------------**//**
        * @bsimethod                                            Jeff.Marker     12/2009
        +-------+---------------+---------------+---------------+---------------+------*/
        void TextBlock_SetNodeProperties (TextBlockR textBlock, RunPropertiesCR runProps)
            {
            textBlock.SetTextNodeProperties (&runProps);
            }

 #if defined (NEEDS_WORK_DGNITEM)
        /*-------------------------------------------------------------------------**//**
        * @bsimethod                                            Jeff.Marker     12/2010
        +-------+---------------+---------------+---------------+---------------+------*/
        bool TextBlock_DoesStartFieldXAttributeExistOnElement (ElementHandle eh)
            {
            return ElementHandle::XAttributeIter (eh, XAttributeHandlerId (XATTRIBUTEID_ECField, FIELD_XAttrId), 0).IsValid ();
            }
        
       /*-------------------------------------------------------------------------**//**
        * @bsimethod                                            Jeff.Marker     12/2010
        +-------+---------------+---------------+---------------+---------------+------*/
        bool TextBlock_DoesEndFieldLinkageExistOnElement (ElementHandle eh)
            {
            UShort  endFieldLinkageId           = STRING_LINKAGE_KEY_EndField;
            WChar endFieldLinkageBuffer[4];   memset (endFieldLinkageBuffer, 0, sizeof (endFieldLinkageBuffer));
            
            return (
                (SUCCESS == LinkageUtil::ExtractStringLinkageByIndex (&endFieldLinkageId, endFieldLinkageBuffer, _countof (endFieldLinkageBuffer), 0, eh.GetElementCP ()))
                && (0 == wcscmp (L"EF", endFieldLinkageBuffer))
                );
            }
#endif

    }; // TextBlock

    /*---------------------------------------------------------------------------------**//**
    * @bsinamespace                                                 JoshSchifter    11/09
    +---------------+---------------+---------------+---------------+---------------+------*/
    namespace NonVisibleViewport
    {
        /*-------------------------------------------------------------------------**//**
        * @bsimethod                                            JoshSchifter    11/09
        +-------+---------------+---------------+---------------+---------------+------*/
        ViewportP Create (ViewControllerR viewInfo)
            {
            return new DgnPlatform::NonVisibleViewport(viewInfo);
            }

        /*-------------------------------------------------------------------------**//**
        * @bsimethod                                            JoshSchifter    11/09
        +-------+---------------+---------------+---------------+---------------+------*/
        void Delete (ViewportP viewport)
            {
            delete viewport;
            }

    }; // NonVisibleViewport

    /*---------------------------------------------------------------------------------**//**
    * @bsinamespace                                                 KevinNyman      07/09
    +---------------+---------------+---------------+---------------+---------------+------*/
    namespace PolyfaceQueryP
    {
        /*-------------------------------------------------------------------------**//**
        * @bsimethod                                            KevinNyman      07/09
        +-------+---------------+---------------+---------------+---------------+------*/
        double SumTetrahedralVolumes (BentleyApi::PolyfaceQueryP p, DPoint3dCR origin)
            {
            return p->SumTetrahedralVolumes (origin);
            }
    };



}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt DgnDbTestDgnManager::FindTestData (BeFileName& fullFileName, WCharCP fileName, CharCP callerSourceFile)
    {
    return TestDataManager::FindTestData (fullFileName, fileName, GetUtDatPath(callerSourceFile));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Majd.Uddin                      05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt DgnDbTestDgnManager::GetTestDataOut (BeFileName& outFullFileName, WCharCP fileName, WCharCP outName, CharCP callerSourceFile)
    {
    BeFileName fullFileName;
    if (TestDataManager::FindTestData (fullFileName, fileName, GetUtDatPath(callerSourceFile)) != SUCCESS)
        return ERROR;
    //Copy this to out path and then set that path
    BeTest::GetHost().GetOutputRoot (outFullFileName);
    outFullFileName.AppendToPath (outName);
    BeFileName::CreateNewDirectory (BeFileName::GetDirectoryName(outFullFileName).c_str());
    if (BeFileName::BeCopyFile (fullFileName, outFullFileName) != BeFileNameStatus::Success)
        return ERROR;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName DgnDbTestDgnManager::GetUtDatPath (CharCP callerSourceFile)
    {
    if (NULL == callerSourceFile || !*callerSourceFile)
        return BeFileName (L"DgnDb");   // just look in the docs root

    // start looking in the sub-directory that is specific to this UT. (Then look in each parent directory in turn.)
    BeFileName path (L"DgnDb");
    path.AppendToPath (L"Published");           // *** NB: This must agree with the directory structure of the DgnPlatformTest/DgnDbUnitTests repository
    path.AppendToPath (L"DgnHandlers");
    path.AppendToPath (BeFileName (BeFileName::Basename, BeFileName(callerSourceFile)).GetName());
    return path;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName DgnDbTestDgnManager::GetReadOnlyTestData (WCharCP fileName, CharCP callerSourceFile, bool forceMakeCopy)
    {
    BeFileName fullFileName;
    StatusInt status;
    if (forceMakeCopy)
        status = GetTestDataOut (fullFileName, fileName, fileName, callerSourceFile);
    else
        status = TestDataManager::FindTestData (fullFileName, fileName, GetUtDatPath(callerSourceFile));

    if (SUCCESS != status)
        {
        NativeLogging::LoggingManager::GetLogger (L"BeTest")->errorv (L"failed to find project name=\"%ls\" callerSourceFile=\"%ls\"", fileName, WString(callerSourceFile,BentleyCharEncoding::Utf8).c_str());
        BeAssert (false && "failed to open find input project file");
        return BeFileName(L"");
        }
    return fullFileName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName DgnDbTestDgnManager::GetWritableTestData (WCharCP fileName, CharCP callerSourceFile)
    {
    BeFileName fullFileName;
    if (GetTestDataOut (fullFileName, fileName, fileName, callerSourceFile) != SUCCESS)
        {
        NativeLogging::LoggingManager::GetLogger (L"BeTest")->errorv (L"failed to find or copy project name=\"%ls\" callerSourceFile=\"%ls\"", fileName, WString(callerSourceFile,BentleyCharEncoding::Utf8).c_str());
        BeAssert (false && "failed to open find or copy input project file");
        return BeFileName(L"");
        }
    return fullFileName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbTestDgnManager::DgnDbTestDgnManager (WCharCP dgnfilename, CharCP callerSourceFile, FileOpenMode omode, DgnInitializeMode imode, bool forceMakeCopy) 
    : TestDgnManager ((OPENMODE_READONLY==omode)? GetReadOnlyTestData(dgnfilename,callerSourceFile, forceMakeCopy): GetWritableTestData(dgnfilename,callerSourceFile), omode, imode) 
    {;}

END_DGNDB_UNIT_TESTS_NAMESPACE
