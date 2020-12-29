#include<stdio.h>
#include<Windows.h>
#include<process.h>
#include"ring_buffer.h"
#include"qrencode.h"
#include"hook.h"
#pragma comment(lib, "HKtest.lib")

DWORD WINAPI ThreadFunc(LPVOID LpParameter)

{

    //::MessageBox(NULL, _T("setupDCB success!"), _T("Information"), MB_OK);

    WSADATA WSAData = { 0 };
    int ret = 0;
    ret = WSAStartup(MAKEWORD(2, 2), &WSAData);
    if (ret < 0) {
       // print_error("WSAStartup()", GetLastError());
        return EXIT_FAILURE;
    }

    run_server_mode();
    WSACleanup();

    Sleep(1000);
    //MessageBox(NULL, TEXT("test"), TEXT("test"), MB_OK);


  //  printf(" ");
    return 0;

}







LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    PSTR szCmdLine, int iCmdShow)
{

    make_code();//���ɶ�ά��

    static TCHAR szAppName[] = TEXT("Bricks1");
    HWND         hwnd;
    MSG          msg;
    WNDCLASS     wndclass;

    wndclass.style = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc = WndProc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = hInstance;
    wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = szAppName;

    if (!RegisterClass(&wndclass))
    {
        MessageBox(NULL, TEXT("This program requires Windows NT!"),
            szAppName, MB_ICONERROR);
        return 0;
    }

    hwnd = CreateWindow(szAppName, TEXT("Demo"),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, iCmdShow);
    UpdateWindow(hwnd);

   //  setHook(1, VK_RETURN);//��װ����

    DWORD mainth = GetCurrentThreadId();
    HANDLE hThread = _beginthreadex(NULL, 0, ThreadFunc, &mainth, 0, NULL);
   // HANDLE hThread = CreateThread(NULL, 0, ThreadFunc, NULL, 0, NULL);//ͨ���߳�

    //�رն���
  
   
   // setHook(1, VK_RETURN);
    


    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (msg.message == Message_login)
        {
            //MessageBox(NULL, TEXT("test"), TEXT("test"), MB_OK);
            //stopHook();//�յ��ϻ��źŽ���
        }else if(msg.message == Message_exit)
        {
            //setHook(1, VK_RETURN);//���յ��»��ź�����
        }
        Sleep(20);

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    CloseHandle(hThread);
    return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HBITMAP hBitmap;
    static int     cxClient, cyClient, cxSource, cySource;
    BITMAP         bitmap;
    HDC            hdc, hdcMem;//�����ڴ�
    HINSTANCE      hInstance;
    int            x = 0, y = 100;
    PAINTSTRUCT    ps;


    switch (message)
    {
    case WM_CREATE:
        hInstance = ((LPCREATESTRUCT)lParam)->hInstance;

        hBitmap = (HBITMAP)LoadImage(NULL, L"temp.bmp", IMAGE_BITMAP, 400, 400, LR_LOADFROMFILE);//���ض�ά��
        //hBitmap = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BITMAP4));
        GetObject(hBitmap, sizeof(BITMAP), &bitmap);

        cxSource = bitmap.bmWidth;
        cySource = bitmap.bmHeight;
        //WCHAR str[100];
        //wsprintf(str, TEXT("%ld"), cxSource);
        //MessageBox(NULL, str, TEXT("cySource"), MB_ICONINFORMATION | MB_YESNO);

        return 0;

    case WM_SIZE:
        cxClient = LOWORD(lParam);
        cyClient = HIWORD(lParam);
        //cxClient = 300;
        //cyClient = 300;
        return 0;

    case WM_PAINT:
        hdc = BeginPaint(hwnd, &ps);

        // Ϊ������ͼ��λ���ڿͻ���������
        // ˮƽ����ֱ������ʾ
        x = cxClient / 2 - cxSource / 2;
        y = cyClient / 2 - cySource / 2;

        hdcMem = CreateCompatibleDC(hdc);
        SelectObject(hdcMem, hBitmap);

        BitBlt(hdc, x, y, cxSource, cySource, hdcMem, 0, 0, SRCCOPY);//��bmpͼƬд�������ڴ�DC

        DeleteDC(hdcMem);
        EndPaint(hwnd, &ps);

        //MessageBox(NULL, "", TEXT(GetLastError()), MB_ICONINFORMATION | MB_YESNO);
        return 0;

    case WM_DESTROY:
        DeleteObject(hBitmap);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
} 