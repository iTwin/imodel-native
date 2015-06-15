/*-------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/RasterBaseModel.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
void RasterBaseModel::_AddGraphicsToScene (ViewContextR context)
    {
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
AxisAlignedBox3d RasterBaseModel::_QueryModelRange() const
    {
    BeAssert(false);     // Child class must implement this method.
    return AxisAlignedBox3d();    
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
void RasterBaseModel::_ToPropertiesJson(Json::Value& v) const
    {
    T_Super::_ToPropertiesJson(v);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
void RasterBaseModel::_FromPropertiesJson(Json::Value const& v)
    {
    T_Super::_FromPropertiesJson(v);
    }