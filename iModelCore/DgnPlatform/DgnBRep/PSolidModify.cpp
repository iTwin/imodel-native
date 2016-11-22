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
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus PSolidUtil::DoBoolean(IBRepEntityR targetEntity, IBRepEntityPtr* toolEntities, size_t nTools, PK_boolean_function_t operation, PKIBooleanOptionEnum options, bool resolveNodeIdConflicts)
    {
    if (0 == nTools)
        return ERROR;

    PK_ENTITY_t targetEntityTag = PSolidUtil::GetEntityTagForModify(targetEntity);

    if (PK_ENTITY_null == targetEntityTag)
        return ERROR;

    PK_MARK_t            markTag = PK_ENTITY_null;
    bvector<PK_ENTITY_t> toolEntityTags;

    PK_MARK_create(&markTag);

    Transform   invTargetTransform;
 
    invTargetTransform.InverseOf(targetEntity.GetEntityTransform());

    // Get tool bodies in coordinates of target...
    for (size_t iTool = 0; iTool < nTools; ++iTool)
        {
        PK_ENTITY_t toolEntityTag = PSolidUtil::GetEntityTagForModify(*toolEntities[iTool]);

        if (PK_ENTITY_null == toolEntityTag)
            continue;

        Transform   toolTransform;

        toolTransform.InitProduct(invTargetTransform, toolEntities[iTool]->GetEntityTransform());
        PSolidUtil::TransformBody(toolEntityTag, toolTransform);                                                                                                                                                

        toolEntityTags.push_back(toolEntityTag);
        }

    uint32_t highestNodeId, lowestNodeId;

    // If node ids are assigned to target body, avoid duplicate node ids with tool bodies...otherwise assume caller doesn't care about ids...
    if (resolveNodeIdConflicts && SUCCESS == PSolidTopoId::FindNodeIdRange(targetEntityTag, highestNodeId, lowestNodeId))
        {
        for (PK_ENTITY_t toolEntityTag: toolEntityTags)
            {
            uint32_t highestToolNodeId, lowestToolNodeId;

            if (SUCCESS == PSolidTopoId::FindNodeIdRange(toolEntityTag, highestToolNodeId, lowestToolNodeId))
                {
                if (highestToolNodeId < lowestNodeId)
                    {
                    // No overlap...new lowest found...
                    lowestNodeId = lowestToolNodeId;
                    }
                else if (lowestToolNodeId > highestNodeId)
                    {
                    // No overlap...new highest found...
                    highestNodeId = highestToolNodeId;
                    }
                else
                    {
                    int32_t increment = abs ((int) highestNodeId - (int) lowestToolNodeId) + 1;

                    PSolidTopoId::IncrementNodeIdAttributes(toolEntityTag, increment);
                    highestNodeId = highestToolNodeId + increment;
                    lowestNodeId  = (lowestNodeId < lowestToolNodeId ? lowestNodeId : lowestToolNodeId);
                    }
                }
            else
                {
                PSolidTopoId::AddNodeIdAttributes(toolEntityTag, ++highestNodeId, false);
                }
            }
        }

    BentleyStatus   status = (SUCCESS == PSolidUtil::Boolean (NULL, NULL, operation, false, targetEntityTag, &toolEntityTags.front (), (int) toolEntityTags.size (), options) ? SUCCESS : ERROR);

    if (SUCCESS == status)
        {
        // Invalidate tool entities that were consumed in boolean...
        for (size_t iTool = 0; iTool < nTools; ++iTool)
            PSolidUtil::ExtractEntityTag(*toolEntities[iTool]);
        }
    else
        {
        // Undo copy/transform of input entities...
        PK_MARK_goto(markTag);
        }

    PK_MARK_delete(markTag);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      11/07
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidUtil::ConvertSolidBodyToSheet (PK_BODY_t body)
    {
    PK_BODY_type_t  bodyType;

    PK_BODY_ask_type (body, &bodyType);

    if (bodyType != PK_BODY_type_solid_c)
        return ERROR;

    int                 nRegions;
    PK_REGION_t*        regions = NULL;
    PK_BODY_ask_regions (body, &nRegions, &regions);

    for  (int i=0; i<nRegions; i++)
        PK_REGION_make_void (regions[i]);

    PK_MEMORY_free (regions);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/01
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidUtil::SweepBodyVector (PK_BODY_t bodyTag, DVec3dCR direction, double distance)
    {
    PK_LOGICAL_t    sweptSpun;

    PK_SESSION_ask_swept_spun_surfs (&sweptSpun);
    PK_SESSION_set_swept_spun_surfs (true);

    DVec3d      extrude;

    extrude.ScaleToLength (direction, distance);

    PK_VECTOR_t path;

    path.coord[0] = extrude.x;
    path.coord[1] = extrude.y;
    path.coord[2] = extrude.z;

    StatusInt           status;
    int                 nLaterals;
    PK_TOPOL_t          *lateralP = NULL, *baseP = NULL;
    PK_local_check_t    localCheck;

    if (SUCCESS == (status = PK_BODY_sweep (bodyTag, path, PK_LOGICAL_true, &nLaterals, &lateralP, &baseP, &localCheck)))
        {
        PSolidTopoId::AssignSweptProfileLateralIds (nLaterals, baseP, lateralP); // Propagate ids from profile edges (if any) onto lateral faces...

        if (PK_local_check_ok_c == localCheck)
            {
            int     nGeoms;

            PK_BODY_simplify_geom (bodyTag, PK_LOGICAL_true, &nGeoms, NULL);
            PK_TOPOL_delete_redundant (bodyTag);
            }
        else
            {
            status = ERROR;
            }

        PK_MEMORY_free (lateralP);
        PK_MEMORY_free (baseP);
        }

    PK_SESSION_set_swept_spun_surfs (sweptSpun);

    return (BentleyStatus) status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/01
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidUtil::SweepBodyAxis (PK_BODY_t bodyTag, DVec3dCR revolveAxis, DPoint3dCR radiusPt, double sweep)
    {
    if (0.0 == sweep)
        return ERROR;

    PK_LOGICAL_t    sweptSpun;

    PK_SESSION_ask_swept_spun_surfs (&sweptSpun);
    PK_SESSION_set_swept_spun_surfs (true);

    PK_AXIS1_sf_t   sweepAxis;

    sweepAxis.location.coord[0] = radiusPt.x;
    sweepAxis.location.coord[1] = radiusPt.y;
    sweepAxis.location.coord[2] = radiusPt.z;

    sweepAxis.axis.coord[0] = revolveAxis.x;
    sweepAxis.axis.coord[1] = revolveAxis.y;
    sweepAxis.axis.coord[2] = revolveAxis.z;

    ((DVec3dR) sweepAxis.axis.coord).Normalize ();

    StatusInt           status;
    int                 nLaterals;
    PK_TOPOL_t          *lateralP = NULL, *baseP = NULL;
    PK_local_check_t    localCheck;

    if (SUCCESS == (status = PK_BODY_spin (bodyTag, &sweepAxis, sweep, PK_LOGICAL_true, &nLaterals, &lateralP, &baseP, &localCheck)))
        {
        PSolidTopoId::AssignSweptProfileLateralIds (nLaterals, baseP, lateralP); // Propagate ids from profile edges (if any) onto lateral faces...

        if (PK_local_check_ok_c == localCheck)
            {
            int     nGeoms;

            PK_BODY_simplify_geom (bodyTag, PK_LOGICAL_true, &nGeoms, NULL);
            PK_TOPOL_delete_redundant (bodyTag);
            }
        else
            {
            status = ERROR;
            }

        PK_MEMORY_free (lateralP);
        PK_MEMORY_free (baseP);
        }

    PK_SESSION_set_swept_spun_surfs (sweptSpun);

    return (BentleyStatus) status;
    }


