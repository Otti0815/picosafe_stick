#!/usr/bin/lua

require "os"
require "io"
require "posix"

function unescape (s)
  s = string.gsub(s, "+", " ")
  s = string.gsub(s, "%%(%x%x)", function (h)
        return string.char(tonumber(h, 16))
      end)
  return s
end

function decode (s)
  local cgi = {}
  for name, value in string.gfind(s, "([^&=]+)=([^&=]+)") do
    name = unescape(name)
    value = unescape(value)
    cgi[name] = value
  end

  return cgi
end

local content_length = tonumber(os.getenv("CONTENT_LENGTH"))
local cgi = decode(io.read(content_length))

-- try open luks container
if cgi["password"] ~= nil then
  local cmd = "/usr/sbin/cryptsetup-wrapper /dev/mmcblk0p3 emmentaler"
  local fh = io.popen(cmd, "w")
  fh:write(cgi["password"])
  fh:close()
end

print "Content-type: text/html\n\n"

--print "<pre>"

if posix.stat("/dev/mapper/emmentaler") then
  print "ok"
else
  print "wrong pw"
end

--print "</pre>"
