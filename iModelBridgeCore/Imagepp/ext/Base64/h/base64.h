#ifndef _base64_h_
#define _base64_h_

#include <iostream>

namespace b64
{
        bool encode(std::istream& input, std::ostream& output);
        bool decode(std::istream& input, std::ostream& output);
};

#endif
