#include <iostream>
#include <cstring>
#include <cassert>

#include "main.h"

using namespace std;

int main()
{
	Main& main = Main::get();
	shared_ptr<Editor> editor = make_shared<Editor>();
	
	main.setScreen(editor);
	main.tick();
	
    return 0;
}

Main::Main() {
	//Инициализация окна
	InitWindow(800, 600, "petscii editor");
	SetWindowState(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_MAXIMIZED);
	
	SetExitKey(0);
	SetTargetFPS(60);
}

Main::~Main() {
	scr = nullptr;
	nextScr = nullptr;
	
	CloseWindow();
}

shared_ptr<Screen> Main::getScreen() {
	return scr;
}

void Main::setScreen(shared_ptr<Screen> scr) {
	nextScr = scr;
}

void Main::stop() {
	run = false;
}

void Main::tick() {
	while(run) {
		//Change screen to next screen
		if(nextScr != nullptr) {
			scr = nullptr;

			scr = nextScr;
			nextScr = nullptr;
			scr->show();
		}

		if(scr != nullptr) {
			scr->tick();
		}
		
		if(WindowShouldClose()) {
			stop();
		}
	}
}