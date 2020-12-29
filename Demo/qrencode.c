#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <stdio.h>
#include <tchar.h>
#include "qrencode.h"
//using namespace std;
#pragma warning(disable:4099)

#pragma comment(lib,"QRcode.lib")//����qrencode�����ɵ�lib�ļ�

void make_code()
{
	//const char* szSourceSring = "TEST";
	//��ȡ�������
	
	TCHAR szName[50] = { 0 };
	DWORD nSize = 50;
	CHAR str[50];
	if (GetComputerName(szName, &nSize))
		_tprintf(TEXT("%s\n"), szName);
	sprintf(str, "%ws", szName);
	
	unsigned int    unWidth, x, y, l, n, unWidthAdjusted, unDataBytes;
	unsigned char* pRGBData, * pSourceData, * pDestData;
	QRcode* pQRC;
	FILE* f;
	//ʹ�õ��������ṩ��QRcode_encodeString�������ַ���ת���ɶ�ά�룬�����û�ͼAPI��ͼƬ����
	if (pQRC = QRcode_encodeString(str, 1, QR_ECLEVEL_L, QR_MODE_8, 1))
	{
		unWidth = pQRC->width;
		unWidthAdjusted = unWidth * 8 * 3;
		if (unWidthAdjusted % 4)
			unWidthAdjusted = (unWidthAdjusted / 4 + 1) * 8;
		unDataBytes = unWidthAdjusted * unWidth * 8;
		// Allocate pixels buffer
		if (!(pRGBData = (unsigned char*)malloc(unDataBytes)))
		{
			exit(-1);
		}
		// Preset to white
		memset(pRGBData, 0xff, unDataBytes);
		// Prepare bmp headers
		BITMAPFILEHEADER kFileHeader;
		kFileHeader.bfType = 0x4D42;              //����ͼƬ��ʽΪbmp
		kFileHeader.bfSize = sizeof(BITMAPFILEHEADER) +
			sizeof(BITMAPINFOHEADER) +
			unDataBytes;						  //�ļ���С
		kFileHeader.bfReserved1 = 0;              //����������Ϊ0
		kFileHeader.bfReserved2 = 0;              //����������Ϊ0
		kFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) +
			sizeof(BITMAPINFOHEADER);             //���ļ�ͷ��ʼ��ƫ����
		BITMAPINFOHEADER kInfoHeader;
		kInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
		kInfoHeader.biWidth = unWidth * 8;        //��
		kInfoHeader.biHeight = unWidth * 8;       //��
		kInfoHeader.biPlanes = 1;                 //ΪĿ���豸˵��λ��������ֵ������Ϊ1
		kInfoHeader.biBitCount = 24;	          //˵�����أ�24λ
		kInfoHeader.biCompression = BI_RGB;       //ͼ�����ݵ�ѹ������
		kInfoHeader.biSizeImage = unDataBytes;    //ͼƬ��С
		kInfoHeader.biXPelsPerMeter = 0x60;       //ˮƽ�ֱ���
		kInfoHeader.biYPelsPerMeter = 0x60;       //��ֱ�ֱ���
		kInfoHeader.biClrUsed = 0;                //λͼʵ��ʹ�õĲ�ɫ���е���ɫ������
		kInfoHeader.biClrImportant = 0;           //��ͼ����ʾ����ҪӰ�����ɫ��������Ŀ
		// Convert QrCode bits to bmp pixels
		pSourceData = pQRC->data;
		for (y = 0; y < unWidth; y++)
		{
			pDestData = pRGBData + unWidthAdjusted * y * 8;
			for (x = 0; x < unWidth; x++)
			{
				if (*pSourceData & 1)
				{
					for (l = 0; l < 8; l++)
					{
						for (n = 0; n < 8; n++)
						{
							//this is qrcode color default black
							*(pDestData + n * 3 + unWidthAdjusted * l) = 0x00;
							*(pDestData + 1 + n * 3 + unWidthAdjusted * l) = 0;
							*(pDestData + 2 + n * 3 + unWidthAdjusted * l) = 0;
						}
					}
				}
				pDestData += 3 * 8;
				pSourceData++;
			}
		}
		//��ͼƬ�ֽ�����copy���ֽ�������
		int dwSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + unDataBytes * sizeof(unsigned char);
		unsigned char* imgbytes = (unsigned char*)malloc(dwSize);
		memcpy(imgbytes, &kFileHeader, sizeof(BITMAPFILEHEADER));
		memcpy(imgbytes + sizeof(BITMAPFILEHEADER), &kInfoHeader, sizeof(BITMAPINFOHEADER));
		memcpy(imgbytes + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER), pRGBData, sizeof(unsigned char) * unDataBytes);
		//�ǵ��ͷ�
		//delete[] imgbytes;
		//memset(imgbytes, 0, dwSize);
		free(imgbytes);
		// Output the bmp file
		if (!(fopen_s(&f, "temp.bmp", "wb+")))
		{
			fwrite(&kFileHeader, sizeof(BITMAPFILEHEADER), 1, f);
			fwrite(&kInfoHeader, sizeof(BITMAPINFOHEADER), 1, f);
			fwrite(pRGBData, sizeof(unsigned char), unDataBytes, f);
			fclose(f);
		}
		else
		{
			printf("Unable to open file");
			exit(-1);
		}
		// Free data
		free(pRGBData);
		QRcode_free(pQRC);
	}
	else
	{
		printf("NULL returned");
		exit(-1);
	}
	//getchar();
	//return 0;
}
