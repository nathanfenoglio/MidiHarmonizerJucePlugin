/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include <algorithm>

//==============================================================================
/**
*/
class MidiEffectAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    MidiEffectAudioProcessorEditor (MidiEffectAudioProcessor&);
    ~MidiEffectAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    // to hold the scale that will be used to harmonize converted from the user input from harmonyScaleInputText
    std::vector <int> scale_from_input_text;

    // MAYBE DON'T NEED TO WORRY ABOUT DETERMINING HOW MANY OCTAVES ANY LONGER...
    // # of octaves that the scale spans
    int num_octaves = 0;
    int min_note = 0;
    int tot_chromatic_span_of_scale = 0;
    int min_note_lowest_octave_rep = 0;

    // to hold the intervals (like 3rd, 7th, 9th etc) for the harmonized notes
    std::vector <int> harmony_intervals_from_input_text;

    // to hold whether or not to place the harmonized note below the note (true) or above the note (false)
    std::vector <bool> harmonize_lower_from_input_text;

    // function that is called when scale input text is changed
    void scaleInputTextChanged();
    
    void intervalInputTextChanged();

    void harmonizeLowerInputTextChanged();

    // parses a vector of ints from a comma separated string of ints
    std::vector <int> parseCommaSeparatedIntegers(const juce::String& str);

    // parses a vector of booleans either inputted as true/false (case doesn't matter) or 1/0 NEED TO TEST STILL...
    std::vector<bool> parseCommaSeparatedBooleans(const juce::String& str);

    // function to make sure that the # of inputs for intervals and harmonize up or down are the same at all costs
    void keepSameNumInputs();

    void translate_scale_to_lowest_octave(std::vector<int>& orig_scale_rep, int min_note, int min_note_lowest_octave_rep);

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    // you end up using it for the final pass of the harmony scale to be able to get it into the actual processing logic
    MidiEffectAudioProcessor& audioProcessor;

    juce::TextButton button_1;

    juce::ToggleButton toggle_1;

    juce::Label harmonyScaleInputText;
    juce::Label harmonyScaleInputLabel; // the label for the scale input text box

    juce::Label harmonyIntervalsInputText;
    juce::Label harmonyIntervalsInputLabel;

    // options per interval to harmonize lower or higher
    // maybe should do as toggle boxes instead per interval
    juce::Label harmonizeLowerInputText;
    juce::Label harmonizeLowerInputLabel;

    juce::Label justDebugLabel;
    juce::Label justDebugText;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiEffectAudioProcessorEditor)
};
