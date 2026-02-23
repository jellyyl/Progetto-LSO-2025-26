FROM gcc:11.4.0

# Crea e imposta la directory di lavoro nel container
WORKDIR /app

# Copia i file sorgenti all'interno del container
COPY . .

# Compila il progetto utilizzando make
RUN make

# Esponi la porta su cui il server è in ascolto
EXPOSE 5200

# Comando di default per avviare il server
CMD ["./server_exec"]
