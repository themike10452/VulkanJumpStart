@ECHO OFF
mkdir build
cd build
cmake.exe .. -DCMAKE_BUILD_TYPE=Release -DUSE_PLATFORM=VK_USE_PLATFORM_WIN32_KHR -G "Visual Studio 15 2017 Win64"
start "" "VulkanJumpStart.sln"
cd ..