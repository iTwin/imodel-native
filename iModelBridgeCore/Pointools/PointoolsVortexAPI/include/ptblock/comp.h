#ifndef COMPARISION_OPERATORS_H
#define COMPARISION_OPERATORS_H

struct str_cmp
{
    typedef const char* first_argument_type;
    bool operator () (const char* lhs, const char* rhs) const
    {
        return strcmp(lhs, rhs) < 0 ? true : false;
    }
};

#endif