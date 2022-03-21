# LAB06
## 2019A7PS0136G


### Compilation

```bash
gcc ./server/server.c -o ./server/server.out
gcc ./client/client.c -o ./client/client.out
```
_OR_
```bash
make build
```


### Start the server first

```bash
cd server
./server.out 4444
```
_OR_
```bash
make server
```


### Then start the client

```bash
cd client
./client.out 127.0.0.1 4444
```
_OR_
```bash
make client
```


> NOTE: `cd` command should not be omitted. Running `./client/client.out` will write to incorrect directory

