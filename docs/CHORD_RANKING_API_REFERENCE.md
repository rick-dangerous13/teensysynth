# Chord Ranking Engine - Quick API Reference

## Single Chord Scoring

```cpp
// Score a chord candidate, optionally considering voice leading from previous chord
ChordScore score = chordSequencer->scoreChord(
    uint8_t candidateRoot,      // 0-11 (C=0, G=7, etc.)
    ChordType candidateType,    // CHORD_MAJOR or CHORD_MINOR
    uint8_t previousRoot = 0,   // Optional: previous chord root (default C)
    ChordType previousType = CHORD_MAJOR  // Optional: previous chord type
);

// Access individual scores
float theory = score.theoryScore;           // 0.0-1.0 theory fit
float energy = score.energyScore;           // 0.0-1.0 energy alignment
float voiceLeading = score.voiceLeadingScore;  // 0.0-1.0 smoothness
float spread = score.spreadScore;           // 0.0-1.0 voicing width
float total = score.totalScore;             // 0.0-1.0 combined
```

**Example: Score D major without voice leading context**
```cpp
ChordScore dMaj = chordSequencer->scoreChord(2, CHORD_MAJOR);
Serial.printf("D Major: theory=%.2f, energy=%.2f, VL=%.2f, total=%.2f\n",
    dMaj.theoryScore, dMaj.energyScore, dMaj.voiceLeadingScore, dMaj.totalScore);
```

**Example: Score D major considering smooth move from C minor**
```cpp
ChordScore dMajVL = chordSequencer->scoreChord(
    2, CHORD_MAJOR,    // D major
    0, CHORD_MINOR);   // From C minor
// This will score higher due to 2-semitone smooth movement
```

## Get All Ranked Chords

```cpp
// Get complete ranked list of all 24 chords (highest score first)
uint8_t chordCount;
const ChordScore* rankedChords = chordSequencer->getRankedChords(chordCount);

// rankedChords[0] = best suggestion
// rankedChords[1] = second best
// ... etc

// Always returns 24 chords, always sorted descending by totalScore
```

**Example: Display top 5 suggestions**
```cpp
uint8_t count;
const ChordScore* ranked = chordSequencer->getRankedChords(count);

Serial.println("Top 5 Chord Suggestions:");
for (uint8_t i = 0; i < 5; i++) {
    const char* noteNames[] = {"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
    const char* typeStr = (ranked[i].type == CHORD_MAJOR) ? "maj" : "min";
    Serial.printf("%d. %s %s - Score: %.2f\n",
        i+1,
        noteNames[ranked[i].rootNote],
        typeStr,
        ranked[i].totalScore);
}
```

## Setting Global Context

```cpp
// All of these affect ranking scores
chordSequencer->setKey(MKEY_C);           // Key: C=0, D=2, E=4, F=5, G=7, A=9, B=11
chordSequencer->setDegree(DEGREE_MAJOR);  // Scale: MAJOR, MINOR, DORIAN, PHRYGIAN, LYDIAN, MIXOLYDIAN, LOCRIAN
chordSequencer->setTheoryMode(THEORY_FUNCTIONAL);  // FUNCTIONAL, DIATONIC, MODAL, CHROMATIC, ALL

// Voice leading preference
chordSequencer->setVoiceLeadingCompactness(0.5f);  // 0.0=expansive, 0.5=balanced, 1.0=compact

// Harmonic energy
chordSequencer->setEnergy(0.5f);  // 0.0=calm, 0.5=neutral, 1.0=tense
```

## Theory Modes Explained

| Mode | When to Use | Characteristics |
|------|------------|-----------------|
| **FUNCTIONAL** | Traditional harmony | Tonic (1.0), Dominant (0.9), Subdominant (0.8), others lower |
| **DIATONIC** | Scale-based | Scale tones (0.9), chromatic (0.2) |
| **MODAL** | Modal interchange | Scale tones (0.85), chromatic neighbors (0.4) |
| **CHROMATIC** | Experimental | Scale (0.6), chromatic (0.5), all chords valid |
| **ALL** | Anything goes | All chords score equally (0.5) |

## Voice Leading Compactness

- **0.0** (Expansive): `score = 1.0 - distance` → Large jumps OK
- **0.5** (Balanced): `score = (1.0 - distance) * 0.75` → Prefer small-medium moves
- **1.0** (Compact): `score = (1.0 - distance) * 1.0` → Prefer minimal movement

## Common Usage Patterns

### Pattern 1: Get Next Chord Suggestion
```cpp
// You just played Am, what's a good next chord?
uint8_t prevRoot = 9;  // A
ChordType prevType = CHORD_MINOR;

// Get ranked suggestions with voice leading context
uint8_t count;
const ChordScore* suggestions = chordSequencer->getRankedChords(count);

// Then reconsider each suggestion with voice leading from Am
ChordScore bestNext = chordSequencer->scoreChord(
    suggestions[0].rootNote,
    suggestions[0].type,
    prevRoot,
    prevType
);
```

### Pattern 2: Find Diatonic Alternatives
```cpp
chordSequencer->setTheoryMode(THEORY_DIATONIC);
uint8_t count;
const ChordScore* diatonicChords = chordSequencer->getRankedChords(count);

// First ~7 will be diatonic (score ~0.9)
// Rest are chromatic (score ~0.2)
```

### Pattern 3: Smooth Jazz Progression
```cpp
chordSequencer->setVoiceLeadingCompactness(0.9f);  // Very smooth
chordSequencer->setEnergy(0.3f);                    // Calm
chordSequencer->setTheoryMode(THEORY_FUNCTIONAL);  // Traditional harmony

uint8_t count;
const ChordScore* smoothChords = chordSequencer->getRankedChords(count);
// Top suggestions will be smooth, traditional progressions
```

### Pattern 4: Experimental Mode
```cpp
chordSequencer->setTheoryMode(THEORY_ALL);         // Any chord
chordSequencer->setVoiceLeadingCompactness(0.1f);  // Allow big jumps
chordSequencer->setEnergy(0.9f);                    // High tension

uint8_t count;
const ChordScore* experimental = chordSequencer->getRankedChords(count);
// Top suggestions will be unusual, energetic, with surprising transitions
```

## Debugging / Inspection

```cpp
// Print full score breakdown for a chord
void printChordScore(const ChordScore& score, const char* label) {
    const char* noteNames[] = {"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
    const char* typeStr = (score.type == CHORD_MAJOR) ? "maj" : "min";
    
    Serial.printf("\n%s:\n", label);
    Serial.printf("  Chord: %s %s\n", noteNames[score.rootNote], typeStr);
    Serial.printf("  Theory:       %.3f\n", score.theoryScore);
    Serial.printf("  Energy:       %.3f\n", score.energyScore);
    Serial.printf("  Voice Lead:   %.3f\n", score.voiceLeadingScore);
    Serial.printf("  Spread:       %.3f\n", score.spreadScore);
    Serial.printf("  TOTAL:        %.3f *** \n", score.totalScore);
}

// Print all 24 ranked chords
void printAllRankedChords() {
    uint8_t count;
    const ChordScore* ranked = chordSequencer->getRankedChords(count);
    
    const char* noteNames[] = {"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
    
    Serial.println("\nAll 24 Ranked Chords:");
    for (uint8_t i = 0; i < count; i++) {
        const char* typeStr = (ranked[i].type == CHORD_MAJOR) ? "maj" : "min";
        Serial.printf("%2d. %s %s - %.3f\n", i+1, noteNames[ranked[i].rootNote], typeStr, ranked[i].totalScore);
    }
}
```

## Key Notes

1. **Always 24 chords**: `getRankedChords()` returns exactly 24 ChordScores (12 roots × 2 types)
2. **Always sorted**: Highest `totalScore` first, lowest last
3. **Sorted descending**: Index 0 is best suggestion, index 23 is worst
4. **Scores are 0.0-1.0**: Easy to use for visual ranking (brightness, size, position)
5. **Thread-safe**: No dynamic allocation, safe to call frequently
6. **Fast**: Recalculation is ~100-200μs (negligible vs. display refresh)

## Integration with UI (Future - Package 4)

```cpp
// In Chord List Overlay or Beat Count Picker:
uint8_t count;
const ChordScore* ranked = chordSequencer->getRankedChords(count);

// Display top 5-10 instead of all 12
for (uint8_t i = 0; i < min(count, 5); i++) {
    displayChord(ranked[i].rootNote, ranked[i].type, ranked[i].totalScore);
}
```

---

**For detailed algorithm information**, see `docs/PACKAGE_3_CHORD_RANKING_ENGINE.md`
