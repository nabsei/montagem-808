cmake_minimum_required(VERSION 3.22)
project(Montagem808 VERSION 0.2.0)

add_subdirectory(libs/JUCE)

if(APPLE)
    set(E808_FORMATS VST3 AU Standalone)
else()
    set(E808_FORMATS VST3 Standalone)
endif()

juce_add_plugin(Montagem808
    COMPANY_NAME "montagem.808"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE Mtgf
    PLUGIN_CODE E808
    FORMATS ${E808_FORMATS}
    PRODUCT_NAME "Montagem 808")

target_sources(Montagem808 PRIVATE
    Source/E808Processor.cpp
    Source/PluginEditor.cpp
    Source/PluginEntry.cpp)

target_compile_definitions(Montagem808 PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(Montagem808
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
