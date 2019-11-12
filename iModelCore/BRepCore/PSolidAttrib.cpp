/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <BRepCore/SolidKernel.h>
#include <BRepCore/PSolidUtil.h>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_DGN

static const int HIDDEN_ATTRIBUTE_Visible = 0;
static const int HIDDEN_ATTRIBUTE_Hidden  = 1;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus PSolidAttrib::CreateEntityIdAttributeDef(bool isSessionStart)
    {
    if (!isSessionStart)
        {
        PK_ATTDEF_t attribDefTag = PK_ENTITY_null;

        if (SUCCESS == PK_ATTDEF_find(PKI_ENTITY_ID_ATTRIB_NAME, &attribDefTag) && PK_ENTITY_null != attribDefTag)
            return SUCCESS;
        }

    // Create face-id attribute definition
    PK_CLASS_t          entityIdOwnerTypes[] = {PK_CLASS_face, PK_CLASS_edge};
    PK_ATTRIB_field_t   entityIdFieldTypes[] = {PK_ATTRIB_field_integer_c};
    PK_ATTDEF_sf_t      entityIdAttribDefStruct;
    PK_ATTDEF_t         entityIdAttribDefTag;

    entityIdAttribDefStruct.name          = const_cast<CharP>(PKI_ENTITY_ID_ATTRIB_NAME);
    entityIdAttribDefStruct.attdef_class  = PK_ATTDEF_class_06_c;
    entityIdAttribDefStruct.n_owner_types = sizeof(entityIdOwnerTypes) / sizeof(entityIdOwnerTypes[0]);
    entityIdAttribDefStruct.owner_types   = entityIdOwnerTypes;
    entityIdAttribDefStruct.n_fields      = sizeof(entityIdFieldTypes) / sizeof(entityIdFieldTypes[0]);
    entityIdAttribDefStruct.field_types   = entityIdFieldTypes;

    return (PK_ERROR_no_errors == PK_ATTDEF_create(&entityIdAttribDefStruct, &entityIdAttribDefTag) ? SUCCESS : ERROR);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus PSolidAttrib::CreateUserDataAttributeDef(bool isSessionStart)
    {
    if (!isSessionStart)
        {
        PK_ATTDEF_t attribDefTag = PK_ENTITY_null;

        if (SUCCESS == PK_ATTDEF_find(PKI_USERDATA_ATTRIB_NAME, &attribDefTag) && PK_ENTITY_null != attribDefTag)
            return SUCCESS;
        }

    // Create userdata attribute definition
    PK_CLASS_t          userDataOwnerTypes[] = {PK_CLASS_body, PK_CLASS_region, PK_CLASS_shell, PK_CLASS_face, PK_CLASS_loop, PK_CLASS_edge, PK_CLASS_vertex};
    PK_ATTRIB_field_t   userDataFieldTypes[] = {PK_ATTRIB_field_integer_c};
    PK_ATTDEF_sf_t      userDataAttribDefStruct;
    PK_ATTDEF_t         userDataAttribDefTag;

    userDataAttribDefStruct.name          = const_cast<CharP>(PKI_USERDATA_ATTRIB_NAME);
    userDataAttribDefStruct.attdef_class  = PK_ATTDEF_class_06_c;
    userDataAttribDefStruct.n_owner_types = sizeof(userDataOwnerTypes) / sizeof(userDataOwnerTypes[0]);
    userDataAttribDefStruct.owner_types   = userDataOwnerTypes;
    userDataAttribDefStruct.n_fields      = sizeof(userDataFieldTypes) / sizeof(userDataFieldTypes[0]);
    userDataAttribDefStruct.field_types   = userDataFieldTypes;

    return (PK_ERROR_no_errors == PK_ATTDEF_create(&userDataAttribDefStruct, &userDataAttribDefTag) ? SUCCESS : ERROR);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus PSolidAttrib::CreateHiddenEntityAttributeDef(bool isSessionStart)
    {
    if (!isSessionStart)
        {
        PK_ATTDEF_t attribDefTag = PK_ENTITY_null;

        if (SUCCESS == PK_ATTDEF_find(PKI_HIDDEN_ENTITY_ATTRIB_NAME, &attribDefTag) && PK_ENTITY_null != attribDefTag)
            return SUCCESS;
        }

    // Create hidden entity attribute definition
    PK_CLASS_t          hiddenAttrOwnerTypes[] = {PK_CLASS_fin, PK_CLASS_edge, PK_CLASS_face, PK_CLASS_body};
    PK_ATTRIB_field_t   hiddenAttrFieldTypes[] = {PK_ATTRIB_field_integer_c};
    PK_ATTDEF_sf_t      hiddenAttrDefStruct;
    PK_ATTDEF_t         hiddenAttrDefTag;

    hiddenAttrDefStruct.name              = const_cast<CharP>(PKI_HIDDEN_ENTITY_ATTRIB_NAME);
    hiddenAttrDefStruct.attdef_class      = PK_ATTDEF_class_01_c;
    hiddenAttrDefStruct.n_owner_types     = sizeof(hiddenAttrOwnerTypes) / sizeof(hiddenAttrOwnerTypes[0]);
    hiddenAttrDefStruct.owner_types       = hiddenAttrOwnerTypes;
    hiddenAttrDefStruct.n_fields          = sizeof(hiddenAttrFieldTypes) / sizeof(hiddenAttrFieldTypes[0]);
    hiddenAttrDefStruct.field_types       = hiddenAttrFieldTypes;

    return (PK_ERROR_no_errors == PK_ATTDEF_create(&hiddenAttrDefStruct, &hiddenAttrDefTag) ? SUCCESS : ERROR);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus PSolidAttrib::CreateFaceMaterialIndexAttributeDef(bool isSessionStart)
    {
    if (!isSessionStart)
        {
        PK_ATTDEF_t attribDefTag = PK_ENTITY_null;

        if (SUCCESS == PK_ATTDEF_find(PKI_FACE_MATERIAL_ATTRIB_NAME, &attribDefTag) && PK_ENTITY_null != attribDefTag)
            return SUCCESS;
        }

    // Create face color/material attachment index attribute definition
    PK_CLASS_t          faceMaterialOwnerTypes[] = {PK_CLASS_face};
    PK_ATTRIB_field_t   faceMaterialFieldTypes[] = {PK_ATTRIB_field_integer_c};
    PK_ATTDEF_sf_t      faceMaterialAttribDefStruct;
    PK_ATTDEF_t         faceMaterialAttribDefTag;

    faceMaterialAttribDefStruct.name          = const_cast<CharP>(PKI_FACE_MATERIAL_ATTRIB_NAME);
    faceMaterialAttribDefStruct.attdef_class  = PK_ATTDEF_class_01_c;
    faceMaterialAttribDefStruct.n_owner_types = sizeof(faceMaterialOwnerTypes) / sizeof(faceMaterialOwnerTypes[0]);
    faceMaterialAttribDefStruct.owner_types   = faceMaterialOwnerTypes;
    faceMaterialAttribDefStruct.n_fields      = sizeof(faceMaterialFieldTypes) / sizeof(faceMaterialFieldTypes[0]);
    faceMaterialAttribDefStruct.field_types   = faceMaterialFieldTypes;

    return (PK_ERROR_no_errors == PK_ATTDEF_create(&faceMaterialAttribDefStruct, &faceMaterialAttribDefTag) ? SUCCESS : ERROR);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Deepak.Malkan   05/97
+---------------+---------------+---------------+---------------+---------------+------*/
void PSolidAttrib::GetAttrib
(
int*            pNumAttribOut,          // <= output number of attributes found
PK_ATTRIB_t**   ppAttribArrayOut,       // <= output attributes array
PK_ENTITY_t     entityTagIn,            // => input entity tag for which to get attrib
char const*     pAttribNameIn           // => input attrib type to get
)
    {
    PK_ATTDEF_t attribDefTag = PK_ENTITY_null;

    *ppAttribArrayOut = nullptr;
    *pNumAttribOut = 0;

    if (nullptr == pAttribNameIn || SUCCESS != PK_ATTDEF_find (pAttribNameIn, &attribDefTag) || PK_ENTITY_null == attribDefTag)
        return;

    PK_ENTITY_ask_attribs (entityTagIn, attribDefTag, pNumAttribOut, ppAttribArrayOut);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Deepak.Malkan   05/97
+---------------+---------------+---------------+---------------+---------------+------*/
void PSolidAttrib::DeleteAttrib
(
PK_ENTITY_t     entityTagIn,            // => input entity tag from which to delete attrib
char*           pAttribNameIn           // => input attrib type to delete
)
    {
    int          numAttrib = 0;
    PK_ATTRIB_t* pAttribArray = NULL;

    PSolidAttrib::GetAttrib (&numAttrib, &pAttribArray, entityTagIn, pAttribNameIn);
    PK_ENTITY_delete (numAttrib, pAttribArray);
    PK_MEMORY_free (pAttribArray);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidAttrib::GetFaceMaterialIndexAttribute(int32_t& index, PK_FACE_t entity)
    {
	index = 0;
    PK_ATTDEF_t attribDefTag = PK_ENTITY_null;

    if (SUCCESS != PK_ATTDEF_find(PKI_FACE_MATERIAL_ATTRIB_NAME, &attribDefTag) || PK_ENTITY_null == attribDefTag)
        return ERROR;

    int          numAttributes = 0;
    PK_ATTRIB_t* attributes = nullptr;

    if (SUCCESS != PK_ENTITY_ask_attribs(entity, attribDefTag, &numAttributes, &attributes))
        return ERROR;

    if (numAttributes > 0)
        {
        int value = 0;

        if (SUCCESS == PK_ATTRIB_ask_nth_int(attributes[0], 0, 0, &value))
            index = value;
        }

    PK_MEMORY_free(attributes);

    return (numAttributes > 0 ? SUCCESS : ERROR);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidAttrib::SetFaceMaterialIndexAttribute(PK_FACE_t entity, int32_t index)
    {
    if (index < 0)
        return ERROR;

    PK_ATTDEF_t attribDefTag = PK_ENTITY_null;

    if (SUCCESS != PK_ATTDEF_find(PKI_FACE_MATERIAL_ATTRIB_NAME, &attribDefTag) || PK_ENTITY_null == attribDefTag)
        return ERROR;

    StatusInt       status = SUCCESS;
    int             numAttributes = 0;
    PK_ATTRIB_t*    attributes = nullptr;

    PK_ENTITY_ask_attribs(entity, attribDefTag, &numAttributes, &attributes);

    if (0 == numAttributes)
        {
        if (0 == index)
            return SUCCESS; // Don't add attribute for "base" attachment info...

        PK_ATTRIB_t newAttribute;

        if (SUCCESS == (status = PK_ATTRIB_create_empty(entity, attribDefTag, &newAttribute)))
            status = PK_ATTRIB_set_ints(newAttribute, 0, 1, &index);
        }
    else if (0 == index)
        {
        PK_ENTITY_delete(numAttributes, attributes); // Remove attribute for "base" attachment info...
        }
    else
        {
        int value = 0;

        PK_ATTRIB_ask_nth_int(attributes[0], 0, 0, &value);

        if (value != index)
            status = PK_ATTRIB_set_ints(attributes[0], 0, 1, &index);
        }

    PK_MEMORY_free(attributes);

    return (BentleyStatus) status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/18
+---------------+---------------+---------------+---------------+---------------+------*/
void            PSolidAttrib::DeleteFaceMaterialIndexAttribute(PK_ENTITY_t entity)
    {
    PK_ATTDEF_t attribDefTag = PK_ENTITY_null;

    if (SUCCESS != PK_ATTDEF_find(PKI_FACE_MATERIAL_ATTRIB_NAME, &attribDefTag) || PK_ENTITY_null == attribDefTag)
        return;

    PK_CLASS_t entityClass = 0;

    PK_ENTITY_ask_class(entity, &entityClass);

    if (PK_CLASS_body == entityClass)
        {
        int         numFaces = 0;
        PK_FACE_t*  faces = nullptr;

        PK_BODY_ask_faces(entity, &numFaces, &faces);

        for (int i=0; i < numFaces; i++)
            {
            int          numAttributes = 0;
            PK_ATTRIB_t* attributes = nullptr;

            PK_ENTITY_ask_attribs(faces[i], attribDefTag, &numAttributes, &attributes);

            if (0 == numAttributes)
                continue;

            PK_ENTITY_delete(numAttributes, attributes);
            PK_MEMORY_free(attributes);
            }

        PK_MEMORY_free(faces);
        return;
        }

    int          numAttributes = 0;
    PK_ATTRIB_t* attributes = nullptr;

    PK_ENTITY_ask_attribs(entity, attribDefTag, &numAttributes, &attributes);

    if (0 == numAttributes)
        return;

    PK_ENTITY_delete(numAttributes, attributes);
    PK_MEMORY_free(attributes);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool PSolidAttrib::PopulateFaceMaterialIndexMap(T_FaceToAttachmentIndexMap& faceToIndexMap, PK_BODY_t entityTag, size_t numAttachments)
    {
    bool invalidIndexFound = false;
    bvector<PK_FACE_t> faces;

    // One chain allowed per thread - protect against mismatched calls to start/stop.
    PK_ERROR_code_t chainStatus = PK_THREAD_chain_start(PK_THREAD_chain_concurrent_c, nullptr);
    bool wasChainStarted = chainStatus == PK_ERROR_no_errors;
    BeAssert (wasChainStarted);

    PSolidTopo::GetBodyFaces(faces, entityTag);
    faceToIndexMap.clear();

    for (size_t i=0; i<faces.size(); i++)
        {
        int32_t attachmentIndex = 0;

        if (SUCCESS != PSolidAttrib::GetFaceMaterialIndexAttribute(attachmentIndex, faces[i]))
            {
            attachmentIndex = 0;
            }
        else if (attachmentIndex <= 0 || attachmentIndex >= numAttachments)
            {
            invalidIndexFound = true;
            attachmentIndex = 0;
            }

        faceToIndexMap[faces[i]] = (size_t) attachmentIndex;
        }

    if (wasChainStarted)
        PK_THREAD_chain_stop(nullptr);

    return !invalidIndexFound;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidAttrib::GetHiddenAttribute (bool& isHidden, PK_ENTITY_t entity)
    {
	isHidden = false;

    PK_ATTDEF_t attribDefTag = PK_ENTITY_null;

    if (SUCCESS != PK_ATTDEF_find (PKI_HIDDEN_ENTITY_ATTRIB_NAME, &attribDefTag) || PK_ENTITY_null == attribDefTag)
        return ERROR;

    int             numAttributes = 0;
    PK_ATTRIB_t*    attributes = NULL;

    if (SUCCESS != PK_ENTITY_ask_attribs (entity, attribDefTag, &numAttributes, &attributes))
        return ERROR;

    if (numAttributes > 0)
        {
        int     hiddenValue;

        isHidden = (SUCCESS == PK_ATTRIB_ask_nth_int (attributes[0], 0, 0, &hiddenValue) && HIDDEN_ATTRIBUTE_Hidden == hiddenValue);
        }

    PK_MEMORY_free (attributes);

    return (numAttributes > 0 ? SUCCESS : ERROR);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidAttrib::SetHiddenAttribute (PK_ENTITY_t entity, bool isHidden)
    {
    PK_ATTDEF_t attribDefTag = PK_ENTITY_null;

    if (SUCCESS != PK_ATTDEF_find (PKI_HIDDEN_ENTITY_ATTRIB_NAME, &attribDefTag) || PK_ENTITY_null == attribDefTag)
        return ERROR;

    StatusInt       status = SUCCESS;
    int             numAttributes = 0, value = (isHidden ? HIDDEN_ATTRIBUTE_Hidden : HIDDEN_ATTRIBUTE_Visible);
    PK_ATTRIB_t*    attributes = NULL;

    PK_ENTITY_ask_attribs (entity, attribDefTag, &numAttributes, &attributes);

    if (0 == numAttributes)
        {
        PK_ATTRIB_t newAttribute;

        if (SUCCESS == (status = PK_ATTRIB_create_empty (entity, attribDefTag, &newAttribute)))
            status = PK_ATTRIB_set_ints (newAttribute, 0, 1, &value);
        }
    else
        {
        int     currentValue;

        PK_ATTRIB_ask_nth_int (attributes[0], 0, 0, &currentValue);

        if (currentValue != value)
            status = PK_ATTRIB_set_ints (attributes[0], 0, 1, &value);
        }

    PK_MEMORY_free (attributes);

    return (BentleyStatus) status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            PSolidAttrib::DeleteHiddenAttribute (PK_ENTITY_t entity)
    {
    PK_ATTDEF_t attribDefTag = PK_ENTITY_null;

    if (SUCCESS != PK_ATTDEF_find (PKI_HIDDEN_ENTITY_ATTRIB_NAME, &attribDefTag) || PK_ENTITY_null == attribDefTag)
        return;

    int             numAttributes = 0;
    PK_ATTRIB_t*    attributes = NULL;

    PK_ENTITY_ask_attribs (entity, attribDefTag, &numAttributes, &attributes);

    if (0 == numAttributes)
        return;

    PK_ENTITY_delete (numAttributes, attributes);
    PK_MEMORY_free (attributes);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool PSolidAttrib::IsEntityHidden (PK_ENTITY_t entity)
    {
    bool hidden;

    return SUCCESS == PSolidAttrib::GetHiddenAttribute (hidden, entity) && hidden;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            PSolidAttrib::HasHiddenEdge (PK_BODY_t entity)
    {
    if (!entity)
        return false;

    bool        hasHiddenEntity = false;
    int         numEdges = 0;
    int*        edges = NULL;

    PK_BODY_ask_edges (entity, &numEdges, &edges);

    for (int i=0; i < numEdges; i++)
        {
        if (SUCCESS == PSolidAttrib::GetHiddenAttribute (hasHiddenEntity, edges[i]) && hasHiddenEntity)
            break;
        }

    PK_MEMORY_free (edges);

    return hasHiddenEntity;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            PSolidAttrib::HasHiddenFace (PK_BODY_t entity)
    {
    if (!entity)
        return false;

    bool        hasHiddenEntity = false;
    int         numFaces = 0;
    int*        faces = NULL;

    PK_BODY_ask_faces (entity, &numFaces, &faces);

    for (int i=0; i < numFaces; i++)
        {
        if (SUCCESS == PSolidAttrib::GetHiddenAttribute (hasHiddenEntity, faces[i]) && hasHiddenEntity)
            break;
        }

    PK_MEMORY_free (faces);

    return hasHiddenEntity;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus PSolidAttrib::GetHiddenBodyEdges (bset<PK_EDGE_t>& edges, PK_BODY_t body)
    {
    edges.clear();
    PK_EDGE_t*  pEdgeTagArray = NULL;
    int         edgeCount = 0;

    if (SUCCESS != PK_BODY_ask_edges (body, &edgeCount, &pEdgeTagArray))
        return ERROR;

    for (int i=0; i<edgeCount; i++)
        if (PSolidAttrib::IsEntityHidden (pEdgeTagArray[i]))
            edges.insert (pEdgeTagArray[i]);

    PK_MEMORY_free (pEdgeTagArray);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus PSolidAttrib::GetHiddenBodyFaces (bset<PK_FACE_t>& faces, PK_BODY_t body)
    {
    faces.clear ();
    PK_FACE_t*  pFaceTagArray = NULL;
    int         faceCount = 0;

    if (SUCCESS != PK_BODY_ask_faces (body, &faceCount, &pFaceTagArray))
        return ERROR;

    for (int i=0; i<faceCount; i++)
        if (PSolidAttrib::IsEntityHidden (pFaceTagArray[i]))
            faces.insert (pFaceTagArray[i]);

    PK_MEMORY_free (pFaceTagArray);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            PSolidAttrib::DeleteHiddenAttributeOnEdges (PK_BODY_t entity)
    {
    int         numEdges = 0;
    int*        pEdges = NULL;

    PK_BODY_ask_edges (entity, &numEdges, &pEdges);

    for (int i = 0; i < numEdges; i++)
        PSolidAttrib::DeleteHiddenAttribute (pEdges[i]);

    PK_MEMORY_free (pEdges);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            PSolidAttrib::DeleteHiddenAttributeOnFaces (PK_BODY_t entity)
    {
    int         numFaces = 0;
    int*        pFaces = NULL;

    PK_BODY_ask_faces (entity, &numFaces, &pFaces);

    for (int i = 0; i < numFaces; i++)
        PSolidAttrib::DeleteHiddenAttribute (pFaces[i]);

    PK_MEMORY_free (pFaces);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidAttrib::GetUserAttributes (bvector<int32_t>& attributes, PK_ENTITY_t entity, int32_t ownerId, int findIndex)
    {
    bool            found = false;
    int             i, index = 0, numAttrib = 0;
    PK_ATTRIB_t*    pAttribArray = NULL;

    PSolidAttrib::GetAttrib (&numAttrib, &pAttribArray, entity, PKI_USERDATA_ATTRIB_NAME);

    for (i = 0; i < numAttrib && !found; i++)
        {
        int     numData = 0, *pData = NULL;

        if (SUCCESS == PK_ATTRIB_ask_ints (pAttribArray[i], 0, &numData, &pData))
            {
            if (numData > 0 && NULL != pData)
                {
                if (pData[0] == ownerId)
                    {
                    if (index++ == findIndex)
                        {
                        numData--;
                        attributes.resize (numData);
                        memcpy (&attributes[0], &pData[1], numData * sizeof (int32_t));
                        found = true;
                        }
                    }

                PK_MEMORY_free (pData);
                }
            }
        }

    PK_MEMORY_free (pAttribArray);
    return (found ? SUCCESS : ERROR);
    }





