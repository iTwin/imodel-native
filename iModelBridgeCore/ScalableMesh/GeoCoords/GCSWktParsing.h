/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#pragma once

#include <ScalableMesh/GeoCoords/Definitions.h>

BEGIN_BENTLEY_SCALABLEMESH_GEOCOORDINATES_NAMESPACE

struct WKTParameter;
struct WKTSection;
struct Unit;
struct TransfoMatrix;

void                            GetLocalCsWkt                              (WString&                    wkt,
                                                                            const Unit&                     unit);


void                            GetLocalCsWkt                              (WString&                    wkt,
                                                                            const Unit&                     horizontalUnit,
                                                                            const Unit&                     verticalUnit);


void                            GetFittedCsWkt                             (WString&                    wkt, 
                                                                            const TransfoMatrix&            transform,
                                                                            bool                            transformIsToBase,
                                                                            const WString&              baseCsWkt);

bool                            HasBentleyAsAuthority                      (const WKTSection&               wktSection);



bool                            ExtractLocalCS                             (const WKTSection&               wktSection,
                                                                            Unit&                           unit);


bool                            ExtractLocalComposedCS                     (const WKTSection&               wktSection,
                                                                            Unit&                           horizontalUnit,
                                                                            Unit&                           verticalUnit);


bool                            ExtractFittedCS                            (const WKTSection&               wktSection,
                                                                            TransfoMatrix&                  transform,
                                                                            bool&                           transformIsToBase,
                                                                            const WKTParameter*&            baseCsWktParameter);

END_BENTLEY_SCALABLEMESH_GEOCOORDINATES_NAMESPACE
