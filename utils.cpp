
#include "utils.h"
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <random>
#include <openssl/rand.h>

using namespace std;

// Генерация случайного числа
uint64_t GenerateRandom(uint64_t min, uint64_t max) {
    random_device rd;
    mt19937_64 gen(rd());
    uniform_int_distribution<uint64_t> dis(min, max);
    return dis(gen);
}

// Генерация ключа DES
bool GenerateDESKey() {
    try {
        // Генерируем случайный 8-байтовый ключ
        unsigned char key[8];
        if (RAND_bytes(key, sizeof(key)) != 1) {
            throw runtime_error("Не удалось сгенерировать случайный ключ");
        }
        
        ofstream keyFile("des_key.txt");
        if (!keyFile) {
            throw runtime_error("Не удалось создать файл des_key.txt");
        }

        // Записываем ключ в hex формате
        keyFile << "DES Key (hex): ";
        for (int i = 0; i < 8; ++i) {
            keyFile << hex << (int)key[i] << " ";
        }
        keyFile << endl;
        
        // Также записываем в виде строки (если возможно)
        keyFile << "DES Key (ASCII): ";
        for (int i = 0; i < 8; ++i) {
            if (key[i] >= 32 && key[i] <= 126) { // Печатные символы
                keyFile << key[i];
            } else {
                keyFile << "?";
            }
        }
        keyFile << endl;
        
        keyFile.close();
        
        cout << "Сгенерирован ключ DES и сохранен в des_key.txt." << endl;
        cout << "Ключ (hex): ";
        for (int i = 0; i < 8; ++i) {
            printf("%02X ", key[i]);
        }
        cout << endl;
        
        // Проверяем, можно ли представить как строку
        cout << "Ключ как строка: ";
        for (int i = 0; i < 8; ++i) {
            if (key[i] >= 32 && key[i] <= 126) {
                cout << key[i];
            } else {
                cout << ".";
            }
        }
        cout << endl;
        cout << "Примечание: некоторые байты могут быть непечатными символами" << endl;

        return true;
    } catch (const exception& e) {
        cerr << "Ошибка при генерации ключа DES: " << e.what() << endl;
        return false;
    }
}

// Генерация ключа RC2
bool GenerateRC2Key() {
    try {
        int key_length;
        
        cout << "Введите желаемую длину ключа в байтах (5-16, рекомендуется 16): ";
        cin >> key_length;
        
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        
        if (key_length < 5 || key_length > 16) {
            throw runtime_error("Длина ключа должна быть от 5 до 16 байт");
        }
        
        // Генерируем случайный ключ
        vector<unsigned char> key(key_length);
        if (RAND_bytes(key.data(), key_length) != 1) {
            throw runtime_error("Не удалось сгенерировать случайный ключ");
        }
        
        ofstream keyFile("rc2_key.txt");
        if (!keyFile) {
            throw runtime_error("Не удалось создать файл rc2_key.txt");
        }

        // Записываем ключ в hex формате
        keyFile << "RC2 Key (hex): ";
        for (int i = 0; i < key_length; ++i) {
            keyFile << hex << (int)key[i] << " ";
        }
        keyFile << endl;
        
        // Также записываем в виде строки
        keyFile << "RC2 Key (ASCII): ";
        for (int i = 0; i < key_length; ++i) {
            if (key[i] >= 32 && key[i] <= 126) {
                keyFile << key[i];
            } else {
                keyFile << "?";
            }
        }
        keyFile << endl;
        
        keyFile << "Key length: " << key_length << " bytes (" << (key_length * 8) << " bits)" << endl;
        keyFile.close();
        
        cout << "Сгенерирован ключ RC2 и сохранен в rc2_key.txt." << endl;
        cout << "Длина ключа: " << key_length << " байт (" << (key_length * 8) << " бит)" << endl;
        cout << "Ключ (hex): ";
        for (int i = 0; i < key_length; ++i) {
            printf("%02X ", key[i]);
        }
        cout << endl;
        
        cout << "Ключ как строка: ";
        for (int i = 0; i < key_length; ++i) {
            if (key[i] >= 32 && key[i] <= 126) {
                cout << key[i];
            } else {
                cout << ".";
            }
        }
        cout << endl;

        return true;
    } catch (const exception& e) {
        cerr << "Ошибка при генерации ключа RC2: " << e.what() << endl;
        return false;
    }
}
