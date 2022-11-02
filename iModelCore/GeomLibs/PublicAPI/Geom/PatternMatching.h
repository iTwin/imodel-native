/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <algorithm>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

//Knuth Morris Pratt pattern searching algorithm
template<typename T>
struct PatternSearching
    {
protected:
    //Longest proper prefix of pattern[0..i]
    //which is also a suffix of pattern[0..i].
    bvector<size_t> _Lps (bvector<T> const& pattern)
        {
        bvector<size_t> lps (pattern.size (), 0);

        //length of the previous longest prefix suffix 
        size_t length = 0;
        for (size_t i = 1; i < lps.size ();)
            {
            if (pattern[i] == pattern[length])
                {
                length++;
                lps[i] = length;
                ++i;
                continue;
                }

            if (length != 0)
                length = lps[length - 1];
            else
                {
                lps[i] = 0;
                ++i;
                }
            }
        return lps;
        }

public:
    bvector<size_t> KmpMatching (bvector<T> const& sequence, bvector<T> const& pattern)
        {
        bvector<size_t> lps (_Lps (pattern));

        bvector<size_t> indices;

        size_t j = 0;
        for (size_t i = 0; i < sequence.size();)
            {
            if (sequence[i] == pattern[j])
                {
                i++;
                j++;
                }

            if (j == pattern.size ())
                {
                indices.push_back (i - j);
                j = lps[j - 1];
                }
            else if (i < sequence.size() && pattern[j] != sequence[i])
                {
                if (j != 0)
                    j = lps[j - 1];
                else
                    i++;
                }
            }

        return indices;
        }
    };

END_BENTLEY_GEOMETRY_NAMESPACE
