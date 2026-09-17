#include <iostream>
#include <string>
#include <cmath>
#include <vector>
#include <fstream>
#include <sstream>
#include <Windows.h>
#define PI 3.14159265358979323846
#define SCREEN_WIDTH 40
#define SCREEN_HEIGHT 30
#define MAX_ELEMENTS 1000
#define MAX_SPRITES 1000

char SCREEN_CHAR_BUFFER[SCREEN_HEIGHT][SCREEN_WIDTH];
struct RGBA {
	unsigned char r; unsigned char g; unsigned char b; unsigned char a;
};
RGBA SCREEN_COLOR_BUFFER[SCREEN_HEIGHT][SCREEN_WIDTH];
struct Pixel { char c; RGBA color; };
template<typename T>
struct Dot2D { T x; T y; };
int SPRITE_COUNT = 0;
struct Sprite {
	std::string name = "default";
	unsigned int id = SPRITE_COUNT;
	Dot2D<int> position = { 0, 0 };
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
                if (pixels[y * width + x].color.a == 0) {
                    continue; // Skip transparent pixels
				}
                Pixel& pixel = pixels[y * width + x];
                SCREEN_CHAR_BUFFER[screenY][screenX] = pixel.c;
                SCREEN_COLOR_BUFFER[screenY][screenX] = pixel.color;
            }
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
        }
    }
}
void renderScreen() {
    for (int y = 0; y < SCREEN_HEIGHT; ++y) {
        for (int x = 0; x < SCREEN_WIDTH; ++x) {
            std::cout << SCREEN_CHAR_BUFFER[y][x];
        }
        std::cout << std::endl;
    }
}

int main() {
	Sprite dog;
	dog.name = "Dog";
	loadPicture("dog.pix", dog);
	dog.blitToScreen();
	renderScreen();
	std::cout << dog.id << std::endl;
}