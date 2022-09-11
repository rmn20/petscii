#include <cstring>

#include "undoredo.h"

UndoStep::~UndoStep() {
	delete[] chars;
	delete[] charsColors;
}

void UndoStep::resize(uint32_t w, uint32_t h) {
	if(this->w * this->h != w * h) {
		delete[] chars;
		delete[] charsColors;
		
		chars = new uint32_t[w * h];
		charsColors = new uint32_t[w * h * 2];
	}
	
	this->w = w;
	this->h = h;
}

void UndoStep::store(uint32_t bckColor, 
	uint32_t* chars, uint32_t* charsColors, 
	uint32_t w, uint32_t h,
	std::string charset, std::string palette
) {
	this->bckColor = bckColor;
	this->charset = charset;
	this->palette = palette;
	
	resize(w, h);
	
	memcpy(this->chars, chars, w * h * sizeof(uint32_t));
	memcpy(this->charsColors, charsColors, w * h * sizeof(uint32_t) * 2);
}
		
void UndoStorage::reset(uint32_t bckColor, 
	uint32_t* chars, uint32_t* charsColors, 
	uint32_t w, uint32_t h,
	std::string charset, std::string palette
) {
	undoCount = 0;
	redoCount = 0;
	index = 0;
	
	//Меняем размеры всех шагов отмены под текущий размер холста для ускорения работы с отменой
	for(size_t i=0; i<MAX_UNDO_STEPS; i++) {
		steps[i].resize(w, h);
	}
	
	steps[index].store(bckColor, chars, charsColors, w, h, charset, palette);
}

void UndoStorage::undo() {
	if(!canUndo()) return;
	
	//Идём назад
	index = (index + MAX_UNDO_STEPS - 1) % MAX_UNDO_STEPS;
	
	undoCount--;
	redoCount++;
}

void UndoStorage::redo() {
	if(!canRedo()) return;
	
	//Идём вперёд
	index = (index + 1) % MAX_UNDO_STEPS;
	
	undoCount++;
	redoCount--;
}
		
UndoStep* UndoStorage::getStep() {
	return &(steps[index]);
}
		
void UndoStorage::store(uint32_t bckColor, 
	uint32_t* chars, uint32_t* charsColors, 
	uint32_t w, uint32_t h,
	std::string charset, std::string palette
) {
	index = (index + 1) % MAX_UNDO_STEPS;
	
	//Число шагов ограничено
	undoCount = std::min(undoCount + 1, (size_t) (MAX_UNDO_STEPS - 1));
	redoCount = 0;
	
	steps[index].store(bckColor, chars, charsColors, w, h, charset, palette);
}