# HeadTracker v2.40 Expo — installation et retour arrière

Cette variante est construite depuis la v2.40 officielle, commit a9ea5ca6e58a34c434aad5dc7ce641dbf03741bb. Le commit des modifications se trouve dans SOURCE_COMMIT.txt. Elle ne constitue pas une version officielle du projet HeadTracker.

1. Avec l'ancienne application, connecter chaque carte et enregistrer ses réglages avec **File / Save to File**. Garder un fichier distinct pour l'émetteur porté sur la tête et le récepteur.
2. Installer **HeadTracker-Expo-Setup.exe**, puis lancer **HeadTracker Expo**. L'installation ne programme aucune carte. Le paquet portable fonctionne aussi après extraction complète.
3. Poser le matériel sur le banc, alimentation de propulsion débranchée. Dans l'application, ouvrir **Firmware Wizard**, choisir **Local File**, puis le fichier du dossier `firmware` correspondant exactement à la carte : **Nano33BLE.bin** pour l'originale, **Nano33BLE-Rev2.bin** pour la Rev2 (Sense Rev2). Ne pas les intervertir.
4. Cliquer **Program**, suivre l'assistant et passer en bootloader par un double appui sur RESET si nécessaire. La variante utilise le bouton de recentrage sur **D3**, comme la configuration v2.40 source. Un montage utilisant D2 doit être adapté ou conserver son firmware D2 : ce paquet n'est pas une variante D2.
5. Reconnecter la carte. Restaurer sa sauvegarde avec **File / Load**, vérifier la calibration, les axes, les gains, les centres, les limites et le sens de déplacement. Les anciennes sauvegardes donnent **0 % d'Expo sur les trois axes**.
6. Ouvrir **Expo / Courbes...**, régler chaque axe, valider avec **OK**, puis cliquer **Save Settings**. Annuler ne change rien. Débrancher/rebrancher la carte et vérifier que les trois valeurs sont conservées.
7. Tester sur banc toute la chaîne utilisée : tête → Bluetooth/PPM/SBUS/CRSF → radio/récepteur → caméra. Vérifier recentrage, limites, absence de mouvements brusques, coupure/rétablissement de liaison et d'alimentation. Ne pas passer au vol avant ces essais.

L'Expo s'applique aux axes calculés par la carte qui mesure la tête. Les canaux reçus et relayés ne reçoivent pas une seconde courbe. Sur une carte réceptrice, vérifier les affectations d'axes et les canaux relayés avant de changer son firmware ; sa mise à jour n'est pas nécessaire à l'Expo de l'émetteur.

## Retour à la v2.40 officielle

Télécharger **HT_2.40.Firmwares.zip** et **HeadTrackerGUI_x64-v2.40.zip** depuis :
https://github.com/headtracker/HeadTracker/releases/tag/v2.40

Extraire le ZIP. Dans Firmware Wizard / Local File, sélectionner le firmware officiel correspondant à la révision exacte et au bouton de recentrage **D3** ou **D2** de la carte. Programmer volontairement comme ci-dessus, reconnecter avec la GUI officielle, restaurer la sauvegarde faite avant la modification et cliquer **Save Settings**. Vérifier la calibration et les mouvements sur banc. Désinstaller l'application Expo seul ne restaure pas le firmware.

## Limites de validation

Les tests automatisés contrôlent le calcul, le modèle de paramètres, les messages, la sérialisation, l'application et l'installateur selon le rapport associé à la compilation. Ils ne remplacent pas les essais des capteurs, de la flash physique, des liaisons radio, du câblage et des servos. L'indication Save Settings de l'application ne prouve pas à elle seule la conservation après une coupure ; la relecture après redémarrage est indispensable.
