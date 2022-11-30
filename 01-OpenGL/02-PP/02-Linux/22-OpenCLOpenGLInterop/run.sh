g++ -I /opt/cuda/include *.cpp -lX11 -lGL -lGLEW /opt/cuda/lib64/libOpenCL.so
./*.out
rm ./*.out
