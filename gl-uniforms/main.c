#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include <GLES3/gl3.h>
#include <GLFW/glfw3.h>

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

typedef struct {
    char name[64];
    int size, location;
    GLenum type;
} Uniform_Location;

typedef struct {
    char *key;
    Uniform_Location value;
} Uniform_Location_Entry;

static void framebuffer_resize_callback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
}

static uint32_t create_shader_from_source(const char *vertex_shader_source, const char *fragment_shader_source) {
    uint32_t vertex_shader, fragment_shader, shader_program;
    int compile_result; char info_log[1024];

    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
    glCompileShader(vertex_shader);
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &compile_result);
    if (!compile_result) {
        glGetShaderInfoLog(vertex_shader, 1024, NULL, info_log);
        fprintf(stderr, "ERROR: failed to compile vertex shader, error: %s\n", info_log);
        return UINT32_MAX;
    }

    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
    glCompileShader(fragment_shader);
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &compile_result);
    if (!compile_result) {
        glGetShaderInfoLog(fragment_shader, 1024, NULL, info_log);
        fprintf(stderr, "ERROR: failed to compile fragment shader, error: %s\n", info_log);
        return UINT32_MAX;
    }

    shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);
    glValidateProgram(shader_program);
    glGetProgramiv(shader_program, GL_LINK_STATUS, &compile_result);
    if (!compile_result) {
        glGetProgramInfoLog(shader_program, 1024, NULL, info_log);
        fprintf(stderr, "ERROR: failed to link shader program, error: %s\n", info_log);
        return UINT32_MAX;
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return shader_program;
}

static const char *triangle_vertex_shader_source = "#version 300 es\n"
    "layout (location = 0) in vec2 in_position;"
    "uniform vec2 urmom;"
    "void main() {"
        "gl_Position = vec4(in_position + urmom, 0.0, 1.0);"
    "}";
static const char *triangle_fragment_shader_source = "#version 300 es\n"
    "precision mediump float;"
    "out vec4 out_color;"
    "uniform vec3 color;"
    "uniform float time;"
    "void main() {"
        "out_color = vec4(color, time);"
    "}";

static Uniform_Location_Entry *get_uniform_locations(uint32_t shader_program) {
    Uniform_Location_Entry *uniform_locations = NULL;
    int uniform_count;
    glGetProgramiv(shader_program, GL_ACTIVE_UNIFORMS, &uniform_count);
    assert(uniform_count != 0);
    for (int i = 0; i < uniform_count; i++) {
        Uniform_Location uniform_location;
        glGetActiveUniform(shader_program, i, sizeof(uniform_location.name), NULL, &uniform_location.size, &uniform_location.type, uniform_location.name);
        uniform_location.location = glGetUniformLocation(shader_program, uniform_location.name);
        shput(uniform_locations, strdup(uniform_location.name), uniform_location);
    }
    return uniform_locations;
}

int main(void) {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    GLFWwindow *window = glfwCreateWindow(800, 600, "urmom", NULL, NULL);
    glfwSetFramebufferSizeCallback(window, framebuffer_resize_callback);

    glfwMakeContextCurrent(window);
    printf("INFO: using %s\n", glGetString(GL_VERSION));
    glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    uint32_t triangle_shader_program = create_shader_from_source(triangle_vertex_shader_source, triangle_fragment_shader_source);
    assert(triangle_shader_program != UINT32_MAX);
    glUseProgram(triangle_shader_program);

    Uniform_Location_Entry *uniform_locations = get_uniform_locations(triangle_shader_program);

    const float triangle_vertices[] = {
         0.0f,  0.5f,
        -0.5f, -0.5f,
         0.5f, -0.5f,
    };

    uint32_t vbo, vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_vertices), triangle_vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), NULL);
    glEnableVertexAttribArray(0);

    const float triangle_color[] = { 1.0f, 0.0f, 0.0f };

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        glUniform3fv(shget(uniform_locations, "color").location, 1, triangle_color);
        glUniform1f(shget(uniform_locations, "time").location, fabsf(sinf((float)glfwGetTime())));
        const float urmom[] = { 0.5f, 0.5f };
        glUniform2fv(shget(uniform_locations, "urmom").location, 1, urmom);

        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
