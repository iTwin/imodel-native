\Iconst VArray_Int\I\W\*=EmbeddedIntArrayConstP\
\IVArray_Int\I\W\*=EmbeddedIntArrayP\
jmdlVArrayInt_grab\I=jmdlEmbeddedIntArray_grab
jmdlVArrayInt_drop\I=jmdlEmbeddedIntArray_drop

SUCCESS \=\= jmdlVArrayInt_getInt\I=jmdlEmbeddedIntArray_getInt
SUCCESS \=\= jmdlVArrayInt_setInt\I=jmdlEmbeddedIntArray_setInt
SUCCESS \=\= jmdlVArrayInt_addArray\I=jmdlEmbeddedIntArray_addIntArray

SUCCESS \=\= jmdlVArrayDPoint3d_getDPoint3d\I=jmdlEmbeddedDPoint3dArray_getDPoint3d
SUCCESS \=\= jmdlVArrayDPoint3d_addPoint\I=jmdlEmbeddedDPoint3dArray_addDPoint3d

SUCCESS \!\= jmdlVArrayInt_getInt\I=\!jmdlEmbeddedIntArray_getInt
SUCCESS \!\= jmdlVArrayInt_setInt\I=\!jmdlEmbeddedIntArray_setInt
SUCCESS \!\= jmdlVArrayInt_set\I=\!jmdlEmbeddedIntArray_setInt
SUCCESS \!\= jmdlVArrayInt_addArray\I=\!jmdlEmbeddedIntArray_addIntArray
SUCCESS \!\= jmdlVArrayInt_copy\I=\!jmdlEmbeddedIntArray_copy
SUCCESS \!\= jmdlVArrayDPoint3d_copy\I=\!jmdlEmbeddedDPoint3dArray_copy
SUCCESS \!\= jmdlVArrayDPoint3d_getDPoint3d\I=\!jmdlEmbeddedDPoint3dArray_getDPoint3d

! for setInt, semicolon on same line prevents bool/statusInt confusion in if tests.
\LjmdlVArrayInt_set\I<u>\;=jmdlEmbeddedIntArray_setInt$1\;
\LjmdlVArrayDPoint3d_setDPoint3d\I<u>\;=jmdlEmbeddedDPoint3dArray_setDPoint3d$1\;
\LjmdlVArrayDPoint3d_getDPoint3d\I<u>\;=jmdlEmbeddedDPoint3dArray_getDPoint3d$1\;
\LjmdlVArrayDPoint3d_addPoint\I<u>\;=jmdlEmbeddedDPoint3dArray_addDPoint3d$1\;
\LjmdlVArrayDPoint3d_setExactBufferSize\I<u>\;=jmdlEmbeddedDPoint3dArray_setExactBufferSize$1\;
\LjmdlVArrayInt_addInt\I<u>\;=jmdlEmbeddedIntArray_addInt$1\;
\LjmdlVArrayInt_add3Int\I<u>\;=jmdlEmbeddedIntArray_add3Int$1\;
\LjmdlVArrayInt_getInt\I<u>\;=jmdlEmbeddedIntArray_getInt$1\;

\Iconst VArray_DPoint3d\I\W\*=EmbeddedDPoint3dArrayConstP\
\IVArray_DPoint3d\I\W\*=EmbeddedDPoint3dArrayP\
jmdlVArrayDPoint3d_grab\I=jmdlEmbeddedDPoint3dArray_grab
jmdlVArrayDPoint3d_drop\I=jmdlEmbeddedDPoint3dArray_drop

jmdlVArrayDPoint3d_empty\I=jmdlEmbeddedDPoint3dArray_empty\I
jmdlVArrayInt_empty\I=jmdlEmbeddedIntArray_empty\I

jmdlVArrayDPoint3d_clear\I=jmdlEmbeddedDPoint3dArray_empty\I
jmdlVArrayInt_clear\I=jmdlEmbeddedIntArray_empty\I

jmdlVArrayDPoint3d_getCount\I=jmdlEmbeddedDPoint3dArray_getCount\I
jmdlVArrayInt_getCount\I=jmdlEmbeddedIntArray_getCount\I

jmdlVArrayDPoint3d_getPtr\I=jmdlEmbeddedDPoint3dArray_getPtr\I
jmdlVArrayInt_getPtr\I=jmdlEmbeddedIntArray_getPtr\I

jmdlVArrayDPoint3d_getConstPtr\I=jmdlEmbeddedDPoint3dArray_getConstPtr\I
jmdlVArrayInt_getConstPtr\I=jmdlEmbeddedIntArray_getConstPtr\I



?=?

