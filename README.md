# SunSet-EXT
SunSet Project, By TagliaficoHvH123

This is the code for my free and open-source project. From here, you can see how things are done and use examples for your projects or continue improving it.

You might think that because it's free and open-source, it's no longer functional or detectable, but no, this program is fully functional since it updates itself with each CS2 update, and currently remains undetectable as long as you don't kill 50 people in a match.

If you just want to use it, you can download the loader I made: https://github.com/TagliaficoHvH/Sunset-CS2-Loader

By the way, don't leave the loader code anywhere, if you modify something in the code and compile it, it will leave an exe in the build folder, if you open it you will have the program.


This project uses C++ with OpenCV 4.12.0.

## Requirements
- Visual Studio 2019/2022
- vcpkg with OpenCV:


## How to compile
1. Clone the repo.
2. Open the solution in Visual Studio.
3. Select the **x64** platform.
4. Compile in Release.
5. Copy `opencv_world4120.dll` (from vcpkg or OpenCV installation) next to the `.exe`. You can also place the dll inside the project and configure it to place it in the build folder every time you compile, just to save yourself the trouble.
