/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "sortutil.h"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public int util_tagSort
(
    int     *tags,
    double  *values,
    int     numValues
)
    {
    int i;

    if (numValues == 0)
        {
        return (ERROR);
        }
    else if (numValues == 1)
        {
        tags[0] = 0;
        return (SUCCESS);
        }
    else
        {
        for (i = 0; i < numValues; i++)
            tags[i] = i;
        auto compare = [values](const int &index1, const int &index2)
            {
            double  value1 = values[index1], value2 = values[index2];
            return value1 < value2;
            };
        std::sort(tags, (tags + numValues), compare);
        }
    return SUCCESS;
    }
END_BENTLEY_GEOMETRY_NAMESPACE
