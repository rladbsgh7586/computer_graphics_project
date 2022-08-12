#pragma once

class particle_t
{
public:
	vec2 pos;
	vec4 color;
	vec2 velocity;
	vec2 scale;
	bool press;
	int effect;
	int division;
	int hit_block;
	bool hit;
	bool combo;
	bool* long_hit;
	unsigned int time;
	unsigned int time_L;
	int note_mode;
	int note_last;
	particle_t() { reset(); }

	void reset()
	{
		pos = vec2(0.0f, 0.0f);
		color = vec4(1, 1, 1, 1);
		scale = vec2(0.2f);
		velocity = vec2(0.0f, 0.0f);
		time = 0;
		time_L = 0;
		note_mode = 1;
		effect = 1;
		hit = false;
		combo = false;
	}
	void operator=(const particle_t& parti)
	{
		pos = parti.pos;
		color = parti.color;
		velocity = parti.velocity;
		scale = parti.scale;
		press = parti.press;
		effect = parti.effect;
		time = parti.time;
		time_L = parti.time;
		note_mode = parti.note_mode;
	}
};