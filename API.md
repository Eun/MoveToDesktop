# MoveToDesktop API

You can use the MoveToDesktop API to move windows around, switch to desktops or create new desktops.  

Requirement is that MoveToDesktop runs and is active on the desired process.  

MoveToDesktop runs helper processes for accessing different processor architectures.  
This means for x64 Windows installs, there will be two helper processes:

    MoveToDesktop.x86.exe
    MoveToDesktop.x64.exe

To use an API for a x86 Process you should use `MoveToDesktop.x86.exe`,  
for x64 `MoveToDesktop.x64.exe`

Each helper has following parameters:

    Desktop Functions
        --create-desktop                     Create a new desktop
        --desktop-count                      Return the current desktop count
        --desktop-index                      Return the current desktop index
        --switch-desktop <INDEX>             Switch the desktop to the INDEX
        --remove-desktop <INDEX> <FALLBACK>  Remove the desktop at INDEX, and switch to FALLBACK if it is the current desktop
        --remove-empty-desktops              Remove all desktops that are empty

    Window Functions
        --move-to-new-desktop <HWND>         Create a new desktop and move the window with the HWND to the desktop
        --move-to-left-desktop <HWND>        Move the window with the HWND to the left desktop
        --move-to-right-desktop <HWND>       Move the window with the HWND to the right desktop
        --move-to-desktop <HWND> <INDEX>     Move the window with the HWND to the desktop at INDEX

    General Functions
        --is-running                         Return 0 if this runner is running
        --exit                               Exit the runner
        --help                               Show the help

    Notice that all return values are exit codes!

You can find the helpers by calling the main `MoveToDesktop.exe` with `--get-api-helper=<HWND>`,  
It will print the path to the helper.

You can find example scripts in the `apiexamples` directory.


