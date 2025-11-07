#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <unordered_map>
#include <openssl/evp.h>
 
using namespace std;
 
string sha1(const string& filepath) {
    ifstream fin(filepath, ios_base::binary);
    if (!fin.is_open()){
        cout << "Не удалось окрыть файл" << endl;
    }
    // Выделяем и возвращаем объект, который хранит все промежуточные состояния хэш-функции во время вычисления хэша.
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(ctx, EVP_sha1(), nullptr);
    char buffer[8192];

    while (fin.read(buffer, sizeof(buffer)) or (fin.gcount() > 0))
    { 
        EVP_DigestUpdate(ctx, buffer, fin.gcount()); // обновление, добавляем очередной блок данных к вычисляемому хешу. 
    }

    unsigned char hash[EVP_MAX_MD_SIZE]; // для результата хэширования
    unsigned int len = 0; // для длины хэша
    // вычисляет итоговый хэш, результат
    EVP_DigestFinal_ex(ctx, hash, &len);
    // форматируем результат и возвращаем его
    ostringstream out;
    out << hex << setfill('0');
    for (unsigned int i = 0; i < len; ++i)
        out << setw(2) << (int)hash[i];
    return out.str();
}

void get_files(vector<string>& paths, const string& path) {
    for(const auto& file : filesystem::directory_iterator(path)){
        if (filesystem::is_directory(file)){
             get_files(paths, file.path().string());
        }
        else {paths.push_back(file.path().string());}
    }
}

int main() {
    setlocale(LC_ALL, "Russian");
    vector<string> paths;
    get_files(paths, "/home/dimon/lab_UNIX/lab3/test_dir_5");
    unordered_map<string, filesystem::path> hash_map; 

    for (const auto& file_path : paths) {
        // вычисляем хэш
        string hash = sha1(file_path);
        auto iter = hash_map.find(hash);

        if (iter == hash_map.end()) {
            hash_map[hash] = filesystem::path(file_path);
        } else {
            filesystem::path original = iter->second;
            cout << "Удаляем" << file_path << endl;
            filesystem::remove(file_path);
            // создание жесткой ссылки 
            filesystem::create_hard_link(original, file_path); 
            cout << "Дубликат: " << file_path << ": " << "Исходный файл: " << original << endl;
        }
    }
}
