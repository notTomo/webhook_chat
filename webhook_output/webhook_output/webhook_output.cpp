#include <iostream>
#include <string>
#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

const int BUFFER_SIZE = 4096;
const int SERVER_PORT = 8080;
const std::string WEBHOOK_URL = "YOUR_WEBHOOK";

void sendToDiscordWebhook( const std::string& message, const std::string& username )
{
    // Construct the JSON payload with the message and username
    std::string payload = "{\"content\":\"" + message + "\",\"username\":\"" + username + "\"}";


    WSADATA wsaData;
    if( WSAStartup( MAKEWORD( 2, 2 ), &wsaData ) != 0 )
    {
        std::cerr << "Failed to initialize Winsock." << std::endl;
        return;
    }

    // Create socket
    SOCKET clientSocket = socket( AF_INET, SOCK_STREAM, 0 );
    if( clientSocket == INVALID_SOCKET )
    {
        std::cerr << "Failed to create client socket." << std::endl;
        WSACleanup( );
        return;
    }

    // Extract host and path from the webhook URL
    std::string host;
    std::string path;

    size_t protocolIndex = WEBHOOK_URL.find( "://" );
    if( protocolIndex != std::string::npos )
    {
        size_t hostIndex = protocolIndex + 3;
        size_t pathIndex = WEBHOOK_URL.find( "/", hostIndex );

        if( pathIndex != std::string::npos )
        {
            host = WEBHOOK_URL.substr( hostIndex, pathIndex - hostIndex );
            path = WEBHOOK_URL.substr( pathIndex );
        }
        else
        {
            host = WEBHOOK_URL.substr( hostIndex );
            path = "/";
        }
    }

    // Resolve host name to IP address
    addrinfo* result = nullptr;
    if( getaddrinfo( host.c_str( ), "https", nullptr, &result ) != 0 )
    {
        std::cerr << "Failed to resolve host name." << std::endl;
        closesocket( clientSocket );
        WSACleanup( );
        return;
    }

    sockaddr_in serverAddress{ };
    serverAddress.sin_family = AF_INET;
    inet_pton( AF_INET, "127.0.0.1", &( serverAddress.sin_addr ) ); // Replace with the server IP address
    serverAddress.sin_port = htons( 443 );

    // Connect to the server
    if( connect( clientSocket, ( sockaddr* )&serverAddress, sizeof( serverAddress ) ) == SOCKET_ERROR )
    {
    //    std::cerr << "Failed to connect to the server." << std::endl;
        closesocket( clientSocket );
        WSACleanup( );
        return;
    }

    // Construct HTTP POST request
    std::string request =
        "POST " + path + " HTTP/1.1\r\n"
        "Host: " + host + "\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: " + std::to_string( message.length( ) ) + "\r\n"
        "\r\n"
        + message;

    // Send request to the server
    int bytesSent = send( clientSocket, request.c_str( ), request.length( ), 0 );
    if( bytesSent == SOCKET_ERROR )
    {
        std::cerr << "Failed to send request to the server." << std::endl;
        closesocket( clientSocket );
        WSACleanup( );
        return;
    }

    // Receive response from the server (optional)
    char buffer[ BUFFER_SIZE ];
    memset( buffer, 0, sizeof( buffer ) );

    int bytesRead;
    while( ( bytesRead = recv( clientSocket, buffer, sizeof( buffer ) - 1, 0 ) ) > 0 )
    {
        // Process response here if needed
        // ...
        memset( buffer, 0, sizeof( buffer ) );
    }

    // Close the socket
    closesocket( clientSocket );

    // Cleanup Winsock
    WSACleanup( );
}

int main( )
{
    WSADATA wsaData;
    if( WSAStartup( MAKEWORD( 2, 2 ), &wsaData ) != 0 )
    {
        std::cerr << "Failed to initialize Winsock." << std::endl;
        return 1;
    }

    // Create socket
    SOCKET clientSocket = socket( AF_INET, SOCK_STREAM, 0 );
    if( clientSocket == INVALID_SOCKET )
    {
        std::cerr << "Failed to create client socket." << std::endl;
        WSACleanup( );
        return 1;
    }

    // Set up server address
    sockaddr_in serverAddress{ };
    serverAddress.sin_family = AF_INET;
    inet_pton( AF_INET, "127.0.0.1", &( serverAddress.sin_addr ) ); // Replace with the server IP address
    serverAddress.sin_port = htons( SERVER_PORT );

    // Connect to the server
    if( connect( clientSocket, ( sockaddr* )&serverAddress, sizeof( serverAddress ) ) == SOCKET_ERROR )
    {
    //    std::cerr << "Failed to connect to the server." << std::endl;
        closesocket( clientSocket );
        WSACleanup( );
        return 1;
    }

    // Get the username from the user
    std::string username;
    std::cout << "Enter your username: ";
    std::getline( std::cin, username );

    // Send messages to the server
    std::string message;
    do
    {
        std::cout << "Enter a message (or 'q' to quit): ";
        std::getline( std::cin, message );

        // Send username to the server
        int usernameBytesSent = send( clientSocket, username.c_str( ), username.length( ), 0 );
        if( usernameBytesSent == SOCKET_ERROR )
        {
            std::cerr << "Failed to send username to the server." << std::endl;
            break;
        }

        // Send message to the server
        int messageBytesSent = send( clientSocket, message.c_str( ), message.length( ), 0 );
        if( messageBytesSent == SOCKET_ERROR )
        {
            std::cerr << "Failed to send message to the server." << std::endl;
            break;
        }

        // Send message to Discord webhook
        sendToDiscordWebhook( message, username );

    } while( message != "q" );

    // Close the socket
    closesocket( clientSocket );

    // Cleanup Winsock
    WSACleanup( );

    return 0;
}
