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
	Helicopter::Helicopter(const std::string name, const Resource *geometry, const Resource *material,const Resource *texture) : SceneNode(name, geometry, material,texture){
		forward_ = glm::vec3(0.0, 0.0, 1.0);
		side_ = glm::vec3(1.0, 0.0, 0.0);
		
		health = 20.0;
		armour = 1.0;
		visible_ = true;
		time = glfwGetTime();
		this->blending_ = true;
		isHostage = false;
		free = false;


	}
	Helicopter::Helicopter(SceneNode* node) {
		
	}

	Helicopter::~Helicopter() {

	}
	void Helicopter::SetHostage(bool host) {
		isHostage = host;
	}

	void Helicopter::Hit(float dmg) {
		this->health -= glm::max(0.1f, dmg * 1 / armour);
		this->wasHit = glfwGetTime();
	}


	glm::vec3 Helicopter::GetForward(void) const {
		glm::vec3 current_forward = orientation_ * forward_;
		if (texture_ == 0) {
			return -current_forward;// Return forward since the camera coordinate system points in the opposite direction but texture is loaded in opposite direction
		}
		return current_forward; // Return -forward since the camera coordinate system points in the opposite direction
	}


	glm::vec3 Helicopter::GetSide(void) const {

		glm::vec3 current_side = orientation_ * side_;
		if (texture_ == 0) {
			return current_side;// Return side when texture isnt used
		}
		return -current_side;//return - side because texture is inverted
	}


	glm::vec3 Helicopter::GetUp(void) const {

		glm::vec3 current_forward = orientation_ * forward_;
		glm::vec3 current_side = orientation_ * side_;
		glm::vec3 current_up = glm::cross(current_forward, current_side);
		current_up = glm::normalize(current_up);
		return current_up;
	}

	void Helicopter::SetFreedom(bool isFree) {
		free = isFree;
	}

	bool Helicopter::GetFreedom() {
		bool check = true;
		for (int i = 0; i < 4; i++) {
			if (captors == NULL)
				break;
			if (captors[i] == NULL) {
				continue;
			}
			if (captors[i]->GetVisible()) {
				check = false;
				break;
			}
		}

		return check;
	}

	void Helicopter::SetCaptors(SceneNode** c) {
		this->captors = c;
	}

	SceneNode** Helicopter::GetCaptors() {
		return captors;
	}


	void Helicopter::Pitch(float angle) {

		glm::quat rotation = glm::angleAxis(angle, GetSide());
		orientation_ = rotation * orientation_;
	}


	void Helicopter::Yaw(float angle) {

		//glm::quat rotation = glm::angleAxis(angle, GetUp());
		glm::quat rotation = glm::angleAxis(angle, glm::vec3(0.0, 1.0, 0.0));
		orientation_ = rotation * orientation_;
	}


	void Helicopter::Roll(float angle) {

		glm::quat rotation = glm::angleAxis(angle, GetForward());
		orientation_ = rotation * orientation_;
	}

	void Helicopter::Update() {
		float dtime = glfwGetTime() - time;
		time = glfwGetTime();
		if (this->hit && wasHit == 0.0) {
			Hit(hitDmg);
		}
		if ((time - wasHit) < 3.0) {
			if (visible_)
				this->SetVisible(false);
			else
				this->SetVisible(true);
		}
		else if (wasHit > 0.0) {
			wasHit = 0.0;
			this->hit = false;
			this->SetVisible(true);
		}

		Rotate(angm_);
	}




}

