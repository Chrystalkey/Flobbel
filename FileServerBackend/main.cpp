#include <iostream>
#include <string.h>
#include <fstream>

void process_arguments(int,char**);

int main(int argc, char **argv){
    process_arguments(argc, argv);
    return 0;
}

void process_arguments(int argc, char **argv){
    if(argc < 2){
        std::cout << "ERROR: No specific target directory given\n";
        exit(1);
    }
    if(strcmp(argv[1],"--help")){
        std::cout << "--help prints this text\n"
                  << "<STRING> sets the save directory\n";
    }

    std::ofstream testfile(std::string(argv[1])+"/test.test");
    if(testfile.is_open()){
        std::cout << "Congrats, you were able to write down a writable directory (:\n";
        testfile.close();
        remove((std::string(argv[1])+"/test.test").c_str());
    }else{
        std::cout << "ERROR: No writable directory specified. Please try again later\n";
        exit(2);
    }
}