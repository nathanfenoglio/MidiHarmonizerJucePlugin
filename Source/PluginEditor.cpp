/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
MidiEffectAudioProcessorEditor::MidiEffectAudioProcessorEditor (MidiEffectAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (600, 500);

    // JUST A BUTTON NOT USING FOR ANYTHING
    addAndMakeVisible(button_1);
    button_1.setButtonText("Whoah a Button");
    button_1.setClickingTogglesState(true);
    button_1.setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::aquamarine);
    button_1.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::black);
    button_1.setColour(juce::TextButton::ColourIds::textColourOnId, juce::Colours::black);
    button_1.setColour(juce::TextButton::ColourIds::textColourOffId, juce::Colours::white);

    // JUST A TOGGLE BUTTON NOT USING FOR ANYTHING
    addAndMakeVisible(toggle_1);
    toggle_1.setButtonText("TOGGLE");

    // Scale to Harmonize text box
    addAndMakeVisible(harmonyScaleInputLabel);
    harmonyScaleInputLabel.setText("Scale to Harmonize With (comma separated):", juce::dontSendNotification);
    harmonyScaleInputLabel.attachToComponent(&harmonyScaleInputText, false); // onLeft	if true, the label will stay on the left of its component; if false, it will stay above it
    harmonyScaleInputLabel.setColour(juce::Label::textColourId, juce::Colours::orange);

    addAndMakeVisible(harmonyScaleInputText);
    harmonyScaleInputText.setEditable(true);
    harmonyScaleInputText.setColour(juce::Label::backgroundColourId, juce::Colours::darkblue);
    // when text is changed scaleInputTextChanged function is called which gets a vector of ints from the user input string in harmonyScaleInputText
    harmonyScaleInputText.onTextChange = [this] { scaleInputTextChanged(); };

    // intervals to use for harmony, like up a 3rd, up an 11th etc
    addAndMakeVisible(harmonyIntervalsInputLabel);
    harmonyIntervalsInputLabel.setText("Harmony Intervals (comma separated):", juce::dontSendNotification);
    harmonyIntervalsInputLabel.attachToComponent(&harmonyIntervalsInputText, false); // onLeft	if true, the label will stay on the left of its component; if false, it will stay above it
    harmonyIntervalsInputLabel.setColour(juce::Label::textColourId, juce::Colours::orange);

    addAndMakeVisible(harmonyIntervalsInputText);
    harmonyIntervalsInputText.setEditable(true);
    harmonyIntervalsInputText.setColour(juce::Label::backgroundColourId, juce::Colours::darkblue);
    // when text is changed intervalInputTextChanged which ...
    harmonyIntervalsInputText.onTextChange = [this] { intervalInputTextChanged(); };

    // whether or not you want to harmonize an octave lower or higher, represented as "true" or "false" or "0" or "1" right now, may switch to toggle boxes per interval later
    addAndMakeVisible(harmonizeLowerInputLabel);
    harmonizeLowerInputLabel.setText("Option to Harmonize Octave Lower (comma separated true/false):", juce::dontSendNotification);
    harmonizeLowerInputLabel.attachToComponent(&harmonizeLowerInputText, false); // onLeft	if true, the label will stay on the left of its component; if false, it will stay above it
    harmonizeLowerInputLabel.setColour(juce::Label::textColourId, juce::Colours::orange);

    addAndMakeVisible(harmonizeLowerInputText);
    harmonizeLowerInputText.setEditable(true);
    harmonizeLowerInputText.setColour(juce::Label::backgroundColourId, juce::Colours::darkblue);
    // when text is changed intervalInputTextChanged which ...
    harmonizeLowerInputText.onTextChange = [this] { harmonizeLowerInputTextChanged(); };

    // Debug text box
    addAndMakeVisible(justDebugLabel);
    justDebugLabel.setText("Just Debugging Output:", juce::dontSendNotification);
    justDebugLabel.attachToComponent(&justDebugText, false); // onLeft	if true, the label will stay on the left of its component; if false, it will stay above it
    justDebugLabel.setColour(juce::Label::textColourId, juce::Colours::orange);

    addAndMakeVisible(justDebugText);
    justDebugText.setColour(juce::Label::backgroundColourId, juce::Colours::darkblue);
}

MidiEffectAudioProcessorEditor::~MidiEffectAudioProcessorEditor()
{
}

//==============================================================================
void MidiEffectAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
}

void MidiEffectAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    auto button_1_width = getWidth() * 0.2;
    auto button_1_height = getWidth() * 0.1;
    auto toggle_1_size = getWidth() * 0.2;

    button_1.setBounds(getWidth() / 2, getHeight() / 2, button_1_width, button_1_height);
    toggle_1.setBounds(button_1.getX(), button_1.getY() + button_1.getHeight() + 12, toggle_1_size, toggle_1_size);

    harmonyScaleInputText.setBounds(100, 50, getWidth() - 110, 20);
    harmonyIntervalsInputText.setBounds(100, 100, getWidth() - 110, 20);
    harmonizeLowerInputText.setBounds(100, 150, getWidth() - 110, 20);

    justDebugText.setBounds(100, 200, getWidth() - 110, 20);
}

// modifies the original vector of ints that is passed in by reference
void MidiEffectAudioProcessorEditor::translate_scale_to_lowest_octave(std::vector<int>& orig_scale_rep, int min_note, int min_note_lowest_octave_rep) {
    // translate scale to closest to 0 octave possible 
    // subtract 12 from each note repeatedly until the result would be negative 
    for (int i = 0; i < orig_scale_rep.size(); i++) {
        int dist_from_min_note = orig_scale_rep[i] - min_note;
        // exclude any notes that are more than 1 octave higher than the min note that the user entered
        if ((orig_scale_rep[i] - min_note) > 11) {
            continue;
        }

        int lowest_octave_rep = orig_scale_rep[i];
        while (lowest_octave_rep >= 0) {
            lowest_octave_rep -= 12;
        }

        // you will have subtracted 12 one too many times when the while loop ends
        lowest_octave_rep += 12;

        // check if new representation is lower than the min_note for situations like 
        // 1, 2, 3, 5, 7, 8, 10, 12, 14 -> 1, 2, 3, 5, 7, 8, 10, 0, 14
        if (lowest_octave_rep < min_note_lowest_octave_rep) {
            lowest_octave_rep += 12;
        }
        // replace original scale entry with lowest octave representation
        orig_scale_rep[i] = lowest_octave_rep;
    }
}

// need to turn all midi notes off when any of the input fields change to stop hanging on notes that don't get the note off message when things change
void MidiEffectAudioProcessorEditor::scaleInputTextChanged() {
    // get vector of ints from the scale inputLabel from the user
    scale_from_input_text = parseCommaSeparatedIntegers(harmonyScaleInputText.getText());

    // SHOULD REFACTOR AS A SEPARATE FUNCTION...
    auto min_note_ptr = std::min_element(scale_from_input_text.begin(), scale_from_input_text.end());
    min_note = *min_note_ptr;
    auto max_note_ptr = std::max_element(scale_from_input_text.begin(), scale_from_input_text.end());
    int max_note = *max_note_ptr;
    audioProcessor.min_note_of_scale = min_note;

    std::vector <int> scale_from_input_only_one_octave;
    for (int i = 0; i < scale_from_input_text.size(); i++) {
        // exclude all notes entered by user that are more than 1 octave higher than the min note
        if ((scale_from_input_text[i] - min_note) < 12) {
            scale_from_input_only_one_octave.push_back(scale_from_input_text[i]);
        }
    }

    min_note_lowest_octave_rep = min_note;
    while (min_note_lowest_octave_rep >= 0) {
        min_note_lowest_octave_rep -= 12;
    }

    // you will have subtracted 12 one too many times when the while loop ends
    min_note_lowest_octave_rep += 12;
    audioProcessor.min_note_lowest_octave_rep = min_note_lowest_octave_rep;

    // translate scale to closest to 0 octave possible
    // function modifies original scale_from_input_text vector
    translate_scale_to_lowest_octave(scale_from_input_only_one_octave, min_note, min_note_lowest_octave_rep);

    // pass the scale vector to the audioProcessor from PluginProcessor.h file which is then passed to the MidiProcessor object 
    // which uses it for picking the harmonized midi notes to insert in the processed buffer
    audioProcessor.user_input_scale_vector = scale_from_input_only_one_octave;

    // just getting the result of getting the scale from input text to check out
    juce::String scale_from_input_text_string;
    for (int i = 0; i < scale_from_input_only_one_octave.size(); i++) {
        scale_from_input_text_string += juce::String(scale_from_input_only_one_octave[i]);
        scale_from_input_text_string += ", ";
    }

    justDebugText.setText(juce::String(scale_from_input_text_string), juce::NotificationType::dontSendNotification);
    //justDebugText.setText(juce::String(min_note_lowest_octave_rep), juce::NotificationType::dontSendNotification);
    
    // if this is working for getting the scale into an int vector, need to check input to make sure that you're getting a comma separated int array and nothing else
    // have had some difficulty figuring out how to successfully check, like if you enter random words instead of #s 0 is pushed into the vector with the token.getIntValue() call
}

void MidiEffectAudioProcessorEditor::intervalInputTextChanged() {
    // get vector of intervals from the user
    harmony_intervals_from_input_text = parseCommaSeparatedIntegers(harmonyIntervalsInputText.getText());
    // check for if there is not at least the corresponding amount of harmonize_lower booleans when the user changes the intervals
    if (harmony_intervals_from_input_text.size() != audioProcessor.user_input_harmonize_lower_vector.size()) {
        justDebugText.setText(juce::String("# of intervals does not equal # of harmonize lower booleans"), juce::NotificationType::dontSendNotification);

        // create a function keepSameNumInputs to check and see which of harmony_intervals_from_input_text or user_input_harmonize_lower_vector needs adjusted
        keepSameNumInputs();
    }
    // pass the interval vector to the audioProcessor from PluginProcessor.h file which is then passed to the MidiProcessor object
    // which uses it for determining the distance in the scale from the current note to the harmonized note
    audioProcessor.user_input_interval_vector = harmony_intervals_from_input_text;

    // just getting the result of getting the intervals from input text to check out
    juce::String harmony_intervals_from_input_text_string;
    for (int i = 0; i < harmony_intervals_from_input_text.size(); i++) {
        harmony_intervals_from_input_text_string += juce::String(harmony_intervals_from_input_text[i]);
        harmony_intervals_from_input_text_string += ", ";
    }
    justDebugText.setText(juce::String(harmony_intervals_from_input_text_string), juce::NotificationType::dontSendNotification);
    // not checking that user input is valid yet...
    
}

void MidiEffectAudioProcessorEditor::harmonizeLowerInputTextChanged() {
    // get vector of booleans from the user
    // NEED TO CHECK FOR CORRECTLY FORMATTED INPUT...
    harmonize_lower_from_input_text = parseCommaSeparatedBooleans(harmonizeLowerInputText.getText());
    
    // need to check if there are less harmonize lower booleans than intervals to avoid crashing...
    keepSameNumInputs();

    // pass the boolean vector to the audioProcessor from PluginProcessor.h file which is then passed to the MidiProcessor object
    // which uses it for determining whether to place the harmonized note lower or higher than the original note
    audioProcessor.user_input_harmonize_lower_vector = harmonize_lower_from_input_text;

    // just getting the result of getting the booleans for harmonize lower or not from input text to check out
    juce::String harmonize_lower_from_input_text_string;
    for (int i = 0; i < harmonize_lower_from_input_text.size(); i++) {
        if (harmonize_lower_from_input_text[i]) {
            harmonize_lower_from_input_text_string += "true, ";
        }
        else {
            harmonize_lower_from_input_text_string += "false, ";
        }
    }

    justDebugText.setText(juce::String(harmonize_lower_from_input_text_string), juce::NotificationType::dontSendNotification);
    // not checking that user input is valid yet...
}

void MidiEffectAudioProcessorEditor::keepSameNumInputs() {
    // if there are more intervals than lower_or_highers, then add as many falses as needed to audioProcessor.user_input_harmonize_lower_vector
    if (harmony_intervals_from_input_text.size() > audioProcessor.user_input_harmonize_lower_vector.size()) {

        int num_falses_to_add = harmony_intervals_from_input_text.size() - audioProcessor.user_input_harmonize_lower_vector.size();
        for (int i = 0; i < num_falses_to_add; i++) {
            audioProcessor.user_input_harmonize_lower_vector.push_back(false);
        }

        // create string to put in harmonizeLowerInputText text box so user can see that default falses were added
        juce::String harmonize_lower_from_input_text_string;
        for (int i = 0; i < audioProcessor.user_input_harmonize_lower_vector.size(); i++) {
            if (audioProcessor.user_input_harmonize_lower_vector[i]) {
                harmonize_lower_from_input_text_string += "true, ";
            }
            else {
                harmonize_lower_from_input_text_string += "false, ";
            }
        }

        harmonizeLowerInputText.setText(juce::String(harmonize_lower_from_input_text_string), juce::NotificationType::dontSendNotification);
    }
}

// parses a vector of booleans either inputted as true/false (case doesn't matter) or 1/0 NEED TO TEST STILL...
// IT'S CRASHING WHEN YOU JUST PUT IN A SPACE
std::vector<bool> MidiEffectAudioProcessorEditor::parseCommaSeparatedBooleans(const juce::String& str)
{
    std::vector<bool> result;

    juce::StringArray tokens;
    tokens.addTokens(str, ",", "");

    for (const juce::String& token : tokens)
    {
        bool boolValue = false;
        juce::String lowercaseToken = token.toLowerCase().trim();

        if (lowercaseToken == "true" || lowercaseToken == "1")
            boolValue = true;

        result.push_back(boolValue);
    }

    return result;
}

// attempting to stop the crash but unsuccessful so far...
//std::vector<bool> MidiEffectAudioProcessorEditor::parseCommaSeparatedBooleans(const juce::String& str)
//{
//    std::vector<bool> result;
//
//    juce::StringArray tokens;
//    tokens.addTokens(str, ",", "");
//
//    for (const juce::String& token : tokens)
//    {
//        juce::String lowercaseToken = token.toLowerCase().trim();
//
//        if (lowercaseToken.length() == 0) {
//            continue;
//        }
//        if (lowercaseToken == "true" || lowercaseToken == "1")
//            result.push_back(true);
//        else if (lowercaseToken == "false" || lowercaseToken == "0")
//            result.push_back(false);
//        else if (lowercaseToken.isNotEmpty())
//        {
//            // Handle the case where the token is not recognized as a boolean value
//            // You can choose to handle this case differently, such as skipping the token or throwing an exception
//            result.push_back(false);
//        }
//    }
//
//    // If the input string was empty or contained only spaces, add a single false value
//    if (result.empty() && str.trim().isEmpty())
//        result.push_back(false);
//
//    return result;
//}



std::vector<int> MidiEffectAudioProcessorEditor::parseCommaSeparatedIntegers(const juce::String& str)
{
    std::vector<int> result;

    juce::StringArray tokens;
    tokens.addTokens(str, ",", "");

    for (const juce::String& token : tokens)
    {
        // trying to not add if the value is not parsed as an int
        // getIntValue() just seems to return 0 though when it's a word or nan 
        //auto an_int_perhaps = token.getIntValue();
        //if (check_if_int(an_int_perhaps)) {
        //    result.push_back(token.getIntValue());
        //}

        result.push_back(token.getIntValue());
    }

    return result;
}
