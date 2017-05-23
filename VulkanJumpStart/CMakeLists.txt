cmake_minimum_required ( VERSION 3.5 )

project ( "VulkanJumpStart" )
set ( CMAKE_CXX_STANDARD 11 )

if ( NOT CMAKE_BUILD_TYPE )
	set ( CMAKE_BUILD_TYPE "Debug" )
endif()

message ( "${CMAKE_BUILD_TYPE} build" )

if ( NOT USE_PLATFORM MATCHES "VK_USE_PLATFORM_.*" )
	if ( WIN32 )
		set ( USE_PLATFORM "VK_USE_PLATFORM_WIN32_KHR" )
	elseif ( UNIX AND NOT APPLE )
		set ( USE_PLATFORM "VK_USE_PLATFORM_XLIB_KHR" )
	endif()
endif()

if ( WIN32 )
	message( "MS Windows" )

	add_definitions ( -DWIN32_LEAN_AND_MEAN )
	add_definitions ( -D_CONSOLE )
	add_definitions ( -D_CRT_SECURE_NO_WARNINGS )

	set( CMAKE_C_FLAGS_RELWITHDEBINFO    "${CMAKE_C_FLAGS_RELWITHDEBINFO} /MT" )
	set( CMAKE_C_FLAGS_RELEASE           "${CMAKE_C_FLAGS_RELEASE} /MT" )
	set( CMAKE_C_FLAGS_DEBUG             "${CMAKE_C_FLAGS_DEBUG} /MTd" )
	set( CMAKE_CXX_FLAGS_RELWITHDEBINFO  "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /MT" )
	set( CMAKE_CXX_FLAGS_RELEASE         "${CMAKE_CXX_FLAGS_RELEASE} /MT" )
	set( CMAKE_CXX_FLAGS_DEBUG           "${CMAKE_CXX_FLAGS_DEBUG} /MTd" )
elseif ( UNIX AND NOT APPLE )
	message( "Linux" )

	cmake_minimum_required ( VERSION 3.8 )
	find_package(Vulkan REQUIRED)

	add_compile_options( -std=c++11 )
	link_libraries( X11 dl ${Vulkan_LIBRARIES} )
endif()

if ( CMAKE_BUILD_TYPE STREQUAL Debug )
	add_definitions ( -D_DEBUG )
	add_definitions ( -DVKFW_ENABLE_VALIDATION )
endif()

add_definitions ( -DVK_NO_PROTOTYPES )
add_definitions ( -D${USE_PLATFORM} )

set ( build_dir "${CMAKE_BINARY_DIR}/build/" )

macro ( configure_runtime_files srcDir )
	set ( out_dir "${build_dir}/${srcDir}" )
	make_directory ( ${out_dir} )
	file ( COPY "${CMAKE_SOURCE_DIR}/${srcDir}" DESTINATION ${out_dir} )
	unset ( out_dir )
endmacro( configure_runtime_files )

include_directories ( "Include" )

set ( SOURCE_FILES
		"vkfw.cpp"
		"internal.cpp"
		"vulkan_functions.cpp"
		"win32_window.cpp"
		"x11_window.cpp"
		"win32_input.cpp"
		"x11_input.cpp"
		"tools.cpp")

configure_runtime_files( "Data/Shaders/" )
add_executable ( VulkanJumpStart main.cpp ${SOURCE_FILES} )
