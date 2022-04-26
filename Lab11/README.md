# LAB11
## 2019A7PS0136G


### Compilation

```bash
gcc -pthread server.c -o server.out
gcc -pthread client.c -o client.out
```

### Start the server first

```bash
./server.out 4444
```

### Then start the clients

```bash
./client.out 127.0.0.1 4444
```
