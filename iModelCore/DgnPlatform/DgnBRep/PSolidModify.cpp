/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnBRep/PSolidModify.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnBRep/PSolidUtil.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Deepak.Malkan   04/96
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidUtil::Boolean
(
PK_BODY_t**             ppResultBodies,     // <= array of resultant bodies
int*                    pNumResultBodies,   // <= number of manifold bodies in result
PK_boolean_function_t   boolOpIn,           // => input boolean operation type
bool                    generalTopology,    // => allow non reqular result (normally false!)
PK_BODY_t               blankBodyIn,        // => input blank body (may be same as resultant body)
PK_BODY_t*              pToolBodies,        // => input tool bodies (consumed)
int                     numToolBodiesIn,    // => input tool bodies count
PKIBooleanOptionEnum    booleanOptions      // => options for boolean
)
    {
    int                 failureCode;
    int                 boolStartMark;
    PK_BODY_boolean_o_t options;
    PK_TOPOL_track_r_t  tracking;
    PK_boolean_r_t      results;
    bool                allowDisjoint = TO_BOOL (booleanOptions & PKI_BOOLEAN_OPTION_AllowDisjoint);
    bool                sheetSolidFenceNone = TO_BOOL (booleanOptions & PKI_BOOLEAN_OPTION_SheetSolidFenceNone);

    memset (&tracking, 0, sizeof (tracking));
    memset (&results, 0, sizeof (results));

    // Set defaults for option structure
    PK_BODY_boolean_o_m (options);

    // Set merge imprinted to true
    options.merge_imprinted = PK_LOGICAL_true;

    // Disjoint result returned as 1 body
    options.allow_disjoint = allowDisjoint;

    if (generalTopology)
        PK_SESSION_set_general_topology (PK_LOGICAL_true);

    // Fence option to only return one side for solid/sheet trimming
    if (boolOpIn == PK_boolean_subtract)
        {
        PK_BODY_type_t      bodyType = 0;

        PK_BODY_ask_type (blankBodyIn, &bodyType);

        if (PK_BODY_type_solid_c == bodyType)
            {
            options.fence = PK_boolean_fence_front_c;
            }
        else if (!sheetSolidFenceNone)
            {
            for (int i=0; i<numToolBodiesIn; i++)
                {
                PK_BODY_ask_type (pToolBodies[i], &bodyType);

                if (PK_BODY_type_solid_c == bodyType)
                    {
                    options.fence = PK_boolean_fence_front_c;
                    break;
                    }
                }
            }
        }

    // Now set the specific boolean operation.
    options.function = boolOpIn;

    // Create pre-boolean session mark
    PK_MARK_create (&boolStartMark);

    failureCode = PK_BODY_boolean_2 (blankBodyIn, numToolBodiesIn, pToolBodies, &options, &tracking, &results);

    // NOTE: PK_boolean_result_no_clash_C is not considered a failure condition...
    if (SUCCESS != failureCode || 0 == results.n_bodies)
        {
        failureCode = failureCode ? failureCode : ERROR;
        }
    else if (SUCCESS == failureCode && results.result != PK_boolean_result_success_c)
        {
        failureCode = PK_boolean_result_failed_c == results.result ? PK_boolean_result_failed_c : SUCCESS;
        }
    else
        {
        PK_BODY_type_t body_type;

        if (1 == results.n_bodies)
            {
            PK_BODY_ask_type(results.bodies[0], &body_type);
            
            if (PK_BODY_type_empty_c == body_type) // TR349612
                failureCode = PK_boolean_result_failed_c;
            }
        }

    if (SUCCESS != failureCode)
        {
        if (pNumResultBodies)
            *pNumResultBodies = 0;

        if (ppResultBodies)
            *ppResultBodies = NULL;

        PK_MARK_goto (boolStartMark);
        }
    else
        {
        if (pNumResultBodies)
            *pNumResultBodies = results.n_bodies;

        if (ppResultBodies)
            {
            PK_MEMORY_alloc (results.n_bodies * sizeof (PK_BODY_t), (void **) ppResultBodies);
            memcpy (*ppResultBodies, &results.bodies[0], results.n_bodies * sizeof (PK_BODY_t));
            }
        }

    PK_boolean_r_f (&results);
    PK_TOPOL_track_r_f (&tracking);

    PK_MARK_delete (boolStartMark);

    if (generalTopology)
        PK_SESSION_set_general_topology (PK_LOGICAL_false);

    return (BentleyStatus) failureCode;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      11/07
+---------------+---------------+---------------+---------------+---------------+------*/
void PSolidUtil::ConvertSolidBodyToSheet (PK_BODY_t body)
    {
    PK_BODY_type_t  bodyType;

    PK_BODY_ask_type (body, &bodyType);

    if (bodyType != PK_BODY_type_solid_c)
         return;

    int                 nRegions;
    PK_REGION_t*        regions = NULL;
    PK_BODY_ask_regions (body, &nRegions, &regions);

    for  (int i=0; i<nRegions; i++)
        PK_REGION_make_void (regions[i]);

    PK_MEMORY_free (regions);
    }


