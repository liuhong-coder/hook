#pragma once

#ifndef __RING_BUFFER_H__
#define __RING_BUFFER_H__

#include <stdlib.h>
#include <Windows.h>
#include <strsafe.h>

#define Message_login (WM_USER + 10)
#define Message_exit (WM_USER + 11)
typedef struct ring_buffer_s
{
    char* buffer;
    char* end;

    size_t capacity;
    size_t count;

    char* head;
    char* tail;

} ring_buffer_t;

ring_buffer_t* create_ring_buffer(size_t capacity);//创建缓冲区
void push_data_in_ring_buffer(ring_buffer_t* ring_buffer, char* data, size_t len);//将数据写入缓冲区
char* pop_data_from_ring_buffer(ring_buffer_t* ring_buffer, size_t len);//将数据从缓冲区取出
void delete_ring_buffer(ring_buffer_t* ring_buffer);//删除缓冲区

static void print_error(char const* where, int code)
{
    fprintf(stderr, "Error on %s: code %d\n", where, code);
}

BOOL run_server_mode();//启动蓝牙通信模块

#endif
