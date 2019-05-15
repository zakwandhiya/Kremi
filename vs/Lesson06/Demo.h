#ifndef DEMO_H
#define DEMO_H


#include <SOIL/SOIL.h>

#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>

#include <ft2build.h>
#include <freetype/freetype.h>
#include <map>

#include "Game.h"

#define NUM_FRAMES 1
#define FRAME_DUR 80

#define FONTSIZE 40
#define FONTNAME "hongkonghustle.ttf"
#define NUM_BUTTON 3


using namespace glm;

struct Character {
	GLuint TextureID; // ID handle of the glyph texture
	ivec2 Size; // Size of glyph
	ivec2 Bearing; // Offset from baseline to left/top of glyph
	GLuint Advance; // Offset to advance to next glyph
};


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
		gravity = 0, xVelocity = .2, 
		yVelocity = 0, yposGround = 0,
		xBackgroundPos[3], yBackgroundPos[3];
	bool isPaused = false;
	GLuint VBO, VAO, EBO, texture, program, VBO2, VAO2, EBO2, textureBackgrounds[3], backgrounds[3];
	GLuint obs_vbo[10], obs_vao[10], obs_ebo[10], obs_texture[10], obs_program[10];
	GLuint text_vao, text_vbo, text_shader_program;
	int obsLength = 10, obs_distance = 200;
	float velocity = 5;
	float obs_x_pos[10], obs_y_pos[10];
	float obs_frame_width[10], obs_frame_height[10];
	map<GLchar, Character> Characters;
	int score = 0;
	void InitText();
	void RenderText(string text, GLfloat x, GLfloat y, GLfloat scale, vec3 color);
	void ResetAllObstacles();
	void CheckCollisions();
	void BuildObstacles();
	void BuildObstacleSprite(int i);
	void DrawObstacles();
	void DrawObstacleSprite(int i);
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

