#include "Demo.h"
#pragma warning(disable: 4996)


Demo::Demo()
{
}


Demo::~Demo()
{
}

void Demo::Init()
{
	BuildPlayerSprite();
	BuildBackgroundSprite();
}

void Demo::Update(float deltaTime)
{
	if (IsKeyDown("Quit")) {
		SDL_Quit();
		exit(0);
	}
	
	ControlPlayerSprite(deltaTime);
	UpdateBackground();
}

void Demo::UpdateBackground() {
	yBackgroundPos[0] += 1.5;
	for (int i = 1; i < bgLength; i++) {
		yBackgroundPos[i] = yBackgroundPos[i - 1] - frameHeightBackgrounds[i - 1];
	}

	if (yBackgroundPos[0] > frameHeightBackgrounds[0] * (bgLength - 1)) {
		yBackgroundPos[0] = 0;
	}
}

void Demo::Render()
{
	//Setting Viewport
	glViewport(0, 0, GetScreenWidth(), GetScreenHeight());

	//Clear the color and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Set the background color
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

	DrawBackgroundSprite();
	DrawPlayerSprite();
	

}

void Demo::UpdatePlayerSpriteAnim(float deltaTime)
{
	// Update animation
	frame_dur += deltaTime;

	if (walk_anim && frame_dur > FRAME_DUR) {
		frame_dur = 0;
		if (frame_idx == NUM_FRAMES - 1) frame_idx = 0;  else frame_idx++;

		// Pass frameIndex to shader
		UseShader(this->program);
		glUniform1i(glGetUniformLocation(this->program, "frameIndex"), frame_idx);
	}
}

void Demo::ControlPlayerSprite(float deltaTime)
{
	walk_anim = false;
	oldxpos = xpos;
	oldypos = ypos;

	bool above = false;

	if (IsKeyDown("Move Right")) {
		xpos += deltaTime * xVelocity;
		flip = 0;
		walk_anim = true;
	}

	if (IsKeyDown("Move Left")) {
		xpos -= deltaTime * xVelocity;
		flip = 1;
		walk_anim = true;
	}

	/*if (IsKeyDown("Jump")) {
		if (onGround) {
			yVelocity = -12.0f;
			onGround = false;
		}
	}

	if (IsKeyUp("Jump")) {
		if (yVelocity < -6.0f) {
			yVelocity = -6.0f;
		}
	}*/

	yVelocity += gravity * deltaTime;
	ypos += deltaTime * yVelocity;



	if (ypos > yposGround) {
		ypos = yposGround;
		yVelocity = 0;
		onGround = true;
	}

	// check collision between bart and crate
	/*if (IsCollided(xpos, ypos, frame_width, frame_height, xpos2, ypos2, frame_width2, frame_height2) 
		&& (xpos < xpos2 + frame_width2 && xpos + frame_width > xpos2)) {
		ypos = oldypos;

	}

	if (IsCollided(xpos, ypos, frame_width, frame_height, xpos2, ypos2, frame_width2, frame_height2)
		&& (ypos < ypos2 + frame_height2 )) {
		xpos = oldxpos;

	}*/


	/*if (xpos < xpos2 + frame_width2 && xpos + frame_width > xpos2) {

		xpos = oldxpos;
	}


	if (ypos < ypos2 + frame_height2 && ypos + frame_height > ypos2) {
		ypos = oldypos;
	}*/
}



void Demo::DrawPlayerSprite() {
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Bind Textures using texture units
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	UseShader(this->program);
	glUniform1i(glGetUniformLocation(this->program, "ourTexture"), 0);

	// set flip
	glUniform1i(glGetUniformLocation(this->program, "flip"), flip);
	mat4 model;
	// Translate sprite along x-axis
	model = translate(model, vec3(xpos, ypos, 0.0f));
	// Scale sprite 
	model = scale(model, vec3(frame_width, frame_height, 1));
	glUniformMatrix4fv(glGetUniformLocation(this->program, "model"), 1, GL_FALSE, value_ptr(model));

	// Draw sprite
	glBindVertexArray(VAO);
	glDrawElements(GL_QUADS, 4, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture when done, so we won't accidentily mess up our texture.
	glDisable(GL_BLEND);
}

void Demo::BuildPlayerSprite()
{
	this->program = BuildShader("playerSprite.vert", "playerSprite.frag");

	// Pass n to shader
	UseShader(this->program);
	glUniform1f(glGetUniformLocation(this->program, "n"), 1.0f / NUM_FRAMES);

	// Load and create a texture 
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture); // All upcoming GL_TEXTURE_2D operations now have effect on our texture object

	// Set texture filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Load, create texture 
	int width, height;
	unsigned char* image = SOIL_load_image("Utama3.png", &width, &height, 0, SOIL_LOAD_RGBA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture when done, so we won't accidentily mess up our texture.

	// Set up vertex data (and buffer(s)) and attribute pointers
	/*frame_width = ((float)width) / NUM_FRAMES;
	frame_height = (float)height;*/

	frame_width = ((float)width / 30);
	frame_height = (float)height / 30;
	GLfloat vertices[] = {
		// Positions   // Colors           // Texture Coords
		1,  1, 0.0f,   1.0f, 1.0f, 1.0f,   1.0f, 1.0f, // Bottom Right
		1,  0, 0.0f,   1.0f, 1.0f, 1.0f,   1.0f, 0.0f, // Top Right
		0,  0, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 0.0f, // Top Left
		0,  1, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 1.0f  // Bottom Left 
	};

	GLuint indices[] = {  // Note that we start from 0!
		0, 3, 2, 1
	};

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray(0);
	// Color attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	// TexCoord attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	glBindVertexArray(0); // Unbind VAO

	// Set orthographic projection
	mat4 projection;
	projection = ortho(0.0f, static_cast<GLfloat>(GetScreenWidth()), static_cast<GLfloat>(GetScreenHeight()), 0.0f, -1.0f, 1.0f);
	glUniformMatrix4fv(glGetUniformLocation(this->program, "projection"), 1, GL_FALSE, value_ptr(projection));

	// set sprite position, gravity, velocity
	xpos = (GetScreenWidth() - frame_width) / 2;
	yposGround = GetScreenHeight() - frame_height;
	ypos = yposGround;
	gravity = 0.05f;
	xVelocity = 0.1f;

	// Add input mapping
	// to offer input change flexibility you can save the input mapping configuration in a configuration file (non-hard code) !
	InputMapping("Move Right", SDLK_RIGHT);
	InputMapping("Move Left", SDLK_LEFT);
	InputMapping("Move Right", SDLK_d);
	InputMapping("Move Left", SDLK_a);
	InputMapping("Move Right", SDL_BUTTON_RIGHT);
	InputMapping("Move Left", SDL_BUTTON_LEFT);
	InputMapping("Move Right", SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
	InputMapping("Move Left", SDL_CONTROLLER_BUTTON_DPAD_LEFT);
	InputMapping("Quit", SDLK_ESCAPE);
	InputMapping("Jump", SDLK_SPACE);
	InputMapping("Jump", SDL_CONTROLLER_BUTTON_A);
}

void Demo::BuildBackgroundSprite() {
	for (int i = 0; i < bgLength; i++) {
		BuildBackground(i);
	}
}

void Demo::BuildBackground(int i)
{
	string vertFileName = "background" + std::to_string(i + 1) + ".vert";
	string fragFileName = "background" + std::to_string(i + 1) + ".frag";
	/*char* vertFileNameInChar = new char[vertFileName.length() + 1];
	strcpy(vertFileNameInChar, vertFileName.c_str());
	char* fragFileNameInChar = new char[fragFileName.length() + 1];
	strcpy(fragFileNameInChar, fragFileName.c_str());*/

	//this->backgrounds[i] = BuildShader(vertFileName.c_str(), fragFileName.c_str());
	this->backgrounds[i] = BuildShader("background1.vert", "background1.frag");
	UseShader(this->backgrounds[i]);

	// Load and create a texture 
	glGenTextures(1, &(textureBackgrounds[i]));
	glBindTexture(GL_TEXTURE_2D, textureBackgrounds[i]); // All upcoming GL_TEXTURE_2D operations now have effect on our texture object

	// Set texture filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Load, create texture 
	int realIndex = i + 1;
	if (i == bgLength - 1) realIndex = 1;
	string imageFileName = "M" + std::to_string(realIndex) + ".png";
	/*char* imageFileNameInChar = new char[imageFileName.length() + 1];
	strcpy(imageFileNameInChar, imageFileName.c_str());*/
	int width, height;
	unsigned char* image = SOIL_load_image(imageFileName.c_str(), &width, &height, 0, SOIL_LOAD_RGBA);
	//unsigned char* image = SOIL_load_image("M1.png", &width, &height, 0, SOIL_LOAD_RGBA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture when done, so we won't accidentily mess up our texture.

	// Set up vertex data (and buffer(s)) and attribute pointers
	frameWidthBackgrounds[i] = (float) GetScreenWidth();
	frameHeightBackgrounds[i] = (float) GetScreenHeight();
	GLfloat vertices[] = {
		// Positions   // Colors           // Texture Coords
		1,  1, 0.0f,   1.0f, 1.0f, 1.0f,   1.0f, 1.0f, // Bottom Right
		1,  0, 0.0f,   1.0f, 1.0f, 1.0f,   1.0f, 0.0f, // Top Right
		0,  0, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 0.0f, // Top Left
		0,  1, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 1.0f  // Bottom Left 
	};

	GLuint indices[] = {  // Note that we start from 0!
		0, 3, 2, 1
	};

	glGenVertexArrays(1, &VAO2);
	glGenBuffers(1, &VBO2);
	glGenBuffers(1, &EBO2);

	glBindVertexArray(VAO2);

	glBindBuffer(GL_ARRAY_BUFFER, VBO2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO2);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray(0);
	// Color attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	// TexCoord attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	glBindVertexArray(0); // Unbind VAO

	// Set orthographic projection
	mat4 projection;
	projection = ortho(0.0f, static_cast<GLfloat>(GetScreenWidth()), static_cast<GLfloat>(GetScreenHeight()), 0.0f, -1.0f, 1.0f);
	glUniformMatrix4fv(glGetUniformLocation(this->backgrounds[i], "projection"), 1, GL_FALSE, value_ptr(projection));

	// set sprite position, gravity, velocity
	xBackgroundPos[i] = (GetScreenWidth() - frameWidthBackgrounds[i]) / 4;
	yBackgroundPos[i] = (float) 0;
	if (i != 0) {
		yBackgroundPos[i] = yBackgroundPos[i - 1] - frameHeightBackgrounds[i - 1];
	}
}

void Demo::DrawBackgroundSprite() {
	for (int i = 0; i < bgLength; i++) {
		DrawBackground(i);
	}
}

void Demo::DrawBackground(int i) {
	// Bind Textures using texture units
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, textureBackgrounds[i]);
	// Activate shader
	UseShader(this->backgrounds[i]);
	glUniform1i(glGetUniformLocation(this->backgrounds[i], "ourTexture"), 1);

	mat4 model;
	// Translate sprite along x-axis
	model = translate(model, vec3(xBackgroundPos[i], yBackgroundPos[i], 0.0f));
	// Scale sprite 
	model = scale(model, vec3(frameWidthBackgrounds[i], frameHeightBackgrounds[i], 1));
	glUniformMatrix4fv(glGetUniformLocation(this->backgrounds[i], "model"), 1, GL_FALSE, value_ptr(model));

	// Draw sprite
	glBindVertexArray(VAO2);
	glDrawElements(GL_QUADS, 4, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture when done, so we won't accidentily mess up our texture.
}

bool Demo::IsCollided(float x1, float y1, float width1, float height1,
	float x2, float y2, float width2, float height2) {
	return (x1 < x2 + width2 && x1 + width1 > x2 && y1 < y2 + height2 && y1 + height1 > y2);
}



int main(int argc, char** argv) {

	Engine::Game &game = Demo();
	game.Start("Collision Detection using AABB", 480, 768, false, WindowFlag::WINDOWED, 60, 1);
	//game.Start("Collision Detection using AABB", 1600, 2560, false, WindowFlag::WINDOWED, 60, 1);
	return 0;
}

