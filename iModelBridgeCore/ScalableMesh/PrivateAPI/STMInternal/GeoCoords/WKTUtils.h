/*--------------------------------------------------------------------------------------+
|
|     $Source: PrivateAPI/STMInternal/GeoCoords/WKTUtils.h $
|    $RCSfile: WKTUtils.h,v $
|   $Revision: 1.3 $
|       $Date: 2011/12/20 16:24:03 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <ScalableMesh/GeoCoords/Definitions.h>
#include <deque>

//NEEDS_WORK_SM : Might be better if put in D:\BSI\DgnDb06SM\src\ScalableMesh\PrivateAPI
#include "../../../STM/Stores/SMStoreUtils.h"

namespace ISMStore
    {


    enum WktFlavor
        {
        WktFlavor_Oracle9 = 1,
        WktFlavor_Autodesk,
        WktFlavor_End,
        };
    }

BEGIN_BENTLEY_SCALABLEMESH_GEOCOORDINATES_NAMESPACE


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                   Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct WKTKeyword
    {
    enum Type
        {
        TYPE_NULL,
        TYPE_AUTHORITY,
        TYPE_COMPD_CS,
        TYPE_FITTED_CS,
        TYPE_INVERSE_MT,
        TYPE_LOCAL_CS,
        TYPE_LOCAL_DATUM,
        TYPE_PARAM_MT,
        TYPE_PARAMETER,
        TYPE_UNIT,
        TYPE_UNKNOWN,
        };


    const WChar*                         str;
    const WChar*                         strEnd;
    size_t                              strLen;
    Type                                type;

    template <size_t NAME_SIZE>
    explicit                            WKTKeyword                                 (const WChar (&n)[NAME_SIZE], 
                                                                                    Type                t);
    };


const WChar*                             FindWKTSectionKeyword                      (const WChar*         wktBegin, 
                                                                                    const WChar*         wktEnd);

const WKTKeyword&                       GetWKTKeyword                              (const WChar*         keywordBegin);
const WKTKeyword&                       GetWKTKeyword                              (WKTKeyword::Type    type);


ISMStore::WktFlavor                     GetWKTFlavor                       (WString* wktWithoutFlavorStr, const WString& wktStr);

WKTKeyword::Type                        GetWktType                         (WString wkt);

bool MapWktFlavorEnum(BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCS::WktFlavor& baseGcsWktFlavor, ISMStore::WktFlavor fileWktFlavor);


END_BENTLEY_SCALABLEMESH_GEOCOORDINATES_NAMESPACE
