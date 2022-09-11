#ifndef LISTITEM_H
#define LISTITEM_H

#include <functional>

#include "raylib.h"

#include "itemList/itemlist.h"

class ListItem {
	public:
		//Функции
		std::function<void(ListItem* item)> onSelectedf = nullptr;
		std::function<void(ListItem* item)> onEnterf = nullptr;
		std::function<void(ListItem* item, Vector2, Vector2)> onClickf = nullptr;
		
		//Положение элемента в списке, высота элемента
		float y = 0, height = 0;
		//Пропускать ли элемент при скролле
		bool skip = false;
		
		ListItem() {}
		virtual ~ListItem() {}
		
		//Обновить размер элемента в соответствии с новым размером области списка и высотой шрифта
		virtual void updateListSize(Vector2 size, int fontHeight) {}
		
		virtual void render(Vector2 listPos, Vector2 size, float yScroll, bool selected) {}
		
		virtual void onSelected() {if(onSelectedf != nullptr) onSelectedf(this);}
		virtual void onEnter() {if(onEnterf != nullptr) onEnterf(this);}
		virtual void onClick(Vector2 listPos, Vector2 mousePos) {
			
			if(onClickf != nullptr) {
				onClickf(this,listPos, mousePos);
			} else onEnter();
		}
};

#endif