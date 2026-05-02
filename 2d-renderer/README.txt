simple 2d batched renderer in C and opengl.
dependencies (installed globally on /usr/* or /usr/local/*):
- GLFW 3.4
- GLEW (need GPU with GL 3.3 core profile support)
- cglm
these dependencies are easily replaceable, maybe I'll replace these with far more smaller
single header libs (rgfw, custom gl loader, raymath...) in the future.

TODO:
- replace dependencies
- sprite atlas (stb_image + stb_rect_pack)
- font rendering (just rasterize a ttf using stb_truetype lol)
