# GDExtension Visual Studio Solution Template

## Prerequisite
To deploy development environment, the following software needs to be installed:
| Type    | Software |
| -------- | ------- |
| Debugging  | Visual Studio    |
| Generate Project | CMake     |
| Compiling Godot    | Python - scons    |


## 1. Build Godot Engine
To properly debug GDExtension or engine-level code, you must build the Godot Engine from source with debug symbols.

1.  Navigate to your [Godot Engine](https://github.com/godotengine/godot) source directory.
2.  Run the following command to generate Visual Studio Project:
    ```bash
    scons platform=windows vsproj=yes dev_build=yes
    ```
    *Note: Change the `platform` argument if you are building for a different operating system.*
3. Compile the source code so that godot executable is in ./bin


## 2. Generate Project Files
The template uses CMake to generate the Visual Studio solution. 
For compilation options including extra include path and target lib, see [`CMakeLists.txt`](addons/gdextension/CMakeLists.txt)

1.  This repository contains submodule of [godot-cpp](https://github.com/godotengine/godot-cpp), make sure the submodule is up-to-date by running:
    ```bash
    git submodule init
    ```
    Then
    ```bash
    git submodule update
    ```
2.  Locate the [`GenerateProjectFile.bat`](addons/gdextension/GenerateProjectFiles.bat) file in the root directory.
3.  **Visual Studio Version:** If you are using a specific version of Visual Studio, open the `.bat` file and modify the generator line:
    * Find: `set CMAKE_GENERATOR="Visual Studio 17 2022"`
    * Change to your required version.
4.  Run `GenerateProjectFile.bat`.
5.  The generated Visual Studio project files will be located in the **`./Build`** folder.


## 3. Visual Studio Configuration & Debugging
To debug your code while running the Godot Editor, you must configure the project properties.

### Project Setup
1.  Open the solution inside the `Build` folder.
2.  **Set Startup Project:** Right-click the **Visual Studio** project in the Solution Explorer and select **Set as StartUp Project**.
3.  (Optional) Add your Godot Project(godot.vcxproj) to the solution for easier navigation and breakpoint debugging.

### Debugger Settings
Right-click the **Visual Studio** project, go to **Properties**, and navigate to **Configuration Properties > Debugging**:

* **Command:** Set this to the absolute path of your compiled Godot dev-build executable (ensure the `.pdb` file exists).
    * *Example:* `E:\godot\bin\godot.windows.editor.dev.x86_64.exe`
* **Command Arguments:** Use the `-e` flag followed by the path to your Godot project directory.
    * *Example:* `-e "E:\Projects\GDExtension"`
* **Working Directory:** Set this to your Godot project working directory.
    * *Example:* `E:\Projects\GDExtension`