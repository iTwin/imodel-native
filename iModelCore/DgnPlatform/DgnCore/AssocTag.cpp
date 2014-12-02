/*----------------------------------------------------------------------+
|
|     $Source: DgnCore/AssocTag.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+----------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/01
+---------------+---------------+---------------+---------------+---------------+------*/
static bool findTagInDescriptor (MSElementDescrP edP, MSElementDescr **nodeEdPP, ElementId reqTag)
    {
    if (edP == NULL)
        return false;

    if (edP->Element().GetElementId() == reqTag)
        {
        if (nodeEdPP)
            *nodeEdPP = edP;

        return true;
        }
    
    for (auto& child : edP->Components())
        {
        if (findTagInDescriptor (child.get(), nodeEdPP, reqTag))
            return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/01
+---------------+---------------+---------------+---------------+---------------+------*/
MSElementDescrP DependencyManagerLinkage::FindElementIDInDescriptor (MSElementDescrP edP, ElementId reqTag)
    {
    MSElementDescr  *nodeEdP = NULL;
    return findTagInDescriptor (edP, &nodeEdP, reqTag) ? nodeEdP :NULL;
    }
