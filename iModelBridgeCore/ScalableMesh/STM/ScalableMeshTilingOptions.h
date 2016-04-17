/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshTilingOptions.h $
|    $RCSfile: ScalableMeshTilingOptions.h,v $
|   $Revision: 1.5 $
|       $Date: 2011/08/02 14:59:39 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------+
|                                                                        |
|    ScalableMeshFileCreator.h                              (C) Copyright 2001.        |
|                                                BCIVIL Corporation.        |
|                                                All Rights Reserved.    |
|                                                                       |
+----------------------------------------------------------------------*/

#pragma once

/*__PUBLISH_SECTION_START__*/

/*----------------------------------------------------------------------+
| Exported classes and definition                                       |
+----------------------------------------------------------------------*/
/*
class BCDIGITALTM_DLLE BcDTM;
class BCDIGITALTM_DLLE IBcDTMDrapedLine;
class BCDIGITALTM_DLLE IBcDTMDrapedLinePoint;
class BCDIGITALTM_DLLE IBcDTMFeature;
class BCDIGITALTM_DLLE IBcDTMFeatureEnumerator;
class BCDIGITALTM_DLLE IBcDTMLinearFeature;
class BCDIGITALTM_DLLE IBcDTMComplexLinearFeature;
class BCDIGITALTM_DLLE IBcDTMSpot;
class BCDIGITALTM_DLLE IBcDTMInput;
*/

/*----------------------------------------------------------------------+
| This template class must be exported, so we instanciate and export it |
+----------------------------------------------------------------------*/

/*----------------------------------------------------------------------+
| CONSTANT definitions                                                  |
+----------------------------------------------------------------------*/

/*----------------------------------------------------------------------+
| CLASS definitions                                                     |
+----------------------------------------------------------------------*/
/*
typedef int (*PFBrowseContourCallback) 
(
int featureType, 
BcDTMUserTag   featureTag,     
BcDTMFeatureId featureId, 
DPoint3d       *tPoint, 
int            nPoint, 
void *userArgP
);

class DTMFeatureCache;

typedef int (*PFBrowseFeatureCacheCallback)
(
DTMFeatureCache* cache,
void *userP
);

typedef int (*PFBrowseFeatureCallback)
(
int            featureType, 
BcDTMUserTag   featureTag,
BcDTMFeatureId featureId, 
DPoint3d       *tPoint, 
int            nPoint, 
void           *userP
);

typedef int (*PFBrowseSinglePointFeatureCallback)
(
int featureType, 
DPoint3d *point,
void *userP
);

typedef int (*PFBrowseSlopeIndicatorCallback)
(
bool major,
DPoint3d *point1,
DPoint3d *point2,
void *userP
);


typedef int (*PFDuplicatePointsCallback)
(
double X,
double Y,
DTM_DUPLICATE_POINT_ERROR *dupErrorsP,
long numDupErrors,
void      *userP
);

typedef int (*PFCrossingFeaturesCallback)
(
DTM_CROSSING_FEATURE_ERROR crossError,
void      *userP
);

typedef int (*PFTriangleMeshCallback)
(
 int       featureType,
 int       numTriangles,
 int       numMeshPoints,
 DPoint3d  *meshPointsP,
 int       numMeshFaces,
 int       *meshFacesP,
 void      *userP
);

struct IBcDTMEdges
{
public: virtual int GetEdgeCount() = 0;
public: virtual DPoint3d* GetEdgeStartPoint(int index) = 0;
public: virtual DPoint3d* GetEdgeEndPoint(int index) = 0;
public: virtual void Release() = 0;
public: virtual void AddRef() = 0;
};

struct IBcDTMMeshFace
{
public: virtual DPoint3d GetCoordinates(int index) = 0;
public: virtual int GetMeshPointIndex(int index) = 0;
public: virtual void Release() = 0;
public: virtual void AddRef() = 0;
};

struct IBcDTMMesh
{
public: virtual int GetPointCount() = 0;
public: virtual int GetFaceCount() = 0;
public: virtual DPoint3d GetPoint(int index) = 0;
public: virtual IBcDTMMeshFace* GetFace(int index) = 0;
public: virtual void Release() = 0;
public: virtual void AddRef() = 0;
};
*/

/*----------------------------------------------------------------------------+
|Class ScalableMeshTilingOptionsBase
|
| Each different tiling method should have its own tiling option class.
+----------------------------------------------------------------------------*/
class ScalableMeshTilingOptionsBase
    {
    public:     
        
        BENTLEY_SM_EXPORT ScalableMeshTilingOptionsBase();

        BENTLEY_SM_EXPORT virtual ~ScalableMeshTilingOptionsBase();
    };