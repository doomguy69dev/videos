#include <GLFW/glfw3.h>
#include <dlfcn.h>
#include <stdio.h>

#define GL_COLOR_BUFFER_BIT 0x00004000
#define GL_VERSION 0x1F02

// name, return type, args...
#define GL_FUNCS\
    X(glClear, void, int)\
    X(glClearColor, void, float, float, float, float)\
    X(glGetString, unsigned char*, unsigned int)

int main(void) {
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow *win = glfwCreateWindow(800, 600, "test", NULL, NULL);
    glfwMakeContextCurrent(win);

    void *gl = dlopen("libGL.so", RTLD_LAZY);
    if (!gl) return 1;
#define X(name,return_type,...) return_type (*name)(__VA_ARGS__) = dlsym(gl, #name);\
        if (!name) { fprintf(stderr, "ded at %s:%d\n", __FILE__, __LINE__); return 1; }
    GL_FUNCS;
#undef X

    printf("gl version: %s\n", glGetString(GL_VERSION));

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

    while (!glfwWindowShouldClose(win)) {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT);
        glfwSwapBuffers(win);
    }

    dlclose(gl);

    glfwDestroyWindow(win);
    glfwTerminate();
    return 0;
}
