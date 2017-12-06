#pragma once
#include "scene_node.h"

#include "resource_manager.h"
//#include "helicopter.h"

namespace game {

	class Enemy : public SceneNode {

	public:
		Enemy(int enemyType, const std::string name, const Resource *geometry, const Resource *material, Resource* texture, SceneNode* player, ResourceManager* resman);
		~Enemy();

		class EnemyException : public std::exception
		{
		private:
			std::string message_;
		public:
			EnemyException(std::string message) : message_(message) {};
			virtual const char* what() const throw() { return message_.c_str(); };
			virtual ~EnemyException() throw() {};
		};
		// Get relative attributes of camera
		glm::vec3 GetForward(void) const;
		glm::vec3 GetSide(void) const;
		glm::vec3 GetUp(void) const;

		// Perform relative transformations of camera
		void Pitch(float angle);
		void Yaw(float angle);
		void Roll(float angle);
		bool Shoot();
		glm::quat* GetShotOrientation();
		glm::vec3 GetShotDirection();
		int GetType();
		void SetType(int t);
		void SetAgro(bool agro);



	protected:

		void Update();

		SceneNode* CreateMissileInstance(std::string entity_name, std::string object_name, std::string material_name);

		enum enemy_type {
			STATIONARY = 0,
			MOVING = 1,
			FLYING = 2
		};
		int type;
		float damage;
		float time;
		float shoottime;
		float incrementingFloat;
		bool hasShot;
		int shotCount;
		bool agro;

		glm::quat shotOrientation;
		glm::vec3 shotDirection;

		SceneNode* player;
		//glm::vec3 playerpos;

		ResourceManager* enemyResMan;

		void AttackCollisions();

		glm::vec3 forward_; // Initial forward vector
		glm::vec3 side_; // Initial side vector


	};
}