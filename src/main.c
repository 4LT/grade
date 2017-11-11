#include <stdlib.h>
#include <stddef.h>

#define NO_SDL_GLEXT
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <SDL/SDL_opengl.h>
#include <GL/glu.h>

static GLuint progId;
static GLint vertPos2dLoc = -1;
static GLint vertColorLoc = -1;
static GLuint vbo = 0;
static GLuint vcbo = 0;
static GLuint ibo = 0;
static GLuint vao = 0;

int main(int argc, char *argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Failed to init SDL: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_Window *window;
    SDL_GLContext ctx;

    /* Use OpenGL 3.1 core */
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(
            SDL_GL_CONTEXT_PROFILE_MASK,
            SDL_GL_CONTEXT_PROFILE_CORE);

    window = SDL_CreateWindow("grade",
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            640, 480,
            SDL_WINDOW_OPENGL);

    if (!window) {
        fprintf(stderr, "Failed to create window: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    ctx = SDL_GL_CreateContext(window);
    if (!ctx) {
        fprintf(stderr, "Failed to create GL context: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    glewExperimental = GL_TRUE; 
    GLenum glewErr = glewInit();
    if (glewErr != GLEW_OK) {
        fprintf(stderr, "Failed to init GLEW: %s\n",
                glewGetErrorString(glewErr));
    }

    puts((const char *)glGetString(GL_VERSION));
    puts((const char *)glGetString(GL_SHADING_LANGUAGE_VERSION));

    progId = glCreateProgram();
    char log[200] = "";
    int logLen;

    GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
    const GLchar* vertexShaderSource[] = {
        "#version 140\n"
        "in vec2 vertPos; "
        "in vec3 vertColor; "
        "out vec4 vsColor; "
        "void main() { "
            "gl_Position = vec4( vertPos.x, vertPos.y, 0, 1 ); "
            "vsColor.xyz = vertColor; "
            "vsColor.w = 1; "
        "}"
    };

    const GLchar* fragShaderSource[] = {
        "#version 140\n"
        "in vec4 vsColor; "
        "out vec4 color; "
        "void main() { "
            "color = vsColor; "
        "}"
    };

    glShaderSource(vertShader, 1, vertexShaderSource, (void *)0);
    glCompileShader(vertShader);
    GLint wasShaderCompiled = GL_FALSE;
    glGetShaderiv(vertShader, GL_COMPILE_STATUS, &wasShaderCompiled);

    if (wasShaderCompiled != GL_TRUE) {
        fprintf(stderr, "Failed to compile vertex shader\n");
        glGetShaderInfoLog(vertShader, 200, &logLen, log);
        GLenum err = glGetError();
        switch (err) {
            case GL_INVALID_VALUE:
                fprintf(stderr, "Bad shader index\n");
                break;
            case GL_INVALID_OPERATION:
                fprintf(stderr, "Not a shader object\n");
                break;
            default:
                fprintf(stderr, "%s\n", log);
        }
        return EXIT_FAILURE;
    }

    glAttachShader(progId, vertShader);

    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragShader, 1, fragShaderSource, (void *)0);
    glCompileShader(fragShader);
    wasShaderCompiled = GL_FALSE;
    glGetShaderiv(fragShader, GL_COMPILE_STATUS, &wasShaderCompiled);
    
    if (wasShaderCompiled != GL_TRUE) {
        fprintf(stderr, "Failed to compile fragment shader\n");
        glGetShaderInfoLog(fragShader, 80, &logLen, log);
        fprintf(stderr, "%s\n", log);
        return EXIT_FAILURE;
    }

    glAttachShader(progId, fragShader);
    glLinkProgram(progId);

    GLint wasProgramLinked = GL_FALSE;
    glGetProgramiv(progId, GL_LINK_STATUS, &wasProgramLinked);

    if (wasProgramLinked != GL_TRUE) {
        fprintf(stderr, "Failed to link shader program\n");
        glGetProgramInfoLog(progId, 80, &logLen, log);
        fprintf(stderr, "%s\n", log);
        return EXIT_FAILURE;
    }

    vertPos2dLoc = glGetAttribLocation(progId, "vertPos");
    vertColorLoc = glGetAttribLocation(progId, "vertColor");

    GLfloat verts[] = {
        -0.5f, -0.5f,
         0.5f, -0.5f,
         0.5f,  0.5f,
        -0.5f,  0.5f,
    };

    GLfloat colors[] = {
        1, 0, 0,
        1, 1, 0,
        0, 1, 0,
        0, 0, 1,
    };

    GLint indices[] = { 0, 1, 2, 3 };

    glClearColor(0.5f, 0.5f, 0.5f, 0.5f);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER,
            4 * 2 * sizeof(GLfloat), verts, GL_STATIC_DRAW);

    glGenBuffers(1, &vcbo);
    glBindBuffer(GL_ARRAY_BUFFER, vcbo);
    glBufferData(GL_ARRAY_BUFFER,
            4 * 3 * sizeof(GLfloat), colors, GL_STATIC_DRAW);

    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
            4 * sizeof(GLint), indices, GL_STATIC_DRAW);

    glGenVertexArrays(1, &vao);

    /* Use VAO */
    glBindVertexArray(vao);

    glEnableVertexAttribArray(vertPos2dLoc);
    glEnableVertexAttribArray(vertColorLoc);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(vertPos2dLoc, 2, GL_FLOAT, GL_FALSE,
            2 * sizeof(GLfloat), (void *)0);
    GLenum err = glGetError();
    glBindBuffer(GL_ARRAY_BUFFER, vcbo);
    glVertexAttribPointer(vertColorLoc, 3, GL_FLOAT, GL_FALSE,
            3 * sizeof(GLfloat), (void *)0);
    GLenum err2 = glGetError();


    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

    glBindVertexArray(0);
    /* End use of VAO */
    
    /* render */
    glUseProgram(progId);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindVertexArray(vao);
    glDrawElements( GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT, (void *)0 );

    glDisableVertexAttribArray(vertColorLoc);
    glDisableVertexAttribArray(vertPos2dLoc);

    glUseProgram(0);

    if (err != GL_NO_ERROR) {
        printf("%d\n", err);
    }

    if (err2 != GL_NO_ERROR) {
        printf("%d\n", err2);
    }

    SDL_GL_SwapWindow(window);
    SDL_Delay(2000);

    SDL_GL_DeleteContext(ctx);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return EXIT_SUCCESS;
}
