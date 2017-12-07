#version 400

// Vertex buffer
in vec3 vertex;
in vec3 normal;
in vec3 color;

// Uniform (global) buffer
uniform mat4 world_mat;
uniform mat4 view_mat;
uniform float timer;
uniform vec3 control_point[64];
uniform vec3 up_vec;

// Attributes forwarded to the geometry shader
out float particle_id;

// Define some useful constants
const float pi = 3.1415926536;
const float pi_over_two = 1.5707963268;
const float two_pi = 2.0*pi;


void main()
{   
    // Define particle id
    particle_id = color.r; // Derived from the particle color. We use the id to keep track of particles

    // The phase of the particle repeats in a cyclic manner and is dependent of the particle id
    float phase = two_pi*particle_id;
    float circtime = mod((timer / 1.5 + phase), 24.0); // A cycle lasts 16 seconds
    
    // Change position of the particle based on a spline
    vec3 position = vertex;
    float t = circtime - floor(circtime); // Fractional part, 0-1
    // Spline evaluation
    float p1w = (1 - t)*(1 - t)*(1 - t);
    float p2w = 3 * t*(1 - t)*(1 - t);
    float p3w = 3 * t*t*(1 - t);
    float p4w = t*t*t; 
    int wsec = int(floor(circtime))*4; // Picks which set of control points are used           
    vec3 Bt = p1w*control_point[0+wsec] + p2w*control_point[1+wsec] + p3w*control_point[2+wsec] + p4w*control_point[3+wsec];
    position += Bt;

    // Transform new position
    vec4 out_position = view_mat * world_mat * vec4(position, 1.0);
    gl_Position = out_position;
}
