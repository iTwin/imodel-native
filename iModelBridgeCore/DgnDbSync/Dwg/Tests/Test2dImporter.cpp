/*--------------------------------------------------------------------------------------+
|
|     $Source: Dwg/Tests/Test2dImporter.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Dwg/DwgBridge.h>

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          02/19
+===============+===============+===============+===============+===============+======*/
struct Test2dImporter : public DwgImporter
    {
    DEFINE_T_SUPER (DwgImporter)
    Test2dImporter (DwgImporter::Options& options) : T_Super(options)
        {
        }
    DgnClassId _GetElementType (DwgDbBlockTableRecordCR block) override
        {
        return GetDgnDb().Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_DrawingGraphic);
        }
    DgnClassId _GetModelType (DwgDbBlockTableRecordCR block) override
        {
        return GetDgnDb().Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_DrawingModel);
        }
    };  // Test2dImporter

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          02/19
+===============+===============+===============+===============+===============+======*/
struct Test2dBridge : public DwgBridge
{
DEFINE_T_SUPER (DwgBridge)
DwgImporter*    _CreateDwgImporter () override
    {
    DwgImporter::Options* params = static_cast<DwgImporter::Options*> (&_GetParams());
    return nullptr == params ? nullptr : new Test2dImporter(*params);
    }
};  // Test2dBridge

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/19
+---------------+---------------+---------------+---------------+---------------+------*/
int wmain (int argc, wchar_t const* argv[])
    {
    Test2dBridge    test;
    return  (int)test.RunAsStandaloneExe(argc, argv);
    }

