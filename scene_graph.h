#ifndef SCENE_GRAPH_H_
#define SCENE_GRAPH_H_

#include <string>
#include <vector>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "scene_node.h"
#include "resource.h"
#include "camera.h"
#include "helicopter.h"

namespace game {

	// Class that manages all the objects in a scene
	class SceneGraph {

	private:
		// Background color
		glm::vec3 background_color_;

		// Root of the hierarchy
		SceneNode * root_;

	public:
		SceneGraph(void);
		~SceneGraph();

		// Background color
		void SetBackgroundColor(glm::vec3 color);
		glm::vec3 GetBackgroundColor(void) const;

		// Set root of the hierarchy
		void SetRoot(SceneNode *node);
		// Find a scene node with a specific name
		SceneNode *GetNode(std::string node_name) const;

		// Draw the entire scene
		void Draw(Camera *camera);

		// Update entire scene
		void Update(void);

	}; // class SceneGraph

} // namespace game

#endif // SCENE_GRAPH_H_
