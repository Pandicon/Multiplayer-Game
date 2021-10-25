import socket

from configHandler import loadConfigData
from clientClass import Client

def main():
    mainConfig = loadConfigData("../config.json")
    HEADER = mainConfig["HEADER"]
    PORT = mainConfig["PORT"]
    SERVER_IP = mainConfig["SERVER_IP"]
    ENCODE_FORMAT = mainConfig["ENCODE_FORMAT"]
    DISCONNECT_MESSAGE = mainConfig["DISCONNECT_MESSAGE"]
    SERVER_ADDRESS = (SERVER_IP, PORT)
    
    client = Client(HEADER, PORT, SERVER_IP, ENCODE_FORMAT, DISCONNECT_MESSAGE, SERVER_ADDRESS)

    response = client.send(input("Your message:\n"))
    print(f"[SERVER] {response}")

    response = client.send(DISCONNECT_MESSAGE)
    print(f"[SERVER] {response}")

if __name__ == "__main__":
    main()