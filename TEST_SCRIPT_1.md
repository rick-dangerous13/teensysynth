# TEST SCRIPT #1 — SIMPLE CIRCLE-OF-FIFTHS SEED

## Purpose
Test global parameter setting, ranking from C major, selecting F minor as chord #2, ranking after modal mixture, and adding Bb major as chord #3.

## Test Execution Log

### Step 1: RESET SEQUENCER
```
Command: TEST RESET
```

**Expected outcome:**
- chordCount = 0
- currentChordSlot = 0
- beatCounter = 0

**Verification:** TEST STATE

**Expected output:**
```
STATE chords=0
```

---

### Step 2: SET GLOBAL PARAMETERS
```
Command: TEST GLOBAL root=0 degree=0 theory=0 vl=0.5 energy=0.5
```

**Meaning:**
- root = 0 (C)
- degree = 0 (Ionian/Major)
- theory mode = 0 (Functional)
- voice leading compactness = 0.5
- energy = 0.5

**Verification:** (No direct verification; ranking check coming next)

---

### Step 3: CHECK RANKING BASELINE
```
Command: TEST RANK
```

**Expected outcome:**
24 chords listed. Highest scores should cluster around:
- C major (I) — root chord, highest
- G major (V) — perfect fifth above, very strong
- F major (IV) — perfect fourth below, strong
- A minor (vi) — relative minor, diatonic
- E minor (iii) — diatonic
- D minor (ii) — diatonic

**NOT in top 3:** F minor (iv modal mixture, not diatonic to C major)

---

### Step 4: ADD FIRST CHORD (C MAJOR)
```
Command: TEST CHORD slot=0 root=0 type=0 beats=16
```

**Verification:** TEST STATE

**Expected output:**
```
STATE chords=1
  0: C maj beats=16
```

---

### Step 5: REQUEST RANKING FOR NEXT CHORD
```
Command: TEST RANK
```

**After C major is set as current chord, ranking engine should score candidates based on:**
- Voice leading distance from C major
- Functional harmonic role in C major
- Diatonic fit
- Energy match

**Expected top suggestions (not strict order, but should appear in top 12):**
1. G major (V) — strong functional pull, perfect fifth distance
2. F major (IV) — strong functional, fourth distance
3. A minor (vi) — diatonic, relative minor
4. E minor (iii) — diatonic
5. D minor (ii) — diatonic
6. C major (I) — same chord (likely low score for repetition unless energy drives it)

**F minor should be present but NOT in top 3** — it's not diatonic to C major in functional theory.

---

### Step 6: SELECT F MINOR AS CHORD #2
```
Command: TEST CHORD slot=1 root=5 type=1 beats=8
```

Note: root=5 is F (C=0, C#=1, D=2, D#=3, E=4, F=5)
type=1 is minor

**Verification:** TEST STATE

**Expected output:**
```
STATE chords=2
  0: C maj beats=16
  1: F min beats=8
```

---

### Step 7: REQUEST RANKING FOR NEXT CHORD (AFTER F MINOR)
```
Command: TEST RANK
```

**Key insight:** 
- Current chord is now F minor (slot 1)
- Ranking engine scores candidates relative to F minor
- **Modal mixture effect:** F minor (iv in C) introduces chromatic context
- Voice leading from F minor to potential next chords changes scoring

**Expected outcome — top suggestions should now include:**
1. **Bb major** (bVII in C, but naturally in F Dorian/Aeolian context) — strong pull
2. **G major** (V of C, but also V of F minor context) — remains functional
3. **F major** (parallel major to F minor) — natural modal exchange
4. **Ab major** (bVI in C, part of modal mixture family) — chromatic neighbor
5. **C major** (back to tonic)
6. **A minor** (still diatonic to C)

**Key difference from Step 5:**
- Modal mixture chords (Bb, Ab, Db, etc.) should rank **higher** than before because F minor is a modal mixture chord itself
- Functional pull to G major remains but may be slightly reduced if voice-leading or energy changes the balance

---

### Step 8: ADD THIRD CHORD (Bb MAJOR)
```
Command: TEST CHORD slot=2 root=10 type=0 beats=8
```

Note: root=10 is Bb (C=0, ..., Bb=10)
type=0 is major

**Verification:** TEST STATE

**Expected final output:**
```
STATE chords=3
  0: C maj beats=16
  1: F min beats=8
  2: Bb maj beats=8
```

---

## Summary of Assertions

| Step | Command | Expected Result | Pass/Fail |
|------|---------|-----------------|-----------|
| 1 | TEST RESET | chordCount=0 | \_\_\_\_ |
| 2 | TEST GLOBAL ... | (settings accepted) | \_\_\_\_ |
| 3 | TEST RANK (baseline) | G, F, Am, Em, Dm in top 12; Fm not in top 3 | \_\_\_\_ |
| 4 | TEST CHORD slot=0 C maj | STATE shows C maj, beats=16 | \_\_\_\_ |
| 5 | TEST RANK (after C) | Top includes G maj, F maj, Am, Em, Dm | \_\_\_\_ |
| 6 | TEST CHORD slot=1 F min | STATE shows 2 chords; Fm beats=8 | \_\_\_\_ |
| 7 | TEST RANK (after Fm) | Bb maj appears higher; modal mixture visible | \_\_\_\_ |
| 8 | TEST CHORD slot=2 Bb maj | STATE shows 3 chords; Bb maj beats=8 | \_\_\_\_ |

---

## How to Execute

1. Open serial monitor at 115200 baud.
2. Copy each TEST command below into the serial input one at a time, pressing Enter after each.
3. Record the output.
4. Compare against expected outcomes.

### Commands to Send (in order):
```
TEST RESET
TEST STATE
TEST GLOBAL root=0 degree=0 theory=0 vl=0.5 energy=0.5
TEST RANK
TEST CHORD slot=0 root=0 type=0 beats=16
TEST STATE
TEST RANK
TEST CHORD slot=1 root=5 type=1 beats=8
TEST STATE
TEST RANK
TEST CHORD slot=2 root=10 type=0 beats=8
TEST STATE
```

---

## Notes

- **Voice Leading:** The ranking engine considers distance between chord roots and inversions. F minor (F=5) to Bb major (Bb=10) is a 5-semitone movement, which is a fourth—relatively smooth.
- **Theory Mode 0 (Functional):** Prioritizes traditional harmonic function (I, IV, V, vi, ii, iii). Modal mixture (Bb in C major) is chromatic but adds color.
- **Energy 0.5:** Neutral energy preference; ranking balances function with voice leading.
- **Voice Leading 0.5:** Neutral compactness preference; balances smooth voice leading with harmonic variety.

