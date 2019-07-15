
rem ---- bring files that depend on dtransform3d.c and dmatrix3d.c up to date.

set __BUILD=1
if /i "%1" NEQ "nobuild" goto endBuildParam
    set __BUILD=0
    shift
    :endBuildParam

attrib -r structs\transform.c
attrib -r structs\rotmatrix.c
attrib -r include\rotmatrix.fdf
attrib -r include\transform.fdf
attrib -r include\dmatrix3d.fdf
attrib -r include\dtransform3d.fdf

copy structs\dtransform3d.c structs\transform.c
copy structs\dmatrix3d.c structs\rotmatrix.c

call gemageom transform transform
call gemageom transform rotmatrix

call fdf transform
call fdf rotmatrix
call fdf dtransform3d
call fdf dmatrix3d

if %__BUILD% EQU 1 bmake -dmethods -ddls bsibasegeom
