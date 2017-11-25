#version 130

// Attributes passed from the vertex shader
in vec3 position_interp;
in vec3 normal_interp;
in vec3 light_pos;

float phong_exponent = 90.0;




void main()
{
     vec3 N, // Interpolated normal for fragment
         L, // Light-source direction
         V,
         R;//Reflection direction
		 
	N = normalize(normal_interp);
        L = (light_pos - position_interp);
        L = normalize(L);
        R = (2*(dot(L,N))*N) - L;
        V = - position_interp; // Eye position is (0, 0, 0) in view coordinates
        V = normalize(V);
        float Id = max(dot(N, L), 0.0);
        //compute specular term for phong shading
        float spec_angle_cos = max(dot(V, R), 0.0);
        float Is = pow(spec_angle_cos, phong_exponent); 
	//used to determine intensity of shade
	float intensity = Id + Is;
	vec4 color;        
	float df = 0.8;	
	color = vec4(1.0,1.0,0.4,1.0);

	if (intensity > 0.95)
		color = vec4(1.0,1.0,0.4,1.0);
	else if (intensity > 0.5){
		color = vec4(color[0] * df, color[1] * df, color[2] * df,1.0);
		}
	else if (intensity > 0.25){
		color = vec4(color[0] * df, color[1] * df, color[2] * df,1.0);
		color = vec4(color[0] * df, color[1] * df, color[2] * df,1.0);
		}
	else{
		color = vec4(color[0] * df, color[1] * df, color[2] * df,1.0);
		color = vec4(color[0] * df, color[1] * df, color[2] * df,1.0);
		color = vec4(color[0] * df, color[1] * df, color[2] * df,1.0);
		}

//colors black in places where threshold less than a value to draw outline 
        if(max(dot(N,V),0.0)<0.0)
                color = vec4(0.0,0.0,0.0,1.0);
	gl_FragColor = color;

}
