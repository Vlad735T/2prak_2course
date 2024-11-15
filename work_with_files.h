#pragma once
#include <filesystem>
#include <string>
#include <cjson/cJSON.h>
#include "headers/LinkedList.h"

void update_up_pksq(const string& tableName){

    string way_to_dir = filesystem::current_path();
    way_to_dir += "/" + tableName;

    ifstream take_pksq(way_to_dir + "/" + tableName + "_pk_sequence"); 
    if (!take_pksq.is_open()){
        throw runtime_error("Error opening pk_sequence and reading it");
    }

    string used_line;
    getline(take_pksq, used_line);
    take_pksq.close();

    int update_use_line = stoi(used_line) + 1;

    ofstream upload_pkseq(way_to_dir + "/" + tableName + "_pk_sequence");
    upload_pkseq << update_use_line;
    upload_pkseq.close();
}

void update_down_pksq(const string& tableName){

    string way_to_dir = filesystem::current_path(); 
    way_to_dir += "/" + tableName;

    ifstream take_pksq(way_to_dir + "/" + tableName + "_pk_sequence"); 
    if (!take_pksq.is_open()){
        throw runtime_error("Error opening pk_sequence and reading it");
    }

    string used_line;
    getline(take_pksq, used_line);
    take_pksq.close();

    int decreasedLinesAmount = stoi(used_line) - 1;

    ofstream upload_pkseq(way_to_dir + "/" + tableName + "_pk_sequence");
    upload_pkseq << decreasedLinesAmount;
    upload_pkseq.close();
}

int getPKSEQ(string tableName){

    string pathToDir = filesystem::current_path(); 
    pathToDir += "/" + tableName;

    string fileInput;
    
    ifstream PKSEQ(pathToDir + "/" + tableName + "_pk_sequence");
    if (!PKSEQ.is_open()){
        throw runtime_error("Error opening pk_sequence and reading it");
    }
    getline(PKSEQ, fileInput);
    PKSEQ.close();

    return stoi(fileInput);
}




LinkedList<string> get_columns_from_file(string tableName){

    string way_dir = filesystem::current_path();
    way_dir += "/" + tableName;

    ifstream col_names(way_dir + "/1.csv");
    string fileInput;
    getline(col_names, fileInput, '\n');
    
    LinkedList<string> columnNames;
    string word = "";
    for (auto symbol : fileInput) {
        if (symbol == ','){
            columnNames.addtail(word);
            word = "";
            continue;
        }
        word += symbol;
    }
    if (!word.empty()) columnNames.addtail(word);
    col_names.close();

    return columnNames;
}

string last_name_aft_slash(const string& path) {

    size_t last_pos_slash = path.rfind('/');

    if (last_pos_slash != std::string::npos) {
        return path.substr(last_pos_slash + 1);
    }
    
    return path;
}




void unlock_table(const string& pathToDir){
    
    string tableName = last_name_aft_slash(pathToDir);
    ofstream lockFile(pathToDir + "/" + tableName + "_lock");
    lockFile << 0;
    lockFile.close();
}

void lock_table(const string& pathToDir){
    
    string tableName = last_name_aft_slash(pathToDir);
    ofstream lockFile(pathToDir + "/" + tableName + "_lock");
    lockFile << 1;
    lockFile.close();
}



string readJSON(const string& fileName) { 
    fstream file(fileName);
    if (!file.is_open()){
        throw runtime_error("Error opening " + fileName + ".json file!");
    } 

    stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    return buffer.str();
}


bool create_directory(const string& name_of_data_base){
    string path_to_dir = filesystem::current_path();
    path_to_dir += "/" + name_of_data_base;
    if (filesystem::create_directory(path_to_dir)) return true;
    else return false;
}

void create_files_in_subFolder(const cJSON* table, const cJSON* structure, const string& sub_name){
    
    LinkedList<string> column_names; 
    cJSON* arr_in_tablica = cJSON_GetObjectItem(structure, table->string); 
    int size_arr = cJSON_GetArraySize(arr_in_tablica);

    for (size_t i = 0; i < size_arr; i++) {
        cJSON* name_column = cJSON_GetArrayItem(arr_in_tablica, i);
        column_names.addtail(name_column->valuestring);
    }

    string path_to_directory = filesystem::current_path();
    path_to_directory += sub_name;
    
    ofstream CSV(path_to_directory + "/1.csv"); 
    for (size_t i = 0; i < column_names.size(); i++){   

        if (i < column_names.size()-1){
            CSV << column_names.get(i) << ",";
        } else {
            CSV << column_names.get(i);
        }
        
    }
    CSV << "\n";
    CSV.close();

    string path_to_PQ = path_to_directory + "/" + table->string + "_pk_sequence";
    ofstream PKSEQ(path_to_PQ); 
    PKSEQ << "1";
    PKSEQ.close();

    unlock_table(path_to_directory);
}

void createDataBase(){   

    LinkedList<string> way_to_table;
    string content_json = readJSON("schema.json"); 

    if (content_json.empty()){
        throw runtime_error("Error reading schema, content is empty!");
    }

    cJSON* json = cJSON_Parse(content_json.c_str());

    if (json == nullptr){
        throw runtime_error("Error parsing schema file!");
    }

    cJSON* name_schema = cJSON_GetObjectItem(json, "name"); 
    
    //valuestring for dostupa to str value
    string nameDataBase = name_schema->valuestring; // name of data base

    fstream checkDB("check_data_base");
    if (checkDB.is_open()){
        string path = filesystem::current_path(); // save put' where exists our program
        filesystem::current_path(path + "/" + nameDataBase);
        checkDB.close();
        return;
    }

    ofstream DataBaseFlag("check_data_base"); 
    DataBaseFlag.close();

    cJSON* limit = cJSON_GetObjectItem(json, "tuples_limit"); 
    int tuplesLimit = limit->valueint;

    create_directory(nameDataBase); 

    string path = filesystem::current_path();
    filesystem::current_path(path + "/" + nameDataBase);
    cJSON* structure = cJSON_GetObjectItem(json,"structure"); 

//create tablica
    for (cJSON* table = structure->child; table != nullptr; table = table->next) {
        string sub_folder_path = filesystem::current_path();
        string name_table = table->string;
        sub_folder_path =  "/" + name_table;
        way_to_table.addhead(sub_folder_path);

        create_directory(sub_folder_path);
        create_files_in_subFolder(table, structure, sub_folder_path);
    }
    cJSON_Delete(json);
}

