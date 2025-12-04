/**
 * Chord Sequencer Script Header
 * 
 * Inspired by Oxi One's chord mode
 * Sets musical scale for Poliquencer by defining chord progressions
 * Plays 4 chords in sequence, changing every 16 beats
 * Provides "legal notes" (scale notes) for melodic sequences
 * 
 * Default progression: Am - Dm - Fmaj - Gmin (classic minor key progression)
 */

#ifndef CHORD_SEQUENCER_SCRIPT_H
#define CHORD_SEQUENCER_SCRIPT_H

#include <Arduino.h>
#include "config.h"

// Chord quality/type
enum ChordType {
    CHORD_MAJOR = 0,
    CHORD_MINOR = 1
};

// Chord definition
struct Chord {
    uint8_t rootNote;      // 0-11 (C=0, C#=1, D=2, etc.)
    ChordType type;        // Major or Minor
};

// Scale note structure (for passing to Poliquencer)
struct ScaleInfo {
    uint8_t rootNote;      // Current scale root
    int8_t notes[7];       // 7 scale degrees (semitones from root)
    uint8_t numNotes;      // Always 7 for major/minor
    char name[16];         // Human-readable scale name
};

class ChordSequencerScript {
public:
    ChordSequencerScript();
    
    // Initialize the chord sequencer
    bool begin();
    
    // Update sequencer state (call frequently)
    void update();
    
    // Stop the sequencer
    void stop();
    
    // Chord progression management
    void setChord(uint8_t slot, uint8_t rootNote, ChordType type);
    void getChord(uint8_t slot, uint8_t* rootNote, ChordType* type);
    void setChordBeats(uint8_t slot, uint8_t beats);  // Set beats for individual chord
    uint8_t getChordBeats(uint8_t slot) const;
    
    // Playback control
    void setTempo(float bpm);
    void setGlobalTempo(float bpm) { setTempo(bpm); }  // Alias for consistency with ScriptManager
    
    // Get current state
    uint8_t getCurrentChordSlot() const { return currentChordSlot; }
    uint8_t getBeatCounter() const { return beatCounter; }
    
    // Get current scale info (for Poliquencer integration)
    void getCurrentScale(ScaleInfo* scale);
    
    // Display info
    void getDisplayText(char* buffer, size_t bufferSize);
    
private:
    // Chord progression (4 chords)
    Chord chords[4];
    uint8_t chordBeats[4];         // Beats per chord (1-32)
    
    // Timing
    uint8_t currentChordSlot;      // 0-3
    uint8_t beatCounter;           // 0 to current chord's beat count
    unsigned long lastBeatMicros;
    unsigned long beatDurationMicros;
    
    // Scale generation
    void generateScale(uint8_t rootNote, ChordType type, int8_t* scaleNotes);
    
    // Note name helpers
    const char* getNoteName(uint8_t note);
    const char* getChordTypeName(ChordType type);
};

#endif // CHORD_SEQUENCER_SCRIPT_H
