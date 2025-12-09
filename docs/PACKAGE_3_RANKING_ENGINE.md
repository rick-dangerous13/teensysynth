# Package 3: Chord Suggestion Ranking Engine

## Overview

The Chord Ranking Engine is a backend music theory system that scores chord candidates based on multiple harmonic criteria. It uses the Key, Degree, and other global parameters to intelligently rank which chords would be musically appropriate suggestions for the next chord in a progression.

## Architecture

### Core Components

1. **ChordRankingEngine** (`include/chord_ranking_engine.h`, `src/chord_ranking_engine.cpp`)
   - Standalone scoring engine
   - No UI dependencies
   - Evaluates all 24 possible chords (12 roots × 2 types)

2. **Integration Layer** (ScriptManager)
   - `rankChordsForSequencer()` method
   - Bridges UI/sequencer state to ranking engine
   - Handles context setup and local overrides

3. **Score Components**
   - Theory Match (40% weight)
   - Energy/Tension (25% weight)
   - Voice Leading (20% weight)
   - Spread Preference (15% weight)

## Key Features

### 1. Scale Generation (Mode Support)

The engine generates diatonic scales for all 7 modes:

```cpp
enum ScaleDegree {
    DEGREE_MAJOR = 0,       // Ionian: W-W-H-W-W-W-H
    DEGREE_MINOR,           // Aeolian: W-H-W-W-H-W-W
    DEGREE_DORIAN,          // W-H-W-W-W-H-W
    DEGREE_PHRYGIAN,        // H-W-W-W-H-W-W
    DEGREE_LYDIAN,          // W-W-W-H-W-W-H
    DEGREE_MIXOLYDIAN,      // W-W-H-W-W-H-W
    DEGREE_LOCRIAN           // H-W-W-H-W-W-W
};
```

Each mode defines a unique scale pattern that affects harmony theory:
- **Major**: Functional harmony (I-IV-V progressions)
- **Minor**: Darker, introspective feel
- **Dorian**: Jazzy, floating quality
- **Lydian**: Bright, raised 4th degree
- **Etc.**

### 2. Theory Matching Scores

#### Diatonic vs. Chromatic
- Diatonic chords (from the scale) = 0.9 base score
- Chromatic chords (outside scale) = 0.3 base score

#### Theory Mode Adjustments

**THEORY_FUNCTIONAL** (40% boost for I/IV/V, -30% penalty for chromatic)
- Prioritizes functional harmony
- Strong preference for tonic, subdominant, dominant
- Classical Western harmony

**THEORY_DIATONIC** (strict scale constraint)
- Only diatonic chords considered
- Heavy penalty for chromatic
- Very predictable progressions

**THEORY_MODAL** (scale flexibility)
- Encourages diatonic but allows modal interchange
- Medium penalty for borrowed chords
- Jazz/contemporary feel

**THEORY_CHROMATIC** (any chord, varies by distance)
- All chords viable
- Score based on semitone distance
- Very chromatic possibilities

**THEORY_ALL** (equal opportunity)
- All 24 chords treated equally
- Useful for experimental/atonal music

### 3. Energy/Tension Matching

Chord functions have inherent tension levels:

```
FUNCTION_TONIC        → 0.2 (very stable)
FUNCTION_SUBDOMINANT  → 0.45 (forward motion)
FUNCTION_DOMINANT     → 0.8 (wants resolution)
FUNCTION_BORROWED     → 0.65 (color, moderate tension)
FUNCTION_CHROMATIC    → 0.7 (high tension)
```

The global `energy` parameter (0.0-1.0) selects target tension:
- **0.0** (calm): Prefers tonic and subdominant chords
- **0.5** (balanced): Mix of all functions equally
- **1.0** (energetic): Prefers dominant and chromatic chords

### 4. Voice Leading Smoothness

Computes semitone distance from current chord root to next chord root:

```
Current: C major → Candidate: A minor
Distance: 9 semitones (or 3 semitones reverse)
Uses minimum of both directions
```

**VoiceLeadingCompactness** interpretation:
- **0.0** (spread): Prefers distant chords (larger leaps)
- **0.5** (neutral): All distances equally viable
- **1.0** (compact): Prefers close chords (stepwise motion)

Special bonus: Stepwise motion (1-2 semitone intervals or perfect 4th) gets +0.15 boost

### 5. Spread Preference

Currently a placeholder for future voicing algorithms. Returns 0.6 base score with minor adjustments based on chord type and settings.

## API Usage

### Basic Ranking

```cpp
// Get the script manager instance
ScriptManager& manager = getScriptManager();

// Request top chord suggestions for a sequencer slot
RankedChord results[24];
uint8_t numCandidates = manager.rankChordsForSequencer(0, results, 24);

// results[0] = best suggestion
// results[1] = second best
// etc.

// Access score details
Serial.printf("Best chord: C%s, Score: %.2f\n",
    results[0].type == CHORD_MAJOR ? "maj" : "min",
    results[0].totalScore);
```

### Access Individual Scores

```cpp
// Get component scores for display or debugging
float theoryScore = engine.getTheoryScore(0, CHORD_MAJOR);  // C major
float energyScore = engine.getEnergyScore(0, CHORD_MAJOR);
float voiceLeadingScore = engine.getVoiceLeadingScore(0, CHORD_MAJOR);
float spreadScore = engine.getSpreadScore(0, CHORD_MAJOR);
```

### Manual Context Setup

```cpp
ChordRankingEngine engine;
GlobalParameters globals = {
    .key = MKEY_C,
    .degree = DEGREE_MINOR,
    .theoryMode = THEORY_FUNCTIONAL,
    .voiceLeadingCompactness = 0.7f,
    .energy = 0.5f
};

engine.setContext(globals, 0, CHORD_MAJOR);  // Context: C major
engine.setLocalOverrides(THEORY_DIATONIC, 0.5f, 0);  // Override theory mode

RankedChord results[24];
uint8_t count = engine.rankChords(results, 24);
```

## Data Structures

### RankedChord
```cpp
struct RankedChord {
    uint8_t rootNote;           // 0-11 (C=0, C#=1, etc.)
    ChordType type;             // CHORD_MAJOR or CHORD_MINOR
    float totalScore;           // Composite score 0.0-1.0
    float theoryScore;          // 40% weight
    float energyScore;          // 25% weight
    float voiceLeadingScore;    // 20% weight
    float spreadScore;          // 15% weight
};
```

## Implementation Details

### Scoring Formula

```
totalScore = (theoryScore × 0.40) +
             (energyScore × 0.25) +
             (voiceLeadingScore × 0.20) +
             (spreadScore × 0.15)
```

### Diatonic Scale Patterns

All 7 modes use the universal scale intervals (semitones from root):

```
Major/Ionian:     [0, 2, 4, 5, 7, 9, 11]
Minor/Aeolian:    [0, 2, 3, 5, 7, 8, 10]
Dorian:           [0, 2, 3, 5, 7, 9, 10]
Phrygian:         [0, 1, 3, 5, 6, 8, 10]
Lydian:           [0, 2, 4, 6, 7, 9, 11]
Mixolydian:       [0, 2, 4, 5, 7, 9, 10]
Locrian:          [0, 1, 3, 5, 6, 8, 10]
```

### Chord Function Classification

For C major scale (C D E F G A B):

- C major/minor → I (Tonic, 0.2 tension)
- D major/minor → II (Subdominant, 0.45 tension)
- E major/minor → III (Mediant/Tonic, 0.2 tension)
- F major/minor → IV (Subdominant, 0.45 tension)
- G major/minor → V (Dominant, 0.8 tension)
- A major/minor → VI (Submediant/Tonic, 0.2 tension)
- B major/minor → VII (Leading/Dominant, 0.8 tension)

## Future Enhancements

1. **Voicing Algorithms**: Enhance `computeSpreadScore()` to analyze actual note voicings
2. **Voice Leading History**: Track last N chords for smoother progressions
3. **Harmonic Rhythm**: Weight chords differently based on current beat position
4. **Secondary Dominants**: Recognize V/x harmony patterns
5. **Pedal Points**: Support sustained bass notes across chord changes
6. **Custom Scoring Weights**: Allow user to adjust 40/25/20/15 weights
7. **Learned Preferences**: Track which chord progressions user prefers

## Testing

To verify the ranking engine is working:

1. Set global parameters (key, degree, theory mode, energy, voice leading)
2. Call `rankChordsForSequencer()` with a test slot
3. Verify results are sorted by totalScore (highest first)
4. Spot-check individual scores make musical sense
5. Test all 7 scale degrees and 5 theory modes

Example test case:
```
Key: C, Degree: Major, TheoryMode: Functional, Energy: 0.5, VL: 0.7

Current chord: C major
Expected top suggestions: G major (V), F major (IV), A minor (VI)
Should NOT rank high: B minor (VII), C# major (outside key)
```

## Notes

- The ranking engine is **stateless** except for cached scale info
- It can be called multiple times per frame without performance issues
- All 24 possible chords are always evaluated and ranked
- Sorting is done with a simple O(n²) bubble sort (fine for 24 items)
- All calculations use float arithmetic for smoothness
