#include <stdio.h>
#include "8b10b.h"

struct decode8b10b decodeLU;

void init8b10b(void)
{
    int i, c;
    unsigned char lu;

    for (i=0; i<64; i++)   //2^6 = 64
    {
        decodeLU.data6m[i] = -1;
        decodeLU.data6p[i] = -1;
        decodeLU.disp6[i] = disparity(i, 6);
    }

    for (i=0; i<16; i++)   //2^4 = 16
    {
        decodeLU.data4m[i] = -1;
        decodeLU.data4p[i] = -1;
        decodeLU.ctrl4m[i] = -1;
        decodeLU.ctrl4p[i] = -1;
        decodeLU.disp4[i] = disparity(i, 4);
    }

    for (i=0; i<32; i++)    //5 bit to encode
    {
        lu = data5b6b[i];
        decodeLU.data6m[RD_CODE6(lu, 0)] = i;  //RD=-1
        decodeLU.data6p[RD_CODE6(lu, 1)] = i;  //RD=+1
    }

    for (i=0; i<8; i++)    //3 bit to encode, plus alternate code
    {
        lu = data3b4b[i];
        printf("d4 lu: %03x\n", RD_CODE4(lu, 1));
        decodeLU.data4m[RD_CODE4(lu, 0)] = i;  //RD=-1
        decodeLU.data4p[RD_CODE4(lu, 1)] = i;  //RD=+1
    }

    for (i=0; i<8; i++)    //3 bit to encode
    {
        lu = ctrl3b4b[i];
        printf("c4 lu: %03x\n", RD_CODE4(lu, 1));
        decodeLU.ctrl4m[RD_CODE4(lu, 0)] = i;  //RD=-1
        decodeLU.ctrl4p[RD_CODE4(lu, 1)] = i;  //RD=+1
    }
}

/*
    8B10B encoding
    in     input buffer
    out    output buffer (10 bit words)
    length number of bytes to convert
    rdp    RD is +1 (1 RD=+1, 0 RD=-1)
    returns final rdp
*/
int
encodeData8b10b(short *out, char *in, int length, int rdp)
{
    unsigned char b, lu;
    int i, x, y, code6, code4;

    rdp &= 1; // this should be 0 or 1

    for (i=0; i < length; i++)
    {
        b = in[i];
        x = b & 0x1F;   //HGFEDCBA => EDCBA
        lu = data5b6b[x];
        code6 = RD_CODE6(lu, rdp); //EDCBA => abcdei
        rdp = RD_NEXT(lu, rdp);

        y = b >> 5;     //HGFEDBCA => HGF
        if (y==7 &&
           (rdp && (x == 11 || x == 13 || x == 14) ||  //RD=+1
           !rdp && (x == 17 || x == 18 || x == 20)))   //RD=-1
            lu = data3b4b[A7];  //use alternate code to prevent runs of 5
        else
            lu = data3b4b[y];

        code4 = RD_CODE4(lu, rdp);  //HGF => fghj
        rdp = RD_NEXT(lu, rdp);

        out[i] = code4 << 6 | code6;
    }
    return rdp;
}

int
encodeControl8b10b(short *out, int x, int y, int rdp)
{
    unsigned char lu;
    int code6, code4;

    rdp &= 1; // this should be 0 or 1
            // y=0 thru 7
            // x=23, 27, 28, 29, 30

    if (x == 28)
        lu = data5b6b[K28];
    else
        lu = data5b6b[x];
    code6 = RD_CODE6(lu, rdp);
    rdp = RD_NEXT(lu, rdp);

    lu = ctrl3b4b[y];
    code4 = RD_CODE4(lu, rdp);
    rdp = RD_NEXT(lu, rdp);

    *out = code4 << 6 | code6;

    return rdp;
}


int
decode8b10b(short *in, char *out, int length)
{
    // search D.x table for 6 bit code
    //   if K28,
    //      search control code table for K28.x.y
    // search D.x.y table for 4 bit code
    //   if D.x.A7 && D.x = 23 27 29 30
    //      search K.x.y table for control code
    int i, u, v;
    int xn, xp;
    int yn, yp;
    char buf[16];

    for (i = 0; i < length; i++)
    {
        u = in[i] & 0x3F;
        v = in[i] >> 6;
        xn = decodeLU.data6m[u];
        xp = decodeLU.data6p[u];
        yn = decodeLU.data4m[v];
        yp = decodeLU.data4p[v];
        printf("%s xn=%d xp=%d yn=%d yp=%d\n", itobr(buf, in[i], 10), xn, xp, yn, yp);
    }

    return 0;
}

int
bitreverse(int x, int width)
{
    int i, y=0;

    for (i=0; i < width; i++)
        y = (y << 1) | (x >> i & 1);

    return y;
}

/* int to binary string */
char *
itob(char *str, int x, int width)
{
    int i;
    char *s = str;

    for (i = width - 1; i >= 0; i--)
        *s++ = '0' + (x >> i & 1);
    *s = 0;

    return str;
}

/* int to binary string reversed */
char *
itobr(char *str, int x, int width)
{
    int i;
    char *s = str;

    for (i = 0; i < width; i++)
        *s++ = '0' + (x >> i & 1);
    *s = 0;

    return str;
}

int
disparity(int x, int width)
{
    int i, sum = 0;

    for (i = 0; i < width; i++)
    {
        if ((x >> i) & 1)
            sum += 1;
        else
            sum -= 1;
    }
    return sum;
}

void
dump10b(short *data, int length)
{
    int i;
    char buf[16];

    for (i=0; i < length; i++)
    {
        if (i%7==0)
            printf("\n");

        printf("%s ", itobr(buf, data[i], 10));
    }
    printf("\n");
}

/*
    dump RD (running disparity)
    rdp RD is +1 (1 RD=+1, 0 RD=-1)
    returns RD
*/
int
dumpRD10b(short *data, int length, int rdp)
{
    int i, u, v, rd;
    char buf[16];

    rd = ((rdp&1) << 1) - 1;

    for (i=0; i < length; i++)
    {
        u = data[i] & 0x3F;
        v = data[i] >> 6;

        if (i%3==0)
            printf("\n");
        printf("%03d:",i);

        rd += disparity(u,6);
        printf("%s:%+2d,", itobr(buf, u, 6), rd);

        rd += disparity(v,4);
        printf("%s:%+2d ", itobr(buf, v, 4), rd);
    }
    printf("\n");

    return rd;
}

void
dumpTable8b10b(char *table, int first, int last, char *label)
{
    int i;
    char e;
    int count = last-first+1;

    if (count != 1)
        printf("\n       RD=-1 RD=+1 T\n");

    for (i=first; i <= last; i++)
    {
        e = table[i];

        if (count == 1)
            printf("%6s ", label);
        else
            printf("%4s%02d ", label, i);

        printf("[%03o] [%03o] %d\n", RD_CODE6(e,0), RD_CODE6(e,1), RD_NEXT(e,0));
    }
}

void
dumpTable8b10bC(char *table, int length, char *label, int width)
{
    int i;
    int lu, a, t, cm, cp, code;
    char buf[16];

    printf("\n//RD=-1, RD=+1, T\n");
    for (i=0; i < length; i++)
    {
        lu = table[i];

        cm = RD_CODE6(lu, 0);
        cp = RD_CODE6(lu, 1);
        a = cm != cp;
        t = RD_NEXT(lu, 0);

        code = cm;
        code |= a << 6;
        code |= t << 7;

        printf("    %04o,  // %s%02d T=%d A=%d ",
                code, label, i, t, a);

        printf("%s", itobr(buf, cm, width));
        if (a)
            printf(", %s", itobr(buf, cp, width));

        printf("\n");
    }
}

void
dumpAllTables(void)
{
    dumpTable8b10b(data5b6b, 0, 31, "D.");
    dumpTable8b10b(data5b6b, K28, K28, "K.28");
    dumpTable8b10b(data3b4b, 0, 6, "D.x.");
    dumpTable8b10b(data3b4b, P7, P7, "D.x.P7");
    dumpTable8b10b(data3b4b, A7, A7, "D.x.A7");
    dumpTable8b10b(ctrl3b4b, 0, 7, "K.x.");
}

void
dumpAllTablesC(void)
{
    dumpTable8b10bC(data5b6b, 33, "D.", 6);
    dumpTable8b10bC(data3b4b, 9, "D.x.", 4);
    dumpTable8b10bC(ctrl3b4b, 8, "K.x.", 4);
}

//From tables at http://en.wikipedia.org/wiki/8b/10b_encoding

//Note: for 8bit byte to encode HGFEDCBA, bits are sent out as abcdeifghj
//      (the lsb first)
//      5b6b maps EDCBA to abcdei
//      3b4b maps HGF to fghj

char data5b6b[33] =
{
            //              RD=-1   RD=+1 (lsb first)
    0371,   // D.00 T=1 A=1 100111, 011000
    0356,   // D.01 T=1 A=1 011101, 100010
    0355,   // D.02 T=1 A=1 101101, 010010
    0043,   // D.03 T=0 A=0 110001
    0353,   // D.04 T=1 A=1 110101, 001010
    0045,   // D.05 T=0 A=0 101001
    0046,   // D.06 T=0 A=0 011001
    0107,   // D.07 T=0 A=1 111000, 000111
    0347,   // D.08 T=1 A=1 111001, 000110
    0051,   // D.09 T=0 A=0 100101
    0052,   // D.10 T=0 A=0 010101
    0013,   // D.11 T=0 A=0 110100
    0054,   // D.12 T=0 A=0 001101
    0015,   // D.13 T=0 A=0 101100
    0016,   // D.14 T=0 A=0 011100
    0372,   // D.15 T=1 A=1 010111, 101000
    0366,   // D.16 T=1 A=1 011011, 100100
    0061,   // D.17 T=0 A=0 100011
    0062,   // D.18 T=0 A=0 010011
    0023,   // D.19 T=0 A=0 110010
    0064,   // D.20 T=0 A=0 001011
    0025,   // D.21 T=0 A=0 101010
    0026,   // D.22 T=0 A=0 011010
    0327,   // D.23 T=1 A=1 111010, 000101
    0363,   // D.24 T=1 A=1 110011, 001100
    0031,   // D.25 T=0 A=0 100110
    0032,   // D.26 T=0 A=0 010110
    0333,   // D.27 T=1 A=1 110110, 001001
    0034,   // D.28 T=0 A=0 001110
    0335,   // D.29 T=1 A=1 101110, 010001
    0336,   // D.30 T=1 A=1 011110, 100001
    0365,   // D.31 T=1 A=1 101011, 010100
    0374    // K.28 T=1 A=1 001111, 110000
};

char data3b4b[9] =
{
            //                RD=-1 RD=+1 (lsb first)
    0315,   // D.x.00 T=1 A=1 1011, 0100
    0011,   // D.x.01 T=0 A=0 1001
    0012,   // D.x.02 T=0 A=0 0101
    0103,   // D.x.03 T=0 A=1 1100, 0011
    0313,   // D.x.04 T=1 A=1 1101, 0010
    0005,   // D.x.05 T=0 A=0 1010
    0006,   // D.x.06 T=0 A=0 0110
    0307,   // D.x.P7 T=1 A=1 1110, 0001
    0316,   // D.x.A7 T=1 A=1 0111, 1000
};

char ctrl3b4b[8] =
{
            //                RD=-1 RD=+1 (lsb first)
    0315,   // K.x.00 T=1 A=1 1011, 0100
    0106,   // K.x.01 T=0 A=1 0110, 1001
    0105,   // K.x.02 T=0 A=1 1010, 0101
    0103,   // K.x.03 T=0 A=1 1100, 0011
    0313,   // K.x.04 T=1 A=1 1101, 0010
    0112,   // K.x.05 T=0 A=1 0101, 1010
    0111,   // K.x.06 T=0 A=1 1001, 0110
    0316,   // K.x.07 T=1 A=1 0111, 1000
};
