#pragma once
#include "scene_node.h"

namespace game {
	class Enemy : public SceneNode {

	public:
		Enemy(const std::string name, const Resource *geometry, const Resource *material);
		~Enemy();

		// Get relative attributes of camera
		glm::vec3 GetForward(void) const;
		glm::vec3 GetSide(void) const;
		glm::vec3 GetUp(void) const;

		// Perform relative transformations of camera
		void Pitch(float angle);
		void Yaw(float angle);
		void Roll(float angle);



	protected:

		void Update();


		glm::vec3 forward_; // Initial forward vector
		glm::vec3 side_; // Initial side vector


	};
}