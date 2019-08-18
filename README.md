## Projekt for Testing the Sundtek Api
 
see [Api Dokumentation](http://sundtek.de/wiki/index.php?title=DAB/DAB%2B)

## Commandline Comands
```console
/opt/bin/mediaclient -m RADIO -f 90200000 -d /dev/radio0
/opt/bin/mediaclient -m RADIO -f 89300 -d /dev/radio0

Lockin Frq Radio
/opt/bin/mediaclient --readrds -d /dev/radio0

read the rds "stream"
/opt/bin/mediaclient -m RADIO -g off

mute off
/opt/bin/mediaclient --help
get some infos for commandline
cat /dev/dab0 | aplay
cat /dev/radio0 | aplay
```


