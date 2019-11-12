/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <BRepCore/SolidKernel.h>
#include <BRepCore/PSolidUtil.h>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_DGN

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Deepak.Malkan   04/96
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidUtil::Boolean
(
bvector<PK_BODY_t>*     resultBodies,       // <= vector of resultant bodies
PK_boolean_function_t   boolOpIn,           // => input boolean operation type
bool                    generalTopology,    // => allow non reqular result (normally false!)
PK_BODY_t&              blankBodyIn,        // => input blank body (may be same as resultant body)
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
    bool                allowDisjoint = 0 != (booleanOptions & PKI_BOOLEAN_OPTION_AllowDisjoint);
    bool                sheetSolidFenceNone = 0 != (booleanOptions & PKI_BOOLEAN_OPTION_SheetSolidFenceNone);

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

    if (SUCCESS == failureCode)
        {
        if (PK_boolean_result_failed_c != results.result)
            {
            if (1 == results.n_bodies)
                {
                PK_BODY_type_t body_type;

                PK_BODY_ask_type(results.bodies[0], &body_type);

                if (PK_BODY_type_empty_c == body_type) // TR349612
                    failureCode = PK_boolean_result_failed_c;
                }
            else if (nullptr == resultBodies && results.n_bodies > 1)
                {
                failureCode = PK_boolean_result_failed_c; // Require resultBodies to return disjoint result as separate bodies...
                }
            }
        else
            {
            failureCode = PK_boolean_result_failed_c;
            }
        }

    if (SUCCESS != failureCode)
        {
        PK_MARK_goto (boolStartMark);
        }
    else
        {
        if (0 == results.n_bodies)
            blankBodyIn = PK_ENTITY_null; // Target fully consumed, tag no longer valid...

        if (nullptr != resultBodies && results.n_bodies > 0)
            resultBodies->insert(resultBodies->end(), &results.bodies[0], &results.bodies[results.n_bodies]);
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
BentleyStatus PSolidUtil::DoBoolean(IBRepEntityPtr& targetEntity, IBRepEntityPtr* toolEntities, size_t nTools, PK_boolean_function_t operation, PKIBooleanOptionEnum options, bool resolveNodeIdConflicts)
    {
    if (0 == nTools)
        return ERROR;

    PK_ENTITY_t targetEntityTag = (targetEntity.IsValid() ? PSolidUtil::GetEntityTagForModify(*targetEntity) : PK_ENTITY_null);

    if (PK_ENTITY_null == targetEntityTag)
        return ERROR;

    PK_MARK_t            markTag = PK_ENTITY_null;
    bvector<PK_ENTITY_t> toolEntityTags;

    PK_MARK_create(&markTag);

    Transform   invTargetTransform;

    invTargetTransform.InverseOf(targetEntity->GetEntityTransform());

    IFaceMaterialAttachmentsP targetAttachments = targetEntity->GetFaceMaterialAttachmentsP();

    // Get tool bodies in coordinates of target...
    for (size_t iTool = 0; iTool < nTools; ++iTool)
        {
        PK_ENTITY_t toolEntityTag = PSolidUtil::GetEntityTagForModify(*toolEntities[iTool]);

        if (PK_ENTITY_null == toolEntityTag)
            continue;

        IFaceMaterialAttachmentsP toolAttachments = toolEntities[iTool]->GetFaceMaterialAttachmentsP();

        if (nullptr != toolAttachments)
            {
            T_FaceAttachmentsVec const& toolFaceAttachmentsVec = toolAttachments->_GetFaceAttachmentsVec();

            if (nullptr == targetAttachments)
                {
                FaceAttachment baseAttachment; // Don't care, replaced with tool attachments...
                IFaceMaterialAttachmentsPtr newAttachments = PSolidUtil::CreateNewFaceAttachments(targetEntityTag, baseAttachment);

                if (newAttachments.IsValid())
                    {
                    newAttachments->_GetFaceAttachmentsVecR() = toolFaceAttachmentsVec;
                    PSolidUtil::SetFaceAttachments(*targetEntity, newAttachments.get());
                    targetAttachments = targetEntity->GetFaceMaterialAttachmentsP();
                    }
                else
                    {
                    PSolidAttrib::DeleteFaceMaterialIndexAttribute(toolEntityTag);
                    }
                }
            else
                {
                T_FaceAttachmentsVec& targetFaceAttachmentsVec = targetAttachments->_GetFaceAttachmentsVecR();
                bvector<size_t> toolToTargetIndex;
                bool indexRemapRequired = false;

                toolToTargetIndex.insert(toolToTargetIndex.begin(), toolFaceAttachmentsVec.size(), 0);

                for (size_t iToolAttachIdx = 0; iToolAttachIdx < toolFaceAttachmentsVec.size(); ++iToolAttachIdx)
                    {
                    FaceAttachment  toolFaceAttachment = toolFaceAttachmentsVec.at(iToolAttachIdx);
                    size_t          targetAttachIdx = 0;

                    T_FaceAttachmentsVec::iterator foundTargetAttachment = std::find(targetFaceAttachmentsVec.begin(), targetFaceAttachmentsVec.end(), toolFaceAttachment);

                    if (foundTargetAttachment == targetFaceAttachmentsVec.end())
                        {
                        targetFaceAttachmentsVec.push_back(toolFaceAttachment);
                        targetAttachIdx = targetFaceAttachmentsVec.size()-1;
                        }
                    else
                        {
                        targetAttachIdx = std::distance(targetFaceAttachmentsVec.begin(), foundTargetAttachment);
                        }

                    toolToTargetIndex[iToolAttachIdx] = targetAttachIdx;

                    if (iToolAttachIdx != targetAttachIdx)
                        indexRemapRequired = true;
                    }

                if (indexRemapRequired)
                    {
                    T_FaceToAttachmentIndexMap toolFaceToIndexMap;

                    PSolidAttrib::PopulateFaceMaterialIndexMap(toolFaceToIndexMap, toolEntityTag, targetFaceAttachmentsVec.size());

                    for (T_FaceToAttachmentIndexMap::const_iterator curr = toolFaceToIndexMap.begin(); curr != toolFaceToIndexMap.end(); ++curr)
                        {
                        size_t remappedIndex = toolToTargetIndex.at(curr->second);

                        if (remappedIndex == curr->second)
                            continue;

                        PSolidAttrib::SetFaceMaterialIndexAttribute(curr->first, (int32_t) remappedIndex);
                        }
                    }
                }
            }

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

    BentleyStatus   status = (SUCCESS == PSolidUtil::Boolean(nullptr, operation, false, targetEntityTag, &toolEntityTags.front(), (int) toolEntityTags.size(), options) ? SUCCESS : ERROR);

    if (SUCCESS == status)
        {
        if (PK_ENTITY_null == targetEntityTag)
            targetEntity = nullptr; // NOTE: Don't extract from original...need to support dynamics rollback for push/pull when calling BooleanCut to punch holes...

        // Invalidate tool entities that were consumed in boolean...
        for (size_t iTool = 0; iTool < nTools; ++iTool)
            toolEntities[iTool] = nullptr;
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
* @bsimethod                                                    Brien.Bastings  11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus PSolidUtil::ImprintCurves(PK_ENTITY_t targetTag, bvector<PK_CURVE_t> const& toolCurves, bvector<PK_INTERVAL_t> const& toolIntervals, DVec3dCP direction, bool extend, bool connectSides)
    {
    if (PK_ENTITY_null == targetTag)
        return ERROR;

    if (toolCurves.empty() || (toolCurves.size() != toolIntervals.size()))
        return ERROR;

    PK_CURVE_project_o_t options;
    PK_CURVE_project_r_t results;
    PK_ENTITY_track_r_t  tracking;

    PK_CURVE_project_o_m(options);

    options.function = PK_proj_function_imprint_c;
    options.complete = extend ? PK_proj_complete_edge_c : PK_proj_complete_no_c;

    if (nullptr != direction)
        {
        direction->GetComponents(options.direction.coord[0], options.direction.coord[1], options.direction.coord[2]);
        options.have_direction = PK_LOGICAL_true;
        options.bidirectional = PK_LOGICAL_true;
        options.connect = connectSides ? PK_proj_connect_side_c : PK_proj_connect_none_c;
        }

    memset(&results, 0, sizeof(results));
    memset(&tracking, 0, sizeof(tracking));

    PK_MARK_t   markTag = PK_ENTITY_null;

    PK_MARK_create(&markTag);

    BentleyStatus status = (SUCCESS == PK_CURVE_project((int) toolCurves.size(), &toolCurves.front(), &toolIntervals.front(), 1, &targetTag, &options, &results, &tracking)) ? SUCCESS : ERROR;

    if (SUCCESS == status && extend && tracking.n_track_records > 1)
        {
        bvector<PK_EDGE_t> edges;

        for (int iResult = 0; iResult < tracking.n_track_records; ++iResult)
            {
            for (int iProduct = 0; iProduct < tracking.track_records[iResult].n_product_entities; ++iProduct)
                {
                PK_ENTITY_t entityTag = tracking.track_records[iResult].product_entities[iProduct];
                PK_CLASS_t  entityClass;

                PK_ENTITY_ask_class(entityTag, &entityClass);

                if (PK_CLASS_edge == entityClass)
                    edges.push_back(entityTag);
                }
            }

        if (edges.size() > 1)
            {
            PK_TOPOL_track_r_t tracking2;
            PK_TOPOL_delete_redundant_2_o_s options2;

            memset(&tracking2, 0, sizeof(tracking2));
            PK_TOPOL_delete_redundant_2_o_m(options2);
            options2.max_topol_dimension = PK_TOPOL_dimension_0_c; // Only remove vertices...
            options2.scope = PK_redundant_merge_in_c;

            PK_TOPOL_delete_redundant_2((int) edges.size(), &edges.front(), &options2, &tracking2);

            PK_TOPOL_track_r_f(&tracking2);
            }
        }

    // NOTE: I don't think a "feature" should add it's node id to anything but the new edges. PSolidTopoId::AddNodeIdAttributes will resolve the
    //       duplicate face ids after a split and id the new edges. Trying to have the feature "own" a face as per SmartFeatures is problematic
    //       when an open element is used to split a face. Another option might be to add the new node id (don't overwrite) to all the modified
    //       faces...but that's problematic for assigning robust node ids when the target is a body and the faces split come from multiple features.

    PK_ENTITY_track_r_f(&tracking);
    PK_CURVE_project_r_f(&results);

    if (SUCCESS != status)
        PK_MARK_goto(markTag);

    PK_MARK_delete(markTag);

    return status;
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidUtil::FixBlends(PK_BODY_t bodyTag)
    {
    PK_MARK_t   markTag = PK_ENTITY_null;

    PK_MARK_create(&markTag);

    PK_BODY_fix_blends_o_t options;

    PK_BODY_fix_blends_o_m(options);
    options.local_check = PK_LOGICAL_true;

    BentleyStatus    status = SUCCESS;
    int              nBlends = 0;
    PK_EDGE_t*       edges = nullptr;
    PK_FACE_t*       blends = nullptr;
    PK_FACE_array_t* faces = nullptr;
    PK_blend_fault_t fault;
    PK_EDGE_t        faultEdge;
    PK_TOPOL_t       faultTopo;
    PK_ERROR_code_t  failureCode = PK_BODY_fix_blends(bodyTag, &options, &nBlends, &blends, &faces, &edges, &fault, &faultEdge, &faultTopo);

    if (PK_ERROR_no_errors != failureCode || 0 == nBlends || (PK_blend_fault_no_fault_c != fault && PK_blend_fault_repaired_c != fault))
        status = ERROR;

    PK_MEMORY_free(blends);
    PK_MEMORY_free(edges);

    for (int i=0; i < nBlends; i++)
        PK_MEMORY_free(faces[i].array);

    PK_MEMORY_free(faces);

    if (SUCCESS != status)
        PK_MARK_goto(markTag);

    PK_MARK_delete(markTag);

    return status;
    }
