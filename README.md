# System Monitor
C/Gtk/glade system monitor inspired by deepin system mon, task man &amp; gnome system monitor (parts of the codes such as, but not limited to images are pulled from the web).

Just learned Gtk/glade/c in one week so please bear with the funny codes (it might still be funny for some time)

At present, it relies on kernel /proc as these is the rawest we can get to the kernel.

# Set up
The application was compiled and developed for a 32 core CentOS 7 machine (I choose this because this is the most stable Linux I have used so far, and I prefer it to be stable in the next 5 years). 
```bash
gcc -o gladewin main.c -Wall `pkg-config --cflags --libs gtk+-3.0` -lcairo -export-dynamic -lm -O2 -lcairo-script-interpreter
./galdewin
```
Since my machine is 32 core, I dont know yet if it could work in other core machine.


# TODO
These are the following things that are needed to be implemented. Well, usually need help here so that it can become a full pledge system monitor and that we might have an alternative to gnome system monitor.

**High priority**
- [x] Making the three buttons in headerbar move in the center like in Gnome System Monitor.
- [x] GtkStack
- [x] GtkStackSwitcher for switching window content
- [x] Process list like in Gnome System Monitor
- [x] Implement sorting process by name
- [x] Implement sorting process by mem usage
- [x] Implement sorting process by cpu usage
- [ ] Dynamic cpu core
- [ ] Implement end task

**Low priority**
- [ ] Create CentOS 7 package & submit to epel
- [ ] Fix file system 
- [ ] Speed up Cairo rendering
- [ ] Implement switch to total cpu & show cpu core graph
- [ ] Implement GtkStack for Ram, Disk & Network

# Screenshoots
Recent
![alt text](https://github.com/jobs-git/system-monitor/blob/master/Screenshot%20from%202019-02-05%2023-17-47.png)
![alt text](https://github.com/jobs-git/system-monitor/blob/master/Screenshot%20from%202019-02-05%2023-18-37.png)
![alt text](https://github.com/jobs-git/system-monitor/blob/master/Screenshot%20from%202019-02-05%2023-17-37.png)


Previous
![alt text](https://github.com/jobs-git/gtk-system-monitor/blob/master/Screenshot%20from%202019-01-25%2000-23-03.png)
![alt text](https://github.com/jobs-git/gtk-system-monitor/blob/master/Screenshot%20from%202019-01-25%2000-19-51.png)
