rc.exe D3D.rc
cl.exe /c /EHsc *.cpp
link.exe *.obj *.res user32.lib gdi32.lib ole32.lib oleaut32.lib /SUBSYSTEM:WINDOWS
del *.obj *.res