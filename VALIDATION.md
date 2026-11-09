# Validation — CHGame port 1.3

## Target build

Compiled using Arduino CLI 1.5.2-rc.1, CHGame platform 0.3.0, WCH RISC-V GCC 8.2.0, CHGame library 0.1.0 and CHGfx 1.3.2 and CHSd 1.0.0. Exact upstream source revisions are recorded in THIRD_PARTY.md.

FQBN: `CHGame:ch32v:rev0:opt=osstd,rtlib=nano,periph=game,usb=uploadonly`

The main build is -Os without LTO. Only the hot packed bitmap renderer is compiled locally at -O2. The build used local tool paths for the toolchain/ctags and disabled the automatic packaging hook. Official CHG/cartridge tools packaged and verified the final binary independently.

| Measurement | Bytes |
|---|---:|
| Raw application image | 49,404 |
| Arduino-reported program size | 49,140 |
| Application region capacity | 50,944 |
| Space before both reserved save pages | 1,028 |
| .data | 1,624 |
| .bss | 15,460 |
| Static data total | 17,084 |
| Separately reserved stack | 2,048 |
| Remaining data/heap gap below stack | 1,332 |

The raw image includes startup/vector/init-array sections omitted by the Arduino size recipe. Flash save pages 0xF500/0xF600 remain outside the program image. The full save record fits CHGame's 244-byte data limit. The new record is 226 bytes with format version 2. Older version-1 game/sound records are read and preserved, then converted on the next manual save or changed-options save. Video settings and rumble enable do not overwrite unsaved game progress into the manual save.

## Rendering work

Original monochrome data resides in flash as native vertical 1-bit bitmaps and remains byte-identical to version 1.2. COLOR first checks ASTAROK.ART for a per-frame override; unchanged frames use the original bitmap and header tint. Custom color frames use row-major RLE runs with 16 opaque slots, inherited BASE and ALPHA. Runs draw directly into the packed framebuffer, without a decompressed image buffer. A seven-sector (3,584-byte) LRU cache bounds RAM usage. The supplied color pack is 1,427 bytes with nine edited sheets. Editing more art grows the SD file rather than firmware flash or static RAM. Card access waits for LCD DMA before using the shared SPI bus.

The packed renderer clips once and writes two color pixels per byte. It skips entirely off-screen sprites, supports mirrors/masks, and caches sprite-color lookups. Daytime clearing writes the next background once.

After a one-time full panel initialization, normal frames transfer only the active 128×64 view. Pixel data drops from 32,768 to **16,384 bytes per frame**. A separate small transfer updates the save-error border only when it changes. The panel retains its previous completed view during the next draw.

The simulator's 2.947 MB/s SPI model measures a normal transfer at **5,605 microseconds**, versus approximately 11.2 ms for the old full panel. This is a model result, not a measured hardware timing.

## CPU comparison

Host g++ -O2, same simulator/library source, best of three 1,000-frame runs after warmup. This isolates host rendering CPU work; the simulator's transfer advances virtual time without sleeping. It does **not** establish a CH32 hardware speedup or whole-game frame rate. Version 1.2 draws the previously missing objects during these runs.

| View | Port 1.0, microseconds/frame | Port 1.2, microseconds/frame |
|---|---:|---:|
| town 255 | 49.395 | 18.289 |
| town 120 | 45.469 | 16.271 |
| town -150 | 49.076 | 17.823 |
| dungeon 10 | 30.255 | 14.459 |

With the supplied 1.3 SD color pack enabled, a new run measured town 255 **17.508**, town 120 **15.935**, town -150 **17.096**, and dungeon 10 **14.107 microseconds per host CPU frame**. These are host-only checks under the same method and do not predict MCU speed.

Reproduce with:

```sh
python tests/benchmark.py /path/to/CHGame
python tests/benchmark.py /path/to/CHGame --sketch /path/to/port-1.0/CurseOfAstarok
```

## Automated checks

- **147 color sheets / 190 frames** are independently editable. A synthetic SD pack exercises every frame, all 16 RGB slots, BASE and ALPHA with **2,280 frame/clip/mirror pixel comparisons**. It also checks native and external-mask wrapper paths, player BASE behavior, frozen monochrome images and independent mono/color palettes. Missing and malformed packs fall back to built-in artwork. AddressSanitizer and UndefinedBehaviorSanitizer report no errors.
- **176 sprite sheets / 219 frames** pass readable text → generated source bitmap → decoded pixels round trips. The tooling's legacy compressed codec also passes empty/full/noise cases at odd sizes and 128×64.
- **1,470 sprite/frame/clip/mirror comparisons** check every viewport pixel against expected art, including ordinary and formerly compressed assets, masks, partial clipping and both mirror axes.
- Explicit world-object regression tests draw NPCs, tombstones, the save post and crates from **negative village coordinates**, and test a moved pickup. This exercises `TownItem::render`, which the old direct-sprite tests did not cover.
- The town tombstone is normalized from inverted art to a transparent foreground drawing.
- A display test checks the 64-row transfer duration and independent showing/clearing of the save-error border.
- AddressSanitizer and UndefinedBehaviorSanitizer pass the complete host scenario: town, inventory, all six item purchases, all four rune purchases, shop Exit rows, save/load, sound setting changes preserving the save, five-of-a-kind scoring, and damage/victory paths across all 15 dungeons, including Astarok.
- A 240-frame button-scripted splash/title/town/options/save/resume run presents at **39.8 fps in virtual time**, with **zero DMA/framebuffer errors**. This checks pacing, not hardware rendering capacity.
- All Python tools pass syntax compilation. A live Tk GUI test under Xvfb exercised painting, undo/redo, eyedropper, RGB editing, PNG import/export and save. Exported PNG tests distinguish transparent ALPHA from opaque black. Hashes prove editor saves leave all monochrome source images unchanged. The included editor preview was captured from that running GUI.
- Options tests use the actual main loop and START/D-pad/B inputs to check four rows, palette and player-color cycling/wrapping, paused game state/frame count, closing-input isolation, options persistence, preserved manual progress/sound, and migration of a valid old save.
- Monochrome rendering is checked to contain only black and the selected tint; NPC silhouettes and the save-error border retain correct contrast/tint. Player bodies change in COLOR mode while masks/helmets stay independent.
- Rumble tests cover disabled output, blocked attacks, armor reduction, weak/strong impacts, lethal-HP clamping, direct Astarok drain, healing, timeout and stopping/suppressing pulses while paused.
- A separate no-CHSIM test compiles the actual hardware rumble backend against fake registers. It checks PC0 only, TIM2 channel 4/20 kHz period and duty, output stopping, and preservation of neighboring GPIO configuration and the TIM1 speaker remap/registers. This is register-logic verification, not a physical waveform measurement.
- Official tools verify `ASTAROK.CHG` and `CurseOfAstarok.chgame`. The CHG payload matches `firmware/CurseOfAstarok.bin` byte-for-byte; the cartridge bundles the same ASTAROK.ART as sdcard/.

Run checks with Python and a host g++/official CHGame checkout:

```sh
python tests/test_art.py
python tests/test_color_art.py
python tests/run_color_tests.py /path/to/CHGame
python tests/run_game_tests.py /path/to/CHGame
python tests/test_rumble_hardware.py
# Optional, with a desktop display:
python tests/test_editor.py
```

`ASTAROK_SCREENSHOTS=/some/directory` optionally captures game scenario screens as PPM files. Previews show actual host-rendered town, shops and battles.

## Options and feedback implementation

START is intercepted before the game state machine. Open-menu frames return before game updates and restore the game animation counter; the underlying frame/input clock still runs so controls and the hold-START shortcut work. The four rows use Font3x6 and the inventory corner artwork in a centered 104×54 box. A deferred render rebuilds the frozen scene behind the box; game variables, animation clock and fade state are restored afterward, and the completed menu frame is transferred once. No second framebuffer snapshot is needed. Settings write only when a changed menu is closed. A versioned record stores mode, monochrome tint, optional player override and rumble enable alongside the last manually saved game and sound setting.

MONO collapses the display palette to black plus one tint and restores the original dark erase/bitmap polarity for town silhouettes. COLOR uses the SD color palette when available and the header palette otherwise. Player override changes BASE pixels on idle/walk/cast artwork while explicitly painted details keep their slots. No full image buffer is added. Editor RGB changes do not affect the independent monochrome tint palette.

`Player::inflictDMG()` applies defense then calls the same HP-loss helper used by direct health drain. That helper passes the actual HP lost (including the lethal clamp) to the rumble envelope. TIM2 channel 4 drives GPIO1/PC0 with hardware PWM; TIM1 remains the speaker timer. The default is OFF. Enabling it permits a 20 ms startup kick followed by scaled duty and a bounded 50 + 12×HP millisecond pulse. Updating/stopping uses unsigned elapsed time and adds no blocking delay.

## Retained portability fixes

The local compatibility layer uses the CHGame input mapping, clock, graphics, piezo audio and flash-save APIs. No AVR register access or AVR assembly remains. Original note sequences use the CHGame driver's 20 Hz pitch and 2 ms duration steps; long notes are split to avoid its 510 ms per-step limit.

The final boss's zero-range random reward returns its lower bound, the rune-shop Exit row avoids a non-existent fifth price record, and an upstream castle mask is normalized to the headerless external-mask format.

## Remaining device checks

No physical CHGame was available for this revision. Achieved hardware frame rate, LCD behavior, speaker output, physical rumble strength/waveform and motor-driver compatibility, USB/SD launch, and power-cycle save persistence require a device check. The simulator models drawing and DMA contracts, not the MCU instruction stream, physical SD throughput or electrical timing. Large, intricate color packs may need more SD reads than the supplied pack. Saves retain the standard shared CHGame flash pages rather than per-game SD files.
