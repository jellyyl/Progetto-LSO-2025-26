import tkinter as tk
from tkinter import ttk

# ---------------- DATI ----------------
partite = [
    {"id": 1, "nome": "Partita 1", "user": "192.0.0.1"},
    {"id": 2, "nome": "Partita 2", "user": "192.0.0.2"},
    {"id": 3, "nome": "Partita 3", "user": "192.0.0.3"},
    {"id": 4, "nome": "Partita 4", "user": "192.0.0.4"},
    {"id": 5, "nome": "Partita 5", "user": "192.0.0.5"},
    {"id": 6, "nome": "Partita 6", "user": "192.0.0.6"},
]

# ---------------- FUNZIONI ----------------
def connetti(part):
    print(f"Connessione richiesta alla partita {part['nome']} ({part['user']})")


def crea_partita():
    attesa = tk.Toplevel(root)
    attesa.title("In attesa...")
    attesa.geometry("300x120")
    attesa.resizable(False, False)
    attesa.transient(root)
    attesa.grab_set()

    ttk.Label(
        attesa,
        text="In attesa di un giocatore...",
        font=("Segoe UI", 11)
    ).pack(expand=True, pady=20)

    ttk.Button(attesa, text="Annulla", command=attesa.destroy).pack(pady=5)


# ---------------- FINESTRA PRINCIPALE ----------------
root = tk.Tk()
root.title("Lista Partite")
root.geometry("420x340")
root.resizable(False, False)

# ---------------- STILE ----------------
style = ttk.Style()
style.theme_use("clam")

style.configure(
    "Header.TFrame",
    background="#4da6ff"
)

style.configure(
    "Header.TButton",
    font=("Segoe UI", 10, "bold"),
    padding=8,
    background="#4da6ff",
    foreground="white"
)

style.map(
    "Header.TButton",
    background=[("active", "#3399ff")]
)

style.configure(
    "Card.TFrame",
    background="#e6e6e6",
    relief="solid",
    borderwidth=1
)

style.configure(
    "Card.TLabel",
    background="#e6e6e6",
    font=("Segoe UI", 10)
)

style.configure(
    "Card.TButton",
    padding=6
)

# ---------------- HEADER ----------------
top_frame = ttk.Frame(root, style="Header.TFrame")
top_frame.pack(fill="x")

btn_crea = ttk.Button(
    top_frame,
    text="➕ Crea partita",
    style="Header.TButton",
    command=crea_partita
)
btn_crea.pack(side="right", padx=10, pady=6)

# ---------------- AREA SCROLL ----------------
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

# ---------------- PARTITE ----------------
for part in partite:
    card = ttk.Frame(scroll_frame, style="Card.TFrame", padding=8)
    card.pack(fill="x", pady=6, padx=8)

    info = f"ID: {part['id']}   |   {part['nome']}   |   {part['user']}"
    ttk.Label(card, text=info, style="Card.TLabel").grid(row=0, column=0, sticky="w")

    ttk.Button(
        card,
        text="Connettiti",
        style="Card.TButton",
        command=lambda p=part: connetti(p)
    ).grid(row=0, column=1, padx=10)

    card.columnconfigure(0, weight=1)

# ---------------- START ----------------
root.mainloop()
