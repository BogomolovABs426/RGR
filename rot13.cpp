
#include "rot13.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <stdexcept>

using namespace std;

// Функция преобразования ROT13 для одного символа
unsigned char rot13_transform(unsigned char c) {
    if (c >= 'A' && c <= 'Z') {
        return ((c - 'A' + 13) % 26) + 'A';
    } else if (c >= 'a' && c <= 'z') {
        return ((c - 'a' + 13) % 26) + 'a';
    } else {
        return c;
    }
}

// Шифрование ROT13
bool EncryptFileROT13(const string& inputFile, const string& outputFile) {
    try {
        cout << "Шифрование ROT13" << endl;
        cout << "Примечание: ROT13 шифрует только латинские буквы, остальные символы остаются без изменений." << endl;

        // Читаем входной файл
        ifstream input(inputFile);
        if (!input) {
            throw runtime_error("Не удалось открыть входной файл: " + inputFile);
        }

        string content;
        char ch;
        while (input.get(ch)) {
            content += ch;
        }
        input.close();

        if (content.empty()) {
            cout << "Входной файл пуст" << endl;
        }

        // Применяем ROT13 ко всем символам
        string encrypted_content;
        for (unsigned char c : content) {
            encrypted_content += rot13_transform(c);
        }

        // Записываем результат
        ofstream output(outputFile);
        if (!output) {
            throw runtime_error("Не удалось создать выходной файл: " + outputFile);
        }

        output << encrypted_content;
        output.close();

        cout << "Файл успешно зашифрован ROT13: " << outputFile << endl;
        cout << "Пример преобразования (первые 100 символов):" << endl;
        
        // Показываем пример преобразования
        size_t sample_size = min(content.size(), size_t(100));
        cout << "Исходный текст: " << content.substr(0, sample_size) << endl;
        cout << "Зашифрованный:  " << encrypted_content.substr(0, sample_size) << endl;

        return true;
    } catch (const exception& e) {
        cerr << "Ошибка в EncryptFileROT13: " << e.what() << endl;
        return false;
    }
}

// Дешифрование ROT13 (аналогично шифрованию)
bool DecryptFileROT13(const string& inputFile, const string& outputFile) {
    try {
        cout << "Дешифрование ROT13" << endl;
        cout << "Примечание: Для ROT13 дешифрование идентично шифрованию." << endl;

        // Читаем входной файл
        ifstream input(inputFile);
        if (!input) {
            throw runtime_error("Не удалось открыть входной файл: " + inputFile);
        }

        string encrypted_content;
        char ch;
        while (input.get(ch)) {
            encrypted_content += ch;
        }
        input.close();

        if (encrypted_content.empty()) {
            cout << "Входной файл пуст" << endl;
        }

        // Применяем ROT13 для дешифрования (такая же операция)
        string decrypted_content;
        for (unsigned char c : encrypted_content) {
            decrypted_content += rot13_transform(c);
        }

        // Записываем результат
        ofstream output(outputFile);
        if (!output) {
            throw runtime_error("Не удалось создать выходной файл: " + outputFile);
        }

        output << decrypted_content;
        output.close();

        cout << "Файл успешно дешифрован ROT13: " << outputFile << endl;
        cout << "Пример преобразования (первые 100 символов):" << endl;
        
        // Показываем пример преобразования
        size_t sample_size = min(encrypted_content.size(), size_t(100));
        cout << "Зашифрованный текст: " << encrypted_content.substr(0, sample_size) << endl;
        cout << "Дешифрованный:       " << decrypted_content.substr(0, sample_size) << endl;

        return true;
    } catch (const exception& e) {
        cerr << "Ошибка в DecryptFileROT13: " << e.what() << endl;
        return false;
    }
}
