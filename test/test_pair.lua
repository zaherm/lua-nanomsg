local nanomsg = require("nanomsg")

print("version:", nanomsg._VERSION)
print("copyright:", nanomsg._COPYRIGHT)
print("description:", nanomsg._DESCRIPTION)
addr = "ipc:///tmp/survey.ipc"

function log(role, id, msg)
  local out = string.format(">> %s[%s] - %s", role, id, msg)
  print(out)
end

function parse(msg)
  return string.match(msg, "cid=(%d+),mid=(%d+),t=(%w+)")
end

function mkserver()
  local s = nanomsg.bind(addr, nanomsg.AF_SP, nanomsg.PAIR)
  local msgid = 1
  local msg
  local clients = {}
  print(">> server - started")
  while true do
    msg = s:recv()
    if msg then
      log("server", "master", msg)
      local cid, mid, t = parse(msg)
      print(cid,mid,t)
      s:send("cid="..cid..",mid="..mid..",t=ack")
    end
  end
  s:shutdown()
  print(">> server - done")
end

function mkclient(id)
  local c = nanomsg.connect(addr, nanomsg.AF_SP, nanomsg.PAIR)
  local i = 0
  local msg
  print(">> client["..id.."] - started")
  while i < 10 do
    c:send("cid="..id..",mid="..i..",t=msg")
    msg = c:recv()
    if msg then
      local cid, mid, t = parse(msg)
      print(cid,mid,t)
      i = i + 1
    end
  end
  c:shutdown()
  print(">> client["..id.."] - done")
end

local role = arg[1]
local id = arg[2]
if not role then
  command = "lua test/test_pair.lua server &"
  os.execute(command)
  for i = 1,10 do
    command = "lua test/test_pair.lua client "..i.." &"
    print(command)
    os.execute(command)
  end
else
  if role == "client" then
    local sleep = math.floor(math.random()*10) - id
    mkclient(id)
    os.execute("sleep "..sleep)
  end
  if role == "server" then
    mkserver()
  end
end

