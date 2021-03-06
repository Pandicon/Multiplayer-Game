This is Multiplayer-Game client.
# How to use
Download pre-build client in releases (will be "soon") or build it yourself (check below). Then run
the multi-client (/client/bin/multi-client) executable, input nickname, server IP and port and
connect to server. You'll see a textbox and game board. You can write messages into the textbox,
but you can write length of your path prefixed with '+' there. When you are showing, you can click
onto the robots and arrows will show up, which you can use to move the robots. There are also
commands:
command | effect | aliases
--- | --- | ---
!bruteforce | starts/stops bruteforcer | !bf
!bruteforce nextpath | shows next path | !bf n
!bruteforce pathcount | shows path count | !bf pathc
!bruteforce prevpath | shows previous path | !bf p
!clear | clears chat | none
!deletetrail | hides trail | none
!exit | exits | none
!giveup | gives up when showing | !gu
!renick \<name\> | sets a new name | none
!reset | resets bot positions | !r
!settrg \<color\> \<x\> \<y\> | sets target | none
!undo | takes back last move | !u

Enjoy! :)
# Used libraries
It uses multiple libraries, which are also
needed to compile the client. Used libraries are:
library | download link | note
--- | --- | ---
glfw3 | https://www.glfw.org/download.html | (window)
glad | https://glad.dav1d.de/#language=c&specification=gl&api=gl%3D4.0&api=gles1%3Dnone&api=gles2%3Dnone&api=glsc2%3Dnone&profile=core&loader=on | (OpenGL loader)
stb | https://github.com/nothings/stb | only stb_image.h (texture loading)
glm | https://github.com/g-truc/glm | (math)
asio | https://think-async.com/Asio/Download.html | (networking)
nlohmann's json | https://github.com/nlohmann/json | (json parser)

If you want to compile client yourself, put header files inside include folder,
source files (glad.c) directly into libs folder and libraries (glfw) into
libs/<system>/ where <system> is win, mingw or linux. Structure of include
should be:
```
include
+--+ glad
|  +--- glad.h
|
+--+ GLFW
|  +--- glfw3.h
|  +--- glfw3native.h
|
+--+ glm
|  +--- <glm headers, there is a lot of them>
|
+--+ KHR (added by glad)
|  +--- khrplatform.h
|
+--+ stb
|  +--- <stb headers>
|
+--+ asio
|  + <asio headers>
|
+--+ asio.hpp
|
+--+ json.hpp
```
# Compilation
Compiling should be pretty easy once you setup libraries (check "Used libraries"). Just use CMake, for example this way:
```shell
mkdir build
cmake .. -DCMAKE_BUILD_TYPE=Release
<then compile using make, vs...>
```
the output executable should then be in bin.
