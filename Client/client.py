import gui.interface as gui
import network.socket as net
import socket
import re
import random

# ---------------------------- CONFIG ----------------------------
ip = "127.0.0.1"
porta = 5200

# ---------------------------- GLOBAL ----------------------------
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)    # Socket del client
game_id = None      # ID della partita che si sta giocando
agg_var = None

# ---------------------------- EVENTI ----------------------------
def on_start():
    attesa = gui.mostra_attesa("Connettendo")

    def connetti():
        conn_test = net.connetti_socket(sock, ip, porta)
        gui.nascondi_finestra(attesa)
        if(conn_test == -1):
            gui.mostra_errore("Connessione al server fallita", "Riprova", on_start, on_esci_errore)
        else:
            attiva_aggiornamento()

    gui.root.after(1400, connetti)

def on_crea_partita():
    disattiva_aggiornamento()
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
            if(conferma[0:13] == "START_PLAYER1"):
                gui.nascondi_finestra(finestra_attesa)
                finestra = gui.mostra_partita('X', on_click_cella)
                loop_partita(0, finestra)
                

def on_connetti(partita):
    disattiva_aggiornamento()
    global game_id
    net.invia_intero(sock, 3)   # (JOIN)
    id_partita = int(partita[0])
    game_id = id_partita
    net.invia_intero(sock, id_partita)

    finestra_attesa = gui.mostra_attesa("In attesa di conferma")
    conferma = net.raw_a_string(net.richiedi_dato(sock, gui.root, None))
    gui.nascondi_finestra(finestra_attesa)
    if(conferma[0:7] == "JOIN_OK"):
        finestra = gui.mostra_partita('O', on_click_cella)
        loop_partita(1, finestra)
    elif(conferma[0:11] == "JOIN_DENIED"):
        gui.mostra_errore("L'host ha rifiutato")
    else:
        gui.mostra_errore("Si è verificato un errore")
    gui.nascondi_finestra(finestra_attesa)

def on_click_cella(r, c):
    invia_mossa(r, c)

def on_esci():
    net.chiudi_socket(sock)
    disattiva_aggiornamento()
    gui.root.destroy()
    exit()

def on_esci_errore():
    net.chiudi_socket(sock)
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

def aggiorna_griglia(str_griglia): # Da rimuovere in seguito (arriverà dal server la stringa già pulita)
    str_clean = re.sub(r'[^XO ]', '', str_griglia)
    for i in range(0, 9):
        if(str_clean[i] == ' '):
            continue
        else:
            cella = divmod(i, 3)
            x = cella[0]
            y = cella[1]
            gui.riempi_cella_partita(str_clean[i], x, y)

def loop_partita(giocatore, finestra_partita):    # giocatore = 0 se creatore, 1 se sfidante
    cmd = 0
    while(cmd != 3): # (CMD_OVER)
        cmd_raw = net.richiedi_dato(sock, gui.root, None)
        cmd = net.raw_a_int(cmd_raw[0:4])
        str_griglia = ""

        match(cmd):
            case 1: # (CMD_PLAY)
                str_griglia = net.raw_a_string(cmd_raw[5:4096])
                if(str_griglia != ""):
                    aggiorna_griglia(str_griglia)
                gui.abilita_griglia_partita()
                gui.aggiorna_label_partita("È il tuo turno", "#ff4d4d" if giocatore == 0 else "#4da6ff", False)
                
            case 2: # (CMD_WAIT)
                gui.disabilita_griglia_partita()
                str_griglia = net.raw_a_string(cmd_raw[5:4096])
                if(str_griglia != ""):
                    aggiorna_griglia(str_griglia)
                gui.aggiorna_label_partita("Turno dell'avversario", "#ff4d4d" if giocatore == 1 else "#4da6ff", True)
            case 3: # (CMD_OVER)
                gui.disabilita_griglia_partita()
                str_griglia = net.raw_a_string(cmd_raw[5:4096])
                if(str_griglia != ""):
                    aggiorna_griglia(str_griglia)

                if("vinto" in str_griglia):
                    gui.aggiorna_label_partita("Hai vinto!", "#5ECF4D", False)
                elif("CAZZ" in str_griglia):
                    gui.aggiorna_label_partita("Hai perso!", "#ff4d4d", False)
                
                def nascondi_partita():
                    gui.nascondi_finestra(finestra_partita)
                    gui.mostra_errore("REMATCH DA GESTIRE", "OK", on_esci_errore)

            
                gui.root.after(2000, nascondi_partita)
                # QUA GESTIRE REMATCH

            case 4: # (CMD_INVALID)   
                print("INVALIDO")
            case _:
                print("IGNOTO") 
    print("FINEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE")       

def invia_mossa(r, c):
    net.invia_intero(sock, 4) # (MOVE)
    net.invia_intero(sock, game_id)
    net.invia_intero(sock, r)
    net.invia_intero(sock, c)

def attiva_aggiornamento():
    global agg_var
    aggiorna_partite()
    agg_var = gui.root.after(2000, attiva_aggiornamento)

def disattiva_aggiornamento():
    global agg_var
    gui.root.after_cancel(agg_var)

# ----------------------------- MAIN -----------------------------
gui.root.after(200, on_start)
gui.mostra_home("", on_crea_partita, on_connetti, on_esci, on_focus, on_unfocus)