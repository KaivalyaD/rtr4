cls

del *.exe
del *.obj
del *.res

cl.exe /c /EHsc *.cpp

rc.exe OGL.rc

link.exe *.obj *.res user32.lib gdi32.lib winmm.lib /SUBSYSTEM:WINDOWS /MACHINE:x64
