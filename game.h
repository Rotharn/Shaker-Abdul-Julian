#ifndef GAME_H_
#define GAME_H_

#include <exception>
#include <string>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "scene_graph.h"
#include "resource_manager.h"
#include "camera.h"
#include "asteroid.h"
#include "Enemy.h"

#include <deque>

namespace game {

    // Exception type for the game
    class GameException: public std::exception
    {
        private:
            std::string message_;
        public:
            GameException(std::string message) : message_(message) {};
            virtual const char* what() const throw() { return message_.c_str(); };
            virtual ~GameException() throw() {};
    };

    // Game application
    class Game {

        public:
            // Constructor and destructor
            Game(void);
            ~Game();
            // Call Init() before calling any other method
            void Init(void); 
            // Set up resources for the game
            void SetupResources(void);
            // Set up initial scene
            void SetupScene(void);
            // Run the game: keep the application active
            void MainLoop(void); 

			void Update(GLFWwindow*);

			void CreateMissileInstance(std::string, std::string, std::string);

			void checkForCollisions(GLFWwindow* window, bool laser);

			void PrintVec3(glm::vec3);

			void SetupHelicopter();
			void SetupHelicopterOld();
			void SetupWorld();
			void SetupEnemies();
			void SetupHostage(std::string name);

        protected:
            // GLFW window
            GLFWwindow* window_;

            // Scene graph containing all nodes to render
            SceneGraph scene_;

            // Resources available to the game
            ResourceManager resman_;

            // Camera abstraction
			SceneNode* cameraNode;
            Camera camera_;

            // Flag to turn animation on/off
            bool animating_;

			glm::vec3 ship_velocity = glm::vec3(0.0, 0.0, 0.0);
			glm::vec3 ship_rotation = glm::vec3(0.0, 0.0, 0.0);

			SceneNode* lazerref;
			std::deque<SceneNode*> missiles;
			std::deque<std::deque<SceneNode*>> childmissiles;
			std::vector<SceneNode*> hostages;
			std::deque<SceneNode*> childlasers;
			std::vector<bool> hostcollected;
			std::vector<SceneNode *> collidables;
			std::vector<Enemy*> enemies;
			std::vector<glm::vec3> spawnPoints;

			bool input_up, input_down, input_left, input_right, input_s, input_x, input_a, input_z, input_e, input_q,
				 input_j, input_l, input_i, input_k, input_c, input_m, input_t, input_w, input_d, input_b, input_space, input_shift,
				 input_m1, input_m2, input_m3;
			float offsetx, offsety;

			// Scene graph containing all nodes to render
			Helicopter* heli;
			SceneNode* body2;
			SceneNode* tail;
			SceneNode* rotor1;
			SceneNode* rotor2;
			SceneNode* rotor3;

			SceneNode* test;

			//Queue containing past 5 positions of helicopter
			std::deque<glm::vec3> positions;

            // Methods to initialize the game
            void InitWindow(void);
            void InitView(void);
            void InitEventHandlers(void);
 
            // Methods to handle events
            static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
			static void MouseButtonCallback(GLFWwindow* window, int key, int action, int mods);
			static void ScrollWheelCallback(GLFWwindow* window, double xoffset, double yoffset);
            static void ResizeCallback(GLFWwindow* window, int width, int height);

            // Asteroid field
            // Create instance of one asteroid
            Asteroid *CreateAsteroidInstance(std::string entity_name, std::string object_name, std::string material_name);
			Enemy* CreateEnemyInstance(std::string entity_name, std::string object_namee, std::string material_name);
            // Create entire random asteroid field
            void CreateAsteroidField(int num_asteroids = 200);
			void CreateLaserInstance(std::string entity_name, std::string object_name, std::string material_name);
			SceneNode *CreateInstance(std::string entity_name, std::string object_name, std::string material_name);
			SceneNode *CreateInstance(std::string entity_name, std::string object_name, std::string material_name, std::string parent_name);
			Helicopter* CreateHeliInstance(std::string entity_name, std::string object_name, std::string material_name, std::string parent_name);

    }; // class Game

} // namespace game

#endif // GAME_H_
