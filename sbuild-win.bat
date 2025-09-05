set BUILD_TYPE=%1
IF "%BUILD_TYPE%"=="" set BUILD_TYPE=Release

IF "%FFMPEG_MINIMAL%"=="" set FFMPEG_MINIMAL=OFF

set JOBS=4

cmake ^
    -S toucan\cmake\SuperBuild ^
    -B sbuild-%BUILD_TYPE% ^
    -DCMAKE_INSTALL_PREFIX=%CD%\install-%BUILD_TYPE% ^
    -DCMAKE_PREFIX_PATH=%CD%\install-%BUILD_TYPE% ^
    -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
    -Dtoucan_FFmpeg_MINIMAL=%FFMPEG_MINIMAL%
cmake --build sbuild-%BUILD_TYPE% -j %JOBS% --config %BUILD_TYPE%

cmake ^
    -S toucan ^
    -B build-%BUILD_TYPE% ^
    -DCMAKE_INSTALL_PREFIX=%CD%\install-%BUILD_TYPE% ^
    -DCMAKE_PREFIX_PATH=%CD%\install-%BUILD_TYPE% ^
    -DCMAKE_BUILD_TYPE=%BUILD_TYPE%
cmake --build build-%BUILD_TYPE% -j %JOBS% --config %BUILD_TYPE%
cmake --build build-%BUILD_TYPE% --config %BUILD_TYPE% --target INSTALL

