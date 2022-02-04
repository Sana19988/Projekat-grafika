#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>
#include <model_manager.h>

float* initStandVertices(unsigned &size);
float* initCubemapVertices(unsigned &size);
unsigned int loadTexture(const char *path);
unsigned int loadCubemapTexture();

void setModelShaderUniforms(Shader &shader);
void setStandShaderUniforms(Shader &shader);

void framebuffer_size_callback(GLFWwindow *window, int width, int height);

//void mouse_callback(GLFWwindow *window, double xpos, double ypos); radi plsssss

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

void processInput(GLFWwindow *window);

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

void setViewAndProjectionMatrixForAllShaders(vector<Shader*> &shaders);

void initPodiumMatrices(vector<glm::mat4> &standModel, vector<glm::vec3> &standPosition);
//glm::mat4 drawStand(unsigned int VAO, glm::mat4 &model, Shader shader, int indices_count);

//void renderScene(const Shader &shader);
//void renderCube();

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
//bool shadows = true;
//bool shadowsKeyPressed = false;


Camera camera(glm::vec3(2.0f, -8.0f, 80.0f));

//bool firstMouse = true;
//float lastX = SCR_WIDTH / 2.0f;
//float lastY = SCR_HEIGHT / 2.0f;
//bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

unsigned selectedStand = 0;
glm::vec3 * light_position;

struct PointLight {
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float constant;
    float linear;
    float quadratic;
};


int main() {
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Six paths of Pain", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
   // glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
    // tell GLFW to capture our mouse
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    //face culling :)
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE); //sta se odseca
    glFrontFace(GL_CCW);   // sta je front face

    unsigned sizeof_standVertices;
    float* standVertices = initStandVertices(sizeof_standVertices);

    int indices_count = 36;
    unsigned int VBO, VAO;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof_standVertices, standVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*) nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float )));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float )));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    //CUBEMAP !!

    unsigned sizeof_cubemapVertices;
    float* cubemapVertices = initCubemapVertices(sizeof_cubemapVertices);

    unsigned int cubemapVAO, cubemapVBO;
    glGenVertexArrays(1, &cubemapVAO);
    glGenBuffers(1, &cubemapVBO);
    glBindVertexArray(cubemapVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubemapVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof_cubemapVertices, cubemapVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*) nullptr);
    glEnableVertexAttribArray(0);

    Shader cubemapShader("resources/shaders/cubemap.vs", "resources/shaders/cubemap.fs");
    cubemapShader.setInt("cubemap", 0);

    unsigned int cubemapTexture = loadCubemapTexture();

    vector<glm::mat4> standModel;
    vector<glm::vec3> standPosition;
    initPodiumMatrices(standModel, standPosition);

    //MODELI !!

    Shader modelShader("resources/shaders/model_lighting.vs", "resources/shaders/model_lighting.fs");
    ModelManager modelm = ModelManager(modelShader);


    vector<Shader*> shaders = {&modelShader, &cubemapShader};  // I DRUGA DVA KAD POPRAVIS
    // render loop
    while (!glfwWindowShouldClose(window)) {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        processInput(window);

        modelm.setSelectModel(static_cast<Character>(selectedStand));
        light_position = &standPosition[selectedStand];

        //PROVERA
        glClearColor(0.05f, 0.05f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT |GL_DEPTH_BUFFER_BIT);

        //za sve sejdere
        setViewAndProjectionMatrixForAllShaders(shaders);

        //crtamo model
        setModelShaderUniforms(modelShader);
        modelm.drawCharacters(deltaTime * 150);


        //cubemap

        cubemapShader.use();
        glDepthMask(GL_FALSE);
        glDepthFunc(GL_LEQUAL);  //Passes if the incoming depth value is less than or equal to the stored depth value.
        glBindVertexArray(cubemapVAO);
        glActiveTexture(GL_TEXTURE0);  //ili obicna tekstura?
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glDepthFunc(GL_LESS);
        glDepthMask(GL_TRUE);

        glfwSwapBuffers(window);
        glfwPollEvents();

    }

    glfwTerminate();
    delete standVertices;
    delete cubemapVertices;

    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);

    if(glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
        camera.ProcessMouseMovement(0.0, 30.0);
    if(glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
        camera.ProcessMouseMovement(0.0, -30.0);
    if(glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
        camera.ProcessMouseMovement(30.0, 0.0);
    if(glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
        camera.ProcessMouseMovement(-30.0, 0.0);

}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
    (void)window;
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(yoffset);
    (void)xoffset;
    (void)window;
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {

    if(key == GLFW_KEY_UP && action == GLFW_PRESS && selectedStand < 5)
        selectedStand++;
    if(key == GLFW_KEY_DOWN && action == GLFW_PRESS && selectedStand > 0)
        selectedStand--;
    if(action == GLFW_RELEASE || action == GLFW_REPEAT)
        return;

    (void)scancode;
    (void)mods;
    (void)window;
}


//SVE DA SIJA :))
void setViewAndProjectionMatrixForAllShaders(vector<Shader*> &shaders){

    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

    glm::mat4 view = camera.GetViewMatrix();

    for(Shader* shader : shaders){
        shader->use();
        shader->setMat4("projection", projection);
        shader->setMat4("view", view);
    }
}

glm::mat4 drawStand(unsigned int VAO, glm::mat4 &model, Shader shader, int indices_count){

    shader.use();
    shader.setMat4("model", model);
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, indices_count);

    return model;
}

void initPodiumMatrices(vector<glm::mat4> &standModel, vector<glm::vec3> &standPosition){
    //provera za koordinate :))
    standPosition.emplace_back(-33.3f, -0.4f, 0.0f);
    standPosition.emplace_back(-21.3f, -0.4f, 0.0f);
    standPosition.emplace_back(-7.3f, -0.4f, 0.0f);
    standPosition.emplace_back(6.9f, -0.4f, 0.0f);
    standPosition.emplace_back(21.3f, -0.4f, 0.0f);
    standPosition.emplace_back(36.3, -0.4f, 0.0f);

    for(auto& j : standPosition)
        standModel.push_back(glm::translate(glm::mat4(5.0f), j));
}

//ucitavanje teksture
unsigned int loadTexture(const char *path){

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;

    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if(!data){
        cout << "Texture failed to load" << endl;
        stbi_image_free(data);
        return textureID;
    }

    GLint format;
    if(nrComponents == 2)
        format = GL_RED;
    else if(nrComponents == 3)
        format = GL_RGB;
    else if(nrComponents == 4)
        format = GL_RGBA;
    else{
        cout << "Greska u ucitavanju texture (format)" << endl;
        return textureID;
    }

    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
    return textureID;
}

//ucitavanje teksture za cubemap
unsigned int loadCubemapTexture(){

    vector<std::string> faces = {
            FileSystem::getPath("resources/textures/cubemap/right.jpg"),
            FileSystem::getPath("resources/textures/cubemap/left.jpg"),
            FileSystem::getPath("resources/textures/cubemap/top.jpg"),
            FileSystem::getPath("resources/textures/cubemap/bottom.jpg"),
            FileSystem::getPath("resources/textures/cubemap/front.jpg"),
            FileSystem::getPath("resources/textures/cubemap/back.jpg"),
    };

    unsigned int textureID;
    glGenBuffers(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int height, width, nrChannels;
    unsigned char* data;

    for(int i = 0; i < (int)faces.size(); i++){
        data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if(data)
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        else{
            cout << "failed to load" << endl;
            return -1;
        }
        stbi_image_free(data);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

float* initStandVertices(unsigned &size){

    unsigned numOfCol = 8;
    unsigned numOfVert = 36;

    size = numOfCol * numOfVert * sizeof (float );
    auto* vertices = new float[numOfVert * numOfCol]{

        //top
            1.0f, 0.1f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,      //A0
            1.0f, 0.1f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,     //B1
            -1.0f, 0.1f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,     //C2

            1.0f, 0.1f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,     //B1
            -1.0f, 0.1f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,    //D3
            -1.0f, 0.1f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,    //C2

        //right
            1.0f, 0.1f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.2f,     //B1
            1.0f, 0.1f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.2f,      //A0
            1.0f, -0.1f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,     //E4

            1.0f, 0.1f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.2f,     //B1
            1.0f, -0.1f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,     //E4
            1.0f, -0.1f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,    //F5

        //back
            -1.0f, 0.1f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.2f,    //D3
            1.0f, 0.1f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.2f,     //B1
            1.0f, -0.1f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,    //F5

            -1.0f, 0.1f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.2f,    //D3
            1.0f, -0.1f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,    //F5
            -1.0f, -0.1f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,   //H7

        //left
            -1.0f, 0.1f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.2f,    //C2
            -1.0f, 0.1f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.2f,   //D3
            -1.0f, -0.1f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,   //G6

            -1.0f, -0.1f, 1.0f,  -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,  //G6
            -1.0f, 0.1f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.2f,   //D3
            -1.0f, -0.1f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,  //H7

        //front
            1.0f, 0.1f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.2f,     //A0
            -1.0f, 0.1f, 1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.2f,    //C2
            1.0f, -0.1f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,    //E4

            1.0f, -0.1f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,    //E4
            -1.0f, 0.1f, 1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.2f,    //C2
            -1.0f, -0.1f, 1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,   //G6

        //bottom
            1.0f, -0.1f, -1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,   //F5
            1.0f, -0.1f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f,    //E4
            -1.0f, -0.1f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,   //G6

            1.0f, -0.1f, -1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,   //F5
            -1.0f, -0.1f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,   //G6
            -1.0f, -0.1f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f,  //H7
    };

    return vertices;
}

float* initCubemapVertices(unsigned &size){

    unsigned numOfCol = 3;
    unsigned numOfVert = 36;

    size = numOfCol * numOfVert * sizeof (float );
    auto* vertices = new float[numOfVert * numOfCol]{
            -1.0f, 1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, 1.0f, -1.0f,
            -1.0f, 1.0f, -1.0f,

            -1.0f, -1.0f, 1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f, 1.0f, -1.0f,
            -1.0f, 1.0f, -1.0f,
            -1.0f, 1.0f, 1.0f,
            -1.0f, -1.0f, 1.0f,

            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f, 1.0f,
            -1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, -1.0f, 1.0f,
            -1.0f, -1.0f, 1.0f,

            -1.0f, 1.0f, -1.0f,
            1.0f, 1.0f, -1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            -1.0f, 1.0f, 1.0f,
            -1.0f, 1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f, 1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f, 1.0f,
            1.0f, -1.0f, 1.0f
    };

    return vertices;
}

void setModelShaderUniforms(Shader &shader){
    shader.use();

    shader.setVec3("pointLight.position", *light_position);
    shader.setVec3("cameraPos", camera.Position);

    shader.setVec3("dirLight.direction", -1.0f, -1.0f, -1.0f);
    shader.setVec3("dirLight.ambient", 0.1f, 0.1f, 0.1f);
    shader.setVec3("dirLight.diffuse", 0.7f, 0.7f, 0.7f);
    shader.setVec3("dirLight.specular", 0.7f, 0.7f, 0.7f);

    shader.setVec3("pointLight.ambient", 0.4f, 0.4f, 0.4f);
    shader.setVec3("pointLight.diffuse", 2.0f, 2.0f, 2.0f);
    shader.setVec3("pointLight.specular", 2.0f, 2.0f, 2.0f);

    shader.setFloat("pointLight.constant", 0.5f);
    shader.setFloat("pointLight.linear", 0.04f);
    shader.setFloat("pointLight.quadratic", 0.012f);
}
