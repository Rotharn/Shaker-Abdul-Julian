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

// Materials 
const std::string material_directory_g = MATERIAL_DIRECTORY;


Game::Game(void){

    // Don't do work in the constructor, leave it for the Init() function
}


void Game::Init(void){

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
	positions = std::deque<glm::vec3>(25,glm::vec3(0.0,0.0,0.0));
}

       
void Game::InitWindow(void){

    // Initialize the window management library (GLFW)
    if (!glfwInit()){
        throw(GameException(std::string("Could not initialize the GLFW library")));
    }

    // Create a window and its OpenGL context
    if (window_full_screen_g){
        window_ = glfwCreateWindow(window_width_g, window_height_g, window_title_g.c_str(), glfwGetPrimaryMonitor(), NULL);
    } else {
        window_ = glfwCreateWindow(window_width_g, window_height_g, window_title_g.c_str(), NULL, NULL);
    }
    if (!window_){
        glfwTerminate();
        throw(GameException(std::string("Could not create window")));
    }

    // Make the window's context the current one
    glfwMakeContextCurrent(window_);

    // Initialize the GLEW library to access OpenGL extensions
    // Need to do it after initializing an OpenGL context
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK){
        throw(GameException(std::string("Could not initialize the GLEW library: ")+std::string((const char *) glewGetErrorString(err))));
    }
}


void Game::InitView(void){

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


void Game::InitEventHandlers(void){

    // Set event callbacks
    glfwSetKeyCallback(window_, KeyCallback);
	glfwSetMouseButtonCallback(window_, MouseButtonCallback);
	glfwSetScrollCallback(window_, ScrollWheelCallback);
    glfwSetFramebufferSizeCallback(window_, ResizeCallback);

    // Set pointer to game object, so that callbacks can access it
    glfwSetWindowUserPointer(window_, (void *) this);
}


void Game::SetupResources(void){

    // Create a simple sphere to represent the asteroids
    resman_.CreateSphere("SimpleSphereMesh", 1.0, 10, 10);
	resman_.CreateCylinder("SimpleCylinderMesh", 0.0, 0.05,3,30);
	resman_.CreateCylinder("CylinderMesh",0.0f, 0.2f,3,10,-1);
	resman_.CreateCylinder("LaserMesh", 0.0f, 0.2f, 3, 5, 0);
	resman_.CreateCube("CubeMesh");

    // Load material to be applied to asteroids
    std::string filename = std::string(MATERIAL_DIRECTORY) + std::string("/material");
    resman_.LoadResource(Material, "ObjectMaterial", filename.c_str());

	filename = std::string(MATERIAL_DIRECTORY) + std::string("/shiny_blue");
	resman_.LoadResource(Material, "ShinyBlueMaterial", filename.c_str());

	filename = std::string(MATERIAL_DIRECTORY) + std::string("/toon");
	resman_.LoadResource(Material, "ToonRingMaterial", filename.c_str());

	filename = std::string(MATERIAL_DIRECTORY) + std::string("/toon_heli");
	resman_.LoadResource(Material, "ToonHeliMaterial", filename.c_str());

	filename = std::string(MATERIAL_DIRECTORY) + std::string("/helicoptero_1.1.obj");
	resman_.LoadResource(Mesh, "HeliBodyMesh", filename.c_str());

	filename = std::string(MATERIAL_DIRECTORY) + std::string("/helicoptero_1.22.obj");
	resman_.LoadResource(Mesh, "HeliRotorMesh", filename.c_str());

	filename = std::string(MATERIAL_DIRECTORY) + std::string("/helicoptero_1.3.obj");
	resman_.LoadResource(Mesh, "HeliTailRotorMesh", filename.c_str());


}
void Game::InitInputs() {

	input_up = false; input_down = false; 
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

void Game::SetupScene(void){
	
	InitInputs();
	offsetx = 20.0f;
	offsety = 2.0f;

    // Set background color for the scene
	scene_.SetBackgroundColor(viewport_background_color_g);
	cameraNode = CreateInstance("Camera", "", "");
	SceneNode* root = CreateInstance("Root", "", "");
	root->SetPosition(glm::vec3(0.0, 0.0, 0.0));
	scene_.SetRoot(root);
	root->AddChild(cameraNode);
	SetupHelicopterOld();
	heli->Rotate(glm::angleAxis(3.14159f, glm::vec3(0.0, 1.0, 0.0)));
	SetupWorld();
	SetupEnemies();
	positions = std::deque<glm::vec3>(25,heli->GetPosition());
	SetupHostage("Hostage 1");
	SetupHostage("Hostage 2");
	SetupHostage("Hostage 3");
	hostages[0]->SetPosition(glm::vec3(10.0, 0.2,20.0));
	hostages[1]->SetPosition(glm::vec3(-22.0, 0.2, -13.0));
	hostages[2]->SetPosition(glm::vec3(-44.0, 0.2, 5.0));
	// Create asteroid field
	CreateAsteroidField();
	CreateLaserInstance("laser", "LaserMesh", "ObjectMaterial");


	camera_position_g = glm::vec3(heli->GetPosition().x, heli->GetPosition().y - offsety, heli->GetPosition().z - offsetx);
	camera_.SetView(camera_position_g, heli->GetPosition(), camera_up_g);

}


void Game::MainLoop(void){

	

    // Loop while the user did not close the window
    while (!glfwWindowShouldClose(window_)){
        // Animate the scene
        if (animating_){
            static double last_time = 0;
            double current_time = glfwGetTime();
            if ((current_time - last_time) > 1.0/60.0){
                scene_.Update();
				Update(window_);
                last_time = current_time;
            }
        }
		if (glm::length(positions.front() - heli->GetPosition()) > 0.3) {
			positions.push_front(heli->GetPosition());

			positions.pop_back();
			for (int i = 0; i < hostages.size(); i++) {
				if (hostcollected[i]) {
					hostages[i]->SetPosition(positions[(i + 1) * 5]);
					hostages[i]->SetOrientation(heli->GetOrientation());
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

	//game->test->Scale(glm::vec3(1.01, 1.01, 1.01));
	//PrintVec3(game->test->GetScale());

	if ((game->input_up == true || game->input_space == true) && game->ship_velocity[1] < 2.0f) {
		game->ship_velocity[1] += 0.02;
	}
	if ((game->input_up == false && game->input_space == false) && game->ship_velocity[1] > 0.0f) {
		if (game->ship_velocity[1] < 0.02f)
			game->ship_velocity[1] = 0.0;
		else
			game->ship_velocity[1] -= 0.02;
	}

	if ((game->input_down == true || game->input_shift == true) && game->ship_velocity[1] > -2.0f) {

		game->ship_velocity[1] -= 0.02;
	}
	if ((game->input_down == false && game->input_shift == false) && game->ship_velocity[1] < 0.0f) {
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

	if (game->input_w == true && game->ship_velocity[2] < 5.0f) {
		game->ship_velocity[2] += 0.02f;
	}
	if (game->input_w == false && game->ship_velocity[2] > 0.0f) {
		if (game->ship_velocity[2] < 0.02f)
			game->ship_velocity[2] = 0.0;
		else
			game->ship_velocity[2] -= 0.02f;
	}

	if (game->input_s == true && game->ship_velocity[2] > -5.0f) {
		game->ship_velocity[2] -= 0.05;
	}
	if (game->input_s == false && game->ship_velocity[2] < 0.0f) {
		if (game->ship_velocity[2] > -0.02f)
			game->ship_velocity[2] = 0.0;
		else
			game->ship_velocity[2] += 0.02;
	}

	if (game->input_a == true && game->ship_velocity[0] < 5.0f) {
		game->ship_velocity[0] += 0.05;
	}
	if (game->input_a == false && game->ship_velocity[0] > 0.0f) {
		if (game->ship_velocity[0] < 0.02f)
			game->ship_velocity[0] = 0.0;
		else
			game->ship_velocity[0] -= 0.02;
	}

	if (game->input_d == true && game->ship_velocity[0] > -5.0f) {
		game->ship_velocity[0] -= 0.05;
	}
	if (game->input_d == false && game->ship_velocity[0] < 0.0f) {
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

	game->camera_.Translate(game->heli->GetForward()*trans_factor*ship_velocity[2] * -1.0f);
	game->camera_.Translate(-game->heli->GetSide()*trans_factor*ship_velocity[0] * -1.0f);
	game->camera_.Translate(glm::vec3(0.0, 1.0, 0.0)*trans_factor*ship_velocity[1]);

	//glm::vec3 mov = glm::vec3(game->camera_.GetForward().x * ship_velocity[2] + game->camera_.GetSide().x * ship_velocity[0] + game->camera_.GetUp().x *ship_velocity[1]
		//, game->camera_.GetForward().y * ship_velocity[2] + game->camera_.GetSide().y * ship_velocity[0] + game->camera_.GetUp().y *ship_velocity[1]
		//, game->camera_.GetForward().z * ship_velocity[2] + game->camera_.GetSide().z * ship_velocity[0] + game->camera_.GetUp().z *ship_velocity[1]);

	glm::vec3* pos = &game->camera_.GetPosition();
	if (pos->z < 0) {
		game->camera_.SetPosition(glm::vec3(pos->x, pos->y, 0));
		ship_velocity[2] = 0;
	}
	if (pos->z > 1200) {
		game->camera_.SetPosition(glm::vec3(pos->x, pos->y, 1200));
		ship_velocity[2] = 0;
	}
	if (pos->x > 1200) {
		game->camera_.SetPosition(glm::vec3(1200, pos->y, pos->z));
		ship_velocity[0] = 0;
	}
	if (pos->x < 0) {
		game->camera_.SetPosition(glm::vec3(0, pos->y, pos->z));
		ship_velocity[0] = 0;
	}
	if (pos->y < 0 ) {
		game->camera_.SetPosition(glm::vec3(pos->x, -0, pos->z));
		ship_velocity[1] = 0;
	}
	if (pos->y > 350) {
		game->camera_.SetPosition(glm::vec3(pos->x, 350, pos->z));
		ship_velocity[1] = 0;
	}
	/*
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
	
	game->heli->SetVisible(true);

	game->heli->SetPosition(game->camera_.GetPosition() + (game->camera_.GetForward() * offsetx) - (game->camera_.GetUp() * offsety));

	if (game->input_c == true || game->input_m2 == true) {
		lazerref->SetVisible(true);
		lazerref->SetPosition(this->heli->GetPosition()/* + this->heli->GetForward() /* -45.0f/* + game->camera_.GetUp()*((float)-0.1)*/); 
		lazerref->SetOrientation(this->heli->GetOrientation());
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
	if (game->input_m == true || game->input_m1 == true) {
		CreateMissileInstance("missile", "LaserMesh", "ObjectMaterial");
	}
	for (int i = 0; i < missiles.size(); i++) {
		if (missiles[i]->GetPosition().x < 0 || missiles[i]->GetPosition().z < 0 || missiles[i]->GetPosition().x > 1200 || missiles[i]->GetPosition().z > 1200 || missiles[i]->GetPosition().y > 350 || missiles[i]->GetPosition().y < 0)
			missiles[i]->SetVisible(false);
		else
		missiles[i]->SetPosition(missiles[i]->GetPosition() + glm::normalize(missiles[i]->direction));
	}
	for (int i = 0; i < enemymissiles.size(); i++) {
		if (enemymissiles[i]->GetPosition().x < 0 || enemymissiles[i]->GetPosition().z < 0 || enemymissiles[i]->GetPosition().x > 1200 || enemymissiles[i]->GetPosition().z > 1200 || enemymissiles[i]->GetPosition().y > 350 || enemymissiles[i]->GetPosition().y < 0)
			enemymissiles[i]->SetVisible(false);
		else
			enemymissiles[i]->SetPosition(enemymissiles[i]->GetPosition() + glm::normalize(enemymissiles[i]->direction)*3.0f);
	}
	for (int i = 0; i < childmissiles.size(); i++) {
		if (hostcollected[i]) {
			for (int j = 0; j < childmissiles[i].size(); j++)
				childmissiles[i][j]->SetPosition(childmissiles[i][j]->GetPosition() + glm::normalize(childmissiles[i][j]->direction));
		}
	}

	for (int i = 0; i < enemies.size(); ++i) {
		if (enemies[i]->Shoot()) {
			CreateEnemyMissile("enemymissile", "LaserMesh", "ObjectMaterial", enemies[i]);
		}
	}


	cameraNode->SetOrientation(game->camera_.GetOrientation());
	cameraNode->SetPosition(game->camera_.GetPosition());
	PrintVec3(heli->GetPosition());
	checkForCollisions(window, false);
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

void Game::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods){

    // Get user data with a pointer to the game class
    void* ptr = glfwGetWindowUserPointer(window);
    Game *game = (Game *) ptr;

    // Quit game if 'q' is pressed
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS){
        glfwSetWindowShouldClose(window, true);
    }

    // Ship control
    float rot_factor(glm::pi<float>() / 180);
    float trans_factor = 1.0;

	if (key == GLFW_KEY_B && action == GLFW_PRESS) {
		game->input_b = true;
	}else if (key == GLFW_KEY_B && action == GLFW_RELEASE) {
		game->input_b = false;
	}
	if (key == GLFW_KEY_UP && action == GLFW_PRESS) {
		game->input_up = true;
	}else if (key == GLFW_KEY_UP && action == GLFW_RELEASE) {
		game->input_up = false;
	}
	if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) {
		game->input_down = true;
	}else if (key == GLFW_KEY_DOWN && action == GLFW_RELEASE) {
		game->input_down = false;
	}
    if (key == GLFW_KEY_LEFT && action == GLFW_PRESS){
		game->input_left = true;
    }else if (key == GLFW_KEY_LEFT && action == GLFW_RELEASE) {
		game->input_left = false;
	}
    if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS){
		game->input_right = true;
    }else if (key == GLFW_KEY_RIGHT && action == GLFW_RELEASE) {
		game->input_right = false;
	}
    if (key == GLFW_KEY_S && action == GLFW_PRESS){
		game->input_s = true;
    }else if (key == GLFW_KEY_S && action == GLFW_RELEASE) {
		game->input_s = false;
	}
    if (key == GLFW_KEY_X && action == GLFW_PRESS){
		game->input_x = true;
    }else if (key == GLFW_KEY_X && action == GLFW_RELEASE) {
		game->input_x = false;
	}
    if (key == GLFW_KEY_A && action == GLFW_PRESS){
		game->input_a = true;
	} else if (key == GLFW_KEY_A && action == GLFW_RELEASE) {
		game->input_a = false;
	}
	if (key == GLFW_KEY_W && action == GLFW_PRESS) {
		game->input_w = true;
	}else if (key == GLFW_KEY_W && action == GLFW_RELEASE) {
		game->input_w = false;
	}
	if (key == GLFW_KEY_D && action == GLFW_PRESS) {
		game->input_d = true;
	}else if (key == GLFW_KEY_D && action == GLFW_RELEASE) {
		game->input_d = false;
	}
    if (key == GLFW_KEY_Z && action == GLFW_PRESS){
		game->input_z = true;
    }else if (key == GLFW_KEY_Z && action == GLFW_RELEASE) {
		game->input_z = false;
	}
    if (key == GLFW_KEY_J && action == GLFW_PRESS){
		game->input_j = true;
    }else if (key == GLFW_KEY_J && action == GLFW_RELEASE) {
		game->input_j = false;
	}
    if (key == GLFW_KEY_L && action == GLFW_PRESS){
		game->input_l = true;
    }else if (key == GLFW_KEY_L && action == GLFW_RELEASE) {
		game->input_l = false;
	}
    if (key == GLFW_KEY_I && action == GLFW_PRESS){
		game->input_i = true;
    }else if (key == GLFW_KEY_I && action == GLFW_RELEASE) {
		game->input_i = false;
	}
    if (key == GLFW_KEY_K && action == GLFW_PRESS){
		game->input_k = true;
    }else if (key == GLFW_KEY_K && action == GLFW_RELEASE) {
		game->input_k = false;
	}
	if (key == GLFW_KEY_C && action == GLFW_PRESS) {
		game->input_c = true;
	}else if (key == GLFW_KEY_C && action == GLFW_RELEASE) {
		game->input_c = false;
	}
	if (key == GLFW_KEY_M && action == GLFW_PRESS) {
		game->input_m = true;
	}else if (key == GLFW_KEY_M && action == GLFW_RELEASE) {
		game->input_m = false;
	}
	if (key == GLFW_KEY_T && action == GLFW_PRESS) {
		game->input_t = true;
	}else if (key == GLFW_KEY_T && action == GLFW_RELEASE) {
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
	if (mods == GLFW_MOD_SHIFT && action == GLFW_PRESS) {
		game->input_shift = true;
	}
	else {
		game->input_shift = false;
	}
}


void Game::ResizeCallback(GLFWwindow* window, int width, int height){

    // Set up viewport and camera projection based on new window size
    glViewport(0, 0, width, height);
    void* ptr = glfwGetWindowUserPointer(window);
    Game *game = (Game *) ptr;
    game->camera_.SetProjection(camera_fov_g, camera_near_clip_distance_g, camera_far_clip_distance_g, width, height);
}


Game::~Game(){
    
    glfwTerminate();
}


void Game::SetupHelicopter(void) {

	heli = (Helicopter *)CreateInstance("heli", "HeliBodyMesh", "ToonHeliMaterial", "Root");
	//body2 = CreateInstance("body2", "CubeMesh", "ToonHeliMaterial", "heli");

	rotor1 = CreateInstance("rotor1", "CylinderMesh", "ToonHeliMaterial", "heli");
	rotor2 = CreateInstance("rotor2", "HeliRotorMesh", "ToonHeliMaterial", "rotor1");
	//tail = CreateInstance("tail", "CylinderMesh", "ToonHeliMaterial", "heli");
	rotor3 = CreateInstance("rotor3", "CylinderMesh", "ToonHeliMaterial", "heli");
	heli->Rotate(glm::angleAxis((float)90.0, glm::vec3(0.0, 1.0, 0.0)));
	heli->Translate(glm::vec3(0.0, 0.0, 0.0));
	//body2->Translate(glm::vec3(0.0, 0.3, -0.25));
	//heli->Scale(glm::vec3(1.0, 0.4, 3.5));
	//body2->Scale(glm::vec3(1.0, 0.3, 3.0));
	glm::quat rotation = glm::angleAxis((float)4.709, glm::vec3(1.0, 0.0, 0.0));
	//rotor1->SetOrientation(rotation);
	rotor1->Translate(glm::vec3(0.0, 0.4 - 0.175, 0.0));
	//rotor1->Scale(glm::vec3(1.0, 1.0, 0.1));
	rotation = glm::angleAxis(3.3f, glm::vec3(0.0, 1.0, 0.0));
	//rotor2->Translate(glm::vec3(0.0, 0.0, 0.1));
	//rotor2->SetOrientation(rotation);
	//rotor2->Scale(glm::vec3(1.0, 0.2, 1.5));
	rotor2->SetAngM(glm::quat(glm::angleAxis((float) 0.5, glm::vec3(0.0, 1.0, 0.0))));
	//tail->Scale(glm::vec3(0.8, 0.6, 1.24));
	//tail->Translate(glm::vec3(0.0, 0.1, -1.5));
	rotation = glm::angleAxis(8.0f, glm::vec3(1.0, 0.0, 0.0));
	rotor3->Rotate(rotation);
	//rotor3->Scale(glm::vec3(0.2, .75, 0.4));
	rotor3->Translate(glm::vec3(0.0, 0.0, -1.5));
	rotor3->SetAngM(glm::quat(glm::angleAxis((float) 1.0, glm::vec3(1.0, 0.0, 0.0))));
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

void Game::SetupHostage(std::string name) {

	Helicopter* host = CreateHeliInstance(name, "SimpleSphereMesh", "ToonHeliMaterial", "Root");

	SceneNode* nose = (Helicopter *)CreateInstance(name+" nose", "SimpleSphereMesh", "ToonHeliMaterial", name);
	SceneNode* rot1 = CreateInstance(name+" rotor1", "CylinderMesh", "ToonHeliMaterial", name);
	SceneNode* rot2 = CreateInstance(name+" rotor2", "CylinderMesh", "ToonHeliMaterial", name + " rotor1");
	SceneNode* tale = CreateInstance(name+" tail", "CylinderMesh", "ToonHeliMaterial", name);
	SceneNode* rot3 = CreateInstance(name+" rotor3", "CylinderMesh", "ToonHeliMaterial", name + " tail");
	host->Rotate(glm::angleAxis((float) 180.0, glm::vec3(0.0, 1.0, 0.0)));
	host->Translate(glm::vec3(0.0, 0.0, -5.0));
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

	hostages.push_back(host);
	hostcollected.push_back(false);
	childmissiles.push_back(std::deque<SceneNode*>());

}

void Game::SetupWorld() {
	int sizeOfBuildingArea = 1200;
	bool** table = (bool**)calloc(sizeof(bool*), sizeOfBuildingArea);
	for (int i = 0; i < sizeOfBuildingArea; i++) {
		table[i] = (bool*)calloc(sizeof(bool), sizeOfBuildingArea);
	}
	//test = CreateInstance("test", "CubeMesh", "ToonRingMaterial", "Root");

	//test->SetPosition(glm::vec3(0.0, 0.0, 0.0));

	SceneNode* ground = CreateInstance("cubeg", "CubeMesh", "ShinyBlueMaterial", "Root");
	ground->SetPosition(glm::vec3(600, -5, 600));
	ground->Scale(glm::vec3(1200, 10, 1200));
	SceneNode* b;
	std::stack<bool *> occupiedArea;
	glm::vec3* vertices;
	glm::vec3 scaleFactor;
	for (int i = 0; i < sizeOfBuildingArea / 5; i++) {
		b = CreateInstance("EnvironmentCube" + std::to_string(i), "CubeMesh", "ShinyBlueMaterial", "Root");
		scaleFactor = glm::vec3((float)(4.0 + ((float)(rand() % 20))), (float)(4.0 + ((float)(rand() % 20))), (float)(4.0 + ((float)(rand() % 20))));
		float factor = (float)(1.0 + ((float)(rand() % 20)));
		b->SetPosition(glm::vec3((float)(rand() % 1200), scaleFactor.y / 2.0f, (float)(rand() % 1200)));
		bool check = false;
		int patience = 0;
		bool yes = true;
		while (yes) {
			for (int k = 0 - (int)(scaleFactor.x / 2.0f); k < (int)scaleFactor.x; k++) {
				for (int j = 0 - (int)(scaleFactor.z / 2.0f); j < (int)scaleFactor.z; j++) {
					int x = (int)b->GetPosition().x + k;
					int z = (int)b->GetPosition().z + j;
					if (x < 1200 && z < 1200 && x >= 0 && z >= 0)
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
			spawnPoints.push_back(vertices[roofCorners]);

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
	for (int i = 0; i < 300; i++)
		delete(table[i]);
	delete(table);



}

void Game::SetupEnemies() {
	float num = 30;
	while (num > spawnPoints.size())
		num = num / 2;
	
	for (int i = 0; i < num; i++) {
		int location = rand() % spawnPoints.size();

		std::stringstream ss;
		ss << i;
		std::string index = ss.str();
		std::string name = "StationaryEnemy" + index;

		Enemy* bad_dude = CreateEnemyInstance(name, "LaserMesh", "ObjectMaterial", 0);

		bad_dude->SetPosition(spawnPoints.at(location));
		bad_dude->SetScale(glm::vec3(3.0, 3.0, 3.0));
		std::vector<SceneNode *>::const_iterator bad_child = bad_dude->children_begin();
		(*bad_child)->SetPosition(glm::vec3(0.0, 0.0, 0.0));
		(*bad_child)->SetScale(glm::vec3(1.0, 1.0, 1.0));

		spawnPoints.erase(spawnPoints.begin() + location);
		scene_.GetNode("Root")->AddChild(bad_dude);
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
		
		Enemy* enemy_temp = new Enemy(entity_name, geom, mat, (SceneNode*)heli, &resman_);
		enemy_temp->AddChild(new SceneNode(entity_name + "(Base)", geom2, mat2));
		return enemy_temp;
	}

}

Asteroid *Game::CreateAsteroidInstance(std::string entity_name, std::string object_name, std::string material_name){

    // Get resources
    Resource *geom = resman_.GetResource(object_name);
    if (!geom){
        throw(GameException(std::string("Could not find resource \"")+object_name+std::string("\"")));
    }

    Resource *mat = resman_.GetResource(material_name);
    if (!mat){
        throw(GameException(std::string("Could not find resource \"")+material_name+std::string("\"")));
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
	for (int j = 0; j < collidables.size(); j++) {
		a = collidables[j]->GetPosition();

		if (laser) {

			//s = starting point
			//a = center of asteroids
			//l = unit vector in direction of asteroid
			//d = unit vector in cam direction

			l = glm::normalize(s - a);
			float theta = acos(glm::dot(l, d));

			float test = tan(theta) * glm::length(s - a);

			//child lasers
			for (int i=0; i < childlasers.size(); ++i) {
				if (hostcollected[i]) {
					l = glm::normalize(hostages[i]->GetPosition() - a);
					float theta = acos(glm::dot(l, d));

					test = tan(theta) * glm::length(hostages[i]->GetPosition() - a);
				}
			}
			
			if (test < 0.0 && test > -1.0)
				collidables[j]->SetVisible(false);

		}
		else {
			for (int z = 0; z < missiles.size(); z++) {

				mp = missiles[z]->GetPosition();
				ma = mp - collidables[j]->GetPosition();

				if (((std::pow(ma[0], 2) + std::pow(ma[1], 2) + std::pow(ma[2], 2)) <= 1.0)) {
					collidables[j]->SetVisible(false);
				}
				if (missiles.size() > 5) {
					missiles[z]->SetVisible(false);
					missiles.pop_front();
				}
			}
			for (int i = 0; i < childmissiles.size(); i++) {
				if (hostcollected[i]) {
					for (int k = 0; k < childmissiles[i].size(); k++) {
						mp = childmissiles[i][k]->GetPosition();
						ma = mp - collidables[j]->GetPosition();

						if (((std::pow(ma[0], 2) + std::pow(ma[1], 2) + std::pow(ma[2], 2)) <= 1.0)) {
							collidables[j]->SetVisible(false);
						}
						if (childmissiles[i].size() > 5) {
							childmissiles[i][k]->SetVisible(false);
							childmissiles[i].pop_front();
						}
					}
				}
			}
		}
	}
	for (int j = 0; j < hostages.size(); j++) {
		
		if (!hostcollected[j]) {
			hp = hostages[j]->GetPosition();
			ha = heli->GetPosition() - hp;

			if (((std::sqrt(std::pow(ha[0], 2) + std::pow(ha[1], 2) + std::pow(ha[2], 2))) <= 1.0)) {
				hostcollected[j] = true;
			}
		}
	}
	for (int j = 0; j < enemies.size(); j++) {
		if (glm::length(enemies[j]->GetPosition() - heli->GetPosition()) <= 200.0f) {
			enemies[j]->SetAgro(true);
		}
		else {
			enemies[j]->SetAgro(false);
		}

	}

	for (int j = 0; j < buildings.size(); j++) {
		//insert sphere+boundingbox collision here
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
	SceneNode *laser = new SceneNode(entity_name, geom, mat);
	laser->Scale(glm::vec3(1.0, 1.0, 40));
	scene_.GetNode("Root")->AddChild(laser);
	float off = 0.0;
	lazerref = laser;
	
	for (int i = 0; i < hostages.size(); ++i) {
		SceneNode *laser = new SceneNode(entity_name, geom, mat);
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
	SceneNode *missile = new SceneNode(entity_name, geom, mat);
	
	missile->SetVisible(true);
	missile->SetPosition(this->heli->GetPosition());
	missile->SetOrientation(this->camera_.GetOrientation());

	scene_.GetNode("Root")->AddChild(missile);
	float off = 0.0;
	missile->direction = camera_.GetForward();
	missiles.push_back(missile);

	// Create Missiles for children
	
	for (int i = 0; i < childmissiles.size(); i++) {
		if (hostcollected[i]) {
			SceneNode *childmis = new SceneNode(entity_name, geom, mat);

			childmis->SetVisible(true);
			childmis->SetPosition(hostages[i]->GetPosition());
			childmis->SetOrientation(this->camera_.GetOrientation());

			scene_.GetNode("Root")->AddChild(childmis);
			float off = 0.0;
			childmis->direction = camera_.GetForward();
			childmissiles[i].push_back(childmis);
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
	SceneNode *missile = new SceneNode(entity_name, geom, mat);

	missile->SetVisible(true);
	//missile->Scale(glm::vec3(10.0));
	missile->SetPosition(enemy->GetPosition());
	missile->SetOrientation(enemy->GetOrientation());

	scene_.GetNode("Root")->AddChild(missile);
	float off = 0.0;
	missile->direction = enemy->GetForward();
	enemymissiles.push_back(missile);

}

void Game::CreateAsteroidField(int num_asteroids){

    // Create a number of asteroid instances
    for (int i = 0; i < 0; i++){
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
        ast->SetPosition(glm::vec3(-300.0 + 600.0*((float) rand() / RAND_MAX), 0 + 600.0*((float) rand() / RAND_MAX), 600.0*((float) rand() / RAND_MAX)));
        ast->SetOrientation(glm::normalize(glm::angleAxis(glm::pi<float>()*((float) rand() / RAND_MAX), glm::vec3(((float) rand() / RAND_MAX), ((float) rand() / RAND_MAX), ((float) rand() / RAND_MAX)))));
        ast->SetAngM(glm::normalize(glm::angleAxis(0.05f*glm::pi<float>()*((float) rand() / RAND_MAX), glm::vec3(((float) rand() / RAND_MAX), ((float) rand() / RAND_MAX), ((float) rand() / RAND_MAX)))));
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
	SceneNode *node = new SceneNode(entity_name, geom, mat);
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
	SceneNode *node = new SceneNode(entity_name, geom, mat);
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
	Helicopter* node = new Helicopter(entity_name, geom, mat);
	scene_.GetNode(parent_name)->AddChild(node);
	return node;
}

} // namespace game

