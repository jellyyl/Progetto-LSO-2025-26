import gui.interface as gui
import network.socket as net
import socket
import re

# ---------------------------- CONFIG ----------------------------
ip = "127.0.0.1"
porta = 5200

# ---------------------------- GLOBAL ----------------------------
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)    # Socket del client
game_id = None      # Id della partita che si sta giocando
# turno_offset = 0    # 0 se si sta giocando come creatore, 2 se come sfidante

# ---------------------------- EVENTI ----------------------------
def on_start():
    attesa = gui.mostra_attesa("Connettendo")

    def connetti():
        conn_test = net.connetti_socket(sock, ip, porta)
        gui.nascondi_finestra(attesa)
        if(conn_test == -1):
            gui.mostra_errore("Connessione al server fallita", "Riprova", on_start, on_esci)
        else:
            aggiorna_partite() # AD HOME

    gui.root.after(1400, connetti)

def on_crea_partita():
    global game_id
    net.invia_intero(sock, 1)   # (CREATE)
    scelta = 1
    while(scelta == 1):
        finestra_attesa = gui.mostra_attesa("In attesa di un giocatore")

        raw = net.richiedi_dato(sock, gui.root, None)
        req = net.raw_a_string(raw[0:12])
        game_id = net.raw_a_int(raw[13:17])

        if(req == "JOIN_REQUEST"):
            scelta = gui.mostra_scelta("Un giocatore vuole unirsi") - 1

            net.invia_intero(sock, 6)   # (APPROVE)
            net.invia_intero(sock, game_id)
            net.invia_intero(sock, scelta)

        if(scelta == 0):
            conferma = net.raw_a_string(net.richiedi_dato(sock, gui.root, None))
            if(conferma == "START_PLAYER1"):
                gui.nascondi_finestra(finestra_attesa)
                gui.mostra_partita(1, on_click_cella)
                # INIZI TU
                

def on_connetti(partita):
    global game_id
    net.invia_intero(sock, 3)   # (JOIN)
    id_partita = int(partita[0])
    game_id = id_partita
    net.invia_intero(sock, id_partita)

    finestra_attesa = gui.mostra_attesa("In attesa di conferma")
    conferma = net.raw_a_string(net.richiedi_dato(sock, gui.root, None))
    gui.nascondi_finestra(finestra_attesa)
    if(conferma == "JOIN_OK"):
        gui.mostra_partita(2, on_click_cella)
        
    elif(conferma == "JOIN_DENIED"):
        gui.mostra_errore("L'host ha rifiutato")
    else:
        gui.mostra_errore("Si è verificato un errore")
    gui.nascondi_finestra(finestra_attesa)

def on_click_cella(r, c):
    invia_mossa(game_id, r, c)
    net.richiedi_dato(sock, gui.root, None)

def on_esci():
    net.chiudi_socket(sock)
    #gui.disattiva_aggiornamento()
    gui.root.destroy()
    exit()

def on_focus(event):
    pass
    # gui.attiva_aggiornamento()
    # print("Focus su home")

def on_unfocus(event):
    pass
    # gui.disattiva_aggiornamento()
    # print("Unfocus su home")

# --------------------------- FUNZIONI ---------------------------
def aggiorna_partite():
    net.invia_intero(sock, 2)
    stringa_raw = net.richiedi_dato(sock, gui.root, 1)
    stringa_partite = net.raw_a_string(stringa_raw) if stringa_raw != None else ""
    gui.aggiorna_partite(stringa_partite, on_connetti)

def aggiorna_griglia(str_griglia):
    str_clean = re.sub(r'[^XO ]', '', str_griglia)[0:9]
    for i in range(0, 9):
        if(str_clean[i] == ' '):
            continue
        else:
            cella = divmod(i, 3)
            x = cella[0]
            y = cella[1]
            gui.riempi_cella_partita(str_clean[i], x, y)

def set_turno(turno):   # turno = {0,1} per chi ha creato la partita, {2,3} per chi ha joinato
    if(turno%2 == 0):
        gui.abilita_griglia_partita()
        colore = "#ff4d4d" if turno == 0 else "#4da6ff"
        gui.aggiorna_label_partita("È il tuo turno", colore, False)
    elif(turno%2 == 1):
        gui.disabilita_griglia_partita()
        colore = "#ff4d4d" if turno == 3 else "#4da6ff"
        gui.aggiorna_label_partita("Turno dell'avversario", colore, True)

def inizia_partita():
    cmd = None
    while(True):
        cmd = net.raw_a_int(net.richiedi_dato(sock, gui.root, None))
        str_griglia = net.raw_a_string(net.richiedi_dato(sock, gui.root, None))
        aggiorna_griglia(str_griglia)

        match(cmd):
            case 1: # (CMD_PLAY)
                
            case 2: # (CMD_WAIT)
            case 3: # (CMD_OVER)
            case 4: # (CMD_INVALID)           

def invia_mossa(r, c):
    net.invia_intero(sock, 4) # (MOVE)
    net.invia_intero(sock, game_id)
    net.invia_intero(sock, r)
    net.invia_intero(sock, c)

# ----------------------------- MAIN -----------------------------
gui.root.after(200, on_start)
gui.mostra_home("", on_crea_partita, on_connetti, on_esci, on_focus, on_unfocus)