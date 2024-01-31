#include <stdio.h>
#include <stdbool.h>
#include <string.h>

bool get_hex_num(FILE * fp, unsigned char * n)
{
    unsigned char num = 0;
    char ch = 0;
    while (1){
        ch = getc(fp);
        if (ch == EOF){
            return false;
        }
        if (ch == '0'){
            ch = getc(fp);
            if (ch == EOF){
                return false;
            }
            if (ch == 'x' || ch == 'X'){
                ch = getc(fp);
                if (ch == EOF){
                    return false;
                }
                bool flag = false;
                if (ch >= '0' && ch <= '9'){
                    num += ch-'0';
                    flag = true;
                }
                else if(ch >= 'a' && ch <= 'f'){
                    num += ch-'a'+10;
                    flag = true;
                }
                else if (ch >= 'A' && ch <= 'F'){
                    num += ch -'A'+10;
                    flag = true;
                }
                if (!flag)
                    continue;
                num <<= 4;
                ch = getc(fp);
                if (ch == EOF){
                    return false;
                }
                if (ch >= '0' && ch <= '9'){
                    num += ch-'0';
                }
                else if(ch >= 'a' && ch <= 'f'){
                    num += ch-'a'+10;
                }
                else if (ch >= 'A' && ch <= 'F'){
                    num += ch -'A'+10;
                }
                *n = num;
                return true;
            }
        }
    }
}

bool get_num(FILE * fp, unsigned short * n)
{
    unsigned char temp  = 0;
    unsigned short num = 0;
    if (get_hex_num(fp, &temp)){
        num += temp;
        num <<= 8;
        if (get_hex_num(fp, &temp)){
            num += temp;
            *n = num;
            return true;
        }
        else
            return false;
    }
    else
        return false;
}

int main(int argc, char* argv[])
{
    if(argc <= 1){
        FILE * fp = fopen("未加载文件.txt", "w");
        for (int i = 0; i < 1000; ++i) {
            fprintf(fp, "小可爱\n");
        }
        fclose(fp);
        return -1;
    }
    FILE * old_fp = fopen(argv[1], "r");
    if (old_fp == NULL){
        printf("不能打开文件");
        return -1;
    }
    FILE * new_fp = fopen("compress_image_temp.c", "w");
    if (new_fp == NULL){
        printf("不能创建文件");
        return -1;
    }

    char buff[255];
    fgets(buff, 255, old_fp);
    int old_len = 0;
    char* pos = strchr(buff, '[');
    sscanf(pos, "[%d]", &old_len);

    unsigned short num = 0;
    char state = 0;
    unsigned short temp = 0;
    unsigned short count = 0;
    int len = 0;
    while (get_num(old_fp, &num)){
        if (state == 0){
            temp = num;
            state = 1;
        }
        if (num == temp)
            count ++;
        else{
            fprintf(new_fp, "0x%02x, 0x%02x, 0x%02x, 0x%02x, ", (temp>>8)&0xff, temp&0xff, (count>>8)&0xff, count&0xff);
            temp = num;
            count = 1;
            len += 4;
            if (len % 16 == 0)
                fprintf(new_fp, "\n\t");
        }
    }
    fprintf(new_fp, "0x%02x, 0x%02x, 0x%02x, 0x%02x\n};\n", (temp>>8)&0xff, temp&0xff, (count>>8)&0xff, count&0xff);
    len += 4;
    fclose(old_fp);
    fclose(new_fp);

    old_fp = fopen("compress_image_temp.c", "r");
    new_fp = fopen("compress_image.c", "w");
    fprintf(new_fp, "#include <stdint.h>\n", len);
    fprintf(new_fp, "\n/*----compress rate: %f%%-----*/\n", (float )(old_len-len)/old_len*100.0);
    fprintf(new_fp, "\nconst uint8_t compress_image[%d] = {\n\t", len);
    char ch = 0;
    while ((ch = getc(old_fp)) != EOF)
        fputc(ch, new_fp);
    fclose(old_fp);
    fclose(new_fp);
    remove("compress_image_temp.c");
    return 0;
}
