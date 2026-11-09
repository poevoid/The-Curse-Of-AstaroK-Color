# 1.3

- Per-pixel multicolor sprite editor with editable RGB palette, BASE brush and dedicated ALPHA transparency; opaque black remains separate.
- Color-only source and SD art pack; protected original monochrome images/masks and independent mono tint palette.
- Color silhouettes carry their own transparency, without editing a separate mask. PNG import/export preserves transparency.
- START options use the inventory border and Font3x6 in a centered box over the paused scene. All four settings, save migration and damage-scaled GPIO1 rumble remain.
- SD color art can be changed without recompiling firmware. Copy ASTAROK.ART to the card root alongside the updated game.

# 1.2

- Added a START-button options screen with four rows: display mode, monochrome tint, player color, and GPIO1 rumble enable.
- Pauses game state and animation counters while the menu is open; closing inputs are consumed.
- Added true two-tone rendering, preserving black town silhouettes, and a runtime override for player body/casting colors.
- Added persistent settings with migration from the previous save format. Changing options or sound preserves the last manual game save.
- Added nonblocking hardware PWM rumble on GPIO1/PC0 (H1.8), using TIM2 channel 4 independently of the speaker's TIM1.
- Scales rumble strength and duration from actual HP lost after defense/clamping, including Astarok's drain. Blocked damage/healing are silent; rumble defaults OFF and stops on menu entry/disable/timeout.
- Added RumbleConfig.h, menu/monochrome/save-migration/rumble regressions, PWM register tests, and options previews.

# 1.1

- Fixed village NPCs, tombstones and other negative-coordinate objects disappearing. `TownItem` used an unsigned 16-bit position; promotion to CHGame's 32-bit `int` produced positions around 65,000 instead of a negative village coordinate. Storage/accessors now use signed 16-bit values. The save structure's size, offsets and stored bits are unchanged.
- Normalized the inverted town tombstone art to a transparent foreground sprite, so its assigned color applies to the gravestone instead of a rectangular background.
- Eliminated per-frame artwork decompression. The additional native bitmap data fits in flash and leaves 4,760 bytes before the standard save pages.
- Added one-time clipping before drawing and a packed renderer that writes two color pixels per byte. The hot renderer uses -O2; the rest retains the -Os/no-LTO build.
- Added a small sprite-color lookup cache and removed a redundant daytime background clear.
- Normal frames send 128×64 pixels (16,384 pixel-data bytes), rather than repainting the 128×128 display including unused black borders (32,768 bytes). Full-panel initialization and save-error border changes are handled separately.
- Retained the original 40 fps game clock, controls, sprite colors/editor, and save format.
- Added town-coordinate regressions, half-screen transfer/error-border checks, and refreshed previews.
