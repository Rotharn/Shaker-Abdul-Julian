#version 400

// Attributes passed from the geometry shader
in vec2 tex_coord;

// Uniform (global) buffer
uniform sampler2D texture_map;

// Simulation parameters (constants)
uniform vec4 particle_color = vec4(0.1, 0.4, 0.7, 1.0);


void main (void)
{
    // Color spline based on constant color
    gl_FragColor = vec4(particle_color.r, particle_color.g, particle_color.b, 0.5);
}
