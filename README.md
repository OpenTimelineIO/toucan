[![Build Status](https://github.com/OpenTimelineIO/toucan/actions/workflows/ci-workflow.yml/badge.svg)](https://github.com/OpenTimelineIO/toucan/actions/workflows/ci-workflow.yml)

<img src="images/toucan.svg" alt="toucan" width="100">

Toucan
======
Toucan is an experimental project for rendering OpenTimelineIO files. The
project currently consists of:
* C++ library for rendering timelines
* Collection of OpenFX image effect plugins
* Command line renderer
* Interactive viewer
* Example .otio files

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

![Composite Tracks Graph](images/CompositeTracksGraph.svg)

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

![Transition Graph](images/TransitionGraph.svg)

Dissolve transition with an invert effect on the clips:

![Transition 2](images/Transition2.png)

![Transition 2 Graph](images/Transition2Graph.svg)

Wipe transitions:

![Transition Wipe](images/TransitionWipe.png)

Transforms; resize, rotate, flip, and flop:

![Transforms](images/Transform.png)

Color space conversion:

![Color Space](images/ColorSpace.png)

Multiple effects on clips, tracks, and stacks:

![Track Effects](images/MultipleEffects.png)

![Multiple Effects Graph](images/MultipleEffectsGraph.svg)

FFmpeg Encoding
===============
Toucan can output rendered images directly to FFmpeg for encoding. The
images are piped to FFmpeg without incurring the overhead of disk I/O.

Example command line for piping images to FFmpeg:
```
toucan-render Transition.otio - -raw rgba | ffmpeg -y -f rawvideo -pix_fmt rgba -s 1280x720 -r 24 -i pipe: output.mov
```
* `Transition.otio`: The input timeline file.
* `-`: Write to standard out instead of a file.
* `-raw rgba`: Set the pixel format of the output frames. This should
match the `-pix_fmt` option given to FFmpeg. One exception is that toucan
options do not specify the endian, the endian of the current machine is used.
So for example the toucan option `-raw rgbaf16` might match the FFmpeg option
`-pix_fmt rgbaf16le` on the current machine. Check the `toucan-render` command
line help for the list of available formats.
* `-y`: Overwrite the output file if it already exists.
* `-f rawvideo`: Set the input to raw video frames.
* `-pix_fmt rgba`: Set the input pixel format as described above.
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

