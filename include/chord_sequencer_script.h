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

// Musical key enumeration
enum MusicalKey {
    MKEY_C = 0, MKEY_CS, MKEY_D, MKEY_DS, MKEY_E, MKEY_F,
    MKEY_FS, MKEY_G, MKEY_GS, MKEY_A, MKEY_AS, MKEY_B
};

// Theory mode for chord suggestions
enum TheoryMode {
    THEORY_FUNCTIONAL = 0,  // Function-based harmony (I, IV, V)
    THEORY_DIATONIC,        // Diatonic scale-based
    THEORY_MODAL,           // Modal harmony
    THEORY_CHROMATIC,       // Chromatic relationships
    THEORY_ALL              // All chords available
};

// Scale degree/mode (defines scale quality)
enum ScaleDegree {
    DEGREE_MAJOR = 0,       // Ionian mode (major scale)
    DEGREE_MINOR,           // Aeolian mode (natural minor)
    DEGREE_DORIAN,          // Dorian mode
    DEGREE_PHRYGIAN,        // Phrygian mode
    DEGREE_LYDIAN,          // Lydian mode
    DEGREE_MIXOLYDIAN,      // Mixolydian mode
    DEGREE_LOCRIAN          // Locrian mode
};

// Global musical parameters
struct GlobalParameters {
    MusicalKey key;                    // Current key root note (C, C#, D, etc.)
    ScaleDegree degree;                // Scale quality (major, minor, dorian, etc.)
    TheoryMode theoryMode;             // Theory mode for suggestions
    float voiceLeadingCompactness;     // 0.0-1.0: prefer minimal voice movement
    float energy;                      // 0.0-1.0: harmonic energy/tension level
};

// Chord definition
struct Chord {
    uint8_t rootNote;      // 0-11 (C=0, C#=1, D=2, etc.)
    ChordType type;        // Major or Minor
    
    // Per-chord override parameters (use special values for "auto"/null)
    uint8_t localInversion;      // 0-2 for inversion level, 255 = auto (use global)
    float localSpread;           // 0.0-1.0 for spread, -1.0 = auto (use global)
    uint8_t localTheoryMode;     // TheoryMode enum value, 255 = auto (use global)
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
    
    // Per-chord parameter accessors
    void setChordInversion(uint8_t slot, uint8_t inversion) { 
        if (slot < MAX_CHORD_SLOTS) chords[slot].localInversion = inversion; 
    }
    uint8_t getChordInversion(uint8_t slot) const { 
        return (slot < MAX_CHORD_SLOTS) ? chords[slot].localInversion : 255; 
    }
    
    void setChordSpread(uint8_t slot, float spread) { 
        if (slot < MAX_CHORD_SLOTS) chords[slot].localSpread = constrain(spread, -1.0f, 1.0f); 
    }
    float getChordSpread(uint8_t slot) const { 
        return (slot < MAX_CHORD_SLOTS) ? chords[slot].localSpread : -1.0f; 
    }
    
    void setChordTheoryMode(uint8_t slot, uint8_t mode) { 
        if (slot < MAX_CHORD_SLOTS) chords[slot].localTheoryMode = mode; 
    }
    uint8_t getChordTheoryMode(uint8_t slot) const { 
        return (slot < MAX_CHORD_SLOTS) ? chords[slot].localTheoryMode : 255; 
    }
    
    // Playback control
    void setTempo(float bpm);
    void setGlobalTempo(float bpm) { setTempo(bpm); }  // Alias for consistency with ScriptManager
    
    // Get current state
    uint8_t getCurrentChordSlot() const { return currentChordSlot; }
    uint8_t getBeatCounter() const { return beatCounter; }
    uint8_t getChordCount() const { return chordCount; }
    
    // Global parameter accessors
    const GlobalParameters& getGlobalParameters() const { return globals; }
    void setKey(MusicalKey key) { globals.key = key; }
    void setDegree(ScaleDegree degree) { globals.degree = degree; }
    ScaleDegree getDegree() const { return globals.degree; }
    void setTheoryMode(TheoryMode mode) { globals.theoryMode = mode; }
    void setVoiceLeadingCompactness(float value) { globals.voiceLeadingCompactness = constrain(value, 0.0f, 1.0f); }
    void setEnergy(float value) { globals.energy = constrain(value, 0.0f, 1.0f); }
    
    // Get current scale info (for Poliquencer integration)
    void getCurrentScale(ScaleInfo* scale);
    
    // Display info
    void getDisplayText(char* buffer, size_t bufferSize);
    
private:
    // Global musical parameters
    GlobalParameters globals;
    
    // Chord progression (up to MAX_CHORD_SLOTS chords)
    Chord chords[MAX_CHORD_SLOTS];
    uint8_t chordBeats[MAX_CHORD_SLOTS];   // Beats per chord (1-32)
    uint8_t chordCount;                    // Active chord slots
    
    // Timing
    uint8_t currentChordSlot;      // 0-(chordCount-1)
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
