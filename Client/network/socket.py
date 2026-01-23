import socket
import struct
import threading
import tkinter

sock = None

def connetti_socket(ip, porta): # Imposta il socket globale "sock"
    global sock
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        sock.connect((ip, porta))
    except (ConnectionRefusedError, socket.timeout) as e:
        print("Errore di rete: ", e)

def invia_intero(num):
    global sock
    if(sock != None):
        sock.sendall(struct.pack("<i", num))
    else:
        print("Errore: socket non impostato")

def svuota_buffer():
    global sock
    sock.setblocking(False)
    try:
        while True:
            data = sock.recv(4096)
            if not data:
                break
    except BlockingIOError:
        pass
    finally:
        sock.setblocking(True)

def richiedi_dato(root):
    global sock
    svuota_buffer()
    risultato = None
    done = threading.Event()

    def ricevi():
        nonlocal risultato
        try:
            risultato = sock.recv(1024)  # BLOCCA QUI
        except OSError:
            risultato = None
        finally:
            done.set()

    threading.Thread(target=ricevi, daemon=True).start()

    while not done.is_set():
        try:
            root.update_idletasks()
            root.update()
        except tkinter.Tk.TclError:
            break

    return risultato

        
''' ROBA DA RISCRIVERE
def ricevi_stringa():
    global sock
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
