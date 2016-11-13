#include "lua_nanomsg.h"

#if !defined LUA_VERSION_NUM || LUA_VERSION_NUM==501
LUALIB_API void luaL_setfuncs (lua_State *L, const luaL_Reg *l, int nup) {
  luaL_checkstack(L, nup+1, "too many upvalues");
  for (; l->name != NULL; l++) {  /* fill the table with given functions */
    int i;
    lua_pushstring(L, l->name);
    for (i = 0; i < nup; i++)  /* copy upvalues to the top */
      lua_pushvalue(L, -(nup+1));
    lua_pushcclosure(L, l->func, nup);  /* closure with those upvalues */
    lua_settable(L, -(nup + 3));
  }
  lua_pop(L, nup);  /* remove upvalues */
}
#endif

LUALIB_API void setmeta(lua_State *L, const char *name) {
  luaL_getmetatable(L, name);
  lua_setmetatable(L, -2);
}

LUALIB_API int createmeta(lua_State *L, const char *name, const luaL_Reg *methods) {
  if (!luaL_newmetatable(L, name)) {
    return 0;
  }

  lua_pushstring(L, "__index");
  lua_newtable(L);
  for (; methods->name; methods++) {
    lua_pushstring(L, methods->name);
    lua_pushcfunction(L, methods->func);
    lua_rawset(L, methods->name[0] == '_' ? -5: -3);
  }
  lua_rawset(L, -3);
  lua_pop(L, 1);
  return 1;
}

LUALIB_API void lnn_push_symbols(lua_State *L) {
  int value, i, len;
  char *nice_name;
  for (i = 0; ; ++i) {
    const char* name = nn_symbol (i, &value);
    if (name == NULL) break;
    len = strlen(name);
    if(strstr(name, "NN_")) {
      nice_name = strndup(name + 3, len - 3);
    }
    else {
      nice_name = strndup(name + 0, len);
    }
    lua_pushinteger(L, value);
    lua_setfield(L, -2, nice_name);
  }
}

LUALIB_API lnn_connection_t *lnn_get_connection(lua_State *L, size_t index) {
  lnn_connection_t *c = (lnn_connection_t *) luaL_checkudata(L, index, "connection");
  luaL_argcheck(L, c != NULL, index, "connection expected");
  return c;
}

LUALIB_API void lnn_push_connection(lua_State *L, lnn_socket_t *socket, int eid) {
  lnn_connection_t *c = (lnn_connection_t *) lua_newuserdata(L, sizeof(lnn_connection_t));
  c->eid = eid;
  c->socket = socket;
  setmeta(L, "connection");
}

/* socket params expected 2 & 3 */
LUALIB_API lnn_socket_t* lnn_socket(lua_State *L) {
    int domain = luaL_optnumber(L, 2, AF_SP);
    int protocol = luaL_optnumber(L, 3, NN_REQ);
    int socket = nn_socket(domain, protocol);
    /* success */
    if(socket >= 0) {
      lnn_socket_t *s = malloc(sizeof(lnn_socket_t));
      s->domain = domain;
      s->protocol = protocol;
      s->fd = socket;
      return s;
    }
    return NULL;
}

LUALIB_API int lnn_bind(lua_State *L) {
    const char *addr = luaL_checkstring(L, 1);
    lnn_socket_t *socket = lnn_socket(L);
    if(socket != NULL) {
      int to = 100;
      nn_setsockopt(socket->fd, NN_SOL_SOCKET, NN_RCVTIMEO, &to, sizeof(to));
      nn_setsockopt(socket->fd, NN_SOL_SOCKET, NN_SNDTIMEO, &to, sizeof(to));
      int eid = nn_bind(socket->fd, addr);
      if(eid >= 0) {
        lnn_push_connection(L, socket, eid);
      }
    }
    return 1;
}

LUALIB_API int lnn_connect(lua_State *L) {
  const char *addr = luaL_checkstring(L, 1);
  lnn_socket_t *socket = lnn_socket(L);
  /* success */
  if(socket != NULL) {
    int to = 200;
    nn_setsockopt(socket->fd, NN_SOL_SOCKET, NN_RCVTIMEO, &to, sizeof(to));
    nn_setsockopt(socket->fd, NN_SOL_SOCKET, NN_SNDTIMEO, &to, sizeof(to));
    int eid = nn_connect(socket->fd, addr);
    if(eid >= 0) {
      lnn_push_connection(L, socket, eid);
    }
  }
  return 1;
}

LUALIB_API int lnn_send(lua_State *L) {
  lnn_connection_t *c = lnn_get_connection(L, 1);
  size_t data_len;
  const char *data = luaL_checklstring(L, 2, &data_len);
  int rc = nn_send (c->socket->fd, data, data_len + 1, 0);
  return 1;
}

LUALIB_API int lnn_recv(lua_State *L) {
  lnn_connection_t *c = lnn_get_connection(L, 1);
  char *buf = NULL;
  int bytes = nn_recv(c->socket->fd, &buf, NN_MSG, 0);
  if(bytes >= 0) {
    lua_pushlstring(L, buf, bytes);
    nn_freemsg(buf);
    return 1;
  }
  return 0;
}

LUALIB_API int lnn_shutdown(lua_State *L) {
  lnn_connection_t *c = lnn_get_connection(L, 1);
  nn_shutdown(c->socket->fd, c->eid);
  return 1;
}

LUALIB_API int lnn_errno(lua_State *L) {
  int rc = nn_errno();
  if(rc) {
    lua_pushinteger(L, rc);
  }
  return 1;
}

static const struct luaL_Reg connection_reg[] = {
  { "send", lnn_send },
  { "recv", lnn_recv },
  { "shutdown", lnn_shutdown },
  { NULL, NULL }
};

static const luaL_Reg funcs[] = {
  { "connect", lnn_connect },
  { "bind", lnn_bind },
  { "errno", lnn_errno },
  { NULL, NULL }
};

LUALIB_API int luaopen_nanomsg(lua_State *L) {
  lua_newtable(L);
  createmeta(L, "connection", connection_reg);

  luaL_setfuncs(L, funcs, 0);

  lua_pushliteral(L, LUANANOMSG_VERSION);
  lua_setfield(L, -2, "_VERSION");
  lua_pushliteral(L, LUANANOMSG_COPYRIGHT);
  lua_setfield(L, -2, "_COPYRIGHT");
  lua_pushliteral(L, LUANANOMSG_DESCRIPTION);
  lua_setfield(L, -2, "_DESCRIPTION");

  lnn_push_symbols(L);
  return 1;
}
