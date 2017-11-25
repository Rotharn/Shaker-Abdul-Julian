#pragma once
#include "scene_node.h"

#include "resource_manager.h"
//#include "helicopter.h"

namespace game {
	class Enemy : public SceneNode {

	public:
		Enemy(const std::string name, const Resource *geometry, const Resource *material, SceneNode* player, ResourceManager* resman);
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



	protected:
		
		void Update();
		void Shoot();
		SceneNode* CreateMissileInstance(std::string entity_name, std::string object_name, std::string material_name);
		
		enum enemy_type {
			STATIONARY = 0,
			MOVING = 1,
			FLYING = 2
		};
		int type;
		float damage;
		float time;
		SceneNode* player;
		//glm::vec3 playerpos;

		ResourceManager* enemyResMan;

		void AttackCollisions();

		glm::vec3 forward_; // Initial forward vector
		glm::vec3 side_; // Initial side vector


	};
}