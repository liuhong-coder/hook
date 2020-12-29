#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <stdio.h>
#include <tchar.h>
#include "qrencode.h"
//using namespace std;
#pragma warning(disable:4099)

#pragma comment(lib,"QRcode.lib")//编译qrencode库生成的lib文件

void make_code()
{
	//const char* szSourceSring = "TEST";
	//获取计算机名
	
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
	//使用第三方库提供的QRcode_encodeString函数将字符串转换成二维码，再利用画图API将图片画出
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
		kFileHeader.bfType = 0x4D42;              //定义图片格式为bmp
		kFileHeader.bfSize = sizeof(BITMAPFILEHEADER) +
			sizeof(BITMAPINFOHEADER) +
			unDataBytes;						  //文件大小
		kFileHeader.bfReserved1 = 0;              //保留，必须为0
		kFileHeader.bfReserved2 = 0;              //保留，必须为0
		kFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) +
			sizeof(BITMAPINFOHEADER);             //从文件头开始的偏移量
		BITMAPINFOHEADER kInfoHeader;
		kInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
		kInfoHeader.biWidth = unWidth * 8;        //宽
		kInfoHeader.biHeight = unWidth * 8;       //高
		kInfoHeader.biPlanes = 1;                 //为目标设备说明位面数，其值总是设为1
		kInfoHeader.biBitCount = 24;	          //说明像素，24位
		kInfoHeader.biCompression = BI_RGB;       //图像数据的压缩类型
		kInfoHeader.biSizeImage = unDataBytes;    //图片大小
		kInfoHeader.biXPelsPerMeter = 0x60;       //水平分辨率
		kInfoHeader.biYPelsPerMeter = 0x60;       //垂直分辨率
		kInfoHeader.biClrUsed = 0;                //位图实际使用的彩色表中的颜色索引数
		kInfoHeader.biClrImportant = 0;           //对图像显示有重要影响的颜色索引的数目
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
		//把图片字节数据copy到字节数组中
		int dwSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + unDataBytes * sizeof(unsigned char);
		unsigned char* imgbytes = (unsigned char*)malloc(dwSize);
		memcpy(imgbytes, &kFileHeader, sizeof(BITMAPFILEHEADER));
		memcpy(imgbytes + sizeof(BITMAPFILEHEADER), &kInfoHeader, sizeof(BITMAPINFOHEADER));
		memcpy(imgbytes + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER), pRGBData, sizeof(unsigned char) * unDataBytes);
		//记得释放
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
