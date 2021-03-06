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
	BuildObstacles();
	BuildPlayerSprite();
	BuildBackgroundSprite();
	InitText();
	PlayMusic();
}

void Demo::PlayMusic() {
	Mix_Music *music = Mix_LoadMUS("music.wav");
	Mix_PlayMusic(music, -1);
}

void Demo::PlayCrashSound() {
	Mix_Chunk *sound = Mix_LoadWAV("crash.wav");
	sound->volume = 64;
	Mix_PlayChannel(-1, sound, 0);
}

void Demo::Update(float deltaTime)
{
	if (IsKeyDown("Quit")) {
		SDL_Quit();
		exit(0);
	}

	if (IsKeyDown("Jump")) {
		velocity = 5;
		xVelocity = .2;
		obs_distance = 200;
		ResetAllObstacles();
		score = 0;
		isPaused = false;
	}

	if (isPaused) return;
	
	ControlPlayerSprite(deltaTime);
	UpdateBackground();
	ControlObstacleSprite(deltaTime);

	score++;
}

void Demo::UpdateBackground() {
	yBackgroundPos[0] += velocity / 3;
	for (int i = 1; i < bgLength; i++) {
		yBackgroundPos[i] = yBackgroundPos[i - 1] - frameHeightBackgrounds[i - 1];
	}

	//cout << "screen width " << GetScreenWidth() << endl;
	//cout << obs_frame_width[0] << endl;

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
	DrawObstacles();
	DrawPlayerSprite();
	CheckCollisions();

	std::string out_string;
	std::stringstream ss;
	ss << score;
	out_string = ss.str();
	
	RenderText(out_string, 10, 10, 1.0f, vec3(244.0f / 255.0f, 12.0f / 255.0f, 116.0f / 255.0f));
}

void Demo::CheckCollisions() {
	if (isPaused) return;

	for (int i = 0; i < obsLength; i++) {
		if (IsCollided(
			obs_x_pos[i], obs_y_pos[i], obs_frame_width[i], obs_frame_height[i],
			xpos, ypos, frame_width, frame_height
		)) {
			isPaused = true;
			PlayCrashSound();
		}
	}
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

	if (xpos > GetScreenWidth() - frame_width) {
		xpos = GetScreenWidth() - frame_width;
	} else if (xpos < 0) {
		xpos = 0;
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

void Demo::BuildObstacles() {
	for (int i = 0; i < obsLength; i++) {
		BuildObstacleSprite(i);
	}

	ResetAllObstacles();
}

void Demo::BuildObstacleSprite(int i)
{
	this->obs_program[i] = BuildShader("background1.vert", "background1.frag");
	UseShader(this->obs_program[i]);

	//glUniform1f(glGetUniformLocation(this->obs_program, "n"), 1.0f / NUM_FRAMES);

	// Load and create a texture 
	glGenTextures(1, &this->obs_texture[i]);
	glBindTexture(GL_TEXTURE_2D, obs_texture[i]); // All upcoming GL_TEXTURE_2D operations now have effect on our texture object

	// Set texture filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Load, create texture 
	int width, height;
	unsigned char* image = SOIL_load_image("obstacle2.png", &width, &height, 0, SOIL_LOAD_RGBA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture when done, so we won't accidentily mess up our texture.

	// Set up vertex data (and buffer(s)) and attribute pointers

	obs_frame_width[i] = (float)width;
	obs_frame_height[i] = (float)height;
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

	glGenVertexArrays(1, &obs_vao[i]);
	glGenBuffers(1, &obs_vbo[i]);
	glGenBuffers(1, &obs_ebo[i]);

	glBindVertexArray(obs_vao[i]);

	glBindBuffer(GL_ARRAY_BUFFER, obs_vbo[i]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obs_ebo[i]);
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
	glUniformMatrix4fv(glGetUniformLocation(this->obs_program[i], "projection"), 1, GL_FALSE, value_ptr(projection));

	//obs_x_pos[i] = GetScreenWidth() / 2;
	//obs_y_pos[i] = GetScreenHeight() / 2;
}

void Demo::DrawObstacles() {
	for (int i = 0; i < obsLength; i++) {
		DrawObstacleSprite(i);
	}
}

void Demo::DrawObstacleSprite(int i)
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Bind Textures using texture units
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, obs_texture[i]);
	UseShader(this->obs_program[i]);
	glUniform1i(glGetUniformLocation(this->obs_program[i], "ourTexture"), 0);

	// set flip
	//glUniform1i(glGetUniformLocation(this->obs_program, "flip"), flip);
	mat4 model;
	// Translate sprite along x-axis
	model = translate(model, vec3(obs_x_pos[i], obs_y_pos[i], 0.0f));
	// Scale sprite 
	model = scale(model, vec3(obs_frame_width[i], obs_frame_height[i], 1));
	glUniformMatrix4fv(glGetUniformLocation(this->obs_program[i], "model"), 1, GL_FALSE, value_ptr(model));

	// Draw sprite
	glBindVertexArray(obs_vao[i]);
	glDrawElements(GL_QUADS, 4, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture when done, so we won't accidentily mess up our texture.
	glDisable(GL_BLEND);
}

void Demo::ControlObstacleSprite(float delta_time)
{
	for (int i = 0; i < obsLength; i++) {
		obs_y_pos[i] += velocity;
	}
	if (obs_y_pos[obsLength - 1] > GetScreenHeight()) {
		ResetAllObstacles();
		velocity += 1;
		xVelocity += .05;
		obs_distance += 50;

		if (velocity > 20) velocity = 20;
		if (xVelocity > .5) xVelocity = .5;
		if (obs_distance > 300) obs_distance = 300;
	}
}

void Demo::ResetAllObstacles() {
	for (int i = 0; i < obsLength; i++) {
		float random = rand() % 100 + 1;
		random /= 100;

		if (i % 2 == 0) {
			cout << random << endl;
			obs_x_pos[i] = (float)-random * (obs_frame_width[i] - 250) - 230;
			cout << obs_x_pos[i] << " " << GetScreenHeight() << " " << obs_frame_width[i] << endl;
			obs_y_pos[i] = (float)(-obs_frame_height[i] * (i / 2) - obs_distance * (i / 2)) - obs_frame_height[i];
		}
		else {
			obs_x_pos[i] = (float)obs_x_pos[i - 1] + obs_frame_width[i] + 200;
			obs_y_pos[i] = (float)obs_y_pos[i - 1];
		}
	}
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
	unsigned char* image = SOIL_load_image("charfit.png", &width, &height, 0, SOIL_LOAD_RGBA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture when done, so we won't accidentily mess up our texture.

	// Set up vertex data (and buffer(s)) and attribute pointers
	/*frame_width = ((float)width) / NUM_FRAMES;
	frame_height = (float)height;*/

	frame_width = (float)width / 6;
	frame_height = (float)height / 6;
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
	yposGround = GetScreenHeight() - frame_height - 100;
	ypos = yposGround;
	gravity = 0.05f;

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
	/*char* imageFileNameInChar = new char[imageFileName.length() + 1];
	strcpy(imageFileNameInChar, imageFileName.c_str());*/
	int width, height;
	unsigned char* image = SOIL_load_image("bg.png", &width, &height, 0, SOIL_LOAD_RGBA);
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

void Demo::InitText() {
	this->text_shader_program = BuildShader("text_shader.vert", "text_shader.frag");

	// Init Freetype
	FT_Library ft;
	if (FT_Init_FreeType(&ft)) {
		Err("ERROR::FREETYPE: Could not init FreeType Library");
	}
	FT_Face face;
	if (FT_New_Face(ft, FONTNAME, 0, &face)) {
		Err("ERROR::FREETYPE: Failed to load font");
	}

	FT_Set_Pixel_Sizes(face, 0, FONTSIZE);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Disable byte-alignment restriction
	for (GLubyte c = 0; c < 128; c++)
	{
		// Load character glyph
		if (FT_Load_Char(face, c, FT_LOAD_RENDER))
		{
			std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
			continue;
		}
		// Generate texture
		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RED,
			face->glyph->bitmap.width,
			face->glyph->bitmap.rows,
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			face->glyph->bitmap.buffer
		);
		// Set texture options
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// Now store character for later use
		Character character = {
			texture,
			ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
			ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
			face->glyph->advance.x
		};
		Characters.insert(pair<GLchar, Character>(c, character));
	}

	FT_Done_Face(face);
	FT_Done_FreeType(ft);

	glGenVertexArrays(1, &this->text_vao);
	glGenBuffers(1, &this->text_vbo);
	glBindVertexArray(this->text_vao);
	glBindBuffer(GL_ARRAY_BUFFER, this->text_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 4 * 4, NULL,
		GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	UseShader(this->text_shader_program);
	// Set orthographic projection
	mat4 projection;
	projection = ortho(0.0f, static_cast<GLfloat>(GetScreenWidth()), static_cast<GLfloat>(GetScreenHeight()), 0.0f, -1.0f, 1.0f);
	glUniformMatrix4fv(glGetUniformLocation(this->text_shader_program, "projection"), 1, GL_FALSE, value_ptr(projection));

}

void Demo::RenderText(string text, GLfloat x, GLfloat y, GLfloat scale, vec3 color)
{
	// Activate corresponding render state
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	UseShader(this->text_shader_program);

	glUniform3f(glGetUniformLocation(this->text_shader_program, "ourColor"), color.x, color.y, color.z);
	glUniform1i(glGetUniformLocation(this->text_shader_program, "text"), 1);
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(this->text_shader_program, "ourTexture"), 0);
	mat4 model;
	glUniformMatrix4fv(glGetUniformLocation(this->text_shader_program, "model"), 1, GL_FALSE, value_ptr(model));
	glBindVertexArray(this->text_vao);

	// Iterate through all characters
	string::const_iterator c;
	for (c = text.begin(); c != text.end(); c++)
	{
		Character ch = Characters[*c];
		GLfloat xpos = x + ch.Bearing.x * scale;
		GLfloat ypos = y + (this->Characters['H'].Bearing.y - ch.Bearing.y) * scale;
		GLfloat w = ch.Size.x * scale;
		GLfloat h = ch.Size.y * scale;
		// Update VBO for each character

		GLfloat vertices[] = {
			// Positions   // Texture Coords
			xpos + w,	ypos + h,	1.0f, 1.0f, // Bottom Right
			xpos + w,	ypos,		1.0f, 0.0f, // Top Right
			xpos,		ypos,		0.0f, 0.0f, // Top Left
			xpos,		ypos + h,	0.0f, 1.0f  // Bottom Left 
		};
		// Render glyph texture over quad
		glBindTexture(GL_TEXTURE_2D, ch.TextureID);
		// Update content of VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, this->text_vbo);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		// Render quad
		glDrawArrays(GL_QUADS, 0, 4);
		// Now advance cursors for next glyph (note that advance is number of 1 / 64 pixels)
		x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels(2 ^ 6 = 64)
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_BLEND);
}



int main(int argc, char** argv) {

	Engine::Game &game = Demo();
	game.Start("Collision Detection using AABB", 480, 768, false, WindowFlag::WINDOWED, 60, 1);
	//game.Start("Collision Detection using AABB", 1600, 2560, false, WindowFlag::WINDOWED, 60, 1);
	return 0;
}

