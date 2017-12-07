#ifndef SHADER_ATTRIBUTE_H_
#define SHADER_ATTRIBUTE_H_

#include <string>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#define GLM_FORCE_RADIANS
#include <glm/gtc/quaternion.hpp>

#include "resource.h"
#include "camera.h"

namespace game {

    // Possible data types passed to the shader
    // The array passed to the ShaderAttribute class can be packed into
    // one of these types
    typedef enum DtType { FloatType, Vec2Type, Vec3Type, Vec4Type } DataType;

    // Class that manages extra attributes that can be passed to a shader
    //
    // 'size' denotes the total number of floats in the 'data' vector.
    // Thus, it should be a multiple of 3 if the specified data type is
    // Vec3Type, a multiple of 4 for Vec4Type, and so on
    //
    class ShaderAttribute {

        public:
            ShaderAttribute(std::string name, DataType type, int size, GLfloat *data);
            ~ShaderAttribute();

            std::string GetName(void) const;
            DataType GetType(void) const;
            int GetSize(void) const;
            GLfloat *GetData(void) const;
 
            // Set attribute in the shader
            void SetupShader(GLuint program);

        private:
            std::string name_; // Name of the attribute
            DataType type_; // Type of the attribute
            int size_; // Data size
            GLfloat *data_; // Actual data

    }; // class ShaderAttribute

} // namespace game

#endif // SHADER_ATTRIBUTE_H_
