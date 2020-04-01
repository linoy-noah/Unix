//
// Created by linux on 1/21/20.
//



#include "shell.h"
ostream& operator<<(ostream& os,tasks& tas){
    cout<<tas.path<<":"<<tas.pid<<endl;
}
tasks::tasks() {
 parms=NULL;
}
tasks::~tasks(){
    if(parms){
        char** curr=parms;
        char** temp=curr;
        ++temp;
       /* while(curr){
            delete[](*curr);
            curr=temp;
            temp++;
        }*/
        delete[](parms);
    }
}
shell::shell(){
    list=NULL;
   curr_his=0;
   for(;;) {
       command();
   }
}
bool shell::add_to_list(tasks* t){
    if(NULL==t){
        return false;
    }
    t->next=list;
    list=t;
    return true;
}
bool shell::add_to_Hlist(string t){
   history[curr_his%200]=t;
   curr_his=(curr_his+1)%200;
    return true;
}
bool shell::remove_from_list(int p_id){
    bool found= false;
    tasks* curr=NULL;
    tasks* temp=NULL;
    tasks* prev=list;
    for(curr=list;curr!=NULL&&found!=true;curr=temp ){
        temp=curr->next;
        if(curr->pid==p_id){
            found=true;
           // int st;
            waitpid(p_id,NULL,0);
            if(curr!=list){
                prev->next=temp;
            }
            else{
                list=list->next;
            }
            delete (curr);
        }
        if(curr!=list) {
            prev = curr;
        }
    }
    return found;

}
bool shell::print_env(){
    int i;
    for(i=0;environ[i]!=NULL;++i){
        cout<<environ[i]<<endl;
    }
    return true;
}

bool shell::command(string t){
    cout<<">";
    if(t != ""){
       cout<<t;
    }
    else{
        getline (cin,t);
    }
    cout<<endl;

    string temp="";

    if(t=="tasks"){
        print_tasks();
    }
    if(t=="show_history"){
        show_history();
        //return true;
    }
    if(t=="print_env"){
        this->print_env();
    }
    if(t[0]=='!'){
        string a="!";
        char* z;
        string path= strtok_r(const_cast<char*>(t.c_str()),a.c_str(),&z);
        int num = atoi(z);
        run_history(num);
    }
    if(t[0]=='/') {
        add_to_Hlist(t);
        string a = " ";
        char *z;
        const char *path = strtok_r(const_cast<char *>(t.c_str()), a.c_str(), &z);
        string rest = z;
        size_t output_pos = string::npos, input_pos = string::npos, amper_pos = string::npos;
        string ampers = "&";
        string op = ">";
        string ip = "<";

        //cout<<"res before > check: "<< rest<<endl;
        output_pos = rest.find(op);
        //cout<<"res after > check: "<< rest<<endl;
        input_pos = rest.find(ip);
        //cout<<"res after < check: "<< rest<<endl;
        amper_pos = rest.find(ampers);
        //cout<<"res after & check: "<< rest<<endl;

        int count = 0;
        char **parms = NULL;
        if (rest != "") {
            count = 1;
            size_t f;


            for (f = rest.find(' ');
                 rest[f + 1] != '&' && rest[f + 1] != '>' && rest[f + 1] != '<' && f != string::npos; count++) {


                f = rest.find(' ', f + 1);
            }
            int r = 0;
            if (input_pos != string::npos || output_pos != string::npos || amper_pos != string::npos) {
                r++;
            }
            if (r == 0 || count < 1) {
                parms = new char *[count + 1];

                parms[0] = (char *) path;
                int i = 1;
                while (i < 1 + count) {
                    parms[i] = strtok_r(z, a.c_str(), &z);
                    i++;
                }
            } else {
                parms = new char *[2];
                parms[0] = (char *) path;
            }
        } else {
            parms = new char *[2];
            parms[0] = (char *) path;
        }

        /////// NEW ///////
        // void  execute(char **argv)
        //{
/*            pid_t  pid;
            int    status;

            if ((pid = fork()) < 0) {      fork a child process
                printf("*** ERROR: forking child process failed\n");
                exit(1);
            }
            else if (pid == 0) {           for the child process:
                if (execv(t[0], t) < 0) {      execute the command
                    printf("*** ERROR: exec failed\n");
                    exit(1);
                }
            }
            else {                                   for the parent:
                while (wait(&status) != pid) {}       wait for completion
            }*/
        // }
        //////// END-NEW ////////////
        pid_t pid, w;
        int c1 = 0;
        int c2 = 0;
        if (output_pos != string::npos || input_pos != string::npos) {
            string o, i;
            int out, in;
            if (input_pos != string::npos && output_pos != string::npos) {
                c1 = 18;
                c2 = 19;
                if (input_pos < output_pos) {
                    i = rest.substr(input_pos+1, output_pos - 2);
                    o = rest.substr(output_pos+1);
                    out = open(o.c_str(), O_WRONLY | O_CREAT | O_TRUNC);
                    in = open(i.c_str(), O_RDONLY);
                    dup2(0, 18);
                    dup2(1, 19);
                    dup2(out, 1);
                    dup2(in, 0);
                } else {
                    o = rest.substr(output_pos+1, input_pos - 2);
                    i = rest.substr(input_pos+1);
                    out = open(o.c_str(), O_WRONLY | O_CREAT | O_TRUNC);
                    in = open(i.c_str(), O_RDONLY);
                    dup2(0, 18);
                    dup2(1, 19);
                    dup2(out, 1);
                    dup2(in, 0);
                }


            } else {
                if (output_pos != string::npos) {
                    o = rest.substr(output_pos+1);
                    out = open(o.c_str(), O_WRONLY | O_CREAT | O_TRUNC);
                    // dup2(0,18);
                    dup2(1, 19);
                    dup2(out, 1);
                    // c1=18;
                    c2 = 19;
                }
                if (input_pos != string::npos) {
                    i = rest.substr(input_pos+1);
                    in = open(i.c_str(), O_RDONLY);
                    dup2(0, 18);
                    // dup2(1,19);
                    dup2(in, 0);
                    c1 = 18;
                    // c2=19;
                }
            }
        }
        pid = fork();
        if (pid == 0) {
            execve(path, parms, NULL);
            //if(execvp(reinterpret_cast<const char *>(*path), parms)<0){
            //perror("execve");
            exit(1);

            exit(0);
        }
        if (c1 != 0) {
            dup2(18, 0);
        }
        if (c2 != 0) {
            dup2(19, 1);
        }

        if (pid > 0 && amper_pos == string::npos) {
            int status;
            w = waitpid(pid, &status, 0);
            if (w < 0) {
                perror("waitpid");
                exit(1);
            }
        }
        if (pid > 0 && amper_pos != std::string::npos) {
            tasks *temp = new tasks();
            temp->path = path;
            temp->parms = parms;
            temp->pid = pid;
            add_to_list(temp);

        }
    }
    size_t found;
    found= t.find("return");
    if(found!=std::string::npos){
        string a=" ";
        char* z;
        string path= strtok_r(const_cast<char*>(t.c_str()),a.c_str(),&z);
        int num = atoi(z);
        remove_from_list(num);
    }
    found= t.find("=");
    if(found!=std::string::npos){
        add_env(t);
    }


    ///////////////

}

bool shell::print_tasks(){
    tasks* curr = NULL;
    for(curr=list;curr!=NULL;curr=curr->next){
        //cout<<curr<<endl;
        cout<<curr->path<<":"<<curr->pid<<endl;
    }
    return true;
}
/////----------NEW-------------/////
bool shell::add_env(string& env){

    if(putenv(const_cast<char*>(env.c_str()))) {
        cerr << strerror(errno) << "set env" << endl;
        return false;
    }
    else
        return true;
}
/////---------END-NEW----------/////
bool shell::show_history(){
    int i=0;
    while(i<curr_his){
        cout<<i+1<<") "<< history[i]<<endl;
        ++i;
    }
    return true;
}
bool shell::run_history(int num) {
    if (num > curr_his) { return false; }
    command(history[num]);////
    return true;
}
shell::~shell(){
    tasks* curr=NULL;
    tasks* temp=NULL;
    for(curr=list;curr!=NULL;curr=temp){
        temp=curr->next;
        delete(curr);
    }

}