dq 0 ; ID
dq 0 ; Inject after
db 'first', 0
db 'This is the first segment', 0


dq 1 ; ID
dq 0 ; Inject after
db 'second', 0
db 'This is the second segment', 0

dq 2 ; ID
dq 1 ; Inject after
db 'third', 0
db 'This is the third segment', 0

dq 3 ; ID
dq 2 ; Inject after
db 'fouth', 0
db 'This is the fourth segment', 0

dq 4 ; ID
dq 3 ; Inject after
db 'fifth', 0
db 'This is the fith segment', 0

