@ECHO OFF
mkdir build
cd build
cmake.exe .. -DUSE_PLATFORM=VK_USE_PLATFORM_WIN32_KHR -G "Visual Studio 14 2015 Win64"
start "" "VulkanJumpStart.sln"
cd ..