# C-Shell

Meine eigene kleine Shell in C. Ich habe sie gebaut, um besser zu verstehen, was im Hintergrund passiert, wenn man im Terminal einen Befehl eintippt.

Sie kann nicht viel, aber das, was sie kann, habe ich selbst geschrieben und verstanden.

## Was sie kann

- Befehle einlesen und ausführen (`ls`, `grep`, `mkdir` und alles andere, was im `PATH` liegt)
- Eigene Built-ins: `cd`, `pwd` und `exit`
- Im Prompt steht immer das aktuelle Verzeichnis

## Ausprobieren

Du brauchst Linux (getestet unter Ubuntu) oder WSL und einen C-Compiler:

```bash
sudo apt install build-essential
gcc -Wall -Wextra -o cshell cshell.c
./cshell
```

Dann kannst du ganz normal Befehle eintippen:

```
c-shell [/home/test]> ls
c-shell [/home/test]> cd ..
c-shell [/home]> pwd
c-shell [/home]> exit
```

## Per Suche starten (optional)

Wenn du die Shell wie ein normales Programm über die Ubuntu-Suche öffnen willst:

```bash
mkdir -p ~/.local/bin ~/.local/share/applications
cp cshell ~/.local/bin/

cat > ~/.local/share/applications/c-shell.desktop << EOF
[Desktop Entry]
Type=Application
Name=C-Shell
Comment=Meine selbstgebaute Shell in C
Exec=$HOME/.local/bin/cshell
Terminal=true
Categories=System;TerminalEmulator;
EOF

update-desktop-database ~/.local/share/applications
```

Danach einfach die Super-Taste drücken und "C-Shell" eintippen.

## Hinweis zu Windows

Unter reinem Windows läuft der Code nicht, weil `fork()` und `unistd.h` dort nicht existieren. Mit WSL funktioniert alles. Eine native Windows-Version ist als späteres Ziel geplant.