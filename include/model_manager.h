//
// Created by aleksandra on 1.2.22..
//

#ifndef PROJECT_BASE_MODEL_MANAGER_H
#define PROJECT_BASE_MODEL_MANAGER_H

#include <learnopengl/model.h>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "learnopengl/filesystem.h"

using namespace std;

enum Character{ANIMAL = 0, ASURA = 1, DEVA = 2, HUMAN = 3, NARAKA = 4, PRETA = 5};

class MyModel{

public:
    Character character;
    Model *model{};

    glm::mat4 modelMatrix = glm::mat4(1.0f);
    glm::mat4 initialModelMatrix;

    explicit MyModel(Character character){
        this->character = character;

        switch(character) {
            case ANIMAL: makeAnimal(); break;
            case ASURA: makeAsura(); break;
            case DEVA: makeDeva(); break;
            case HUMAN: makeHuman(); break;
            case NARAKA: makeNaraka(); break;
            case PRETA: makePreta(); break;
            default: cout << "Invalid character" << endl;
        }
        memcpy(glm::value_ptr(initialModelMatrix), glm::value_ptr(modelMatrix), sizeof(modelMatrix));
    }

    void resetModelMatrix(){
        memcpy(glm::value_ptr(modelMatrix), glm::value_ptr(initialModelMatrix), sizeof(initialModelMatrix));
    }

private:
    void makeAnimal(){
        model = new Model(FileSystem::getPath("resources/objects/Pain/Animal Path/animal5.obj"));
        modelMatrix = glm::translate(modelMatrix, glm::vec3(-33.3f, -1.7f, 0.0f));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.1f));
    }
    void makeAsura(){
        model = new Model(FileSystem::getPath("resources/objects/Pain/Asura Path/asura6.obj"));
        modelMatrix = glm::translate(modelMatrix, glm::vec3(-21.3f, -2.0f, 0.0f));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.1f));
    }
    void makeDeva(){
        model = new Model(FileSystem::getPath("resources/objects/Pain/Deva Path/deva5.obj"));
       //provera
        modelMatrix = glm::translate(modelMatrix, glm::vec3(-7.3f, -2.0f, 0.0f));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.1f));
    }
    void makeHuman(){
        model = new Model(FileSystem::getPath("resources/objects/Pain/Human Path/human5.obj"));
      //provera
        modelMatrix = glm::translate(modelMatrix, glm::vec3(6.9f, -2.0f, 0.0f));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.1f));
    }
    void makeNaraka(){
        model = new Model(FileSystem::getPath("resources/objects/Pain/Naraka Path/naraka5.obj"));
        modelMatrix = glm::translate(modelMatrix, glm::vec3(21.3f, -2.0f, 0.0f));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.1f));
    }
    void makePreta(){
        model = new Model(FileSystem::getPath("resources/objects/Pain/Preta Path/preta5.obj"));
        modelMatrix = glm::translate(modelMatrix, glm::vec3(36.3f, -2.0f, 0.0f));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.1f));
    }
};

class ModelManager{

public:
    Character selectModel = ANIMAL;
    vector<MyModel*> models = {
            new MyModel(ANIMAL),
            new MyModel(ASURA),
            new MyModel(DEVA),
            new MyModel(HUMAN),
            new MyModel(NARAKA),
            new MyModel(PRETA)
    };

    Shader &shader;

    explicit ModelManager(Shader &shader1) : shader(shader1) {}  //u main :))

    void drawCharacter(Character modelType){
        shader.setMat4("model", models[modelType]->modelMatrix);
        getModel(modelType)->Draw(shader);
    }

    void drawCharacters(float val){
        models[selectModel]->modelMatrix = glm::rotate(models[selectModel]->modelMatrix,
                                                          glm::radians(val),
                                                          glm::vec3(0.0f, 1.0f, 0.0f));

        shader.use();
        drawCharacter(ANIMAL);
        drawCharacter(ASURA);
        drawCharacter(DEVA);
        drawCharacter(HUMAN);
        drawCharacter(NARAKA);
        drawCharacter(PRETA);
    }

    Model* getModel(Character modelType){
        return models[modelType]->model;
    }

    void setSelectModel(Character type){
        if(this->selectModel != type){
            models[selectModel]->resetModelMatrix();
            this->selectModel = type;
        }
    }
};

#endif //PROJECT_BASE_MODEL_MANAGER_H
