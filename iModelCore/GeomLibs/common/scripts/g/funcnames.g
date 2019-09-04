RenameFunctions:bsiDMatrix3d_initIdentity\W(\
\W<ARGUMENT>\W)=@NativeInstanceCall{$1}initIdentity ()

RenameFunctions:bsiDMatrix3d_zero\W(\
\W<ARGUMENT>\W)=@NativeInstanceCall{$1}zero ()

RenameFunctions:bsiDMatrix3d_determinant\W(\
\W<ARGUMENT>\W)=@NativeInstanceCall{$1}determinant ()

RenameFunctions:bsiDMatrix3d_isIdentity\W(\
\W<ARGUMENT>\W)=@NativeInstanceCall{$1}isIdentity ()

RenameFunctions:bsiDMatrix3d_initFromAxisAndRotationAngle\W(\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W)=@NativeInstanceCall{$1}initFromAxisAndRotationAngle ($2, $3)

RenameFunctions:bsiDMatrix3d_multiplyTransposeDPoint3d\W(\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W)=@NativeInstanceCall{$1}multiplyTranspose ($2)

RenameFunctions:bsiDMatrix3d_initFromScaledOuterProduct\W(\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W)=@NativeInstanceCall{$1}initFromScaledOuterProduct ($2, $3, $4)

RenameFunctions:bsiDMatrix3d_multiplyDPoint3dArray\W(\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W)=@NativeInstanceCall{$1}multiply ($2, $3, $4)

RenameFunctions:jmdlDMatrix3d_squareAndNormalizeColumns\W(\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W)=@NativeInstanceCall{$1}squareAndNormalizeColumns ($2, $3, $4)

RenameFunctions:bsiDMatrix3d_initFromRowVectors\W(\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W)=@NativeInstanceCall{$1}initFromRowVectors ($2, $3, $4)


RenameFunctions:bsiDMatrix3d_initFromScaleFactors\W(\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W)=@NativeInstanceCall{$1}initFromScaleFactors ($2, $3, $4)



RenameFunctions:bsiDMatrix3d_scaleColumns\W(\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W)=@NativeInstanceCall{$1}scaleColumns ($2, $3, $4, $5)

RenameFunctions:bsiDMatrix3d_multiplyRange\W(\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W)=@NativeInstanceCall{$1}multiply ($2, $3)

RenameFunctions:bsiDMatrix3d_multiplyDMatrix3dDMatrix3d\W(\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W)=@NativeInstanceCall{$1}productOf ($2, $3)

RenameFunctions:bsiDMatrix3d_getRow\W(\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W)=@NativeInstanceCall{$1}getRow ($2, $3)

RenameFunctions:bsiDMatrix3d_transpose\W(\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W)=@NativeInstanceCall{$1}transposeOf ($2)

RenameFunctions:bsiDPoint3d_scale\W(\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W)=@NativeInstanceCall{$1}scale ($2, $3)

RenameFunctions:bsiDPoint3d_scaleInPlace\W(\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W)=@NativeInstanceCall{$1}scale ($2)

RenameFunctions:bsiDPoint3d_fromArray\W(\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W)=@NativeInstanceCall{$1}initFromArray ($2)


RenameFunctions:bsiDPoint3d_zero\W(\
\W<ARGUMENT>\W)=@NativeInstanceCall{$1}zero ()

RenameFunctions:bsiDPoint3d_normalize\W(\
\W<ARGUMENT>\W)=@NativeInstanceCall{$1}normalize ()

RenameFunctions:bsiDPoint3d_magnitudeSquared\W(\
\W<ARGUMENT>\W)=@NativeInstanceCall{$1}magnitudeSquared ()

RenameFunctions:bsiDPoint3d_dotProductXY\W(\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W)=@NativeInstanceCall{$1}dotProductXY ($2)

RenameFunctions:bsiDPoint3d_crossProductXY\W(\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W)=@NativeInstanceCall{$1}crossProductXY ($2)

RenameFunctions:bsiDPoint3d_dotProduct\W(\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W)=@NativeInstanceCall{$1}dotProduct ($2)

RenameFunctions:bsiDPoint3d_magnitude\W(\
\W<ARGUMENT>\W)=@NativeInstanceCall{$1}magnitude ()

RenameFunctions:bsiDPoint3d_addScaledDPoint3d\W(\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W)=@NativeInstanceCall{$1}sumOf ($2, $3, $4)


RenameFunctions:jmdlDPoint3d_getTriad\W(\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W)=@NativeInstanceCall{$1}getTriad ($2, $3, $4)

RenameFunctions:jmdlDPoint3d_getNormalizedTriad\W(\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W)=@NativeInstanceCall{$1}getNormalizedTriad ($2, $3, $4)


RenameFunctions:bsiGeom_cyclic3dAxes=Geom.cyclic3dAxes
RenameFunctions:bsiGeom_cyclic3dAxis=Geom.cyclic3dAxis

RenameFunctions:bsiDPoint3d_add2ScaledDPoint3d\W(\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W)=@NativeInstanceCall{$1}sumOf ($2, $3, $4, $5, $6)

RenameFunctions:bsiDPoint3d_signedAngleBetweenVectors\W(\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W)=@NativeInstanceCall{$1}signedAngleTo ($2, $3)

RenameFunctions:bsiDPoint3d_setComponent\W(\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W)=@NativeInstanceCall{$1}setComponent ($2, $3)

RenameFunctions:bsiDPoint3d_setComponents\W(\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W)=@NativeInstanceCall{$1}init ($2, $3, $4)

RenameFunctions:bsiDPoint3d_getComponent\W(\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W)=@NativeInstanceCall{$1}getComponent ($2)

RenameFunctions:bsiDPoint3d_crossProduct\W(\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W)=@NativeInstanceCall{$1}crossProduct ($2, $3)

RenameFunctions:bsiDPoint3d_normalizedCrossProduct\W(\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W)=@NativeInstanceCall{$1}normalizedCrossProduct ($2, $3)

RenameFunctions:bsiDPoint3d_scaleToLength\W(\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W)=@NativeInstanceCall{$1}scaleToLength ($2, $3)

RenameFunctions:bsiDPoint3d_tripleProduct\W(\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W)=@NativeInstanceCall{$1}tripleProduct ($2, $3)

RenameFunctions:bsiDPoint3d_subtractDPoint3dDPoint3d\W(\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W)=@NativeInstanceCall{$1}differenceOf ($2, $3)

RenameFunctions:bsiDPoint3d_normalizeInPlace\W(\
\W<ARGUMENT>\W)=@NativeInstanceCall{$1}normalize ()


RenameFunctions:bsiDRange3d_box2Points\W(\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W)=@NativeInstanceCall{$1}get8Corners ($2)

RenameFunctions:bsiDRange3d_isNull\W(\
\W<ARGUMENT>\W)=@NativeInstanceCall{$1}isNull ()

RenameFunctions:bsiDRange3d_initFromArray\W(\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W)=@NativeInstanceCall{$1}initFrom ($2, $3)

RenameFunctions:bsiDRange3d_getLargestCoordinate\W(\
\W<ARGUMENT>\W)=@NativeInstanceCall{$1}largestCoordinate ()

RenameFunctions:bsiDPoint3d_isVectorInSmallerSector\W(\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W)=@NativeInstanceCall{$1}isVectorBetween ($2, $3)

RenameFunctions:bsiDPoint3d_isVectorInCCWSector\W(\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W,\
\W<ARGUMENT>\W)=@NativeInstanceCall{$1}isVectorBetween ($2, $3, $4)

RenameFunctions:?=?
