#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 800


static void glfw_error_callback(int e, const char *d) {
    fprintf(stderr, "Error %d: %s\n", e, d);
}

int main() {

    glfwSetErrorCallback(glfw_error_callback);

    if (!glfwInit()) {
        fprintf(stderr, "[GFLW] failed to init!\n");
        exit(1);
    }

    /* glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // do not use OpenGL */
    GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT,
                                    "GLFW + Vulkan demo", NULL, NULL);
    if (!window) {
        fprintf(stderr, "[GFLW] failed to init window!\n");
        glfwTerminate();
        exit(1);
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // enable vsync


    int idx = 0;
    int time_prev = glfwGetTime() + 1.0;
    int fps = 0;
#define FPS_DISPLAY()                               \
    {                                               \
        const int time_now = glfwGetTime();         \
        if (!(time_now > time_prev)) {              \
            fps++;                                  \
        } else {                                    \
            /* fprintf(stderr, "%d fps\n", fps); */ \
            time_prev = time_now + 1.0;             \
            fps = 0;                                \
        }                                           \
    }

    while (!glfwWindowShouldClose(window)) {
        idx++;
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glfwSwapBuffers(window);

        /* FPS_DISPLAY(); */
        /* fprintf(stderr, "%d: %d %d\n", idx, width, height); */

        /* glfwPollEvents(); */
        glfwWaitEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    fprintf(stderr, "Program exited successfully!\n");
    return 0;
}
