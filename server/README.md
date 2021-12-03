This is Multiplayer-Game server.
# How to use
You need python 3+ first. Then change the values inside /config.json (not /server/config.json):
value name | effect
--- | ---
HEADER | port for header server (not implemented)
PORT | server port
SERVER_MODE | can be localhost (server IP is 127.0.0.1), local (server IP is set by machine's LAN IP) or public (public IP)
ENCODE_FORMAT | message encoding format (should be kept on utf-8)

Then you can run main.py with
```shell
$ python3 main.py
```
or any other way you want.