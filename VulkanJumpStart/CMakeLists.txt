cmake_minimum_required ( VERSION 3.0 )

project ( "VulkanJumpStart" )

if ( WIN32 )
	set( USE_PLATFORM "VK_USE_PLATFORM_WIN32_KHR" )
	add_definitions( -DWIN32_LEAN_AND_MEAN )
	add_definitions( -D_CONSOLE )
	add_definitions( -D_CRT_SECURE_NO_WARNINGS )
endif()

add_definitions ( -D_DEBUG )
add_definitions ( -DVK_NO_PROTOTYPES )
add_definitions ( -D${USE_PLATFORM} )
add_definitions ( -std=c++11 )

set( bin_dir "${CMAKE_BINARY_DIR}/bin/" )

macro( configure_runtime_files srcDir )
	set( out_dir "${bin_dir}/${srcDir}" )
	make_directory( ${out_dir} )
	file( COPY "${CMAKE_SOURCE_DIR}/${srcDir}" DESTINATION ${out_dir} )
	unset( out_dir )
endmacro(configure_files)

include_directories ( "Include" )

set ( HEADER_FILES
		"Include/vulkan/vk_platform.h"
		"Include/vulkan/vulkan.h"
		"Include/types.h"
		"Include/internal.h"
		"Include/platform.h"
		"Include/smartptr.h"
		"Include/vkptr.h"
		"Include/vkfw.h"
		"Include/vulkan_functions.h"
		"Include/vulkan_functions.inl"
		"Include/win32_window.h"
		"Include/tools.h")

set ( SOURCE_FILES
		"vkfw.cpp"
		"internal.cpp"
		"vulkan_functions.cpp"
		"win32_window.cpp"
		"tools.cpp")

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${bin_dir} )

configure_runtime_files( "Data/Shaders/" )
add_executable ( VulkanJumpStart main.cpp ${HEADER_FILES} ${SOURCE_FILES} )