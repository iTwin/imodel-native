@echo off
@if .%1 EQU .debug echo on

rem gema -f cgToProto.g in\LineSegment.fields >proto\LineSegment.proto
rem type proto\lineSegment.proto

for %%f in (in\*.fields) do gema -f cgToProto.g %%f  proto\%%~nf.proto

del allcg.proto
rem for %%f in (proto\*.proto) do type *f

rem copy in\head.proto+proto\*.proto  allCG.proto

rem copy proto\AdjacentSurfacePatches.proto+proto\Block.proto+proto\BsplineCurve.proto+proto\BsplineSurface.proto+proto\CircularArc.proto+proto\CircularCone.proto+proto\CircularCylinder.proto+proto\CircularDisk.proto+proto\Coordinate.proto+proto\CurveChain.proto+proto\CurveGroup.proto+proto\CurveReference.proto+proto\EllipticArc.proto+proto\EllipticDisk.proto+proto\Group.proto+proto\IndexedMesh.proto+proto\InterpolatingCurve.proto+proto\LineSegment.proto+proto\LineString.proto+proto\Operation.proto+proto\parametricSurfacePatch.proto+proto\PointChain.proto+proto\PointGroup.proto+proto\Polygon.proto+proto\PrimitiveCurveReference.proto+proto\SharedGroupDef.proto+proto\SharedGroupInstance.proto+proto\ShelledSolid.proto+proto\SingleLineText.proto+proto\SkewedCone.proto+proto\SolidBySweptSurface.proto+proto\SolidGroup.proto+proto\Sphere.proto+proto\Spiral.proto+proto\SurfaceBySweptCurve.proto+proto\SurfaceGroup.proto+proto\SurfacePatch.proto+proto\TorusPipe.proto+proto\TransformedGeometry.proto+proto\TransitionSpiral.proto+proto\Vector.proto allcg.proto
type in\head.proto >> allcg.proto
type proto\Block.proto >> allcg.proto
type proto\BsplineCurve.proto >> allcg.proto
type proto\BsplineSurface.proto >> allcg.proto
type proto\CircularArc.proto >> allcg.proto
type proto\CircularCone.proto >> allcg.proto
type proto\CircularCylinder.proto >> allcg.proto
type proto\CircularDisk.proto >> allcg.proto
type proto\Coordinate.proto >> allcg.proto
type proto\CurveReference.proto >> allcg.proto
type proto\EllipticArc.proto >> allcg.proto
type proto\EllipticDisk.proto >> allcg.proto
type proto\IndexedMesh.proto >> allcg.proto
type proto\InterpolatingCurve.proto >> allcg.proto
type proto\LineSegment.proto >> allcg.proto
type proto\LineString.proto >> allcg.proto
type proto\Polygon.proto >> allcg.proto
type proto\PrimitiveCurveReference.proto >> allcg.proto
type proto\SharedGroupInstance.proto >> allcg.proto
type proto\ShelledSolid.proto >> allcg.proto
type proto\SingleLineText.proto >> allcg.proto
type proto\SkewedCone.proto >> allcg.proto
type proto\SolidBySweptSurface.proto >> allcg.proto
type proto\Sphere.proto >> allcg.proto
type proto\Spiral.proto >> allcg.proto
type proto\SurfaceBySweptCurve.proto >> allcg.proto
type proto\TorusPipe.proto >> allcg.proto
type proto\TransformedGeometry.proto >> allcg.proto
type proto\TransitionSpiral.proto >> allcg.proto
type proto\Vector.proto >> allcg.proto
 
type proto\AdjacentSurfacePatches.proto >> allcg.proto
type proto\Group.proto >> allcg.proto
type proto\CurveChain.proto >> allcg.proto
type proto\CurveGroup.proto >> allcg.proto
type proto\Operation.proto >> allcg.proto
type proto\parametricSurfacePatch.proto >> allcg.proto
type proto\PointChain.proto >> allcg.proto
type proto\PointGroup.proto >> allcg.proto
type proto\SharedGroupDef.proto >> allcg.proto
type proto\SolidGroup.proto >> allcg.proto
 
type proto\SurfaceGroup.proto >> allcg.proto
type proto\SurfacePatch.proto >> allcg.proto
 
BeProtoc --cpp_out=cpp allcg.proto

if .%1 EQU .fixup call gemaproc fixup cpp\allcg.pb.cpp
 