#ifndef SCREEN_H
#define SCREEN_H

class Screen {
	protected:
	Screen() {}
	
	public:
	virtual ~Screen() {}
	
	virtual void show() {} //вызывается при первом появлении экрана
	virtual void tick() {} //вызывается покадрово
};

#endif