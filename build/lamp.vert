#version 330 core

//vertex data
in vec3 position;

//values that stay constant for whole mesh
uniform mat4 MVP;

void main()
{
	//output position of vertex - MVP position
    gl_Position = MVP*vec4(position,1.0f);
}
