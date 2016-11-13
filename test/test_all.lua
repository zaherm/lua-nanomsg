local nanomsg = require("nanomsg")

print("version:", nanomsg._VERSION)
print("copyright:", nanomsg._COPYRIGHT)
print("description:", nanomsg._DESCRIPTION)


for k,v in pairs(nanomsg) do
  print("k=", k, "v=", v)
end
