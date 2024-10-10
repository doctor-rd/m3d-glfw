#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <GLES2/gl2.h>
#include <GLFW/glfw3.h>

const int width = 960;
const int height = 540;

const char* vertex_shader =
"#version 100\n"
"attribute vec2 position;"
"uniform float time;"
"uniform vec2 move;"
"uniform mat4 Pmatrix;"
"uniform mat4 Vmatrix;"
"uniform mat4 Mmatrix;"
"float func(vec2 p, float t) {"
"  return sin(3.*length(p)+t)*pow(.8, pow(length(p), 2.));"
"}"
"void main(void) {"
"  vec2 position = position + move;"
"  gl_Position = Pmatrix*Vmatrix*Mmatrix*vec4(position.x, func(position, time), position.y, 1.0);"
"}";

const char* fragment_shader =
"#version 100\n"
"void main() {"
"  gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);"
"}";

int n_vertices;
GLfloat dim;
GLuint vertex_buffer_x, vertex_buffer_y;

void setdimensions() {
    std::vector<GLfloat> vertices;
    n_vertices = 0;
    dim = 3.f;

    for (GLfloat y=-dim; y<=dim+dim/40.f; y+=dim/20.f) {
        vertices.push_back(0.f);
        vertices.push_back(y);
        n_vertices++;
    }
    glGenBuffers(1, &vertex_buffer_y);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_y);
    glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);

    vertices.erase(vertices.begin());
    vertices.push_back(0.f);
    glGenBuffers(1, &vertex_buffer_x);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_x);
    glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);
}

double _scroll = 0.;

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    _scroll = yoffset;
}

double get_scroll() {
    double result = _scroll;
    _scroll = 0.;
    return result;
}

int main() {
    if (!glfwInit())
        return 1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(width, height, "m3d", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);

    glfwSetScrollCallback(window, scroll_callback);

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertex_shader, NULL);
    glCompileShader(vs);
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragment_shader, NULL);
    glCompileShader(fs);
    GLuint shader_programme = glCreateProgram();
    glAttachShader(shader_programme, fs);
    glAttachShader(shader_programme, vs);
    glLinkProgram(shader_programme);
    GLuint loc_time = glGetUniformLocation(shader_programme, "time");
    GLuint loc_move = glGetUniformLocation(shader_programme, "move");
    GLuint loc_Pmatrix = glGetUniformLocation(shader_programme, "Pmatrix");
    GLuint loc_Vmatrix = glGetUniformLocation(shader_programme, "Vmatrix");
    GLuint loc_Mmatrix = glGetUniformLocation(shader_programme, "Mmatrix");
    GLuint loc_position= glGetAttribLocation(shader_programme, "position");
    glEnableVertexAttribArray(loc_position);

    double xpos, ypos;
    bool drag = false;
    float theta = 0.5f;
    float phi = 0.4f;
    float fov = 1.f;
    const glm::mat4 View = glm::translate(glm::mat4(1.f), glm::vec3(0.f, .5f, -5.f));

    setdimensions();

    while (!glfwWindowShouldClose(window))
    {
        glm::mat4 Model = glm::mat4(1.f);
        Model = glm::rotate(Model, phi, glm::vec3(1.f, 0.f, 0.f));
        Model = glm::rotate(Model, theta, glm::vec3(0.f, 1.f, 0.f));

        glm::mat4 Projection = glm::perspective(fov, (GLfloat)width / (GLfloat)height, 1.0f, 100.0f);

        glUseProgram(shader_programme);
        glUniform1f(loc_time, glfwGetTime());
        glUniformMatrix4fv(loc_Pmatrix, 1, GL_FALSE, glm::value_ptr(Projection));
        glUniformMatrix4fv(loc_Vmatrix, 1, GL_FALSE, glm::value_ptr(View));
        glUniformMatrix4fv(loc_Mmatrix, 1, GL_FALSE, glm::value_ptr(Model));
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_x);
        glVertexAttribPointer(loc_position, 2, GL_FLOAT, GL_FALSE, 0, NULL);
        for (GLfloat y=-dim; y<=dim; y+=.1f) {
            glUniform2fv(loc_move, 1, glm::value_ptr(glm::vec2(0.f, y)));
            glDrawArrays(GL_LINE_STRIP, 0, n_vertices);
        }

        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_y);
        glVertexAttribPointer(loc_position, 2, GL_FLOAT, GL_FALSE, 0, NULL);
        for (GLfloat x=-dim; x<=dim; x+=.1f) {
            glUniform2fv(loc_move, 1, glm::value_ptr(glm::vec2(x, 0.f)));
            glDrawArrays(GL_LINE_STRIP, 0, n_vertices);
        }

        glfwPollEvents();
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            break;
        int btnstate = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
        if (btnstate == GLFW_PRESS) {
            double x, y;
            glfwGetCursorPos(window, &x, &y);
            if (drag) {
                theta += (x-xpos)*2.*glm::pi<double>()/width;
                phi += (y-ypos)*2.*glm::pi<double>()/height;
                if (phi < -.8) phi = -.8;
                if (phi > .8) phi = .8;
            }
            {
                xpos = x;
                ypos = y;
                drag = true;
            }
        }
        if (btnstate == GLFW_RELEASE) {
            drag = false;
        }
        fov -= get_scroll()/10.;
        if (fov < .1f) fov = .1f;
        if (fov > 3.f) fov = 3.f;
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}
