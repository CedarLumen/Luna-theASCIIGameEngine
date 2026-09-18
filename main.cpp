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

std::string cardIdToFileName(int id) {
    static constexpr std::array<const char*, 13> kRanks = {
        "A", "2", "3", "4", "5", "6", "7", "8", "9", "X", "J", "Q", "K"
    };
    static constexpr std::array<const char*, 4> kSuits = {
        "club", "dia", "hrt", "spd"
    };
    if (id < 0 || id > 53) {
        throw std::out_of_range("card id must be in [0, 53]");
    }
    if (id == 52) return std::string("PokerCards\\") + "JokerS.pix";
    if (id == 53) return std::string("PokerCards\\") + "JokerB.pix";
    const int suit = id / 13;
    const int rank = id % 13;
    return std::string("PokerCards\\") + kRanks[rank] + kSuits[suit] + ".pix";
}

int selectedCard = 0;
int initCards[] = { 1, 2, 4, 6, 8, 9, 10, 11, 12, 0, 52, 53 };
int cardKeys[] = { 1, 2, 4, 6, 8, 9, 10, 11, 12, 0, 52, 53 };
int cardValues[] = { 2, 3, 5, 7, 9, 10, 11, 12, 13, 14, 15, 21 };

int cur_sum = 0;
int cur_score = 0;
int terminal_score = 20;
int cur_player = 1;
int player1Cards[4];
int player2Cards[4];

enum TurnState { SELECT_CARD, SELECT_OP };
TurnState turnState = SELECT_CARD;
int selectedOp = 0;   // 0: 加(+), 1: 乘(x)
int pauseFrames = 0;

int cardIdToValue(int id) {
    for (int i = 0; i < 12; ++i) {
        if (cardKeys[i] == id) return cardValues[i];
    }
    return 0;
}

void distributeCards() {
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(std::begin(initCards), std::end(initCards), g);
    for (int i = 0; i < 4; i++) {
        player1Cards[i] = initCards[i];
        player2Cards[i] = initCards[i + 4];
    }
}

void operateX(int X, int A) {
    if (X == 0) {
        cur_sum += A;
    }
    else if (X == 1) {
        cur_sum *= A;
    }
    cur_sum %= 10;
    cur_score += cur_sum * cur_player;
}

int testCardNotAllowed(int value) {
    if (cur_sum == 0) {
        return (value == 10) ? 1 : 0;
    }
    if (cur_sum == 1) {
        return 0;
    }
    return (value % cur_sum == 0) ? 1 : 0;
}

bool hasValidMove(int* cards) {
    for (int i = 0; i < 4; ++i) {
        if (cards[i] != 55) {
            int value = cardIdToValue(cards[i]);
            if (!testCardNotAllowed(value)) return true;
        }
    }
    return false;
}

//=============endline===============

int main() {
    initConsole();

    int pokerIDList[56];
    for (int i = 0; i < 54; i++) {
        pokerIDList[i] = addSprite("poker" + std::to_string(i));
        sprites[pokerIDList[i]].width = 2;
        sprites[pokerIDList[i]].height = 3;
        loadPicture(cardIdToFileName(i), sprites[pokerIDList[i]]);
    }
    pokerIDList[54] = addSprite("unshown");
    pokerIDList[55] = addSprite("empty");
    loadPicture("PokerCards\\unshown.pix", sprites[pokerIDList[54]]);
    loadPicture("PokerCards\\empty.pix", sprites[pokerIDList[55]]);
    sprites[pokerIDList[54]].width = 2;
    sprites[pokerIDList[54]].height = 3;
    sprites[pokerIDList[55]].width = 2;
    sprites[pokerIDList[55]].height = 3;
    distributeCards();

    int idSelected = addSprite("hud");
    sprites[idSelected].setText(
        " +===+\n"
        " |   |\n"
        " |   |\n"
        " |   |\n"
        " +=^=+\n",
        { 255, 255, 0, 255 }
    );

    int idScore = addSprite("score");
    sprites[idScore].position = { 25, 1 };

    bool gameOver = false;
    int winner = 0;

    while (true) {
        clearScreen();

        // ============ 渲染：选择框置于底层 ============
        if (cur_player == 1) {
            sprites[idSelected].position = { 1 + selectedCard * 3, 4 };
        }
        else {
            sprites[idSelected].position = { 1 + selectedCard * 3, 14 };
        }
        blitSprite(idSelected);   // 先画选择框

        // 再画手牌，覆盖在选择框之上
        for (int i = 0; i < 4; i++) {
            sprites[pokerIDList[player1Cards[i]]].position = { 2 + i * 3, 5 };
            blitSprite(pokerIDList[player1Cards[i]]);
        }
        for (int i = 0; i < 4; i++) {
            sprites[pokerIDList[player2Cards[i]]].position = { 2 + i * 3, 15 };
            blitSprite(pokerIDList[player2Cards[i]]);
        }

        // ============ 输入处理 ============
        bool canInput = false;
        if (pauseFrames > 0) {
            pauseFrames--;
        }
        else {
            canInput = !gameOver;
        }

        if (canInput) {
            int* currentCards = (cur_player == 1) ? player1Cards : player2Cards;

            if (turnState == SELECT_CARD) {
                if (!hasValidMove(currentCards)) {
                    // 无牌可出：对方得 10 分，洗牌
                    if (cur_player == 1) cur_score -= 10;
                    else                cur_score += 10;
                    distributeCards();
                    cur_player = -cur_player;
                    selectedCard = 0;
                    pauseFrames = 2;
                }
                else {
                    if (isKeyReleased(VK_LEFT)) {
                        for (int k = 0; k < 4; ++k) {
                            selectedCard = (selectedCard + 3) % 4;
                            if (currentCards[selectedCard] != 55) break;
                        }
                    }
                    if (isKeyReleased(VK_RIGHT)) {
                        for (int k = 0; k < 4; ++k) {
                            selectedCard = (selectedCard + 1) % 4;
                            if (currentCards[selectedCard] != 55) break;
                        }
                    }
                    if (isKeyReleased(VK_SPACE)) {
                        if (currentCards[selectedCard] != 55) {
                            int value = cardIdToValue(currentCards[selectedCard]);
                            if (!testCardNotAllowed(value)) {   // 合法才能进入选操作
                                turnState = SELECT_OP;
                                selectedOp = 0;
                            }
                            // 非法牌：直接忽略本次输入，停留原处
                        }
                    }
                    if (isKeyReleased(VK_ESCAPE)) {
                        break;
                    }
                }
            }
            else if (turnState == SELECT_OP) {
                if (isKeyReleased(VK_LEFT))  selectedOp = 0;
                if (isKeyReleased(VK_RIGHT)) selectedOp = 1;

                if (isKeyReleased(VK_ESCAPE)) {
                    turnState = SELECT_CARD;
                }

                if (isKeyReleased(VK_SPACE)) {
                    int cardId = currentCards[selectedCard];
                    if (cardId != 55) {
                        int value = cardIdToValue(cardId);
                        int playingPlayer = cur_player; // 记录本回合玩家

                        // 1. 出牌、结算
                        operateX(selectedOp, value);
                        currentCards[selectedCard] = 55;

                        // 2. 回合结束：切换玩家、重置选择
                        cur_player = -cur_player;
                        selectedCard = 0;
                        turnState = SELECT_CARD;

                        // 3. 回合结束后判定：检查刚出牌的玩家是否已出完所有牌
                        bool allEmpty = true;
                        for (int i = 0; i < 4; ++i) {
                            if (currentCards[i] != 55) { allEmpty = false; break; }
                        }

                        if (allEmpty) {
                            // 出完所有牌 → 洗牌
                            distributeCards();
                        }

                        // 4. 自动跳到当前行动玩家的第一张非空牌
                        int* newCards = (cur_player == 1) ? player1Cards : player2Cards;
                        for (int k = 0; k < 4; ++k) {
                            if (newCards[selectedCard] != 55) break;
                            selectedCard = (selectedCard + 1) % 4;
                        }

                        pauseFrames = 2;

                        // 5. 当 player2 出牌后才判定是否到线
                        if (playingPlayer == -1) {
                            if (cur_score >= terminal_score) {
                                gameOver = true; winner = 1;
                            }
                            else if (cur_score <= -terminal_score) {
                                gameOver = true; winner = 2;
                            }
                        }
                    }
                }
            }
        }

        // ============ 状态文字 ============
        std::string statusStr;
        if (gameOver) {
            if (winner == 1) statusStr = "Winner: Player 1";
            else if (winner == 2) statusStr = "Winner: Player 2";
            else statusStr = "Draw";
        }
        else if (turnState == SELECT_OP) {
            statusStr = "Select Op: " + std::string(selectedOp == 0 ? "[ADD +]" : "[MUL x]")
                + "  (Left/Right, Esc cancel)";
        }
        else {
            statusStr = "Current Player: "
                + std::string(cur_player == 1 ? "Player 1" : "Player 2");
        }

        sprites[idScore].setText(
            "Current Sum: " + std::to_string(cur_sum) + "\n"
            "Current Score: " + std::to_string(cur_score) + "\n"
            + statusStr + "\n"
            "Target Score: " + std::to_string(terminal_score) + "\n",
            { 0, 255, 255, 255 }
        );

        blitSprite(idScore);
        renderScreen();
        Sleep(16);
    }

    return 0;
}