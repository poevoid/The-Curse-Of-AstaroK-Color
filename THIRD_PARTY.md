# Source and notices

Original game/art: Press Play On Tape, **The Curse Of AstaroK**, BSD-3-Clause.
Source commit: `0403de7b8948b16512d55252110cc8e3ccf05d81`.
https://github.com/Press-Play-On-Tape/The-Curse-Of-AstaroK

The tooling preserves the original ArdBitmap 2.0.3 codec, © 2016 Ignacio Vina,
modified by Filmote and Vampirics, Apache-2.0. Version 1.1 stores native 1-bit
bitmaps in flash and does not decode the compressed format on the handheld.
Custom color sprites use a new RLE SD art format. The display code uses a clipped, packed-color framebuffer blitter.

Compiled runtime: CHGame board platform 0.3.0; CHGame library 0.1.0; CHGfx 1.3.2 and CHSd (bundled with platform 0.3.0).
Source commit: `e1f8d05940ddb7bbef89c13326da75c003f02904`.
https://github.com/bateske/CHGame

CHGame is MIT. CHGfx includes its MIT license and notices for incorporated font,
Adafruit GFX, and other components. Arduino core portions are LGPL-2.1-or-later;
USB CDC and WCH components retain notices in their source. The matching core,
variant, system, CHGame, CHGfx and CHSd source/build definitions are included in
**support/CHGame-0.3.0-build-source.zip**. This is a source reference; normal users
should install the official board package as described in README.md. The sketch
and the supplied runtime source permit rebuilding/relinking with modifications.

New port/editor/tooling code is MIT. All original notices are retained. License
texts are in **licenses/** and in the source archive. Arduboy2 and ArduboyTones
libraries are not linked: the sketch uses a local compatibility layer.
