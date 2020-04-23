/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

BEGIN_BENTLEY_IMODELHUB_NAMESPACE
//=======================================================================================
//@bsistruct                                      Algirdas.Mikoliunas           04/2020
//=======================================================================================
template <typename T>
struct ResultRange : RefCountedBase
    {
private:
    T m_value;
    Utf8String m_skipToken;

public:
    ResultRange() {}
    ResultRange(T value, Utf8String skipToken) : m_value(value), m_skipToken(skipToken) {}

    Utf8String GetSkipToken() const { return m_skipToken; }
    T GetValue() const { return m_value; }
};
END_BENTLEY_IMODELHUB_NAMESPACE
