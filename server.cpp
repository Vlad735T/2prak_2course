#include "work_with_files.h"

#include "headers/Hash.h"

//for server
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>

#include <mutex>
#include <thread>





LinkedList<string> parse_command(string userInput){

    LinkedList<string> recycle_input;
    string word = "";
    for (auto symbol : userInput){
        if (symbol == '\'' || symbol == '(' || symbol == ')' || symbol == ' ' || symbol == ','){
            if (!word.empty()){
                recycle_input.addtail(word);
            }
            word = "";
            continue;
        }
        word += symbol;
    }
    return recycle_input;
}



LinkedList<HASH<string>> get_lines_in_table(const string& tableName){
    
    LinkedList<HASH<string>> table;
    int cout_lines_in_file = getPKSEQ(tableName);
    LinkedList<string> columnNames = get_columns_from_file(tableName);
    
    int filesCounter = ceil(static_cast<double>(cout_lines_in_file)/1000); 

    for (int i = 0; i < filesCounter; i++){
        int startRow = i * 1000;
        int endRow = min(startRow + 1000, cout_lines_in_file);
        string fileInput;

        string pathToDir = filesystem::current_path();
        pathToDir += "/" + tableName;

        ifstream csv(pathToDir + "/" + to_string(i+1) + ".csv");
        for (int row = startRow; row < endRow; row++){

            getline(csv, fileInput, '\n');
            HASH<string> tableLine(columnNames.size());
            string word = "";
            int wordCounter = 0;
            for (auto symbol : fileInput){
                if (symbol == ','){
                    tableLine.HSET(columnNames.get(wordCounter), word);
                    word = "";
                    wordCounter++;
                    continue;
                }
                word += symbol;
            }
            if (!word.empty()) tableLine.HSET(columnNames.get(wordCounter), word);
            table.addtail(tableLine);
        }
        csv.close();
    }
    return table;
}

LinkedList<HASH<string>> read_table(const string& tableName){

    LinkedList<HASH<string>> table;
    string pathToDir = filesystem::current_path();
    pathToDir += "/" + tableName;

    int cout_of_lines = getPKSEQ(tableName);
    int fileCount = ceil(static_cast<double>(cout_of_lines) / 1000);
    
    for (int i = 0; i < fileCount; ++i){
        fstream csv_file(pathToDir + "/" + to_string(i+1) + ".csv");
        //create file
        if (!csv_file.good()){
            ofstream newFile(pathToDir + "/" + to_string(i+1) + ".csv");
            newFile.flush();
            newFile.close();
        }
        csv_file.flush();
        csv_file.close();
    }

    table = get_lines_in_table(tableName);
    return table;
}



bool check_table(const string& element){
    for (auto sym : element){
        if(sym == '.') return true;
    }
    return false;
}

string extract_table(const string& word){
    string tableName = "";
    for (auto sym : word){
        if (sym == '.') return tableName;
        else tableName += sym;
    }
    return "";
}   

string get_column(const string& word){
    bool writeMode = 0;
    string tableName = "";
    for (auto sym : word){
        if (writeMode == 1){
            tableName += sym;
        }
        if (sym == '.') writeMode = 1;
    }
    return tableName;
}

LinkedList<string> get_tables_before_where(LinkedList<string> commandList) {

    LinkedList<string> selected;
    
    bool writeMode = false;
    for (int i = 0; i < commandList.size(); i++) {
        auto token = commandList.get(i);
        
        if (token == "WHERE") break;  
        if (writeMode) selected.addtail(token); 
        if (token == "FROM") writeMode = true;   
    }
    return selected;
}

bool end_result(LinkedList<bool> results, LinkedList<string> operators){

    bool finalRes;
    if (operators.size() == 0) return results.get(0);
    else{
        for (int i = 0; i < results.size() - 1; i++){
            if (i == 0) finalRes = results.get(0);
            if (operators.get(i)== "AND"){
                finalRes = finalRes && results.get(i + 1);
            }
            else if (operators.get(i)== "OR"){
                finalRes = finalRes || results.get(i + 1);
            }
        }
    }
    return finalRes;
}




//REALIZATION OF INSERT
void upload_table(LinkedList<HASH<string>> table, string tableName){
    
    int num_lines = getPKSEQ(tableName);
    int count_of_csv = ceil(static_cast<double>(num_lines) / 1000);
    LinkedList<string> columnNames = get_columns_from_file(tableName);
    string path_directory = filesystem::current_path();
    path_directory += "/" + tableName;

    for (int i = 0; i < count_of_csv; ++i) {
        //diapozon strok
        int start_row = i * 1000;
        int endrow = min(start_row + 1000, num_lines);
        // for perezapis
        ofstream update(path_directory + "/" + to_string(i + 1) + ".csv", ios::out | ios::trunc);
        if (!update.is_open()) throw runtime_error("Error opening csv for table upload");

        for (int row = start_row; row < endrow; row++){

            for (int column = 0; column < columnNames.size(); column++){

                auto current_row = table.get(row);

                if (column == columnNames.size() - 1){
                    update << current_row.HGET(columnNames.get(column));
                    update << "\n";
                }
                else{
                    update << current_row.HGET(columnNames.get(column));
                    update << ",";
                }
                
            }
        }
        update.flush();
        update.close();
    }
}

void INSERT(LinkedList<string> values, string tableName){

    mutex for_insert;
    lock_guard<mutex> INSERT_MUTEX(for_insert);

    lock_table(tableName);

    LinkedList<HASH<string>> table = read_table(tableName);
    LinkedList<string> columnNames = get_columns_from_file(tableName);
    HASH<string> row(columnNames.size());

    if (values.size() == columnNames.size()){   
        for (int i = 0; i < columnNames.size(); i++){
            row.HSET(columnNames.get(i),values.get(i));
        }
        table.addtail(row);
        update_up_pksq(tableName);
    }
    else if (values.size() < columnNames.size()){
        for (int i = 0; i < columnNames.size(); i++){
            if (i >= values.size()){
                row.HSET(columnNames.get(i),"EMPTY");
            }else{
                row.HSET(columnNames.get(i),values.get(i));
            }
        }
        table.addtail(row);
        update_up_pksq(tableName);
    }
    else{
        unlock_table(tableName);
        throw runtime_error("Error: the number of words is more than the number of columns in table!!!");
    }

    upload_table(table, tableName);
    unlock_table(tableName);
}

LinkedList<string> values_for_insert(LinkedList<string> commandList){   
    LinkedList<string> values;
    if (commandList.get(3) != "VALUES"){
        throw runtime_error("Syntax error in pasing values for INSERT");
    }
    for (int i = 4; i < commandList.size(); i++){
        values.addtail(commandList.get(i));
    }
    return values;
}

bool check_where(LinkedList<string> commandList){
    return commandList.search("WHERE");
}

void condition_insert(LinkedList<string> commandList){

    if (commandList.get(0) == "INSERT" && commandList.get(1) == "INTO" && check_where(commandList) == 0){
        LinkedList<string> values = values_for_insert(commandList);
        string name_table = commandList.get(2);
        INSERT(values, name_table);
    } else {
        throw runtime_error("Syntax error in input query!");
    }
}



//REALIZATION OF SELECT
LinkedList<string> get_table_for_select(LinkedList<string> commandList){
    LinkedList<string> selected;
    
    bool writeMode = 0;
    for (int i = 0; i < commandList.size(); i++){
        auto token = commandList.get(i);
        if (token == "FROM") break;
        if ( writeMode == 1) selected.addtail(token);
        if (token == "SELECT") writeMode = 1;
    }
    return selected;
}

bool check_cond_with_oper(string first_table, HASH<string> row1, 
                    string second_table, HASH<string> row2, 
                    LinkedList<string> conditions, LinkedList<string> operators){

    LinkedList<bool> results;

    for (int i = 0; i < conditions.size(); i += 3){
        string left = conditions.get(i);
        string op = conditions.get(i + 1);
        string right = conditions.get(i + 2);

        if (check_table(left) && op == "="){
            if (extract_table(left) ==  first_table){
                left = row1.HGET(get_column(left));
            }
            else if (extract_table(left) ==  second_table){
                left = row2.HGET(get_column(left));
            }
            else{
                throw runtime_error("Wrong table name used");
            }


            if (check_table(right)){
                if (extract_table(right) ==  first_table){
                    right = row1.HGET(get_column(left));
                }
                else if (extract_table(right) ==  second_table){
                    right = row2.HGET(get_column(right));
                }
                else{
                    throw runtime_error("Wrong table name used");
                }
            }

            results.addtail(left == right); 
        }
        else{
            throw runtime_error("Wrong syntax in chosen columns");
        }
    }

    return end_result(results, operators);

}

void condition_select(LinkedList<string> inputList, int client_socket){
    
    mutex SELECT_MUTEX;
    lock_guard<mutex> MUTEX_SELECT(SELECT_MUTEX);


    LinkedList<string> select_col_with_tables = get_table_for_select(inputList);
    LinkedList<string> select_tables = get_tables_before_where(inputList);

    LinkedList<string> conditions;
    LinkedList<string> operators;
    
    bool startWrite = 0;
    string element;
    for (int i = 0; i < inputList.size(); i++){
        element = inputList.get(i);
        if (startWrite){
            if (element == "OR" || element == "AND"){
                operators.addtail(element);
            }
            else{
                conditions.addtail(element);
            }
        }
        if (element == "WHERE") startWrite = 1;
    }

    if (select_tables.size() == 2 && select_col_with_tables.size() == 2){

        LinkedList<HASH<string>> table_fir = read_table(select_tables.get(0));
        LinkedList<HASH<string>> table_sec = read_table(select_tables.get(1));

        for (int i = 1; i < table_fir.size(); i++){

            HASH<string> row_first = table_fir.get(i);
            for (int j = 1; j < table_sec.size(); j++){

                HASH<string> row_second = table_sec.get(j);
                if (check_cond_with_oper(select_tables.get(0),row_first, select_tables.get(1), row_second, conditions, operators)){

                    cout << table_fir.get(i).HGET(get_column(select_col_with_tables.get(0))) << " " 
                    <<  table_sec.get(j).HGET(get_column(select_col_with_tables.get(1))) << endl;
                }
            }
        }
    }else{
        throw runtime_error("Wrong amount of tables chosen!");
    }
}



//REALIZATION OF DELETE
bool check_cond_without_oper(string fir_table, HASH<string> row, 
                    LinkedList<string> conditions, LinkedList<string> operators){

    LinkedList<bool> results;

    for (int i = 0; i < conditions.size(); i += 3){
        string left = conditions.get(i);
        string op = conditions.get(i + 1);
        string right = conditions.get(i + 2);

        if (check_table(left) && op == "=" && !check_table(right)){
            if (extract_table(left) ==  fir_table){
                left = row.HGET(get_column(left));
            }
            else{
                throw runtime_error("Wrong condition for delete");
            }

            results.addtail(left == right); 
        }
        else{
            throw runtime_error("Wrong syntax in delete");
        }
    }
    return end_result(results, operators);
}

void condition_delete(LinkedList<string> inputList){

    mutex DELETE_MUTEX;
    lock_guard<mutex> MUTEX_DELETE(DELETE_MUTEX);



    LinkedList<string> tables_selected = get_tables_before_where(inputList);
    string name_table = tables_selected.get(0);
    LinkedList<HASH<string>> table = read_table(name_table);

    lock_table(name_table);

    //obrabotka WHERE
    LinkedList<string> conditions;
    LinkedList<string> operators;
    bool start_write = 0;
    string element;

    for (int i = 0; i < inputList.size(); i++){
        element = inputList.get(i);
        if (start_write){
            if (element == "OR" || element == "AND"){
                operators.addtail(element);
            }
            else{
                conditions.addtail(element);
            }
        }
        if (element == "WHERE") start_write = 1;
        
    }

    LinkedList<HASH<string>> new_table;
    if (tables_selected.size() == 1){
        for(int i = 0; i < table.size(); i++){
            HASH<string> currentRow = table.get(i);
            if (!check_cond_without_oper(name_table, currentRow, conditions, operators)){
                new_table.addtail(currentRow);
            }
            else update_down_pksq(name_table);
        }

        upload_table(new_table, name_table);
    } 
    else{
        throw runtime_error("Wrong syntax in delete from table");
    }

    unlock_table(name_table);
}



void MENU(auto client_input, auto client_socket){

    mutex for_menu;

    try {

        LinkedList<string> inputList = parse_command(client_input);
        string fir_key_word = inputList.get(0);

        if (fir_key_word == "quit" || fir_key_word == "exit"){
            cout << "Waiting for next query" << endl;
            return;
        }

        if (fir_key_word == "INSERT"){
            condition_insert(inputList);
        } else if (fir_key_word == "DELETE"){
            condition_delete(inputList);
        } else if (fir_key_word == "SELECT"){
            condition_select(inputList, client_socket);
        } else  {
            throw runtime_error("Wrong fir_key_word called!");
        }    
    }
    catch (const runtime_error& e){
        lock_guard<mutex> lock(for_menu);
        cerr << "ERROR: " << e.what() << endl;
        string errorMessage = "ERROR: " + string(e.what());
        send(client_socket, errorMessage.c_str(), errorMessage.size(), 0);
    }
}



sockaddr_in server_initialization (){

    sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(7432);
    server_address.sin_addr.s_addr = INADDR_ANY;
    return server_address;
}



void request_client(int client_socket){

    char buffer[1024];
    int received_bytes_from_server = recv(client_socket, buffer, sizeof(buffer)-1, 0);

    mutex for_client;

    if(received_bytes_from_server  > 0){

        buffer[received_bytes_from_server] = '\0';

        {
            lock_guard<mutex> lock(for_client);
            cout << "Received message:" << buffer << "\n";
        }

        try{
            MENU(buffer, client_socket);
        }
        catch (const exception& e){
            cerr << "Error in client handling: " << e.what() << endl;
        }
    } 

    {
        lock_guard<mutex> lock(for_client);
        cout << "Client disconnected." << endl;
    }

    close(client_socket); // Закрываем клиентский сокет после обработки одного запроса
}






void listener_server(int server_socket){

    sockaddr_in client_address;
    socklen_t size_client_address = sizeof(client_address); // for accept

    mutex for_client;

    while(true){

        int client_socket = accept(server_socket, (sockaddr*)&client_address, &size_client_address);
        if(client_socket == -1){
            cerr << "The error occurred when connecting to the client!!!";
            continue;
        }

        {
            lock_guard<mutex> lg(for_client);
            cout << "Client connected!" << endl;
        }

        // для обеспечения многопоточности 
        thread client_thread(request_client, client_socket);
        client_thread.detach();
    }
}




void work_with_server(){

    int socket_server = socket(AF_INET, SOCK_STREAM,0);
    if(socket_server == -1){
        cerr << "Error creating socket";
        return;
    }


    sockaddr_in server_address = server_initialization();
    int bind_soc_with_port_and_ip = bind(socket_server, (sockaddr*)&server_address, sizeof(server_address));
    if(bind_soc_with_port_and_ip == -1){
        cout << "Error binding socket";
        return;
    }


    listen(socket_server, 3);
    cout << "The server was able to start and is now listening on port 7432" << endl;

    listener_server(socket_server);//  wait  входящих подключений


    close(socket_server);
}




int main(){
    setlocale(LC_ALL, "RU");
    createDataBase();
    work_with_server();

    return 0;
}
