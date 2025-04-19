#include <GL/gl.h>
#include <GLFW/glfw3.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "util.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 800

#define GRID_WIDTH 256
#define GRID_HEIGHT 256


#define GLSL_VERSION "#version 460\n"
const char *vertex_shader =
GLSL_VERSION
"in vec2 position;"
"void main() {"
"    gl_Position = vec4(position, 0.0, 1.0);"
"}";

const char *fragment_shader =
GLSL_VERSION
"out vec4 outColor;"
"void main() {"
"    outColor = vec4(1.0, 1.0, 1.0, 1.0);"
"}";

#define GL_ERROR_PRINT() \
{                        \
    GLenum err = glGetError();  \
    fprintf(stderr, "OpenGL error: line %d:  %x\n", __LINE__, err); \
}


GLuint compile_shader_program(const char *vertex_code, const char *fragment_code);

static void glfw_error_callback(int e, const char *d) {
    fprintf(stderr, "Error %d: %s\n", e, d);
}


int main() {
    glfwSetErrorCallback(glfw_error_callback);

    if (!glfwInit()) {
        fprintf(stderr, "[GFLW] failed to init!\n");
        exit(1);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // We want OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT,
                                    "GLFW + OpenGL demo", NULL, NULL);
    if (!window) {
        fprintf(stderr, "[GFLW] failed to init window!\n");
        glfwTerminate();
        exit(1);
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // enable vsync
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    // Generate grid of vertices
    // To generate the grid of vertices, we pass grid_size and grid_location to the vertex shader
    // then the shader generates hovewer many vertices in range from -1 to 1
    //
    // Maybe, pass 4 boundary vertices, then do tesselation N times, or geometric shader

    // New idea:
    // Generate a cachable "image" in batches of set size, like on the website.
    // Do not update the "image" on every event, use cache instead.
    int grid[GRID_WIDTH*GRID_HEIGHT];

    static const GLfloat g_vertex_buffer_data[] = {
        -1.0f, -1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        0.0f,  1.0f, 0.0f,
    };

    GLuint vertex_array;
    GLuint vertexbuffer;
    glGenVertexArrays(1, &vertex_array);
    glGenBuffers(1, &vertexbuffer);

    glBindVertexArray(vertex_array);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
    /* glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer); */

    GLuint program = compile_shader_program(vertex_shader, fragment_shader);
    glUseProgram(program);

    GLuint position_attribute = glGetAttribLocation(program, "position");
    glEnableVertexAttribArray(position_attribute);
    glVertexAttribPointer(position_attribute, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    int i = 0;
    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.0, 0.0, 0.5 * (1 + sin(i++ * 0.02)), 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLES, 0, 3);


        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glfwSwapBuffers(window);

        /* fprintf(stderr, "%d: %d %d\n", idx, width, height); */

        glfwPollEvents();
        if (glfwGetKey(window, GLFW_KEY_Q ) == GLFW_PRESS) {
            break;
        }
        /* glfwWaitEvents(); */
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    fprintf(stderr, "Program exited successfully!\n");
    return 0;
}


GLuint compile_shader_program(const char *vertex_code, const char *fragment_code) {
    GLuint status = GL_FALSE;
    int log_length = 0;
    int vertex_code_len = strlen(vertex_code);
    int fragment_code_len = strlen(fragment_code);
    GLuint vertex_id = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragment_id = glCreateShader(GL_FRAGMENT_SHADER);
    // Compile shaders
    fprintf(stderr, "Compiling vertex shader :\n");
    glShaderSource(vertex_id, 1, &vertex_code, NULL);
    glCompileShader(vertex_id);
    glGetShaderiv(vertex_id, GL_COMPILE_STATUS, &status);
    glGetShaderiv(vertex_id, GL_INFO_LOG_LENGTH, &log_length);
    if (log_length > 0) {
        char *log = (char*)malloc(sizeof(char) * log_length + 1);
        glGetShaderInfoLog(vertex_id, log_length, NULL, log);
        fprintf(stderr, "Compiling vertex shader : %s", log);
        free(log);
    }
    fprintf(stderr, "Compiling fragment shader :\n");
    glShaderSource(fragment_id, 1, &fragment_code, NULL);
    glCompileShader(fragment_id);
    glGetShaderiv(fragment_id, GL_COMPILE_STATUS, &status);
    glGetShaderiv(fragment_id, GL_INFO_LOG_LENGTH, &log_length);
    if (log_length > 0) {
        char *log = (char*)malloc(sizeof(char) * log_length + 1);
        glGetShaderInfoLog(fragment_id, log_length, NULL, log);
        fprintf(stderr, "Compiling fragment shader : %s", log);
        free(log);
    }
    // Link program
    GLuint program_id = glCreateProgram();
    fprintf(stderr, "Linking program :\n");
    glAttachShader(program_id, vertex_id);
    glAttachShader(program_id, fragment_id);
    glLinkProgram(program_id);

    glGetProgramiv(program_id, GL_LINK_STATUS, &status);
	glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &log_length);
	if ( log_length > 0 ){
        char *log = (char*)malloc(sizeof(char) * log_length + 1);
		glGetProgramInfoLog(program_id, log_length, NULL, log);
        fprintf(stderr, "Linking program: %s", log);
        free(log);
	}
    // Cleanup
	glDetachShader(program_id, vertex_id);
	glDetachShader(program_id, fragment_id);
	glDeleteShader(vertex_id);
	glDeleteShader(fragment_id);
    return program_id;
}
