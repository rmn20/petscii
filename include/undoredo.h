#ifndef UNDOREDO_H
#define UNDOREDO_H

#include <string>

#define MAX_UNDO_STEPS 100

//Класс шага отмены
class UndoStep {
	public:
		//Данные холста
		uint32_t bckColor;
		
		uint32_t* chars = nullptr;
		uint32_t* charsColors = nullptr;
		
		uint32_t w = 0, h = 0;
		
		std::string charset, palette;
		
		//Функции
		UndoStep() {}
		UndoStep(const UndoStep &undoStep) = delete;
		void operator=(const UndoStep &undoStep) = delete;
		~UndoStep();
		
		void resize(uint32_t w, uint32_t h);
		
		//Сохранить данные в шаг
		void store(uint32_t bckColor, 
			uint32_t* chars, uint32_t* charsColors, 
			uint32_t w, uint32_t h,
			std::string charset, std::string palette
		);
};

//Класс хранилища шагов отмены
class UndoStorage {
	private:
		UndoStep steps[MAX_UNDO_STEPS];
		
		//Положение в списке, кол-во шагов отмены и повтора
		size_t index = 0;
		size_t undoCount = 0;
		size_t redoCount = 0;
	
	public:
		UndoStorage() {}
		UndoStorage(const UndoStorage &undoStorage) = delete;
		void operator=(const UndoStorage &undoStorage) = delete;
		
		bool canUndo() {return undoCount > 0;}
		bool canRedo() {return redoCount > 0;}
		
		//Сбросить хранилище и сохранить текущее состояние
		void reset(uint32_t bckColor, 
			uint32_t* chars, uint32_t* charsColors, 
			uint32_t w, uint32_t h,
			std::string charset, std::string palette);
		
		void undo();
		void redo();
		
		UndoStep* getStep(); //Получить состояние текущего шага
		//Сохранить состояние
		void store(uint32_t bckColor, 
			uint32_t* chars, uint32_t* charsColors, 
			uint32_t w, uint32_t h,
			std::string charset, std::string palette);
};

#endif