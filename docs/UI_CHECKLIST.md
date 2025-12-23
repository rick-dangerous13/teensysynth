# UI Rendering Checklist

Before implementing ANY new UI screen or element, verify:

## Planning Phase
- [ ] What state needs to be displayed?
- [ ] What can change during runtime?
- [ ] What user actions trigger updates?
- [ ] Can updates be batched or localized?

## Implementation Phase
- [ ] Added `last*` tracking variables for all displayed state
- [ ] Initialized `last*` to impossible values (e.g., 255, -1)
- [ ] Compare before drawing: `if (current != last)`
- [ ] Clear only changed regions with `fillRect()`
- [ ] Update `last*` after drawing
- [ ] No `fillScreen()` calls during normal operation
- [ ] No unnecessary redraws in loops

## Testing Phase
- [ ] Build and upload to hardware
- [ ] Interact with encoder - no visible flicker?
- [ ] Rapidly change values - smooth updates?
- [ ] All edit modes work correctly?
- [ ] CV output changes as expected?

## Code Review
- [ ] Every draw call inside a state comparison?
- [ ] State synced between UI and ScriptManager?
- [ ] No stale data fetches after UI modifications?
- [ ] Comments explain drawing coordinate system?

## Common Patterns

### Good ✅
```cpp
if (currentStep != lastCurrentStep) {
    fillRect(oldX, oldY, w, h, COLOR_BG);  // Clear old
    fillRect(newX, newY, w, h, COLOR_ACCENT);  // Draw new
    lastCurrentStep = currentStep;
}

// Separate trigger indication from control updates
bool isEditingThisStep = (i == editStep && editMode == 0);
if (!isEditingThisStep && i == currentStep) {
    // Trigger flash only on non-edited controls
    handleColor = RED;
}

// Single highlight - only currently editable control
if (i == editStep && editMode == THIS_MODE) {
    drawRect(x, y, w, h, COLOR_HIGHLIGHT);  // Double box
    drawRect(x+1, y+1, w-2, h-2, COLOR_HIGHLIGHT);
}
```

### Bad ❌
```cpp
fillRect(x, y, w, h, COLOR_BG);  // Clears every frame
drawIndicator(currentStep);      // Draws every frame
// No state tracking, causes flicker

// DON'T: Redraw on every step change
if (i == currentStep || i == lastCurrentStep) needsUpdate = true;

// DON'T: Multiple highlights at once
if (i == editStep) drawHighlight();  // Ignores edit mode
if (i == currentStep) drawHighlight();  // Conflicts with above
```
