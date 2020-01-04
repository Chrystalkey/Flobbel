netsh advfirewall firewall show rule name=all 
rem alle firewallregeln stand jetzt
netsh advfirewall monitor show consec 
rem infos for connection safety
netsh advfirewall monitor show currentprofile 
rem currently activated profiles
netsh advfirewall monitor show firewall 
rem aktueller firewallstatus
netsh advfirewall show allprofiles 
rem alle profil eigenschaften
netsh advfirewall show currentprofile 
rem gleiches, nur weniger
netsh advfirewall show domainprofile 
rem domänenprofil
netsh advfirewall show global 
rem globale eigenschaften
netsh advfirewall show store 
rem richtlinienspeicher anzeigen
netsh bridge show adapter 
rem adapter, die als bridge konfiguriert sind
netsh dnsclient show state 
rem status anzeigen, bei mir nicht konfiguriert
netsh dump 
rem da ist was hinter, wahrscheinlich alle skripte
netsh http show cacheparam 
rem Cache-Parameter aufführen
netsh http show cachestate 
rem zwischengespeicherte URI-Ress und deren eigenschaften
netsh http show iplisten 
rem ip-abhörliste
netsh http show servicestate 
rem snapshot des httpdienstes
netsh http show setting 
rem einstellungen
netsh http show sslcert 
rem alle zertifikatverbindungen
netsh http show timeout 
rem timeout-Werte
netsh http show urlacl 
rem URL-Namespacereservierungen
netsh interface 6to4 show interface 
rem ipv6 to v4 konfigurationen
netsh interface 6to4 show relay 
rem ipv6 to v4 relayinfo
netsh interface 6to4 show routing
netsh interface 6to4 show state
netsh interface httpstunnel show interfaces
netsh interface httpstunnel show statistics
netsh interface ipv4 show addresses level=verbose
netsh interface ipv4 show compartments level=verbose 
rem depotparameter
netsh interface ipv4 show config 
rem ip adressen des geräts+ zusatzzeug
netsh interface ipv4 show destinationcache level=verbose 
rem zielcache-einträge
netsh interface ipv4 show dnsservers 
rem configurierte dns-server
netsh interface ipv4 show dynamicportrange protocol=udp
netsh interface ipv4 show dynamicportrange protocol=tcp
netsh interface ipv4 show excludedportrange protocol=udp
netsh interface ipv4 show excludedportrange protocol=tcp
netsh interface ipv4 show global 
rem konfigparameter global
netsh interface ipv4 show icmpstats
netsh interface ipv4 show interfaces level=verbose 
rem interfaceparameter
netsh interface ipv4 show ipaddresses level=verbose
netsh interface ipv4 show ipnettomedia 
rem zuordnung ips zu physikalischem medium
netsh interface ipv4 show ipstats
netsh interface ipv4 show joins level=verbose 
rem zusammengeführte multicastgruppen
netsh interface ipv4 show neighbors level=verbose 
rem nachbaraddressen im cache
netsh interface ipv4 show offload 
rem abladeinfos der schnittstellen
netsh interface ipv4 show route level=verbose 
rem routentabelleneinträge
netsh interface ipv4 show subinterfaces level=verbose 
rem infos zu subschnittstellen
netsh interface ipv4 show tcpconnections 
rem alle aktuellen tcp-verbindungen + statusinfos
netsh interface ipv4 show tcpstats
netsh interface ipv4 show udpconnections 
rem gleiches wie bei tcp nur mit udp
netsh interface ipv4 show udpstats 
netsh interface ipv4 show winsservers 
rem WINS Serverkonfigs
netsh interface isatap show router
netsh interface isatap show state
netsh interface ipv6 show addresses level=verbose
netsh interface ipv6 show compartments level=verbose 
rem depotparameter
netsh interface ipv6 show destinationcache level=verbose 
rem zielcache-einträge
netsh interface ipv6 show dnsservers 
rem configurierte dns-server
netsh interface ipv6 show dynamicportrange protocol=udp
netsh interface ipv6 show dynamicportrange protocol=tcp
netsh interface ipv6 show excludedportrange protocol=udp
netsh interface ipv6 show excludedportrange protocol=tcp
netsh interface ipv6 show global 
rem konfigparameter global
netsh interface ipv6 show interfaces level=verbose 
rem interfaceparameter
netsh interface ipv6 show ipstats
netsh interface ipv6 show joins level=verbose 
rem zusammengeführte multicastgruppen
netsh interface ipv6 show neighbors level=verbose 
rem nachbaraddressen im cache
netsh interface ipv6 show offload 
rem abladeinfos der schnittstellen
netsh interface ipv6 show potentialrouters
netsh interface ipv6 show prefixpolicies
netsh interface ipv6 show privacy
netsh interface ipv6 show route level=verbose 
rem routentabelleneinträge
netsh interface ipv6 show siteprefixes 
rem standortpräfixtabelle
netsh interface ipv6 show subinterfaces level=verbose 
rem infos zu subschnittstellen
netsh interface ipv6 show tcpstats 
netsh interface ipv6 show teredo 
rem zeigt teredozustand 
netsh interface ipv6 show tfofallback 
rem zeigt TCP-Fastopen-Fallbackzustand pro Netzwerk an
netsh interface ipv6 show udpstats
netsh interface portproxy show v4tov6
netsh interface portproxy show v4tov4
netsh interface portproxy show v6tov6
netsh interface portproxy show v6tov4
netsh interface show interface 
rem Zeigt liste mit interfaces an
netsh interface tcp show global
netsh interface tcp show heuristics
netsh interface tcp show rscstats 
rem zeigt tcp-rsc tabelle an
netsh interface tcp show rscstats {idx} 
rem zeigt tcp-rsc details an für idx aus der tabelle vorher
netsh interface tcp show security 
rem sicherheitsparameter
netsh interface tcp show supplemental template=datacenter 
rem zusätzliche Parameter, jeweils vorlagen
netsh interface tcp show supplemental template=internet
netsh interface tcp show supplemental template=compat
netsh interface tcp show supplemental
netsh interface teredo show state 
netsh lan show interfaces
netsh lan show profiles
netsh lan show settings
netsh lan show tracing
netsh mbn show * 
rem stuff about mobile connections
netsh namespace effectivepolicy 
rem namensauflösungs-richtlinie
netsh namespace policy
netsh p2p idmgr show itentities ALL
netsh p2p idmgr show statistics
netsh rpc filter show filter
rem alle rpc firewallfilter
netsh rpc show 
rem benutzerdefinierten Bindungsstatus für ejdes SUbnetz auf dem System
netsh show alias 
rem definierte aliasse
netsh show helper 
rem hilfsprogramm auf oberer Ebene
netsh trace show CaptureFilterHelp 
rem Auflistung der aufzeichungsfilter+ verwendung
netsh trace show globalKeywordsAndLevels 
rem Auflistung globaler Keywords und Ebenen, die eventuell mit dem Startbefehl benutzt werden
netsh trace show interfaces 
rem mögliche Interfaces
netsh trace show providerFilterHelp 
rem ein paar anbieter anzeigen, die das unterstützen
netsh trace show provider name = {finds raus} 
rem providerinfos fürs tracing anzeigen
netsh trace show providerFilterHelp Microsoft-Windows-TCPIP
netsh trace show providerFilterHelp Microsoft-Windows-Ras-NdisWanPacketCapture
netsh trace show scenarios 
rem list configured scenarios
netsh trace show scenoario name = {finds raus}
netsh trace show status 
rem wenn gerade getraced wird, status
netsh wcn query SSID={finds raus} 
rem zeig access-point infos an
netsh winhttp show proxy 
rem für winhttp configurierte Proxys
netsh winsock catalog 
rem Winsock Katalogeinträge
netsh wlan export hostednetworkprofile 
rem xmldatei mit infos, auf Verzeichnis achten
netsh wlan export profile folder={zielordner} key=clear
rem xmldateien mit infos, auf Verzeichnis achten, wenn nicht admin-nutzer ist das pw verschüsselt
netsh wlan show all 
rem alle show-befehle auf einmal, spart zeit
systeminfo /FO CSV 
rem allgemeine systeminfos im CSV-Format