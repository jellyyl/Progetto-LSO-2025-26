import socket
import struct
import threading
import tkinter

def connetti_socket(sock, ip, porta):
    try:
        sock.connect((ip, porta))
        return 0
    except (ConnectionRefusedError, socket.timeout, OSError):
        return -1
    
def chiudi_socket(sock):
    sock.close()
    
def invia_intero(sock, num):
    if(sock != None):
        sock.sendall(struct.pack("<i", num))
    else:
        print("Errore: socket non impostato")
    
def svuota_buffer(sock):
    sock.setblocking(False)
    try:
        while True:
            data = sock.recv(4096)
            if not data:
                break
    except BlockingIOError, ConnectionAbortedError:
        pass
    finally:
        sock.setblocking(True)

def richiedi_dato(sock, root, timeout=None):  # Passa anche l'interfaccia in modo tale da non bloccarla
    if(sock != None):                         # timeout = None: socket bloccante
        svuota_buffer(sock)
        sock.settimeout(timeout)
        risultato = None
        done = threading.Event()

        def ricevi():
            nonlocal risultato
            try:
                risultato = sock.recv(4096)
                print(risultato) # DEBUGGGGGGGGGGGGGGG
            except socket.timeout, OSError:
                risultato = None
            finally:
                done.set()

        threading.Thread(target=ricevi, daemon=True).start()

        while not done.is_set():
            try:
                root.update_idletasks()
                root.update()
            except tkinter.TclError:
                break
        return risultato
    
def raw_a_int(dati_raw):
    if(dati_raw != None):
        return struct.unpack("<i", dati_raw)[0]
    else:
        return None

def raw_a_string(dati_raw):
    if(dati_raw != None):
        return dati_raw.decode("utf-8")
    else:
        return None
    
def recv_exact(sock, n):
    data = b''
    while len(data) < n:
        chunk = sock.recv(n - len(data))
        if not chunk:
            raise ConnectionError("Socket chiusa")
        data += chunk
    return data