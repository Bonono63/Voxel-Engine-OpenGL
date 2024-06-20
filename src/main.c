#include <stdint.h>
#include <stdio.h>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <cglm/struct.h>
#include <io.h>
#include <render.h>

typedef struct Voxel
{
    uint8_t type;
    //binary flags indicating voxel details
    uint8_t flags;
} Voxel;

typedef struct Chunk
{
    // 32*32*32 one dimensional array because it is quick despite it's resource inefficiency
    struct Voxel data[32768];
} Chunk;

int main ()
{
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    uint32_t height=600,width=800;
    GLFWwindow* window;

    if (!glfwInit())
    {
        printf("Unable to initialize glfw.\n");
        return -1;
    }

    window = glfwCreateWindow(width, height, "V Project", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        printf("Unable to open a window context.\n");
        return -1;
    }

    glfwMakeContextCurrent(window);
    gladLoadGL(glfwGetProcAddress);

    glfwSetFramebufferSizeCallback(window, window_resize_callback);


    struct Chunk chunk;
    for ( int i = 0 ; i < 32768 ; i++)
    {
        struct Voxel v;
        v.type = rand() % 2;
        chunk.data[i] = v;
    }

    // For now allocates the maximum amount of space required for our chunk
    float *mesh_data = (float*) calloc(0, 0);

    int voxel_count = 0;
    for (int i = 0 ; i < 32768 ; i++)
    {
        int id = chunk.data->type+i;
        if (id == 1)
        {
            voxel_count++;
            int x = i % 32;
            int y = i / 32 / 32 % 32;
            int z = i / 32 % 32;
            
            // arbitrary size of mesh, 12 vertices per voxel, 3 floats per vertex, 4 bytes per float
            mesh_data = realloc(mesh_data, voxel_count * 12 * 3 * sizeof(float));

            // top face

        }
    }

    struct Mesh chunk_mesh;

    chunk_mesh = create_mesh(mesh_data, sizeof(mesh_data)*sizeof(float), "resources/chunk.glsl", "resources/chunk_fragment.glsl");

    //Test triangle
    float vbo_data[] = {
        //pos color
         1.0f, 1.0f,0.0,
        -1.0f, 1.0f,0.0,
        -1.0f,-1.0f,0.0,
        
        -1.0f,-1.0f,0.0,
         1.0f,-1.0f,0.0,
         1.0f, 1.0f,0.0
    };

    struct Mesh cube;

    cube = create_mesh(vbo_data, sizeof(vbo_data)*sizeof(float), "resources/vertex.glsl", "resources/fragment.glsl");

    glEnable(GL_CULL_FACE);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    struct Camera camera = {.fov = 70, .speed = 4, .sensitivity = 0.5};
    camera.position[0] = 0;
    camera.position[1] = 0;
    camera.position[2] = 0;

    glfwGetWindowSize(window, &width, &height);
    float aspect = width/height;
    glm_perspective(camera.fov, aspect, 0.001, 1000.0, camera.projection);

    while(!glfwWindowShouldClose(window))
    {
        glClearColor(0.1f,0.1f,0.1f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        camera_process(&camera);

        render_mesh(&cube, &camera);

        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
