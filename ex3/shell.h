//
// Created by linux on 1/21/20.
//

#ifndef EX3_SHELL_H
#define EX3_SHELL_H
#include <iostream>
#include <string.h>
#include <string>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

extern char** environ;

using namespace std;

class tasks{
public:
    int pid;
    string path;
    tasks* next;
    char** parms;
    tasks();
    ~tasks();
};
ostream& operator<<(ostream& os,tasks& tas);

class shell {
private:
    tasks* list;
    string history[200];
    int curr_his;
public:
    shell();
    bool add_to_Hlist(string t);
    bool add_to_list(tasks* t);
    bool remove_from_list(int p_id);
    bool command(string t="");
    bool print_tasks();
    bool print_env();
    bool add_env(string& env);
    bool show_history();
    bool run_history(int num);
    ~shell();
};


#endif //EX3_SHELL_H
