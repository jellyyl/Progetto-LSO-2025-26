import tkinter as tk

partite = [
    {"id": 1, "nome": "Partita 1", "user": "192.0.0.1"},
    {"id": 2, "nome": "Partita 2", "user": "192.0.0.2"},
    {"id": 3, "nome": "Partita 3", "user": "192.0.0.3"},
]

def connetti(part):
    print(f"Connessione richiesta alla partita {part['nome']} contro ({part['user']})")

# -----------------------------------------------------------------------------------

# Creazione finestra
root = tk.Tk()
root.title("Lista Partite")
root.geometry("400x300")
root.resizable(False, False)

canvas = tk.Canvas(root)
scrollbar = tk.Scrollbar(root, orient="vertical", command=canvas.yview)
frame = tk.Frame(canvas)

frame.bind(
    "<Configure>",
    lambda e: canvas.configure(scrollregion=canvas.bbox("all"))
)

canvas.create_window((0,0), window=frame, anchor="nw")
canvas.configure(yscrollcommand=scrollbar.set)

canvas.pack(side="left", fill="both", expand=True)
scrollbar.pack(side="right", fill="y")

# Creazione pannello per ogni partita
for part in partite:
    panel = tk.Frame(frame, bg="#cccccc", bd=1, relief="solid", padx=5, pady=5)
    panel.pack(fill="x", pady=5, padx=5)

    # Testo partita
    info = f"ID: {part['id']}  |  Nome: {part['nome']}  |  IP: {part['user']}"
    label = tk.Label(panel, text=info, bg="#cccccc")
    label.pack(side="left", padx=5)

    # Bottone connetti
    btn = tk.Button(panel, text="Connettiti", command=lambda s=part: connetti(s))
    btn.pack(side="right", padx=5)

root.mainloop()
