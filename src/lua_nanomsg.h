#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "lauxlib.h"
#include <nanomsg/nn.h>
#include <nanomsg/reqrep.h>
#include <nanomsg/survey.h>
#include <nanomsg/pair.h>

#define LUANANOMSG_VERSION "lua-nanomsg 0.0.1"
#define LUANANOMSG_COPYRIGHT "Copyright (C) 2016, Zaher Marzuq"
#define LUANANOMSG_DESCRIPTION "nanomsg binding for Lua"

LUALIB_API int lnn_socket(lua_State *L);
LUALIB_API int lnn_close(lua_State *L);
LUALIB_API int lnn_setsockopt(lua_State *L);
LUALIB_API int lnn_getsockopt (lua_State *L);
LUALIB_API int lnn_bind(lua_State *L);
LUALIB_API int lnn_connect(lua_State *L);
LUALIB_API int lnn_shutdown(lua_State *L);
LUALIB_API int lnn_send(lua_State *L);
LUALIB_API int lnn_recv(lua_State *L);
LUALIB_API int lnn_sendmsg(lua_State *L);
LUALIB_API int lnn_recvmsg(lua_State *L);

typedef struct {
  int domain;
  int protocol;
  int fd;
} lnn_socket_t;
/* connection struct */
typedef struct {
  int eid;
  lnn_socket_t *socket;
  const char *addr;
} lnn_connection_t;

