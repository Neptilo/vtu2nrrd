This manual is only intended for Windows for now.

This tool hasn't been tested with other operating systems.

Build instructions
==================

* Download CMake and add `cmake.exe` to your PATH environment variable, e.g. `C:\Program Files\CMake\bin\`.
* Download [Visual Studio 16 2019 Build Tools](https://visualstudio.microsoft.com/thank-you-downloading-visual-studio/?sku=BuildTools&rel=16) because when using MinGW, some functions are missing and the build fails. Add `msbuild` to your PATH, e.g. `C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\MSBuild\Current\Bin`.
* Download VTK 8.2.0 source [from here](https://vtk.org/download/) and unzip it under `vtu2nrrd\libs`.
* Download InsightToolkit (ITK) 5.0.1 source [from here](https://itk.org/download/) and unzip it under `vtu2nrrd\libs`.
* Build VTK with Visual Studio:
  ```
  cd vtu2nrrd\libs\VTK-8.2.0\build
  cmake -DCMAKE_CONFIGURATION_TYPES=Debug build ..
  msbuild VTK.sln
  ```

* Build ITK with Visual Studio:
  * Go to the ITK build folder:
  ```
  cd vtu2nrrd\libs\ITK-5.0.1\build
  ```
  * Configure and generate ITK:

    In the command line below, replace `<path>` with the path to your VTK installation, ending with `\libs\VTK-8.2.0\build`. It apparently requires an absolute path.

    The module `ITKVtkGlue` is added to use a VTK to ITK image converter. Caution: in CMake GUI, this module is confusingly called "Module_ItkVtkGlue" with lowercase "tk". Go figure.

    `ITK_BUILD_DEFAULT_MODULES` is off to build as few modules as possible.

    In `CMAKE_FLAGS`, `/FS` has been added to the default flags to work around a bug that is supposed to be fixed in the next VS version 16.6.

  ```
  cmake -DModule_ITKVtkGlue:BOOL=ON -DVTK_DIR:PATH=<path> -DITK_BUILD_DEFAULT_MODULES:BOOL=OFF -DCMAKE_CONFIGURATION_TYPES=Debug -DCMAKE_CXX_FLAGS="/FS /DWIN32 /D_WINDOWS /W3" build ..
  ```

  * Build ITK:
  ```
  msbuild ITK.sln
  ```

* Add the VTK build path, ending with `\libs\VTK-8.2.0\build\bin\Debug` to your PATH environment variable.
* Build the program with Visual Studio 16 2019:
  ```
  cd vtu2nrrd\build
  cmake -DVTK_DIR:PATH=libs\VTK-8.2.0/build -DITK_DIR:PATH=libs\ITK-5.0.1/build -DCMAKE_CONFIGURATION_TYPES=Debug build ..
  msbuild vtu2nrrd.vcxproj
  ```

Usage
=====

Run:
  ```
  Debug\vtu2nrrd.exe path\to\test.vtu
  ```
