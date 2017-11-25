#include "Enemy.h"


namespace game{
	Enemy::Enemy(const std::string name, const Resource *geometry, const Resource *material, Helicopter* player, ResourceManager* resman) : SceneNode ( name, geometry, material) {
		
		
		this->type = STATIONARY;
		this->damage = 1.0;


		this->time = glfwGetTime();
		this->player = player;
		this->enemyResMan = resman;
	}

	Enemy::~Enemy() {

	}


	void Enemy::Update() {
		float dtime = time - glfwGetTime();
		dtime += (2.0 + (float) (rand() % 5)) / 10.0f;
		if (type == STATIONARY) {
			SetOrientation(glm::angleAxis(glm::dot(this->position_, player->GetPosition()), glm::cross(this->position_, player->GetPosition())));
			if (dtime > 2.0) {
				time = glfwGetTime();
				Shoot();
			}
		}


	}

	void Enemy::Shoot() {

		if (type == STATIONARY) {

			this->children_.push_back(CreateMissileInstance("missile", "SimpleCylinderMesh", "ObjectMaterial"));

		}

	}
	SceneNode* Enemy::CreateMissileInstance(std::string entity_name, std::string object_name, std::string material_name) {

		// Get resources
		Resource *geom = enemyResMan->GetResource(object_name);
		if (!geom) {
			throw(EnemyException(std::string("Could not find resource \"") + object_name + std::string("\"")));
		}

		Resource *mat = enemyResMan->GetResource(material_name);
		if (!mat) {
			throw(EnemyException(std::string("Could not find resource \"") + material_name + std::string("\"")));
		}


		// Create Missile instance
		SceneNode *missile = new SceneNode(entity_name, geom, mat);

		missile->SetVisible(true);
		missile->SetPosition(this->GetPosition());
		missile->SetOrientation(GetOrientation());
		missile->direction = this->GetForward();
	}
	
	void Enemy::AttackCollisions() {

		if (this->children_.size() > 0) {
			for (std::vector<SceneNode *>::const_iterator it = this->children_begin();
			it != children_end(); it++) {
				SceneNode* current = *it;
				if (glm::length((current->GetPosition() - player->GetPosition())) < 2.0)
					player->Hit(this->damage);
			}
		}
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