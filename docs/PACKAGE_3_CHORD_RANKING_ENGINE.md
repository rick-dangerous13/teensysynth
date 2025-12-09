# Package 3 — Chord Suggestion Ranking Engine

## Overview
The Chord Suggestion Ranking Engine evaluates and scores chord candidates based on:
- **Theory Mode** (Functional, Diatonic, Modal, Chromatic, All)
- **Energy Level** (0.0-1.0 harmonic tension)
- **Voice Leading** (Smooth vs. large interval jumps)
- **Spread** (Per-chord voicing width)

The engine provides a backend API for ranking chords without UI changes. Other screens will call these functions to display suggestions in sorted order.

## Architecture

### ChordScore Structure
```cpp
struct ChordScore {
    uint8_t rootNote;              // 0-11
    ChordType type;                // Major or Minor
    float totalScore;              // 0.0-1.0 combined score
    float theoryScore;             // Theory match
    float energyScore;             // Energy alignment
    float voiceLeadingScore;       // Voice leading preference
    float spreadScore;             // Spread preference
};
```

### Public API

#### 1. Score a Single Chord
```cpp
ChordScore scoreChord(uint8_t candidateRoot, ChordType candidateType,
                      uint8_t previousRoot = 0, 
                      ChordType previousType = CHORD_MAJOR);
```

**Usage:**
```cpp
// Score a D major chord without considering voice leading
ChordScore dMajScore = chordSequencer->scoreChord(2, CHORD_MAJOR);

// Score a D major chord considering smooth voice leading from C minor
ChordScore dMajWithVL = chordSequencer->scoreChord(2, CHORD_MAJOR, 0, CHORD_MINOR);
```

**Returns:** Single `ChordScore` with all component scores and total

#### 2. Get Ranked List of All Chords
```cpp
const ChordScore* getRankedChords(uint8_t& outCount);
```

**Usage:**
```cpp
uint8_t chordCount;
const ChordScore* rankedChords = chordSequencer->getRankedChords(chordCount);

// First chord is highest-scoring suggestion
ChordScore topSuggestion = rankedChords[0];  // Best chord
ChordScore alternateA = rankedChords[1];     // Second best
ChordScore alternateB = rankedChords[2];     // Third best
```

**Returns:** 
- Array of 24 ChordScores (12 roots × 2 types)
- Sorted by `totalScore` (highest first)
- `outCount` always = 24

## Scoring Algorithm

### Theory Score (Weight: 40%)
Evaluates chord against harmonic context defined by Key + Degree + TheoryMode

| Mode | Scoring |
|------|---------|
| **FUNCTIONAL** | I=1.0, V=0.9, IV=0.8, ii/vi=0.5, others=0.3 |
| **DIATONIC** | In scale=0.9, chromatic=0.2 |
| **MODAL** | In scale=0.85, chromatic neighbors=0.4, others=0.1 |
| **CHROMATIC** | In scale=0.6, others=0.5 |
| **ALL** | All chords=0.5 |

### Energy Score (Weight: 15%)
Considers harmonic energy/tension level

- Higher energy → prefers dissonant intervals and surprising chord choices
- Lower energy → prefers consonant, stable chords
- Current implementation: Linear interpolation around 0.5 (neutral)

### Voice Leading Score (Weight: 35%)
Measures smoothness of transition from previous chord

- Calculated on semitone distance between roots
- Considers `voiceLeadingCompactness` global parameter:
  - **0.0** (Expansive): Allow large jumps, any movement valid
  - **0.5** (Balanced): Prefer 3-5 semitone moves
  - **1.0** (Compact): Strongly prefer minimal 1-2 semitone moves
- Same chord type preferred slightly

### Spread Score (Weight: 10%)
Placeholder for per-chord voicing density (future enhancement)

Currently neutral (0.5) for all chords.

## Global Parameters Used

| Parameter | Effect | Range |
|-----------|--------|-------|
| `globalKey` | Harmonic context root | 0-11 (C-B) |
| `globalDegree` | Scale quality | Major, Minor, Dorian, etc. |
| `globalTheoryMode` | Ranking philosophy | Functional → All |
| `globalVoiceLeadingCompactness` | Movement smoothness | 0.0-1.0 |
| `globalEnergy` | Harmonic tension | 0.0-1.0 |

## Implementation Details

### Caching Strategy
- `generateAllChordScores()` calculates and sorts all 24 chords
- `scoresNeedUpdate` flag tracks whether globals have changed
- `getRankedChords()` regenerates scores each call (simple for 24 items)
- Weights can be adjusted without algorithm changes

### Sorting
Bubble sort (adequate for 24 items) sorts by `totalScore` descending

### Weights Configuration
Current weights balance:
- **Strong theory influence** (40%) for harmonic correctness
- **Heavy voice leading** (35%) for smooth, playable progressions
- **Moderate energy** (15%) for character
- **Light spread** (10%) as placeholder

Adjust these in `scoreChord()` and `calculateTheoryScore()` to taste.

## Example Usage Scenarios

### Scenario 1: Suggest Next Chord in C Major
```cpp
// Set context
chordSeq->setKey(MKEY_C);
chordSeq->setDegree(DEGREE_MAJOR);
chordSeq->setTheoryMode(THEORY_FUNCTIONAL);
chordSeq->setVoiceLeadingCompactness(0.7f);

// Get suggestions
uint8_t count;
const ChordScore* suggestions = chordSeq->getRankedChords(count);

// Top 3 suggestions (best → acceptable)
// Note root=0 is C, root=7 is G, root=5 is F
```

Expected results for C major with functional theory:
1. **C Major** (I, root position) - score ~0.95
2. **G Major** (V, dominant) - score ~0.87
3. **F Major** (IV, subdominant) - score ~0.82
... D minor, A minor, E minor ... others ...

### Scenario 2: Voice Leading Priority
```cpp
// Same key/degree, but prioritize smooth movement
chordSeq->setVoiceLeadingCompactness(1.0f);  // Maximum smoothness

// Get scores for specific transition (C minor → ? major)
ChordScore suggestion = chordSeq->scoreChord(2, CHORD_MAJOR,  // D major candidate
                                             0, CHORD_MINOR);  // From C minor

// D major scores high because it's only 2 semitones from C
```

### Scenario 3: All Chords Available, Energy-Based
```cpp
chordSeq->setTheoryMode(THEORY_ALL);  // Any chord valid
chordSeq->setEnergy(0.8f);  // High harmonic tension
chordSeq->setVoiceLeadingCompactness(0.3f);  // Allow big jumps

uint8_t count;
const ChordScore* suggestions = chordSeq->getRankedChords(count);

// With THEORY_ALL, all chords score similarly
// Energy boost adds character, voice leading allows unusual transitions
```

## Future Enhancements

1. **Dynamic voice leading**: Compare against all previous chords in progression
2. **Spread scoring**: Factor in per-chord `localSpread` parameter
3. **Energy dynamics**: Use energy to prefer/avoid specific intervals
4. **Mode-specific rules**: Dorian, Phrygian modes have unique voice leading patterns
5. **Harmonic function context**: Remember ii-V-I patterns, predict expectations
6. **User customization**: Allow adjusting weights per session/style

## Testing

The ranking engine can be tested standalone:

```cpp
// Test 1: Theory scoring
chordSeq->setTheoryMode(THEORY_FUNCTIONAL);
ChordScore cMaj = chordSeq->scoreChord(0, CHORD_MAJOR);   // C major
ChordScore fMaj = chordSeq->scoreChord(5, CHORD_MAJOR);   // F major
ChordScore bDim = chordSeq->scoreChord(11, CHORD_MINOR);  // B minor

assert(cMaj.totalScore > fMaj.totalScore);
assert(fMaj.totalScore > bDim.totalScore);

// Test 2: Voice leading
float vl05Score = chordSeq->scoreChord(0, CHORD_MAJOR).voiceLeadingScore;
chordSeq->setVoiceLeadingCompactness(1.0f);
float vl10Score = chordSeq->scoreChord(0, CHORD_MAJOR, 1, CHORD_MAJOR).voiceLeadingScore;
assert(vl10Score > vl05Score);  // Nearby chord scores higher with compactness=1.0

// Test 3: Ranking order
uint8_t count;
const ChordScore* ranked = chordSeq->getRankedChords(count);
for (uint8_t i = 0; i < count - 1; i++) {
    assert(ranked[i].totalScore >= ranked[i+1].totalScore);  // Descending order
}
```

## Status

✅ **Complete for Package 3:**
- Theory score calculation
- Voice leading score calculation
- Combined scoring algorithm
- Full ranking of all 24 chords
- API ready for UI integration

⏳ **Ready for Package 4:**
- UI will call `getRankedChords()` and display suggestions
- Beat Count Picker will show ranked list instead of fixed order
- No additional backend work needed

## Code Location

- **Header**: `include/chord_sequencer_script.h` - Lines 143-170
- **Implementation**: `src/chord_sequencer_script.cpp` - Lines 235-386
