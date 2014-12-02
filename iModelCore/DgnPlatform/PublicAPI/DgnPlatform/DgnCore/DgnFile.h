/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/DgnFile.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "DgnProject.h"
#include "DgnModel.h"
#include "ElementRef.h"

/** @addtogroup DgnFileGroup

Classes for working with elements by model within a project.

Files and models are used to organize and access \ref ElementRefGroup within \ref DgnProjectGroup.

<h4>Models</h4>
A "model" is a group of \ref ElementRefGroup, created for some application-specific purpose. A model is
represented in memory by a BentleyApi::DgnPlatform::DgnModel object. An element is assigned to a DgnModel when it is created and can never move.

A DgnModel is a convenient way to load all elements in the model. See BentleyApi::DgnPlatform::DgnModel#FillSections. A DgnModel can be used
to access element data only after it has been filled. When filled, a DgnModel holds a BentleyApi::DgnPlatform::PersistentElementRef to each element.

The DgnModelId property of an element can be used in queries. The element data table is indexed on DgnModelId.

<h4>Files</h4>
A "file" is used to load and access DgnModels. A file is represented in memory by a BentleyApi::DgnPlatform::DgnFile object.
A DgnFile can contain multiple DgnModels. A DgnModel belongs to a single DgnFile.
A DgnFile is a collection of DgnModels. A DgnFile is therefore a way of accessing \ref ElementRefGroup via models.

The BentleyApi::DgnPlatform::DgnProject#Models table is a way of querying models without loading them.

An element can also be loaded in memory without filling its model. BentleyApi::DgnPlatform::DgnFile#GetElementById will load a single element.

*/

//__PUBLISH_SECTION_END__
#include "../DgnPlatformErrors.r.h"
//__PUBLISH_SECTION_START__

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   05/13
//=======================================================================================
enum DgnFileSchemaVersion
    {
    DGNFILE_CURRENT_VERSION_Major = 10,
    DGNFILE_CURRENT_VERSION_Minor = 2,
    DGNFILE_CURRENT_VERSION_Sub1  = 0,
    DGNFILE_CURRENT_VERSION_Sub2  = 0,

    DGNFILE_SUPPORTED_VERSION_Major = 10,  // oldest version of the project schema supported by current api (after auto-upgrades)
    DGNFILE_SUPPORTED_VERSION_Minor = 2,
    DGNFILE_SUPPORTED_VERSION_Sub1  = 0,
    DGNFILE_SUPPORTED_VERSION_Sub2  = 0,
    };

//__PUBLISH_SECTION_END__
//=======================================================================================
// @bsiclass                                                    Keith.Bentley   11/10
//=======================================================================================
struct DbElementReader
{
protected:
    BeSQLite::SnappyFromBlob m_snappy;
    DgnProjectCR m_project;
    BeSQLite::CachedStatementPtr m_selectStmt;

    virtual void _OnElementSelected (PersistentElementRefR, bool wasLoaded) {}
    virtual void _LoadElement (PersistentElementRefR);
    virtual DgnModelP _GetDgnModelForElement();
    virtual PersistentElementRefP _FindExistingElementRef (ElementId id);

    Utf8CP GetSelectElementSql();
    BeSQLite::DbResult GetOneElement (PersistentElementRefP& elRef, bool& wasLoaded);

public:
    DbElementReader (DgnProjectCR project) : m_project(project) {}
    virtual ~DbElementReader () {}
    UInt32 ReadElements (ICheckStopP checkStop);
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   11/10
//=======================================================================================
struct DgnDbFileIOElementReader : DbElementReader
{
public:
    DGNPLATFORM_EXPORT DgnDbFileIOElementReader (DgnProjectCR, DgnModelId);
    DGNPLATFORM_EXPORT PersistentElementRefPtr LoadElement();
};

//__PUBLISH_SECTION_START__
END_BENTLEY_DGNPLATFORM_NAMESPACE
