#include <stdint.h>
#include <stdio.h>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <cglm/struct.h>
#include <io.h>
#include <render.h>
#include <voxel.h>

float frame_delta = 0.0f;
double last_x, last_y;
bool first_mouse = true;

void cursor_position_callback(GLFWwindow* window, double x, double y);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

bool w=false, a=false, s=false, d=false, shift=false, space=false, wire_frame=false;
void input_process();

struct Camera camera = {
    .fov = 70.0f,
    .speed = 4.0f,
    .sensitivity = 0.15f,
    .position = {0.0f,0.0f,-3.0f},
    .direction = {0.0f,0.0f,-1.0f},
    .up = {0.0f,1.0f,0.0f},
    .pitch = 0.0f,
    .yaw = 0.0f,
    .roll = 0.0f
};

int main (void)
{
    int error = 0;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    int32_t height=900,width=900;
    GLFWwindow* window;

    if (!glfwInit())
    {
        printf("Unable to initialize glfw.\n");
        return -1;
    }

    window = glfwCreateWindow(width, height, "Voyager [OpenGL]", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        printf("Unable to open a window context.\n");
        return -1;
    }

    glfwMakeContextCurrent(window);
    gladLoadGL(glfwGetProcAddress);

    printf("Opengl Version: %s\n",glGetString(GL_VERSION));

    glfwSetFramebufferSizeCallback(window, window_resize_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetKeyCallback(window, key_callback);

    error = glGetError();
    printf("[Initialization] error: 0x%x\n", error);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // 32**3 = 32768
    struct Chunk chunk;
    for ( int i = 0 ; i < CHUNK_DATA_SIZE ; i++)
    {
        chunk.voxel_type[i] = rand() % 2;
    }

    generate_chunk_bitmask(&chunk);

    struct Lattice chunk_mesh;

    chunk_mesh = create_lattice("resources/lattice_vertex.glsl", "resources/lattice_fragment.glsl", 32);

    chunk_mesh.texture = generate_chunk_lattice_texture(&chunk);

    // initialize camera view matrix
    glm_mat4_identity(camera.view);

    glfwGetWindowSize(window, &width, &height);

    //TODO: formalize this somewhere else
    float aspect = (float) width/height;
    glm_perspective(camera.fov, aspect, 0.001f, 4000.0f, camera.projection);
    camera.width = width;
    camera.height = height;

    // initialize object variables
    glm_mat4_identity(chunk_mesh.object_transform);
    vec3 transform = {0.0f,0.0f,1.0f};
    glm_translate(chunk_mesh.object_transform, transform);

    // set to zero for no vsync
    glfwSwapInterval(0);

    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    
    //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    //if (chunk_mesh.texture == 0 )
    //{
    //    printf("Error generating lattice chunk texture\n");
    //    exit(-1);
    //}

    printf("chunk mesh texture id: %d\n", chunk_mesh.texture);
    glBindTexture(GL_TEXTURE_3D, chunk_mesh.texture);

    printf("client render loop start\n");

    error = glGetError();
    printf("[Post Initialization] error: 0x%x\n", error);

    printf("Lattice info:\n\ttexture: %u\n", chunk_mesh.texture);

    float prev_frame_time = 0.0f;
    // render loop
    while(!glfwWindowShouldClose(window))
    {
        printf("\r");

        glClearColor(0.1f,0.1f,0.1f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float current_frame_time = glfwGetTime();
        frame_delta = current_frame_time - prev_frame_time;
        prev_frame_time = current_frame_time;

        printf("fd: %f ", frame_delta);

        input_process();
        printf("wire_frame: %d ", wire_frame);

        camera_process(&camera);

        render_lattice(&chunk_mesh, &camera);

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

    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
        space = true;
    if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE)
        space = false;

    if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_PRESS)
        shift = true;
    if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_RELEASE)
        shift = false;

    if (key == GLFW_KEY_G && action == GLFW_RELEASE)
    {
        if (wire_frame)
        {
            wire_frame = false;
        }
        else
        {
            wire_frame = true;
        }
    }
}

void input_process(void)
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
        glm_vec3_scale(transform, frame_delta*camera.speed, transform);
        glm_vec3_add(camera.position, transform, camera.position);
    }

    if (space)
    {
        vec3 transform;
        glm_vec3_copy(camera.up, transform);
        glm_vec3_scale(transform, frame_delta*camera.speed, transform);
        glm_vec3_add(camera.position, transform, camera.position);
    }

    if (shift)
    {
        vec3 transform;
        glm_vec3_copy(camera.up, transform);
        glm_vec3_scale(transform, -frame_delta*camera.speed, transform);
        glm_vec3_add(camera.position, transform, camera.position);
    }
}

/*
 * Changes the size of the viewport and updates the 3D camera's 
 * perspective matrix according to the new viewport height and width.
 *
 * Without updating the perspective matrix we end up warping the environment.
 * */
void window_resize_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0,0, width, height);
    camera.width = width;
    camera.height = height;
    glm_perspective(camera.fov, (float) width/height, 0.000001f, MAX_RENDER_DISTANCE, camera.projection);
}
