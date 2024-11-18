[![Build Status](https://github.com/OpenTimelineIO/toucan/actions/workflows/ci-workflow.yml/badge.svg)](https://github.com/OpenTimelineIO/toucan/actions/workflows/ci-workflow.yml)

<img src="images/toucan.svg" alt="toucan" width="100">

Toucan
======
Toucan is a software renderer for OpenTimelineIO files. Toucan can render an
OpenTimelineIO file with multiple tracks, clips, transitions, and effects
into an image sequence or movie file.

The project currently consists of:
* C++ library for rendering timelines
* Collection of OpenFX image effect plugins
* Command line renderer
* Interactive viewer
* Example .otio files

Current limitations:
* Audio is not yet supported
* Nested timelines are not yet supported
* Exporting movie files currently relies on the FFmpeg command line program
(see below: FFmpeg Encoding)

Toucan relies on the following libraries:
* [OpenTimelineIO](https://github.com/PixarAnimationStudios/OpenTimelineIO)
* [OpenFX](https://github.com/AcademySoftwareFoundation/openfx)
* [OpenImageIO](https://github.com/AcademySoftwareFoundation/OpenImageIO)
* [OpenColorIO](https://github.com/AcademySoftwareFoundation/OpenColorIO)
* [OpenEXR](https://www.openexr.com/)
* [FFmpeg](https://ffmpeg.org)

Supported VFX platforms: 2024, 2023, 2022

OpenFX Plugins
==============
The OpenFX image effect plugins include:
* Generators: Checkers, Fill, Gradient, Noise
* Drawing: Box, Line, Text
* Filters: Blur, Color Map, Invert, Power, Saturate, Unsharp Mask
* Transforms: Crop, Flip, Flop, Resize, Rotate
* Transitions: Dissolve, Horizontal Wipe, Vertical Wipe
* Color spaces: Color Convert, Premultiply Alpha, Un-Premultiply Alpha

Example Renders
===============
Compositing multiple tracks:

![CompositeTracks](images/CompositeTracks.png)

Filter effects; color map, invert, power, saturate, blur, and unsharp mask:

![Filters](images/Filter.png)

Gap:

![Gap](images/Gap.png)

Linear time warps:

![LinearTimeWarp](images/LinearTimeWarp.png)

Generators; fill, gradient, checkers, and noise:

![Generators](images/Generator.png)

Drawing; line, box, and text:

![Render](images/Draw.png)

Dissolve transition:

![Transition](images/Transition.png)

Dissolve transition with an invert effect on the clips:

![Transition 2](images/Transition2.png)

Wipe transitions:

![Transition Wipe](images/TransitionWipe.png)

Transforms; resize, rotate, flip, and flop:

![Transforms](images/Transform.png)

Color space conversion:

![Color Space](images/ColorSpace.png)

Multiple effects on clips, tracks, and stacks:

![Track Effects](images/MultipleEffects.png)

FFmpeg Encoding
===============
Toucan can send rendered images to the FFmpeg command line program for encoding.
The images can be sent as either the y4m format or RGB images. The images
are piped directly to FFmpeg without the overhead of disk I/O.

Example command line using the y4m format:
```
toucan-render Transition.otio - -y4m 444 | ffmpeg -y -i pipe: output.mov
```
* `Transition.otio`: The input timeline file.
* `-`: Write to standard out instead of a file.
* '-y4m 444': Set the pixel format of the output images. Possible values: 422,
444, 444alpha, 444p16
* `-y`: Overwrite the output file if it already exists.
* `-i pipe:`: Read from standard input instead of a file.
* `output.mov`: The output movie file.

Example command line using RGB images:
```
toucan-render Transition.otio - -raw rgba | ffmpeg -y -f rawvideo -pix_fmt rgba -s 1280x720 -r 24 -i pipe: output.mov
```
* `Transition.otio`: The input timeline file.
* `-`: Write to standard out instead of a file.
* `-raw rgba`: Set the pixel format of the output images. Possible values:
rgb24, rgb48, rgba, rgba64, rgbaf16, rgbaf32, rgbf32
* `-y`: Overwrite the output file if it already exists.
* `-f rawvideo`: Set the input to raw video frames.
* `-pix_fmt rgba`: Set the pixel format same as above.
* `-s 1280x720`: Set the size of the input frames. The image size
can be found by running `toucan-render` with the `-print_size` option.
* `-r 24`: Set the frame rate. The frame rate can be found by running
`toucan-render` with the `-print_rate` option.
* `-i pipe:`: Read from standard input instead of a file.
* `output.mov`: The output movie file.

Building
========

Building on Linux
-----------------
Clone the repository:
```
git clone https://github.com/OpenTimelineIO/toucan.git
```
Run the super build:
```
sh toucan/SuperBuild.sh Release
```
Run the viewer application:
```
export LD_LIBRARY_PATH=$PWD/install-Release/lib:$LD_LIBRARY_PATH
```
```
install-Release/bin/toucan-view toucan/data/Transition.otio
```

Building on macOS
-----------------
Clone the repository:
```
git clone https://github.com/OpenTimelineIO/toucan.git
```
Run the super build:
```
sh toucan/SuperBuild.sh Release
```
Run the viewer application:
```
install-Release/bin/toucan-view toucan/data/Transition.otio
```

Building on Windows
-------------------
Clone the repository:
```
git clone https://github.com/OpenTimelineIO/toucan.git
```
Run the super build:
```
toucan\SuperBuild.bat Release
```
Run the viewer application:
```
set PATH=%CD%\install-Release\bin;%PATH%
```
```
install-Release\bin\toucan-view.exe toucan\data\Transition.otio
```

