
#include "scene_node.h"

#include <string>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#define GLM_FORCE_RADIANS
#include <glm/gtc/quaternion.hpp>



namespace game {
	class Helicopter : public SceneNode {
	public:
		Helicopter(const std::string name, const Resource *geometry, const Resource *material,const Resource *texture);
		Helicopter(SceneNode*);
		~Helicopter();


		// Set global camera attributes


		// Get relative attributes of camera
		glm::vec3 GetForward(void) const;
		glm::vec3 GetSide(void) const;
		glm::vec3 GetUp(void) const;

		void SetHostage(bool);
		void SetFreedom(bool);
		bool GetFreedom();
		void SetCaptors(SceneNode**);
		SceneNode** GetCaptors();


		// Perform relative transformations of camera
		void Pitch(float angle);
		void Yaw(float angle);
		void Roll(float angle);

		void Update();

		void Hit(float dmg);

		// Update geometry configuration

	private:

		float time;

		float health;
		float armour;
		float wasHit;

		SceneNode** captors;
		bool isHostage;
		bool free;


		glm::vec3 forward_; // Initial forward vector
		glm::vec3 side_; // Initial side vector
	};
}