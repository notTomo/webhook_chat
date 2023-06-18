#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

const int MAX_CLIENTS = 10;
const int BUFFER_SIZE = 4096;
const int SERVER_PORT = 8080;

void handleClient( int clientSocket, const std::string& webhookUrl )
{
    char buffer[ BUFFER_SIZE ];

    while( true )
    {
        // Receive username from client
        memset( buffer, 0, sizeof( buffer ) );
        int bytesRead = recv( clientSocket, buffer, sizeof( buffer ) - 1, 0 );

        if( bytesRead <= 0 )
        {
            // Client disconnected
            break;
        }

        std::string username( buffer );

        // Receive message from client
        memset( buffer, 0, sizeof( buffer ) );
        bytesRead = recv( clientSocket, buffer, sizeof( buffer ) - 1, 0 );

        if( bytesRead <= 0 )
        {
            // Client disconnected
            break;
        }

        std::string message( buffer );

        // Send message to Discord webhook
        // Replace this code with your logic to send the message to the webhook
        std::cout << "[R] " << username << ": " << message << std::endl;
    }

    closesocket( clientSocket );
}

int main( )
{
    int serverSocket;
    sockaddr_in serverAddress{ };
    std::vector<std::thread> clientThreads;

    WSADATA wsaData;
    if( WSAStartup( MAKEWORD( 2, 2 ), &wsaData ) != 0 )
    {
        std::cerr << "Failed to initialize Winsock." << std::endl;
        return 1;
    }

    // Create server socket
    serverSocket = socket( AF_INET, SOCK_STREAM, 0 );
    if( serverSocket == -1 )
    {
        std::cerr << "Failed to create server socket." << std::endl;
        return 1;
    }

    // Set up server address
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons( SERVER_PORT );

    // Bind server socket to the specified port
    if( bind( serverSocket, ( struct sockaddr* )&serverAddress, sizeof( serverAddress ) ) < 0 )
    {
        std::cerr << "Failed to bind server socket to port " << SERVER_PORT << "." << std::endl;
        return 1;
    }

    // Start listening for client connections
    if( listen( serverSocket, MAX_CLIENTS ) < 0 )
    {
        std::cerr << "Failed to start listening for client connections." << std::endl;
        return 1;
    }

    std::cout << "Chat server is running. Listening for incoming connections..." << std::endl;

    while( true )
    {
        // Accept client connection
        sockaddr_in clientAddress{ };
        socklen_t clientAddressLength = sizeof( clientAddress );
        int clientSocket = accept( serverSocket, ( struct sockaddr* )&clientAddress, &clientAddressLength );
        if( clientSocket < 0 )
        {
            std::cerr << "Failed to accept client connection." << std::endl;
            continue;
        }

        // Start a new thread to handle the client
        std::thread clientThread( handleClient, clientSocket, "YOUR_WEBHOOK" );
        clientThreads.push_back( std::move( clientThread ) );
    }

    // Wait for client threads to finish
    for( auto& thread : clientThreads )
    {
        thread.join( );
    }

    // Close server socket
    closesocket( serverSocket );

    return 0;
}
