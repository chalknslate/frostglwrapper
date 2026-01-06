#ifndef WINDOW_H_
#define WINDOW_H_
    #include <stdio.h>
    #define GLFW_INCLUDE_NONE
    #include <GLFW/glfw3.h>
    #include <glad.h>
    struct Window {
        GLFWwindow* window;
        void(*makeContextCurrent)(struct Window* this);
        int(*getClose)(struct Window* this);
        void (*swapPoll)(struct Window* this);
        void (*destroy)(struct Window* this);
        int (*setClearColor)(struct Window* this, float r, float g, float b, float a);
        int (*clear)(struct Window* this, int attributes);
        int (*setViewport)(struct Window* this, int width, int height);
    };
    static int setViewport(struct Window* this, int width, int height) {
        glViewport(0, 0, width, height);
        return 0;
    }
    static int clear(struct Window* this, int attributes) {
        glClear(attributes);
        return 0;
    }
    static int setClearColor(struct Window* this, float r, float g, float b, float a) {
        glClearColor(r, g, b, a);
        return 0;
    }
    static void destroy(struct Window* this) {
        glfwDestroyWindow(this->window);
        glfwTerminate();
    }
    static int getClose(struct Window* this) {
        return glfwWindowShouldClose(this->window);
    }
    static void makeContextCurrent(struct Window* this) {
        glfwMakeContextCurrent(this->window);
        gladLoadGL();
    }
    static void swapPoll(struct Window* this) {
        glfwPollEvents();
        glfwSwapBuffers(this->window);
    }
    static struct Window newWindow(int width, int height, const char* title) {
        if (!glfwInit()) {
            fprintf(stderr, ("Failed to initialize GLFW"));
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwSwapInterval(1);
        return (struct Window) {
            .window = glfwCreateWindow(width, height, title, NULL, NULL),
            .makeContextCurrent = &makeContextCurrent,
            .getClose = &getClose,
            .swapPoll = &swapPoll,
            .destroy = &destroy,
            .clear = &clear,
            .setClearColor = &setClearColor,
            .setViewport = &setViewport,
        };
    }


     static const struct {
		struct Window (*new)(int width, int height, const char* title);
	} Window={ .new = &newWindow};

#endif