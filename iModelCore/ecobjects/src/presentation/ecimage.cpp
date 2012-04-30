/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/ecimage.cpp $
|
|   $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

#include <EcPresentation/auipresentationapi.h>

USING_NAMESPACE_EC

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IECNativeImagePtr     ECPresentationImageProvider::GetImage (ECImageKeyCR imageKey, DPoint2dCR size)
    {
    return _GetImage(imageKey, size);
    }

