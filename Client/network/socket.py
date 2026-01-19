import socket
import struct


def connetti_socket(ip, porta): # Imposta il socket "sock"
    global sock
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        sock.connect((ip, porta))
    except (ConnectionRefusedError, socket.timeout) as e:
        print("Errore di rete: ", e)

def invia_intero(num):
    if(sock != None):
        sock.sendall(struct.pack("<i", num))
    else:
        print("Errore: socket non impostato")

def ricevi_stringa():
    if(sock != None):
        # Riceve e decodifica la lunghezza della stringa
        lun = sock.recv(4)
        lun = struct.unpack("<i", lun)[0]

        # Compone man mano la stringa
        str = b""
        while(len(str) < lun):
            char = sock.recv(lun - len(str))
            if not char:
                return None
            str += char
        return str.decode("utf-8")
    else:
        print("Errore: socket non impostato")

'''
def richiedi_lista_partite():
    global sock
    if(sock != None):
        print("OK!")
    else:
        print("Errore: socket non impostato")

def invia_crea_partita():
    global sock
    if(sock != None):
        print("OK!")
    else:
        print("Errore: socket non impostato")

def attendi_giocatore():
    global sock
    if(sock != None):
        print("OK!")
    else:
        print("Errore: socket non impostato")

'''
