lua-nanomsg
===
Lua binding for nanomsg library.

Example
===
The below code demonstrates a clinet-server pair, sending and receiving 5 messages.

```lua
local nanomsg = require("nanomsg")
local rcvtimeo = 100
-- create a server socket with 100msec receive timeout
local server = nanomsg.socket(nanomsg.AP_SP, nanomsg.PAIR)
server:setopt(nanomsg.SOL_SOCKET, nanomsg.RCVTIMEO, rcvtimeo)
local server_ok, _ = server:bind("tcp://127.0.0.1:9094")
assert(server_ok)
-- create a client socket with 100msec receive timeout
local client = nanomsg.socket(nanomsg.AP_SP, nanomsg.PAIR)
client:setopt(nanomsg.SOL_SOCKET, nanomsg.RCVTIMEO, rcvtimeo)
local client_ok, _ = client:connect("tcp://127.0.0.1:9094")
assert(client_ok)
local server_ack, client_ack = false, false
local i = 0
while i < 5 do
  local server_msg = server:recv()
  local client_msg = client:recv()
  if server_msg then
    print("server_msg:", server_msg)
    server:send("hello from server")
    server_ack = true
  end
  if client_msg then
    print("client_msg:", client_msg)
    client:send("hello from client")
    client_ack = true
  end
  -- if both client and server received the message.
  if server_ack and client_ack then
    i = i + 1
    server_ack, client_ack = false, false
  end
end

-- shutdown both sockets.
server:shutdown()
client:shutdown()
```

Doc
===

nanomsg.socket(domain, protocol)
==
Create an a SP socket.
Returns a socket object if the function succeeds, otherwise error will be triggered.

socket:setopt(level, option, option_value)
==
Set a socket option.
See http://nanomsg.org/v1.0.0/nn_setsockopt.3.html for list of options/values. Refer to options with the _NN__ prefix.

* level (int) - specifies the protocol level at which the option resides. For generic options use SOL_SOCKET.
* option (int) - option identifier.
* option_value (int/string) - sets the option value, string or int values. otherwise an error will be triggered.

socket:getopt(level, option)
==

See: http://nanomsg.org/v1.0.0/nn_getsockopt.3.html, the same applies regarding the _NN__ prefix.

* level (int) - specifies the protocol level at which the option resides. For generic options use SOL_SOCKET.
* option (int) - option identifier.
* returns int/string - value of the retrieved option

socket:bind(addr)
==

Binds an endpoint to local socket.
See : http://nanomsg.org/v1.0.0/nn_bind.3.html

* addr (string) - the addr consists of two parts _transport_ :// _address_.
* returns true on success, error on failure.

socket:sconnect(addr)
==

Connects socket to an endpoint.
See : http://nanomsg.org/v1.0.0/nn_bind.3.html

* addr (string) - the addr consists of two parts _transport_ :// _address_.
* returns true on success, error on failure.

socket:recv()
==
Receive from socket.

* returns (string) 

socket:send(message)
==

Send a message.

* message (string)
* returns true on success, error on failure.

socket:shutdown()
==

Shuts down a socket.
See: http://nanomsg.org/v1.0.0/nn_shutdown.3.html


