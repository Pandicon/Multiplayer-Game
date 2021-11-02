import urllib.request
import socket

def getPublicIP():
	return urllib.request.urlopen('https://api.ipify.org/').read().decode('utf8')

def getLocalIP():
	return socket.gethostbyname(socket.gethostname())