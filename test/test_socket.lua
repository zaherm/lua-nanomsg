local nanomsg = require("nanomsg")
local rcvtimeo = 100

local server = nanomsg.socket(nanomsg.AP_SP, nanomsg.PAIR)
server:setopt(nanomsg.SOL_SOCKET, nanomsg.RCVTIMEO, rcvtimeo)
local server_ok, _ = server:bind("tcp://127.0.0.1:9094")
print("server_ok", server_ok)
assert(server_ok)
local client = nanomsg.socket(nanomsg.AP_SP, nanomsg.PAIR)
client:setopt(nanomsg.SOL_SOCKET, nanomsg.RCVTIMEO, rcvtimeo)
local client_ok, _ = client:connect("tcp://127.0.0.1:9094")
assert(client_ok)
os.execute("sleep 1")
client:send("hello from client")
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
  if server_ack and client_ack then
    i = i + 1
    server_ack, client_ack = false, false
  end
end

server:close()
client:close()
server:shutdown()
client:shutdown()

