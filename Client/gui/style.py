from tkinter import ttk

# ---------------- STILI ----------------

def setup_style(root):
    style = ttk.Style(root)
    style.theme_use("clam")

    # Pannello Superiore Home
    style.configure(
        "Header.TFrame",
        background="#4da6ff"
    )

    # Bottone Blu Stilizzato (Crea Partita, Accetta)
    style.configure(
        "Header.TButton",
        font=("Segoe UI", 10, "bold"),
        padding=8,
        background="#4da6ff",
        foreground="white"
    )

    style.map(
        "Header.TButton",
        background=[("active", "#3a9dff")]
    )

    # Bottone Grigio Stilizzato (Rifiuta)
    style.configure(
        "Header2.TButton",
        font=("Segoe UI", 10),
        padding=8,
        background="#d9d7d2"
    )

    style.map(
        "Header2.TButton",
        background=[("active", "#bcb8b4")]
    )

    # Bottone Rosso Stilizzato (Esci)
    style.configure(
        "Header3.TButton",
        font=("Segoe UI", 10, "bold"),
        padding=8,
        background="#ff4d4d",
        foreground="white"
    )

    style.map(
        "Header3.TButton",
        background=[("active", "#ff3333")]
    )

    # Pannello Inferiore Home
    style.configure(
        "Card.TFrame",
        background="#e6e6e6",
        relief="solid",
        borderwidth=1
    )

    # Pannelli Partite Home
    style.configure(
        "Card.TLabel",
        background="#e6e6e6",
        font=("Segoe UI", 10)
    )

    # Bottone Grigio Base (Connetti)
    style.configure(
        "Card.TButton",
        padding=6
    )