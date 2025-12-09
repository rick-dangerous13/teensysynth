/**
 * Chord Sequencer Script Implementation
 * 
 * Inspired by Oxi One chord mode
 * Sequences through 4 chords, changing every 16 beats
 * Provides scale context for melodic sequencing
 */

#include "chord_sequencer_script.h"
#include <string.h>

// Note names for display
static const char* noteNames[12] = {
    "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
};

// Major scale intervals (semitones from root)
static const int8_t majorScaleIntervals[7] = {0, 2, 4, 5, 7, 9, 11};

// Natural minor scale intervals (semitones from root)
static const int8_t minorScaleIntervals[7] = {0, 2, 3, 5, 7, 8, 10};

ChordSequencerScript::ChordSequencerScript()
    : chordCount(0)
    , currentChordSlot(0)
    , beatCounter(0)
    , lastBeatMicros(0)
    , beatDurationMicros(500000) {  // 120 BPM default

    // Initialize global parameters with sensible defaults
    globals.key = MKEY_C;
    globals.theoryMode = THEORY_FUNCTIONAL;
    globals.voiceLeadingCompactness = 0.5f;
    globals.energy = 0.5f;

    // Seed default chord shapes but start with zero active chords
    chords[0] = {9, CHORD_MINOR, 255, -1.0f, 255};   // A minor (Am) with auto overrides
    chords[1] = {2, CHORD_MINOR, 255, -1.0f, 255};   // D minor (Dm) with auto overrides
    chords[2] = {5, CHORD_MAJOR, 255, -1.0f, 255};   // F major (Fmaj) with auto overrides
    chords[3] = {7, CHORD_MINOR, 255, -1.0f, 255};   // G minor (Gm) with auto overrides

    chordBeats[0] = 16;
    chordBeats[1] = 8;
    chordBeats[2] = 8;
    chordBeats[3] = 8;

    for (uint8_t i = 4; i < MAX_CHORD_SLOTS; i++) {
        chords[i] = {0, CHORD_MAJOR, 255, -1.0f, 255};  // Auto overrides
        chordBeats[i] = 8;
    }
}

bool ChordSequencerScript::begin() {
    lastBeatMicros = micros();
    currentChordSlot = 0;
    beatCounter = 0;
    
    Serial.println("ChordSequencer: Initialized");
    Serial.println("  Default progression seeded (inactive)");
    Serial.println("  Tempo: 120 BPM");
    Serial.print("  Chord slots active: ");
    Serial.println(chordCount);
    
    return true;
}

void ChordSequencerScript::update() {
    unsigned long currentMicros = micros();
    unsigned long elapsed = currentMicros - lastBeatMicros;

    if (chordCount == 0) {
        return;  // No active chords yet
    }
    if (currentChordSlot >= chordCount) {
        currentChordSlot = 0;
    }
    if (chordBeats[currentChordSlot] == 0) {
        chordBeats[currentChordSlot] = 1;  // Guard against invalid beat counts
    }
    
    // Check if it's time for next beat
    if (elapsed >= beatDurationMicros) {
        beatCounter++;
        
        // Wrap beat counter based on current chord's beat count
        if (beatCounter >= chordBeats[currentChordSlot]) {
            beatCounter = 0;
            // Auto-advance to next chord, wrap to first after the last active slot
            currentChordSlot = (currentChordSlot + 1) % chordCount;
        }
        
        lastBeatMicros = currentMicros;
    }
}

void ChordSequencerScript::stop() {
    Serial.println("ChordSequencer: Stopped");
}

void ChordSequencerScript::setChord(uint8_t slot, uint8_t rootNote, ChordType type) {
    if (slot >= MAX_CHORD_SLOTS) return;
    if (rootNote >= 12) return;
    
    chords[slot].rootNote = rootNote;
    chords[slot].type = type;

    if (slot + 1 > chordCount) {
        chordCount = slot + 1;
    }
    
    Serial.print("ChordSequencer: Chord ");
    Serial.print(slot + 1);
    Serial.print(" set to ");
    Serial.print(getNoteName(rootNote));
    Serial.println(getChordTypeName(type));
}

void ChordSequencerScript::getChord(uint8_t slot, uint8_t* rootNote, ChordType* type) {
    if (slot >= MAX_CHORD_SLOTS) return;
    if (rootNote) *rootNote = chords[slot].rootNote;
    if (type) *type = chords[slot].type;
}

void ChordSequencerScript::setTempo(float bpm) {
    if (bpm < 20.0f) bpm = 20.0f;
    if (bpm > 300.0f) bpm = 300.0f;
    beatDurationMicros = (unsigned long)((60.0f / bpm) * 1000000.0f);
}

void ChordSequencerScript::setChordBeats(uint8_t slot, uint8_t beats) {
    if (slot >= MAX_CHORD_SLOTS) return;
    if (beats < 1) beats = 1;
    if (beats > 32) beats = 32;
    chordBeats[slot] = beats;

    if (slot + 1 > chordCount) {
        chordCount = slot + 1;
    }
}

uint8_t ChordSequencerScript::getChordBeats(uint8_t slot) const {
    if (slot >= MAX_CHORD_SLOTS) return 0;
    return chordBeats[slot];
}

void ChordSequencerScript::getCurrentScale(ScaleInfo* scale) {
    if (!scale) return;
    
    if (chordCount == 0) {
        scale->rootNote = 0;
        scale->numNotes = 0;
        memset(scale->notes, 0, sizeof(scale->notes));
        snprintf(scale->name, sizeof(scale->name), "(no chord)");
        return;
    }

    if (currentChordSlot >= chordCount) {
        currentChordSlot = 0;
    }

    // Get current chord
    Chord& currentChord = chords[currentChordSlot];
    
    // Set root note
    scale->rootNote = currentChord.rootNote;
    scale->numNotes = 7;
    
    // Generate scale based on chord type
    generateScale(currentChord.rootNote, currentChord.type, scale->notes);
    
    // Create human-readable name
    snprintf(scale->name, sizeof(scale->name), "%s %s",
             getNoteName(currentChord.rootNote),
             currentChord.type == CHORD_MAJOR ? "Major" : "Minor");
}

void ChordSequencerScript::generateScale(uint8_t rootNote, ChordType type, int8_t* scaleNotes) {
    const int8_t* intervals = (type == CHORD_MAJOR) ? majorScaleIntervals : minorScaleIntervals;
    
    for (int i = 0; i < 7; i++) {
        scaleNotes[i] = intervals[i];
    }
}

const char* ChordSequencerScript::getNoteName(uint8_t note) {
    if (note >= 12) note = note % 12;
    return noteNames[note];
}

const char* ChordSequencerScript::getChordTypeName(ChordType type) {
    return (type == CHORD_MAJOR) ? "maj" : "min";
}

void ChordSequencerScript::getDisplayText(char* buffer, size_t bufferSize) {
    if (chordCount == 0) {
        snprintf(buffer, bufferSize,
                 "SYMPHONY CHROD SEQUENCER\n(no chords)\nBeat:0/0\nChord:0/0");
        return;
    }

    if (currentChordSlot >= chordCount) {
        currentChordSlot = 0;
    }

    Chord& currentChord = chords[currentChordSlot];
    
    // Display current chord and beat
    snprintf(buffer, bufferSize,
             "SYMPHONY CHROD SEQUENCER\n%s%s\nBeat:%d/%d\nChord:%d/%d",
             getNoteName(currentChord.rootNote),
             getChordTypeName(currentChord.type),
             beatCounter + 1,
             chordBeats[currentChordSlot],
             currentChordSlot + 1,
             chordCount);
}
