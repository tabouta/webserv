# Webserv - Projet 42

## Description

Webserv est une implémentation d’un serveur HTTP en C réalisée dans le cadre du cursus de l’école 42.
Le but du projet est de reproduire le fonctionnement d’un serveur web en gérant les connexions, les requêtes HTTP et les réponses.

---

## Compilation

Pour compiler le projet :

```bash
make
```

Cela génère l’exécutable `webserv`.

---

## Lancement du serveur

Après compilation, lancez le serveur avec le fichier de configuration :

```bash
./webserv webserv.conf
```

Le fichier `webserv.conf` permet de configurer le serveur (ports, routes, comportement, etc.).

---

## Accès au site

Une fois le serveur lancé, ouvrez un navigateur et accédez à :

```
http://localhost:8081/
```

---

## Utilisation

Depuis le navigateur, vous pouvez :

* Accéder aux fichiers servis par le serveur (HTML, CSS, images, etc.)
* Tester les routes définies dans le fichier de configuration
* Envoyer des requêtes HTTP selon les fonctionnalités implémentées (GET, POST, DELETE)
* Vérifier le comportement du serveur face aux erreurs (404, 403, etc.)

---

## Tests

Vous pouvez tester le serveur avec un navigateur ou en ligne de commande :

```bash
curl http://localhost:8081/
```

---

## Notes

* Assurez-vous que le port 8081 est disponible
* Vérifiez la configuration en cas de problème au lancement
* Le comportement dépend des fonctionnalités que vous avez implémentées

---
