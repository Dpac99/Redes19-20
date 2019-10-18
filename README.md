# Redes19-20
## Work of Diogo Pacheco, Andreia Pereira and Tiago Lé

* ## **Server File Strucute**
...+The servers stores everything in a dir named TOPICS.
...+Inside TOPICS exists a dir for every topic, with *topic_name*.
...+Inside each topic exists a file called **USER.txt** that contains the uid of who proposed it.
...There is also a dir for every question, with *question_name*. 
...+Inside each question dir exists a dir called **QUESTION_DATA**.
...Also inside the question dir exists a dir for every answer named *question_XX* where XX ranges from 01-99.
...These dirs contain a file with the uid **USER.txt**, a file with qimg and ext **IMG_INFO.txt**, and the data files, **QDATA.txt**/**ANS_DATA.txt** and **IMG.ext**.