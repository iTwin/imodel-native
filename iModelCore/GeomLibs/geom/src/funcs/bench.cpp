/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/funcs/bench.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------+
|                                                                       |
|   Include Files                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*-----------------------------------------------------------------*//**
* @bsimethod                                                    EarlinLutz      03/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiGeom_noop0

(
)
    {

    }


/*-----------------------------------------------------------------*//**
* @bsimethod                                                    EarlinLutz      03/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiGeom_noopD

(
double  x
)
    {

    }


/*-----------------------------------------------------------------*//**
* @bsimethod                                                    EarlinLutz      03/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiGeom_noopDD

(
double x,
double y
)
    {

    }


/*-----------------------------------------------------------------*//**
* @bsimethod                                                    EarlinLutz      03/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiGeom_noopDDD

(
double x,
double y,
double z
)
    {

    }



/*-----------------------------------------------------------------*//**
* @bsimethod                                                    EarlinLutz      03/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiGeom_repeatMatrixMatrix

(
RotMatrixCP pA,
RotMatrixCP pB,
int count
)
    {
    int i;
    RotMatrix C;
    for (i = 0; i < count; i++)
        {
        bsiRotMatrix_multiplyRotMatrixRotMatrix (&C, pA, pB);
        }
    }
END_BENTLEY_GEOMETRY_NAMESPACE
