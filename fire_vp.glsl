#version 400

// Vertex buffer
in vec3 vertex;
in vec3 normal;
in vec3 color;

// Uniform (global) buffer
uniform mat4 world_mat;
uniform mat4 view_mat;
uniform mat4 normal_mat;
uniform float timer;

// Attributes forwarded to the geometry shader
out vec4 particle_color;
out float particle_id;
out float timestep;

// Simulation parameters (constants)
uniform vec3 up_vec = vec3(0.0, 1.0, 0.0);
uniform vec3 object_color = vec3(0.8, 0.8, 0.8);
float grav = 0.5; // Gravity
float speed = 2.5; // Allows to control the speed of the explosion


void main()
{
    // Define particle id
    particle_id = color.r; // Derived from the particle color. We use the id to keep track of particles
	
	// Let time cycle every four seconds
    float circtime = timer - 4.0 * floor(timer / 4);
    float t = circtime; // Our time parameter
    
    // Let's first work in model space (apply only world matrix)
    vec4 position = world_mat * vec4(vertex, 1.0);
    vec4 norm = normal_mat * vec4(normal, 1.0);
	if(t<0.2){
		position.y += t*speed ;
	}
	else{
		// Move point along normal and down with t*t (acceleration under gravity)
		position.x += norm.x*t*speed - grav*speed*up_vec.x*t*t;
		position.y += norm.y*t*speed - grav*speed*up_vec.y*t*t + t*speed ;
		position.z += norm.z*t*speed - grav*speed*up_vec.z*t*t;
	}
    
    
    // Now apply view transformation
    gl_Position = view_mat * position;
        
    // Define color of vertex
    //vertex_color = color.rgb; // Color defined during the construction of the particles
   // vertex_color = object_color; // Uniform color 
    //vertex_color = vec3(t, 0.0, 1-t);
    //vertex_color = vec3(1.0, 1-t, 0.0);

	 // Define amount of blending depending on the cyclic time
    float alpha = 1.0 - circtime/2;
    particle_color = vec4(color, alpha);

    timestep = t;
}
