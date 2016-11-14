Build Instructions
===================

If you are here you possibly know that MoveToDesktop ships two helpers:

    MoveToDesktop.x86.exe - for x86 processes
    MoveToDesktop.x64.exe - for x64 processes

Each helper contains one `hook.dll` compiled for the same architecture as the helper.

Thats why MoveToDesktop (Any CPU) relays on MoveToDesktopRunner (x86, x64) and the Runner relays on hook.dll (x86, x64) and hook.lib (x86, x64).

hook.dll is used for the injection.

hook.lib is a static library used by the Runner to perform desktop tasks, such as switch to a deskop.

The build order is:

1. hook.lib (x86, x64)
2. hook.dll (x86, x64)
3. MoveToDesktopRunner (x86, x64)
4. MoveToDesktop (Any CPU)

To debug you should run the runner directly, choose x86 or x64, set the runner as `StartupProject` and run it.  
To build the complete application you should use batch build and select x86 and x64 for hook.lib, hook.dll and MoveToDekstopRunner. Select Any CPU for MoveToDesktop.  
After the build completes MoveToDesktop should bundle all files (in Release).