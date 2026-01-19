import tkinter as tk
from tkinter import ttk

from .style import setup_style

from pathlib import Path

root = tk.Tk()

# Immagini
BASE_DIR = Path(__file__).resolve().parent
img_x = tk.PhotoImage(file=BASE_DIR / "assets" / "x.png")
img_o = tk.PhotoImage(file=BASE_DIR / "assets" / "o.png")


def centra_finestra(win, parent, width, height):
    parent.update_idletasks()

    x = parent.winfo_x() + (parent.winfo_width() // 2) - (width // 2)
    y = parent.winfo_y() + (parent.winfo_height() // 2) - (height // 2)

    win.geometry(f"{width}x{height}+{x}+{y}")

def mostra_home(partite, on_crea_partita, on_connetti, on_esci):
    # Finestra Principale
    setup_style(root)
    root.title("Lista Partite")
    root.geometry("800x400")
    root.resizable(False, False)

    # Header
    top_frame = ttk.Frame(root, style="Header.TFrame")
    top_frame.pack(fill="x")

    btn_crea = ttk.Button(
        top_frame,
        text="➕ Crea partita",
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

    # Scrollbar
    container = ttk.Frame(root)
    container.pack(fill="both", expand=True)

    canvas = tk.Canvas(container, highlightthickness=0)
    scrollbar = ttk.Scrollbar(container, orient="vertical", command=canvas.yview)

    scroll_frame = ttk.Frame(canvas)

    scroll_frame.bind(
        "<Configure>",
        lambda e: canvas.configure(scrollregion=canvas.bbox("all"))
    )

    canvas.create_window((0, 0), window=scroll_frame, anchor="nw")
    canvas.configure(yscrollcommand=scrollbar.set)

    canvas.pack(side="left", fill="both", expand=True)
    scrollbar.pack(side="right", fill="y")

    # Pannello Partite
    for part in partite:
        card = ttk.Frame(scroll_frame, style="Card.TFrame", padding=8)
        card.pack(fill="x", pady=6, padx=8)

        info = f"ID: {part[0]}   |   {part[1]}   |   {part[2]}"
        ttk.Label(card, text=info, style="Card.TLabel").grid(row=0, column=0, sticky="w")

        ttk.Button(
            card,
            text="Connetti",
            style="Card.TButton",
            command=lambda p=part: on_connetti(p)
        ).grid(row=0, column=1, padx=10)

        card.columnconfigure(0, weight=1)

    root.mainloop()

def mostra_attesa(messaggio):
    attesa = tk.Toplevel(root)
    attesa.title("In attesa...")
    attesa.geometry("320x130")
    attesa.resizable(False, False)

    centra_finestra(attesa, root, 320, 130)

    main = ttk.Frame(attesa)
    main.pack(fill="both", expand=True)

    attesa.transient(root)
    attesa.grab_set()

    ttk.Label(
        main,
        text=messaggio,
        font=("Segoe UI", 11),
        wraplength=280,
        justify="center"
    ).pack(expand=True, pady=20, padx=20)

    ttk.Button( # Simbolico
        main,
        text="Annulla",
        command=attesa.destroy
    ).pack(pady=8)

    return attesa # Restituisce la finestra per eventualmente nasconderla con nascondi_attesa

def nascondi_attesa(attesa):
    attesa.destroy()

def mostra_scelta(messaggio, testo_btn1, testo_btn2): # Restituisce 1 se viene premuto btn1, 2 se btn2
    risultato = None

    scelta = tk.Toplevel(root)
    scelta.title("Conferma")
    scelta.geometry("360x150")
    scelta.resizable(False, False)

    centra_finestra(scelta, root, 360, 150)

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

    def scegli(valore): # Funzione provvisoria per impostare il valore di ritorno
        nonlocal risultato
        risultato = valore
        scelta.destroy()

    ttk.Button(
        btn_frame,
        text=testo_btn1,
        style="Header.TButton",
        command=lambda: scegli(1)
    ).pack(side="left", padx=10)

    ttk.Button(
        btn_frame,
        text=testo_btn2,
        style="Header2.TButton",
        command=lambda: scegli(2)
    ).pack(side="left", padx=10)

    scelta.wait_window()
    return risultato

def riempi_cella_partita(giocatore, r, c): # Usata dal client per disegnare le mosse dell'avversario
    x = c * 101.6 + 100 // 2
    y = r * 101.6 + 100 // 2
    tris_canvas.create_image(x, y, image = img_x if giocatore==1 else img_o)
    tris_canvas.itemconfig(r*3 + c+1, state="disabled")
    

def mostra_partita(giocatore, on_click_cella): # Giocatore = 1: gioca X, Giocatore = 2: gioca O
    partita = tk.Toplevel(root)
    partita.title("Tris")
    partita.geometry("360x420")
    partita.resizable(False, False)

    centra_finestra(partita, root, 360, 420)

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
        riempi_cella_partita(giocatore, r, c)
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
                fill="#C4C4C4"
            )

            tris_canvas.tag_bind(rect, "<Enter>", lambda e, r=rect: tris_canvas.itemconfig(r, fill="#B3B3B3"))
            tris_canvas.tag_bind(rect, "<Leave>", lambda e, r=rect: tris_canvas.itemconfig(r, fill="#C4C4C4"))
            tris_canvas.tag_bind(rect, "<Button-1>", lambda e, r=r, c=c: click_cella(r, c))
    
    # Disegno griglia
    line_width = 5

    tris_canvas.create_line(100, 0, 100, 300, width=line_width)
    tris_canvas.create_line(200, 0, 200, 300, width=line_width)

    tris_canvas.create_line(0, 100, 300, 100, width=line_width)
    tris_canvas.create_line(0, 200, 300, 200, width=line_width)

    # Label turno
    ttk.Label(
        main,
        font=("Segoe UI", 11, "bold")
    ).pack(pady=10)

    return partita


