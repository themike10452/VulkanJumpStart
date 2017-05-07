#include "tools.h"
#include "helper_macros.h"

#include <fstream>

namespace tools
{
    void readBinaryFile( const VkfwString filename, std::vector<VkfwChar>* buffer )
    {
        REQUIRE_PTR( filename );
        REQUIRE_PTR( buffer );

        std::ifstream stream( filename, std::ios::ate | std::ios::binary );

        if (!stream.is_open())
            throw std::runtime_error("Failed to open file");

        VkfwSize size = static_cast<VkfwSize>(stream.tellg());
        buffer->resize( size );

        stream.seekg( 0 );
        stream.read( buffer->data(), size );

        stream.close();
    }
}
