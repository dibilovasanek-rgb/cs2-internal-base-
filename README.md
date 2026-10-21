# CS2 INTERNAL BASE

<img width="1920" height="1080" alt="image" src="https://github.com/user-attachments/assets/f59ad514-960b-4733-a48c-9e3697385e3c" /> 

An internal C++ project serving as a base for a Counter-Strike 2 internal cheat. The project is built as an x64 DLL.

## Structure

```text
main/
├─ cs2-project.sln          Visual Studio solution
├─ cs2-project/              DLL source code
│  ├─ core/                  Initialization, hooks, and menu
│  ├─ dependencies/          ImGui, MinHook, Protobuf, and auxiliary libraries
│  ├─ features/              Game features
│  ├─ memory/                Address scanning and memory operations
│  ├─ render/                DirectX 11 and UI rendering
│  └─ sdk/                   CS2 interfaces and structures
├─ models/                   Additional model resources
├─ build/                    Build output directory
└─ vcpkg_installed/          Local project dependencies
```

## Requirements

- Windows 10/11 x64
- Visual Studio with the **Desktop development with C++** workload
- Windows SDK
- MSVC toolset specified in `cs2-project/cs2-project.vcxproj`
- An installed and running copy of CS2 for functional testing

## Building in Visual Studio

1. Open `cs2-project.sln`.
2. Select the `x64` platform.
3. Use `Debug` for routine testing and `Release` for the final build.
4. Run `Build → Rebuild Solution`.

Default output paths:

```text
build\debug\cs2-project.dll
build\release\cs2-project.dll
```

## Building from Developer PowerShell

From the `main\main` directory:

```powershell
msbuild .\cs2-project.sln /t:Rebuild /p:Configuration=Release /p:Platform=x64 /m:1
```

If multiple builds are running simultaneously, use separate intermediate directories:

```powershell
msbuild .\cs2-project.sln /t:Rebuild `
  /p:Configuration=Release /p:Platform=x64 /m:1 `
  /p:IntDir=x64\Release-local\
```

## Resources

UI resources should be placed in an `assets` directory alongside the DLL. If a configuration file or model changer is used, verify the corresponding paths in the project settings and keep Debug and Release resources separate.

## Build Troubleshooting

- `MSB3073` usually indicates that a child PowerShell/CMake step failed. Run the command from the error message separately and save the full output.
- `C1041` often occurs during parallel builds. Use `/m:1` and a new `IntDir`.
- If the DLL builds successfully but the UI does not appear, first check DirectX 11 initialization and ensure that the architecture matches (`x64`).
- After a CS2 update, verify that signatures and offsets are up to date before functional testing.

## Limitations

Updates are required for every patch. Get patterns and offsets from:

https://www.cs2-sdk.com/

<img width="1014" height="513" alt="image" src="https://github.com/user-attachments/assets/f25882e7-b659-45a7-8721-8ebfc9ef087e" />

<img width="1920" height="1080" alt="image" src="https://github.com/user-attachments/assets/1c22a2da-5c12-4791-94e5-0baa476ec7ab" />

