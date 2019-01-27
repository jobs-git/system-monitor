# gtk-system-monitor
C/Gtk/glade system monitor inspired by deepin system mon, task man &amp; gnome system monitor (parts of the codes such as, but not limited to images are pulled from the web)

# Set up
```bash
gcc -o gladewin main.c -Wall `pkg-config --cflags --libs gtk+-3.0` -lcairo -export-dynamic -lm -O2 -lcairo-script-interpreter
./galdewin
```

# Screenshoots
![alt text](https://github.com/jobs-git/gtk-system-monitor/blob/master/Screenshot%20from%202019-01-25%2000-19-51.png)
![alt text](https://github.com/jobs-git/gtk-system-monitor/blob/master/Screenshot%20from%202019-01-25%2000-23-03.png)
