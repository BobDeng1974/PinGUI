#include "Shader_Program.h"

/**

    PinGUI

    Copyright (c) 2017 Lubomir Barantal <l.pinsius@gmail.com>

    This software is provided 'as-is', without any express or implied
    warranty. In no event will the authors be held liable for any damages
    arising from the use of this software.

    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
       claim that you wrote the original software. If you use this software
       in a product, an acknowledgment in the product documentation would be
       appreciated but is not required.
    2. Altered source versions must be plainly marked as such, and must not be
       misrepresented as being the original software.
    3. This notice may not be removed or altered from any source distribution.

**/

namespace PinGUI{

    int Shader_Program::_attributes = 0;

    GLuint Shader_Program::_programID = 0;

    GLuint Shader_Program::_vertexShaderID = 0;

    GLuint Shader_Program::_fragmentShaderID = 0;

    GLint Shader_Program::_samplerLocation = 0;

    void Shader_Program::initShaders(){

        // Compile our color shader
        compileShaders("PinGUI/Shaders/vertexShader.txt", "PinGUI/Shaders/fragmentShader.txt");

        //Now add variables
        addAttribute("vertexPosition");
        addAttribute("vertexColor");
        addAttribute("vertexUV");
        addAttribute("instancePos");

        //Link the shaders
        linkShaders();

        PinGUI::CameraManager::setMatrixLocation(PROJECTION,Shader_Program::getUniformLocation("P"));

        _samplerLocation = Shader_Program::getUniformLocation("sampler");
    }

    void Shader_Program::compileShaders(const std::string& vertexShaderFilePath,const std::string& fragmentShaderFilePath){

        _vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
        if (_vertexShaderID==0){
            ErrorManager::systemError("_vertexShader creation fail");
        }

        _fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
        if (_fragmentShaderID==0){
            ErrorManager::systemError("_fragmentShader creation fail");
        }
        _programID = glCreateProgram();

        compileShader(vertexShaderFilePath,_vertexShaderID);
        compileShader(fragmentShaderFilePath,_fragmentShaderID);

    }

    void Shader_Program::compileShader(const std::string& filePath,GLuint id){

        std::ifstream vertexFile(filePath);

        if (vertexFile.fail()){
            ErrorManager::fileError(filePath);
        }

        std::string fileContent = "";
        std::string line;

        while(std::getline(vertexFile,line)){
            fileContent += line + "\n";
        }

        vertexFile.close();

        const char* contentsPtr = fileContent.c_str();
        glShaderSource(id,1,&contentsPtr,nullptr);

        glCompileShader(id);

        GLint success = 0;
        glGetShaderiv(id,GL_COMPILE_STATUS,&success);

        if (success == GL_FALSE){
            GLint maxLength = 0;
            glGetShaderiv(id, GL_INFO_LOG_LENGTH, &maxLength);

            // The maxLength includes the NULL character
            std::vector<GLchar> errorLog(maxLength);
            glGetShaderInfoLog(id, maxLength, &maxLength, &errorLog[0]);

            glDeleteShader(id);

            ErrorManager::systemError( &(errorLog[0]));
        }
    }

    void Shader_Program::linkShaders(){

        //Attach shaders to program
        glAttachShader(_programID, _vertexShaderID);
        glAttachShader(_programID, _fragmentShaderID);

        //Link our program
        glLinkProgram(_programID);

        GLint isLinked = 0;
        glGetProgramiv(_programID, GL_LINK_STATUS, (int *)&isLinked);

        //In case of fail
        if(isLinked == GL_FALSE)
        {
            GLint maxLength = 0;
            glGetProgramiv(_programID, GL_INFO_LOG_LENGTH, &maxLength);

            std::vector<GLchar> infoLog(maxLength);
            glGetProgramInfoLog(_programID, maxLength, &maxLength, &infoLog[0]);

            //Delete program
            glDeleteProgram(_programID);

            //Delete shaders
            glDeleteShader(_vertexShaderID);
            glDeleteShader(_fragmentShaderID);
            ErrorManager::systemError(&(infoLog[0]));
        }

        //Always detach shaders after a successful link.
        glDetachShader(_programID, _vertexShaderID);
        glDetachShader(_programID, _fragmentShaderID);
        glDeleteShader(_vertexShaderID);
        glDeleteShader(_fragmentShaderID);
    }

    void Shader_Program::addAttribute(const std::string& attributeName){
        glBindAttribLocation(_programID,_attributes++,attributeName.c_str());
    }

    void Shader_Program::use(){

        glUseProgram(_programID);

        for (int i=0;i<_attributes;i++){
            glEnableVertexAttribArray(i);
        }

        glActiveTexture(GL_TEXTURE0);

        glUniform1i(_samplerLocation, 0);

        //Projection matrix
        glUniformMatrix4fv(PinGUI::CameraManager::getMatrixLocation(PROJECTION), 1, GL_FALSE, glm::value_ptr(PinGUI::CameraManager::getCameraMatrix()));

    }

    void Shader_Program::unuse(){

        glUseProgram(0);

        for (int i=0;i<_attributes;i++){
            glDisableVertexAttribArray(i);
        }

        //unbind the texture
        glBindTexture(GL_TEXTURE_2D, 0);

    }

    GLuint Shader_Program::getUniformLocation(const std::string& uniformName){

        GLint loc = glGetUniformLocation(_programID,uniformName.c_str());

        if (loc == GL_INVALID_INDEX){
                ErrorManager::systemError("Uniform failed to load to shader. " + uniformName);
        }
        else return loc;

		return 0;
    }
}

