import tkinter as tk
from tkinter import ttk

from .style import setup_style

from pathlib import Path

root = tk.Tk()

# Immagini
BASE_DIR = Path(__file__).resolve().parent
img_x = tk.PhotoImage(file=BASE_DIR / "assets" / "x.png")
img_o = tk.PhotoImage(file=BASE_DIR / "assets" / "o.png")
icona = tk.PhotoImage(file=BASE_DIR / "assets" / "icon.png")

# Variabili di aggiornamento
anim_id = None
scroll_attivo = False

def calcola_geometria(width, height):
    x = root.winfo_x() + (root.winfo_width() // 2) - (width // 2)
    y = root.winfo_y() + (root.winfo_height() // 2) - (height // 2)
    return f"{width}x{height}+{x}+{y}"

def converti_str_in_partite(stringa):
    stringa = stringa.replace(" ", "")
    stringa = stringa.replace("\n", "")
    dati = stringa.split(";")
    partite = []
    partita = []
    for dato in dati:
        partita.append(dato)
        if(len(partita) >= 3):
            if(partita[2] == "1"):
                partite.append(partita)
            partita = []
    return partite

def gestisci_riduzione_a_icona(finestra):
    def on_root_unmap(event):
        if finestra.winfo_exists():
            finestra.grab_release()

    def on_root_map(event):
        if finestra.winfo_exists():
            finestra.grab_set()

    root.bind("<Unmap>", on_root_unmap)
    root.bind("<Map>", on_root_map)

def aggiorna_partite(str_newpartite, on_connetti):
        global label_nopartite, scroll_attivo
        new_partite = converti_str_in_partite(str_newpartite)
        home_canvas.delete("all")
        home_canvas.create_window((0,0), window=scroll_frame, anchor="nw")
        for widget in scroll_frame.winfo_children():
            widget.destroy() 

        if(len(new_partite) <= 0):
            if(label_nopartite == None):
                label_nopartite = tk.Label(
                    home_canvas,
                    text="Non ci sono partite disponibili",
                    font=("Segoe UI", 11, "italic"),
                    fg="#666666",
                    justify="center"
                )
                label_nopartite.pack(expand=True)
            ttk.Label(scroll_frame, text="", style="Card.TLabel") \
                    .grid(row=0, column=0, sticky="w")
        else:
            if(label_nopartite != None):
                label_nopartite.destroy()
                label_nopartite = None
            
        for part in new_partite:
                card = ttk.Frame(scroll_frame, style="Card.TFrame", padding=8, width=500, height=60)
                card.pack(fill="x", pady=6, padx=8)

                card.pack_propagate(False)

                info = f"ID: {part[0]} | Host: {part[1]}   "
                ttk.Label(card, text=info, style="Card.TLabel") \
                    .grid(row=0, column=0, sticky="w")

                ttk.Button(
                    card,
                    text="Connetti",
                    style="Card.TButton",
                    command=lambda p=part: on_connetti(p)
                ).grid(row=0, column=1, padx=10)

                card.columnconfigure(0, weight=1)

        home_canvas.update_idletasks()

        canvas_height = home_canvas.winfo_height()
        content_height = home_canvas.bbox("all")[3] if home_canvas.bbox("all") else 0
        scroll_attivo = content_height > canvas_height
        if scroll_attivo:
            scrollbar.configure(command=home_canvas.yview)
        else:
            scrollbar.configure(command=lambda *args: None)
                
        home_canvas.configure(scrollregion=home_canvas.bbox("all"))

def mostra_home(str_partite, on_crea_partita, on_connetti, on_esci, on_focus, on_unfocus):
    # Finestra Principale
    setup_style(root)
    root.title("Home")
    root.geometry("800x400")
    root.resizable(False, False)

    # Header
    top_frame = ttk.Frame(root, style="Header.TFrame")
    top_frame.pack(fill="x")

    btn_crea = ttk.Button(
        top_frame,
        text="+ Crea partita",
        style="Header.TButton",
        command=on_crea_partita
    )
    btn_crea.pack(side="right", padx=10, pady=6)

    btn_esci = ttk.Button(
        top_frame,
        text="Esci",
        style="Header3.TButton",
        command=on_esci
    )
    btn_esci.pack(side="right", padx=0, pady=6)

    root.bind("<FocusIn>", lambda e: on_focus(e) if e.widget is root else None)
    root.bind("<FocusOut>", lambda e: on_unfocus(e) if e.widget is root else None)

    # Scrollbar
    container = ttk.Frame(root)
    container.pack(fill="both", expand=True)

    global home_canvas, label_nopartite, scrollbar
    home_canvas = tk.Canvas(container, highlightthickness=0)
    label_nopartite = None
    scrollbar = ttk.Scrollbar(container, orient="vertical", command=home_canvas.yview)
    
    global scroll_frame
    scroll_frame = ttk.Frame(home_canvas)

    scroll_frame.bind(
        "<Configure>",
        lambda e: home_canvas.configure(scrollregion=home_canvas.bbox("all"))
    )

    home_canvas.create_window((0, 0), window=scroll_frame, anchor="nw")
    home_canvas.configure(yscrollcommand=scrollbar.set)

    home_canvas.pack(side="left", fill="both", expand=True)
    scrollbar.pack(side="right", fill="y")

    def _on_mousewheel(event):
        if not scroll_attivo:
            return
        if event.delta:
            home_canvas.yview_scroll(int(-1 * (event.delta / 120)), "units")
        elif event.num == 4:
            home_canvas.yview_scroll(-1, "units")
        elif event.num == 5:
            home_canvas.yview_scroll(1, "units")

    home_canvas.bind_all("<MouseWheel>", _on_mousewheel)
    home_canvas.bind_all("<Button-4>", _on_mousewheel)
    home_canvas.bind_all("<Button-5>", _on_mousewheel)

    # Pannello Partite
    aggiorna_partite(str_partite, on_connetti)

    root.iconphoto(True, icona)
    root.mainloop()

def mostra_attesa(messaggio):
    attesa = tk.Toplevel(root)
    attesa.title("In attesa...")
    attesa.geometry(calcola_geometria(320, 130))
    attesa.resizable(False, False)

    main = ttk.Frame(attesa)
    main.pack(fill="both", expand=True)

    attesa.transient(root)
    attesa.grab_set()

    label = ttk.Label(
        main,
        text=messaggio,
        font=("Segoe UI", 11),
        wraplength=280,
        justify="center"
    )
    label.pack(expand=True, pady=20, padx=20)
    n_puntini = 0

    def animazione_puntini():
        nonlocal n_puntini
        n_puntini = (n_puntini + 1) % 4
        label.configure(text=messaggio + "." * n_puntini)
        attesa.after(500, animazione_puntini)

    animazione_puntini()

    ttk.Button(
        main,
        text="Annulla",
        command=attesa.destroy
    ).pack(pady=8)

    gestisci_riduzione_a_icona(attesa)
    attesa.iconphoto(True, icona)
    return attesa

def nascondi_finestra(finestra):
    finestra.destroy()

def mostra_scelta(messaggio, testo_btn1="Accetta", testo_btn2="Rifiuta"): # Restituisce 1 se viene premuto btn1, 2 se btn2
    risultato = None

    scelta = tk.Toplevel(root)
    scelta.title("Conferma")
    scelta.geometry(calcola_geometria(360, 150))
    scelta.resizable(False, False)

    main = ttk.Frame(scelta)
    main.pack(fill="both", expand=True)
    
    scelta.transient(root)
    scelta.grab_set()

    ttk.Label(
        main,
        text=messaggio,
        font=("Segoe UI", 11),
        wraplength=320,
        justify="center"
    ).pack(padx=20, pady=20)

    btn_frame = ttk.Frame(main)
    btn_frame.pack(pady=10)

    def scegli(valore):
        nonlocal risultato
        risultato = valore
        scelta.destroy()

    ttk.Button(
        btn_frame,
        text=testo_btn1,
        style="Header.TButton",
        command=lambda: scegli(0)
    ).pack(side="left", padx=10)

    ttk.Button(
        btn_frame,
        text=testo_btn2,
        style="Header2.TButton",
        command=lambda: scegli(1)
    ).pack(side="left", padx=10)

    gestisci_riduzione_a_icona(scelta)
    scelta.iconphoto(True, icona)
    scelta.wait_window()
    return risultato

def mostra_errore(messaggio, testo_btn1="OK", on_press=lambda: None, on_esci=None): # Se on_esci = None il bottone esci non viene mostrato
    errore = tk.Toplevel(root)
    errore.title("Errore")
    errore.geometry(calcola_geometria(320, 170))
    errore.resizable(False, False)

    errore.transient(root)
    errore.grab_set()

    main = ttk.Frame(errore, padding=15)
    main.pack(fill="both", expand=True)

    content = ttk.Frame(main)
    content.pack(fill="x", pady=0)

    ttk.Label(
        content,
        text="✖",
        font=("Segoe UI", 36, "bold"),
        foreground="#C00000"
    ).pack(side="left", padx=(0, 15))

    ttk.Label(
        content,
        text=messaggio,
        wraplength=240,
        font=("Segoe UI", 11)
    ).pack(side="left", fill="x", expand=True)

    def destroy_and_onpress():
        errore.destroy()
        on_press()

    btn_frame = ttk.Frame(main)
    btn_frame.pack(pady=10)

    ttk.Button(
        btn_frame,
        text=testo_btn1,
        style="Header2.TButton",
        command=destroy_and_onpress
    ).pack(side="left", padx=10)

    if(on_esci != None):
        ttk.Button(
            btn_frame,
            text="Esci",
            style="Header3.TButton",
            command=on_esci
        ).pack(side="left", padx=10)

    gestisci_riduzione_a_icona(errore)
    errore.iconphoto(True, icona)
    errore.wait_window()


def riempi_cella_partita(simbolo, r, c):
    if(tris_canvas.gettags(r*3+c+1)[0] == "0"):
        x = c * 101.6 + 100 // 2
        y = r * 101.6 + 100 // 2
        tris_canvas.create_image(x, y, image = img_x if simbolo=='X' else img_o)
        tris_canvas.itemconfig(r*3 + c+1, state="disabled", tags=(1))

def aggiorna_label_partita(testo, colore_sfondo, waiting=False):
    global label_turno, panel_turno, anim_id 
    label_turno.configure(text=testo, background=colore_sfondo)
    panel_turno.configure(background=colore_sfondo)

    if(anim_id is not None):
        root.after_cancel(anim_id)
        anim_id = None

    if(waiting):
        n_puntini = 0
        def animazione_puntini():
            global anim_id
            nonlocal n_puntini
            n_puntini = (n_puntini + 1) % 4
            label_turno.configure(text=testo + "." * n_puntini)
            anim_id = root.after(500, animazione_puntini)
        animazione_puntini()

def disabilita_griglia_partita():
    for i in range(1, 10):
        tris_canvas.itemconfig(i, state="disabled")

def abilita_griglia_partita():
    for i in range(1, 10):
        if(tris_canvas.gettags(i)[0] == "0"):
            tris_canvas.itemconfig(i, state="normal")

def mostra_partita(simbolo_giocatore, on_click_cella):
    partita = tk.Toplevel(root)
    partita.title("Tris")
    partita.geometry(calcola_geometria(360, 380))
    partita.resizable(False, False)

    partita.transient(root)
    partita.grab_set()

    main = ttk.Frame(partita, padding=10)
    main.pack(fill="both", expand=True)

    partita.img_x = img_x
    partita.img_o = img_o

    global tris_canvas
    tris_canvas = tk.Canvas(main, width=300, height=300, highlightthickness=0, background="#DCDAD5")
    tris_canvas.pack(pady=10)

    size = 100 # Dimensione celle
    gap = 1.6 # Distanza fra una cella e l'altra
    step = size + gap

    # Disegno cella cliccata e chiamata evento
    def click_cella(r, c):
        riempi_cella_partita(simbolo_giocatore, r, c)
        on_click_cella(r, c)

    # Disegno celle
    for r in range(3):
        for c in range(3):
            x1 = c * step
            y1 = r * step
            x2 = x1 + size
            y2 = y1 + size

            rect = tris_canvas.create_rectangle(
                x1, y1, x2, y2,
                outline="",
                fill="#F0F0F0",      # grigio = #C4C4C4 grigio scuro = B3B3B3
                tags=(0)    # 0 = cella libera, 1 = cella occupata
            )

            tris_canvas.tag_bind(rect, "<Enter>", lambda e, r=rect: tris_canvas.itemconfig(r, fill="#D0D0D0"))
            tris_canvas.tag_bind(rect, "<Leave>", lambda e, r=rect: tris_canvas.itemconfig(r, fill="#F0F0F0"))
            tris_canvas.tag_bind(rect, "<Button-1>", lambda e, r=r, c=c: click_cella(r, c))
    
    # Disegno griglia
    line_width = 5

    tris_canvas.create_line(100, 0, 100, 300, width=line_width, fill="#314A75")
    tris_canvas.create_line(200, 0, 200, 300, width=line_width, fill="#314A75")

    tris_canvas.create_line(0, 100, 300, 100, width=line_width, fill="#314A75")
    tris_canvas.create_line(0, 200, 300, 200, width=line_width, fill="#314A75")

    # Pannello
    global panel_turno
    panel_turno = tk.Frame(partita, background="#4da6ff")
    panel_turno.pack(fill="x", side="bottom")

    global label_turno
    label_turno = tk.Label(
        panel_turno,
        text="...",
        font=("Segoe UI", 11, "bold"),
        foreground="white",
        background="#4da6ff"
    )
    label_turno.pack(anchor="n", pady=(6, 8))

    gestisci_riduzione_a_icona(partita)
    partita.iconphoto(True, icona)
    return partita


