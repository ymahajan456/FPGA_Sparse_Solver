# Instruction Planning #
### BRAM Ports ###
Port | Width | Description
----- | ----- | -------
Address | 13 (10) | 
Read From External | 1 | Read from DMA or similar option **(Future DevOp)**
RD/ WR | 1 | Read (0); Write (1)

### AU Ports ###
Port | Width | Description
---- | ----- | -----------
Op | 1 | Operation selection (0: C+AB, 1: C-AB)

## Connection Settings ##
BRAM In (16) | BRAM Out (16) | AU In (20) | AU Out (8)
------- | -------- | ----- | ------
MAC Outs (4) | MAC Ins (3*4 = 12) | BRAM Outs (16) | BRAM Ins (16)
DIV Outs (4) | DIV Ins (2\*4 = 8)  | MAC Outs (4)   | MAC Ins (3*4 = 12)
Sys In (Future) | Sys Out (Future) | DIV Ins (4) | DIV Ins (2\*4 = 8)
- | - | Zero | -

