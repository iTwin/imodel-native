/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
@nodoc
@description Pull one node from the free node pool.
@param graphP IN OUT graph header
@return pointer to allocated node
@group "VU Memory Management"
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP VuP             vu_allocateVuNode
(
VuSetP graphP
);



END_BENTLEY_GEOMETRY_NAMESPACE

