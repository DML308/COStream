#ifndef FILEGENERATEHELPER_H
#define FILEGENERATEHELPER_H

#include <string>
using namespace std;

class FileGenerateHelper
{
public:
    static void CopyRuntimeFile(const string& destDir)
    {
        // add by mobinsheng begin
        // string temp_path = output_file;

        string pathvar;
        pathvar = getenv("COSTREAM_LIB");
        if(pathvar.size() == 0)
        {
            printf("The environment variable COSTREAM_LIB is not set! Please Set! COStreamC exited\n");
            exit(-1);
        }
        printf("COSTREAM_LIB=%s\n",pathvar.c_str());

        string baseCmd = "cp " + pathvar + "/";
        string cmd = baseCmd + "runtime/X86Lib2/*.h " + destDir;
        system(cmd.c_str());
        cmd = baseCmd + "runtime/X86Lib2/*.cpp " + destDir;
        system(cmd.c_str());
        cmd = baseCmd + "runtime/X86Lib2/noCheckBoundary/*.h " + destDir;
        system(cmd.c_str());

        // add by mobinsheng end
    }

private:
};

#endif // FILEGENERATEHELPER_H
