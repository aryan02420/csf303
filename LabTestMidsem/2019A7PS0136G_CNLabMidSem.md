# Midsem Lab
## 2019A7PS0136G (31120190136)
### 7 March 2022

## Q1

### README

#### 11 Digit Campus ID: `31120190136`

#### Compilation

```bash
gcc server.c -o server.out
gcc client.c -o client.out
```

#### Running

```bash
./server.out 4444
```

```bash
./client.out 127.0.0.1 4444
```

##### Screenshots

###### 1. 

![image-20220307155728935](assets/image-20220307155728935.png)

###### 2. 

![image-20220307155749683](assets/image-20220307155749683.png)

###### 3. After connecting to the server, the client reads your campus id (3112xxxxxxx)

![image-20220307152349141](assets/image-20220307152349141.png)

###### 4. The server prints the received campus id and then calculates the value of y

![image-20220307153104843](assets/image-20220307153104843.png)

###### 5. extract the equation whose index is y obtained in step 4. **SEE CODE**

###### 6. The server solves the equation and sends the answer to the client.

![image-20220307155228746](assets/image-20220307155228746.png)

###### 7. The server takes the student’s first name as input. If all alphabets are in lower case,   the server sends the name to the client.

![image-20220307155155982](assets/image-20220307155155982.png)

###### 8. The client displays the &lt;name&gt;,&lt;campus id&gt;,&lt;y&gt; on its screen and exits.

![image-20220307154937233](assets/image-20220307154937233.png)

###### 9. The server is ready to accept a new client

![image-20220307153701733](assets/image-20220307153701733.png)

## Q2

### 1. 
IP address of local machine (the client) is `10.0.2.15`
The client sends `5` http requests (http.request filter used excluding OCSP)

![image-20220307142245335](assets/image-20220307142245335.png)

### 2.

The client receives `4` http responses.

![image-20220307142304399](assets/image-20220307142304399.png)

### 3.

`730 bytes`

![image-20220307142705517](assets/image-20220307142705517.png)

### 4.

Right click column headers and choose `Column Preferences`

![image-20220307143638455](assets/image-20220307143638455.png)

Under `Appearance > Column` click `+`

Give appropriate `Title` and `Type` to the columns as shown below and click `OK`

![image-20220307143338300](assets/image-20220307143338300.png)

![image-20220307143657849](assets/image-20220307143657849.png)

### 5.

RTT for the request sent to www.sougata-sen.com is `543.5 ms`

![image-20220307142852184](assets/image-20220307142852184.png)

### 6.

![image-20220307144400738](assets/image-20220307144400738.png)

Hex value of the destination port in the TCP header of the response is `b6 52`
