#include <chrono>
#include "cgmath.h"			// slee's simple math library
#include "cgut.h"			// slee's OpenGL utility
#include "Hit_object.h"
#include "main.h"
#include "irrKlang/irrKlang.h"
#pragma comment(lib, "irrKlang.lib")

//*************************************
// global constants
static const char* window_name = "cgbase - particle";
static const char* vert_shader_path = "../bin/shaders/particle.vert";
static const char* frag_shader_path = "../bin/shaders/particle.frag";


//*************************************
// include stb_image with the implementation preprocessor definition
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

//*************************************
//Func
//inline const float random_range(float min, float max) { return min + rand() / (RAND_MAX / (max - min)); }
std::vector<particle_t> parse_hit_object(std::string name);
std::vector<time_obj> parse_time(std::string name);
void music_init();
void text_initialize();
void render_text(std::string text, GLfloat _x, GLfloat _y, GLfloat scale, vec4 color, int fontID);

//*************************************
// window objects
GLFWwindow* window = nullptr;
ivec2		window_size = ivec2(720, 480);	// initial window size

//*************************************
// OpenGL objects
GLuint program = 0;	// ID holder for GPU program
GLuint vertex_buffer = 0;
GLuint sl = 0;
GLuint pbg = 0;
GLuint stage[2];
GLuint TEX[8];
GLuint key_button[4];
GLuint note_tex[2];
GLuint noteL_tex[2];
GLuint background[3];
GLuint pause_menu[2];

//*************************************
// global variables
int		frame = 0;				// index of rendering frames
int		num_images = 9;			// number of hit effect images
int		num_musics = 5;			// number of background images
int		which_music = 1;		// number for selecting music
int		time_index = 1;
int		playhead = 0;
int		pause_time = 0;
int		miss = 0;
int		max_combo = 0;
int		score = 0;
int		sync = 400;
int		key_pt[4], key_rt[4];
bool	b_wireframe = false;	// this is the default
bool	sound_play = false;		// checking if music stop
bool	playing = false;		// check if music is playing
bool	music_start = false;	// music start checker
bool	is_music_init = false;
bool	restart_check = false;
bool	beat[4] = { false };
bool	debug_time = true;
uint	play_offset = 0;		// offset for pause music
float	combo = 0;
float	total_hit = 0;
float	speed_scale = 1.0f;
float	aspect_ratio = float(window_size.x) / float(window_size.y);
std::vector<time_obj>	timing;
std::vector<particle_t> hit_obj;
std::vector<particle_t> particles;
std::vector<particle_t> note_p; // note vectors

particle_t pause_bg_p;
particle_t pause_menu_p[2];
particle_t background_p;		// background texture instance
particle_t button[4];			// button texture
particle_t stage_l;				// stage light texture
particle_t stage_lr[2];			// stage left right texture

std::chrono::time_point<std::chrono::system_clock>   start_timer;
std::chrono::time_point<std::chrono::system_clock>   pause_timer;
std::chrono::time_point<std::chrono::system_clock>   restart_timer;
std::chrono::time_point<std::chrono::system_clock>   music_timer;
std::chrono::time_point<std::chrono::system_clock>   key_press_timer[4];
std::chrono::time_point<std::chrono::system_clock>   key_release_timer[4];
std::chrono::duration<double, std::milli>			 key_interval[4];
struct note
{
	bool d = false;
	bool f = false;
	bool j = false;
	bool k = false;
} note;

irrklang::ISoundEngine* engine = nullptr;
irrklang::ISoundSource** sound_src = nullptr;
irrklang::ISound* player = nullptr;
//*************************************
void update()
{
	// setup texture
	
	if (sound_play && !playing)
	{
		// if music start parse the text file and put them into note_p vector
		if (!music_start)
		{
			music_init();
			music_start = true;
			//parse starts here
			switch (which_music)
			{
			case 1:
				hit_obj = std::move(parse_hit_object("DropZ-Line"));
				timing = std::move(parse_time("DropZ-Line"));
				break;
			case 2:
				hit_obj = std::move(parse_hit_object("C-Show - On the FM"));
				timing = std::move(parse_time("C-Show - On the FM"));
				break;
			case 3:
				hit_obj = std::move(parse_hit_object("anima"));
				timing = std::move(parse_time("anima"));
				break;
			case 4:
				hit_obj = std::move(parse_hit_object("play time"));
				timing = std::move(parse_time("play time"));
				break;
			case 5:
				hit_obj = std::move(parse_hit_object("Myosotis"));
				timing = std::move(parse_time("Myosotis"));
				break;
			//Add music here
			default:
				printf("ERROR: There is no such music %d.\n", which_music);
				exit(1);
			}
			start_timer = std::chrono::system_clock::now();
			pause_timer = std::chrono::system_clock::now();
			total_hit = 0;
			score = 0;
			max_combo = 0;
		}
		restart_check = true;
		playing = true;
		player->setPlayPosition(play_offset);
		player->setIsPaused(false);
		restart_timer = std::chrono::system_clock::now();
	}
	else if (!sound_play && is_music_init)
	{
		playing = false;
		play_offset = player->getPlayPosition();
		player->setIsPaused();
	}

	//If music play finished
	if (is_music_init&&player->isFinished())
	{
		sound_play = false;
		playing = false;
		music_start = false;
		is_music_init = false;
		play_offset = 0;
		pause_time = 0;
		combo = 0;
		miss = 0;
	}
	glUseProgram(program);
	GLint uloc = glGetUniformLocation(program, "aspect_ratio"); if (uloc > -1) glUniform1f(uloc, aspect_ratio);
}

void render()
{
	// clear screen (with background color) and clear depth buffer
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	// notify GL that we use our own program and buffers
	glUseProgram(program);
	
	// bind vertex attributes to your shader program
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	cg_bind_vertex_attributes(program);

	GLint uloc;
	//background image rendering-----------------------------------------------------------------------------------------------
	switch (which_music) 
	{
	case 1:
		glUniform1i(glGetUniformLocation(program, "TEX"), 20);
		break;
	case 2:
		glUniform1i(glGetUniformLocation(program, "TEX"), 21);
		break;
	case 3:
		glUniform1i(glGetUniformLocation(program, "TEX"), 22);
		break;
	case 4:
		glUniform1i(glGetUniformLocation(program, "TEX"), 23);
		break;
	case 5:
		glUniform1i(glGetUniformLocation(program, "TEX"), 24);
		break;
	//add cases when more background images are included
	default:
		printf("error: no background for music %d\n", which_music);
	}
	uloc = glGetUniformLocation(program, "color"); if (uloc > -1) glUniform4fv(uloc, 1, background_p.color);
	uloc = glGetUniformLocation(program, "center"); if (uloc > -1) glUniform2fv(uloc, 1, background_p.pos);
	uloc = glGetUniformLocation(program, "scale"); if (uloc > -1) glUniform2f(uloc, background_p.scale.x, background_p.scale.y);

	// render quad vertices
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	//stage rendering-----------------------------------------------------------------------------------------------------------
	for (int i = 1; i <= 2; i++)
	{
		glUniform1i(glGetUniformLocation(program, "TEX"), 13 + i);
		uloc = glGetUniformLocation(program, "color"); if (uloc > -1) glUniform4fv(uloc, 1, stage_lr[i-1].color);
		uloc = glGetUniformLocation(program, "center"); if (uloc > -1) glUniform2fv(uloc, 1, stage_lr[i-1].pos);
		uloc = glGetUniformLocation(program, "scale"); if (uloc > -1) glUniform2f(uloc, stage_lr[i-1].scale.x, stage_lr[i-1].scale.y);

		// render quad vertices
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}

	if (note.d) { particles[0].press = true; button[0].press = true; }
	else { particles[0].press = false; button[0].press = false; }
	if (note.f) { particles[1].press = true; button[1].press = true; }
	else { particles[1].press = false; button[1].press = false; }
	if (note.j) { particles[2].press = true; button[2].press = true; }
	else { particles[2].press = false; button[2].press = false; }
	if (note.k) { particles[3].press = true; button[3].press = true; }
	else { particles[3].press = false; button[3].press = false; }

	for (int i = 0; i < 4; i++)
	{
		if (button[i].press)
		{
			stage_l.color = (i == 0 || i == 3) ? vec4(0.004f,0.769f,0.973f, 0.9f) : vec4(0.573f,0.2f,0.976f, 0.9f);
			glUniform1i(glGetUniformLocation(program, "TEX"), 13);
			uloc = glGetUniformLocation(program, "color"); if (uloc > -1) glUniform4fv(uloc, 1, stage_l.color);
			uloc = glGetUniformLocation(program, "center"); if (uloc > -1) glUniform2fv(uloc, 1, button[i].pos + vec2(0, 1.0f) );
			uloc = glGetUniformLocation(program, "scale"); if (uloc > -1) glUniform2f(uloc, button[i].scale.x, button[i].scale.y+0.8f);

			// render quad vertices
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		}
	}

	//button---------------------------------------------------------------------------------------------------------------------------
	for (int i = 0; i < 4; i++)
	{
		if (i == 0 || i == 3)
		{
			if (!button[i].press)
			{
				glUniform1i(glGetUniformLocation(program, "TEX"), 9);
			}
			else
			{
				glUniform1i(glGetUniformLocation(program, "TEX"), 11);
			}
		}
		else
		{
			if (!button[i].press)
			{
				glUniform1i(glGetUniformLocation(program, "TEX"), 10);
			}
			else
			{
				glUniform1i(glGetUniformLocation(program, "TEX"), 12);
			}
		}
		uloc = glGetUniformLocation(program, "color"); if (uloc > -1) glUniform4fv(uloc, 1, button[i].color);
		uloc = glGetUniformLocation(program, "center"); if (uloc > -1) glUniform2fv(uloc, 1, button[i].pos);
		uloc = glGetUniformLocation(program, "scale"); if (uloc > -1) glUniform2f(uloc, button[i].scale.x, button[i].scale.y);

		// render quad vertices
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}

	//note ---------------------------------------------------------------------------------------------------------------------------------------------------
	
	if (playing)
	{
		if (restart_check) 
		{
			pause_time += int(std::chrono::duration<double, std::milli>(restart_timer - pause_timer).count()); 
			restart_check = false;
		}
		music_timer = std::chrono::system_clock::now();
		std::chrono::duration<double, std::milli> diff = music_timer - start_timer;
		//float long_note_scaler = 16.666666f;
		unsigned int playtime = int(diff.count()) + sync - pause_time;
		//printf("\rplaytime in milliseconds: %d", playtime);
		/*if ((playtime - sync) > (timing[time_index].time - 20) && (playtime - sync) < (timing[time_index].time + 20))
		{
			speed_scale = -1.0f / timing[time_index++].speed_scale;
		}*/
		for (auto& n : hit_obj)
		{
			if (debug_time == true)
			{
				if (n.note_mode != 128 && (-1000 + playtime > n.time || n.time > playtime + 1000))    continue;
				if (n.note_mode == 128)
				{
					int blocks = (n.time_L - n.time) / n.division + 1;
					int note_location = (int)(n.pos.x + 0.1f);
					int initial_judge = key_pt[note_location] - n.time;
					if (button[note_location].press && abs(initial_judge) < 200)
					{
						int pivot = 0;
						while (n.long_hit[pivot] == true)    pivot++;
						int judge = (int)diff.count() - (n.time + pivot * n.division) - 130;
						if (abs(judge) < 45 && n.long_hit[pivot] == false)
						{
							n.long_hit[pivot] = true;
							combo += 0.5f;
							total_hit += 0.5f;
							score += 50 + int(float(100) * combo / 66);
						}
					}
				}
				
				int note_color = ((n.pos.x == 0.0f) || (n.pos.x == 3.0f)) ? 16 : 17;
				note_color = (n.note_mode == 128) ? note_color + 2 : note_color;
				float ty = ((float)n.time - (float)playtime) / 1000.0f;
				float tk = (n.note_mode == 128) ?  2.328f/2328.0f*float(n.time_L - n.time) : n.scale.y;
				float bar_offset = (n.note_mode == 128) ? tk-0.022f : 0.0f;
				float accepted_time = (n.time_L == 0) ? 1.0f : 20.0f;
				float note_height = n.pos.y + -0.6f + ty * 2.0f + bar_offset;

				
				if (abs(note_height) <= accepted_time && !n.hit)
				{
					glUniform1i(glGetUniformLocation(program, "TEX"), note_color);
					if (note_height < -0.8f && note_height > -1.0f) 
					{
						if (n.note_mode == 1) 
						{
							combo = 0; miss++;
						}
					}
					uloc = glGetUniformLocation(program, "color"); if (uloc > -1) glUniform4fv(uloc, 1, n.color);
					uloc = glGetUniformLocation(program, "center"); if (uloc > -1) glUniform2fv(uloc, 1, vec2(n.pos.x * 0.1f - 0.547f, note_height));
					uloc = glGetUniformLocation(program, "scale"); if (uloc > -1) glUniform2f(uloc, n.scale.x, tk);
					// render quad vertices
					glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
				}
				//hit effect
				if (n.hit && n.effect < 9 && n.note_mode == 1)
				{
					if (!n.combo)
					{
						n.combo = true;
						combo++;
						total_hit++;
						score += 100 + int(combo / 33);
					}
					glUniform1i(glGetUniformLocation(program, "TEX"), n.effect++);
					uloc = glGetUniformLocation(program, "color"); if (uloc > -1) glUniform4fv(uloc, 1, vec4(1,1,1,0.9f));
					uloc = glGetUniformLocation(program, "center"); if (uloc > -1) glUniform2fv(uloc, 1, vec2(n.pos.x*0.1f - 0.547f, -0.6f));
					uloc = glGetUniformLocation(program, "scale"); if (uloc > -1) glUniform2f(uloc, 0.2f, 0.2f);

					// render quad vertices
					glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
				}
			}
		}
	}
	//combo text rendering
	max_combo = max(max_combo, int(combo));
	
	if(int(combo) != 0)render_text(std::to_string(int(combo)), 0.28f*float(window_size.x), 0.6f*float(window_size.y), float(window_size.x)/1000.0f, vec4(0, 0.5f, 1, 1), 0);
	if (!playing) 
	{
		render_text("Press D F J K", 0.55f * float(window_size.x), 0.1f * float(window_size.y), float(window_size.y) / 1000.0f, vec4(1, 1, 1, 1), 3);
		render_text("at the right timing!", 0.55f * float(window_size.x), 0.15f * float(window_size.y), float(window_size.y) / 1000.0f, vec4(1, 1, 1, 1), 3);
		render_text("Long notes are bonus combo", 0.55f * float(window_size.x), 0.2f * float(window_size.y), float(window_size.y) / 1000.0f, vec4(1, 1, 1, 1), 3);
		render_text("Press 1~5 to select music!", 0.55f * float(window_size.x), 0.25f * float(window_size.y), float(window_size.y) / 1000.0f, vec4(1, 1, 1, 1), 3);
		render_text("Press T to start and pause music", 0.55f * float(window_size.x), 0.3f * float(window_size.y), float(window_size.y) / 1000.0f, vec4(1, 1, 1, 1), 3);
		render_text("1: DropZ Line 2", 0.55f * float(window_size.x), 0.4f * float(window_size.y), float(window_size.y) / 1000.0f, vec4(1, 1, 1, 1), 3);
		render_text("2: On the FM  3.5", 0.55f * float(window_size.x), 0.45f * float(window_size.y), float(window_size.y) / 1000.0f, vec4(1, 1, 1, 1), 3);
		render_text("3: Anima         4", 0.55f * float(window_size.x), 0.5f * float(window_size.y), float(window_size.y) / 1000.0f, vec4(1, 1, 1, 1), 3);
		render_text("4: Play Time   4", 0.55f * float(window_size.x), 0.55f * float(window_size.y), float(window_size.y) / 1000.0f, vec4(1, 1, 1, 1), 3);
		render_text("5: Myosotis    3", 0.55f * float(window_size.x), 0.60f * float(window_size.y), float(window_size.y) / 1000.0f, vec4(1, 1, 1, 1), 3);
	}

	render_text(std::string("Total Hit:"+std::to_string(int(total_hit))).c_str(), 0.6f * float(window_size.x), 0.75f * float(window_size.y), float(window_size.y) / 1000.0f, vec4(1, 1, 1, 1), 3);
	render_text(std::string("MAX COMBO:" + std::to_string(max_combo)).c_str(), 0.6f * float(window_size.x), 0.8f * float(window_size.y), float(window_size.y) / 1000.0f, vec4(1, 1, 1, 1), 3);
	render_text(std::string("SCORE:" + std::to_string(score)).c_str(), 0.6f * float(window_size.x), 0.95f * float(window_size.y), float(window_size.y) / 1000.0f, vec4(1, 1, 1, 1), 3);
	glUseProgram(program);
	//pause
	if (!sound_play && is_music_init)
	{
		printf("\rpaused time: %d", pause_time);
		glUniform1i(glGetUniformLocation(program, "TEX"), 50);
		uloc = glGetUniformLocation(program, "color"); if (uloc > -1) glUniform4fv(uloc, 1, pause_bg_p.color);
		uloc = glGetUniformLocation(program, "center"); if (uloc > -1) glUniform2fv(uloc, 1, pause_bg_p.pos);
		uloc = glGetUniformLocation(program, "scale"); if (uloc > -1) glUniform2f(uloc, pause_bg_p.scale.x, pause_bg_p.scale.y);

		// render quad vertices
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		for (int i = 0; i < 2; i++)
		{
			glUniform1i(glGetUniformLocation(program, "TEX"), 51 + i);
			uloc = glGetUniformLocation(program, "color"); if (uloc > -1) glUniform4fv(uloc, 1, pause_menu_p[i].color);
			uloc = glGetUniformLocation(program, "center"); if (uloc > -1) glUniform2fv(uloc, 1, pause_menu_p[i].pos);
			uloc = glGetUniformLocation(program, "scale"); if (uloc > -1) glUniform2f(uloc, pause_menu_p[i].scale.x, pause_menu_p[i].scale.y);

			// render quad vertices
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		}
	}

	// swap front and back buffers, and display to screen
	glfwSwapBuffers( window );
}

void reshape( GLFWwindow* window, int width, int height )
{
	// set current viewport in pixels (win_x, win_y, win_width, win_height)
	// viewport: the window area that are affected by rendering 
	window_size = ivec2(width,height);
	aspect_ratio = window_size.x / float(window_size.y);
	glViewport( 0, 0, width, height );
}

void print_help()
{
	printf( "[help]\n" );
	printf( "- press ESC or 'q' to terminate the program\n" );
	printf( "- press F1 or 'h' to see help\n" );
	printf( "- press 'w' to toggle wireframe\n" );
	printf("- press 'T' to start/pause music\n");
	printf("- press key pad 1~5 to select music\n");
	printf( "\n" );
}
void music_off()
{
	if (sound_play) player->stop();
	sound_play = false;
	playing = false;
	music_start = false;
	is_music_init = false;
	combo = 0;
	miss = 0;
	play_offset = 0;
	pause_time = 0;
}
void keyboard( GLFWwindow* window, int key, int scancode, int action, int mods )
{
	if(action==GLFW_PRESS)
	{
		if (key == GLFW_KEY_ESCAPE || key == GLFW_KEY_Q)	glfwSetWindowShouldClose(window, GL_TRUE);
		else if (key == GLFW_KEY_H || key == GLFW_KEY_F1)	print_help();
		else if (key == GLFW_KEY_W)
		{
			b_wireframe = !b_wireframe;
			glPolygonMode(GL_FRONT_AND_BACK, b_wireframe ? GL_LINE : GL_FILL);
			printf("> using %s mode\n", b_wireframe ? "wireframe" : "solid");
		}
		else if (key == GLFW_KEY_T)
		{
			sound_play = !sound_play;
			if(!sound_play)	pause_timer = std::chrono::system_clock::now();
		}
		else if (key == GLFW_KEY_KP_1) { which_music = 1; printf("Play music 1\n"); music_off(); }
		else if (key == GLFW_KEY_KP_2) { which_music = 2; printf("Play music 2\n"); music_off(); }
		else if (key == GLFW_KEY_KP_3) { which_music = 3; printf("Play music 3\n"); music_off(); }
		else if (key == GLFW_KEY_KP_4) { which_music = 4; printf("Play music 4\n"); music_off(); }
		else if (key == GLFW_KEY_KP_5) { which_music = 5; printf("Play music 5\n"); music_off(); }
		else if (key == GLFW_KEY_KP_ADD) { sync += 10; printf("\rsync: %d", sync); }
		else if (key == GLFW_KEY_KP_SUBTRACT) { sync -= 10; printf("\rsync: %d", sync);}
		if (key == GLFW_KEY_D) 
		{
			note.d = true;
			key_press_timer[0] = std::chrono::system_clock::now();
		}
		if (key == GLFW_KEY_F)
		{
			note.f = true;
			key_press_timer[1] = std::chrono::system_clock::now();
		}
		if (key == GLFW_KEY_J)
		{
			note.j = true;
			key_press_timer[2] = std::chrono::system_clock::now();
		}
		if (key == GLFW_KEY_K)
		{
			note.k = true;
			key_press_timer[3] = std::chrono::system_clock::now();
		}
		

	}
	if (action == GLFW_RELEASE)
	{
		if (key == GLFW_KEY_D)
		{
			note.d = false;
			key_release_timer[0] = std::chrono::system_clock::now();
		}
		if (key == GLFW_KEY_F) 
		{
			note.f = false;
			key_release_timer[1] = std::chrono::system_clock::now();
		}
		if (key == GLFW_KEY_J) 
		{
			note.j = false;
			key_release_timer[2] = std::chrono::system_clock::now();
		}
		if (key == GLFW_KEY_K)
		{
			note.k = false;
			key_release_timer[3] = std::chrono::system_clock::now();
		}
	}
	std::chrono::system_clock::time_point music_timer = std::chrono::system_clock::now();
	std::chrono::duration<double, std::milli> diff = music_timer - start_timer;
	unsigned int playtime = int(diff.count()) - pause_time;
	bool lock[4];
	for (int i = 0; i < 4; i++)   lock[i] = false;
	for (auto& n : hit_obj)
	{
		if (n.note_mode != 128 && (-1000 + playtime > n.time || n.time > playtime + 1000))    continue;
		for (int i = 0; i < 4; i++) key_pt[i] = int(std::chrono::duration<double, std::milli>(key_press_timer[i] - start_timer).count()) - pause_time;
		for (int i = 0; i < 4; i++) key_rt[i] = int(std::chrono::duration<double, std::milli>(key_press_timer[i] - key_release_timer[i]).count());

		int judge = key_pt[int(n.pos.x + 0.1f)] - n.time;
		if ( judge > -200 && judge < 200 && lock[int(n.pos.x + 0.1f)] == false)
		{
			if (n.note_mode != 128)
			{
				n.hit = true;
				lock[int(n.pos.x + 0.1f)] = true;
			}
		}
	}
}
void mouse( GLFWwindow* window, int button, int action, int mods )
{
	if(button==GLFW_MOUSE_BUTTON_LEFT)
	{
		dvec2 pos; glfwGetCursorPos(window,&pos.x,&pos.y);
		vec2 npos = vec2( float(pos.x)/float(window_size.x-1), float(pos.y)/float(window_size.y-1) );
		printf("pos: %lf %lf", npos.x, npos.y);
		if (!sound_play && is_music_init)
		{
			if (npos.x >= 0.3f && npos.x <= 0.7f)
			{
				if (npos.y >= 0.2f && npos.y <= 0.5f)
				{
					sound_play = true;
				}
				else if (npos.y <= 0.8f && npos.y >= 0.6f)
				{
					sound_play = false;
					playing = false;
					music_start = false;
					is_music_init = false ;
					combo = 0;
					miss = 0;
					play_offset = 0;
					pause_time = 0;
					player->stop();
				}
			}
		}
	}
}

void motion( GLFWwindow* window, double x, double y )
{
}
void music_init()
{
	player = engine->play2D(sound_src[which_music-1], false, true, true);
	player->setVolume(0.6f);
	is_music_init = true;
}
void stage_init()
{
	//stage texture loading-------------------------------------------------------------------------------------------------------------------------------------------
	std::string stage_left = "../bin/images/stage-left.png";
	std::string stage_right = "../bin/images/stage-right.png";
	int width, height, comp;
	unsigned char* pimage;
	unsigned char* pimage0 = stbi_load(stage_left.c_str(), &width, &height, &comp, 4);
	int stride0 = width * comp, stride1 = (stride0 + 4) & (~4);
	pimage = (unsigned char*)malloc(sizeof(unsigned char) * stride1 * height);
	for (int y = 0; y < height; y++) memcpy(pimage + (height - 1 - y) * stride1, pimage0 + y * stride0, stride0);
	stbi_image_free(pimage0);

	glGenTextures(2, stage);
	glActiveTexture(GL_TEXTURE0 + num_images + 4 + 1);
	glBindTexture(GL_TEXTURE_2D, stage[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pimage);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	free(pimage);

	pimage0 = stbi_load(stage_right.c_str(), &width, &height, &comp, 4);
	stride0 = width * comp; stride1 = (stride0 + 4) & (~4);
	pimage = (unsigned char*)malloc(sizeof(unsigned char) * stride1 * height);
	for (int y = 0; y < height; y++) memcpy(pimage + (height - 1 - y) * stride1, pimage0 + y * stride0, stride0);
	glActiveTexture(GL_TEXTURE0 + num_images + 4 + 2);
	glBindTexture(GL_TEXTURE_2D, stage[1]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pimage);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	free(pimage);
}
void stage_light_init()
{
	//------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// effect for press keys
	std::string stage_light = "../bin/images/stage-light.png";
	int width, height, comp;
	unsigned char* pimage;
	unsigned char* pimage0 = stbi_load(stage_light.c_str(), &width, &height, &comp, 4);
	int stride0 = width * comp, stride1 = (stride0 + 4) & (~4);
	pimage = (unsigned char*)malloc(sizeof(unsigned char) * stride1 * height);
	for (int y = 0; y < height; y++) memcpy(pimage + (height - 1 - y) * stride1, pimage0 + y * stride0, stride0);
	stbi_image_free(pimage0);

	glGenTextures(1, &sl);
	glActiveTexture(GL_TEXTURE0 + num_images + 4);
	glBindTexture(GL_TEXTURE_2D, sl);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pimage);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	free(pimage);
}
void hit_effect_init()
{
	std::string* image_path = new std::string[num_images];
	for (int i = 0; i < num_images; i++)
	{
		image_path[i] = "../bin/images/lightingL-" + std::to_string(i) + ".png";
	}

	int* width, * height, * comp;
	width = (int*)malloc(sizeof(int) * num_images);
	height = (int*)malloc(sizeof(int) * num_images);
	comp = (int*)malloc(sizeof(int) * num_images);
	unsigned char** pimage = (unsigned char**)malloc(sizeof(unsigned char**) * num_images);

	for (int i = 0; i < num_images; i++)
	{
		unsigned char* pimage0 = stbi_load(image_path[i].c_str(), &width[i], &height[i], &comp[i], 4);
		int stride0 = width[i] * comp[i], stride1 = (stride0 + 4) & (~4);
		pimage[i] = (unsigned char*)malloc(sizeof(unsigned char) * stride1 * height[i]);
		for (int y = 0; y < height[i]; y++) memcpy(pimage[i] + (height[i] - 1 - y) * stride1, pimage0 + y * stride0, stride0);  // vertical flip
		stbi_image_free(pimage0); // release the original image
	}

	// create effect textures
	glGenTextures(num_images, TEX);
	for (int i = 0; i < num_images; i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, TEX[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width[i], height[i], 0, GL_RGBA, GL_UNSIGNED_BYTE, pimage[i]);
		// configure texture parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}

	// release the new image
	for (int i = 0; i < num_images; i++)
	{
		free(pimage[i]);
	}
	free(pimage); free(width); free(height); free(comp);
}
void background_init()
{
	std::string* background_path = new std::string[num_musics];
	for (int i = 0; i < num_musics; i++)
	{
		background_path[i] = "../bin/images/background" + std::to_string(i) + ".jpg";
	}
	int* width, * height, * comp;
	width = (int*)malloc(sizeof(int) * num_musics);
	height = (int*)malloc(sizeof(int) * num_musics);
	comp = (int*)malloc(sizeof(int) * num_musics);
	unsigned char ** pimage = (unsigned char**)malloc(sizeof(unsigned char**) * num_musics);
	for (int i = 0; i < num_musics; i++)
	{
		unsigned char* pimage0 = stbi_load(background_path[i].c_str(), &width[i], &height[i], &comp[i], 3);
		int stride0 = width[i] * comp[i], stride1 = (stride0 + 4) & (~4);
		pimage[i] = (unsigned char*)malloc(sizeof(unsigned char) * stride1 * height[i]);
		for (int y = 0; y < height[i]; y++) memcpy(pimage[i] + (height[i] - 1 - y) * stride1, pimage0 + y * stride0, stride0);  // vertical flip
		stbi_image_free(pimage0); // release the original image
	}
	glGenTextures(num_musics, background);
	for (int i = 0; i < num_musics; i++)
	{
		//num images = 10 effects, 4 buttons, 2 stages, i background
		glActiveTexture(GL_TEXTURE0 + num_images + 4 + 3 + 2 + 2 + i);
		glBindTexture(GL_TEXTURE_2D, background[i]);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width[i], height[i], 0, GL_RGB, GL_UNSIGNED_BYTE, pimage[i]);
		// configure texture parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}
	for (int i = 0; i < num_musics; i++)
	{
		free(pimage[i]);
	}
	free(pimage); free(width); free(height); free(comp);
}
void pause_bg_init()
{
	//------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// effect for press keys
	std::string stage_light = "../bin/images/pause_background.jpg";
	int width, height, comp;
	unsigned char* pimage;
	unsigned char* pimage0 = stbi_load(stage_light.c_str(), &width, &height, &comp, 3);
	int stride0 = width * comp, stride1 = (stride0 + 4) & (~4);
	pimage = (unsigned char*)malloc(sizeof(unsigned char) * stride1 * height);
	for (int y = 0; y < height; y++) memcpy(pimage + (height - 1 - y) * stride1, pimage0 + y * stride0, stride0);
	stbi_image_free(pimage0);

	glGenTextures(1, &pbg);
	glActiveTexture(GL_TEXTURE0 + 50);
	glBindTexture(GL_TEXTURE_2D, pbg);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pimage);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	free(pimage);
}
void pause_menu_init()
{
	//stage texture loading-------------------------------------------------------------------------------------------------------------------------------------------
	std::string stage_left = "../bin/images/pause-continue.png";
	std::string stage_right = "../bin/images/pause-back.png";
	int width, height, comp;
	unsigned char* pimage;
	unsigned char* pimage0 = stbi_load(stage_left.c_str(), &width, &height, &comp, 4);
	int stride0 = width * comp, stride1 = (stride0 + 4) & (~4);
	pimage = (unsigned char*)malloc(sizeof(unsigned char) * stride1 * height);
	for (int y = 0; y < height; y++) memcpy(pimage + (height - 1 - y) * stride1, pimage0 + y * stride0, stride0);
	stbi_image_free(pimage0);

	glGenTextures(2, pause_menu);
	glActiveTexture(GL_TEXTURE0 + 51);
	glBindTexture(GL_TEXTURE_2D, pause_menu[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pimage);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	free(pimage);

	pimage0 = stbi_load(stage_right.c_str(), &width, &height, &comp, 4);
	stride0 = width * comp; stride1 = (stride0 + 4) & (~4);
	pimage = (unsigned char*)malloc(sizeof(unsigned char) * stride1 * height);
	for (int y = 0; y < height; y++) memcpy(pimage + (height - 1 - y) * stride1, pimage0 + y * stride0, stride0);
	stbi_image_free(pimage0);
	glActiveTexture(GL_TEXTURE0 + 52);
	glBindTexture(GL_TEXTURE_2D, pause_menu[1]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pimage);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	free(pimage);
}
void hit_button_init()
{
	std::string* image_path_button = new std::string[4];
	for (int i = 0; i < 4; i++)
	{
		image_path_button[i] = "../bin/images/button" + std::to_string(i) + ".png";
	}

	int* width_button, * height_button, * comp_button;
	width_button = (int*)malloc(sizeof(int) * 4);
	height_button = (int*)malloc(sizeof(int) * 4);
	comp_button = (int*)malloc(sizeof(int) * 4);

	unsigned char** pimage_button = (unsigned char**)malloc(sizeof(unsigned char**) * 4);

	for (int i = 0; i < 4; i++)
	{
		unsigned char* pimage0 = stbi_load(image_path_button[i].c_str(), &width_button[i], &height_button[i], &comp_button[i], 4);
		int stride0 = width_button[i] * comp_button[i], stride1 = (stride0 + 4) & (~4);
		pimage_button[i] = (unsigned char*)malloc(sizeof(unsigned char) * stride1 * height_button[i]);
		for (int y = 0; y < height_button[i]; y++) memcpy(pimage_button[i] + (height_button[i] - 1 - y) * stride1, pimage0 + y * stride0, stride0);  // vertical flip
		stbi_image_free(pimage0); // release the original image
	}

	// create a particle texture
	glGenTextures(4, key_button);
	for (int i = 0; i < 4; i++)
	{
		glActiveTexture(GL_TEXTURE0 + num_images + i);
		glBindTexture(GL_TEXTURE_2D, key_button[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_button[i], height_button[i], 0, GL_RGBA, GL_UNSIGNED_BYTE, pimage_button[i]);
		// configure texture parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}

}
void note_init()
{
	std::string* image_path_note = new std::string[2];
	for (int i = 0; i < 2; i++)
	{
		image_path_note[i] = "../bin/images/note" + std::to_string(i) + ".png";
	}

	int* width_note, * height_note, * comp_note;
	width_note = (int*)malloc(sizeof(int) * 2);
	height_note = (int*)malloc(sizeof(int) * 2);
	comp_note = (int*)malloc(sizeof(int) * 2);

	unsigned char** pimage_note = (unsigned char**)malloc(sizeof(unsigned char**) * 2);

	for (int i = 0; i < 2; i++)
	{
		unsigned char* pimage0 = stbi_load(image_path_note[i].c_str(), &width_note[i], &height_note[i], &comp_note[i], 4);
		int stride0 = width_note[i] * comp_note[i], stride1 = (stride0 + 4) & (~4);
		pimage_note[i] = (unsigned char*)malloc(sizeof(unsigned char) * stride1 * height_note[i]);
		for (int y = 0; y < height_note[i]; y++) memcpy(pimage_note[i] + (height_note[i] - 1 - y) * stride1, pimage0 + y * stride0, stride0);  // vertical flip
		stbi_image_free(pimage0); // release the original image
	}

	// create a particle texture
	glGenTextures(2, note_tex);
	for (int i = 0; i < 2; i++)
	{
		glActiveTexture(GL_TEXTURE0 + num_images + 4 + 3 + i);
		glBindTexture(GL_TEXTURE_2D, note_tex[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_note[i], height_note[i], 0, GL_RGBA, GL_UNSIGNED_BYTE, pimage_note[i]);
		// configure texture parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}
}
void noteL_init()
{
	std::string* image_path_note = new std::string[2];
	for (int i = 0; i < 2; i++)
	{
		image_path_note[i] = "../bin/images/noteL" + std::to_string(i) + ".png";
	}

	int* width_note, * height_note, * comp_note;
	width_note = (int*)malloc(sizeof(int) * 2);
	height_note = (int*)malloc(sizeof(int) * 2);
	comp_note = (int*)malloc(sizeof(int) * 2);

	unsigned char** pimage_note = (unsigned char**)malloc(sizeof(unsigned char**) * 2);

	for (int i = 0; i < 2; i++)
	{
		unsigned char* pimage0 = stbi_load(image_path_note[i].c_str(), &width_note[i], &height_note[i], &comp_note[i], 4);
		int stride0 = width_note[i] * comp_note[i], stride1 = (stride0 + 4) & (~4);
		pimage_note[i] = (unsigned char*)malloc(sizeof(unsigned char) * stride1 * height_note[i]);
		for (int y = 0; y < height_note[i]; y++) memcpy(pimage_note[i] + (height_note[i] - 1 - y) * stride1, pimage0 + y * stride0, stride0);  // vertical flip
		stbi_image_free(pimage0); // release the original image
	}

	// create a particle texture
	glGenTextures(2, noteL_tex);
	for (int i = 0; i < 2; i++)
	{
		glActiveTexture(GL_TEXTURE0 + num_images + 4 + 3 + 2 + i);
		glBindTexture(GL_TEXTURE_2D, noteL_tex[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_note[i], height_note[i], 0, GL_RGBA, GL_UNSIGNED_BYTE, pimage_note[i]);
		// configure texture parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}
}
bool user_init()
{
	//music engine start
	engine = irrklang::createIrrKlangDevice();
	if (!engine) return false;
	std::string* mp3_path = new std::string[num_musics];
	mp3_path[0] = "../bin/musics/dropzline.mp3";
	mp3_path[1] = "../bin/musics/C-Show - On the FM.mp3";
	mp3_path[2] = "../bin/musics/anima.mp3";
	mp3_path[3] = "../bin/musics/play time.mp3";
	mp3_path[4] = "../bin/musics/Myosotis.mp3";
	sound_src = new irrklang::ISoundSource*[num_musics];
	for (int i = 0; i < num_musics; i++)
	{
		sound_src[i] = engine->addSoundSourceFromFile(mp3_path[i].c_str());
	}
	text_initialize();
	// log hotkeys
	print_help();
	// init GL states
	glClearColor( 39/255.0f, 40/255.0f, 34/255.0f, 1.0f );	// set clear color
	glEnable( GL_CULL_FACE );								// turn on backface culling
	//glEnable( GL_DEPTH_TEST );								// turn on depth tests
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable( GL_TEXTURE_2D );
	
	static vertex vertices[] = { {vec3(-1,-1,0),vec3(0,0,1),vec2(0,0)}, {vec3(1,-1,0),vec3(0,0,1),vec2(1,0)}, {vec3(-1,1,0),vec3(0,0,1),vec2(0,1)}, {vec3(1,1,0),vec3(0,0,1),vec2(1,1)} }; // strip ordering [0, 1, 3, 2]

	// generation of vertex buffer: use vertices as it is
	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * 4, &vertices[0], GL_STATIC_DRAW);

	stage_init(); 
	stage_light_init();
	hit_effect_init();
	background_init();
	hit_button_init();
	note_init();
	noteL_init();
	pause_bg_init();
	pause_menu_init();

	//TEXTURE PRESET CONFIG=============================================================================================================================================================
	//stage left and right
	//left and right are wrong
	stage_lr[0].pos = vec2(-0.395f, 0.0f);
	stage_lr[1].pos = vec2(-0.4f, 0.0f);
	stage_lr[0].scale = vec2(0.2f, 1.0f);
	stage_lr[1].scale = vec2(0.2f, 1.0f);
	//stage lighting color
	stage_l.color = vec4(0, 0.5f, 1, 0.3f);
	//buttons
	button[0].pos = vec2(-0.548f, -0.6f);
	button[1].pos = vec2(-0.448f, -0.6f);
	button[2].pos = vec2(-0.348f, -0.6f);
	button[3].pos = vec2(-0.248f, -0.6f);
	for (int i = 0; i < 4; i++)
	{
		button[i].scale.x = 0.05f;
		button[i].scale.y = 0.16f;
	}
	//hit effect
	particles.resize(4);
	for (int i = 0; i < 4; i++)
	{
		particles[i].pos.x = button[i].pos.x;
		particles[i].pos.y = button[i].pos.y + 0.05f;
		particles[i].color = vec4(1, 1, 1, 0.75f);
	}
	//note x axis 
	note_p.resize(4);
	for (int i = 0; i < 4; i++)
	{
		note_p[i].pos.x = button[i].pos.x;
		note_p[i].pos.y = 1.0f;
		note_p[i].scale.x = 0.0495f;
		note_p[i].scale.y = 0.02f;
	}
	//background texture
	background_p.color = vec4(0.22f, 0.22f, 0.22f, 1.0f); //make image darker
	background_p.scale = vec2(1.0f, 1.0f); // full program window size

	pause_bg_p.color = vec4(0.5f, 0.5f, 0.5f, 1.0f); //make image darker
	pause_bg_p.scale = vec2(1.0f, 1.0f); // full program window size

	pause_menu_p[0].pos = vec2(0, 0.3f);
	pause_menu_p[0].scale = vec2(0.3f, 0.1f);
	pause_menu_p[1].pos = vec2(0, -0.3f);
	pause_menu_p[1].scale = vec2(0.3f, 0.1f);

	return true;
}

void user_finalize()
{
	engine->drop();
}

int main( int argc, char* argv[] )
{
	// initialization
	if(!glfwInit()){ printf( "[error] failed in glfwInit()\n" ); return 1; }

	// create window and initialize OpenGL extensions
	if(!(window = cg_create_window( window_name, window_size.x, window_size.y ))){ glfwTerminate(); return 1; }
	if(!cg_init_extensions( window )){ glfwTerminate(); return 1; }	// version and extensions

	// initializations and validations
	if(!(program=cg_create_program( vert_shader_path, frag_shader_path ))){ glfwTerminate(); return 1; }	// create and compile shaders/program
	if(!user_init()){ printf( "Failed to user_init()\n" ); glfwTerminate(); return 1; }					// user initialization
	 
	// register event callbacks
	glfwSetWindowSizeCallback( window, reshape );	// callback for window resizing events
    glfwSetKeyCallback( window, keyboard );			// callback for keyboard events
	glfwSetMouseButtonCallback( window, mouse );	// callback for mouse click inputs
	glfwSetCursorPosCallback( window, motion );		// callback for mouse movement

	// enters rendering/event loop
	for( frame=0; !glfwWindowShouldClose(window); frame++ )
	{
		glfwPollEvents();	// polling and processing of events
		update();			// per-frame update
		render();			// per-frame render
	}
	
	// normal termination
	user_finalize();
	cg_destroy_window(window);

	return 0;
}

