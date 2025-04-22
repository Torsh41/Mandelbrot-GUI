#include <GL/gl.h>
#include <GLFW/glfw3.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 800

#define CHUNK_WIDTH_PX 50
#define CHUNK_HEIGHT_PX 50
#define CHUNK_SIZE_RESERVE_MULTIPLIER 4.0

#define GL_ERROR_PRINT() \
{                        \
    GLenum err = glGetError();  \
    fprintf(stderr, "OpenGL error: line %d:  %x\n", __LINE__, err); \
}


GLuint create_shader(GLenum type, const char *code);
GLuint create_shader_from_file(GLenum type, const char *filename);
GLuint link_program(GLuint vertex_id, GLuint geometry_id, GLuint fragment_id);
void compute_mandelbrot_chunk(const GLfloat pos[2], const GLfloat size[2], GLsizei width_px, GLsizei height_px, GLfloat *chunk);
GLfloat compute_mandelbrot(GLfloat x, GLfloat y);


static void glfw_error_callback(int e, const char *d) {
    fprintf(stderr, "Error %d: %s\n", e, d);
}

static void window_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}



int main() {
    glfwSetErrorCallback(glfw_error_callback);

    if (!glfwInit()) {
        fprintf(stderr, "[GFLW] failed to init!\n");
        exit(1);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // We want OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    /* glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); */
    GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT,
                                    "GLFW + OpenGL demo", NULL, NULL);
    if (!window) {
        fprintf(stderr, "[GFLW] failed to init window!\n");
        glfwTerminate();
        exit(1);
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // enable vsync
    glfwSetWindowSizeCallback(window, window_size_callback);
    GLfloat window_rec[4] = { -1.8f, -1.0f, 2.4f, 2.0f };
    const GLFWvidmode *screen_resolution = glfwGetVideoMode(glfwGetPrimaryMonitor());
    // New New idea:
    // Generate a pool of textures, then draw stuff onto them pixel by pixel.
    // The textures are mostly persistent in memory. The shaders will have to
    // only draw these textures in a location specified by vertices
    // Also, the first "chunk" will be the placeholder texture, that will be
    // displayed if the acrual texture is not yet calculated
    const GLsizei chunk_width = CHUNK_WIDTH_PX;
    const GLsizei chunk_height = CHUNK_HEIGHT_PX;
    // TODO: calculate chunk count properly. If too many chunks, everything turns black
    GLsizei chunk_count_x = (int)(screen_resolution->width / chunk_width) + 1 - 6;
    GLsizei chunk_count_y = (int)(screen_resolution->height / chunk_height) + 1 - 7;
    const GLsizei chunk_count = (int)(CHUNK_SIZE_RESERVE_MULTIPLIER * chunk_count_x * chunk_count_y);
    GLfloat chunk_size[2] = {
        window_rec[2] / chunk_count_x,
        window_rec[3] / chunk_count_y,
    };
    GLsizei chunk_pixel_data_len = chunk_width * chunk_height * chunk_count + 1;
    GLfloat *chunk_pixel_data = (GLfloat*)malloc(chunk_pixel_data_len * sizeof(chunk_pixel_data[0]));
    const GLsizei chunk_vertex_len = 3;
    GLsizei chunk_vertex_data_len = chunk_vertex_len * chunk_count;
    GLfloat *chunk_vertex_data = (GLfloat*)malloc(chunk_vertex_data_len * sizeof(chunk_vertex_data[0]));
    // Init gray placeholder texture
    for (int i = 0; i < chunk_width * chunk_height; i++) {
        chunk_pixel_data[i] = 0.5f;
    }
    // Init vertices
    for (int i = 0; i < chunk_vertex_len * chunk_count; i++) {
        chunk_pixel_data[i] = 0.0f;
    }
    // Calculate chunk pixel values
    int counter = 1;
    for (int i = 0; i < chunk_count_y + 1; i++) {
        for (int j = 0; j < chunk_count_x + 1; j++) {
            int vertex_data_offset = (counter - 1) * chunk_vertex_len;
            chunk_vertex_data[vertex_data_offset + 0] = window_rec[0] + j * chunk_size[0];
            chunk_vertex_data[vertex_data_offset + 1] = window_rec[1] + i * chunk_size[1];
            chunk_vertex_data[vertex_data_offset + 2] = (GLfloat)counter;
            int chunk_pixel_offset = counter * chunk_width * chunk_height;
            compute_mandelbrot_chunk(&chunk_vertex_data[vertex_data_offset], chunk_size, chunk_width, chunk_height, &chunk_pixel_data[chunk_pixel_offset]);
            counter++;
        }
    }

    GLuint chunk_array_texture;
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &chunk_array_texture);
    glBindTexture(GL_TEXTURE_2D_ARRAY, chunk_array_texture);
    glTextureStorage3D(chunk_array_texture, 1, GL_R32F, chunk_width, chunk_height, chunk_count);
    glTextureSubImage3D(chunk_array_texture, 0, 0, 0, 0, chunk_width, chunk_height, chunk_count, GL_RED, GL_FLOAT, chunk_pixel_data);

    glTextureParameteri(chunk_array_texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(chunk_array_texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(chunk_array_texture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(chunk_array_texture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    /* glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, { 1.0f, 0.0f, 0.0f, 1.0f }); */
    // TODO: use a texture mipmap instead

    GLuint vertex_array;
    GLuint vertexbuffer;
    glGenVertexArrays(1, &vertex_array);
    glGenBuffers(1, &vertexbuffer);
    glBindVertexArray(vertex_array);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, chunk_count * sizeof(chunk_vertex_data[0]), chunk_vertex_data, GL_DYNAMIC_DRAW);
    GLuint shader_data_ubo;
    glGenBuffers(1, &shader_data_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, shader_data_ubo);
    glBufferData(GL_UNIFORM_BUFFER, 4 * sizeof(GLfloat) + 2 * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, shader_data_ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, 4 * sizeof(GLfloat), window_rec);
    glBufferSubData(GL_UNIFORM_BUFFER, 4 * sizeof(GLfloat), 2 * sizeof(GLfloat), chunk_size);

    // Chunk program config
    GLuint chunk_vertex_shader = create_shader_from_file(GL_VERTEX_SHADER, "shaders/chunk.vert");
    GLuint chunk_geometry_shader = create_shader_from_file(GL_GEOMETRY_SHADER, "shaders/chunk.geom");
    GLuint chunk_fragment_shader = create_shader_from_file(GL_FRAGMENT_SHADER, "shaders/chunk.frag");
    GLuint chunk_program = link_program(chunk_vertex_shader, chunk_geometry_shader, chunk_fragment_shader);
    glUseProgram(chunk_program);
    GLuint position_attribute = glGetAttribLocation(chunk_program, "position");
    glEnableVertexAttribArray(position_attribute);
    glVertexAttribPointer(position_attribute, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
    GLuint chunk_index_attribute = glGetAttribLocation(chunk_program, "chunk_index");
    glEnableVertexAttribArray(chunk_index_attribute);
    glVertexAttribPointer(chunk_index_attribute, 1, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));
    GLuint chunk_array_attribute = glGetUniformLocation(chunk_program, "chunk_array");
    glUniform1i(chunk_array_attribute, 0);
    GLuint window_rec_index = glGetUniformBlockIndex(chunk_program, "window_rec");
    glUniformBlockBinding(chunk_program, window_rec_index, 0);


    // Axis program config
    /* GLuint axis_vertex_shader = create_shader_from_file(GL_VERTEX_SHADER, "shaders/axis.vert"); */
    /* GLuint axis_fragment_shader = create_shader_from_file(GL_FRAGMENT_SHADER, "shaders/fragment.vert"); */
    /* GLuint chunk_program = link_program(axis_vertex_shader, 0, axis_fragment_shader); */
    /* glUseProgram(axis_program); */


    enum key_pressed_idx {
        KEY_QUIT,
        KEY_ZOOM_IN,
        KEY_ZOOM_OUT,
        MOUSE_BUTTON_LEFT,
        MOUSE_POSITION_X,
        MOUSE_POSITION_Y,
        VERTEX_RECALCULATE,
        KEY_ACTION_COUNT
    };
    int key_pressed[KEY_ACTION_COUNT] = { 0 };
    float mouse_posx, mouse_posy;
    int window_width, window_height;
    glfwGetFramebufferSize(window, &window_width, &window_height);
    int i = 0;
    // TODO: scroll input event
    while (!glfwWindowShouldClose(window)) {
        // Events handling
        glfwPollEvents();
        /* glfwWaitEvents(); */
        int curr_window_width, curr_window_height;
        glfwGetFramebufferSize(window, &curr_window_width, &curr_window_height);
        if (curr_window_width != window_width || curr_window_height != window_height) {
            window_rec[2] *= (float)curr_window_width / window_width;
            window_rec[3] *= (float)curr_window_height / window_height;
            window_width = curr_window_width;
            window_height = curr_window_height;
        }
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
            break;
        }
        if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS ||
            glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS) {
            if (!key_pressed[KEY_ZOOM_IN]) {
                key_pressed[KEY_ZOOM_IN] = 1;
                key_pressed[VERTEX_RECALCULATE] = 1;
                #define ZOOM_IN 0.2 // (1 out of 5)
                GLfloat dx = ZOOM_IN * window_rec[2];
                GLfloat dy = ZOOM_IN * window_rec[3];
                window_rec[0] += 0.5 * dx;
                window_rec[1] += 0.5 * dy;
                window_rec[2] -= dx;
                window_rec[3] -= dy;
                chunk_size[0] -= ZOOM_IN * chunk_size[0];
                chunk_size[1] -= ZOOM_IN * chunk_size[1];
                glBindBuffer(GL_UNIFORM_BUFFER, shader_data_ubo);
                glBufferSubData(GL_UNIFORM_BUFFER, 0, 4 * sizeof(GLfloat), window_rec);
                glBufferSubData(GL_UNIFORM_BUFFER, 4 * sizeof(GLfloat), 2 * sizeof(GLfloat), chunk_size);
            }
        } else {
            key_pressed[KEY_ZOOM_IN] = 0;
        }
        if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS ||
            glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS) {
            if (!key_pressed[KEY_ZOOM_OUT]) {
                key_pressed[KEY_ZOOM_OUT] = 1;
                key_pressed[VERTEX_RECALCULATE] = 1;
                #define ZOOM_OUT 0.25 // (1 out of 4)
                GLfloat dx = ZOOM_OUT * window_rec[2];
                GLfloat dy = ZOOM_OUT * window_rec[3];
                window_rec[0] -= 0.5 * dx;
                window_rec[1] -= 0.5 * dy;
                window_rec[2] += dx;
                window_rec[3] += dy;
                chunk_size[0] += ZOOM_OUT * chunk_size[0];
                chunk_size[1] += ZOOM_OUT * chunk_size[1];
                glBindBuffer(GL_UNIFORM_BUFFER, shader_data_ubo);
                glBufferSubData(GL_UNIFORM_BUFFER, 0, 4 * sizeof(GLfloat), window_rec);
                glBufferSubData(GL_UNIFORM_BUFFER, 4 * sizeof(GLfloat), 2 * sizeof(GLfloat), chunk_size);
            }
        } else {
            key_pressed[KEY_ZOOM_OUT] = 0;
        }
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            double posx, posy;
            glfwGetCursorPos(window, &posx, &posy);
            if (!key_pressed[MOUSE_BUTTON_LEFT]) {
                key_pressed[MOUSE_BUTTON_LEFT] = 1;
                // also, init MOUSE_POSITION_X and MOUSE_POSITION_Y
            } else {
                float dx = posx - mouse_posx;
                float dy = posy - mouse_posy;
                window_rec[0] -= dx / window_width * window_rec[2];
                window_rec[1] += dy / window_height * window_rec[3];
                glBindBuffer(GL_UNIFORM_BUFFER, shader_data_ubo);
                glBufferSubData(GL_UNIFORM_BUFFER, 0, 2 * sizeof(GLfloat), window_rec);
            }
            mouse_posx = posx;
            mouse_posy = posy;
        } else {
            key_pressed[MOUSE_BUTTON_LEFT] = 0;
        }
        if (key_pressed[VERTEX_RECALCULATE]) {
            key_pressed[VERTEX_RECALCULATE] = 0;
            int counter = 1;
            for (int i = 0; i < chunk_count_y + 1; i++) {
                for (int j = 0; j < chunk_count_x + 1; j++) {
                    int vertex_data_offset = (counter - 1) * chunk_vertex_len;
                    chunk_vertex_data[vertex_data_offset + 0] = window_rec[0] + j * chunk_size[0];
                    chunk_vertex_data[vertex_data_offset + 1] = window_rec[1] + i * chunk_size[1];
                    chunk_vertex_data[vertex_data_offset + 2] = (GLfloat)counter;
                    int chunk_pixel_offset = counter * chunk_width * chunk_height;
                    compute_mandelbrot_chunk(&chunk_vertex_data[vertex_data_offset], chunk_size, chunk_width, chunk_height, &chunk_pixel_data[chunk_pixel_offset]);
                    counter++;
                }
            }
            glTextureSubImage3D(chunk_array_texture, 0, 0, 0, 0, chunk_width, chunk_height, chunk_count, GL_RED, GL_FLOAT, chunk_pixel_data);
            glBufferData(GL_ARRAY_BUFFER, chunk_count * sizeof(chunk_vertex_data[0]), chunk_vertex_data, GL_DYNAMIC_DRAW);
        }


        printf("%lf, %lf, %lf, %lf;;; ", window_rec[0], window_rec[1], window_rec[2], window_rec[3]);
        printf("%lf, %lf", chunk_size[0], chunk_size[1]);
        printf("\n");

        // Draw
        glClearColor(0.0, 0.0, 0.5 * (1 + sin(i++ * 0.02)), 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        /* glDrawArrays(GL_POINTS, 0, 4); */
        glDrawArrays(GL_POINTS, 0, chunk_count);
        glfwSwapBuffers(window);
    }

    free(chunk_pixel_data);
    free(chunk_vertex_data);
    glDeleteTextures(1, &chunk_array_texture);

    glDeleteShader(chunk_vertex_shader);
    glDeleteShader(chunk_geometry_shader);
    glDeleteShader(chunk_fragment_shader);
    glDeleteProgram(chunk_program);
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteVertexArrays(1, &vertex_array);
    glDeleteBuffers(1, &shader_data_ubo);

    glfwDestroyWindow(window);
    glfwTerminate();

    fprintf(stderr, "Program exited successfully!\n");
    return 0;
}


GLuint create_shader(GLenum type, const char *code) {
    if (type == GL_VERTEX_SHADER) {
        fprintf(stderr, "Compiling vertex shader :\n");
    } else if (type == GL_GEOMETRY_SHADER) {
        fprintf(stderr, "Compiling geometry shader :\n");
    } else if (type == GL_FRAGMENT_SHADER) {
        fprintf(stderr, "Compiling fragment shader :\n");
    }
    // Create shader
    GLuint id = glCreateShader(type);
    glShaderSource(id, 1, &code, NULL);
    glCompileShader(id);
    // Check errors
    GLuint status = GL_FALSE;
    int length = 0;
    glGetShaderiv(id, GL_COMPILE_STATUS, &status);
    glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
    if (length > 0) {
        char *log = (char*)malloc(sizeof(char) * length + 1);
        glGetShaderInfoLog(id, length, NULL, log);
        fprintf(stderr, "Compiling shader error log : %s\n", log);
        fprintf(stderr, ": %s\n", code);
        free(log);
    }
    return id;
}


GLuint create_shader_from_file(GLenum type, const char *filename) {
    char *shader = 0;
    long length;
    FILE *f = fopen (filename, "rb");
    fprintf(stderr, "Loading shader from file :\n");
    if (f) {
        fseek (f, 0, SEEK_END);
        length = ftell(f);
        fseek (f, 0, SEEK_SET);
        shader = malloc(length + 1);
        shader[length] = '\0';
        if (shader) {
            fread(shader, 1, length, f);
        }
        fclose (f);
    }

    GLuint id = 0;
    if (shader) {
        id = create_shader(type, shader);
        free(shader);
    } else {
        fprintf(stderr, "Loading shader from file : some file error\n");
    }
    return id;
}

// If shader_id == 0, it is assumed that that shader is not used in the program
GLuint link_program(GLuint vertex_id, GLuint geometry_id, GLuint fragment_id) {
    // Link program
    GLuint program_id = glCreateProgram();
    fprintf(stderr, "Linking program :\n");
    if (vertex_id   != 0) glAttachShader(program_id, vertex_id);
    if (geometry_id != 0) glAttachShader(program_id, geometry_id);
    if (fragment_id != 0) glAttachShader(program_id, fragment_id);
    glLinkProgram(program_id);
    // Check errors
    GLuint status = GL_FALSE;
    int log_length = 0;
    glGetProgramiv(program_id, GL_LINK_STATUS, &status);
	glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &log_length);
	if (log_length > 0){
        char *log = (char*)malloc(sizeof(char) * log_length + 1);
		glGetProgramInfoLog(program_id, log_length, NULL, log);
        fprintf(stderr, "Linking program : %s\n", log);
        free(log);
	}
    // Cleanup
	if (vertex_id   != 0) glDetachShader(program_id, vertex_id);
	if (geometry_id != 0) glDetachShader(program_id, geometry_id);
	if (fragment_id != 0) glDetachShader(program_id, fragment_id);
	if (vertex_id   != 0) glDeleteShader(vertex_id);
	if (geometry_id != 0) glDeleteShader(geometry_id);
	if (fragment_id != 0) glDeleteShader(fragment_id);
    return program_id;
}


void compute_mandelbrot_chunk(const GLfloat pos[2], const GLfloat size[2], GLsizei width_px, GLsizei height_px, GLfloat *chunk) {
    GLfloat step_x = size[0] / width_px;
    GLfloat step_y = - size[1] / height_px; // reverse Y axis
    for (int i = 0; i < height_px; i++) {
        GLfloat y =  pos[1] + (i + 0.5) * step_y;
        for (int j = 0; j < width_px; j++) {
            GLfloat x =  pos[0] + (j + 0.5) * step_x; // 0.5 to center the integration
            chunk[i * width_px + j] = compute_mandelbrot(x, y);
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
