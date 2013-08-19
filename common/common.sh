CROSSCOMPILER="/opt/eldk-5.2.1/picosafe.sh"

createpem() 
{
  PEM="$1"
  CERT="`mktemp`"
  [ -e "$PEM" ] || touch "$PEM"
  chmod 600 "$PEM" "$CERT"

  openssl req -new -newkey rsa:4096 -days 365 -nodes -x509 -subj "/C=US/ST=Denial/L=Springfield/O=Dis/CN=172.24.42.1" -keyout "$PEM" -out "$CERT" 2> /dev/null
  echo "" >> "$PEM"
  cat "$CERT" >> "$PEM"
  echo "" >> "$PEM"
  rm "$CERT"
}

drop_priviledges()
{
  if [ $UID -eq "0" ]; then
      exec sudo -u $SUDO_USER "$0" $@
  fi
}


set_crosscompiler()
{
  # set environment for cross-compiler
  if [ "$ARCH" != "arm" ];
  then
    if [ -e "$CROSSCOMPILER" ];
    then
      . "$CROSSCOMPILER"
    else
      echo "Can't set crosscompiler" > /dev/stderr
      exit 1
    fi
  fi
}

check_root()
{
  if [ $EUID != 0 ];
  then
    echo "You need to be root." > /dev/stderr
    exit 1
  fi
}

lc()
{
  echo "$1" | tr '[:upper:]' '[:lower:]'
}

uc()
{
  echo "$1" | tr '[:lower:]' '[:upper:]'
}
