#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/select.h>

#define PORT 3457

// Объявление обработчика сигнала
volatile sig_atomic_t wasSigHup = 0;

void sigHupHandler(int r)
{
    wasSigHup = 1;
}

int main()
{
    int clientSocket = -1;
    struct sockaddr_in serverAddr; 
    fd_set fds;  // Множество файловых дескрипторов для pselect
    sigset_t blockedMask, origMask;
    struct sigaction sa;
    char buffer[256];
    
    // Создание сокета
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0); 
    if (serverSocket == -1) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT); 
    

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("bind failed");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }
    
    if (listen(serverSocket, 5) == -1) {
        perror("listen failed");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }
    
    printf("Server listening on port %d\n", PORT);
    
    // Регистрация обработчика сигнала
    sigaction(SIGHUP, NULL, &sa); 
    sa.sa_handler = sigHupHandler; 
    sa.sa_flags |= SA_RESTART; 
    sigaction(SIGHUP, &sa, NULL); 
    
    // Блокировка сигнала
    sigemptyset(&blockedMask); 
    sigaddset(&blockedMask, SIGHUP); 
    sigprocmask(SIG_BLOCK, &blockedMask, &origMask); 
    
    while (1) {
        FD_ZERO(&fds);
        FD_SET(serverSocket, &fds);
        int maxFd = serverSocket;
        
        //Подготовка списка файловых дескрипторов
        if (clientSocket != -1) {
            FD_SET(clientSocket, &fds);
            if (clientSocket > maxFd) {
                maxFd = clientSocket;
            }
        }
        
        if (pselect(maxFd + 1, &fds, NULL, NULL, NULL, &origMask) == -1) { //мониторинг дескрипторов с временной разблокировкой сигнала
            if (errno == EINTR && wasSigHup) { // Если прервано сигналом и это наш SIGHUP
                printf("Received SIGHUP signal\n");
                wasSigHup = 0;
                continue;
            }
            perror("pselect failed");
            break;
        }
        
        // Новое подключение
        if (FD_ISSET(serverSocket, &fds)) {
            int newClient = accept(serverSocket, NULL, NULL);
            if (newClient == -1) {
                perror("accept failed");
            } else {
                printf("New connection accepted\n");
                if (clientSocket == -1) {
                    clientSocket = newClient;  // Сохраняем только первого клиента
                } else {
                    printf("The server is busy, disable newClient\n");
                    close(newClient); // Закрываем дополнительных клиентов
                }
            }
        }
        
        // Вывод данных от клиента
        if (clientSocket != -1 && FD_ISSET(clientSocket, &fds)) {
            ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
            if (bytesRead > 0) {
                printf("Received %zd bytes\n", bytesRead);
            } else {
                close(clientSocket);
                clientSocket = -1;
            }
        }
    }
    // Завершение работы
    if (clientSocket != -1) {
        close(clientSocket);
    }
    close(serverSocket);
    
    return 0;
}