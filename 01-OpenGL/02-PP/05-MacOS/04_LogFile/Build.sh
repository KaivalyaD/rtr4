mkdir -p Window.app/Contents/MacOS
clang -o Window.app/Contents/MacOS/Window Window.m -framework Cocoa
