/*
some text decode and encode method��
*/

#include "pch.h"

bool CheckUtf8(LPCSTR p);
void ConvertBig(TCHAR* p);
char* UnicodeToANSI(const wchar_t* str);
wchar_t* ANSIToUnicode(const char* str);
wchar_t* UTF8ToUnicode(const char* str);
char* UnicodeToUTF8(const wchar_t* str);
char* UTF8ToANSI(const char* str);
int GetEncodeType(unsigned char* p);


int GetEncodeType(unsigned char* p)
{
    if (p[0] == 0xFE && p[1] == 0xFF) // unicode BE
    {
        return ENCODE_BE;
    }else if (p[0] == 0xFF && p[1] == 0xFE) // unicode LE
    {
        return ENCODE_LE;
    }
    else if (p[0] == 0xEF && p[1] == 0xBB && p[2] == 0xBF)
    {
        return ENCODE_BOMU8;   // utf-8 with bom;
    }
    else
    {
        if (CheckUtf8((LPCSTR)p))  // utf-8 without bom
            return ENCODE_NOBOMU8;
        else
            return ENCODE_ANSII;  // asnii encode
    }
}


UINT GetFileLengthxl(FILE* fp);

UINT GetFileLengthxl(FILE* fp)
{
    fseek(fp, 0, SEEK_END);
    UINT len = ftell(fp);
    fseek(fp, 0, SEEK_SET);  // ָ��ص���ʼλ��
    return len;
}
bool CheckUtf8(LPCSTR p)
{
    /*
    ASCII �ַ� ��ʮ���� 65���� UTF-8 ������ 		01000001��  --------
                                                &0x80
    ˫�ֽڱ��� ��ʮ���� 224���� UTF-8 ������ 		11000010 10000010��-----
                                                &0xC0
    ���ֽڱ��� ��ʮ���� 8364���� UTF-8 ������ 	11100010 10000101 10011000��-----
                                                &0xE0
    ���ֽڱ��� ��ʮ���� 128512���� UTF-8 ������ 11110000 10000000 10001111 10000010��
                                                &0xF0

    */
    auto q = p;
    while (*p)  // ���������ֽ�
    {
        BYTE c = *p;
        int n = 0;   // ���ֽ�ʱ�����Լ����ַ����ɼ����ֽ����
        while ((c & 0x80) == 0x80)  // С��0x80 �� ascll��������ǵ��ֽڵ��� 0x80 ���Ƕ��ֽ�
            ++n, c <<= 1;   // n == 2,��c��2�ֽ���� n==3 ��3�ֽڣ�n==4,����4�ֽ����

        if (n == 1 || n > 4)   // �����10xx xxxx ����δ֪�ֽڣ���������utf-8
            return false;

        ++p;  // p�ƶ����ֽڱ��봦

        while (--n > 0)  // if n=4, s��n3-->n2-->n1-->n0-->end;
        {
            // 11110000 10000000 10001111 10000010
            c = *p++;
            if (c >> 6 != 2)//00000010   // utf-8 �ֽڱ��뿪ʼ������λһ����10
                return false;
        }

    }
    return true;
}

void ConvertBig(TCHAR* p)
{
    while (*p)
    {
        *p = ((*p&0xFF00)>>8) | ((*p&0x00FF)<<8);
        ++p;
    }

    /*
    while (*(WORD*)p)
    {
        *p = *p ^ *(p+1);
        *(p + 1) = *p ^ *(p + 1);
        *p = *p ^ *(p + 1);
        p += 2;
    }
    */
}

char* UnicodeToANSI(const wchar_t* str)
{
    int n = WideCharToMultiByte(CP_ACP, 0, str, -1, NULL, 0, NULL, NULL);  // get length
    auto p = new char[n + 1];
    n = WideCharToMultiByte(CP_ACP, 0, str, -1, p, n, NULL, NULL);
    p[n] = 0;
    return p;
}

wchar_t* ANSIToUnicode(const char* str)
{
    int n = MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, 0);   // get length
    auto p = new wchar_t[n + 1];
    MultiByteToWideChar(CP_ACP, 0, str, -1, p, n);
    return p;
}

wchar_t* UTF8ToUnicode(const char* str)
{
    int n = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
    auto p = new wchar_t[n + 1];
    n = MultiByteToWideChar(CP_UTF8, 0, str, -1, p, n);
    p[n] = 0;
    return p;
}

char* UnicodeToUTF8(const wchar_t* str)
{
    int n = WideCharToMultiByte(CP_UTF8, 0, str, -1, NULL, 0, NULL, NULL);
    auto p = new char[n + 1];
    n = WideCharToMultiByte(CP_UTF8, 0, str, -1, p, n, NULL, NULL);
    p[n] = 0;
    return p;
}

char* UTF8ToANSI(const char* str)
{
    auto p = UTF8ToUnicode(str);
    auto q = UnicodeToANSI(p);
    delete[]p;
    return q;
}