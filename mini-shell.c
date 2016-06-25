#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>  /* pid_t */
#include <sys/wait.h>   /* waitpid */
#include <unistd.h>     /* exit, fork */

#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <algorithm>



std::vector<std::string> explode(std::string const & s, char delim)
{
    std::vector<std::string> result;
    std::istringstream iss(s);

    for ( std::string token; std::getline(iss, token, delim); )
        result.push_back(std::move(token));

    return result;
}


// char* string_to_char_array(char** arg, std::string s)
// {
//     strcpy(*arg, s.c_str());
//     return *arg;
// }

void error_handler(char *e)
{
    perror(e);
    exit(EXIT_FAILURE);
}

std::vector<int> find_pipes( std::vector<std::string> ins )
{
    std::vector<int> pipe_pos;
    pipe_pos.push_back(-1);
    for (uint i = 0; i < ins.size(); ++i)
        if (ins.at(i) == "|") pipe_pos.push_back(i);
    pipe_pos.push_back(ins.size());
    return pipe_pos;
}


int run(std::string in)
{

    std::vector<std::string> ins = explode(in,' ');
    int pipes = std::count(ins.begin(), ins.end(), "|");
    std::vector<int> pipe_pos = find_pipes(ins);

    char* args[ins.size() + 1];
    // REFERENCE ARRAY[i] TO EACH STRING
    for (uint i = 0; i < ins.size(); ++i)
        args[i] = &(ins[i][0]);
    args[ins.size()] = '\0';

    int pipeline[pipes+2][2]; // PIPELINE[0] NOT CURRENTLY IN USE :P
    // close(pipeline[0][0]);
    // close(pipeline[0][1]);

    for (int i = 0; i < pipes+2; ++i)
    {
        if ( pipe(pipeline[i]) == -1 )
            error_handler("at pipe()");
    }

    for (int i = 1; i < pipes+2; ++i)
    {

        pid_t pid = fork();
        if ( pid == -1 )
            error_handler("at fork()");

        if ( pid == 0 )
        {
            // CLOSE PIPES THAT DOESN'T BELONG
            for (int j = 0; j < pipes+2; ++j)
            {
                if (i!=j)
                {
                    if ( close(pipeline[j][0]) == -1 )
                        error_handler("at close()");
                    if ( close(pipeline[(j+1)%(pipes+2)][1]) )
                        error_handler("at close()");
                }
            }

            dup2(pipeline[i][0],0); // REFERENCE STDIN TO READ END OF PIPE
            // dup2(pipeline[(i+1)%(pipes+2)][1],1); // REFERENCE STDOUT TO WRITE END OF NEXT PROCESS'S PIPE
            if (i!=pipes+1)
                dup2(pipeline[(i+1)][1],1); // REFERENCE STDOUT TO WRITE END OF NEXT PROCESS'S PIPE

            char* buf[pipe_pos[i]-pipe_pos[i-1]]; // ARRAY OF THE SIZE OF COMMAND + ARGS + \0
            memcpy(buf , &args[pipe_pos[i-1]+1] , 8*(pipe_pos[i]-pipe_pos[i-1]+1)); // MAGIA
            buf[pipe_pos[i]-pipe_pos[i-1]-1] = '\0';
            // char* buf[ins.size() + 1];
            // read(pipeline[i][0], buf, sizeof(buf));
            
            if ( execvp(buf[0], buf) == -1 ){
                std::cout << "\n";
                error_handler(*buf);
            }
        }
        else
        {

            // int status;
            // int a = waitpid(pid,&status,0);
            // printf("%d\n%d\n",status,a );
            // write(pipeline[1][1], args, sizeof(args));
            // if (i == 2)
            // {
            // char buf[102400];
            // read(pipeline[0][0], buf, 100000); // While read > 0 para cosas que escriben varias veces (ej. ping)
            // std::cout << buf;
            // memset(buf, 0, sizeof buf);
            // }
        }
    }

    // CLOSE PARENT'S PIPES
    for (int j = 0; j < pipes+2; ++j)
    {
        if ( close(pipeline[j][0]) == -1 )
            error_handler("at close()");
        if ( close(pipeline[(j+1)%(pipes+2)][1]) )
            error_handler("at close()");
    }

    // WAIT FOR ALL MY CHILDREN
    for (int i = 1; i < pipes+2; ++i)
        wait(NULL); // TODO: waitpid

    return 0;

}


std::string prompt()
{
    std::string in("");

    while (1)
    {
        while ( in == "\0" )
        {
            std::cout << "[Mini-shell]$ ";
            std::getline(std::cin,in);
        } 

        run(in);
        in = "";
    }
    
}


int main(int argc, char* argv[])
{
    return run(prompt());
}

