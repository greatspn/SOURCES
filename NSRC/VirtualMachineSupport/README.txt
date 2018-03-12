
NOTA:

Questa cartella contiene gli script di supporto per la virtual machine
di GreatSPN.

Lo script di post-installazione per la virtual machine, vbox_install_script.sh,
si bassa sulla presenza del file /home/user/.greatspn-on-vbox per stabilire
che il make install è eseguito sulla macchina virtuale e non su qualche altra
installazione di GreatSPN.

Di conseguenza, la virtual machine necessita del file /home/user/.greatspn-on-vbox
per avere l'auto-aggiornamento funzionante.