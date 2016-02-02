#include <bits/stdc++.h>
#include <sys/time.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

float width = 1280.0f;
float height = 720.0f;
float size = 7200.0f;
float gsize = 75.0f;
float psize = 5.0f;
float bsize = 5.0f;
float depth = 5.0f;
float bdepth = 10.0f;
float vz = 0.0f;
float zoom = 0.5f;
float pan = 0;
float g = 0.12;
int lives = 8;
bool mouse_drag = false;
bool drag_start = false;
int score = 0;
float drag_x;

int maze[10][10] = { 
    {1,1,1,0,1,1,1,0,0,1},
    {1,0,1,0,1,0,1,1,1,1},
    {1,1,1,0,1,0,0,0,1,1},
    {1,0,1,1,1,1,1,0,1,0},
    {1,1,1,0,1,0,1,1,1,1},
    {1,0,1,0,1,1,1,0,1,0},
    {0,1,1,0,1,0,1,0,1,1},
    {1,1,1,1,1,0,1,1,1,0},
    {1,0,1,0,1,1,1,0,1,0},
    {1,1,1,0,1,1,1,0,1,1}
};

typedef struct VAO {
    GLuint VertexArrayID;
    GLuint VertexBuffer;
    GLuint ColorBuffer;

    GLenum PrimitiveMode;
    GLenum FillMode;
    int NumVertices;

}VAO;

struct GLMatrices {
    glm::mat4 projection;
    glm::mat4 model;
    glm::mat4 view;
    GLuint MatrixID;
} Matrices;

GLuint programID;

typedef struct Cube{
    VAO *obj;
    float x,y,z;
    bool d;
}Cube;

typedef struct Flag{
    VAO *obj;
    float x,y,z;
    bool d;
}Flag;

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

    // Create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    // Read the Vertex Shader code from the file
    std::string VertexShaderCode;
    std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
    if(VertexShaderStream.is_open())
    {
        std::string Line = "";
        while(getline(VertexShaderStream, Line))
            VertexShaderCode += "\n" + Line;
        VertexShaderStream.close();
    }

    // Read the Fragment Shader code from the file
    std::string FragmentShaderCode;
    std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
    if(FragmentShaderStream.is_open()){
        std::string Line = "";
        while(getline(FragmentShaderStream, Line))
            FragmentShaderCode += "\n" + Line;
        FragmentShaderStream.close();
    }

    GLint Result = GL_FALSE;
    int InfoLogLength;

    // Compile Vertex Shader
    printf("Compiling shader : %s\n", vertex_file_path);
    char const * VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
    glCompileShader(VertexShaderID);

    // Check Vertex Shader
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> VertexShaderErrorMessage(InfoLogLength);
    glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
    fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

    // Compile Fragment Shader
    printf("Compiling shader : %s\n", fragment_file_path);
    char const * FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
    glCompileShader(FragmentShaderID);

    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
    glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
    fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

    // Link the program
    fprintf(stdout, "Linking program\n");
    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);

    // Check the program
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
    glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
    fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);

    return ProgramID;
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}

/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
    struct VAO* vao = new struct VAO;
    vao->PrimitiveMode = primitive_mode;
    vao->NumVertices = numVertices;
    vao->FillMode = fill_mode;

    // Create Vertex Array Object
    // Should be done after CreateWindow and before any other GL calls
    glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
    glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
    glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

    glBindVertexArray (vao->VertexArrayID); // Bind the VAO 
    glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
    glVertexAttribPointer(
            0,                  // attribute 0. Vertices
            3,                  // size (x,y,z)
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void*)0            // array buffer offset
            );

    glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
    glVertexAttribPointer(
            1,                  // attribute 1. Color
            3,                  // size (r,g,b)
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void*)0            // array buffer offset
            );

    return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
    GLfloat* color_buffer_data = new GLfloat [3*numVertices];
    for (int i=0; i<numVertices; i++) {
        color_buffer_data [3*i] = red;
        color_buffer_data [3*i + 1] = green;
        color_buffer_data [3*i + 2] = blue;
    }

    return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
    // Change the Fill Mode for this object
    glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

    // Bind the VAO to use
    glBindVertexArray (vao->VertexArrayID);

    // Enable Vertex Attribute 0 - 3d Vertices
    glEnableVertexAttribArray(0);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

    // Enable Vertex Attribute 1 - Color
    glEnableVertexAttribArray(1);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

    // Draw the geometry !
    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

/**************************
 * Customizable functions *
 **************************/

VAO *ground, *xaxis, *yaxis, *zaxis;
Cube block[10][10];
Cube player;
Flag flag;
float camera_rotation_angle = 90;
int view = 0;
/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_RELEASE) 
    {
        switch (key) 
        {
            default:
                break;
        }
    }
    else if (action == GLFW_PRESS || action == GLFW_REPEAT) 
    {
        switch (key) 
        {
            case GLFW_KEY_V:
                view = (view + 1)%3;
                zoom = 0.5f;
                vz = 0;
                break;
            case GLFW_KEY_UP:
                player.x -= 2*bsize;
                break;
            case GLFW_KEY_DOWN:
                player.x += 2*bsize;
                break;
            case GLFW_KEY_LEFT:
                player.z += 2*bsize;
                break;
            case GLFW_KEY_RIGHT:
                player.z -= 2*bsize;
                break;
            case GLFW_KEY_W:
                zoom -= 0.1;
                vz -= 1;
                if( zoom <= 0.1 )
                    zoom = 0.1;
                if(vz <= -100)
                    vz = -100;
                break;
            case GLFW_KEY_S:
                zoom += 0.1;
                vz += 1;
                if( zoom >= 1 )
                    zoom = 1;
                if(vz >= 50)
                    vz = 50;
                break;
            case GLFW_KEY_ESCAPE:
                quit(window);
                break;
            default:
                break;
        }
    }
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
    switch (key) {
        case 'Q':
        case 'q':
            quit(window);
            break;
        default:
            break;
    }
}

/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
    switch (button) {
        default:
            break;
    }
}

/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
    int fbwidth=width, fbheight=height;
    /* With Retina display on Mac OS X, GLFW's FramebufferSize
       is different from WindowSize */
    glfwGetFramebufferSize(window, &fbwidth, &fbheight);

    GLfloat fov = 120.0;

    // sets the viewport of openGL renderer
    glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

    // set the projection matrix as perspective
    /* glMatrixMode (GL_PROJECTION);
       glLoadIdentity ();
       gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
    // Store the projection matrix in a variable for future use
    // Perspective projection for 3D views
    // Ortho projection for 2D views

    if( view == 2) 
        Matrices.projection = glm::ortho(zoom*(-width/2) , (width/2)*zoom , zoom*(-height/2), (height/2)*zoom, 1.0f, 300.0f);
    else
        Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 1.0f, 300.0f);
}

void createGround  ()
{
    // GL3 accepts only Triangles. Quads are not supported
    static const GLfloat vertex_buffer_data [] = {
        -gsize, -depth, gsize,
        gsize, -depth, gsize,
        -gsize, depth, gsize,

        gsize, depth, gsize,
        -gsize, depth, gsize,
        gsize, -depth, gsize,

        -gsize, -depth, -gsize,
        gsize, -depth, -gsize,
        -gsize, depth, -gsize,

        gsize, depth, -gsize,
        -gsize, depth, -gsize,
        gsize, -depth, -gsize,

        -gsize, depth, -gsize,
        gsize, depth, -gsize,
        -gsize, depth, gsize,

        gsize, depth, gsize,
        -gsize, depth, gsize,
        gsize, depth, -gsize,

        -gsize, -depth, -gsize,
        gsize, -depth, -gsize,
        -gsize, -depth, gsize,

        gsize, -depth, gsize,
        -gsize, -depth, gsize,
        gsize, -depth, -gsize,

        gsize, -depth, -gsize,
        gsize, depth, -gsize,
        gsize, -depth, gsize,

        gsize, depth, gsize,
        gsize, -depth, gsize,
        gsize, depth, -gsize,

        -gsize, -depth, -gsize,
        -gsize, depth, -gsize,
        -gsize, -depth, gsize,

        -gsize, depth, gsize,
        -gsize, -depth, gsize,
        -gsize, depth, -gsize
    };

    static const GLfloat color_buffer_data [] = {
        0,0.4,0,
        0,0.4,0,
        0,0.4,0,

        0,0.4,0,
        0,0.4,0,
        0,0.4,0,

        0,0.4,0,
        0,0.4,0,
        0,0.4,0,

        0,0.4,0,
        0,0.4,0,
        0,0.4,0,

        0.5,0.5,1,
        0.5,0.5,1,
        0.5,0.5,1,

        0.5,0.5,1,
        0.5,0.5,1,
        0.5,0.5,1,

        0.5,0.5,1,
        0.5,0.5,1,
        0.5,0.5,1,

        0.5,0.5,1,
        0.5,0.5,1,
        0.5,0.5,1,


        0,0.4,0,
        0,0.4,0,
        0,0.4,0,

        0,0.4,0,
        0,0.4,0,
        0,0.4,0,

        0,0.4,0,
        0,0.4,0,
        0,0.4,0,

        0,0.4,0,
        0,0.4,0,
        0,0.4,0,
    };

    // create3DObject creates and returns a handle to a VAO that can be used later
    ground = create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createPlayer  ()
{
    static const GLfloat vertex_buffer_data [] = {
        -psize, -psize, psize,
        psize, -psize, psize,
        -psize, psize, psize,

        psize, psize, psize,
        -psize, psize, psize,
        psize, -psize, psize,

        -psize, -psize, -psize,
        psize, -psize, -psize,
        -psize, psize, -psize,

        psize, psize, -psize,
        -psize, psize, -psize,
        psize, -psize, -psize,

        -psize, psize, -psize,
        psize, psize, -psize,
        -psize, psize, psize,

        psize, psize, psize,
        -psize, psize, psize,
        psize, psize, -psize,

        -psize, -psize, -psize,
        psize, -psize, -psize,
        -psize, -psize, psize,

        psize, -psize, psize,
        -psize, -psize, psize,
        psize, -psize, -psize,

        psize, -psize, -psize,
        psize, psize, -psize,
        psize, -psize, psize,

        psize, psize, psize,
        psize, -psize, psize,
        psize, psize, -psize,

        -psize, -psize, -psize,
        -psize, psize, -psize,
        -psize, -psize, psize,

        -psize, psize, psize,
        -psize, -psize, psize,
        -psize, psize, -psize
    };

    static const GLfloat color_buffer_data [] = {
        1,1,1, 
        1,1,1, 
        1,1,1, 
        1,1,1, 
        1,1,1, 
        1,1,1, 
        1,1,1, 
        1,1,1, 
        1,1,1, 
        1,1,1, 
        1,1,1, 
        1,1,1, 
        1,1,1, 
        1,1,1, 
        1,1,1, 
        1,1,1, 
        1,1,1, 
        1,1,1, 
        1,1,1, 
        1,1,1, 
        1,1,1, 
        1,1,1, 
        1,1,1, 
        1,1,1, 
        1,1,1, 
        1,1,1, 
        1,1,1, 
        1,1,1, 
        1,1,1, 
        1,1,1, 
        1,1,1, 
        1,1,1, 
        1,1,1, 
        1,1,1, 
        1,1,1, 
        1,1,1 
    };

    player.x = 9*bsize;
    player.z = -9*bsize;
    player.y = depth+2*bdepth+psize;
    player.d = true;
    player.obj = create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createBlock  ()
{
    for( int i = 0 ; i < 10 ; i++)
    {
        for( int j = 0 ; j < 10 ; j++)
        {
            static const GLfloat vertex_buffer_data [] = {
                -bsize, -bdepth, bsize,
                bsize, -bdepth, bsize,
                -bsize, bdepth, bsize,

                bsize, bdepth, bsize,
                -bsize, bdepth, bsize,
                bsize, -bdepth, bsize,

                -bsize, -bdepth, -bsize,
                bsize, -bdepth, -bsize,
                -bsize, bdepth, -bsize,

                bsize, bdepth, -bsize,
                -bsize, bdepth, -bsize,
                bsize, -bdepth, -bsize,

                -bsize, bdepth, -bsize,
                bsize, bdepth, -bsize,
                -bsize, bdepth, bsize,

                bsize, bdepth, bsize,
                -bsize, bdepth, bsize,
                bsize, bdepth, -bsize,

                -bsize, -bdepth, -bsize,
                bsize, -bdepth, -bsize,
                -bsize, -bdepth, bsize,

                bsize, -bdepth, bsize,
                -bsize, -bdepth, bsize,
                bsize, -bdepth, -bsize,

                bsize, -bdepth, -bsize,
                bsize, bdepth, -bsize,
                bsize, -bdepth, bsize,

                bsize, bdepth, bsize,
                bsize, -bdepth, bsize,
                bsize, bdepth, -bsize,

                -bsize, -bdepth, -bsize,
                -bsize, bdepth, -bsize,
                -bsize, -bdepth, bsize,

                -bsize, bdepth, bsize,
                -bsize, -bdepth, bsize,
                -bsize, bdepth, -bsize
            };

            static const GLfloat color_buffer_data [] = {
                (float)139/255,(float)69/255,(float)19/255,
                (float)139/255,(float)69/255,(float)19/255,
                (float)139/255,(float)69/255,(float)19/255,

                (float)139/255,(float)69/255,(float)19/255,
                (float)139/255,(float)69/255,(float)19/255,
                (float)139/255,(float)69/255,(float)19/255,

                (float)139/255,(float)69/255,(float)19/255,
                (float)139/255,(float)69/255,(float)19/255,
                (float)139/255,(float)69/255,(float)19/255,

                (float)139/255,(float)69/255,(float)19/255,
                (float)139/255,(float)69/255,(float)19/255,
                (float)139/255,(float)69/255,(float)19/255,

                0,0.4,0,
                0,0.4,0,
                0,0.4,0,

                0,0.4,0,
                0,0.4,0,
                0,0.4,0,

                0,0.4,0,
                0,0.4,0,
                0,0.4,0,

                0,0.4,0,
                0,0.4,0,
                0,0.4,0,

                (float)139/255,(float)69/255,(float)19/255,
                (float)139/255,(float)69/255,(float)19/255,
                (float)139/255,(float)69/255,(float)19/255,

                (float)139/255,(float)69/255,(float)19/255,
                (float)139/255,(float)69/255,(float)19/255,
                (float)139/255,(float)69/255,(float)19/255,

                (float)139/255,(float)69/255,(float)19/255,
                (float)139/255,(float)69/255,(float)19/255,
                (float)139/255,(float)69/255,(float)19/255,

                (float)139/255,(float)69/255,(float)19/255,
                (float)139/255,(float)69/255,(float)19/255,
                (float)139/255,(float)69/255,(float)19/255,
            };

            block[i][j].x = (float)(2*i-9)*bsize;
            block[i][j].y = depth+bdepth;
            block[i][j].z = (float)(2*j-9)*bsize;
            block[i][j].obj = create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);
        }
    }
}

void createFlag  ()
{
    static const GLfloat vertex_buffer_data [] = {
        -1, -bdepth, 1,
        1, -bdepth, 1,
        -1, bdepth, 1,

        1, bdepth, 1,
        -1, bdepth, 1,
        1, -bdepth, 1,

        -1, -bdepth, -1,
        1, -bdepth, -1,
        -1, bdepth, -1,

        1, bdepth, -1,
        -1, bdepth, -1,
        1, -bdepth, -1,

        -1, bdepth, -1,
        1, bdepth, -1,
        -1, bdepth, 1,

        1, bdepth, 1,
        -1, bdepth, 1,
        1, bdepth, -1,

        -1, -bdepth, -1,
        1, -bdepth, -1,
        -1, -bdepth, 1,

        1, -bdepth, 1,
        -1, -bdepth, 1,
        1, -bdepth, -1,

        1, -bdepth, -1,
        1, bdepth, -1,
        1, -bdepth, 1,

        1, bdepth, 1,
        1, -bdepth, 1,
        1, bdepth, -1,

        -1, -bdepth, -1,
        -1, bdepth, -1,
        -1, -bdepth, 1,

        -1, bdepth, 1,
        -1, -bdepth, 1,
        -1, bdepth, -1,

        0,0,0,
        0,bdepth,0,
        0,bdepth/2,bdepth/2,
    };

    static const GLfloat color_buffer_data [] = {
        1,0,0,
        1,0,0,
        1,0,0,
        1,0,0,
        1,0,0,
        1,0,0,
        1,0,0,
        1,0,0,
        1,0,0,
        1,0,0,
        1,0,0,
        1,0,0,
        1,0,0,
        1,0,0,
        1,0,0,
        1,0,0,
        1,0,0,
        1,0,0,
        1,0,0,
        1,0,0,
        1,0,0,
        1,0,0,
        1,0,0,
        1,0,0,
        1,0,0,
        1,0,0,
        1,0,0,
        1,0,0,
        1,0,0,
        1,0,0,
        1,0,0,
        1,0,0,
        1,0,0,
        1,0,0,
        1,0,0,
        1,0,0,
        1,0,0,
        1,0,0,
        1,0,0
    };

    flag.x = -9*bsize;
    flag.z = 9*bsize;
    flag.y = depth+2*bdepth+psize;
    flag.d = true;
    flag.obj = create3DObject(GL_TRIANGLES, 39, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createXAxis  ()
{
    // GL3 accepts only Triangles. Quads are not supported
    static const GLfloat vertex_buffer_data [] = {
        -size/2, 0, 0, // vertex 1
        size/2, 0, 0 // vertex 2
    };

    static const GLfloat color_buffer_data [] = {
        1,0,0, // color 1
        1,0,0, // color 2
    };

    // create3DObject creates and returns a handle to a VAO that can be used later
    xaxis = create3DObject(GL_LINES, 2, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createYAxis  ()
{
    static const GLfloat vertex_buffer_data [] = {
        0, -size/2, 0,
        0, size/2, 0
    };

    static const GLfloat color_buffer_data [] = {
        0,1,0, // color 1
        0,1,0, // color 2
    };

    // create3DObject creates and returns a handle to a VAO that can be used later
    yaxis = create3DObject(GL_LINES, 2, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createZAxis  ()
{
    // GL3 accepts only Triangles. Quads are not supported
    static const GLfloat vertex_buffer_data [] = {
        0, 0, -size/2,
        0, 0, size/2
    };

    static const GLfloat color_buffer_data [] = {
        0,0,1, // color 1
        0,0,1, // color 2
    };

    // create3DObject creates and returns a handle to a VAO that can be used later
    zaxis = create3DObject(GL_LINES, 2, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void checkBlock ()
{
    int i =  (player.x/bsize + 9)/2;
    int j =  (player.z/bsize + 9)/2;
    if( !maze[i][j] || i<0 || j<0 || i>=10 || j>=10)
    {
        player.x = 9*bsize;
        player.z = -9*bsize;
    }
}

void checkFlag ()
{
        if(player.x == flag.x && player.z == flag.z)
            flag.d = false;
}

/* Render the scene with openGL */
/* Edit this function according to your assignment */
void draw ( GLFWwindow* window)
{
    reshapeWindow (window, width, height);
    // clear the color and depth in the frame buffer
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // use the loaded shader program
    // Don't change unless you know what you are doing
    glUseProgram (programID);

    // Eye - Location of camera. Don't change unless you are sure!!
    glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
    // Target - Where is the camera looking at.  Don't change unless you are sure!!
    glm::vec3 target (0, 0, 0);
    // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
    glm::vec3 up (0, 1, 0);

    // Compute Camera matrix (view)
    // Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
    //  Don't change unless you are sure!!
    if(view == 0)
        Matrices.view = glm::lookAt(glm::vec3(100,150+vz,-100), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane
    if(view == 1)
        Matrices.view = glm::lookAt(glm::vec3(0,0,200), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane
    if(view == 2)
        Matrices.view = glm::lookAt(glm::vec3(0.0001f,100,-0.0001f), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

    // Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
    //  Don't change unless you are sure!!
    glm::mat4 VP = Matrices.projection * Matrices.view;

    // Send our transformation to the currently bound shader, in the "MVP" uniform
    // For each model you render, since the MVP will be different (at least the M part)
    //  Don't change unless you are sure!!
    glm::mat4 MVP;	// MVP = Projection * View * Model

    /* Render your scene */

    // Pop matrix to undo transformations till last push matrix instead of recomputing model matrix
    // glPopMatrix ();

    /* Axes */
    Matrices.model = glm::mat4(1.0f);
    MVP = VP;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(xaxis);
    Matrices.model = glm::mat4(1.0f);
    MVP = VP;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(yaxis);
    Matrices.model = glm::mat4(1.0f);
    MVP = VP;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(zaxis);

    /* Ground */ 
    Matrices.model = glm::mat4(1.0f);
    MVP = VP;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(ground);

    for(int i = 0 ; i < 10 ; i++)
    {
        for(int j = 0 ; j < 10 ; j++)
        {
            if( maze[i][j] )
            {
                Matrices.model = glm::mat4(1.0f);
                glm::mat4 translateBlock = glm::translate (glm::vec3(block[i][j].x,block[i][j].y,block[i][j].z));
                Matrices.model *= translateBlock;
                MVP = VP * Matrices.model;
                glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
                draw3DObject(block[i][j].obj);
            }
        }
    }

    if( player.d)
    {
        Matrices.model = glm::mat4(1.0f);
        glm::mat4 translatePlayer = glm::translate (glm::vec3(player.x, player.y, player.z));
        Matrices.model *= translatePlayer;
        MVP = VP * Matrices.model;
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        draw3DObject(player.obj);

        checkBlock();
    }
    
    if( flag.d )
    {
        Matrices.model = glm::mat4(1.0f);
        glm::mat4 translateFlag = glm::translate (glm::vec3(flag.x, flag.y, flag.z));
        Matrices.model *= translateFlag;
        MVP = VP * Matrices.model;
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        draw3DObject(flag.obj);
        checkFlag();
    }
    //camera_rotation_angle++; // Simulating camera rotation

}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
    GLFWwindow* window; // window desciptor/handle

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval( 1 );

    /* --- register callbacks with GLFW --- */

    /* Register function to handle window resizes */
    /* With Retina display on Mac OS X GLFW's FramebufferSize
       is different from WindowSize */
    glfwSetFramebufferSizeCallback(window, reshapeWindow);
    glfwSetWindowSizeCallback(window, reshapeWindow);

    /* Register function to handle window close */
    glfwSetWindowCloseCallback(window, quit);

    /* Register function to handle keyboard input */
    glfwSetKeyCallback(window, keyboard);      // general keyboard input
    glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

    /* Register function to handle mouse click */
    glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks

    return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
    /* Objects should be created before any other gl function and shaders */
    // Create the models
    // Generate the VAO, VBOs, vertices data & copy into the array buffer
    createXAxis();
    createYAxis();
    createZAxis();
    createBlock();
    createGround();
    createPlayer();
    createFlag();

    // Create and compile our GLSL program from the shaders
    programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
    // Get a handle for our "MVP" uniform
    Matrices.MatrixID = glGetUniformLocation(programID, "MVP");

    reshapeWindow (window, width, height);

    // Background color of the scene
    glClearColor ((float)0/255, (float)31/255, (float)63/255, 0.0f); // R, G, B, A
    glClearDepth (1.0f);

    glEnable (GL_DEPTH_TEST);
    glDepthFunc (GL_LEQUAL);

    cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
    cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
    cout << "VERSION: " << glGetString(GL_VERSION) << endl;
    cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

int main (int argc, char** argv)
{
    GLFWwindow* window = initGLFW(width, height);

    initGL (window, width, height);

    double last_update_time = glfwGetTime(), current_time;

    /* Draw in loop */
    while (!glfwWindowShouldClose(window)) {

        // OpenGL Draw commands
        draw(window);

        // Swap Frame Buffer in double buffering
        glfwSwapBuffers(window);

        // Poll for Keyboard and mouse events
        glfwPollEvents();

        // Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
        current_time = glfwGetTime(); // Time in seconds
        if ((current_time - last_update_time) >= 0.5) { // atleast 0.5s elapsed since last frame
            // do something every 0.5 seconds ..
            last_update_time = current_time;
        }
    }

    glfwTerminate();
    exit(EXIT_SUCCESS);
}
