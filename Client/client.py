import gui.interface
import network.socket

# ---------------------------- EVENTI ----------------------------

def on_click_cella(r, c):
    # (Manda mossa al server)
    # (Aspetta e riceve la mossa dell'avversario, mostrandola)
    gui.interface.riempi_cella_partita(2, 2, 2) # PLACEHOLDER

def on_crea_partita():
    scelta = 2
    while(scelta == 2):
        finestra_attesa = gui.interface.mostra_attesa("In attesa di un giocatore")
        # (Manda messaggio di creazione partita al server e aspetta un giocatore)
        # (Se un giocatore si connette):
        scelta = gui.interface.mostra_scelta("Errore31 vuole giocare", "Accetta", "Rifiuta")
        if(scelta == 1):
            gui.interface.nascondi_finestra(finestra_attesa)
            gui.interface.mostra_partita(1, on_click_cella)
        

def on_connetti(partita):
    finestra_attesa = gui.interface.mostra_attesa("In attesa di conferma")
    # (Manda messaggio all'host della partita e aspetta una risposta)
    #gui.interface.nascondi_finestra(finestra_attesa)
    # (Se viene accettato dall'host):
    # (Inizia la partita)

def on_esci():
    # (Chiude il socket)
    quit()

def on_focus(event):
    pass
    # gui.interface.attiva_aggiornamento()
    # print("Focus su home")

def on_unfocus(event):
    pass
    # gui.interface.disattiva_aggiornamento()
    # print("Unfocus su home")
    

# --------------------------- FUNZIONI ---------------------------

def converti_str_in_partite(stringa): # stringa: nel formato "id;nome;indirizzo;id2;nome2;indirizzo2..."
    dati = stringa.split(";")
    partite = []
    partita = []
    i = 0
    for dato in dati:
        partita.append(dato)
        i = i + 1
        if(i >= 3):
            partite.append(partita)
            partita = []
            i = 0
    return partite

def aggiorna_partite():
    # (Richiede al server la nuova stringa partite e la converte)
    # gui.interface.aggiorna_partite(new_partite, on_connetti)
    print("Aggiorno!")

    
# ----------------------------- MAIN -----------------------------
# (Si connette e ricava stringa partite dal server)

stringa_esempio = "1;Partita 1;192.0.0.1;2;Partita 2;192.0.0.2;3;Partita 3;192.0.0.3;4;Partita 4;192.0.0.4;5;Partita 5;192.0.0.5;6;Partita 6;192.0.0.6"
partite = converti_str_in_partite(stringa_esempio)

gui.interface.mostra_home(partite, on_crea_partita, on_connetti, on_esci, on_focus, on_unfocus)
