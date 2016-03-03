/*--------------------------------------------------------------------------------------+
|
|     $Source: PCLWrapper/PublicAPI/IStatus.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "IDefines.h"

BEGIN_PCLWRAPPER_NAMESPACE

struct IStatus;

typedef BENTLEY_NAMESPACE_NAME::RefCountedPtr<IStatus>             IStatusPtr;

/*=================================================================================**//**
* Interface feature line.
* @bsiclass                                                     Bentley Systems
+===============+===============+===============+===============+===============+======*/
struct IStatus : public BENTLEY_NAMESPACE_NAME::RefCountedBase
    {
    public :

        enum Type
        {
        SUCCESS = 0,
        ERROR,
        ERROR_CYLINDER_SEED_POINT_NOT_IN_POINTCLOUD,
        ERROR_CYLINDER_SEED_POINT_TO_FAR_FROM_FOUND_CYLINDER,
        };

    /*__PUBLISH_SECTION_END__*/

    private:

    /*__PUBLISH_CLASS_VIRTUAL__*/
    protected:

        virtual Type _GetType() const = 0;

    /*__PUBLISH_SECTION_START__*/
    public:
        //! Gets the number of points of the DTM.
        //! @return The number of points of the DTM..

        //! Gets the draping interface.
        //! @return The draping interface.

        PCLWRAPPER_EXPORT Type GetType() const;

        PCLWRAPPER_EXPORT static IStatusPtr CreateStatus(IStatus::Type& statusType);

    };

END_PCLWRAPPER_NAMESPACE
