/*--------------------------------------------------------------------------------------+
|
|     $Source: GeoCoords/GCSWktParsing.cpp $
|    $RCSfile: GCSWktParsing.cpp,v $
|   $Revision: 1.2 $
|       $Date: 2011/10/20 18:47:32 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableMeshPCH.h>
#include "../STM/ImagePPHeaders.h"
#include "GCSWktParsing.h"
#include "WktUtils.h"
#include <STMInternal/Foundations/FoundationsPrivateTools.h>
#include <ScalableMesh/GeoCoords/GCS.h>
#include <ScalableMesh/GeoCoords/Transformation.h>
#include <ScalableMesh/GeoCoords/LocalTransform.h>
#include <ScalableMesh\Import\DataTypeDescription.h>

#include <STMInternal/Foundations/PrivateStringTools.h>




using namespace std;


BEGIN_BENTLEY_SCALABLEMESH_GEOCOORDINATES_NAMESPACE

namespace {

enum BentleyLocalDatumType
    {
    BLDT_START = 11000,
    BLDT_ANYWHERE_XYZ = BLDT_START,
    BLDT_ANYWHERE_LAT_LONG,
    BLDT_ANYWHERE_VERTICAL,
    };

enum BentleyAuthorityID
    {
    BAI_UNDEFINED = 0,
    BAI_LOCAL_DATUM_TYPE_START = 11000,
    BAI_ANYWHERE_XYZ = BAI_LOCAL_DATUM_TYPE_START,
    BAI_ANYWHERE_LAT_LONG,
    BAI_ANYWHERE_VERTICAL,
    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void PrintTransformWkt(wostringstream&       transformStream,
                       const TransfoMatrix& transform,
                       bool                 isInversed)
    {
    // Open transform section
    transformStream 
        << (isInversed ? L"INVERSE_MT[" : L"") << L"PARAM_MT[\"Affine\", ";

    size_t alteredParamRow[3*4];
    size_t transfoParamColumn[3*4];
    size_t alteredParamQty = 0;

    // Find a listing of altered indexes when compared to identity
        {
        TransfoMatrix identity;
        for (size_t rowIdx = 0; rowIdx < 3; ++rowIdx)
            {
            for (size_t colIdx = 0; colIdx < 4; ++colIdx)
                {
                const double value = transform[rowIdx][colIdx];
                if (HNumeric<double>::EQUAL_EPSILON(identity[rowIdx][colIdx], value))
                    continue;

                alteredParamRow[alteredParamQty] = rowIdx;
                transfoParamColumn[alteredParamQty] = colIdx;
                ++alteredParamQty;
                }
            }
        }

    // Print all but last parameter with colon
    const size_t lastAlteredParamIdx = (alteredParamQty - 1);
    for (size_t i = 0; i < lastAlteredParamIdx; ++i)
        {
        transformStream <<
            L"PARAMETER[\"elt_" << alteredParamRow[i] << L"_" << transfoParamColumn[i] << L"\", " << 
            transform[alteredParamRow[i]][transfoParamColumn[i]] << L"],";
        }

    // Print last parameter without colon
    transformStream <<
            L"PARAMETER[\"elt_" << alteredParamRow[lastAlteredParamIdx] << L"_" << transfoParamColumn[lastAlteredParamIdx] << L"\", " << 
            transform[alteredParamRow[lastAlteredParamIdx]][transfoParamColumn[lastAlteredParamIdx]] << L"]";

    // Close transform section
    transformStream 
        << (isInversed ? L"]]" : L"]");
    }
}

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void GetLocalCsWkt         (WString&         wkt,
                            const Unit&     unit)
    {
    wostringstream localCsWktStream;
    localCsWktStream.precision(numeric_limits<double>::digits10 + 1);

    const WString unitName(unit.GetNameCStr());

    localCsWktStream 
        << L"LOCAL_CS[\"\", " << 
           L"LOCAL_DATUM[\"AnywhereXYZ\", " << BLDT_ANYWHERE_XYZ << L", AUTHORITY[\"BENTLEY_SYSTEMS\", \"" << BAI_ANYWHERE_XYZ << L"\"]], " <<
           L"UNIT[\"" << unitName << L"\", " << unit.GetRatioToBase() << L"], " << 
           L"AXIS[\"X\", OTHER], AXIS[\"Y\", OTHER], AXIS[\"Z\", OTHER], AUTHORITY[\"BENTLEY_SYSTEMS\", \"0\"]]";

    wkt = localCsWktStream.str().c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void GetLocalCsWkt         (WString&         wkt,
                            const Unit&     horizontalUnit,
                            const Unit&     verticalUnit)
    {
    wostringstream composedCsWktStream;
    composedCsWktStream.precision(numeric_limits<double>::digits10 + 1);

    const WString horizontalUnitName(horizontalUnit.GetNameCStr());
    const WString verticalUnitName(verticalUnit.GetNameCStr());

    composedCsWktStream 
        << L"COMPD_CS[\"\", " <<
           L"LOCAL_CS[\"\", " << 
           L"LOCAL_DATUM[\"AnywhereLatLong\", " << BLDT_ANYWHERE_LAT_LONG << L", AUTHORITY[\"BENTLEY_SYSTEMS\", \"" << BAI_ANYWHERE_LAT_LONG << L"\"]], " <<
           L"UNIT[\"" << horizontalUnitName << L"\", " << horizontalUnit.GetRatioToBase() << L"], " << 
           L"AXIS[\"Lat\", OTHER], AXIS[\"Long\", OTHER], AUTHORITY[\"BENTLEY_SYSTEMS\", \"0\"]], " <<
           L"LOCAL_CS[\"\", " << 
           L"LOCAL_DATUM[\"AnywhereVertical\", " << BLDT_ANYWHERE_VERTICAL << L", AUTHORITY[\"BENTLEY_SYSTEMS\", \"" << BAI_ANYWHERE_VERTICAL << L"\"]], " <<
           L"UNIT[\"" << verticalUnitName << L"\", " << verticalUnit.GetRatioToBase() << L"], " << 
           L"AXIS[\"Up\", UP], AUTHORITY[\"BENTLEY_SYSTEMS\", \"0\"]], " <<
           L"AUTHORITY[\"BENTLEY_SYSTEMS\", \"0\"]]";

    wkt = composedCsWktStream.str().c_str();
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void GetFittedCsWkt    (WString&                 wkt, 
                        const TransfoMatrix&    transform,
                        bool                    transformIsToBase,
                        const WString&           baseCsWkt)
    {
    wostringstream fittedCSStream;
    fittedCSStream.precision(numeric_limits<double>::digits10 + 1);

    fittedCSStream 
        << L"FITTED_CS[\"\", ";

    PrintTransformWkt(fittedCSStream, transform, !transformIsToBase);

    fittedCSStream
        << L", " <<
        baseCsWkt << L", " << 
        L"AUTHORITY[\"BENTLEY_SYSTEMS\", \"0\"]]";

    wkt = fittedCSStream.str().c_str();
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool HasBentleyAsAuthority (const WKTSection& wktSection)
    {
    if (0 == wktSection.GetSize())
        return false;

    const WKTParameter& authorityParameter = *(wktSection.end() - 1);
    if (!authorityParameter.IsSection())
        return false;

    const WKTSection& authoritySection = authorityParameter.GetSection();

    if (2 != authoritySection.GetSize() && 
        WKTKeyword::TYPE_AUTHORITY != GetWKTKeyword(authoritySection.keywordBegin()).type)
        return false;

    const WKTParameter& authorityNameParameter = authoritySection[0];

    static const WChar BENTLEY_AUTHORITY_NAME[] = L"\"BENTLEY_SYSTEMS\"";
    static const size_t BENTLEY_AUTHORITY_NAME_LEN = CSTRING_LEN(BENTLEY_AUTHORITY_NAME);
    

    const bool hasBentleyAsAuthority 
        = (BENTLEY_AUTHORITY_NAME_LEN == authorityNameParameter.strLen()) &&
          std::equal(BENTLEY_AUTHORITY_NAME, BENTLEY_AUTHORITY_NAME + BENTLEY_AUTHORITY_NAME_LEN,
                     authorityNameParameter.strBegin());

    return hasBentleyAsAuthority;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool ExtractLocalCS(const WKTSection&       wktSection,
                            Unit&                   unit)
    {
    if (WKTKeyword::TYPE_LOCAL_CS != GetWKTKeyword(wktSection.keywordBegin()).type ||
        wktSection.GetSize() < 3)
        return false;

    const WKTParameter& localDatumParameter(wktSection[1]);
    const WKTParameter& unitParameter(wktSection[2]);
    if (!localDatumParameter.IsSection() ||
        !unitParameter.IsSection())
        return false;

    const WKTSection& localDatumSection(localDatumParameter.GetSection());
    const WKTSection& unitSection(unitParameter.GetSection());

    if (WKTKeyword::TYPE_LOCAL_DATUM != GetWKTKeyword(localDatumSection.keywordBegin()).type ||
        WKTKeyword::TYPE_UNIT != GetWKTKeyword(unitSection.keywordBegin()).type ||
        localDatumSection.GetSize() < 2 ||
        unitSection.GetSize() < 2)
        return false;

    const WKTParameter& unitNameParameter(unitSection[0]);
    const WKTParameter& unitConvFactorParamerter(unitSection[1]);
    const WKTParameter& localDatumTypeParameter(localDatumSection[1]);
    if (unitNameParameter.IsSection() ||
        !unitNameParameter.IsQuoted() ||
        unitConvFactorParamerter.IsSection() ||
        localDatumTypeParameter.IsSection())
        return false;

    const WString unitName(unitNameParameter.strBeginInsideQuote(), unitNameParameter.strEndInsideQuote());

    WChar* unitConvFactorEnd = 0;
    const double ratioToBase = wcstod(unitConvFactorParamerter.strBegin(), &unitConvFactorEnd);

    WChar* localDatumTypeEnd = 0;
    const uint64_t localDatumType = wcstoul(localDatumTypeParameter.strBegin(), &localDatumTypeEnd, 10);

    if (0.0 == ratioToBase ||
        unitConvFactorEnd != unitConvFactorParamerter.strEnd() ||
        localDatumTypeEnd != localDatumTypeParameter.strEnd())
        return false;

    switch (localDatumType)
        {
    case BLDT_ANYWHERE_LAT_LONG:
        unit = Unit::CreateAngularFrom(unitName.c_str(), ratioToBase);
        return true;
    case BLDT_ANYWHERE_XYZ:
    case BLDT_ANYWHERE_VERTICAL:
        unit = Unit::CreateLinearFrom(unitName.c_str(), ratioToBase);
        return true;
        }

    assert(!"Unexpected!");
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool ExtractLocalComposedCS(const WKTSection&       wktSection,
                            Unit&                   horizontalUnit,
                            Unit&                   verticalUnit)
    {
    if (WKTKeyword::TYPE_COMPD_CS != GetWKTKeyword(wktSection.keywordBegin()).type ||
        3 > wktSection.GetSize())
        return false;
    
    const WKTParameter& horizontalCSParameter(wktSection[1]);
    if (!horizontalCSParameter.IsSection())
        return false;

    const WKTParameter& verticalCSParameter(wktSection[2]);
    if (!verticalCSParameter.IsSection())
        return false;

    return ExtractLocalCS(horizontalCSParameter.GetSection(), horizontalUnit) &&
           ExtractLocalCS(verticalCSParameter.GetSection(), verticalUnit) &&
           horizontalUnit.IsAngular() &&
           verticalUnit.IsLinear();
    }


namespace {

struct MatrixParameter
    {
    size_t      row;
    size_t      col;
    double      value;
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool ExtractParameter (const WKTSection&    wktSection,
                       MatrixParameter&     extractedParameter)
    {
    if (WKTKeyword::TYPE_PARAMETER != GetWKTKeyword(wktSection.keywordBegin()).type ||
        wktSection.GetSize() != 2)
        return false;

    const WKTParameter& nameParameter = wktSection[0];
    const WKTParameter& valueParameter = wktSection[1];

    if (!nameParameter.IsQuoted())
        return false;

    const WChar* name = nameParameter.strBeginInsideQuote();

    static const WChar ELT_PREFIX[] = L"elt_";

    if (0 != wcsnicmp(ELT_PREFIX, &name[0], CSTRING_LEN(ELT_PREFIX)) ||
        !isdigit(name[4]),
        L'_' != name[5] ||
        !isdigit(name[6]) ||
        L'\"' != name[7])
        return false;

    WChar* rowEnd = 0;
    const size_t row = wcstoul(&name[4], &rowEnd, 10);

    WChar* colEnd = 0;
    const size_t col = wcstoul(&name[6], &colEnd, 10);

    WChar* valueEnd = 0;
    const double value = wcstod(valueParameter.strBegin(), &valueEnd);
    
    if (rowEnd != &name[5] ||
        colEnd != &name[7] ||
        valueEnd != valueParameter.strEnd() ||
        3 <= row ||
        4 <= col)
        return false;

    extractedParameter.row = row;
    extractedParameter.col = col;
    extractedParameter.value = value;

    return true;
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool ExtractParamMt (const WKTSection&   wktSection,
                     TransfoMatrix&      transform)
    {
    if (WKTKeyword::TYPE_PARAM_MT != GetWKTKeyword(wktSection.keywordBegin()).type ||
        wktSection.GetSize() < 1)
        return false;
        
    const WKTParameter& nameParameter = wktSection[0];

    static const WChar AFFINE_NAME[] = L"Affine";

    if (!nameParameter.IsQuoted() ||
        0 != wcsnicmp(AFFINE_NAME, wktSection[0].strBeginInsideQuote(), CSTRING_LEN(AFFINE_NAME)))
        return false;

    TransfoMatrix extracted;
    
    for (WKTSection::const_iterator paramIt = wktSection.begin() + 1, paramsEnd = wktSection.end();
         paramIt != paramsEnd;
         ++paramIt)
        {
        MatrixParameter parameter = {0, 0, 0.0};

        if (!paramIt->IsSection() ||
            !ExtractParameter(paramIt->GetSection(), parameter))
            return false;

        extracted[parameter.row][parameter.col] = parameter.value;
        }

    transform = extracted;
    return true;
    }

}

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool ExtractFittedCS   (const WKTSection&       wktSection,
                        TransfoMatrix&          transform,
                        bool&                   transformIsToBase,
                        const WKTParameter*&    baseCsWktParameter)
    {
    if (WKTKeyword::TYPE_FITTED_CS != GetWKTKeyword(wktSection.keywordBegin()).type ||
        wktSection.GetSize() < 3)
        return false;
    
    const WKTParameter& toBaseParameter(wktSection[1]);
    const WKTParameter& baseCsParameter(wktSection[2]);

    if (!toBaseParameter.IsSection() ||
        !baseCsParameter.IsSection())
        return false;

    const WKTSection& toBaseSection = toBaseParameter.GetSection();

    TransfoMatrix extractedTransform;
    bool insideInverseSection = false;

    switch(GetWKTKeyword(toBaseSection.keywordBegin()).type)
        {
    case WKTKeyword::TYPE_INVERSE_MT:
        if (toBaseSection.GetSize() < 1 ||
            !toBaseSection[0].IsSection())
            return false;

        if (!ExtractParamMt(toBaseSection[0].GetSection(), extractedTransform))
            return false;

        insideInverseSection = true;
        break;
    case WKTKeyword::TYPE_PARAM_MT:
        if (!ExtractParamMt(toBaseSection, extractedTransform))
            return false;
        break;
    default:
        return false;
        }

    transform = extractedTransform;
    transformIsToBase = !insideInverseSection;
    baseCsWktParameter = &baseCsParameter;
    return true;
    }



END_BENTLEY_SCALABLEMESH_GEOCOORDINATES_NAMESPACE
