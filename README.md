# EfiGopPictureLib
 EGP (EFI GOP Picture) format decoder library (currently WIP)
# Requirements
 This project needs the following present:
- Visual Studio 2017 or newer (with Desktop development with C++ installed)
- [VisualUEFI](https://github.com/ionescu007/VisualUefi) by Alex Ionescu
# Usage
 To compile this project, copy its files on to <VisualUEFI project path>\EDKII\EfiGopPictureLib, then add the respective project onto the EDK-II.sln solution.

 To use this in a VisualUEFI EDK2 app, add <VisualUEFI project path>\EDKII\EfiGopPictureLib onto the include directories and $(SolutionDir)..\EDK-II\ onto the library directories, then add EfiGopPictureLib.lib onto the additional linker dependencies.
# Progress
 Currently only the following things are done: initialization of the external structure.
