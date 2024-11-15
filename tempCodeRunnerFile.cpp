#include <iostream>
#include <string>
#include <stdexcept>

//for work with socket and network connection
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;



void connect_to_server_and_send_request(const int& port_serv, const string& ip_serv, const string& input){

    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(client_socket ==  -1){
        cerr << "Error: Failed to create socket\n";
        exit(EXIT_FAILURE);
    }


    sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port_serv);
    inet_pton(AF_INET, ip_serv.c_str(), &server_address.sin_addr);


    int status = connect(client_socket, (sockaddr*)&server_address, sizeof(server_address));
    if( status== -1 ){
        cerr << "Connection to the server failed";
        close(client_socket);
        return;
    }


    send(client_socket, input.c_str(), input.size(), 0);

    //answer from server
    char buffer[1024];
    int received_bytes_from_server = recv(client_socket, buffer, sizeof(buffer)-1, 0);

    if(received_bytes_from_server  > 0){
        buffer[received_bytes_from_server] = '\0';
        cout << "Server answer: " << buffer << "\n";
    } else{
        cout << "No answer from server\n";
    }

    close(client_socket);
}


void menu_for_client(){
    cout << "Menu:\n";
    cout << "[1] - Connect to the server and send it a request \n";
    cout << "[2] - Exit\n";
}


void MENU(const int& port_serv, const string& ip_serv){

    while(true){
        menu_for_client();
        cout<< "Select a number: 1 or 2\n";
        string choice;
        getline(cin, choice);

        if(choice == "1"){
            cout << "Enter a query: ";
            string query;
            getline(cin, query);

            connect_to_server_and_send_request(port_serv, ip_serv, query);
        }
        else if(choice == "2"){
            cout << "Goodbye!!!\n";
            break;
        }
        else{
            cout << "Input error: select 1 or 2\n";
        }
    }
}


int main(){

    const int SERVER_PORT = 7432;
    const string SERVER_IP = "127.0.0.1";
    MENU(SERVER_PORT, SERVER_IP);

    return 0;
}