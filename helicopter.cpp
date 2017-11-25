#include <stdexcept>
#define GLM_FORCE_RADIANS
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include "helicopter.h"


namespace game {
	Helicopter::Helicopter(){

	}
	Helicopter::Helicopter(SceneNode* node) {
		
	}

	Helicopter::~Helicopter() {

	}



	glm::vec3 Helicopter::GetForward(void) const {

		glm::vec3 current_forward = orientation_ * forward_;
		return -current_forward; // Return -forward since the camera coordinate system points in the opposite direction
	}


	glm::vec3 Helicopter::GetSide(void) const {

		glm::vec3 current_side = orientation_ * side_;
		return current_side;
	}


	glm::vec3 Helicopter::GetUp(void) const {

		glm::vec3 current_forward = orientation_ * forward_;
		glm::vec3 current_side = orientation_ * side_;
		glm::vec3 current_up = glm::cross(current_forward, current_side);
		current_up = glm::normalize(current_up);
		return current_up;
	}


	void Helicopter::Pitch(float angle) {

		glm::quat rotation = glm::angleAxis(angle, GetSide());
		orientation_ = rotation * orientation_;
	}


	void Helicopter::Yaw(float angle) {

		glm::quat rotation = glm::angleAxis(angle, GetUp());
		orientation_ = rotation * orientation_;
	}


	void Helicopter::Roll(float angle) {

		glm::quat rotation = glm::angleAxis(angle, GetForward());
		orientation_ = rotation * orientation_;
	}



}

