cls

del *.exe
del *.obj
del *.res

cl.exe /c /EHsc *.cpp

rc.exe OGL.rc

link.exe *.obj *.res "C:\MyProjects\Requirements\glew\lib\Release\x64\glew32.lib" user32.lib gdi32.lib /SUBSYSTEM:WINDOWS /MACHINE:x64
