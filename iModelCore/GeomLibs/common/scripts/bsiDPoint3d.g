@define{@read{\\g\\balancedCodePatterns.g}}

!! One arg ...

bsiDPoint3d_zero\W(<BalancedArg>)=@CallThroughPointer{$1}Zero ()
bsiDPoint3d_magnitude\W(<BalancedArg>)=@CallThroughPointer{$1}Magnitude ()
bsiDPoint3d_magnitudeXY\W(<BalancedArg>)=@CallThroughPointer{$1}MagnitudeXY ()
bsiDPoint3d_magnitude\W(<BalancedArg>)=@CallThroughPointer{$1}Magnitude ()
bsiDVec3d_magnitude\W(<BalancedArg>)=@CallThroughPointer{$1}Magnitude ()
bsiDVec3d_negate\W(<BalancedArg>)=@CallThroughPointer{$1}Negate()
bsiDPoint3d_magnitudeSquared\W(<BalancedArg>)=@CallThroughPointer{$1}MagnitudeSquared ()
bsiVec3d_magnitudeSquared\W(<BalancedArg>)=@CallThroughPointer{$1}MagnitudeSquared ()

bsiDPoint3d_isDisconnect\W(<BalancedArg>)=@CallThroughPointer{$1}IsDisconnect ()

bsiDPoint3d_normalizeInPlace\W(<BalancedArg>)=@CallThroughPointer{$1}Normalize ()

bsiDPoint2d_zero\W(<BalancedArg>)=@CallThroughPointer{$1}Zero ()
bsiDPoint2d_magnitude\W(<BalancedArg>)=@CallThroughPointer{$1}Magnitude ()
bsiDPoint2d_magnitudeSquared\W(<BalancedArg>)=@CallThroughPointer{$1}MagnitudeSquared ()

bsiTransform_isIdentity\W(<BalancedArg>)=@CallThroughPointer{$1}IsIdentity ()
bsiRotMatrix_isIdentity\W(<BalancedArg>)=@CallThroughPointer{$1}IsIdentity ()

bsiTransform_initIdentity\W(<BalancedArg>)=@CallThroughPointer{$1}InitIdentity ()
bsiRotMatrix_initIdentity\W(<BalancedArg>)=@CallThroughPointer{$1}InitIdentity ()



!! Two args ...
bsiDPoint3d_scale\W(<BalancedArg>,<BalancedArg>)=@CallThroughPointer{$1}Scale ($2)

bsiDVec3d_negate\W(<BalancedArg>,<BalancedArg>)=@CallThroughPointer{$1}Negate (@PointerToRef{$2})

bsiDPoint3d_distance\W(<BalancedArg>,<BalancedArg>)=@CallThroughPointer{$1}Distance (@PointerToRef{$2})
bsiDPoint3d_distanceXY\W(<BalancedArg>,<BalancedArg>)=@CallThroughPointer{$1}DistanceXY (@PointerToRef{$2})
bsiDPoint3d_distanceSquared\W(<BalancedArg>,<BalancedArg>)=@CallThroughPointer{$1}DistanceSquared (@PointerToRef{$2})
bsiDPoint3d_distanceSquaredXY\W(<BalancedArg>,<BalancedArg>)=@CallThroughPointer{$1}DistanceSquaredXY (@PointerToRef{$2})

bsiDPoint3d_negate\W(<BalancedArg>,<BalancedArg>)=@CallThroughPointer{$1}Negate (@PointerToRef{$2})

bsiDPoint2d_distance\W(<BalancedArg>,<BalancedArg>)=@CallThroughPointer{$1}Distance (@PointerToRef{$2})
bsiDPoint2d_distanceSquared\W(<BalancedArg>,<BalancedArg>)=@CallThroughPointer{$1}DistanceSquared (@PointerToRef{$2})

bsiDVec3d_scaleInPlace\W(<BalancedArg>,<BalancedArg>)=@CallThroughPointer{$1}Scale ($2)

bsiDPoint3d_dotProduct\W(<BalancedArg>,<BalancedArg>)=@CallThroughPointer{$1}DotProduct (@PointerToRef{$2})
bsiDPoint3d_dotProductXY\W(<BalancedArg>,<BalancedArg>)=@CallThroughPointer{$1}DotProductXY (@PointerToRef{$2})

bsiDVec3d_dotProduct\W(<BalancedArg>,<BalancedArg>)=@CallThroughPointer{$1}DotProduct (@PointerToRef{$2})
bsiDPoint3d_crossProductXY\W(<BalancedArg>,<BalancedArg>)=@CallThroughPointer{$1}CrossProductXY (@PointerToRef{$2})
bsiDPoint2d_crossProduct\W(<BalancedArg>,<BalancedArg>)=@CallThroughPointer{$1}CrossProduct (@PointerToRef{$2})
bsiDPoint2d_dotProduct\W(<BalancedArg>,<BalancedArg>)=@CallThroughPointer{$1}DotProduct (@PointerToRef{$2})


bsiRotMatrix_multiplyDPoint3d\W(<BalancedArg>,<BalancedArg>)=@CallThroughPointer{$1}Multiply (@PointerToRef{$2})
bsiRotMatrix_multiplyTransposeDPoint3d\W(<BalancedArg>,<BalancedArg>)=@CallThroughPointer{$1}MultiplyTranspose (@PointerToRef{$2})
bsiTransform_multiplyDPoint3dInPlace\W(<BalancedArg>,<BalancedArg>)=@CallThroughPointer{$1}Multiply (@PointerToRef{$2})

bsiTransform_invert\W(<BalancedArg>,<BalancedArg>)=@CallThroughPointer{$1}InverseOf (@PointerToRef{$2})
bsiTransform_invertTransform\W(<BalancedArg>,<BalancedArg>)=@CallThroughPointer{$1}InverseOf (@PointerToRef{$2})

bsiDPoint3d_addDPoint3dInPlace\W(<BalancedArg>,<BalancedArg>)=@CallThroughPointer{$1}Add (@PointerToRef{$2})


bsiDVec3d_addInPlace\W(<BalancedArg>,<BalancedArg>)=@CallThroughPointer{$1}Add (@PointerToRef{$2})

!! Three args
bsiDPoint3d_scale\W(<BalancedArg>,<BalancedArg>,<BalancedArg>)=@CallThroughPointer{$1}Scale (@PointerToRef{$2}, $3)
bsiDVec3d_scale\W(<BalancedArg>,<BalancedArg>,<BalancedArg>)=@CallThroughPointer{$1}Scale (@PointerToRef{$2}, $3)
bsiDPoint2d_scale\W(<BalancedArg>,<BalancedArg>,<BalancedArg>)=@CallThroughPointer{$1}Scale (@PointerToRef{$2}, $3)
bsiDPoint3d_subtractDPoint3dDPoint3d\W(<BalancedArg>,<BalancedArg>,<BalancedArg>)=@CallThroughPointer{$1}DifferenceOf (@PointerToRef{$2}, @PointerToRef{$3})
bsiDVec3d_subtractDPoint3dDPoint3d\W(<BalancedArg>,<BalancedArg>,<BalancedArg>)=@CallThroughPointer{$1}DifferenceOf (@PointerToRef{$2}, @PointerToRef{$3})
bsiDPoint3d_crossProduct\W(<BalancedArg>,<BalancedArg>,<BalancedArg>)=@CallThroughPointer{$1}CrossProduct (@PointerToRef{$2}, @PointerToRef{$3})
bsiDVec3d_crossProduct\W(<BalancedArg>,<BalancedArg>,<BalancedArg>)=@CallThroughPointer{$1}CrossProduct (@PointerToRef{$2}, @PointerToRef{$3})
bsiDVec3d_normalizedCrossProduct\W(<BalancedArg>,<BalancedArg>,<BalancedArg>)=@CallThroughPointer{$1}NormalizedCrossProduct (@PointerToRef{$2}, @PointerToRef{$3})


bsiDPoint3d_computeNormal\W(<BalancedArg>,<BalancedArg>,<BalancedArg>)=@CallThroughPointer{$1}NormalizedDifference (@PointerToRef{$2}, @PointerToRef{$3})
bsiDPoint3d_addDPoint3dDPoint3d\W(<BalancedArg>,<BalancedArg>,<BalancedArg>)=@CallThroughPointer{$1}SumOf (@PointerToRef{$2}, @PointerToRef{$3})
bsiDPoint4d_initFromDPoint3dAndWeight\W(<BalancedArg>,<BalancedArg>,<BalancedArg>)=@CallThroughPointer{$1}InitFrom (@PointerToRef{$2}, $3)
bsiDPoint3d_pointEqualTolerance\W(<BalancedArg>,<BalancedArg>,<BalancedArg>)=@CallThroughPointer{$1}IsEqual (@PointerToRef{$2}, $3)

bsiTransform_initFromMatrixAndTranslation\W(<BalancedArg>,<BalancedArg>,<BalancedArg>)=@CallThroughPointer{$1}InitFrom (@PointerToRef{$2}, @PointerToRef{$3})

bsiDRange3d_scaleAboutCenter\W(<BalancedArg>,<BalancedArg>,<BalancedArg>)=@CallThroughPointer{$1}ScaleAboutCenter(@PointerToRef{$2}, $3)

bsiDPoint3d_addDPoint3dArray\W(<BalancedArg>,<BalancedArg>,<BalancedArg>)=DPoint3d\:\:AddToArray ($1, $3, @PointerToRef{$2})

!! Four args
bsiDPoint3d_addScaledDVec3d\W(<BalancedArg>,<BalancedArg>,<BalancedArg>,<BalancedArg>)=@CallThroughPointer{$1}SumOf (@PointerToRef{$2}, @PointerToRef{$3}, $4)
bsiDPoint3d_addScaledDPoint3d\W(<BalancedArg>,<BalancedArg>,<BalancedArg>,<BalancedArg>)=@CallThroughPointer{$1}SumOf (@PointerToRef{$2}, @PointerToRef{$3}, $4)
bsiDPoint3d_interpolate\W(<BalancedArg>,<BalancedArg>,<BalancedArg>,<BalancedArg>)=@CallThroughPointer{$1}Interpolate (@PointerToRef{$2}, $3, @PointerToRef{$4})


bsiRotMatrix_multiplyDPoint3dArray\W(<BalancedArg>,<BalancedArg>,<BalancedArg>,<BalancedArg>)=@CallThroughPointer{$1}Multiply ($2, $3, $4)
bsiRotMatrix_multiplyTransposeDPoint3dArray\W(<BalancedArg>,<BalancedArg>,<BalancedArg>,<BalancedArg>)=@CallThroughPointer{$1}MultiplyTranspose ($2, $3, $4)

bsiTransform_multiplyDPoint3dArray\W(<BalancedArg>,<BalancedArg>,<BalancedArg>,<BalancedArg>)=@CallThroughPointer{$1}Multiply ($2, $3, $4)


bsiRotMatrix_initFromRowVectors\W(<BalancedArg>,<BalancedArg>,<BalancedArg>,<BalancedArg>)=@CallThroughPointer{$1}InitFromRowVectors (@PointerToRef{$2}, @PointerToRef{$3}, @PointerToRef{$4})

!! Leading parse -- convert arg1 to instance call, trailing args unchanged.
bsiRotMatrix_initFromRowValues\W(<BalancedArg>,=@CallThroughPointer{$1}InitFromRowValues (
bsiTransform_initFromRowValues\W(<BalancedArg>,=@CallThroughPointer{$1}InitFromRowValues (

bsiDPoint3d_setXY\W(<BalancedArg>,=@CallThroughPointer{$1}Init (
bsiDPoint3d_setXYZ\W(<BalancedArg>,=@CallThroughPointer{$1}Init (

bsiTransform_multiplyComponents\W(<BalancedArg>,<BalancedArg>,=@CallThroughPointer{$1}Multiply (@PointerToRef{$2},


!! Prodedure becomes static method ...
bsiDPoint3d_getLargestXYCoordinate\W(<BalancedArg>,<BalancedArg>)=DPoint3dOps\:\:LargestXYCoordinate ($1, @CastToSizeT{$2})

?=?

CallThroughPointer:\A\W<I>\W\+<u>\Z=$1\[$2\].
CallThroughPointer:\A\W\&\W<I>\W\Z=$1.
CallThroughPointer:\A\W<I>\Z=$1-\>
CallThroughPointer:\A\W\&<I>-\><I>\Z=$1-\>$2.
CallThroughPointer:\A\W\&<I>\W\[<BalancedText>\]\W\Z=$1\[$2\].
CallThroughPointer:\A\W\&<u>=($1).
CallThroughPointer:<u>=($0)-\>

PointerToRef:\A\W<I>\W\+<u>\Z=$1\[$2\]
PointerToRef:\A\W\&<I>\Z=$1
PointerToRef:\A\W\&<I>\W\[<BalancedText>\]\Z=$1\[$2\]
PointerToRef:\A\W\&<I>-\><I>\Z=$1-\>$2
PointerToRef:\A\W<I>\W\Z=\*$1
PointerToRef:\A<u>\Z=\*($0)



CastToSizeT:\A\W<N>\W\Z=$0!! compiler will convert explicit integer
CastToSizeT:\A\W<I>\W\Z=(size_t)$1!! no parentheses needed on bare identifier
CastToSizeT:\A<u>\Z=(size_t)($1)!! otherwise wrap carefully
