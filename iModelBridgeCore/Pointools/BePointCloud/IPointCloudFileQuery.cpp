/*--------------------------------------------------------------------------------------+
|
|     $Source: IPointCloudFileQuery.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <BePointCloudInternal.h>

USING_NAMESPACE_BENTLEY_BEPOINTCLOUD

/*---------------------------------------------------------------------------------**//**
* Published methods that wrap around the virtual methods.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP         IPointCloudFileQuery::GetFileName() const {return _GetFileName();}
uint64_t        IPointCloudFileQuery::GetNumberOfPoints() const {return _GetNumberOfPoints();}
uint32_t        IPointCloudFileQuery::GetNumberOfClouds() const {return _GetNumberOfClouds();}
bool            IPointCloudFileQuery::HasIntensityChannel() const {return _HasIntensityChannel();}
bool            IPointCloudFileQuery::HasClassificationChannel() const {return _HasClassificationChannel();}
bool            IPointCloudFileQuery::HasRGBChannel() const {return _HasRGBChannel();}
bool            IPointCloudFileQuery::HasNormalChannel() const {return _HasNormalChannel();}
StatusInt       IPointCloudFileQuery::GetMetaTag(WStringCR tagName, WStringR value) const {return _GetMetaTag(tagName, value);}
uint32_t        IPointCloudFileQuery::GetNumUserMetaSections() const {return _GetNumUserMetaSections();}
StatusInt       IPointCloudFileQuery::GetMetaData (WStringR name, uint32_t& numClouds, uint64_t& numPoints, DPoint3dP lowerBound, DPoint3dP upperBound) const {return _GetMetaData(name, numClouds, numPoints, lowerBound, upperBound);}
uint32_t        IPointCloudFileQuery::GetNumUserMetaTagsInSection(int32_t sectionIndex) const {return _GetNumUserMetaTagsInSection(sectionIndex);}
StatusInt       IPointCloudFileQuery::GetUserMetaSectionName (int32_t sectionIndex, WStringR name) const {return _GetUserMetaSectionName(sectionIndex, name);}
StatusInt       IPointCloudFileQuery::GetUserMetaTagByIndex(int32_t sectionIndex, int32_t tagIndex, WStringR tagName, WStringR tagValue) const {return _GetUserMetaTagByIndex(sectionIndex, tagIndex, tagName, tagValue);}
StatusInt       IPointCloudFileQuery::GetUserMetaTagByName(WStringCR tagName, WStringR tagValue) const {return _GetUserMetaTagByName(tagName, tagValue);}
