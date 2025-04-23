# Application to explore the Mandelbrot set

This is a GUI application, written in C, which allows user to explore fractal
patterns of the Mandelbrot set. The user can move around the complex plane, see
the fractal patterns, and zoom in/out to take a closer look at them.

### How to

The application uses GLFW and OpenGL for GUI.

The application is developed for Linux. It should work on windows, but
compatability issues with OpenGL may arise
[(see 'supports OpenGL')](https://www.glfw.org/docs/latest/quick.html).

To compile the program, run the `make` command.

To run a compiled program run the `exec` command.

### UI Controls

You can move around by using arrow keys, or by dragging a mouse cursor.

Press '+' or '-' to zoom in or out.

Press 'R' to refresh the image on the screen.

Press escape or 'Q' to exit the application.

### TODOs

The program can be further developed, and here are a couple of possible
directions:

- compute chunks in a separate thread/process;
- dynamically append new chunks as the user moves around;
- more comprehensive UI, showing current scale, coordinates, and user hints;
- further optimisation of API calls.

### Licence

[MIT](LICENSE)
