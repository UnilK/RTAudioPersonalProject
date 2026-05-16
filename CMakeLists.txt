cmake_minimum_required(VERSION 3.25)

# Windows static runtime
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# MacOS unversal binary
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")

# MacOS minimum deployment target
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15")

# Main project
project(MRTA VERSION 0.1.0 LANGUAGES C CXX)

# Get 'add_plugin' function and add JUCE to project
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(add_plugin)


# General variables
# You should change this to something more interesting
set(company_name "Sounds of Untok")
set(company_copyright "Sounds of Untok")
set(company_code "UnoK")
set(company_bundle_id "com.soundsofuntok")


## Plugin projects

set(mrtaa_source ${CMAKE_CURRENT_SOURCE_DIR}/projects/App)

add_plugin(mrtaa
    VERSION 0.1.0
    PLUGIN_NAME "Synthit"
    PROD_NAME Synthit
    PROD_CODE Sn1t
    SYNTH TRUE
    SOURCES
        ${mrtaa_source}/PluginEditor.cpp
        ${mrtaa_source}/PluginProcessor.cpp
        ${mrtaa_source}/math/commonMath.cpp
        ${mrtaa_source}/math/fft-impl.cpp
        ${mrtaa_source}/math/fft.cpp
        ${mrtaa_source}/dsp/pitch.cpp
        ${mrtaa_source}/dsp/waveletio.cpp
    INCLUDE_DIRS
        ${mrtaa_source}
        ${mrtaa_source}/math
        ${mrtaa_source}/dsp)

set_property(TARGET mrtaa PROPERTY CXX_STANDARD 20)
