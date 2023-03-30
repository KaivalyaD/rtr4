rc.exe D3D.rc
cl.exe /c /EHsc *.cpp
link.exe *.obj *.res user32.lib gdi32.lib /SUBSYSTEM:WINDOWS /MACHINE:x64
del *.obj *.res