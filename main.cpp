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

void drawChatbox(int x, int y, int width, int height, const std::string& text, RGBA boxColor = { 0, 0, 0, 255 }, RGBA textColor = { 255, 255, 255, 255 }, char boxChar = '#') {
	drawBox(x, y, width, height, boxColor, boxChar);
	drawText(x + 1, y + 2, text, textColor);
}

// ---- 演示用的精灵工具：创建一个纯色块精灵 ----
int createBlockSprite(const std::string& name, int w, int h, char c, RGBA color) {
	int id = addSprite(name);
	delete[] sprites[id].pixels;                 // 释放默认 16x16
	sprites[id].width = w;
	sprites[id].height = h;
	sprites[id].pixels = new Pixel[w * h];
	for (int i = 0; i < w * h; ++i) sprites[id].pixels[i] = { c, color };
	sprites[id].collisionBox = { w, h };         // 碰撞盒与尺寸一致
	return id;
}
//=============endline===============

// 教程页面内容
std::string scripts[] = {
	"Hello, Luna Engine!\n\n --A Project by Cedar Lumen.\n\n   (press SPACE to continue...)",
	"Tutorials:\n\n 1. Sprites   - images / text / shapes\n 2. Keyboard  - isKeyDown / isKeyPressed\n 3. Collision - collisionCheck()\n 4. Rendering - clearScreen + renderScreen\n\n   (press SPACE...)",
	"Sprite:\n\n addSprite(\"name\") creates a sprite.\n setPixel(x, y, c, color) draws pixels.\n blitSprite(id) draws it to screen.\n\n   (press SPACE...)",
	"Text & Shapes:\n\n setText() makes a text sprite.\n setLine() draws a Bresenham line.\n drawBox() = 4 lines = a rectangle!\n\n   (press SPACE...)",
	"Keyboard:\n\n isKeyDown(key)   - held this frame\n isKeyPressed(key)- just pressed\n VK_SPACE / 'W' / VK_ESCAPE ...\n\n   (press SPACE...)",
	"Collision:\n\n collisionCheck(id1, id2)\n uses position + collisionBox.\n Try pushing the wall in the demo!\n\n   (press SPACE for DEMO...)"
};
const int SCRIPT_COUNT = sizeof(scripts) / sizeof(scripts[0]);

int main() {
	initConsole();

	int scriptIndex = 0;
	bool inDemo = false;

	// ---- 演示场景：玩家(青色) 与 墙(红色) ----
	int idPlayer = createBlockSprite("player", 4, 2, '#', { 0, 255, 255, 255 });
	int idWall = createBlockSprite("wall", 10, 6, '#', { 255, 60, 60, 255 });
	sprites[idPlayer].position = { 5, 40 };
	sprites[idWall].position = { 40, 30 };

	while (true) {
		clearScreen();

		if (!inDemo) {
			// 教程阅读模式：空格翻页
			drawChatbox(0, 0, 60, 22, scripts[scriptIndex]);
			drawText(0, 23, "Page " + std::to_string(scriptIndex + 1) + " / " + std::to_string(SCRIPT_COUNT), { 180, 180, 180, 255 });

			if (isKeyPressed(VK_SPACE)) {
				scriptIndex++;
				if (scriptIndex >= SCRIPT_COUNT) {
					scriptIndex = 0;
					inDemo = true;   // 最后一页之后进入演示
				}
			}
		}
		else {
			// 演示模式：WASD 移动，ESC 返回教程
			if (isKeyDown('W')) sprites[idPlayer].position.y--;
			if (isKeyDown('S')) sprites[idPlayer].position.y++;
			if (isKeyDown('A')) sprites[idPlayer].position.x--;
			if (isKeyDown('D')) sprites[idPlayer].position.x++;

			// 屏幕边界
			if (sprites[idPlayer].position.x < 0) sprites[idPlayer].position.x = 0;
			if (sprites[idPlayer].position.y < 0) sprites[idPlayer].position.y = 0;
			if (sprites[idPlayer].position.x + sprites[idPlayer].width > SCREEN_WIDTH)  sprites[idPlayer].position.x = SCREEN_WIDTH - sprites[idPlayer].width;
			if (sprites[idPlayer].position.y + sprites[idPlayer].height > SCREEN_HEIGHT) sprites[idPlayer].position.y = SCREEN_HEIGHT - sprites[idPlayer].height;

			// 撞墙检测：先试探移动，撞上就回退
			if (collisionCheck(idPlayer, idWall)) {
				if (isKeyDown('W')) sprites[idPlayer].position.y++;
				if (isKeyDown('S')) sprites[idPlayer].position.y--;
				if (isKeyDown('A')) sprites[idPlayer].position.x++;
				if (isKeyDown('D')) sprites[idPlayer].position.x--;
			}

			blitSprite(idWall);
			blitSprite(idPlayer);

			drawChatbox(0, 0, 45, 8, "DEMO: WASD to move.\nPush the red wall!\nESC = back to tutorial.");
			if (isKeyPressed(VK_ESCAPE)) {
				inDemo = false;
				scriptIndex = 0;
				sprites[idPlayer].position = { 5, 40 };
			}
		}

		renderScreen();
		Sleep(33);  // ~30 FPS，按键轮询节奏
	}

	return 0;
}