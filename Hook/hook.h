extern BOOL GetCurrentDesktopIndex(PUINT index, PUINT count);
extern BOOL SwitchDesktop(UINT index);
extern BOOL MoveWindowToDesktop(HWND hwnd, UINT index);
#undef CreateDesktop
extern BOOL CreateDesktop(PUINT index);
extern BOOL RemoveDesktop(UINT index, UINT fallbackIndex);
extern BOOL HasDesktopWindows(UINT index);