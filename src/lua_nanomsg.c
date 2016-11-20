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

LUALIB_API void lnn_setmeta(lua_State *L, const char *name) {
  luaL_getmetatable(L, name);
  lua_setmetatable(L, -2);
}

LUALIB_API int lnn_createmeta(lua_State *L, const char *name, const luaL_Reg *methods) {
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

/*** s: socket ***/
lnn_socket_t* lnn_get_socket(lua_State *L, int index) {
  lnn_socket_t *s = (lnn_socket_t *) luaL_checkudata(L, index, "socket");
  luaL_argcheck(L, s != NULL, index, "socket expected");
  return s;
}

LUALIB_API int lnn_socket(lua_State *L) {
  int domain = luaL_optnumber(L, 1, AF_SP);
  int protocol = luaL_optnumber(L, 2, NN_REQ);
  int fd = nn_socket(domain, protocol);
  if(fd >= 0) {
    lnn_socket_t *s = (lnn_socket_t *) lua_newuserdata(L, sizeof(lnn_socket_t));
    s->domain = domain;
    s->protocol = protocol;
    s->fd = fd;
    lnn_setmeta(L, "socket");
    return 1;
  }
  return 0;
}

LUALIB_API int lnn_socket_setopt(lua_State *L) {
  lnn_socket_t *s = lnn_get_socket(L, 1);
  int level = luaL_checkint(L, 2);
  int option = luaL_checkint(L, 3);
  int opttype = lua_type(L, 4);
  switch(opttype) {
    case LUA_TSTRING: {
      size_t optvallen;
      const char* optval = luaL_checklstring(L, 4, &optvallen);
      nn_setsockopt(s->fd, level, option, optval, optvallen);
      break;
    }
    case LUA_TNUMBER: {
      int optval = luaL_checkint(L, 4);
      nn_setsockopt(s->fd, level, option, &optval, sizeof(optval));
      break;
    }
    default: {
      luaL_error(L, "unkown type");
      break;
    }
  }
  return 0;
}

/* int nn_getsockopt (int s, int level, int option,
 *                    void *optval, size_t *optvallen);
 */

LUALIB_API int lnn_socket_getopt(lua_State *L) {
  lnn_socket_t *s = lnn_get_socket(L, 1);
  int level = luaL_checkint(L, 2);
  int option = luaL_checkint(L, 3);
    int rc = -1;
  switch(option) {
    case NN_LINGER:
    case NN_SNDBUF:
    case NN_RCVBUF:
    case NN_SNDTIMEO:
    case NN_RCVTIMEO:
    case NN_RECONNECT_IVL:
    case NN_RECONNECT_IVL_MAX:
    case NN_SNDPRIO:
    case NN_RCVPRIO:
    case NN_SNDFD:
    case NN_RCVFD:
    case NN_DOMAIN:
    case NN_PROTOCOL:
    case NN_IPV4ONLY:
    case NN_RCVMAXSIZE:
    case NN_MAXTTL: {
      int optval;
      size_t optvallen = sizeof(optval);
      rc = nn_getsockopt(s->fd, level, option, &optval, &optvallen);
      if(rc != -1) {
        lua_pushinteger(L, optval);
      }
      break;
    }
    case NN_SOCKET_NAME: {
      char buf[8];
      size_t sz = sizeof(buf);
      rc = nn_getsockopt(s->fd, level, option, &buf, &sz);
      printf("ccc = %s", buf);
      if(rc != -1) {
        lua_pushlstring(L, buf, sz);
      }
      break;
    }
  }
  if(rc == -1) {
    lua_pushnil(L);
  }
  return 0;
}

/*** e: socket ***/

LUALIB_API lnn_connection_t *lnn_get_connection(lua_State *L, size_t index) {
  lnn_connection_t *c = (lnn_connection_t *) luaL_checkudata(L, index, "connection");
  luaL_argcheck(L, c != NULL, index, "connection expected");
  return c;
}

LUALIB_API void lnn_push_connection(lua_State *L, lnn_socket_t *socket, int eid) {
  lnn_connection_t *c = (lnn_connection_t *) lua_newuserdata(L, sizeof(lnn_connection_t));
  c->eid = eid;
  c->socket = socket;
  lnn_setmeta(L, "connection");
}


LUALIB_API int lnn_bind(lua_State *L) {
  lnn_socket_t *socket = lnn_get_socket(L, 1);
  const char *addr = luaL_checkstring(L, 2);

  if(socket != NULL && socket->fd >= 0) {
    int eid = nn_bind(socket->fd, addr);
    if(eid >= 0) {
      lnn_push_connection(L, socket, eid);
      return 1;
    }
  }
  return 0;
}

LUALIB_API int lnn_connect(lua_State *L) {
  lnn_socket_t *socket = lnn_get_socket(L, 1);
  const char *addr = luaL_checkstring(L, 2);

  if(socket != NULL && socket->fd >= 0) {
    int eid = nn_connect(socket->fd, addr);
    if(eid >= 0) {
      lnn_push_connection(L, socket, eid);
      return 1;
    }
  }
  return 0;
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
    return 1;
  }
  return 0;
}

LUALIB_API int lnn_strerror(lua_State *L) {
  int errnum = luaL_checkint(L, 1);
  if(errnum) {
    const char* err = nn_strerror(errno);
    if(err) {
      lua_pushstring(L, err);
      return 1;
    }
  }
  return 0;
}

static const struct luaL_Reg connection_reg[] = {
  { "send", lnn_send },
  { "recv", lnn_recv },
  { "shutdown", lnn_shutdown },
  { NULL, NULL }
};

static const luaL_Reg nanomsg_reg[] = {
  { "connect", lnn_connect },
  { "bind", lnn_bind },
  { "errno", lnn_errno },
  { "strerror", lnn_strerror },
  { "socket", lnn_socket },
  { NULL, NULL }
};

/* socket methods */
static const struct luaL_Reg socket_reg[] = {
  { "setopt", lnn_socket_setopt },
  { "getopt", lnn_socket_getopt },
  { NULL, NULL }
};

LUALIB_API int luaopen_nanomsg(lua_State *L) {
  lua_newtable(L);

  lnn_createmeta(L, "connection", connection_reg);
  lnn_createmeta(L, "socket", socket_reg);
  lnn_push_symbols(L);
  luaL_setfuncs(L, nanomsg_reg, 0);

  lua_pushliteral(L, LUANANOMSG_VERSION);
  lua_setfield(L, -2, "_VERSION");
  lua_pushliteral(L, LUANANOMSG_COPYRIGHT);
  lua_setfield(L, -2, "_COPYRIGHT");
  lua_pushliteral(L, LUANANOMSG_DESCRIPTION);
  lua_setfield(L, -2, "_DESCRIPTION");

  return 1;
}
