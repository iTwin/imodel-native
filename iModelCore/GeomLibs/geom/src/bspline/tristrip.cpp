/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "msbsplinemaster.h"

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

#define MAX_TRISTRIP_POINTS         30
#define IMPOSSIBLE_INDEX (-1)
typedef int (*TristripOutputFunction)(DPoint3d*,DPoint3d*,DPoint2d*,int,void*);

typedef struct triStrip
    {
    int         index[MAX_TRISTRIP_POINTS];     /* Indices of points */
    int npt;                            /* Number of points in active tristrip */
    DPoint2d    **refParamPP;           /*  array of pointers to uv parameters */
    DPoint3d    *refPointP;             /*  array of pointP coordinates*/
    DPoint3d    *refNormalP;            /*  array of normal coordinates */
    DPoint2d    *scaleP;                /*  pointer to a single vector of xy scale
                                                factors to be applied to all param
                                                vectors */
    void *userDataP;            /* User defined data pointers */
    TristripOutputFunction outputFunction;      /* Callback to receive buffered points */
    int status;                 /* SUCCESS or ERROR */
    }
Tristrip;

static Tristrip tristrip;       /* A single buffer for the ongoing output */
int     bspmesh_nTriStrip = 0;  /* number of tristrips output */
int     bspmesh_nTriangle = 0;  /* number of triangles output */


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    earlinLutz      10/94
+---------------+---------------+---------------+---------------+---------------+------*/
/*Rotate indices 0,1,2 in the tristrip so that the one at i goes to 1.
It is assumed that index 3 alreay
matches index 0 and index 4 may be overwritten to allow quick
copy back to 0'th position without worrying about saving a temporary */
static void tristrip_rotate
(
int i           /* The local index of the base point of the rotated triangle */
/*This is assumed to be 0,1,or 2. */
)
    {
    if(i !=  1)
        {
        int *source = &tristrip.index[i == 2 ? 1 : 2];
        int *dest  = &tristrip.index[0];
        tristrip.index[4] = tristrip.index[1];
        *dest++ = *source++;
        *dest++ = *source++;
        *dest++ = *source++;
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    earlinLutz      10/94
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void tristrip_init       /* initialize the tristrip buffer */
(
DPoint2d **refParamPP,
DPoint3d *refPointP,
DPoint3d *refNormalP,
int (*triangleFunction)(),
void *userDataP,
DPoint2d *scaleP
)
    {
    tristrip.refParamPP = refParamPP;
    tristrip.refPointP = refPointP;
    tristrip.refNormalP = refNormalP;
    tristrip.scaleP = scaleP;
    tristrip.npt = 0;
    tristrip.outputFunction = (TristripOutputFunction)triangleFunction;
    tristrip.userDataP = userDataP;
    tristrip.status = SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    earlinLutz      10/94
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int tristrip_addTriangle         /* Put a (possibly permuted) triangle in the buffer */
(
int i0,         /* First vertex index */
int i1,         /* Second vertex index */
int i2          /* Third vertex index */
)
    {
    int stat = 0;
    int connect_id = IMPOSSIBLE_INDEX;
    if(tristrip.status != SUCCESS)
        {
        return tristrip.status;
        }
    else if (tristrip.npt < 3)
        {
        /*      There is no prior triangle to consider attaching to  */
        /*      This block is put first with an explicit test because*/
        /*      it is the most common single case                    */
        }
    if( (tristrip.npt > 4) && (tristrip.npt < MAX_TRISTRIP_POINTS) )
        {
        /*      This is the second most common case */
        int j0,j1;
        int n = tristrip.npt;
        if(n & 0x01)
            {
            j1 = tristrip.index[n-2];
            j0 = tristrip.index[n-1];
            }
        else
            {
            j0 = tristrip.index[n-2];
            j1 = tristrip.index[n-1];
            }

        if (i0 == j0 && i1 == j1)
            {
            connect_id = i2;
            }
        else if(i1==j0 && i2==j1)
            {
            connect_id = i0;
            }
        else if(i2==j0 && i0==j1)
            {
            connect_id = i1;
            }
        }
    else if(tristrip.npt == 3)
        {
        /*      Compare each (reversed) edge of the new triangle to
        each edge of the first one.  If necessary, rotate the
        first one into a better position for attaching.  Note
        that the first triangle is assumed to have index[3]==index[0].
        */
        int i,j0;
        for(i = 0;i<3;i++)
            {
            j0 = tristrip.index[i];
            if(j0 == i0 && tristrip.index[i+1] == i2)
                {
                connect_id = i1;
                tristrip_rotate(i);
                i = 3;
                }
            else if(j0 == i1 && tristrip.index[i+1] == i0)
                {
                connect_id = i2;
                tristrip_rotate(i);
                i = 3;
                }
            else if(j0 == i2 && tristrip.index[i+1] == i1)
                {
                connect_id = i0;
                tristrip_rotate(i);
                i = 3;
                }
            }
        }
    else if(tristrip.npt == 4)
        {
        /*      If 0 2 1 3 exist,                           */
        /*      4 3 2 can be added as 0 0 1 2 3 4.          */
        /*      2 4 3 can be added as 0 0 1 2 3 4.          */
        /*      3 2 4 can be added as 0 0 1 2 3 4.          */
        /*      The extra 0 in the begining is to maintain  */
        /*      the vertices  direction.                    */

        if ( (i1 == tristrip.index[3]) &&
             (i2 == tristrip.index[1]) )
            {
            tristrip.index[4] = tristrip.index[3];
            tristrip.index[3] = tristrip.index[1];
            tristrip.index[1] = tristrip.index[0];
            connect_id = i0;
            tristrip.npt++;
            }
        else if ( (i0 == tristrip.index[1]) &&
                  (i2 == tristrip.index[3]) )
            {
            tristrip.index[4] = tristrip.index[3];
            tristrip.index[3] = tristrip.index[1];
            tristrip.index[1] = tristrip.index[0];
            connect_id = i1;
            tristrip.npt++;
            }
        else if ( (i0 == tristrip.index[3]) &&
                  (i1 == tristrip.index[1]) )
            {
            tristrip.index[4] = tristrip.index[3];
            tristrip.index[3] = tristrip.index[1];
            tristrip.index[1] = tristrip.index[0];
            connect_id = i2;
            tristrip.npt++;
            }
        else
            {
            int j0,j1;
            int n = tristrip.npt;
            if(n & 0x01)
                {
                j1 = tristrip.index[n-2];
                j0 = tristrip.index[n-1];
                }
            else
                {
                j0 = tristrip.index[n-2];
                j1 = tristrip.index[n-1];
                }
            if (i0 == j0 && i1 == j1)
                {
                connect_id = i2;
                }
            else if(i1==j0 && i2==j1)
                {
                connect_id = i0;
                }
            else if(i2==j0 && i0==j1)
                {
                connect_id = i1;
                }
            }
        }

    if(connect_id == IMPOSSIBLE_INDEX)
        {
        /*      Clear out all prior points ... */
        if(tristrip.npt > 0)
            {
            stat = tristrip_flush();
            }
        /*      and insert the new ones right at the start */
        tristrip.index[0] = i0;
        tristrip.index[1] = i1;
        tristrip.index[2] = i2;
        /*      Force point 0 in as (uncounted) point 3 to save a wraparound check
        one the next triangle comes in */
        tristrip.index[3] = i0;
        tristrip.npt = 3;
        }
    else
        {
        tristrip.index[tristrip.npt++] = connect_id;
        }
    return stat;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    earlinLutz      10/94
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int tristrip_flush       /* Flush the tristrip buffer */
(
)
    {
    DPoint2d bufferParamP[MAX_TRISTRIP_POINTS],**uvPP;
    DPoint3d bufferPointP[MAX_TRISTRIP_POINTS];
    DPoint3d bufferNormalP[MAX_TRISTRIP_POINTS];
    int i;
    int stat = 0;
    DPoint3d *pointP,*normalP;
    if (tristrip.status != SUCCESS)
        {
        stat = tristrip.status;
        tristrip.npt = 0;
        }
    else if(tristrip.npt >  0 && tristrip.outputFunction)
        {
        /*      Collect up the arrays of pointP and normalP */
        DPoint2d* outputParams = NULL;
        DPoint3d* outputNormals = NULL;
        if(uvPP = tristrip.refParamPP)
            {
            outputParams = bufferParamP;
            for(i = 0;i<tristrip.npt;i++)
                {
                DPoint2d *uvP = uvPP[tristrip.index[i]];
                bufferParamP[i].x =  uvP->x * tristrip.scaleP->x;
                bufferParamP[i].y =  uvP->y * tristrip.scaleP->y;
                }
            }
        if(pointP = tristrip.refPointP)
            {
            for(i = 0;i<tristrip.npt;i++) bufferPointP[i] = pointP[tristrip.index[i]];
            }
        if(normalP = tristrip.refNormalP)
            {
            outputNormals = bufferNormalP;
            for(i = 0;i<tristrip.npt;i++) bufferNormalP[i] = normalP[tristrip.index[i]];
            }

        bspmesh_nTriangle +=  tristrip.npt - 2;
        bspmesh_nTriStrip +=  1;
        stat = tristrip.status = tristrip.outputFunction(
                        bufferPointP, outputNormals, outputParams,
                        tristrip.npt,tristrip.userDataP);

        tristrip.npt = 0;

        }
    return stat;
    }

/*======================================================================+
|                                                                       |
|   Minor Code Section                                                  |
|                                                                       |
+======================================================================*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    earlinLutz      10/94
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bspmesh_startCounters       /* Clear tristrip statistics counters */
(
)
    {
    bspmesh_nTriangle = bspmesh_nTriStrip = 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    earlinLutz      10/94
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bspmesh_summaryCounters
(
)
    {
    }

END_BENTLEY_GEOMETRY_NAMESPACE
