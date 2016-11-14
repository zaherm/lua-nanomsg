local nanomsg = require("nanomsg")
local socket = nanomsg.socket(nanomsg.AF_SP, nanomsg.REQ)

assert(socket)
rcvtimeo = 100
socket:setopt(nanomsg.SOL_SOCKET, nanomsg.RCVTIMEO, rcvtimeo)
assert(socket:getopt(nanomsg.SOL_SOCKET, nanomsg.RCVTIMEO) == rcvtimeo)
print("done")

local conn = nanomsg.connect(socket, "tcp://127.0.0.1:1234")
assert(conn)
conn:send("ping")
print(conn:recv())
conn:shutdown()


