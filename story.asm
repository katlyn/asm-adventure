dq 0 ; ID
dq -1 ; Inject after
db '', 0
db "You step into the classroom, fluorescent lights shining overhead, and look around. You're a few minutes late, but it looks like your professor is just now getting started. There's an open seat at the front of the classroom, and one towards the back. Where do you sit?", 0

dq 1 ; ID
dq 0 ; Inject after
db 'front', 0
db "Walking up to the front of the classroom, you can feel the other students watching. Just a few minutes late isn't too bad, but you feel a bit silly walking up in front of everyone while the professor is talking. You sit down and pull out your laptop, ready to take notes.", 0

dq 2 ; ID
dq 0 ; Inject after
db 'back', 0
db "You walk quickly to the back and sit quietly in the open seat. Looking forward, everyone seems to have their notebooks and laptops open already, so you work to get everything out and ready for notes. As you rummage through your backpack, you notice that you've forgotten any pens or pencils. Should you ask your neighbor or just skip notes for today?", 0

dq 3 ; ID
dq 2 ; Inject after
db 'ask', 0
db 'Begrudgingly, your classmate hands you a mechanical pencil on the condition that you give it back at the end of class. Of course, you forget about the informal agreement and your trusting classmate is shorter one pencil at the end of the day.', 0

dq 4 ; ID
dq 2 ; Inject after
db 'skip', 0
db "You decide to skip your notes for today. As soon as you walk out of the classroom, you've happily forgotten everything that was talked about.", 0
