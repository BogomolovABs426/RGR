#include "des.h"
#include "utils.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <stdexcept>
#include <limits>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <openssl/provider.h>

using namespace std;

// Вспомогательная функция для вывода ошибок OpenSSL
void print_openssl_error() {
    char error_buffer[256];
    ERR_error_string_n(ERR_get_error(), error_buffer, sizeof(error_buffer));
    cerr << "OpenSSL Error: " << error_buffer << endl;
}

// Шифрование DES с legacy-провайдером
bool EncryptFileDES(const string& inputFile, const string& outputFile) {
    EVP_CIPHER_CTX* ctx = nullptr;
    OSSL_PROVIDER* legacy_provider = nullptr;
    
    try {
        // Загружаем legacy-провайдер для поддержки DES
        legacy_provider = OSSL_PROVIDER_load(nullptr, "legacy");
        if (!legacy_provider) {
            cerr << "Предупреждение: не удалось загрузить legacy-провайдер" << endl;
        }
        
        string key_str;
        
        cout << "Введите ключ DES (8 байт, например: mykey123): ";
        getline(cin, key_str);
        
        // Обеспечиваем ключ длиной 8 байт
        if (key_str.length() < 8) {
            cout << "Ключ слишком короткий. Дополняем пробелами до 8 байт." << endl;
            key_str.resize(8, ' ');
        } else if (key_str.length() > 8) {
            cout << "Ключ слишком длинный. Обрезаем до 8 байт." << endl;
            key_str = key_str.substr(0, 8);
        }
        
        cout << "Используемый ключ: '" << key_str << "'" << endl;
        
        // Генерируем случайный вектор инициализации
        unsigned char iv[8];
        if (RAND_bytes(iv, sizeof(iv)) != 1) {
            throw runtime_error("Не удалось сгенерировать IV");
        }
        
        // Создаем контекст шифрования
        ctx = EVP_CIPHER_CTX_new();
        if (!ctx) {
            throw runtime_error("Не удалось создать контекст шифрования");
        }
        
        // Инициализируем контекст для DES-CBC с legacy-провайдером
        const EVP_CIPHER* cipher = EVP_CIPHER_fetch(nullptr, "DES-CBC", "provider=legacy");
        if (!cipher) {
            cipher = EVP_des_cbc(); // Попробуем стандартный способ
        }
        
        if (!cipher) {
            throw runtime_error("Алгоритм DES-CBC не поддерживается");
        }
        
        if (EVP_EncryptInit_ex(ctx, cipher, nullptr, 
                              reinterpret_cast<const unsigned char*>(key_str.c_str()), 
                              iv) != 1) {
            print_openssl_error();
            throw runtime_error("Ошибка инициализации шифрования");
        }
        
        // Читаем входной файл
        ifstream input(inputFile, ios::binary);
        if (!input) {
            throw runtime_error("Не удалось открыть входной файл: " + inputFile);
        }
        
        input.seekg(0, ios::end);
        size_t file_size = input.tellg();
        input.seekg(0, ios::beg);
        
        if (file_size == 0) {
            throw runtime_error("Входной файл пуст");
        }
        
        vector<unsigned char> plaintext(file_size);
        if (!input.read(reinterpret_cast<char*>(plaintext.data()), file_size)) {
            throw runtime_error("Ошибка чтения файла");
        }
        input.close();
        
        // Выделяем память для шифротекста
        vector<unsigned char> ciphertext(plaintext.size() + EVP_MAX_BLOCK_LENGTH);
        int len = 0;
        int ciphertext_len = 0;
        
        // Шифруем данные
        if (EVP_EncryptUpdate(ctx, ciphertext.data(), &len, 
                             plaintext.data(), plaintext.size()) != 1) {
            print_openssl_error();
            throw runtime_error("Ошибка шифрования данных");
        }
        ciphertext_len = len;
        
        // Завершаем шифрование
        if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len) != 1) {
            print_openssl_error();
            throw runtime_error("Ошибка завершения шифрования");
        }
        ciphertext_len += len;
        
        // Изменяем размер
        ciphertext.resize(ciphertext_len);
        
        // Записываем результат
        ofstream output(outputFile, ios::binary);
        if (!output) {
            throw runtime_error("Не удалось создать выходной файл: " + outputFile);
        }
        
        // Сохраняем размер и IV
        output.write(reinterpret_cast<const char*>(&file_size), sizeof(file_size));
        output.write(reinterpret_cast<const char*>(iv), sizeof(iv));
        output.write(reinterpret_cast<const char*>(ciphertext.data()), ciphertext.size());
        output.close();
        
        // Освобождаем ресурсы
        EVP_CIPHER_CTX_free(ctx);
        if (cipher && cipher != EVP_des_cbc()) {
            EVP_CIPHER_free((EVP_CIPHER*)cipher);
        }
        if (legacy_provider) {
            OSSL_PROVIDER_unload(legacy_provider);
        }
        
        cout << "Файл успешно зашифрован DES: " << outputFile << endl;
        return true;
    } catch (const exception& e) {
        if (ctx) EVP_CIPHER_CTX_free(ctx);
        if (legacy_provider) OSSL_PROVIDER_unload(legacy_provider);
        cerr << "Ошибка в EncryptFileDES: " << e.what() << endl;
        return false;
    }
}

// Дешифрование DES
bool DecryptFileDES(const string& inputFile, const string& outputFile) {
    EVP_CIPHER_CTX* ctx = nullptr;
    OSSL_PROVIDER* legacy_provider = nullptr;
    
    try {
        // Загружаем legacy-провайдер
        legacy_provider = OSSL_PROVIDER_load(nullptr, "legacy");
        
        string key_str;
        
        cout << "Введите ключ DES (8 байт): ";
        getline(cin, key_str);
        
        if (key_str.length() < 8) {
            key_str.resize(8, ' ');
        } else if (key_str.length() > 8) {
            key_str = key_str.substr(0, 8);
        }
        
        // Читаем входной файл
        ifstream input(inputFile, ios::binary);
        if (!input) {
            throw runtime_error("Не удалось открыть входной файл: " + inputFile);
        }
        
        size_t original_size;
        if (!input.read(reinterpret_cast<char*>(&original_size), sizeof(original_size))) {
            throw runtime_error("Не удалось прочитать размер данных");
        }
        
        unsigned char iv[8];
        if (!input.read(reinterpret_cast<char*>(iv), sizeof(iv))) {
            throw runtime_error("Не удалось прочитать IV");
        }
        
        vector<unsigned char> ciphertext(
            (istreambuf_iterator<char>(input)),
            istreambuf_iterator<char>()
        );
        input.close();
        
        // Создаем контекст
        ctx = EVP_CIPHER_CTX_new();
        if (!ctx) {
            throw runtime_error("Не удалось создать контекст");
        }
        
        // Инициализируем для дешифрования
        const EVP_CIPHER* cipher = EVP_CIPHER_fetch(nullptr, "DES-CBC", "provider=legacy");
        if (!cipher) {
            cipher = EVP_des_cbc();
        }
        
        if (EVP_DecryptInit_ex(ctx, cipher, nullptr,
                              reinterpret_cast<const unsigned char*>(key_str.c_str()),
                              iv) != 1) {
            print_openssl_error();
            throw runtime_error("Ошибка инициализации дешифрования");
        }
        
        // Дешифруем
        vector<unsigned char> plaintext(ciphertext.size() + EVP_MAX_BLOCK_LENGTH);
        int len = 0;
        int plaintext_len = 0;
        
        if (EVP_DecryptUpdate(ctx, plaintext.data(), &len,
                             ciphertext.data(), ciphertext.size()) != 1) {
            print_openssl_error();
            throw runtime_error("Ошибка дешифрования");
        }
        plaintext_len = len;
        
        if (EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len) != 1) {
            print_openssl_error();
            throw runtime_error("Ошибка завершения дешифрования");
        }
        plaintext_len += len;
        
        plaintext.resize(plaintext_len);
        
        // Записываем результат
        ofstream output(outputFile, ios::binary);
        if (!output) {
            throw runtime_error("Не удалось создать выходной файл");
        }
        
        output.write(reinterpret_cast<const char*>(plaintext.data()), plaintext.size());
        output.close();
        
        // Освобождаем ресурсы
        EVP_CIPHER_CTX_free(ctx);
        if (cipher && cipher != EVP_des_cbc()) {
            EVP_CIPHER_free((EVP_CIPHER*)cipher);
        }
        if (legacy_provider) {
            OSSL_PROVIDER_unload(legacy_provider);
        }
        
        cout << "Файл успешно дешифрован DES: " << outputFile << endl;
        return true;
    } catch (const exception& e) {
        if (ctx) EVP_CIPHER_CTX_free(ctx);
        if (legacy_provider) OSSL_PROVIDER_unload(legacy_provider);
        cerr << "Ошибка в DecryptFileDES: " << e.what() << endl;
        return false;
    }
}
