#ifndef SCENE_NODE_H_
#define SCENE_NODE_H_

#include <string>
#include <vector>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#define GLM_FORCE_RADIANS
#include <glm/gtc/quaternion.hpp>

#include "resource.h"
#include "camera.h"
#include "shader_attribute.h"
#include <iostream>
namespace game {

	// Class that manages one object in a scene 
	class SceneNode {

	public:
		// Create scene node from given resources
		SceneNode();
		SceneNode(const std::string name, const Resource *geometry, const Resource *material, const Resource *texture);

		// Destructor
		~SceneNode();

		// Get name of node
		const std::string GetName(void) const;

		// Get node attributes
		glm::vec3 GetPosition(void) const;
		glm::quat GetOrientation(void) const;
		glm::vec3 GetScale(void) const;

		// Set node attributes
		void SetPosition(glm::vec3 position);
		void SetOrientation(glm::quat orientation);
		void SetScale(glm::vec3 scale);
		void SetMaterial(GLuint material);
		void SetBoundingBox(glm::vec3* box);

		// Perform transformations on node
		void Translate(glm::vec3 trans);
		void Rotate(glm::quat rot);
		void Scale(glm::vec3 scale);
		void SetVisible(bool visible);
		bool GetVisible();
		void SetParticle(bool particle);
		void SetBlending(bool blend);
		// Shader attributes
		void AddShaderAttribute(std::string name, DataType type, int size, GLfloat *data);
		void RemoveShaderAttribute(std::string name);
		void ClearShaderAttributes(void);
		

		// Draw the node according to scene parameters in 'camera'
		// variable
		virtual glm::mat4 Draw(Camera *camera, glm::mat4 parent_transf);

		// Update the node
		virtual void Update(void);

		// OpenGL variables
		GLenum GetMode(void) const;
		GLuint GetArrayBuffer(void) const;
		GLuint GetElementArrayBuffer(void) const;
		GLsizei GetSize(void) const;
		GLuint GetMaterial(void) const;

		void SetHit(float dmg);


		glm::quat GetAngM(void) const;
		void SetAngM(glm::quat angm);

		// Hierarchy-related methods
		void AddChild(SceneNode *node);
		void AddChild(SceneNode *node, bool test);
		std::vector<SceneNode *>::const_iterator children_begin() const;
		std::vector<SceneNode *>::const_iterator children_end() const;


		glm::vec3 direction;

		/*
		[0] - [3] Top Four Vertices
		[4] - [7] Bottom Four Vertices
		Order: 0 -> 1 -> 2 -> 3
		4 -> 5 -> 6 -> 7
		With 4 directly below 0 and so on.
		*/
		glm::vec3* boundingBox;
		// Hierarchy
	protected:
		std::string name_; // Name of the scene node
		GLuint array_buffer_; // References to geometry: vertex and array buffers
		GLuint element_array_buffer_;
		GLuint texture_;//texture
		GLenum mode_; // Type of geometry
		GLsizei size_; // Number of primitives in geometry
		GLuint material_; // Reference to shader program
		glm::vec3 position_; // Position of node
		glm::quat orientation_; // Orientation of node
		glm::vec3 scale_; // Scale of node
		glm::vec3 forward;
		bool visible_;
		bool particle_;
		bool hit;
		bool blending_;
		float hitDmg;
		glm::quat angm_;
		
		SceneNode *parent_;
		std::vector<SceneNode *> children_;
		std::vector<ShaderAttribute> shader_att_; // Shader attributes

		// Set matrices that transform the node in a shader program
		// Return transformation of current node combined with
		// parent transformation, without including scaling
		glm::mat4 SetupShader(GLuint program, glm::mat4 parent_transf);

	}; // class SceneNode

} // namespace game

#endif // SCENE_NODE_H_
