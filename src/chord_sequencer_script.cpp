/**
 * Chord Sequencer Script Implementation
 * 
 * Inspired by Oxi One chord mode
 * Sequences through 4 chords, changing every 16 beats
 * Provides scale context for melodic sequencing
 */

#include "chord_sequencer_script.h"

// Note names for display
static const char* noteNames[12] = {
    "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
};

// Major scale intervals (semitones from root)
static const int8_t majorScaleIntervals[7] = {0, 2, 4, 5, 7, 9, 11};

// Natural minor scale intervals (semitones from root)
static const int8_t minorScaleIntervals[7] = {0, 2, 3, 5, 7, 8, 10};

ChordSequencerScript::ChordSequencerScript()
    : currentChordSlot(0)
    , beatCounter(0)
    , lastBeatMicros(0)
    , beatDurationMicros(500000) {  // 120 BPM default
    
    // Initialize default chord progression: Am - Dm - Fmaj - Gm
    // Classic minor key progression (i - iv - VI - VII)
    chords[0] = {9, CHORD_MINOR};   // A minor (Am)
    chords[1] = {2, CHORD_MINOR};   // D minor (Dm)
    chords[2] = {5, CHORD_MAJOR};   // F major (Fmaj)
    chords[3] = {7, CHORD_MINOR};   // G minor (Gm)
    
    // Initialize beat counts (default: equal distribution)
    chordBeats[0] = 16;
    chordBeats[1] = 8;
    chordBeats[2] = 8;
    chordBeats[3] = 8;
}

bool ChordSequencerScript::begin() {
    lastBeatMicros = micros();
    currentChordSlot = 0;
    beatCounter = 0;
    
    Serial.println("ChordSequencer: Initialized");
    Serial.println("  Default progression: Am - Dm - Fmaj - Gm");
    Serial.println("  Tempo: 120 BPM");
    Serial.println("  Beats per chord: 16");
    
    return true;
}

void ChordSequencerScript::update() {
    unsigned long currentMicros = micros();
    unsigned long elapsed = currentMicros - lastBeatMicros;
    
    // Check if it's time for next beat
    if (elapsed >= beatDurationMicros) {
        beatCounter++;
        
        // Wrap beat counter based on current chord's beat count
        if (beatCounter >= chordBeats[currentChordSlot]) {
            beatCounter = 0;
            // Auto-advance to next chord, wrap to first after the fourth
            currentChordSlot = (currentChordSlot + 1) % 4;
        }
        
        lastBeatMicros = currentMicros;
    }
}

void ChordSequencerScript::stop() {
    Serial.println("ChordSequencer: Stopped");
}

void ChordSequencerScript::setChord(uint8_t slot, uint8_t rootNote, ChordType type) {
    if (slot >= 4) return;
    if (rootNote >= 12) return;
    
    chords[slot].rootNote = rootNote;
    chords[slot].type = type;
    
    Serial.print("ChordSequencer: Chord ");
    Serial.print(slot + 1);
    Serial.print(" set to ");
    Serial.print(getNoteName(rootNote));
    Serial.println(getChordTypeName(type));
}

void ChordSequencerScript::getChord(uint8_t slot, uint8_t* rootNote, ChordType* type) {
    if (slot >= 4) return;
    if (rootNote) *rootNote = chords[slot].rootNote;
    if (type) *type = chords[slot].type;
}

void ChordSequencerScript::setTempo(float bpm) {
    if (bpm < 20.0f) bpm = 20.0f;
    if (bpm > 300.0f) bpm = 300.0f;
    beatDurationMicros = (unsigned long)((60.0f / bpm) * 1000000.0f);
}

void ChordSequencerScript::setChordBeats(uint8_t slot, uint8_t beats) {
    if (slot >= 4) return;
    if (beats < 1) beats = 1;
    if (beats > 32) beats = 32;
    chordBeats[slot] = beats;
}

uint8_t ChordSequencerScript::getChordBeats(uint8_t slot) const {
    if (slot >= 4) return 0;
    return chordBeats[slot];
}

void ChordSequencerScript::getCurrentScale(ScaleInfo* scale) {
    if (!scale) return;
    
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
    Chord& currentChord = chords[currentChordSlot];
    
    // Display current chord and beat
    snprintf(buffer, bufferSize,
             "SYMPHONY CHROD SEQUENCER\n%s%s\nBeat:%d/%d\nChord:%d/4",
             getNoteName(currentChord.rootNote),
             getChordTypeName(currentChord.type),
             beatCounter + 1,
             chordBeats[currentChordSlot],
             currentChordSlot + 1);
}
