#include <iostream>
#include "shader_attribute.h"

namespace game {

ShaderAttribute::ShaderAttribute(std::string name, DataType type, int size, GLfloat *data){

    name_ = name;
    type_ = type;
    size_ = size;
    data_ = data;
}


ShaderAttribute::~ShaderAttribute(){ 
}


std::string ShaderAttribute::GetName(void) const {

    return name_;
}


DataType ShaderAttribute::GetType(void) const {

    return type_;
}


int ShaderAttribute::GetSize(void) const {

    return size_;
}


GLfloat *ShaderAttribute::GetData(void) const {

    return data_;
}


void ShaderAttribute::SetupShader(GLuint program){

    // Set data in the shader
    GLint location = glGetUniformLocation(program, name_.c_str());

    if (type_ == FloatType){
        glUniform3fv(location, size_, data_);
    } else if (type_ == Vec2Type){
        glUniform2fv(location, size_ / 2, data_);
    } else if (type_ == Vec3Type){
        glUniform3fv(location, size_ / 3, data_);
    } else if (type_ == Vec4Type){
        glUniform4fv(location, size_ / 4, data_);
    }
}

} // namespace game
