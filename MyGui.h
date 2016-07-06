/*
 * MyGui.h
 *
 *  Created on: 2016年6月26日
 *      Author: user
 */

#ifndef MYGUI_H_
#define MYGUI_H_


struct border
{
	float high, low, left, right;

	void set_border(float a, float b, float x, float y)
	{
		high = a;
		low = b;
		left = x;
		right = y;
	}
};

bool border_check(border b, float x, float y);

class gui_object
{
public:
	border obj_border;

	unsigned int fbo, vao, vbo[2];
	unsigned int texture;


};

class back : public gui_object
{
public:
	char* filename;
	unsigned int texture;
};

class button : public gui_object
{
public:
	int type, state;
	char* filename[3];
	unsigned int texture[3];
};

class Gui
{
public:
	//window size
	int window_w, window_h;

	//bottom number
	int button_num;

	//ui range
	border ui_border;

	//render area border
	border render_border[2];

	//bottoms
	button *buttons;

	//background, only use to print at left
	back background;

	//ui state, the int state is the total state of program, default as 0
	int file, smooth_times, algorithm, render_model, state;

	Gui();
	~Gui();

	int check_button(int x, int y);
	void clear_button_state();
	void clear_button_state(int id);

};


#endif /* MYGUI_H_ */
