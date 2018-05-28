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

Youtube : https://console.developers.google.com/apis
Facebook : https://developers.facebook.com/
Instagram : https://www.instagram.com/developer/

Pour Twitter seul le nom d'utilisateur suffit, pas besoin de clé d'API.

Si vous avez des idées d'améliorations n'hésitez pas à les soumettre ;)
