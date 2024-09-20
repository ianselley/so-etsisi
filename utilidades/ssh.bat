@echo off

@rem Utiliza el tipo de terminal que Minix espera
set TERM=xterm

@rem Puerto 5522
@rem Conexión con @localhost
@rem Establece los algoritmos de cifrado, intercambio de claves y algoritmos de clave de host
ssh -p 5522 -oKexAlgorithms=+diffie-hellman-group1-sha1 -oHostKeyAlgorithms=+ssh-rsa,ssh-dss usuario@localhost
