//
// Created by maomao on 2021/2/2.
//

int CalCRc(int crc,const char *buf,int len)
{
    unsigned int byte;
    unsigned char k;
    unsigned short ACC,TOPBIT;
    unsigned short remainder=crc;
    TOPBIT = 0x8000;
    for (byte = 0; byte < len; ++byte) {
        ACC=buf[byte];
        remainder ^= (ACC << 8);
        for (k = 8; k >0 ; --k) {
            if (remainder & TOPBIT)
            {
                remainder=(remainder << 1) ^0x8005;
            }
            else
            {
                remainder=(remainder << 1);
            }
        }
    }
    remainder =remainder ^0x0000;
    return remainder;
}
