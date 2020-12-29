#define _CRT_SECURE_NO_WARNINGS
#include <WinSock2.h>
#include <stdio.h>
#include <initguid.h>
#include <ws2bth.h>
#include "log.h"
#include <stdint.h>
#include <tchar.h>
#include "hook.h"
#include "ring_buffer.h"
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "HKtest.lib")

// {B62C4E8D-62CC-404b-BBBF-BF3E3BBB1374}
DEFINE_GUID(g_guidServiceClass, 0xb62c4e8d, 0x62cc, 0x404b, 0xbb, 0xbf, 0xbf, 0x3e, 0x3b, 0xbb, 0x13, 0x74);

#define DEFAULT_LISTEN_BACKLOG        4
#define INSTANCE_STR L"BluetoothWindows"

#define LEN_RECV 256


#define LEN_HEADER 2
typedef struct packet_s
{
    char header[LEN_HEADER];
    //int32_t x;
    //int32_t y;
} packet_t;

/*
static void print_error(char const* where, int code)
{
    fprintf(stderr, "Error on %s: code %d\n", where, code);
}
*/

//�󶨶˿�
BOOL bind_socket(SOCKET local_socket, SOCKADDR_BTH* sock_addr_bth_local)
{
    int addr_len = sizeof(SOCKADDR_BTH);

    /* Setting address family to AF_BTH indicates winsock2 to use Bluetooth port. */
    sock_addr_bth_local->addressFamily = AF_BTH;
    sock_addr_bth_local->port = BT_PORT_ANY;

    if (bind(local_socket, (struct sockaddr*) sock_addr_bth_local, sizeof(SOCKADDR_BTH)) == SOCKET_ERROR) {
 //       print_error("bind()", WSAGetLastError());
        return FALSE;
    }

    if (getsockname(local_socket, (struct sockaddr*)sock_addr_bth_local, &addr_len) == SOCKET_ERROR) {
 //       print_error("getsockname()", WSAGetLastError());
        return FALSE;
    }
    return TRUE;
}

//�����豸��ַ��Ϣ
 LPCSADDR_INFO create_addr_info(SOCKADDR_BTH* sock_addr_bth_local)
{
    LPCSADDR_INFO addr_info = calloc(1, sizeof(CSADDR_INFO));

    if (addr_info == NULL) {
//        print_error("malloc(addr_info)", WSAGetLastError());
        return NULL;
    }

    addr_info[0].LocalAddr.iSockaddrLength = sizeof(SOCKADDR_BTH);
    addr_info[0].LocalAddr.lpSockaddr = (LPSOCKADDR)sock_addr_bth_local;
    addr_info[0].RemoteAddr.iSockaddrLength = sizeof(SOCKADDR_BTH);
    addr_info[0].RemoteAddr.lpSockaddr = (LPSOCKADDR)&sock_addr_bth_local;
    addr_info[0].iSocketType = SOCK_STREAM;
    addr_info[0].iProtocol = BTHPROTO_RFCOMM;
    return addr_info;
}

/*
instance_name is a pointer to wchar_t* which is malloc'ed by this function.
Must be free manually after.
*/

//�㲥����
 BOOL advertise_service_accepted(LPCSADDR_INFO addr_info, wchar_t** instance_name)
{
    WSAQUERYSET wsa_query_set = { 0 };
    wchar_t computer_name[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD len_computer_name = MAX_COMPUTERNAME_LENGTH + 1;
    size_t instance_name_size = 0;
    HRESULT res;

    if (!GetComputerName(computer_name, &len_computer_name)) {
//        print_error("GetComputerName()", WSAGetLastError());
        return FALSE;
    }

    /*
    Adding a byte to the size to account for the space in the
    format string in the swprintf call. This will have to change if converted
    to UNICODE.
    */
    res = StringCchLength(computer_name, sizeof(computer_name), &instance_name_size);
    if (FAILED(res)) {
//        print_error("ComputerName specified is too large", WSAGetLastError());
        return FALSE;
    }

    instance_name_size += sizeof(INSTANCE_STR) + 1;
    *instance_name = malloc(instance_name_size);
    if (*instance_name == NULL) {
 //       print_error("malloc(instance_name)", WSAGetLastError());
        return FALSE;
    }

    /* If we got an address, go ahead and advertise it. */
    ZeroMemory(&wsa_query_set, sizeof(wsa_query_set));
    wsa_query_set.dwSize = sizeof(wsa_query_set);
    wsa_query_set.lpServiceClassId = (LPGUID)&g_guidServiceClass;

    StringCbPrintf(*instance_name, instance_name_size, L"%s %s", computer_name, INSTANCE_STR);
    wsa_query_set.lpszServiceInstanceName = *instance_name;
    wsa_query_set.lpszComment = L"Example of server on Windows expecting bluetooth connections";
    wsa_query_set.dwNameSpace = NS_BTH;
    wsa_query_set.dwNumberOfCsAddrs = 1; /* Must be 1. */
    wsa_query_set.lpcsaBuffer = addr_info; /* Req'd */

    /*
    As long as we use a blocking accept(), we will have a race between advertising the service and actually being ready to
    accept connections.  If we use non-blocking accept, advertise the service after accept has been called.
    */
    if (WSASetService(&wsa_query_set, RNRSERVICE_REGISTER, 0) == SOCKET_ERROR) {
        free(instance_name);
//        print_error("WSASetService()", WSAGetLastError());
        return FALSE;
    }
    return TRUE;
}


//��������
 receive_data(SOCKET client_socket, ring_buffer_t* rb)
{
    char* buffer = NULL;
    int len_read = 0;

    buffer = calloc(LEN_RECV, sizeof(char*));
    if (buffer == NULL) {
 //       print_error("malloc(buffer)", WSAGetLastError());
        return FALSE;
    }

    len_read = recv(client_socket, buffer, LEN_RECV, 0);
    if (len_read == SOCKET_ERROR) {
        free(buffer);
 //       print_error("recv()", WSAGetLastError());
        return FALSE;
    }
    if (len_read == 0) {
        free(buffer);
        fprintf(stderr, "Nothing read, end of communication\n");
        return FALSE;
    }
    push_data_in_ring_buffer(rb, buffer, len_read);
    free(buffer);
    return TRUE;
}

//��������
int handle_data_read(ring_buffer_t* rb)
{
    packet_t* packet;

    packet = (packet_t*)pop_data_from_ring_buffer(rb, sizeof(packet_t));
    if (packet == NULL) {
        fprintf(stderr, "Cannot handle packet read\n");
        return 0;
    }
    /* printf("Packet: header=[%s] x=%d, y=%d\n", packet->header, packet->x, packet->y); */
    /*
    for (int i = 0; i < 1; ++i) {
        //printf("%c", packet->header[i]);
        //stopHook();
        if (packet->header[i] != 2) {
            MessageBox(NULL, TEXT("�����ɹ�"), TEXT("test"), MB_OK);
          // write_log()
        }

    }*/

    //���ַ�����ת��Ϊ�ַ���
    char* dest_str;
    dest_str = (char*)malloc(sizeof(char) * (sizeof(packet->header) + 1));
    if (dest_str == NULL)
        return 0;

    strncpy(dest_str, packet->header, sizeof(packet->header));
    dest_str[sizeof(packet->header)] = '\0';
    if (strcmp(dest_str, "10") == 0) {
       //  stopHook();
        // MessageBox(NULL, L(packet->header), TEXT("test"), MB_OK);

        HWND hWnd = FindWindow(NULL, TEXT("Demo"));//��ȡ�����ھ��
       PostMessage(hWnd, Message_login, 0, 0);//�����̷߳��ͽ����ź�

        MessageBox(NULL, TEXT("�����ɹ�"), TEXT("test"), MB_OK);
        free(dest_str);
        free(packet);
        return 1;
    }
    else if (strcmp(dest_str, "11") == 0) {
        HWND hWnd = FindWindow(NULL, TEXT("Demo"));
        PostMessage(hWnd, Message_exit, 0, 0);//�����̷߳��������ź�
        MessageBox(NULL, TEXT("�»�"), TEXT("test"), MB_OK);
        free(dest_str);
        free(packet);
        return 2;
    }

    free(dest_str);
    /*
    else {
        MessageBox(NULL, TEXT("����ʧ��"), TEXT("test"), MB_OK);
        return 0;
    }
    */
    // FILE* file = fopen("log.txt", "a");
     //write_log(file, "%s\n", "is working");




    // printf("\n");
    free(packet);
    return 0;
}

//����������
BOOL run_server_mode()
{
   
    
    wchar_t* instance_name = NULL;
    SOCKET          local_socket = INVALID_SOCKET;
    SOCKADDR_BTH    sock_addr_bth_local = { 0 };
    LPCSADDR_INFO   addr_info = NULL;
    ring_buffer_t* rb = NULL;
    BOOL ret = FALSE;

    /* Open a bluetooth socket using RFCOMM protocol. */
    local_socket = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);
    if (local_socket == INVALID_SOCKET) {
//        print_error("socket()", WSAGetLastError());
        MessageBox(NULL, TEXT("����socketʧ��"), TEXT("test"), MB_OK);
        return FALSE;
        //exit(1);
    }

    ret = bind_socket(local_socket, &sock_addr_bth_local);
    if (!ret) {
        MessageBox(NULL, TEXT("bindʧ��"), TEXT("test"), MB_OK);
        return FALSE;
        //exit(1);

    }
    addr_info = create_addr_info(&sock_addr_bth_local);
    if (!addr_info) {
        MessageBox(NULL, TEXT("������ַʧ��"), TEXT("test"), MB_OK);
        return FALSE;
        //exit(1);

    }
    ret = advertise_service_accepted(addr_info, &instance_name);
    if (!ret) {
        free(addr_info);
        if (instance_name) {
            free(instance_name);
        }
        return FALSE;
        // exit(1);
    }

    if (listen(local_socket, DEFAULT_LISTEN_BACKLOG) == SOCKET_ERROR) {
 //       print_error("listen()", WSAGetLastError());
        free(addr_info);
        free(instance_name);
        return FALSE;
        // exit(1);
    }
     FILE* file = fopen("log.txt", "a");
    // write_log(file, "%s\n", "is waiting");
    while (1) {
        // printf("Waiting for client connection...");
       // MessageBox(NULL, TEXT("wait"), TEXT("test"), MB_OK);
        SOCKET client_socket = accept(local_socket, NULL, NULL);
        if (client_socket == INVALID_SOCKET) {
 //           print_error("accept()", WSAGetLastError());
            return FALSE;
            // exit(1);
        }
        // printf("Client connected !\n");
        //MessageBox(NULL, TEXT("connected"), TEXT("test"), MB_OK);
        rb = create_ring_buffer(LEN_RECV * 3);
        if (rb == NULL) {
            break;
        }

        ret = TRUE;
        while (ret == TRUE) {
            ret = receive_data(client_socket, rb);
            while (rb->count >= sizeof(packet_t)) {
                int answer = handle_data_read(rb);
                if (answer == 1) {
                     write_log(file, "%s\n", "��ʼʹ��");
                   //  stopHook();
                }
                else if (answer == 2) {
                    // send(local_socket, "00", 3, 0);
                     write_log(file, "%s\n", "����ʹ��");
                    /*
                     typedef void(*lpAddFun)(int, WORD);
                     HINSTANCE hDll;
                     hDll = LoadLibrary(_T("HKtest.dll"));
                     lpAddFun addFun;
                     addFun = (lpAddFun)GetProcAddress(hDll, "setHook");
                     addFun(1, VK_RETURN);
                     */
                }


            }
            
        }
        fclose(file);
        //printf("Communication over\n");
        //MessageBox(NULL, TEXT("over"), TEXT("test"), MB_OK);
       // write_log(file, "%s\n", "����ʹ��");
        closesocket(client_socket);
        delete_ring_buffer(rb);

    }
     
    free(addr_info);
    free(instance_name);
    closesocket(local_socket);
    return TRUE;
}