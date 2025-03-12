#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <fstream>
#include <arpa/inet.h>
#include <thread>

#define PORT 2500
const int MAX_CLIENTS = 10;
const int BUFFER_SIZE = 1024;

// Функция для обработки клиентских подключений
void handleClient(int client_socket) {
    char buffer[BUFFER_SIZE];
    std::string response;

// Чтение HTTP-запроса от клиента
    memset(buffer, 0, BUFFER_SIZE);
    ssize_t bytesRead = read(client_socket, buffer, BUFFER_SIZE);
    if (bytesRead <= 0) {
        std::cerr << "Связь прервана. Сокет: " << client_socket << std::endl;
        close(client_socket);
        return;
    }else {
            std::cout << "Обрабатываем данные, полученные из сокета " << client_socket << ": " << buffer << std::endl;
        }

// Извлекаем запрошенное имя файла из HTTP-запроса
    std::string request(buffer);
    std::size_t pos1 = request.find("GET /") + 5;
    std::size_t pos2 = request.find(" HTTP/1.1");
    if (pos1 != std::string::npos && pos2 != std::string::npos) {
        std::string fileName = request.substr(pos1, pos2 - pos1);

        if (fileName == "HEAD") {
            // Формируем ответ HEAD
            response = "HTTP/1.1 200 OK\r\n";
            response += "Content-Type: head type\r\n";
            response += "Connection: close\r\n";
            response += "\r\n";

            // сохраняем в текстовый файл 
            std::ofstream outputFile("header.txt");
            if (outputFile) {
                outputFile << response;
                outputFile.close();
            } else {
                std::cerr << "Не удалось открыть файл header.txt для записи" << std::endl;
            }
        } else {
            // Отправляем запрошенный текст в качестве ответа
            response = "HTTP/1.1 200 OK\r\n";
            response += "Content-Type: text type\r\n";
            response += "Connection: close\r\n";
            response += "\r\n";
            response += "You requested: " + fileName;
        }
    } else {
        std::cerr << "Недопустимый формат HTTP-запроса." << std::endl;
        close(client_socket);
        return;
    }

// Отправляем HTTP-ответ клиенту
    ssize_t bytesSent = send(client_socket, response.c_str(), response.length(), 0);
    if (bytesSent < 0) {
        std::cerr << "Не удалось отправить HTTP-ответ клиенту" << std::endl;
    }

    close(client_socket);
}

int main() {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        std::cerr << "Не удалось создать сокет" << std::endl;
        return EXIT_FAILURE;
    }

    struct sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cerr << "Не удалось привязать сокет" << std::endl;
        return EXIT_FAILURE;
    }

    if (listen(server_socket, MAX_CLIENTS) == -1) {
        std::cerr << "Не удалось прослушать соединение" << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "Сервер работает по адресу http://localhost: " << PORT << std::endl;

    while (true) {
        sockaddr_in clientAddr{};
        socklen_t clientAddrSize = sizeof(clientAddr);
        int clientSocket = accept(server_socket, (struct sockaddr *)&clientAddr, &clientAddrSize);
        if (clientSocket == -1) {
            std::cerr <<
"Не удалось принять соединение" << std::endl;
            return EXIT_FAILURE;
        }

        char ipAddr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(clientAddr.sin_addr), ipAddr, INET_ADDRSTRLEN);
        std::cout << "Новое подключение принято.\nIP-адрес клиента: " << ipAddr
                  << ",\nПорт: " << ntohs(clientAddr.sin_port) << std::endl;

        // // Создаем новый поток для обработки клиентского соединения
        std::thread clientThread(handleClient, clientSocket);
        clientThread.detach(); 
    }

    close(server_socket);
    return 0;
}
