/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Licensing/Licensing.h>
#include <Bentley/WString.h>
#include <sstream>

BEGIN_BENTLEY_LICENSING_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct SCVWritter
    {
private:
    std::stringstream m_sstr;

public:
    template <typename T>
    void AddValue(T t)
        {
        m_sstr << t;
        }

    template<typename T, typename... Args>
    void AddValue(T t, Args... args) // recursive variadic function
        {
        m_sstr << t << ",";

        AddValue(args...);
        }
        
    template<typename T, typename... Args>
    void AddRow(T t, Args... args)
        {
        if (!m_sstr.str().empty())
            m_sstr << std::endl;

        AddValue(t, std::forward<Args>(args)...);
        }

    LICENSING_EXPORT Utf8String ToString();

    LICENSING_EXPORT BentleyStatus WriteToFile(BeFileNameCR filePath);
    };

END_BENTLEY_LICENSING_NAMESPACE
