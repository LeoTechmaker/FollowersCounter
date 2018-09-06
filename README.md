# FollowersCounter

Voici le code du compteur d'abonnés, présenté dans la vidéo "FABRIQUER UN COMPTEUR YOUTUBE / FACEBOOK / TWITTER ? [DIY]" (https://www.youtube.com/watch?v=8_9clGGASNQ)
Puis amélioré dans la vidéo "J'AMÉLIORE LE TROPHÉE YOUTUBE ?!" (https://www.youtube.com/watch?v=sWKcnsrpM98)

Les LEDs sont connectées à la broche D8.

Il suffit de modifier le fichier "Config.h" pour y indiquer les ID de pages ainsi que les différentes clés d'API.
Ensuite, on envoie le code un ESP8266, et voilà.

Une fois le programme démarré, on peut accéder aux paramètres en se connectant à l'ESP8266 grâce à son adresse IP ainsi qu'aux identifients indiqués dans le fichier "Config.h".
Pour connaitre son IP, on peut se connecter à l'ESP avec un câble USB puis ouvrir la liaison série, et effectuer un reset. Au démarrage, le programme envoie l'IP sur la liaison série.
On peut aussi utiliser une appli comme "Fing" pour effectuer un scan du réseau.

Une fois le programme envoyé une première fois, les mises à jour peuvent être effectuées à travers le réseau (OTA) grâce au mot de passe indiqué dans le fichier "Config.h".

Pour récupérer les clés d'API :

* Youtube : https://console.developers.google.com/apis
* Facebook : https://developers.facebook.com/
* Instagram : https://www.instagram.com/developer/

Pour Twitter seul le nom d'utilisateur suffit, pas besoin de clé d'API.

Installation du core Arduino ESP8266 dans une version différente de 2.4.1 (détail plus bas dans la description) : https://github.com/esp8266/Arduino

Librairies nécessaires :
* "ArduinoJson" par bblanchon dans une version < 6 (détail plus bas dans la description) : https://github.com/bblanchon/ArduinoJson/releases/tag/v5.13.2
* "arduino-youtube-api" par witnessmenow : https://github.com/witnessmenow/arduino-youtube-api
* "arduino-facebook-api" par witnessmenow : https://github.com/witnessmenow/arduino-facebook-api
* "Adafruit_NeoPixel" par adafruit : https://github.com/adafruit/Adafruit_NeoPixel


Si vous avez des idées d'améliorations n'hésitez pas à les soumettre ;)


Pourquoi faut-il une version du core Arduino ESP8266 dans une version différente de 2.4.1 ?
Car cette version contient une fuite de mémoire. le programme fonctionne mais finit par crasher après un peu de temps. Après chaque requête, la quantité de RAM libre baisse un peu. L'appel à la fonction HTTPClient.GET() provoque un crash si la quantité disponible de RAM est trop faible. Les requêtes avec prise en charge du SSL demandent pas mal de RAM.
Le problème a été corrigé dans la version 2.4.2, attention donc à bien être à jour.

Pourquoi faut-il une version de "ArduinoJson" dans une version inférieure à 6 ?
Car à la version 6 (et donc pour les versions ultérieures), des modifications changeant la façon dont s'utilise la librairie ont été effectuées. Le code écrit pour une version 5.x n'est donc pas utilisable avec la version 6.
Il faudrait adapter le code si on voulait qu'il soit compatible avec les versions 6 et ultérieures.

Une erreur avec Facebook ? Le problème le plus courant est dû à un problème de la librairie FacebookApi avec les nouvelles clés.
En attendant qu'il soit résolu sur la release, vous pouvez faire fonctionner en modifiant la ligne 153 du fichier "FacebookApi.cpp"
de ça
```sh
String command = "/v2.9/" + pageId + "?fields=fan_count&access_token=" + _appId + "|" + _appSecret;
```
à ça
```sh
String command = "/v2.9/" + pageId + "?fields=fan_count&access_token=" + _accessToken;
```
