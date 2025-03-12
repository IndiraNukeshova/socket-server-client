#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <fstream>

#define BUFFER_SIZE 1024
#define PORT 2500

int main() {
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        std::cerr << "Не удалось создать сокет" << std::endl;
        return EXIT_FAILURE;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, "127.0.0.1", &(serverAddr.sin_addr)) <= 0) {
        std::cerr << "Неверный адрес" << std::endl;
        return EXIT_FAILURE;
    }

    // установка связи с сервером. connect принимает сокет-дескриптор клиента, указатель на адрес сервера (структура sockaddr), длина структуры адреса
    if (connect(client_socket, reinterpret_cast<struct sockaddr *>(&serverAddr), sizeof(serverAddr)) < 0) {
        std::cerr <<"Связь с сервером не удалось установить"  << std::endl;
        return EXIT_FAILURE;
    }

    // для обработки запросов с терминала 
    std::string userInput;
    std::cout << "Введите запрос в виде простого сообщения/image.png/HEAD: ";
    std::getline(std::cin, userInput);

    // отправка запроса серверу
    std::string request = "HEAD /" + userInput + " HTTP/1.1\r\n\r\n";
    if (send(client_socket, request.c_str(), request.size(), 0) < 0) {
        std::cerr <<"не удалось отправить сообщ серверу" << std::endl;
        return EXIT_FAILURE;
    }

    // формируем ответ от сервера
    char buffer[BUFFER_SIZE];
    ssize_t bytesRead = recv(client_socket, buffer, BUFFER_SIZE, 0);
    if (bytesRead > 0) {
      // Выводим полученные данные в терминал
      std::cout.write(buffer, bytesRead);
    }

    close(client_socket);
    return 0;
}
