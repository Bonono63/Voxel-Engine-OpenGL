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

float frame_delta = 0.0f;
double last_x, last_y;
bool first_mouse = true;
struct Camera camera = {
    .fov = 70.0f,
    .speed = 4.0f,
    .sensitivity = 0.125f,
    .position = {0.0f,0.0f,-3.0f},
    .direction = {0.0f,0.0f,-1.0f},
    .up = {0.0f,1.0f,0.0f},
    .pitch = 0.0f,
    .yaw = 0.0f,
    .roll = 0.0f
};
void cursor_position_callback(GLFWwindow* window, double x, double y);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

bool w=false, a=false, s=false, d=false, shift=false, space=false;
void input_process();

int main ()
{
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    int32_t height=600,width=800;
    GLFWwindow* window;

    if (!glfwInit())
    {
        printf("Unable to initialize glfw.\n");
        return -1;
    }

    window = glfwCreateWindow(width, height, "Voyager", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        printf("Unable to open a window context.\n");
        return -1;
    }

    glfwMakeContextCurrent(window);
    gladLoadGL(glfwGetProcAddress);

    glfwSetFramebufferSizeCallback(window, window_resize_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetKeyCallback(window, key_callback);

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
            //*(mesh_data+(voxel_count * 36)) = 
        }
    }

    //struct Mesh chunk_mesh;

    //chunk_mesh = create_mesh(mesh_data, sizeof(mesh_data)*sizeof(float), "resources/chunk.glsl", "resources/chunk_fragment.glsl");

    //Test triangle
    float vbo_data[] = {
        //pos color
         1.0f, 1.0f,1.0f,
        -1.0f, 1.0f,1.0f,
        -1.0f,-1.0f,1.0f,
        
        -1.0f,-1.0f,1.0f,
         1.0f,-1.0f,1.0f,
         1.0f, 1.0f,1.0f
    };

    struct Mesh cube;

    cube = create_mesh(vbo_data, sizeof(vbo_data)*sizeof(float), "resources/vertex.glsl", "resources/fragment.glsl");

    //glEnable(GL_CULL_FACE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // initialize camera view matrix
    glm_mat4_identity(camera.view);

    glfwGetWindowSize(window, &width, &height);
    float aspect = width/height;
    glm_perspective(camera.fov, aspect, 0.001f, 1000.0f, camera.projection);

    // initialize object variables
    glm_mat4_identity(cube.object_transform);
    vec3 transform = {0.0f,0.0f,1.0f};
    glm_translate(cube.object_transform, transform);

    // set to zero for no vsync
    glfwSwapInterval(1);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    float prev_frame_time = 0.0f;
    // render loop
    while(!glfwWindowShouldClose(window))
    {
        glClearColor(0.1f,0.1f,0.1f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        float current_frame_time = glfwGetTime();
        frame_delta = current_frame_time - prev_frame_time;
        prev_frame_time = current_frame_time;

        input_process();

        camera_process(&camera);

        render_mesh_camera(&cube, &camera);

        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void cursor_position_callback(GLFWwindow *window, double x, double y)
{
    if (first_mouse)
    {
        last_x = x;
        last_y = y;
        first_mouse = false;
    }

    float x_offset = x - last_x;
    float y_offset = y - last_y;
    last_x = x;
    last_y = y;

    x_offset *= camera.sensitivity;
    y_offset *= camera.sensitivity;

    camera.yaw += x_offset;
    camera.pitch -= y_offset;

    if (camera.pitch > 89.0f)
        camera.pitch = 89.0f;
    if (camera.pitch < -89.0f)
        camera.pitch = -89.0f;

    if (camera.yaw > 360)
        camera.yaw = 0;
    if (camera.yaw < 0)
        camera.yaw += 360;

    camera.direction[0] = cos(glm_rad(camera.yaw)) * cos(glm_rad(camera.pitch));
    camera.direction[1] = sin(glm_rad(camera.pitch));
    camera.direction[2] = sin(glm_rad(camera.yaw)) * cos(glm_rad(camera.pitch));
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (key == GLFW_KEY_W && action == GLFW_PRESS)
        w = true;
    if (key == GLFW_KEY_W && action == GLFW_RELEASE)
        w = false;

    if (key == GLFW_KEY_A && action == GLFW_PRESS)
        a = true;
    if (key == GLFW_KEY_A && action == GLFW_RELEASE)
        a = false;

    if (key == GLFW_KEY_S && action == GLFW_PRESS)
        s = true;
    if (key == GLFW_KEY_S && action == GLFW_RELEASE)
        s = false;
    
    if (key == GLFW_KEY_D && action == GLFW_PRESS)
        d = true;
    if (key == GLFW_KEY_D && action == GLFW_RELEASE)
        d = false;
}

void input_process()
{
    if (w)
    {
        vec3 transform;
        glm_vec3_copy(camera.direction, transform);
        glm_normalize(transform);
        glm_vec3_scale(transform, frame_delta, transform);
        glm_vec3_scale(transform, camera.speed, transform);
        glm_vec3_add(camera.position, transform, camera.position);
    }
    
    if (s)
    {
        vec3 transform;
        glm_vec3_copy(camera.direction, transform);
        glm_normalize(transform);
        glm_vec3_scale(transform, frame_delta, transform);
        glm_vec3_scale(transform, camera.speed, transform);
        glm_vec3_sub(camera.position, transform, camera.position);
    }

    if (a)
    {
        vec3 transform;
        // right vector?
        glm_vec3_crossn(camera.direction, camera.up, transform);
        glm_vec3_scale(transform, frame_delta, transform);
        glm_vec3_scale(transform, camera.speed, transform);
        glm_vec3_sub(camera.position, transform, camera.position);
    }
    
    if (d)
    {
        vec3 transform;
        // right vector?
        glm_vec3_crossn(camera.direction, camera.up, transform);
        glm_vec3_scale(transform, frame_delta, transform);
        glm_vec3_scale(transform, camera.speed, transform);
        glm_vec3_add(camera.position, transform, camera.position);
    }
}
