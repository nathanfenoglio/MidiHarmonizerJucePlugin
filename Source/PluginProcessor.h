/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "MidiProcessor.h"

//==============================================================================
/**
*/
class MidiEffectAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    MidiEffectAudioProcessor();
    ~MidiEffectAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // get the scale array from harmonize scale text label and declare the variable here
    std::vector <int> user_input_scale_vector{ 0, 2, 3, 5, 7, 8, 11 }; // just declaring a default scale if none has been inputted yet 

    int min_note_of_scale = 0;
    int min_note_lowest_octave_rep = 0;

    std::vector <int> user_input_interval_vector{ 3, 7, 11 }; // just declaring default intervals if none has been inputted yet

    std::vector <bool> user_input_harmonize_lower_vector{ true, false, false }; // just declaring default booleans if none has been inputted yet

private:
    MidiProcessor midiProcessor;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiEffectAudioProcessor)
};
