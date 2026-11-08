# The Curse of Astarok — CHGame color port 1.3

This is **The Curse of Astarok**, the original Press Play On Tape town-and-dungeon game, **not Trials of Astarok**. The original game logic, 15 dungeon encounters, item/rune shops, conversations, inventory, music and sound-effect sequences are included.

Target: **CHGame Rev0 / CH32X035G8U6 / 128×128 ST7735**, official **CHGame 0.3.0** board package.

The original 128×64 view is centered at native pixel size, with black space above and below. It is not stretched. The game keeps its original 40 fps logic rate. A finished color frame is sent by CHGfx/DMA; the panel is not blanked while drawing. Version 1.1 fixes negative-coordinate village objects, stores artwork directly in flash without frame-time decompression, draws two display pixels per packed byte, and transfers only the active 128×64 view after the first frame.

## Play immediately

Copy **sdcard/GAMES/ASTAROK.CHG** to **GAMES/ASTAROK.CHG**, and **sdcard/ASTAROK.ART** to **ASTAROK.ART at the card root**. Launch **CURSE OF ASTAROK** from the SD game menu. The ART file contains your color sprites and color palette. Restart the game after replacing it. If it is missing or incompatible, the game falls back to its built-in artwork and header colors.

**CurseOfAstarok.chgame** is also supplied for the official cartridge tools/uploader. **firmware/** contains the same application as `.bin` and `.hex`. These are application images for the official **0x3000** application layout; keep your existing bootloader.

To update from 1.0–1.2, replace `GAMES/ASTAROK.CHG` and copy `ASTAROK.ART` to the card root and launch it again from the SD menu. Older saves are imported with default options; your saved progress and sound preference are retained. Your existing `SpriteColors.h` assignments can also be retained when rebuilding the new source.

## Controls

| Button | Action |
|---|---|
| D-pad Left / Right | Walk; navigate inventory panes |
| Up | Jump/select runes to recast; enter a shop at its door |
| Down / Up | Navigate shop goods and menu choices |
| A | Start; talk/read; confirm; cast/recast runes; advance battle messages |
| B | Open/close inventory; cancel; toggle sound on the title screen |
| START, short press | Open/close the options screen; the game pauses |
| Hold START, 3 seconds | Return to the SD game menu |
| Hold Down on title screen | Original game's bootloader shortcut |

In a fight, press A to cast. Move underneath a rune and press Up to mark/unmark it for recasting, then press A again. B opens your items during your turn. After winning, walk onward or return to town.

Save and restore at the **sword/save post** to the left of the initial starting point: stand nearby and press A. Saving uses CHGame's CRC-checked, versioned flash save record and retains the original manual-save behavior. Changing sound options preserves your last saved game. These standard CHGame save pages are shared with other games; another game can replace the record. A failed write shows **SAVE FAILED** and does not claim success.

## Options screen

Tap **START** from any game screen. The centered box uses the inventory’s border artwork and Font3x6, with the paused game visible around its edges. The four rows are:

| Row | Setting |
|---|---|
| DISPLAY | COLOR or MONO |
| MONO TINT | One of the 15 visible palette colors |
| PLAYER | DEFAULT (the header artwork colors), or one of the 15 visible colors |
| RUMBLE | ON or OFF; defaults OFF |

Use **Up/Down** to select a row and **Left/Right** to change it. **A** also advances the value. Press **B or START** to close. Settings save once when you close a changed menu; no writes occur while cycling values. Your last manually saved game is retained rather than replaced by current unsaved progress.

The game state and animation clock pause while options are open. Closing controls are consumed so they do not also cast a rune, open inventory or advance dialogue. Rumble stops when the menu opens. Holding START for three seconds still returns to the SD game menu.

MONO uses black plus the chosen tint, including the original dark town silhouettes. Player color changes the **BASE** pixels in COLOR mode on the player’s casting/walking/idle animations. Explicitly painted colors (eyes, boots, etc.) keep their assigned slots; helmets retain their own sprite colors. In MONO, the player follows the overall tint so the image stays monochrome. DEFAULT restores the individual `SpriteColors.h` assignments.

## Rumble pack — GPIO1

The output is the board's **GPIO1 label: PC0, expansion header H1 pin 8**, not Arduino digital pin 1. It is an **active-high 3.3 V control signal** for a suitable motor/rumble driver. Share ground with the driver and power the motor from a supply matched to it. Do not connect a motor directly to the GPIO; a bare ERM motor needs a transistor/MOSFET driver and flyback protection.

Enable **RUMBLE: ON** in options after connecting the driver. Hardware **TIM2 channel 4** supplies **20 kHz PWM**, independent of the TIM1 speaker output and the LCD. No delay is added to battle logic.

Every actual loss of player HP triggers an impact. Armor reduction is applied first, and a lethal attack is capped to the HP the player actually had. Fully blocked attacks and healing create no pulse. Astarok's direct HP drain uses the same path. Sound can be muted while rumble remains enabled.

The default envelope has a 20 ms spin-up kick, then damage-scaled duty (about 31–100% with full strength at 40 HP lost). Duration is **50 ms + 12 ms per HP lost**, capped at **650 ms**. More damage therefore produces stronger, longer feedback. Turning rumble off, opening options, or finishing the pulse stops the output. These physical strength values need calibration for your motor/driver.

Edit **CurseOfAstarok/RumbleConfig.h** to adjust PWM frequency, minimum duty, full-strength damage threshold, kick and duration limits. The pin mapping is fixed to GPIO1/PC0.

## Edit multicolor sprites and the palette

Run **EDIT_SPRITES.bat**, or `python tools/sprite_editor.py`. The editor uses Python 3’s built-in Tkinter; the normal Windows Python installer needs no pip packages or virtual environment.

1. Search/select a color sprite, then choose its animation frame.
2. Choose a palette brush and left-drag to paint. A sprite can use multiple colors.
3. Choose **ALPHA**, or right-drag, to make pixels transparent. The checkerboard shows transparency; **BLACK** paints opaque black.
4. Double-click any RGB swatch except black to edit that color. The palette is shared across color-mode sprites.
5. Click **Save + export SD art**, copy **sdcard/ASTAROK.ART** to the SD card root, and restart. Art/palette edits need no firmware recompile.

There are **147 color sprite sheets / 190 animation frames**. The editor exposes color artwork only. It never changes the original monochrome artwork or its masks, so switching to MONO always uses the protected original images. Custom color silhouettes have their own transparency; you do not edit separate masks.

| Brush | Meaning |
|---|---|
| BLACK through WHITE | 16 opaque RGB palette slots; black stays slot 0 |
| BASE | Inherit this sprite/frame’s header assignment; player BASE pixels follow the PLAYER option |
| ALPHA | Fully transparent pixel; leave the scene underneath visible |

ALPHA is a dedicated brush/encoded value, separate from all 16 RGB slots. It is binary transparency (opaque or transparent), with no partial-alpha blending. **Import PNG** reads transparent pixels as ALPHA and maps opaque pixels to the closest palette color. **Export PNG** preserves transparency and opaque black. Frames must keep their original dimensions. PNGs resolve BASE to its current header color; use the text source to retain BASE assignments during external editing.

**Undo/Redo** operates on pixel edits. **Reset frame** restores that color frame’s original template. The included color art gives the player gold facial details and brown boots, and the pumpkin a green stem, while leaving the rest ready for your edits.

For direct text editing, use **art/ColorSprites.txt**:

```text
@SpriteName
..GG..
.G22G.
G2002G
---
..GG..
.G22G.
G2002G
```

`.` means ALPHA, `0..9/A..F` select the 16 RGB slots, `G` is BASE, and `---` separates frames. RGB values live in **art/ColorPalette.json**. Keep sprite names, dimensions and frame counts. Run **REBUILD_ART.bat** or `python tools/color_art.py` to export the SD file after direct edits. **ColorBase.txt** is the reset/difference template; **MonochromeSprites.txt** is a read-only reference, and **color_manifest.json** records color asset IDs and dimensions.

## Header color assignments

**CurseOfAstarok/SpriteColors.h** still defines the named colors and per-family/per-sprite BASE assignments:

```cpp
#define SPRITE_PLAYER COLOR_CYAN
#define SPRITE_SKELETON COLOR_SILVER
#define ART_PUMPKIN COLOR_ORANGE
```

Rune symbols and potion types retain their named frame assignments. Header assignment changes require recompiling/uploading; the editor’s RGB palette and explicit pixel slots are loaded from the SD file. **BASE** pixels follow header/player settings. Slot 0 must remain black. `ASTAROK_PALETTE` is the built-in fallback palette and the independent monochrome tint palette; editor RGB changes affect COLOR mode only.

## Compile/upload

Board Manager URL:

https://github.com/bateske/CHGame/releases/latest/download/package_chgame_index.json

Open **CurseOfAstarok/CurseOfAstarok.ino**, keeping its complete folder intact. Use the libraries included in the official CHGame board package; Arduboy libraries are not required.

| Arduino setting | Value |
|---|---|
| Board | CHGame Rev0 |
| Optimize | Smallest (-Os), without LTO |
| C Runtime | Newlib Nano |
| Peripherals | Game |
| USB | Upload only |

CLI equivalent with an installed board package:

```sh
arduino-cli compile -b CHGame:ch32v:rev0:opt=osstd,rtlib=nano,periph=game,usb=uploadonly --export-binaries CurseOfAstarok
```

Select your CHGame USB port and use Upload. The current build leaves about 1 KB before the standard save pages; check build size if adding firmware code. Color art is RLE encoded on the SD card with a fixed sector cache, so editing more sprites does not increase firmware flash or static RAM usage. Extremely detailed art can cause more SD reads; use a reasonably fast card. The hot bitmap renderer uses -O2 locally; the rest of the program still builds with -Os, without LTO.

## Validation and source

See **VALIDATION.md** for compilation, memory use and automated checks. No physical CHGame was available: LCD orientation, speaker volume, physical rumble response, USB/SD launch and power-cycle save behavior still need a device check. The included previews are actual host-rendered game screens.

Original source: https://github.com/Press-Play-On-Tape/The-Curse-Of-AstaroK

Original game and artwork © 2019 Press Play On Tape, BSD-3-Clause. The tooling retains an ArdBitmap codec derived from Ignacio Vina's Apache-2.0 code, modified by Filmote and Vampirics; the handheld reads original native bitmaps from flash and custom RLE color sprites from SD. CHGame and CHGfx notices accompany the linked firmware in **licenses/**. New port/editor code is MIT; see **LICENSE**. This is an unofficial port.
