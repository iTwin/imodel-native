/*--------------------------------------------------------------------------------------+
|
|     $Source: TerrainModelNET/DTMHelpers.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "StdAfx.h"
#include ".\dtmhelpers.h"
#using <mscorlib.dll>

BEGIN_BENTLEY_TERRAINMODELNET_NAMESPACE

DTMFeature^ DTMHelpers::GetAsSpecificFeature(BcDTMFeature* pFeature)
{
    // 1. Try to make a linear feature
    if (pFeature->AsLinear ())
    {
        return gcnew DTMLinearFeature(pFeature->AsLinear());
    }

    if(pFeature->AsComplexLinear())
    {
        return gcnew DTMComplexLinearFeature(pFeature->AsComplexLinear());
    }

    // 2. Try to make a spot feature
    if (pFeature->AsSpot())
    {
        return gcnew DTMSpot(pFeature->AsSpot());
    }

    return nullptr;
}

/// This method should probably be in a utility class for managed C++ encapsulation
void DTMHelpers::Copy (DPoint3d &targetPt, BGEO::DPoint3d% sourcePt)
{
    targetPt.x = sourcePt.X;
    targetPt.y = sourcePt.Y;
    targetPt.z = sourcePt.Z;
}

/// This method should probably be in a utility class for managed C++ encapsulation
void DTMHelpers::Copy (BGEO::DPoint3d% targetPt, DPoint3d &sourcePt)
{
    targetPt.X = sourcePt.x;
    targetPt.Y = sourcePt.y;
    targetPt.Z = sourcePt.z;
}
/// This method should probably be in a utility class for managed C++ encapsulation
void DTMHelpers::Copy (DPoint3d** targetPtPP, array<BGEO::DPoint3d>^ sourcePt, int nPoint)
{
    *targetPtPP = (DPoint3d*)bcMem_malloc (nPoint*sizeof(DPoint3d));
    for (int iPoint = 0; iPoint < nPoint; iPoint++)
    {
        DTMHelpers::Copy((*targetPtPP)[iPoint], sourcePt[iPoint]);
    }
}

int DTMHelpers::Copy (DPoint3d** targetPtPP, System::Collections::Generic::IEnumerable<BGEO::DPoint3d>^ sourcePt)
{
    if (sourcePt == nullptr)
        {
        *targetPtPP = NULL;
        return 0;
        }
    System::Collections::Generic::IEnumerator<BGEO::DPoint3d>^ enumerator = sourcePt->GetEnumerator();

    int nPoint = 0;
    enumerator->Reset();
    while(enumerator->MoveNext()) nPoint++;

    *targetPtPP = (DPoint3d*)bcMem_malloc (nPoint*sizeof(DPoint3d));
    int iPoint = 0;

    for each(BGEO::DPoint3d pt in sourcePt)
        {
        DTMHelpers::Copy((*targetPtPP)[iPoint], pt);
        iPoint++;
        }
    return nPoint;
}


void DTMHelpers::Copy (DRange1d** targetPP, array<BGEO::DRange1d>^ source, int nInt)
{
    *targetPP = (DRange1d*)bcMem_malloc (nInt*sizeof(DRange1d));
    for (int iInt = 0; iInt < nInt; iInt++)
    {
        if(source[iInt].Low > source[iInt].High)
            {
            (*targetPP)[iInt].high = source[iInt].Low;
            (*targetPP)[iInt].low = source[iInt].High;
            }
        else
            {
            (*targetPP)[iInt].high = source[iInt].High;
            (*targetPP)[iInt].low = source[iInt].Low;
            }
    }
}

int DTMHelpers::Copy (DRange1d** targetPP, System::Collections::Generic::IEnumerable<BGEO::DRange1d>^ source)
{
    System::Collections::Generic::IEnumerator<BGEO::DRange1d>^ enumerator = source->GetEnumerator();

    int nInt = 0;
    enumerator->Reset();
    while(enumerator->MoveNext()) nInt++;

    *targetPP = (DRange1d*)bcMem_malloc (nInt*sizeof(DRange1d));
    int iInt = 0;

    for each(BGEO::DRange1d range in source)
        {
        if(range.Low > range.High)
            {
            (*targetPP)[iInt].high = range.Low;
            (*targetPP)[iInt].low = range.High;
            }
        else
            {
            (*targetPP)[iInt].high = range.High;
            (*targetPP)[iInt].low = range.Low;
            }
        iInt++;
        }
    return nInt;
}

void DTMHelpers::Copy (DTMFeatureType &targetType, ::DTMFeatureType sourceType)
{
    targetType = (DTMFeatureType)sourceType;
}

void DTMHelpers::Copy(::DTMFeatureType& target, DTMFeatureType feature) 
{ 
    target = (::DTMFeatureType)feature;
}

void DTMHelpers::Copy (DTMDynamicFeatureType &targetType, ::DTMFeatureType sourceType)
{
    targetType = (DTMDynamicFeatureType)sourceType;
}

void DTMHelpers::Copy(::DTMFeatureType& target, DTMDynamicFeatureType feature) 
{ 
    target = (::DTMFeatureType)feature;
}

END_BENTLEY_TERRAINMODELNET_NAMESPACE
