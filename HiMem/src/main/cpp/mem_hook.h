//
// Created by creat on 2020/3/24 0024.
//

#ifndef SOCKETS_NATIVE_SOCKET_HOOK_H
#define SOCKETS_NATIVE_SOCKET_HOOK_H

extern int MODE;

void set_hook_debug(int enable);

void do_hook();

int rehook_for_iterate();

void clear_hook();

#endif //SOCKETS_NATIVE_SOCKET_HOOK_H
