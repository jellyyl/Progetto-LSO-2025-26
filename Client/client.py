import gui.interface as gui
import network.socket as net
import socket
from enum import IntEnum

# ---------------------------- CONFIG ----------------------------
ip = "127.0.0.1"
porta = 5200

# ---------------------------- GLOBAL ----------------------------
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)    # Socket del client
game_id = None      # ID della partita che si sta giocando
agg_var = None

# ----------------------------- ENUM -----------------------------
class Actions(IntEnum):
    CREATE = 0
    LIST = 1
    JOIN = 2
    MOVE = 3
    REMATCH = 4
    APPROVE = 5
    CANCEL = 6

class ResponseCode(IntEnum):
    MSG_JOIN_OK = 200
    MSG_JOIN_REQUEST = 201
    MSG_JOIN_DENIED = 202
    MSG_START_PLAYER1 = 300
    MSG_START_PLAYER2 = 301
    MSG_CANCELLED = 302
    MSG_CHANGE_OWNER = 303
    MSG_REMATCH_REQUEST = 400
    MSG_REMATCH_DECLINED = 401
    ERR_JOIN_NOT_FOUND = 500
    ERR_JOIN_OWNER_LEFT = 501
    ERR_APPROVE_NOT_FOUND = 502
    ERR_REMATCH_NOT_FOUND = 503
    ERR_REMATCH_NOT_PLAYER = 504
    ERR_REMATCH_NOT_FINISHED = 505
    ERR_REMATCH_NOT_OWNER = 506

class GameCommand(IntEnum):
    UNKNOWN = 0
    PLAY = 1
    WAIT = 2
    OVER = 3
    WIN = 4
    DRAW = 5
    LOSE = 6
    INVALID = 7
    QUIT = 8

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
    global game_id
    disattiva_aggiornamento()
    net.invia_intero(sock, Actions.CREATE)
    attendi_sfidante()

def attendi_sfidante():
    global game_id
    print(game_id)
    scelta = 1
    while(scelta == 1):
        finestra_attesa = gui.mostra_attesa("In attesa di un giocatore")

        raw = net.richiedi_dato(sock, gui.root)
        if raw is None: 
            return

        req = net.raw_a_int(raw[0:4])

        # The join request logic in server: sends [MSG_JOIN_REQUEST, game_id] (2 ints)
        if req == ResponseCode.MSG_JOIN_REQUEST:
            game_id = net.raw_a_int(raw[4:8])
            scelta = gui.mostra_scelta("Un giocatore vuole unirsi")

            net.invia_intero(sock, Actions.APPROVE)
            net.invia_intero(sock, game_id)
            net.invia_intero(sock, scelta)

        if(scelta == 0):
            conferma = net.richiedi_intero(sock, gui.root)
            if conferma == ResponseCode.MSG_START_PLAYER1:
                gui.nascondi_finestra(finestra_attesa)
                finestra = gui.mostra_partita('X', on_click_cella)
                loop_partita(0, finestra)

def on_connetti(partita):
    disattiva_aggiornamento()
    global game_id
    net.invia_intero(sock, Actions.JOIN)
    id_partita = int(partita[0])
    game_id = id_partita
    net.invia_intero(sock, id_partita)

    finestra_attesa = gui.mostra_attesa("In attesa di conferma")
    conferma = net.richiedi_intero(sock, gui.root)
    
    gui.nascondi_finestra(finestra_attesa)
    
    if conferma == ResponseCode.MSG_JOIN_OK:
        finestra = gui.mostra_partita('O', on_click_cella)
        loop_partita(1, finestra)
    elif conferma == ResponseCode.MSG_JOIN_DENIED:
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
    net.invia_intero(sock, Actions.LIST)
    stringa_raw = net.richiedi_dato(sock, gui.root, 1)
    stringa_partite = net.raw_a_string(stringa_raw) if stringa_raw != None else ""
    gui.aggiorna_partite(stringa_partite, on_connetti)

def aggiorna_griglia(str_griglia): # Da rimuovere in seguito (arriverà dal server la stringa già pulita)
    for i in range(0, 9):
        if(str_griglia[i] == ' '):
            continue
        else:
            cella = divmod(i, 3)
            x = cella[0]
            y = cella[1]
            gui.riempi_cella_partita(str_griglia[i], x, y)

def loop_partita(giocatore, finestra_partita):    # giocatore = 0 se creatore, 1 se sfidante
    cmd = GameCommand.UNKNOWN
    
    # Loop finché non arriva un comando terminale o quit
    while cmd not in [GameCommand.WIN, GameCommand.LOSE, GameCommand.DRAW, GameCommand.QUIT]:
        cmd_raw = net.richiedi_dato(sock, gui.root)
        if cmd_raw is None: 
            break
            
        cmd = net.raw_a_int(cmd_raw[0:4])
        str_griglia = ""

        match(cmd):
            case GameCommand.PLAY:
                str_griglia = net.raw_a_string(cmd_raw[4:4096])
                if(str_griglia != ""):
                    aggiorna_griglia(str_griglia)
                gui.abilita_griglia_partita()
                gui.aggiorna_label_partita("È il tuo turno", "#ff4d4d" if giocatore == 0 else "#4da6ff", False)
                
            case GameCommand.WAIT:
                gui.disabilita_griglia_partita()
                str_griglia = net.raw_a_string(cmd_raw[4:4096])
                if(str_griglia != ""):
                    aggiorna_griglia(str_griglia)
                gui.aggiorna_label_partita("Turno dell'avversario", "#ff4d4d" if giocatore == 1 else "#4da6ff", True)
            
            case GameCommand.WIN | GameCommand.LOSE | GameCommand.DRAW as end_cmd:
                gui.disabilita_griglia_partita()
                str_griglia = net.raw_a_string(cmd_raw[4:4096])
                if(str_griglia != ""):
                    aggiorna_griglia(str_griglia)

                match(end_cmd):
                    case GameCommand.WIN:
                        gui.aggiorna_label_partita("Hai vinto!", "#5ecf4d", False)
                    case GameCommand.LOSE:
                        gui.aggiorna_label_partita("Hai perso!", "#ff4d4d", False)
                    case GameCommand.DRAW:
                        gui.aggiorna_label_partita("Pareggio!", "#aaaaaa", False)
                
                def gestisci_rematch():
                    gui.nascondi_finestra(finestra_partita)
                    if(end_cmd != GameCommand.LOSE): 
                        msg = net.richiedi_intero(sock, gui.root)
                        print(msg)
                        add_text = ""
                        if(msg == ResponseCode.MSG_CHANGE_OWNER):
                            add_text = "Sei diventato il proprietario della partita! "
                            msg = net.richiedi_intero(sock, gui.root)

                        if(msg == ResponseCode.MSG_REMATCH_REQUEST):
                            print(msg)
                            scelta = 1 - gui.mostra_scelta((add_text + "Vuoi rimanere in attesa di un nuovo sfidante?") 
                                                            if end_cmd != GameCommand.DRAW 
                                                            else "Vuoi rigiocare con questo sfidante?")  # "1-" da levare
                            net.invia_intero(sock, Actions.REMATCH)
                            net.invia_intero(sock, game_id)
                            net.invia_intero(sock, scelta)
                            if(end_cmd == GameCommand.DRAW and scelta == 1):
                                finestra_attesa = gui.mostra_attesa("Attendendo la risposta dello sfidante")
                                msg = net.richiedi_intero(sock, gui.root)
                                gui.nascondi_finestra(finestra_attesa)

                                if(msg == ResponseCode.MSG_REMATCH_DECLINED):
                                    gui.mostra_errore("Lo sfidante ha rifiutato")
                                elif(msg == ResponseCode.MSG_START_PLAYER1):
                                    new_finestra = gui.mostra_partita('X', on_click_cella)
                                    loop_partita(1, new_finestra)
                                elif(msg == ResponseCode.MSG_START_PLAYER2):
                                    new_finestra = gui.mostra_partita('O', on_click_cella)
                                    loop_partita(0, new_finestra)
                            else:
                                if(scelta == 1):
                                    attendi_sfidante()
                                else:
                                    attiva_aggiornamento()
                    else:
                        attiva_aggiornamento() # GESTIRE ATTIVA AGGIORNAMENTO DOPO REMATCH

                gui.root.after(2000, gestisci_rematch)

            case GameCommand.INVALID:   
                print("Mossa Non Valida")
                
            case GameCommand.QUIT:
                print("Opponent Quit")
                gui.mostra_errore("L'avversario si è disconnesso", "Esci", on_esci_errore)
                
            case _:
                print(f"Comando ignoto: {cmd}") 
    print("PARTITA TERMINATA")       

def invia_mossa(r, c):
    global game_id
    net.invia_intero(sock, Actions.MOVE)
    net.invia_intero(sock, game_id)
    net.invia_intero(sock, r)
    net.invia_intero(sock, c)

def attiva_aggiornamento():
    global agg_var
    aggiorna_partite()
    agg_var = gui.root.after(2000, attiva_aggiornamento)

def disattiva_aggiornamento():
    global agg_var
    if(agg_var != None):
        gui.root.after_cancel(agg_var)

# ----------------------------- MAIN -----------------------------
gui.root.after(200, on_start)
gui.mostra_home("", on_crea_partita, on_connetti, on_esci, on_focus, on_unfocus)