#ifndef HIT_OBJECT_H
#define HIT_OBJECT_H

#include <fstream>
#include <iostream>
#include <cstring>
#include <string>
#include <cstdlib>

using namespace std;
typedef struct time_object
{
	unsigned int time;
	float speed_scale;
}time_obj;
class Hit_object
{
    public:
        Hit_object();
        virtual ~Hit_object();
        void SetNote(int first, int second, int third, int fourth, int fifth, std::string sixth);
        void setX(int x_info);
        void setY(int y_info);
        void setTime(int time_info);
        void setMode(int mode_info);
        void setDummy(int dummy_info);
        void setLongNoteTime(std::string long_note_time_info);
        int getX();
        int getY();
        int getTime();
        int getMode();
        int getDummy();
        std::string getLongNoteTime();
        void showObject();

    private:
        int x, y, time, mode, dummy;
        std::string long_note_time;
};

#endif // HIT_OBJECT_H
