//
// Created by Lana Matic on 28.11.23..
//

#ifndef GRAFIKA_MESH_H
#define GRAFIKA_MESH_H

#include <iostream>
#include <glad/glad.h>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <learnopengl/shader.h>
#include <string>
#include <rg/Error.h>

struct Vertex{
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;

    glm::vec3 Tangent;
    glm::vec3 Bitangent;
};

struct Texture{

    unsigned int id;
    std::string type; //texture_diffuse, texture_specular, texture_normal, texture_height
    std::string path; //putanja do texture
};

class Mesh{
public:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    Mesh(std::vector<Vertex> vs, std::vector<unsigned int> ind, std::vector<Texture> tex){
        this->vertices = vs;
        this->indices = ind;
        this->textures = tex;

        setupMesh();
    }

    void Draw(Shader& shader){

        unsigned int diffuseNr = 1;
        unsigned int specularNr = 1;
        unsigned int normalNr = 1;
        unsigned int heightNr = 1;

        for(unsigned int i = 0; i < textures.size(); i++){
            glActiveTexture(GL_TEXTURE0 + i);
            std::string name = textures[i].type;
            std::string number;

            if(name == "texture_diffuse"){
                number = std::to_string(diffuseNr++);
            }else if(name == "texture_specular"){
                number = std::to_string(specularNr++);
            }else if(name  == "texture_normal"){
                number = std::to_string(normalNr++);
            }else if(name == "texture_height"){
                number = std::to_string(heightNr++);
            }else{
                ASSERT(false, "Unknown texture type");
            }
            name.append(number);

            shader.setInt(name, i);
            glBindTexture(GL_TEXTURE_2D, textures[i].id);
        }

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

        glBindVertexArray(0);
        glActiveTexture(GL_TEXTURE0);
    }

private:
    unsigned int VAO;
    void setupMesh(){

        unsigned int VBO, EBO;

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, Position)));
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, Normal)));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, TexCoords)));
        glEnableVertexAttribArray(2);

        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, Tangent)));
        glEnableVertexAttribArray(3);

        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, Bitangent)));
        glEnableVertexAttribArray(4);

        glBindVertexArray(0);
    }

};

#endif //GRAFIKA_MESH_H
