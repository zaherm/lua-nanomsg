local nanomsg = require("nanomsg")
local rcvtimeo = 100

local server_socket = nanomsg.socket(nanomsg.AP_SP, nanomsg.PAIR)
server_socket:setopt(nanomsg.SOL_SOCKET, nanomsg.RCVTIMEO, rcvtimeo)
local server = nanomsg.bind(server_socket, "tcp://127.0.0.1:9094")

local client_socket = nanomsg.socket(nanomsg.AP_SP, nanomsg.PAIR)
client_socket:setopt(nanomsg.SOL_SOCKET, nanomsg.RCVTIMEO, rcvtimeo)
local client = nanomsg.connect(client_socket, "tcp://127.0.0.1:9094")
os.execute("sleep 1")
client:send("hello from client")
while true do
    local server_msg = server:recv()
    local client_msg = client:recv()
    if server_msg then
        print("server_msg:", server_msg)
        server:send("hello from server")
    end
    if client_msg then
        print("client_msg:", client_msg)
        client:send("hello from client")
    end
    
end

server:shutdown()
client:shutdown()
