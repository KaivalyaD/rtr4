del res.res
del *.exe *.obj
rc.exe res.rc
cl.exe /c /EHsc mandelbrot.cpp
link.exe mandelbrot.obj res.res user32.lib gdi32.lib /SUBSYSTEM:WINDOWS /MACHINE:x64
mandelbrot.exe
