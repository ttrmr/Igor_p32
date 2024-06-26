#pragma region Бібліотеки
#include <iostream>
#include <string>
#include <Windows.h> 
#include <vector>
#include <algorithm>
#include <ctime>
#include <cmath>
#include <queue>
#include <chrono>
#include <fstream>

#define I int 
#define E endl
#define C const
using namespace std;
#pragma endregion

#pragma region Змінні до секундомеру
chrono::steady_clock::time_point startPlayer;
chrono::steady_clock::time_point endPlayer;
#pragma endregion

#pragma region Розміри поля та кількість мін в залежності від рівня складності

C I EASY_WIDTH = 10;
C I EASY_HEIGHT = 8;
C I EASY_BOMBS = 10;

C I MEDIUM_WIDTH = 18;
C I MEDIUM_HEIGHT = 13;
C I MEDIUM_BOMBS = 40;

C I HARD_WIDTH = 24;
C I HARD_HEIGHT = 20;
C I HARD_BOMBS = 99;
#pragma endregion

#pragma region Фарбування чисел
void SetColor(I value) {
    switch (value) {
    case 1:
        cout << "\033[34m";
        break;
    case 2:
        cout << "\033[32m";
        break;
    case 3:
        cout << "\033[33m";
        break;
    case 4:
        cout << "\033[35m";
        break;
    case 5:
        cout << "\033[36m";
        break;
    case 6:
    case 7:
    case 8:
        cout << "\033[31m";
        break;
    default:
        cout << "\033[0m";
        break;
    }
}
#pragma endregion

#pragma region Перевірка на перемогу
bool IsWin(I** revealed, I** field, I width, I height, I bombs) {
    I revealedCount = 0;
    for (I i = 0; i < height; i++) {
        for (I j = 0; j < width; j++) {
            if (revealed[i][j] == 1 && field[i][j] != 9) {
                revealedCount++;
            }
        }
    }
    return revealedCount == (width * height - bombs);
}
#pragma endregion

#pragma region Відкриття сусідніх нульових клітин якщо користувач відкриває клітину з нулем
void RevealZero(I** field, I** revealed, I width, I height, I x, I y) {
    queue<pair<int, int>> toReveal;
    toReveal.push({ x, y });
    while (!toReveal.empty()) {
        I cx = toReveal.front().first;
        I cy = toReveal.front().second;
        toReveal.pop();
        for (I dx = -1; dx <= 1; dx++) {
            for (I dy = -1; dy <= 1; dy++) {
                I nx = cx + dx;
                I ny = cy + dy;
                if (nx >= 0 && nx < height && ny >= 0 && ny < width && revealed[nx][ny] == 0) {
                    revealed[nx][ny] = 1;
                    if (field[nx][ny] == 0) {
                        toReveal.push({ nx, ny });
                    }
                }
            }
        }
    }
}

#pragma endregion

#pragma region Функції стосовно поля гри
void PrintField(I** field, I** revealed, I** flagged, I width, I height) {
    cout << "\n\n\t";
    for (I j = 0; j < width; j++) {
        cout << " " << char('A' + j) << "   ";
    }
    cout << E;
    cout << "       +";
    for (I j = 0; j < width; j++) {
        cout << "----+";
    }
    cout << E;

    for (I i = 0; i < height; i++) {
        cout << (i < 10 ? "  " : " ") << i << "    |";
        for (I j = 0; j < width; j++) {
            if (flagged[i][j] == 1) {
                cout << "\033[31m F \033[0m |";
            }
            else if (revealed[i][j] == 1) {
                if (field[i][j] == 9) {
                    cout << " *  |";
                }
                else {
                    SetColor(field[i][j]);
                    cout << " " << field[i][j] << "  ";
                    cout << "\033[0m";
                    cout << "|";
                }
            }
            else {
                cout << "    |";
            }
        }
        cout << E;
        cout << "       +";
        for (I j = 0; j < width; j++) {
            cout << "----+";
        }
        cout << E;
    }
}

void InitializeField(I**& field, I**& revealed, I**& flagged, I width, I height) {
    field = new int* [height];
    revealed = new int* [height];
    flagged = new int* [height];
    for (I i = 0; i < height; i++) {
        field[i] = new int[width]();
        revealed[i] = new int[width]();
        flagged[i] = new int[width]();
    }
}

void DeleteField(I**& field, I**& revealed, I**& flagged, I height) {
    for (I i = 0; i < height; i++) {
        delete[] field[i];
        delete[] revealed[i];
        delete[] flagged[i];
    }
    delete[] field;
    delete[] revealed;
    delete[] flagged;
}

#pragma endregion

#pragma region Правила
void Rules() {
    cout << "\033[36mПравила гри в сапера : \033[0m\n";
    cout << "\033[33m1. Поле складається з клітин, деякі з яких містять міни.\n";
    cout << "2. Мета гри - відкрити всі клітини, що не містять мін.\n";
    cout << "3. Якщо ви відкриваєте комірку з міною, ви програєте.\n";
    cout << "4. Числа на відкритих клітинах вказують кількість мін у сусідніх клітинах.\n";
    cout << "5. Ви можете помітити комірки прапорцями, які, на вашу думку, містять міни.\n";
    cout << "6. Гра триває доти, доки ви не відкриєте всі комірки без мін або не натрапите на міну.\033[0m\n\n";
    cout << "Натисніть будь-яку клавішу, щоб повернутися до головного меню...";
    cin.ignore();
    cin.get();
    cin.clear();
}
#pragma endregion

#pragma region Перевірка вводу користувача
bool ValidUser(char col, I row, I width, I height) {
    return (col >= 'A' && col < 'A' + width) && (row >= 0 && row < height);
}
#pragma endregion

#pragma region Стосовно рекордів
void ShowRecords() {
    ifstream inFile("records.txt");
    if (!inFile.is_open()) {
        cout << "Нема збережених рекордів." << E;
        return;
    }
    struct Record {
        string difficulty;
        double time;
    };
    vector<Record> bestTimes; //вектор для рекордів
    string difficulty;
    double time;
    while (inFile >> difficulty >> time) {
        bool updated = false;
        for (auto& record : bestTimes) {
            if (record.difficulty == difficulty) {
                if (time < record.time) {
                    record.time = time;
                }
                updated = true;
                break;
            }
        }
        if (!updated) {
            bestTimes.push_back({ difficulty, time });
        }
    }
    inFile.close();
    if (bestTimes.empty()) {
        cout << "Нема збережених рекордів." << E;
        return;
    }
    cout << "Рекорди:" << E;
    cout << "\033[33mСкладність\tЧас (сек)\033[0m" << E;
    for (C auto& record : bestTimes) {
        cout << record.difficulty << "\t\t" << record.time << E;
    }
    cout << "Натисніть будь-яку клавішу, щоб повернутися до головного меню.";
    cin.ignore();
    cin.get();
    cin.clear();
}

void UpdRecords(I difficulty, double userTime) {
    vector<pair<string, double>> records;
    string difficultyStr;
    switch (difficulty) {
    case 1:
        difficultyStr = "Легкий ";
        break;
    case 2:
        difficultyStr = "Середній ";
        break;
    case 3:
        difficultyStr = "Тяжкий ";
        break;
    }
    ifstream inFile("records.txt");
    string diff;
    double time;
    while (inFile >> diff >> time) {
        records.push_back({ diff, time });
    }
    inFile.close();
    bool updated = false;
    for (auto& record : records) {
        if (record.first == difficultyStr) {
            if (userTime < record.second) {
                record.second = userTime;
                updated = true;
            }
        }
    }
    if (!updated) {
        records.push_back({ difficultyStr, userTime });
    }
    ofstream outFile("records.txt");
    for (C auto& record : records) {
        outFile << record.first << " " << record.second << E;
    }
    outFile.close();
}
#pragma endregion

#pragma region Код самої гри
void StartGame() {
    I width, height, bombs;
    I** field;
    I** revealed;
    I** flagged;
    I difficulty;
    cout << "Виберіть рівень складності: 1 - \033[32mЛегкий\033[0m, 2 - \033[33mСередній\033[0m, 3 - \033[31mТяжкий\033[0m:  ";
    cin >> difficulty;

    switch (difficulty) {
    case 1:
        width = EASY_WIDTH;
        height = EASY_HEIGHT;
        bombs = EASY_BOMBS;
        break;
    case 2:
        width = MEDIUM_WIDTH;
        height = MEDIUM_HEIGHT;
        bombs = MEDIUM_BOMBS;
        break;
    case 3:
        width = HARD_WIDTH;
        height = HARD_HEIGHT;
        bombs = HARD_BOMBS;
        break;
    default:
        cout << "Некоректний вибір гра буде запущена на легкому рівні." << E;
        width = EASY_WIDTH;
        height = EASY_HEIGHT;
        bombs = EASY_BOMBS;
        break;
    }

    InitializeField(field, revealed, flagged, width, height);
    PrintField(field, revealed, flagged, width, height);
    I bombsAround = 0;
    I playerX, playerY;
    char action;
    char col;
    cout << "\nВведіть координати першого хода (літера число): ";
    cin >> col >> playerY;
    col = toupper(col);
    playerX = col - 'A';
    while (!ValidUser(col, playerY, width, height) || cin.fail()) {
        cout << "Некоректні координати. Спробуйте ще раз: ";
        cin.clear();
        cin.ignore(10000, '\n');
        cin >> col >> playerY;
        playerX = col - 'A';
    }
    revealed[playerY][playerX] = 1;
    startPlayer = chrono::steady_clock::now();
    //розміщення бомб
    for (I i = 0; i < bombs; i++) {
        I bombX, bombY;
        do {
            bombX = rand() % height;
            bombY = rand() % width;
        } while ((bombX == playerY && bombY == playerX) || field[bombX][bombY] == 9); //перевірка щоб бомба не була на місці першого ходу гравця

        field[bombX][bombY] = 9;
    }
    //розміщення цифр на полі
    for (I i = 0; i < height; i++) {
        for (I j = 0; j < width; j++) {
            if (field[i][j] != 9) {
                for (I k = i - 1; k <= i + 1; k++) {
                    for (I l = j - 1; l <= j + 1; l++) {
                        if (k < 0 || k > height - 1 || l < 0 || l > width - 1) {
                            continue;
                        }
                        else {
                            if (field[k][l] == 9) {
                                bombsAround++;
                            }
                        }
                    }
                }
                field[i][j] = bombsAround;
                bombsAround = 0;
            }
        }
    }

    //відкриття області навколо першого ходу
    if (field[playerY][playerX] == 0) {
        RevealZero(field, revealed, width, height, playerY, playerX);
    }

    //ігровий цикл
    bool gameOver = false;
    while (!gameOver) {
        PrintField(field, revealed, flagged, width, height);

        cout << "Виберіть дію:" << E;
        cout << "a) Відкрити комірку" << E;
        cout << "b) Поставити прапорець" << E;

        char action;
        cin >> action;

        cout << "Введіть координати (літера цифра): ";
        char col;
        I playerY;
        cin >> col >> playerY;
        col = toupper(col);
        I playerX = col - 'A';

        if (action == 'b') {
            if (revealed[playerY][playerX] == 1) {
                cout << "Цю комірку вже відкрито. Прапор недоступний." << E;
                continue;
            }
            flagged[playerY][playerX] = !flagged[playerY][playerX];
        }
        else if (action == 'a') {
            if (revealed[playerY][playerX] == 1) {
                cout << "Цю комірку вже відкрито. Спробуйте знову." << E;
                continue;
            }

            if (field[playerY][playerX] == 9) {
                cout << "Ви потрапили на бомбу! Гра закінчена." << E;
                gameOver = true;
            }
            else {
                revealed[playerY][playerX] = 1;
                if (field[playerY][playerX] == 0) {
                    RevealZero(field, revealed, width, height, playerY, playerX);
                }
                if (IsWin(revealed, field, width, height, bombs)) {
                    endPlayer = chrono::steady_clock::now();
                    chrono::duration<double> elapsedPlayer = endPlayer - startPlayer;
                    double userTime = elapsedPlayer.count();

                    cout << "Вітаємо! Ви виграли!" << E;
                    Sleep(3000);
                    ofstream outFile("records.txt", ios::app);
                    if (outFile.is_open()) {
                        cout << "Складність: ";
                        switch (difficulty) {
                        case 1:
                            outFile << "Легкий ";
                            cout << "Легкий";
                            break;
                        case 2:
                            outFile << "Середній ";
                            cout << "Середній";
                            break;
                        case 3:
                            outFile << "Тяжкий ";
                            cout << "Тяжкий";
                            break;
                        }
                        outFile << userTime << E;
                        cout << ", Час: " << userTime << " секунд " << E;
                        outFile.close();
                    }

                    gameOver = true;
                }
            }
        }
    }
    //показати все поле наприкінці гри
    for (I i = 0; i < height; i++) {
        for (I j = 0; j < width; j++) {
            revealed[i][j] = 1;
            flagged[i][j] = 0;
        }
    }
    endPlayer = chrono::steady_clock::now();
    chrono::duration<double> elapsedPlayer = endPlayer - startPlayer;
    double userTime = elapsedPlayer.count();

    PrintField(field, revealed, flagged, width, height);
    DeleteField(field, revealed, flagged, height);
}

I main() {
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    srand(time(NULL));
    I choice;
    bool exitGame = false;
    cout << "\033[33mЛАСКАВО ПРОСИМО ДО ГРИ САПЕР!\033[0m\n\n" << E;
    while (!exitGame) {
        cout << "МЕНЮ:" << E;
        cout << "\033[36m1. Розпочати гру" << E;
        cout << "2. Переглянути правила" << E;
        cout << "3. Рекорди" << E;
        cout << "4. Вийти\033[0m" << E;
        cout << "\nОберіть дію:" << E;
        cin >> choice;
        cout << "\n";

        //перевірка на некоректне введення
        if (cin.fail()) {
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "Неправильний вибір. Будь ласка, введіть число від 1 до 4." << E;
            continue;
        }
        switch (choice) {
        case 1:
            StartGame();

            break;
        case 2:
            Rules();
            break;
        case 3:
            ShowRecords();
            break;
        case 4:
            exitGame = true;
            cout << "До побачення!" << E;
            break;
        default:
            cout << "Некоректний вибір." << E;
            break;
        }
    }

    return 0;
}
#pragma endregion