import socket
import struct

port = 5200
host = '127.0.0.1'

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

try:
    s.connect((host, port))

    num = int(input("Inserisci un intero: "))
    s.sendall(struct.pack("<i", num)) # Codifica del numero in un binario compatibile con gli int del C ("<i" = Little endian, int32)
    
    data_in = s.recv(4)
    num_in = struct.unpack("<i", data_in)[0]

    print(num_in)

    s.close()

except (ConnectionRefusedError, socket.timeout) as e:
    print("Errore di rete: ", e)
