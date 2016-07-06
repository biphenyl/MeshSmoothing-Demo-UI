#include "MyGui.h"

Gui::Gui()
{
	button_num = 18;
	buttons = new button[button_num];

	background.filename = "bmp/UIbackground.bmp";
	background.obj_border.set_border(600, 0, 600, 800);

	//100*40
	buttons[0].filename[0] = "bmp/Bunny.bmp";
	buttons[0].filename[1] = "bmp/Bunny_1.bmp";
	buttons[1].filename[0] = "bmp/Body.bmp";
	buttons[1].filename[1] = "bmp/Body_1.bmp";
	buttons[2].filename[0] = "bmp/T-Rex.bmp";
	buttons[2].filename[1] = "bmp/T-Rex_1.bmp";

	//30*30
	buttons[3].filename[0] = "bmp/10.bmp";
	buttons[3].filename[1] = "bmp/10_1.bmp";
	buttons[4].filename[0] = "bmp/20.bmp";
	buttons[4].filename[1] = "bmp/20_1.bmp";
	buttons[5].filename[0] = "bmp/40.bmp";
	buttons[5].filename[1] = "bmp/40_1.bmp";
	buttons[6].filename[0] = "bmp/80.bmp";
	buttons[6].filename[1] = "bmp/80_1.bmp";
	buttons[7].filename[0] = "bmp/100.bmp";
	buttons[7].filename[1] = "bmp/100_1.bmp";
	buttons[8].filename[0] = "bmp/200.bmp";
	buttons[8].filename[1] = "bmp/200_1.bmp";
	buttons[9].filename[0] = "bmp/400.bmp";
	buttons[9].filename[1] = "bmp/400_1.bmp";
	buttons[10].filename[0] = "bmp/800.bmp";
	buttons[10].filename[1] = "bmp/800_1.bmp";

	//60*30
	buttons[11].filename[0] = "bmp/1000.bmp";
	buttons[11].filename[1] = "bmp/1000_1.bmp";
	buttons[12].filename[0] = "bmp/10000.bmp";
	buttons[12].filename[1] = "bmp/10000_1.bmp";

	//50*30
	buttons[13].filename[0] = "bmp/Simple.bmp";
	buttons[13].filename[1] = "bmp/Simple_1.bmp";
	buttons[14].filename[0] = "bmp/Normal.bmp";
	buttons[14].filename[1] = "bmp/Normal_1.bmp";
	buttons[15].filename[0] = "bmp/flat.bmp";
	buttons[15].filename[1] = "bmp/flat_1.bmp";

	//30*30
	buttons[16].filename[0] = "bmp/1.bmp";
	buttons[16].filename[1] = "bmp/1_1.bmp";
	buttons[17].filename[0] = "bmp/2.bmp";
	buttons[17].filename[1] = "bmp/2_1.bmp";

	buttons[0].obj_border.set_border(595, 555, 680, 780);
	buttons[1].obj_border.set_border(550, 510, 680, 780);
	buttons[2].obj_border.set_border(505, 465, 680, 780);

	buttons[3].obj_border.set_border(430, 400, 620, 650);
	buttons[4].obj_border.set_border(430, 400, 660, 690);
	buttons[5].obj_border.set_border(430, 400, 700, 730);
	buttons[6].obj_border.set_border(430, 400, 740, 770);

	buttons[7].obj_border.set_border(395, 365, 620, 650);
	buttons[8].obj_border.set_border(395, 365, 660, 690);
	buttons[9].obj_border.set_border(395, 365, 700, 730);
	buttons[10].obj_border.set_border(395, 365, 740, 770);

	buttons[11].obj_border.set_border(360, 330, 620, 680);
	buttons[12].obj_border.set_border(360, 330, 710, 770);

	buttons[13].obj_border.set_border(290, 260, 625, 675);
	buttons[14].obj_border.set_border(255, 225, 625, 675);
	buttons[15].obj_border.set_border(220, 190, 625, 675);

	buttons[16].obj_border.set_border(280, 250, 715, 745);
	buttons[17].obj_border.set_border(280, 250, 755, 785);

	for(int i=0; i<button_num; ++i)
	{
		buttons[i].state = 0;
	}

	//set initial algorithm as laplacian
	buttons[16].state = 1;
}

Gui::~Gui()
{
	delete buttons;
}

int Gui::check_button(int x, int y)
{
	border b;
	for(int i=0; i<button_num; ++i)
	{
		b = this->buttons[i].obj_border;
		if(border_check(b, x, y))
			return i;
	}

	return -1;
}

// clear all button's state
void Gui::clear_button_state()
{
	for(int i=0; i<button_num; ++i)
	{
		buttons[i].state = 0;
	}
}

// clear a group of button which contain the id
void Gui::clear_button_state(int id)
{
	if(id < 3)
	{
		for(int i=0; i<3; ++i)
			buttons[i].state = 0;
	}
	else if(id < 13)
	{
		for(int i=3; i<13; ++i)
			buttons[i].state = 0;
	}
	else if(id < 16)
	{
		for(int i=13; i<16; ++i)
			buttons[i].state = 0;
	}
	else
	{
		for(int i=16; i<20; ++i)
			buttons[i].state = 0;
	}

}

bool border_check(border b, float x, float y)
{
	if(x < b.right && x > b.left && y > b.low && y < b.high)
		return true;
	else
		return false;
}
