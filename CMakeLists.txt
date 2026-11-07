cmake_minimum_required(VERSION 3.22)
project(AUTOTUNE_MQAWD_PRO VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.8
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(AUTOTUNE_MQAWD_PRO
    COMPANY_NAME "Mqawd Studio"
    BUNDLE_ID "com.mqawdstudio.autotunemqawdpro"
    PLUGIN_MANUFACTURER_CODE Mqwd
    PLUGIN_CODE Amqp
    FORMATS VST3 Standalone
    PRODUCT_NAME "AUTOTUNE MQAWD PRO"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
)

target_sources(AUTOTUNE_MQAWD_PRO PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
)

target_compile_definitions(AUTOTUNE_MQAWD_PRO PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(AUTOTUNE_MQAWD_PRO PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_audio_plugin_client
    juce::juce_audio_basics
)
