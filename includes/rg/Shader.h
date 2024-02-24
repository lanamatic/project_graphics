//
// Created by matf-rg on 30.10.20..
//

#ifndef GRAFIKA_SHADER_H
#define GRAFIKA_SHADER_H

#include <string>
#include <glad/glad.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <rg/Error.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

std::string readFileContents(std::string path){

    //citanje fajla input file stream
    std::ifstream in(path);
    //stringstream omogucava da pisemo u baffer kao na standardni izlaz
    //i mogu da se kombinuju stringovi, intovi ...
    std::stringstream buffer;
    //ucitamo sve
    buffer << in.rdbuf();

    std::cout << path << std::endl;

    return buffer.str();
}

class Shader{

    unsigned int m_Id;

public:
    //konstruktor
    Shader(std::string vertexShaderPath, std::string fragmentShaderPath){

        int success = 0;
        char infoLog[512];

        //citamo fajl
        std::string vsString = readFileContents(vertexShaderPath);
        //ne sme da bude prazan
        ASSERT(!vsString.empty(), "Vertex shader source is empty!");
        //treba nam pokazivac na string
        const char* vertexShaderSource = vsString.c_str();

        unsigned vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
        glCompileShader(vertexShader);

        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if(!success){
            glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
            std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED!\n" << infoLog << '\n';
        }

        //citamo fajl;
        std::string fgString = readFileContents(fragmentShaderPath);
        //ne sme da bude prazan
        ASSERT(!fgString.empty(), "Fragment shader source is empty!");
        //pokazivac na string
        const char* fragmentShaderSource = fgString.c_str();
        unsigned fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
        glCompileShader(fragmentShader);

        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if(!success){
            glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
            std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED!\n" << infoLog << '\n';
        }

        unsigned shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);

        glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
        if(!success){
            glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
            std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED!\n" << infoLog << '\n';
        }

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        m_Id = shaderProgram;
    }

    void use(){
        ASSERT(m_Id > 0, "Using undefined or deleted program!");
        glUseProgram(m_Id);
    }
    void deleteProgram(){
        glDeleteProgram(m_Id);
        m_Id = 0;
    }

    void setBool(const std::string &name, bool value) const
    {
        glUniform1i(glGetUniformLocation(m_Id, name.c_str()), (int)value);
    }
    // ------------------------------------------------------------------------
    void setInt(const std::string &name, int value) const
    {
        glUniform1i(glGetUniformLocation(m_Id, name.c_str()), value);
    }
    // ------------------------------------------------------------------------
    void setFloat(const std::string &name, float value) const
    {
        glUniform1f(glGetUniformLocation(m_Id, name.c_str()), value);
    }
    // ------------------------------------------------------------------------
    void setVec2(const std::string &name, const glm::vec2 &value) const
    {
        glUniform2fv(glGetUniformLocation(m_Id, name.c_str()), 1, &value[0]);
    }
    void setVec2(const std::string &name, float x, float y) const
    {
        glUniform2f(glGetUniformLocation(m_Id, name.c_str()), x, y);
    }
    // ------------------------------------------------------------------------
    void setVec3(const std::string &name, const glm::vec3 &value) const
    {
        glUniform3fv(glGetUniformLocation(m_Id, name.c_str()), 1, &value[0]);
    }
    void setVec3(const std::string &name, float x, float y, float z) const
    {
        glUniform3f(glGetUniformLocation(m_Id, name.c_str()), x, y, z);
    }
    // ------------------------------------------------------------------------
    void setVec4(const std::string &name, const glm::vec4 &value) const
    {
        glUniform4fv(glGetUniformLocation(m_Id, name.c_str()), 1, &value[0]);
    }
    void setVec4(const std::string &name, float x, float y, float z, float w)
    {
        glUniform4f(glGetUniformLocation(m_Id, name.c_str()), x, y, z, w);
    }
    // ------------------------------------------------------------------------
    void setMat2(const std::string &name, const glm::mat2 &mat) const
    {
        glUniformMatrix2fv(glGetUniformLocation(m_Id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
    // ------------------------------------------------------------------------
    void setMat3(const std::string &name, const glm::mat3 &mat) const
    {
        glUniformMatrix3fv(glGetUniformLocation(m_Id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
    // ------------------------------------------------------------------------
    void setMat4(const std::string &name, const glm::mat4 &mat) const
    {
        glUniformMatrix4fv(glGetUniformLocation(m_Id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }

};

#endif //GRAFIKA_SHADER_H
