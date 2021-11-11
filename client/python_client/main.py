from configHandler import loadConfigData
from clientClass import Client

def main():
    mainConfig = loadConfigData("../../config.json")
    PORT = mainConfig["PORT"]
    SERVER_IP = mainConfig["SERVER_IP"]
    DISCONNECT_MESSAGE = mainConfig["DISCONNECT_MESSAGE"]
    SERVER_ADDRESS = (SERVER_IP, PORT)
    
    client = Client(PORT, SERVER_IP, DISCONNECT_MESSAGE, SERVER_ADDRESS)

    client.send(input("Your message:\n"))
    responseType, response = client.receive(1024)
    print("[SERVER] " + str(response))

    client.send("", 0) # disconnect
    #responseType, response = client.receive(1024)
    #print(f"[SERVER] {response}")

if __name__ == "__main__":
    main()