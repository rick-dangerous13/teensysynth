# Polyphonion Design Principles

## Philosophy
Create artistic, flicker-free interfaces inspired by Norns aesthetics using only native Teensy display primitives.

## Visual Design
- **Metaphorical controls**: Use real-world analogies (levers, switches, dials)
- **Selective color**: Grayscale base with accent colors for meaning
- **Tactile feel**: Bevels, shadows, textures using simple primitives
- **Information density**: Show everything needed without clutter
- **Typography hierarchy**: Titles always FONT_SMALL in top-left (x+20, y+5), labels FONT_SMALL throughout

## Performance
- **60 FPS target**: Updates must be imperceptible
- **Selective redrawing**: Only changed pixels
- **State comparison**: Always track and compare
- **No blocking operations**: Keep UI responsive
- **Separate concerns**: Trigger indicators ≠ control redraws
- **Stable during edit**: No flicker on control being actively manipulated

## User Experience
- **Immediate feedback**: Visual response to every input
- **Clear affordances**: Obvious what can be edited
- **Consistent patterns**: Same interactions across screens
- **No dead states**: Every mode should do something meaningful
- **Single focus indicator**: Only one control highlighted at a time (double yellow box)
- **Visual hierarchy**: Edit highlight > trigger flash > current step > inactive
- **Stable manipulation**: Control being edited stays rock solid, no flicker

## Implementation
- **State-driven**: UI reflects internal state, not commands
- **Sync carefully**: UI ↔ ScriptManager coordination
- **Test continuously**: Build and verify frequently
- **Document visually**: ASCII art in comments for complex layouts
