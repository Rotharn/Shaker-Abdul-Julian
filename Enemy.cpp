#include "Enemy.h"


namespace game {
	Enemy::Enemy(int enemyType, const std::string name, const Resource *geometry, const Resource *material, Resource* texture, SceneNode* player, ResourceManager* resman) : SceneNode(name, geometry, material, texture) {

		forward_ = glm::vec3(0.0, 0.0, 1.0);
		side_ = glm::vec3(1.0, 0.0, 0.0);

		this->type = enemyType;
		this->damage = 1.0;
		shoottime = 0.0;

		this->time = glfwGetTime();
		this->player = player;
		this->enemyResMan = resman;
		//this->SetScale(glm::vec3(0.01, 0.01, 0.01));
		incrementingFloat = 0.0;
		hasShot = false;
		agro = false;
		parentEnemy = NULL;
	}

	Enemy::~Enemy() {

	}


	void Enemy::Update() {
		if (agro && visible_) {
			float dtime = glfwGetTime() - time;
			time = glfwGetTime();
			shoottime += dtime;
			dtime += (2.0 + (float)(rand() % 5)) / 10.0f;
			if (type == STATIONARY) {

				glm::vec3 zax = glm::normalize(player->GetPosition() - this->position_);
				glm::vec3 xax = glm::normalize(glm::cross(this->GetUp(), zax));
				glm::vec3 yax = glm::cross(zax, xax);
				glm::mat3 aax = glm::mat3(xax, yax, zax);
				SetOrientation(glm::quat(aax));
				//SetOrientation(glm::normalize(glm::angleAxis(glm::normalize(1.0f * glm::dot(this->position_, this->position_ - player->GetPosition())), glm::normalize(glm::cross(this->GetPosition(), this->position_ - player->GetPosition())))));

				if (this->parentEnemy != NULL) {
					SetPosition(parentEnemy->GetPosition() + glm::vec3(0.0, 3.0, 0.0));
				}

				incrementingFloat += 0.01;
				if (shoottime > 2.0) {
					shoottime = 0;
					time = glfwGetTime();
					hasShot = true;
					shotCount = 1;
				}
			}
			else if (type == MOVING) {



			}

			else if (type == FLYING) {


				glm::vec3 zax = glm::normalize(player->GetPosition() - this->position_);
				glm::vec3 xax = glm::normalize(glm::cross(this->GetUp(), zax));
				glm::vec3 yax = glm::vec3(0.0, 1.0, 0.0);
				glm::mat3 aax = glm::mat3(xax, yax, zax);
				SetOrientation(glm::quat(aax));

				Translate(this->GetForward() * 1.5f);


				incrementingFloat += 0.01;
				if (shoottime > 2.0) {
					shoottime = 0;
					time = glfwGetTime();
					hasShot = true;
					shotCount = 5;
				}


			}
		}

	}

	bool Enemy::Shoot() {
		bool shooting = hasShot;
		if (hasShot) {
			shotCount--;
			if (shotCount == 0)
				hasShot = !hasShot;
		}
		glm::vec3 zax = glm::normalize(player->GetPosition() - this->position_);
		glm::vec3 xax = glm::normalize(glm::cross(this->GetUp(), zax));
		glm::vec3 yax = glm::cross(zax, xax);
		glm::mat3 aax = glm::mat3(xax, yax, zax);
		shotOrientation = (glm::quat(aax));

		shotDirection = shotOrientation * forward_;


		return shooting;


	}

	void Enemy::SetParentEnemy(Enemy* parEn) {
		parentEnemy = parEn;
	}

	Enemy* Enemy::GetParentEnemy() {
		return parentEnemy;
	}

	int Enemy::GetType() {
		return type;
	}

	glm::quat* Enemy::GetShotOrientation() {
		return &shotOrientation;
	}
	glm::vec3 Enemy::GetShotDirection() {
		return shotDirection;
	}

	void Enemy::SetType(int t) {
		this->type = t;
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
		SceneNode *missile = new SceneNode(entity_name, geom, mat, 0);

		missile->SetVisible(true);
		missile->SetPosition(this->GetPosition());
		missile->SetOrientation(GetOrientation());
		missile->direction = this->GetForward();

		return missile;
	}

	void Enemy::AttackCollisions() {

		if (this->children_.size() > 0) {
			for (std::vector<SceneNode *>::const_iterator it = this->children_begin();
			it != children_end(); it++) {
				SceneNode* current = *it;
				if (glm::length((current->GetPosition() - player->GetPosition())) < 2.0)
					player->SetHit(this->damage);
			}
		}
	}


	glm::vec3 Enemy::GetForward(void) const {
		glm::vec3 current_forward = orientation_ * forward_;
		return current_forward; // Return -forward since the camera coordinate system points in the opposite direction
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

	void Enemy::SetAgro(bool agro_) {
		agro = agro_;
	}


	void Enemy::Roll(float angle) {

		glm::quat rotation = glm::angleAxis(angle, GetForward());
		orientation_ = rotation * orientation_;
	}

}