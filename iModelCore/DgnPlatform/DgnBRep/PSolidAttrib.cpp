/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnBRep/PSolidAttrib.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnBRep/PSolidUtil.h>

static const int HIDDEN_ATTRIBUTE_Visible = 0;
static const int HIDDEN_ATTRIBUTE_Hidden  = 1;

// Brien claims we shouldn't have to check these attributes, or if for a particular body we must it should be indicated in the flat buffer.
// Looking them up is a significant multi-threading bottleneck in Parasolid 
#define DGN_BREP_IGNORE_HIDDEN_ATTRIBS

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
* @bsimethod                                                    Brien.Bastings  12/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PSolidAttrib::GetHiddenAttribute (bool& isHidden, PK_ENTITY_t entity)
    {
	isHidden = false;

#if !defined(DGN_BREP_IGNORE_HIDDEN_ATTRIBS)
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
#else
    return ERROR;
#endif
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
#if !defined(DGN_BREP_IGNORE_HIDDEN_ATTRIBS)
    int         numEdges = 0;
    int*        edges = NULL;

    PK_BODY_ask_edges (entity, &numEdges, &edges);

    for (int i=0; i < numEdges; i++)
        {
        if (SUCCESS == PSolidAttrib::GetHiddenAttribute (hasHiddenEntity, edges[i]) && hasHiddenEntity)
            break;
        }

    PK_MEMORY_free (edges);
#endif

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
#if !defined(DGN_BREP_IGNORE_HIDDEN_ATTRIBS)
    int         numFaces = 0;
    int*        faces = NULL;

    PK_BODY_ask_faces (entity, &numFaces, &faces);

    for (int i=0; i < numFaces; i++)
        {
        if (SUCCESS == PSolidAttrib::GetHiddenAttribute (hasHiddenEntity, faces[i]) && hasHiddenEntity)
            break;
        }

    PK_MEMORY_free (faces);
#endif
    return hasHiddenEntity;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus PSolidAttrib::GetHiddenBodyEdges (bset<PK_EDGE_t>& edges, PK_BODY_t body)
    {
    int         edgeCount = 0;
    edges.clear();
#if !defined(DGN_BREP_IGNORE_HIDDEN_ATTRIBS)
    PK_EDGE_t*  pEdgeTagArray = NULL;

    if (SUCCESS != PK_BODY_ask_edges (body, &edgeCount, &pEdgeTagArray))
        return ERROR;

    for (int i=0; i<edgeCount; i++)
        if (PSolidAttrib::IsEntityHidden (pEdgeTagArray[i]))
            edges.insert (pEdgeTagArray[i]);

    PK_MEMORY_free (pEdgeTagArray);
#endif
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus PSolidAttrib::GetHiddenBodyFaces (bset<PK_FACE_t>& faces, PK_BODY_t body)
    {
    int         faceCount = 0;
    faces.clear ();
#if !defined(DGN_BREP_IGNORE_HIDDEN_ATTRIBS)
    PK_FACE_t*  pFaceTagArray = NULL;

    if (SUCCESS != PK_BODY_ask_faces (body, &faceCount, &pFaceTagArray))
        return ERROR;

    for (int i=0; i<faceCount; i++)
        if (PSolidAttrib::IsEntityHidden (pFaceTagArray[i]))
            faces.insert (pFaceTagArray[i]);

    PK_MEMORY_free (pFaceTagArray);
#endif
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





