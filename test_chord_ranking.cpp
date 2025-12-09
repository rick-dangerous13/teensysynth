/**
 * Chord Ranking Engine Test
 * 
 * Verification tests for the ranking engine scores
 * Can be compiled and run independently to verify music theory logic
 */

#include <stdio.h>
#include <math.h>
#include "../include/chord_ranking_engine.h"

// Helper to print chord name
const char* getChordName(uint8_t root, ChordType type) {
    static const char* noteNames[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    static char buffer[8];
    sprintf(buffer, "%s%s", noteNames[root], type == CHORD_MAJOR ? "" : "m");
    return buffer;
}

void printRankedChords(const RankedChord* results, uint8_t count) {
    printf("\n=== Ranked Chords ===\n");
    printf("Rank | Chord      | Total | Theory | Energy | VL    | Spread\n");
    printf("-----|------------|-------|--------|--------|-------|-------\n");
    
    for (uint8_t i = 0; i < count && i < 10; i++) {
        printf("%2d   | %-10s | %.3f | %.3f   | %.3f   | %.3f  | %.3f\n",
            i + 1,
            getChordName(results[i].rootNote, results[i].type),
            results[i].totalScore,
            results[i].theoryScore,
            results[i].energyScore,
            results[i].voiceLeadingScore,
            results[i].spreadScore);
    }
}

void testDiatonicScale() {
    printf("\n=== Test: Diatonic Scale Generation ===\n");
    
    ChordRankingEngine engine;
    GlobalParameters globals = {
        .key = 0,  // C
        .degree = DEGREE_MAJOR,
        .theoryMode = THEORY_FUNCTIONAL,
        .voiceLeadingCompactness = 0.5f,
        .energy = 0.5f
    };
    
    engine.setContext(globals, 0, CHORD_MAJOR);
    
    printf("C Major scale should have: C, D, E, F, G, A, B\n");
    printf("Diatonic chords: Cmaj, Dmin, Emin, Fmaj, Gmaj, Amin, Bdim\n\n");
    
    // Test theory scores for diatonic chords
    printf("Diatonic chords should score high (>0.85):\n");
    for (uint8_t root = 0; root < 12; root++) {
        for (uint8_t type = 0; type < 2; type++) {
            float score = engine.getTheoryScore(root, (ChordType)type);
            if (score > 0.85f) {
                printf("  %s: %.3f\n", getChordName(root, (ChordType)type), score);
            }
        }
    }
    
    printf("\nNon-diatonic chords should score lower (<0.50):\n");
    for (uint8_t root = 0; root < 12; root++) {
        for (uint8_t type = 0; type < 2; type++) {
            float score = engine.getTheoryScore(root, (ChordType)type);
            if (score < 0.50f) {
                printf("  %s: %.3f\n", getChordName(root, (ChordType)type), score);
            }
        }
    }
}

void testEnergyScoring() {
    printf("\n=== Test: Energy/Tension Matching ===\n");
    
    ChordRankingEngine engine;
    GlobalParameters globals;
    globals.key = 0;  // C
    globals.degree = DEGREE_MAJOR;
    globals.theoryMode = THEORY_FUNCTIONAL;
    globals.voiceLeadingCompactness = 0.5f;
    
    // Test calm energy preference
    printf("Calm energy (0.0) - should prefer C (I), F (IV), A (VI):\n");
    globals.energy = 0.0f;
    engine.setContext(globals, 0, CHORD_MAJOR);
    
    float cMajorEnergy = engine.getEnergyScore(0, CHORD_MAJOR);      // I
    float fMajorEnergy = engine.getEnergyScore(5, CHORD_MAJOR);      // IV
    float gMajorEnergy = engine.getEnergyScore(7, CHORD_MAJOR);      // V
    
    printf("  C (I/Tonic):      %.3f\n", cMajorEnergy);
    printf("  F (IV/Subdominant): %.3f\n", fMajorEnergy);
    printf("  G (V/Dominant):   %.3f\n", gMajorEnergy);
    
    // Test energetic preference
    printf("\nEnergetic (1.0) - should prefer G (V), B (VII):\n");
    globals.energy = 1.0f;
    engine.setContext(globals, 0, CHORD_MAJOR);
    
    cMajorEnergy = engine.getEnergyScore(0, CHORD_MAJOR);
    gMajorEnergy = engine.getEnergyScore(7, CHORD_MAJOR);
    float bMinorEnergy = engine.getEnergyScore(11, CHORD_MINOR);
    
    printf("  C (I/Tonic):      %.3f\n", cMajorEnergy);
    printf("  G (V/Dominant):   %.3f\n", gMajorEnergy);
    printf("  B (VII/Leading):  %.3f\n", bMinorEnergy);
}

void testVoiceLeadingScoring() {
    printf("\n=== Test: Voice Leading Smoothness ===\n");
    
    ChordRankingEngine engine;
    GlobalParameters globals = {
        .key = 0,  // C
        .degree = DEGREE_MAJOR,
        .theoryMode = THEORY_FUNCTIONAL,
        .voiceLeadingCompactness = 1.0f,  // Compact preference
        .energy = 0.5f
    };
    
    engine.setContext(globals, 0, CHORD_MAJOR);  // Current: C
    
    printf("Compact voice leading (1.0) - close intervals should score high:\n");
    
    // Test close intervals
    float dScore = engine.getVoiceLeadingScore(2, CHORD_MAJOR);  // D (2 semitones)
    float bScore = engine.getVoiceLeadingScore(11, CHORD_MAJOR); // B (1 semitone down)
    float fScore = engine.getVoiceLeadingScore(5, CHORD_MAJOR);  // F (5 semitones, or 7 reverse)
    float gScore = engine.getVoiceLeadingScore(7, CHORD_MAJOR);  // G (7 semitones, or 5 reverse)
    
    printf("  C→D (2 steps):    %.3f\n", dScore);
    printf("  C→B (1 step):     %.3f\n", bScore);
    printf("  C→F (5 semitones): %.3f\n", fScore);
    printf("  C→G (5 semitones): %.3f\n", gScore);
    
    printf("\nSpread voice leading (0.0) - distant intervals should score high:\n");
    globals.voiceLeadingCompactness = 0.0f;
    engine.setContext(globals, 0, CHORD_MAJOR);
    
    dScore = engine.getVoiceLeadingScore(2, CHORD_MAJOR);
    gScore = engine.getVoiceLeadingScore(7, CHORD_MAJOR);
    
    printf("  C→D (2 steps):    %.3f\n", dScore);
    printf("  C→G (5 semitones): %.3f\n", gScore);
}

void testTheoryModes() {
    printf("\n=== Test: Theory Mode Preferences ===\n");
    
    ChordRankingEngine engine;
    GlobalParameters globals = {
        .key = 0,  // C
        .degree = DEGREE_MAJOR,
        .theoryMode = THEORY_FUNCTIONAL,
        .voiceLeadingCompactness = 0.5f,
        .energy = 0.5f
    };
    
    printf("THEORY_FUNCTIONAL - prefer I/IV/V:\n");
    globals.theoryMode = THEORY_FUNCTIONAL;
    engine.setContext(globals, 0, CHORD_MAJOR);
    
    RankedChord results[24];
    uint8_t count = engine.rankChords(results, 24);
    printRankedChords(results, count);
    
    printf("\n\nTHEORY_DIATONIC - only scale tones:\n");
    globals.theoryMode = THEORY_DIATONIC;
    engine.setContext(globals, 0, CHORD_MAJOR);
    
    count = engine.rankChords(results, 24);
    printRankedChords(results, count);
    
    printf("\n\nTHEORY_CHROMATIC - all chords, varied by distance:\n");
    globals.theoryMode = THEORY_CHROMATIC;
    engine.setContext(globals, 0, CHORD_MAJOR);
    
    count = engine.rankChords(results, 24);
    printRankedChords(results, count);
}

void testScaleDegrees() {
    printf("\n=== Test: Scale Degrees (Modes) ===\n");
    
    ChordRankingEngine engine;
    GlobalParameters globals = {
        .key = 0,  // C
        .degree = DEGREE_MAJOR,
        .theoryMode = THEORY_DIATONIC,
        .voiceLeadingCompactness = 0.5f,
        .energy = 0.5f
    };
    
    static const char* degreeNames[] = {
        "Major (Ionian)",
        "Minor (Aeolian)",
        "Dorian",
        "Phrygian",
        "Lydian",
        "Mixolydian",
        "Locrian"
    };
    
    for (uint8_t degree = 0; degree < 7; degree++) {
        printf("\n%s in C:\n", degreeNames[degree]);
        globals.degree = (ScaleDegree)degree;
        engine.setContext(globals, 0, CHORD_MAJOR);
        
        RankedChord results[24];
        uint8_t count = engine.rankChords(results, 24);
        
        // Show top 3
        for (uint8_t i = 0; i < 3 && i < count; i++) {
            printf("  %d. %s (%.3f)\n", i + 1, 
                getChordName(results[i].rootNote, results[i].type),
                results[i].totalScore);
        }
    }
}

int main() {
    printf("====================================\n");
    printf("Chord Ranking Engine Tests\n");
    printf("====================================\n");
    
    testDiatonicScale();
    testEnergyScoring();
    testVoiceLeadingScoring();
    testTheoryModes();
    testScaleDegrees();
    
    printf("\n====================================\n");
    printf("All tests completed!\n");
    printf("====================================\n");
    
    return 0;
}
