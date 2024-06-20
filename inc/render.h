#include <GLFW/glfw3.h>
#include <cglm/struct.h>

typedef struct Camera
{
    float fov,speed,sensitivity,yaw,pitch,roll;
    vec3 position,direction,front,up,right;
    mat4 view,projection;
} Camera;

typedef struct Lattice
{
    uint8_t scale;
    unsigned int vbo,vao,shader;
    unsigned long long vbo_size;
    mat4 transform;
} Lattice;

typedef struct Mesh
{
    unsigned int vbo, vao, shader;
    size_t vbo_size;
    mat4 object_transform;
} Mesh;

void camera_process(struct Camera *camera);
unsigned int load_shader(const char* vertex_shader_path, const char* fragment_shader_path);
void set_shader_value_float(const char *loc, float value, unsigned int shader_program);
void set_shader_value_float_array(const char *loc, float* value, int size, unsigned int shader_program);
void set_shader_value_vec2(const char *loc, vec2 value, unsigned int shader_program);
void set_shader_value_vec3(const char *loc, vec3 value, unsigned int shader_program);
void set_shader_value_matrix4(const char *loc, mat4 value, unsigned int shader_program);

// basic mesh with just vertices
struct Mesh create_mesh(float *vbo_data, size_t vbo_size, const char* vertex_shader_path, const char* fragment_shader_path);
void render_mesh(struct Mesh *i);
void render_mesh(struct Mesh *i, struct Camera *camera);

struct Lattice create_lattice(const char *vertex_path, const char *fragment_path, uint8_t size);
void draw_lattice(GLFWwindow *window, struct Lattice *lattice, struct Camera *camera);
void create_lattice_mesh_data(uint8_t scale, float voxel_scale, float **out, unsigned long long *out_size);
void window_resize_callback(GLFWwindow* window, int width, int height);
