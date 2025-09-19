[![Build Status](https://github.com/OpenTimelineIO/toucan/actions/workflows/ci-workflow.yml/badge.svg)](https://github.com/OpenTimelineIO/toucan/actions/workflows/ci-workflow.yml)

# ![DJV Icon](images/toucan_64.svg) Toucan

Toucan is a software renderer for OpenTimelineIO files. Toucan can render an
OpenTimelineIO file with multiple tracks, clips, transitions, and effects
into an image sequence or movie file.

The project consists of:
* C++ library for rendering timelines
* Collection of OpenFX image effect plugins
* Command line renderer
* Interactive viewer
* Example .otio files

OpenFX Plugins:
* Generators: Checkers, Fill, Gradient, Noise
* Drawing: Box, Line, Text
* Filters: Blur, Color Map, Invert, Power, Saturate, Unsharp Mask
* Transforms: Crop, Flip, Flop, Resize, Rotate
* Transitions: Dissolve, Horizontal Wipe, Vertical Wipe
* Color space: Color Convert, Premultiply Alpha, Un-Premultiply Alpha

Toucan relies on the following libraries:
* [OpenTimelineIO](https://github.com/PixarAnimationStudios/OpenTimelineIO)
* [OpenFX](https://github.com/AcademySoftwareFoundation/openfx)
* [OpenImageIO](https://github.com/AcademySoftwareFoundation/OpenImageIO)
* [OpenColorIO](https://github.com/AcademySoftwareFoundation/OpenColorIO)
* [OpenEXR](https://www.openexr.com/)
* [FFmpeg](https://ffmpeg.org)
* [feather-tk](https://github.com/darbyjohnston/feather-tk)

Supported VFX platforms: 2024, 2023, 2022

Screenshot of the toucan viewer:
![toucan-view](images/toucan-view-screenshot1.png)


## Example Renders

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


## FFmpeg Encoding

Toucan can write movies with FFmpeg directly, or send raw images to the FFmpeg
command line program over a pipe.

Example command line writing a movie directly:
```
toucan-render Transition.otio Transition.mov -vcodec MJPEG
```

Raw images can be sent to FFmpeg as either the y4m format or raw video.

Example command line using the y4m format:
```
toucan-render Transition.otio - -y4m 444 | ffmpeg -y -i pipe: output.mov
```
* `Transition.otio`: The input timeline file.
* `-`: Write to standard out instead of a file.
* `-y4m 444`: Set the pixel format of the output images. Possible values: 422,
444, 444alpha, 444p16
* `-y`: Overwrite the output file if it already exists.
* `-i pipe:`: Read from standard input instead of a file.
* `output.mov`: The output movie file.

Example command line using raw video:
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


## Building

### Building on Linux

Dependencies:
* CMake 3.31

Install system packages (Debian based systems):
```
sudo apt-get install xorg-dev libglu1-mesa-dev mesa-common-dev mesa-utils
```

Install system packages (Rocky 9):
```
sudo dnf install libX11-devel libXrandr-devel libXinerama-devel libXcursor-devel libXi-devel mesa-libGL-devel
```

Clone the repository:
```
git clone https://github.com/OpenTimelineIO/toucan.git
```
Run the super build:
```
sh toucan/sbuild-linux.sh
```
Run the viewer application:
```
export LD_LIBRARY_PATH=$PWD/install-Release/lib:$LD_LIBRARY_PATH
```
```
install-Release/bin/toucan-view toucan/data/Transition.otio
```

### Building on macOS

Dependencies:
* Xcode
* CMake 3.31

Clone the repository:
```
git clone https://github.com/OpenTimelineIO/toucan.git
```
Run the super build:
```
sh toucan/sbuild-macos.sh
```
Run the viewer application:
```
install-Release/bin/toucan-view toucan/data/Transition.otio
```

### Building on Windows

Dependencies:
* Visual Studio 2022
* CMake 3.31
* MSYS2 (https://www.msys2.org) for compiling FFmpeg.

Open the Visual Studio command console "x64 Native Tools Command Prompt for VS 2022".
This can be found in the Start menu, in the "Visual Studio 2022" folder.

Clone the repository:
```
git clone https://github.com/OpenTimelineIO/toucan.git
```
Run the super build:
```
toucan\sbuild-win.bat
```
Run the viewer application:
```
set PATH=%CD%\install-Release\bin;%PATH%
```
```
install-Release\bin\toucan-view.exe toucan\data\Transition.otio
```

