/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/BePointCloud/IPointCloudFileEdit.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/
#include "IPointCloudFileQuery.h"

BEGIN_BENTLEY_BEPOINTCLOUD_NAMESPACE

/*=================================================================================**//**
* \addtogroup Pointcloud
*/
//@{


/*__PUBLISH_SECTION_END__*/
/*---------------------------------------------------------------------------------**//**
* Interface to edit persistent information about a point cloud file.
* @bsiinterface
+---------------+---------------+---------------+---------------+---------------+------*/
struct IPointCloudFileEdit : public IPointCloudFileQuery
    {
    protected:
        virtual StatusInt   _SetMetaTag     (WStringCR tagName, WStringCR tagValue) = 0;
        virtual StatusInt   _WriteMetaTags  () = 0;

    public:

        /*---------------------------------------------------------------------------------**//**
        * Assign a Metatag to a pointcloud
        * @param tagName IN Name of the tag
        * @param tagValue IN Value of the tag
        * @return   SUCCESS or ERROR
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        BEPOINTCLOUD_EXPORT StatusInt SetMetaTag(WStringCR tagName, WStringCR tagValue);

        /*---------------------------------------------------------------------------------**//**
        * Write the tags we edited
        * @return   SUCCESS or ERROR
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        BEPOINTCLOUD_EXPORT StatusInt WriteMetaTags();

    };

/*__PUBLISH_SECTION_START__*/
//@}

END_BENTLEY_BEPOINTCLOUD_NAMESPACE
