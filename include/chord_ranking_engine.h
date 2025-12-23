/**
 * Chord Ranking Engine
 * 
 * Implements intelligent chord suggestion scoring based on:
 * - Scale/Key compatibility (diatonic vs chromatic)
 * - Theory mode preferences (functional, modal, chromatic)
 * - Energy/harmonic tension levels
 * - Voice leading smoothness
 * - Spread preferences
 */

#ifndef CHORD_RANKING_ENGINE_H
#define CHORD_RANKING_ENGINE_H

#include <Arduino.h>
#include "chord_sequencer_script.h"
#include "config.h"

// Chord candidate with ranking score
struct RankedChord {
    uint8_t rootNote;           // 0-11 (C=0, C#=1, etc.)
    ChordType type;             // Major or Minor
    float totalScore;           // Composite ranking score (0.0-1.0)
    float theoryScore;          // Scale/theory fitness
    float energyScore;          // Energy/tension match
    float voiceLeadingScore;    // Smoothness from current chord
    float spreadScore;          // Spread preference match
};

class ChordRankingEngine {
public:
    ChordRankingEngine();
    
    // Initialize with current musical context
    void setContext(const GlobalParameters& globals, 
                   uint8_t currentChordRoot, 
                   ChordType currentChordType);
    
    // Set local overrides for the next chord (use 255/special values for "use global")
    void setLocalOverrides(uint8_t localTheoryMode, float localSpread, uint8_t localInversion);
    
    // Generate ranked chord list (up to MAX_CHORD_SLOTS candidates)
    // Returns number of chords ranked
    uint8_t rankChords(RankedChord* results, uint8_t maxResults);
    
    // Get individual scores for debugging
    float getTheoryScore(uint8_t rootNote, ChordType type) const;
    float getEnergyScore(uint8_t rootNote, ChordType type) const;
    float getVoiceLeadingScore(uint8_t rootNote, ChordType type) const;
    float getSpreadScore(uint8_t rootNote, ChordType type) const;
    
private:
    // Current context
    GlobalParameters globals;
    uint8_t currentChordRoot;
    ChordType currentChordType;
    
    // Local overrides
    uint8_t effectiveTheoryMode;
    float effectiveSpread;
    uint8_t effectiveInversion;
    
    // Cached scale information
    int8_t diatonicScale[7];    // Scale degrees (semitones from root)
    uint8_t scaleRoot;
    
    // Helper functions
    void generateDiatonicScale(uint8_t root, ScaleDegree degree);
    bool isScaleTone(uint8_t note, uint8_t root, ScaleDegree degree) const;
    bool isDiatonicChord(uint8_t rootNote, ChordType type) const;
    
    // Score component calculations
    float computeTheoryMatchScore(uint8_t rootNote, ChordType type) const;
    float computeEnergyScore(uint8_t rootNote, ChordType type) const;
    float computeVoiceLeadingScore(uint8_t rootNote, ChordType type) const;
    float computeSpreadScore(uint8_t rootNote, ChordType type) const;
    
    // Chord function/role classification
    enum ChordFunction {
        FUNCTION_TONIC = 0,         // I (stable)
        FUNCTION_SUBDOMINANT,       // IV/II (preparation)
        FUNCTION_DOMINANT,          // V/VII (tension)
        FUNCTION_BORROWED,          // chords from parallel key
        FUNCTION_CHROMATIC          // non-diatonic
    };
    ChordFunction getChordFunction(uint8_t rootNote, ChordType type) const;
    
    // Note helpers
    uint8_t semitoneDistance(uint8_t from, uint8_t to) const;
    uint8_t intervalQuality(uint8_t interval) const;  // 0-11 semitones
};

#endif // CHORD_RANKING_ENGINE_H
