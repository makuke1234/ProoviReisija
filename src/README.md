# Proovireisija

> Tarkvara projekt, mille eesmärk on ära lahendada proovireisija ülesanne.

[![Release version](https://img.shields.io/github/v/release/makuke1234/ProoviReisija?display_name=release&include_prereleases)](https://github.com/makuke1234/ProoviReisija/releases/latest)
[![Total downloads](https://img.shields.io/github/downloads/makuke1234/ProoviReisija/total)](https://github.com/makuke1234/ProoviReisija/releases)
![C version](https://img.shields.io/badge/version-C99-blue.svg)
![Tests](https://img.shields.io/badge/build-none-red.svg)


# Praegune plaan

Esialgu prooviks ülesande ära lahendada graafiteoreetiliselt, programmile antakse ristmike xy-koordinaadid
(ristmikel on identifikaatorid) ning info millised ristmikud on omavahel ühendatud. Esialgu katsetaks
väikeste proovifailidega. Samuti antakse programmile kuni 10 xy-koordinaati, kus on vaja peatuda, algus- ja lõpp-punkt jäävad alati paika.
Programm leiaks, millised teed on lähimad antud punktidele, teedele lisatakse peatused uute vahepunktidena/graafi tippudena.
Programm leiaks kasutades [Dijkstra algoritmi](https://en.wikipedia.org/wiki/Dijkstra%27s_algorithm) lühimad kaugused soovitud punktide kõikvõimalike kombinatsioonide vahel.
Nendest lühimatest kaugustest tekib N*N maatriks (N sihtpunkti). Seal saab hakata järjest läbi proovima, millises järjekorras punkte läbides
tekiks optimaalne teekond.

![Ajakava](./images/ajakava.png)


