/*--------------------------------------------------------------------------------------+
|
|     $Source: TerrainModelNET/DTMDelegates.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
BEGIN_BENTLEY_TERRAINMODELNET_NAMESPACE

ref class DTMDuplicatePointError;
ref class DTMCrossingFeatureError;
ref class DTMFeatureInfo;
ref class DTMDynamicFeatureInfo;

/// <summary>
/// Delegate for transforming points.
/// </summary>
public delegate bool DTMTransformCallbackDelegate (array<BGEO::DPoint3d>^ tPoint, System::Object^ oArg);

/// <summary>
/// Delegate for contours browsing methods.
/// </summary>
public delegate bool ContoursBrowsingDelegate (array<BGEO::DPoint3d>^ tPoint, BGEO::DPoint3d contourDirection, System::Object^ oArg);

/// <summary>
/// Delegate for dynamic features browsing methods.
/// </summary>
public delegate bool DynamicFeaturesBrowsingDelegate (DTMDynamicFeatureInfo^ featureInfo, System::Object^ oArg);

/// <summary>
/// Delegate for features browsing methods.
/// </summary>
public delegate bool LinearFeaturesBrowsingDelegate (DTMFeatureInfo^ featureInfo, System::Object^ oArg);

/// <summary>
/// Delegate for Points browsing methods.
/// </summary>
public delegate bool PointsBrowsingDelegate (array<BGEO::DPoint3d>^ tPoint, System::Object^ oArg);

/// <summary>
/// Delegate for Point features browsing methods.
/// </summary>
public delegate bool PointFeaturesBrowsingDelegate (DTMFeatureInfo^ dtmFeatureInfo, System::Object^ oArg);

/// <summary>
/// Delegate for single point features browsing methods.
/// </summary>
public delegate bool SinglePointFeaturesBrowsingDelegate (DTMDynamicFeatureType featureType, BGEO::DPoint3d point, System::Object^ oArg);

/// <summary>
/// Delegate for slop indicator browsing methods.
/// </summary>
public delegate bool SlopeIndicatorsBrowsingDelegate (bool major, BGEO::DPoint3d startPoint, BGEO::DPoint3d endPoint, System::Object^ oArg);

/// <summary>
/// Delegate for duplicate points browsing methods.
/// </summary>
public delegate bool DuplicatePointsBrowsingDelegate (double X, double Y, array<DTMDuplicatePointError^>^ dupErrors, System::Object^ userP);

/// <summary>
/// Delegate for crossing features browsing methods.
/// </summary>
public delegate bool CrossingFeaturesBrowsingDelegate (DTMCrossingFeatureError^ crossError, System::Object^ userP);

/// <summary>
/// Delegate for triangle mesh browsing methods.
/// </summary>
public delegate bool TriangleMeshBrowsingDelegate (array<BGEO::DPoint3d>^ meshPoints, array<int>^ meshTriangles, System::Object^ userP);

END_BENTLEY_TERRAINMODELNET_NAMESPACE
