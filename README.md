# Progetto Tris

Questo progetto implementa un'architettura **client-server multi-client** per giocare a Tris.
Il server gestisce le partite, mentre più client possono connettersi simultaneamente.

Il progetto include:

- `Server/docker-compose.yml` per il **server**
- `Client/docker-compose.yml` per **client** multipli 

---

# Esecuzione del progetto 

Il progetto può essere eseguito tramite docker. Per le build dei container si devono eseguire questi comandi in questo ordine:

``` bash
# 1 creazione container server
cd server
docker compose up --build

# 2 torna indietro
cd ..

# 3 creazione container client
cd client
docker compose up --scale client=[Numeri client] # inserire un numero per specificare quanti client avviare

```