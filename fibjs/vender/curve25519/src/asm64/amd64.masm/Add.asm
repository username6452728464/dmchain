;  The MIT License (MIT)
; 
; Copyright (c) 2015 mehdi sotoodeh
; 
; Permission is hereby granted, free of charge, to any person obtaining 
; a copy of this software and associated documentation files (the 
; "Software"), to deal in the Software without restriction, including 
; without limitation the rights to use, copy, modify, merge, publish, 
; distribute, sublicense, and/or sell copies of the Software, and to 
; permit persons to whom the Software is furnished to do so, subject to 
; the following conditions:
; 
; The above copyright notice and this permission notice shall be included 
; in all copies or substantial portions of the Software.
; 
; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS 
; OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
; MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
; IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY 
; CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, 
; TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE 
; SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
;

include defines.inc

; _______________________________________________________________________
;
;   Z = X + Y
;   U64 ecp_Add(U64* Z, const U64* X, const U64* Y) 
; _______________________________________________________________________
Z   equ ARG1
X   equ ARG2
Y   equ ARG3

PUBPROC ecp_Add
    LOADA   Y
    xor     ACL,ACL
    ADDA    [X+24],[X+16],[X+8],[X]
    adc     ACL,ACL
    STOREA  Z
    ret
ENDPROC ecp_Add
   
; _______________________________________________________________________
;
;   Z = X + Y
;   void ecp_AddReduce(U64* Z, const U64* X, const U64* Y) 
; _______________________________________________________________________

PUBPROC ecp_AddReduce
    LOADA   Y
    ADDA    [X+24],[X+16],[X+8],[X]
    jnc     short ar_2
ar_1:
    ADDA    0,0,0,38
    jc      short ar_1
ar_2:
    STOREA  Z
    ret
ENDPROC ecp_AddReduce
END