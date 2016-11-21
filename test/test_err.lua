local nanomsg = require("nanomsg")
local socket = nanomsg.socket(nanomsg.AF_SP, nanomsg.REQ)

assert(socket)
rcvtimeo = 100
--socket:setopt(nanomsg.SOL_SOCKET, nanomsg.RCVTIMEO, rcvtimeo)
--assert(socket:getopt(nanomsg.SOL_SOCKET, nanomsg.RCVTIMEO) == rcvtimeo)

--local conn = socket:connect("tcp://127.0.0.1:1234")
--local errno = nanomsg.errno()
--assert(errno == nanomsg.EINPROGRESS)
--local errstr = nanomsg.strerror(errno)
--assert(errstr == "Operation now in progress")

