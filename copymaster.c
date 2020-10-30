#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>
#include <sys/types.h>
#include <stdbool.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "options.h"

void PrintCopymasterOptions(struct CopymasterOptions* cpm_options);

int getInfileSize(struct CopymasterOptions cpm_options);

void FatalError(char c, const char* msg, int exit_status);

bool fileExists(int fileSize);

int getOutfileSize(struct CopymasterOptions cpm_options);

bool controlCopyRegular(struct CopymasterOptions cpm_options);

bool openfailure(int prvy, int druhy);


int main(int argc, char* argv[]) {

    
struct CopymasterOptions cpm_options = ParseCopymasterOptions(argc, argv);

    int c = 0;

    PrintCopymasterOptions(&cpm_options);
    

    if (controlCopyRegular(cpm_options)){

	int velkost = getInfileSize(cpm_options);

    if (!fileExists(velkost))
        FatalError('B', "SUBOR NEEXISTUJE", 21);

    struct stat a;

    stat(cpm_options.infile, &a);

    mode_t infileMode = a.st_mode;

    int prvy = open(cpm_options.infile, O_RDONLY);

    int druhy = open(cpm_options.outfile, O_WRONLY | O_CREAT, infileMode);

    if (openfailure(prvy, druhy))
        FatalError('B', "INA CHYBA", 21);

    char buff[velkost];

    if (read(prvy, &buff, velkost) == -1 ||  write(druhy, &buff, velkost) == -1)
        FatalError('B', "INA CHYBA", 21);

    close(prvy);
    close(druhy);
	}
    if (cpm_options.fast){
        int prvy = open(cpm_options.infile, O_RDONLY);
    int druhy = open(cpm_options.outfile, O_WRONLY);

    if (openfailure(prvy, druhy))
	{
        FatalError('f', "INA CHYBA", -1);
    }

    int velkost = getInfileSize(cpm_options);
    char buff[velkost];

    if (read(prvy, &buff, velkost) == -1 ||  write(druhy, &buff, velkost) == -1)
	{
        FatalError('f', "INA CHYBA", -1);
    }
    
    close(prvy);
    close(druhy);
    }
    else if (cpm_options.slow)
	{
    int prvy = open(cpm_options.infile, O_RDONLY),
    druhy = open(cpm_options.outfile, O_WRONLY);

    if (openfailure(prvy, druhy))
	{

    FatalError('s', "INA CHYBA", -1);
	}
    int i;
    int velkost = getInfileSize(cpm_options);
    char buff[velkost];

    while((i = read(prvy, &buff, 1)))
	{
        if (i == -1 || write(druhy, &buff, 1) == -1){
        FatalError('s', "INA CHYBA", -1);
        }
    }
    
    close(prvy);
    close(druhy);
    ;
    }
    else if (cpm_options.create)
	 { 
	 mode_t create_mode = cpm_options.create_mode;

    if (create_mode < 1 || create_mode > 777)
	{
        FatalError('c', "ZLE PRAVA", 23);
    }

    if (fileExists(getOutfileSize(cpm_options)))
	{
        FatalError('c', "SUBOR EXISTUJE", 23);
    }
    
    int prvy = open(cpm_options.infile, O_RDONLY),
    druhy = open(cpm_options.outfile, O_WRONLY | O_CREAT, create_mode);

    if (openfailure(prvy, druhy))
	{
        FatalError('c', "INA CHYBA", 23);
    }    
    int velkost = getInfileSize(cpm_options);
    char buff[velkost];

    if (read(prvy, &buff, velkost) == -1 || write(druhy, &buff, velkost) == -1)
	{
        FatalError('c', "INA CHYBA", 23);
    }

    close(prvy);
    close(druhy);
    }
    if (cpm_options.fast && cpm_options.slow && c == 0) {
        fprintf(stderr, "CHYBA PREPINACOV\n"); 
        exit(EXIT_FAILURE);
    }
    return 0;
}


void FatalError(char c, const char* msg, int exit_status)
{
    fprintf(stderr, "%c:%d\n", c, errno); 
    fprintf(stderr, "%c:%s\n", c, strerror(errno));
    fprintf(stderr, "%c:%s\n", c, msg);
    exit(exit_status);
}


void PrintCopymasterOptions(struct CopymasterOptions* cpm_options)
{
    if (cpm_options == 0)
        return;
    
    printf("infile:        %s\n", cpm_options->infile);
    printf("outfile:       %s\n", cpm_options->outfile);
    
    printf("fast:          %d\n", cpm_options->fast);
    printf("slow:          %d\n", cpm_options->slow);
    printf("create:        %d\n", cpm_options->create);
    printf("create_mode:   %o\n", (unsigned int)cpm_options->create_mode);
    printf("overwrite:     %d\n", cpm_options->overwrite);
    printf("append:        %d\n", cpm_options->append);
    printf("lseek:         %d\n", cpm_options->lseek);
    
    printf("lseek_options.x:    %d\n", cpm_options->lseek_options.x);
    printf("lseek_options.pos1: %ld\n", cpm_options->lseek_options.pos1);
    printf("lseek_options.pos2: %ld\n", cpm_options->lseek_options.pos2);
    printf("lseek_options.num:  %lu\n", cpm_options->lseek_options.num);
    
    printf("directory:     %d\n", cpm_options->directory);
    printf("delete_opt:    %d\n", cpm_options->delete_opt);
    printf("chmod:         %d\n", cpm_options->chmod);
    printf("chmod_mode:    %o\n", (unsigned int)cpm_options->chmod_mode);
    printf("inode:         %d\n", cpm_options->inode);
    printf("inode_number:  %lu\n", cpm_options->inode_number);
    
    printf("umask:\t%d\n", cpm_options->umask);
    for(unsigned int i=0; i<kUMASK_OPTIONS_MAX_SZ; ++i) {
        if (cpm_options->umask_options[i][0] == 0) {
            // dosli sme na koniec zoznamu nastaveni umask
            break;
        }
        printf("umask_options[%u]: %s\n", i, cpm_options->umask_options[i]);
    }
    
    printf("link:          %d\n", cpm_options->link);
    printf("truncate:      %d\n", cpm_options->truncate);
    printf("truncate_size: %ld\n", cpm_options->truncate_size);
    printf("sparse:        %d\n", cpm_options->sparse);
}

bool openfailure(int prvy, int druhy) {

    return prvy == -1 || druhy == -2;
}

int getInfileSize(struct CopymasterOptions cpm_options) {
    struct stat a;
    stat(cpm_options.infile, &a);
    int velkost = a.st_size;
    return velkost;
}

int getOutfileSize(struct CopymasterOptions cpm_options) {
    struct stat st;
    stat(cpm_options.outfile, &st);
    int size = st.st_size;
    return size;
}

bool controlCopyRegular(struct CopymasterOptions cpm_options) {

    return !cpm_options.fast && !cpm_options.slow && !cpm_options.create &&
    !cpm_options.overwrite && !cpm_options.append && !cpm_options.lseek &&
    !cpm_options.directory && !cpm_options.delete_opt && !cpm_options.chmod &&
    !cpm_options.inode && !cpm_options.umask && !cpm_options.link &&
    !cpm_options.truncate && !cpm_options.sparse;
}

bool fileExists(int fileSize) {
    return fileSize > 0;
}

