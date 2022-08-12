#pragma once
#include "cgmath.h"			// slee's simple math library
#include "cgut.h"			// slee's OpenGL utility
#include "Hit_object.h"
#include "main.h"

using namespace std;

//Hit_object function implementation
Hit_object::Hit_object()
{
	this->x = -1;
	this->y = -1;
	this->time = -1;
	this->mode = -1;
	this->dummy = -1;
	this->long_note_time = -1;
}

Hit_object::~Hit_object()
{
	//dtor
}

// set all variable
void Hit_object::SetNote(int first, int second, int third, int fourth, int fifth, string sixth) {
	this->x = first;
	this->y = second;
	this->time = third;
	this->mode = fourth;
	this->dummy = fifth;
	this->long_note_time = sixth;
}

// set variable individually
void Hit_object::setX(int x_info) {
	this->x = x_info;
}

void Hit_object::setY(int y_info) {
	this->y = y_info;
}

void Hit_object::setTime(int time_info) {
	this->time = time_info;
}

void Hit_object::setMode(int mode_info) {
	this->mode = mode_info;
}

void Hit_object::setDummy(int dummy_info) {
	this->dummy = dummy_info;
}

void Hit_object::setLongNoteTime(string long_note_time_info) {
	this->long_note_time = long_note_time_info;
}

// get variable individually
int Hit_object::getX() {
	return this->x;
}

int Hit_object::getY() {
	return this->y;
}

int Hit_object::getTime() {
	return this->time;
}

int Hit_object::getMode() {
	return this->mode;
}

int Hit_object::getDummy() {
	return this->dummy;
}

string Hit_object::getLongNoteTime() {
	return this->long_note_time;
}

void Hit_object::showObject() {
	cout << x << '/' << y << '/' << time << '/' << mode << '/' << dummy << '/' << long_note_time << endl;
}
//========================================================================================================================

std::vector<time_obj> parse_time(std::string name)
{
	std::vector<time_obj> timing;
	string in_line;
	char str[100];
	ifstream in("../bin/musics/"+name+".time");
	while (getline(in, in_line)) {
		if (in_line == "[TimingPoints]") break;
	}
	while (getline(in, in_line)) {
		strcpy(str, in_line.c_str());
		time_obj temp;
		char* tok = strtok(str, ",");
		temp.time = atoi(tok);
		tok = strtok(NULL, ",");
		temp.speed_scale = float(atof(tok));
		timing.push_back(temp);
	}
	in.close();
	return timing;
}
std::vector<particle_t> parse_hit_object(std::string name)
{
	Hit_object* note = new Hit_object[10000];
	string in_line;
	char str[100];
	ifstream in("../bin/musics/" + name + ".hit");
	while (getline(in, in_line)) {
		if (in_line == "[HitObjects]") break;
	}
	int index = 0;
	while (getline(in, in_line)) {
		strcpy(str, in_line.c_str());

		char* tok = strtok(str, ",");
		note[index].setX(atoi(tok));
		tok = strtok(NULL, ",");
		note[index].setY(atoi(tok));
		tok = strtok(NULL, ",");
		note[index].setTime(atoi(tok));
		tok = strtok(NULL, ",");
		note[index].setMode(atoi(tok));
		tok = strtok(NULL, ",");
		note[index].setDummy(atoi(tok));
		tok = strtok(NULL, ",");
		note[index].setLongNoteTime(tok);
		index++;
	}
	in.close();

	for (int i = 0; i < index; i++)
	{
		strcpy(str, note[i].getLongNoteTime().c_str());
		char* tok = strtok(str, ":");
		note[i].setLongNoteTime(tok);
	}
	vector<particle_t>	hit_map;
	hit_map.resize(index);
	int time_index = 0;
	vector<time_obj> timing = std::move(parse_time(name));
	int beat = int(timing[0].speed_scale) / 10;
	if (beat == 0) beat = 1;
	for (int i = 0; i < index; i++)
	{
		if (note[i].getX() == 64) hit_map[i].pos.x = 0.0f;
		else if (note[i].getX() == 192) hit_map[i].pos.x = 1.0f;
		else if (note[i].getX() == 320) hit_map[i].pos.x = 2.0f;
		else if (note[i].getX() == 448) hit_map[i].pos.x = 3.0f;
		hit_map[i].pos.y = 1.0f;
		if (note[i].getMode() == 128)
		{
			hit_map[i].time = note[i].getTime();
			hit_map[i].time_L = stoi(note[i].getLongNoteTime());
			hit_map[i].note_mode = 128;
			int division = beat;
			hit_map[i].hit_block = (hit_map[i].time_L - hit_map[i].time) / division + 1;
			hit_map[i].long_hit = new bool[hit_map[i].hit_block];
			for (int j = 0; j < hit_map[i].hit_block; j++) hit_map[i].long_hit[j] = false;
			hit_map[i].division = division;
		}
		else
		{
			hit_map[i].time = note[i].getTime();
			hit_map[i].note_mode = 1;
		}
		hit_map[i].scale.x = 0.0495f;
		hit_map[i].scale.y = 0.02f;
	}
	return hit_map;
}