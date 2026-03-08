# Daemon Processes

## Exercises

### 13.1

**As we might guess from Figure 13.2, when the `syslog` facility is initialized,
either by calling openlog directly or on the first call to `syslog`,
the special device file for the UNIX domain datagram socket, `/dev/log`, has to be opened.
What happens if the user process (the daemon) calls `chroot` before calling `openlog`?**

Remember that this is not changing the current working directory, but changing the root, as the name of the function implies.
As such, if I for example chrooted to `/home/daniel`, if I then opened `/`, that `/` refers to `/home/daniel`.

Unless the program is chrooting to the actual original root `/`, then you won't be able to access `/dev/log`,
because the daemon will no longer have access to the true absolute root of the file system.

The solution is for the daemon to call `openlog` with an option of `LOG_NDELAY`, before calling `chroot`.
This opens the special device file (the UNIX domain datagram socket),
    yielding a descriptor that is still valid, even after a call to `chroot`.
This scenario is encountered in daemons, such as `ftpd` (the File Transfer Protocol daemon),
    that specifically call `chroot` for security reasons but still need to call `syslog` to log error conditions.

### 13.2

**Recall the sample `ps` output from Section 13.2.
The only user-level daemon that isn't a session leader is the `rsyslogd` process.
Explain why the `rsyslogd` daemon isn't a session leader.**

`syslogd` is not a session leader because it forks again after calling `setsid()`.
The child process that remains running is therefore no longer a session leader,
    which prevents it from ever acquiring a controlling terminal again.

### 13.3

**List all the daemons active on your system, and identify the function of each one.**

The active system daemons on my machine include:

- init/systemd: system initialization and service manager
- systemd-journald: system log collection
- systemd-udevd: device event manager
- systemd-resolved: DNS resolver
- systemd-timesyncd: time synchronization
- cron: scheduled job execution
- NetworkManager: network connection manager
- wpa_supplicant: Wi-Fi authentication
- rsyslogd: syslog service
- sshd: SSH remote login server
- cupsd: print service
- gdm3: graphical login manager
- polkitd: authorization service
- snapd: Snap package manager daemon
- mysqld: MySQL database server
- udisksd: disk management daemon
- ModemManager: modem management service
- avahi-daemon: Zeroconf/mDNS service discovery
- upowerd: power management service
- fwupd: firmware update daemon

The process list also includes many kernel threads, such as `[kthreadd]` and `[kworker/...]`, which are not user-level daemons.
It also includes many user-session background services, such as `pipewire`, `ibus-daemon`, and `gnome-keyring-daemon`.

### 13.4

**Write a program that calls the daemonize function in Figure 13.1.
After calling this function, call `getlogin` (Section 8.15)
    to see whether the process has a login name now that it has become a daemon.
Print the results to a file.**

[ex13_4.c](ex13_4.c)

On Linux, it does not work and still show the login name.

On Linux, this can still happen because glibc's `getlogin()` does not strictly follow POSIX.

Although a daemon normally loses its controlling terminal after `setsid()`,
    the Linux manual notes that glibc `getlogin` uses standard input (`stdin`) instead of `/dev/tty`,
    and it may also consult the utmp login record.
So `getlogin()` can still return a username even after daemonization.

In other words: the login name does not prove the daemon still has a controlling terminal;
    it is mostly a Linux/glibc implementation [quirk](https://man7.org/linux/man-pages/man3/getlogin.3.html).
