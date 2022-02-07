cl.exe /c /EHsc Window.c
rc.exe Window.rc
link.exe Window.obj Window.res user32.lib gdi32.lib /SUBSYSTEM:WINDOWS /MACHINE:x64
rm *.obj *.res
Window.exe
rm *.exe
