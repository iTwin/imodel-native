/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/ECImageProvider.cpp $
|
|   $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

#include <EcPresentation/auipresentationapi.h>

USING_NAMESPACE_BENTLEY_EC

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IECNativeImagePtr ECPresentationImageProvider::GetImage (ECImageKeyCR imageKey, DPoint2dCR size)
    {
    return _GetImage(imageKey, size);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
IECNativeImagePtr ECPresentationImageProvider::GetOverlayImage (IAUIDataContextCR context, DPoint2dCR size)
    {
    return _GetOverlayImage(context, size);
    }