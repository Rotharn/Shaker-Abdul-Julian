#include "Enemy.h"


namespace game{
	Enemy::Enemy(const std::string name, const Resource *geometry, const Resource *material) : SceneNode ( name, geometry, material) {

	}

	Enemy::~Enemy() {

	}


	void Enemy::Update() {

	}

	glm::vec3 Enemy::GetForward(void) const {
		glm::vec3 current_forward = orientation_ * forward_;
		return -current_forward; // Return -forward since the camera coordinate system points in the opposite direction
	}


	glm::vec3 Enemy::GetSide(void) const {

		glm::vec3 current_side = orientation_ * side_;
		return current_side;
	}


	glm::vec3 Enemy::GetUp(void) const {

		glm::vec3 current_forward = orientation_ * forward_;
		glm::vec3 current_side = orientation_ * side_;
		glm::vec3 current_up = glm::cross(current_forward, current_side);
		current_up = glm::normalize(current_up);
		return current_up;
	}


	void Enemy::Pitch(float angle) {

		glm::quat rotation = glm::angleAxis(angle, GetSide());
		orientation_ = rotation * orientation_;
	}


	void Enemy::Yaw(float angle) {

		//glm::quat rotation = glm::angleAxis(angle, GetUp());
		glm::quat rotation = glm::angleAxis(angle, glm::vec3(0.0, 1.0, 0.0));
		orientation_ = rotation * orientation_;
	}


	void Enemy::Roll(float angle) {

		glm::quat rotation = glm::angleAxis(angle, GetForward());
		orientation_ = rotation * orientation_;
	}

}