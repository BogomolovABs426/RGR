
#include <iostream>
#include <string>
#include <stdexcept>
#include <limits>
#include <fstream>
#include <vector>
#include <dlfcn.h>
#include <map>
#include "utils.h"

using namespace std;

enum CryptoMethod {
    METHOD_NONE = 0,
    METHOD_DES = 1,
    METHOD_RC2 = 2,
    METHOD_ROT13 = 3
};

enum MenuAction {
    ACTION_ENCRYPT = 1,
    ACTION_DECRYPT = 2,
    ACTION_GENERATE_KEYS = 3,
    ACTION_EXIT = 4
};

// Объявления типов функций для динамической загрузки
typedef bool (*EncryptFunc)(const string&, const string&);
typedef bool (*DecryptFunc)(const string&, const string&);

// Структура для хранения указателей на функции библиотеки
struct LibraryFunctions {
    void* handle;
    EncryptFunc encrypt;
    DecryptFunc decrypt;
    string name;
    
    LibraryFunctions() : handle(nullptr), encrypt(nullptr), decrypt(nullptr), name("") {}
};

// Функция для работы с библиотеками
bool LoadLibrary(const string& libName, LibraryFunctions& lib, int mode = RTLD_LAZY) {
    lib.handle = dlopen(libName.c_str(), mode);
    if (!lib.handle) {
        return false;
    }
    
    // Загружаем функции шифрования/дешифрования
    lib.encrypt = (EncryptFunc)dlsym(lib.handle, "EncryptFileDES");
    if (!lib.encrypt) {
        lib.encrypt = (EncryptFunc)dlsym(lib.handle, "EncryptFileRC2");
    }
    if (!lib.encrypt) {
        lib.encrypt = (EncryptFunc)dlsym(lib.handle, "EncryptFileROT13");
    }
    
    lib.decrypt = (DecryptFunc)dlsym(lib.handle, "DecryptFileDES");
    if (!lib.decrypt) {
        lib.decrypt = (DecryptFunc)dlsym(lib.handle, "DecryptFileRC2");
    }
    if (!lib.decrypt) {
        lib.decrypt = (DecryptFunc)dlsym(lib.handle, "DecryptFileROT13");
    }
    
    return (lib.encrypt != nullptr && lib.decrypt != nullptr);
}

void UnloadLibrary(LibraryFunctions& lib) {
    if (lib.handle) {
        dlclose(lib.handle);
        lib.handle = nullptr;
        lib.encrypt = nullptr;
        lib.decrypt = nullptr;
    }
}

// Проверка пароля
bool CheckPassword() {
    string password;
    cout << "Введите пароль для доступа к программе: ";
    cin >> password;
    
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    
    return password == "123";
}

void ShowMainMenu() {
    cout << "\n=== Криптографическая система ===" << endl;
    cout << ACTION_ENCRYPT << ". Шифрование" << endl;
    cout << ACTION_DECRYPT << ". Дешифрование" << endl;
    cout << ACTION_GENERATE_KEYS << ". Генерация ключей" << endl;
    cout << ACTION_EXIT << ". Выход" << endl;
    cout << "Выберите действие: ";
}

void ShowMethodMenu(const map<CryptoMethod, LibraryFunctions>& availableMethods) {
    cout << "\n=== Выбор метода ===" << endl;
    for (const auto& method : availableMethods) {
        cout << method.first << ". " << method.second.name << endl;
    }
    cout << "Выберите метод: ";
}

void ShowKeyGenMenu() {
    cout << "\n=== Генерация ключей ===" << endl;
    cout << METHOD_DES << ". DES" << endl;
    cout << METHOD_RC2 << ". RC2" << endl;
    cout << METHOD_ROT13 << ". ROT13" << endl;
    cout << "Выберите метод: ";
}

// Выбор пользователем способа ввода данных
string GetInputData() {
    int choice;
    cout << "Хотите ввести с клавиатуры или загрузить из файла? (1 - с клавиатуры, 2 - из файла): ";
    cin >> choice;
    
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    
    if (choice == 1) {
        string data;
        cout << "Введите данные для обработки: ";
        getline(cin, data);
        
        // Создание временного файла для шифрования строки
        string tempFile = "temp.txt";
        ofstream out(tempFile);
        if (!out) {
            throw runtime_error("Не удалось создать временный файл");
        }
        out << data;
        out.close();
        
        return tempFile;
    } else if (choice == 2) {
        string filePath;
        cout << "Введите путь до файла: ";
        getline(cin, filePath);
        
        // Проверка существования файла
        ifstream check(filePath);
        if (!check) {
            throw runtime_error("Файл не существует: " + filePath);
        }
        check.close();
        
        return filePath;
    } else {
        throw runtime_error("Неверный выбор способа ввода");
    }
}

// Вывод результата
void DisplayResult(const string& filePath) {
    int choice;
    cout << "Вывести результат на экран? (1 - да, 2 - нет): ";
    cin >> choice;
    
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    
    if (choice == 1) {
        ifstream file(filePath, ios::binary);
        if (!file) {
            cout << "Не удалось открыть файл" << endl;
            return;
        }
        
        vector<unsigned char> buffer(
            (istreambuf_iterator<char>(file)),
            istreambuf_iterator<char>()
        );
        file.close();
        
        cout << "=== Содержимое файла ===" << endl;
        cout.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
        cout << endl << "=== Конец содержимого ===" << endl;
    }
}

int main() {
    try {
        if (!CheckPassword()) {
            cout << "Неверный пароль! Доступ запрещен." << endl;
            return 0;
        }
        
        cout << "Пароль принят. Добро пожаловать!" << endl;
        
        // Инициализация доступных библиотек
        map<CryptoMethod, LibraryFunctions> availableMethods;
        
        // Попытка загрузить каждую библиотеку
        LibraryFunctions desLib;
        if (LoadLibrary("./libdes.so", desLib)) {
            desLib.name = "DES";
            availableMethods[METHOD_DES] = desLib;
            cout << "Библиотека DES загружена" << endl;
        }
        
        LibraryFunctions rc2Lib;
        if (LoadLibrary("./librc2.so", rc2Lib)) {
            rc2Lib.name = "RC2";
            availableMethods[METHOD_RC2] = rc2Lib;
            cout << "Библиотека RC2 загружена" << endl;
        }
        
        LibraryFunctions rot13Lib;
        if (LoadLibrary("./librot13.so", rot13Lib)) {
            rot13Lib.name = "ROT13";
            availableMethods[METHOD_ROT13] = rot13Lib;
            cout << "Библиотека ROT13 загружена" << endl;
        }
        
        if (availableMethods.empty()) {
            cerr << "Не загружено ни одной библиотеки шифрования!" << endl;
            return 1;
        }
        
        int mainChoice, methodChoice;
        string inputFile, outputFile;
        
        while (true) {
            ShowMainMenu();
            cin >> mainChoice;
            
            if (cin.fail()) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Неверный ввод! Попробуйте снова." << endl;
                continue;
            }
            
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            
            switch (static_cast<MenuAction>(mainChoice)) {
                case ACTION_ENCRYPT:
                case ACTION_DECRYPT: {
                    if (availableMethods.empty()) {
                        cout << "Нет доступных методов шифрования" << endl;
                        break;
                    }
                    
                    ShowMethodMenu(availableMethods);
                    cin >> methodChoice;
                    
                    if (cin.fail()) {
                        cin.clear();
                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                        cout << "Неверный ввод! Попробуйте снова." << endl;
                        continue;
                    }
                    
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    
                    if (static_cast<MenuAction>(mainChoice) == ACTION_ENCRYPT) {
                        inputFile = GetInputData();
                    } else {
                        cout << "Введите путь к зашифрованному файлу: ";
                        getline(cin, inputFile);
                    }

                    cout << "Введите путь к выходному файлу: ";
                    getline(cin, outputFile);
                    
                    bool success = false;
                    
                    switch (static_cast<CryptoMethod>(methodChoice)) {
                        case METHOD_DES:
                            if (availableMethods.count(METHOD_DES)) {
                                if (static_cast<MenuAction>(mainChoice) == ACTION_ENCRYPT) {
                                    success = availableMethods[METHOD_DES].encrypt(inputFile, outputFile);
                                } else {
                                    success = availableMethods[METHOD_DES].decrypt(inputFile, outputFile);
                                }
                            } else {
                                cout << "Метод DES недоступен" << endl;
                            }
                            break;
                            
                        case METHOD_RC2:
                            if (availableMethods.count(METHOD_RC2)) {
                                if (static_cast<MenuAction>(mainChoice) == ACTION_ENCRYPT) {
                                    success = availableMethods[METHOD_RC2].encrypt(inputFile, outputFile);
                                } else {
                                    success = availableMethods[METHOD_RC2].decrypt(inputFile, outputFile);
                                }
                            } else {
                                cout << "Метод RC2 недоступен" << endl;
                            }
                            break;
                            
                        case METHOD_ROT13:
                            if (availableMethods.count(METHOD_ROT13)) {
                                if (static_cast<MenuAction>(mainChoice) == ACTION_ENCRYPT) {
                                    success = availableMethods[METHOD_ROT13].encrypt(inputFile, outputFile);
                                } else {
                                    success = availableMethods[METHOD_ROT13].decrypt(inputFile, outputFile);
                                }
                            } else {
                                cout << "Метод ROT13 недоступен" << endl;
                            }
                            break;
                            
                        default:
                            cout << "Неверный выбор метода" << endl;
                            continue;
                    }
                    
                    if (success) {
                        cout << "Операция завершена успешно" << endl;
                        DisplayResult(outputFile);
                    } else {
                        cout << "Ошибка при выполнении операции" << endl;
                    }
                    
                    // Удаляем временный файл
                    if (static_cast<MenuAction>(mainChoice) == ACTION_ENCRYPT && inputFile == "temp.txt") {
                        remove("temp.txt");
                    }
                    break;
                }
                    
                case ACTION_GENERATE_KEYS: {
                    ShowKeyGenMenu();
                    cin >> methodChoice;
                    
                    if (cin.fail()) {
                        cin.clear();
                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                        cout << "Неверный ввод! Попробуйте снова." << endl;
                        continue;
                    }
                    
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    
                    bool success = false;
                    
                    switch (static_cast<CryptoMethod>(methodChoice)) {
                        case METHOD_DES:
                            success = GenerateDESKey();
                            break;
                            
                        case METHOD_RC2:
                            success = GenerateRC2Key();
                            break;
                            
                        case METHOD_ROT13:
                            cout << "Для ROT13 ключ не требуется" << endl;
                            success = true;
                            break;
                            
                        default:
                            cout << "Неверный выбор метода" << endl;
                            continue;
                    }
                    
                    if (success) {
                        cout << "Ключи успешно сгенерированы!" << endl;
                    } else {
                        cout << "Ошибка при генерации ключей" << endl;
                    }
                    break;
                }
                    
                case ACTION_EXIT:
                    cout << "Выход из программы." << endl;
                    // Выгрузка библиотек
                    for (auto& method : availableMethods) {
                        UnloadLibrary(method.second);
                    }
                    return 0;
                    
                default:
                    cout << "Неверный выбор! Попробуйте снова." << endl;
                    continue;
            }
        }
        
        return 0;
    } catch (const exception& e) {
        cerr << "Критическая ошибка в main: " << e.what() << endl;
        return 1;
    }
}