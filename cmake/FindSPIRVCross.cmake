# FindSPIRVCross.cmake
# Finds the spirv-cross command-line tool.
#
# This module defines:
#   SPIRVCross_FOUND         - True if spirv-cross was found
#   SPIRV_CROSS_EXECUTABLE   - Path to the spirv-cross binary
#
# Usage:
#   find_package(SPIRVCross REQUIRED)

include(FindPackageHandleStandardArgs)

find_program(SPIRV_CROSS_EXECUTABLE
    NAMES spirv-cross
    PATHS
        /usr/bin
        /usr/local/bin
        /opt/vulkan-sdk/bin
        $ENV{VULKAN_SDK}/bin
    DOC "Path to the spirv-cross executable"
)

find_package_handle_standard_args(SPIRVCross
    REQUIRED_VARS SPIRV_CROSS_EXECUTABLE
    FAIL_MESSAGE "Could not find spirv-cross. Install it via your package manager (e.g. 'apt install spirv-cross') or set SPIRV_CROSS_EXECUTABLE."
)

mark_as_advanced(SPIRV_CROSS_EXECUTABLE)
