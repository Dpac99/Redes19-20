Projeto feito por:
  Andreia Pereira, n 89414;
  Diogo Pacheco, n 89433;
  Tiago Lé, n 89550.

Organização do código:
  A pasta server contém o código do FS (FS.c) tal como ficheiros auxiliares ao servidor;
  A pasta client contém o código do user (user.c) tal como ficheiros auxiliares ao user;
  A pasta others contém um ficheiro com includes e defines (consts.h) bem como um ficheiro de funções auxiliares a ambos os executáveis (helpers.c);
  A pasta test_results contém os 10 pdfs referentes aos resultados dos testes com user remoto;
  A Makefile encontra-se fora destas pastas e deve ser mantida lá.

  Exemplo:
  ./Redes/Makefile
         /server/FS.c
                /tcp_server_handlers.c
                /tcp_server_handlers.h
                /udp_server_handlers.c
                /udp_server_handlers.h
         /client/user.c
                /client_commands.c
                /client_commands.h
                /client_communication.c
                /client_communication.h
                /client_handlers.c
                /client_handlers.h
         /others/consts.h
                /helpers.c
                /helpers.h

  Os executáveis ('FS' e 'user') estarão no mesmo nível que a Makefile.

Utilização da Makefile:
  make: compila tanto o user como o FS;
  make FS: compila apenas o FS;
  make user: compila apenas o user;
  make clean: remove os executáveis.
