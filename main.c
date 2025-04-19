#include <GL/gl.h>
#include <GLFW/glfw3.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 800

#define GRID_WIDTH 256
#define GRID_HEIGHT 256


#define GLSL_VERSION "#version 460\n"
const char *vertex_shader =
GLSL_VERSION
"in vec2 position;"
"in vec3 texcoord;"
"out vec3 Texcoord;"
"void main() {"
"   Texcoord = texcoord;"
"   gl_Position = vec4(position, 0.0, 1.0);"
"}";

const char *fragment_shader =
GLSL_VERSION
"in vec3 Texcoord;"
"out vec4 outColor;"
"uniform sampler2DArray chunk;"
"void main() {"
"   outColor = texture(chunk, Texcoord).rrrr;"
"}";

#define GL_ERROR_PRINT() \
{                        \
    GLenum err = glGetError();  \
    fprintf(stderr, "OpenGL error: line %d:  %x\n", __LINE__, err); \
}


GLuint compile_shader_program(const char *vertex_code, const char *fragment_code);
void compute_mandelbrot_chunk(const GLfloat position_rect[4], GLsizei width, GLsizei height, GLfloat *chunk);
GLfloat compute_mandelbrot(GLfloat x, GLfloat y);


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

    // New New idea:
    // Generate a pool of textures, then draw stuff onto them pixel by pixel.
    // The textures are mostly persistent in memory. The shaders will have to
    // only draw these textures in a location specified by vertices
    int grid[GRID_WIDTH*GRID_HEIGHT];
    GLsizei width = 1000;
    GLsizei height = 1000;
    GLsizei chunk_count = 1;
    GLfloat pixels[1000*1000];
    GLfloat chunk_position_rect[] = {
        -2.0f, -1.0f, 0.4f, 1.0f,
    };
    compute_mandelbrot_chunk(&chunk_position_rect[0], width, height, &pixels[0]);

    GLuint chunk;
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &chunk);
    glBindTexture(GL_TEXTURE_2D_ARRAY, chunk);
    glTextureStorage3D(chunk, 1, GL_R32F, width, height, chunk_count);
    glTextureSubImage3D(chunk, 0, 0, 0, 0, width, height, chunk_count, GL_RED, GL_FLOAT, &pixels[0]);

    glTextureParameteri(chunk, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(chunk, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(chunk, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(chunk, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    /* glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, { 1.0f, 0.0f, 0.0f, 1.0f }); */
    // TODO: use a texture mipmap instead

    static const GLfloat g_vertex_buffer_data[] = {
    //  Position        Texcoord
    //      X      Y       Z      X      Y
        -0.5f, -0.5f,   1.0f,  0.0f,  1.0f,
         0.5f, -0.5f,   1.0f,  1.0f,  1.0f,
         0.5f,  0.5f,   1.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,   1.0f,  0.0f,  0.0f,
    };

    GLuint vertex_array;
    GLuint vertexbuffer;
    glGenVertexArrays(1, &vertex_array);
    glGenBuffers(1, &vertexbuffer);

    glBindVertexArray(vertex_array);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

    GLuint program = compile_shader_program(vertex_shader, fragment_shader);
    glUseProgram(program);

    GLuint position_attribute = glGetAttribLocation(program, "position");
    glEnableVertexAttribArray(position_attribute);
    glVertexAttribPointer(position_attribute, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)0);
    GLuint texcoord_attribute = glGetAttribLocation(program, "texcoord");
    glEnableVertexAttribArray(texcoord_attribute);
    glVertexAttribPointer(texcoord_attribute, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));

    glUniform1i(glGetUniformLocation(program, "chunk"), 0);

    int i = 0;
    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.0, 0.0, 0.5 * (1 + sin(i++ * 0.02)), 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        /* glDrawArrays(GL_POINTS, 0, 4); */
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);


        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glfwSwapBuffers(window);

        glfwPollEvents();
        /* glfwWaitEvents(); */
        if (glfwGetKey(window, GLFW_KEY_Q ) == GLFW_PRESS) {
            break;
        }
    }

    glDeleteTextures(1, &chunk);

    glDeleteProgram(program);
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteVertexArrays(1, &vertex_array);

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


void compute_mandelbrot_chunk(const GLfloat position_rect[4], GLsizei width, GLsizei height, GLfloat *chunk) {
    GLfloat step_x = (position_rect[2] - position_rect[0]) / width;
    GLfloat step_y = (position_rect[3] - position_rect[1]) / height;
    for (int i = 0; i < height; i++) {
        GLfloat y =  position_rect[1] + (i + 0.5) * step_y;
        for (int j = 0; j < width; j++) {
            GLfloat x =  position_rect[0] + (j + 0.5) * step_x; // 0.5 to center the integration
            chunk[i * width + j] = compute_mandelbrot(x, y);
        }
    }
}

#define DEPTH 500
GLfloat compute_mandelbrot(GLfloat x, GLfloat xi) {
    float z = 0.0;
    float zi = 0.0;
    for (int i = 1; i < DEPTH; i++) {
        float zprev = z;
        z = z * z - zi * zi;
        zi = 2 * zprev * zi;
        if (z * z + zi * zi < 4) {
            z += x;
            zi += xi;
        } else {
            return 1.0f;
        }
    }
    return 0.0f;
}
