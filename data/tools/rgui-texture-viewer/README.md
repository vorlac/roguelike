<img align="left" src="logo/rtexviewer_256x256.png" width=256>

# rTexViewer

A simple and easy-to-use textures viewer and formats converter. 

Visualize multiple image file formats and convert between different pixel-formats in real-time. Some interesting image editing functionality included!

`rTexViewer` can be used for free as a [WebAssembly online tool](https://raylibtech.itch.io/rtexviewer) and it can also be downloaded as a **standalone tool** for _Windows_ and _Linux_ with some extra features.

<br>

## rTexViewer Features

 - Load **multiple image file formats**: PNG, BMP, TGA, JPG, GIF, HDR
 - Load **compressed texture** file formats: DDS, PKM, KTX, PVR, ASTC (if supported by GPU)
 - **Export** edited image as PNG, **QOI, RAW or CODE (.h)**
 - Select desired image **pixel-format with real-time preview**
 - **Edit** your images: Flip, Rotate, Resize, Crop, Alpha management, Mipmaps...
 - **Visualization options**: Zoom, Pan, Background and image Helpers
 - **Visual image crop mode**: just draw a rectangle and crop
 - **Image information provided**: Size, Data size, Selected Pixel info, **Palette**
 - Multiple UI styles supported, select your best style
 - **Export image palette** as PNG/PAL (up to 256 colors)
 - **Export alpha channel** as PNG image

### rTexViewer Standalone Additional Features

 - **Navigate directory** images (supported formats)
 - **PNG text chunk edition** and saving
 - **TXT file-to-image** conversion with extra viewing info
 - Command-line support for **batch image processing** and formats conversion
 - **Completely portable (single-file, no-dependencies)**
 
## rTexViewer Screenshot

![rTexViewer](screenshots/rtexviewer_v200_shot01.png)
 
## rTexViewer Usage

Open the tool, drag & drop your image and view/edit it with the dition tools (right panel).

Formatted image can be exported as `.png`, `.qoi`, `.raw` and `.h` code file.

`rTexViewer Standalone` supports directory navigation for viewing. When an image is opened, program checks directory for all supported image file formats for further navigation with LEFT/RIGHT keys or botton bar navigation buttons.

`rTexViewer Standalone` comes with command-line support for batch image processing and format conversion. Up to 256 chainned transformations can be applied per image on the command-line. For usage help:

 > rtexviewer.exe --help

## rTexViewer License

`rTexViewer` online tool can be used completely for free.

`rTexViewer Standalone` desktop tool could be downloaded with a small donation. 

In any case, consider a donation to help the author keep working on software for games development.

*Copyright (c) 2015-2022 raylib technologies ([@raylibtech](https://twitter.com/raylibtech))*
