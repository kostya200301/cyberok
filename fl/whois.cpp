#include <iostream>
#include <string>
#include <curl/curl.h>

// Callback функция для записи ответа в строку
size_t writeCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t totalSize = size * nmemb;
    output->append((char*)contents, totalSize);
    return totalSize;
}

std::string getWhoisInfo(const std::string& domain) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Ошибка инициализации CURL" << std::endl;
        return "";
    }

    std::string response;
    std::string url = "https://rdap.verisign.com/com/v1/domain/" + domain;

    // Устанавливаем URL для запроса
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    // Устанавливаем функцию обратного вызова для записи ответа в строку
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    // Выполняем запрос
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "Ошибка выполнения запроса: " << curl_easy_strerror(res) << std::endl;
        response = "";
    }

    curl_easy_cleanup(curl);

    return response;
}


