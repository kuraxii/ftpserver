# Simple ftp server


## How to install:

```
cd to the bulid dir
make
execute with ./server
```

## Commands that are currently implemented:

* USER PASS
* SYST USER PASS CWD PWD LIST PASV RETR STOR DELE RMD MKD QUIT SIZE FEAT TYPE

## TODO

* PORT ABOR

## tips
This server currently doesn't support ASCII mode but this sould not be a
problem with any modern system or ftp client.