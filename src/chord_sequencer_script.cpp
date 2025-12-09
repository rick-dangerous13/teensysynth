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
    , beatDurationMicros(500000)  // 120 BPM default
    , scoresNeedUpdate(true) {

    // Initialize global parameters with sensible defaults
    globals.key = MKEY_C;
    globals.degree = DEGREE_MAJOR;
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

// ============ CHORD SUGGESTION RANKING ENGINE (Package 3) ============

float ChordSequencerScript::calculateTheoryScore(uint8_t chordRoot, ChordType chordType) {
    // Score chord based on theory mode and current key/degree
    // Higher score = better fit for current harmonic context
    
    const int8_t* scaleIntervals = (globals.degree == DEGREE_MAJOR || globals.degree == DEGREE_MINOR) 
        ? majorScaleIntervals 
        : majorScaleIntervals;  // Simplified: use major scale for all for now
    
    // Calculate chord root's interval from key
    int8_t interval = (chordRoot - globals.key + 12) % 12;
    
    switch (globals.theoryMode) {
        case THEORY_FUNCTIONAL: {
            // Prefer tonic (I), subdominant (IV), dominant (V) in major key
            // For C major: C (0) = 1.0, F (5) = 0.8, G (7) = 0.9, others = 0.3
            if (interval == 0) return 1.0f;      // Tonic
            if (interval == 5) return 0.8f;      // Subdominant (IV)
            if (interval == 7) return 0.9f;      // Dominant (V)
            if (interval == 2 || interval == 9) return 0.5f;  // ii, vi
            return 0.3f;
        }
        case THEORY_DIATONIC: {
            // Any chord built on diatonic scale degree scores high
            // Non-diatonic scores low
            for (int i = 0; i < 7; i++) {
                if (scaleIntervals[i] == interval) return 0.9f;
            }
            return 0.2f;
        }
        case THEORY_MODAL: {
            // Similar to diatonic but more nuanced
            for (int i = 0; i < 7; i++) {
                if (scaleIntervals[i] == interval) return 0.85f;
            }
            // Chromatic neighbors get some score
            if ((interval + 1) % 12 == scaleIntervals[0] || (interval - 1 + 12) % 12 == scaleIntervals[6]) {
                return 0.4f;
            }
            return 0.1f;
        }
        case THEORY_CHROMATIC: {
            // All chords valid, slight preference for diatonic
            for (int i = 0; i < 7; i++) {
                if (scaleIntervals[i] == interval) return 0.6f;
            }
            return 0.5f;
        }
        case THEORY_ALL:
        default: {
            // All chords equally valid
            return 0.5f;
        }
    }
}

float ChordSequencerScript::calculateVoiceLeadingScore(uint8_t prevRoot, ChordType prevType,
                                                       uint8_t nextRoot, ChordType nextType) {
    // Score based on voice leading distance (smooth transitions = higher score)
    // voiceLeadingCompactness: 0.0 = allow large jumps, 1.0 = prefer minimal movement
    
    // Calculate semitone distance between roots (minimal path)
    int8_t rootDistance = (nextRoot - prevRoot + 12) % 12;
    if (rootDistance > 6) rootDistance = 12 - rootDistance;  // Use shorter path
    
    // Preference is quadratic based on distance and compactness setting
    // At compactness=0.5: prefer moves of 3-5 semitones
    // At compactness=1.0: strongly prefer moves of 1-2 semitones
    float distancePenalty = (float)rootDistance / 12.0f;
    
    // Type matching: same type slightly preferred
    float typeBonus = (prevType == nextType) ? 0.1f : 0.0f;
    
    // Calculate score: prefer small distances when compactness is high
    float voiceLeadingScore = (1.0f - distancePenalty) * (0.5f + globals.voiceLeadingCompactness * 0.5f) + typeBonus;
    
    return constrain(voiceLeadingScore, 0.0f, 1.0f);
}

void ChordSequencerScript::generateAllChordScores() {
    // Generate scores for all 24 possible chords (12 roots × 2 types)
    // Scores are based on current globals but independent of previous chord
    
    uint8_t scoreIdx = 0;
    for (uint8_t root = 0; root < 12; root++) {
        for (uint8_t typeIdx = 0; typeIdx < 2; typeIdx++) {
            ChordType type = (typeIdx == 0) ? CHORD_MAJOR : CHORD_MINOR;
            
            ChordScore& score = allChordScores[scoreIdx];
            score.rootNote = root;
            score.type = type;
            
            // Calculate individual scores
            score.theoryScore = calculateTheoryScore(root, type);
            score.energyScore = 0.5f;  // Placeholder: energy scoring could be enhanced
            score.voiceLeadingScore = 0.5f;  // Neutral when not comparing to previous
            score.spreadScore = 0.5f;  // Placeholder: spread is per-chord instance
            
            // Combine scores with weighted average (adjust weights to taste)
            float theoryWeight = 0.4f;
            float energyWeight = 0.2f;
            float voiceLeadingWeight = 0.2f;
            float spreadWeight = 0.2f;
            
            score.totalScore = (score.theoryScore * theoryWeight +
                              score.energyScore * energyWeight +
                              score.voiceLeadingScore * voiceLeadingWeight +
                              score.spreadScore * spreadWeight);
            
            scoreIdx++;
        }
    }
    
    // Sort by totalScore (bubble sort, fine for 24 items)
    for (int i = 0; i < 24; i++) {
        for (int j = i + 1; j < 24; j++) {
            if (allChordScores[j].totalScore > allChordScores[i].totalScore) {
                // Swap
                ChordScore temp = allChordScores[i];
                allChordScores[i] = allChordScores[j];
                allChordScores[j] = temp;
            }
        }
    }
    
    scoresNeedUpdate = false;
}

ChordSequencerScript::ChordScore ChordSequencerScript::scoreChord(uint8_t candidateRoot, ChordType candidateType,
                                                                   uint8_t previousRoot, ChordType previousType) {
    // Score a single chord candidate, optionally considering previous chord for voice leading
    ChordScore score;
    score.rootNote = candidateRoot;
    score.type = candidateType;
    
    score.theoryScore = calculateTheoryScore(candidateRoot, candidateType);
    score.energyScore = 0.5f + (globals.energy - 0.5f) * 0.2f;  // Slight energy modulation
    score.voiceLeadingScore = calculateVoiceLeadingScore(previousRoot, previousType, candidateRoot, candidateType);
    score.spreadScore = 0.5f;  // Will vary per-chord instance, not applicable here
    
    // Weighted combination
    float theoryWeight = 0.4f;
    float energyWeight = 0.15f;
    float voiceLeadingWeight = 0.35f;
    float spreadWeight = 0.1f;
    
    score.totalScore = (score.theoryScore * theoryWeight +
                       score.energyScore * energyWeight +
                       score.voiceLeadingScore * voiceLeadingWeight +
                       score.spreadScore * spreadWeight);
    
    return score;
}

const ChordSequencerScript::ChordScore* ChordSequencerScript::getRankedChords(uint8_t& outCount) {
    // Return cached ranked list of all chords
    // Regenerate if globals have changed significantly
    
    // In a full implementation, you might track which globals changed
    // For now, always regenerate (fine on an embedded system with reasonable update rate)
    generateAllChordScores();
    
    outCount = 24;
    return allChordScores;
}

