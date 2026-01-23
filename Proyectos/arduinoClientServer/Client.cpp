// --- Windows sockets (SIEMPRE primero) ---
#include <winsock2.h>
#include <ws2tcpip.h>

// --- Windows serial ---
#include <windows.h>

// --- C++ estándar ---
#include <iostream>
#include <string>
#include <cstring>

int main() {
    /* ======================
       1. WINSOCK INIT
       ====================== */
       
    // 1. Initialize Winsock   
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        return 1;
    }
	
	// 2. Create socket
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Socket creation failed\n";
        WSACleanup();
        return 1;
    }
	
	// 3. Server address
    sockaddr_in server{};
    server.sin_family = AF_INET;
    server.sin_port = htons(65432);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
	
	// 4. Connect to Python server
    if (connect(sock, (sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
        std::cerr << "Connection failed\n";
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    std::cout << "Connected to Python server\n";

    /* ======================
       2. SERIAL INIT
       ====================== */
    
    // 5. Send data (simulate iterations)
    HANDLE hSerial = CreateFile(
        "\\\\.\\COM4",   // <<< CAMBIA COM
        GENERIC_READ,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
    );

    if (hSerial == INVALID_HANDLE_VALUE) {
        std::cerr << "Error opening serial port\n";
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    DCB dcb = {0};
    dcb.DCBlength = sizeof(dcb);
    GetCommState(hSerial, &dcb);
    dcb.BaudRate = CBR_9600;
    dcb.ByteSize = 8;
    dcb.StopBits = ONESTOPBIT;
    dcb.Parity   = NOPARITY;
    SetCommState(hSerial, &dcb);

    std::cout << "Serial connected. Streaming...\n";

    /* ======================
       3. STREAM LOOP
       ====================== */
    char ch;
    DWORD bytesRead;
    std::string line;

    while (true) {
        ReadFile(hSerial, &ch, 1, &bytesRead, NULL);

        if (bytesRead == 0) continue;

        if (ch == '\n') {
            // Línea completa desde Arduino
            std::cout << "Serial: " << line << std::endl;

            // Enviar EXACTAMENTE esa línea a Python
            std::string msg = line + "\n";
            send(sock, msg.c_str(), msg.size(), 0);

            line.clear();
        }
        else if (ch != '\r') {
            line += ch;
        }
    }

    /* ======================
       4. CLEANUP (nunca llega)
       ====================== */
    CloseHandle(hSerial);
    closesocket(sock);
    WSACleanup();
    return 0;
}
