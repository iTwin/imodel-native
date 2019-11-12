/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Licensing/Licensing.h>
#include <BeJsonCpp/BeJsonUtilities.h>

BEGIN_BENTLEY_LICENSING_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct InMemoryJsonLocalState : IJsonLocalState
    {
private:
    bmap<Utf8String, Utf8String> m_state;

protected:
    LICENSING_EXPORT virtual void _SaveValue(Utf8CP nameSpace, Utf8CP key, Utf8StringCR value);

    LICENSING_EXPORT virtual Utf8String _GetValue(Utf8CP nameSpace, Utf8CP key) const;

public:
    virtual ~InMemoryJsonLocalState() {};
    };

END_BENTLEY_LICENSING_NAMESPACE
