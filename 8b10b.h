// 010101 1110 110100 1000 001101 1110
// D.10.P7     D.11.A7     D.12.P7   
//?    -    +     +     -    -     +
//input[0] = 3;   //D.03.0
//input[1] = 224; //D.11.7
//input[2] = 225; //D.12.7

#define P7 7
#define A7 8
#define K28 32

extern char data5b6b[];
extern char data3b4b[];
extern char ctrl3b4b[];

struct decode8b10b
{
    signed char disp6[64];
    signed char data6m[64]; //D.x
    signed char data6p[64];
    
    signed char disp4[16];

    signed char data4m[16]; //D.x.y
    signed char data4p[16];

    signed char ctrl4m[16]; //K.x.y
    signed char ctrl4p[16];        
};

extern struct decode8b10b decodeLU;

/* RD_CODE If RD=-1 (rdp==0) then use code (lower 6 bits of x)
           If Rd=+1 (rdp==1) then see if alternate code bit set
           if so, the code is the complement (ones) of lower 6 bits of x*/
#define RD_CODE4(x,rdp) ((rdp)==0 || !((x) >> 6 & 1) ? (x) & 0xF :~(x) & 0xF)
#define RD_CODE6(x,rdp) ((rdp)==0 || !((x) >> 6 & 1) ? (x) & 0x3F :~(x) & 0x3F)
/* RD_NEXT next value of rdp based on code and current rdp */
#define RD_NEXT(x,rdp) (((x) >> 7 & 1) ^ rdp)

void dumpTable8b10b(char *table, int first, int last, char *label);
void dumpTable8b10bC(char *table, int length, char *label, int width);
void dumpAllTables(void);
void dumpAllTablesC(void);

int  encodeData8b10b(short *out, char *in, int length, int rd);
int  encodeControl8b10b(short *out, int x, int y, int rd);
void dump10b(short *data, int length);
int  dumpRD10b(short *data, int length, int rd);

int  disparity(int x, int width);
int  bitreverse(int x, int width);
char *itob(char *str, int x, int width);
char *itobr(char *str, int x, int width);