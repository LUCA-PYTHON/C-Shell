# C-Shell

Meine eigene Shell in C. Ich entwickle sie, um besser zu verstehen, was im Hintergrund passiert, wenn man in der Eingabeaufforderung einen Befehl eingibt.

## Was sie kann

- Befehle einlesen und ausführen (`ls`, `grep`, `mkdir` und alles andere, was im `PATH` liegt)
- Eigene Built-ins: `cd`, `pwd`, `echo`, `env`, `export`, `file`, `help` und `exit`
- Exit-Status des letzten Befehls mit `echo $?`
- Eingabe mit Zeileneditor: Pfeiltasten links/rechts zum Bewegen, Backspace, Verlauf der letzten 50 Befehle mit Pfeil hoch/runter
- Argumente in Anführungszeichen (`echo "Hello World"`)
- Verlauf wird in Dateien gespeichert bis zu ca 0.5mb.
- Umleitungen mit `>`, `>>` und `<` sind verfügbar
- Pipes (`|`) zwischen externen Befehlen, z. B. `printf hello | tr a-z A-Z`
- Eingabe auch über eine Pipe oder Datei, z. B. `printf 'echo hi\nexit\n' | ./c-shell` (dann ohne Prompt und ohne Echo)
- Im Prompt steht immer das aktuelle Verzeichnis

Das Projekt entsteht Schritt für Schritt und hilft mir dabei, meine C-Kenntnisse zu vertiefen und die Funktionsweise einer Linux-Shell selbst nachzuvollziehen.

## Befehle

| Befehl | Beschreibung |
|---|---|
| `cd [verzeichnis]` | Verzeichnis wechseln |
| `pwd` | Aktuelles Verzeichnis anzeigen |
| `echo [text]` | Text ausgeben, `echo $?` zeigt den letzten Exit-Status |
| `env [NAME]` | Umgebungsvariablen anzeigen |
| `export NAME=WERT` | Umgebungsvariable setzen |
| `file ...` | Dateien und Ordner bearbeiten (siehe unten) |
| `help` oder `h` | Befehlsübersicht anzeigen |
| `exit [status]` | Shell beenden, optional mit Exit-Status |

Alle anderen Befehle (`ls`, `cat`, `gcc`, ...) werden als externe Programme aus dem `PATH` gestartet.

### Der Befehl `file`

| Aufruf | Beschreibung |
|---|---|
| `file -o <pfad>` | Datei mit hilfe von "xdg-open" im Terminal geöffnet |
| `file -c <pfad>` | Datei anlegen. Fehlende Ordner im Pfad werden automatisch erstellt (`file -c test/test.txt`). |
| `file -w <pfad> <text>` | Text in die Datei schreiben (überschreibt den Inhalt, legt die Datei bei Bedarf an) |
| `file -a <pfad> <text>` | Text an die Datei anhängen (legt die Datei bei Bedarf an) |
| `file -r <pfad>` | Dateiinhalt ausgeben |
| `file -d <pfad>` | Datei löschen (Ordner bleiben bestehen) |
| `file -D <ordner>` | Leeren Ordner löschen. Ordner mit Inhalt werden nicht gelöscht. |

Hinweis: Durch das automatische Anlegen von Ordnern bei `file -c` erzeugt auch ein Tippfehler im Pfad (`tset/test.txt`) einen neuen Ordner.

## Tests

Alle Tests auf einmal:

```bash
make test
```

Einzeln:

```bash
make test-unit          # Unit-Tests für Parser, Builtins, Executor und Input
make test-integration   # Die fertige Shell wird von außen über Pipes getestet
make test-asan          # Unit-Tests mit AddressSanitizer und UBSan
```

## Projektstruktur

Der Code ist in mehrere Dateien aufgeteilt, damit einzelne Aufgaben getrennt entwickelt und getestet werden können.

```text
C-Shell/
├── include/
│   ├── builtins.h          # Eingebaute Befehle
│   ├── executor.h          # Externe Programme starten
│   ├── file_operations.h   # Der Befehl "file"
│   ├── input.h             # Eingabe und Zeileneditor
│   ├── parser.h            # Zeile in Argumente zerlegen
│   └── shell_status.h      # Gemeinsame Statuscodes
├── src/
│   ├── cshell.c            # main() und Hauptschleife
│   ├── builtins.c
│   ├── executor.c
│   ├── file_operations.c
│   ├── input.c
│   └── parser.c
├── tests/
│   ├── unit/
│   └── integration/
├── Makefile
├── LICENSE
└── README.md
```

## Plattformen

Die Shell ist für Linux entwickelt und wird unter Ubuntu in WSL getestet. Da sie Unix-Systemaufrufe wie `fork()`, `execvp()` und `termios` verwendet, läuft sie nicht direkt unter reinem Windows. Eine Windows-Version ist als späteres Ziel geplant.

## Bekannte Einschränkungen

- Keine Verkettung von Befehlen (`;`, `&&`)
- Keine Platzhalter wie `*` und keine Expansion von `~`
- Kein Job-Control (kein `&`, kein `Strg+Z`)

## Branch-Workflow

Änderungen laufen in eine Richtung durch die Branches:

1. `feature/*` von `develop` erstellen und per Pull Request nach `develop` mergen.
2. Getestete Änderungen von `develop` per Pull Request nach `main` übernehmen.
3. `main` bleibt der stabile Stand; direkte Pushes und Force-Pushes auf `main` vermeiden.

## Lizenz

Apache License 2.0, siehe [LICENSE](LICENSE).