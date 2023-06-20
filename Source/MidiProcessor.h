#pragma once

#include <JuceHeader.h>

// HOW TO BUILD THIS AS A VST3 PLUGIN AND NOT JUST A STANDALONE APPLICATION:
// needed to ctrl + shift + b build solution and then found MidiEffect.vst3 in F:\Juce\my_projects\MidiEffect\Builds\VisualStudio2019\x64\Debug\VST3 folder
// copied MidiEffect.vst3 into F:\Program Files\Native Instruments\VSTPlugins 64 bit
// loaded this plugin on an empty midi track in Ableton and then selected another instrument to take midi from this plugin and it works for affecting the notes that are sent from supercollider
class MidiProcessor {
public:
	bool include_orig_note = true; // option to include original note or not

	bool include_determined_orig_note_for_debugging = true; // just checking things out
	
	// option for more than 1 note to be added
	std::vector<int> intervals; // will subtract later to get correct scale array offset so that you can specify 3 instead of 2 for a 3rd for instance

	// vector of booleans specifying for each corresponding harmony interval whether you want the note to be lower than the original note or higher
	std::vector<bool> harmonize_lower;

	// use a vector for the notes of a particular scale and then harmonize that way 
	// with the interval meaning how many notes up or down in the scale you would add
	// so would just add these values to the original note to come up with the harmonized note, 
	// don't have to worry about coming up with the octave here because will be determined relatively from the original note
	std::vector<int> scale;
	int num_octaves; // NOT SURE THAT YOU NEED ANYMORE...
	int min_note_of_scale = 0;
	int tot_chromatic_span_of_scale = 0;
	int min_note_lowest_octave_rep = 0;

	// finds actual scale index if the note value exists in the scale vector, otherwise finds closest note in scale, 
	// in the event of a tie between lower and higher notes picks lower note by default
	int find_closest_scale_index(std::vector<int> scale, int orig_note_lowest_octave_rep) {
		// get a copy of the scale vector and sort it so that you can find the closest scale value if the incoming midi note is not in the scale
		std::vector<int> scale_copy_sorted = scale; // in c++ this is actually a copy not a pointer to the original
		std::sort(scale_copy_sorted.begin(), scale_copy_sorted.end());

		// if the scale is for instance 3, 5, 7, 8, 10, 12, 14 and 12 has been decreased to 0, need to check if +12 of the orig_note_lowest_ocatave_rep as well
		if (orig_note_lowest_octave_rep < scale_copy_sorted[0] && (orig_note_lowest_octave_rep + 12) <= scale_copy_sorted[scale_copy_sorted.size() - 1]) {
			orig_note_lowest_octave_rep = orig_note_lowest_octave_rep + 12;
		}
		
		// what if the scale is for instance 3, 5, 7, 8, 10, 12, 14 and 15... checking and adjusting MAYBE A MORE ELEGANT WAY TO DO THIS...
		if (orig_note_lowest_octave_rep > scale_copy_sorted[scale_copy_sorted.size() - 1] && orig_note_lowest_octave_rep - 12 >= scale_copy_sorted[0]) {
			orig_note_lowest_octave_rep = orig_note_lowest_octave_rep - 12;
		}

		// if the incoming midi note is in the scale, just get the scale index
		int incoming_midi_note_scale_index = -1;
		for (int i = 0; i < scale.size(); i++) {
			if (orig_note_lowest_octave_rep == scale[i]) {
				incoming_midi_note_scale_index = i;
				break;
			}
		}

		// NEED TO TEST IF NOTE IS NOT IN SCALE STILL...
		if (incoming_midi_note_scale_index < 0) { // if the incoming midi note is not found in the scale
			int closest_midi_note = -1;
			int closest_lower_midi_note = -1;
			int closest_higher_midi_note = -1;
			for (int i = 0; i < scale_copy_sorted.size(); i++) {
				if (orig_note_lowest_octave_rep > scale_copy_sorted[i]) {
					if (i == scale_copy_sorted.size() - 1) { // if it is the highest of the sorted vector, then there is no higher note to check, so the closest note is here
						closest_midi_note = scale_copy_sorted[i];
					}
					else { // not higher than the highest note of the scale
						closest_lower_midi_note = scale_copy_sorted[i];
						closest_higher_midi_note = scale_copy_sorted[i + 1];
						// if incoming note is closer to lower scale note or equidistant to lower and higer note, go with lower scale note
						if ((orig_note_lowest_octave_rep - closest_lower_midi_note) <= (closest_higher_midi_note - orig_note_lowest_octave_rep)) {
							closest_midi_note = closest_lower_midi_note;
						}
						else { // go with higher closest note
							closest_midi_note = closest_higher_midi_note;
						}
					}
				}
			}

			// if original midi note is less than all of the values in scale vector, then it is closest to the 1st value of the sorted vector
			if (closest_midi_note < 0) { // in this situation closest_midi_note will not have changed from default value of -1
				closest_midi_note = scale_copy_sorted[0];
			}

			// you have found the closest midi note value, now need to find the index in the original unsorted vector
			for (int i = 0; i < scale.size(); i++) {
				if (closest_midi_note == scale[i]) {
					incoming_midi_note_scale_index = i;
					break;
				}
			}

		}
		return incoming_midi_note_scale_index;
	
	}

	// receives the midi buffer and determines the harmonized notes to add and adds to the processed midi buffer to send out
	void process(juce::MidiBuffer& midiMessages) {
		// juce uses a linked list to store midi information
		juce::MidiBuffer::Iterator it(midiMessages);
		juce::MidiMessage currentMessage;
		int samplePos;
		juce::MidiBuffer processedBuffer;
		processedBuffer.clear();

		while (it.getNextEvent(currentMessage, samplePos)) {
			// check if midi message is of type note on or note off to ignore information that you might receive that are like channel pressure or stuff like that
			if (currentMessage.isNoteOnOrOff()) {
				auto transposedMessage = currentMessage;
				//auto transposedMessage2 = currentMessage; // just for debugging
				auto oldNoteNum = transposedMessage.getNoteNumber();

				// THINK YOU CAN GET RID OF BECAUSE DISCARDING ANYTHING THAT THE USER INPUTTED THAT IS MORE THAN AN OCTAVE AWAY FROM THE MINIMUM NOTE INPUTTED
				// well need to account for how many octaves the scale spans, not just the lowest octave representation
				int orig_note_lowest_octave_rep = oldNoteNum;
				
				// decrease octave representation of original note as far as you can
				while (orig_note_lowest_octave_rep >= 0) {
					orig_note_lowest_octave_rep -= 12;
				}

				
				// PERHAPS DON'T NEED BECAUSE DISCARDING ANYTHING THAT THE USER INPUTTED THAT IS MORE THAN AN OCTAVE AWAY FROM THE MINIMUM NOTE INPUTTED
				int dist_from_min_note = orig_note_lowest_octave_rep - min_note_lowest_octave_rep;
				int num_octaves_from_min_note = dist_from_min_note / 12;


				// you will have subtracted 12 one too many times when the while loop ends
				orig_note_lowest_octave_rep += 12 * (num_octaves_from_min_note + 1);

				// check if new representation is lower than the min_note for situations like 
				// 1, 2, 3, 5, 7, 8, 10, 12, 14 -> 1, 2, 3, 5, 7, 8, 10, 0, 14
				if (orig_note_lowest_octave_rep < min_note_lowest_octave_rep) {
					orig_note_lowest_octave_rep += 12;
				}

				// have the correct orig_note_lowest_octave_rep at this point
				
				// finds closest scale index, defaults to lower scale index in the event that there is a tie
				// if the incoming midi note is found in the scale, then it just returns that index
				int incoming_midi_note_scale_index = find_closest_scale_index(scale, orig_note_lowest_octave_rep);

				// option for more than 1 note to be added, go through all specified intervals to add harmonized notes for
				for (int i = 0; i < intervals.size(); i++) {
					int scale_index = (static_cast<int>(incoming_midi_note_scale_index) + (intervals[i] - 1)) % scale.size();
					int chromatic_diff_to_add = scale[scale_index] - scale[incoming_midi_note_scale_index];
					
					// check if harmonized note will be lower than original note, if not then calculate higher harmonized note
					if (harmonize_lower[i]) {
						// subtract 1 from interval so that like if you want the 3rd of a note you can say 3 for interval instead of 2 for instance
						if ((oldNoteNum + chromatic_diff_to_add) < oldNoteNum) {
							transposedMessage.setNoteNumber(oldNoteNum + chromatic_diff_to_add);
						}
						else {
							transposedMessage.setNoteNumber(oldNoteNum - 12 + chromatic_diff_to_add);
						}
					}
					else { // harmonize higher
						// subtract 1 from interval so that like if you want the 3rd of a note you can say 3 for interval instead of 2 for instance
						if ((oldNoteNum + chromatic_diff_to_add) > oldNoteNum) {
							transposedMessage.setNoteNumber(oldNoteNum + chromatic_diff_to_add);
						}
						else {
							transposedMessage.setNoteNumber(oldNoteNum + 12 + chromatic_diff_to_add);
						}
					}

					processedBuffer.addEvent(transposedMessage, samplePos);

				}

			}

			if (include_orig_note) {
				// this line adds also the original note to the processedBuffer so including the original note and transposed note for harmony to play at the same time
				processedBuffer.addEvent(currentMessage, samplePos); 
			}
			
		}

		// replace MidiBuffer & midiMessages with the modified processedBuffer
		midiMessages.swapWith(processedBuffer);

	}
};