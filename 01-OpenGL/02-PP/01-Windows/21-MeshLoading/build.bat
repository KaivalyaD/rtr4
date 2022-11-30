cls

cl.exe /c /EHsc meshloader/cvector/cvector.c meshloader/meshloader.c ./OGL.cpp
rc.exe OGL.rc
link.exe cvector.obj meshloader.obj OGL.obj *.res /OUT:OGL.exe "C:\MyProjects\Requirements\glew\lib\Release\x64\glew32.lib" user32.lib gdi32.lib /SUBSYSTEM:WINDOWS /MACHINE:x64

del OGL.obj meshloader.obj cvector.obj
del *.res