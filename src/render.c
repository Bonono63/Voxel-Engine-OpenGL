#include <glad/gl.h>
#include <render.h>
#include <io.h>

void camera_process(struct Camera *camera)
{
    printf("%5.3f %5.3f %5.3f %f %f\r", camera->position[0], camera->position[1], camera->position[2], camera->yaw, camera->pitch);
    glm_look(camera->position, camera->direction, camera->up, camera->view);
}

unsigned int load_shader(const char * vertex_shader_path, const char * fragment_shader_path)
{
    // VERTEX
    char * vertex_source;
    int vertex_file = read_file(vertex_shader_path, &vertex_source);
    if (vertex_file != 0)
    {
        printf("Unable to compile shader. Vertex shader couldn't be found.\n");
        return -1;
    }

    //printf("vertex shader contents: %s", vertex_source);

    unsigned int vertex_shader;
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, (const char* const*)&vertex_source, NULL);
    glCompileShader(vertex_shader);

    int vertex_success;
    char vertex_info_log[512];
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &vertex_success);
    if (!vertex_success)
    {
        glGetShaderInfoLog(vertex_shader, 512, NULL, vertex_info_log);
        printf("Unable to compile shader. Error compiling. %s\n", vertex_info_log);
    }
    free(vertex_source);

    // FRAGMENT

    char * fragment_source;
    int fragment_file = read_file(fragment_shader_path, &fragment_source);
    if (fragment_file != 0)
    {
        printf("Unable to compile shader. Fragment shader couldn't be found.\n");
        return -1;
    }

    unsigned int fragment_shader;
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, (const char* const*)&fragment_source, NULL);
    glCompileShader(fragment_shader);

    int fragment_success;
    char fragment_info_log[512];
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &fragment_success);
    if (!fragment_success)
    {
        glGetShaderInfoLog(fragment_shader, 512, NULL, fragment_info_log);
        printf("Unable to compile shader. Error compiling %s\n",fragment_info_log);
    }
    free(fragment_source);

    // SHADER LINKING
    unsigned int shader = glCreateProgram();
    glAttachShader(shader, vertex_shader);
    glAttachShader(shader, fragment_shader);
    glLinkProgram(shader);

    int shader_success;
    char shader_info_log[512];
    glGetProgramiv(shader, GL_LINK_STATUS, &shader_success);
    if(!shader_success)
    {
        glGetProgramInfoLog(shader, 512, NULL, shader_info_log);
        printf("Unable to Link the shader program. %s\n",shader_info_log);
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return shader;
}

void set_shader_value_float(const char *loc, float value, unsigned int shader_program)
{
    int location = glGetUniformLocation(shader_program, loc);
    if (location == -1)
        return;
    else
        glUniform1f(location, value);
}

void set_shader_value_float_array(const char *loc, float* value, int size, unsigned int shader_program)
{
    int location = glGetUniformLocation(shader_program, loc);
    if (location == -1)
        return;
    else
        glUniform1fv(location, size, value);
}

void set_shader_value_vec2(const char *loc, vec2 value, unsigned int shader_program)
{
    int location = glGetUniformLocation(shader_program, loc);
    if (location == -1)
        return;
    else
        glUniform2f(location, value[0], value[1]);
}

void set_shader_value_vec3(const char *loc, vec3 value, unsigned int shader_program)
{
    int location = glGetUniformLocation(shader_program, loc);
    if (location == -1)
        return;
    else
        glUniform3f(location, value[0], value[1], value[2]);
}

void set_shader_value_matrix4(const char *loc, mat4 value, unsigned int shader_program)
{
    int location = glGetUniformLocation(shader_program, loc);
    if (location == -1)
        return;
    else
        // If this doesn't work replace value[0] with (float*)value;
        glUniformMatrix4fv(location, 1, GL_FALSE, value[0]);
}

struct Lattice create_lattice(const char *vertex_path, const char *fragment_path, uint8_t size)
{
    struct Lattice result;

    glGenVertexArrays(1, &result.vao);

    glGenBuffers(1, &result.vbo);

    glBindVertexArray(result.vao);

    glBindBuffer(GL_ARRAY_BUFFER, result.vbo);

    // Generate lattice data
    
    float *vbo_data;
    unsigned long long int vbo_size = 0;

    create_lattice_mesh_data(size, 1.0f, &vbo_data, &vbo_size);

    result.vbo_size = vbo_size;

    glBufferData(GL_ARRAY_BUFFER, vbo_size, vbo_data, GL_STATIC_DRAW);

    // Configure vertex data

    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,6*sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,6*sizeof(float),(void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    result.shader = load_shader(vertex_path, fragment_path);
    if (result.shader == -1)
    {
        printf("Shader program didn't compile properly.\n");
    }

    return result;
}

void draw_lattice(GLFWwindow *window, struct Lattice *lattice, struct Camera *camera)
{
    glBindVertexArray(lattice->vao);

    glUseProgram(lattice->shader);

    int width,height;
    glfwGetWindowSize(window, &width, &height);
    glm_perspective(camera->fov, width/height, 0.000001f, 400.0f, camera->projection);

    set_shader_value_matrix4("model", lattice->transform, lattice->shader);
    set_shader_value_matrix4("view", camera->view, lattice->shader);
    set_shader_value_matrix4("perspective", camera->projection, lattice->shader);

    glDrawArrays(GL_TRIANGLES, 0, lattice->vbo_size);
}

void create_lattice_mesh_data(uint8_t size, float voxel_scale, float **out, unsigned long long *out_size)
{
    const int vertex_stride = 6;
    const int index_stride = 6;

    long long int vertex_offset = 0;

    printf("lattice dimensions: %dx%dx%d\n", size, size, size);
    size_t face_count = size*6;
    size_t float_count = face_count * index_stride * vertex_stride;
    size_t byte_count = float_count*sizeof(float);
    
    *out = (float *) malloc(byte_count);
    *out_size = byte_count;

    if (*out == NULL)
    {
        printf("Unable to create lattice data heap.\n");
    }
    
    // Negative Z faces
    for (int z = 0 ; z < size ; z++)
    {
        float layer = ((float)z)/(size-1);
        if (z == size-1)
                layer-=0.000001;
        // BOTTOM FACE
        *(*out+vertex_offset+0) = 1.0f*voxel_scale*size;
        *(*out+vertex_offset+1) = 1.0f*voxel_scale*size;
        *(*out+vertex_offset+2) = -z*voxel_scale;
        *(*out+vertex_offset+3) = 0.0f;
        *(*out+vertex_offset+4) = 1.0f;
        *(*out+vertex_offset+5) = layer;

        vertex_offset+=vertex_stride;

        *(*out+vertex_offset+0) = 1.0f*voxel_scale*size;
        *(*out+vertex_offset+1) = 0.0f;
        *(*out+vertex_offset+2) = -z*voxel_scale;
        *(*out+vertex_offset+3) = 0.0f;
        *(*out+vertex_offset+4) = 0.0f;
        *(*out+vertex_offset+5) = layer;

        vertex_offset+=vertex_stride;

        *(*out+vertex_offset+0) = 0.0f;
        *(*out+vertex_offset+1) = 0.0f;
        *(*out+vertex_offset+2) = -z*voxel_scale;
        *(*out+vertex_offset+3) = 1.0f;
        *(*out+vertex_offset+4) = 0.0f;
        *(*out+vertex_offset+5) = layer;

        vertex_offset+=vertex_stride;

        // TOP FACE
        *(*out+vertex_offset+0) = 0.0f;
        *(*out+vertex_offset+1) = 0.0f;
        *(*out+vertex_offset+2) = -z*voxel_scale;
        *(*out+vertex_offset+3) = 1.0f;
        *(*out+vertex_offset+4) = 0.0f;
        *(*out+vertex_offset+5) = layer;

        vertex_offset+=vertex_stride;

        *(*out+vertex_offset+0) = 0.0f;
        *(*out+vertex_offset+1) = 1.0f*voxel_scale*size;
        *(*out+vertex_offset+2) = -z*voxel_scale;
        *(*out+vertex_offset+3) = 1.0f;
        *(*out+vertex_offset+4) = 1.0f;
        *(*out+vertex_offset+5) = layer;

        vertex_offset+=vertex_stride;

        *(*out+vertex_offset+0) = 1.0f*voxel_scale*size;
        *(*out+vertex_offset+1) = 1.0f*voxel_scale*size;
        *(*out+vertex_offset+2) = -z*voxel_scale;
        *(*out+vertex_offset+3) = 0.0f;
        *(*out+vertex_offset+4) = 1.0f;
        *(*out+vertex_offset+5) = layer;

        vertex_offset+=vertex_stride;
    }

    // POSITIVE Z FACES
    for (int z = 0 ; z < size ; z++)
    {
        float layer = ((float)z)/(size-1);
        if (z == size-1)
                layer-=0.000001;
        // BOTTOM FACE
        *(*out+vertex_offset+0) = 0.0f;
        *(*out+vertex_offset+1) = 0.0f;
        *(*out+vertex_offset+2) = -z*voxel_scale+(1.0f*voxel_scale);
        *(*out+vertex_offset+3) = 1.0f;
        *(*out+vertex_offset+4) = 0.0f;
        *(*out+vertex_offset+5) = layer;

        vertex_offset+=vertex_stride;

        *(*out+vertex_offset+0) = 1.0f*voxel_scale*size;
        *(*out+vertex_offset+1) = 0.0f;
        *(*out+vertex_offset+2) = -z*voxel_scale+(1.0f*voxel_scale);
        *(*out+vertex_offset+3) = 0.0f;
        *(*out+vertex_offset+4) = 0.0f;
        *(*out+vertex_offset+5) = layer;

        vertex_offset+=vertex_stride;


        *(*out+vertex_offset+0) = 1.0f*voxel_scale*size;
        *(*out+vertex_offset+1) = 1.0f*voxel_scale*size;
        *(*out+vertex_offset+2) = -z*voxel_scale+(1.0f*voxel_scale);
        *(*out+vertex_offset+3) = 0.0f;
        *(*out+vertex_offset+4) = 1.0f;
        *(*out+vertex_offset+5) = layer;

        vertex_offset+=vertex_stride;

        // TOP FACE
        *(*out+vertex_offset+0) = 1.0f*voxel_scale*size;
        *(*out+vertex_offset+1) = 1.0f*voxel_scale*size;
        *(*out+vertex_offset+2) = -z*voxel_scale+(1.0f*voxel_scale);
        *(*out+vertex_offset+3) = 0.0f;
        *(*out+vertex_offset+4) = 1.0f;
        *(*out+vertex_offset+5) = layer;

        vertex_offset+=vertex_stride;

        *(*out+vertex_offset+0) = 0.0f;
        *(*out+vertex_offset+1) = 1.0f*voxel_scale*size;
        *(*out+vertex_offset+2) = -z*voxel_scale+(1.0f*voxel_scale);
        *(*out+vertex_offset+3) = 1.0f;
        *(*out+vertex_offset+4) = 1.0f;
        *(*out+vertex_offset+5) = layer;

        vertex_offset+=vertex_stride;

        *(*out+vertex_offset+0) = 0.0f;
        *(*out+vertex_offset+1) = 0.0f;
        *(*out+vertex_offset+2) = -z*voxel_scale+(1.0f*voxel_scale);
        *(*out+vertex_offset+3) = 1.0f;
        *(*out+vertex_offset+4) = 0.0f;
        *(*out+vertex_offset+5) = layer;
    
        vertex_offset+=vertex_stride;
    }

    // NEGATIVE X FACES
    for (int x = 0 ; x < size ; x++)
    {
        float layer = 1.0f-((float)x)/(size-1);
        if (x == 0)
                layer -= 0.000001;
        // BOTTOM FACE
        *(*out+vertex_offset+0) = x*voxel_scale+(1.0f*voxel_scale);
        *(*out+vertex_offset+1) = 0.0f;
        *(*out+vertex_offset+2) = 1.0f*voxel_scale;
        *(*out+vertex_offset+3) = layer;
        *(*out+vertex_offset+4) = 0.0f;
        *(*out+vertex_offset+5) = 0.0f;

        vertex_offset+=vertex_stride;

        *(*out+vertex_offset+0) = x*voxel_scale+(1.0f*voxel_scale);
        *(*out+vertex_offset+1) = 0.0f;
        *(*out+vertex_offset+2) = -1.0f*voxel_scale*size+(1.0f*voxel_scale);
        *(*out+vertex_offset+3) = layer;
        *(*out+vertex_offset+4) = 0.0f;
        *(*out+vertex_offset+5) = 1.0f;

        vertex_offset+=vertex_stride;

        *(*out+vertex_offset+0) = x*voxel_scale+(1.0f*voxel_scale);
        *(*out+vertex_offset+1) = 1.0f*voxel_scale*size;
        *(*out+vertex_offset+2) = -1.0f*voxel_scale*size+(1.0f*voxel_scale);
        *(*out+vertex_offset+3) = layer;
        *(*out+vertex_offset+4) = 1.0f;
        *(*out+vertex_offset+5) = 1.0f;

        vertex_offset+=vertex_stride;

        // TOP FACE
        *(*out+vertex_offset+0) = x*voxel_scale+(1.0f*voxel_scale);
        *(*out+vertex_offset+1) = 1.0f*voxel_scale*size;
        *(*out+vertex_offset+2) = -1.0f*voxel_scale*size+(1.0f*voxel_scale);
        *(*out+vertex_offset+3) = layer;
        *(*out+vertex_offset+4) = 1.0f;
        *(*out+vertex_offset+5) = 1.0f;

        vertex_offset+=vertex_stride;

        *(*out+vertex_offset+0) = x*voxel_scale+(1.0f*voxel_scale);
        *(*out+vertex_offset+1) = 1.0f*voxel_scale*size;
        *(*out+vertex_offset+2) = 1.0f*voxel_scale;
        *(*out+vertex_offset+3) = layer;
        *(*out+vertex_offset+4) = 1.0f;
        *(*out+vertex_offset+5) = 0.0f;

        vertex_offset+=vertex_stride;

        *(*out+vertex_offset+0) = x*voxel_scale+(1.0f*voxel_scale);
        *(*out+vertex_offset+1) = 0.0f;
        *(*out+vertex_offset+2) = 1.0f*voxel_scale;
        *(*out+vertex_offset+3) = layer;
        *(*out+vertex_offset+4) = 0.0f;
        *(*out+vertex_offset+5) = 0.0f;
    
        vertex_offset+=vertex_stride;
    }

    // POSITIVE X FACES
    for (int x = 0 ; x < size ; x++)
    {
        float layer = 1.0f-((float)x)/(size-1);
        if (x == 0)
                layer -= 0.000001;
        // BOTTOM FACE
        *(*out+vertex_offset+0) = x*voxel_scale;
        *(*out+vertex_offset+1) = 1.0f*voxel_scale*size;
        *(*out+vertex_offset+2) = -1.0f*voxel_scale*size+(1.0f*voxel_scale);
        *(*out+vertex_offset+3) = layer;
        *(*out+vertex_offset+4) = 1.0f;
        *(*out+vertex_offset+5) = 1.0f;

        vertex_offset+=vertex_stride;

        *(*out+vertex_offset+0) = x*voxel_scale;
        *(*out+vertex_offset+1) = 0.0f;
        *(*out+vertex_offset+2) = -1.0f*voxel_scale*size+(1.0f*voxel_scale);
        *(*out+vertex_offset+3) = layer;
        *(*out+vertex_offset+4) = 0.0f;
        *(*out+vertex_offset+5) = 1.0f;

        vertex_offset+=vertex_stride;

        *(*out+vertex_offset+0) = x*voxel_scale;
        *(*out+vertex_offset+1) = 0.0f;
        *(*out+vertex_offset+2) = 1.0f*voxel_scale;
        *(*out+vertex_offset+3) = layer;
        *(*out+vertex_offset+4) = 0.0f;
        *(*out+vertex_offset+5) = 0.0f;

        vertex_offset+=vertex_stride;

        // TOP FACE
        *(*out+vertex_offset+0) = x*voxel_scale;
        *(*out+vertex_offset+1) = 0.0f;
        *(*out+vertex_offset+2) = 1.0f*voxel_scale;
        *(*out+vertex_offset+3) = layer;
        *(*out+vertex_offset+4) = 0.0f;
        *(*out+vertex_offset+5) = 0.0f;

        vertex_offset+=vertex_stride;

        *(*out+vertex_offset+0) = x*voxel_scale;
        *(*out+vertex_offset+1) = 1.0f*voxel_scale*size;
        *(*out+vertex_offset+2) = 1.0f*voxel_scale;
        *(*out+vertex_offset+3) = layer;
        *(*out+vertex_offset+4) = 1.0f;
        *(*out+vertex_offset+5) = 0.0f;

        vertex_offset+=vertex_stride;

        *(*out+vertex_offset+0) = x*voxel_scale;
        *(*out+vertex_offset+1) = 1.0f*voxel_scale*size;
        *(*out+vertex_offset+2) = -1.0f*voxel_scale*size+(1.0f*voxel_scale);
        *(*out+vertex_offset+3) = layer;
        *(*out+vertex_offset+4) = 1.0f;
        *(*out+vertex_offset+5) = 1.0f;
    
        vertex_offset+=vertex_stride;
    }

    // NEGATIVE Y FACES
    for (int y = 0 ; y < size ; y++)
    {
        float layer = ((float)y)/(size-1);
        if (y == size-1)
                layer-=0.000001;
        // BOTTOM FACE
        *(*out+vertex_offset+0) = 1.0f*voxel_scale*size;
        *(*out+vertex_offset+1) = y*voxel_scale;
        *(*out+vertex_offset+2) = -1.0f*voxel_scale*size+(1.0f*voxel_scale);
        *(*out+vertex_offset+3) = 0.0f;
        *(*out+vertex_offset+4) = layer;
        *(*out+vertex_offset+5) = 1.0f;

        vertex_offset+=vertex_stride;

        *(*out+vertex_offset+0) = 1.0f*voxel_scale*size;
        *(*out+vertex_offset+1) = y*voxel_scale;
        *(*out+vertex_offset+2) = 1.0f*voxel_scale;
        *(*out+vertex_offset+3) = 0.0f;
        *(*out+vertex_offset+4) = layer;
        *(*out+vertex_offset+5) = 0.0f;

        vertex_offset+=vertex_stride;

        *(*out+vertex_offset+0) = 0.0f;
        *(*out+vertex_offset+1) = y*voxel_scale;
        *(*out+vertex_offset+2) = 1.0f*voxel_scale;
        *(*out+vertex_offset+3) = 1.0f;
        *(*out+vertex_offset+4) = layer;
        *(*out+vertex_offset+5) = 0.0f;

        vertex_offset+=vertex_stride;

        // TOP FACE
        *(*out+vertex_offset+0) = 0.0f;
        *(*out+vertex_offset+1) = y*voxel_scale;
        *(*out+vertex_offset+2) = 1.0f*voxel_scale;
        *(*out+vertex_offset+3) = 1.0f;
        *(*out+vertex_offset+4) = layer;
        *(*out+vertex_offset+5) = 0.0f;

        vertex_offset+=vertex_stride;

        *(*out+vertex_offset+0) = 0.0f;
        *(*out+vertex_offset+1) = y*voxel_scale;
        *(*out+vertex_offset+2) = -1.0f*voxel_scale*size+(1.0f*voxel_scale);
        *(*out+vertex_offset+3) = 1.0f;
        *(*out+vertex_offset+4) = layer;
        *(*out+vertex_offset+5) = 1.0f;

        vertex_offset+=vertex_stride;

        *(*out+vertex_offset+0) = 1.0f*voxel_scale*size;
        *(*out+vertex_offset+1) = y*voxel_scale;
        *(*out+vertex_offset+2) = -1.0f*voxel_scale*size+(1.0f*voxel_scale);
        *(*out+vertex_offset+3) = 0.0f;
        *(*out+vertex_offset+4) = layer;
        *(*out+vertex_offset+5) = 1.0f;
        
        vertex_offset+=vertex_stride;
    }

    // POSITIVE Y FACES
    for (int y = 0 ; y < size ; y++)
    {
        float layer = ((float)y)/(size-1);
        if (y == size-1)
                layer-=0.000001;
        // BOTTOM FACE
        *(*out+vertex_offset+0) = 0.0f;
        *(*out+vertex_offset+1) = y*voxel_scale+(1.0f*voxel_scale);
        *(*out+vertex_offset+2) = 1.0f*voxel_scale;
        *(*out+vertex_offset+3) = 1.0f;
        *(*out+vertex_offset+4) = layer;
        *(*out+vertex_offset+5) = 0.0f;

        vertex_offset+=vertex_stride;

        *(*out+vertex_offset+0) = 1.0f*voxel_scale*size;
        *(*out+vertex_offset+1) = y*voxel_scale+(1.0f*voxel_scale);
        *(*out+vertex_offset+2) = 1.0f*voxel_scale;
        *(*out+vertex_offset+3) = 0.0f;
        *(*out+vertex_offset+4) = layer;
        *(*out+vertex_offset+5) = 0.0f;

        vertex_offset+=vertex_stride;

        *(*out+vertex_offset+0) = 1.0f*voxel_scale*size;
        *(*out+vertex_offset+1) = y*voxel_scale+(1.0f*voxel_scale);
        *(*out+vertex_offset+2) = -1.0f*voxel_scale*size+(1.0f*voxel_scale);
        *(*out+vertex_offset+3) = 0.0f;
        *(*out+vertex_offset+4) = layer;
        *(*out+vertex_offset+5) = 1.0f;

        vertex_offset+=vertex_stride;

        // TOP FACE
        *(*out+vertex_offset+0) = 1.0f*voxel_scale*size;
        *(*out+vertex_offset+1) = y*voxel_scale+(1.0f*voxel_scale);
        *(*out+vertex_offset+2) = -1.0f*voxel_scale*size+(1.0f*voxel_scale);
        *(*out+vertex_offset+3) = 0.0f;
        *(*out+vertex_offset+4) = layer;
        *(*out+vertex_offset+5) = 1.0f;

        vertex_offset+=vertex_stride;

        *(*out+vertex_offset+0) = 0.0f;
        *(*out+vertex_offset+1) = y*voxel_scale+(1.0f*voxel_scale);
        *(*out+vertex_offset+2) = -1.0f*voxel_scale*size+(1.0f*voxel_scale);
        *(*out+vertex_offset+3) = 1.0f;
        *(*out+vertex_offset+4) = layer;
        *(*out+vertex_offset+5) = 1.0f;
        
        vertex_offset+=vertex_stride;

        *(*out+vertex_offset+0) = 0.0f;
        *(*out+vertex_offset+1) = y*voxel_scale+(1.0f*voxel_scale);
        *(*out+vertex_offset+2) = 1.0f*voxel_scale;
        *(*out+vertex_offset+3) = 1.0f;
        *(*out+vertex_offset+4) = layer;
        *(*out+vertex_offset+5) = 0.0f;
        
        vertex_offset+=vertex_stride;
    }
}

struct Mesh create_mesh(float *vbo_data, size_t vbo_size, const char* vertex_shader_path, const char* fragment_shader_path)
{
    struct Mesh mesh;

    mesh.vbo_size = vbo_size;
    mesh.shader = load_shader(vertex_shader_path, fragment_shader_path);

    if (mesh.shader == -1)
        printf("Error encountered while compiling shader for mesh\n");

    glGenVertexArrays(1, &mesh.vao);
    glGenBuffers(1, &mesh.vbo);
    glBindVertexArray(mesh.vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);

    glBufferData(GL_ARRAY_BUFFER, mesh.vbo_size, vbo_data, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return mesh;
}

void render_mesh(struct Mesh *i)
{
    glUseProgram(i->shader);
    glBindVertexArray(i->vao);
    glDrawArrays(GL_TRIANGLES, 0, i->vbo_size);
}

void render_mesh_camera(struct Mesh *i, struct Camera *camera)
{
    glUseProgram(i->shader);
    glBindVertexArray(i->vao);

    set_shader_value_matrix4("proj", camera->projection, i->shader);
    set_shader_value_matrix4("view", camera->view, i->shader);
    set_shader_value_matrix4("model", i->object_transform, i->shader);
    glDrawArrays(GL_TRIANGLES, 0, i->vbo_size);
}

void window_resize_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0,0,width,height);
}
