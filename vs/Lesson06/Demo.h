#ifndef DEMO_H
#define DEMO_H


#include <SOIL/SOIL.h>

#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>

#include "Game.h"

#define NUM_FRAMES 1
#define FRAME_DUR 80

using namespace glm;

class Demo :
	public Engine::Game
{
public:
	Demo();
	~Demo();
	virtual void Init();
	virtual void Update(float deltaTime);
	virtual void Render();
	int bgLength = 3;
	float frame_width = 0, frame_height = 0,
		frameWidthBackgrounds[3],
		frameHeightBackgrounds[3];
private:
	float frame_dur = 0, 
		oldxpos = 0, oldypos = 0, 
		xpos = 0, ypos = 0, 
		gravity = 0, xVelocity = 0, 
		yVelocity = 0, yposGround = 0,
		xBackgroundPos[3], yBackgroundPos[3];
	GLuint VBO, VAO, EBO, texture, program, VBO2, VAO2, EBO2, textureBackgrounds[3], backgrounds[3];
	GLuint obs_vbo, obs_vao, obs_ebo, obs_texture, obs_program;
	float obs_x_pos = 0, obs_y_pos = 0;
	float obs_frame_width = 0, obs_frame_height = 0;
	void BuildObstacleSprite();
	void DrawObstacleSprite();
	void ControlObstacleSprite(float delta_time);
	bool walk_anim = false, onGround = true;
	unsigned int frame_idx = 0, flip = 0;
	void BuildPlayerSprite();
	void UpdateBackground();
	void BuildBackgroundSprite();
	void BuildBackground(int i);
	void DrawBackgroundSprite();
	void DrawBackground(int i);
	bool IsCollided(float x1, float y1, float width1, float height1, float x2, float y2, float width2, float height2);
	void DrawPlayerSprite();
	void UpdatePlayerSpriteAnim(float deltaTime);
	void ControlPlayerSprite(float deltaTime);
};
#endif

