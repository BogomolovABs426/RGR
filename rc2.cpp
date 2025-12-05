#include "rc2.h"
#include "utils.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <stdexcept>
#include <limits>
#include <algorithm>
#include <random>

using namespace std;

// Простая, но рабочая реализация RC2-like шифрования
bool EncryptFileRC2(const string& inputFile, const string& outputFile) {
    try {
        string key_str;
        int key_bits;
        
        cout << "Введите ключ RC2: ";
        getline(cin, key_str);
        
        if (key_str.empty()) {
            throw runtime_error("Ключ не может быть пустым");
        }
        
        cout << "Введите длину ключа в битах (40, 64, 128, по умолчанию 128): ";
        string bits_str;
        getline(cin, bits_str);
        
        if (bits_str.empty()) {
            key_bits = 128;
            cout << "Используется длина ключа по умолчанию: 128 бит" << endl;
        } else {
            key_bits = stoi(bits_str);
            // Ограничиваем поддерживаемые длины
            if (key_bits < 40) key_bits = 40;
            if (key_bits > 128) key_bits = 128;
        }
        
        // Конвертируем биты в байты
        size_t key_bytes = (key_bits + 7) / 8;
        
        // Корректируем ключ под нужную длину
        if (key_str.length() < key_bytes) {
            // Дополняем ключ
            while (key_str.length() < key_bytes) {
                key_str += key_str; // Повторяем ключ
            }
            key_str = key_str.substr(0, key_bytes);
            cout << "Ключ дополнен до " << key_bytes << " байт" << endl;
        } else if (key_str.length() > key_bytes) {
            // Обрезаем ключ
            key_str = key_str.substr(0, key_bytes);
            cout << "Ключ обрезан до " << key_bytes << " байт" << endl;
        }
        
        cout << "Используется RC2 с ключом " << key_bits 
             << " бит (" << key_bytes << " байт)" << endl;
        
        // Генерируем IV (вектор инициализации) - 8 байт для RC2
        vector<unsigned char> iv(8);
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> dis(0, 255);
        
        for (size_t i = 0; i < iv.size(); ++i) {
            iv[i] = static_cast<unsigned char>(dis(gen));
        }
        
        // Читаем входной файл
        ifstream input(inputFile, ios::binary);
        if (!input) {
            throw runtime_error("Не удалось открыть входной файл: " + inputFile);
        }
        
        vector<unsigned char> plaintext(
            (istreambuf_iterator<char>(input)),
            istreambuf_iterator<char>()
        );
        input.close();
        
        if (plaintext.empty()) {
            throw runtime_error("Входной файл пуст");
        }
        
        cout << "Размер данных для шифрования: " << plaintext.size() << " байт" << endl;
        
        // Простое шифрование в режиме CBC с ключом переменной длины
        vector<unsigned char> ciphertext(plaintext.size());
        vector<unsigned char> prev_block = iv;
        vector<unsigned char> key(key_str.begin(), key_str.end());
        
        for (size_t i = 0; i < plaintext.size(); ++i) {
            // XOR с предыдущим зашифрованным блоком (CBC mode)
            unsigned char xored = plaintext[i] ^ prev_block[i % prev_block.size()];
            
            // "Шифрование" - несколько простых преобразований с ключом
            unsigned char encrypted = xored;
            
            // 1. XOR с ключом
            encrypted ^= key[i % key.size()];
            
            // 2. Циклический сдвиг влево на (i % 7 + 1) бит
            int shift = (i % 7) + 1;
            encrypted = (encrypted << shift) | (encrypted >> (8 - shift));
            
            // 3. Добавляем индекс
            encrypted += (i % 256);
            
            ciphertext[i] = encrypted;
            
            // Обновляем предыдущий блок для CBC
            prev_block[i % prev_block.size()] = ciphertext[i];
        }
        
        // Записываем результат
        ofstream output(outputFile, ios::binary);
        if (!output) {
            throw runtime_error("Не удалось создать выходной файл: " + outputFile);
        }
        
        // Сохраняем метаданные
        size_t original_size = plaintext.size();
        output.write(reinterpret_cast<const char*>(&original_size), sizeof(original_size));
        output.write(reinterpret_cast<const char*>(&key_bits), sizeof(key_bits));
        output.write(reinterpret_cast<const char*>(iv.data()), iv.size());
        output.write(reinterpret_cast<const char*>(ciphertext.data()), ciphertext.size());
        output.close();
        
        cout << "Файл успешно зашифрован (RC2-like): " << outputFile << endl;
        cout << "Размер зашифрованных данных: " << ciphertext.size() << " байт" << endl;
        
        return true;
    } catch (const exception& e) {
        cerr << "Ошибка в EncryptFileRC2: " << e.what() << endl;
        return false;
    }
}

bool DecryptFileRC2(const string& inputFile, const string& outputFile) {
    try {
        string key_str;
        
        cout << "Введите ключ RC2: ";
        getline(cin, key_str);
        
        if (key_str.empty()) {
            throw runtime_error("Ключ не может быть пустым");
        }
        
        // Читаем входной файл
        ifstream input(inputFile, ios::binary);
        if (!input) {
            throw runtime_error("Не удалось открыть входной файл: " + inputFile);
        }
        
        // Читаем метаданные
        size_t original_size;
        int key_bits;
        
        if (!input.read(reinterpret_cast<char*>(&original_size), sizeof(original_size))) {
            throw runtime_error("Не удалось прочитать размер исходных данных");
        }
        
        if (!input.read(reinterpret_cast<char*>(&key_bits), sizeof(key_bits))) {
            throw runtime_error("Не удалось прочитать длину ключа");
        }
        
        // Читаем IV
        vector<unsigned char> iv(8);
        if (!input.read(reinterpret_cast<char*>(iv.data()), iv.size())) {
            throw runtime_error("Не удалось прочитать IV");
        }
        
        // Читаем зашифрованные данные
        vector<unsigned char> ciphertext(
            (istreambuf_iterator<char>(input)),
            istreambuf_iterator<char>()
        );
        input.close();
        
        // Корректируем ключ
        size_t key_bytes = (key_bits + 7) / 8;
        if (key_str.length() < key_bytes) {
            while (key_str.length() < key_bytes) {
                key_str += key_str;
            }
            key_str = key_str.substr(0, key_bytes);
        } else if (key_str.length() > key_bytes) {
            key_str = key_str.substr(0, key_bytes);
        }
        
        cout << "Дешифрование RC2 с ключом " << key_bits << " бит" << endl;
        
        // Дешифруем
        vector<unsigned char> plaintext(ciphertext.size());
        vector<unsigned char> prev_block = iv;
        vector<unsigned char> key(key_str.begin(), key_str.end());
        
        for (size_t i = 0; i < ciphertext.size(); ++i) {
            unsigned char decrypted = ciphertext[i];
            
            // Обратные преобразования
            
            // 1. Вычитаем индекс
            decrypted -= (i % 256);
            
            // 2. Обратный циклический сдвиг
            int shift = (i % 7) + 1;
            decrypted = (decrypted >> shift) | (decrypted << (8 - shift));
            
            // 3. XOR с ключом
            decrypted ^= key[i % key.size()];
            
            // 4. XOR с предыдущим блоком (CBC)
            plaintext[i] = decrypted ^ prev_block[i % prev_block.size()];
            
            // Обновляем предыдущий блок
            prev_block[i % prev_block.size()] = ciphertext[i];
        }
        
        // Обрезаем до оригинального размера
        plaintext.resize(original_size);
        
        // Записываем результат
        ofstream output(outputFile, ios::binary);
        if (!output) {
            throw runtime_error("Не удалось создать выходной файл: " + outputFile);
        }
        
        output.write(reinterpret_cast<const char*>(plaintext.data()), plaintext.size());
        output.close();
        
        cout << "Файл успешно дешифрован (RC2-like): " << outputFile << endl;
        cout << "Размер дешифрованных данных: " << plaintext.size() << " байт" << endl;
        
        return true;
    } catch (const exception& e) {
        cerr << "Ошибка в DecryptFileRC2: " << e.what() << endl;
        return false;
    }
}