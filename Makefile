all:
	gcc -lv4l2 main.c webcam.c -Wall -Wextra -Wshadow -Wno-cast-function-type -o linux/webcam
	i686-w64-mingw32-gcc main.c webcam.c -Wall -Wextra -Wshadow -Wno-cast-function-type -o win32/webcam.exe
	x86_64-w64-mingw32-gcc main.c webcam.c -Wall -Wextra -Wshadow -Wno-cast-function-type -o win64/webcam.exe

clear:
	rm -f linux/webcam
	rm -f win32/webcam.exe
	rm -f win64/webcam.exe
