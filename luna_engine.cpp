#pragma once
#include <iostream>
#include <string>
#include <cmath>
#include <vector>
#include <fstream>
#include <sstream>
#include <Windows.h>
#include "luna_engine_console.cpp"
#define PI 3.14159265358979323846
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 60
#define MAX_ELEMENTS 1000
#define MAX_SPRITES 1000


HANDLE hConsole;
void initConsole() {
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	// 开启 VT 转义序列支持（真彩色、光标控制必须）
	DWORD mode = 0;
	GetConsoleMode(hConsole, &mode);
	SetConsoleMode(hConsole, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);

	// 隐藏光标，消除光标跳动带来的闪烁感
	CONSOLE_CURSOR_INFO cursorInfo;
	GetConsoleCursorInfo(hConsole, &cursorInfo);
	cursorInfo.bVisible = FALSE;
	SetConsoleCursorInfo(hConsole, &cursorInfo);

	// 清一次屏 + 光标归位
	DWORD w = 0;
	const char* reset = "\033[2J\033[H";
	WriteConsoleA(hConsole, reset, (DWORD)strlen(reset), &w, NULL);
}
// ===== 键盘监听（非阻塞，不干扰控制台输入缓冲）=====
// 检测按键当前是否被按住
bool isKeyDown(int vKey) {
	return (GetAsyncKeyState(vKey) & 0x8000) != 0;
}

// 检测按键本帧是否"刚刚按下"（边沿触发，长按只触发一次）
bool isKeyPressed(int vKey) {
	static bool prevState[256] = { false };
	if (vKey < 0 || vKey > 255) return false;
	bool now = isKeyDown(vKey);
	bool pressed = now && !prevState[vKey];
	prevState[vKey] = now;
	return pressed;
}

// 图像精灵渲染时是否在像素后补一个空格（让图像看起来宽松，文本保持紧凑）
const bool IMAGE_SPACING = true;

char SCREEN_CHAR_BUFFER[SCREEN_HEIGHT][SCREEN_WIDTH];
struct RGBA {
	unsigned char r; unsigned char g; unsigned char b; unsigned char a;
};
RGBA SCREEN_COLOR_BUFFER[SCREEN_HEIGHT][SCREEN_WIDTH];
bool SCREEN_IS_TEXT[SCREEN_HEIGHT][SCREEN_WIDTH];
struct Pixel { char c; RGBA color; };
template<typename T>
struct Dot2D { T x; T y; };

int SPRITE_COUNT = 0;
struct Sprite {
	std::string name = "default";
	unsigned int id = SPRITE_COUNT;
	bool isVisible = 1;
	bool isText = 0;
	std::string text = "";
	RGBA textColor = { 255, 255, 255, 255 };
	Dot2D<int> position = { 0, 0 };
	Dot2D<int> collisionBox = { 16, 16 };
	Dot2D<int> pivot = { 0, 0 };
	Dot2D<int> collisionPivot = { 0, 0 };
	Dot2D<int> textPivot = { 0, 0 };
	float rotation = 0.0f;
	int width = 16;
	int height = 16;
	Pixel* pixels = new Pixel[width * height];
	Sprite() {
		SPRITE_COUNT++;
	}
	~Sprite() {
		delete[] pixels;
	}
	void setPixel(int x, int y, char c, RGBA color) {
		if (x < 0 || x >= width || y < 0 || y >= height) {
			std::cerr << "Error: Pixel coordinates out of bounds" << std::endl;
			return;
		}
		pixels[y * width + x] = { c, color };
	}
	void setPixel(int index, char c, RGBA color) {
		if (index < 0 || index >= width * height) {
			std::cerr << "Error: Pixel index out of bounds" << std::endl;
			return;
		}
		pixels[index] = { c, color };
	}
	void blitToScreen() {
		for (int y = 0; y < height; ++y) {
			for (int x = 0; x < width; ++x) {
				int screenX = position.x + x;
				int screenY = position.y + y;
				if (screenX < 0 || screenX >= SCREEN_WIDTH || screenY < 0 || screenY >= SCREEN_HEIGHT) {
					continue; // Skip pixels outside the screen bounds
				}
				Pixel& pixel = pixels[y * width + x];
				if (pixel.color.a == 0) {
					continue; // Skip transparent pixels
				}
				SCREEN_CHAR_BUFFER[screenY][screenX] = pixel.c;
				SCREEN_COLOR_BUFFER[screenY][screenX] = pixel.color;
				SCREEN_IS_TEXT[screenY][screenX] = 0; // 标记：图像来源
			}
		}
	}
	void setText(const std::string& t, RGBA color = { 255, 255, 255, 255 }) {
		isText = 1;
		text = t;
		textColor = color;
	}
	void blitTextToScreen() {
		int startX = position.x + textPivot.x;
		int cx = startX;
		int cy = position.y + textPivot.y;
		for (size_t i = 0; i < text.size(); ++i) {
			char ch = text[i];
			if (ch == '\n') {          // 换行
				cx = startX;
				cy++;
				continue;
			}
			if (ch == '\r') continue;  // 忽略 \r
			if (cx >= 0 && cx < SCREEN_WIDTH && cy >= 0 && cy < SCREEN_HEIGHT) {
				SCREEN_CHAR_BUFFER[cy][cx] = ch;
				SCREEN_COLOR_BUFFER[cy][cx] = textColor;
				SCREEN_IS_TEXT[cy][cx] = 1; // 标记：文本来源
			}
			cx++;
		}
	}
};



void loadPicture(const std::string& filename, Sprite& sprite) {
	std::ifstream file(filename);
	if (!file.is_open()) {
		std::cerr << "Error: Could not open file " << filename << std::endl;
		return;
	}
	std::string line;
	size_t index = 0;
	const size_t total = static_cast<size_t>(sprite.width) * sprite.height;
	while (std::getline(file, line)) {
		if (line.empty()) continue;
		if (line[0] == '#') continue;
		std::istringstream iss(line);
		char c;
		int r, g, b, a;
		if (!(iss >> c >> r >> g >> b >> a)) {
			std::cerr << "Error: Invalid line format in file " << filename << std::endl;
			continue;
		}
		if (r < 0 || r > 255 || g < 0 || g > 255 ||
			b < 0 || b > 255 || a < 0 || a > 255) {
			std::cerr << "Error: Color value out of range in file " << filename << std::endl;
			continue;
		}
		if (index >= total) {
			std::cerr << "Error: Too many pixels in file " << filename << std::endl;
			break;
		}
		sprite.pixels[index] = {
			c,
			{
				static_cast<unsigned char>(r),
				static_cast<unsigned char>(g),
				static_cast<unsigned char>(b),
				static_cast<unsigned char>(a)
			}
		};
		++index;
	}
	if (index < total) {
		std::cerr << "Warning: Not enough pixels in file " << filename
			<< ", loaded " << index << " / " << total << std::endl;
	}
}



void clearScreen() {
	for (int y = 0; y < SCREEN_HEIGHT; ++y) {
		for (int x = 0; x < SCREEN_WIDTH; ++x) {
			SCREEN_CHAR_BUFFER[y][x] = ' ';
			SCREEN_COLOR_BUFFER[y][x] = { 0, 0, 0, 0 };
			SCREEN_IS_TEXT[y][x] = 0;
		}
	}
}

void renderScreen() {
	static std::string out;
	out.clear();
	out.reserve(SCREEN_HEIGHT * SCREEN_WIDTH * 32 + 64);

	out += "\033[H"; // 光标回左上角，不清屏

	for (int y = 0; y < SCREEN_HEIGHT; ++y) {
		int lr = -1, lg = -1, lb = -1;
		for (int x = 0; x < SCREEN_WIDTH; ++x) {
			RGBA c = SCREEN_COLOR_BUFFER[y][x];
			char ch = SCREEN_CHAR_BUFFER[y][x];

			if (c.a == 0) {
				out += ' ';
				// 图像区域即使在透明格也补一个空格，保证整行对齐
				if (IMAGE_SPACING && !SCREEN_IS_TEXT[y][x]) out += ' ';
				continue;
			}
			if (c.r != lr || c.g != lg || c.b != lb) {
				out += "\033[38;2;";
				out += std::to_string(c.r); out += ';';
				out += std::to_string(c.g); out += ';';
				out += std::to_string(c.b); out += 'm';
				lr = c.r; lg = c.g; lb = c.b;
			}
			out += ch;
			// 图像精灵像素后补一个空格，文本精灵不补
			if (IMAGE_SPACING && !SCREEN_IS_TEXT[y][x]) out += ' ';
		}
		out += "\033[0m\033[K";                      // 重置颜色 + 擦到行尾
		if (y + 1 < SCREEN_HEIGHT) out += '\n';      // 最后一行不换行，防滚动
	}

	DWORD written = 0;
	WriteConsoleA(hConsole, out.data(), (DWORD)out.size(), &written, NULL);
}


Sprite sprites[MAX_SPRITES];
int spriteCount = 0;
int addSprite(std::string name) {
	if (spriteCount >= MAX_SPRITES) {
		std::cerr << "Error: Maximum number of sprites reached" << std::endl;
		return -1;
	}
	sprites[spriteCount].name = name;
	return spriteCount++;
}
int removeSprite(int id) {
	if (id < 0 || id >= spriteCount) {
		std::cerr << "Error: Invalid sprite ID" << std::endl;
		return -1;
	}
	for (int i = id; i < spriteCount - 1; ++i) {
		sprites[i] = sprites[i + 1];
	}
	spriteCount--;
	return 0;
}
int blitSprite(int id) {
	if (id < 0 || id >= spriteCount) {
		std::cerr << "Error: Invalid sprite ID: " << id << std::endl;
		return -1;
	}
	if (sprites[id].isVisible) {
		if (sprites[id].isText) {
			sprites[id].blitTextToScreen();
		}
		else {
			sprites[id].blitToScreen();
		}
	}
	return 0;
}
int collisionCheck(int id1, int id2) {
	if (id1 < 0 || id1 >= spriteCount || id2 < 0 || id2 >= spriteCount) {
		std::cerr << "Error: Invalid sprite ID(s)" << std::endl;
		return -1;
	}
	Sprite& s1 = sprites[id1];
	Sprite& s2 = sprites[id2];
	int left1 = s1.position.x + s1.collisionPivot.x;
	int right1 = left1 + s1.collisionBox.x;
	int top1 = s1.position.y + s1.collisionPivot.y;
	int bottom1 = top1 + s1.collisionBox.y;
	int left2 = s2.position.x + s2.collisionPivot.x;
	int right2 = left2 + s2.collisionBox.x;
	int top2 = s2.position.y + s2.collisionPivot.y;
	int bottom2 = top2 + s2.collisionBox.y;
	return !(left1 >= right2 || right1 <= left2 || top1 >= bottom2 || bottom1 <= top2);
}

int transportPicture(int desid, int srcid) {
	if (desid < 0 || desid >= SPRITE_COUNT || srcid < 0 || srcid >= SPRITE_COUNT) {
		std::cerr << "Error: Invalid sprite ID(s)" << std::endl;
		return -1;
	}
	Sprite& des = sprites[desid];
	Sprite& src = sprites[srcid];
	if (des.width != src.width || des.height != src.height) {
		std::cerr << "Error: Sprite dimensions do not match" << std::endl;
		return -1;
	}
	for (int y = 0; y < des.height; ++y) {
		for (int x = 0; x < des.width; ++x) {
			int index = y * des.width + x;
			des.pixels[index] = src.pixels[index];
		}
	}
	return 0;
}

