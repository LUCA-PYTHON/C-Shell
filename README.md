# C-Shell

Meine eigene kleine Shell in C. Ich entwickle sie, um besser zu verstehen, was im Hintergrund passiert, wenn man im Terminal einen Befehl eingibt.

Sie kann nicht viel, aber das, was sie kann, habe ich selbst geschrieben und verstanden.

## Was sie kann

- Befehle einlesen und ausführen (`ls`, `grep`, `mkdir` und alles andere, was im `PATH` liegt)
- Eigene Built-ins: `cd`, `pwd`, `echo`, `help` und `exit`
- Im Prompt steht immer das aktuelle Verzeichnis

Das Projekt entsteht Schritt für Schritt und hilft mir dabei, meine C-Kenntnisse zu vertiefen und die Funktionsweise einer Linux-Shell selbst nachzuvollziehen.

## Ausprobieren

Du benötigst Linux oder Windows mit WSL und Ubuntu sowie GCC und GNU Make.

```bash
sudo apt update
sudo apt install build-essential
```

Im Projektordner kannst du die Shell kompilieren und starten:

```bash
make
./c-shell
```

Beenden kannst du sie mit:

```text
exit
```

## Projektstruktur

Der Code ist in mehrere Dateien aufgeteilt, damit einzelne Aufgaben getrennt entwickelt und gewartet werden können.

```text
C-Shell/
├── include/
├── src/
├── Makefile
└── .gitignore
```

## Plattformen

Die Shell ist für Linux entwickelt und wird unter Ubuntu in WSL getestet. Da sie Unix-Systemaufrufe wie `fork()` und `execvp()` verwendet, läuft sie nicht direkt unter reinem Windows. Eine native Windows-Version ist als späteres Ziel geplant.

## Entwicklungsstand

Das Projekt befindet sich in Entwicklung. Neue Funktionen und Verbesserungen werden schrittweise umgesetzt, während ich die Grundlagen von C und Linux vertiefe.