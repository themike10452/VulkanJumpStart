#ifndef TOOLS_HEADER
#define TOOLS_HEADER

#include "types.h"

#include <vector>

#define TEST_FLAGS( LHS, RHS ) ( (LHS & RHS) == RHS )

namespace tools
{
    void readBinaryFile( const VkfwString filename, std::vector<VkfwChar>* buffer );
}

#endif // !TOOLS_HEADER
