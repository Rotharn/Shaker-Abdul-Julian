#include <iostream>
#include <time.h>
#include <sstream>

#include "game.h"
#include "bin/path_config.h"
#include <stack>

namespace game {

	// Some configuration constants
	// They are written here as global variables, but ideally they should be loaded from a configuration file

	// Main window settings
	const std::string window_title_g = "Demo";
	const unsigned int window_width_g = 1920;
	const unsigned int window_height_g = 1080;
	const bool window_full_screen_g = false;

	// Viewport and camera settings
	float camera_near_clip_distance_g = 0.01;
	float camera_far_clip_distance_g = 1000.0;
	float camera_fov_g = 60.0; // Field-of-view of camera
	const glm::vec3 viewport_background_color_g(0.0, 0.0, 0.0);
	glm::vec3 camera_position_g(0.0, 0.0, 15.0);
	glm::vec3 camera_look_at_g(0.0, 0.0, 0.0);
	glm::vec3 camera_up_g(0.0, 1.0, 0.0);
	glm::vec2 cursorPos(0.0, 0.0);
	glm::vec2 windowCenter(((float)window_width_g) / 2.0f,((float) window_height_g) / 2.0f);

	// Materials 
	const std::string material_directory_g = MATERIAL_DIRECTORY;


	Game::Game(void) {

		// Don't do work in the constructor, leave it for the Init() function
	}

	GLFWcursor* Game::CreateBlankCursor()
	{
		const int w = 1;//16;
		const int h = 1;//16;
		unsigned char pixels[w * h * 4];
		memset(pixels, 0x00, sizeof(pixels));
		GLFWimage image;
		image.width = w;
		image.height = h;
		image.pixels = pixels;
		return glfwCreateCursor(&image, 0, 0);
	}

	void Game::Init(void) {

		// Run all initialization steps
		InitWindow();
		InitView();
		InitEventHandlers();

		time_t theLocalTime;
		struct tm * timeinfo;
		time(&theLocalTime);
		timeinfo = localtime(&theLocalTime);
		srand(timeinfo->tm_hour * 60 * 60 + timeinfo->tm_min * 60 + timeinfo->tm_sec);

		// Set variables
		animating_ = true;
		positions = std::deque<glm::vec3>(120, glm::vec3(0.0, 0.0, 0.0));
	}


	void Game::InitWindow(void) {

		// Initialize the window management library (GLFW)
		if (!glfwInit()) {
			throw(GameException(std::string("Could not initialize the GLFW library")));
		}

		// Create a window and its OpenGL context
		if (window_full_screen_g) {
			window_ = glfwCreateWindow(window_width_g, window_height_g, window_title_g.c_str(), glfwGetPrimaryMonitor(), NULL);
		}
		else {
			window_ = glfwCreateWindow(window_width_g, window_height_g, window_title_g.c_str(), NULL, NULL);
		}
		if (!window_) {
			glfwTerminate();
			throw(GameException(std::string("Could not create window")));
		}

		// Make the window's context the current one
		glfwMakeContextCurrent(window_);

		// Initialize the GLEW library to access OpenGL extensions
		// Need to do it after initializing an OpenGL context
		glewExperimental = GL_TRUE;
		GLenum err = glewInit();
		if (err != GLEW_OK) {
			throw(GameException(std::string("Could not initialize the GLEW library: ") + std::string((const char *)glewGetErrorString(err))));
		}
	}


	void Game::InitView(void) {

		// Set up z-buffer
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		// Set viewport
		int width, height;
		glfwGetFramebufferSize(window_, &width, &height);
		glViewport(0, 0, width, height);


		// Set up camera
		// Set current view
		camera_.SetView(camera_position_g, camera_look_at_g, camera_up_g);
		// Set projection
		camera_.SetProjection(camera_fov_g, camera_near_clip_distance_g, camera_far_clip_distance_g, width, height);
	}


	void Game::InitEventHandlers(void) {

		// Set event callbacks
		glfwSetKeyCallback(window_, KeyCallback);
		glfwSetMouseButtonCallback(window_, MouseButtonCallback);
		glfwSetScrollCallback(window_, ScrollWheelCallback);
		glfwSetFramebufferSizeCallback(window_, ResizeCallback);

		// Set pointer to game object, so that callbacks can access it
		glfwSetWindowUserPointer(window_, (void *) this);
	}


	void Game::SetupResources(void) {

		// Create a simple sphere to represent the asteroids
		resman_.CreateSphere("SimpleSphereMesh", 1.0, 10, 10);
		resman_.CreateCylinder("SimpleCylinderMesh", 0.0, 0.05, 3, 30);
		resman_.CreateCylinder("CylinderMesh", 0.0f, 0.2f, 3, 10, -1);
		resman_.CreateCylinder("LaserMesh", 0.0f, 0.2f, 3, 5, 0);
		resman_.CreateCube("CubeMesh");
		resman_.CreateMissileParticles("MissileParticles");
		resman_.CreateMissileParticles("MissileParticle");
		resman_.CreateTorusParticles("TorusParticles");
		resman_.CreateControlPoints("ControlPoints", 64);

		// Load material to be applied to asteroids
		std::string filename = std::string(MATERIAL_DIRECTORY) + std::string("/material");
		resman_.LoadResource(Material, "ObjectMaterial", filename.c_str());

		filename = std::string(MATERIAL_DIRECTORY) + std::string("/shiny_blue");
		resman_.LoadResource(Material, "ShinyBlueMaterial", filename.c_str());

		filename = std::string(MATERIAL_DIRECTORY) + std::string("/toon");
		resman_.LoadResource(Material, "ToonRingMaterial", filename.c_str());

		filename = std::string(MATERIAL_DIRECTORY) + std::string("/toon_heli");
		resman_.LoadResource(Material, "ToonHeliMaterial", filename.c_str());

		filename = std::string(MATERIAL_DIRECTORY) + std::string("/texture");
		resman_.LoadResource(Material, "textureMaterial", filename.c_str());

		filename = std::string(MATERIAL_DIRECTORY) + std::string("/camo_cloth_woodland_2048.png");
		resman_.LoadResource(Texture, "Camo", filename.c_str());

		filename = std::string(MATERIAL_DIRECTORY) + std::string("/road.png");
		resman_.LoadResource(Texture, "Ground", filename.c_str());

		filename = std::string(MATERIAL_DIRECTORY) + std::string("/metal.png");
		resman_.LoadResource(Texture, "Metal", filename.c_str());

		filename = std::string(MATERIAL_DIRECTORY) + std::string("/sphere_fire.png");
		resman_.LoadResource(Texture, "Explosion", filename.c_str());

		filename = std::string(MATERIAL_DIRECTORY) + std::string("/flame4x4orig.png");
		resman_.LoadResource(Texture, "Fire", filename.c_str());
		filename = std::string(MATERIAL_DIRECTORY) + std::string("/Block-3343.png");
		resman_.LoadResource(Texture, "Building", filename.c_str());

		filename = std::string(MATERIAL_DIRECTORY) + std::string("/tank2.obj");
		resman_.LoadResource(Mesh, "tankMesh", filename.c_str());

		filename = std::string(MATERIAL_DIRECTORY) + std::string("/helicoptero_1.1.obj");
		resman_.LoadResource(Mesh, "HeliBodyMesh", filename.c_str());

		filename = std::string(MATERIAL_DIRECTORY) + std::string("/helicoptero_1.2.obj");
		resman_.LoadResource(Mesh, "HeliStockRotorMesh", filename.c_str());

		filename = std::string(MATERIAL_DIRECTORY) + std::string("/helicoptertailrotor.obj");
		resman_.LoadResource(Mesh, "HeliTailRotorMesh", filename.c_str());

		filename = std::string(MATERIAL_DIRECTORY) + std::string("/missile");
		resman_.LoadResource(Material, "MissileMaterial", filename.c_str());
		
		filename = std::string(MATERIAL_DIRECTORY) + std::string("/bullet");
		resman_.LoadResource(Material, "BulletMaterial", filename.c_str());

		filename = std::string(MATERIAL_DIRECTORY) + std::string("/spline");
		resman_.LoadResource(Material, "SplineMaterial", filename.c_str());



	}
	void Game::InitInputs() {

		input_up = false; 
		input_down = false;
		input_left = false;
		input_right = false;
		input_s = false;
		input_x = false;
		input_a = false;
		input_z = false;
		input_e = false;
		input_q = false;
		input_j = false;
		input_l = false;
		input_i = false;
		input_k = false;
		input_c = false;
		input_m = false;
		input_t = false;
		input_w = false;
		input_d = false;
		input_b = false;
		input_space = false;
		input_shift = false;
		input_m1 = false;
		input_m2 = false;
		input_m3 = false;
	}

	void Game::SetupScene(void) {

		InitInputs();
		offsetx = 20.0f;
		offsety = 2.0f;

		worldXmax = 600;
		worldXmin = 0;
		worldZmax = 600;
		worldZmin = 0;

		missileFireRate = 2.0f;
		missileTimer = missileFireRate;

		// Set background color for the scene
		glfwSetCursor(window_, CreateBlankCursor());
		scene_.SetBackgroundColor(viewport_background_color_g);
		cameraNode = CreateInstance("Camera", "", "");
		SceneNode* root = CreateInstance("Root", "", "");
		root->SetPosition(glm::vec3(0.0, 0.0, 0.0));
		scene_.SetRoot(root);
		root->AddChild(cameraNode);
		SetupHelicopter(0, NULL);		
		SetupWorld();
		positions = std::deque<glm::vec3>(120, heli->GetPosition());
		// Create asteroid field
		CreateLaserInstance("laser", "LaserMesh", "ObjectMaterial");

		for (int i = 0; i < 4; i++) {
			SpawnRandomHostage();
		}



		camera_position_g = glm::vec3(heli->GetPosition().x, heli->GetPosition().y - offsety, heli->GetPosition().z - offsetx);
		camera_.SetView(camera_position_g, heli->GetPosition(), camera_up_g);

	}

	SceneNode* Game::CreateExplosionSphere(glm::vec3 pos) {
		SceneNode* exSphere = CreateTexturedInstance("ExplosionSphere", "SimpleSphereMesh", "textureMaterial", "Explosion");
		exSphere->SetPosition(pos);
		exSphere->SetScale(glm::vec3(1.0, 1.0, 1.0));
		scene_.GetNode("Root")->AddChild(exSphere);
		explosionSpheres.push_back(exSphere);
		return exSphere;
	}
	void Game::UpdateExplosions(float dTime) {
		for (int i = 0; i < explosionSpheres.size(); i++) {
			if (glm::length(explosionSpheres[i]->GetScale()) > 20.0) {
				explosionSpheres[i]->SetVisible(false);
				explosionSpheres.erase(explosionSpheres.begin() + i);
			}
			else
				explosionSpheres[i]->SetScale(explosionSpheres[i]->GetScale() + (1.0f + 6.5f * dTime));
		}
	}


	glm::vec2 Game::CursorMovement() {
		return glm::vec2(windowCenter.x - cursorPos.x, windowCenter.y - cursorPos.y);
	}

	void Game::MainLoop(void) {
		double cursorGetX, cursorGetY;


		// Loop while the user did not close the window
		while (!glfwWindowShouldClose(window_)) {
			// Animate the scene
			if (animating_) {
				static double last_time = 0;
				double current_time = glfwGetTime();
				double deltaTime = current_time - last_time;
				if (deltaTime > 1.0 / 60.0) {

					if (missileTimer > -1.0f)
						missileTimer -= deltaTime;
					glfwGetCursorPos(window_, &cursorGetX, &cursorGetY);
					cursorPos = glm::vec2((float)cursorGetX, (float)cursorGetY);
					playerMouse = CursorMovement();

					glfwSetCursorPos(window_, 1920.0 / 2.0, 1080.0 / 2.0);


					scene_.Update();
					Update(window_);
					UpdateExplosions(deltaTime);
					last_time = current_time;
				}
			}
			if (glm::length(positions.front() - heli->GetPosition()) > 0.3) {
				positions.push_front(heli->GetPosition());

				positions.pop_back();
				for (int i = 0; i < hostages.size(); i++) {
					if (hostcollected[i]) {
						hostages[i]->SetPosition(positions[((i + 1) * 5) - 1]);
						hostages[i]->SetOrientation(heli->GetOrientation());
						scene_.GetNode(hostages[i]->GetName() + +"SplineParticles")->SetVisible(false);
					}
				}
			}


			// Draw the scene
			scene_.Draw(&camera_);

			// Push buffer drawn in the background onto the display
			glfwSwapBuffers(window_);

			// Update other events like input handling
			glfwPollEvents();

		}
	}


	void Game::PrintVec3(glm::vec3 vec) {

		std::cout << "x = " << vec.x << " y = " << vec.y << " z = " << vec.z << std::endl;

	}

	void Game::Update(GLFWwindow* window) {

		void* ptr = glfwGetWindowUserPointer(window);
		Game *game = (Game *)ptr;
		float rot_factor(glm::pi<float>() / 180);
		float trans_factor = 1.0;
		ticker = (ticker + 1) % 30;
		//game->test->Scale(glm::vec3(1.01, 1.01, 1.01));
		//PrintVec3(game->test->GetScale());
		glm::vec3 prevpos = heli->GetPosition();

		game->camera_.Translate((game->camera_.GetForward() * game->offsetx) - (game->camera_.GetUp() * game->offsety));

		game->camera_.Rotate(glm::angleAxis(0.001f * playerMouse.y, game->camera_.GetSide()));
		//game->PrintVec3(game->camera_.GetForward());

		game->camera_.Translate((game->camera_.GetForward() * (game->offsetx * -1.0f)) - (game->camera_.GetUp() * game->offsety) * -1.0f);





		if ((game->input_up == true || game->input_space == true) && game->ship_velocity[1] < 2.0f) {
			game->ship_velocity[1] += 0.02;
		}
		if ((game->input_up == false && game->input_space == false) && game->ship_velocity[1] > 0.0f) {
			if (game->ship_velocity[1] < 0.02f)
				game->ship_velocity[1] = 0.0;
			else
				game->ship_velocity[1] -= 0.02;
		}
		if ((game->input_down == true || game->input_shift == true || game->input_x == true) && game->ship_velocity[1] > -2.0f) {

			game->ship_velocity[1] -= 0.02;
		}
		if ((game->input_down == false && game->input_shift == false && game->input_x == false) && game->ship_velocity[1] < 0.0f) {
			if (game->ship_velocity[1] > -0.02f)
				game->ship_velocity[1] = 0.0;
			else
				game->ship_velocity[1] += 0.02;
		}

		if ((game->input_left == true || game->input_q == true) && game->ship_rotation[1] < 2.0f) {
			game->ship_rotation[1] += 0.02;
		}
		if ((game->input_left == false && game->input_q == false) && game->ship_rotation[1] > 0.0f) {
			if (game->ship_rotation[1] < 0.02f)
				game->ship_rotation[1] = 0.0;
			else
				game->ship_rotation[1] -= 0.02;
		}

		if ((game->input_right == true || game->input_e == true) && game->ship_rotation[1] > -2.0f) {
			game->ship_rotation[1] -= 0.02;
		}
		if ((game->input_right == false && game->input_e == false) && game->ship_rotation[1] < 0.0f) {
			if (game->ship_rotation[1] > -0.02f)
				game->ship_rotation[1] = 0.0;
			else
				game->ship_rotation[1] += 0.02;
		}

		if (game->input_s == true && game->ship_velocity[2] < 5.0f) {
			game->ship_velocity[2] += 0.02f;
		}
		if (game->input_s == false && game->ship_velocity[2] > 0.0f) {
			if (game->ship_velocity[2] < 0.02f)
				game->ship_velocity[2] = 0.0;
			else
				game->ship_velocity[2] -= 0.02f;
		}

		if (game->input_w == true && game->ship_velocity[2] > -5.0f) {
			game->ship_velocity[2] -= 0.05;
		}
		if (game->input_w == false && game->ship_velocity[2] < 0.0f) {
			if (game->ship_velocity[2] > -0.02f)
				game->ship_velocity[2] = 0.0;
			else
				game->ship_velocity[2] += 0.02;
		}

		if (game->input_d == true && game->ship_velocity[0] < 5.0f) {
			game->ship_velocity[0] += 0.05;
		}
		if (game->input_d == false && game->ship_velocity[0] > 0.0f) {
			if (game->ship_velocity[0] < 0.02f)
				game->ship_velocity[0] = 0.0;
			else
				game->ship_velocity[0] -= 0.02;
		}

		if (game->input_a == true && game->ship_velocity[0] > -5.0f) {
			game->ship_velocity[0] -= 0.05;
		}
		if (game->input_a == false && game->ship_velocity[0] < 0.0f) {
			if (game->ship_velocity[0] > -0.02f)
				game->ship_velocity[0] = 0.0;
			else
				game->ship_velocity[0] += 0.02;
		}


		if (game->input_i == true && game->ship_rotation[0] < 1.0f) {
			game->ship_rotation[0] += 0.02;
		}
		if (game->input_i == false && game->ship_rotation[0] < 0.0f) {
			game->ship_rotation[0] -= 0.02;
		}
		if (game->input_k == true && game->ship_rotation[0] > -1.0f) {
			game->ship_rotation[0] -= 0.02;
		}
		if (game->input_k == false && game->ship_rotation[0] < 0.0f) {
			game->ship_rotation[0] += 0.02;
		}

		if (game->input_j == true && game->ship_rotation[2] > -1.0f) {
			game->ship_rotation[2] += 0.02;
		}
		if (game->input_j == false && game->ship_rotation[2] < 0.0f) {
			game->ship_rotation[2] -= 0.02;
		}
		if (game->input_l == true && game->ship_rotation[2] > -1.0f) {
			game->ship_rotation[2] -= 0.02;
		}
		if (game->input_l == false && game->ship_rotation[2] < 0.0f) {
			game->ship_rotation[2] += 0.02;
		}

		if (game->input_t == true) {
			ship_rotation *= (float) 0.0;
			ship_velocity *= (float) 0.0;
		}


		ship_rotation[1] = playerMouse.x * 0.05f;


		glm::vec3 currPos = game->camera_.GetPosition();
		glm::vec3 offsetPos = game->camera_.GetPosition() + (game->camera_.GetForward() * offsetx) - (game->camera_.GetUp() * offsety);

		game->camera_.Translate((game->camera_.GetForward() * offsetx) - (game->camera_.GetUp() * offsety));

		game->camera_.Pitch(rot_factor*ship_rotation[0]);
		game->camera_.Yaw(rot_factor*ship_rotation[1]);
		game->camera_.Roll(-rot_factor*ship_rotation[2]);

		if (input_b == false) {
			//heli->SetOrientation(game->camera_.GetOrientation());
			heli->Yaw(rot_factor*ship_rotation[1]);
			//heli->Rotate(glm::quat(glm::angleAxis(((glm::pi<float>() / (float) 1.0)), glm::vec3(0.0, 1.0, 0.0))));
		}
		//game->lazerref->SetForward(game->camera_.GetForward());
		//for (int i = 0; i < childlasers.size(); ++i) {
		//childlasers[i]->SetForward(game->camera_.GetForward());
		//}


		game->camera_.Translate((game->camera_.GetForward() * (offsetx * -1.0f)) - (game->camera_.GetUp() * offsety) * -1.0f);


		//glm::vec3 mov = glm::vec3(game->camera_.GetForward().x * ship_velocity[2] + game->camera_.GetSide().x * ship_velocity[0] + game->camera_.GetUp().x *ship_velocity[1]
		//, game->camera_.GetForward().y * ship_velocity[2] + game->camera_.GetSide().y * ship_velocity[0] + game->camera_.GetUp().y *ship_velocity[1]
		//, game->camera_.GetForward().z * ship_velocity[2] + game->camera_.GetSide().z * ship_velocity[0] + game->camera_.GetUp().z *ship_velocity[1]);

		glm::vec3* pos = &game->heli->GetPosition();
		if (pos->z < 0) {
			game->heli->SetPosition(glm::vec3(pos->x, pos->y, 0));
			ship_velocity[2] = 0;
		}
		if (pos->z > worldZmax) {
			game->heli->SetPosition(glm::vec3(pos->x, pos->y, worldZmax));
			ship_velocity[2] = 0;
		}
		if (pos->x > worldXmax) {
			game->heli->SetPosition(glm::vec3(worldXmax, pos->y, pos->z));
			ship_velocity[0] = 0;
		}
		if (pos->x < 0) {
			game->heli->SetPosition(glm::vec3(0, pos->y, pos->z));
			ship_velocity[0] = 0;
		}
		if (pos->y < 0) {
			game->heli->SetPosition(glm::vec3(pos->x, -0, pos->z));
			ship_velocity[1] = 0;
		}
		if (pos->y > 350) {
			game->heli->SetPosition(glm::vec3(pos->x, 350, pos->z));
			ship_velocity[1] = 0;
		}
		/*
		glm::vec3* pos = &game->heli->GetPosition();
		if (pos->z < 0) {
		game->heli->SetPosition(glm::vec3(pos->x, pos->y, 0));
		ship_velocity[2] = 0;
		}
		if (pos->z > 1200) {
		game->heli->SetPosition(glm::vec3(pos->x, pos->y, 1200));
		ship_velocity[2] = 0;
		}
		if (pos->x > 1200) {
		game->heli->SetPosition(glm::vec3(1200, pos->y, pos->z));
		ship_velocity[0] = 0;
		}
		if (pos->x < 0) {
		game->heli->SetPosition(glm::vec3(0, pos->y, pos->z));
		ship_velocity[0] = 0;
		}
		if (pos->y < 0 ) {
		game->heli->SetPosition(glm::vec3(pos->x, -0, pos->z));
		ship_velocity[1] = 0;
		}
		if (pos->y > 350) {
		game->heli->SetPosition(glm::vec3(pos->x, 350, pos->z));
		ship_velocity[1] = 0;
		}




		if (pos->z < -50 || pos->z > 650) {
		game->camera_.SetPosition(glm::vec3(pos->x, pos->y, pos->z + (mov.z * -1.1f)));
		ship_velocity[2] = 0;
		}
		if (pos->x < -350 || pos->x > 350) {
		game->camera_.SetPosition(glm::vec3(pos->x + (mov.x * -1.1f), pos->y, pos->z));
		ship_velocity[0] = 0;
		}
		if (pos->y < -350 || pos->y > 350) {
		game->camera_.SetPosition(glm::vec3(pos->x, pos->y + (mov.y * -1.1f), pos->z));
		ship_velocity[1] = 0;
		}
		*/


		//game->camera_.Translate(game->heli->GetForward()*trans_factor*ship_velocity[2] * -1.0f);
		//game->camera_.Translate(-game->heli->GetSide()*trans_factor*ship_velocity[0] * -1.0f);
		//game->camera_.Translate(glm::vec3(0.0, 1.0, 0.0)*trans_factor*ship_velocity[1]);
		//game->heli->SetPosition(game->camera_.GetPosition() + (game->camera_.GetForward() * offsetx) - (game->camera_.GetUp() * offsety));	

		game->heli->Translate(game->heli->GetForward()*trans_factor*ship_velocity[2] * -1.0f);
		game->heli->Translate(-game->heli->GetSide()*trans_factor*ship_velocity[0] * -1.0f);
		game->heli->Translate(glm::vec3(0.0, 1.0, 0.0)*trans_factor*ship_velocity[1]);

		std::vector<glm::vec3> explodingMissilePrevPosVector;

		for (int ii = 0; ii < explodingMissiles.size(); ii++) {
			explodingMissilePrevPosVector.push_back(explodingMissiles[ii]->GetPosition());
			if (explodingMissiles[ii]->GetVisible()) {
				if (explodingMissiles[ii]->GetPosition().x < worldXmin || explodingMissiles[ii]->GetPosition().z < worldZmin || explodingMissiles[ii]->GetPosition().x > worldXmax ||
					explodingMissiles[ii]->GetPosition().z > worldZmax || explodingMissiles[ii]->GetPosition().y > 350 || explodingMissiles[ii]->GetPosition().y < -20) {
					explodingMissiles[ii]->SetVisible(false);
				}
				else {
					explodingMissiles[ii]->SetPosition(explodingMissiles[ii]->GetPosition() + glm::normalize(explodingMissiles[ii]->direction) * 5.0f);
				}
			}
		}


		SceneNode* collidedBuilding = NULL;
		glm::vec3 heliPos = heli->GetPosition();
		bool heliCollided = false;
		for (int i = 0; i < buildings.size(); i++) {
			if (PointBoxCollision(heliPos, buildings[i]->boundingBox) && heliCollided == false) {
				collidedBuilding = buildings[i];
				HeliBuildingCollision(collidedBuilding, prevpos);
				heliCollided = true;
			}
			for (int j = 0; j < explodingMissiles.size(); j++)
				if (explodingMissiles[j]->GetVisible() && i < buildings.size()) {
					if (PointBoxCollision(explodingMissiles[j]->GetPosition(), buildings[i]->boundingBox)) {
						collidedBuilding = buildings[i];
						MissileBuidlingCollision(collidedBuilding, explodingMissiles[j], explodingMissilePrevPosVector[j]);
					}
				}
		}

		/*
		box : 0 min x max y max z
		box : 1 max x max y max z *allmaxes*
		box : 2 min x max y min z
		box : 3 max x max y min z
		box : 4 min x min y max z
		box : 5 max x min y max z
		box : 6 min x min y min z *all mins*
		box : 7 max x min y min z
		*/

		game->camera_.Translate(game->heli->GetForward()*trans_factor*ship_velocity[2] * -1.0f);
		game->camera_.Translate(-game->heli->GetSide()*trans_factor*ship_velocity[0] * -1.0f);
		game->camera_.Translate(glm::vec3(0.0, 1.0, 0.0)*trans_factor*ship_velocity[1]);

		std::cout << glm::normalize(glm::dot(glm::vec3(1.0, 4.0, 8.0), glm::vec3(-1.0, -4.0, -8.0))) << std::endl;

		game->camera_.SetPosition(heli->GetPosition() - camera_.GetForward() * offsetx + camera_.GetUp() * offsety);

		if (game->input_c == true || game->input_m2 == true) {
			lazerref->SetVisible(true);
			lazerref->SetPosition(this->heli->GetPosition()/* + this->heli->GetForward() /* -45.0f/* + game->camera_.GetUp()*((float)-0.1)*/);
			lazerref->SetOrientation(-this->heli->GetOrientation());
			for (int i = 0; i < childlasers.size(); ++i) {
				if (hostcollected[i]) {
					childlasers[i]->SetVisible(true);
					childlasers[i]->SetPosition(hostages[i]->GetPosition()/* + hostages[i]->GetForward() * 45.0f/* + game->camera_.GetUp()*((float)-0.1)*/);
					childlasers[i]->SetOrientation(hostages[i]->GetOrientation());
				}
			}
			checkForCollisions(window, true);
		}
		else {
			lazerref->SetVisible(false);
			for (int i = 0; i < childlasers.size(); ++i) {
				childlasers[i]->SetVisible(false);
			}
		}
		if (game->input_m == true || game->input_m3 == true) {
			if (missileTimer < 0.0f) {
				CreateMissileInstance("missile", "LaserMesh", "ObjectMaterial");
				missileTimer = missileFireRate;
			}
		}
		if (game->input_m == true || game->input_m1 == true) {
			if (ticker % 5 == 0)
				CreateBulletInstance("bullet", "MissileParticle", "MissileMaterial");
		}
		for (int i = 0; i < missiles.size(); i++) {
			if (missiles[i]->GetPosition().x < worldXmin || missiles[i]->GetPosition().z < worldZmin || missiles[i]->GetPosition().x > worldXmax || missiles[i]->GetPosition().z > worldZmax || missiles[i]->GetPosition().y > 350 || missiles[i]->GetPosition().y < 0)
				missiles[i]->SetVisible(false);
			else
				missiles[i]->SetPosition(missiles[i]->GetPosition() + glm::normalize(missiles[i]->direction) * 5.0f);
		}
		for (int i = 0; i < enemymissiles.size(); i++) {
			if (enemymissiles[i]->GetPosition().x < worldXmin || enemymissiles[i]->GetPosition().z < worldZmin || enemymissiles[i]->GetPosition().x > worldXmax || enemymissiles[i]->GetPosition().z > worldZmax || enemymissiles[i]->GetPosition().y > 350 || enemymissiles[i]->GetPosition().y < 0)
				enemymissiles[i]->SetVisible(false);
			else
				enemymissiles[i]->SetPosition(enemymissiles[i]->GetPosition() + glm::normalize(enemymissiles[i]->direction)*3.0f);
		}
		for (int i = 0; i < childmissiles.size(); i++) {
			if (hostcollected[i]) {
				for (int j = 0; j < childmissiles[i].size(); j++) {
					if (childmissiles[i][j]->GetPosition().x < worldXmin || childmissiles[i][j]->GetPosition().z < worldZmin || childmissiles[i][j]->GetPosition().x > worldXmax || childmissiles[i][j]->GetPosition().z > worldZmax || childmissiles[i][j]->GetPosition().y > 350 || childmissiles[i][j]->GetPosition().y < 0)
						childmissiles[i][j]->SetVisible(false);
					else
						childmissiles[i][j]->SetPosition(childmissiles[i][j]->GetPosition() + glm::normalize(childmissiles[i][j]->direction) * 5.0f);
				}
			}
		}

		for (int i = 0; i < enemies.size(); ++i) {
			if (enemies[i]->Shoot()) {
				CreateEnemyMissile("enemymissile", "LaserMesh", "ObjectMaterial", enemies[i]);
			}
		}


		cameraNode->SetOrientation(game->camera_.GetOrientation());
		cameraNode->SetPosition(game->camera_.GetPosition());

		int uncollectedHostages = 0;
		for (int i = 0; i < hostages.size(); i++) {
			if (hostcollected[i] == false)
				uncollectedHostages++;
		}
		if (uncollectedHostages < 4) {
			SpawnRandomHostage();
		}

		PrintVec3(heli->GetPosition());
		checkForCollisions(window, false);
	}

	void Game::HeliBuildingCollision(SceneNode* collidedBuilding, glm::vec3 prevpos) {

		for (int sideCounter = 0; sideCounter < 6; sideCounter++)
			if (collidedBuilding != NULL) {
				glm::vec3* box = collidedBuilding->boundingBox;
				glm::vec3* possibleIntersection;
				for (int once = 0; once < 1; once++) {
					possibleIntersection = LinePlaneCollision(glm::vec3(1.0, 0.0, 0.0), box[1], heli->GetPosition(), prevpos);
					if (possibleIntersection != NULL) {
						if (possibleIntersection->x >= heli->GetPosition().x && possibleIntersection->x <= prevpos.x &&
							possibleIntersection->y <= box[1].y && possibleIntersection->z <= box[1].z &&
							possibleIntersection->y >= box[6].y && possibleIntersection->z >= box[6].z) {
							ship_velocity[0] = 0.0;
							heli->SetPosition(glm::vec3(box[1].x, heli->GetPosition().y, heli->GetPosition().z));
							continue;
						}
					}
					possibleIntersection = LinePlaneCollision(glm::vec3(-1.0, 0.0, 0.0), box[6], heli->GetPosition(), prevpos);
					if (possibleIntersection != NULL) {
						if (possibleIntersection->x <= heli->GetPosition().x && possibleIntersection->x >= prevpos.x &&
							possibleIntersection->y <= box[1].y && possibleIntersection->z <= box[1].z &&
							possibleIntersection->y >= box[6].y && possibleIntersection->z >= box[6].z) {
							ship_velocity[0] = 0.0;
							heli->SetPosition(glm::vec3(box[6].x, heli->GetPosition().y, heli->GetPosition().z));
							continue;
						}
					}
					possibleIntersection = LinePlaneCollision(glm::vec3(0.0, 1.0, 0.0), box[1], heli->GetPosition(), prevpos);
					if (possibleIntersection != NULL) {
						if (possibleIntersection->y >= heli->GetPosition().y && possibleIntersection->y <= prevpos.y &&
							possibleIntersection->x <= box[1].x && possibleIntersection->z <= box[1].z &&
							possibleIntersection->x >= box[6].x && possibleIntersection->z >= box[6].z) {
							ship_velocity[1] = 0.0;
							heli->SetPosition(glm::vec3(heli->GetPosition().x, box[1].y, heli->GetPosition().z));
							continue;
						}
					}
					possibleIntersection = LinePlaneCollision(glm::vec3(0.0, -1.0, 0.0), box[6], heli->GetPosition(), prevpos);
					if (possibleIntersection != NULL) {
						if (possibleIntersection->y <= heli->GetPosition().y && possibleIntersection->y >= prevpos.x &&
							possibleIntersection->x <= box[1].x && possibleIntersection->z <= box[1].z &&
							possibleIntersection->x >= box[6].x && possibleIntersection->z >= box[6].z) {
							ship_velocity[1] = 0.0;
							heli->SetPosition(glm::vec3(heli->GetPosition().x, box[6].y, heli->GetPosition().z));
							continue;
						}
					}
					possibleIntersection = LinePlaneCollision(glm::vec3(0.0, 0.0, 1.0), box[1], heli->GetPosition(), prevpos);
					if (possibleIntersection != NULL) {
						if (possibleIntersection->z >= heli->GetPosition().z && possibleIntersection->z <= prevpos.z &&
							possibleIntersection->y <= box[1].y && possibleIntersection->x < box[1].x &&
							possibleIntersection->y >= box[6].y && possibleIntersection->x > box[6].x) {
							ship_velocity[2] = 0.0;
							heli->SetPosition(glm::vec3(heli->GetPosition().x, heli->GetPosition().y, box[1].z));
							continue;
						}
					}
					possibleIntersection = LinePlaneCollision(glm::vec3(0.0, 0.0, -1.0), box[6], heli->GetPosition(), prevpos);
					if (possibleIntersection != NULL) {
						if (possibleIntersection->z <= heli->GetPosition().z && possibleIntersection->z >= prevpos.z &&
							possibleIntersection->y <= box[1].y && possibleIntersection->x <= box[1].x &&
							possibleIntersection->y >= box[6].y && possibleIntersection->x >= box[6].x) {
							ship_velocity[2] = 0.0;
							heli->SetPosition(glm::vec3(heli->GetPosition().x, heli->GetPosition().y, box[6].z));
							continue;
						}
					}
				}
			}
	}



	void Game::MissileBuidlingCollision(SceneNode* collidedBuilding, SceneNode* miss, glm::vec3 prevpos) {

		for (int sideCounter = 0; sideCounter < 6; sideCounter++)
			if (collidedBuilding != NULL) {
				glm::vec3* box = collidedBuilding->boundingBox;
				glm::vec3* possibleIntersection = NULL;
				for (int once = 0; once < 1; once++) {
					possibleIntersection = LinePlaneCollision(glm::vec3(1.0, 0.0, 0.0), box[1], miss->GetPosition(), prevpos);
					if (possibleIntersection != NULL) {
						if (possibleIntersection->x >= miss->GetPosition().x && possibleIntersection->x <= prevpos.x &&
							possibleIntersection->y <= box[1].y && possibleIntersection->z <= box[1].z &&
							possibleIntersection->y >= box[6].y && possibleIntersection->z >= box[6].z) {
							continue;
						}
					}
					possibleIntersection = LinePlaneCollision(glm::vec3(-1.0, 0.0, 0.0), box[6], miss->GetPosition(), prevpos);
					if (possibleIntersection != NULL) {
						if (possibleIntersection->x <= miss->GetPosition().x && possibleIntersection->x >= prevpos.x &&
							possibleIntersection->y <= box[1].y && possibleIntersection->z <= box[1].z &&
							possibleIntersection->y >= box[6].y && possibleIntersection->z >= box[6].z) {
							miss->SetPosition(glm::vec3(box[6].x, miss->GetPosition().y, miss->GetPosition().z));
							continue;
						}
					}
					possibleIntersection = LinePlaneCollision(glm::vec3(0.0, 1.0, 0.0), box[1], miss->GetPosition(), prevpos);
					if (possibleIntersection != NULL) {
						if (possibleIntersection->y >= miss->GetPosition().y && possibleIntersection->y <= prevpos.y &&
							possibleIntersection->x <= box[1].x && possibleIntersection->z <= box[1].z &&
							possibleIntersection->x >= box[6].x && possibleIntersection->z >= box[6].z) {
							continue;
						}
					}
					possibleIntersection = LinePlaneCollision(glm::vec3(0.0, -1.0, 0.0), box[6], miss->GetPosition(), prevpos);
					if (possibleIntersection != NULL) {
						if (possibleIntersection->y <= miss->GetPosition().y && possibleIntersection->y >= prevpos.x &&
							possibleIntersection->x <= box[1].x && possibleIntersection->z <= box[1].z &&
							possibleIntersection->x >= box[6].x && possibleIntersection->z >= box[6].z) {
							continue;
						}
					}
					possibleIntersection = LinePlaneCollision(glm::vec3(0.0, 0.0, 1.0), box[1], miss->GetPosition(), prevpos);
					if (possibleIntersection != NULL) {
						if (possibleIntersection->z >= miss->GetPosition().z && possibleIntersection->z <= prevpos.z &&
							possibleIntersection->y <= box[1].y && possibleIntersection->x < box[1].x &&
							possibleIntersection->y >= box[6].y && possibleIntersection->x > box[6].x) {
							continue;
						}
					}
					possibleIntersection = LinePlaneCollision(glm::vec3(0.0, 0.0, -1.0), box[6], miss->GetPosition(), prevpos);
					if (possibleIntersection != NULL) {
						if (possibleIntersection->z <= miss->GetPosition().z && possibleIntersection->z >= prevpos.z &&
							possibleIntersection->y <= box[1].y && possibleIntersection->x <= box[1].x &&
							possibleIntersection->y >= box[6].y && possibleIntersection->x >= box[6].x) {
							continue;
						}
					}
					possibleIntersection = NULL;
				}
				if (possibleIntersection != NULL) {
					CreateExplosionSphere(*possibleIntersection);
					miss->SetVisible(false);
					return;
				}
			}
	}

	void Game::ScrollWheelCallback(GLFWwindow* window, double xoffset, double yoffset) {
		void* ptr = glfwGetWindowUserPointer(window);
		Game *game = (Game *)ptr;
		//std::cout << "xoffset: " << xoffset << " yoffset: " << yoffset << std::endl;

		//glm::vec2 offsetVec2 = glm::vec2(game->offsetx, game->offsety);
		//glm::vec3 offsetVec = game->camera_.GetForward() * game->offsetx + game->camera_.GetUp() * game->offsety;
		//glm::quat offsetQuat = glm::rotate(game->camera_.GetOrientation(), -0.05f * (float)yoffset, game->camera_.GetSide());

		//offsetVec = offsetVec * offsetQuat;
		//game->offsetx = offsetVec2.x;
		//game->offsety = offsetVec2.y;
		//std::cout << game->offsetx << " : " << game->offsety << std::endl;
		//float offx = game->offsetx * cos(0.05 * yoffset) - game->offsety * sin(0.05 * yoffset);
		//float offy = game->offsety * cos(0.05 * yoffset) + game->offsetx * sin(0.05 * yoffset);

		//game->offsetx = offx;
		//game->offsety = offy;

		//std::cout << game->offsetx << " : " << game->offsety << std::endl;
		game->camera_.Translate((game->camera_.GetForward() * game->offsetx) - (game->camera_.GetUp() * game->offsety));

		game->camera_.Rotate(glm::angleAxis(-0.05f * (float)yoffset, game->camera_.GetSide()));
		//game->PrintVec3(game->camera_.GetForward());

		game->camera_.Translate((game->camera_.GetForward() * (game->offsetx * -1.0f)) - (game->camera_.GetUp() * game->offsety) * -1.0f);


	}

	void Game::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
		void* ptr = glfwGetWindowUserPointer(window);
		Game *game = (Game *)ptr;

		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
			game->input_m1 = true;
		}
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
			game->input_m1 = false;
		}
		if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
			game->input_m2 = true;
		}
		if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
			game->input_m2 = false;
		}
		if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS) {
			game->input_m3 = true;
		}
		if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_RELEASE) {
			game->input_m3 = false;
		}
	}

	void Game::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {

		// Get user data with a pointer to the game class
		void* ptr = glfwGetWindowUserPointer(window);
		Game *game = (Game *)ptr;

		// Quit game if 'q' is pressed
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
			glfwSetWindowShouldClose(window, true);
		}

		// Ship control
		float rot_factor(glm::pi<float>() / 180);
		float trans_factor = 1.0;

		if (key == GLFW_KEY_B && action == GLFW_PRESS) {
			game->input_b = true;
		}
		else if (key == GLFW_KEY_B && action == GLFW_RELEASE) {
			game->input_b = false;
		}
		if (key == GLFW_KEY_UP && action == GLFW_PRESS) {
			game->input_up = true;
		}
		else if (key == GLFW_KEY_UP && action == GLFW_RELEASE) {
			game->input_up = false;
		}
		if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) {
			game->input_down = true;
		}
		else if (key == GLFW_KEY_DOWN && action == GLFW_RELEASE) {
			game->input_down = false;
		}
		if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
			game->input_left = true;
		}
		else if (key == GLFW_KEY_LEFT && action == GLFW_RELEASE) {
			game->input_left = false;
		}
		if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
			game->input_right = true;
		}
		else if (key == GLFW_KEY_RIGHT && action == GLFW_RELEASE) {
			game->input_right = false;
		}
		if (key == GLFW_KEY_S && action == GLFW_PRESS) {
			game->input_s = true;
		}
		else if (key == GLFW_KEY_S && action == GLFW_RELEASE) {
			game->input_s = false;
		}
		if (key == GLFW_KEY_X && action == GLFW_PRESS) {
			game->input_x = true;
		}
		else if (key == GLFW_KEY_X && action == GLFW_RELEASE) {
			game->input_x = false;
		}
		if (key == GLFW_KEY_A && action == GLFW_PRESS) {
			game->input_a = true;
		}
		else if (key == GLFW_KEY_A && action == GLFW_RELEASE) {
			game->input_a = false;
		}
		if (key == GLFW_KEY_W && action == GLFW_PRESS) {
			game->input_w = true;
		}
		else if (key == GLFW_KEY_W && action == GLFW_RELEASE) {
			game->input_w = false;
		}
		if (key == GLFW_KEY_D && action == GLFW_PRESS) {
			game->input_d = true;
		}
		else if (key == GLFW_KEY_D && action == GLFW_RELEASE) {
			game->input_d = false;
		}
		if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
			game->input_z = true;
		}
		else if (key == GLFW_KEY_Z && action == GLFW_RELEASE) {
			game->input_z = false;
		}
		if (key == GLFW_KEY_J && action == GLFW_PRESS) {
			game->input_j = true;
		}
		else if (key == GLFW_KEY_J && action == GLFW_RELEASE) {
			game->input_j = false;
		}
		if (key == GLFW_KEY_L && action == GLFW_PRESS) {
			game->input_l = true;
		}
		else if (key == GLFW_KEY_L && action == GLFW_RELEASE) {
			game->input_l = false;
		}
		if (key == GLFW_KEY_I && action == GLFW_PRESS) {
			game->input_i = true;
		}
		else if (key == GLFW_KEY_I && action == GLFW_RELEASE) {
			game->input_i = false;
		}
		if (key == GLFW_KEY_K && action == GLFW_PRESS) {
			game->input_k = true;
		}
		else if (key == GLFW_KEY_K && action == GLFW_RELEASE) {
			game->input_k = false;
		}
		if (key == GLFW_KEY_C && action == GLFW_PRESS) {
			game->input_c = true;
		}
		else if (key == GLFW_KEY_C && action == GLFW_RELEASE) {
			game->input_c = false;
		}
		if (key == GLFW_KEY_M && action == GLFW_PRESS) {
			game->input_m = true;
		}
		else if (key == GLFW_KEY_M && action == GLFW_RELEASE) {
			game->input_m = false;
		}
		if (key == GLFW_KEY_T && action == GLFW_PRESS) {
			game->input_t = true;
		}
		else if (key == GLFW_KEY_T && action == GLFW_RELEASE) {
			game->input_t = false;
		}
		if (key == GLFW_KEY_Q && action == GLFW_PRESS) {
			game->input_q = true;
		}
		if (key == GLFW_KEY_Q && action == GLFW_RELEASE) {
			game->input_q = false;
		}
		if (key == GLFW_KEY_E && action == GLFW_PRESS) {
			game->input_e = true;
		}
		if (key == GLFW_KEY_E && action == GLFW_RELEASE) {
			game->input_e = false;
		}
		if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
			game->input_space = true;
		}
		if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE) {
			game->input_space = false;
		}
		if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_PRESS) {
			game->input_shift = true;
		}
		if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_RELEASE) {
			game->input_shift = false;
		}
	}


	void Game::ResizeCallback(GLFWwindow* window, int width, int height) {

		// Set up viewport and camera projection based on new window size
		glViewport(0, 0, width, height);
		void* ptr = glfwGetWindowUserPointer(window);
		Game *game = (Game *)ptr;
		game->camera_.SetProjection(camera_fov_g, camera_near_clip_distance_g, camera_far_clip_distance_g, width, height);
	}


	Game::~Game() {

		glfwTerminate();
	}


	void Game::SetupHelicopter(int enemyNumber, Enemy** enemy) {
		if (enemyNumber == 0) {
			heli = (Helicopter *)CreateTexturedHeliInstance("heli", "CubeMesh", "textureMaterial", "Root", "Camo");
			heli->Scale(glm::vec3(0.1, 0.1, 0.1));
			body = (Helicopter *)CreateTexturedInstance("body", "HeliBodyMesh", "textureMaterial", "heli", "Camo");
			body->Rotate(glm::angleAxis(3.14159f, glm::vec3(0.0, 1.0, 0.0)));

			SceneNode *particles = CreateInstance("particles", "MissileParticles", "MissileMaterial", "heli");
			particles->SetVisible(true);
			particles->Scale(glm::vec3(10.0));

			rotorStock = CreateTexturedInstance("rotorStock", "HeliStockRotorMesh", "textureMaterial", "body", "Camo");
			rotorStock->Translate(glm::vec3(0.0, 1.1, 0.0));
			rotor2 = CreateInstance("rotor2", "CylinderMesh", "ShinyBlueMaterial", "rotorStock");
			rotor3 = CreateInstance("rotor3", "HeliTailRotorMesh", "ShinyBlueMaterial", "body");
			glm::quat rotation = glm::angleAxis((float)4.709, glm::vec3(1.0, 0.0, 0.0));
			rotation = glm::angleAxis(3.3f, glm::vec3(0.0, 1.0, 0.0));
			rotor2->Scale(glm::vec3(1.0, 0.2, 1.5));
			rotor2->SetAngM(glm::quat(glm::angleAxis((float) 1.0, glm::vec3(0.0, 1.0, 0.0))));
			rotor3->Translate(glm::vec3(0.1, 0.8, 5.6));
			rotor3->SetAngM(glm::quat(glm::angleAxis((float) 1.0, glm::vec3(1.0, 0.0, 0.0))));
		}
		else if (enemy != NULL) {

			std::stringstream ss;
			ss << enemyNumber;
			std::string index = ss.str();

			*enemy = CreateTexturedEnemyHeliInstance("enemyHeli" + index, "CylinderMesh", "textureMaterial", "Root", "Camo");
			(*enemy)->SetScale(glm::vec3(0.1, 0.1, 0.1));
			SceneNode* enemyBody = CreateTexturedInstance("enemyHeliBody" + index, "HeliBodyMesh", "textureMaterial", "enemyHeli" + index, "Camo");
			enemyBody->Rotate(glm::angleAxis(3.14159f, glm::vec3(0.0, 1.0, 0.0)));

			SceneNode *particles = CreateInstance("particles", "MissileParticles", "MissileMaterial", "heli");
			particles->SetVisible(true);
			particles->Scale(glm::vec3(10.0));

			SceneNode* enemyRotorStock = CreateTexturedInstance("enemyHeliRotorStock" + index, "HeliStockRotorMesh", "textureMaterial", "enemyHeliBody" + index, "Camo");
			enemyRotorStock->Translate(glm::vec3(0.0, 1.1, 0.0));
			SceneNode* enemyRotor2 = CreateInstance("enemyHeliRotor2" + index, "CylinderMesh", "ShinyBlueMaterial", "enemyHeliRotorStock" + index);
			SceneNode* enemyRotor3 = CreateInstance("enemyHeliRotor3" + index, "HeliTailRotorMesh", "ShinyBlueMaterial", "enemyHeliBody" + index);
			glm::quat rotation = glm::angleAxis((float)4.709, glm::vec3(1.0, 0.0, 0.0));
			rotation = glm::angleAxis(3.3f, glm::vec3(0.0, 1.0, 0.0));
			enemyRotor2->Scale(glm::vec3(1.0, 0.2, 1.5));
			enemyRotor2->SetAngM(glm::quat(glm::angleAxis((float) 1.0, glm::vec3(0.0, 1.0, 0.0))));
			enemyRotor3->Translate(glm::vec3(0.1, 0.8, 5.6));
			enemyRotor3->SetAngM(glm::quat(glm::angleAxis((float) 1.0, glm::vec3(1.0, 0.0, 0.0))));
		}
	}

	void Game::SetupHelicopterOld(void) {

		heli = CreateHeliInstance("heli", "CubeMesh", "ToonHeliMaterial", "Root");
		heli->SetPosition(glm::vec3(600.0, 0.0, 0.0));
		body2 = CreateInstance("body2", "CubeMesh", "ToonHeliMaterial", "heli");

		rotor1 = CreateInstance("rotor1", "CylinderMesh", "ToonHeliMaterial", "body2");
		rotor2 = CreateInstance("rotor2", "CylinderMesh", "ToonHeliMaterial", "rotor1");
		tail = CreateInstance("tail", "CylinderMesh", "ToonHeliMaterial", "heli");
		rotor3 = CreateInstance("rotor3", "CylinderMesh", "ToonHeliMaterial", "tail");
		heli->Rotate(glm::angleAxis((float) 3.14, glm::vec3(0.0, 1.0, 0.0)));
		heli->Translate(glm::vec3(0.0, 0.0, 0.0));
		body2->Translate(glm::vec3(0.0, 0.3, -0.25));
		heli->Scale(glm::vec3(1.0, 0.4, 3.5));
		body2->Scale(glm::vec3(1.0, 0.3, 3.0));
		glm::quat rotation = glm::angleAxis((float)4.709, glm::vec3(1.0, 0.0, 0.0));

		rotor1->SetOrientation(rotation);
		rotor1->Translate(glm::vec3(0.0, 0.4 - 0.175, 0.0));
		rotor1->Scale(glm::vec3(1.0, 1.0, 0.1));
		//rotation = glm::angleAxis(3.3f, glm::vec3(0.0, 1.0, 0.0));
		rotor2->Translate(glm::vec3(0.0, 0.0, 0.1));
		rotor2->SetOrientation(rotation);
		rotor2->Scale(glm::vec3(1.0, 0.2, 1.5));
		rotor2->SetAngM(glm::quat(glm::angleAxis((float) 0.5, glm::vec3(0.0, 1.0, 0.0))));
		tail->Scale(glm::vec3(0.8, 0.6, 1.24));
		tail->Translate(glm::vec3(0.0, 0.1, -1.5));
		rotation = glm::angleAxis(8.0f, glm::vec3(1.0, 0.0, 0.0));
		rotor3->Rotate(rotation);
		rotor3->Scale(glm::vec3(0.2, .75, 0.4));
		rotor3->Translate(glm::vec3(0.0, 0.0, -1.5));
		rotor3->SetAngM(glm::quat(glm::angleAxis((float) 1.0, glm::vec3(1.0, 0.0, 0.0))));

	}

	void Game::SetupHostage(std::string name, SceneNode** captors, glm::vec3 p) {

		Helicopter* host = CreateHeliInstance(name, "SimpleSphereMesh", "ToonHeliMaterial", "Root");
		host->SetBlending(false);
		SceneNode* nose = (Helicopter *)CreateInstance(name + " nose", "SimpleSphereMesh", "ToonHeliMaterial", name);
		SceneNode* rot1 = CreateInstance(name + " rotor1", "CylinderMesh", "ToonHeliMaterial", name);
		SceneNode* rot2 = CreateInstance(name + " rotor2", "CylinderMesh", "ToonHeliMaterial", name + " rotor1");
		SceneNode* tale = CreateInstance(name + " tail", "CylinderMesh", "ToonHeliMaterial", name);
		SceneNode* rot3 = CreateInstance(name + " rotor3", "CylinderMesh", "ToonHeliMaterial", name + " tail");
		//host->Rotate(glm::angleAxis((float) 180.0, glm::vec3(0.0, 1.0, 0.0)));
		host->SetPosition(p);
		host->Scale(glm::vec3(0.5, 0.5, 0.6));
		nose->Scale(glm::vec3(0.3, 0.2, 0.2));
		nose->Translate(glm::vec3(0.0, -0.15, 0.5));
		glm::quat rotation = glm::angleAxis((float)4.709, glm::vec3(1.0, 0.0, 0.0));

		rot1->SetOrientation(rotation);
		rot1->Translate(glm::vec3(0.0, 0.5, 0.0));
		rot1->Scale(glm::vec3(0.5, 0.5, 0.1));
		//rotation = glm::angleAxis(3.3f, glm::vec3(0.0, 1.0, 0.0));
		rot2->Translate(glm::vec3(0.0, 0.0, 0.1));
		rot2->SetOrientation(rotation);
		rot2->Scale(glm::vec3(0.8, 0.15, 1.25));
		rot2->SetAngM(glm::quat(glm::angleAxis((float) 0.5, glm::vec3(0.0, 1.0, 0.0))));
		tale->Scale(glm::vec3(0.8, 0.4, 0.4));
		tale->Translate(glm::vec3(0.0, 0.1, -0.8));
		rotation = glm::angleAxis(8.0f, glm::vec3(1.0, 0.0, 0.0));
		rot3->Rotate(rotation);
		rot3->Scale(glm::vec3(0.15, 0.4, 0.3));
		rot3->Translate(glm::vec3(0.0, 0.0, -0.6));
		rot3->SetAngM(glm::quat(glm::angleAxis((float) 1.0, glm::vec3(1.0, 0.0, 0.0))));

		host->SetHostage(true);
		host->SetCaptors(captors);
		host->SetFreedom(false);

		//SPLINE
		// Create particles
		SceneNode *splineparticle = CreateInstance(name + "SplineParticles", "TorusParticles", "SplineMaterial", "Root");
		// Set blending of particles
		splineparticle->SetBlending(true);
		splineparticle->SetParticle(true);
		host->AddChild(splineparticle);

		// Assign control points
		Resource *cp = resman_.GetResource("ControlPoints");
		splineparticle->AddShaderAttribute("control_point", Vec3Type, cp->GetSize(), cp->GetData());


		hostages.push_back(host);
		hostcollected.push_back(false);
		childmissiles.push_back(std::deque<SceneNode*>());

	}

	void Game::SetupWorld() {
		int sizeOfBuildingArea = sqrt((worldXmax - worldXmin) * (worldZmax - worldZmin));
		bool** table = (bool**)calloc(sizeof(bool*), worldXmax);
		for (int i = 0; i < worldXmax; i++) {
			table[i] = (bool*)calloc(sizeof(bool), worldZmax);
		}
		//test = CreateInstance("test", "CubeMesh", "ToonRingMaterial", "Root");

		//test->SetPosition(glm::vec3(0.0, 0.0, 0.0));

		SceneNode* ground = CreateTexturedInstance("cubeg", "CubeMesh", "textureMaterial", "Root","Ground");
		ground->SetPosition(glm::vec3(worldXmax / 2, -5, worldZmax / 2));
		ground->Scale(glm::vec3(worldXmax, 10, worldZmax));

		SceneNode* b;
		std::stack<bool *> occupiedArea;
		glm::vec3* vertices;
		glm::vec3 scaleFactor;
		for (int i = 0; i < sizeOfBuildingArea / 5; i++) {
			b = CreateTexturedInstance("EnvironmentCube" + std::to_string(i), "CubeMesh", "textureMaterial", "Root", "Building");
			scaleFactor = glm::vec3((float)(4.0 + ((float)(rand() % 20))), (float)(4.0 + ((float)(rand() % 20))), (float)(4.0 + ((float)(rand() % 20))));
			float factor = (float)(1.0 + ((float)(rand() % 20)));
			b->SetPosition(glm::vec3((float)(rand() % (worldXmax - 50)) + 50.0f, scaleFactor.y / 2.0f, (float)(rand() % (worldZmax - 50)) + 50.0f));
			bool check = false;
			int patience = 0;
			bool yes = true;
			while (yes) {
				for (int k = 0 - (int)(scaleFactor.x / 2.0f); k < (int)scaleFactor.x; k++) {
					for (int j = 0 - (int)(scaleFactor.z / 2.0f); j < (int)scaleFactor.z; j++) {
						int x = (int)b->GetPosition().x + k;
						int z = (int)b->GetPosition().z + j;
						if (x < worldXmax && z < worldZmax && x >= worldXmin && z >= worldZmin)
							if (table[x][z] == true)
								check = true;
							else
								occupiedArea.push(&table[x][z]);
					}
					if (check)
						break;
				}
				if (check) {
					while (occupiedArea.size() > 0) {
						occupiedArea.pop();
					}
					if ((patience % 30) == 29) {
						scaleFactor = glm::vec3((float)(1.0 + ((float)(rand() % 20))), (float)(1.0 + ((float)(rand() % 20))), (float)(1.0 + ((float)(rand() % 20))));
					}
					else
						patience++;
					b->SetPosition(glm::vec3((float)(rand() % 300), scaleFactor.y / 2.0f, (float)(rand() % 300)));
					check = false;
				}
				else
					yes = false;
			}
			while (occupiedArea.size() > 0) {
				*occupiedArea.top() = true;
				occupiedArea.pop();
			}
			b->Scale(scaleFactor);

			vertices = new glm::vec3[8];
			glm::vec3 top = glm::vec3(0.0, 1.0, 0.0) * (scaleFactor.y / 2.0f);
			glm::vec3 bot = glm::vec3(0.0, 1.0, 0.0) * (scaleFactor.y / 2.0f) * -1.0f;
			glm::vec3 left = glm::vec3(1.0, 0.0, 0.0) * (scaleFactor.x / 2.0f);
			glm::vec3 right = glm::vec3(1.0, 0.0, 0.0) * (scaleFactor.x / 2.0f) * -1.0f;
			glm::vec3 front = glm::vec3(0.0, 0.0, 1.0) * (scaleFactor.z / 2.0f);
			glm::vec3 back = glm::vec3(0.0, 0.0, 1.0) * (scaleFactor.z / 2.0f) * -1.0f;

			vertices[0] = b->GetPosition() + top + front + right;
			vertices[1] = b->GetPosition() + top + front + left;
			vertices[2] = b->GetPosition() + top + back + right;
			vertices[3] = b->GetPosition() + top + back + left;
			vertices[4] = b->GetPosition() + bot + front + right;
			vertices[5] = b->GetPosition() + bot + front + left;
			vertices[6] = b->GetPosition() + bot + back + right;
			vertices[7] = b->GetPosition() + bot + back + left;

			for (int roofCorners = 0; roofCorners < 4; roofCorners++)
				spawnPoints.push_back(vertices);

			b->SetBoundingBox(vertices);
			if (i < 0)
				for (int a = 0; a < 8; a++) {
					std::stringstream ss;
					ss << i;
					std::string index = ss.str();
					std::string name = "CornerInstance" + index;
					SceneNode* sphere = CreateAsteroidInstance(name, "SimpleSphereMesh", "ObjectMaterial");
					sphere->SetPosition(vertices[a]);
					sphere->SetScale(glm::vec3(1.0, 1.0, 1.0) * 1.5f);
					/*
					if (a % 2 == 1) {
					sphere->SetOrientation(glm::normalize(glm::angleAxis(glm::pi<float>()*((float)rand() / RAND_MAX), glm::vec3(((float)rand() / RAND_MAX), ((float)rand() / RAND_MAX), ((float)rand() / RAND_MAX)))));
					sphere->SetAngM(glm::normalize(glm::angleAxis(0.05f*glm::pi<float>()*((float)rand() / RAND_MAX), glm::vec3(((float)rand() / RAND_MAX), ((float)rand() / RAND_MAX), ((float)rand() / RAND_MAX)))));
					}
					*/
					scene_.GetNode("Root")->AddChild(sphere);
				}
			buildings.push_back(b);
		}

		vertices = new glm::vec3[8];
		glm::vec3 top = glm::vec3(0.0, 1.0, 0.0) * (0.0f / 2.0f);
		glm::vec3 bot = glm::vec3(0.0, 1.0, 0.0) * (-10.0f / 2.0f) * -1.0f;
		glm::vec3 left = glm::vec3(1.0, 0.0, 0.0) * ((float)worldXmax);
		glm::vec3 right = glm::vec3(1.0, 0.0, 0.0) * ((float)worldXmin) * -1.0f;
		glm::vec3 front = glm::vec3(0.0, 0.0, 1.0) * ((float)worldZmax);
		glm::vec3 back = glm::vec3(0.0, 0.0, 1.0) * ((float)worldZmax) * -1.0f;

		vertices[0] = ground->GetPosition() + top + front + right;
		vertices[1] = ground->GetPosition() + top + front + left;
		vertices[2] = ground->GetPosition() + top + back + right;
		vertices[3] = ground->GetPosition() + top + back + left;
		vertices[4] = ground->GetPosition() + bot + front + right;
		vertices[5] = ground->GetPosition() + bot + front + left;
		vertices[6] = ground->GetPosition() + bot + back + right;
		vertices[7] = ground->GetPosition() + bot + back + left;
		ground->SetBoundingBox(vertices);
		buildings.push_back(ground);

		for (int i = 0; i < 300; i++)
			delete(table[i]);
		delete(table);


	}

	void Game::SetupEnemies() {

		int location = rand() % spawnPoints.size();

		for (int i = 0; i < 4; i++) {



			std::stringstream ss;
			ss << i;
			std::string index = ss.str();
			std::string name = "StationaryEnemy" + index;

			Enemy* bad_dude = CreateEnemyInstance(name, "LaserMesh", "ObjectMaterial", 0);

			bad_dude->SetPosition(spawnPoints[location][i]);
			bad_dude->SetScale(glm::vec3(3.0, 3.0, 3.0));
			std::vector<SceneNode *>::const_iterator bad_child = bad_dude->children_begin();
			(*bad_child)->SetPosition(glm::vec3(0.0, 0.0, 0.0));
			(*bad_child)->SetScale(glm::vec3(1.0, 1.0, 1.0));

			spawnPoints.erase(spawnPoints.begin() + location);
			scene_.GetNode("Root")->AddChild(bad_dude);
			enemies.push_back(bad_dude);
		}
		SpawnTank(glm::vec3(30.0, 0.0, 30.0));



	}

	void Game::SpawnRandomHostage() {


		int location = rand() % spawnPoints.size();
		std::string unique;
		std::stringstream sss;
		sss << location;

		SceneNode** captors = (SceneNode**)calloc(sizeof(SceneNode*), 4);

		for (int i = 0; i < 4; i++) {

			std::stringstream ss;
			ss << i;
			std::string index = ss.str();
			std::string name = "StationaryEnemy" + sss.str() + index;

			Enemy* bad_dude = CreateEnemyInstance(name, "LaserMesh", "ObjectMaterial", 0);

			bad_dude->SetPosition(spawnPoints[location][i]);
			bad_dude->SetScale(glm::vec3(3.0, 3.0, 3.0));
			std::vector<SceneNode *>::const_iterator bad_child = bad_dude->children_begin();
			(*bad_child)->SetPosition(glm::vec3(0.0, 0.0, 0.0));
			(*bad_child)->SetScale(glm::vec3(1.0, 1.0, 1.0));

			//spawnPoints.erase(spawnPoints.begin() + location);
			scene_.GetNode("Root")->AddChild(bad_dude);
			captors[i] = bad_dude;
			enemies.push_back(bad_dude);
		}

		glm::vec3 hostageSpawn = glm::vec3((spawnPoints[location][0].x + spawnPoints[location][3].x) / 2.0f, spawnPoints[location][0].y + 1.0, (spawnPoints[location][0].z + spawnPoints[location][3].z) / 2.0);

		SetupHostage("Hostage" + sss.str(), captors, hostageSpawn);
	}



	void Game::SpawnEnemyHeli(glm::vec3 heliPos) {

		for (int i = 1; i < 1; i++) {



			Enemy* bad_dude;
			SetupHelicopter(i, &bad_dude);

			bad_dude->SetPosition(glm::vec3(500.0 + ((float)(rand() % 200)), 60.0, 500.0 + ((float)(rand() % 200))));
			bad_dude->SetScale(glm::vec3(0.1, 0.1, 0.1));

			enemies.push_back(bad_dude);
		}
	}

	void Game::SpawnTank(glm::vec3 tankPos){

		for (int i = 0; i < 1; i++) {

			std::stringstream ss;
			ss << i;
			std::string index = ss.str();
			std::string name = "TankTurret" + index;

			Enemy* bad_tank;
			setupTank(i, &bad_tank, (tankPos));
			bad_tank->SetScale(glm::vec3(0.1, 0.1, 0.1));


			enemies.push_back(bad_tank);

			Enemy* bad_dude = CreateEnemyInstance(name, "LaserMesh", "ObjectMaterial", 0);

			bad_dude->SetPosition(glm::vec3(0.0, 1.75f + 0.5f, 0.0));
			bad_dude->SetScale(glm::vec3(3.0, 3.0, 3.0));
			std::vector<SceneNode *>::const_iterator bad_child = bad_dude->children_begin();
			(*bad_child)->SetPosition(glm::vec3(0.0, 0.0, 0.0));
			(*bad_child)->SetScale(glm::vec3(1.0, 1.0, 1.0));

			bad_dude->SetParentEnemy(bad_tank);
			enemies.push_back(bad_dude);


		}

	}

	Enemy* Game::CreateEnemyInstance(std::string entity_name, std::string object_name, std::string material_name, int enemyType) {

		// Get resources

		if (enemyType == 0) {
			Resource *geom = resman_.GetResource(object_name);
			if (!geom) {
				throw(GameException(std::string("Could not find resource \"") + object_name + std::string("\"")));
			}

			Resource *mat = resman_.GetResource(material_name);
			if (!mat) {
				throw(GameException(std::string("Could not find resource \"") + material_name + std::string("\"")));
			}
			Resource* geom2 = resman_.GetResource("SimpleSphereMesh");
			if (!geom2) {
				throw(GameException(std::string("Could not find resource \"") + object_name + std::string("\"")));
			}
			Resource* mat2 = resman_.GetResource("ObjectMaterial");
			if (!mat2) {
				throw(GameException(std::string("Could not find resource \"") + material_name + std::string("\"")));
			}

			Enemy* enemy_temp = new Enemy(enemyType, entity_name, geom, resman_.GetResource("textureMaterial"), resman_.GetResource("Metal"), (SceneNode*)heli, &resman_);
			enemy_temp->AddChild(new SceneNode(entity_name + "(Base)", geom2, mat2, 0));
			return enemy_temp;
		}
		else if (enemyType == 1) {
			Resource *geom = resman_.GetResource(object_name);
			if (!geom) {
				throw(GameException(std::string("Could not find resource \"") + object_name + std::string("\"")));
			}

			Resource *mat = resman_.GetResource(material_name);
			if (!mat) {
				throw(GameException(std::string("Could not find resource \"") + material_name + std::string("\"")));
			}
			Resource* geom2 = resman_.GetResource("SimpleSphereMesh");
			if (!geom2) {
				throw(GameException(std::string("Could not find resource \"") + object_name + std::string("\"")));
			}
			Resource* mat2 = resman_.GetResource("ObjectMaterial");
			if (!mat2) {
				throw(GameException(std::string("Could not find resource \"") + material_name + std::string("\"")));
			}

			Enemy* enemy_temp = new Enemy(enemyType, entity_name, geom, mat, 0, (SceneNode*)heli, &resman_);
			enemy_temp->AddChild(new SceneNode(entity_name + "(Base)", geom2, mat2, 0));
			return enemy_temp;
		}
		else if (enemyType == 2) {
			Resource *geom = resman_.GetResource(object_name);
			if (!geom) {
				throw(GameException(std::string("Could not find resource \"") + object_name + std::string("\"")));
			}

			Resource *mat = resman_.GetResource(material_name);
			if (!mat) {
				throw(GameException(std::string("Could not find resource \"") + material_name + std::string("\"")));
			}
			Resource* geom2 = resman_.GetResource("SimpleSphereMesh");
			if (!geom2) {
				throw(GameException(std::string("Could not find resource \"") + object_name + std::string("\"")));
			}
			Resource* mat2 = resman_.GetResource("ObjectMaterial");
			if (!mat2) {
				throw(GameException(std::string("Could not find resource \"") + material_name + std::string("\"")));
			}

			Enemy* enemy_temp = new Enemy(enemyType, entity_name, geom, mat, 0, (SceneNode*)heli, &resman_);
			enemy_temp->AddChild(new SceneNode(entity_name + "(Base)", geom2, mat2, 0));
			return enemy_temp;
		}
		else
			return NULL;

	}

	Asteroid *Game::CreateAsteroidInstance(std::string entity_name, std::string object_name, std::string material_name) {

		// Get resources
		Resource *geom = resman_.GetResource(object_name);
		if (!geom) {
			throw(GameException(std::string("Could not find resource \"") + object_name + std::string("\"")));
		}

		Resource *mat = resman_.GetResource(material_name);
		if (!mat) {
			throw(GameException(std::string("Could not find resource \"") + material_name + std::string("\"")));
		}

		// Create asteroid instance
		Asteroid *ast = new Asteroid(entity_name, geom, mat);
		collidables.push_back(ast);
		return ast;
	}

	void Game::checkForCollisions(GLFWwindow* window, bool laser) {

		void* ptr = glfwGetWindowUserPointer(window);
		Game *game = (Game *)ptr;

		glm::vec3 a, c, f, l, ma, mp, d, s, hp, ha;
		float t;

		s = heli->GetPosition();//(lazerref->GetPosition() - (this->camera_.GetForward() * 45.0f));
								//PrintVec3(heli->GetPosition());
								//PrintVec3(camera_.GetPosition());
		d = glm::normalize(heli->GetForward());
		for (int j = 0; j < enemies.size(); j++) {
			a = enemies[j]->GetPosition();

			if (laser) {

				//s = starting point
				//a = center of asteroids
				//l = unit vector in direction of asteroid
				//d = unit vector in cam direction

				l = glm::normalize(s - a);
				float theta = acos(glm::dot(l, d));

				float test = tan(theta) * glm::length(s - a);

				//child lasers
				for (int i = 0; i < childlasers.size(); ++i) {
					if (hostcollected[i]) {
						l = glm::normalize(hostages[i]->GetPosition() - a);
						float theta = acos(glm::dot(l, d));

						test = tan(theta) * glm::length(hostages[i]->GetPosition() - a);
					}
				}

				if (test < 0.0 && test > -1.0) {
					if (enemies[j]->LoseHealth(2.0f) <= 0.0f)
						enemies[j]->SetVisible(false);
				}

			}
			else {
				for (int z = 0; z < missiles.size(); z++) {

					if (glm::length(missiles[z]->GetPosition() - enemies[j]->GetPosition()) <= 2.0) {
						if (enemies[j]->LoseHealth(1.0f) <= 0.0f)
							enemies[j]->SetVisible(false);
					}


					if (missiles.size() > 100) {
						missiles[z]->SetVisible(false);
						missiles.pop_front();
					}
				}
				for (int n = 0; n < childmissiles.size(); n++) {
					if (hostcollected[n]) {
						for (int k = 0; k < childmissiles[n].size(); k++) {


							if (glm::length(childmissiles[n][k]->GetPosition() - enemies[j]->GetPosition()) <= 2.0) {
								if (enemies[j]->LoseHealth(1.0f) <= 0.0f)
									enemies[j]->SetVisible(false);
							}
							if (childmissiles[n].size() > 5) {
								childmissiles[n][k]->SetVisible(false);
								childmissiles[n].pop_front();
							}
						}
					}
				}

				for (int h = 0; h < explosionSpheres.size(); h++) {
					if (glm::length(explosionSpheres[h]->GetPosition() - enemies[j]->GetPosition()) < (1.0f + glm::length(explosionSpheres[h]->GetScale().y))) {
						if (enemies[j]->LoseHealth(5.0f) <= 0.0f)
							enemies[j]->SetVisible(false);
					}
				}

			}

		}
		for (int j = 0; j < hostages.size(); j++) {

			if (!hostcollected[j]) {

				if (glm::length(hostages[j]->GetPosition() - heli->GetPosition()) < 2.0f && hostages[j]->GetFreedom())
					hostcollected[j] = true;
			}
		}
		for (int j = 0; j < enemies.size(); j++) {
			if (glm::length(enemies[j]->GetPosition() - heli->GetPosition()) <= 150.0f) {
				enemies[j]->SetAgro(true);
			}
			else {
				enemies[j]->SetAgro(false);
			}

		}

	}




	void Game::CreateLaserInstance(std::string entity_name, std::string object_name, std::string material_name) {

		// Get resources
		Resource *geom = resman_.GetResource(object_name);
		if (!geom) {
			throw(GameException(std::string("Could not find resource \"") + object_name + std::string("\"")));
		}

		Resource *mat = resman_.GetResource(material_name);
		if (!mat) {
			throw(GameException(std::string("Could not find resource \"") + material_name + std::string("\"")));
		}

		// Create Laser instance
		SceneNode *laser = new SceneNode(entity_name, geom, mat, 0);
		laser->Scale(glm::vec3(1.0, 1.0, 40));
		scene_.GetNode("Root")->AddChild(laser);
		float off = 0.0;
		lazerref = laser;

		for (int i = 0; i < hostages.size(); ++i) {
			SceneNode *laser = new SceneNode(entity_name, geom, mat, 0);
			laser->Scale(glm::vec3(1.0, 1.0, 40));
			scene_.GetNode("Root")->AddChild(laser);
			float off = 0.0;
			childlasers.push_back(laser);
		}

	}

	void Game::CreateMissileInstance(std::string entity_name, std::string object_name, std::string material_name) {

		// Get resources
		Resource *geom = resman_.GetResource(object_name);
		if (!geom) {
			throw(GameException(std::string("Could not find resource \"") + object_name + std::string("\"")));
		}

		Resource *mat = resman_.GetResource(material_name);
		if (!mat) {
			throw(GameException(std::string("Could not find resource \"") + material_name + std::string("\"")));
		}


		// Create Missile instance
		SceneNode *missile = new SceneNode(entity_name, geom, mat, 0);
		SceneNode *particles = new SceneNode(entity_name + " particles", resman_.GetResource("MissileParticles"), resman_.GetResource("MissileMaterial"), resman_.GetResource("Fire"));

		missile->SetVisible(true);
		//particles->Scale(glm::vec3(10.0));
		particles->SetParticle(true);
		particles->SetBlending(true);
		particles->SetVisible(true);
		missile->AddChild(particles);

		missile->SetPosition(this->heli->GetPosition());
		missile->SetOrientation(this->camera_.GetOrientation());
		missile->SetScale(glm::vec3(2.0));
		scene_.GetNode("Root")->AddChild(missile, true);
		float off = 0.0;
		missile->direction = camera_.GetForward();
		explodingMissiles.push_back(missile);

		// Create Missiles for children
		/*
		for (int i = 0; i < childmissiles.size(); i++) {
			if (hostcollected[i]) {
				SceneNode *childmis = new SceneNode(entity_name, geom, mat, resman_.GetResource(""));
				SceneNode *childpart = new SceneNode(entity_name + " particles", resman_.GetResource("MissileParticles"), resman_.GetResource("MissileMaterial"), resman_.GetResource("Fire"));

				childmis->SetVisible(true);
				//childpart->Scale(glm::vec3(10.0));
				childpart->SetParticle(true);
				childpart->SetBlending(true);
				childpart->SetVisible(true);
				childmis->AddChild(childpart);
				childmis->SetScale(glm::vec3(2.0));

				childmis->SetPosition(hostages[i]->GetPosition());
				childmis->SetOrientation(this->camera_.GetOrientation());

				scene_.GetNode("Root")->AddChild(childmis, true);
				float off = 0.0;
				childmis->direction = camera_.GetForward();
				childmissiles[i].push_back(childmis);
				if (childmissiles[i].size() > 30) {
					SceneNode *cmis = childmissiles[i].front();
					childmissiles[i].pop_front();
					cmis->SetVisible(false);
					cmis->~SceneNode();
				}
			}
		}*/
	}

	void Game::CreateBulletInstance(std::string entity_name, std::string object_name, std::string material_name) {

		// Get resources
		Resource *geom = resman_.GetResource(object_name);
		if (!geom) {
			throw(GameException(std::string("Could not find resource \"") + object_name + std::string("\"")));
		}

		Resource *mat = resman_.GetResource(material_name);
		if (!mat) {
			throw(GameException(std::string("Could not find resource \"") + material_name + std::string("\"")));
		}


		// Create Missile instance
		SceneNode *bullet = new SceneNode(entity_name, geom, mat, 0);
		SceneNode *particles = new SceneNode(entity_name + " particles", resman_.GetResource("MissileParticles"), resman_.GetResource("BulletMaterial"), resman_.GetResource("Fire"));

		bullet->SetVisible(true);
		//particles->Scale(glm::vec3(10.0));
		particles->SetParticle(true);
		particles->SetBlending(true);
		particles->SetVisible(true);
		bullet->AddChild(particles);
		
		bullet->SetPosition(this->heli->GetPosition());
		bullet->SetOrientation(this->camera_.GetOrientation());
		bullet->SetScale(glm::vec3(2.0));
		scene_.GetNode("Root")->AddChild(bullet, true);
		float off = 0.0;

		float sprayX = (float)((rand() % 1000) - 500) / 20000.0f;
		float sprayY = (float)((rand() % 1000) - 500) / 20000.0f;


		bullet->direction = glm::normalize(camera_.GetForward() + sprayY * camera_.GetUp() + sprayX * camera_.GetSide());
		missiles.push_back(bullet);
		if (missiles.size() > 200) {
			SceneNode *mis = missiles.front();
			missiles.pop_front();
			mis->SetVisible(false);
			mis->~SceneNode();
		}

		// Create Missiles for children
		for (int i = 0; i < childmissiles.size(); i++) {
			if (ticker % 10 == 0) {
				if (hostcollected[i]) {
					SceneNode *childbullet = new SceneNode(entity_name + "Child", geom, mat, 0);
					SceneNode *childpart = new SceneNode(entity_name + " particlesChild", resman_.GetResource("MissileParticles"), resman_.GetResource("BulletMaterial"), resman_.GetResource("Fire"));
					childbullet->SetVisible(true);
					//childpart->Scale(glm::vec3(10.0));
					childpart->SetParticle(true);
					childpart->SetBlending(true);
					childpart->SetVisible(true);
					childbullet->AddChild(childpart);

					childbullet->SetPosition(hostages[i]->GetPosition());
					childbullet->SetOrientation(this->camera_.GetOrientation());
					childbullet->SetScale(glm::vec3(2.0));
					scene_.GetNode("Root")->AddChild(childbullet, true);
					float off = 0.0;
					childbullet->direction = glm::normalize(camera_.GetForward() + sprayY * camera_.GetUp() + sprayX * camera_.GetSide());
					childmissiles[i].push_back(childbullet);
					for (int aaa = 0; aaa < childmissiles[i].size(); aaa++)
						std::cout << childmissiles[i][aaa]->GetVisible() << std::endl;
				}
			}
		}
	}





	void Game::CreateEnemyMissile(std::string entity_name, std::string object_name, std::string material_name, Enemy* enemy) {
		std::cout << "\nFIRE!";
		// Get resources
		Resource *geom = resman_.GetResource(object_name);
		if (!geom) {
			throw(GameException(std::string("Could not find resource \"") + object_name + std::string("\"")));
		}

		Resource *mat = resman_.GetResource(material_name);
		if (!mat) {
			throw(GameException(std::string("Could not find resource \"") + material_name + std::string("\"")));
		}


		// Create Missile instance
		SceneNode *missile = new SceneNode(entity_name, geom, mat, 0);
		SceneNode *particles = new SceneNode(entity_name + " particles", resman_.GetResource("MissileParticles"), resman_.GetResource("MissileMaterial"), resman_.GetResource("Fire"));

		missile->SetVisible(true);
		particles->SetParticle(true);
		particles->SetBlending(true);
		particles->SetVisible(true);
		missile->AddChild(particles);

		missile->SetVisible(true);
		//missile->Scale(glm::vec3(10.0));
		missile->SetPosition(enemy->GetPosition());
		missile->SetOrientation(enemy->GetOrientation());

		scene_.GetNode("Root")->AddChild(missile);
		float off = 0.0;
		missile->direction = enemy->GetForward();
		enemymissiles.push_back(missile);

	}

	void Game::CreateAsteroidField(int num_asteroids) {

		// Create a number of asteroid instances
		for (int i = 0; i < 0; i++) {
			// Create instance name
			std::stringstream ss;
			ss << i;
			std::string index = ss.str();
			std::string name = "AsteroidInstance" + index;

			// Create asteroid instance
			Asteroid *ast = CreateAsteroidInstance(name, "SimpleSphereMesh", "ObjectMaterial");

			// Set attributes of asteroid: random position, orientation, and
			// angular momentum
			//ast->SetPosition(glm::vec3(0, 0, 700));
			ast->SetPosition(glm::vec3(-300.0 + 600.0*((float)rand() / RAND_MAX), 0 + 600.0*((float)rand() / RAND_MAX), 600.0*((float)rand() / RAND_MAX)));
			ast->SetOrientation(glm::normalize(glm::angleAxis(glm::pi<float>()*((float)rand() / RAND_MAX), glm::vec3(((float)rand() / RAND_MAX), ((float)rand() / RAND_MAX), ((float)rand() / RAND_MAX)))));
			ast->SetAngM(glm::normalize(glm::angleAxis(0.05f*glm::pi<float>()*((float)rand() / RAND_MAX), glm::vec3(((float)rand() / RAND_MAX), ((float)rand() / RAND_MAX), ((float)rand() / RAND_MAX)))));
			scene_.GetNode("Root")->AddChild(ast);
		}
	}


	SceneNode *Game::CreateInstance(std::string entity_name, std::string object_name, std::string material_name) {
		Resource *geom;
		if (object_name != std::string("")) {
			geom = resman_.GetResource(object_name);
			if (!geom) {
				throw(GameException(std::string("Could not find resource \"") + object_name + std::string("\"")));
			}
		}
		else {
			geom = NULL;
		}

		Resource *mat;
		if (material_name != std::string("")) {
			mat = resman_.GetResource(material_name);
			if (!mat) {
				throw(GameException(std::string("Could not find resource \"") + material_name + std::string("\"")));
			}
		}
		else {
			mat = NULL;
		}
		// Create instance
		SceneNode *node = new SceneNode(entity_name, geom, mat, 0);
		return node;
	}

	SceneNode *Game::CreateInstance(std::string entity_name, std::string object_name, std::string material_name, std::string parent_name) {

		Resource *geom;
		if (object_name != std::string("")) {
			geom = resman_.GetResource(object_name);
			if (!geom) {
				throw(GameException(std::string("Could not find resource \"") + object_name + std::string("\"")));
			}
		}
		else {
			geom = NULL;
		}

		Resource *mat;
		if (material_name != std::string("")) {
			mat = resman_.GetResource(material_name);
			if (!mat) {
				throw(GameException(std::string("Could not find resource \"") + material_name + std::string("\"")));
			}
		}
		else {
			mat = NULL;
		}
		// Create instance
		SceneNode *node = new SceneNode(entity_name, geom, mat, 0);
		scene_.GetNode(parent_name)->AddChild(node);
		return node;
	}
	Helicopter* Game::CreateHeliInstance(std::string entity_name, std::string object_name, std::string material_name, std::string parent_name) {

		Resource *geom;
		if (object_name != std::string("")) {
			geom = resman_.GetResource(object_name);
			if (!geom) {
				throw(GameException(std::string("Could not find resource \"") + object_name + std::string("\"")));
			}
		}
		else {
			geom = NULL;
		}

		Resource *mat;
		if (material_name != std::string("")) {
			mat = resman_.GetResource(material_name);
			if (!mat) {
				throw(GameException(std::string("Could not find resource \"") + material_name + std::string("\"")));
			}
		}
		else {
			mat = NULL;
		}
		// Create instance
		Helicopter* node = new Helicopter(entity_name, geom, mat, 0);
		scene_.GetNode(parent_name)->AddChild(node);
		return node;
	}

	SceneNode *Game::CreateTexturedInstance(std::string entity_name, std::string object_name, std::string material_name, std::string texture_name)
	{
		Resource *geom;
		if (object_name != std::string("")) {
			geom = resman_.GetResource(object_name);
			if (!geom) {
				throw(GameException(std::string("Could not find resource \"") + object_name + std::string("\"")));
			}
		}
		else {
			geom = NULL;
		}

		Resource *mat;
		if (material_name != std::string("")) {
			mat = resman_.GetResource(material_name);
			if (!mat) {
				throw(GameException(std::string("Could not find resource \"") + material_name + std::string("\"")));
			}
		}
		else {
			mat = NULL;
		}

		Resource *tex;
		if (texture_name != std::string("")) {
			tex = resman_.GetResource(texture_name);
			if (!tex) {
				throw(GameException(std::string("Could not find resource \"") + material_name + std::string("\"")));
			}
		}
		else {
			tex = NULL;
		}

		SceneNode *scn = new SceneNode(entity_name, geom, mat, tex);
		return scn;
	}

	SceneNode *Game::CreateTexturedInstance(std::string entity_name, std::string object_name, std::string material_name, std::string parent_name, std::string texture_name)
	{
		Resource *geom;
		if (object_name != std::string("")) {
			geom = resman_.GetResource(object_name);
			if (!geom) {
				throw(GameException(std::string("Could not find resource \"") + object_name + std::string("\"")));
			}
		}
		else {
			geom = NULL;
		}

		Resource *mat;
		if (material_name != std::string("")) {
			mat = resman_.GetResource(material_name);
			if (!mat) {
				throw(GameException(std::string("Could not find resource \"") + material_name + std::string("\"")));
			}
		}
		else {
			mat = NULL;
		}

		Resource *tex;
		if (texture_name != std::string("")) {
			tex = resman_.GetResource(texture_name);
			if (!tex) {
				throw(GameException(std::string("Could not find resource \"") + material_name + std::string("\"")));
			}
		}
		else {
			tex = NULL;
		}

		SceneNode *scn = new SceneNode(entity_name, geom, mat, tex);
		scene_.GetNode(parent_name)->AddChild(scn);
		return scn;
	}

	Helicopter* Game::CreateTexturedHeliInstance(std::string entity_name, std::string object_name, std::string material_name, std::string parent_name, std::string texture_name) {
		Resource *geom;
		if (object_name != std::string("")) {
			geom = resman_.GetResource(object_name);
			if (!geom) {
				throw(GameException(std::string("Could not find resource \"") + object_name + std::string("\"")));
			}
		}
		else {
			geom = NULL;
		}

		Resource *mat;
		if (material_name != std::string("")) {
			mat = resman_.GetResource(material_name);
			if (!mat) {
				throw(GameException(std::string("Could not find resource \"") + material_name + std::string("\"")));
			}
		}
		else {
			mat = NULL;
		}

		Resource *tex;
		if (material_name != std::string("")) {
			tex = resman_.GetResource(texture_name);
			if (!tex) {
				throw(GameException(std::string("Could not find resource \"") + material_name + std::string("\"")));
			}
		}
		else {
			tex = NULL;
		}

		// Create instance
		Helicopter* node = new Helicopter(entity_name, geom, mat, tex);
		scene_.GetNode(parent_name)->AddChild(node);
		return node;
	}
	Enemy* Game::CreateTexturedEnmeyTank(std::string entity_name, std::string object_name, std::string material_name, std::string parent_name, std::string texture_name)
	{
		Resource *geom;
		if (object_name != std::string("")) {
			geom = resman_.GetResource(object_name);
			if (!geom) {
				throw(GameException(std::string("Could not find resource \"") + object_name + std::string("\"")));
			}
		}
		else {
			geom = NULL;
		}

		Resource *mat;
		if (material_name != std::string("")) {
			mat = resman_.GetResource(material_name);
			if (!mat) {
				throw(GameException(std::string("Could not find resource \"") + material_name + std::string("\"")));
			}
		}
		else {
			mat = NULL;
		}

		Resource *tex;
		if (material_name != std::string("")) {
			tex = resman_.GetResource(texture_name);
			if (!tex) {
				throw(GameException(std::string("Could not find resource \"") + material_name + std::string("\"")));
			}
		}
		else {
			tex = NULL;
		}

		// Create instance
		Enemy* node = new Enemy(1, entity_name, geom, mat, tex, heli, &resman_);
		scene_.GetNode(parent_name)->AddChild(node);
		return node;
	}

	void Game::setupTank(int tankNumber, Enemy** enemy, glm::vec3 position)
	{

		std::stringstream ss;
		ss << tankNumber;
		std::string index = ss.str();

		*enemy = CreateTexturedEnmeyTank("enemyTank" + index, "tankMesh", "textureMaterial", "Root", "Camo");
		(*enemy)->SetScale(glm::vec3(0.1, 0.1, 0.1));
		(*enemy)->SetPosition(position + glm::vec3(0.0, 1.0, 0.0));




	}
	Enemy* Game::CreateTexturedEnemyHeliInstance(std::string entity_name, std::string object_name, std::string material_name, std::string parent_name, std::string texture_name) {
		Resource *geom;
		if (object_name != std::string("")) {
			geom = resman_.GetResource(object_name);
			if (!geom) {
				throw(GameException(std::string("Could not find resource \"") + object_name + std::string("\"")));
			}
		}
		else {
			geom = NULL;
		}

		Resource *mat;
		if (material_name != std::string("")) {
			mat = resman_.GetResource(material_name);
			if (!mat) {
				throw(GameException(std::string("Could not find resource \"") + material_name + std::string("\"")));
			}
		}
		else {
			mat = NULL;
		}

		Resource *tex;
		if (material_name != std::string("")) {
			tex = resman_.GetResource(texture_name);
			if (!tex) {
				throw(GameException(std::string("Could not find resource \"") + material_name + std::string("\"")));
			}
		}
		else {
			tex = NULL;
		}

		// Create instance
		Enemy* node = new Enemy(2, entity_name, geom, mat, tex, heli, &resman_);
		scene_.GetNode(parent_name)->AddChild(node);
		return node;
	}
	bool Game::PointBoxCollision(glm::vec3 point, glm::vec3* box) {
		/*
		box : 0 min x max y max z
		box : 1 max x max y max z *allmaxes*
		box : 2 min x max y min z
		box : 3 max x max y min z
		box : 4 min x min y max z
		box : 5 max x min y max z
		box : 6 min x min y min z *all mins*
		box : 7 max x min y min z
		*/

		if (point.x > box[6].x && point.x < box[1].x && point.y < box[1].y && point.z > box[6].z && point.z < box[1].z)
			return true;
		else
			return false;

	}

	glm::vec3* Game::LinePlaneCollision(glm::vec3 planeVector, glm::vec3 planePoint, glm::vec3 lineVector, glm::vec3 linePoint)
	{
		glm::vec3 returnResult;
		returnResult = glm::vec3();
		float vp1, vp2, vp3, n1, n2, n3, v1, v2, v3, m1, m2, m3, t, vpt;
		vp1 = planeVector[0];
		vp2 = planeVector[1];
		vp3 = planeVector[2];
		n1 = planePoint[0];
		n2 = planePoint[1];
		n3 = planePoint[2];
		v1 = lineVector[0];
		v2 = lineVector[1];
		v3 = lineVector[2];
		m1 = linePoint[0];
		m2 = linePoint[1];
		m3 = linePoint[2];
		vpt = v1 * vp1 + v2 * vp2 + v3 * vp3;

		if (vpt == 0)
		{
			return NULL;
		}
		else
		{
			t = ((n1 - m1) * vp1 + (n2 - m2) * vp2 + (n3 - m3) * vp3) / vpt;
			returnResult[0] = m1 + v1 * t;
			returnResult[1] = m2 + v2 * t;
			returnResult[2] = m3 + v3 * t;
			return &returnResult;
		}
	}

} // namespace game

