# Frequently Asked Questions

## Why can I not run the server?

Your server may not have the execute permission after downloading from the server. Try to give it the permission by changing the mode:
```chmod u+x d1_dump```


## What is a good output for d1_dump?

```
Waiting for D1 packet on port 12121
Waiting for an IPv4 packet of up to 1024 bytes.
Received 16 bytes from 127.0.0.1
Your packet has 16 bytes.
Analyzing the D1Header
Your header fields have the following values:
    Flags decimal: 32768 hexadecimal: 8000 bitfield: 1000000000000000
    Checksum decimal: 33784 hexadecimal: 83f8
    Size decimal: 16 hexadecimal: 10
About flags
    This is a data packet.
    Its sequence number is 0.
About size
    The size field contains the actual size of the packet.
About checksums
total 83f8
total 83f8
    The computed checksum 83f8 is identical to the checksum in the packet 83f8.
```

## What is a good output for d1_server?

```
Created UDP server with socket 3
66634: Received 15 bytes from 127.0.0.1
66634: testing if data (8000) is set in flags (8000)
header chk: 80f total 9c6d
header chk: 80f total e819
66634: received a data packet with header 8000 f 9c6d, size correct, good checksum
66634: Sending an ACK packet, acking 0
66634: Sending a packet
66634: Received >>>connect<<<
66634: Received 12 bytes from 127.0.0.1
66634: testing if data (8000) is set in flags (8080)
header chk: 808c total 9e82
header chk: 808c total 9e82
66634: received a data packet with header 8080 c 9e82, size correct, good checksum
66634: Sending an ACK packet, acking 1
66634: Sending a packet
66634: Received >>>ping<<<
header chk: 80c total 9e4
66634: Sending a DATA packet with header 8000 c 9e04, sending 12 bytes
66634: Sending a packet
66634: Received 8 bytes from 127.0.0.1
66634: testing if ACK (100) is set in flags (100)
66634: received frame with header 100 8 108 size is correct - expected ack 0, advancing next seqno
66634: Received 12 bytes from 127.0.0.1
66634: testing if data (8000) is set in flags (8000)
header chk: 80c total 9e2
header chk: 80c total 9e2
66634: received a data packet with header 8000 c 9e02, size correct, good checksum
66634: Sending an ACK packet, acking 0
66634: Sending a packet
66634: Received >>>ping<<<
header chk: 808c total 9e84
66634: Sending a DATA packet with header 8080 c 9e84, sending 12 bytes
66634: Sending a packet
66634: Received 8 bytes from 127.0.0.1
66634: testing if ACK (100) is set in flags (101)
66634: received frame with header 101 8 109 size is correct - expected ack 1, advancing next seqno
66634: Received 18 bytes from 127.0.0.1
66634: testing if data (8000) is set in flags (8080)
header chk: 8092 total f5e7
header chk: 8092 total f5e7
66634: received a data packet with header 8080 12 f5e7, size correct, good checksum
66634: Sending an ACK packet, acking 1
66634: Sending a packet
66634: Received >>>disconnect<<<
```

## What is a good output for d2_server?

This the output for the server when client sends ID 1007. The number 66663 is the process number of this server; your will be different.

[d2_server-output.txt](d2_server-output.txt)

## I am running the server and client. d1_server works fine while d1_dump doesn't. How do I fix this?

If you are absolutely sure your code has no issues and should be working (yet still doesnt work), try updating the servers and maybe all other files which you are not supposed to edit. There are a few who have had this issue.

### WARNING: Make sure not to override your code! Keeping a backup of your work is a good practice.

## I get segmentation faults and I cannot figure out what the cause is.

Use valgrind. This is a powerful tool that can help you figure most problems. 
A simple '\*' or '&' in the wrong place can cause errors that are hard to detect. 
