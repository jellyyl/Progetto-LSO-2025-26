FROM python:3.10.12-slim

# Installa le dipendenze necessarie per Tkinter (interfaccia grafica Python)
RUN apt-get update && apt-get install -y \
    python3-tk \
    tk \
    tcl \
    && rm -rf /var/lib/apt/lists/*
    #l'ultimo comando cancella la cache generata da apt-get update per ridurre l'uso di memoria. Se facessi più comandi RUN, 
    #rischierei di memorizzare lo stesso la cache e non potrei cancellarla, essendo che ogni comando RUN crea un nuovo layer"

# Crea e imposta la directory di lavoro nel container
WORKDIR /app

# Copia i file sorgenti all'interno del container
COPY . .

# Imposta variabili d'ambiente di default (verranno sovrascritte dal docker-compose)
ENV SERVER_IP="127.0.0.1"
ENV SERVER_PORT="5200"

# Comando di default per avviare il client
CMD ["python3", "client.py"]
