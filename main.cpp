#pragma once
#include <random>
#include <string>
#include <array>
#include <stdexcept>
#define SCREEN_WIDTH 85
#define SCREEN_HEIGHT 50
#define MAX_ELEMENTS 1000
#define MAX_SPRITES 1000
#include "luna_engine.cpp"

//======insert your code here========
int idboxsys = addSprite("boxsys");
void drawBox(int x, int y, int width, int height, RGBA color = { 255, 255, 255, 255 }, char c = '#') {
	sprites[idboxsys].setLine(x, y, x + width - 1, y, color, c); // Top
	blitSprite(idboxsys);
	sprites[idboxsys].setLine(x, y + height - 1, x + width - 1, y + height - 1, color, c); // Bottom
	blitSprite(idboxsys);
	sprites[idboxsys].setLine(x, y, x, y + height - 1, color, c); // Left
	blitSprite(idboxsys);
	sprites[idboxsys].setLine(x + width - 1, y, x + width - 1, y + height - 1, color, c); // Right
	blitSprite(idboxsys);
}
int idtextsys = addSprite("textsys");
void drawText(int x, int y, const std::string& text, RGBA color = { 255, 255, 255, 255 }) {
	sprites[idtextsys].position = { x, y };
	sprites[idtextsys].setText(text, color);
	blitSprite(idtextsys);
}
//=============endline===============

int main() {
    initConsole();
	


    while (true) {
        clearScreen();
		drawBox(5, 3, 20, 10, { 0, 255, 0, 255 }, '*');
		drawText(10, 5, "Hello, Luna Engine!\n\n"
			" ---A Project by CedarLumen", { 255, 255, 0, 255 });
		renderScreen();
    }
    
    return 0;

}