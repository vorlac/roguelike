<img align="left" src="logo/rtexpacker_256x256.png" width=256>

# rTexPacker

A simple and easy-to-use textures packer and font atlas generator. 

Package sprites or fonts into an atlas to improve drawing performance.

`rTexPacker` can be used for free as a [WebAssembly online tool](https://raylibtech.itch.io/rtexpacker) and it can also be downloaded as a **standalone tool** for _Windows_ and _Linux_ with some extra features.

<br>

## rTexPacker Features

 - Package **sprites and font glyphs** into an atlas
 - Configure **packing algorithms and heuristics**
 - Setup sprites **spacing, padding and alpha-trimming**
 - **Font generation options:** Size, SDF fonts, fixed font height
 - Import **custom unicode charset** from UTF-8 file
 - **Unicode** charset duplicates removed automatically
 - **Edit sprites origin** visually, exported with the atlas
 - Atlas **visualization options**: Zoom, Pan, Background, Fill
 - Multiple UI styles available, selectable from main toolbar
 - Load/Save **portable self-contained .rtp file**, containing all sprites
 - Load sprites from **multiple image formats**: `.png`, `.qoi`, `.tga`, `.jpg`
 - Load sprites from **font files**: `.ttf`, `.otf`
 - **Export atlas descriptor** as: text (`.rtpa`), binary (`.rtpb`), `.json`, `.xml` and code (`.h`)
 - **Export atlas image** as: `.png`, `.qoi`, `.dds` and `.raw`
 - Export atlas descriptor as **binary PNG chunk**: `rTPb`
 - **Multiple usage examples** provided to load: `.rtpa`, `.rtpb`, PNG chunk `rTPbp` and code `.h`.

### rTexPacker Standalone Additional Features

 - Maximum atlas size up to **8192x8192 pixels**
 - Command-line support for **batch sprites packing**
 - **Completely portable (single-file, no-dependencies)**
 
## rTexPacker Screenshot

![rTexPacker](screenshots/rtexpacker_v200_shot01.png)
 
## rTexPacker Usage

Open the tool, drag & drop your sprites/fonts and setup atlas packing options.

Generated atlas can be exported as atlas-descriptor text file plus an atlas image file. The formats supported are:

 - Atlas Descriptor: text (`.rtpa`), binary (`.rtpb`), `.xml`, `.json` and code (`.h`)
 - Atlas Image: `.png`, `.qoi`, `.dds` and `.raw`

`rTexPacker Standalone` comes with command-line support for batch sprite packaging and font atlas generation.

 > rtexpacker.exe --help

## rTexpacker Technologies

This tool has been created using the following open-source technologies: 

 - [raylib](https://github.com/raysan5/raylib) - A simple and easy-to-use library to enjoy videogames programming
 - [raygui](https://github.com/raysan5/raygui) - A simple and easy-to-use immediate-mode-gui library
 - [tinyfiledialogs](https://sourceforge.net/projects/tinyfiledialogs/) - File dialogs for desktop platforms

## rTexPacker Custom
 
Do you like this tool? Do you need any specific feature to be added? Maybe a custom import/export format? 
Maybe a custom gui theme? Or maybe a custom version for Linux, macOS, Android, Raspberry Pi or HTML5? 
Just get in touch: ray[at]raylibtech.com

## rTexPacker Issues & Feedback

You can report tool issues and feedback here: https://github.com/raylibtech/rtools  

## rTexPacker License

`rTexPacker` online tool can be used completely for free.

`rTexPacker Standalone` desktop tool could be downloaded with a donation. 

In any case, consider a donation to help the author keep working on software for games development.

*Copyright (c) 2019-2022 raylib technologies ([@raylibtech](https://twitter.com/raylibtech))*
