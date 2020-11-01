#include <stdio.h>
#include "8b10b.h"

#define LENGTH 256
char input[LENGTH];
short output[LENGTH];
    
int 
main(void)
{   
    int i;
    int rdp = 1;  //RD=-1
    char buf[32];    
    
    init8b10b();
                
    for (i = 0; i < LENGTH; i++)
    {             
        input[i] = i;
    } 
                
    encodeData8b10b(output, input, LENGTH, rdp);
    //encodeControl8b10b(output, 28, 1, rdp);
    //dump10b(output, LENGTH);    
    //dumpRD10b(output, LENGTH, rdp);
    //dumpAllTables();
    dumpAllTablesC();
        
    // 46 unique codes used for D.x
    // 2 codes used for K.28           
    decode8b10b(output, input, 16);
    printf("%s\n", itobr(buf, *output, 10));
        
        
    return 0;
}









