/*--------------------------------------------------------------------------------------+
|
|     $Source: TerrainModelNET/DTMHelpers.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "Bentley.Civil.DTM.h"
#include "DTMFeature.h"

//using namespace Bentley::Civil::BCSystem::Base;

BEGIN_BENTLEY_TERRAINMODELNET_NAMESPACE

#define ConvertStruct(mtype, type) \
    static void Copy ([System::Runtime::InteropServices::OutAttribute] mtype% managed, const type & native) \
                                    { \
    pin_ptr <mtype> pinned = &managed; \
    memcpy (pinned, &native, sizeof native); \
                                    } \
    static void Copy (type& native, mtype% managed) \
                                    { \
    pin_ptr <mtype> pinned = &managed; \
    memcpy (&native, pinned, sizeof native); \
                                    }

public ref class DTMHelpers
{
internal:
    static DTMFeature^ GetAsSpecificFeature(BcDTMFeature* pFeature);

    static void Copy (DPoint3d &targetPt, BGEO::DPoint3d% sourcePt);

    static void Copy (BGEO::DPoint3d% targetPt, DPoint3d &sourcePt);

    static void Copy (DPoint3d** targetPtPP, array<BGEO::DPoint3d>^ sourcePt, int nPoint);

    static int Copy (DPoint3d** targetPtPP, System::Collections::Generic::IEnumerable<BGEO::DPoint3d>^ sourcePt);

    static void Copy (DTMDynamicFeatureType &targetType, ::DTMFeatureType sourceType);

    static void Copy (::DTMFeatureType &targetType, DTMDynamicFeatureType sourceType);

    static void Copy (DTMFeatureType &targetType, ::DTMFeatureType sourceType);

    static void Copy (::DTMFeatureType &targetType, DTMFeatureType sourceType);

    static void Copy (DRange1d** targetPP, array<BGEO::DRange1d>^ source, int nInt);

    static int  Copy (DRange1d** targetPP, System::Collections::Generic::IEnumerable<BGEO::DRange1d>^ source);

    ConvertStruct (Bentley::GeometryNET::DMatrix4d, Bentley::DMatrix4d);

    ConvertStruct (Bentley::GeometryNET::DMap4d, Bentley::DMap4d);

};

#undef ConvertStruct

END_BENTLEY_TERRAINMODELNET_NAMESPACE
