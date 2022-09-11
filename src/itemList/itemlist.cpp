#include <iostream>

#include "itemList/itemlist.h"

//public

void ItemList::setSize(Vector2 size, int fontHeight) {
	this->size = size;
	this->fontHeight = fontHeight;
	
	updateElementsSize();
	limitScroll();
}

void ItemList::removeAll() {
	for(size_t i=0; i<items.size(); i++) {
		delete items[i];
	}
	
	items.clear();
	itemPressed = -1;
	elementsHeight = 0;
	topIndex = 0;
}

void ItemList::addVoid() {
	TextItem* item = new TextItem("");
	item->skip = true;
	add(item);
}

void ItemList::add(ListItem* item) {
	item->y = elementsHeight;
	item->updateListSize(size, fontHeight);
	elementsHeight += item->height;
	
	items.push_back(item);
	
	//Центрирование списка по необходимости
	if(vCenter && elementsHeight < size.y) yScroll = (size.y - elementsHeight) / 2;
}


void ItemList::render(Vector2 listPos) {
	//Чтобы элементы списка не рендерились вне области списка
	BeginScissorMode((int) listPos.x, (int) listPos.y, (int) size.x, (int) size.y);
	
	//Поиск самого верхнего элемента на экране (для ускорения рендера)
	ListItem* topItem = items[topIndex];
	
	int searchStep = 0;
	if(topItem->y+topItem->height+yScroll < 0) searchStep = 1;
	else if(topItem->y+yScroll > 0) searchStep = -1;
	
	if(searchStep != 0) {
		while(topIndex >= 0 && topIndex < items.size()) {
			ListItem* item = items[topIndex];
		
			if(yScroll + item->y <= 0 && 
					yScroll + item->y + item->height > 0) {
				break;
			}
			topIndex += searchStep;
		}
	}
	topIndex = std::max(0, std::min((int) items.size()-1, topIndex));
	
	//Рендер элементов начиная с самого верхнего
	for(int i=topIndex; i < items.size(); i++) {
		ListItem* item = items[i];
		
		if(listPos.y+item->y+yScroll >= listPos.y+size.y) break;
		
		item->render(listPos, size, yScroll, index == i);
	}

	EndScissorMode();
}


bool ItemList::isInBox(Vector2 listPos, Vector2 vec) {
	return CheckCollisionPointRec(vec, {listPos.x, listPos.y, size.x, size.y});
}
bool ItemList::isPointsElements(Vector2 listPos, Vector2 vec) {
	return CheckCollisionPointRec(vec, {listPos.x, listPos.y, size.x, elementsHeight});
}

void ItemList::mouseUpdate(Vector2 listPos, Vector2 mousePos) {
	float mouseWheel = GetMouseWheelMove();
	
	//Пропуск обновления мыши, если данные не изменились
	if(mousePos.x == prevMousePos.x && 
		mousePos.y == prevMousePos.y &&
		mouseWheel == 0 && 
		!IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && 
		!IsMouseButtonReleased(MOUSE_BUTTON_LEFT)
	) return;
	
	prevMousePos = mousePos;
	
	//Скролл
	if(mouseWheel != 0) {
		yScroll += mouseWheel * fontHeight;
		limitScroll();
	}
    
	//Заканчиваем выполнение, если курсор вне списка
	if(!isInBox(listPos, mousePos)) {
		index = -1;
		return;
	}
	
	//Выделяем элемент под курсором
	int listY = listPos.y + yScroll;
    int newId = -1;
    for(int i=topIndex; i<items.size(); i++) {
        ListItem* item = items[i];
		if(item->skip) continue;
		
		if(mousePos.y >= listY+item->y && mousePos.y < listY+item->y+item->height) {
			newId = i;
			break;
		} 
	}
	
	if(newId != index) {
		index = newId;
		if(index != -1) itemSelected();
	}
	
	//Заканчиваем выполнение, если никакой элемент не выделен
	if(index == -1) return;
	
	if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
		itemPressed = index;
	}
	
	if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && itemPressed == index) {
		items[index]->onClick({listPos.x, listPos.y + yScroll}, mousePos);
	}
}

void ItemList::keysUpdate() {
	if(IsKeyPressed(KEY_UP)) {
		scroll(-1);
		
	} else if(IsKeyPressed(KEY_DOWN)) {
		scroll(1);
		
	} else if(index != -1 && IsKeyPressed(KEY_ENTER)) {
		items[index]->onEnter();
	}
}

//private
void ItemList::updateElementsSize() {
	//Обновляем размер всех элементов в списке
	elementsHeight = 0;
	
	for(int i=0; i<items.size(); i++) {
		ListItem* item = items[i];
		
		item->y = elementsHeight;
		item->updateListSize(size, fontHeight);
		
		elementsHeight += item->height;
	}
	
	centralize();
}

void ItemList::itemSelected() {
	if(index != -1) items[index]->onSelected();
}

void ItemList::scroll(int dIndex) {
	if(index == -1) index = items.size()/2;
	else index += dIndex;
	
	if(index < 0) index = items.size() - 1;
	else index %= items.size();
	
	//Продолжаем скролл, если текущий элемент нельзя выделить
	if(items[index]->skip) {
		scroll(dIndex);
		return;
	}
	
	scrollToCurrentIndex();
	itemSelected();
}

void ItemList::scrollToCurrentIndex() {
	ListItem* item = items[index];
	yScroll = size.y/2 - item->y - item->height/2;
	limitScroll();
}

void ItemList::limitScroll() {
	if(elementsHeight > size.y) {
		//Начало текста в начале окна
		if(yScroll > 0) yScroll = 0;
		
		//Нижний край в конце окна
		if(yScroll + elementsHeight < size.y) yScroll = size.y - elementsHeight;
	} else {
		//Текст по середине
		centralize();
	}
}

void ItemList::centralize() {
	if(vCenter && elementsHeight < size.y) yScroll = (size.y - elementsHeight) / 2;
    else yScroll = 0;
}