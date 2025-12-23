/**
 * Chord Ranking Engine Implementation
 */

#include "chord_ranking_engine.h"
#include <cmath>

ChordRankingEngine::ChordRankingEngine() 
    : currentChordRoot(0), currentChordType(CHORD_MAJOR),
      effectiveTheoryMode(THEORY_FUNCTIONAL), effectiveSpread(0.5f), effectiveInversion(0),
      scaleRoot(0) {
    // Initialize scale
    for (int i = 0; i < 7; i++) {
        diatonicScale[i] = 0;
    }
}

void ChordRankingEngine::setContext(const GlobalParameters& globals,
                                     uint8_t currentChordRoot,
                                     ChordType currentChordType) {
    this->globals = globals;
    this->currentChordRoot = currentChordRoot;
    this->currentChordType = currentChordType;
    
    // Generate diatonic scale based on root and degree
    generateDiatonicScale(globals.root, globals.degree);
    scaleRoot = globals.root;
    
    // Reset to global overrides
    effectiveTheoryMode = globals.theoryMode;
    effectiveSpread = globals.voiceLeadingCompactness;
    effectiveInversion = 0;
}

void ChordRankingEngine::setLocalOverrides(uint8_t localTheoryMode, 
                                           float localSpread,
                                           uint8_t localInversion) {
    if (localTheoryMode != 255) {
        effectiveTheoryMode = localTheoryMode;
    }
    if (localSpread >= 0.0f && localSpread <= 1.0f) {
        effectiveSpread = localSpread;
    }
    if (localInversion != 255) {
        effectiveInversion = localInversion;
    }
}

void ChordRankingEngine::generateDiatonicScale(uint8_t root, ScaleDegree degree) {
    // Scale intervals (semitones) for each mode starting from root
    // Each mode is a rotation of the major scale pattern
    
    static const int8_t scalePatterns[7][7] = {
        {0, 2, 4, 5, 7, 9, 11},      // Major (Ionian)
        {0, 2, 3, 5, 7, 8, 10},      // Minor (Aeolian)
        {0, 2, 3, 5, 7, 9, 10},      // Dorian
        {0, 1, 3, 5, 6, 8, 10},      // Phrygian
        {0, 2, 4, 6, 7, 9, 11},      // Lydian
        {0, 2, 4, 5, 7, 9, 10},      // Mixolydian
        {0, 1, 3, 5, 6, 8, 10}       // Locrian
    };
    
    // Copy the appropriate scale pattern
    if (degree < 7) {
        for (int i = 0; i < 7; i++) {
            diatonicScale[i] = scalePatterns[degree][i];
        }
    }
}

bool ChordRankingEngine::isScaleTone(uint8_t note, uint8_t root, ScaleDegree degree) const {
    // Normalize note to 0-11
    note = note % 12;
    root = root % 12;
    
    // Check if note is in the diatonic scale
    int8_t interval = (note - root + 12) % 12;
    
    for (int i = 0; i < 7; i++) {
        if (diatonicScale[i] == interval) {
            return true;
        }
    }
    return false;
}

bool ChordRankingEngine::isDiatonicChord(uint8_t rootNote, ChordType type) const {
    // A chord is diatonic if:
    // 1. Its root is in the scale AND
    // 2. Its third (minor or major) matches the scale degree
    
    // First check: root must be a scale tone
    if (!isScaleTone(rootNote, scaleRoot, globals.degree)) {
        return false;
    }
    
    // Second check: chord quality (major/minor) must match the diatonic expectation
    // In major scale: I(maj), ii(min), iii(min), IV(maj), V(maj), vi(min), vii°(dim)
    // In minor scale: i(min), ii°(dim), III(maj), iv(min), v(min), VI(maj), VII(maj)
    
    rootNote = rootNote % 12;
    uint8_t root = scaleRoot % 12;
    int8_t interval = (rootNote - root + 12) % 12;
    
    // For major scale (degree 0 = Ionian)
    if (globals.degree == 0) {  // Major scale
        switch (interval) {
            case 0: return (type == CHORD_MAJOR);  // I = Major
            case 2: return (type == CHORD_MINOR);  // ii = minor
            case 4: return (type == CHORD_MINOR);  // iii = minor
            case 5: return (type == CHORD_MAJOR);  // IV = Major
            case 7: return (type == CHORD_MAJOR);  // V = Major
            case 9: return (type == CHORD_MINOR);  // vi = minor
            case 11: return false;  // vii = diminished (not supported)
            default: return false;
        }
    }
    
    // For minor scale (degree 1 = Aeolian/Natural Minor)
    if (globals.degree == 1) {  // Natural minor scale
        switch (interval) {
            case 0: return (type == CHORD_MINOR);  // i = minor
            case 2: return false;  // ii° = diminished (not supported)
            case 3: return (type == CHORD_MAJOR);  // III = Major
            case 5: return (type == CHORD_MINOR);  // iv = minor
            case 7: return (type == CHORD_MINOR);  // v = minor
            case 8: return (type == CHORD_MAJOR);  // VI = Major
            case 10: return (type == CHORD_MAJOR);  // VII = Major
            default: return false;
        }
    }
    
    // For other modes, just check if root is in scale (less strict)
    return true;
}

ChordRankingEngine::ChordFunction ChordRankingEngine::getChordFunction(uint8_t rootNote, 
                                                                        ChordType type) const {
    rootNote = rootNote % 12;
    uint8_t root = scaleRoot % 12;
    
    // Distance from root of current key
    int8_t interval = (rootNote - root + 12) % 12;
    
    // Functional harmony classification based on scale degree
    // For major scale:
    // I (0) = Tonic, II (2) = Subdominant, III (4) = Mediant/Tonic, 
    // IV (5) = Subdominant, V (7) = Dominant, VI (9) = Submediant/Tonic, VII (11) = Dominant
    
    // Simplified: check root note interval in the scale
    bool isDiatonic = isDiatonicChord(rootNote, type);
    
    if (!isDiatonic) {
        return FUNCTION_CHROMATIC;
    }
    
    // Classify diatonic chords by their scale degree
    switch (interval) {
        case 0: return FUNCTION_TONIC;        // I
        case 2: return FUNCTION_SUBDOMINANT;  // II
        case 4: return FUNCTION_TONIC;        // III
        case 5: return FUNCTION_SUBDOMINANT;  // IV
        case 7: return FUNCTION_DOMINANT;     // V
        case 9: return FUNCTION_TONIC;        // VI
        case 11: return FUNCTION_DOMINANT;    // VII
        default: return FUNCTION_CHROMATIC;
    }
}

uint8_t ChordRankingEngine::semitoneDistance(uint8_t from, uint8_t to) const {
    // Normalized distance in semitones (0-12)
    from = from % 12;
    to = to % 12;
    int8_t dist = (to - from + 12) % 12;
    return (dist <= 6) ? dist : (12 - dist);  // Return shortest distance
}

float ChordRankingEngine::computeTheoryMatchScore(uint8_t rootNote, ChordType type) const {
    float score = 0.0f;
    
    // Base score: diatonic vs chromatic
    if (isDiatonicChord(rootNote, type)) {
        score = 0.9f;  // High score for diatonic chords
    } else {
        score = 0.3f;  // Lower score for chromatic chords
    }
    
    // Adjust based on theory mode
    switch (effectiveTheoryMode) {
        case THEORY_FUNCTIONAL: {
            // Prefer I, IV, V progressions
            ChordFunction func = getChordFunction(rootNote, type);
            switch (func) {
                case FUNCTION_TONIC: score += 0.2f; break;
                case FUNCTION_DOMINANT: score += 0.15f; break;
                case FUNCTION_SUBDOMINANT: score += 0.15f; break;
                case FUNCTION_BORROWED: score += 0.05f; break;
                case FUNCTION_CHROMATIC: score -= 0.3f; break;
            }
            break;
        }
        
        case THEORY_DIATONIC: {
            // Strict: only diatonic chords, slight preference for primary functions
            if (!isDiatonicChord(rootNote, type)) score -= 0.5f;
            else {
                ChordFunction func = getChordFunction(rootNote, type);
                if (func != FUNCTION_CHROMATIC) score += 0.1f;
            }
            break;
        }
        
        case THEORY_MODAL: {
            // Encourage modal interchange and diatonic chords
            // Less restriction than diatonic but prefers scale tones
            if (isDiatonicChord(rootNote, type)) score += 0.15f;
            break;
        }
        
        case THEORY_CHROMATIC: {
            // All chords welcome, slight variation
            score = 0.6f + (fabs((int8_t)(rootNote - currentChordRoot)) / 12.0f) * 0.3f;
            break;
        }
        
        case THEORY_ALL: {
            // All chords equally viable
            score = 0.7f;
            break;
        }
    }
    
    // Clamp to 0.0-1.0
    return constrain(score, 0.0f, 1.0f);
}

float ChordRankingEngine::computeEnergyScore(uint8_t rootNote, ChordType type) const {
    float score = 0.5f;  // Default neutral
    
    // Energy levels based on chord function and stability
    ChordFunction func = getChordFunction(rootNote, type);
    
    // Global energy ranges from 0.0 (very calm, stable) to 1.0 (very tense/energetic)
    float targetTension = globals.energy;
    
    // Assign tension levels to chord functions
    float chordTension = 0.0f;
    switch (func) {
        case FUNCTION_TONIC:        chordTension = 0.2f;  // Very stable
            break;
        case FUNCTION_SUBDOMINANT: chordTension = 0.45f;  // Slightly forward
            break;
        case FUNCTION_DOMINANT:    chordTension = 0.8f;  // Tense, wants resolution
            break;
        case FUNCTION_BORROWED:    chordTension = 0.65f;  // Color, moderate tension
            break;
        case FUNCTION_CHROMATIC:   chordTension = 0.7f;  // High tension
            break;
    }
    
    // If no current chord (first selection), favor tonic chords regardless of energy
    // This prevents non-tonic chords from ranking higher just because the energy level doesn't match
    if (currentChordRoot == 255) {
        // For first chord: tonic gets bonus, everything else neutral
        score = (func == FUNCTION_TONIC) ? 0.9f : 0.5f;
    } else {
        // For subsequent chords: score based on how well chord tension matches target energy
        score = 1.0f - fabs(chordTension - targetTension);
    }
    
    return constrain(score, 0.0f, 1.0f);
}

float ChordRankingEngine::computeVoiceLeadingScore(uint8_t rootNote, ChordType type) const {
    // If current chord root is invalid (255), return neutral score
    // This happens when there are no chords yet
    if (currentChordRoot == 255) {
        return 0.5f;  // Neutral: doesn't favor any chord by voice-leading
    }
    
    float score = 0.5f;
    
    // Compute semitone distance from current chord root to candidate
    uint8_t distance = semitoneDistance(currentChordRoot, rootNote);
    
    // VoiceLeadingCompactness ranges 0.0-1.0
    // 0.0 = prefer distant chords (more movement)
    // 1.0 = prefer close chords (minimal movement)
    
    float normalizedDistance = distance / 6.0f;  // Max distance 6 semitones
    
    if (globals.voiceLeadingCompactness > 0.5f) {
        // Prefer close movement
        score = 1.0f - normalizedDistance;
    } else {
        // Prefer distant movement
        score = normalizedDistance;
    }
    
    // Small bonus for stepwise motion (distance = 1, 2, or 5 semitones up)
    if (distance == 2 || distance == 5) {
        score += 0.15f;
    }
    
    return constrain(score, 0.0f, 1.0f);
}

float ChordRankingEngine::computeSpreadScore(uint8_t rootNote, ChordType type) const {
    // Spread preference (0.0-1.0) affects how widely the chord voicing is spread
    // This is more of a future voicing preference, for now return moderate score
    
    float score = 0.6f;
    
    // Could incorporate chord voicing algorithms here
    // For now, minor chords get slight boost if spread is high (spread uses more notes)
    if (type == CHORD_MINOR && globals.voiceLeadingCompactness < 0.4f) {
        score += 0.1f;
    }
    
    return constrain(score, 0.0f, 1.0f);
}

uint8_t ChordRankingEngine::rankChords(RankedChord* results, uint8_t maxResults) {
    if (!results || maxResults == 0) return 0;
    
    uint8_t resultIdx = 0;
    
    // Iterate through all 24 possible chords (12 roots × 2 types)
    for (uint8_t root = 0; root < 12; root++) {
        for (uint8_t typeIdx = 0; typeIdx < 2; typeIdx++) {
            ChordType type = (typeIdx == 0) ? CHORD_MAJOR : CHORD_MINOR;
            
            // Skip if current chord (only if current is valid, not sentinel 255)
            if (currentChordRoot != 255 && root == currentChordRoot && type == currentChordType) {
                continue;  // Don't suggest the same chord we're already on
            }
            
            if (resultIdx >= maxResults) {
                break;
            }
            
            // Compute individual scores
            float theoryScore = computeTheoryMatchScore(root, type);
            float energyScore = computeEnergyScore(root, type);
            float voiceLeadingScore = computeVoiceLeadingScore(root, type);
            float spreadScore = computeSpreadScore(root, type);
            
            // Weighted combination (adjust weights to taste)
            float totalScore = 
                (theoryScore * 0.40f) +
                (energyScore * 0.25f) +
                (voiceLeadingScore * 0.20f) +
                (spreadScore * 0.15f);
            
            results[resultIdx].rootNote = root;
            results[resultIdx].type = type;
            results[resultIdx].totalScore = constrain(totalScore, 0.0f, 1.0f);
            results[resultIdx].theoryScore = theoryScore;
            results[resultIdx].energyScore = energyScore;
            results[resultIdx].voiceLeadingScore = voiceLeadingScore;
            results[resultIdx].spreadScore = spreadScore;
            
            resultIdx++;
        }
    }
    
    // Sort results by score (descending)
    for (uint8_t i = 0; i < resultIdx - 1; i++) {
        for (uint8_t j = i + 1; j < resultIdx; j++) {
            if (results[j].totalScore > results[i].totalScore) {
                // Swap
                RankedChord temp = results[i];
                results[i] = results[j];
                results[j] = temp;
            }
        }
    }
    
    return resultIdx;
}

float ChordRankingEngine::getTheoryScore(uint8_t rootNote, ChordType type) const {
    return computeTheoryMatchScore(rootNote, type);
}

float ChordRankingEngine::getEnergyScore(uint8_t rootNote, ChordType type) const {
    return computeEnergyScore(rootNote, type);
}

float ChordRankingEngine::getVoiceLeadingScore(uint8_t rootNote, ChordType type) const {
    return computeVoiceLeadingScore(rootNote, type);
}

float ChordRankingEngine::getSpreadScore(uint8_t rootNote, ChordType type) const {
    return computeSpreadScore(rootNote, type);
}
